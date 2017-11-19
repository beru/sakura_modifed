#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "uiparts/WaitCursor.h"
#include "mem/MemoryIterator.h"
#include "_os/OsVersionInfo.h"

// ViewCommanderクラスのコマンド(編集系 高度な操作(除単語/行操作))関数群

using namespace std;

#ifndef FID_RECONVERT_VERSION
#define FID_RECONVERT_VERSION 0x10000000
#endif
#ifndef SCS_CAP_SETRECONVERTSTRING
#define SCS_CAP_SETRECONVERTSTRING 0x00000004
#define SCS_QUERYRECONVERTSTRING 0x00020000
#define SCS_SETRECONVERTSTRING 0x00010000
#endif

// インデント ver1
void ViewCommander::Command_Indent(wchar_t wcChar, IndentType eIndent)
{
	using namespace WCODE;

	auto& selInfo = view.GetSelectionInfo();
#if 1	// ↓ここを残せば選択幅ゼロを最大にする（従来互換挙動）。無くても Command_Indent() ver0 が適切に動作するように変更されたので、削除しても特に不都合にはならない。
	// SPACEorTABインンデントで矩形選択桁がゼロの時は選択範囲を最大にする
	if (eIndent != IndentType::None
		&& selInfo.IsBoxSelecting()
		&& GetSelect().GetFrom().x == GetSelect().GetTo().x
	) {
		GetSelect().SetToX((int)GetDocument().layoutMgr.GetMaxLineKetas());
		view.RedrawAll();
		return;
	}
#endif
	Command_Indent(&wcChar, 1, eIndent);
	return;
}


// インデント ver0
/*
	選択された各行の範囲の直前に、与えられた文字列(pData)を挿入する。
	@param eIndent インデントの種別
*/
void ViewCommander::Command_Indent(
	const wchar_t* const pData,
	const size_t nDataLen,
	IndentType eIndent
	)
{
	if (nDataLen == 0) {
		return;
	}
	auto& layoutMgr = GetDocument().layoutMgr;
	Range selectOld;		// 範囲選択
	Point ptInserted;		// 挿入後の挿入位置
	const struct IsIndentCharSpaceTab {
		IsIndentCharSpaceTab() {}
		bool operator()(const wchar_t ch) const
		{ return ch == WCODE::SPACE || ch == WCODE::TAB; }
	} IsIndentChar;
	struct SoftTabData {
		SoftTabData(size_t nTab) : szTab(NULL), nTab(nTab) {}
		~SoftTabData() { delete[] szTab; }
		operator const wchar_t* ()
		{
			if (!szTab) {
				szTab = new wchar_t[nTab];
				wmemset(szTab, WCODE::SPACE, nTab);
			}
			return szTab;
		}
		size_t Len(size_t nCol) { return nTab - (nCol % nTab); }
		wchar_t* szTab;
		size_t nTab;
	} stabData(layoutMgr.GetTabSpace());

	const bool bSoftTab = (eIndent == IndentType::Tab && view.pTypeData->bInsSpace);
	GetDocument().docEditor.SetModified(true, true);

	auto& caret = GetCaret();
	auto& selInfo = view.GetSelectionInfo();

	if (!selInfo.IsTextSelected()) {			// テキストが選択されているか
		if (eIndent != IndentType::None && !bSoftTab) {
			// ※矩形選択ではないので Command_WCHAR から呼び戻しされるようなことはない
			Command_WCHAR(pData[0]);	// 1文字入力
		}else {
			// ※矩形選択ではないのでここへ来るのは実際にはソフトタブのときだけ
			if (bSoftTab && !view.IsInsMode()) {
				DelCharForOverwrite(pData, nDataLen);
			}
			view.InsertData_CEditView(
				caret.GetCaretLayoutPos(),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(caret.GetCaretLayoutPos().x),
				&ptInserted,
				true
			);
			caret.MoveCursor(ptInserted, true);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
		}
		return;
	}
	const bool bDrawSwitchOld = view.SetDrawSwitch(false);
	// 矩形範囲選択中か
	if (selInfo.IsBoxSelecting()) {

		// 2点を対角とする矩形を求める
		Range rcSel;
		TwoPointToRange(
			&rcSel,
			GetSelect().GetFrom(),	// 範囲選択開始
			GetSelect().GetTo()		// 範囲選択終了
		);
		// 現在の選択範囲を非選択状態に戻す
		selInfo.DisableSelectArea(false);

		/*
			文字を直前に挿入された文字が、それにより元の位置からどれだけ後ろにずれたか。
			これに従い矩形選択範囲を後ろにずらす。
		*/
		int minOffset = -1;
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
		WaitCursor waitCursor(view.GetHwnd(), 1000 < rcSel.GetTo().y - rcSel.GetFrom().y);
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if (waitCursor.IsEnable()) {
			hwndProgress = view.StartProgress();
		}
		for (bool insertionWasDone=false; ; alignFullWidthChar=false) {
			minOffset = -1;
			for (int nLineNum=rcSel.GetFrom().y; nLineNum<=rcSel.GetTo().y; ++nLineNum) {
				const Layout* pLayout = layoutMgr.SearchLineByLayoutY(nLineNum);
				// これがないとEOF行を含む矩形選択中の文字列入力で落ちる
				int nIdxFrom, nIdxTo;
				int xLayoutFrom, xLayoutTo;
				bool reachEndOfLayout = false;
				if (pLayout) {
					// 指定された桁に対応する行のデータ内の位置を調べる
					const struct {
						int keta;
						int* outLogicX;
						int* outLayoutX;
					} sortedKetas[] = {
						{ rcSel.GetFrom().x, &nIdxFrom, &xLayoutFrom },
						{ rcSel.GetTo().x, &nIdxTo, &xLayoutTo },
						{ -1, 0, 0 }
					};
					MemoryIterator it(pLayout, layoutMgr.GetTabSpace());
					for (int i=0; 0<=sortedKetas[i].keta; ++i) {
						for (; !it.end(); it.addDelta()) {
							if (sortedKetas[i].keta == it.getColumn()) {
								break;
							}
							it.scanNext();
							if (sortedKetas[i].keta < (int)(it.getColumn() + it.getColumnDelta())) {
								break;
							}
						}
						*sortedKetas[i].outLogicX = (int)it.getIndex();
						*sortedKetas[i].outLayoutX = (int)it.getColumn();
					}
					reachEndOfLayout = it.end();
				}else {
					nIdxFrom = nIdxTo = 0;
					xLayoutFrom = xLayoutTo = 0;
					reachEndOfLayout = true;
				}
				const bool emptyLine = ! pLayout || pLayout->GetLengthWithoutEOL() == 0;
				const bool selectionIsOutOfLine = reachEndOfLayout && (
					(pLayout && pLayout->GetLayoutEol() != EolType::None) ? xLayoutFrom == xLayoutTo : xLayoutTo < rcSel.GetFrom().x
				);

				// 入力文字の挿入位置
				const Point ptInsert(selectionIsOutOfLine ? rcSel.GetFrom().x : xLayoutFrom, nLineNum);

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
						minOffset = 0;
						continue;
					}
				}

				// 現在位置にデータを挿入
				view.InsertData_CEditView(
					ptInsert,
					!bSoftTab? pData: stabData,
					!bSoftTab? nDataLen: stabData.Len(ptInsert.x),
					&ptInserted,
					false
				);
				insertionWasDone = true;
				minOffset = t_min(
					(0 <= minOffset) ? minOffset : (int)layoutMgr.GetMaxLineKetas(),
					(ptInsert.x <= ptInserted.x) ? (int)(ptInserted.x - ptInsert.x) : t_max(0, (int)layoutMgr.GetMaxLineKetas() - (int)ptInsert.x)
				);

				caret.MoveCursor(ptInserted, false);
				caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

				if (hwndProgress) {
					int newPos = ::MulDiv(nLineNum, 100, rcSel.GetTo().y);
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
			rcSel.GetFrom().x = t_min((int)rcSel.GetFrom().x + minOffset, (int)layoutMgr.GetMaxLineKetas());
			rcSel.GetTo().x = t_min((int)rcSel.GetTo().x + minOffset, (int)layoutMgr.GetMaxLineKetas());
		}

		// カーソルを移動
		caret.MoveCursor(rcSel.GetFrom(), true);
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

		if (!view.bDoing_UndoRedo) {	// Undo, Redoの実行中か
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
			view.DeleteData(false);
			view.InsertData_CEditView(
				caret.GetCaretLayoutPos(),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(caret.GetCaretLayoutPos().x),
				&ptInserted,
				false
			);
			caret.MoveCursor(ptInserted, true);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
		}
	}else {	// 通常選択(複数行)
		selectOld.SetFrom(Point(0, GetSelect().GetFrom().y));
		selectOld.SetTo  (Point(0, GetSelect().GetTo().y ));
		if (GetSelect().GetTo().x > 0) {
			selectOld.GetTo().y++;
		}

		// 現在の選択範囲を非選択状態に戻す
		selInfo.DisableSelectArea(false);

		WaitCursor waitCursor(
			view.GetHwnd(),
			1000 < selectOld.GetTo().y - selectOld.GetFrom().y
		);
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if (waitCursor.IsEnable()) {
			hwndProgress = view.StartProgress();
		}

		for (int i=selectOld.GetFrom().y; i<selectOld.GetTo().y; ++i) {
			size_t nLineCountPrev = layoutMgr.GetLineCount();
			const Layout* pLayout = layoutMgr.SearchLineByLayoutY(i);
			if (!pLayout ||						// テキストが無いEOLの行は無視
				pLayout->GetLogicOffset() > 0 ||				// 折り返し行は無視
				pLayout->GetLengthWithoutEOL() == 0
			) {	// 改行のみの行は無視する。
				continue;
			}

			// カーソルを移動
			caret.MoveCursor(Point(0, i), false);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

			// 現在位置にデータを挿入
			view.InsertData_CEditView(
				Point(0, i),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(0),
				&ptInserted,
				false
			);
			// カーソルを移動
			caret.MoveCursor(ptInserted, false);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

			if (nLineCountPrev != layoutMgr.GetLineCount()) {
				// 行数が変化した!!
				selectOld.GetTo().y += (int)layoutMgr.GetLineCount() - (int)nLineCountPrev;
			}
			if (hwndProgress) {
				int newPos = ::MulDiv(i, 100, selectOld.GetTo().GetY());
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

		caret.MoveCursor(GetSelect().GetTo(), true);
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
		if (!view.bDoing_UndoRedo) {	// Undo, Redoの実行中か
			GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					caret.GetCaretLogicPos()	// 操作前後のキャレット位置
				)
			);
		}
	}
	// 再描画
	view.SetDrawSwitch(bDrawSwitchOld);
	view.RedrawAll();
	return;
}


// 逆インデント
void ViewCommander::Command_Unindent(wchar_t wcChar)
{
	// 選択されていない場合に逆インデントした場合に
	// 注意メッセージを出す
	auto& selInfo = view.GetSelectionInfo();
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
		Command_Indent(wcChar, eIndent);
		view.SendStatusMessage(LS(STR_ERR_UNINDENT1));
		return;
	}

	// 矩形範囲選択中か
	if (selInfo.IsBoxSelecting()) {
		ErrorBeep();
//**********************************************
// 箱型逆インデントについては、保留とする
//**********************************************
	}else {
		GetDocument().docEditor.SetModified(true, true);

		Range selectOld;	// 範囲選択
		selectOld.SetFrom(Point(0, GetSelect().GetFrom().y));
		selectOld.SetTo  (Point(0, GetSelect().GetTo().y ));
		if (GetSelect().GetTo().x > 0) {
			selectOld.GetTo().y++;
		}

		// 現在の選択範囲を非選択状態に戻す
		selInfo.DisableSelectArea(false);

		WaitCursor waitCursor(view.GetHwnd(), 1000 < selectOld.GetTo().GetY() - selectOld.GetFrom().GetY());
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if (waitCursor.IsEnable()) {
			hwndProgress = view.StartProgress();
		}

		auto& caret = GetCaret();
		auto& layoutMgr = GetDocument().layoutMgr;
		size_t nDelLen;
		for (int i = selectOld.GetFrom().y; i < selectOld.GetTo().y; ++i) {
			size_t nLineCountPrev = layoutMgr.GetLineCount();

			const Layout*	pLayout;
			size_t nLineLen;
			const wchar_t*	pLine = layoutMgr.GetLineStr(i, &nLineLen, &pLayout);
			if (!pLayout || pLayout->GetLogicOffset() > 0) { // 折り返し以降の行はインデント処理を行わない
				continue;
			}

			if (wcChar == WCODE::TAB) {
				if (pLine[0] == wcChar) {
					nDelLen = 1;
				}else {
					// 削り取る半角スペース数 (1～タブ幅分) -> nDelLen
					size_t i;
					size_t nTabSpaces = layoutMgr.GetTabSpace();
					for (i=0; i<nLineLen; ++i) {
						if (pLine[i] != WCODE::SPACE) {
							break;
						}
						// LayoutMgrの値を使う
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
				nDelLen = 1;
			}

			// カーソルを移動
			caret.MoveCursor(Point(0, i), false);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
			
			// 指定位置の指定長データ削除
			view.DeleteData2(
				Point(0, i),
				nDelLen,
				nullptr
			);
			if (nLineCountPrev != layoutMgr.GetLineCount()) {
				// 行数が変化した!!
				selectOld.GetTo().y += (int)layoutMgr.GetLineCount() - (int)nLineCountPrev;
			}
			if (hwndProgress) {
				int newPos = ::MulDiv(i, 100, selectOld.GetTo().GetY());
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

		caret.MoveCursor(GetSelect().GetTo(), true);
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
		if (!view.bDoing_UndoRedo) {	// Undo, Redoの実行中か
			GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					caret.GetCaretLogicPos()	// 操作前後のキャレット位置
				)
			);
		}
	}

	// 再描画
	view.RedrawAll();
}


// from ViewCommander_New.cpp
/*! TRIM Step1
	非選択時はカレント行を選択して view.ConvSelectedArea → ConvMemory へ
*/
void ViewCommander::Command_Trim(
	bool bLeft	//  [in] false: 右TRIM / それ以外: 左TRIM
	)
{
	bool bBeDisableSelectArea = false;
	ViewSelect& viewSelect = view.GetSelectionInfo();

	if (!viewSelect.IsTextSelected()) {	// 非選択時は行選択に変更
		viewSelect.select.SetFrom(
			Point(
				0,
				GetCaret().GetCaretLayoutPos().GetY()
			)
		);
		viewSelect.select.SetTo(
			Point(
				(int)GetDocument().layoutMgr.GetMaxLineKetas(),
				GetCaret().GetCaretLayoutPos().GetY()
			)
		);
		bBeDisableSelectArea = true;
	}

	view.ConvSelectedArea(bLeft ? F_LTRIM : F_RTRIM);

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
int64_t CStringRef_comp(
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
		return (int64_t)c1.GetLength() - (int64_t)c2.GetLength();
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
*/
void ViewCommander::Command_Sort(bool bAsc)	// bAsc:true=昇順, false=降順
{
	Range rangeA;
	Range selectOld;

	size_t nColumnFrom, nColumnTo;
	int	nCF(0), nCT(0);
	int	nCaretPosYOLD;
	bool bBeginBoxSelectOld;
	size_t nLineLen;
	std::vector<SortData*> sta;

	auto& selInfo = view.GetSelectionInfo();
	if (!selInfo.IsTextSelected()) {			// テキストが選択されているか
		return;
	}
	auto& layoutMgr = GetDocument().layoutMgr;
	if (selInfo.IsBoxSelecting()) {
		rangeA = selInfo.select;
		if (selInfo.select.GetFrom().x == selInfo.select.GetTo().x) {
			selInfo.select.SetToX((int)layoutMgr.GetMaxLineKetas());
		}
		if (selInfo.select.GetFrom().x<selInfo.select.GetTo().x) {
			nCF = selInfo.select.GetFrom().x;
			nCT = selInfo.select.GetTo().x;
		}else {
			nCF = selInfo.select.GetTo().x;
			nCT = selInfo.select.GetFrom().x;
		}
	}
	bBeginBoxSelectOld = selInfo.IsBoxSelecting();
	auto& caret = GetCaret();
	nCaretPosYOLD = caret.GetCaretLayoutPos().GetY();
	layoutMgr.LayoutToLogic(
		selInfo.select,
		&selectOld
	);

	if (bBeginBoxSelectOld) {
		selectOld.GetTo().y++;
	}else {
		// カーソル位置が行頭じゃない ＆ 選択範囲の終端に改行コードがある場合は
		// その行も選択範囲に加える
		if (selectOld.GetTo().x > 0) {
			const DocLine* pDocLine = GetDocument().docLineMgr.GetLine(selectOld.GetTo().y);
			if (pDocLine && EolType::None != pDocLine->GetEol()) {
				selectOld.GetTo().y++;
			}
		}
	}
	selectOld.SetFromX(0);
	selectOld.SetToX(0);

	// 行選択されてない
	if (selectOld.IsLineOne()) {
		return;
	}
	
	sta.reserve(selectOld.GetTo().y - selectOld.GetFrom().y);
	for (int i=selectOld.GetFrom().y; i<selectOld.GetTo().y; ++i) {
		const DocLine* pDocLine = GetDocument().docLineMgr.GetLine(i);
		const NativeW& memLine = pDocLine->_GetDocLineDataWithEOL();
		const wchar_t* pLine = memLine.GetStringPtr(&nLineLen);
		size_t nLineLenWithoutEOL = pDocLine->GetLengthWithoutEOL();
		if (!pLine) {
			continue;
		}
		SortData* pst = new SortData;
		if (bBeginBoxSelectOld) {
			nColumnFrom = view.LineColumnToIndex(pDocLine, nCF);
			nColumnTo = view.LineColumnToIndex(pDocLine, nCT);
			if (nColumnTo < nLineLenWithoutEOL) {	// BOX選択範囲の右端が行内に収まっている場合
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
		size_t nlen = sta[sta.size() - 1]->pMemLine->GetStringLength();
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
	repData.resize(sta.size());
	int opeSeq = GetDocument().docEditor.opeBuf.GetNextSeq();
	size_t sz = sta.size();
	for (size_t i=0; i<sz; ++i) {
		repData[i].nSeq = opeSeq;
		repData[i].memLine.SetString(sta[i]->pMemLine->GetStringPtr(), sta[i]->pMemLine->GetStringLength());
		if (pStrLast == sta[i]->pMemLine->GetStringPtr()) {
			// 元最終行に改行がないのでつける
			Eol cWork = GetDocument().docEditor.GetNewLineCode();
			repData[i].memLine.AppendString(cWork.GetValue2(), cWork.GetLen());
		}
	}
	if (pStrLast) {
		// 最終行の改行を削除
		LineData& lastData = repData[repData.size() - 1];
		size_t nLen = lastData.memLine.GetStringLength();
		bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
		while (0 <nLen && WCODE::IsLineDelimiter(lastData.memLine[nLen-1], bExtEol)) {
			--nLen;
		}
		lastData.memLine._SetStringLength(nLen);
	}
	// swapで削除
	{
		std::vector<SortData*> temp;
		temp.swap(sta);
	}

	Range selectOld_Layout;
	layoutMgr.LogicToLayout(selectOld, &selectOld_Layout);
	view.ReplaceData_CEditView3(
		selectOld_Layout,
		nullptr,
		&repData,
		false,
		view.bDoing_UndoRedo ? nullptr : GetOpeBlk(),
		opeSeq,
		nullptr
	);

	// 選択エリアの復元
	if (bBeginBoxSelectOld) {
		selInfo.SetBoxSelect(bBeginBoxSelectOld);
		selInfo.select = rangeA;
	}else {
		selInfo.select = selectOld_Layout;
	}
	if (nCaretPosYOLD == selInfo.select.GetFrom().y || selInfo.IsBoxSelecting()) {
		caret.MoveCursor(selInfo.select.GetFrom(), true);
	}else {
		caret.MoveCursor(selInfo.select.GetTo(), true);
	}
	caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX();
	if (!view.bDoing_UndoRedo) {	// Undo, Redoの実行中か
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				caret.GetCaretLogicPos()	// 操作前後のキャレット位置
			)
		);
	}
	view.RedrawAll();
}


// from ViewCommander_New.cpp
/*! @brief 物理行のマージ

	連続する物理行で内容が同一の物を1行にまとめます．
	
	矩形選択時はなにも実行しません．
	
	@note 改行コードを含むデータを比較しているので、
	ファイルの最終行はソート対象外にしています
*/
void ViewCommander::Command_Merge(void)
{
	int	nCaretPosYOLD;
	size_t	nLineLen;

	auto& selInfo = view.GetSelectionInfo();
	if (!selInfo.IsTextSelected()) {			// テキストが選択されているか
		return;
	}
	if (selInfo.IsBoxSelecting()) {
		return;
	}
	auto& layoutMgr = GetDocument().layoutMgr;
	auto& caret = GetCaret();
	nCaretPosYOLD = caret.GetCaretLayoutPos().GetY();
	Range sSelectOld; // 範囲選択
	layoutMgr.LayoutToLogic(
		selInfo.select,
		&sSelectOld
	);

	// カーソル位置が行頭じゃない ＆ 選択範囲の終端に改行コードがある場合は
	// その行も選択範囲に加える
	if (sSelectOld.GetTo().x > 0) {
#if 0
		const Layout* pLayout = layoutMgr.SearchLineByLayoutY(selInfo.select.GetTo().y);
		if (pLayout && EolType::None != pLayout->GetLayoutEol()) {
			selectOld.GetToPointer()->y++;
			//selectOld.GetTo().y++;
		}
#else
		// ソートと仕様を合わせる
		const DocLine* pDocLine = GetDocument().docLineMgr.GetLine(sSelectOld.GetTo().y);
		if (pDocLine && EolType::None != pDocLine->GetEol()) {
			sSelectOld.GetTo().y++;
		}
#endif
	}

	sSelectOld.SetFromX(0);
	sSelectOld.SetToX(0);

	// 行選択されてない
	if (sSelectOld.IsLineOne()) {
		return;
	}

	size_t j = GetDocument().docLineMgr.GetLineCount();
	size_t nMergeLayoutLines = layoutMgr.GetLineCount();

	Range selectOld_Layout;
	layoutMgr.LogicToLayout(sSelectOld, &selectOld_Layout);

	// NUL対応修正
	std::vector<StringRef> lineArr;
	const wchar_t* pLinew = NULL;
	size_t nLineLenw = 0;
	bool bMerge = false;
	lineArr.reserve(sSelectOld.GetTo().y - sSelectOld.GetFrom().y);
	for (int i=sSelectOld.GetFrom().y; i<sSelectOld.GetTo().y; ++i) {
		const wchar_t* pLine = GetDocument().docLineMgr.GetLine(i)->GetDocLineStrWithEOL(&nLineLen);
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
		size_t nSize = lineArr.size();
		repData.resize(nSize);
		int opeSeq = GetDocument().docEditor.opeBuf.GetNextSeq();
		for (size_t idx=0; idx<nSize; ++idx) {
			repData[idx].nSeq = opeSeq;
			repData[idx].memLine.SetString(lineArr[idx].GetPtr(), lineArr[idx].GetLength());
		}
		view.ReplaceData_CEditView3(
			selectOld_Layout,
			nullptr,
			&repData,
			false,
			view.bDoing_UndoRedo ? nullptr : GetOpeBlk(),
			opeSeq,
			nullptr
		);
	}else {
		// 未変更なら変更しない
	}

	ASSERT_GE(j, GetDocument().docLineMgr.GetLineCount());
	ASSERT_GE(nMergeLayoutLines, layoutMgr.GetLineCount());
	j -= GetDocument().docLineMgr.GetLineCount();
	nMergeLayoutLines -= layoutMgr.GetLineCount();

	// 選択エリアの復元
	selInfo.select = selectOld_Layout;
	selInfo.select.GetTo().y -= (int)nMergeLayoutLines;

	if (nCaretPosYOLD == selInfo.select.GetFrom().y) {
		caret.MoveCursor(selInfo.select.GetFrom(), true);
	}else {
		caret.MoveCursor(selInfo.select.GetTo(), true);
	}
	caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX();
	if (!view.bDoing_UndoRedo) {	// Undo, Redoの実行中か
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				caret.GetCaretLogicPos()	// 操作前後のキャレット位置
			)
		);
	}
	view.RedrawAll();

	if (j) {
		TopOkMessage(view.GetHwnd(), LS(STR_ERR_DLGEDITVWCMDNW7), j);
	}else {
		InfoMessage(view.GetHwnd(), LS(STR_ERR_DLGEDITVWCMDNW8));
	}
}


// from ViewCommander_New.cpp
/* メニューからの再変換対応 */
void ViewCommander::Command_Reconvert(void)
{
	const int ATRECONVERTSTRING_SET = 1;

	// サイズを取得
	LRESULT nSize = view.SetReconvertStruct(NULL, UNICODE_BOOL);
	if (nSize == 0)  // サイズ０の時は何もしない
		return ;

	bool bUseUnicodeATOK = false;
	// バージョンチェック
	if (!OsSupportReconvert()) {
		
		// MSIMEかどうか
		HWND hWnd = ImmGetDefaultIMEWnd(view.GetHwnd());
		if (SendMessage(hWnd, view.uWM_MSIME_RECONVERTREQUEST, FID_RECONVERT_VERSION, 0)) {
			SendMessage(hWnd, view.uWM_MSIME_RECONVERTREQUEST, 0, (LPARAM)view.GetHwnd());
			return ;
		}

		// ATOKが使えるかどうか
		TCHAR sz[256];
		ImmGetDescription(GetKeyboardLayout(0), sz, _countof(sz)); // 説明の取得
		if ((_tcsncmp(sz, _T("ATOK"),4) == 0) && view.AT_ImmSetReconvertString) {
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
		nSize = view.SetReconvertStruct(NULL, UNICODE_BOOL || bUseUnicodeATOK);
		if (nSize == 0) { // サイズ０の時は何もしない
			return;
		}
	}

	// IMEのコンテキスト取得
	HIMC hIMC = ::ImmGetContext(view.GetHwnd());
	
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
	view.SetReconvertStruct(pReconv, UNICODE_BOOL || bUseUnicodeATOK);
	
	// 変換範囲の調整
	if (bUseUnicodeATOK) {
		(*view.AT_ImmSetReconvertString)(hIMC, ATRECONVERTSTRING_SET, pReconv, pReconv->dwSize);
	}else {
		::ImmSetCompositionString(hIMC, SCS_QUERYRECONVERTSTRING, pReconv, pReconv->dwSize, NULL, 0);
	}

	// 調整した変換範囲を選択する
	view.SetSelectionFromReonvert(pReconv, UNICODE_BOOL || bUseUnicodeATOK);
	
	// 再変換実行
	if (bUseUnicodeATOK) {
		(*view.AT_ImmSetReconvertString)(hIMC, ATRECONVERTSTRING_SET, pReconv, pReconv->dwSize);
	}else {
		::ImmSetCompositionString(hIMC, SCS_SETRECONVERTSTRING, pReconv, pReconv->dwSize, NULL, 0);
	}

	// 領域解放
	::HeapFree(GetProcessHeap(), 0, (LPVOID)pReconv);
	::ImmReleaseContext(view.GetHwnd(), hIMC);
}

