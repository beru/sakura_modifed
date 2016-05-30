/*!	@file
	@brief EditViewクラスのコマンド処理系関数群

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, genta, asa-o, hor
	Copyright (C) 2002, YAZAKI, hor, genta. aroka, MIK, minfu, KK, かろと
	Copyright (C) 2003, MIK, Moca
	Copyright (C) 2004, genta, Moca
	Copyright (C) 2005, ryoji, genta, D.S.Koba
	Copyright (C) 2006, genta, Moca, fon
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "view/EditView.h"
#include "SearchAgent.h"
#include "uiparts/WaitCursor.h"
#include "charset/charcode.h"
#include "Ope.h" ///	2002/2/3 aroka from here
#include "OpeBlk.h" ///
#include "doc/EditDoc.h"	//	2002/5/13 YAZAKI ヘッダ整理
#include "doc/DocReader.h"
#include "doc/layout/Layout.h"
#include "doc/logic/DocLine.h"
#include "cmd/ViewCommander_inline.h"
#include "window/EditWnd.h"
#include "dlg/DlgCtrlCode.h"	// コントロールコードの入力(ダイアログ)
#include "dlg/DlgFavorite.h"	// 履歴の管理	//@@@ 2003.04.08 MIK
#include "debug/RunningTimer.h"

using namespace std; // 2002/2/3 aroka

static void StringToOpeLineData(const wchar_t* pLineData, size_t nLineDataLen, OpeLineData& lineData, int opeSeq)
{
	size_t nBegin = 0;
	size_t i;
	bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
	for (i=0; i<nLineDataLen; ++i) {
		if (WCODE::IsLineDelimiter(pLineData[i], bExtEol)) {
			if (i + 1 < nLineDataLen && pLineData[i] == WCODE::CR && pLineData[i+1] == WCODE::LF) {
				++i;
			}
			lineData.emplace_back();
			LineData& insertLine = lineData.back();
			insertLine.memLine.SetString(&pLineData[nBegin], i - nBegin + 1);
			insertLine.nSeq = opeSeq;
			nBegin = i + 1;
		}
	}
	if (nBegin < i) {
		lineData.emplace_back();
		LineData& insertLine = lineData.back();
		insertLine.memLine.SetString(&pLineData[nBegin], nLineDataLen - nBegin);
		insertLine.nSeq = opeSeq;
	}
}


/*!	現在位置にデータを挿入 Ver0

	@date 2002/03/24 YAZAKI bUndo削除
*/
void EditView::InsertData_CEditView(
	Point ptInsertPos,		// [in] 挿入位置
	const wchar_t* pData,	// [in] 挿入テキスト
	size_t nDataLen,		// [in] 挿入テキスト長。文字単位。
	Point* pptNewPos,		// [out] 挿入された部分の次の位置のレイアウト位置
	bool bRedraw
)
{
#ifdef _DEBUG
	MY_RUNNINGTIMER(runningTimer, "EditView::InsertData_CEditView");
#endif

	// 2007.10.18 kobake COpe処理をここにまとめる
	InsertOpe* pOpe = nullptr;
	int opeSeq;
	if (!bDoing_UndoRedo) {	// Undo, Redoの実行中か
		pOpe = new InsertOpe();
		pOpe->ptCaretPos_PHY_Before = pEditDoc->layoutMgr.LayoutToLogic(ptInsertPos);
		opeSeq = GetDocument().docEditor.opeBuf.GetNextSeq();
	}else {
		opeSeq = 0;
	}

	pptNewPos->y = 0;			// 挿入された部分の次の位置のレイアウト行
	pptNewPos->x = 0;			// 挿入された部分の次の位置のレイアウト位置

	// テキストが選択されているか
	if (GetSelectionInfo().IsTextSelected()) {
		DeleteData(bRedraw);
		ptInsertPos = GetCaret().GetCaretLayoutPos();
	}

	// テキスト取得 -> pLine, nLineLen, pLayout
	bool bHintPrev = false;	// 更新が前行からになる可能性があることを示唆する
	bool bHintNext = false;	// 更新が次行からになる可能性があることを示唆する
	size_t nLineLen;
	const Layout* pLayout;
	const wchar_t*	pLine = pEditDoc->layoutMgr.GetLineStr(ptInsertPos.y, &nLineLen, &pLayout);
	bool bLineModifiedChange = (pLine)? !ModifyVisitor().IsLineModified(pLayout->GetDocLineRef(),
		GetDocument().docEditor.opeBuf.GetNoModifiedSeq()): true;

	// 禁則の有無
	// 禁則がある場合は1行前から再描画を行う	@@@ 2002.04.19 MIK
	bool bKinsoku = 0
			|| pTypeData->bWordWrap
			|| pTypeData->bKinsokuHead	//@@@ 2002.04.19 MIK
			|| pTypeData->bKinsokuTail	//@@@ 2002.04.19 MIK
			|| pTypeData->bKinsokuRet	//@@@ 2002.04.19 MIK
			|| pTypeData->bKinsokuKuto;	//@@@ 2002.04.19 MIK

	size_t nLineAllColLen;
	ASSERT_GE(ptInsertPos.x, 0);
	size_t nColumnFrom = (size_t)ptInsertPos.x;
	NativeW	mem(L"");
	OpeLineData insData;
	if (pLine) {
		// 更新が前行からになる可能性を調べる	// 2009.02.17 ryoji
		// ※折り返し行頭への句読点入力で前の行だけが更新される場合もある
		// ※挿入位置は行途中でも句読点入力＋ワードラップで前の文字列から続けて前行に回り込む場合もある
		if (pLayout->GetLogicOffset() && bKinsoku) {	// 折り返しレイアウト行か？
			bHintPrev = true;	// 更新が前行からになる可能性がある
		}

		// 更新が次行からになる可能性を調べる	// 2009.02.17 ryoji
		// ※折り返し行末への文字入力や文字列貼り付けで現在行は更新されず次行以後が更新される場合もある
		// 指定された桁に対応する行のデータ内の位置を調べる
		size_t nIdxFrom = LineColumnToIndex2(pLayout, ptInsertPos.x, &nLineAllColLen);

		// 行終端より右に挿入しようとした
		if (nLineAllColLen > 0) {
			// 終端直前から挿入位置まで空白を埋める為の処理
			// 行終端が何らかの改行コードか?
			if (EolType::None != pLayout->GetLayoutEol()) {
				nIdxFrom = nLineLen - 1;
				mem.AllocStringBuffer((ptInsertPos.x - nLineAllColLen + 1) + nDataLen);
				ASSERT_GE(ptInsertPos.x, (int)nLineAllColLen);
				for (int i=0; i<ptInsertPos.x-(int)nLineAllColLen+1; ++i) {
					mem += L' ';
				}
				mem.AppendString(pData, nDataLen);
			}else {
				nIdxFrom = nLineLen;
				ASSERT_GE(ptInsertPos.x, (int)nLineAllColLen);
				mem.AllocStringBuffer((ptInsertPos.x - nLineAllColLen) + nDataLen);
				for (int i=0; i<ptInsertPos.x-(int)nLineAllColLen; ++i) {
					mem += L' ';
				}
				mem.AppendString(pData, nDataLen);
				// 1行多く更新する必要がある可能性がある
				bHintNext = true;
			}
			StringToOpeLineData(mem.GetStringPtr(), mem.GetStringLength(), insData, opeSeq);
			mem.Clear();
			nColumnFrom = LineIndexToColumn(pLayout, nIdxFrom);
		}else {
			StringToOpeLineData(pData, nDataLen, insData, opeSeq);
		}
	}else {
		// 更新が前行からになる可能性を調べる	// 2009.02.17 ryoji
		const Layout* pLayoutWk = pEditDoc->layoutMgr.GetBottomLayout();
		if (pLayoutWk && pLayoutWk->GetLayoutEol() == EolType::None && bKinsoku) {	// 折り返しレイアウト行か？（前行の終端で調査）
			bHintPrev = true;	// 更新が前行からになる可能性がある
		}
		if (0 < ptInsertPos.x) {
			mem.AllocStringBuffer(ptInsertPos.x + nDataLen);
			for (int i=0; i<ptInsertPos.x; ++i) {
				mem += L' ';
			}
			mem.AppendString(pData, nDataLen);
			StringToOpeLineData(mem.GetStringPtr(), mem.GetStringLength(), insData, opeSeq);
			mem.Clear();
		}else {
			StringToOpeLineData(pData, nDataLen, insData, opeSeq);
		}
		nColumnFrom = 0;
	}

	if (!bDoing_UndoRedo && pOpe) {	// Undo, Redoの実行中か
		pOpe->ptCaretPos_PHY_Before = pEditDoc->layoutMgr.LayoutToLogic(Point((int)nColumnFrom, ptInsertPos.y));
	}

	// 文字列挿入
	int nModifyLayoutLinesOld = 0;
	size_t nInsLineNum;		// 挿入によって増えたレイアウト行の数
	int	nInsSeq;
	{
		LayoutReplaceArg arg;
		arg.delRange.Set(Point((int)nColumnFrom, ptInsertPos.y));
		arg.pMemDeleted = nullptr;
		arg.pInsData = &insData;
		arg.nDelSeq = opeSeq;
		pEditDoc->layoutMgr.ReplaceData_CLayoutMgr(&arg);
		nInsLineNum = arg.nAddLineNum;
		nModifyLayoutLinesOld = arg.nModLineTo - arg.nModLineFrom + 1;
		*pptNewPos = arg.ptLayoutNew;
		nInsSeq = arg.nInsSeq;
	}

	// 指定された行のデータ内の位置に対応する桁の位置を調べる
	size_t nLineLen2;
	const wchar_t* pLine2 = pEditDoc->layoutMgr.GetLineStr(pptNewPos->y, &nLineLen2, &pLayout);
	if (pLine2) {
		// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
		pptNewPos->x = pptNewPos->x; //LineIndexToColumn(pLayout, pptNewPos->x);
	}

	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	if (pptNewPos->x >= (int)pEditDoc->layoutMgr.GetMaxLineKetas()) {
		if (pTypeData->bKinsokuRet
		 || pTypeData->bKinsokuKuto
		) {	//@@@ 2002.04.16 MIK
			if (pEditDoc->layoutMgr.IsEndOfLine(*pptNewPos)) {	//@@@ 2002.04.18
				pptNewPos->x = 0;
				pptNewPos->y++;
			}
		}else {
			// Oct. 7, 2002 YAZAKI
			pptNewPos->x = (int)(pLayout->GetNextLayout() ? pLayout->GetNextLayout()->GetIndent() : 0);
			pptNewPos->y++;
		}
	}

	// 状態遷移
	if (!bDoing_UndoRedo) {	// Undo, Redoの実行中か
		pEditDoc->docEditor.SetModified(true, bRedraw);	//	Jan. 22, 2002 genta
	}

	// 再描画
	// 行番号表示に必要な幅を設定
	if (editWnd.DetectWidthOfLineNumberAreaAllPane(bRedraw)) {
		// キャレットの表示・更新
		GetCaret().ShowEditCaret();
	}else {
		PAINTSTRUCT ps;

		if (bRedraw) {
			int nStartLine(ptInsertPos.y);
			// 2013.05.08 折り返し行でEOF直前で改行したときEOFが再描画されないバグの修正
			if (nModifyLayoutLinesOld < 1) {
				nModifyLayoutLinesOld = 1;
			}
			// 2011.12.26 正規表現キーワード・検索文字列などは、ロジック行頭までさかのぼって更新する必要がある
			{
				const Layout* pLayoutLineFirst = pEditDoc->layoutMgr.SearchLineByLayoutY(ptInsertPos.y);
				while (pLayoutLineFirst && pLayoutLineFirst->GetLogicOffset() != 0) {
					pLayoutLineFirst = pLayoutLineFirst->GetPrevLayout();
					if (bHintPrev) {
						bHintPrev = false;
					}
					--nStartLine;
					++nModifyLayoutLinesOld;
				}
			}
			int nLayoutTop;
			int nLayoutBottom;
			auto& textArea = GetTextArea();
			if (nInsLineNum != 0) {
				// スクロールバーの状態を更新する
				AdjustScrollBars();

				// 描画開始行位置を調整する	// 2009.02.17 ryoji
				if (bHintPrev) {	// 更新が前行からになる可能性がある
					--nStartLine;
				}

				ps.rcPaint.left = 0;
				ps.rcPaint.right = textArea.GetAreaRight();
				ps.rcPaint.top = textArea.GenerateYPx(nStartLine);
				ps.rcPaint.bottom = textArea.GetAreaBottom();
				nLayoutTop = nStartLine;
				nLayoutBottom = -1;
			}else {
				// 描画開始行位置と描画行数を調整する	// 2009.02.17 ryoji
				if (bHintPrev) {	// 更新が前行からになる可能性がある
					--nStartLine;
					++nModifyLayoutLinesOld;
				}
				if (bHintNext) {	// 更新が次行からになる可能性がある
					++nModifyLayoutLinesOld;
				}

	//			ps.rcPaint.left = textArea.GetAreaLeft();
				ps.rcPaint.left = 0;
				ps.rcPaint.right = textArea.GetAreaRight();

				// 2002.02.25 Mod By KK 次行 (ptInsertPos.y - textArea.GetViewTopLine() - 1); => (ptInsertPos.y - textArea.GetViewTopLine());
				//ps.rcPaint.top = textArea.GetAreaTop() + GetTextMetrics().GetHankakuDy() * (ptInsertPos.y - textArea.GetViewTopLine() - 1);
				ps.rcPaint.top = textArea.GenerateYPx(nStartLine);
				ps.rcPaint.bottom = textArea.GenerateYPx(nStartLine + nModifyLayoutLinesOld);
				nLayoutTop = nStartLine;
				nLayoutBottom = nStartLine + nModifyLayoutLinesOld;
			}
			HDC hdc = this->GetDC();
			OnPaint(hdc, &ps, FALSE);
			this->ReleaseDC(hdc);
			// 2014.07.16 他のビュー(ミニマップ)の再描画を抑制する
			if (nInsLineNum == 0) {
				for (int i=0; i<editWnd.GetAllViewCount(); ++i) {
					EditView* pView = &editWnd.GetView(i);
					if (pView == this) {
						continue;
					}
					pView->RedrawLines(nLayoutTop, nLayoutBottom);
				}
				editWnd.GetMiniMap().RedrawLines(nLayoutTop, nLayoutBottom);
				if (!bDoing_UndoRedo && pOpe) {
					GetDocument().docEditor.nOpeBlkRedawCount++;
				}
			}

#if 0 // すでに行頭から描画済み
			// 行番号（変更行）表示は改行単位の行頭から更新する必要がある	// 2009.03.26 ryoji
			if (bLineModifiedChange) {	// 無変更だった行が変更された
				const Layout* pLayoutWk = pEditDoc->layoutMgr.SearchLineByLayoutY(nStartLine);
				if (pLayoutWk && pLayoutWk->GetLogicOffset()) {	// 折り返しレイアウト行か？
					Call_OnPaint(PaintAreaType::LineNumber, false);
				}
			}
#endif
		}
	}

	// 2007.10.18 kobake ここでCOpe処理をまとめる
	if (!bDoing_UndoRedo) {
		pOpe->ptCaretPos_PHY_After = pEditDoc->layoutMgr.LayoutToLogic(*pptNewPos);
		pOpe->nOrgSeq = nInsSeq;

		// 操作の追加
		commander.GetOpeBlk()->AppendOpe(pOpe);
	}
}


/*!	指定位置の指定長データ削除

	@param _ptCaretPos [in]  削除データの位置
	@param nDelLen [out] 削除データのサイズ
	@param pMem [out]  削除したデータ(nullptr可能)

	@date 2002/03/24 YAZAKI bUndo削除
	@date 2002/05/12 YAZAKI bRedraw, bRedraw2削除（常にFALSEだから）
	@date 2007/10/17 kobake (重要)pMemの所有者が条件によりCOpeに移ったり移らなかったりする振る舞いは
	                        非常にややこしく混乱の元になるため、常に、pMemの所有者は移さないように仕様変更。
*/
void EditView::DeleteData2(
	const Point&	_ptCaretPos,
	size_t			nDelLen,
	NativeW*		pMem
	)
{
#ifdef _DEBUG
	MY_RUNNINGTIMER(runningTimer, "EditView::DeleteData(1)");
#endif
	size_t nLineLen;
	const Layout* pLayout;
	const wchar_t* pLine = pEditDoc->layoutMgr.GetLineStr(_ptCaretPos.y, &nLineLen, &pLayout);
	if (!pLine) {
		return;
	}
	size_t nIdxFrom = LineColumnToIndex(pLayout, _ptCaretPos.x);

	// 2007.10.18 kobake COpeの生成をここにまとめる
	DeleteOpe*	pOpe = nullptr;
	size_t columnFrom = LineIndexToColumn(pLayout, nIdxFrom);
	size_t columnTo = LineIndexToColumn(pLayout, nIdxFrom + nDelLen);
	if (!bDoing_UndoRedo) {
		pOpe = new DeleteOpe();
		pOpe->ptCaretPos_PHY_Before = pEditDoc->layoutMgr.LayoutToLogic(Point((int)columnFrom, _ptCaretPos.y));
		pOpe->ptCaretPos_PHY_To = pEditDoc->layoutMgr.LayoutToLogic(Point((int)columnTo, _ptCaretPos.y));
	}
	OpeLineData memDeleted;
	OpeLineData* pmemDeleted = nullptr;
	if (pMem || pOpe) {
		pmemDeleted = &memDeleted;
	}

	// データ削除
	{
		LayoutReplaceArg arg;
		arg.delRange.SetFrom(_ptCaretPos);
		arg.delRange.SetTo(Point((int)columnTo, _ptCaretPos.y));
		arg.pMemDeleted = pmemDeleted;
		arg.pInsData = nullptr;
		arg.nDelSeq = GetDocument().docEditor.opeBuf.GetNextSeq();
		pEditDoc->layoutMgr.ReplaceData_CLayoutMgr(&arg);
	}

	// 選択エリアの先頭へカーソルを移動
	GetCaret().MoveCursor(_ptCaretPos, false);
	GetCaret().nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();

	if (pMem) {
		size_t size = memDeleted.size();
		size_t bufSize = 0;
		for (size_t i=0; i<size; ++i) {
			bufSize += memDeleted[i].memLine.GetStringLength();
		}
		pMem->SetString(L"");
		pMem->AllocStringBuffer(bufSize);
		for (size_t i=0; i<size; ++i) {
			pMem->AppendNativeData(memDeleted[i].memLine);
		}
	}
	// 2007.10.18 kobake COpeの追加をここにまとめる
	if (pOpe) {
		pOpe->opeLineData.swap(memDeleted);
		pOpe->ptCaretPos_PHY_After = pEditDoc->layoutMgr.LayoutToLogic(_ptCaretPos);
		// 操作の追加
		commander.GetOpeBlk()->AppendOpe(pOpe);
	}

}


/*!	カーソル位置または選択エリアを削除

	@date 2002/03/24 YAZAKI bUndo削除
*/
void EditView::DeleteData(
	bool	bRedraw
//	BOOL	bUndo	// Undo操作かどうか
	)
{
#ifdef _DEBUG
	MY_RUNNINGTIMER(runningTimer, "EditView::DeleteData(2)");
#endif
	size_t nLineLen;
	size_t nLineNum;
	size_t nIdxFrom;
	size_t nIdxTo;
	size_t nDelLen;
	size_t nDelLenNext;
	Rect rcSel;
	const Layout*	pLayout;

	auto& selInfo = GetSelectionInfo();
	auto& caret = GetCaret();
	// テキストの存在しないエリアの削除は、選択範囲のキャンセルとカーソル移動のみとする	// 2008.08.05 ryoji
	if (selInfo.IsTextSelected()) {		// テキストが選択されているか
		if (IsEmptyArea(selInfo.select.GetFrom(), selInfo.select.GetTo(), true, selInfo.IsBoxSelecting())) {
			// カーソルを選択範囲の左上に移動
			caret.MoveCursor(
				Point(
					selInfo.select.GetFrom().x < selInfo.select.GetTo().x ? selInfo.select.GetFrom().x : selInfo.select.GetTo().x,
					selInfo.select.GetFrom().y < selInfo.select.GetTo().y ? selInfo.select.GetFrom().y : selInfo.select.GetTo().y
				), bRedraw
			);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
			selInfo.DisableSelectArea(bRedraw);
			return;
		}
	}else {
		if (IsEmptyArea(caret.GetCaretLayoutPos())) {
			return;
		}
	}

	Point ptCaretPosOld = caret.GetCaretLayoutPos();

	// テキストが選択されているか
	if (selInfo.IsTextSelected()) {
		WaitCursor waitCursor(this->GetHwnd());  // 2002.02.05 hor
		if (!bDoing_UndoRedo) {	// Undo, Redoの実行中か
			// 操作の追加
			commander.GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					caret.GetCaretLogicPos()	// 操作前後のキャレット位置
				)
			);
		}

		// 矩形範囲選択中か
		if (selInfo.IsBoxSelecting()) {
			pEditDoc->docEditor.SetModified(true, bRedraw);	//	2002/06/04 YAZAKI 矩形選択を削除したときに変更マークがつかない。

			SetDrawSwitch(false);	// 2002.01.25 hor
			// 選択範囲のデータを取得
			// 正常時はTRUE,範囲未選択の場合はFALSEを返す
			// ２点を対角とする矩形を求める
			TwoPointToRect(
				&rcSel,
				selInfo.select.GetFrom(),	// 範囲選択開始
				selInfo.select.GetTo()		// 範囲選択終了
			);
			// 現在の選択範囲を非選択状態に戻す
			selInfo.DisableSelectArea(bRedraw);

			nIdxFrom = 0;
			nIdxTo = 0;
			for (nLineNum=rcSel.bottom; (int)nLineNum>=rcSel.top-1; --nLineNum) {
				nDelLenNext	= nIdxTo - nIdxFrom;
				const wchar_t* pLine = pEditDoc->layoutMgr.GetLineStr(nLineNum, &nLineLen, &pLayout);
				if (pLine) {
					using namespace WCODE;

					// 指定された桁に対応する行のデータ内の位置を調べる
					nIdxFrom = LineColumnToIndex(pLayout, (size_t)rcSel.left);
					nIdxTo	 = LineColumnToIndex(pLayout, (size_t)rcSel.right);

					bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
					for (size_t i=nIdxFrom; i<=nIdxTo; ++i) {
						if (WCODE::IsLineDelimiter(pLine[i], bExtEol)) {
							nIdxTo = i;
							break;
						}
					}
				}else {
					nIdxFrom = 0;
					nIdxTo	 = 0;
				}
				nDelLen	= nDelLenNext;
				if ((int)nLineNum < rcSel.bottom && 0 < nDelLen) {
					// 指定位置の指定長データ削除
					DeleteData2(
						Point(rcSel.left, (int)nLineNum + 1),
						nDelLen,
						nullptr
					);
				}
			}
			SetDrawSwitch(true);	// 2002.01.25 hor

			// 行番号表示に必要な幅を設定
			if (editWnd.DetectWidthOfLineNumberAreaAllPane(true)) {
				// キャレットの表示・更新
				caret.ShowEditCaret();
			}
			if (bRedraw) {
				// スクロールバーの状態を更新する
				AdjustScrollBars();

				// 再描画
				Call_OnPaint((int)PaintAreaType::LineNumber | (int)PaintAreaType::Body, false);
			}
			// 選択エリアの先頭へカーソルを移動
			this->UpdateWindow();
			
			Point caretOld(rcSel.left, rcSel.top);
			pEditDoc->layoutMgr.GetLineStr(rcSel.top, &nLineLen, &pLayout);
			if (rcSel.left <= (LONG)pLayout->CalcLayoutWidth(pEditDoc->layoutMgr)) {
				// EOLより左なら文字の単位にそろえる
				size_t nIdxCaret = LineColumnToIndex(pLayout, rcSel.left);
				caretOld.SetX((int)LineIndexToColumn(pLayout, nIdxCaret));
			}
			caret.MoveCursor(caretOld, bRedraw);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX();
			if (!bDoing_UndoRedo) {	// Undo, Redoの実行中か
				MoveCaretOpe* pOpe = new MoveCaretOpe();
				pOpe->ptCaretPos_PHY_Before = pEditDoc->layoutMgr.LayoutToLogic(ptCaretPosOld);
				pOpe->ptCaretPos_PHY_After = caret.GetCaretLogicPos();	// 操作後のキャレット位置
				// 操作の追加
				commander.GetOpeBlk()->AppendOpe(pOpe);
			}
		}else {
			// データ置換 削除&挿入にも使える
			ReplaceData_CEditView(
				selInfo.select,
				L"",					// 挿入するデータ
				0,			// 挿入するデータの長さ
				bRedraw,
				bDoing_UndoRedo ? nullptr : commander.GetOpeBlk()
			);
		}
	}else {
		// 現在行のデータを取得
		const wchar_t* pLine = pEditDoc->layoutMgr.GetLineStr(caret.GetCaretLayoutPos().y, &nLineLen, &pLayout);
		if (!pLine) {
			goto end_of_func;
//			return;
		}
		// 最後の行にカーソルがあるかどうか
		bool bLastLine = (caret.GetCaretLayoutPos().GetY() == pEditDoc->layoutMgr.GetLineCount() - 1);

		// 指定された桁に対応する行のデータ内の位置を調べる
		size_t nCurIdx = LineColumnToIndex(pLayout, caret.GetCaretLayoutPos().x);
//		MYTRACE(_T("nLineLen=%d nCurIdx=%d \n"), nLineLen, nCurIdx);
		if (nCurIdx == nLineLen && bLastLine) {	// 全テキストの最後
			goto end_of_func;
//			return;
		}

		// 指定された桁の文字のバイト数を調べる
		size_t nNxtIdx;
		size_t nNxtPos;
		bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
		if (WCODE::IsLineDelimiter(pLine[nCurIdx], bExtEol)) {
			// 改行
			nNxtIdx = nCurIdx + pLayout->GetLayoutEol().GetLen();
			nNxtPos = caret.GetCaretLayoutPos().GetX() + pLayout->GetLayoutEol().GetLen(); // ※改行コードの文字数を文字幅と見なす
		}else {
			nNxtIdx = NativeW::GetCharNext(pLine, nLineLen, &pLine[nCurIdx]) - pLine;
			// 指定された行のデータ内の位置に対応する桁の位置を調べる
			nNxtPos = LineIndexToColumn(pLayout, nNxtIdx);
		}

		// データ置換 削除&挿入にも使える
		Range delRange;
		delRange.SetFrom(caret.GetCaretLayoutPos());
		delRange.SetTo(Point((int)nNxtPos, caret.GetCaretLayoutPos().GetY()));
		Range delRangeLogic;
		delRangeLogic.SetFrom(caret.GetCaretLogicPos());
		delRangeLogic.SetTo(Point((int)nNxtIdx + pLayout->GetLogicOffset(), caret.GetCaretLogicPos().GetY()));
		ReplaceData_CEditView(
			delRange,
			L"",		// 挿入するデータ
			0,			// 挿入するデータの長さ
			bRedraw,
			bDoing_UndoRedo ? nullptr : commander.GetOpeBlk(),
			false,
			&delRangeLogic
		);
	}

	pEditDoc->docEditor.SetModified(true, bRedraw);	//	Jan. 22, 2002 genta

	if (pEditDoc->layoutMgr.GetLineCount() > 0) {
		if (caret.GetCaretLayoutPos().GetY() > (int)pEditDoc->layoutMgr.GetLineCount() - 1) {
			// 現在行のデータを取得
			const Layout*	pLayout;
			const wchar_t* pLine = pEditDoc->layoutMgr.GetLineStr(pEditDoc->layoutMgr.GetLineCount() - 1, &nLineLen, &pLayout);
			if (!pLine) {
				goto end_of_func;
			}
			// 改行で終わっているか
			if (EolType::None != pLayout->GetLayoutEol()) {
				goto end_of_func;
			}
			// ファイルの最後に移動
			GetCommander().Command_GoFileEnd(false);
		}
	}
end_of_func:;

	return;
}


void EditView::ReplaceData_CEditView(
	const Range&	delRange,			// [in]  削除範囲レイアウト単位
	const wchar_t*	pInsData,			// [in]  挿入するデータ
	size_t			nInsDataLen,		// [in]  挿入するデータの長さ
	bool			bRedraw,
	OpeBlk*			pOpeBlk,
	bool			bFastMode,
	const Range*	psDelRangeLogicFast
	)
{
	auto& opeBuf = GetDocument().docEditor.opeBuf;
	int opeSeq = bDoing_UndoRedo ? opeBuf.GetCurrentPointer() : opeBuf.GetNextSeq();
	if (nInsDataLen == 0) {
		ReplaceData_CEditView3(delRange, nullptr, nullptr, bRedraw, pOpeBlk, opeSeq, nullptr, bFastMode, psDelRangeLogicFast);
	}else {
		OpeLineData insData;
		StringToOpeLineData(pInsData, nInsDataLen, insData, opeSeq);
		ReplaceData_CEditView3(delRange, nullptr, &insData, bRedraw, pOpeBlk, opeSeq, nullptr, bFastMode, psDelRangeLogicFast);
	}
}

void EditView::ReplaceData_CEditView2(
	const Range&	delRange,			// 削除範囲。ロジック単位。
	const wchar_t*	pInsData,			// 挿入するデータ
	size_t			nInsDataLen,		// 挿入するデータの長さ
	bool			bRedraw,
	OpeBlk*			pOpeBlk,
	bool			bFastMode
	)
{
	Range sDelRangeLayout;
	if (!bFastMode) {
		pEditDoc->layoutMgr.LogicToLayout(delRange, &sDelRangeLayout);
	}
	ReplaceData_CEditView(sDelRangeLayout, pInsData, nInsDataLen, bRedraw, pOpeBlk, bFastMode, &delRange);
}


// データ置換 削除&挿入にも使える
// Jun 23, 2000 genta 変数名を書き換え忘れていたのを修正
// Jun. 1, 2000 genta DeleteDataから移動した
bool EditView::ReplaceData_CEditView3(
	Range		delRange,			// [in]  削除範囲レイアウト単位
	OpeLineData*	pMemCopyOfDeleted,	// [out] 削除されたデータのコピー(NULL可能)
	OpeLineData*	pInsData,			// [in]  挿入するデータ
	bool			bRedraw,
	OpeBlk*			pOpeBlk,
	int				nDelSeq,
	int*			pnInsSeq,
	bool			bFastMode,			// [in] CDocLineMgrを更新しない,行末チェックを省略する。bRedraw==falseの必要あり
	const Range*	psDelRangeLogicFast
	)
{
	assert((bFastMode && !bRedraw) || (!bFastMode)); // bFastModeのときは bReadraw == false
	bool bLineModifiedChange;
	bool bUpdateAll = true;
	auto& layoutMgr = pEditDoc->layoutMgr;
	bool bDelRangeUpdate = false;
	{
		//	May. 29, 2000 genta
		//	From Here
		//	行の後ろが選択されていたときの不具合を回避するため，
		//	選択領域から行末以降の部分を取り除く．

		//	先頭
		const Layout* pLayout;
		size_t len;
		const wchar_t* line = NULL;
		if (!bFastMode) {
			line = layoutMgr.GetLineStr(delRange.GetFrom().y, &len, &pLayout);
		}
		bLineModifiedChange = (line)? !ModifyVisitor().IsLineModified(pLayout->GetDocLineRef(), GetDocument().docEditor.opeBuf.GetNoModifiedSeq()): true;
		if (line) {
			size_t pos = LineColumnToIndex(pLayout, delRange.GetFrom().x);
			//	Jun. 1, 2000 genta
			//	同一行の行末以降のみが選択されている場合を考慮する

			//	Aug. 22, 2000 genta
			//	開始位置がEOFの後ろのときは次行に送る処理を行わない
			//	これをやってしまうと存在しない行をPointして落ちる．
			if (delRange.GetFrom().y < (int)layoutMgr.GetLineCount() - 1 && pos >= len) {
				if (delRange.GetFrom().y == delRange.GetTo().y) {
					//	GetSelectionInfo().select.GetFrom().y <= GetSelectionInfo().select.GetTo().y はチェックしない
					Point tmp = delRange.GetFrom();
					tmp.y++;
					tmp.x = 0;
					delRange.Set(tmp);
				}else {
					delRange.GetFrom().y++;
					delRange.SetFromX(0);
				}
				bDelRangeUpdate = true;
			}
		}

		//	末尾
		if (!bFastMode) {
			line = layoutMgr.GetLineStr(delRange.GetTo().y, &len, &pLayout);
			if (line) {
				int p = (int)LineIndexToColumn(pLayout, len);
				if (delRange.GetTo().x > p) {
					delRange.SetToX(p);
					bDelRangeUpdate = true;
				}
			}
		}
		//	To Here
	}

	// 削除範囲ロジック単位 delRange -> delRangeLogic
	Range sDelRangeLogic;
	if (!bDelRangeUpdate && psDelRangeLogicFast) {
		sDelRangeLogic = *psDelRangeLogicFast;
	}else {
		layoutMgr.LayoutToLogic(
			delRange,
			&sDelRangeLogic
		);
	}

	auto& caret = GetCaret();
	Point ptCaretPos_PHY_Old = caret.GetCaretLogicPos();
	if (pOpeBlk) {	// Undo, Redoの実行中か
		// 操作の追加
		if (sDelRangeLogic.GetFrom() != caret.GetCaretLogicPos()) {
			pOpeBlk->AppendOpe(
				new MoveCaretOpe(
					caret.GetCaretLogicPos()	// 操作前後のキャレット位置
				)
			);
		}
	}

	ReplaceOpe* pReplaceOpe = nullptr;	// 編集操作要素 COpe
	if (pOpeBlk) {
		pReplaceOpe = new ReplaceOpe();
		pReplaceOpe->ptCaretPos_PHY_Before = sDelRangeLogic.GetFrom();
		pReplaceOpe->ptCaretPos_PHY_To = sDelRangeLogic.GetTo();
		pReplaceOpe->ptCaretPos_PHY_After = pReplaceOpe->ptCaretPos_PHY_Before;	// 操作後のキャレット位置
	}

	OpeLineData* pMemDeleted = nullptr;
	OpeLineData opeData;
	if (pOpeBlk || pMemCopyOfDeleted) {
		pMemDeleted = &opeData;
	}


	// 現在の選択範囲を非選択状態に戻す
	// 2009.07.18 ryoji 置換後→置換前に位置を変更（置換後だと反転が不正になって汚い Wiki BugReport/43）
	GetSelectionInfo().DisableSelectArea(bRedraw);

	// 文字列置換
	LayoutReplaceArg LRArg;
	DocLineReplaceArg DLRArg;
	if (bFastMode) {
		DLRArg.delRange = sDelRangeLogic;
		DLRArg.pMemDeleted = pMemDeleted;
		DLRArg.pInsData = pInsData;
		DLRArg.nDelSeq = nDelSeq;
		// DLRArg.ptNewPos;
		SearchAgent(GetDocument().docLineMgr).ReplaceData(&DLRArg);
	}else {
		LRArg.delRange    = delRange;		// 削除範囲レイアウト
		LRArg.pMemDeleted = pMemDeleted;	// [out] 削除されたデータ
		LRArg.pInsData     = pInsData;		// 挿入するデータ
		LRArg.nDelSeq      = nDelSeq;		// 挿入するデータの長さ
		layoutMgr.ReplaceData_CLayoutMgr(&LRArg);
	}

	//	Jan. 30, 2001 genta
	//	再描画の時点でファイル更新フラグが適切になっていないといけないので
	//	関数の末尾からここへ移動
	// 状態遷移
	if (pOpeBlk) {	// Undo, Redoの実行中か
		pEditDoc->docEditor.SetModified(true, bRedraw);	//	Jan. 22, 2002 genta
	}

	// 行番号表示に必要な幅を設定
	if (editWnd.DetectWidthOfLineNumberAreaAllPane(bRedraw)) {
		// キャレットの表示・更新
		caret.ShowEditCaret();
	}else {
		// 再描画
		if (bRedraw) {
			// 再描画ヒント レイアウト行の増減
			//	Jan. 30, 2001 genta	貼り付けで行数が減る場合の考慮が抜けていた
			if (LRArg.nAddLineNum != 0) {
				Call_OnPaint((int)PaintAreaType::LineNumber | (int)PaintAreaType::Body, false);
			}else {
				// 文書末が改行なし→ありに変化したら				// 2009.11.11 ryoji
				// EOFのみ行が追加になるので、1行余分に描画する。
				// （文書末が改行あり→なしに変化する場合の末尾EOF消去は描画関数側で行われる）
				int nAddLine = (LRArg.ptLayoutNew.y > LRArg.delRange.GetTo().y)? 1: 0;

				PAINTSTRUCT ps;

				ps.rcPaint.left = 0;
				ps.rcPaint.right = GetTextArea().GetAreaRight();

				/* 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う) */
				ps.rcPaint.top = GetTextArea().GenerateYPx(LRArg.nModLineFrom);
				// 2011.12.26 正規表現キーワード・検索文字列などは、ロジック行頭までさかのぼって更新する必要がある
				{
					const Layout* pLayoutLineFirst = layoutMgr.SearchLineByLayoutY(LRArg.nModLineFrom);
					while (pLayoutLineFirst && pLayoutLineFirst->GetLogicOffset() != 0) {
						pLayoutLineFirst = pLayoutLineFirst->GetPrevLayout();
						ps.rcPaint.top -= GetTextMetrics().GetHankakuDy();
						if (ps.rcPaint.top < 0) {
							break;
						}
					}
				}
				if (ps.rcPaint.top < 0) {
					ps.rcPaint.top = 0;
				}
				ps.rcPaint.bottom = GetTextArea().GenerateYPx(LRArg.nModLineTo + 1 + nAddLine);
				if (GetTextArea().GetAreaBottom() < ps.rcPaint.bottom) {
					ps.rcPaint.bottom = GetTextArea().GetAreaBottom();
				}

				HDC hdc = this->GetDC();
				OnPaint(hdc, &ps, FALSE);
				this->ReleaseDC(hdc);

				int nLayoutTop = LRArg.nModLineFrom;
				int nLayoutBottom = LRArg.nModLineTo + 1 + nAddLine;
				for (int i=0; i<editWnd.GetAllViewCount(); ++i) {
					EditView* pView = &editWnd.GetView(i);
					if (pView == this) {
						continue;
					}
					pView->RedrawLines(nLayoutTop, nLayoutBottom);
				}
				editWnd.GetMiniMap().RedrawLines(nLayoutTop, nLayoutBottom);
				if (!bDoing_UndoRedo && pOpeBlk) {
					GetDocument().docEditor.nOpeBlkRedawCount++;
				}
				bUpdateAll = false;
#if 0 // すでに1行まとめて描画済み
				// 行番号（変更行）表示は改行単位の行頭から更新する必要がある	// 2009.03.26 ryoji
				if (bLineModifiedChange) {	// 無変更だった行が変更された
					const Layout* pLayoutWk = pEditDoc->layoutMgr.SearchLineByLayoutY( LRArg.nModLineFrom );
					if (pLayoutWk && pLayoutWk->GetLogicOffset()) {	// 折り返しレイアウト行か？
						Call_OnPaint(PaintAreaType::LineNumber, false);
					}
				}
#endif
			}
		}
	}

	// 削除されたデータのコピー(NULL可能)
	if (pMemDeleted && 0 < pMemDeleted->size()) {
		if (pMemCopyOfDeleted) {
			if (pOpeBlk) {
				pReplaceOpe->pMemDataDel = *pMemDeleted;
			}
			pMemCopyOfDeleted->swap(*pMemDeleted);
		}else if (pOpeBlk) {
			pReplaceOpe->pMemDataDel.swap(*pMemDeleted);
		}
	}

	if (pOpeBlk) {
		if (bFastMode) {
			pReplaceOpe->ptCaretPos_PHY_After = DLRArg.ptNewPos;
			pReplaceOpe->nOrgInsSeq = DLRArg.nInsSeq;
		}else {
			pReplaceOpe->ptCaretPos_PHY_After = layoutMgr.LayoutToLogic(LRArg.ptLayoutNew);
			pReplaceOpe->nOrgInsSeq = LRArg.nInsSeq;
		}
		// 操作の追加
		pOpeBlk->AppendOpe(pReplaceOpe);
	}

	// 挿入直後位置へカーソルを移動
	if (bFastMode) {
		caret.MoveCursorFastMode(DLRArg.ptNewPos);
	}else {
		caret.MoveCursor(
			LRArg.ptLayoutNew,	// 挿入された部分の次の位置
			bRedraw
		);
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX();
	}

// 2013.06.29 MoveCaretOpeは不要。ReplaceOpeのみにする
	if (pnInsSeq) {
		*pnInsSeq = bFastMode ? DLRArg.nInsSeq : LRArg.nInsSeq;
	}

	//	Jan. 30, 2001 genta
	//	ファイル全体の更新フラグが立っていないと各行の更新状態が表示されないので
	//	フラグ更新処理を再描画より前に移動する
	return  bUpdateAll;
}


// 2005.10.11 ryoji 前の行にある末尾の空白を削除
void EditView::RTrimPrevLine(void)
{
	auto& caret = GetCaret();
	if (caret.GetCaretLogicPos().y <= 0) {
		return;
	}
	size_t nLineLen;
	const wchar_t* pLine = DocReader(pEditDoc->docLineMgr).GetLineStrWithoutEOL(caret.GetCaretLogicPos().y - 1, &nLineLen);
	if (!pLine || nLineLen == 0) {
		return;
	}
	size_t i = 0;
	size_t j = 0;
	while (i < nLineLen) {
		size_t nCharChars = NativeW::GetSizeOfChar(pLine, nLineLen, i);
		if (!WCODE::IsBlank(pLine[i])) {
			j = i + nCharChars;
		}
		i += nCharChars;
	}
	if (j >= nLineLen) {
		return;
	}
	Range rangeA;
	auto& layoutMgr = pEditDoc->layoutMgr;
	rangeA.SetFrom(layoutMgr.LogicToLayout(Point((int)j, caret.GetCaretLogicPos().y - 1)));
	rangeA.SetTo(layoutMgr.LogicToLayout(Point((int)nLineLen, caret.GetCaretLogicPos().y - 1)));
	if (rangeA.GetFrom().x < rangeA.GetTo().x || rangeA.GetFrom().y != rangeA.GetTo().y) {
		return;
	}
	ReplaceData_CEditView(
		rangeA,
		nullptr,			// 挿入するデータ
		0,	// 挿入するデータの長さ
		true,
		bDoing_UndoRedo ? nullptr : commander.GetOpeBlk()
	);
	Point ptCaretPos_PHY = caret.GetCaretLogicPos();
	Point ptCP = layoutMgr.LogicToLayout(ptCaretPos_PHY);
	caret.MoveCursor(ptCP, true);

	if (!bDoing_UndoRedo) {	// Undo, Redoの実行中か
		// 操作の追加
		commander.GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				caret.GetCaretLogicPos()	// 操作前後のキャレット位置
			)
		);
	}
}


