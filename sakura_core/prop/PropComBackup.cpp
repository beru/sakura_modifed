/*!	@file
	@brief 共通設定ダイアログボックス、「バックアップ」ページ
*/
#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids[] = {	//10000
	IDC_BUTTON_BACKUP_FOLDER_REF,	HIDC_BUTTON_BACKUP_FOLDER_REF,	// バックアップフォルダ参照
	IDC_CHECK_BACKUP,				HIDC_CHECK_BACKUP,				// バックアップの作成
	IDC_CHECK_BACKUP_YEAR,			HIDC_CHECK_BACKUP_YEAR,			// バックアップファイル名（西暦年）
	IDC_CHECK_BACKUP_MONTH,			HIDC_CHECK_BACKUP_MONTH,		// バックアップファイル名（月）
	IDC_CHECK_BACKUP_DAY,			HIDC_CHECK_BACKUP_DAY,			// バックアップファイル名（日）
	IDC_CHECK_BACKUP_HOUR,			HIDC_CHECK_BACKUP_HOUR,			// バックアップファイル名（時）
	IDC_CHECK_BACKUP_MIN,			HIDC_CHECK_BACKUP_MIN,			// バックアップファイル名（分）
	IDC_CHECK_BACKUP_SEC,			HIDC_CHECK_BACKUP_SEC,			// バックアップファイル名（秒）
	IDC_CHECK_BACKUPDIALOG,			HIDC_CHECK_BACKUPDIALOG,		// 作成前に確認
	IDC_CHECK_BACKUPFOLDER,			HIDC_CHECK_BACKUPFOLDER,		// 指定フォルダに作成
	IDC_CHECK_BACKUP_FOLDER_RM,		HIDC_CHECK_BACKUP_FOLDER_RM,	// 指定フォルダに作成(リムーバブルメディアのみ)
	IDC_CHECK_BACKUP_DUSTBOX,		HIDC_CHECK_BACKUP_DUSTBOX,		// バックアップファイルをごみ箱に放り込む
	IDC_EDIT_BACKUPFOLDER,			HIDC_EDIT_BACKUPFOLDER,			// 保存フォルダ名
	IDC_EDIT_BACKUP_3,				HIDC_EDIT_BACKUP_3,				// 世代数
	IDC_RADIO_BACKUP_TYPE1,			HIDC_RADIO_BACKUP_TYPE1,		// バックアップの種類（拡張子）
	IDC_RADIO_BACKUP_TYPE3,			HIDC_RADIO_BACKUP_TYPE3NEWHID,	// バックアップの種類（連番）
	IDC_RADIO_BACKUP_DATETYPE1,		HIDC_RADIO_BACKUP_DATETYPE1,	// 付加する日時の種類（作成日時）
	IDC_RADIO_BACKUP_DATETYPE2,		HIDC_RADIO_BACKUP_DATETYPE2,	// 付加する日時の種類（更新日時）
	IDC_SPIN_BACKUP_GENS,			HIDC_EDIT_BACKUP_3,				// 保存する世代数のスピン
	IDC_CHECK_BACKUP_RETAINEXT,		HIDC_CHECK_BACKUP_RETAINEXT,	// 元の拡張子を保存
	IDC_CHECK_BACKUP_ADVANCED,		HIDC_CHECK_BACKUP_ADVANCED,		// 詳細設定
	IDC_EDIT_BACKUPFILE,			HIDC_EDIT_BACKUPFILE,			// 詳細設定のエディットボックス
	IDC_RADIO_BACKUP_DATETYPE1A,	HIDC_RADIO_BACKUP_DATETYPE1A,	// 付加する日時の種類（作成日時）※詳細設定ON用
	IDC_RADIO_BACKUP_DATETYPE2A,	HIDC_RADIO_BACKUP_DATETYPE2A,	// 付加する日時の種類（更新日時）※詳細設定ON用
//	IDC_STATIC,						-1,
	0, 0
};

/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK PropBackup::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropBackup::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}


// メッセージ処理
INT_PTR PropBackup::DispatchEvent(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	WORD		wNotifyCode;
	WORD		wID;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
	int			nVal;

	auto& csBackup = common.backup;

	switch (uMsg) {
	case WM_INITDIALOG:
		// ダイアログデータの設定 Backup
		SetData(hwndDlg);
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// ユーザーがエディット コントロールに入力できるテキストの長さを制限する
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_BACKUPFOLDER), _countof2(csBackup.szBackUpFolder) - 1 - 1);
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_BACKUPFILE), _countof2(csBackup.szBackUpPathAdvanced) - 1 - 1);
		return TRUE;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch (idCtrl) {
		default:
			switch (pNMHDR->code) {
			case PSN_HELP:
				OnHelp(hwndDlg, IDD_PROP_BACKUP);
				return TRUE;
			case PSN_KILLACTIVE:
				// ダイアログデータの取得 Backup
				GetData(hwndDlg);
				return TRUE;
			case PSN_SETACTIVE:
				nPageNum = ID_PROPCOM_PAGENUM_BACKUP;
				return TRUE;
			}
			break;

		case IDC_SPIN_BACKUP_GENS:
			// バックアップファイルの世代数
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_BACKUP_3, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 1) {
				nVal = 1;
			}
			if (nVal > 99) {
				nVal = 99;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_BACKUP_3, nVal, FALSE);
			return TRUE;
		}
		break;

	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	// 通知コード
		wID			= LOWORD(wParam);	// 項目ID､ コントロールID､ またはアクセラレータID
		switch (wNotifyCode) {
		// ボタン／チェックボックスがクリックされた
		case BN_CLICKED:
			switch (wID) {
			case IDC_RADIO_BACKUP_TYPE1:
			case IDC_RADIO_BACKUP_TYPE3:
			case IDC_CHECK_BACKUP:
			case IDC_CHECK_BACKUPFOLDER:
			case IDC_CHECK_AUTOSAVE:
			case IDC_RADIO_BACKUP_DATETYPE1:
			case IDC_RADIO_BACKUP_DATETYPE2:
			case IDC_CHECK_BACKUP_ADVANCED:
				GetData(hwndDlg);
				UpdateBackupFile(hwndDlg);
				EnableBackupInput(hwndDlg);
				return TRUE;
			case IDC_BUTTON_BACKUP_FOLDER_REF:	// フォルダ参照
				{
					// バックアップを作成するフォルダ
					TCHAR szFolder[_MAX_PATH];
					::DlgItem_GetText(hwndDlg, IDC_EDIT_BACKUPFOLDER, szFolder, _countof(szFolder));

					if (SelectDir(hwndDlg, LS(STR_PROPCOMBK_SEL_FOLDER), szFolder, szFolder)) {
						_tcscpy(csBackup.szBackUpFolder, szFolder);
						::DlgItem_SetText(hwndDlg, IDC_EDIT_BACKUPFOLDER, csBackup.szBackUpFolder);
					}
					UpdateBackupFile(hwndDlg);
				}
				return TRUE;
			default:
				GetData(hwndDlg);
				UpdateBackupFile(hwndDlg);
			}
			break;	// BN_CLICKED
		case EN_CHANGE:
			switch (wID) {
			case IDC_EDIT_BACKUPFOLDER:
				// 後ろに\が追加されるので，1文字余裕をみる必要がある．
				::DlgItem_GetText(hwndDlg, IDC_EDIT_BACKUPFOLDER, csBackup.szBackUpFolder, _countof2(csBackup.szBackUpFolder) - 1);
				UpdateBackupFile(hwndDlg);
				break;
			}
			break;	// EN_CHANGE
		}
		break;	// WM_COMMAND

	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids);
		}
		return TRUE;
		// NOTREACHED
		//break;

	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids);
		return TRUE;

	}
	return FALSE;
}


/*! ダイアログデータの設定 */
void PropBackup::SetData(HWND hwndDlg)
{
//	BOOL	bRet;

//	BOOL	bGrepExitConfirm;	// Grepモードで保存確認するか

	auto& csBackup = common.backup;

	// バックアップの作成
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUP, csBackup.bBackUp);
	// バックアップの作成前に確認
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUPDIALOG, csBackup.bBackUpDialog);
	// バックアップファイル名のタイプ 1=(.bak) 2=*_日付.*
	switch (csBackup.GetBackupType()) {
	case 2:
		::CheckDlgButton(hwndDlg, IDC_RADIO_BACKUP_DATETYPE1, 1);	// 付加する日付のタイプ(現時刻)
		break;
	case 3:
	case 6:
		::CheckDlgButton(hwndDlg, IDC_RADIO_BACKUP_TYPE3, 1);
		break;
	case 4:
		::CheckDlgButton(hwndDlg, IDC_RADIO_BACKUP_DATETYPE2, 1);	// 付加する日付のタイプ(前回の保存時刻)
		break;
	case 5:
	case 1:
	default:
		::CheckDlgButton(hwndDlg, IDC_RADIO_BACKUP_TYPE1, 1);
		break;
	}
	
	::CheckDlgButton(hwndDlg, IDC_CHECK_BACKUP_RETAINEXT,
		(csBackup.GetBackupType() == 5 || csBackup.GetBackupType() == 6) ? 1 : 0
	);

	// バックアップファイル名：日付の年
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUP_YEAR, csBackup.GetBackupOpt(BKUP_YEAR));
	// バックアップファイル名：日付の月
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUP_MONTH, csBackup.GetBackupOpt(BKUP_MONTH));
	// バックアップファイル名：日付の日
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUP_DAY, csBackup.GetBackupOpt(BKUP_DAY));
	// バックアップファイル名：日付の時
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUP_HOUR, csBackup.GetBackupOpt(BKUP_HOUR));
	// バックアップファイル名：日付の分
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUP_MIN, csBackup.GetBackupOpt(BKUP_MIN));
	// バックアップファイル名：日付の秒
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUP_SEC, csBackup.GetBackupOpt(BKUP_SEC));

	// 指定フォルダにバックアップを作成する
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUPFOLDER, csBackup.bBackUpFolder);
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUP_FOLDER_RM, csBackup.bBackUpFolderRM);

	// バックアップを作成するフォルダ
	::DlgItem_SetText(hwndDlg, IDC_EDIT_BACKUPFOLDER, csBackup.szBackUpFolder);

	// バックアップファイルをごみ箱に放り込む	
	::CheckDlgButton(hwndDlg, IDC_CHECK_BACKUP_DUSTBOX, csBackup.bBackUpDustBox ? BST_CHECKED : BST_UNCHECKED);

	// バックアップ先フォルダを詳細設定する
	::CheckDlgButton(hwndDlg, IDC_CHECK_BACKUP_ADVANCED, csBackup.bBackUpPathAdvanced ? BST_CHECKED : BST_UNCHECKED);

	// バックアップを作成するフォルダの詳細設定
	::DlgItem_SetText(hwndDlg, IDC_EDIT_BACKUPFILE, csBackup.szBackUpPathAdvanced);

	// バックアップを作成するフォルダの詳細設定
	switch (csBackup.GetBackupTypeAdv()) {
	case 2:
		::CheckDlgButton(hwndDlg, IDC_RADIO_BACKUP_DATETYPE1A, 1);	// 付加する日付のタイプ(現時刻)
		break;
	case 4:
		::CheckDlgButton(hwndDlg, IDC_RADIO_BACKUP_DATETYPE2A, 1);	// 付加する日付のタイプ(前回の保存時刻)
		break;
	default:
		::CheckDlgButton(hwndDlg, IDC_RADIO_BACKUP_DATETYPE1A, 1);
		break;
	}

	int nN = csBackup.GetBackupCount();
	nN = nN < 1  ?  1 : nN;
	nN = nN > 99 ? 99 : nN;

	::SetDlgItemInt(hwndDlg, IDC_EDIT_BACKUP_3, nN, FALSE);

	UpdateBackupFile(hwndDlg);

	EnableBackupInput(hwndDlg);
	return;
}


// ダイアログデータの取得
int PropBackup::GetData(HWND hwndDlg)
{
	auto& csBackup = common.backup;

	// バックアップの作成
	csBackup.bBackUp = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP);
	// バックアップの作成前に確認
	csBackup.bBackUpDialog = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUPDIALOG);
//	// 指定フォルダにバックアップを作成する
//	csBackup.bBackUpFolder = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUPFOLDER);

	// バックアップファイル名のタイプ 1=(.bak) 2=*_日付.*
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_TYPE1)) {
		if (DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_RETAINEXT)) {
			csBackup.SetBackupType(5);
		}else {
			csBackup.SetBackupType(1);
		}
	}
		// 日付のタイプ
		if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_DATETYPE1)) {
			csBackup.SetBackupType(2);	// 現時刻
		}
		if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_DATETYPE2)) {
			csBackup.SetBackupType(4);	// 前回の保存時刻
		}

	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_TYPE3)) {
		if (DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_RETAINEXT)) {
			csBackup.SetBackupType(6);
		}else {
			csBackup.SetBackupType(3);
		}
	}

	// バックアップファイル名：日付の年
	csBackup.SetBackupOpt(BKUP_YEAR, DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_YEAR));
	// バックアップファイル名：日付の月
	csBackup.SetBackupOpt(BKUP_MONTH, DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_MONTH));
	// バックアップファイル名：日付の日
	csBackup.SetBackupOpt(BKUP_DAY, DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_DAY));
	// バックアップファイル名：日付の時
	csBackup.SetBackupOpt(BKUP_HOUR, DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_HOUR));
	// バックアップファイル名：日付の分
	csBackup.SetBackupOpt(BKUP_MIN, DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_MIN));
	// バックアップファイル名：日付の秒
	csBackup.SetBackupOpt(BKUP_SEC, DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_SEC));

	// 指定フォルダにバックアップを作成する
	csBackup.bBackUpFolder = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUPFOLDER);
	csBackup.bBackUpFolderRM = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_FOLDER_RM);

	// バックアップを作成するフォルダ
	::DlgItem_GetText(hwndDlg, IDC_EDIT_BACKUPFOLDER, csBackup.szBackUpFolder, _countof2(csBackup.szBackUpFolder) - 1);

	// バックアップファイルをごみ箱に放り込む
	csBackup.bBackUpDustBox = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_DUSTBOX);

	// 指定フォルダにバックアップを作成する詳細設定
	csBackup.bBackUpPathAdvanced = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_ADVANCED);
	// バックアップを作成するフォルダ
	::DlgItem_GetText(hwndDlg, IDC_EDIT_BACKUPFILE, csBackup.szBackUpPathAdvanced, _countof2(csBackup.szBackUpPathAdvanced) - 1);

	// 詳細設定の日付のタイプ
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_DATETYPE1A)) {
		csBackup.SetBackupTypeAdv(2);	// 現時刻
	}
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_DATETYPE2A)) {
		csBackup.SetBackupTypeAdv(4);	// 前回の保存時刻
	}

	//	世代数の取得
	int	 nN;
	nN = ::GetDlgItemInt(hwndDlg, IDC_EDIT_BACKUP_3, NULL, FALSE);

	nN = nN < 1  ?  1 : nN;
	nN = nN > 99 ? 99 : nN;
	csBackup.SetBackupCount(nN);

	return TRUE;
}

/*!	チェック状態に応じてダイアログボックス要素のEnable/Disableを適切に設定する */
static inline
void ShowEnable(
	HWND hWnd,
	BOOL bShow,
	BOOL bEnable
	)
{
	::ShowWindow(hWnd, bShow? SW_SHOW: SW_HIDE);
	::EnableWindow(hWnd, bEnable && bShow);		// bShow=false,bEnable=trueの場合ショートカットキーが変な動きをするので
}

void PropBackup::EnableBackupInput(HWND hwndDlg)
{
	#define SHOWENABLE(id, show, enable) ShowEnable(::GetDlgItem(hwndDlg, id), show, enable)

	bool bBackup = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP);
	bool bAdvanced = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_ADVANCED);
	bool bType1 = DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_TYPE1);
	//bool bType2 = DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_TYPE2);
	bool bType3 = DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_TYPE3);
	bool bDate1 = DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_DATETYPE1);
	bool bDate2 = DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_DATETYPE2);
	bool bFolder = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUPFOLDER);

	SHOWENABLE(IDC_CHECK_BACKUP_ADVANCED,	TRUE, bBackup);
	SHOWENABLE(IDC_RADIO_BACKUP_TYPE1,		!bAdvanced, bBackup);
	SHOWENABLE(IDC_RADIO_BACKUP_TYPE3,		!bAdvanced, bBackup);
	SHOWENABLE(IDC_RADIO_BACKUP_DATETYPE1,	!bAdvanced, bBackup);
	SHOWENABLE(IDC_RADIO_BACKUP_DATETYPE2,	!bAdvanced, bBackup);
	SHOWENABLE(IDC_LABEL_BACKUP_3,			!bAdvanced, bBackup && bType3);
	SHOWENABLE(IDC_EDIT_BACKUP_3,			!bAdvanced, bBackup && bType3);
	SHOWENABLE(IDC_SPIN_BACKUP_GENS,		!bAdvanced, bBackup && bType3);
	SHOWENABLE(IDC_CHECK_BACKUP_RETAINEXT,	!bAdvanced, bBackup && (bType1 || bType3));
	SHOWENABLE(IDC_CHECK_BACKUP_YEAR,		!bAdvanced, bBackup && (bDate1 || bDate2));
	SHOWENABLE(IDC_CHECK_BACKUP_MONTH,		!bAdvanced, bBackup && (bDate1 || bDate2));
	SHOWENABLE(IDC_CHECK_BACKUP_DAY,		!bAdvanced, bBackup && (bDate1 || bDate2));
	SHOWENABLE(IDC_CHECK_BACKUP_HOUR,		!bAdvanced, bBackup && (bDate1 || bDate2));
	SHOWENABLE(IDC_CHECK_BACKUP_MIN,		!bAdvanced, bBackup && (bDate1 || bDate2));
	SHOWENABLE(IDC_CHECK_BACKUP_SEC,		!bAdvanced, bBackup && (bDate1 || bDate2));

	// 詳細設定
	SHOWENABLE(IDC_EDIT_BACKUPFILE,			TRUE, bBackup && bAdvanced);
//	SHOWENABLE(IDC_LABEL_BACKUP_HELP,		bAdvanced, bBackup);	// 不可視のまま放置（他コントロール隠しの方式は廃止）
	SHOWENABLE(IDC_LABEL_BACKUP_HELP2,		bAdvanced, bBackup);
	SHOWENABLE(IDC_RADIO_BACKUP_DATETYPE1A,	bAdvanced, bBackup);
	SHOWENABLE(IDC_RADIO_BACKUP_DATETYPE2A,	bAdvanced, bBackup);

	SHOWENABLE(IDC_CHECK_BACKUPFOLDER,			TRUE, bBackup);
	SHOWENABLE(IDC_LABEL_BACKUP_4,				TRUE, bBackup && bFolder);
	SHOWENABLE(IDC_CHECK_BACKUP_FOLDER_RM,		TRUE, bBackup && bFolder);
	SHOWENABLE(IDC_EDIT_BACKUPFOLDER,			TRUE, bBackup && bFolder);
	SHOWENABLE(IDC_BUTTON_BACKUP_FOLDER_REF,	TRUE, bBackup && bFolder);
	SHOWENABLE(IDC_CHECK_BACKUP_DUSTBOX,		TRUE, bBackup);

	// 作成前に確認
	SHOWENABLE(IDC_CHECK_BACKUPDIALOG,		TRUE, bBackup);

	#undef SHOWENABLE
}


/*!	バックアップファイルの詳細設定エディットボックスを適切に更新する
	@note 詳細設定切り替え時のデフォルトをオプションに合わせるため、
		szBackUpPathAdvanced を更新する
*/
void PropBackup::UpdateBackupFile(HWND hwndDlg)	//	バックアップファイルの詳細設定
{
	wchar_t temp[MAX_PATH];
	auto& csBackup = common.backup;
	// バックアップを作成するファイル
	if (!csBackup.bBackUp) {
		temp[0] = 0;
	}else {
		if (csBackup.bBackUpFolder) {
			temp[0] = 0;
		}else if (csBackup.bBackUpDustBox) {
			auto_sprintf_s(temp, LTEXT("%ls\\"), LSW(STR_PROPCOMBK_DUSTBOX));
		}else {
			auto_sprintf_s(temp, LTEXT(".\\"));
		}

		switch (csBackup.GetBackupType()) {
		case 1: // .bak
			wcscat(temp, LTEXT("$0.bak"));
			break;
		case 5: // .*.bak
			wcscat(temp, LTEXT("$0.*.bak"));
			break;
		case 3: // .b??
			wcscat(temp, LTEXT("$0.b??"));
			break;
		case 6: // .*.b??
			wcscat(temp, LTEXT("$0.*.b??"));
			break;
		case 2:	//	日付，時刻
		case 4:	//	日付，時刻
			wcscat(temp, LTEXT("$0_"));

			if (csBackup.GetBackupOpt(BKUP_YEAR)) {	// バックアップファイル名：日付の年
				wcscat(temp, LTEXT("%Y"));
			}
			if (csBackup.GetBackupOpt(BKUP_MONTH)) {	// バックアップファイル名：日付の月
				wcscat(temp, LTEXT("%m"));
			}
			if (csBackup.GetBackupOpt(BKUP_DAY)) {	// バックアップファイル名：日付の日
				wcscat(temp, LTEXT("%d"));
			}
			if (csBackup.GetBackupOpt(BKUP_HOUR)) {	// バックアップファイル名：日付の時
				wcscat(temp, LTEXT("%H"));
			}
			if (csBackup.GetBackupOpt(BKUP_MIN)) {	// バックアップファイル名：日付の分
				wcscat(temp, LTEXT("%M"));
			}
			if (csBackup.GetBackupOpt(BKUP_SEC)) {	// バックアップファイル名：日付の秒
				wcscat(temp, LTEXT("%S"));
			}

			wcscat(temp, LTEXT(".*"));
			break;
		default:
			break;
		}
	}
	if (!csBackup.bBackUpPathAdvanced) {	// 詳細設定モードでないときだけ自動更新する
		auto_sprintf(csBackup.szBackUpPathAdvanced, _T("%ls"), temp);
		::DlgItem_SetText(hwndDlg, IDC_EDIT_BACKUPFILE, csBackup.szBackUpPathAdvanced);
	}
	return;
}

