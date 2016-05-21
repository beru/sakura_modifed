/*!	@file
	@brief IMEの処理

	@author Norio Nakatani
	@date	1998/03/13 作成
	@date   2008/04/13 CEditView.cppから分離
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, genta, JEPRO, MIK
	Copyright (C) 2001, genta, GAE, MIK, hor, asa-o, Stonee, Misaka, novice, YAZAKI
	Copyright (C) 2002, YAZAKI, hor, aroka, MIK, Moca, minfu, KK, novice, ai, Azumaiya, genta
	Copyright (C) 2003, MIK, ai, ryoji, Moca, wmlhq, genta
	Copyright (C) 2004, genta, Moca, novice, naoh, isearch, fotomo
	Copyright (C) 2005, genta, MIK, novice, aroka, D.S.Koba, かろと, Moca
	Copyright (C) 2006, Moca, aroka, ryoji, fon, genta
	Copyright (C) 2007, ryoji, じゅうじ, maru

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "EditView.h"
#include <algorithm>
#include "charset/ShiftJis.h"
#include "doc/EditDoc.h"
#include "env/DllSharedData.h"
#include "_main/AppMode.h"
#include "window/EditWnd.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           IME                               //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/*!	IME ONか

	@date  2006.12.04 ryoji 新規作成（関数化）
*/
bool EditView::IsImeON(void)
{
	bool bRet;
	DWORD conv, sent;

	//	From here Nov. 26, 2006 genta
	HIMC hIme = ImmGetContext(GetHwnd());
	if (ImmGetOpenStatus(hIme) != FALSE) {
		ImmGetConversionStatus(hIme, &conv, &sent);
		if ((conv & IME_CMODE_NOCONVERSION) == 0) {
			bRet = true;
		}else {
			bRet = false;
		}
	}else {
		bRet = false;
	}
	ImmReleaseContext(GetHwnd(), hIme);
	//	To here Nov. 26, 2006 genta

	return bRet;
}

// IME編集エリアの位置を変更
void EditView::SetIMECompFormPos(void)
{
	//
	// If current composition form mode is near caret operation,
	// application should inform IME UI the caret position has been
	// changed. IME UI will make decision whether it has to adjust
	// composition window position.
	//
	//
	COMPOSITIONFORM	compForm;
	HIMC			hIMC = ::ImmGetContext(GetHwnd());
	POINT			point;

	::GetCaretPos(&point);
	compForm.dwStyle = CFS_POINT;
	compForm.ptCurrentPos.x = (long) point.x;
	compForm.ptCurrentPos.y = (long) point.y + GetCaret().GetCaretSize().cy - GetTextMetrics().GetHankakuHeight();

	if (hIMC) {
		::ImmSetCompositionWindow(hIMC, &compForm);
	}
	::ImmReleaseContext(GetHwnd() , hIMC);
}


// IME編集エリアの表示フォントを変更
void EditView::SetIMECompFormFont(void)
{
	//
	// If current composition form mode is near caret operation,
	// application should inform IME UI the caret position has been
	// changed. IME UI will make decision whether it has to adjust
	// composition window position.
	//
	//
	HIMC hIMC = ::ImmGetContext(GetHwnd());
	if (hIMC) {
		::ImmSetCompositionFont(hIMC, const_cast<LOGFONT *>(&(editWnd.GetLogfont())));
	}
	::ImmReleaseContext(GetHwnd() , hIMC);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          再変換・変換補助
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*!
	@brief IMEの再変換/前後参照で、カーソル位置から前後200chars位を取り出してRECONVERTSTRINGを埋める
	@param  pReconv  [out]  RECONVERTSTRING構造体へのポインタ。NULLあり
	@param  bUnicode        trueならばUNICODEで構造体を埋める
	@param  bDocumentFeed   trueならばIMR_DOCUMENTFEEDとして処理する
	@return   RECONVERTSTRINGのサイズ。0ならIMEは何もしない(はず)
	@date 2002.04.09 minfu
	@date 2010.03.16 Moca IMR_DOCUMENTFEED対応
*/
LRESULT EditView::SetReconvertStruct(
	PRECONVERTSTRING pReconv,
	bool bUnicode,
	bool bDocumentFeed
	)
{
	if (!bDocumentFeed) {
		nLastReconvIndex = -1;
		nLastReconvLine  = -1;
	}
	
	// 矩形選択中は何もしない
	if (GetSelectionInfo().IsBoxSelecting())
		return 0;

	// 2010.04.06 ビューモードでは何もしない
	if (AppMode::getInstance().IsViewMode()) {
		return 0;
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      選択範囲を取得                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 選択範囲を取得 -> ptSelect, ptSelectTo, nSelectedLen
	LogicPoint	ptSelect;
	LogicPoint	ptSelectTo;
	int			nSelectedLen;
	auto& caret = GetCaret();
	if (GetSelectionInfo().IsTextSelected()) {
		// テキストが選択されているとき
		pEditDoc->layoutMgr.LayoutToLogic(GetSelectionInfo().select.GetFrom(), &ptSelect);
		pEditDoc->layoutMgr.LayoutToLogic(GetSelectionInfo().select.GetTo(), &ptSelectTo);
		
		// 選択範囲が複数行の時、１ロジック行以内に制限
		if (ptSelectTo.y != ptSelect.y) {
			if (bDocumentFeed) {
				// 暫定：未選択として振舞う
				// 改善案：選択範囲は置換されるので、選択範囲の前後をIMEに渡す
				// ptSelectTo.y = ptSelectTo.y;
				ptSelectTo.x = ptSelect.x;
			}else {
				// 2010.04.06 対象をptSelect.yの行からカーソル行に変更
				const DocLine* pDocLine = pEditDoc->docLineMgr.GetLine(caret.GetCaretLogicPos().y);
				LogicInt targetY = caret.GetCaretLogicPos().y;
				// カーソル行が実質無選択なら、直前・直後の行を選択
				if (ptSelect.y == caret.GetCaretLogicPos().y
					&& pDocLine && pDocLine->GetLengthWithoutEOL() == caret.GetCaretLogicPos().x
				) {
					// カーソルが上側行末 => 次の行。行末カーソルでのShift+Upなど
					targetY = t_min(pEditDoc->docLineMgr.GetLineCount(),
						caret.GetCaretLogicPos().y + 1);
					pDocLine = pEditDoc->docLineMgr.GetLine(targetY);
				}else if (1
					&& ptSelectTo.y == caret.GetCaretLogicPos().y
					&& caret.GetCaretLogicPos().x == 0
				) {
					// カーソルが下側行頭 => 前の行。 行頭でShift+Down/Shift+End→Rightなど
					targetY = caret.GetCaretLogicPos().y - 1;
					pDocLine = pEditDoc->docLineMgr.GetLine(targetY);
				}
				// 選択範囲をxで指定：こちらはカーソルではなく選択範囲基準
				if (targetY == ptSelect.y) {
					// ptSelect.x; 未変更
					ptSelectTo.x = pDocLine ? pDocLine->GetLengthWithoutEOL() : 0;
				}else if (targetY == ptSelectTo.y) {
					ptSelect.x = 0;
					// ptSelectTo.x; 未変更
				}else {
					ptSelect.x = 0;
					ptSelectTo.x = pDocLine ? pDocLine->GetLengthWithoutEOL() : 0;
				}
				ptSelect.y = targetY;
				// ptSelectTo.y = targetY; 以下未使用
			}
		}
	}else {
		// テキストが選択されていないとき
		pEditDoc->layoutMgr.LayoutToLogic(caret.GetCaretLayoutPos(), &ptSelect);
		ptSelectTo = ptSelect;
	}
	nSelectedLen = ptSelectTo.x - ptSelect.x;
	// 以下 ptSelect.y ptSelect.x, nSelectedLen を使用

	// ドキュメント行取得 -> pCurDocLine
	DocLine* pCurDocLine = pEditDoc->docLineMgr.GetLine(ptSelect.GetY2());
	if (!pCurDocLine)
		return 0;

	// テキスト取得 -> pLine, nLineLen
	const int nLineLen = pCurDocLine->GetLengthWithoutEOL();
	if (nLineLen == 0)
		return 0;
	const wchar_t* pLine = pCurDocLine->GetPtr();

	// 2010.04.17 行頭から←選択だと「SelectToが改行の後ろの位置」にあるため範囲を調整する
	// フリーカーソル選択でも行末より後ろにカーソルがある
	if (nLineLen < ptSelect.x) {
		// 改行直前をIMEに渡すカーソル位置ということにする
		ptSelect.x = LogicInt(nLineLen);
		nSelectedLen = 0;
	}
	if (nLineLen <  ptSelect.x + nSelectedLen) {
		nSelectedLen = nLineLen - ptSelect.x;
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//              再変換範囲・考慮文字を修正                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// 再変換考慮文字列開始  // 行の中で再変換のAPIにわたすとする文字列の開始位置
	int nReconvIndex = 0;
	int nInsertCompLen = 0; // DOCUMENTFEED用。変換中の文字列をdwStrに混ぜる
	// Iはカーソル　[]が選択範囲=dwTargetStrLenだとして
	// 行：日本語をIします。
	// IME：にゅうｒ
	// APIに渡す文字列：日本語を[にゅうｒ]Iします。

	// 選択開始位置より前後200(or 50)文字ずつを考慮文字列にする
	const int nReconvMaxLen = (bDocumentFeed ? 50 : 200); //$$マジックナンバー注意
	while (ptSelect.x - nReconvIndex > nReconvMaxLen) {
		nReconvIndex = t_max<int>(nReconvIndex + 1, ::CharNextW_AnyBuild(pLine + nReconvIndex) - pLine);
	}
	
	// 再変換考慮文字列終了  // 行の中で再変換のAPIにわたすとする文字列の長さ
	int nReconvLen = nLineLen - nReconvIndex;
	if ((nReconvLen + nReconvIndex - ptSelect.x) > nReconvMaxLen) {
		const wchar_t*       p = pLine + ptSelect.x;
		const wchar_t* const q = pLine + ptSelect.x + nReconvMaxLen;
		while (p <= q) {
			p = t_max(p + 1, const_cast<LPCWSTR>(::CharNextW_AnyBuild(p)));
		}
		nReconvLen = p - pLine - nReconvIndex;
	}
	
	// 対象文字列の調整
	if (ptSelect.x + nSelectedLen > nReconvIndex + nReconvLen) {
		// 考慮分しかAPIに渡さないので、選択範囲を縮小
		nSelectedLen = nReconvLen + nReconvIndex - ptSelect.x;
	}
	
	if (bDocumentFeed) {
		// IMR_DOCUMENTFEEDでは、再変換対象はIMEから取得した入力中文字列
		nInsertCompLen = auto_strlen(szComposition);
		if (nInsertCompLen == 0) {
			// 2回呼ばれるので、szCompositionに覚えておく
			HWND hwnd = GetHwnd();
			HIMC hIMC = ::ImmGetContext(hwnd);
			if (!hIMC) {
				return 0;
			}
			auto_memset(szComposition, _T('\0'), _countof(szComposition));
			LONG immRet = ::ImmGetCompositionString(hIMC, GCS_COMPSTR, szComposition, _countof(szComposition));
			if (immRet == IMM_ERROR_NODATA || immRet == IMM_ERROR_GENERAL) {
				szComposition[0] = _T('\0');
			}
			::ImmReleaseContext(hwnd, hIMC);
			nInsertCompLen = auto_strlen(szComposition);
		}
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      構造体設定要素                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// 行の中で再変換のAPIにわたすとする文字列の長さ
	int         cbReconvLenWithNull; // byte
	DWORD       dwReconvTextLen;    // CHARs
	DWORD       dwReconvTextInsLen; // CHARs
	DWORD       dwCompStrOffset;    // byte
	DWORD       dwCompStrLen;       // CHARs
	DWORD       dwInsByteCount = 0; // byte
	NativeW    memBuf1;
	NativeA    memBuf2;
	const void* pszReconv; 
	const void* pszInsBuffer;

	// UNICODE→UNICODE
	if (bUnicode) {
		const WCHAR* pszCompInsStr = L"";
		size_t nCompInsStr = 0;
		if (nInsertCompLen) {
			pszCompInsStr = to_wchar(szComposition);
			nCompInsStr   = wcslen(pszCompInsStr);
		}
		dwInsByteCount      = nCompInsStr * sizeof(wchar_t);
		dwReconvTextLen     = nReconvLen;
		dwReconvTextInsLen  = dwReconvTextLen + nCompInsStr;                 // reconv文字列長。文字単位。
		cbReconvLenWithNull = (dwReconvTextInsLen + 1) * sizeof(wchar_t);    // reconvデータ長。バイト単位。
		dwCompStrOffset     = (Int)(ptSelect.x - nReconvIndex) * sizeof(wchar_t);    // compオフセット。バイト単位。
		dwCompStrLen        = nSelectedLen + nCompInsStr;                            // comp文字列長。文字単位。
		pszReconv           = reinterpret_cast<const void*>(pLine + nReconvIndex);   // reconv文字列へのポインタ。
		pszInsBuffer        = pszCompInsStr;
	}else {
		// UNICODE→ANSI
		const wchar_t* pszReconvSrc =  pLine + nReconvIndex;

		// 考慮文字列の開始から対象文字列の開始まで -> dwCompStrOffset
		if (ptSelect.x - nReconvIndex > 0) {
			memBuf1.SetString(pszReconvSrc, ptSelect.x - nReconvIndex);
			ShiftJis::UnicodeToSJIS(memBuf1, memBuf2._GetMemory());
			dwCompStrOffset = memBuf2._GetMemory()->GetRawLength();				//compオフセット。バイト単位。
		}else {
			dwCompStrOffset = 0;
		}
		
		pszInsBuffer = "";
		// 対象文字列の開始から対象文字列の終了まで -> dwCompStrLen
		if (nSelectedLen > 0) {
			memBuf1.SetString(pszReconvSrc + ptSelect.x, nSelectedLen);  
			ShiftJis::UnicodeToSJIS(memBuf1, memBuf2._GetMemory());
			dwCompStrLen = memBuf2._GetMemory()->GetRawLength();					// comp文字列長。文字単位。
		}else if (nInsertCompLen > 0) {
			// nSelectedLen と nInsertCompLen が両方指定されることはないはず
			const ACHAR* pComp = to_achar(szComposition);
			pszInsBuffer = pComp;
			dwInsByteCount = strlen(pComp);
			dwCompStrLen = dwInsByteCount;
		}else {
			dwCompStrLen = 0;
		}
		
		// 考慮文字列すべて
		memBuf1.SetString(pszReconvSrc , nReconvLen);
		ShiftJis::UnicodeToSJIS(memBuf1, memBuf2._GetMemory());
		
		dwReconvTextLen    = memBuf2._GetMemory()->GetRawLength();				// reconv文字列長。文字単位。
		dwReconvTextInsLen = dwReconvTextLen + dwInsByteCount;						// reconv文字列長。文字単位。
		cbReconvLenWithNull = memBuf2._GetMemory()->GetRawLength() + dwInsByteCount + sizeof(char);		// reconvデータ長。バイト単位。
		
		pszReconv = reinterpret_cast<const void*>(memBuf2._GetMemory()->GetRawPtr());	// reconv文字列へのポインタ
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        構造体設定                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if (pReconv) {
		// 再変換構造体の設定
		DWORD dwOrgSize = pReconv->dwSize;
		// 2010.03.17 Moca dwSizeはpReconvを用意する側(IME等)が設定
		//     のはずなのに Win XP+IME2002+TSF では dwSizeが0で送られてくる
		if (dwOrgSize != 0 && dwOrgSize < sizeof(*pReconv) + cbReconvLenWithNull) {
			// バッファ不足
			szComposition[0] = _T('\0');
			return 0;
		}else if (dwOrgSize == 0) {
			pReconv->dwSize = sizeof(*pReconv) + cbReconvLenWithNull;
		}
		pReconv->dwVersion         = 0;
		pReconv->dwStrLen          = dwReconvTextInsLen;	// 文字単位
		pReconv->dwStrOffset       = sizeof(*pReconv) ;
		pReconv->dwCompStrLen      = dwCompStrLen;		// 文字単位
		pReconv->dwCompStrOffset   = dwCompStrOffset;	// バイト単位
		pReconv->dwTargetStrLen    = dwCompStrLen;		// 文字単位
		pReconv->dwTargetStrOffset = dwCompStrOffset;	// バイト単位
		
		// 2004.01.28 Moca ヌル終端の修正
		if (bUnicode) {
			WCHAR* p = (WCHAR*)(pReconv + 1);
			if (dwInsByteCount) {
				// カーソル位置に、入力中IMEデータを挿入
				CHAR* pb = (CHAR*)p;
				CopyMemory(pb, pszReconv, dwCompStrOffset);
				pb += dwCompStrOffset;
				CopyMemory(pb, pszInsBuffer, dwInsByteCount);
				pb += dwInsByteCount;
				CopyMemory(pb, ((char*)pszReconv) + dwCompStrOffset,
					dwReconvTextLen*sizeof(wchar_t) - dwCompStrOffset);
			}else {
				CopyMemory(p, pszReconv, cbReconvLenWithNull - sizeof(wchar_t));
			}
			// \0があると応答なしになることがある
			for (DWORD i=0; i<dwReconvTextInsLen; ++i) {
				if (p[i] == 0) {
					p[i] = L' ';
				}
			}
			p[dwReconvTextInsLen] = L'\0';
		}else {
			ACHAR* p = (ACHAR*)(pReconv + 1);
			if (dwInsByteCount) {
				CHAR* pb = p;
				CopyMemory(p, pszReconv, dwCompStrOffset);
				pb += dwCompStrOffset;
				CopyMemory(pb, pszInsBuffer, dwInsByteCount);
				pb += dwInsByteCount;
				CopyMemory(pb, ((char*)pszReconv) + dwCompStrOffset,
					dwReconvTextLen - dwCompStrOffset);
			}else {
				CopyMemory(p, pszReconv, cbReconvLenWithNull - sizeof(char));
			}
			// \0があると応答なしになることがある
			for (DWORD i=0; i<dwReconvTextInsLen; ++i) {
				if (p[i] == 0) {
					p[i] = ' ';
				}
			}
			p[dwReconvTextInsLen] = '\0';
		}
	}
	
	if (!bDocumentFeed) {
		// 再変換情報の保存
		nLastReconvIndex = nReconvIndex;
		nLastReconvLine  = ptSelect.y;
	}
	if (bDocumentFeed && pReconv) {
		szComposition[0] = _T('\0');
	}
	return sizeof(RECONVERTSTRING) + cbReconvLenWithNull;
}

// 再変換用 エディタ上の選択範囲を変更する 2002.04.09 minfu
LRESULT EditView::SetSelectionFromReonvert(
	const RECONVERTSTRING* pReconv,
	bool bUnicode
	)
{
	// 再変換情報が保存されているか
	if ((nLastReconvIndex < 0) || (nLastReconvLine < 0))
		return 0;

	if (GetSelectionInfo().IsTextSelected()) 
		GetSelectionInfo().DisableSelectArea(true);

	if (pReconv->dwVersion != 0) {
		return 0;
	}
	
	DWORD dwOffset, dwLen;

	// UNICODE→UNICODE
	if (bUnicode) {
		dwOffset = pReconv->dwCompStrOffset/sizeof(WCHAR);	// 0またはデータ長。バイト単位。→文字単位
		dwLen    = pReconv->dwCompStrLen;					// 0または文字列長。文字単位。
	}else {
	// ANSI→UNICODE
		NativeW	memBuf;

		// 考慮文字列の開始から対象文字列の開始まで
		if (pReconv->dwCompStrOffset > 0) {
			if (pReconv->dwSize < (pReconv->dwStrOffset + pReconv->dwCompStrOffset)) {
				return 0;
			}
			// 2010.03.17 sizeof(pReconv)+1ではなくdwStrOffsetを利用するように
			const char* p = ((const char*)(pReconv)) + pReconv->dwStrOffset;
			memBuf._GetMemory()->SetRawData(p, pReconv->dwCompStrOffset );
			ShiftJis::SJISToUnicode(*(memBuf._GetMemory()), &memBuf);
			dwOffset = memBuf.GetStringLength();
		}else {
			dwOffset = 0;
		}

		// 対象文字列の開始から対象文字列の終了まで
		if (pReconv->dwCompStrLen > 0) {
			if (pReconv->dwSize <
					pReconv->dwStrOffset + pReconv->dwCompStrOffset + pReconv->dwCompStrLen*sizeof(char)
			) {
				return 0;
			}
			// 2010.03.17 sizeof(pReconv)+1ではなくdwStrOffsetを利用するように
			const char* p= ((const char*)pReconv) + pReconv->dwStrOffset;
			memBuf._GetMemory()->SetRawData(p + pReconv->dwCompStrOffset, pReconv->dwCompStrLen);
			ShiftJis::SJISToUnicode(*(memBuf._GetMemory()), &memBuf);
			dwLen = memBuf.GetStringLength();
		}else {
			dwLen = 0;
		}
	}
	
	// 選択開始の位置を取得
	pEditDoc->layoutMgr.LogicToLayout(
		LogicPoint(nLastReconvIndex + dwOffset, nLastReconvLine),
		GetSelectionInfo().select.GetFromPointer()
	);

	// 選択終了の位置を取得
	pEditDoc->layoutMgr.LogicToLayout(
		LogicPoint(nLastReconvIndex + dwOffset + dwLen, nLastReconvLine),
		GetSelectionInfo().select.GetToPointer()
	);

	// 単語の先頭にカーソルを移動
	GetCaret().MoveCursor(GetSelectionInfo().select.GetFrom(), true);

	// 選択範囲再描画 
	GetSelectionInfo().DrawSelectArea();

	// 再変換情報の破棄
	nLastReconvIndex = -1;
	nLastReconvLine  = -1;

	return 1;

}

