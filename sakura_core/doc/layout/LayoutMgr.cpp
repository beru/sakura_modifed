#include "StdAfx.h"
#include "doc/layout/LayoutMgr.h"
#include "doc/layout/Layout.h"
#include "doc/EditDoc.h"
#include "doc/DocReader.h"
#include "doc/DocEditor.h"
#include "doc/logic/DocLine.h"
#include "doc/logic/DocLineMgr.h"
#include "charset/charcode.h"
#include "mem/Memory.h"
#include "mem/MemoryIterator.h"
#include "basis/SakuraBasis.h"
#include "SearchAgent.h"
#include "debug/RunningTimer.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        生成と破棄                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

LayoutMgr::LayoutMgr()
	:
	getIndentOffset(&LayoutMgr::getIndentOffset_Normal)
{
	pDocLineMgr = nullptr;
	pTypeConfig = nullptr;
	nMaxLineKetas = MAXLINEKETAS;
	nTabSpace = 4;
	pszKinsokuHead_1.clear();				// 行頭禁則
	pszKinsokuTail_1.clear();				// 行末禁則
	pszKinsokuKuto_1.clear();				// 句読点ぶらさげ

	nTextWidth = 0;			// テキスト最大幅の記憶
	nTextWidthMaxLine = 0;	// 最大幅のレイアウト行

	Init();
}


LayoutMgr::~LayoutMgr()
{
	_Empty();

	pszKinsokuHead_1.clear();	// 行頭禁則
	pszKinsokuTail_1.clear();	// 行末禁則
	pszKinsokuKuto_1.clear();	// 句読点ぶらさげ
}


/*
||
|| 行データ管理クラスのポインタを初期化します
||
*/
void LayoutMgr::Create(
	EditDoc* pEditDoc,
	DocLineMgr* pDocLineMgr
	)
{
	_Empty();
	Init();
	// Jun. 20, 2003 genta EditDocへのポインタ追加
	this->pEditDoc = pEditDoc;
	this->pDocLineMgr = pDocLineMgr;
}


void LayoutMgr::Init()
{
	pLayoutTop = nullptr;
	pLayoutBot = nullptr;
	nPrevReferLine = 0;
	pLayoutPrevRefer = nullptr;
	nLines = 0;			// 全物理行数
	nLineTypeBot = COLORIDX_DEFAULT;

	// EOFレイアウト位置記憶
	nEOFLine = -1;
	nEOFColumn = -1;
}


void LayoutMgr::_Empty()
{
	Layout* pLayout = pLayoutTop;
	while (pLayout) {
		Layout* pLayoutNext = pLayout->GetNextLayout();
		delete pLayout;
		pLayout = pLayoutNext;
	}
}


/*! レイアウト情報の変更
	@param bDoLayout [in] レイアウト情報の再作成
	@param refType [in] タイプ別設定
*/
void LayoutMgr::SetLayoutInfo(
	bool				bDoLayout,
	const TypeConfig&	refType,
	size_t				nTabSpace,
	size_t				nMaxLineKetas
	)
{
	MY_RUNNINGTIMER(runningTimer, "LayoutMgr::SetLayoutInfo");

	assert_warning((!bDoLayout && nMaxLineKetas == nMaxLineKetas) || bDoLayout);
	assert_warning((!bDoLayout && nTabSpace == refType.nTabSpace) || bDoLayout);

	// タイプ別設定
	pTypeConfig = &refType;
	nMaxLineKetas = nMaxLineKetas;
	nTabSpace = nTabSpace;

	// タイプによって処理関数を変更する
	// 数が増えてきたらテーブルにすべき
	switch (refType.nIndentLayout) {	// 折り返しは2行目以降を字下げ表示
	case 1:
		getIndentOffset = &LayoutMgr::getIndentOffset_Tx2x;
		break;
	case 2:
		getIndentOffset = &LayoutMgr::getIndentOffset_LeftSpace;
		break;
	default:
		getIndentOffset = &LayoutMgr::getIndentOffset_Normal;
		break;
	}

	// 句読点ぶら下げ文字
	pszKinsokuKuto_1.clear();
	if (refType.bKinsokuKuto) {	// bKinsokuKutoで振り分ける(Fix)
		for (const wchar_t* p=refType.szKinsokuKuto; *p; ++p) {
			pszKinsokuKuto_1.push_back_unique(*p);
		}
	}

	// 行頭禁則文字
	pszKinsokuHead_1.clear();
	for (const wchar_t* p=refType.szKinsokuHead; *p; ++p) {
		if (pszKinsokuKuto_1.exist(*p)) {
			continue;
		}else {
			pszKinsokuHead_1.push_back_unique(*p);
		}
	}

	// 行末禁則文字
	pszKinsokuTail_1.clear();
	for (const wchar_t* p=refType.szKinsokuTail; *p; ++p) {
		pszKinsokuTail_1.push_back_unique(*p);
	}

	// レイアウト
	if (bDoLayout) {
		_DoLayout();
	}
}


/*!
	@brief 指定された物理行のレイアウト情報を取得

	@param nLineNum [in] 物理行番号 (0～)
*/
const Layout* LayoutMgr::SearchLineByLayoutY(
	size_t nLineLayout
	) const
{
	if (nLines == 0) {
		return nullptr;
	}
	size_t nLineNum = nLineLayout;
	Layout*	pLayout;
	size_t nCount;

//	// +++++++ 低速版 +++++++++
//	if (nLineNum < (nLines / 2)) {
//		nCount = 0;
//		pLayout = pLayoutTop;
//		while (pLayout) {
//			if (nLineNum == nCount) {
//				pLayoutPrevRefer = pLayout;
//				nPrevReferLine = nLineNum;
//				return pLayout;
//			}
//			pLayout = pLayout->GetNextLayout();
//			++nCount;
//		}
//	}else {
//		nCount = nLines - 1;
//		pLayout = pLayoutBot;
//		while (pLayout) {
//			if (nLineNum == nCount) {
//				pLayoutPrevRefer = pLayout;
//				nPrevReferLine = nLineNum;
//				return pLayout;
//			}
//			pLayout = pLayout->GetPrevLayout();
//			--nCount;
//		}
//	}


	// +++++++わずかに高速版+++++++
	// pLayoutPrevReferより、Top,Botのほうが近い場合は、そちらを利用する
	int nPrevToLineNumDiff = t_abs(nPrevReferLine - (int)nLineNum);
	if (0
		|| !pLayoutPrevRefer
		|| (int)nLineNum < nPrevToLineNumDiff
		|| (int)nLines - (int)nLineNum < nPrevToLineNumDiff
	) {
		if (nLineNum < (nLines / 2)) {
			nCount = 0;
			pLayout = pLayoutTop;
			while (pLayout) {
				if (nLineNum == nCount) {
					pLayoutPrevRefer = pLayout;
					nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->GetNextLayout();
				++nCount;
			}
		}else {
			nCount = nLines - 1;
			pLayout = pLayoutBot;
			while (pLayout) {
				if (nLineNum == nCount) {
					pLayoutPrevRefer = pLayout;
					nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->GetPrevLayout();
				--nCount;
			}
		}
	}else {
		if (nLineNum == nPrevReferLine) {
			return pLayoutPrevRefer;
		}else if ((int)nLineNum > nPrevReferLine) {
			nCount = nPrevReferLine + 1;
			pLayout = pLayoutPrevRefer->GetNextLayout();
			while (pLayout) {
				if (nLineNum == nCount) {
					pLayoutPrevRefer = pLayout;
					nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->GetNextLayout();
				++nCount;
			}
		}else {
			nCount = nPrevReferLine - 1;
			pLayout = pLayoutPrevRefer->GetPrevLayout();
			while (pLayout) {
				if (nLineNum == nCount) {
					pLayoutPrevRefer = pLayout;
					nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->GetPrevLayout();
				--nCount;
			}
		}
	}
	return nullptr;
}


void LayoutMgr::AddLineBottom(Layout* pLayout)
{
	if (nLines == 0) {
		pLayoutBot = pLayoutTop = pLayout;
		pLayoutTop->pPrev = nullptr;
	}else {
		pLayoutBot->pNext = pLayout;
		pLayout->pPrev = pLayoutBot;
		pLayoutBot = pLayout;
	}
	pLayout->pNext = nullptr;
	++nLines;
	return;
}

Layout* LayoutMgr::InsertLineNext(
	Layout* pLayoutPrev,
	Layout* pLayout
	)
{
	if (nLines == 0) {
		// 初
		pLayoutBot = pLayoutTop = pLayout;
		pLayoutTop->pPrev = nullptr;
		pLayoutTop->pNext = nullptr;
	}else if (!pLayoutPrev) {
		// 先頭に挿入
		pLayoutTop->pPrev = pLayout;
		pLayout->pPrev = nullptr;
		pLayout->pNext = pLayoutTop;
		pLayoutTop = pLayout;
	}else
	if (!pLayoutPrev->GetNextLayout()) {
		// 最後に挿入
		pLayoutBot->pNext = pLayout;
		pLayout->pPrev = pLayoutBot;
		pLayout->pNext = nullptr;
		pLayoutBot = pLayout;
	}else {
		// 途中に挿入
		Layout* pLayoutNext = pLayoutPrev->GetNextLayout();
		pLayoutPrev->pNext = pLayout;
		pLayoutNext->pPrev = pLayout;
		pLayout->pPrev = pLayoutPrev;
		pLayout->pNext = pLayoutNext;
	}
	++nLines;
	return pLayout;
}

/* Layoutを作成する */
Layout* LayoutMgr::CreateLayout(
	DocLine*		pDocLine,
	Point			ptLogicPos,
	int				nLength,
	EColorIndexType	nTypePrev,
	int				nIndent,
	int				nPosX,
	LayoutColorInfo*	colorInfo
	)
{
	Layout* pLayout = new Layout(
		pDocLine,
		ptLogicPos,
		nLength,
		nTypePrev,
		nIndent,
		colorInfo
	);
	auto& eol = pDocLine->GetEol();
	if (eol == EolType::None) {
		pLayout->eol.SetType(EolType::None);	// 改行コードの種類
	}else {
		if (pLayout->GetLogicOffset() + pLayout->GetLengthWithEOL() >
			pDocLine->GetLengthWithEOL() - eol.GetLen()
		) {
			pLayout->eol = eol;	// 改行コードの種類
		}else {
			pLayout->eol = EolType::None;	// 改行コードの種類
		}
	}

	// 「折り返さない」選択時のみレイアウト長を記憶する
	// 「折り返さない」以外で計算しないのはパフォーマンス低下を防ぐ目的なので、
	// パフォーマンスの低下が気にならない程なら全ての折り返し方法で計算する
	// ようにしても良いと思う。
	// （その場合LayoutMgr::CalculateTextWidth()の呼び出し箇所をチェック）
	pLayout->SetLayoutWidth((pEditDoc->nTextWrapMethodCur == TextWrappingMethod::NoWrapping) ? nPosX : 0);

	return pLayout;
}


/*
|| 指定された物理行のデータへのポインタとその長さを返す Ver0
*/
const wchar_t* LayoutMgr::GetLineStr(
	size_t nLine,
	size_t* pnLineLen
	) const //#####いらんやろ
{
	const Layout* pLayout;
	if (!(pLayout = SearchLineByLayoutY(nLine))) {
		return NULL;
	}
	*pnLineLen = pLayout->GetLengthWithEOL();
	return pLayout->GetDocLineRef()->GetPtr() + pLayout->GetLogicOffset();
}

/*!	指定された物理行のデータへのポインタとその長さを返す Ver1 */
const wchar_t* LayoutMgr::GetLineStr(
	size_t nLine,
	size_t* pnLineLen,
	const Layout** ppcLayoutDes
	) const
{
	if (!((*ppcLayoutDes) = SearchLineByLayoutY(nLine))) {
		return NULL;
	}
	*pnLineLen = (*ppcLayoutDes)->GetLengthWithEOL();
	return (*ppcLayoutDes)->pDocLine->GetPtr() + (*ppcLayoutDes)->GetLogicOffset();
}

/*
|| 指定された位置がレイアウト行の途中の行末かどうか調べる
*/
bool LayoutMgr::IsEndOfLine(
	const Point& ptLinePos
	)
{
	const Layout* pLayout;

	if (!(pLayout = SearchLineByLayoutY(ptLinePos.y))) {
		return false;
	}

	if (pLayout->GetLayoutEol().GetType() == EolType::None) {
		// この行に改行はない
		// この行の最後か？
		if (ptLinePos.x == pLayout->GetLengthWithEOL()) {
			return true; //$$ 単位混在
		}
	}

	return false;
}

/*!	@brief ファイル末尾のレイアウト位置を取得する

	ファイル末尾まで選択する場合に正確な位置情報を与えるため

	既存の関数では物理行からレイアウト位置を変換する必要があり，
	処理に無駄が多いため，専用関数を作成
*/
void LayoutMgr::GetEndLayoutPos(
	Point* ptLayoutEnd // [out]
	)
{
	if (nEOFLine != -1) {
		ptLayoutEnd->x = nEOFColumn;
		ptLayoutEnd->y = nEOFLine;
		return;
	}

	if (nLines == 0 || !pLayoutBot) {
		// データが空
		ptLayoutEnd->x = 0;
		ptLayoutEnd->y = 0;
		nEOFColumn = 0;
		nEOFLine = 0;
		return;
	}

	Layout* btm = pLayoutBot;
	if (btm->eol != EolType::None) {
		// 末尾に改行がある
		ptLayoutEnd->Set(0, (int)GetLineCount());
	}else {
		MemoryIterator it(btm, GetTabSpace());
		while (!it.end()) {
			it.scanNext();
			it.addDelta();
		}
		ptLayoutEnd->Set((int)it.getColumn(), (int)GetLineCount() - 1);
	}
	nEOFColumn = ptLayoutEnd->x;
	nEOFLine = ptLayoutEnd->y;
}


// 論理行の指定範囲に該当するレイアウト情報を削除して
// 削除した範囲の直前のレイアウト情報のポインタを返す
Layout* LayoutMgr::DeleteLayoutAsLogical(
	Layout*	pLayoutInThisArea,
	int			nLineOf_pLayoutInThisArea,
	int			nLineFrom,
	int			nLineTo,
	Point		ptDelLogicalFrom,
	size_t*		pnDeleteLines
	)
{
	*pnDeleteLines = 0;
	if (nLines == 0) {	// 全物理行数
		return nullptr;
	}
	if (!pLayoutInThisArea) {
		return nullptr;
	}

	pLayoutPrevRefer = pLayoutInThisArea->GetPrevLayout();
	nPrevReferLine = nLineOf_pLayoutInThisArea - 1;

	// 範囲内先頭に該当するレイアウト情報をサーチ
	Layout* pLayoutWork = pLayoutInThisArea->GetPrevLayout();
	while (pLayoutWork && nLineFrom <= pLayoutWork->GetLogicLineNo()) {
		pLayoutWork = pLayoutWork->GetPrevLayout();
	}

	Layout* pLayout = pLayoutWork ? pLayoutWork->GetNextLayout() : pLayoutTop;
	while (pLayout) {
		if (pLayout->GetLogicLineNo() > nLineTo) {
			break;
		}
		Layout* pLayoutNext = pLayout->GetNextLayout();
		if (!pLayoutWork) {
			// 先頭行の処理
			pLayoutTop = pLayout->GetNextLayout();
			if (pLayout->GetNextLayout()) {
				pLayout->pNext->pPrev = nullptr;
			}
		}else {
			pLayoutWork->pNext = pLayout->GetNextLayout();
			if (pLayout->GetNextLayout()) {
				pLayout->pNext->pPrev = pLayoutWork;
			}
		}
		if (0
			|| (1
				&& ptDelLogicalFrom.y == pLayout->GetLogicLineNo()
				&& ptDelLogicalFrom.x < pLayout->GetLogicOffset() + (int)pLayout->GetLengthWithEOL()
			)
			|| (ptDelLogicalFrom.y < pLayout->GetLogicLineNo())
		) {
			(*pnDeleteLines)++;
		}

		if (pLayoutPrevRefer == pLayout) {
			DEBUG_TRACE(_T("バグバグ\n"));
		}

		delete pLayout;

		--nLines;	// 全物理行数
		if (!pLayoutNext) {
			pLayoutBot = pLayoutWork;
		}
		pLayout = pLayoutNext;
	}
//	MYTRACE(_T("(*pnDeleteLines)=%d\n"), (*pnDeleteLines));

	return pLayoutWork;
}


// 指定行より後の行のレイアウト情報について、論理行番号を指定行数だけシフトする
// 論理行が削除された場合は０より小さい行数
// 論理行が挿入された場合は０より大きい行数
void LayoutMgr::ShiftLogicalLineNum(
	Layout* pLayoutPrev,
	int nShiftLines
	)
{
	MY_RUNNINGTIMER(runningTimer, "LayoutMgr::ShiftLogicalLineNum");

	if (nShiftLines == 0) {
		return;
	}
	Layout* pLayout = pLayoutPrev ? pLayoutPrev->GetNextLayout() : pLayoutTop;
	// レイアウト情報全体を更新する(なな、なんと!!!)
	while (pLayout) {
		pLayout->OffsetLogicLineNo(nShiftLines);	// 対応する論理行番号
		pLayout = pLayout->GetNextLayout();
	}
	return;
}

bool LayoutMgr::ChangeLayoutParam(
	size_t nTabSize,
	size_t nMaxLineKetas
	)
{
	if (nTabSize < 1 || nTabSize > 64) {
		return false;
	}
	if (nMaxLineKetas < MINLINEKETAS || nMaxLineKetas > MAXLINEKETAS) {
		return false;
	}

	nTabSpace = nTabSize;
	nMaxLineKetas = nMaxLineKetas;

	_DoLayout();

	return true;
}


// 現在位置の単語の範囲を調べる
bool LayoutMgr::WhereCurrentWord(
	size_t		nLineNum,
	size_t		nIdx,
	Range*		pSelect,		// [out]
	NativeW*	pcmcmWord,		// [out]
	NativeW*	pcmcmWordLeft	// [out]
	)
{
	const Layout* pLayout = SearchLineByLayoutY(nLineNum);
	if (!pLayout) {
		return false;
	}

	// 現在位置の単語の範囲を調べる -> ロジック単位pSelect, pMemWord, pMemWordLeft
	size_t nFromX;
	size_t nToX;
	bool ret = SearchAgent(*pDocLineMgr).WhereCurrentWord(
		pLayout->GetLogicLineNo(),
		pLayout->GetLogicOffset() + nIdx,
		&nFromX,
		&nToX,
		pcmcmWord,
		pcmcmWordLeft
	);

	if (ret) {
		// 論理位置→レイアウト位置変換
		Point ptFrom = LogicToLayout(Point((int)nFromX, pLayout->GetLogicLineNo()), nLineNum);
		Point ptTo = LogicToLayout(Point((int)nToX, pLayout->GetLogicLineNo()), nLineNum);
		pSelect->SetFrom(ptFrom);
		pSelect->SetTo(ptTo);
	}
	return ret;

}


// 現在位置の左右の単語の先頭位置を調べる
bool LayoutMgr::PrevOrNextWord(
	size_t	nLineNum,
	size_t	nIdx,
	Point*	pptLayoutNew,
	bool	bLeft,
	bool	bStopsBothEnds
	)
{
	const Layout* pLayout = SearchLineByLayoutY(nLineNum);
	if (!pLayout) {
		return false;
	}

	// 現在位置の左右の単語の先頭位置を調べる
	size_t nPosNew;
	bool ret = SearchAgent(*pDocLineMgr).PrevOrNextWord(
		pLayout->GetLogicLineNo(),
		pLayout->GetLogicOffset() + nIdx,
		&nPosNew,
		bLeft,
		bStopsBothEnds
	);

	if (ret) {
		// 論理位置→レイアウト位置変換
		*pptLayoutNew = LogicToLayout(
			Point((int)nPosNew, pLayout->GetLogicLineNo()),
			nLineNum
		);
	}
	return ret;
}


// 単語検索
/*
	@retval 0 見つからない
*/
bool LayoutMgr::SearchWord(
	size_t nLine,						// [in] 検索開始レイアウト行
	size_t nIdx,						// [in] 検索開始データ位置
	SearchDirection searchDirection,	// [in] 検索方向
	Range* pMatchRange,					// [out] マッチレイアウト範囲
	const SearchStringPattern& pattern
	)
{
	const Layout* pLayout = this->SearchLineByLayoutY(nLine);
	if (!pLayout) {
		return false;
	}

	// 単語検索 -> logicRange (データ位置)
	Range logicRange;
	bool ret = SearchAgent(*pDocLineMgr).SearchWord(
		Point(pLayout->GetLogicOffset() + (int)nIdx, pLayout->GetLogicLineNo()),
		searchDirection,
		&logicRange, //pMatchRange,
		pattern
	);

	// 論理位置→レイアウト位置変換
	// logicRange -> pMatchRange
	if (ret) {
		LogicToLayout(
			logicRange,
			pMatchRange
		);
	}
	return ret;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        単位の変換                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@brief カーソル位置変換 物理→レイアウト

	物理位置(行頭からのバイト数、折り返し無し行位置)
	→レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
*/
Point LayoutMgr::LogicToLayout(
	const Point&	ptLogic,	// [in]  ロジック位置
	int				nLineHint	// [in]  レイアウトY値のヒント。求める値に近い値を渡すと高速に検索できる。
	)
{
	Point ptLayout;	// レイアウト位置
	if (GetLineCount() == 0) {
		return ptLayout; // 変換不可
	}
	// サーチ開始地点 -> pLayout, nCaretPosX, nCaretPosY
	size_t nCaretPosX = 0;
	size_t nCaretPosY;
	const Layout* pLayout;
	{
		nLineHint = t_min((int)GetLineCount() - 1, nLineHint);
		nCaretPosY = t_max<int>(ptLogic.y, nLineHint);

		// pLayoutPrevReferを見る
		if (1
			&& (int)nCaretPosY <= nPrevReferLine
			&& pLayoutPrevRefer
			&& pLayoutPrevRefer->GetLogicLineNo() <= ptLogic.y
		) {
			// ヒントより現在位置のほうが後ろか同じぐらいで近い
			nCaretPosY = ptLogic.y - pLayoutPrevRefer->GetLogicLineNo() + nPrevReferLine;
			pLayout = SearchLineByLayoutY(nCaretPosY);
		}else {
			pLayout = SearchLineByLayoutY(nCaretPosY);
		}
		if (!pLayout) {
			ptLayout.SetY((int)nLines);
			return ptLayout;
		}
		
		// ロジックYがでかすぎる場合は、一致するまでデクリメント (
		while (pLayout && pLayout->GetLogicLineNo() > ptLogic.y) {
			pLayout = pLayout->GetPrevLayout();
			--nCaretPosY;
		}

		// ロジックYが同じでOffsetが行き過ぎている場合は戻る
		if (pLayout->GetLogicLineNo() == ptLogic.y) {
			while (1
				&& pLayout->GetPrevLayout()
				&& pLayout->GetPrevLayout()->GetLogicLineNo() == ptLogic.y
				&& ptLogic.x < pLayout->GetLogicOffset()
			) {
				pLayout = pLayout->GetPrevLayout();
				--nCaretPosY;
			}
		}
	}

	// Layoutを１つずつ先に進めながらptLogic.yが物理行に一致するLayoutを探す
	do {
		if (ptLogic.y == pLayout->GetLogicLineNo()) {
			// 2013.05.10 Moca 高速化
			const Layout* pLayoutNext = pLayout->GetNextLayout();
			if (1
				&& pLayoutNext
				&& ptLogic.y == pLayoutNext->GetLogicLineNo()
				&& pLayoutNext->GetLogicOffset() <= ptLogic.x
			) {
				++nCaretPosY;
				pLayout = pLayout->GetNextLayout();
				continue;
			}

			// 2004.06.16 Moca インデント表示の際に位置がずれる(TAB位置ずれによる)
			// TAB幅を正確に計算するには当初からインデント分を加えておく必要がある．
			nCaretPosX = pLayout->GetIndent();
			const wchar_t*	pData;
			pData = pLayout->GetDocLineRef()->GetPtr() + pLayout->GetLogicOffset();
			size_t nDataLen = pLayout->GetLengthWithEOL();

			size_t i;
			for (i=0; i<nDataLen; ++i) {
				if (pLayout->GetLogicOffset() + (int)i >= ptLogic.x) {
					break;
				}

				// 文字ロジック幅 -> nCharChars
				size_t nCharChars = NativeW::GetSizeOfChar(pData, nDataLen, i);
				if (nCharChars == 0) {
					nCharChars = 1;
				}
				
				// 文字レイアウト幅 -> nCharKetas
				size_t nCharKetas;
				if (pData[i] == WCODE::TAB) {
					nCharKetas = GetActualTabSpace(nCaretPosX);
				}else {
					nCharKetas = NativeW::GetKetaOfChar(pData, nDataLen, i);
				}
				// レイアウト加算
				nCaretPosX += nCharKetas;

				// ロジック加算
				if (pData[i] == WCODE::TAB) {
					nCharChars = 1;
				}
				i += nCharChars - 1;
			}
			if (i < nDataLen) {
				// ptLogic.x, ptLogic.yがこの行の中に見つかったらループ打ち切り
				break;
			}

			if (!pLayout->GetNextLayout()) {
				// 当該位置に達していなくても，レイアウト末尾ならデータ末尾のレイアウト位置を返す．
				nCaretPosX = pLayout->CalcLayoutWidth(*this) + ((pLayout->GetLayoutEol().GetLen() > 0) ? 1 : 0);
				break;
			}

			if (ptLogic.y < pLayout->pNext->GetLogicLineNo()) {
				// 次のLayoutが当該物理行を過ぎてしまう場合はデータ末尾のレイアウト位置を返す．
				nCaretPosX = pLayout->CalcLayoutWidth(*this) + ((pLayout->GetLayoutEol().GetLen() > 0) ? 1 : 0);
				break;
			}
		}
		if (ptLogic.y < pLayout->GetLogicLineNo()) {
			// ふつうはここには来ないと思うが... (genta)
			// Layoutの指す物理行が探している行より先を指していたら打ち切り
			break;
		}

		// 次の行へ進む
		++nCaretPosY;
		pLayout = pLayout->GetNextLayout();
	} while (pLayout);

	// 2004.06.16 Moca インデント表示の際の位置ずれ修正
	ptLayout.Set(
		pLayout ? (int)nCaretPosX : 0,
		(int)nCaretPosY
	);
	return ptLayout;
}

/*!
	@brief カーソル位置変換  レイアウト→物理

	レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	→物理位置(行頭からのバイト数、折り返し無し行位置)
*/
PointEx LayoutMgr::LayoutToLogicEx(
	const Point& ptLayout	// [in]  レイアウト位置
	) const
{
	PointEx ptLogic;	// [out] ロジック位置
	ptLogic.ext = 0;
	if (ptLayout.y > (int)nLines) {
		ptLogic.Set(0, (int)pDocLineMgr->GetLineCount());
		return ptLogic;
	}

	size_t nDataLen;
	const wchar_t*	pData;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        Ｙ値の決定                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	bool bEOF = false;
	size_t nX;
	const Layout* pLayout = SearchLineByLayoutY(ptLayout.y);
	if (!pLayout) {
		if (0 < ptLayout.y) {
			pLayout = SearchLineByLayoutY(ptLayout.y - 1);
			if (!pLayout) {
				ptLogic.Set(0, (int)pDocLineMgr->GetLineCount());
				return ptLogic;
			}else {
				pData = GetLineStr(ptLayout.y - 1, &nDataLen);
				if (WCODE::IsLineDelimiter(pData[nDataLen - 1], GetDllShareData().common.edit.bEnableExtEol)) {
					ptLogic.Set(0, (int)pDocLineMgr->GetLineCount());
					return ptLogic;
				}else {
					ptLogic.y = pDocLineMgr->GetLineCount() - 1; // 2002/2/10 aroka DocLineMgr変更
					bEOF = true;
					// nX = MAXLINEKETAS;
					nX = pLayout->GetIndent();
					goto checkloop;
				}
			}
		}
		ptLogic.Set(0, (int)pDocLineMgr->GetLineCount());
		return ptLogic;
	}else {
		ptLogic.y = pLayout->GetLogicLineNo();
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        Ｘ値の決定                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	pData = GetLineStr(ptLayout.y, &nDataLen);
	nX = pLayout ? pLayout->GetIndent() : 0;

checkloop:;
	size_t i;
	for (i=0; i<nDataLen; ++i) {
		// 文字ロジック幅 -> nCharChars
		size_t nCharChars;
		nCharChars = NativeW::GetSizeOfChar(pData, nDataLen, i);
		if (nCharChars == 0) {
			nCharChars = 1;
		}
		
		// 文字レイアウト幅 -> nCharKetas
		size_t nCharKetas;
		if (pData[i] == WCODE::TAB) {
			nCharKetas = GetActualTabSpace(nX);
		}else {
			nCharKetas = NativeW::GetKetaOfChar(pData, nDataLen, i);
		}

		// レイアウト加算
		if ((int)(nX + nCharKetas) > ptLayout.x && !bEOF) {
			break;
		}
		nX += nCharKetas;

		// ロジック加算
		if (pData[i] == WCODE::TAB) {
			nCharChars = 1;
		}
		i += nCharChars - 1;
	}
	i += pLayout->GetLogicOffset();
	ptLogic.x = i;
	ptLogic.ext = ptLayout.x - nX;
	return ptLogic;
}


Point LayoutMgr::LayoutToLogic(
	const Point& ptLayout
	) const
{
	return LayoutToLogicEx(ptLayout);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         デバッグ                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// テスト用にレイアウト情報をダンプ
void LayoutMgr::DUMP()
{
#ifdef _DEBUG
	size_t nDataLen;
	MYTRACE(_T("------------------------\n"));
	MYTRACE(_T("nLines=%d\n"), nLines);
	MYTRACE(_T("pLayoutTop=%08lxh\n"), pLayoutTop);
	MYTRACE(_T("pLayoutBot=%08lxh\n"), pLayoutBot);
	MYTRACE(_T("nMaxLineKetas=%d\n"), nMaxLineKetas);

	MYTRACE(_T("nTabSpace=%d\n"), nTabSpace);
	Layout* pLayout = pLayoutTop;
	while (pLayout) {
		Layout* pLayoutNext = pLayout->GetNextLayout();
		MYTRACE(_T("\t-------\n"));
		MYTRACE(_T("\tthis=%08lxh\n"),		pLayout);
		MYTRACE(_T("\tpPrev =%08lxh\n"),	pLayout->GetPrevLayout());
		MYTRACE(_T("\tpNext =%08lxh\n"),	pLayout->GetNextLayout());
		MYTRACE(_T("\tnLinePhysical=%d\n"),	pLayout->GetLogicLineNo());
		MYTRACE(_T("\tnOffset=%d\n"),		pLayout->GetLogicOffset());
		MYTRACE(_T("\tnLength=%d\n"),		pLayout->GetLengthWithEOL());
		MYTRACE(_T("\tenumEOLType =%ls\n"),	pLayout->GetLayoutEol().GetName());
		MYTRACE(_T("\tnEOLLen =%d\n"),		pLayout->GetLayoutEol().GetLen());
		MYTRACE(_T("\tnTypePrev=%d\n"),		pLayout->GetColorTypePrev());
		const wchar_t* pData = DocReader(*pDocLineMgr).GetLineStr(pLayout->GetLogicLineNo(), &nDataLen);
		MYTRACE(_T("\t[%ls]\n"), pData);
		pLayout = pLayoutNext;
	}
	MYTRACE(_T("------------------------\n"));
#endif
	return;
}

