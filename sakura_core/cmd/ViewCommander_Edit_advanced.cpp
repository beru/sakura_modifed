/*!	@file
@brief ViewCommanderクラスのコマンド(編集系 高度な操作(除単語/行操作))関数群

	2012/12/17	ViewCommander.cpp,ViewCommander_New.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta, みつ
	Copyright (C) 2001, MIK, Stonee, Misaka, asa-o, novice, hor, YAZAKI
	Copyright (C) 2002, hor, YAZAKI, novice, genta, aroka, Azumaiya, minfu, MIK, oak, すなふき, Moca, ai
	Copyright (C) 2003, MIK, genta, かろと, zenryaku, Moca, ryoji, naoh, KEITA, じゅうじ
	Copyright (C) 2004, isearch, Moca, gis_dur, genta, crayonzen, fotomo, MIK, novice, みちばな, Kazika
	Copyright (C) 2005, genta, novice, かろと, MIK, Moca, D.S.Koba, aroka, ryoji, maru
	Copyright (C) 2006, genta, aroka, ryoji, かろと, fon, yukihane, Moca
	Copyright (C) 2007, ryoji, maru, Uchi
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, ryoji, nasukoji
	Copyright (C) 2010, ryoji
	Copyright (C) 2011, ryoji
	Copyright (C) 2012, Moca, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "uiparts/WaitCursor.h"
#include "mem/MemoryIterator.h"	// @@@ 2002.09.28 YAZAKI
#include "_os/OsVersionInfo.h"

using namespace std; // 2002/2/3 aroka to here

#ifndef FID_RECONVERT_VERSION  // 2002.04.10 minfu 
#define FID_RECONVERT_VERSION 0x10000000
#endif
#ifndef SCS_CAP_SETRECONVERTSTRING
#define SCS_CAP_SETRECONVERTSTRING 0x00000004
#define SCS_QUERYRECONVERTSTRING 0x00020000
#define SCS_SETRECONVERTSTRING 0x00010000
#endif

// インデント ver1
void ViewCommander::Command_INDENT(wchar_t wcChar, IndentType eIndent)
{
	using namespace WCODE;

	auto& selInfo = m_view.GetSelectionInfo();
#if 1	// ↓ここを残せば選択幅ゼロを最大にする（従来互換挙動）。無くても Command_INDENT() ver0 が適切に動作するように変更されたので、削除しても特に不都合にはならない。
	// From Here 2001.12.03 hor
	// SPACEorTABインンデントで矩形選択桁がゼロの時は選択範囲を最大にする
	// Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	if (eIndent != IndentType::None
		&& selInfo.IsBoxSelecting()
		&& GetSelect().GetFrom().x == GetSelect().GetTo().x
	) {
		GetSelect().SetToX(GetDocument()->m_layoutMgr.GetMaxLineKetas());
		m_view.RedrawAll();
		return;
	}
	// To Here 2001.12.03 hor
#endif
	Command_INDENT(&wcChar, LogicInt(1), eIndent);
	return;
}


// インデント ver0
/*
	選択された各行の範囲の直前に、与えられた文字列(pData)を挿入する。
	@param eIndent インデントの種別
*/
void ViewCommander::Command_INDENT(
	const wchar_t* const pData,
	const LogicInt nDataLen,
	IndentType eIndent
	)
{
	if (nDataLen <= 0) {
		return;
	}
	auto& layoutMgr = GetDocument()->m_layoutMgr;
	LayoutRange selectOld;		// 範囲選択
	LayoutPoint ptInserted;		// 挿入後の挿入位置
	const struct IsIndentCharSpaceTab {
		IsIndentCharSpaceTab() {}
		bool operator()(const wchar_t ch) const
		{ return ch == WCODE::SPACE || ch == WCODE::TAB; }
	} IsIndentChar;
	struct SoftTabData {
		SoftTabData(LayoutInt nTab) : m_szTab(NULL), m_nTab((Int)nTab) {}
		~SoftTabData() { delete []m_szTab; }
		operator const wchar_t* ()
		{
			if (!m_szTab) {
				m_szTab = new wchar_t[m_nTab];
				wmemset(m_szTab, WCODE::SPACE, m_nTab);
			}
			return m_szTab;
		}
		int Len(LayoutInt nCol) { return m_nTab - ((Int)nCol % m_nTab); }
		wchar_t* m_szTab;
		int m_nTab;
	} stabData(layoutMgr.GetTabSpace());

	const bool bSoftTab = (eIndent == IndentType::Tab && m_view.m_pTypeData->bInsSpace);
	GetDocument()->m_docEditor.SetModified(true, true);	// Jan. 22, 2002 genta

	auto& caret = GetCaret();
	auto& selInfo = m_view.GetSelectionInfo();

	if (!selInfo.IsTextSelected()) {			// テキストが選択されているか
		if (eIndent != IndentType::None && !bSoftTab) {
			// ※矩形選択ではないので Command_WCHAR から呼び戻しされるようなことはない
			Command_WCHAR(pData[0]);	// 1文字入力
		}else {
			// ※矩形選択ではないのでここへ来るのは実際にはソフトタブのときだけ
			if (bSoftTab && !m_view.IsInsMode()) {
				DelCharForOverwrite(pData, nDataLen);
			}
			m_view.InsertData_CEditView(
				caret.GetCaretLayoutPos(),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(caret.GetCaretLayoutPos().GetX2()),
				&ptInserted,
				true
			);
			caret.MoveCursor(ptInserted, true);
			caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
		}
		return;
	}
	const bool bDrawSwitchOld = m_view.SetDrawSwitch(false);	// 2002.01.25 hor
	// 矩形範囲選択中か
	if (selInfo.IsBoxSelecting()) {
// 2012.10.31 Moca 上書きモードのときの選択範囲削除をやめる
#if 0
		// From Here 2001.12.03 hor
		// 上書モードのときは選択範囲削除
		if (!m_view.IsInsMode() /* Oct. 2, 2005 genta */) {
			selectOld = GetSelect();
			m_view.DeleteData(false);
			GetSelect() = selectOld;
			selInfo.SetBoxSelect(true);
		}
		// To Here 2001.12.03 hor
#endif

		// 2点を対角とする矩形を求める
		LayoutRange rcSel;
		TwoPointToRange(
			&rcSel,
			GetSelect().GetFrom(),	// 範囲選択開始
			GetSelect().GetTo()		// 範囲選択終了
		);
		// 現在の選択範囲を非選択状態に戻す
		selInfo.DisableSelectArea(false/*true 2002.01.25 hor*/);

		/*
			文字を直前に挿入された文字が、それにより元の位置からどれだけ後ろにずれたか。
			これに従い矩形選択範囲を後ろにずらす。
		*/
		LayoutInt minOffset(-1);
		/*
			■全角文字の左側の桁揃えについて
			(1) eIndent == IndentType::Tab のとき
				選択範囲がタブ境界にあるときにタブを入力すると、全角文字の前半が選択範囲から
				はみ出している行とそうでない行でタブの幅が、1から設定された最大までと大きく異なり、
				最初に選択されていた文字を選択範囲内にとどめておくことができなくなる。
				最初は矩形選択範囲内にきれいに収まっている行にはタブを挿入せず、ちょっとだけはみ
				出している行にだけタブを挿入することとし、それではどの行にもタブが挿入されない
				とわかったときはやり直してタブを挿入する。
			(2) eIndent == IndentType::Space のとき（※従来互換的な動作）
				幅1で選択している場合のみ全角文字の左側を桁揃えする。
				最初は矩形選択範囲内にきれいに収まっている行にはスペースを挿入せず、ちょっとだけはみ
				出している行にだけスペースを挿入することとし、それではどの行にもスペースが挿入されない
				とわかったときはやり直してスペースを挿入する。
		*/
		bool alignFullWidthChar = (eIndent == IndentType::Tab) && ((rcSel.GetFrom().x % layoutMgr.GetTabSpace()) == 0);
#if 1	// ↓ここを残せば選択幅1のSPACEインデントで全角文字を揃える機能(2)が追加される。
		alignFullWidthChar = alignFullWidthChar || (eIndent == IndentType::Space && rcSel.GetTo().x - rcSel.GetFrom().x == 1);
#endif
		WaitCursor waitCursor(m_view.GetHwnd(), 1000 < rcSel.GetTo().y - rcSel.GetFrom().y);
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if (waitCursor.IsEnable()) {
			hwndProgress = m_view.StartProgress();
		}
		for (bool insertionWasDone=false; ; alignFullWidthChar=false) {
			minOffset = LayoutInt(-1);
			for (LayoutInt nLineNum=rcSel.GetFrom().y; nLineNum<=rcSel.GetTo().y; ++nLineNum) {
				const Layout* pLayout = layoutMgr.SearchLineByLayoutY(nLineNum);
				// Nov. 6, 2002 genta NULLチェック追加
				// これがないとEOF行を含む矩形選択中の文字列入力で落ちる
				LogicInt nIdxFrom, nIdxTo;
				LayoutInt xLayoutFrom, xLayoutTo;
				bool reachEndOfLayout = false;
				if (pLayout) {
					// 指定された桁に対応する行のデータ内の位置を調べる
					const struct {
						LayoutInt keta;
						LogicInt* outLogicX;
						LayoutInt* outLayoutX;
					} sortedKetas[] = {
						{ rcSel.GetFrom().x, &nIdxFrom, &xLayoutFrom },
						{ rcSel.GetTo().x, &nIdxTo, &xLayoutTo },
						{ LayoutInt(-1), 0, 0 }
					};
					MemoryIterator it(pLayout, layoutMgr.GetTabSpace());
					for (int i=0; 0<=sortedKetas[i].keta; ++i) {
						for (; !it.end(); it.addDelta()) {
							if (sortedKetas[i].keta == it.getColumn()) {
								break;
							}
							it.scanNext();
							if (sortedKetas[i].keta < it.getColumn() + it.getColumnDelta()) {
								break;
							}
						}
						*sortedKetas[i].outLogicX = it.getIndex();
						*sortedKetas[i].outLayoutX = it.getColumn();
					}
					reachEndOfLayout = it.end();
				}else {
					nIdxFrom = nIdxTo = LogicInt(0);
					xLayoutFrom = xLayoutTo = LayoutInt(0);
					reachEndOfLayout = true;
				}
				const bool emptyLine = ! pLayout || pLayout->GetLengthWithoutEOL() == 0;
				const bool selectionIsOutOfLine = reachEndOfLayout && (
					(pLayout && pLayout->GetLayoutEol() != EolType::None) ? xLayoutFrom == xLayoutTo : xLayoutTo < rcSel.GetFrom().x
				);

				// 入力文字の挿入位置
				const LayoutPoint ptInsert(selectionIsOutOfLine ? rcSel.GetFrom().x : xLayoutFrom, nLineNum);

				// TABやスペースインデントの時
				if (eIndent != IndentType::None) {
					if (emptyLine || selectionIsOutOfLine) {
						continue; // インデント文字をインデント対象が存在しない部分(改行文字の後ろや空行)に挿入しない。
					}
					/*
						入力がインデント用の文字のとき、ある条件で入力文字を挿入しないことで
						インデントを揃えることができる。
						http://sakura-editor.sourceforge.net/cgi-bin/cyclamen/cyclamen.cgi?log=dev&v=4103
					*/
					if (nIdxFrom == nIdxTo // 矩形選択範囲の右端までに範囲の左端にある文字の末尾が含まれておらず、
						&& ! selectionIsOutOfLine && pLayout && IsIndentChar(pLayout->GetPtr()[nIdxFrom]) // その、末尾の含まれていない文字がインデント文字であり、
						&& rcSel.GetFrom().x < rcSel.GetTo().x // 幅0矩形選択ではない(<<互換性とインデント文字挿入の使い勝手のために除外する)とき。
					) {
						continue;
					}
 					// 全角文字の左側の桁揃え
					if (alignFullWidthChar
						&& (ptInsert.x == rcSel.GetFrom().x || (pLayout && IsIndentChar(pLayout->GetPtr()[nIdxFrom])))
					) {	// 文字の左側が範囲にぴったり収まっている
						minOffset = LayoutInt(0);
						continue;
					}
				}

				// 現在位置にデータを挿入
				m_view.InsertData_CEditView(
					ptInsert,
					!bSoftTab? pData: stabData,
					!bSoftTab? nDataLen: stabData.Len(ptInsert.x),
					&ptInserted,
					false
				);
				insertionWasDone = true;
				minOffset = t_min(
					0 <= minOffset ? minOffset : layoutMgr.GetMaxLineKetas(),
					ptInsert.x <= ptInserted.x ? ptInserted.x - ptInsert.x : t_max(LayoutInt(0), layoutMgr.GetMaxLineKetas() - ptInsert.x)
				);

				caret.MoveCursor(ptInserted, false);
				caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();

				if (hwndProgress) {
					int newPos = ::MulDiv((Int)nLineNum, 100, (Int)rcSel.GetTo().y);
					if (newPos != nProgressPos) {
						nProgressPos = newPos;
						Progress_SetPos(hwndProgress, newPos + 1);
						Progress_SetPos(hwndProgress, newPos);
					}
				}
			}
			if (insertionWasDone || !alignFullWidthChar) {
				break; // ループの必要はない。(1.文字の挿入が行われたから。2.そうではないが文字の挿入を控えたせいではないから)
			}
		}

		if (hwndProgress) {
			::ShowWindow(hwndProgress, SW_HIDE);
		}

		// 挿入された文字の分だけ選択範囲を後ろにずらし、rcSelにセットする。
		if (0 < minOffset) {
			rcSel.GetFromPointer()->x = t_min(rcSel.GetFrom().x + minOffset, layoutMgr.GetMaxLineKetas());
			rcSel.GetToPointer()->x = t_min(rcSel.GetTo().x + minOffset, layoutMgr.GetMaxLineKetas());
		}

		// カーソルを移動
		caret.MoveCursor(rcSel.GetFrom(), true);
		caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();

		if (!m_view.m_bDoing_UndoRedo) {	// Undo, Redoの実行中か
			// 操作の追加
			GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					caret.GetCaretLogicPos(),	// 操作前のキャレット位置
					caret.GetCaretLogicPos()	// 操作後のキャレット位置
				)
			);
		}
		GetSelect().SetFrom(rcSel.GetFrom());	// 範囲選択開始位置
		GetSelect().SetTo(rcSel.GetTo());		// 範囲選択終了位置
		selInfo.SetBoxSelect(true);
	}else if (GetSelect().IsLineOne()) {	// 通常選択(1行内)
		if (eIndent != IndentType::None && !bSoftTab) {
			// ※矩形選択ではないので Command_WCHAR から呼び戻しされるようなことはない
			Command_WCHAR(pData[0]);	// 1文字入力
		}else {
			// ※矩形選択ではないのでここへ来るのは実際にはソフトタブのときだけ
			m_view.DeleteData(false);
			m_view.InsertData_CEditView(
				caret.GetCaretLayoutPos(),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(caret.GetCaretLayoutPos().GetX2()),
				&ptInserted,
				false
			);
			caret.MoveCursor(ptInserted, true);
			caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
		}
	}else {	// 通常選択(複数行)
		selectOld.SetFrom(LayoutPoint(LayoutInt(0), GetSelect().GetFrom().y));
		selectOld.SetTo  (LayoutPoint(LayoutInt(0), GetSelect().GetTo().y ));
		if (GetSelect().GetTo().x > 0) {
			selectOld.GetToPointer()->y++;
		}

		// 現在の選択範囲を非選択状態に戻す
		selInfo.DisableSelectArea(false);

		WaitCursor waitCursor(
			m_view.GetHwnd(),
			1000 < selectOld.GetTo().GetY2() - selectOld.GetFrom().GetY2()
		);
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if (waitCursor.IsEnable()) {
			hwndProgress = m_view.StartProgress();
		}

		for (LayoutInt i=selectOld.GetFrom().GetY2(); i<selectOld.GetTo().GetY2(); ++i) {
			LayoutInt nLineCountPrev = layoutMgr.GetLineCount();
			const Layout* pLayout = layoutMgr.SearchLineByLayoutY(i);
			if (!pLayout ||						// テキストが無いEOLの行は無視
				pLayout->GetLogicOffset() > 0 ||				// 折り返し行は無視
				pLayout->GetLengthWithoutEOL() == 0
			) {	// 改行のみの行は無視する。
				continue;
			}

			// カーソルを移動
			caret.MoveCursor(LayoutPoint(LayoutInt(0), i), false);
			caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();

			// 現在位置にデータを挿入
			m_view.InsertData_CEditView(
				LayoutPoint(LayoutInt(0), i),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(LayoutInt(0)),
				&ptInserted,
				false
			);
			// カーソルを移動
			caret.MoveCursor(ptInserted, false);
			caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();

			if (nLineCountPrev != layoutMgr.GetLineCount()) {
				// 行数が変化した!!
				selectOld.GetToPointer()->y += layoutMgr.GetLineCount() - nLineCountPrev;
			}
			if (hwndProgress) {
				int newPos = ::MulDiv((Int)i, 100, (Int)selectOld.GetTo().GetY());
				if (newPos != nProgressPos) {
					nProgressPos = newPos;
					Progress_SetPos(hwndProgress, newPos + 1);
					Progress_SetPos(hwndProgress, newPos);
				}
			}
		}

		if (hwndProgress) {
			::ShowWindow(hwndProgress, SW_HIDE);
		}

		GetSelect() = selectOld;

		// From Here 2001.12.03 hor
		caret.MoveCursor(GetSelect().GetTo(), true);
		caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
		if (!m_view.m_bDoing_UndoRedo) {	// Undo, Redoの実行中か
			GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					caret.GetCaretLogicPos()	// 操作前後のキャレット位置
				)
			);
		}
		// To Here 2001.12.03 hor
	}
	// 再描画
	m_view.SetDrawSwitch(bDrawSwitchOld);	// 2002.01.25 hor
	m_view.RedrawAll();			// 2002.01.25 hor	// 2009.07.25 ryoji Redraw()->RedrawAll()
	return;
}


// 逆インデント
void ViewCommander::Command_UNINDENT(wchar_t wcChar)
{
	// Aug. 9, 2003 genta
	// 選択されていない場合に逆インデントした場合に
	// 注意メッセージを出す
	auto& selInfo = m_view.GetSelectionInfo();
	if (!selInfo.IsTextSelected()) {	// テキストが選択されているか
		IndentType eIndent;
		switch (wcChar) {
		case WCODE::TAB:
			eIndent = IndentType::Tab;	// ※[SPACEの挿入]オプションが ON ならソフトタブにする（Wiki BugReport/66）
			break;
		case WCODE::SPACE:
			eIndent = IndentType::Space;
			break;
		default:
			eIndent = IndentType::None;
		}
		Command_INDENT(wcChar, eIndent);
		m_view.SendStatusMessage(LS(STR_ERR_UNINDENT1));
		return;
	}

	// 矩形範囲選択中か
	if (selInfo.IsBoxSelecting()) {
		ErrorBeep();
//**********************************************
// 箱型逆インデントについては、保留とする (1998.10.22)
//**********************************************
	}else {
		GetDocument()->m_docEditor.SetModified(true, true);	// Jan. 22, 2002 genta

		LayoutRange selectOld;	// 範囲選択
		selectOld.SetFrom(LayoutPoint(LayoutInt(0), GetSelect().GetFrom().y));
		selectOld.SetTo  (LayoutPoint(LayoutInt(0), GetSelect().GetTo().y ));
		if (GetSelect().GetTo().x > 0) {
			selectOld.GetToPointer()->y++;
		}

		// 現在の選択範囲を非選択状態に戻す
		selInfo.DisableSelectArea(false);

		WaitCursor waitCursor(m_view.GetHwnd(), 1000 < selectOld.GetTo().GetY() - selectOld.GetFrom().GetY());
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if (waitCursor.IsEnable()) {
			hwndProgress = m_view.StartProgress();
		}

		auto& caret = GetCaret();
		auto& layoutMgr = GetDocument()->m_layoutMgr;
		LogicInt nDelLen;
		for (LayoutInt i = selectOld.GetFrom().GetY2(); i < selectOld.GetTo().GetY2(); ++i) {
			LayoutInt nLineCountPrev = layoutMgr.GetLineCount();

			const Layout*	pLayout;
			LogicInt		nLineLen;
			const wchar_t*	pLine = layoutMgr.GetLineStr(i, &nLineLen, &pLayout);
			if (!pLayout || pLayout->GetLogicOffset() > 0) { // 折り返し以降の行はインデント処理を行わない
				continue;
			}

			if (wcChar == WCODE::TAB) {
				if (pLine[0] == wcChar) {
					nDelLen = LogicInt(1);
				}else {
					// 削り取る半角スペース数 (1～タブ幅分) -> nDelLen
					LogicInt i;
					LogicInt nTabSpaces = LogicInt((Int)layoutMgr.GetTabSpace());
					for (i=LogicInt(0); i<nLineLen; ++i) {
						if (pLine[i] != WCODE::SPACE) {
							break;
						}
						// Sep. 23, 2002 genta LayoutMgrの値を使う
						if (i >= nTabSpaces) {
							break;
						}
					}
					if (i == 0) {
						continue;
					}
					nDelLen = i;
				}
			}else {
				if (pLine[0] != wcChar) {
					continue;
				}
				nDelLen = LogicInt(1);
			}

			// カーソルを移動
			caret.MoveCursor(LayoutPoint(LayoutInt(0), i), false);
			caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
			
			// 指定位置の指定長データ削除
			m_view.DeleteData2(
				LayoutPoint(LayoutInt(0), i),
				nDelLen,	// 2001.12.03 hor
				NULL
			);
			if (nLineCountPrev != layoutMgr.GetLineCount()) {
				// 行数が変化した!!
				selectOld.GetToPointer()->y += layoutMgr.GetLineCount() - nLineCountPrev;
			}
			if (hwndProgress) {
				int newPos = ::MulDiv((Int)i, 100, (Int)selectOld.GetTo().GetY());
				if (newPos != nProgressPos) {
					nProgressPos = newPos;
					Progress_SetPos(hwndProgress, newPos + 1);
					Progress_SetPos(hwndProgress, newPos);
				}
			}
		}
		if (hwndProgress) {
			::ShowWindow(hwndProgress, SW_HIDE);
		}
		GetSelect() = selectOld;	// 範囲選択

		// From Here 2001.12.03 hor
		caret.MoveCursor(GetSelect().GetTo(), true);
		caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
		if (!m_view.m_bDoing_UndoRedo) {	// Undo, Redoの実行中か
			GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					caret.GetCaretLogicPos()	// 操作前後のキャレット位置
				)
			);
		}
		// To Here 2001.12.03 hor
	}

	// 再描画
	m_view.RedrawAll();	// 2002.01.25 hor	// 2009.07.25 ryoji Redraw()->RedrawAll()
}


// from ViewCommander_New.cpp
/*! TRIM Step1
	非選択時はカレント行を選択して m_view.ConvSelectedArea → ConvMemory へ
	@author hor
	@date 2001.12.03 hor 新規作成
*/
void ViewCommander::Command_TRIM(
	bool bLeft	//  [in] FALSE: 右TRIM / それ以外: 左TRIM
	)
{
	bool bBeDisableSelectArea = false;
	ViewSelect& viewSelect = m_view.GetSelectionInfo();

	if (!viewSelect.IsTextSelected()) {	// 非選択時は行選択に変更
		viewSelect.m_select.SetFrom(
			LayoutPoint(
				LayoutInt(0),
				GetCaret().GetCaretLayoutPos().GetY()
			)
		);
		viewSelect.m_select.SetTo(
			LayoutPoint(
				GetDocument()->m_layoutMgr.GetMaxLineKetas(),
				GetCaret().GetCaretLayoutPos().GetY()
			)
		);
		bBeDisableSelectArea = true;
	}

	m_view.ConvSelectedArea(bLeft ? F_LTRIM : F_RTRIM);

	if (bBeDisableSelectArea) {
		viewSelect.DisableSelectArea(true);
	}
}


// from ViewCommander_New.cpp
// 物理行のソートに使う構造体
struct SortData {
	const NativeW* pMemLine;
	StringRef sKey;
};

inline
int CNativeW_comp(
	const NativeW& lhs,
	const NativeW& rhs
	)
{
	// 比較長には終端NULを含めないといけない
	return wmemcmp(
		lhs.GetStringPtr(),
		rhs.GetStringPtr(),
		t_min(lhs.GetStringLength() + 1, rhs.GetStringLength() + 1)
	);
}

// 物理行のソートに使う関数(昇順)
bool SortByLineAsc (SortData* pst1, SortData* pst2) {return CNativeW_comp(*pst1->pMemLine, *pst2->pMemLine) < 0;}

// 物理行のソートに使う関数(降順)
bool SortByLineDesc(SortData* pst1, SortData* pst2) {return CNativeW_comp(*pst1->pMemLine, *pst2->pMemLine) > 0;}

inline
int CStringRef_comp(
	const StringRef& c1,
	const StringRef& c2
	)
{
	int ret = wmemcmp(
		c1.GetPtr(),
		c2.GetPtr(),
		t_min(c1.GetLength(), c2.GetLength())
	);
	if (ret == 0) {
		return c1.GetLength() - c2.GetLength();
	}
	return ret;
}

// 物理行のソートに使う関数(昇順)
bool SortByKeyAsc(SortData* pst1, SortData* pst2)  {return CStringRef_comp(pst1->sKey, pst2->sKey) < 0 ;}

// 物理行のソートに使う関数(降順)
bool SortByKeyDesc(SortData* pst1, SortData* pst2) {return CStringRef_comp(pst1->sKey, pst2->sKey) > 0 ;}

/*!	@brief 物理行のソート

	非選択時は何も実行しない．矩形選択時は、その範囲をキーにして物理行をソート．
	
	@note とりあえず改行コードを含むデータをソートしているので、
	ファイルの最終行はソート対象外にしています
	@author hor
	@date 2001.12.03 hor 新規作成
	@date 2001.12.21 hor 選択範囲の調整ロジックを訂正
	@date 2010.07.27 行ソートでコピーを減らす/NULより後ろも比較対照に
	@date 2013.06.19 Moca 矩形選択時最終行に改行がない場合は付加+ソート後の最終行の改行を削除
*/
void ViewCommander::Command_SORT(bool bAsc)	// bAsc:true=昇順, false=降順
{
	LayoutRange rangeA;
	LogicRange selectOld;

	int			nColumnFrom, nColumnTo;
	LayoutInt	nCF(0), nCT(0);
	LayoutInt	nCaretPosYOLD;
	bool		bBeginBoxSelectOld;
	LogicInt	nLineLen;
	std::vector<SortData*> sta;

	auto& selInfo = m_view.GetSelectionInfo();
	if (!selInfo.IsTextSelected()) {			// テキストが選択されているか
		return;
	}
	auto& layoutMgr = GetDocument()->m_layoutMgr;
	if (selInfo.IsBoxSelecting()) {
		rangeA = selInfo.m_select;
		if (selInfo.m_select.GetFrom().x == selInfo.m_select.GetTo().x) {
			// Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
			selInfo.m_select.SetToX(layoutMgr.GetMaxLineKetas());
		}
		if (selInfo.m_select.GetFrom().x<selInfo.m_select.GetTo().x) {
			nCF = selInfo.m_select.GetFrom().GetX2();
			nCT = selInfo.m_select.GetTo().GetX2();
		}else {
			nCF = selInfo.m_select.GetTo().GetX2();
			nCT = selInfo.m_select.GetFrom().GetX2();
		}
	}
	bBeginBoxSelectOld = selInfo.IsBoxSelecting();
	nCaretPosYOLD = GetCaret().GetCaretLayoutPos().GetY();
	layoutMgr.LayoutToLogic(
		selInfo.m_select,
		&selectOld
	);

	if (bBeginBoxSelectOld) {
		selectOld.GetToPointer()->y++;
	}else {
		// カーソル位置が行頭じゃない ＆ 選択範囲の終端に改行コードがある場合は
		// その行も選択範囲に加える
		if (selectOld.GetTo().x > 0) {
			// 2006.03.31 Moca nSelectLineToOldは、物理行なのでLayout系からDocLine系に修正
			const DocLine* pDocLine = GetDocument()->m_docLineMgr.GetLine(selectOld.GetTo().GetY2());
			if (pDocLine && EolType::None != pDocLine->GetEol()) {
				selectOld.GetToPointer()->y++;
			}
		}
	}
	selectOld.SetFromX(LogicInt(0));
	selectOld.SetToX(LogicInt(0));

	// 行選択されてない
	if (selectOld.IsLineOne()) {
		return;
	}
	
	sta.reserve(selectOld.GetTo().GetY2() - selectOld.GetFrom().GetY2());
	for (LogicInt i=selectOld.GetFrom().GetY2(); i<selectOld.GetTo().y; ++i) {
		const DocLine* pDocLine = GetDocument()->m_docLineMgr.GetLine(i);
		const NativeW& memLine = pDocLine->_GetDocLineDataWithEOL();
		const wchar_t* pLine = memLine.GetStringPtr(&nLineLen);
		LogicInt nLineLenWithoutEOL = pDocLine->GetLengthWithoutEOL();
		if (!pLine) {
			continue;
		}
		SortData* pst = new SortData;
		if (bBeginBoxSelectOld) {
			nColumnFrom = m_view.LineColumnToIndex(pDocLine, nCF);
			nColumnTo   = m_view.LineColumnToIndex(pDocLine, nCT);
			if (nColumnTo < nLineLenWithoutEOL) {	// BOX選択範囲の右端が行内に収まっている場合
				// 2006.03.31 genta std::string::assignを使って一時変数削除
				pst->sKey = StringRef(&pLine[nColumnFrom], nColumnTo - nColumnFrom);
			}else if (nColumnFrom < nLineLenWithoutEOL) {	// BOX選択範囲の右端が行末より右にはみ出している場合
				pst->sKey = StringRef(&pLine[nColumnFrom], nLineLenWithoutEOL - nColumnFrom);
			}else {
				// 選択範囲の左端もはみ出している == データなし
				pst->sKey = StringRef(L"", 0);
			}
		}
		pst->pMemLine = &memLine;
		sta.push_back(pst);
	}
	const wchar_t* pStrLast = NULL; // 最後の行に改行がなければそのポインタ
	if (0 < sta.size()) {
		pStrLast = sta[sta.size() - 1]->pMemLine->GetStringPtr();
		int nlen = sta[sta.size() - 1]->pMemLine->GetStringLength();
		if (0 < nlen) {
			if (WCODE::IsLineDelimiter(pStrLast[nlen - 1], GetDllShareData().common.edit.bEnableExtEol)) {
				pStrLast = NULL;
			}
		}
	}
	if (bBeginBoxSelectOld) {
		std::stable_sort(
			sta.begin(),
			sta.end(),
			(bBeginBoxSelectOld ? (bAsc ? SortByKeyAsc : SortByKeyDesc) : (bAsc ? SortByLineAsc : SortByLineDesc))
		);
	}
	OpeLineData repData;
	int j = (int)sta.size();
	repData.resize(sta.size());
	int opeSeq = GetDocument()->m_docEditor.m_opeBuf.GetNextSeq();
	for (int i=0; i<j; ++i) {
		repData[i].nSeq = opeSeq;
		repData[i].memLine.SetString(sta[i]->pMemLine->GetStringPtr(), sta[i]->pMemLine->GetStringLength());
		if (pStrLast == sta[i]->pMemLine->GetStringPtr()) {
			// 元最終行に改行がないのでつける
			Eol cWork = GetDocument()->m_docEditor.GetNewLineCode();
			repData[i].memLine.AppendString(cWork.GetValue2(), cWork.GetLen());
		}
	}
	if (pStrLast) {
		// 最終行の改行を削除
		LineData& lastData = repData[repData.size() - 1];
		int nLen = lastData.memLine.GetStringLength();
		bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
		while (0 <nLen && WCODE::IsLineDelimiter(lastData.memLine[nLen-1], bExtEol)) {
			--nLen;
		}
		lastData.memLine._SetStringLength(nLen);
	}
	// 2010.08.22 Moca swapで削除
	{
		std::vector<SortData*> temp;
		temp.swap(sta);
	}

	LayoutRange selectOld_Layout;
	layoutMgr.LogicToLayout(selectOld, &selectOld_Layout);
	m_view.ReplaceData_CEditView3(
		selectOld_Layout,
		NULL,
		&repData,
		false,
		m_view.m_bDoing_UndoRedo ? NULL : GetOpeBlk(),
		opeSeq,
		NULL
	);

	// 選択エリアの復元
	if (bBeginBoxSelectOld) {
		selInfo.SetBoxSelect(bBeginBoxSelectOld);
		selInfo.m_select = rangeA;
	}else {
		selInfo.m_select = selectOld_Layout;
	}
	if (nCaretPosYOLD == selInfo.m_select.GetFrom().y || selInfo.IsBoxSelecting()) {
		GetCaret().MoveCursor(selInfo.m_select.GetFrom(), true);
	}else {
		GetCaret().MoveCursor(selInfo.m_select.GetTo(), true);
	}
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();
	if (!m_view.m_bDoing_UndoRedo) {	// Undo, Redoの実行中か
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
			)
		);
	}
	m_view.RedrawAll();
}


// from ViewCommander_New.cpp
/*! @brief 物理行のマージ

	連続する物理行で内容が同一の物を1行にまとめます．
	
	矩形選択時はなにも実行しません．
	
	@note 改行コードを含むデータを比較しているので、
	ファイルの最終行はソート対象外にしています
	
	@author hor
	@date 2001.12.03 hor 新規作成
	@date 2001.12.21 hor 選択範囲の調整ロジックを訂正
*/
void ViewCommander::Command_MERGE(void)
{
	LayoutInt	nCaretPosYOLD;
	LogicInt	nLineLen;
	LayoutInt	nMergeLayoutLines;

	auto& selInfo = m_view.GetSelectionInfo();
	if (!selInfo.IsTextSelected()) {			// テキストが選択されているか
		return;
	}
	if (selInfo.IsBoxSelecting()) {
		return;
	}
	auto& layoutMgr = GetDocument()->m_layoutMgr;
	nCaretPosYOLD = GetCaret().GetCaretLayoutPos().GetY();
	LogicRange sSelectOld; // 範囲選択
	layoutMgr.LayoutToLogic(
		selInfo.m_select,
		&sSelectOld
	);

	// 2001.12.21 hor
	// カーソル位置が行頭じゃない ＆ 選択範囲の終端に改行コードがある場合は
	// その行も選択範囲に加える
	if (sSelectOld.GetTo().x > 0) {
#if 0
		const Layout* pLayout = layoutMgr.SearchLineByLayoutY(selInfo.m_select.GetTo().GetY2()); // 2007.10.09 kobake 単位混在バグ修正
		if (pLayout && EolType::None != pLayout->GetLayoutEol()) {
			selectOld.GetToPointer()->y++;
			//selectOld.GetTo().y++;
		}
#else
		// 2010.08.22 Moca ソートと仕様を合わせる
		const DocLine* pDocLine = GetDocument()->m_docLineMgr.GetLine(sSelectOld.GetTo().GetY2());
		if (pDocLine && EolType::None != pDocLine->GetEol()) {
			sSelectOld.GetToPointer()->y++;
		}
#endif
	}

	sSelectOld.SetFromX(LogicInt(0));
	sSelectOld.SetToX(LogicInt(0));

	// 行選択されてない
	if (sSelectOld.IsLineOne()) {
		return;
	}

	int j = GetDocument()->m_docLineMgr.GetLineCount();
	nMergeLayoutLines = layoutMgr.GetLineCount();

	LayoutRange selectOld_Layout;
	layoutMgr.LogicToLayout(sSelectOld, &selectOld_Layout);

	// 2010.08.22 NUL対応修正
	std::vector<StringRef> lineArr;
	const wchar_t* pLinew = NULL;
	int nLineLenw = 0;
	bool bMerge = false;
	lineArr.reserve(sSelectOld.GetTo().y - sSelectOld.GetFrom().GetY2());
	for (LogicInt i=sSelectOld.GetFrom().GetY2(); i<sSelectOld.GetTo().y; ++i) {
		const wchar_t* pLine = GetDocument()->m_docLineMgr.GetLine(i)->GetDocLineStrWithEOL(&nLineLen);
		if (!pLine) continue;
		if (!pLinew || nLineLen != nLineLenw || wmemcmp(pLine, pLinew, nLineLen)) {
			lineArr.emplace_back(pLine, nLineLen);
		}else {
			bMerge = true;
		}
		pLinew = pLine;
		nLineLenw = nLineLen;
	}
	if (bMerge) {
		OpeLineData repData;
		int nSize = (int)lineArr.size();
		repData.resize(nSize);
		int opeSeq = GetDocument()->m_docEditor.m_opeBuf.GetNextSeq();
		for (int idx=0; idx<nSize; ++idx) {
			repData[idx].nSeq = opeSeq;
			repData[idx].memLine.SetString(lineArr[idx].GetPtr(), lineArr[idx].GetLength());
		}
		m_view.ReplaceData_CEditView3(
			selectOld_Layout,
			NULL,
			&repData,
			false,
			m_view.m_bDoing_UndoRedo ? NULL : GetOpeBlk(),
			opeSeq,
			NULL
		);
	}else {
		// 2010.08.23 未変更なら変更しない
	}

	j -= GetDocument()->m_docLineMgr.GetLineCount();
	nMergeLayoutLines -= layoutMgr.GetLineCount();

	// 選択エリアの復元
	selInfo.m_select = selectOld_Layout;
	// 2010.08.22 座標混在バグ
	selInfo.m_select.GetToPointer()->y -= nMergeLayoutLines;

	if (nCaretPosYOLD == selInfo.m_select.GetFrom().y) {
		GetCaret().MoveCursor(selInfo.m_select.GetFrom(), true);
	}else {
		GetCaret().MoveCursor(selInfo.m_select.GetTo(), true);
	}
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();
	if (!m_view.m_bDoing_UndoRedo) {	// Undo, Redoの実行中か
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
			)
		);
	}
	m_view.RedrawAll();

	if (j) {
		TopOkMessage(m_view.GetHwnd(), LS(STR_ERR_DLGEDITVWCMDNW7), j);
	}else {
		InfoMessage(m_view.GetHwnd(), LS(STR_ERR_DLGEDITVWCMDNW8));
	}
}


// from ViewCommander_New.cpp
/* メニューからの再変換対応 minfu 2002.04.09

	@date 2002.04.11 YAZAKI COsVersionInfoのカプセル化を守りましょう。
	@date 2010.03.17 ATOK用はSCS_SETRECONVERTSTRING => ATRECONVERTSTRING_SETに変更
		2002.11.20 Stoneeさんの情報
*/
void ViewCommander::Command_Reconvert(void)
{
	const int ATRECONVERTSTRING_SET = 1;

	// サイズを取得
	int nSize = m_view.SetReconvertStruct(NULL, UNICODE_BOOL);
	if (nSize == 0)  // サイズ０の時は何もしない
		return ;

	bool bUseUnicodeATOK = false;
	// バージョンチェック
	if (!OsSupportReconvert()) {
		
		// MSIMEかどうか
		HWND hWnd = ImmGetDefaultIMEWnd(m_view.GetHwnd());
		if (SendMessage(hWnd, m_view.m_uWM_MSIME_RECONVERTREQUEST, FID_RECONVERT_VERSION, 0)) {
			SendMessage(hWnd, m_view.m_uWM_MSIME_RECONVERTREQUEST, 0, (LPARAM)m_view.GetHwnd());
			return ;
		}

		// ATOKが使えるかどうか
		TCHAR sz[256];
		ImmGetDescription(GetKeyboardLayout(0), sz, _countof(sz)); // 説明の取得
		if ((_tcsncmp(sz, _T("ATOK"),4) == 0) && m_view.m_AT_ImmSetReconvertString) {
			bUseUnicodeATOK = true;
		}else {
			// 対応IMEなし
			return;
		}
	}else {
		// 現在のIMEが対応しているかどうか
		// IMEのプロパティ
		if (!(ImmGetProperty(GetKeyboardLayout(0), IGP_SETCOMPSTR) & SCS_CAP_SETRECONVERTSTRING)) {
			// 対応IMEなし
			return ;
		}
	}

	// サイズ取得し直し
	if (!UNICODE_BOOL && bUseUnicodeATOK) {
		nSize = m_view.SetReconvertStruct(NULL, UNICODE_BOOL || bUseUnicodeATOK);
		if (nSize == 0)  // サイズ０の時は何もしない
			return;
	}

	// IMEのコンテキスト取得
	HIMC hIMC = ::ImmGetContext(m_view.GetHwnd());
	
	// 領域確保
	PRECONVERTSTRING pReconv = (PRECONVERTSTRING)::HeapAlloc(
		GetProcessHeap(),
		HEAP_GENERATE_EXCEPTIONS,
		nSize
	);
	
	// 構造体設定
	// Sizeはバッファ確保側が設定
	pReconv->dwSize = nSize;
	pReconv->dwVersion = 0;
	m_view.SetReconvertStruct(pReconv, UNICODE_BOOL || bUseUnicodeATOK);
	
	// 変換範囲の調整
	if (bUseUnicodeATOK) {
		(*m_view.m_AT_ImmSetReconvertString)(hIMC, ATRECONVERTSTRING_SET, pReconv, pReconv->dwSize);
	}else {
		::ImmSetCompositionString(hIMC, SCS_QUERYRECONVERTSTRING, pReconv, pReconv->dwSize, NULL, 0);
	}

	// 調整した変換範囲を選択する
	m_view.SetSelectionFromReonvert(pReconv, UNICODE_BOOL || bUseUnicodeATOK);
	
	// 再変換実行
	if (bUseUnicodeATOK) {
		(*m_view.m_AT_ImmSetReconvertString)(hIMC, ATRECONVERTSTRING_SET, pReconv, pReconv->dwSize);
	}else {
		::ImmSetCompositionString(hIMC, SCS_SETRECONVERTSTRING, pReconv, pReconv->dwSize, NULL, 0);
	}

	// 領域解放
	::HeapFree(GetProcessHeap(), 0, (LPVOID)pReconv);
	::ImmReleaseContext(m_view.GetHwnd(), hIMC);
}

