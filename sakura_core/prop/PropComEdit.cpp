/*!	@file
	@brief 共通設定ダイアログボックス、「編集」ページ
*/
#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "env/DllSharedData.h"
#include "env/FileNameManager.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//10210
	IDC_CHECK_ADDCRLFWHENCOPY,			HIDC_CHECK_ADDCRLFWHENCOPY,				// 折り返し行に改行を付けてコピー
	IDC_CHECK_COPYnDISABLESELECTEDAREA,	HIDC_CHECK_COPYnDISABLESELECTEDAREA,	// コピーしたら選択解除
	IDC_CHECK_bEnableNoSelectCopy,		HIDC_CHECK_bEnableNoSelectCopy,			// 選択なしでコピーを可能にする	// 2007.11.18 ryoji
	IDC_CHECK_bEnableLineModePaste,		HIDC_CHECK_bEnableLineModePaste,		// ラインモード貼り付けを可能にする	// 2007.10.08 ryoji
	IDC_CHECK_DRAGDROP,					HIDC_CHECK_DRAGDROP,					// Drag&Drop編集する
	IDC_CHECK_DROPSOURCE,				HIDC_CHECK_DROPSOURCE,					// ドロップ元にする
	IDC_CHECK_bNotOverWriteCRLF,		HIDC_CHECK_bNotOverWriteCRLF,			// 上書きモード
	IDC_CHECK_bOverWriteFixMode,		HIDC_CHECK_bOverWriteFixMode,			// 文字幅に合わせてスペースを詰める
	IDC_CHECK_bOverWriteBoxDelete,		HIDC_CHECK_bOverWriteBoxDelete,			// 矩形入力で選択範囲を削除する
	//	2007.02.11 genta クリッカブルURLをこのページに移動
	IDC_CHECK_bSelectClickedURL,		HIDC_CHECK_bSelectClickedURL,			// クリッカブルURL
	IDC_CHECK_CONVERTEOLPASTE,			HIDC_CHECK_CONVERTEOLPASTE,				// 改行コードを変換して貼り付ける
	IDC_RADIO_CURDIR,					HIDC_RADIO_CURDIR,						// カレントフォルダ
	IDC_RADIO_MRUDIR,					HIDC_RADIO_MRUDIR,						// 最近使ったフォルダ
	IDC_RADIO_SELDIR,					HIDC_RADIO_SELDIR,						// 指定フォルダ
	IDC_EDIT_FILEOPENDIR,				HIDC_EDIT_FILEOPENDIR,					// 指定フォルダパス
	IDC_BUTTON_FILEOPENDIR, 			HIDC_EDIT_FILEOPENDIR,					// 指定フォルダパス
	IDC_CHECK_ENABLEEXTEOL,				HIDC_CHECK_ENABLEEXTEOL,				// 改行コードNEL,PS,LSを有効にする
	IDC_CHECK_BOXSELECTLOCK,			HIDC_CHECK_BOXSELECTLOCK,				// 矩形選択移動で選択をロックする
//	IDC_STATIC,							-1,
	0, 0
};
//@@@ 2001.02.04 End

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK PropEdit::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropEdit::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}
//	To Here Jun. 2, 2001 genta

// メッセージ処理
INT_PTR PropEdit::DispatchEvent(
    HWND		hwndDlg,	// handle to dialog box
    UINT		uMsg,		// message
    WPARAM		wParam,		// first message parameter
    LPARAM		lParam 		// second message parameter
	)
{
	WORD		wNotifyCode;
	WORD		wID;
	NMHDR*		pNMHDR;
//	int			nVal;
//	LPDRAWITEMSTRUCT pDis;

	switch (uMsg) {
	case WM_INITDIALOG:
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_FILEOPENDIR), _MAX_PATH - 1);
		// ダイアログデータの設定 Edit
		SetData(hwndDlg);
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// ユーザーがエディット コントロールに入力できるテキストの長さを制限する
		return TRUE;
		
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	// 通知コード
		wID			= LOWORD(wParam);	// 項目ID､ コントロールID､ またはアクセラレータID
		switch (wNotifyCode) {
		// ボタン／チェックボックスがクリックされた
		case BN_CLICKED:
			switch (wID) {
			case IDC_CHECK_DRAGDROP:	// タスクトレイを使う
				if (DlgButton_IsChecked(hwndDlg, IDC_CHECK_DRAGDROP)) {
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_DROPSOURCE), TRUE);
				}else {
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_DROPSOURCE), FALSE);
				}
				return TRUE;
			case IDC_RADIO_CURDIR:
			case IDC_RADIO_MRUDIR:
			case IDC_RADIO_SELDIR:
				EnableEditPropInput(hwndDlg);
				return TRUE;
			case IDC_BUTTON_FILEOPENDIR:
				{
					TCHAR szMetaPath[_MAX_PATH];
					TCHAR szPath[_MAX_PATH];
					::DlgItem_GetText(hwndDlg, IDC_EDIT_FILEOPENDIR, szMetaPath, _countof(szMetaPath));
					FileNameManager::ExpandMetaToFolder(szMetaPath, szPath, _countof(szPath));
					if (SelectDir(hwndDlg, LS(STR_PROPEDIT_SELECT_DIR), szPath, szPath)) {
						NativeT mem(szPath);
						mem.Replace(_T("%"), _T("%%"));
						::DlgItem_SetText(hwndDlg, IDC_EDIT_FILEOPENDIR, mem.GetStringPtr());
					}
				}
				return TRUE;
			}
			break;
		}
		break;

	case WM_NOTIFY:
		pNMHDR = (NMHDR*) lParam;
		switch (pNMHDR->code) {
		case PSN_HELP:
			OnHelp(hwndDlg, IDD_PROP_EDIT);
			return TRUE;
		case PSN_KILLACTIVE:
			DEBUG_TRACE(_T("Edit PSN_KILLACTIVE\n"));
			// ダイアログデータの取得 Edit
			GetData(hwndDlg);
			return TRUE;

		case PSN_SETACTIVE: //@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			nPageNum = ID_PROPCOM_PAGENUM_EDIT;
			return TRUE;
		}
		break;	// WM_NOTIFY

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids);	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		// NOTREACHED
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids);	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}


// ダイアログデータの設定
void PropEdit::SetData(HWND hwndDlg)
{
	auto& csEdit = common.edit;
	// ドラッグ & ドロップ編集
	::CheckDlgButton(hwndDlg, IDC_CHECK_DRAGDROP, csEdit.bUseOLE_DragDrop);
	if (DlgButton_IsChecked(hwndDlg, IDC_CHECK_DRAGDROP)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_DROPSOURCE), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_DROPSOURCE), FALSE);
	}

	// DropSource
	::CheckDlgButton(hwndDlg, IDC_CHECK_DROPSOURCE, csEdit.bUseOLE_DropSource);

	// 折り返し行に改行を付けてコピー
	::CheckDlgButton(hwndDlg, IDC_CHECK_ADDCRLFWHENCOPY, csEdit.bAddCRLFWhenCopy ? BST_CHECKED : BST_UNCHECKED);

	// コピーしたら選択解除
	::CheckDlgButton(hwndDlg, IDC_CHECK_COPYnDISABLESELECTEDAREA, csEdit.bCopyAndDisablSelection);

	// 選択なしでコピーを可能にする	// 2007.11.18 ryoji
	::CheckDlgButton(hwndDlg, IDC_CHECK_bEnableNoSelectCopy, csEdit.bEnableNoSelectCopy);

	// ラインモード貼り付けを可能にする	// 2007.10.08 ryoji
	::CheckDlgButton(hwndDlg, IDC_CHECK_bEnableLineModePaste, csEdit.bEnableLineModePaste ? BST_CHECKED : BST_UNCHECKED);

	// 改行は上書きしない
	::CheckDlgButton(hwndDlg, IDC_CHECK_bNotOverWriteCRLF, csEdit.bNotOverWriteCRLF);

	// 文字幅に合わせてスペースを詰める
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_bOverWriteFixMode, csEdit.bOverWriteFixMode);

	// 矩形入力で選択範囲を削除する
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_bOverWriteBoxDelete, csEdit.bOverWriteFixMode);

	// URLがクリックされたら選択するか	// 2007.02.11 genta このページへ移動
	::CheckDlgButton(hwndDlg, IDC_CHECK_bSelectClickedURL, csEdit.bSelectClickedURL);

	// 改行コードを変換して貼り付ける	// 2009.02.28 salarm
	::CheckDlgButton(hwndDlg, IDC_CHECK_CONVERTEOLPASTE, csEdit.bConvertEOLPaste ? BST_CHECKED : BST_UNCHECKED);

	// ファイルダイアログの初期位置
	if (csEdit.eOpenDialogDir == OPENDIALOGDIR_CUR) {
		::CheckDlgButton(hwndDlg, IDC_RADIO_CURDIR, TRUE);
	}
	if (csEdit.eOpenDialogDir == OPENDIALOGDIR_MRU) {
		::CheckDlgButton(hwndDlg, IDC_RADIO_MRUDIR, TRUE);
	}
	if (csEdit.eOpenDialogDir == OPENDIALOGDIR_SEL) {
		::CheckDlgButton(hwndDlg, IDC_RADIO_SELDIR, TRUE);
	}
	::DlgItem_SetText(hwndDlg, IDC_EDIT_FILEOPENDIR, csEdit.openDialogSelDir);

	// 改行コードNEL,PS,LSを有効にする
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_ENABLEEXTEOL, csEdit.bEnableExtEol);
	// 矩形選択移動で選択をロックする
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_BOXSELECTLOCK, csEdit.bBoxSelectLock);

	EnableEditPropInput(hwndDlg);
}


// ダイアログデータの取得
int PropEdit::GetData(HWND hwndDlg)
{
	auto& csEdit = common.edit;
	
	// ドラッグ & ドロップ編集
	csEdit.bUseOLE_DragDrop = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DRAGDROP);
	// DropSource
	csEdit.bUseOLE_DropSource = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DROPSOURCE);

	// 折り返し行に改行を付けてコピー
	csEdit.bAddCRLFWhenCopy = DlgButton_IsChecked(hwndDlg, IDC_CHECK_ADDCRLFWHENCOPY);

	// コピーしたら選択解除
	csEdit.bCopyAndDisablSelection = DlgButton_IsChecked(hwndDlg, IDC_CHECK_COPYnDISABLESELECTEDAREA);

	// 選択なしでコピーを可能にする	// 2007.11.18 ryoji
	csEdit.bEnableNoSelectCopy = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bEnableNoSelectCopy);

	// ラインモード貼り付けを可能にする	// 2007.10.08 ryoji
	csEdit.bEnableLineModePaste = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bEnableLineModePaste);

	// 改行は上書きしない
	csEdit.bNotOverWriteCRLF = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bNotOverWriteCRLF);

	// 文字幅に合わせてスペースを詰める
	csEdit.bOverWriteFixMode = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bOverWriteFixMode);

	// 矩形入力で選択範囲を削除する
	csEdit.bOverWriteBoxDelete = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bOverWriteBoxDelete);

	// URLがクリックされたら選択するか	// 2007.02.11 genta このページへ移動
	csEdit.bSelectClickedURL = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bSelectClickedURL);

	//	改行コードを変換して貼り付ける	// 2009.02.28 salarm
	csEdit.bConvertEOLPaste = DlgButton_IsChecked(hwndDlg, IDC_CHECK_CONVERTEOLPASTE);

	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_CURDIR)) {
		csEdit.eOpenDialogDir = OPENDIALOGDIR_CUR;
	}
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_MRUDIR)) {
		csEdit.eOpenDialogDir = OPENDIALOGDIR_MRU;
	}
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_SELDIR)) {
		csEdit.eOpenDialogDir = OPENDIALOGDIR_SEL;
	}
	::DlgItem_GetText(hwndDlg, IDC_EDIT_FILEOPENDIR, csEdit.openDialogSelDir, _countof2(csEdit.openDialogSelDir));

	// 改行コードNEL,PS,LSを有効にする
	csEdit.bEnableExtEol = DlgButton_IsChecked(hwndDlg, IDC_CHECK_ENABLEEXTEOL);
	// 矩形選択移動で選択をロックする
	csEdit.bBoxSelectLock = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BOXSELECTLOCK);

	return TRUE;
}

/*!	チェック状態に応じてダイアログボックス要素のEnable/Disableを
	適切に設定する

	@param hwndDlg プロパティシートのWindow Handle
*/
void PropEdit::EnableEditPropInput(HWND hwndDlg)
{
	// 指定フォルダ
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_SELDIR)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_FILEOPENDIR), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_FILEOPENDIR), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_FILEOPENDIR), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_FILEOPENDIR), FALSE);
	}
}


