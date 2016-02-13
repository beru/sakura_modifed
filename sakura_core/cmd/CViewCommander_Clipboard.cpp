/*!	@file
@brief ViewCommanderクラスのコマンド(クリップボード系)関数群

	2012/12/20	CViewCommander.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, novice
	Copyright (C) 2002, hor, genta, Azumaiya, すなふき
	Copyright (C) 2004, Moca
	Copyright (C) 2005, genta
	Copyright (C) 2007, ryoji
	Copyright (C) 2010, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"
#include "view/colors/CColorStrategy.h"
#include "view/colors/CColor_Found.h"
#include "uiparts/CWaitCursor.h"
#include "util/os.h"


/** 切り取り(選択範囲をクリップボードにコピーして削除)

	@date 2007.11.18 ryoji 「選択なしでコピーを可能にする」オプション処理追加
*/
void ViewCommander::Command_CUT(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	// 範囲選択がされていない
	if (!selInfo.IsTextSelected()) {
		// 非選択時は、カーソル行を切り取り
		if (!GetDllShareData().m_common.m_edit.m_bEnableNoSelectCopy) {	// 2007.11.18 ryoji
			return;	// 何もしない（音も鳴らさない）
		}
		// 行切り取り(折り返し単位)
		Command_CUT_LINE();
		return;
	}
	bool bBeginBoxSelect = selInfo.IsBoxSelecting();

	// 選択範囲のデータを取得
	// 正常時はTRUE,範囲未選択の場合はFALSEを返す
	NativeW cmemBuf;
	if (!m_pCommanderView->GetSelectedData(&cmemBuf, false, NULL, false, GetDllShareData().m_common.m_edit.m_bAddCRLFWhenCopy)) {
		ErrorBeep();
		return;
	}
	// クリップボードにデータを設定
	if (!m_pCommanderView->MySetClipboardData(cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), bBeginBoxSelect)) {
		ErrorBeep();
		return;
	}
	cmemBuf.Clear();

	// カーソル位置または選択エリアを削除
	m_pCommanderView->DeleteData(true);
	return;
}


/**	選択範囲をクリップボードにコピー

	@date 2007.11.18 ryoji 「選択なしでコピーを可能にする」オプション処理追加
*/
void ViewCommander::Command_COPY(
	bool		bIgnoreLockAndDisable,	// [in] 選択範囲を解除するか？
	bool		bAddCRLFWhenCopy,		// [in] 折り返し位置に改行コードを挿入するか？
	EolType	neweol					// [in] コピーするときのEOL。
	)
{
	NativeW cmemBuf;
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	auto& csEdit = GetDllShareData().m_common.m_edit;
	// クリップボードに入れるべきテキストデータを、cmemBufに格納する
	if (!selInfo.IsTextSelected()) {
		// 非選択時は、カーソル行をコピーする
		if (!csEdit.m_bEnableNoSelectCopy) {	// 2007.11.18 ryoji
			return;	// 何もしない（音も鳴らさない）
		}
		m_pCommanderView->CopyCurLine(
			bAddCRLFWhenCopy,
			neweol,
			csEdit.m_bEnableLineModePaste
		);
	}else {
		// テキストが選択されているときは、選択範囲のデータを取得
		bool bBeginBoxSelect = selInfo.IsBoxSelecting();
		// 選択範囲のデータを取得
		// 正常時はTRUE,範囲未選択の場合はFALSEを返す
		if (!m_pCommanderView->GetSelectedData(&cmemBuf, false, NULL, false, bAddCRLFWhenCopy, neweol)) {
			ErrorBeep();
			return;
		}

		// クリップボードにデータcmemBufの内容を設定
		if (!m_pCommanderView->MySetClipboardData(
			cmemBuf.GetStringPtr(),
			cmemBuf.GetStringLength(),
			bBeginBoxSelect,
			false
			)
		) {
			ErrorBeep();
			return;
		}
	}
	cmemBuf.Clear();

	// 選択範囲の後片付け
	if (!bIgnoreLockAndDisable) {
		// 選択状態のロック
		if (selInfo.m_bSelectingLock) {
			selInfo.m_bSelectingLock = false;
			selInfo.PrintSelectionInfoMsg();
			if (!selInfo.IsTextSelected()) {
				GetCaret().m_cUnderLine.CaretUnderLineON(true, false);
			}
		}
	}
	if (csEdit.m_bCopyAndDisablSelection) {	// コピーしたら選択解除
		// テキストが選択されているか
		if (selInfo.IsTextSelected()) {
			// 現在の選択範囲を非選択状態に戻す
			selInfo.DisableSelectArea(true);
		}
	}
	return;
}


/** 貼り付け(クリップボードから貼り付け)
	@param [in] option 貼り付け時のオプション
	@li 0x01 改行コード変換有効
	@li 0x02 改行コード変換無効
	@li 0x04 ラインモード貼り付け有効
	@li 0x08 ラインモード貼り付け無効
	@li 0x10 矩形コピーは常に矩形貼り付け
	@li 0x20 矩形コピーは常に通常貼り付け

	@date 2007.10.04 ryoji MSDEVLineSelect形式の行コピー対応処理を追加（VS2003/2005のエディタと類似の挙動に）
*/
void ViewCommander::Command_PASTE(int option)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();

	if (selInfo.IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	auto& commonSetting = GetDllShareData().m_common;
	// クリップボードからデータを取得 -> memClip, bColumnSelect
	NativeW	cmemClip;
	bool		bColumnSelect;
	bool		bLineSelect = false;
	bool		bLineSelectOption = 
		((option & 0x04) == 0x04) ? true :
		((option & 0x08) == 0x08) ? false :
		commonSetting.m_edit.m_bEnableLineModePaste;

	if (!m_pCommanderView->MyGetClipboardData(cmemClip, &bColumnSelect, bLineSelectOption ? &bLineSelect: NULL)) {
		ErrorBeep();
		return;
	}

	// クリップボードデータ取得 -> pszText, nTextLen
	LogicInt nTextLen;
	const wchar_t*	pszText = cmemClip.GetStringPtr(&nTextLen);

	bool bConvertEol = 
		((option & 0x01) == 0x01) ? true :
		((option & 0x02) == 0x02) ? false :
		commonSetting.m_edit.m_bConvertEOLPaste;

	bool bAutoColumnPaste = 
		((option & 0x10) == 0x10) ? true :
		((option & 0x20) == 0x20) ? false :
		commonSetting.m_edit.m_bAutoColumnPaste != FALSE;
	
	// 矩形コピーのテキストは常に矩形貼り付け
	if (bAutoColumnPaste) {
		// 矩形コピーのデータなら矩形貼り付け
		if (bColumnSelect) {
			if (selInfo.IsMouseSelecting()) {
				ErrorBeep();
				return;
			}
			if (!commonSetting.m_view.m_bFontIs_FIXED_PITCH) {
				return;
			}
			Command_PASTEBOX(pszText, nTextLen);
			m_pCommanderView->AdjustScrollBars();
			m_pCommanderView->Redraw();
			return;
		}
	}

	// 2007.10.04 ryoji
	// 行コピー（MSDEVLineSelect形式）のテキストで末尾が改行になっていなければ改行を追加する
	// ※レイアウト折り返しの行コピーだった場合は末尾が改行になっていない
	if (bLineSelect) {
		if (!WCODE::IsLineDelimiter(pszText[nTextLen-1], GetDllShareData().m_common.m_edit.m_bEnableExtEol)) {
			cmemClip.AppendString(GetDocument()->m_docEditor.GetNewLineCode().GetValue2());
			pszText = cmemClip.GetStringPtr(&nTextLen);
		}
	}

	if (bConvertEol) {
		LogicInt nConvertedTextLen = ConvertEol(pszText, nTextLen, NULL);
		std::vector<wchar_t> szConvertedText(nConvertedTextLen);
		wchar_t* pszConvertedText = &szConvertedText[0];
		ConvertEol(pszText, nTextLen, pszConvertedText);
		// テキストを貼り付け
		Command_INSTEXT(true, pszConvertedText, nConvertedTextLen, true, bLineSelect);	// 2010.09.17 ryoji
	}else {
		// テキストを貼り付け
		Command_INSTEXT(true, pszText, nTextLen, true, bLineSelect);	// 2010.09.17 ryoji
	}

	return;
}



//<< 2002/03/28 Azumaiya
// メモリデータを矩形貼り付け用のデータと解釈して処理する。
//  なお、この関数は Command_PASTEBOX(void) と、
// 2769 : GetDocument()->m_docEditor.SetModified(true, true);	// Jan. 22, 2002 genta
// から、
// 3057 : m_pCommanderView->SetDrawSwitch(true);	// 2002.01.25 hor
// 間まで、一緒です。
//  ですが、コメントを削ったり、#if 0 のところを削ったりしていますので、Command_PASTEBOX(void) は
// 残すようにしました(下にこの関数を使った使ったバージョンをコメントで書いておきました)。
//  なお、以下にあげるように Command_PASTEBOX(void) と違うところがあるので注意してください。
// > 呼び出し側が責任を持って、
// ・マウスによる範囲選択中である。
// ・現在のフォントは固定幅フォントである。
// の 2 点をチェックする。
// > 再描画を行わない
// です。
//  なお、これらを呼び出し側に期待するわけは、「すべて置換」のような何回も連続で呼び出す
// ときに、最初に一回チェックすればよいものを何回もチェックするのは無駄と判断したためです。
// @note 2004.06.30 現在、すべて置換では使用していない
void ViewCommander::Command_PASTEBOX(
	const wchar_t* szPaste,
	int nPasteSize
	)
{
	/* これらの動作は残しておきたいのだが、呼び出し側で責任を持ってやってもらうことに変更。
	if (m_pCommanderView->GetSelectionInfo().IsMouseSelecting())	// マウスによる範囲選択中
	{
		ErrorBeep();
		return;
	}
	if (!GetDllShareData().m_common.m_bFontIs_FIXED_PITCH)	// 現在のフォントは固定幅フォントである
	{
		return;
	}
	*/

	GetDocument()->m_docEditor.SetModified(true, true);	// Jan. 22, 2002 genta

	bool bDrawSwitchOld = m_pCommanderView->SetDrawSwitch(false);	// 2002.01.25 hor

	// とりあえず選択範囲を削除
	// 2004.06.30 Moca m_pCommanderView->GetSelectionInfo().IsTextSelected()がないと未選択時、一文字消えてしまう
	if (m_pCommanderView->GetSelectionInfo().IsTextSelected()) {
		m_pCommanderView->DeleteData(false/*true 2002.01.25 hor*/);
	}

	WaitCursor waitCursor(m_pCommanderView->GetHwnd(), 10000 < nPasteSize);
	HWND hwndProgress = NULL;
	int nProgressPos = 0;
	if (waitCursor.IsEnable()) {
		hwndProgress = m_pCommanderView->StartProgress();
	}

	LayoutPoint ptCurOld = GetCaret().GetCaretLayoutPos();

	LayoutInt nCount = LayoutInt(0);
	bool bExtEol = GetDllShareData().m_common.m_edit.m_bEnableExtEol;

	// Jul. 10, 2005 genta 貼り付けデータの最後にCR/LFが無い場合の対策
	// データの最後まで処理 i.e. nBgnがnPasteSizeを超えたら終了
	//for (nPos = 0; nPos < nPasteSize;)
	int				nPos;
	LayoutPoint	ptLayoutNew;	// 挿入された部分の次の位置
	for (int nBgn=nPos=0; nBgn<nPasteSize;) {
		// Jul. 10, 2005 genta 貼り付けデータの最後にCR/LFが無いと
		// 最終行のPaste処理が動かないので，
		// データの末尾に来た場合は強制的に処理するようにする
		if (WCODE::IsLineDelimiter(szPaste[nPos], bExtEol) || nPos == nPasteSize) {
			// 現在位置にデータを挿入
			if (nPos - nBgn > 0) {
				m_pCommanderView->InsertData_CEditView(
					ptCurOld + LayoutPoint(LayoutInt(0), nCount),
					&szPaste[nBgn],
					nPos - nBgn,
					&ptLayoutNew,
					false
				);
			}

			// この行の挿入位置へカーソルを移動
			GetCaret().MoveCursor(ptCurOld + LayoutPoint(LayoutInt(0), nCount), false);
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			// カーソル行が最後の行かつ行末に改行が無く、挿入すべきデータがまだある場合
			bool bAddLastCR = false;
			const Layout*	pLayout;
			LogicInt		nLineLen = LogicInt(0);
			const wchar_t* pLine = GetDocument()->m_layoutMgr.GetLineStr(GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pLayout);

			if (pLine && 1 <= nLineLen) {
				if (WCODE::IsLineDelimiter(pLine[nLineLen - 1], bExtEol)) {
				}else {
					bAddLastCR = true;
				}
			}else { // 2001/10/02 novice
				bAddLastCR = true;
			}

			if (bAddLastCR) {
//				MYTRACE(_T(" カーソル行が最後の行かつ行末に改行が無く、\n挿入すべきデータがまだある場合は行末に改行を挿入。\n"));
				LayoutInt nInsPosX = m_pCommanderView->LineIndexToColumn(pLayout, nLineLen);

				m_pCommanderView->InsertData_CEditView(
					LayoutPoint(nInsPosX, GetCaret().GetCaretLayoutPos().GetY2()),
					GetDocument()->m_docEditor.GetNewLineCode().GetValue2(),
					GetDocument()->m_docEditor.GetNewLineCode().GetLen(),
					&ptLayoutNew,
					false
				);
			}

			if (
				(nPos + 1 < nPasteSize) &&
				(szPaste[nPos] == L'\r' && szPaste[nPos + 1] == L'\n')
			) {
				nBgn = nPos + 2;
			}else {
				nBgn = nPos + 1;
			}

			nPos = nBgn;
			++nCount;
		}else {
			++nPos;
		}
		if ((nPos % 100) == 0 && hwndProgress) {
			int newPos = ::MulDiv(nPos, 100, nPasteSize);
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

	// 挿入データの先頭位置へカーソルを移動
	GetCaret().MoveCursor(ptCurOld, true);
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	if (!m_pCommanderView->m_bDoing_UndoRedo) {	// アンドゥ・リドゥの実行中か
		// 操作の追加
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
			)
		);
	}

	m_pCommanderView->SetDrawSwitch(bDrawSwitchOld);	// 2002.01.25 hor
	return;
}


/** 矩形貼り付け(クリップボードから矩形貼り付け)
	@param [in] option 未使用

	@date 2004.06.29 Moca 未使用だったものを有効にする
	オリジナルのCommand_PASTEBOX(void)はばっさり削除 (genta)
*/
void ViewCommander::Command_PASTEBOX(int option)
{
	if (m_pCommanderView->GetSelectionInfo().IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	if (!GetDllShareData().m_common.m_view.m_bFontIs_FIXED_PITCH) {	// 現在のフォントは固定幅フォントである
		return;
	}

	// クリップボードからデータを取得
	NativeW cmemClip;
	if (!m_pCommanderView->MyGetClipboardData(cmemClip, NULL)) {
		ErrorBeep();
		return;
	}
	// 2004.07.13 Moca \0コピー対策
	int nstrlen;
	const wchar_t* lptstr = cmemClip.GetStringPtr(&nstrlen);

	Command_PASTEBOX(lptstr, nstrlen);
	m_pCommanderView->AdjustScrollBars(); // 2007.07.22 ryoji
	m_pCommanderView->Redraw();			// 2002.01.25 hor
}


// 矩形文字列挿入
void ViewCommander::Command_INSBOXTEXT(
	const wchar_t* pszPaste,
	int nPasteSize
	)
{
	if (m_pCommanderView->GetSelectionInfo().IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	if (!GetDllShareData().m_common.m_view.m_bFontIs_FIXED_PITCH) {	// 現在のフォントは固定幅フォントである
		return;
	}

	Command_PASTEBOX(pszPaste, nPasteSize);
	m_pCommanderView->AdjustScrollBars(); // 2007.07.22 ryoji
	m_pCommanderView->Redraw();			// 2002.01.25 hor
}


/*! テキストを貼り付け
	@date 2004.05.14 Moca '\\0'を受け入れるように、引数に長さを追加
	@date 2010.09.17 ryoji ラインモード貼り付けオプションを追加して以前の Command_PASTE() との重複部を整理・統合
	@date 2013.05.10 Moca 高速モード
*/
void ViewCommander::Command_INSTEXT(
	bool			bRedraw,		// 
	const wchar_t*	pszText,		// [in] 貼り付ける文字列。
	LogicInt		nTextLen,		// [in] pszTextの長さ。-1を指定すると、pszTextをNUL終端文字列とみなして長さを自動計算する
	bool			bNoWaitCursor,	// 
	bool			bLinePaste,		// [in] ラインモード貼り付け
	bool			bFastMode,		// [in] 高速モード(レイアウト座標は無視する)
	const LogicRange*	pSelectLogic	// [in] オプション。高速モードのときの削除範囲ロジック単位
	)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	if (nTextLen < 0) {
		nTextLen = LogicInt(wcslen(pszText));
	}

	WaitCursor waitCursor(
		m_pCommanderView->GetHwnd(),
		10000 < nTextLen && !selInfo.IsBoxSelecting()
	);

	GetDocument()->m_docEditor.SetModified(true, bRedraw);	// Jan. 22, 2002 genta

	// テキストが選択されているか
	if (selInfo.IsTextSelected() || bFastMode) {
		// 矩形範囲選択中か
		if (selInfo.IsBoxSelecting()) {
			// 改行までを抜き出す
			LogicInt i;
			bool bExtEol = GetDllShareData().m_common.m_edit.m_bEnableExtEol;
			for (i=LogicInt(0); i<nTextLen; ++i) {
				if (WCODE::IsLineDelimiter(pszText[i], bExtEol)) {
					break;
				}
			}
			Command_INDENT(pszText, i);
			goto end_of_func;
		}else {
			// Jun. 23, 2000 genta
			// 同一行の行末以降のみが選択されている場合には選択無しと見なす
			bool bAfterEOLSelect = false;
			if (!bFastMode) {
				LogicInt len;
				const Layout* pLayout;
				const wchar_t* line = GetDocument()->m_layoutMgr.GetLineStr(GetSelect().GetFrom().GetY2(), &len, &pLayout);
				int pos = (!line) ? 0 : m_pCommanderView->LineColumnToIndex(pLayout, GetSelect().GetFrom().GetX2());

				// 開始位置が行末より後ろで、終了位置が同一行
				if (pos >= len && GetSelect().IsLineOne()) {
					GetCaret().SetCaretLayoutPos(LayoutPoint(GetSelect().GetFrom().x, GetCaret().GetCaretLayoutPos().y)); // キャレットX変更
					selInfo.DisableSelectArea(false);
					bAfterEOLSelect = true;
				}
			}
			if (!bAfterEOLSelect) {
				// データ置換 削除&挿入にも使える
				// 行コピーの貼り付けでは選択範囲は削除（後で行頭に貼り付ける）	// 2007.10.04 ryoji
				m_pCommanderView->ReplaceData_CEditView(
					GetSelect(),				// 選択範囲
					bLinePaste? L"": pszText,	// 挿入するデータ
					bLinePaste? LogicInt(0): nTextLen,	// 挿入するデータの長さ
					bRedraw,
					m_pCommanderView->m_bDoing_UndoRedo ? NULL : GetOpeBlk(),
					bFastMode,
					pSelectLogic
				);
				if (!bLinePaste)	// 2007.10.04 ryoji
					goto end_of_func;
			}
		}
	}

	{	// 非選択時の処理 or ラインモード貼り付け時の残りの処理
		LogicInt	nPosX_PHY_Delta(0);
		if (bLinePaste) {	// 2007.10.04 ryoji
			// 挿入ポイント（折り返し単位行頭）にカーソルを移動
			LogicPoint ptCaretBefore = GetCaret().GetCaretLogicPos();	// 操作前のキャレット位置
			Command_GOLINETOP(false, 1);								// 行頭に移動(折り返し単位)
			LogicPoint ptCaretAfter = GetCaret().GetCaretLogicPos();	// 操作後のキャレット位置

			// 挿入ポイントと元の位置との差分文字数
			nPosX_PHY_Delta = ptCaretBefore.x - ptCaretAfter.x;

			// UNDO用記録
			if (!m_pCommanderView->m_bDoing_UndoRedo) {
				GetOpeBlk()->AppendOpe(
					new MoveCaretOpe(
						ptCaretBefore,	// 操作前のキャレット位置
						ptCaretAfter	// 操作後のキャレット位置
					)
				);
			}
		}

		// 現在位置にデータを挿入
		LayoutPoint ptLayoutNew; // 挿入された部分の次の位置
		m_pCommanderView->InsertData_CEditView(
			GetCaret().GetCaretLayoutPos(),
			pszText,
			nTextLen,
			&ptLayoutNew,
			bRedraw
		);

		// 挿入データの最後へカーソルを移動
		GetCaret().MoveCursor(ptLayoutNew, bRedraw);
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

		if (bLinePaste) {	// 2007.10.04 ryoji
			// 元の位置へカーソルを移動
			LogicPoint ptCaretBefore = GetCaret().GetCaretLogicPos();	// 操作前のキャレット位置
			LayoutPoint ptLayout;
			GetDocument()->m_layoutMgr.LogicToLayout(
				ptCaretBefore + LogicPoint(nPosX_PHY_Delta, LogicInt(0)),
				&ptLayout
			);
			GetCaret().MoveCursor(ptLayout, bRedraw);					// カーソル移動
			LogicPoint ptCaretAfter = GetCaret().GetCaretLogicPos();	// 操作後のキャレット位置
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;

			// UNDO用記録
			if (!m_pCommanderView->m_bDoing_UndoRedo) {
				GetOpeBlk()->AppendOpe(
					new MoveCaretOpe(
						ptCaretBefore,	// 操作前のキャレット位置Ｘ
						ptCaretAfter	// 操作後のキャレット位置Ｘ
					)
				);
			}
		}
	}

end_of_func:

	return;
}


// 最後にテキストを追加
void ViewCommander::Command_ADDTAIL(
	const wchar_t*	pszData,	// 追加するテキスト
	int				nDataLen	// 追加するテキストの長さ。文字単位。-1を指定すると、テキスト終端まで。
	)
{
	// テキスト長自動計算
	if (nDataLen == -1 && pszData) {
		nDataLen = wcslen(pszData);
	}

	GetDocument()->m_docEditor.SetModified(true, true);	// Jan. 22, 2002 genta

	// ファイルの最後に移動
	Command_GOFILEEND(false);

	// 現在位置にデータを挿入
	LayoutPoint ptLayoutNew;	// 挿入された部分の次の位置
	m_pCommanderView->InsertData_CEditView(
		GetCaret().GetCaretLayoutPos(),
		pszData,
		nDataLen,
		&ptLayoutNew,
		true
	);

	// 挿入データの最後へカーソルを移動
	// Sep. 2, 2002 すなふき アンダーラインの表示が残ってしまう問題を修正
	GetCaret().MoveCursor(ptLayoutNew, true);
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
}


// 選択範囲内全行コピー
void ViewCommander::Command_COPYLINES(void)
{
	// 選択範囲内の全行をクリップボードにコピーする
	m_pCommanderView->CopySelectedAllLines(
		NULL,	// 引用符
		false	// 行番号を付与する
	);
	return;
}


// 選択範囲内全行引用符付きコピー
void ViewCommander::Command_COPYLINESASPASSAGE(void)
{
	// 選択範囲内の全行をクリップボードにコピーする
	m_pCommanderView->CopySelectedAllLines(
		GetDllShareData().m_common.m_format.m_szInyouKigou,	// 引用符
		false 									// 行番号を付与する
	);
	return;
}


// 選択範囲内全行行番号付きコピー
void ViewCommander::Command_COPYLINESWITHLINENUMBER(void)
{
	// 選択範囲内の全行をクリップボードにコピーする
	m_pCommanderView->CopySelectedAllLines(
		NULL,	// 引用符
		true	// 行番号を付与する
	);
	return;
}


static bool AppendHTMLColor(
	const ColorAttr& colorAttrLast, ColorAttr& colorAttrLast2,
	const FontAttr& fontAttrLast, FontAttr& fontAttrLast2,
	const WCHAR* pAppendStr, int nLen,
	NativeW& memClip
	)
{
	if (fontAttrLast.m_bBoldFont != fontAttrLast2.m_bBoldFont
		|| fontAttrLast.m_bUnderLine != fontAttrLast2.m_bUnderLine
		|| colorAttrLast.m_cTEXT != colorAttrLast2.m_cTEXT
		|| colorAttrLast.m_cBACK != colorAttrLast2.m_cBACK
	) {
		if (fontAttrLast2.m_bBoldFont) {
			memClip.AppendString(L"</b>", 4);
		}
		if (fontAttrLast2.m_bUnderLine) {
			if (colorAttrLast.m_cTEXT != colorAttrLast2.m_cTEXT
				|| colorAttrLast.m_cBACK != colorAttrLast2.m_cBACK
				|| fontAttrLast.m_bUnderLine != fontAttrLast2.m_bUnderLine
			) {
				memClip.AppendString(L"</u>", 4);
			}
		}
		if (colorAttrLast.m_cTEXT != colorAttrLast2.m_cTEXT
			|| colorAttrLast.m_cBACK != colorAttrLast2.m_cBACK
		) {
			if (colorAttrLast2.m_cTEXT != (COLORREF)-1) {
				memClip.AppendString(L"</span>", 7);
			}
			if (colorAttrLast.m_cTEXT != (COLORREF)-1) {
				if (colorAttrLast.m_cTEXT != colorAttrLast2.m_cTEXT
					|| colorAttrLast.m_cBACK != colorAttrLast2.m_cBACK
				) {
					WCHAR szColor[60];
					DWORD dwTEXTColor = (GetRValue(colorAttrLast.m_cTEXT) << 16) + (GetGValue(colorAttrLast.m_cTEXT) << 8) + GetBValue(colorAttrLast.m_cTEXT);
					DWORD dwBACKColor = (GetRValue(colorAttrLast.m_cBACK) << 16) + (GetGValue(colorAttrLast.m_cBACK) << 8) + GetBValue(colorAttrLast.m_cBACK);
					swprintf(szColor, L"<span style=\"color:#%06x;background-color:#%06x\">", dwTEXTColor, dwBACKColor);
					memClip.AppendString(szColor);
				}
			}
		}
		if (fontAttrLast.m_bUnderLine) {
			if (colorAttrLast.m_cTEXT != colorAttrLast2.m_cTEXT
				|| colorAttrLast.m_cBACK != colorAttrLast2.m_cBACK
				|| fontAttrLast.m_bUnderLine != fontAttrLast2.m_bUnderLine
			) {
				memClip.AppendString(L"<u>", 3);
			}
		}
		if (fontAttrLast.m_bBoldFont) {
			memClip.AppendString(L"<b>", 3);
		}
		colorAttrLast2 = colorAttrLast;
		fontAttrLast2  = fontAttrLast;
	}
	NativeW cmemBuf(pAppendStr, nLen);
	cmemBuf.Replace(L"&", L"&amp;");
	cmemBuf.Replace(L"<", L"&lt;");
	cmemBuf.Replace(L">", L"&gt;");
	memClip.AppendNativeData(cmemBuf);
	if (0 < nLen) {
		return WCODE::IsLineDelimiter(
			pAppendStr[nLen-1],
			GetDllShareData().m_common.m_edit.m_bEnableExtEol
		);
	}
	return false;
}


// 選択範囲内色付きHTMLコピー
void ViewCommander::Command_COPY_COLOR_HTML(bool bLineNumber)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	if (!selInfo.IsTextSelected()
	  || GetSelect().GetFrom() == GetSelect().GetTo()
	) {
		return;
	}
	const TypeConfig& type = GetDocument()->m_docType.GetDocumentAttribute();
	bool bLineNumLayout = GetDllShareData().m_common.m_edit.m_bAddCRLFWhenCopy
		|| selInfo.IsBoxSelecting();
	LayoutRect rcSel;
	TwoPointToRect(
		&rcSel,
		GetSelect().GetFrom(),	// 範囲選択開始
		GetSelect().GetTo()		// 範囲選択終了
	);
	// 修飾分を除いたバッファの長さをだいたいで計算
	LogicRange sSelectLogic;
	sSelectLogic.Clear(-1);
	int nBuffSize = 0;
	const Layout* pLayoutTop = NULL;
	{
		const Layout* pLayout;
		{
			LogicInt nLineLenTmp;
			GetDocument()->m_layoutMgr.GetLineStr(rcSel.top, &nLineLenTmp, &pLayout);
		}
		pLayoutTop = pLayout;
		LayoutInt i = rcSel.top;
		for (; pLayout && i <= rcSel.bottom; ++i, pLayout = pLayout->GetNextLayout()) {
			// 指定された桁に対応する行のデータ内の位置を調べる
			LogicInt nIdxFrom;
			LogicInt nIdxTo;
			if (selInfo.IsBoxSelecting()) {
				nIdxFrom = m_pCommanderView->LineColumnToIndex(pLayout, rcSel.left);
				nIdxTo   = m_pCommanderView->LineColumnToIndex(pLayout, rcSel.right);
				// 改行は除く
				if (nIdxTo - nIdxFrom > 0) {
					const WCHAR* pLine = pLayout->GetPtr();
					if (pLine[nIdxTo - 1] == L'\n' || pLine[nIdxTo - 1] == L'\r') {
						--nIdxTo;
					}
				}
				if (i == rcSel.top) {
					sSelectLogic.SetFromY(pLayout->GetLogicLineNo());
					sSelectLogic.SetFromX(nIdxFrom);
				}
				if (i == rcSel.bottom) {
					sSelectLogic.SetToY(pLayout->GetLogicLineNo());
					sSelectLogic.SetToX(nIdxTo);
				}
			}else {
				if (i == rcSel.top) {
					nIdxFrom = m_pCommanderView->LineColumnToIndex(pLayout, rcSel.left);
					sSelectLogic.SetFromY(pLayout->GetLogicLineNo());
					sSelectLogic.SetFromX(nIdxFrom);
				}else {
					nIdxFrom = LogicInt(0);
				}
				if (i == rcSel.bottom) {
					nIdxTo = m_pCommanderView->LineColumnToIndex(pLayout, rcSel.right);
					sSelectLogic.SetToY(pLayout->GetLogicLineNo());
					sSelectLogic.SetToX(nIdxTo);
				}else {
					nIdxTo = pLayout->GetLengthWithoutEOL();
				}
			}
			nBuffSize += nIdxTo - nIdxFrom;
			if (bLineNumLayout) {
				nBuffSize += 2;
			}else {
				nBuffSize += pLayout->GetLayoutEol().GetLen();
			}
		}
		if (sSelectLogic.GetTo().x == -1) {
			sSelectLogic.SetToY(GetDocument()->m_docLineMgr.GetLineCount());
			sSelectLogic.SetToX(LogicInt(0));
		}
	}
	// 行番号の幅を計算
	int nLineNumberMaxLen = 0;
	WCHAR szLineFormat[10];
	szLineFormat[0] = L'\0';
	NativeW cmemNullLine;
	if (bLineNumber) {
		int nLineNumberMax;
		if (type.m_bLineNumIsCRLF) {
			nLineNumberMax = sSelectLogic.GetTo().GetY();
		}else {
			nLineNumberMax = (Int)rcSel.bottom;
		}
		int nWork = 10;
		int i;
		cmemNullLine.AppendString(L" ");
		for (i=1; i<12; ++i) {
			if (nWork > nLineNumberMax) {
				break;
			}
			nWork *= 10;
			cmemNullLine.AppendString(L" ");
		}
		nLineNumberMaxLen = i + 1; // "%d:"
		cmemNullLine.AppendString(L":");
		swprintf(szLineFormat, L"%%%dd:", i);
	}
	if (bLineNumLayout) {
		nBuffSize += (Int)(nLineNumberMaxLen * (rcSel.bottom - rcSel.top + 1));
	}else {
		nBuffSize += (Int)(nLineNumberMaxLen * (sSelectLogic.GetTo().y - sSelectLogic.GetFrom().y + 1));
	}
	NativeW cmemClip;
	cmemClip.AllocStringBuffer(nBuffSize + 11);
	{
		COLORREF cBACK = type.m_colorInfoArr[COLORIDX_TEXT].m_colorAttr.m_cBACK;
		DWORD dwBACKColor = (GetRValue(cBACK) << 16) + (GetGValue(cBACK) << 8) + GetBValue(cBACK);
		WCHAR szBuf[50];
		swprintf(szBuf, L"<pre style=\"background-color:#%06x\">", dwBACKColor);
		cmemClip.AppendString(szBuf);
	}
	LayoutInt nLayoutLineNum = rcSel.top;
	const LogicInt nLineNumLast = sSelectLogic.GetTo().y;
	const DocLine* pDocLine = pLayoutTop->GetDocLineRef();
	const Layout* pLayout = pLayoutTop;
	while (pLayout && pLayout->GetLogicOffset()) {
		pLayout = pLayout->GetPrevLayout();
	}
	ColorAttr sColorAttr = { (COLORREF)-1, (COLORREF)-1 };
	ColorAttr sColorAttrNext = { (COLORREF)-1, (COLORREF)-1 };
	ColorAttr sColorAttrLast = { (COLORREF)-1, (COLORREF)-1 };
	ColorAttr sColorAttrLast2 = { (COLORREF)-1, (COLORREF)-1 };
	FontAttr sFontAttr = { false, false };
	FontAttr sFontAttrNext = { false, false };
	FontAttr sFontAttrLast = { false, false };
	FontAttr sFontAttrLast2 = { false, false };
	ColorStrategyPool* pool = ColorStrategyPool::getInstance();
	pool->SetCurrentView(m_pCommanderView);
	for (auto nLineNum = sSelectLogic.GetFrom().y;
		nLineNum <= nLineNumLast;
		++nLineNum, pDocLine = pDocLine->GetNextLine()
	) {
		if (!pDocLine) {
			break;
		}
		pool->NotifyOnStartScanLogic();
		ColorStrategy* pStrategyNormal = nullptr;
		ColorStrategy* pStrategyFound = nullptr;
		ColorStrategy* pStrategy = nullptr;
		StringRef cStringLine(pDocLine->GetPtr(), pDocLine->GetLengthWithEOL());
		{
			pStrategy = pStrategyNormal = pool->GetStrategyByColor(pLayout->GetColorTypePrev());
			if (pStrategy) {
				pStrategy->InitStrategyStatus();
				pStrategy->SetStrategyColorInfo(pLayout->GetColorInfo());
			}
			int nColorIdx = ToColorInfoArrIndex(pLayout->GetColorTypePrev());
			if (nColorIdx != -1) {
				const ColorInfo& info = type.m_colorInfoArr[nColorIdx];
				sFontAttr = info.m_fontAttr;
				sColorAttr = info.m_colorAttr;
			}
		}
		const WCHAR* pLine = pDocLine->GetPtr();
		for (;
			pLayout->GetLogicLineNo() == nLineNum;
			++nLayoutLineNum, pLayout = pLayout->GetNextLayout()
		) {
			LogicInt nIdxFrom;
			LogicInt nIdxTo;
			const int nLineLen = pLayout->GetLengthWithoutEOL() + pLayout->GetLayoutEol().GetLen();
			if (nLayoutLineNum < rcSel.top) {
				nIdxTo = nIdxFrom = LogicInt(-1);
			}else {
				if (selInfo.IsBoxSelecting()) {
					nIdxFrom = m_pCommanderView->LineColumnToIndex(pLayout, rcSel.left);
					nIdxTo   = m_pCommanderView->LineColumnToIndex(pLayout, rcSel.right);
					// 改行は除く
					if (nIdxTo - nIdxFrom > 0) {
						const WCHAR* pLine = pLayout->GetPtr();
						if (pLine[nIdxTo - 1] == L'\n' || pLine[nIdxTo - 1] == L'\r') {
							--nIdxTo;
						}
					}
				}else {
					if (nLayoutLineNum == rcSel.top) {
						nIdxFrom = sSelectLogic.GetFrom().x;
					}else {
						nIdxFrom = LogicInt(0);
					}
					if (nLayoutLineNum == rcSel.bottom) {
						nIdxTo = sSelectLogic.GetTo().x;
					}else {
						nIdxTo = nLineLen;
					}
				}
			}
			// 最後の改行の次の行番号を表示しないように
			if (nIdxTo == 0 && nLayoutLineNum == rcSel.bottom) {
				break;
			}
			if (bLineNumber) {
				WCHAR szLineNum[14];
				if (type.m_bLineNumIsCRLF) {
					if (pLayout->GetLogicOffset() != 0) {
						if (bLineNumLayout) {
							cmemClip.AppendNativeData(cmemNullLine);
						}
					}else {
						int ret = swprintf(szLineNum, szLineFormat, nLineNum + 1);
						cmemClip.AppendString(szLineNum, ret);
					}
				}else {
					if (bLineNumLayout || pLayout->GetLogicOffset() == 0) {
						int ret = swprintf(szLineNum, szLineFormat, nLayoutLineNum + 1);
						cmemClip.AppendString(szLineNum, ret);
					}
				}
			}
			const int nLineStart = pLayout->GetLogicOffset();
			int nBgnLogic = nIdxFrom + nLineStart;
			int iLogic = nLineStart;
			bool bAddCRLF = false;
			for (; iLogic<nLineStart+nLineLen; ++iLogic) {
				bool bChange = false;
				pStrategy = GetColorStrategyHTML(cStringLine, iLogic, pool, &pStrategyNormal, &pStrategyFound, bChange);
				if (bChange) {
					int nColorIdx = ToColorInfoArrIndex(pStrategy ? pStrategy->GetStrategyColor() : COLORIDX_TEXT);
					if (nColorIdx != -1) {
						const ColorInfo& info = type.m_colorInfoArr[nColorIdx];
						sColorAttrNext = info.m_colorAttr;
						sFontAttrNext  = info.m_fontAttr;
					}
				}
				if (nIdxFrom != -1 && nIdxFrom + nLineStart <= iLogic && iLogic <= nIdxTo + nLineStart) {
					if (nIdxFrom + nLineStart == iLogic) {
						sColorAttrLast = sColorAttrNext;
						sFontAttrLast  = sFontAttrNext;
					}else if (
						nIdxFrom + nLineStart < iLogic
						&& (
							sFontAttr.m_bBoldFont != sFontAttrNext.m_bBoldFont
						  	|| sFontAttr.m_bUnderLine != sFontAttrNext.m_bUnderLine
							|| sColorAttr.m_cTEXT != sColorAttrNext.m_cTEXT
						  	|| sColorAttr.m_cBACK != sColorAttrNext.m_cBACK
						)
					) {
						bAddCRLF = AppendHTMLColor(sColorAttrLast, sColorAttrLast2,
							sFontAttrLast, sFontAttrLast2, pLine + nBgnLogic, iLogic - nBgnLogic, cmemClip);
						sColorAttrLast = sColorAttrNext;
						sFontAttrLast  = sFontAttrNext;
						nBgnLogic = iLogic;
					}else if (nIdxTo + nLineStart == iLogic) {
						bAddCRLF = AppendHTMLColor(sColorAttrLast, sColorAttrLast2,
							sFontAttrLast, sFontAttrLast2, pLine + nBgnLogic, iLogic - nBgnLogic, cmemClip);
						nBgnLogic = iLogic;
					}
				}
				sColorAttr = sColorAttrNext;
				sFontAttr = sFontAttrNext;
			}
			if (nIdxFrom != -1 && nIdxTo + nLineStart == iLogic) {
				bAddCRLF = AppendHTMLColor(sColorAttrLast, sColorAttrLast2,
					sFontAttrLast, sFontAttrLast2, pLine + nBgnLogic, iLogic - nBgnLogic, cmemClip);
			}
			if (bLineNumber) {
				bool bAddLineNum = true;
				const Layout* pLayoutNext = pLayout->GetNextLayout();
				if (pLayoutNext) {
					if (type.m_bLineNumIsCRLF) {
						if (bLineNumLayout && pLayoutNext->GetLogicOffset() != 0) {
							bAddLineNum = true;
						}else {
							bAddLineNum = true;
						}
					}else {
						if (bLineNumLayout || pLayoutNext->GetLogicOffset() == 0) {
							bAddLineNum = true;
						}
					}
				}
				if (bAddLineNum) {
					if (sFontAttrLast2.m_bBoldFont) {
						cmemClip.AppendString(L"</b>", 4);
					}
					if (sFontAttrLast2.m_bUnderLine) {
						cmemClip.AppendString(L"</u>", 4);
					}
					if (sColorAttrLast2.m_cTEXT != (COLORREF)-1) {
						cmemClip.AppendString(L"</span>", 7);
					}
					sFontAttrLast.m_bBoldFont = sFontAttrLast2.m_bBoldFont = false;
					sFontAttrLast.m_bUnderLine = sFontAttrLast2.m_bUnderLine = false;
					sColorAttrLast.m_cTEXT = sColorAttrLast2.m_cTEXT = (COLORREF)-1;
					sColorAttrLast.m_cBACK = sColorAttrLast2.m_cBACK = (COLORREF)-1;
				}
			}
			if (bLineNumLayout && !bAddCRLF) {
				cmemClip.AppendString(WCODE::CRLF, 2);
			}
			// 2014.06.25 バッファ拡張
			if (cmemClip.capacity() < cmemClip.GetStringLength() + 100) {
				cmemClip.AllocStringBuffer( cmemClip.capacity() + cmemClip.capacity() / 2 );
			}
		}
	}
	if (sFontAttrLast2.m_bBoldFont) {
		cmemClip.AppendString(L"</b>", 4);
	}
	if (sFontAttrLast2.m_bUnderLine) {
		cmemClip.AppendString(L"</u>", 4);
	}
	if (sColorAttrLast2.m_cTEXT != (COLORREF)-1) {
		cmemClip.AppendString(L"</span>", 7);
	}
	cmemClip.AppendString(L"</pre>", 6);

	Clipboard cClipboard(GetEditWindow()->GetHwnd());
	if (!cClipboard) {
		return;
	}
	cClipboard.Empty();
	cClipboard.SetHtmlText(cmemClip);
	cClipboard.SetText(cmemClip.GetStringPtr(), cmemClip.GetStringLength(), false, false);
}



/*!
	@date 2014.12.30 Moca 同じColorStrategyで違う色に切り替わったときに対応
*/
ColorStrategy* ViewCommander::GetColorStrategyHTML(
	const StringRef&	stringLine,
	int					iLogic,
	const ColorStrategyPool*	pool,
	ColorStrategy**	ppStrategy,
	ColorStrategy**	ppStrategyFound,		// [in,out]
	bool& bChange
	)
{
	// 検索色終了
	if (*ppStrategyFound) {
		if ((*ppStrategyFound)->EndColor(stringLine, iLogic)) {
			*ppStrategyFound = NULL;
			bChange = true;
		}
	}

	// 検索色開始
	if (!*ppStrategyFound) {
		Color_Found*  pcFound  = pool->GetFoundStrategy();
		if (pcFound->BeginColor(stringLine, iLogic)) {
			*ppStrategyFound = pcFound;
			bChange = true;
		}
	}

	// 色終了
	if (*ppStrategy) {
		if ((*ppStrategy)->EndColor(stringLine, iLogic)) {
			*ppStrategy = NULL;
			bChange = true;
		}
	}

	// 色開始
	if (!*ppStrategy) {
		int size = pool->GetStrategyCount();
		for (int i=0; i<size; ++i) {
			if (pool->GetStrategy(i)->BeginColor(stringLine, iLogic)) {
				*ppStrategy = pool->GetStrategy(i);
				bChange = true;
				break;
			}
		}
	}
	if (*ppStrategyFound) {
		return *ppStrategyFound;
	}
	return *ppStrategy;
}

// 選択範囲内行番号色付きHTMLコピー
void ViewCommander::Command_COPY_COLOR_HTML_LINENUMBER()
{
	Command_COPY_COLOR_HTML(true);
}


/*!	現在編集中のファイル名をクリップボードにコピー
	2002/2/3 aroka
*/
void ViewCommander::Command_COPYFILENAME(void)
{
	if (GetDocument()->m_docFile.GetFilePathClass().IsValidPath()) {
		// クリップボードにデータを設定
		const WCHAR* pszFile = to_wchar(GetDocument()->m_docFile.GetFileName());
		m_pCommanderView->MySetClipboardData(pszFile , wcslen(pszFile), false);
	}else {
		ErrorBeep();
	}
}


// 現在編集中のファイルのパス名をクリップボードにコピー
void ViewCommander::Command_COPYPATH(void)
{
	if (GetDocument()->m_docFile.GetFilePathClass().IsValidPath()) {
		// クリップボードにデータを設定
		const TCHAR* szPath = GetDocument()->m_docFile.GetFilePath();
		m_pCommanderView->MySetClipboardData(szPath, _tcslen(szPath), false);
	}else {
		ErrorBeep();
	}
}


// May 9, 2000 genta
// 現在編集中のファイルのパス名とカーソル位置をクリップボードにコピー
void ViewCommander::Command_COPYTAG(void)
{
	if (GetDocument()->m_docFile.GetFilePathClass().IsValidPath()) {
		wchar_t	buf[MAX_PATH + 20];

		LogicPoint ptColLine;

		// 論理行番号を得る
		GetDocument()->m_layoutMgr.LayoutToLogic(GetCaret().GetCaretLayoutPos(), &ptColLine);

		// クリップボードにデータを設定
		auto_sprintf( buf, L"%ts (%d,%d): ", GetDocument()->m_docFile.GetFilePath(), ptColLine.y+1, ptColLine.x+1 );
		m_pCommanderView->MySetClipboardData(buf, wcslen(buf), false);
	}else {
		ErrorBeep();
	}
}


//// キー割り当て一覧をコピー
// Dec. 26, 2000 JEPRO // Jan. 24, 2001 JEPRO debug version (directed by genta)
void ViewCommander::Command_CREATEKEYBINDLIST(void)
{
	NativeW cMemKeyList;
	auto& csKeyBind = GetDllShareData().m_common.m_keyBind;
	KeyBind::CreateKeyBindList(
		G_AppInstance(),
		csKeyBind.m_nKeyNameArrNum,
		csKeyBind.m_pKeyNameArr,
		cMemKeyList,
		&GetDocument()->m_funcLookup,	// Oct. 31, 2001 genta 追加
		FALSE	// 2007.02.22 ryoji 追加
	);

	// Windowsクリップボードにコピー
	// 2004.02.17 Moca 関数化
	SetClipboardText(
		EditWnd::getInstance()->m_splitterWnd.GetHwnd(),
		cMemKeyList.GetStringPtr(),
		cMemKeyList.GetStringLength()
	);
}

