// タイプ別設定 - 支援

#include "StdAfx.h"
#include "PropTypes.h"
#include "dlg/DlgOpenFile.h"
#include "util/module.h"
#include "util/shell.h"
#include "util/fileUtil.h" // _IS_REL_PATH
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids3[] = {	//11500
	IDC_EDIT_HOKANFILE,				HIDC_EDIT_HOKANFILE,				// 単語ファイル名
	IDC_BUTTON_HOKANFILE_REF,		HIDC_BUTTON_HOKANFILE_REF,			// 入力補完 単語ファイル参照
	IDC_COMBO_HOKAN_TYPE,			HIDC_COMBO_HOKAN_TYPE,				// 入力補完タイプ
	IDC_CHECK_HOKANLOHICASE,		HIDC_CHECK_HOKANLOHICASE,			// 入力補完の英大文字小文字
	IDC_CHECK_HOKANBYFILE,			HIDC_CHECK_HOKANBYFILE,				// 現在のファイルから入力補完
	IDC_CHECK_HOKANBYKEYWORD,		HIDC_CHECK_HOKANBYKEYWORD,			// 強調キーワードから入力補完

	IDC_EDIT_TYPEEXTHELP,			HIDC_EDIT_TYPEEXTHELP,				// 外部ヘルプファイル名	// 2006.08.06 ryoji
	IDC_BUTTON_TYPEOPENHELP,		HIDC_BUTTON_TYPEOPENHELP,			// 外部ヘルプファイル参照	// 2006.08.06 ryoji
	IDC_EDIT_TYPEEXTHTMLHELP,		HIDC_EDIT_TYPEEXTHTMLHELP,			// 外部HTMLヘルプファイル名	// 2006.08.06 ryoji
	IDC_BUTTON_TYPEOPENEXTHTMLHELP,	HIDC_BUTTON_TYPEOPENEXTHTMLHELP,	// 外部HTMLヘルプファイル参照	// 2006.08.06 ryoji
	IDC_CHECK_TYPEHTMLHELPISSINGLE,	HIDC_CHECK_TYPEHTMLHELPISSINGLE,	// ビューアを複数起動しない	// 2006.08.06 ryoji

	IDC_CHECK_CHKENTERATEND,		HIDC_CHECK_CHKENTERATEND,			// 保存時に改行コードの混在を警告する	// 2013/4/14 Uchi
	//	IDC_STATIC,						-1,
	0, 0
};


struct HokanMethod {
	int nMethod;
	std::wstring name;
};

static std::vector<HokanMethod>* GetHokanMethodList()
{
	static std::vector<HokanMethod> methodList;
	return &methodList;
}


// 2001/06/13 Start By asa-o: タイプ別設定の支援タブに関する処理

// メッセージ処理
INT_PTR PropTypesSupport::DispatchEvent(
	HWND		hwndDlg,	// handle to dialog box
	UINT		uMsg,		// message
	WPARAM		wParam,		// first message parameter
	LPARAM		lParam 		// second message parameter
	)
{
	WORD		wNotifyCode;
	WORD		wID;
	NMHDR*		pNMHDR;

	switch (uMsg) {
	case WM_INITDIALOG:
		// ダイアログデータの設定 p2
		SetData(hwndDlg);
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// ユーザーがエディット コントロールに入力できるテキストの長さを制限する
		// 入力補完 単語ファイル
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_HOKANFILE), _MAX_PATH - 1);

		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// 通知コード
		wID			= LOWORD(wParam);	// 項目ID､ コントロールID､ またはアクセラレータID
//		hwndCtl		= (HWND) lParam;	// コントロールのハンドル
		switch (wNotifyCode) {
		// ボタン／チェックボックスがクリックされた
		case BN_CLICKED:
			// ダイアログデータの取得 p2
			GetData(hwndDlg);
			switch (wID) {
			case IDC_BUTTON_HOKANFILE_REF:	// 入力補完 単語ファイルの「参照...」ボタン
				{
					DlgOpenFile	dlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
					// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
					if (_IS_REL_PATH(types.szHokanFile)) {
						GetInidirOrExedir(szPath, types.szHokanFile);
					}else {
						_tcscpy(szPath, types.szHokanFile);
					}
					// ファイルオープンダイアログの初期化
					dlgOpenFile.Create(
						hInstance,
						hwndDlg,
						_T("*.*"),
						szPath
					);
					if (dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
						_tcscpy(types.szHokanFile, szPath);
						::DlgItem_SetText(hwndDlg, IDC_EDIT_HOKANFILE, types.szHokanFile);
					}
				}
				return TRUE;
			case IDC_BUTTON_TYPEOPENHELP:	// 外部ヘルプ１の「参照...」ボタン
				{
					DlgOpenFile	dlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
					// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
					if (_IS_REL_PATH(types.szExtHelp)) {
						GetInidirOrExedir(szPath, types.szExtHelp, true);
					}else {
						_tcscpy(szPath, types.szExtHelp);
					}
					// ファイルオープンダイアログの初期化
					dlgOpenFile.Create(
						hInstance,
						hwndDlg,
						_T("*.hlp;*.chm;*.col"),
						szPath
					);
					if (dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
						_tcscpy(types.szExtHelp, szPath);
						::DlgItem_SetText(hwndDlg, IDC_EDIT_TYPEEXTHELP, types.szExtHelp);
					}
				}
				return TRUE;
			case IDC_BUTTON_TYPEOPENEXTHTMLHELP:	// 外部HTMLヘルプの「参照...」ボタン
				{
					DlgOpenFile	dlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
					// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
					if (_IS_REL_PATH(types.szExtHtmlHelp)) {
						GetInidirOrExedir(szPath, types.szExtHtmlHelp, true);
					}else {
						_tcscpy(szPath, types.szExtHtmlHelp);
					}
					// ファイルオープンダイアログの初期化
					dlgOpenFile.Create(
						hInstance,
						hwndDlg,
						_T("*.chm;*.col"),
						szPath
					);
					if (dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
						_tcscpy(types.szExtHtmlHelp, szPath);
						::DlgItem_SetText(hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP, types.szExtHtmlHelp);
					}
				}
				return TRUE;
			}
			break;	// BN_CLICKED
		}
		break;	// WM_COMMAND
	case WM_NOTIFY:
//		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
//		pMNUD  = (NM_UPDOWN*)lParam;
		switch (pNMHDR->code) {
		case PSN_HELP:	// Jul. 03, 2001 JEPRO 支援タブのヘルプを有効化
			OnHelp(hwndDlg, IDD_PROP_SUPPORT);
			return TRUE;
		case PSN_KILLACTIVE:
			// ダイアログデータの取得 p2
			GetData(hwndDlg);
			return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
		case PSN_SETACTIVE:
			nPageNum = ID_PROPTYPE_PAGENUM_SUPPORT;
			return TRUE;
		}
		break;

// From Here Jul. 05, 2001 JEPRO: Popup Help
	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids3);	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		// NOTREACHED
//		break;
// To Here  Jul. 05, 2001

//@@@ 2001.11.17 add start MIK
	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids3);	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//@@@ 2001.11.17 add end MIK

	}
	return FALSE;
}

// ダイアログデータの設定
void PropTypesSupport::SetData(HWND hwndDlg)
{
	// 入力補完 単語ファイル
	::DlgItem_SetText(hwndDlg, IDC_EDIT_HOKANFILE, types.szHokanFile);

	{
		HWND hCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_HOKAN_TYPE);
		std::vector<HokanMethod>* pMedothList = GetHokanMethodList();
		ApiWrap::Combo_AddString(hCombo, LS(STR_SMART_INDENT_NONE));
		Combo_SetCurSel(hCombo, 0);
		size_t nSize = pMedothList->size();
		for (size_t i=0; i<nSize; ++i) {
			ApiWrap::Combo_AddString(hCombo, (*pMedothList)[i].name.c_str());
			if (types.nHokanType == (*pMedothList)[i].nMethod) {
				Combo_SetCurSel(hCombo, i + 1);
			}
		}
	}

//	2001/06/19 asa-o
	// 入力補完機能：英大文字小文字を同一視する
	::CheckDlgButton(hwndDlg, IDC_CHECK_HOKANLOHICASE, types.bHokanLoHiCase ? BST_CHECKED : BST_UNCHECKED);

	// 2003.06.25 Moca ファイルからの補完機能
	::CheckDlgButton(hwndDlg, IDC_CHECK_HOKANBYFILE, types.bUseHokanByFile ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_HOKANBYKEYWORD, types.bUseHokanByKeyword);

	//@@@ 2002.2.2 YAZAKI
	::DlgItem_SetText(hwndDlg, IDC_EDIT_TYPEEXTHELP, types.szExtHelp);
	::DlgItem_SetText(hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP, types.szExtHtmlHelp);
	::CheckDlgButton(hwndDlg, IDC_CHECK_TYPEHTMLHELPISSINGLE, types.bHtmlHelpIsSingle ? BST_CHECKED : BST_UNCHECKED);

	// 保存時に改行コードの混在を警告する	2013/4/14 Uchi
	::CheckDlgButton(hwndDlg, IDC_CHECK_CHKENTERATEND, types.bChkEnterAtEnd ? BST_CHECKED : BST_UNCHECKED);
}

// ダイアログデータの取得
int PropTypesSupport::GetData(HWND hwndDlg)
{
//	2001/06/19	asa-o
	// 入力補完機能：英大文字小文字を同一視する
	types.bHokanLoHiCase = DlgButton_IsChecked(hwndDlg, IDC_CHECK_HOKANLOHICASE);

	types.bUseHokanByFile = DlgButton_IsChecked(hwndDlg, IDC_CHECK_HOKANBYFILE);
	types.bUseHokanByKeyword = DlgButton_IsChecked(hwndDlg, IDC_CHECK_HOKANBYKEYWORD);

	// 入力補完 単語ファイル
	::DlgItem_GetText(hwndDlg, IDC_EDIT_HOKANFILE, types.szHokanFile, _countof2(types.szHokanFile));

	// 入力補完種別
	{
		HWND hCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_HOKAN_TYPE);
		int i = Combo_GetCurSel(hCombo);
		if (i == 0) {
			types.nHokanType = 0;
		}else if (i != CB_ERR) {
			types.nHokanType = (*GetHokanMethodList())[i - 1].nMethod;
		}
	}

	//@@@ 2002.2.2 YAZAKI
	::DlgItem_GetText(hwndDlg, IDC_EDIT_TYPEEXTHELP, types.szExtHelp, _countof2(types.szExtHelp));
	::DlgItem_GetText(hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP, types.szExtHtmlHelp, _countof2(types.szExtHtmlHelp));
	types.bHtmlHelpIsSingle = DlgButton_IsChecked(hwndDlg, IDC_CHECK_TYPEHTMLHELPISSINGLE);

	// 保存時に改行コードの混在を警告する	2013/4/14 Uchi
	types.bChkEnterAtEnd = DlgButton_IsChecked(hwndDlg, IDC_CHECK_CHKENTERATEND);

	return TRUE;
}

// 2001/06/13 End

// 補完種別の追加
void PropTypesSupport::AddHokanMethod(int nMethod, const wchar_t* szName)
{
	HokanMethod item = { nMethod, std::wstring(szName) };
	GetHokanMethodList()->push_back(item);
}

void PropTypesSupport::RemoveHokanMethod(int nMethod, const wchar_t* szName)
{
	int nSize = GetHokanMethodList()->size();
	for (int i=0; i<nSize; ++i) {
		if ((*GetHokanMethodList())[i].nMethod == nMethod) {
			GetHokanMethodList()->erase(GetHokanMethodList()->begin() + i);
			break;
		}
	}
}

