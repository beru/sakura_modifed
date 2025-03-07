#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"
#include "view/colors/ColorStrategy.h"
#include "view/colors/Color_Found.h"
#include "uiparts/WaitCursor.h"
#include "util/os.h"

// ViewCommanderクラスのコマンド(クリップボード系)関数群

/** 切り取り(選択範囲をクリップボードにコピーして削除) */
void ViewCommander::Command_Cut(void)
{
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	auto& csEdit = GetDllShareData().common.edit;

	// 範囲選択がされていない
	if (!selInfo.IsTextSelected()) {
		// 非選択時は、カーソル行を切り取り
		if (!csEdit.bEnableNoSelectCopy) {
			return;	// 何もしない（音も鳴らさない）
		}
		// 行切り取り(折り返し単位)
		Command_Cut_Line();
		return;
	}
	bool bBeginBoxSelect = selInfo.IsBoxSelecting();

	// 選択範囲のデータを取得
	// 正常時は true, 範囲未選択の場合は false を返す
	NativeW memBuf;
	if (!view.GetSelectedData(&memBuf, false, NULL, false, csEdit.bAddCRLFWhenCopy)) {
		ErrorBeep();
		return;
	}
	// クリップボードにデータを設定
	if (!view.MySetClipboardData(memBuf.GetStringPtr(), memBuf.GetStringLength(), bBeginBoxSelect)) {
		ErrorBeep();
		return;
	}
	memBuf.Clear();

	// カーソル位置または選択エリアを削除
	view.DeleteData(true);
	return;
}


/**	選択範囲をクリップボードにコピー */
void ViewCommander::Command_Copy(
	bool	bIgnoreLockAndDisable,	// [in] 選択範囲を解除するか？
	bool	bAddCRLFWhenCopy,		// [in] 折り返し位置に改行コードを挿入するか？
	EolType	neweol					// [in] コピーするときのEOL。
	)
{
	NativeW memBuf;
	auto& selInfo = view.GetSelectionInfo();
	auto& csEdit = GetDllShareData().common.edit;
	// クリップボードに入れるべきテキストデータを、memBufに格納する
	if (!selInfo.IsTextSelected()) {
		// 非選択時は、カーソル行をコピーする
		if (!csEdit.bEnableNoSelectCopy) {
			return;	// 何もしない（音も鳴らさない）
		}
		view.CopyCurLine(
			bAddCRLFWhenCopy,
			neweol,
			csEdit.bEnableLineModePaste
		);
	}else {
		// テキストが選択されているときは、選択範囲のデータを取得
		bool bBeginBoxSelect = selInfo.IsBoxSelecting();
		// 選択範囲のデータを取得
		// 正常時はtrue,範囲未選択の場合はfalseを返す
		if (!view.GetSelectedData(&memBuf, false, NULL, false, bAddCRLFWhenCopy, neweol)) {
			ErrorBeep();
			return;
		}

		// クリップボードにデータmemBufの内容を設定
		if (!view.MySetClipboardData(
			memBuf.GetStringPtr(),
			memBuf.GetStringLength(),
			bBeginBoxSelect,
			false
			)
		) {
			ErrorBeep();
			return;
		}
	}
	memBuf.Clear();

	// 選択範囲の後片付け
	if (!bIgnoreLockAndDisable) {
		// 選択状態のロック
		if (selInfo.bSelectingLock) {
			selInfo.bSelectingLock = false;
			selInfo.PrintSelectionInfoMsg();
			if (!selInfo.IsTextSelected()) {
				GetCaret().underLine.CaretUnderLineON(true, false);
			}
		}
	}
	if (csEdit.bCopyAndDisablSelection) {	// コピーしたら選択解除
		// テキストが選択されているか
		if (selInfo.IsTextSelected()) {
			// 現在の選択範囲を非選択状態に戻す
			selInfo.DisableSelectArea(true);
		}
	}
}


/** 貼り付け(クリップボードから貼り付け)
	@param [in] option 貼り付け時のオプション
	@li 0x01 改行コード変換有効
	@li 0x02 改行コード変換無効
	@li 0x04 ラインモード貼り付け有効
	@li 0x08 ラインモード貼り付け無効
	@li 0x10 矩形コピーは常に矩形貼り付け
	@li 0x20 矩形コピーは常に通常貼り付け
*/
void ViewCommander::Command_Paste(int option)
{
	auto& selInfo = view.GetSelectionInfo();

	if (selInfo.IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	auto& commonSetting = GetDllShareData().common;
	// クリップボードからデータを取得 -> memClip, bColumnSelect
	NativeW	memClip;
	bool	bColumnSelect;
	bool	bLineSelect = false;
	bool	bLineSelectOption = 
		((option & 0x04) == 0x04) ? true :
		((option & 0x08) == 0x08) ? false :
		commonSetting.edit.bEnableLineModePaste;
	
	if (!view.MyGetClipboardData(memClip, &bColumnSelect, bLineSelectOption ? &bLineSelect: nullptr)) {
		ErrorBeep();
		return;
	}

	// クリップボードデータ取得 -> pszText, nTextLen
	size_t nTextLen;
	const wchar_t*	pszText = memClip.GetStringPtr(&nTextLen);

	bool bConvertEol = 
		((option & 0x01) == 0x01) ? true :
		((option & 0x02) == 0x02) ? false :
		commonSetting.edit.bConvertEOLPaste;

	bool bAutoColumnPaste = 
		((option & 0x10) == 0x10) ? true :
		((option & 0x20) == 0x20) ? false :
		commonSetting.edit.bAutoColumnPaste;
	
	// 矩形コピーのテキストは常に矩形貼り付け
	if (bAutoColumnPaste) {
		// 矩形コピーのデータなら矩形貼り付け
		if (bColumnSelect) {
			if (selInfo.IsMouseSelecting()) {
				ErrorBeep();
				return;
			}
			if (!commonSetting.view.bFontIs_FixedPitch) {
				return;
			}
			Command_PasteBox(pszText, nTextLen);
			view.AdjustScrollBars();
			view.Redraw();
			return;
		}
	}

	// 行コピー（MSDEVLineSelect形式）のテキストで末尾が改行になっていなければ改行を追加する
	// ※レイアウト折り返しの行コピーだった場合は末尾が改行になっていない
	if (bLineSelect) {
		if (!WCODE::IsLineDelimiter(pszText[nTextLen-1], GetDllShareData().common.edit.bEnableExtEol)) {
			memClip.AppendString(GetDocument().docEditor.GetNewLineCode().GetValue2());
			pszText = memClip.GetStringPtr(&nTextLen);
		}
	}

	if (bConvertEol) {
		size_t nConvertedTextLen = ConvertEol(pszText, nTextLen, NULL);
		std::vector<wchar_t> szConvertedText(nConvertedTextLen);
		wchar_t* pszConvertedText = &szConvertedText[0];
		ConvertEol(pszText, nTextLen, pszConvertedText);
		// テキストを貼り付け
		Command_InsText(true, pszConvertedText, nConvertedTextLen, true, bLineSelect);
	}else {
		// テキストを貼り付け
		Command_InsText(true, pszText, nTextLen, true, bLineSelect);
	}
}

// メモリデータを矩形貼り付け用のデータと解釈して処理する。
void ViewCommander::Command_PasteBox(
	const wchar_t* szPaste,
	size_t nPasteSize
	)
{
	/* これらの動作は残しておきたいのだが、呼び出し側で責任を持ってやってもらうことに変更。
	if (view.GetSelectionInfo().IsMouseSelecting())	// マウスによる範囲選択中
	{
		ErrorBeep();
		return;
	}
	if (!GetDllShareData().common.bFontIs_FixedPitch)	// 現在のフォントは固定幅フォントである
	{
		return;
	}
	*/

	GetDocument().docEditor.SetModified(true, true);

	bool bDrawSwitchOld = view.SetDrawSwitch(false);

	// とりあえず選択範囲を削除
	if (view.GetSelectionInfo().IsTextSelected()) {
		view.DeleteData(false);
	}

	WaitCursor waitCursor(view.GetHwnd(), 10000 < nPasteSize);
	HWND hwndProgress = NULL;
	int nProgressPos = 0;
	if (waitCursor.IsEnable()) {
		hwndProgress = view.StartProgress();
	}

	auto& caret = GetCaret();
	Point ptCurOld = caret.GetCaretLayoutPos();

	int nCount = 0;
	bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;

	// 貼り付けデータの最後にCR/LFが無い場合の対策
	// データの最後まで処理 i.e. nBgnがnPasteSizeを超えたら終了
	//for (nPos = 0; nPos < nPasteSize;)
	size_t nPos;
	Point ptLayoutNew;	// 挿入された部分の次の位置
	for (size_t nBgn=nPos=0; nBgn<nPasteSize;) {
		// 貼り付けデータの最後にCR/LFが無いと
		// 最終行のPaste処理が動かないので，
		// データの末尾に来た場合は強制的に処理するようにする
		if (WCODE::IsLineDelimiter(szPaste[nPos], bExtEol) || nPos == nPasteSize) {
			// 現在位置にデータを挿入
			ASSERT_GE(nPos, nBgn);
			if (nPos - nBgn > 0) {
				view.InsertData_CEditView(
					ptCurOld + Point(0, nCount),
					&szPaste[nBgn],
					nPos - nBgn,
					&ptLayoutNew,
					false
				);
			}

			// この行の挿入位置へカーソルを移動
			caret.MoveCursor(ptCurOld + Point(0, nCount), false);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
			// カーソル行が最後の行かつ行末に改行が無く、挿入すべきデータがまだある場合
			bool bAddLastCR = false;
			const Layout*	pLayout;
			size_t nLineLen = 0;
			const wchar_t* pLine = GetDocument().layoutMgr.GetLineStr(caret.GetCaretLayoutPos().y, &nLineLen, &pLayout);

			if (pLine && 1 <= nLineLen) {
				if (WCODE::IsLineDelimiter(pLine[nLineLen - 1], bExtEol)) {
				}else {
					bAddLastCR = true;
				}
			}else {
				bAddLastCR = true;
			}

			if (bAddLastCR) {
//				MYTRACE(_T(" カーソル行が最後の行かつ行末に改行が無く、\n挿入すべきデータがまだある場合は行末に改行を挿入。\n"));
				size_t nInsPosX = view.LineIndexToColumn(pLayout, nLineLen);

				view.InsertData_CEditView(
					Point((int)nInsPosX, caret.GetCaretLayoutPos().y),
					GetDocument().docEditor.GetNewLineCode().GetValue2(),
					GetDocument().docEditor.GetNewLineCode().GetLen(),
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
			int newPos = ::MulDiv((int)nPos, 100, (int)nPasteSize);
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
	caret.MoveCursor(ptCurOld, true);
	caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

	if (!view.bDoing_UndoRedo) {	// Undo, Redoの実行中か
		// 操作の追加
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				caret.GetCaretLogicPos()	// 操作前後のキャレット位置
			)
		);
	}

	view.SetDrawSwitch(bDrawSwitchOld);
}


/** 矩形貼り付け(クリップボードから矩形貼り付け)
	@param [in] option 未使用
*/
void ViewCommander::Command_PasteBox(int option)
{
	if (view.GetSelectionInfo().IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	if (!GetDllShareData().common.view.bFontIs_FixedPitch) {	// 現在のフォントは固定幅フォントである
		return;
	}

	// クリップボードからデータを取得
	NativeW memClip;
	if (!view.MyGetClipboardData(memClip, NULL)) {
		ErrorBeep();
		return;
	}
	// \0コピー対策
	size_t nstrlen;
	const wchar_t* lptstr = memClip.GetStringPtr(&nstrlen);

	Command_PasteBox(lptstr, nstrlen);
	view.AdjustScrollBars();
	view.Redraw();
}


// 矩形文字列挿入
void ViewCommander::Command_InsBoxText(
	const wchar_t* pszPaste,
	int nPasteSize
	)
{
	if (view.GetSelectionInfo().IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	if (!GetDllShareData().common.view.bFontIs_FixedPitch) {	// 現在のフォントは固定幅フォントである
		return;
	}

	Command_PasteBox(pszPaste, nPasteSize);
	view.AdjustScrollBars();
	view.Redraw();
}


/*! テキストを貼り付け */
void ViewCommander::Command_InsText(
	bool			bRedraw,		// 
	const wchar_t*	pszText,		// [in] 貼り付ける文字列。
	size_t			nTextLen,		// [in] pszTextの長さ。-1を指定すると、pszTextをNUL終端文字列とみなして長さを自動計算する
	bool			bNoWaitCursor,	// 
	bool			bLinePaste,		// [in] ラインモード貼り付け
	bool			bFastMode,		// [in] 高速モード(レイアウト座標は無視する)
	const Range*	pSelectLogic	// [in] オプション。高速モードのときの削除範囲ロジック単位
	)
{
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	WaitCursor waitCursor(
		view.GetHwnd(),
		10000 < nTextLen && !selInfo.IsBoxSelecting()
	);

	GetDocument().docEditor.SetModified(true, bRedraw);

	auto& caret = GetCaret();

	// テキストが選択されているか
	if (selInfo.IsTextSelected() || bFastMode) {
		// 矩形範囲選択中か
		if (selInfo.IsBoxSelecting()) {
			// 改行までを抜き出す
			size_t i;
			bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
			for (i=0; i<nTextLen; ++i) {
				if (WCODE::IsLineDelimiter(pszText[i], bExtEol)) {
					break;
				}
			}
			Command_Indent(pszText, i);
			goto end_of_func;
		}else {
			// 同一行の行末以降のみが選択されている場合には選択無しと見なす
			bool bAfterEOLSelect = false;
			if (!bFastMode) {
				size_t len;
				const Layout* pLayout;
				const wchar_t* line = GetDocument().layoutMgr.GetLineStr(GetSelect().GetFrom().y, &len, &pLayout);
				size_t pos = (!line) ? 0 : view.LineColumnToIndex(pLayout, GetSelect().GetFrom().x);
				// 開始位置が行末より後ろで、終了位置が同一行
				if (pos >= len && GetSelect().IsLineOne()) {
					caret.SetCaretLayoutPos(Point(GetSelect().GetFrom().x, caret.GetCaretLayoutPos().y)); // キャレットX変更
					selInfo.DisableSelectArea(false);
					bAfterEOLSelect = true;
				}
			}
			if (!bAfterEOLSelect) {
				// データ置換 削除&挿入にも使える
				// 行コピーの貼り付けでは選択範囲は削除（後で行頭に貼り付ける）
				view.ReplaceData_CEditView(
					GetSelect(),				// 選択範囲
					bLinePaste? L"": pszText,	// 挿入するデータ
					bLinePaste? 0: nTextLen,	// 挿入するデータの長さ
					bRedraw,
					view.bDoing_UndoRedo ? NULL : GetOpeBlk(),
					bFastMode,
					pSelectLogic
				);
				if (!bLinePaste)
					goto end_of_func;
			}
		}
	}

	{	// 非選択時の処理 or ラインモード貼り付け時の残りの処理
		int nPosX_PHY_Delta(0);
		if (bLinePaste) {
			// 挿入ポイント（折り返し単位行頭）にカーソルを移動
			Point ptCaretBefore = caret.GetCaretLogicPos();	// 操作前のキャレット位置
			Command_GoLineTop(false, 1);								// 行頭に移動(折り返し単位)
			Point ptCaretAfter = caret.GetCaretLogicPos();	// 操作後のキャレット位置

			// 挿入ポイントと元の位置との差分文字数
			nPosX_PHY_Delta = ptCaretBefore.x - ptCaretAfter.x;

			// UNDO用記録
			if (!view.bDoing_UndoRedo) {
				GetOpeBlk()->AppendOpe(
					new MoveCaretOpe(
						ptCaretBefore,	// 操作前のキャレット位置
						ptCaretAfter	// 操作後のキャレット位置
					)
				);
			}
		}

		// 現在位置にデータを挿入
		Point ptLayoutNew; // 挿入された部分の次の位置
		view.InsertData_CEditView(
			caret.GetCaretLayoutPos(),
			pszText,
			nTextLen,
			&ptLayoutNew,
			bRedraw
		);

		// 挿入データの最後へカーソルを移動
		caret.MoveCursor(ptLayoutNew, bRedraw);
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

		if (bLinePaste) {
			// 元の位置へカーソルを移動
			Point ptCaretBefore = caret.GetCaretLogicPos();	// 操作前のキャレット位置
			Point ptLayout = GetDocument().layoutMgr.LogicToLayout(
				ptCaretBefore + Point(nPosX_PHY_Delta, 0)
			);
			caret.MoveCursor(ptLayout, bRedraw);					// カーソル移動
			Point ptCaretAfter = caret.GetCaretLogicPos();	// 操作後のキャレット位置
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

			// UNDO用記録
			if (!view.bDoing_UndoRedo) {
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
void ViewCommander::Command_AddTail(
	const wchar_t*	pszData,	// 追加するテキスト
	size_t			nDataLen	// 追加するテキストの長さ。文字単位。-1を指定すると、テキスト終端まで。
	)
{
	// テキスト長自動計算
	if ((int)nDataLen == -1 && pszData) {
		nDataLen = wcslen(pszData);
	}

	GetDocument().docEditor.SetModified(true, true);

	// ファイルの最後に移動
	Command_GoFileEnd(false);

	auto& caret = GetCaret();
	// 現在位置にデータを挿入
	Point ptLayoutNew;	// 挿入された部分の次の位置
	view.InsertData_CEditView(
		caret.GetCaretLayoutPos(),
		pszData,
		nDataLen,
		&ptLayoutNew,
		true
	);

	// 挿入データの最後へカーソルを移動
	caret.MoveCursor(ptLayoutNew, true);
	caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
}


// 選択範囲内全行コピー
void ViewCommander::Command_CopyLines(void)
{
	// 選択範囲内の全行をクリップボードにコピーする
	view.CopySelectedAllLines(
		NULL,	// 引用符
		false	// 行番号を付与する
	);
	return;
}


// 選択範囲内全行引用符付きコピー
void ViewCommander::Command_CopyLinesAsPassage(void)
{
	// 選択範囲内の全行をクリップボードにコピーする
	view.CopySelectedAllLines(
		GetDllShareData().common.format.szInyouKigou,	// 引用符
		false 									// 行番号を付与する
	);
	return;
}


// 選択範囲内全行行番号付きコピー
void ViewCommander::Command_CopyLinesWithLineNumber(void)
{
	// 選択範囲内の全行をクリップボードにコピーする
	view.CopySelectedAllLines(
		NULL,	// 引用符
		true	// 行番号を付与する
	);
}


static bool AppendHTMLColor(
	const ColorAttr& colorAttrLast, ColorAttr& colorAttrLast2,
	const FontAttr& fontAttrLast, FontAttr& fontAttrLast2,
	const wchar_t* pAppendStr, int nLen,
	NativeW& memClip
	)
{
	if (fontAttrLast.bBoldFont != fontAttrLast2.bBoldFont
		|| fontAttrLast.bUnderLine != fontAttrLast2.bUnderLine
		|| colorAttrLast.cTEXT != colorAttrLast2.cTEXT
		|| colorAttrLast.cBACK != colorAttrLast2.cBACK
	) {
		if (fontAttrLast2.bBoldFont) {
			memClip.AppendStringLiteral(L"</b>");
		}
		if (fontAttrLast2.bUnderLine) {
			if (colorAttrLast.cTEXT != colorAttrLast2.cTEXT
				|| colorAttrLast.cBACK != colorAttrLast2.cBACK
				|| fontAttrLast.bUnderLine != fontAttrLast2.bUnderLine
			) {
				memClip.AppendStringLiteral(L"</u>");
			}
		}
		if (colorAttrLast.cTEXT != colorAttrLast2.cTEXT
			|| colorAttrLast.cBACK != colorAttrLast2.cBACK
		) {
			if (colorAttrLast2.cTEXT != (COLORREF)-1) {
				memClip.AppendStringLiteral(L"</span>");
			}
			if (colorAttrLast.cTEXT != (COLORREF)-1) {
				if (colorAttrLast.cTEXT != colorAttrLast2.cTEXT
					|| colorAttrLast.cBACK != colorAttrLast2.cBACK
				) {
					wchar_t szColor[60];
					DWORD dwTEXTColor = (GetRValue(colorAttrLast.cTEXT) << 16) + (GetGValue(colorAttrLast.cTEXT) << 8) + GetBValue(colorAttrLast.cTEXT);
					DWORD dwBACKColor = (GetRValue(colorAttrLast.cBACK) << 16) + (GetGValue(colorAttrLast.cBACK) << 8) + GetBValue(colorAttrLast.cBACK);
					swprintf(szColor, L"<span style=\"color:#%06x;background-color:#%06x\">", dwTEXTColor, dwBACKColor);
					memClip.AppendString(szColor);
				}
			}
		}
		if (fontAttrLast.bUnderLine) {
			if (colorAttrLast.cTEXT != colorAttrLast2.cTEXT
				|| colorAttrLast.cBACK != colorAttrLast2.cBACK
				|| fontAttrLast.bUnderLine != fontAttrLast2.bUnderLine
			) {
				memClip.AppendStringLiteral(L"<u>");
			}
		}
		if (fontAttrLast.bBoldFont) {
			memClip.AppendStringLiteral(L"<b>");
		}
		colorAttrLast2 = colorAttrLast;
		fontAttrLast2  = fontAttrLast;
	}
	NativeW memBuf(pAppendStr, nLen);
	memBuf.Replace(L"&", L"&amp;");
	memBuf.Replace(L"<", L"&lt;");
	memBuf.Replace(L">", L"&gt;");
	memClip.AppendNativeData(memBuf);
	if (0 < nLen) {
		return WCODE::IsLineDelimiter(
			pAppendStr[nLen-1],
			GetDllShareData().common.edit.bEnableExtEol
		);
	}
	return false;
}


// 選択範囲内色付きHTMLコピー
void ViewCommander::Command_Copy_Color_HTML(bool bLineNumber)
{
	auto& selInfo = view.GetSelectionInfo();
	if (!selInfo.IsTextSelected()
	  || GetSelect().GetFrom() == GetSelect().GetTo()
	) {
		return;
	}
	const TypeConfig& type = GetDocument().docType.GetDocumentAttribute();
	bool bLineNumLayout = GetDllShareData().common.edit.bAddCRLFWhenCopy
		|| selInfo.IsBoxSelecting();
	Rect rcSel;
	TwoPointToRect(
		&rcSel,
		GetSelect().GetFrom(),	// 範囲選択開始
		GetSelect().GetTo()		// 範囲選択終了
	);
	// 修飾分を除いたバッファの長さをだいたいで計算
	Range sSelectLogic;
	sSelectLogic.Clear(-1);
	size_t nBuffSize = 0;
	const Layout* pLayoutTop = nullptr;
	{
		const Layout* pLayout;
		{
			size_t nLineLenTmp;
			GetDocument().layoutMgr.GetLineStr(rcSel.top, &nLineLenTmp, &pLayout);
		}
		pLayoutTop = pLayout;
		int i = rcSel.top;
		for (; pLayout && i <= rcSel.bottom; ++i, pLayout = pLayout->GetNextLayout()) {
			// 指定された桁に対応する行のデータ内の位置を調べる
			size_t nIdxFrom;
			size_t nIdxTo;
			if (selInfo.IsBoxSelecting()) {
				nIdxFrom = view.LineColumnToIndex(pLayout, rcSel.left);
				nIdxTo   = view.LineColumnToIndex(pLayout, rcSel.right);
				// 改行は除く
				if (nIdxTo - nIdxFrom > 0) {
					const wchar_t* pLine = pLayout->GetPtr();
					if (pLine[nIdxTo - 1] == L'\n' || pLine[nIdxTo - 1] == L'\r') {
						--nIdxTo;
					}
				}
				if (i == rcSel.top) {
					sSelectLogic.SetFromY(pLayout->GetLogicLineNo());
					sSelectLogic.SetFromX((int)nIdxFrom);
				}
				if (i == rcSel.bottom) {
					sSelectLogic.SetToY(pLayout->GetLogicLineNo());
					sSelectLogic.SetToX((int)nIdxTo);
				}
			}else {
				if (i == rcSel.top) {
					nIdxFrom = view.LineColumnToIndex(pLayout, rcSel.left);
					sSelectLogic.SetFromY(pLayout->GetLogicLineNo());
					sSelectLogic.SetFromX((int)nIdxFrom);
				}else {
					nIdxFrom = 0;
				}
				if (i == rcSel.bottom) {
					nIdxTo = view.LineColumnToIndex(pLayout, rcSel.right);
					sSelectLogic.SetToY(pLayout->GetLogicLineNo());
					sSelectLogic.SetToX((int)nIdxTo);
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
			sSelectLogic.SetToY((int)GetDocument().docLineMgr.GetLineCount());
			sSelectLogic.SetToX(0);
		}
	}
	// 行番号の幅を計算
	int nLineNumberMaxLen = 0;
	wchar_t szLineFormat[10];
	szLineFormat[0] = L'\0';
	NativeW memNullLine;
	if (bLineNumber) {
		int nLineNumberMax;
		if (type.bLineNumIsCRLF) {
			nLineNumberMax = sSelectLogic.GetTo().GetY();
		}else {
			nLineNumberMax = rcSel.bottom;
		}
		int nWork = 10;
		int i;
		memNullLine.AppendStringLiteral(L" ");
		for (i=1; i<12; ++i) {
			if (nWork > nLineNumberMax) {
				break;
			}
			nWork *= 10;
			memNullLine.AppendStringLiteral(L" ");
		}
		nLineNumberMaxLen = i + 1; // "%d:"
		memNullLine.AppendStringLiteral(L":");
		swprintf(szLineFormat, L"%%%dd:", i);
	}
	if (bLineNumLayout) {
		nBuffSize += nLineNumberMaxLen * (rcSel.bottom - rcSel.top + 1);
	}else {
		nBuffSize += nLineNumberMaxLen * (sSelectLogic.GetTo().y - sSelectLogic.GetFrom().y + 1);
	}
	NativeW memClip;
	memClip.AllocStringBuffer(nBuffSize + 11);
	{
		COLORREF cBACK = type.colorInfoArr[COLORIDX_TEXT].colorAttr.cBACK;
		DWORD dwBACKColor = (GetRValue(cBACK) << 16) + (GetGValue(cBACK) << 8) + GetBValue(cBACK);
		wchar_t szBuf[50];
		swprintf(szBuf, L"<pre style=\"background-color:#%06x\">", dwBACKColor);
		memClip.AppendString(szBuf);
	}
	int nLayoutLineNum = rcSel.top;
	const int nLineNumLast = sSelectLogic.GetTo().y;
	const DocLine* pDocLine = pLayoutTop->GetDocLineRef();
	const Layout* pLayout = pLayoutTop;
	while (pLayout && pLayout->GetLogicOffset()) {
		pLayout = pLayout->GetPrevLayout();
	}
	ColorAttr colorAttr = { (COLORREF)-1, (COLORREF)-1 };
	ColorAttr colorAttrNext = { (COLORREF)-1, (COLORREF)-1 };
	ColorAttr colorAttrLast = { (COLORREF)-1, (COLORREF)-1 };
	ColorAttr colorAttrLast2 = { (COLORREF)-1, (COLORREF)-1 };
	FontAttr fontAttr = { false, false };
	FontAttr fontAttrNext = { false, false };
	FontAttr fontAttrLast = { false, false };
	FontAttr fontAttrLast2 = { false, false };
	auto& pool = ColorStrategyPool::getInstance();
	pool.SetCurrentView(&view);
	for (auto nLineNum = sSelectLogic.GetFrom().y;
		nLineNum <= nLineNumLast;
		++nLineNum, pDocLine = pDocLine->GetNextLine()
	) {
		if (!pDocLine) {
			break;
		}
		pool.NotifyOnStartScanLogic();
		ColorStrategy* pStrategyNormal = nullptr;
		ColorStrategy* pStrategyFound = nullptr;
		ColorStrategy* pStrategy = nullptr;
		StringRef cStringLine(pDocLine->GetPtr(), pDocLine->GetLengthWithEOL());
		{
			pStrategy = pStrategyNormal = pool.GetStrategyByColor(pLayout->GetColorTypePrev());
			if (pStrategy) {
				pStrategy->InitStrategyStatus();
				pStrategy->SetStrategyColorInfo(pLayout->GetColorInfo());
			}
			int nColorIdx = ToColorInfoArrIndex(pLayout->GetColorTypePrev());
			if (nColorIdx != -1) {
				const ColorInfo& info = type.colorInfoArr[nColorIdx];
				fontAttr = info.fontAttr;
				colorAttr = info.colorAttr;
			}
		}
		const wchar_t* pLine = pDocLine->GetPtr();
		for (;
			pLayout->GetLogicLineNo() == nLineNum;
			++nLayoutLineNum, pLayout = pLayout->GetNextLayout()
		) {
			int nIdxFrom;
			int nIdxTo;
			const size_t nLineLen = pLayout->GetLengthWithoutEOL() + pLayout->GetLayoutEol().GetLen();
			if (nLayoutLineNum < rcSel.top) {
				nIdxTo = nIdxFrom = -1;
			}else {
				if (selInfo.IsBoxSelecting()) {
					nIdxFrom = (int)view.LineColumnToIndex(pLayout, rcSel.left);
					nIdxTo   = (int)view.LineColumnToIndex(pLayout, rcSel.right);
					// 改行は除く
					if (nIdxTo - nIdxFrom > 0) {
						const wchar_t* pLine = pLayout->GetPtr();
						if (pLine[nIdxTo - 1] == L'\n' || pLine[nIdxTo - 1] == L'\r') {
							--nIdxTo;
						}
					}
				}else {
					if (nLayoutLineNum == rcSel.top) {
						nIdxFrom = sSelectLogic.GetFrom().x;
					}else {
						nIdxFrom = 0;
					}
					if (nLayoutLineNum == rcSel.bottom) {
						nIdxTo = sSelectLogic.GetTo().x;
					}else {
						nIdxTo = (int)nLineLen;
					}
				}
			}
			// 最後の改行の次の行番号を表示しないように
			if (nIdxTo == 0 && nLayoutLineNum == rcSel.bottom) {
				break;
			}
			if (bLineNumber) {
				wchar_t szLineNum[14];
				if (type.bLineNumIsCRLF) {
					if (pLayout->GetLogicOffset() != 0) {
						if (bLineNumLayout) {
							memClip.AppendNativeData(memNullLine);
						}
					}else {
						int ret = swprintf(szLineNum, szLineFormat, nLineNum + 1);
						memClip.AppendString(szLineNum, ret);
					}
				}else {
					if (bLineNumLayout || pLayout->GetLogicOffset() == 0) {
						int ret = swprintf(szLineNum, szLineFormat, nLayoutLineNum + 1);
						memClip.AppendString(szLineNum, ret);
					}
				}
			}
			const int nLineStart = pLayout->GetLogicOffset();
			int nBgnLogic = nIdxFrom + nLineStart;
			int iLogic = nLineStart;
			bool bAddCRLF = false;
			for (; iLogic<nLineStart+(int)nLineLen; ++iLogic) {
				bool bChange = false;
				pStrategy = GetColorStrategyHTML(cStringLine, iLogic, &pool, &pStrategyNormal, &pStrategyFound, bChange);
				if (bChange) {
					int nColorIdx = ToColorInfoArrIndex(pStrategy ? pStrategy->GetStrategyColor() : COLORIDX_TEXT);
					if (nColorIdx != -1) {
						const ColorInfo& info = type.colorInfoArr[nColorIdx];
						colorAttrNext = info.colorAttr;
						fontAttrNext  = info.fontAttr;
					}
				}
				if (nIdxFrom != -1 && nIdxFrom + nLineStart <= iLogic && iLogic <= nIdxTo + nLineStart) {
					if (nIdxFrom + nLineStart == iLogic) {
						colorAttrLast = colorAttrNext;
						fontAttrLast  = fontAttrNext;
					}else if (
						nIdxFrom + nLineStart < iLogic
						&& (
							fontAttr.bBoldFont != fontAttrNext.bBoldFont
						  	|| fontAttr.bUnderLine != fontAttrNext.bUnderLine
							|| colorAttr.cTEXT != colorAttrNext.cTEXT
						  	|| colorAttr.cBACK != colorAttrNext.cBACK
						)
					) {
						bAddCRLF = AppendHTMLColor(colorAttrLast, colorAttrLast2,
							fontAttrLast, fontAttrLast2, pLine + nBgnLogic, iLogic - nBgnLogic, memClip);
						colorAttrLast = colorAttrNext;
						fontAttrLast  = fontAttrNext;
						nBgnLogic = iLogic;
					}else if (nIdxTo + nLineStart == iLogic) {
						bAddCRLF = AppendHTMLColor(colorAttrLast, colorAttrLast2,
							fontAttrLast, fontAttrLast2, pLine + nBgnLogic, iLogic - nBgnLogic, memClip);
						nBgnLogic = iLogic;
					}
				}
				colorAttr = colorAttrNext;
				fontAttr = fontAttrNext;
			}
			if (nIdxFrom != -1 && nIdxTo + nLineStart == iLogic) {
				bAddCRLF = AppendHTMLColor(colorAttrLast, colorAttrLast2,
					fontAttrLast, fontAttrLast2, pLine + nBgnLogic, iLogic - nBgnLogic, memClip);
			}
			if (bLineNumber) {
				bool bAddLineNum = true;
				const Layout* pLayoutNext = pLayout->GetNextLayout();
				if (pLayoutNext) {
					if (type.bLineNumIsCRLF) {
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
					if (fontAttrLast2.bBoldFont) {
						memClip.AppendStringLiteral(L"</b>");
					}
					if (fontAttrLast2.bUnderLine) {
						memClip.AppendStringLiteral(L"</u>");
					}
					if (colorAttrLast2.cTEXT != (COLORREF)-1) {
						memClip.AppendStringLiteral(L"</span>");
					}
					fontAttrLast.bBoldFont = fontAttrLast2.bBoldFont = false;
					fontAttrLast.bUnderLine = fontAttrLast2.bUnderLine = false;
					colorAttrLast.cTEXT = colorAttrLast2.cTEXT = (COLORREF)-1;
					colorAttrLast.cBACK = colorAttrLast2.cBACK = (COLORREF)-1;
				}
			}
			if (bLineNumLayout && !bAddCRLF) {
				memClip.AppendStringLiteral(WCODE::CRLF);
			}
			if (memClip.capacity() < memClip.GetStringLength() + 100) {
				memClip.AllocStringBuffer( memClip.capacity() + memClip.capacity() / 2 );
			}
		}
	}
	if (fontAttrLast2.bBoldFont) {
		memClip.AppendStringLiteral(L"</b>");
	}
	if (fontAttrLast2.bUnderLine) {
		memClip.AppendStringLiteral(L"</u>");
	}
	if (colorAttrLast2.cTEXT != (COLORREF)-1) {
		memClip.AppendStringLiteral(L"</span>");
	}
	memClip.AppendStringLiteral(L"</pre>");

	Clipboard clipboard(GetEditWindow().GetHwnd());
	if (!clipboard) {
		return;
	}
	clipboard.Empty();
	clipboard.SetHtmlText(memClip);
	clipboard.SetText(memClip.GetStringPtr(), memClip.GetStringLength(), false, false);
}

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
			*ppStrategyFound = nullptr;
			bChange = true;
		}
	}

	// 検索色開始
	if (!*ppStrategyFound) {
		Color_Found* pcFound  = pool->GetFoundStrategy();
		if (pcFound->BeginColor(stringLine, iLogic)) {
			*ppStrategyFound = pcFound;
			bChange = true;
		}
	}

	// 色終了
	if (*ppStrategy) {
		if ((*ppStrategy)->EndColor(stringLine, iLogic)) {
			*ppStrategy = nullptr;
			bChange = true;
		}
	}

	// 色開始
	if (!*ppStrategy) {
		size_t size = pool->GetStrategyCount();
		for (size_t i=0; i<size; ++i) {
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
void ViewCommander::Command_Copy_Color_HTML_LineNumber()
{
	Command_Copy_Color_HTML(true);
}


/*!	現在編集中のファイル名をクリップボードにコピー */
void ViewCommander::Command_CopyFileName(void)
{
	if (GetDocument().docFile.GetFilePathClass().IsValidPath()) {
		// クリップボードにデータを設定
		const wchar_t* pszFile = to_wchar(GetDocument().docFile.GetFileName());
		view.MySetClipboardData(pszFile , wcslen(pszFile), false);
	}else {
		ErrorBeep();
	}
}


// 現在編集中のファイルのパス名をクリップボードにコピー
void ViewCommander::Command_CopyPath(void)
{
	if (GetDocument().docFile.GetFilePathClass().IsValidPath()) {
		// クリップボードにデータを設定
		const TCHAR* szPath = GetDocument().docFile.GetFilePath();
		view.MySetClipboardData(szPath, _tcslen(szPath), false);
	}else {
		ErrorBeep();
	}
}

// 現在編集中のファイルのパス名とカーソル位置をクリップボードにコピー
void ViewCommander::Command_CopyTag(void)
{
	if (GetDocument().docFile.GetFilePathClass().IsValidPath()) {
		wchar_t	buf[MAX_PATH + 20];

		// 論理行番号を得る
		Point ptColLine = GetDocument().layoutMgr.LayoutToLogic(GetCaret().GetCaretLayoutPos());

		// クリップボードにデータを設定
		auto_sprintf( buf, L"%ts (%d,%d): ", GetDocument().docFile.GetFilePath(), ptColLine.y+1, ptColLine.x+1 );
		view.MySetClipboardData(buf, wcslen(buf), false);
	}else {
		ErrorBeep();
	}
}


//// キー割り当て一覧をコピー
void ViewCommander::Command_CreateKeyBindList(void)
{
	NativeW memKeyList;
	auto& csKeyBind = GetDllShareData().common.keyBind;
	KeyBind::CreateKeyBindList(
		G_AppInstance(),
		csKeyBind.nKeyNameArrNum,
		csKeyBind.pKeyNameArr,
		memKeyList,
		&GetDocument().funcLookup,
		false
	);

	// Windowsクリップボードにコピー
	SetClipboardText(
		EditWnd::getInstance().splitterWnd.GetHwnd(),
		memKeyList.GetStringPtr(),
		memKeyList.GetStringLength()
	);
}

