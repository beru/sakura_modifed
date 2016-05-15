/*!	@file
	@brief テキストのレイアウト情報管理

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK, YAZAKI, genta, aroka
	Copyright (C) 2003, genta, Moca
	Copyright (C) 2004, genta, Moca
	Copyright (C) 2005, D.S.Koba, Moca
	Copyright (C) 2009, ryoji, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "doc/layout/LayoutMgr.h"
#include "doc/layout/Layout.h"/// 2002/2/10 aroka
#include "doc/EditDoc.h"
#include "doc/DocReader.h" // for _DEBUG
#include "doc/DocEditor.h"
#include "doc/logic/DocLine.h"/// 2002/2/10 aroka
#include "doc/logic/DocLineMgr.h"/// 2002/2/10 aroka
#include "charset/charcode.h"
#include "mem/Memory.h"/// 2002/2/10 aroka
#include "mem/MemoryIterator.h" // 2006.07.29 genta
#include "basis/SakuraBasis.h"
#include "SearchAgent.h"
#include "debug/RunningTimer.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        生成と破棄                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

LayoutMgr::LayoutMgr()
	:
	getIndentOffset(&LayoutMgr::getIndentOffset_Normal)	// Oct. 1, 2002 genta	//	Nov. 16, 2002 メンバー関数ポインタにはクラス名が必要
{
	pDocLineMgr = nullptr;
	pTypeConfig = nullptr;
	nMaxLineKetas = LayoutInt(MAXLINEKETAS);
	nTabSpace = LayoutInt(4);
	pszKinsokuHead_1.clear();				// 行頭禁則	//@@@ 2002.04.08 MIK
	pszKinsokuTail_1.clear();				// 行末禁則	//@@@ 2002.04.08 MIK
	pszKinsokuKuto_1.clear();				// 句読点ぶらさげ	//@@@ 2002.04.17 MIK

	nTextWidth = LayoutInt(0);			// テキスト最大幅の記憶		// 2009.08.28 nasukoji
	nTextWidthMaxLine = LayoutInt(0);	// 最大幅のレイアウト行		// 2009.08.28 nasukoji

	Init();
}


LayoutMgr::~LayoutMgr()
{
	_Empty();

	pszKinsokuHead_1.clear();	// 行頭禁則
	pszKinsokuTail_1.clear();	// 行末禁則			//@@@ 2002.04.08 MIK
	pszKinsokuKuto_1.clear();	// 句読点ぶらさげ	//@@@ 2002.04.17 MIK
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
	nPrevReferLine = LayoutInt(0);
	pLayoutPrevRefer = nullptr;
	nLines = LayoutInt(0);			// 全物理行数
	nLineTypeBot = COLORIDX_DEFAULT;

	// EOFレイアウト位置記憶	// 2006.10.07 Moca
	nEOFLine = LayoutInt(-1);
	nEOFColumn = LayoutInt(-1);
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
	LayoutInt			nTabSpace,
	LayoutInt			nMaxLineKetas
	)
{
	MY_RUNNINGTIMER(runningTimer, "LayoutMgr::SetLayoutInfo");

	assert_warning((!bDoLayout && nMaxLineKetas == nMaxLineKetas) || bDoLayout);
	assert_warning((!bDoLayout && nTabSpace == refType.nTabSpace) || bDoLayout);

	// タイプ別設定
	pTypeConfig = &refType;
	nMaxLineKetas = nMaxLineKetas;
	nTabSpace = nTabSpace;

	// Oct. 1, 2002 genta タイプによって処理関数を変更する
	// 数が増えてきたらテーブルにすべき
	switch (refType.nIndentLayout) {	// 折り返しは2行目以降を字下げ表示	//@@@ 2002.09.29 YAZAKI
	case 1:
		// Nov. 16, 2002 メンバー関数ポインタにはクラス名が必要
		getIndentOffset = &LayoutMgr::getIndentOffset_Tx2x;
		break;
	case 2:
		getIndentOffset = &LayoutMgr::getIndentOffset_LeftSpace;
		break;
	default:
		getIndentOffset = &LayoutMgr::getIndentOffset_Normal;
		break;
	}

	// 句読点ぶら下げ文字	// 2009.08.07 ryoji
	// refType.szKinsokuKuto → pszKinsokuKuto_1
	pszKinsokuKuto_1.clear();
	if (refType.bKinsokuKuto) {	// 2009.08.06 ryoji bKinsokuKutoで振り分ける(Fix)
		for (const wchar_t* p=refType.szKinsokuKuto; *p; ++p) {
			pszKinsokuKuto_1.push_back_unique(*p);
		}
	}

	// 行頭禁則文字
	// refType.szKinsokuHead → (句読点以外) pszKinsokuHead_1
	pszKinsokuHead_1.clear();
	for (const wchar_t* p=refType.szKinsokuHead; *p; ++p) {
		if (pszKinsokuKuto_1.exist(*p)) {
			continue;
		}else {
			pszKinsokuHead_1.push_back_unique(*p);
		}
	}

	// 行末禁則文字
	// refType.szKinsokuTail → pszKinsokuTail_1
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
	LayoutInt nLineLayout
	) const
{
	LayoutInt nLineNum = nLineLayout;

	Layout*	pLayout;
	LayoutInt	nCount;
	if (nLines == LayoutInt(0)) {
		return nullptr;
	}

	// Mar. 19, 2003 Moca nLineNumが負の場合のチェックを追加
	if (LayoutInt(0) > nLineNum || nLineNum >= nLines) {
		if (LayoutInt(0) > nLineNum) {
			DEBUG_TRACE(_T("LayoutMgr::SearchLineByLayoutY() nLineNum = %d\n"), nLineNum);
		}
		return nullptr;
	}
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
	// 2004.03.28 Moca pLayoutPrevReferより、Top,Botのほうが近い場合は、そちらを利用する
	LayoutInt nPrevToLineNumDiff = t_abs(nPrevReferLine - nLineNum);
	if (0
		|| !pLayoutPrevRefer
		|| nLineNum < nPrevToLineNumDiff
		|| nLines - nLineNum < nPrevToLineNumDiff
	) {
		if (nLineNum < (nLines / 2)) {
			nCount = LayoutInt(0);
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
			nCount = nLines - LayoutInt(1);
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
		}else if (nLineNum > nPrevReferLine) {
			nCount = nPrevReferLine + LayoutInt(1);
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
			nCount = nPrevReferLine - LayoutInt(1);
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


//@@@ 2002.09.23 YAZAKI Layout*を作成するところは分離して、InsertLineNext()と共通化
void LayoutMgr::AddLineBottom(Layout* pLayout)
{
	if (nLines == LayoutInt(0)) {
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

//@@@ 2002.09.23 YAZAKI Layout*を作成するところは分離して、AddLineBottom()と共通化
Layout* LayoutMgr::InsertLineNext(
	Layout* pLayoutPrev,
	Layout* pLayout
	)
{
	if (nLines == LayoutInt(0)) {
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

/* Layoutを作成する
	@@@ 2002.09.23 YAZAKI
	@date 2009.08.28 nasukoji	レイアウト長を引数に追加
*/
Layout* LayoutMgr::CreateLayout(
	DocLine*		pDocLine,
	LogicPoint		ptLogicPos,
	LogicInt		nLength,
	EColorIndexType	nTypePrev,
	LayoutInt		nIndent,
	LayoutInt		nPosX,
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

	if (pDocLine->GetEol() == EolType::None) {
		pLayout->eol.SetType(EolType::None);	// 改行コードの種類
	}else {
		if (pLayout->GetLogicOffset() + pLayout->GetLengthWithEOL() >
			pDocLine->GetLengthWithEOL() - pDocLine->GetEol().GetLen()
		) {
			pLayout->eol = pDocLine->GetEol();	// 改行コードの種類
		}else {
			pLayout->eol = EolType::None;	// 改行コードの種類
		}
	}

	// 2009.08.28 nasukoji	「折り返さない」選択時のみレイアウト長を記憶する
	// 「折り返さない」以外で計算しないのはパフォーマンス低下を防ぐ目的なので、
	// パフォーマンスの低下が気にならない程なら全ての折り返し方法で計算する
	// ようにしても良いと思う。
	// （その場合LayoutMgr::CalculateTextWidth()の呼び出し箇所をチェック）
	pLayout->SetLayoutWidth((pEditDoc->nTextWrapMethodCur == TextWrappingMethod::NoWrapping) ? nPosX : LayoutInt(0));

	return pLayout;
}


/*
|| 指定された物理行のデータへのポインタとその長さを返す Ver0

	@date 2002/2/10 aroka CMemory変更
*/
const wchar_t* LayoutMgr::GetLineStr(
	LayoutInt nLine,
	LogicInt* pnLineLen
	) const //#####いらんやろ
{
	const Layout* pLayout;
	if (!(pLayout = SearchLineByLayoutY(nLine))) {
		return NULL;
	}
	*pnLineLen = LogicInt(pLayout->GetLengthWithEOL());
	return pLayout->GetDocLineRef()->GetPtr() + pLayout->GetLogicOffset();
}

/*!	指定された物理行のデータへのポインタとその長さを返す Ver1
	@date 2002/03/24 YAZAKI GetLineStr(int nLine, int* pnLineLen)と同じ動作に変更。
*/
const wchar_t* LayoutMgr::GetLineStr(
	LayoutInt nLine,
	LogicInt* pnLineLen,
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

	@date 2002/4/27 MIK
*/
bool LayoutMgr::IsEndOfLine(
	const LayoutPoint& ptLinePos
	)
{
	const Layout* pLayout;

	if (!(pLayout = SearchLineByLayoutY(ptLinePos.GetY2()))) {
		return false;
	}

	if (pLayout->GetLayoutEol().GetType() == EolType::None) {
		// この行に改行はない
		// この行の最後か？
		if (ptLinePos.x == (Int)pLayout->GetLengthWithEOL()) {
			return true; //$$ 単位混在
		}
	}

	return false;
}

/*!	@brief ファイル末尾のレイアウト位置を取得する

	ファイル末尾まで選択する場合に正確な位置情報を与えるため

	既存の関数では物理行からレイアウト位置を変換する必要があり，
	処理に無駄が多いため，専用関数を作成
	
	@date 2006.07.29 genta
	@date 2006.10.01 Moca メンバで保持するように。データ変更時には、_DoLayout/DoLayout_Rangeで無効にする。
*/
void LayoutMgr::GetEndLayoutPos(
	LayoutPoint* ptLayoutEnd // [out]
	)
{
	if (nEOFLine != -1) {
		ptLayoutEnd->x = nEOFColumn;
		ptLayoutEnd->y = nEOFLine;
		return;
	}

	if (nLines == LayoutInt(0) || !pLayoutBot) {
		// データが空
		ptLayoutEnd->x = LayoutInt(0);
		ptLayoutEnd->y = LayoutInt(0);
		nEOFColumn = ptLayoutEnd->x;
		nEOFLine = ptLayoutEnd->y;
		return;
	}

	Layout* btm = pLayoutBot;
	if (btm->eol != EolType::None) {
		// 末尾に改行がある
		ptLayoutEnd->Set(LayoutInt(0), GetLineCount());
	}else {
		MemoryIterator it(btm, GetTabSpace());
		while (!it.end()) {
			it.scanNext();
			it.addDelta();
		}
		ptLayoutEnd->Set(it.getColumn(), GetLineCount() - LayoutInt(1));
		// [EOF]のみ折り返すのはやめる	// 2009.02.17 ryoji
		//// 2006.10.01 Moca Start [EOF]のみのレイアウト行処理が抜けていたバグを修正
		//if (GetMaxLineKetas() <= ptLayoutEnd->GetX2()) {
		//	ptLayoutEnd->SetX(LayoutInt(0));
		//	ptLayoutEnd->y++;
		//}
		//// 2006.10.01 Moca End
	}
	nEOFColumn = ptLayoutEnd->x;
	nEOFLine = ptLayoutEnd->y;
}


// 論理行の指定範囲に該当するレイアウト情報を削除して
// 削除した範囲の直前のレイアウト情報のポインタを返す
Layout* LayoutMgr::DeleteLayoutAsLogical(
	Layout*	pLayoutInThisArea,
	LayoutInt	nLineOf_pLayoutInThisArea,
	LogicInt	nLineFrom,
	LogicInt	nLineTo,
	LogicPoint	ptDelLogicalFrom,
	LayoutInt*	pnDeleteLines
	)
{
	*pnDeleteLines = LayoutInt(0);
	if (nLines == LayoutInt(0)) {	// 全物理行数
		return nullptr;
	}
	if (!pLayoutInThisArea) {
		return nullptr;
	}

	// 1999.11.22
	pLayoutPrevRefer = pLayoutInThisArea->GetPrevLayout();
	nPrevReferLine = nLineOf_pLayoutInThisArea - LayoutInt(1);

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
//		if (pLayoutPrevRefer == pLayout) {
//			// 1999.12.22 前にずらすだけでよいのでは
//			pLayoutPrevRefer = pLayout->GetPrevLayout();
//			--nPrevReferLine;
//		}

		if (0
			|| (1
				&& ptDelLogicalFrom.GetY2() == pLayout->GetLogicLineNo()
				&& ptDelLogicalFrom.GetX2() < pLayout->GetLogicOffset() + pLayout->GetLengthWithEOL()
			)
			|| (ptDelLogicalFrom.GetY2() < pLayout->GetLogicLineNo())
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
	LogicInt nShiftLines
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
	LayoutInt	nTabSize,
	LayoutInt	nMaxLineKetas
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
	LayoutInt		nLineNum,
	LogicInt		nIdx,
	LayoutRange*	pSelect,		// [out]
	NativeW*		pcmcmWord,		// [out]
	NativeW*		pcmcmWordLeft	// [out]
	)
{
	const Layout* pLayout = SearchLineByLayoutY(nLineNum);
	if (!pLayout) {
		return false;
	}

	// 現在位置の単語の範囲を調べる -> ロジック単位pSelect, pMemWord, pMemWordLeft
	LogicInt nFromX;
	LogicInt nToX;
	bool nRetCode = SearchAgent(*pDocLineMgr).WhereCurrentWord(
		pLayout->GetLogicLineNo(),
		pLayout->GetLogicOffset() + LogicInt(nIdx),
		&nFromX,
		&nToX,
		pcmcmWord,
		pcmcmWordLeft
	);

	if (nRetCode) {
		// 論理位置→レイアウト位置変換
		LayoutPoint ptFrom;
		LogicToLayout(LogicPoint(nFromX, pLayout->GetLogicLineNo()), &ptFrom, nLineNum);
		pSelect->SetFrom(ptFrom);

		LayoutPoint ptTo;
		LogicToLayout(LogicPoint(nToX, pLayout->GetLogicLineNo()), &ptTo, nLineNum);
		pSelect->SetTo(ptTo);
	}
	return nRetCode;

}


// 現在位置の左右の単語の先頭位置を調べる
int LayoutMgr::PrevOrNextWord(
	LayoutInt		nLineNum,
	LogicInt		nIdx,
	LayoutPoint*	pptLayoutNew,
	bool			bLeft,
	bool			bStopsBothEnds
	)
{
	const Layout* pLayout = SearchLineByLayoutY(nLineNum);
	if (!pLayout) {
		return FALSE;
	}

	// 現在位置の左右の単語の先頭位置を調べる
	LogicInt nPosNew;
	int nRetCode = SearchAgent(*pDocLineMgr).PrevOrNextWord(
		pLayout->GetLogicLineNo(),
		pLayout->GetLogicOffset() + nIdx,
		&nPosNew,
		bLeft,
		bStopsBothEnds
	);

	if (nRetCode) {
		// 論理位置→レイアウト位置変換
		LogicToLayout(
			LogicPoint(nPosNew, pLayout->GetLogicLineNo()),
			pptLayoutNew,
			nLineNum
		);
	}
	return nRetCode;
}


// 単語検索
/*
	@retval 0 見つからない
*/
int LayoutMgr::SearchWord(
	LayoutInt				nLine,				// [in] 検索開始レイアウト行
	LogicInt				nIdx,				// [in] 検索開始データ位置
	SearchDirection			searchDirection,	// [in] 検索方向
	LayoutRange*			pMatchRange,		// [out] マッチレイアウト範囲
	const SearchStringPattern&	pattern
	)
{
	const Layout* pLayout = this->SearchLineByLayoutY(nLine);
	if (!pLayout) {
		return FALSE;
	}

	// 単語検索 -> logicRange (データ位置)
	LogicRange logicRange;
	int nRetCode = SearchAgent(*pDocLineMgr).SearchWord(
		LogicPoint(pLayout->GetLogicOffset() + nIdx, pLayout->GetLogicLineNo()),
		searchDirection,
		&logicRange, //pMatchRange,
		pattern
	);

	// 論理位置→レイアウト位置変換
	// logicRange -> pMatchRange
	if (nRetCode) {
		LogicToLayout(
			logicRange,
			pMatchRange
		);
	}
	return nRetCode;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        単位の変換                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@brief カーソル位置変換 物理→レイアウト

	物理位置(行頭からのバイト数、折り返し無し行位置)
	→レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)

	@date 2004.06.16 Moca インデント表示の際のTABを含む行の座標ずれ修正
	@date 2007.09.06 kobake 関数名をCaretPos_Phys2LogからLogicToLayoutに変更
*/
void LayoutMgr::LogicToLayout(
	const LogicPoint&	ptLogic,	// [in]  ロジック位置
	LayoutPoint*		pptLayout,	// [out] レイアウト位置
	LayoutInt			nLineHint	// [in]  レイアウトY値のヒント。求める値に近い値を渡すと高速に検索できる。
	)
{
	pptLayout->Clear();

	if (GetLineCount() == 0) {
		return; // 変換不可
	}
	// サーチ開始地点 -> pLayout, nCaretPosX, nCaretPosY
	LayoutInt		nCaretPosX = LayoutInt(0);
	LayoutInt		nCaretPosY;
	const Layout*	pLayout;
	// 2013.05.15 ヒント、ありなしの処理を統合
	{
		nLineHint = t_min(GetLineCount() - 1, nLineHint);
		nCaretPosY = t_max(LayoutInt(ptLogic.y), nLineHint);

		// 2013.05.12 pLayoutPrevReferを見る
		if (1
			&& nCaretPosY <= nPrevReferLine
			&& pLayoutPrevRefer
			&& pLayoutPrevRefer->GetLogicLineNo() <= ptLogic.y
		) {
			// ヒントより現在位置のほうが後ろか同じぐらいで近い
			nCaretPosY = LayoutInt(ptLogic.y - pLayoutPrevRefer->GetLogicLineNo()) + nPrevReferLine;
			pLayout = SearchLineByLayoutY(nCaretPosY);
		}else {
			pLayout = SearchLineByLayoutY(nCaretPosY);
		}
		if (!pLayout) {
			pptLayout->SetY(nLines);
			return;
		}
		
		// ロジックYがでかすぎる場合は、一致するまでデクリメント (
		while (pLayout->GetLogicLineNo() > ptLogic.GetY2()) {
			pLayout = pLayout->GetPrevLayout();
			--nCaretPosY;
		}

		// ロジックYが同じでOffsetが行き過ぎている場合は戻る
		if (pLayout->GetLogicLineNo() == ptLogic.GetY2()) {
			while (1
				&& pLayout->GetPrevLayout()
				&& pLayout->GetPrevLayout()->GetLogicLineNo() == ptLogic.GetY2()
				&& ptLogic.x < pLayout->GetLogicOffset()
			) {
				pLayout = pLayout->GetPrevLayout();
				--nCaretPosY;
			}
		}
	}

	// Layoutを１つずつ先に進めながらptLogic.yが物理行に一致するLayoutを探す
	do {
		if (ptLogic.GetY2() == pLayout->GetLogicLineNo()) {
			// 2013.05.10 Moca 高速化
			const Layout* pLayoutNext = pLayout->GetNextLayout();
			if (1
				&& pLayoutNext
				&& ptLogic.GetY2() == pLayoutNext->GetLogicLineNo()
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
			pData = pLayout->GetDocLineRef()->GetPtr() + pLayout->GetLogicOffset(); // 2002/2/10 aroka CMemory変更
			LogicInt	nDataLen = (LogicInt)pLayout->GetLengthWithEOL();

			LogicInt i;
			for (i=LogicInt(0); i<nDataLen; ++i) {
				if (pLayout->GetLogicOffset() + i >= ptLogic.x) {
					break;
				}

				// 文字ロジック幅 -> nCharChars
				LogicInt nCharChars = NativeW::GetSizeOfChar(pData, nDataLen, i);
				if (nCharChars == 0) {
					nCharChars = LogicInt(1);
				}
				
				// 文字レイアウト幅 -> nCharKetas
				LayoutInt nCharKetas;
				if (pData[i] == WCODE::TAB) {
					// Sep. 23, 2002 genta メンバー関数を使うように
					nCharKetas = GetActualTabSpace(nCaretPosX);
				}else {
					nCharKetas = NativeW::GetKetaOfChar(pData, nDataLen, i);
				}
//				if (nCharKetas == 0)				// 削除 サロゲートペア対策	2008/7/5 Uchi
//					nCharKetas = LayoutInt(1);

				// レイアウト加算
				nCaretPosX += nCharKetas;

				// ロジック加算
				if (pData[i] == WCODE::TAB) {
					nCharChars = LogicInt(1);
				}
				i += nCharChars - LogicInt(1);
			}
			if (i < nDataLen) {
				// ptLogic.x, ptLogic.yがこの行の中に見つかったらループ打ち切り
				break;
			}

			if (!pLayout->GetNextLayout()) {
				// 当該位置に達していなくても，レイアウト末尾ならデータ末尾のレイアウト位置を返す．
				nCaretPosX = pLayout->CalcLayoutWidth(*this) + LayoutInt(pLayout->GetLayoutEol().GetLen() > 0 ? 1 : 0);
				break;
			}

			if (ptLogic.y < pLayout->pNext->GetLogicLineNo()) {
				// 次のLayoutが当該物理行を過ぎてしまう場合はデータ末尾のレイアウト位置を返す．
				nCaretPosX = pLayout->CalcLayoutWidth(*this) + LayoutInt(pLayout->GetLayoutEol().GetLen() > 0 ? 1 : 0);
				break;
			}
		}
		if (ptLogic.GetY2() < pLayout->GetLogicLineNo()) {
			// ふつうはここには来ないと思うが... (genta)
			// Layoutの指す物理行が探している行より先を指していたら打ち切り
			break;
		}

		// 次の行へ進む
		++nCaretPosY;
		pLayout = pLayout->GetNextLayout();
	} while (pLayout);

	// 2004.06.16 Moca インデント表示の際の位置ずれ修正
	pptLayout->Set(
		pLayout ? nCaretPosX : LayoutInt(0),
		nCaretPosY
	);
}

/*!
	@brief カーソル位置変換  レイアウト→物理

	レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	→物理位置(行頭からのバイト数、折り返し無し行位置)

	@date 2007.09.06 kobake 関数名をCaretPos_Log2Phys→LayoutToLogicに変更
*/
void LayoutMgr::LayoutToLogicEx(
	const LayoutPoint&	ptLayout,	// [in]  レイアウト位置
	LogicPointEx*		pptLogic	// [out] ロジック位置
	) const
{
	pptLogic->Set(LogicInt(0), LogicInt(0));
	pptLogic->ext = 0;
	if (ptLayout.GetY2() > nLines) {
		// 2007.10.11 kobake Y値が間違っていたので修正
		//pptLogic->Set(0, nLines);
		pptLogic->Set(LogicInt(0), pDocLineMgr->GetLineCount());
		return;
	}

	LogicInt		nDataLen;
	const wchar_t*	pData;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        Ｙ値の決定                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	bool bEOF = false;
	LayoutInt nX;
	const Layout* pLayout = SearchLineByLayoutY(ptLayout.GetY2());
	if (!pLayout) {
		if (0 < ptLayout.y) {
			pLayout = SearchLineByLayoutY(ptLayout.GetY2() - LayoutInt(1));
			if (!pLayout) {
				pptLogic->Set(LogicInt(0), pDocLineMgr->GetLineCount());
				return;
			}else {
				pData = GetLineStr(ptLayout.GetY2() - LayoutInt(1), &nDataLen);
				if (WCODE::IsLineDelimiter(pData[nDataLen - 1], GetDllShareData().common.edit.bEnableExtEol)) {
					pptLogic->Set(LogicInt(0), pDocLineMgr->GetLineCount());
					return;
				}else {
					pptLogic->y = pDocLineMgr->GetLineCount() - 1; // 2002/2/10 aroka DocLineMgr変更
					bEOF = true;
					// nX = LayoutInt(MAXLINEKETAS);
					nX = pLayout->GetIndent();
					goto checkloop;
				}
			}
		}
		// 2007.10.11 kobake Y値が間違っていたので修正
		//pptLogic->Set(0, nLines);
		pptLogic->Set(LogicInt(0), pDocLineMgr->GetLineCount());
		return;
	}else {
		pptLogic->y = pLayout->GetLogicLineNo();
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        Ｘ値の決定                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	pData = GetLineStr(ptLayout.GetY2(), &nDataLen);
	nX = pLayout ? pLayout->GetIndent() : LayoutInt(0);

checkloop:;
	LogicInt i;
	for (i=LogicInt(0); i<nDataLen; ++i) {
		// 文字ロジック幅 -> nCharChars
		LogicInt	nCharChars;
		nCharChars = NativeW::GetSizeOfChar(pData, nDataLen, i);
		if (nCharChars == 0)
			nCharChars = LogicInt(1);
		
		// 文字レイアウト幅 -> nCharKetas
		LayoutInt	nCharKetas;
		if (pData[i] == WCODE::TAB) {
			nCharKetas = GetActualTabSpace(nX);
		}else {
			nCharKetas = NativeW::GetKetaOfChar(pData, nDataLen, i);
		}
//		if (nCharKetas == 0)				// 削除 サロゲートペア対策	2008/7/5 Uchi
//			nCharKetas = LayoutInt(1);

		// レイアウト加算
		if (nX + nCharKetas > ptLayout.GetX2() && !bEOF) {
			break;
		}
		nX += nCharKetas;

		// ロジック加算
		if (pData[i] == WCODE::TAB) {
			nCharChars = LogicInt(1);
		}
		i += nCharChars - LogicInt(1);
	}
	i += pLayout->GetLogicOffset();
	pptLogic->x = i;
	pptLogic->ext = ptLayout.GetX2() - nX;
	return;
}


void LayoutMgr::LayoutToLogic(
	const LayoutPoint& ptLayout,
	LogicPoint* pptLogic
	) const
{
	LogicPointEx ptEx;
	LayoutToLogicEx(ptLayout, &ptEx);
	*pptLogic = ptEx;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         デバッグ                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// テスト用にレイアウト情報をダンプ
void LayoutMgr::DUMP()
{
#ifdef _DEBUG
	LogicInt nDataLen;
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

