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
#include "doc/layout/CLayoutMgr.h"
#include "doc/layout/CLayout.h"/// 2002/2/10 aroka
#include "doc/CEditDoc.h"
#include "doc/CDocReader.h" // for _DEBUG
#include "doc/CDocEditor.h"
#include "doc/logic/CDocLine.h"/// 2002/2/10 aroka
#include "doc/logic/CDocLineMgr.h"/// 2002/2/10 aroka
#include "charset/charcode.h"
#include "mem/CMemory.h"/// 2002/2/10 aroka
#include "mem/CMemoryIterator.h" // 2006.07.29 genta
#include "basis/SakuraBasis.h"
#include "CSearchAgent.h"
#include "debug/CRunningTimer.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        生成と破棄                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

LayoutMgr::LayoutMgr()
	:
	m_getIndentOffset(&LayoutMgr::getIndentOffset_Normal)	// Oct. 1, 2002 genta	//	Nov. 16, 2002 メンバー関数ポインタにはクラス名が必要
{
	m_pDocLineMgr = NULL;
	m_pTypeConfig = NULL;
	m_nMaxLineKetas = LayoutInt(MAXLINEKETAS);
	m_nTabSpace = LayoutInt(4);
	m_pszKinsokuHead_1.clear();				// 行頭禁則	//@@@ 2002.04.08 MIK
	m_pszKinsokuTail_1.clear();				// 行末禁則	//@@@ 2002.04.08 MIK
	m_pszKinsokuKuto_1.clear();				// 句読点ぶらさげ	//@@@ 2002.04.17 MIK

	m_nTextWidth = LayoutInt(0);			// テキスト最大幅の記憶		// 2009.08.28 nasukoji
	m_nTextWidthMaxLine = LayoutInt(0);	// 最大幅のレイアウト行		// 2009.08.28 nasukoji

	Init();
}


LayoutMgr::~LayoutMgr()
{
	_Empty();

	m_pszKinsokuHead_1.clear();	// 行頭禁則
	m_pszKinsokuTail_1.clear();	// 行末禁則			//@@@ 2002.04.08 MIK
	m_pszKinsokuKuto_1.clear();	// 句読点ぶらさげ	//@@@ 2002.04.17 MIK
}


/*
||
|| 行データ管理クラスのポインタを初期化します
||
*/
void LayoutMgr::Create(
	EditDoc* pcEditDoc,
	DocLineMgr* pcDocLineMgr
	)
{
	_Empty();
	Init();
	// Jun. 20, 2003 genta EditDocへのポインタ追加
	m_pEditDoc = pcEditDoc;
	m_pDocLineMgr = pcDocLineMgr;
}


void LayoutMgr::Init()
{
	m_pLayoutTop = NULL;
	m_pLayoutBot = NULL;
	m_nPrevReferLine = LayoutInt(0);
	m_pLayoutPrevRefer = NULL;
	m_nLines = LayoutInt(0);			// 全物理行数
	m_nLineTypeBot = COLORIDX_DEFAULT;

	// EOFレイアウト位置記憶	// 2006.10.07 Moca
	m_nEOFLine = LayoutInt(-1);
	m_nEOFColumn = LayoutInt(-1);
}


void LayoutMgr::_Empty()
{
	Layout* pLayout = m_pLayoutTop;
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
	MY_RUNNINGTIMER(cRunningTimer, "LayoutMgr::SetLayoutInfo");

	assert_warning((!bDoLayout && m_nMaxLineKetas == nMaxLineKetas) || bDoLayout);
	assert_warning((!bDoLayout && m_nTabSpace == refType.m_nTabSpace) || bDoLayout);

	// タイプ別設定
	m_pTypeConfig = &refType;
	m_nMaxLineKetas = nMaxLineKetas;
	m_nTabSpace = nTabSpace;

	// Oct. 1, 2002 genta タイプによって処理関数を変更する
	// 数が増えてきたらテーブルにすべき
	switch (refType.m_nIndentLayout) {	// 折り返しは2行目以降を字下げ表示	//@@@ 2002.09.29 YAZAKI
	case 1:
		// Nov. 16, 2002 メンバー関数ポインタにはクラス名が必要
		m_getIndentOffset = &LayoutMgr::getIndentOffset_Tx2x;
		break;
	case 2:
		m_getIndentOffset = &LayoutMgr::getIndentOffset_LeftSpace;
		break;
	default:
		m_getIndentOffset = &LayoutMgr::getIndentOffset_Normal;
		break;
	}

	// 句読点ぶら下げ文字	// 2009.08.07 ryoji
	// refType.m_szKinsokuKuto → m_pszKinsokuKuto_1
	m_pszKinsokuKuto_1.clear();
	if (refType.m_bKinsokuKuto) {	// 2009.08.06 ryoji m_bKinsokuKutoで振り分ける(Fix)
		for (const wchar_t* p=refType.m_szKinsokuKuto; *p; ++p) {
			m_pszKinsokuKuto_1.push_back_unique(*p);
		}
	}

	// 行頭禁則文字
	// refType.m_szKinsokuHead → (句読点以外) m_pszKinsokuHead_1
	m_pszKinsokuHead_1.clear();
	for (const wchar_t* p=refType.m_szKinsokuHead; *p; ++p) {
		if (m_pszKinsokuKuto_1.exist(*p)) {
			continue;
		}else {
			m_pszKinsokuHead_1.push_back_unique(*p);
		}
	}

	// 行末禁則文字
	// refType.m_szKinsokuTail → m_pszKinsokuTail_1
	m_pszKinsokuTail_1.clear();
	for (const wchar_t* p=refType.m_szKinsokuTail; *p; ++p) {
		m_pszKinsokuTail_1.push_back_unique(*p);
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
	if (m_nLines == LayoutInt(0)) {
		return NULL;
	}

	// Mar. 19, 2003 Moca nLineNumが負の場合のチェックを追加
	if (LayoutInt(0) > nLineNum || nLineNum >= m_nLines) {
		if (LayoutInt(0) > nLineNum) {
			DEBUG_TRACE(_T("LayoutMgr::SearchLineByLayoutY() nLineNum = %d\n"), nLineNum);
		}
		return NULL;
	}
//	// +++++++ 低速版 +++++++++
//	if (nLineNum < (m_nLines / 2)) {
//		nCount = 0;
//		pLayout = m_pLayoutTop;
//		while (pLayout) {
//			if (nLineNum == nCount) {
//				m_pLayoutPrevRefer = pLayout;
//				m_nPrevReferLine = nLineNum;
//				return pLayout;
//			}
//			pLayout = pLayout->GetNextLayout();
//			++nCount;
//		}
//	}else {
//		nCount = m_nLines - 1;
//		pLayout = m_pLayoutBot;
//		while (pLayout) {
//			if (nLineNum == nCount) {
//				m_pLayoutPrevRefer = pLayout;
//				m_nPrevReferLine = nLineNum;
//				return pLayout;
//			}
//			pLayout = pLayout->GetPrevLayout();
//			--nCount;
//		}
//	}


	// +++++++わずかに高速版+++++++
	// 2004.03.28 Moca m_pLayoutPrevReferより、Top,Botのほうが近い場合は、そちらを利用する
	LayoutInt nPrevToLineNumDiff = t_abs(m_nPrevReferLine - nLineNum);
	if (0
		|| !m_pLayoutPrevRefer
		|| nLineNum < nPrevToLineNumDiff
		|| m_nLines - nLineNum < nPrevToLineNumDiff
	) {
		if (nLineNum < (m_nLines / 2)) {
			nCount = LayoutInt(0);
			pLayout = m_pLayoutTop;
			while (pLayout) {
				if (nLineNum == nCount) {
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->GetNextLayout();
				++nCount;
			}
		}else {
			nCount = m_nLines - LayoutInt(1);
			pLayout = m_pLayoutBot;
			while (pLayout) {
				if (nLineNum == nCount) {
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->GetPrevLayout();
				--nCount;
			}
		}
	}else {
		if (nLineNum == m_nPrevReferLine) {
			return m_pLayoutPrevRefer;
		}else if (nLineNum > m_nPrevReferLine) {
			nCount = m_nPrevReferLine + LayoutInt(1);
			pLayout = m_pLayoutPrevRefer->GetNextLayout();
			while (pLayout) {
				if (nLineNum == nCount) {
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->GetNextLayout();
				++nCount;
			}
		}else {
			nCount = m_nPrevReferLine - LayoutInt(1);
			pLayout = m_pLayoutPrevRefer->GetPrevLayout();
			while (pLayout) {
				if (nLineNum == nCount) {
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->GetPrevLayout();
				--nCount;
			}
		}
	}
	return NULL;
}


//@@@ 2002.09.23 YAZAKI Layout*を作成するところは分離して、InsertLineNext()と共通化
void LayoutMgr::AddLineBottom(Layout* pLayout)
{
	if (m_nLines == LayoutInt(0)) {
		m_pLayoutBot = m_pLayoutTop = pLayout;
		m_pLayoutTop->m_pPrev = NULL;
	}else {
		m_pLayoutBot->m_pNext = pLayout;
		pLayout->m_pPrev = m_pLayoutBot;
		m_pLayoutBot = pLayout;
	}
	pLayout->m_pNext = NULL;
	++m_nLines;
	return;
}

//@@@ 2002.09.23 YAZAKI Layout*を作成するところは分離して、AddLineBottom()と共通化
Layout* LayoutMgr::InsertLineNext(
	Layout* pLayoutPrev,
	Layout* pLayout
	)
{
	if (m_nLines == LayoutInt(0)) {
		// 初
		m_pLayoutBot = m_pLayoutTop = pLayout;
		m_pLayoutTop->m_pPrev = NULL;
		m_pLayoutTop->m_pNext = NULL;
	}else if (!pLayoutPrev) {
		// 先頭に挿入
		m_pLayoutTop->m_pPrev = pLayout;
		pLayout->m_pPrev = NULL;
		pLayout->m_pNext = m_pLayoutTop;
		m_pLayoutTop = pLayout;
	}else
	if (!pLayoutPrev->GetNextLayout()) {
		// 最後に挿入
		m_pLayoutBot->m_pNext = pLayout;
		pLayout->m_pPrev = m_pLayoutBot;
		pLayout->m_pNext = NULL;
		m_pLayoutBot = pLayout;
	}else {
		// 途中に挿入
		Layout* pLayoutNext = pLayoutPrev->GetNextLayout();
		pLayoutPrev->m_pNext = pLayout;
		pLayoutNext->m_pPrev = pLayout;
		pLayout->m_pPrev = pLayoutPrev;
		pLayout->m_pNext = pLayoutNext;
	}
	++m_nLines;
	return pLayout;
}

/* Layoutを作成する
	@@@ 2002.09.23 YAZAKI
	@date 2009.08.28 nasukoji	レイアウト長を引数に追加
*/
Layout* LayoutMgr::CreateLayout(
	DocLine*		pCDocLine,
	LogicPoint		ptLogicPos,
	LogicInt		nLength,
	EColorIndexType	nTypePrev,
	LayoutInt		nIndent,
	LayoutInt		nPosX,
	LayoutColorInfo*	colorInfo
	)
{
	Layout* pLayout = new Layout(
		pCDocLine,
		ptLogicPos,
		nLength,
		nTypePrev,
		nIndent,
		colorInfo
	);

	if (pCDocLine->GetEol() == EolType::None) {
		pLayout->m_eol.SetType(EolType::None);	// 改行コードの種類
	}else {
		if (pLayout->GetLogicOffset() + pLayout->GetLengthWithEOL() >
			pCDocLine->GetLengthWithEOL() - pCDocLine->GetEol().GetLen()
		) {
			pLayout->m_eol = pCDocLine->GetEol();	// 改行コードの種類
		}else {
			pLayout->m_eol = EolType::None;	// 改行コードの種類
		}
	}

	// 2009.08.28 nasukoji	「折り返さない」選択時のみレイアウト長を記憶する
	// 「折り返さない」以外で計算しないのはパフォーマンス低下を防ぐ目的なので、
	// パフォーマンスの低下が気にならない程なら全ての折り返し方法で計算する
	// ようにしても良いと思う。
	// （その場合LayoutMgr::CalculateTextWidth()の呼び出し箇所をチェック）
	pLayout->SetLayoutWidth((m_pEditDoc->m_nTextWrapMethodCur == (int)TextWrappingMethod::NoWrapping) ? nPosX : LayoutInt(0));

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
	return (*ppcLayoutDes)->m_pDocLine->GetPtr() + (*ppcLayoutDes)->GetLogicOffset();
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
	if (m_nEOFLine != -1) {
		ptLayoutEnd->x = m_nEOFColumn;
		ptLayoutEnd->y = m_nEOFLine;
		return;
	}

	if (m_nLines == LayoutInt(0) || !m_pLayoutBot) {
		// データが空
		ptLayoutEnd->x = LayoutInt(0);
		ptLayoutEnd->y = LayoutInt(0);
		m_nEOFColumn = ptLayoutEnd->x;
		m_nEOFLine = ptLayoutEnd->y;
		return;
	}

	Layout* btm = m_pLayoutBot;
	if (btm->m_eol != EolType::None) {
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
	m_nEOFColumn = ptLayoutEnd->x;
	m_nEOFLine = ptLayoutEnd->y;
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
	if (m_nLines == LayoutInt(0)) {	// 全物理行数
		return NULL;
	}
	if (!pLayoutInThisArea) {
		return NULL;
	}

	// 1999.11.22
	m_pLayoutPrevRefer = pLayoutInThisArea->GetPrevLayout();
	m_nPrevReferLine = nLineOf_pLayoutInThisArea - LayoutInt(1);

	// 範囲内先頭に該当するレイアウト情報をサーチ
	Layout* pLayoutWork = pLayoutInThisArea->GetPrevLayout();
	while (pLayoutWork && nLineFrom <= pLayoutWork->GetLogicLineNo()) {
		pLayoutWork = pLayoutWork->GetPrevLayout();
	}

	Layout* pLayout = pLayoutWork ? pLayoutWork->GetNextLayout() : m_pLayoutTop;
	while (pLayout) {
		if (pLayout->GetLogicLineNo() > nLineTo) {
			break;
		}
		Layout* pLayoutNext = pLayout->GetNextLayout();
		if (!pLayoutWork) {
			// 先頭行の処理
			m_pLayoutTop = pLayout->GetNextLayout();
			if (pLayout->GetNextLayout()) {
				pLayout->m_pNext->m_pPrev = NULL;
			}
		}else {
			pLayoutWork->m_pNext = pLayout->GetNextLayout();
			if (pLayout->GetNextLayout()) {
				pLayout->m_pNext->m_pPrev = pLayoutWork;
			}
		}
//		if (m_pLayoutPrevRefer == pLayout) {
//			// 1999.12.22 前にずらすだけでよいのでは
//			m_pLayoutPrevRefer = pLayout->GetPrevLayout();
//			--m_nPrevReferLine;
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

		if (m_pLayoutPrevRefer == pLayout) {
			DEBUG_TRACE(_T("バグバグ\n"));
		}

		delete pLayout;

		--m_nLines;	// 全物理行数
		if (!pLayoutNext) {
			m_pLayoutBot = pLayoutWork;
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
	MY_RUNNINGTIMER(cRunningTimer, "LayoutMgr::ShiftLogicalLineNum");

	if (nShiftLines == 0) {
		return;
	}
	Layout* pLayout = pLayoutPrev ? pLayoutPrev->GetNextLayout() : m_pLayoutTop;
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

	m_nTabSpace = nTabSize;
	m_nMaxLineKetas = nMaxLineKetas;

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

	// 現在位置の単語の範囲を調べる -> ロジック単位pSelect, pcmemWord, pcmemWordLeft
	LogicInt nFromX;
	LogicInt nToX;
	bool nRetCode = SearchAgent(m_pDocLineMgr).WhereCurrentWord(
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
	bool			bLEFT,
	bool			bStopsBothEnds
	)
{
	const Layout* pLayout = SearchLineByLayoutY(nLineNum);
	if (!pLayout) {
		return FALSE;
	}

	// 現在位置の左右の単語の先頭位置を調べる
	LogicInt nPosNew;
	int nRetCode = SearchAgent(m_pDocLineMgr).PrevOrNextWord(
		pLayout->GetLogicLineNo(),
		pLayout->GetLogicOffset() + nIdx,
		&nPosNew,
		bLEFT,
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
	SearchDirection		searchDirection,	// [in] 検索方向
	LayoutRange*			pMatchRange,		// [out] マッチレイアウト範囲
	const SearchStringPattern&	pattern
	)
{
	const Layout* pLayout = this->SearchLineByLayoutY(nLine);
	if (!pLayout) {
		return FALSE;
	}

	// 単語検索 -> cLogicRange (データ位置)
	LogicRange cLogicRange;
	int nRetCode = SearchAgent(m_pDocLineMgr).SearchWord(
		LogicPoint(pLayout->GetLogicOffset() + nIdx, pLayout->GetLogicLineNo()),
		searchDirection,
		&cLogicRange, //pMatchRange,
		pattern
	);

	// 論理位置→レイアウト位置変換
	// cLogicRange -> pMatchRange
	if (nRetCode) {
		LogicToLayout(
			cLogicRange,
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

		// 2013.05.12 m_pLayoutPrevReferを見る
		if (1
			&& nCaretPosY <= m_nPrevReferLine
			&& m_pLayoutPrevRefer
			&& m_pLayoutPrevRefer->GetLogicLineNo() <= ptLogic.y
		) {
			// ヒントより現在位置のほうが後ろか同じぐらいで近い
			nCaretPosY = LayoutInt(ptLogic.y - m_pLayoutPrevRefer->GetLogicLineNo()) + m_nPrevReferLine;
			pLayout = SearchLineByLayoutY(nCaretPosY);
		}else {
			pLayout = SearchLineByLayoutY(nCaretPosY);
		}
		if (!pLayout) {
			pptLayout->SetY(m_nLines);
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

			if (ptLogic.y < pLayout->m_pNext->GetLogicLineNo()) {
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
	if (ptLayout.GetY2() > m_nLines) {
		// 2007.10.11 kobake Y値が間違っていたので修正
		//pptLogic->Set(0, m_nLines);
		pptLogic->Set(LogicInt(0), m_pDocLineMgr->GetLineCount());
		return;
	}

	LogicInt		nDataLen;
	const wchar_t*	pData;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        Ｙ値の決定                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	BOOL bEOF = FALSE;
	LayoutInt nX;
	const Layout* pcLayout = SearchLineByLayoutY(ptLayout.GetY2());
	if (!pcLayout) {
		if (0 < ptLayout.y) {
			pcLayout = SearchLineByLayoutY(ptLayout.GetY2() - LayoutInt(1));
			if (!pcLayout) {
				pptLogic->Set(LogicInt(0), m_pDocLineMgr->GetLineCount());
				return;
			}else {
				pData = GetLineStr(ptLayout.GetY2() - LayoutInt(1), &nDataLen);
				if (WCODE::IsLineDelimiter(pData[nDataLen - 1], GetDllShareData().m_common.m_edit.m_bEnableExtEol)) {
					pptLogic->Set(LogicInt(0), m_pDocLineMgr->GetLineCount());
					return;
				}else {
					pptLogic->y = m_pDocLineMgr->GetLineCount() - 1; // 2002/2/10 aroka DocLineMgr変更
					bEOF = TRUE;
					// nX = LayoutInt(MAXLINEKETAS);
					nX = pcLayout->GetIndent();
					goto checkloop;
				}
			}
		}
		// 2007.10.11 kobake Y値が間違っていたので修正
		//pptLogic->Set(0, m_nLines);
		pptLogic->Set(LogicInt(0), m_pDocLineMgr->GetLineCount());
		return;
	}else {
		pptLogic->y = pcLayout->GetLogicLineNo();
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        Ｘ値の決定                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	pData = GetLineStr(ptLayout.GetY2(), &nDataLen);
	nX = pcLayout ? pcLayout->GetIndent() : LayoutInt(0);

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
	i += pcLayout->GetLogicOffset();
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
	MYTRACE(_T("m_nLines=%d\n"), m_nLines);
	MYTRACE(_T("m_pLayoutTop=%08lxh\n"), m_pLayoutTop);
	MYTRACE(_T("m_pLayoutBot=%08lxh\n"), m_pLayoutBot);
	MYTRACE(_T("m_nMaxLineKetas=%d\n"), m_nMaxLineKetas);

	MYTRACE(_T("m_nTabSpace=%d\n"), m_nTabSpace);
	Layout* pLayout = m_pLayoutTop;
	while (pLayout) {
		Layout* pLayoutNext = pLayout->GetNextLayout();
		MYTRACE(_T("\t-------\n"));
		MYTRACE(_T("\tthis=%08lxh\n"), pLayout);
		MYTRACE(_T("\tm_pPrev =%08lxh\n"),		pLayout->GetPrevLayout());
		MYTRACE(_T("\tm_pNext =%08lxh\n"),		pLayout->GetNextLayout());
		MYTRACE(_T("\tm_nLinePhysical=%d\n"),	pLayout->GetLogicLineNo());
		MYTRACE(_T("\tm_nOffset=%d\n"),		pLayout->GetLogicOffset());
		MYTRACE(_T("\tm_nLength=%d\n"),		pLayout->GetLengthWithEOL());
		MYTRACE(_T("\tm_enumEOLType =%ls\n"),	pLayout->GetLayoutEol().GetName());
		MYTRACE(_T("\tm_nEOLLen =%d\n"),		pLayout->GetLayoutEol().GetLen());
		MYTRACE(_T("\tm_nTypePrev=%d\n"),		pLayout->GetColorTypePrev());
		const wchar_t* pData = DocReader(*m_pDocLineMgr).GetLineStr(pLayout->GetLogicLineNo(), &nDataLen);
		MYTRACE(_T("\t[%ls]\n"), pData);
		pLayout = pLayoutNext;
	}
	MYTRACE(_T("------------------------\n"));
#endif
	return;
}

