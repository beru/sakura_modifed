#include "StdAfx.h"
#include <stdlib.h>
#include "LayoutMgr.h"
#include "Layout.h" // 2002/2/10 aroka
#include "doc/logic/DocLineMgr.h" // 2002/2/10 aroka
#include "charset/charcode.h"
#include "SearchAgent.h"


// 文字列置換
void LayoutMgr::ReplaceData_CLayoutMgr(
	LayoutReplaceArg* pArg
	)
{
	size_t nWork_nLines = nLines;	// 変更前の全行数の保存	@@@ 2002.04.19 MIK

	// 置換先頭位置のレイアウト情報
	EColorIndexType	nCurrentLineType = COLORIDX_DEFAULT;
	LayoutColorInfo* colorInfo = nullptr;
	int nLineWork = pArg->delRange.GetFrom().y;

	Layout* pLayoutWork = SearchLineByLayoutY(pArg->delRange.GetFrom().y);
	if (pLayoutWork) {
		while (pLayoutWork->GetLogicOffset() != 0) {
			pLayoutWork = pLayoutWork->GetPrevLayout();
			--nLineWork;
		}
		nCurrentLineType = pLayoutWork->GetColorTypePrev();
		colorInfo = pLayoutWork->GetLayoutExInfo()->DetachColorInfo();
	}else if (GetLineCount() == pArg->delRange.GetFrom().y) {
		// 2012.01.05 最終行のRedo/Undoでの色分けが正しくないのを修正
		nCurrentLineType = nLineTypeBot;
		colorInfo = layoutExInfoBot.DetachColorInfo();
	}

	/*
	||  カーソル位置変換
	||  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置) →
	||  物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	Point ptFrom = LayoutToLogic(pArg->delRange.GetFrom());
	Point ptTo = LayoutToLogic(pArg->delRange.GetTo());

	/* 指定範囲のデータを置換(削除 & データを挿入)
	  Fromを含む位置からToの直前を含むデータを削除する
	  Fromの位置へテキストを挿入する
	*/
	DocLineReplaceArg dlra;
	dlra.delRange.SetFrom(ptFrom);	// 削除範囲from
	dlra.delRange.SetTo(ptTo);		// 削除範囲to
	dlra.pMemDeleted = pArg->pMemDeleted;	// 削除されたデータを保存
	dlra.pInsData = pArg->pInsData;			// 挿入するデータ
	dlra.nDelSeq = pArg->nDelSeq;
	SearchAgent(*pDocLineMgr).ReplaceData(
		&dlra
	);
	pArg->nInsSeq = dlra.nInsSeq;

	// --- 変更された行のレイアウト情報を再生成 ---
	// 論理行の指定範囲に該当するレイアウト情報を削除して
	// 削除した範囲の直前のレイアウト情報のポインタを返す

	size_t nModifyLayoutLinesOld = 0;
	Layout* pLayoutPrev;
	size_t nWork = t_max(dlra.nDeletedLineNum, dlra.nInsLineNum);

	if (pLayoutWork) {
		pLayoutPrev = DeleteLayoutAsLogical(
			pLayoutWork,
			nLineWork,
			ptFrom.y,
			ptFrom.y + nWork,
			ptFrom,
			&nModifyLayoutLinesOld
		);

		// 指定行より後の行のレイアウト情報について、論理行番号を指定行数だけシフトする
		// 論理行が削除された場合は０より小さい行数
		// 論理行が挿入された場合は０より大きい行数
		if (dlra.nInsLineNum - dlra.nDeletedLineNum != 0) {
			ShiftLogicalLineNum(
				pLayoutPrev,
				(int)dlra.nInsLineNum - (int)dlra.nDeletedLineNum
			);
		}
	}else {
		pLayoutPrev = pLayoutBot;
	}

	// 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする
	int nRowNum;
	if (!pLayoutPrev) {
		if (!pLayoutTop) {
			nRowNum = pDocLineMgr->GetLineCount();
		}else {
			nRowNum = pLayoutTop->GetLogicLineNo();
		}
	}else {
		if (!pLayoutPrev->GetNextLayout()) {
			nRowNum = pDocLineMgr->GetLineCount() - pLayoutPrev->GetLogicLineNo() - 1;
		}else {
			nRowNum = pLayoutPrev->pNext->GetLogicLineNo() - pLayoutPrev->GetLogicLineNo() - 1;
		}
	}

	// 2009.08.28 nasukoji	テキスト最大幅算出用の引数を設定
	CalTextWidthArg ctwArg;
	ctwArg.ptLayout     = pArg->delRange.GetFrom();		// 編集開始位置
	ctwArg.nDelLines    = pArg->delRange.GetTo().y - pArg->delRange.GetFrom().y;	// 削除行数 - 1
	ctwArg.nAllLinesOld = nWork_nLines;								// 編集前のテキスト行数
	ctwArg.bInsData     = (pArg->pInsData && pArg->pInsData->size());			// 追加文字列の有無

	// 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする
	pArg->nModLineTo = DoLayout_Range(
		pLayoutPrev,
		nRowNum,
		ptFrom,
		nCurrentLineType,
		colorInfo,
		ctwArg
	);
	ASSERT_GE(nLines, nWork_nLines);
	pArg->nAddLineNum = nLines - nWork_nLines;	// 変更後の全行数との差分	@@@ 2002.04.19 MIK
	if (pArg->nAddLineNum == 0) {
		pArg->nAddLineNum = nModifyLayoutLinesOld - pArg->nModLineTo;	// 再描画ヒント レイアウト行の増減
	}
	pArg->nModLineFrom = pArg->delRange.GetFrom().y;	// 再描画ヒント 変更されたレイアウト行From
	pArg->nModLineTo += (pArg->nModLineFrom - 1) ;	// 再描画ヒント 変更されたレイアウト行To

	// 2007.10.18 kobake LayoutReplaceArg::ptLayoutNewはここで算出するのが正しい
	pArg->ptLayoutNew = LogicToLayout(dlra.ptNewPos); // 挿入された部分の次の位置
}

