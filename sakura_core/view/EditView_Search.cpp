/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include <limits.h>
#include "EditView.h"
#include "window/EditWnd.h"
#include "parse/WordParse.h"
#include "util/string_ex2.h"

const int STRNCMP_MAX = 100;	// MAXキーワード長：strnicmp文字列比較最大値(EditView::KeySearchCore) 	// 2006.04.10 fon

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           検索                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! キーワード辞書検索の前提条件チェックと、検索

	@date 2006.04.10 fon OnTimer, CreatePopUpMenu_Rから分離
*/
BOOL EditView::KeywordHelpSearchDict(LID_SKH nID, POINT* po, RECT* rc)
{
	NativeW memCurText;
	// キーワードヘルプを使用するか？
	if (!m_pTypeData->bUseKeywordHelp)	// キーワードヘルプ機能を使用する	// 2006.04.10 fon
		goto end_of_search;
	// フォーカスがあるか？
	if (!GetCaret().ExistCaretFocus()) 
		goto end_of_search;
	// ウィンドウ内にマウスカーソルがあるか？
	GetCursorPos(po);
	GetWindowRect(GetHwnd(), rc);
	if (!PtInRect(rc, *po))
		goto end_of_search;
	switch (nID) {
	case LID_SKH_ONTIMER:
		// 右コメントの１～３でない場合
		if (!(1
				&& !m_bInMenuLoop							// １．メニュー モーダル ループに入っていない
				&& m_dwTipTimer != 0						// ２．辞書Tipを表示していない
				&& 300 < ::GetTickCount() - m_dwTipTimer	// ３．一定時間以上、マウスが固定されている
			)
		)
			goto end_of_search;
		break;
	case LID_SKH_POPUPMENU_R:
		if (!(1
				&& !m_bInMenuLoop							// １．メニュー モーダル ループに入っていない
			//	&& m_dwTipTimer != 0			&&			// ２．辞書Tipを表示していない
			//	&& 1000 < ::GetTickCount() - m_dwTipTimer	// ３．一定時間以上、マウスが固定されている
			)
		)
			goto end_of_search;
		break;
	default:
		PleaseReportToAuthor(NULL, _T("EditView::KeywordHelpSearchDict\nnID=%d"), (int)nID);
	}
	// 選択範囲のデータを取得(複数行選択の場合は先頭の行のみ)
	if (GetSelectedDataOne(memCurText, STRNCMP_MAX + 1)) {
	// キャレット位置の単語を取得する処理		2006.03.24 fon
	}else if (GetDllShareData().common.search.bUseCaretKeyword) {
		if (!GetParser().GetCurrentWord(&memCurText))
			goto end_of_search;
	}else
		goto end_of_search;

	if (NativeW::IsEqual(memCurText, m_tipWnd.m_key)	// 既に検索済みか
		&& !m_tipWnd.m_KeyWasHit							// 該当するキーがなかった
	) {
		goto end_of_search;
	}
	m_tipWnd.m_key = memCurText;

	// 検索実行
	if (!KeySearchCore(&m_tipWnd.m_key)) {
		goto end_of_search;
	}
	m_dwTipTimer = 0;		// 辞書Tipを表示している
	m_poTipCurPos = *po;	// 現在のマウスカーソル位置
	return TRUE;			// ここまで来ていればヒット・ワード

	// キーワードヘルプ表示処理終了
end_of_search:
	return FALSE;
}

/*! キーワード辞書検索処理メイン

	@date 2006.04.10 fon KeywordHelpSearchDictから分離
*/
bool EditView::KeySearchCore(const NativeW* pMemCurText)
{
	NativeW*	pMemRefKey;
	int			nCmpLen = STRNCMP_MAX; // 2006.04.10 fon
	int			nLine; // 2006.04.10 fon
	
	m_tipWnd.m_info.SetString(_T(""));	// tooltipバッファ初期化
	// 1行目にキーワード表示の場合
	if (m_pTypeData->bUseKeyHelpKeyDisp) {	// キーワードも表示する	// 2006.04.10 fon
		m_tipWnd.m_info.AppendString(_T("["));
		m_tipWnd.m_info.AppendString(pMemCurText->GetStringT());
		m_tipWnd.m_info.AppendString(_T("]"));
	}
	// 途中まで一致を使う場合
	if (m_pTypeData->bUseKeyHelpPrefix)
		nCmpLen = wcslen(pMemCurText->GetStringPtr());	// 2006.04.10 fon
	m_tipWnd.m_KeyWasHit = false;
	for (int i=0; i<m_pTypeData->nKeyHelpNum; ++i) {	// 最大数：MAX_KEYHELP_FILE
		auto& keyHelpInfo = m_pTypeData->keyHelpArr[i];
		if (keyHelpInfo.bUse) {
			// 2006.04.10 fon (nCmpLen, pMemRefKey,nSearchLine)引数を追加
			NativeW* pMemRefText;
			int nSearchResult = m_dicMgr.DicMgr::Search(
				pMemCurText->GetStringPtr(),
				nCmpLen,
				&pMemRefKey,
				&pMemRefText,
				keyHelpInfo.szPath,
				&nLine
			);
			if (nSearchResult) {
				// 該当するキーがある
				LPWSTR pszWork = pMemRefText->GetStringPtr();
				// 有効になっている辞書を全部なめて、ヒットの都度説明の継ぎ増し
				if (m_pTypeData->bUseKeyHelpAllSearch) {	// ヒットした次の辞書も検索	// 2006.04.10 fon
					// バッファに前のデータが詰まっていたらseparator挿入
					if (m_tipWnd.m_info.GetStringLength() != 0)
						m_tipWnd.m_info.AppendString(LS(STR_ERR_DLGEDITVW5));
					else
						m_tipWnd.m_info.AppendString(LS(STR_ERR_DLGEDITVW6));	// 先頭の場合
					// 辞書のパス挿入
					{
						TCHAR szFile[MAX_PATH];
						// 2013.05.08 表示するのはファイル名(拡張子なし)のみにする
						_tsplitpath(keyHelpInfo.szPath, NULL, NULL, szFile, NULL);
						m_tipWnd.m_info.AppendString(szFile);
					}
					m_tipWnd.m_info.AppendString(_T("\n"));
					// 前方一致でヒットした単語を挿入
					if (m_pTypeData->bUseKeyHelpPrefix) {	// 選択範囲で前方一致検索
						m_tipWnd.m_info.AppendString(pMemRefKey->GetStringT());
						m_tipWnd.m_info.AppendString(_T(" >>\n"));
					}// 調査した「意味」を挿入
					m_tipWnd.m_info.AppendStringW(pszWork);
					delete pMemRefText;
					delete pMemRefKey;	// 2006.07.02 genta
					// タグジャンプ用の情報を残す
					if (!m_tipWnd.m_KeyWasHit) {
						m_tipWnd.m_nSearchDict = i;	// 辞書を開くとき最初にヒットした辞書を開く
						m_tipWnd.m_nSearchLine = nLine;
						m_tipWnd.m_KeyWasHit = true;
					}
				}else {	// 最初のヒット項目のみ返す場合
					// キーワードが入っていたらseparator挿入
					if (m_tipWnd.m_info.GetStringLength() != 0)
						m_tipWnd.m_info.AppendString(_T("\n--------------------\n"));
					
					// 前方一致でヒットした単語を挿入
					if (m_pTypeData->bUseKeyHelpPrefix) {	// 選択範囲で前方一致検索
						m_tipWnd.m_info.AppendString(pMemRefKey->GetStringT());
						m_tipWnd.m_info.AppendString(_T(" >>\n"));
					}
					
					// 調査した「意味」を挿入
					m_tipWnd.m_info.AppendStringW(pszWork);
					delete pMemRefText;
					delete pMemRefKey;	// 2006.07.02 genta
					// タグジャンプ用の情報を残す
					m_tipWnd.m_nSearchDict = i;
					m_tipWnd.m_nSearchLine = nLine;
					m_tipWnd.m_KeyWasHit = true;
					return true;
				}
			}
		}
	}
	if (m_tipWnd.m_KeyWasHit) {
		return true;
	}
	// 該当するキーがなかった場合
	return false;
}

bool EditView::MiniMapCursorLineTip(POINT* po, RECT* rc, bool* pbHide)
{
	*pbHide = true;
	if (!m_bMiniMap) {
		return false;
	}
	// ウィンドウ内にマウスカーソルがあるか？
	GetCursorPos(po);
	GetWindowRect(GetHwnd(), rc);
	rc->right -= ::GetSystemMetrics(SM_CXVSCROLL);
	if (!PtInRect(rc, *po)) {
		return false;
	}
	if (!( !m_bInMenuLoop &&					// １．メニュー モーダル ループに入っていない
		300 < ::GetTickCount() - m_dwTipTimer	// ２．一定時間以上、マウスが固定されている
	)) {
		return false;
	}
	if (WindowFromPoint(*po) != GetHwnd()) {
		return false;
	}

	Point ptClient(*po);
	ScreenToClient(GetHwnd(), &ptClient);
	LayoutPoint ptNew;
	GetTextArea().ClientToLayout(ptClient, &ptNew);
	// 同じ行ならなにもしない
	if (m_dwTipTimer == 0 && m_tipWnd.m_nSearchLine == (Int)ptNew.y) {
		*pbHide = false; // 表示継続
		return false;
	}
	NativeW memCurText;
	LayoutYInt nTipBeginLine = ptNew.y;
	LayoutYInt nTipEndLine = ptNew.y + LayoutYInt(4);
	for (LayoutYInt nCurLine=nTipBeginLine; nCurLine<nTipEndLine; ++nCurLine) {
		const Layout* pLayout = NULL;
		if (0 <= nCurLine) {
			pLayout = GetDocument()->m_layoutMgr.SearchLineByLayoutY( nCurLine );
		}
		if (pLayout) {
			NativeW memCurLine;
			{
				LogicInt nLineLen = pLayout->GetLengthWithoutEOL();
				const wchar_t* pszData = pLayout->GetPtr();
				int nLimitLength = 80;
				int pre = 0;
				int i = 0;
				int k = 0;
				int charSize = NativeW::GetSizeOfChar( pszData, nLineLen, i );
				int charWidth = t_max(1, (int)(Int)NativeW::GetKetaOfChar( pszData, nLineLen, i ));
				int charType = 0;
				// 連続する"\t" " " を " "1つにする
				// 左からnLimitLengthまでの幅を切り取り
				while (i + charSize <= (Int)nLineLen && k + charWidth <= nLimitLength) {
					if (pszData[i] == L'\t' || pszData[i] == L' ') {
						if (charType == 0) {
							memCurLine.AppendString( pszData + pre , i - pre );
							memCurLine.AppendString( L" " );
							charType = 1;
						}
						pre = i + charSize;
						++k;
					}else {
						k += charWidth;
						charType = 0;
					}
					i += charSize;
					charSize = NativeW::GetSizeOfChar( pszData, nLineLen, i );
					charWidth = t_max(1, (int)(Int)NativeW::GetKetaOfChar( pszData, nLineLen, i ));
				}
				memCurLine.AppendString( pszData + pre , i - pre );
			}
			if (nTipBeginLine != nCurLine) {
				memCurText.AppendString( L"\n" );
			}
			memCurLine.Replace( L"\\", L"\\\\" );
			memCurText.AppendNativeData( memCurLine );
		}
	}
	if (memCurText.GetStringLength() <= 0) {
		return false;
	}
	m_tipWnd.m_key = memCurText;
	m_tipWnd.m_info = memCurText.GetStringT();
	m_tipWnd.m_nSearchLine = (Int)ptNew.y;
	m_dwTipTimer = 0;		// 辞書Tipを表示している
	m_poTipCurPos = *po;	// 現在のマウスカーソル位置
	return true;			// ここまで来ていればヒット・ワード
}

// 現在カーソル位置単語または選択範囲より検索等のキーを取得
void EditView::GetCurrentTextForSearch(NativeW& memCurText, bool bStripMaxPath /* = true */, bool bTrimSpaceTab /* = false */)
{
	NativeW memTopic = L"";

	memCurText.SetString(L"");
	if (GetSelectionInfo().IsTextSelected()) {	// テキストが選択されているか
		// 選択範囲のデータを取得
		if (GetSelectedDataOne(memCurText, INT_MAX)) {
			// 検索文字列を現在位置の単語で初期化
			if (bStripMaxPath) {
				LimitStringLengthW(memCurText.GetStringPtr(), memCurText.GetStringLength(), _MAX_PATH - 1, memTopic);
			}else {
				memTopic = memCurText;
			}
		}
	}else {
		LogicInt nLineLen;
		const Layout* pLayout;
		const wchar_t* pLine = m_pEditDoc->m_layoutMgr.GetLineStr(GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pLayout);
		if (pLine) {
			// 指定された桁に対応する行のデータ内の位置を調べる
			LogicInt nIdx = LineColumnToIndex(pLayout, GetCaret().GetCaretLayoutPos().GetX2());

			// 現在位置の単語の範囲を調べる
			LayoutRange range;
			bool bWhere = m_pEditDoc->m_layoutMgr.WhereCurrentWord(
				GetCaret().GetCaretLayoutPos().GetY2(),
				nIdx,
				&range,
				NULL,
				NULL
			);
			if (bWhere) {
				// 選択範囲の変更
				GetSelectionInfo().m_selectBgn = range;
				GetSelectionInfo().m_select    = range;

				// 選択範囲のデータを取得
				if (GetSelectedDataOne(memCurText, INT_MAX)) {
					// 検索文字列を現在位置の単語で初期化
					if (bStripMaxPath) {
						LimitStringLengthW(memCurText.GetStringPtr(), memCurText.GetStringLength(), _MAX_PATH - 1, memTopic);
					}else {
						memTopic = memCurText;
					}
				}
				// 現在の選択範囲を非選択状態に戻す
				GetSelectionInfo().DisableSelectArea(false);
			}
		}
	}

	wchar_t* pTopic2 = memTopic.GetStringPtr();
	if (bTrimSpaceTab) {
		// 前のスペース・タブを取り除く
		while (L'\0' != *pTopic2 && (' ' == *pTopic2 || '\t' == *pTopic2)) {
			++pTopic2;
		}
	}
	int nTopic2Len = (int)wcslen(pTopic2);
	// 検索文字列は改行まで
	bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
	int i;
	for (i=0; i<nTopic2Len; ++i) {
		if (WCODE::IsLineDelimiter(pTopic2[i], bExtEol)) {
			break;
		}
	}
	
	if (bTrimSpaceTab) {
		// 後ろのスペース・タブを取り除く
		int m = i - 1;
		while (0 <= m &&
		    (L' ' == pTopic2[m] || L'\t' == pTopic2[m])
		) {
			--m;
		}
		if (0 <= m) {
			i = m + 1;
		}
	}
	memCurText.SetString(pTopic2, i);
}


/*!	現在カーソル位置単語または選択範囲より検索等のキーを取得（ダイアログ用）
	@return 値を設定したか
	@date 2006.08.23 ryoji 新規作成
	@date 2014.07.01 Moca bGetHistory追加、戻り値をboolに変更
*/
bool EditView::GetCurrentTextForSearchDlg(NativeW& memCurText, bool bGetHistory)
{
	bool bStripMaxPath = false;
	memCurText.SetString(L"");
	
	if (GetSelectionInfo().IsTextSelected()) {	// テキストが選択されている
		GetCurrentTextForSearch(memCurText, bStripMaxPath);
	}else {	// テキストが選択されていない
		bool bGet = false;
		if (GetDllShareData().common.search.bCaretTextForSearch) {
			GetCurrentTextForSearch(memCurText, bStripMaxPath);	// カーソル位置単語を取得
			if (memCurText.GetStringLength() == 0 && bGetHistory) {
				bGet = true;
			}
		}else {
			bGet = true;
		}
		if (bGet) {
			if (1
				&& 0 < GetDllShareData().searchKeywords.searchKeys.size()
				&& m_nCurSearchKeySequence < GetDllShareData().common.search.nSearchKeySequence
			) {
				memCurText.SetString(GetDllShareData().searchKeywords.searchKeys[0]);	// 履歴からとってくる
				return true; // ""でもtrue	
			}else {
				memCurText.SetString(m_strCurSearchKey.c_str());
				return 0 <= m_nCurSearchKeySequence; // ""でもtrue.未設定のときはfalse

			}
		}
	}
	return 0 < memCurText.GetStringLength();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        描画用判定                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 現在位置が検索文字列に該当するか
// 2002.02.08 hor
// 正規表現で検索したときの速度改善のため、マッチ先頭位置を引数に追加
// Jun. 26, 2001 genta	正規表現ライブラリの差し替え
/*
	@retval 0
		(パターン検索時) 指定位置以降にマッチはない。
		(それ以外) 指定位置は検索文字列の始まりではない。
	@retval 1,2,3,...
		(パターン検索時) 指定位置以降にマッチが見つかった。
		(単語検索時) 指定位置が検索文字列に含まれる何番目の単語の始まりであるか。
		(それ以外) 指定位置が検索文字列の始まりだった。
*/
int EditView::IsSearchString(
	const StringRef&	str,
	/*
	const wchar_t*	pszData,
	CLogicInt		nDataLen,
	*/
	LogicInt		nPos,
	LogicInt*		pnSearchStart,
	LogicInt*		pnSearchEnd
	) const
{
	*pnSearchStart = nPos;	// 2002.02.08 hor

	if (m_curSearchOption.bRegularExp) {
		// 行頭ではない?
		// 行頭検索チェックは、CBregexpクラス内部で実施するので不要 2003.11.01 かろと

		/* 位置を0でMatchInfo呼び出すと、行頭文字検索時に、全て true　となり、
		** 画面全体が検索文字列扱いになる不具合修正
		** 対策として、行頭を MacthInfoに教えないといけないので、文字列の長さ・位置情報を与える形に変更
		** 2003.05.04 かろと
		*/
		if (m_curRegexp.Match(str.GetPtr(), str.GetLength(), nPos)) {
			*pnSearchStart = m_curRegexp.GetIndex();	// 2002.02.08 hor
			*pnSearchEnd = m_curRegexp.GetLastIndex();
			return 1;
		}else {
			return 0;
		}
	}else if (m_curSearchOption.bWordOnly) { // 単語検索
		// 指定位置の単語の範囲を調べる
		LogicInt posWordHead, posWordEnd;
		if (!WordParse::WhereCurrentWord_2(str.GetPtr(), LogicInt(str.GetLength()), nPos, &posWordHead, &posWordEnd, NULL, NULL)) {
			return 0; // 指定位置に単語が見つからなかった。
 		}
		if (nPos != posWordHead) {
			return 0; // 指定位置は単語の始まりではなかった。
		}
		const LogicInt wordLength = posWordEnd - posWordHead;
		const wchar_t* const pWordHead = str.GetPtr() + posWordHead;

		// 比較関数
		int (*const fcmp)(const wchar_t*, const wchar_t*, size_t) = m_curSearchOption.bLoHiCase ? wcsncmp : wcsnicmp;

		// 検索語を単語に分割しながら指定位置の単語と照合する。
		int wordIndex = 0;
		const wchar_t* const searchKeyEnd = m_strCurSearchKey.data() + m_strCurSearchKey.size();
		for (const wchar_t* p=m_strCurSearchKey.data(); p<searchKeyEnd; ) {
			LogicInt begin, end; // 検索語に含まれる単語?の位置。WhereCurrentWord_2()の仕様では空白文字列も単語に含まれる。
			if (1
				&& WordParse::WhereCurrentWord_2(p, LogicInt(searchKeyEnd - p), LogicInt(0), &begin, &end, NULL, NULL)
				&& begin == 0
				&& begin < end
			) {
				if (!WCODE::IsWordDelimiter(*p)) {
					++wordIndex;
					// p...(p + end) が検索語に含まれる wordIndex番目の単語。(wordIndexの最初は 1)
					if (wordLength == end && fcmp(p, pWordHead, wordLength) == 0) {
						*pnSearchStart = posWordHead;
						*pnSearchEnd = posWordEnd;
						return wordIndex;
					}
				}
				p += end;
			}else {
				p += NativeW::GetSizeOfChar(p, searchKeyEnd - p, 0);
			}
		}
		return 0; // 指定位置の単語と検索文字列に含まれる単語は一致しなかった。
	}else {
		const wchar_t* pHit = SearchAgent::SearchString(str.GetPtr(), str.GetLength(), nPos, m_searchPattern);
		if (pHit) {
			*pnSearchStart = pHit - str.GetPtr();
			*pnSearchEnd = *pnSearchStart + m_searchPattern.GetLen();
			return 1;
		}
		return 0; // この行はヒットしなかった
	}
	return 0;
}

