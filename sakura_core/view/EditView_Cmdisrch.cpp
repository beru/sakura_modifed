/*!	@file
	@brief EditViewクラスのインクリメンタルサーチ関連コマンド処理系関数群

	@author genta
	@date	2005/01/10 作成
*/
/*
	Copyright (C) 2004, isearch
	Copyright (C) 2005, genta, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "StdAfx.h"
#include "view/EditView.h"
#include "window/EditWnd.h"
#include "doc/EditDoc.h"
#include "doc/logic/DocLine.h"
#include "extmodule/Migemo.h"
#include "sakura_rc.h"

/*!
	コマンドコードの変換(ISearch時)及び
	インクリメンタルサーチモードを抜ける判定

	@return true: コマンド処理済み / false: コマンド処理継続

	@date 2004.09.14 isearch 新規作成
	@date 2005.01.10 genta 関数化, UNINDENT追加

	@note UNINDENTを通常文字として扱うのは，
		SHIFT+文字の後でSPACEを入力するようなケースで
		SHIFTの解放が遅れても文字が入らなくなることを防ぐため．
*/
void EditView::TranslateCommand_isearch(
	EFunctionCode&	nCommand,
	bool&			bRedraw,
	LPARAM&			lparam1,
	LPARAM&			lparam2,
	LPARAM&			lparam3,
	LPARAM&			lparam4
	)
{
	if (nISearchMode <= 0)
		return;

	switch (nCommand) {
	// これらの機能のとき、インクリメンタルサーチに入る
	case F_ISEARCH_NEXT:
	case F_ISEARCH_PREV:
	case F_ISEARCH_REGEXP_NEXT:
	case F_ISEARCH_REGEXP_PREV:
	case F_ISEARCH_MIGEMO_NEXT:
	case F_ISEARCH_MIGEMO_PREV:
		break;

	// 以下の機能のとき、インクリメンタルサーチ中は検索文字入力として処理
	case F_WCHAR:
	case F_IME_CHAR:
		nCommand = F_ISEARCH_ADD_CHAR;
		break;
	case F_INSTEXT_W:
		nCommand = F_ISEARCH_ADD_STR;
		break;

	case F_INDENT_TAB:	// TABはインデントではなく単なるTAB文字と見なす
	case F_UNINDENT_TAB:	// genta追加
		nCommand = F_ISEARCH_ADD_CHAR;
		lparam1 = '\t';
		break;
	case F_INDENT_SPACE:	// スペースはインデントではなく単なるTAB文字と見なす
	case F_UNINDENT_SPACE:	// genta追加
		nCommand = F_ISEARCH_ADD_CHAR;
		lparam1 = ' ';
		break;
	case F_DELETE_BACK:
		nCommand = F_ISEARCH_DEL_BACK;
		break;

	default:
		// 上記以外のコマンドの場合はインクリメンタルサーチを抜ける
		ISearchExit();
	}
}

/*!
	ISearch コマンド処理

	@date 2005.01.10 genta 各コマンドに入っていた処理を1カ所に移動
*/
bool EditView::ProcessCommand_isearch(
	int	nCommand,
	bool	bRedraw,
	LPARAM	lparam1,
	LPARAM	lparam2,
	LPARAM	lparam3,
	LPARAM	lparam4
	)
{
	switch (nCommand) {
	// 検索文字列の変更操作
	case F_ISEARCH_ADD_CHAR:
		ISearchExec((DWORD)lparam1);
		return true;
	
	case F_ISEARCH_DEL_BACK:
		ISearchBack();
		return true;

	case F_ISEARCH_ADD_STR:
		ISearchExec((LPCWSTR)lparam1);
		return true;

	// 検索モードへの移行
	case F_ISEARCH_NEXT:
		ISearchEnter(1, SearchDirection::Forward);		// 前方インクリメンタルサーチ // 2004.10.13 isearch
		return true;
	case F_ISEARCH_PREV:
		ISearchEnter(1, SearchDirection::Backward);	// 後方インクリメンタルサーチ // 2004.10.13 isearch
		return true;
	case F_ISEARCH_REGEXP_NEXT:
		ISearchEnter(2, SearchDirection::Forward);		// 前方正規表現インクリメンタルサーチ  // 2004.10.13 isearch
		return true;
	case F_ISEARCH_REGEXP_PREV:
		ISearchEnter(2, SearchDirection::Backward);	// 後方正規表現インクリメンタルサーチ  // 2004.10.13 isearch
		return true;
	case F_ISEARCH_MIGEMO_NEXT:
		ISearchEnter(3, SearchDirection::Forward);		// 前方MIGEMOインクリメンタルサーチ    // 2004.10.13 isearch
		return true;
	case F_ISEARCH_MIGEMO_PREV:
		ISearchEnter(3, SearchDirection::Backward);	// 後方MIGEMOインクリメンタルサーチ    // 2004.10.13 isearch
		return true;
	}
	return false;
}

/*!
	インクリメンタルサーチモードに入る

	@param mode [in] 検索方法 1:通常, 2:正規表現, 3:MIGEMO
	@param direction [in] 検索方向 0:後方(上方), 1:前方(下方)

	@author isearch
	@date 2011.12.15 Moca sCurSearchOption/sSearchOptionと同期をとる
	@date 2012.10.11 novice sCurSearchOption/sSearchOptionの同期をswitchの前に変更
	@date 2012.10.11 novice MIGEMOの処理をcase内に移動
*/
void EditView::ISearchEnter(int mode, SearchDirection direction)
{

	if (nISearchMode == mode) {
		// 再実行
		nISearchDirection =  direction;
		
		if (bISearchFirst) {
			bISearchFirst = false;
		}
		// ちょっと修正
		ISearchExec(true);

	}else {
		auto& selInfo = GetSelectionInfo();
		// インクリメンタルサーチモードに入るだけ.		
		// 選択範囲の解除
		if (selInfo.IsTextSelected())	
			selInfo.DisableSelectArea(true);

		curSearchOption = GetDllShareData().common.search.searchOption;
		switch (mode) {
		case 1: // 通常インクリメンタルサーチ
			curSearchOption.bRegularExp = false;
			curSearchOption.bLoHiCase = false;
			curSearchOption.bWordOnly = false;
			//SendStatusMessage(_T("I-Search: "));
			break;
		case 2: // 正規表現インクリメンタルサーチ
			if (!curRegexp.IsAvailable()) {
				WarningBeep();
				SendStatusMessage(LS(STR_EDITVWISRCH_REGEX));
				return;
			}
			curSearchOption.bRegularExp = true;
			curSearchOption.bLoHiCase = false;
			//SendStatusMessage(_T("[RegExp] I-Search: "));
			break;
		case 3: // MIGEMOインクリメンタルサーチ
			if (!curRegexp.IsAvailable()) {
				WarningBeep();
				SendStatusMessage(LS(STR_EDITVWISRCH_REGEX));
				return;
			}
			if (!pMigemo) {
				pMigemo = &Migemo::getInstance();
				pMigemo->InitDll();
			}
			// migemo dll チェック
			//	Jan. 10, 2005 genta 設定変更で使えるようになっている
			//	可能性があるので，使用可能でなければ一応初期化を試みる
			if (!pMigemo->IsAvailable() && pMigemo->InitDll() != InitDllResultType::Success) {
				WarningBeep();
				SendStatusMessage(LS(STR_EDITVWISRCH_MIGEGO1));
				return;
			}
			pMigemo->migemo_load_all();
			if (pMigemo->migemo_is_enable()) {
				curSearchOption.bRegularExp = true;
				curSearchOption.bLoHiCase = false;
				//SendStatusMessage(_T("[MIGEMO] I-Search: "));
			}else {
				WarningBeep();
				SendStatusMessage(LS(STR_EDITVWISRCH_MIGEGO2));
				return;
			}
			break;
		}
		
		//	Feb. 04, 2005 genta	検索開始位置を記録
		//	インクリメンタルサーチ間でモードを切り替える場合には開始と見なさない
		if (nISearchMode == 0) {
			ptSrchStartPos_PHY = GetCaret().GetCaretLogicPos();
		}
		
		bCurSrchKeyMark = false;
		nISearchDirection = direction;
		nISearchMode = mode;
		
		nISearchHistoryCount = 0;
		searchHistory[nISearchHistoryCount].Set(GetCaret().GetCaretLayoutPos());

		Redraw();
		
		NativeT msg;
		ISearchSetStatusMsg(&msg);
		SendStatusMessage(msg.GetStringPtr());
		
		bISearchWrap = false;
		bISearchFirst = true;
	}

	// マウスカーソル変更
	if (direction == SearchDirection::Forward) {
		::SetCursor(::LoadCursor(G_AppInstance(), MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_F)));
	}else {
		::SetCursor(::LoadCursor(G_AppInstance(), MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_B)));
	}
}

// インクリメンタルサーチモードから抜ける
void EditView::ISearchExit()
{
	// シーケンスを上書きして現在の検索キーを維持する
	if (strCurSearchKey.size() < _MAX_PATH) {
		SearchKeywordManager().AddToSearchKeys(strCurSearchKey.c_str());
	}
	nCurSearchKeySequence = GetDllShareData().common.search.nSearchKeySequence;
	GetDllShareData().common.search.searchOption = curSearchOption;
	editWnd.toolbar.AcceptSharedSearchKey();
	nISearchDirection = SearchDirection::Backward;
	nISearchMode = 0;
	
	if (nISearchHistoryCount == 0) {
		strCurSearchKey.clear();
	}

	// マウスカーソルを元に戻す
	POINT point1;
	GetCursorPos(&point1);
	OnMOUSEMOVE(0, point1.x, point1.y);

	// ステータス表示エリアをクリア
	SendStatusMessage(_T(""));

}

/*!
	@brief インクリメンタルサーチの実行(1文字追加)
	
	@param wChar [in] 追加する文字 (1byte or 2byte)
*/
void EditView::ISearchExec(DWORD wChar)
{
	// 特殊文字は処理しない
	switch (wChar) {
	case L'\r':
	case L'\n':
		ISearchExit();
		return;
	//case '\t':
	//	break;
	}
	
	if (bISearchFirst) {
		bISearchFirst = false;
		strCurSearchKey.clear();
	}

	if (wChar <= 0xffff) {
		strCurSearchKey.append(1, (WCHAR)wChar);
	}else {
		strCurSearchKey.append(1, (WCHAR)(wChar>>16));
		strCurSearchKey.append(1, (WCHAR)wChar);
	}
	
	ISearchExec(false);
	return ;
}

/*!
	@brief インクリメンタルサーチの実行(文字列追加)
	
	@param pszText [in] 追加する文字列
*/
void EditView::ISearchExec(LPCWSTR pszText)
{
	// 一文字ずつ分解して実行

	const WCHAR* p = pszText;
	DWORD c;
	while (*p != L'\0') {
		if (IsUtf16SurrogHi(*p) && IsUtf16SurrogLow(*(p + 1))) {
			c = (((WORD)*p) << 16) | ((WORD)*(p + 1));
			++p;
		}else {
			c = *p;
		}
		ISearchExec(c);
		++p;
	}
	return ;
}

/*!
	@brief インクリメンタルサーチの実行

	@param bNext [in] true:次の候補を検索, false:現在のカーソル位置のまま検索
*/
void EditView::ISearchExec(bool bNext) 
{
	// 検索を実行する.

	if ((strCurSearchKey.size() == 0) || (nISearchMode == 0)) {
		// ステータスの表示
		NativeT msg;
		ISearchSetStatusMsg(&msg);
		SendStatusMessage(msg.GetStringPtr());
		return ;
	}
	
	ISearchWordMake();
	
	int nLine(0);
	int nIdx1(0);
	
	if (bNext && bISearchWrap) {
		switch (nISearchDirection) {
		case 1:
			nLine = 0;
			nIdx1 = 0;
			break;
		case 0:
			// 最後から検索
			int nLineP;
			int nIdxP;
			auto& docLineMgr = pEditDoc->docLineMgr;
			nLineP =  docLineMgr.GetLineCount() - 1;
			DocLine* pDocLine = docLineMgr.GetLine(nLineP);
			nIdxP = pDocLine->GetLengthWithEOL() -1;
			Point ptTmp = pEditDoc->layoutMgr.LogicToLayout(Point(nIdxP, nLineP));
			nIdx1 = ptTmp.x;
			nLine = ptTmp.y;
		}
	}else if (GetSelectionInfo().IsTextSelected()) {
		auto& select = GetSelectionInfo().select;
		switch ((int)nISearchDirection * 2 + (bNext ? 1: 0)) {
		case 2 : // 前方検索で現在位置から検索のとき
		case 1 : // 後方検索で次を検索のとき
			// 選択範囲の先頭を検索開始位置に
			nLine = select.GetFrom().y;
			nIdx1 = select.GetFrom().x;
			break;
		case 0 : // 前方検索で次を検索
		case 3 : // 後方検索で現在位置から検索
			// 選択範囲の後ろから
			nLine = select.GetTo().y;
			nIdx1 = select.GetTo().x;
			break;
		}
	}else {
		auto& pos = GetCaret().GetCaretLayoutPos();
		nLine = pos.y;
		nIdx1  = pos.x;
	}

	// 桁位置からindexに変換
	Layout* pLayout = pEditDoc->layoutMgr.SearchLineByLayoutY(nLine);
	int nIdx = LineColumnToIndex(pLayout, nIdx1);

	nISearchHistoryCount ++ ;

	NativeT msg;
	ISearchSetStatusMsg(&msg);

	if (nISearchHistoryCount >= 256) {
		nISearchHistoryCount = 156;
		for (int i=100; i<=255; ++i) {
			bISearchFlagHistory[i-100] = bISearchFlagHistory[i];
			searchHistory[i-100] = searchHistory[i];
		}
	}
	bISearchFlagHistory[nISearchHistoryCount] = bNext;

	Range matchRange;

	int nSearchResult = pEditDoc->layoutMgr.SearchWord(
		nLine,						// 検索開始レイアウト行
		nIdx,						// 検索開始データ位置
		nISearchDirection,		// 0==前方検索 1==後方検索
		&matchRange,				// マッチレイアウト範囲
		searchPattern
	);
	if (nSearchResult == 0) {
		// 検索結果がない
		msg.AppendString(LS(STR_EDITVWISRCH_NOMATCH));
		SendStatusMessage(msg.GetStringPtr());
		
		if (bNext) {
			bISearchWrap = true;
		}
		if (GetSelectionInfo().IsTextSelected()) {
			searchHistory[nISearchHistoryCount] = GetSelectionInfo().select;
		}else {
			searchHistory[nISearchHistoryCount].Set(GetCaret().GetCaretLayoutPos());
		}
	}else {
		// 検索結果あり
		// キャレット移動
		GetCaret().MoveCursor(matchRange.GetFrom(), true, _CARETMARGINRATE / 3);
		
		//	2005.06.24 Moca
		GetSelectionInfo().SetSelectArea(matchRange);

		bISearchWrap = false;
		searchHistory[nISearchHistoryCount] = matchRange;
	}

	bCurSrchKeyMark = true;

	Redraw();	
	SendStatusMessage(msg.GetStringPtr());
	return ;
}

// バックスペースを押されたときの処理
void EditView::ISearchBack(void)
{
	if (nISearchHistoryCount == 0) {
		return;
	}
	
	if (nISearchHistoryCount == 1) {
		bCurSrchKeyMark = false;
		bISearchFirst = true;
	}else if (!bISearchFlagHistory[nISearchHistoryCount]) {
		// 検索文字をへらす
		size_t l = strCurSearchKey.size();
		if (l > 0) {
			// 最後の文字の一つ前
			wchar_t* p = (wchar_t*)NativeW::GetCharPrev(strCurSearchKey.c_str(), l, &strCurSearchKey.c_str()[l]);
			size_t new_len = p - strCurSearchKey.c_str();
			strCurSearchKey.resize(new_len);
			//szCurSrchKey[l-1] = '\0';

			if (new_len > 0) 
				ISearchWordMake();
			else
				bCurSrchKeyMark = false;

		}else {
			WarningBeep();
		}
	}
	nISearchHistoryCount --;

	Range range = searchHistory[nISearchHistoryCount];

	if (nISearchHistoryCount == 0) {
		GetSelectionInfo().DisableSelectArea(true);
		range.SetToX(range.GetFrom().x);
	}

	GetCaret().MoveCursor(range.GetFrom(), true, _CARETMARGINRATE / 3);
	if (nISearchHistoryCount != 0) {
		//	2005.06.24 Moca
		GetSelectionInfo().SetSelectArea(range);
	}

	Redraw();

	// ステータス表示
	NativeT msg;
	ISearchSetStatusMsg(&msg);
	SendStatusMessage(msg.GetStringPtr());
	
}

// 入力文字から、検索文字を生成する。
void EditView::ISearchWordMake(void)
{
	switch (nISearchMode) {
	case 1: // 通常インクリメンタルサーチ
	case 2: // 正規表現インクリメンタルサーチ
		searchPattern.SetPattern(this->GetHwnd(), strCurSearchKey.c_str(), strCurSearchKey.size(), curSearchOption, &curRegexp);
		break;
	case 3: // MIGEMOインクリメンタルサーチ
		{
			// migemoで捜す
			std::wstring strMigemoWord = pMigemo->migemo_query_w(strCurSearchKey.c_str());
			
			// 検索パターンのコンパイル
			const wchar_t* p = strMigemoWord.c_str();
			searchPattern.SetPattern(this->GetHwnd(), p, strMigemoWord.size(), curSearchOption, &curRegexp);

		}
		break;
	}
}

/*!	@brief ISearchメッセージ構築

	現在のサーチモード及び検索中文字列から
	メッセージエリアに表示する文字列を構築する
	
	@param msg [out] メッセージバッファ
	
	@author isearch
	@date 2004/10/13
	@date 2005.01.13 genta 文字列修正
*/
void EditView::ISearchSetStatusMsg(NativeT* msg) const
{

	switch (nISearchMode) {
	case 1 :
		msg->SetString(_T("I-Search"));
		break;
	case 2 :
		msg->SetString(_T("[RegExp] I-Search"));
		break;
	case 3 :
		msg->SetString(_T("[Migemo] I-Search"));
		break;
	default:
		msg->SetString(_T(""));
		return;
	}
	if ((int)nISearchDirection == 0) {
		msg->AppendStringLiteral(_T(" Backward: "));
	}else {
		msg->AppendStringLiteral(_T(": "));
	}

	if ((int)nISearchHistoryCount > 0)
		msg->AppendString(to_tchar(strCurSearchKey.c_str()));
}

/*!
	ISearch状態をツールバーに反映させる．
	
	@sa CEditWnd::IsFuncChecked()

	@param nCommand [in] 調べたいコマンドのID
	@return true:チェック有り / false: チェック無し
	
	@date 2005.01.10 genta 新規作成
*/
bool EditView::IsISearchEnabled(int nCommand) const
{
	switch (nCommand) {
	case F_ISEARCH_NEXT:
		return nISearchMode == 1 && nISearchDirection == SearchDirection::Forward;
	case F_ISEARCH_PREV:
		return nISearchMode == 1 && nISearchDirection == SearchDirection::Backward;
	case F_ISEARCH_REGEXP_NEXT:
		return nISearchMode == 2 && nISearchDirection == SearchDirection::Forward;
	case F_ISEARCH_REGEXP_PREV:
		return nISearchMode == 2 && nISearchDirection == SearchDirection::Backward;
	case F_ISEARCH_MIGEMO_NEXT:
		return nISearchMode == 3 && nISearchDirection == SearchDirection::Forward;
	case F_ISEARCH_MIGEMO_PREV:
		return nISearchMode == 3 && nISearchDirection == SearchDirection::Backward;
	}
	return false;
}

