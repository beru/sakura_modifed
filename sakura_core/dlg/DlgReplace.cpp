/*!	@file
	@brief 置換ダイアログ
*/
#include "StdAfx.h"
#include "dlg/DlgReplace.h"
#include "view/EditView.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

// 置換 CDlgReplace.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//11900
	IDC_BUTTON_SEARCHNEXT,			HIDC_REP_BUTTON_SEARCHNEXT,			// 下検索
	IDC_BUTTON_SEARCHPREV,			HIDC_REP_BUTTON_SEARCHPREV,			// 上検索
	IDC_BUTTON_REPALCE,				HIDC_REP_BUTTON_REPALCE,			// 置換
	IDC_BUTTON_REPALCEALL,			HIDC_REP_BUTTON_REPALCEALL,			// 全置換
	IDCANCEL,						HIDCANCEL_REP,						// キャンセル
	IDC_BUTTON_HELP,				HIDC_REP_BUTTON_HELP,				// ヘルプ
	IDC_CHK_PASTE,					HIDC_REP_CHK_PASTE,					// クリップボードから貼り付け
	IDC_CHK_WORD,					HIDC_REP_CHK_WORD,					// 単語単位
	IDC_CHK_LOHICASE,				HIDC_REP_CHK_LOHICASE,				// 大文字小文字
	IDC_CHK_REGULAREXP,				HIDC_REP_CHK_REGULAREXP,			// 正規表現
	IDC_CHECK_NOTIFYNOTFOUND,		HIDC_REP_CHECK_NOTIFYNOTFOUND,		// 見つからないときに通知
	IDC_CHECK_bAutoCloseDlgReplace,	HIDC_REP_CHECK_bAutoCloseDlgReplace,// 自動的に閉じる
	IDC_COMBO_TEXT,					HIDC_REP_COMBO_TEXT,				// 置換前
	IDC_COMBO_TEXT2,				HIDC_REP_COMBO_TEXT2,				// 置換後
	IDC_RADIO_REPLACE,				HIDC_REP_RADIO_REPLACE,				// 置換対象：置換
	IDC_RADIO_INSERT,				HIDC_REP_RADIO_INSERT,				// 置換対象：挿入
	IDC_RADIO_ADD,					HIDC_REP_RADIO_ADD,					// 置換対象：追加
	IDC_RADIO_LINEDELETE,			HIDC_REP_RADIO_LINEDELETE,			// 置換対象：行削除
	IDC_RADIO_SELECTEDAREA,			HIDC_REP_RADIO_SELECTEDAREA,		// 範囲：全体
	IDC_RADIO_ALLAREA,				HIDC_REP_RADIO_ALLAREA,				// 範囲：選択範囲
	IDC_STATIC_JRE32VER,			HIDC_REP_STATIC_JRE32VER,			// 正規表現バージョン
	IDC_BUTTON_SETMARK,				HIDC_REP_BUTTON_SETMARK,			// 2002.01.16 hor 検索該当行をマーク
	IDC_CHECK_SEARCHALL,			HIDC_REP_CHECK_SEARCHALL,			// 2002.01.26 hor 先頭（末尾）から再検索
	IDC_CHECK_CONSECUTIVEALL,		HIDC_REP_CHECK_CONSECUTIVEALL,		//「すべて置換」は置換の繰返し	// 2007.01.16 ryoji
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

DlgReplace::DlgReplace()
{
	searchOption.Reset();	// 検索オプション
	bConsecutiveAll = false;	//「すべて置換」は置換の繰返し	// 2007.01.16 ryoji
	bSelectedArea = false;	// 選択範囲内置換
	nReplaceTarget = 0;		// 置換対象		// 2001.12.03 hor
	bPaste = false;			// 貼り付ける？	// 2001.12.03 hor
	nReplaceCnt = 0;			// すべて置換の実行結果		// 2002.02.08 hor
	bCanceled = false;		// すべて置換を中断したか	// 2002.02.08 hor
}

/*!
	コンボボックスのドロップダウンメッセージを捕捉する

	@date 2013.03.24 novice 新規作成
*/
BOOL DlgReplace::OnCbnDropDown(HWND hwndCtl, int wID)
{
	switch (wID) {
	case IDC_COMBO_TEXT:
		if (::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			size_t nSize = pShareData->searchKeywords.searchKeys.size();
			for (size_t i=0; i<nSize; ++i) {
				Combo_AddString( hwndCtl, pShareData->searchKeywords.searchKeys[i] );
			}
		}
		break;
	case IDC_COMBO_TEXT2:
		if (::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			size_t nSize = pShareData->searchKeywords.replaceKeys.size();
			for (size_t i=0; i<nSize; ++i) {
				Combo_AddString( hwndCtl, pShareData->searchKeywords.replaceKeys[i] );
			}
		}
		break;
	}
	return Dialog::OnCbnDropDown( hwndCtl, wID );
}

// モードレスダイアログの表示
HWND DlgReplace::DoModeless(
	HINSTANCE hInstance,
	HWND hwndParent,
	LPARAM lParam,
	bool bSelected
	)
{
	auto& csSearch = pShareData->common.search;
	searchOption = csSearch.searchOption;		// 検索オプション
	bConsecutiveAll = csSearch.bConsecutiveAll;	//「すべて置換」は置換の繰返し	// 2007.01.16 ryoji
	bSelectedArea = csSearch.bSelectedArea;		// 選択範囲内置換
	bNotifyNotFound = csSearch.bNotifyNotFound;	// 検索／置換  見つからないときメッセージを表示
	bSelected = bSelected;
	ptEscCaretPos_PHY = ((EditView*)lParam)->GetCaret().GetCaretLogicPos();	// 検索/置換開始時のカーソル位置退避
	((EditView*)lParam)->bSearch = true;			// 検索/置換開始位置の登録有無			02/07/28 ai
	return Dialog::DoModeless(hInstance, hwndParent, IDD_REPLACE, lParam, SW_SHOW);
}

// モードレス時：置換・検索対象となるビューの変更
void DlgReplace::ChangeView(LPARAM pcEditView)
{
	lParam = pcEditView;
	return;
}


// ダイアログデータの設定
void DlgReplace::SetData(void)
{
	auto& csSearch = pShareData->common.search;

	// 検索文字列/置換後文字列リストの設定(関数化)	2010/5/26 Uchi
	SetCombosList();

	// 英大文字と英小文字を区別する
	CheckButton(IDC_CHK_LOHICASE, searchOption.bLoHiCase);

	// 2001/06/23 N.Nakatani
	// 単語単位で探す
	CheckButton(IDC_CHK_WORD, searchOption.bWordOnly);

	//「すべて置換」は置換の繰返し  2007.01.16 ryoji
	CheckButton(IDC_CHECK_CONSECUTIVEALL, bConsecutiveAll);

	// From Here Jun. 29, 2001 genta
	// 正規表現ライブラリの差し替えに伴う処理の見直し
	// 処理フロー及び判定条件の見直し。必ず正規表現のチェックと
	// 無関係にCheckRegexpVersionを通過するようにした。
	if (CheckRegexpVersion(GetHwnd(), IDC_STATIC_JRE32VER, false)
		&& searchOption.bRegularExp
	) {
		// 英大文字と英小文字を区別する
		CheckButton(IDC_CHK_REGULAREXP, true);

		// 2001/06/23 N.Nakatani
		// 単語単位で探す
		EnableItem(IDC_CHK_WORD, false);
	}else {
		CheckButton(IDC_CHK_REGULAREXP, false);

		//「すべて置換」は置換の繰返し
		EnableItem(IDC_CHECK_CONSECUTIVEALL, false);	// 2007.01.16 ryoji
	}
	// To Here Jun. 29, 2001 genta

	// 検索／置換  見つからないときメッセージを表示
	CheckButton(IDC_CHECK_NOTIFYNOTFOUND, bNotifyNotFound);

	// 置換 ダイアログを自動的に閉じる
	CheckButton(IDC_CHECK_bAutoCloseDlgReplace, csSearch.bAutoCloseDlgReplace);

	// 先頭（末尾）から再検索 2002.01.26 hor
	CheckButton(IDC_CHECK_SEARCHALL, csSearch.bSearchAll);

	// From Here 2001.12.03 hor
	// クリップボードから貼り付ける？
	CheckButton(IDC_CHK_PASTE, bPaste);
	// 置換対象
	if (nReplaceTarget == 0) {
		CheckButton(IDC_RADIO_REPLACE, true);
	}else if (nReplaceTarget == 1) {
		CheckButton(IDC_RADIO_INSERT, true);
	}else if (nReplaceTarget == 2) {
		CheckButton(IDC_RADIO_ADD, true);
	}else if (nReplaceTarget == 3) {
		CheckButton(IDC_RADIO_LINEDELETE, true);
		EnableItem(IDC_COMBO_TEXT2, false);
		EnableItem(IDC_CHK_PASTE, false);
	}
	// To Here 2001.12.03 hor

	return;
}


// 検索文字列/置換後文字列リストの設定
// 2010/5/26 Uchi
void DlgReplace::SetCombosList(void)
{
	// 検索文字列
	HWND hwndCombo = GetItemHwnd(IDC_COMBO_TEXT);
	while (Combo_GetCount(hwndCombo) > 0) {
		Combo_DeleteString(hwndCombo, 0);
	}
	int nBufferSize = ::GetWindowTextLength(hwndCombo) + 1;
	std::vector<TCHAR> vText;
	vText.resize(nBufferSize);
	Combo_GetText(hwndCombo, &vText[0], nBufferSize);
	if (auto_strcmp(to_wchar(&vText[0]), strText.c_str()) != 0) {
		SetItemText(IDC_COMBO_TEXT, strText.c_str());
	}

	// 置換後文字列
	hwndCombo = GetItemHwnd(IDC_COMBO_TEXT2);
	while (Combo_GetCount(hwndCombo) > 0) {
		Combo_DeleteString(hwndCombo, 0);
	}
	nBufferSize = ::GetWindowTextLength(hwndCombo) + 1;
	vText.resize(nBufferSize);
	Combo_GetText(hwndCombo, &vText[0], nBufferSize);
	if (auto_strcmp(to_wchar(&vText[0]), strText2.c_str()) != 0) {
		SetItemText(IDC_COMBO_TEXT2, strText2.c_str());
	}
}


// ダイアログデータの取得
// 0==条件未入力  0より大きい==正常   0より小さい==入力エラー
int DlgReplace::GetData(void)
{
	auto& csSearch = pShareData->common.search;

	// 英大文字と英小文字を区別する
	searchOption.bLoHiCase = IsButtonChecked(IDC_CHK_LOHICASE);

	// 2001/06/23 N.Nakatani
	// 単語単位で探す
	searchOption.bWordOnly = IsButtonChecked(IDC_CHK_WORD);

	//「すべて置換」は置換の繰返し  2007.01.16 ryoji
	bConsecutiveAll = IsButtonChecked(IDC_CHECK_CONSECUTIVEALL);

	// 正規表現
	searchOption.bRegularExp = IsButtonChecked(IDC_CHK_REGULAREXP);
	// 選択範囲内置換
	bSelectedArea = IsButtonChecked(IDC_RADIO_SELECTEDAREA);
	// 検索／置換  見つからないときメッセージを表示
	bNotifyNotFound = IsButtonChecked(IDC_CHECK_NOTIFYNOTFOUND);

	csSearch.bConsecutiveAll = bConsecutiveAll;	// 1==「すべて置換」は置換の繰返し	// 2007.01.16 ryoji
	csSearch.bSelectedArea = bSelectedArea;		// 選択範囲内置換
	csSearch.bNotifyNotFound = bNotifyNotFound;	// 検索／置換  見つからないときメッセージを表示

	// 検索文字列
	int nBufferSize = ::GetWindowTextLength(GetItemHwnd(IDC_COMBO_TEXT)) + 1;
	std::vector<TCHAR> vText(nBufferSize);
	GetItemText(IDC_COMBO_TEXT, &vText[0], nBufferSize);
	strText = to_wchar(&vText[0]);
	// 置換後文字列
	if (IsButtonChecked(IDC_RADIO_LINEDELETE )) {
		strText2 = L"";
	}else {
		nBufferSize = ::GetWindowTextLength(GetItemHwnd(IDC_COMBO_TEXT2)) + 1;
		vText.resize(nBufferSize);
		GetItemText(IDC_COMBO_TEXT2, &vText[0], nBufferSize);
		strText2 = to_wchar(&vText[0]);
	}

	// 置換 ダイアログを自動的に閉じる
	csSearch.bAutoCloseDlgReplace = IsButtonChecked(IDC_CHECK_bAutoCloseDlgReplace);

	// 先頭（末尾）から再検索 2002.01.26 hor
	csSearch.bSearchAll = IsButtonChecked(IDC_CHECK_SEARCHALL);

	if (0 < strText.size()) {
		// 正規表現？
		// From Here Jun. 26, 2001 genta
		// 正規表現ライブラリの差し替えに伴う処理の見直し
		int nFlag = 0x00;
		nFlag |= searchOption.bLoHiCase ? 0x01 : 0x00;
		if (searchOption.bRegularExp
			&& !CheckRegexpSyntax(strText.c_str(), GetHwnd(), true, nFlag)
		) {
			return -1;
		}
		// To Here Jun. 26, 2001 genta 正規表現ライブラリ差し替え

		// 検索文字列
		//@@@ 2002.2.2 YAZAKI CShareData.AddToSearchKeys()追加に伴う変更
		if (strText.size() < _MAX_PATH) {
			SearchKeywordManager().AddToSearchKeys(strText.c_str());
			csSearch.searchOption = searchOption;		// 検索オプション
		}
		// 2011.12.18 viewに直接設定
		EditView* pEditView = (EditView*)lParam;
		if (pEditView->strCurSearchKey == strText && pEditView->curSearchOption == searchOption) {
		}else {
			pEditView->strCurSearchKey = strText;
			pEditView->curSearchOption = searchOption;
			pEditView->bCurSearchUpdate = true;
		}
		pEditView->nCurSearchKeySequence = GetDllShareData().common.search.nSearchKeySequence;

		// 置換後文字列
		//@@@ 2002.2.2 YAZAKI CShareData.AddToReplaceKeys()追加に伴う変更
		if (strText2.size() < _MAX_PATH) {
			SearchKeywordManager().AddToReplaceKeys(strText2.c_str());
		}
		nReplaceKeySequence = GetDllShareData().common.search.nReplaceKeySequence;

		// From Here 2001.12.03 hor
		// クリップボードから貼り付ける？
		bPaste = IsButtonChecked(IDC_CHK_PASTE);
		EnableItem(IDC_COMBO_TEXT2, !bPaste);
		// 置換対象
		nReplaceTarget = 0;
		if (IsButtonChecked(IDC_RADIO_INSERT)) {
			nReplaceTarget = 1;
		}else if (IsButtonChecked(IDC_RADIO_ADD)) {
			nReplaceTarget = 2;
		}else if (IsButtonChecked(IDC_RADIO_LINEDELETE )) {
			nReplaceTarget = 3;
			bPaste = false;
			EnableItem(IDC_COMBO_TEXT2, false);
		}
		// To Here 2001.12.03 hor

		// 検索文字列/置換後文字列リストの設定	2010/5/26 Uchi
		if (!bModal) {
			SetCombosList();
		}
		return 1;
	}else {
		return 0;
	}
}


BOOL DlgReplace::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	_SetHwnd(hwndDlg);
	// Jun. 26, 2001 genta
	// この位置で正規表現の初期化をする必要はない
	// 他との一貫性を保つため削除

	// ユーザーがコンボ ボックスのエディット コントロールに入力できるテキストの長さを制限する
	// Combo_LimitText(GetItemHwnd(IDC_COMBO_TEXT), _MAX_PATH - 1);
	// Combo_LimitText(GetItemHwnd(IDC_COMBO_TEXT2), _MAX_PATH - 1);

	// コンボボックスのユーザー インターフェイスを拡張インターフェースにする
	Combo_SetExtendedUI(GetItemHwnd(IDC_COMBO_TEXT), TRUE);
	Combo_SetExtendedUI(GetItemHwnd(IDC_COMBO_TEXT2), TRUE);

	// テキスト選択中か
	if (bSelected) {
		EnableItem(IDC_BUTTON_SEARCHPREV, false);	// 2001.12.03 hor コメント解除
		EnableItem(IDC_BUTTON_SEARCHNEXT, false);	// 2001.12.03 hor コメント解除
		EnableItem(IDC_BUTTON_REPALCE, false);		// 2001.12.03 hor コメント解除
		CheckButton(IDC_RADIO_SELECTEDAREA, true);
//		CheckButton(IDC_RADIO_ALLAREA, false);					// 2001.12.03 hor コメント
	}else {
//		EnableItem(IDC_RADIO_SELECTEDAREA), false);	// 2001.12.03 hor コメント
//		CheckButton(IDC_RADIO_SELECTEDAREA, false);				// 2001.12.03 hor コメント
		CheckButton(IDC_RADIO_ALLAREA, true);
	}

	comboDelText = ComboBoxItemDeleter();
	comboDelText.pRecent = &recentSearch;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_TEXT), &comboDelText);
	comboDelText2 = ComboBoxItemDeleter();
	comboDelText2.pRecent = &recentReplace;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_TEXT2), &comboDelText2);

	// フォント設定	2012/11/27 Uchi
	HFONT hFontOld = (HFONT)::SendMessage(GetItemHwnd(IDC_COMBO_TEXT), WM_GETFONT, 0, 0);
	HFONT hFont = SetMainFont(GetItemHwnd(IDC_COMBO_TEXT));
	fontText.SetFont(hFontOld, hFont, GetItemHwnd(IDC_COMBO_TEXT));

	hFontOld = (HFONT)::SendMessage(GetItemHwnd(IDC_COMBO_TEXT2), WM_GETFONT, 0, 0);
	hFont = SetMainFont(GetItemHwnd(IDC_COMBO_TEXT2));
	fontText2.SetFont(hFontOld, hFont, GetItemHwnd(IDC_COMBO_TEXT2));

	// 基底クラスメンバ
	return Dialog::OnInitDialog(hwndDlg, wParam, lParam);
}


BOOL DlgReplace::OnDestroy()
{
	fontText.ReleaseOnDestroy();
	fontText2.ReleaseOnDestroy();
	return Dialog::OnDestroy();
}


BOOL DlgReplace::OnBnClicked(int wID)
{
	int nRet;
	EditView* pEditView = (EditView*)lParam;

	switch (wID) {
	case IDC_CHK_PASTE:
		// テキストの貼り付け
		if (1
			&& IsButtonChecked(IDC_CHK_PASTE)
			&& !pEditView->pEditDoc->docEditor.IsEnablePaste()
		) {
			OkMessage(GetHwnd(), LS(STR_DLGREPLC_CLIPBOARD));
			CheckButton(IDC_CHK_PASTE, false);
		}
		EnableItem(IDC_COMBO_TEXT2, !IsButtonChecked(IDC_CHK_PASTE));
		return TRUE;
		// 置換対象
	case IDC_RADIO_REPLACE:
	case IDC_RADIO_INSERT:
	case IDC_RADIO_ADD:
	case IDC_RADIO_LINEDELETE:
		if (IsButtonChecked(IDC_RADIO_LINEDELETE )) {
			EnableItem(IDC_COMBO_TEXT2, false);
			EnableItem(IDC_CHK_PASTE, false);
		}else {
			EnableItem(IDC_COMBO_TEXT2, true);
			EnableItem(IDC_CHK_PASTE, true);
		}
		return TRUE;
	case IDC_RADIO_SELECTEDAREA:
		// 範囲範囲
		if (IsButtonChecked(IDC_RADIO_ALLAREA)) {
			EnableItem(IDC_BUTTON_SEARCHPREV, true);
			EnableItem(IDC_BUTTON_SEARCHNEXT, true);
			EnableItem(IDC_BUTTON_REPALCE, true);
		}else {
			EnableItem(IDC_BUTTON_SEARCHPREV, false);
			EnableItem(IDC_BUTTON_SEARCHNEXT, false);
			EnableItem(IDC_BUTTON_REPALCE, false);
		}
		return TRUE;
	case IDC_RADIO_ALLAREA:
		// ファイル全体
		if (IsButtonChecked(IDC_RADIO_ALLAREA)) {
			EnableItem(IDC_BUTTON_SEARCHPREV, true);
			EnableItem(IDC_BUTTON_SEARCHNEXT, true);
			EnableItem(IDC_BUTTON_REPALCE, true);
		}else {
			EnableItem(IDC_BUTTON_SEARCHPREV, false);
			EnableItem(IDC_BUTTON_SEARCHNEXT, false);
			EnableItem(IDC_BUTTON_REPALCE, false);
		}
		return TRUE;
// To Here 2001.12.03 hor
	case IDC_BUTTON_HELP:
		//「置換」のヘルプ
		// Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_REPLACE_DIALOG));	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//	case IDC_CHK_LOHICASE:	// 大文字と小文字を区別する
//		MYTRACE(_T("IDC_CHK_LOHICASE\n"));
//		return TRUE;
//	case IDC_CHK_WORDONLY:	// 一致する単語のみ検索
//		MYTRACE(_T("IDC_CHK_WORDONLY\n"));
//		break;
	case IDC_CHK_REGULAREXP:	// 正規表現
//		MYTRACE(_T("IDC_CHK_REGULAREXP ::IsDlgButtonChecked(GetHwnd(), IDC_CHK_REGULAREXP) = %d\n"), ::IsDlgButtonChecked(GetHwnd(), IDC_CHK_REGULAREXP));
		if (IsButtonChecked(IDC_CHK_REGULAREXP)) {
			// From Here Jun. 26, 2001 genta
			// 正規表現ライブラリの差し替えに伴う処理の見直し
			if (!CheckRegexpVersion(GetHwnd(), IDC_STATIC_JRE32VER, true)) {
				CheckButton(IDC_CHK_REGULAREXP, false);
			}else {
			// To Here Jun. 26, 2001 genta

				// 英大文字と英小文字を区別する
				// Jan. 31, 2002 genta
				// 大文字・小文字の区別は正規表現の設定に関わらず保存する
				//CheckButton(IDC_CHK_LOHICASE, true);
				//EnableItem(IDC_CHK_LOHICASE, false);

				// 2001/06/23 N.Nakatani
				// 単語単位で探す
				EnableItem(IDC_CHK_WORD, false);

				//「すべて置換」は置換の繰返し
				EnableItem(IDC_CHECK_CONSECUTIVEALL, true);	// 2007.01.16 ryoji
			}
		}else {
			// 英大文字と英小文字を区別する
			//EnableItem(IDC_CHK_LOHICASE, true);
			// Jan. 31, 2002 genta
			// 大文字・小文字の区別は正規表現の設定に関わらず保存する
			//CheckButton(IDC_CHK_LOHICASE, false);

			// 2001/06/23 N.Nakatani
			// 単語単位で探す
			EnableItem(IDC_CHK_WORD, true);

			//「すべて置換」は置換の繰返し
			EnableItem(IDC_CHECK_CONSECUTIVEALL, false);	// 2007.01.16 ryoji
		}
		return TRUE;
//	case IDOK:			// 下検索
//		// ダイアログデータの取得
//		nRet = GetData();
//		if (0 < nRet) {
//			::EndDialog(hwndDlg, 2);
//		}else
//		if (nRet == 0) {
//			::EndDialog(hwndDlg, 0);
//		}
//		return TRUE;


	case IDC_BUTTON_SEARCHPREV:	// 上検索
		nRet = GetData();
		if (0 < nRet) {

			// 検索開始位置を登録 02/07/28 ai start
			if (pEditView->bSearch != FALSE) {
				pEditView->ptSrchStartPos_PHY = ptEscCaretPos_PHY;
				pEditView->bSearch = FALSE;
			}// 02/07/28 ai end

			// コマンドコードによる処理振り分け
			// 前を検索
			pEditView->GetCommander().HandleCommand(F_SEARCH_PREV, true, (LPARAM)GetHwnd(), 0, 0, 0);
			// 再描画（0文字幅マッチでキャレットを表示するため）
			pEditView->Redraw();	// 前回0文字幅マッチの消去にも必要
		}else if (nRet == 0) {
			OkMessage(GetHwnd(), LS(STR_DLGREPLC_STR));
		}
		return TRUE;
	case IDC_BUTTON_SEARCHNEXT:	// 下検索
		nRet = GetData();
		if (0 < nRet) {

			// 検索開始位置を登録 02/07/28 ai start
			if (pEditView->bSearch) {
				pEditView->ptSrchStartPos_PHY = ptEscCaretPos_PHY;
				pEditView->bSearch = false;
			}// 02/07/28 ai end

			// コマンドコードによる処理振り分け
			// 次を検索
			pEditView->GetCommander().HandleCommand(F_SEARCH_NEXT, true, (LPARAM)GetHwnd(), 0, 0, 0);
			// 再描画（0文字幅マッチでキャレットを表示するため）
			pEditView->Redraw();	// 前回0文字幅マッチの消去にも必要
		}else if (nRet == 0) {
			OkMessage(GetHwnd(), LS(STR_DLGREPLC_STR));
		}
		return TRUE;

	case IDC_BUTTON_SETMARK:	// 2002.01.16 hor 該当行マーク
		nRet = GetData();
		if (0 < nRet) {
			pEditView->GetCommander().HandleCommand(F_BOOKMARK_PATTERN, false, 0, 0, 0, 0);
			::SendMessage(GetHwnd(), WM_NEXTDLGCTL, (WPARAM)GetItemHwnd(IDC_COMBO_TEXT), TRUE);
		}
		return TRUE;

	case IDC_BUTTON_REPALCE:	// 置換
		nRet = GetData();
		if (0 < nRet) {

			// 置換開始位置を登録 02/07/28 ai start
			if (pEditView->bSearch) {
				pEditView->ptSrchStartPos_PHY = ptEscCaretPos_PHY;
				pEditView->bSearch = false;
			}// 02/07/28 ai end

			// 置換
			//@@@ 2002.2.2 YAZAKI 置換コマンドをEditViewに新設
			//@@@ 2002/04/08 YAZAKI 親ウィンドウのハンドルを渡すように変更。
			pEditView->GetCommander().HandleCommand(F_REPLACE, true, (LPARAM)GetHwnd(), 0, 0, 0);
			// 再描画
			pEditView->GetCommander().HandleCommand(F_REDRAW, true, 0, 0, 0, 0);
		}else if (nRet == 0) {
			OkMessage(GetHwnd(), LS(STR_DLGREPLC_STR));
		}
		return TRUE;
	case IDC_BUTTON_REPALCEALL:	// すべて置換
		nRet = GetData();
		if (0 < nRet) {
			// 置換開始位置を登録 02/07/28 ai start
			if (pEditView->bSearch) {
				pEditView->ptSrchStartPos_PHY = ptEscCaretPos_PHY;
				pEditView->bSearch = false;
			}// 02/07/28 ai end

			// すべて行置換時の処置は「すべて置換」は置換の繰返しオプションOFFの場合にして削除 2007.01.16 ryoji
			pEditView->GetCommander().HandleCommand(F_REPLACE_ALL, true, 0, 0, 0, 0);
			pEditView->GetCommander().HandleCommand(F_REDRAW, true, 0, 0, 0, 0);

			// アクティブにする
			ActivateFrameWindow(GetHwnd());

			TopOkMessage(GetHwnd(), LS(STR_DLGREPLC_REPLACE), nReplaceCnt);

			if (!bCanceled) {
				if (bModal) {		// モーダルダイアログか
					// 置換ダイアログを閉じる
					::EndDialog(GetHwnd(), 0);
				}else {
					// 置換 ダイアログを自動的に閉じる
					if (pShareData->common.search.bAutoCloseDlgReplace) {
						::DestroyWindow(GetHwnd());
					}
				}
			}
			return TRUE;
		}else if (nRet == 0) {
			OkMessage(GetHwnd(), LS(STR_DLGREPLC_REPSTR));
		}
		return TRUE;
//	case IDCANCEL:
//		::EndDialog(hwndDlg, 0);
//		return TRUE;
	}

	// 基底クラスメンバ
	return Dialog::OnBnClicked(wID);
}

BOOL DlgReplace::OnActivate(WPARAM wParam, LPARAM lParam)
{
	// 0文字幅マッチ描画のON/OFF	// 2009.11.29 ryoji
	EditView*	pEditView = (EditView*)(this->lParam);
	Range rangeSel = pEditView->GetSelectionInfo().select;
	if (rangeSel.IsValid() && rangeSel.IsLineOne() && rangeSel.IsOne())
		pEditView->InvalidateRect(NULL);	// アクティブ化／非アクティブ化が完了してから再描画

	return Dialog::OnActivate(wParam, lParam);
}

//@@@ 2002.01.18 add start
LPVOID DlgReplace::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


