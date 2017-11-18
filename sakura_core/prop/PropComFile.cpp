/*! @file
	@brief 共通設定ダイアログボックス、「書式」ページ
*/
#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "PropertyManager.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"


static const DWORD p_helpids[] = {	//01310
	IDC_COMBO_FILESHAREMODE,				HIDC_COMBO_FILESHAREMODE,				// 排他制御
	IDC_CHECK_bCheckFileTimeStamp,			HIDC_CHECK_bCheckFileTimeStamp,			// 更新の監視
	IDC_EDIT_AUTOLOAD_DELAY,				HIDC_EDIT_AUTOLOAD_DELAY,				// 自動読込時遅延
	IDC_SPIN_AUTOLOAD_DELAY,				HIDC_EDIT_AUTOLOAD_DELAY,
	IDC_CHECK_bUneditableIfUnwritable,		HIDC_CHECK_bUneditableIfUnwritable,		// 上書き禁止検出時は編集禁止にする
	IDC_CHECK_ENABLEUNMODIFIEDOVERWRITE,	HIDC_CHECK_ENABLEUNMODIFIEDOVERWRITE,	// 無変更でも上書き
	IDC_CHECK_AUTOSAVE,						HIDC_CHECK_AUTOSAVE,					// 自動的に保存
	IDC_CHECK_bDropFileAndClose,			HIDC_CHECK_bDropFileAndClose,			// 閉じて開く
	IDC_CHECK_RestoreCurPosition,			HIDC_CHECK_RestoreCurPosition,			// カーソル位置の復元
	IDC_CHECK_AutoMIMEDecode,				HIDC_CHECK_AutoMIMEDecode,				// MIMEデコード
	IDC_EDIT_AUTOBACKUP_INTERVAL,			HIDC_EDIT_AUTOBACKUP_INTERVAL,			// 自動保存間隔
	IDC_EDIT_nDropFileNumMax,				HIDC_EDIT_nDropFileNumMax,				// ファイルドロップ最大数
	IDC_SPIN_AUTOBACKUP_INTERVAL,			HIDC_EDIT_AUTOBACKUP_INTERVAL,
	IDC_SPIN_nDropFileNumMax,				HIDC_EDIT_nDropFileNumMax,
	IDC_CHECK_RestoreBookmarks,				HIDC_CHECK_RestoreBookmarks,			// 2002.01.16 hor ブックマークの復元
	IDC_CHECK_QueryIfCodeChange,			HIDC_CHECK_QueryIfCodeChange,			// 前回と異なる文字コードのとき問い合わせを行う	// 2006.08.06 ryoji
	IDC_CHECK_AlertIfFileNotExist,			HIDC_CHECK_AlertIfFileNotExist,			// 開こうとしたファイルが存在しないとき警告する	// 2006.08.06 ryoji
	IDC_CHECK_ALERT_IF_LARGEFILE,			HIDC_CHECK_ALERT_IF_LARGEFILE,			// 開こうとしたファイルが大きい場合に警告する
	IDC_CHECK_NoFilterSaveNew,				HIDC_CHECK_NoFilterSaveNew,				// 新規から保存時は全ファイル表示	// 2006.11.16 ryoji
	IDC_CHECK_NoFilterSaveFile,				HIDC_CHECK_NoFilterSaveFile,			// 新規以外から保存時は全ファイル表示	// 2006.11.16 ryoji
//	IDC_STATIC,								-1,
	0, 0
};

TYPE_NAME_ID<FileShareMode> ShareModeArr[] = {
	{ FileShareMode::NonExclusive,	STR_EXCLU_NO_EXCLUSIVE },	// _T("しない") },
	{ FileShareMode::DenyWrite,		STR_EXCLU_DENY_READWRITE },	// _T("上書きを禁止する") },
	{ FileShareMode::DenyReadWrite,	STR_EXCLU_DENY_WRITE },		// _T("読み書きを禁止する") },
};

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK PropFile::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropFile::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}
// To Here Jun. 2, 2001 genta

// ファイルページ メッセージ処理
INT_PTR PropFile::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,	// message
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
	WORD		wNotifyCode;
	WORD		wID;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
//	int			nVal;
	int			nVal;	// Sept.21, 2000 JEPRO スピン要素を加えたので復活させた
//	char		szFolder[_MAX_PATH];

	switch (uMsg) {
	case WM_INITDIALOG:
		// ダイアログデータの設定 File
		SetData(hwndDlg);
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		return TRUE;
//****	From Here Sept. 21, 2000 JEPRO ダイアログ要素にスピンを入れるので以下のWM_NOTIFYをコメントアウトにし下に修正を置いた
//	case WM_NOTIFY:
//		idCtrl = (int)wParam;
//		pNMHDR = (NMHDR*)lParam;
//		pMNUD  = (NM_UPDOWN*)lParam;
////		switch (idCtrl) {
////		default:
//			switch (pNMHDR->code) {
//			case PSN_HELP:
//				OnHelp(hwndDlg, IDD_PROP_FILE);
//				return TRUE;
//			case PSN_KILLACTIVE:
////				MYTRACE(_T("p2 PSN_KILLACTIVE\n"));
//				// ダイアログデータの取得 p2
//				GetData_p2(hwndDlg);
//				return TRUE;
//			}
////		}
//		break;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch (idCtrl) {
		default:
			switch (pNMHDR->code) {
			case PSN_HELP:
				OnHelp(hwndDlg, IDD_PROP_FILE);
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE(_T("File PSN_KILLACTIVE\n"));
				// ダイアログデータの取得 File
				GetData(hwndDlg);
				return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			case PSN_SETACTIVE:
				nPageNum = ID_PROPCOM_PAGENUM_FILE;
				return TRUE;
			}
			break;
		case IDC_SPIN_AUTOLOAD_DELAY:
			// 自動読込時遅延
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_AUTOLOAD_DELAY, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 0) {
				nVal = 0;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_AUTOLOAD_DELAY, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_nDropFileNumMax:
			// 一度にドロップ可能なファイル数
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_nDropFileNumMax, NULL, FALSE);
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
			::SetDlgItemInt(hwndDlg, IDC_EDIT_nDropFileNumMax, nVal, FALSE);
			return TRUE;
//@@@ 2001.03.21 Start by MIK
			// NOTREACHED
//			break;
		case IDC_SPIN_AUTOBACKUP_INTERVAL:
			//  バックアップ間隔
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 1) {
				nVal = 1;
			}
			if (nVal > 35791) {
				nVal = 35791;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_ALERT_FILESIZE:
			// ファイルの警告サイズ
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_ALERT_FILESIZE, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else 
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 1) {
				nVal = 1;
			}
			if (nVal > 2048) {
				nVal = 2048;  // 最大 2GB まで
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_ALERT_FILESIZE, nVal, FALSE);
			return TRUE;
			// NOTREACHED
//			break;
//@@@ 2001.03.21 End by MIK
		}
//****	To Here Sept. 21, 2000 JEPRO ダイアログ要素にスピンを入れるのでWM_NOTIFYをコメントアウトにしその下に修正を置いた
		break;

	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	// 通知コード
		wID			= LOWORD(wParam);	// 項目ID､ コントロールID､ またはアクセラレータID

		if (wID == IDC_COMBO_FILESHAREMODE && wNotifyCode == CBN_SELCHANGE) {	// コンボボックスの選択変更
			EnableFilePropInput(hwndDlg);
			break;
		}

		switch (wNotifyCode) {
		// ボタン／チェックボックスがクリックされた
		case BN_CLICKED:
			switch (wID) {
			case IDC_CHECK_bCheckFileTimeStamp:	// 更新の監視
			case IDC_CHECK_bDropFileAndClose:// ファイルをドロップしたときは閉じて開く
			case IDC_CHECK_AUTOSAVE:
			case IDC_CHECK_ALERT_IF_LARGEFILE:
				EnableFilePropInput(hwndDlg);
				break;
			}
			break;
		}
		break;

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids);	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		// NOTREACHED
//		break;
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


/*! ファイルページ: ダイアログデータの設定
	共有メモリからデータを読み出して各コントロールに値を設定する。

	@par バックアップ世代数が妥当な値かどうかのチェックも行う。不適切な値の時は
	最も近い適切な値を設定する。

	@param hwndDlg プロパティページのWindow Handle
*/
void PropFile::SetData(HWND hwndDlg)
{
	auto& csFile = common.file;
	//--- File ---
	// ファイルの排他制御モード
	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_FILESHAREMODE);
	Combo_ResetContent(hwndCombo);
	size_t nSelPos = 0;
	for (size_t i=0; i<_countof(ShareModeArr); ++i) {
		Combo_InsertString(hwndCombo, (int)i, LS(ShareModeArr[i].nNameId));
		if (ShareModeArr[i].nMethod == csFile.nFileShareMode) {
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	// 更新の監視
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_bCheckFileTimeStamp, csFile.bCheckFileTimeStamp);

	// 自動読込時遅延
	::SetDlgItemInt(hwndDlg, IDC_EDIT_AUTOLOAD_DELAY, csFile.nAutoloadDelay, FALSE);

	// 上書き禁止検出時は編集禁止にする
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_bUneditableIfUnwritable, csFile.bUneditableIfUnwritable);

	// 無変更でも上書きするか
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_ENABLEUNMODIFIEDOVERWRITE, csFile.bEnableUnmodifiedOverwrite);

	// ファイルをドロップしたときは閉じて開く
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_bDropFileAndClose, csFile.bDropFileAndClose);
	// 一度にドロップ可能なファイル数
	::SetDlgItemInt(hwndDlg, IDC_EDIT_nDropFileNumMax, csFile.nDropFileNumMax, FALSE);

	//	From Here Aug. 21, 2000 genta
	//	自動保存の有効・無効
	::CheckDlgButton(hwndDlg, IDC_CHECK_AUTOSAVE, common.backup.IsAutoBackupEnabled());

	TCHAR buf[6];
	int nN = common.backup.GetAutoBackupInterval();
	nN = nN < 1  ?  1 : nN;
	nN = nN > 35791 ? 35791 : nN;

	auto_sprintf_s(buf, _T("%d"), nN);
	::DlgItem_SetText(hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL, buf);
	//	To Here Aug. 21, 2000 genta

	//	Oct. 27, 2000 genta	カーソル位置復元フラグ
	::CheckDlgButton(hwndDlg, IDC_CHECK_RestoreCurPosition, csFile.GetRestoreCurPosition());
	// 2002.01.16 hor ブックマーク復元フラグ
	::CheckDlgButton(hwndDlg, IDC_CHECK_RestoreBookmarks, csFile.GetRestoreBookmarks());
	//	Nov. 12, 2000 genta	MIME Decodeフラグ
	::CheckDlgButton(hwndDlg, IDC_CHECK_AutoMIMEDecode, csFile.GetAutoMIMEdecode());
	//	Oct. 03, 2004 genta 前回と異なる文字コードのときに問い合わせを行うかどうかのフラグ
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_QueryIfCodeChange, csFile.GetQueryIfCodeChange());
	//	Oct. 09, 2004 genta 開こうとしたファイルが存在しないとき警告するかどうかのフラグ
	::CheckDlgButton(hwndDlg, IDC_CHECK_AlertIfFileNotExist, csFile.GetAlertIfFileNotExist());
	//	ファイルサイズが大きい場合に警告を出す
	::CheckDlgButton(hwndDlg, IDC_CHECK_ALERT_IF_LARGEFILE, csFile.bAlertIfLargeFile);
	::SetDlgItemInt(hwndDlg, IDC_EDIT_ALERT_FILESIZE, csFile.nAlertFileSize, FALSE);

	// ファイル保存ダイアログのフィルタ設定	// 2006.11.16 ryoji
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_NoFilterSaveNew, csFile.bNoFilterSaveNew);	// 新規から保存時は全ファイル表示
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_NoFilterSaveFile, csFile.bNoFilterSaveFile);	// 新規以外から保存時は全ファイル表示

	EnableFilePropInput(hwndDlg);
	return;
}

/*! ファイルページ ダイアログデータの取得
	ダイアログボックスに設定されたデータを共有メモリに反映させる

	@par バックアップ世代数が妥当な値かどうかのチェックも行う。不適切な値の時は
	最も近い適切な値を設定する。

	@param hwndDlg プロパティページのWindow Handle
	@return 常にTRUE
*/
int PropFile::GetData(HWND hwndDlg)
{
	auto& csFile = common.file;

	// ファイルの排他制御モード
	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_FILESHAREMODE);
	int nSelPos = Combo_GetCurSel(hwndCombo);
	csFile.nFileShareMode = ShareModeArr[nSelPos].nMethod;

	// 更新の監視
	csFile.bCheckFileTimeStamp = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bCheckFileTimeStamp);

	// 自動読込時遅延
	csFile.nAutoloadDelay = ::GetDlgItemInt(hwndDlg, IDC_EDIT_AUTOLOAD_DELAY, NULL, FALSE);

	// 上書き禁止検出時は編集禁止にする
	csFile.bUneditableIfUnwritable = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bUneditableIfUnwritable);

	// 無変更でも上書きするか
	csFile.bEnableUnmodifiedOverwrite = DlgButton_IsChecked(hwndDlg, IDC_CHECK_ENABLEUNMODIFIEDOVERWRITE);

	// ファイルをドロップしたときは閉じて開く
	csFile.bDropFileAndClose = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bDropFileAndClose);
	// 一度にドロップ可能なファイル数
	csFile.nDropFileNumMax = ::GetDlgItemInt(hwndDlg, IDC_EDIT_nDropFileNumMax, NULL, FALSE);
	if (1 > csFile.nDropFileNumMax) {
		csFile.nDropFileNumMax = 1;
	}
	if (99 < csFile.nDropFileNumMax) {	// Sept. 21, 2000, JEPRO 16より大きいときに99と制限されていたのを修正(16→99と変更)
		csFile.nDropFileNumMax = 99;
	}

	//	From Here Aug. 16, 2000 genta
	//	自動保存を行うかどうか
	common.backup.EnableAutoBackup(DlgButton_IsChecked(hwndDlg, IDC_CHECK_AUTOSAVE));

	//	自動保存間隔の取得
	TCHAR szNumBuf[/*6*/ 7];	//@@@ 2001.03.21 by MIK
	::DlgItem_GetText(hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL, szNumBuf, /*5*/ 6);	//@@@ 2001.03.21 by MIK

	int nN;
	TCHAR* pDigit;
	for (nN=0, pDigit=szNumBuf; *pDigit!=_T('\0'); ++pDigit) {
		if (_T('0') <= *pDigit && *pDigit <= _T('9')) {
			nN = nN * 10 + *pDigit - _T('0');
		}else {
			break;
		}
	}
	nN = nN < 1  ?  1 : nN;
	nN = nN > 35791 ? 35791 : nN;
	common.backup.SetAutoBackupInterval(nN);

	//	To Here Aug. 16, 2000 genta

	//	Oct. 27, 2000 genta	カーソル位置復元フラグ
	csFile.SetRestoreCurPosition(DlgButton_IsChecked(hwndDlg, IDC_CHECK_RestoreCurPosition));
	// 2002.01.16 hor ブックマーク復元フラグ
	csFile.SetRestoreBookmarks(DlgButton_IsChecked(hwndDlg, IDC_CHECK_RestoreBookmarks));
	//	Nov. 12, 2000 genta	MIME Decodeフラグ
	csFile.SetAutoMIMEdecode(DlgButton_IsChecked(hwndDlg, IDC_CHECK_AutoMIMEDecode));
	//	Oct. 03, 2004 genta 前回と異なる文字コードのときに問い合わせを行うかどうかのフラグ
	csFile.SetQueryIfCodeChange(DlgButton_IsChecked(hwndDlg, IDC_CHECK_QueryIfCodeChange));
	//	Oct. 03, 2004 genta 前回と異なる文字コードのときに問い合わせを行うかどうかのフラグ
	csFile.SetAlertIfFileNotExist(DlgButton_IsChecked(hwndDlg, IDC_CHECK_AlertIfFileNotExist));
	// 開こうとしたファイルが大きい場合に警告する
	csFile.bAlertIfLargeFile = DlgButton_IsChecked(hwndDlg, IDC_CHECK_ALERT_IF_LARGEFILE);
	csFile.nAlertFileSize = ::GetDlgItemInt(hwndDlg, IDC_EDIT_ALERT_FILESIZE, NULL, FALSE);
	if (csFile.nAlertFileSize < 1) {
		csFile.nAlertFileSize = 1;
	}
	if (csFile.nAlertFileSize > 2048) {
		csFile.nAlertFileSize = 2048;
	}

	// ファイル保存ダイアログのフィルタ設定	// 2006.11.16 ryoji
	csFile.bNoFilterSaveNew = DlgButton_IsChecked(hwndDlg, IDC_CHECK_NoFilterSaveNew);	// 新規から保存時は全ファイル表示
	csFile.bNoFilterSaveFile = DlgButton_IsChecked(hwndDlg, IDC_CHECK_NoFilterSaveFile);	// 新規以外から保存時は全ファイル表示

	return TRUE;
}

//	From Here Aug. 21, 2000 genta
/*!	チェック状態に応じてダイアログボックス要素のEnable/Disableを
	適切に設定する

	@param hwndDlg プロパティシートのWindow Handle
*/
void PropFile::EnableFilePropInput(HWND hwndDlg)
{

	//	Drop時の動作
	if (DlgButton_IsChecked(hwndDlg, IDC_CHECK_bDropFileAndClose)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOSAVE3), FALSE);	// added Sept. 6, JEPRO 自動保存にしたときだけEnableになるように変更
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOSAVE4), FALSE);	// added Sept. 6, JEPRO	同上
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_nDropFileNumMax), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_nDropFileNumMax), FALSE);// added Oct. 6, JEPRO ファイルオープンを「閉じて開く」にしたときはDisableになるように変更
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOSAVE3), TRUE);	// added Sept. 6, JEPRO	同上
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOSAVE4), TRUE);	// added Sept. 6, JEPRO	同上
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_nDropFileNumMax), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_nDropFileNumMax), TRUE);// added Oct. 6, JEPRO ファイルオープンを「複数ファイルドロップ」にしたときだけEnableになるように変更
	}

	//	排他するかどうか
	int nSelPos = Combo_GetCurSel(::GetDlgItem(hwndDlg, IDC_COMBO_FILESHAREMODE));
	if (ShareModeArr[nSelPos].nMethod == FileShareMode::NonExclusive) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_bCheckFileTimeStamp), TRUE);
		if (DlgButton_IsChecked(hwndDlg, IDC_CHECK_bCheckFileTimeStamp)) {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOLOAD_DELAY), TRUE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_AUTOLOAD_DELAY),  TRUE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_AUTOLOAD_DELAY),  TRUE);
		}else {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOLOAD_DELAY), FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_AUTOLOAD_DELAY),  FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_AUTOLOAD_DELAY),  FALSE);
		}
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_bCheckFileTimeStamp), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOLOAD_DELAY), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_AUTOLOAD_DELAY),  FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_AUTOLOAD_DELAY),  FALSE);
	}

	//	自動保存
	if (DlgButton_IsChecked(hwndDlg, IDC_CHECK_AUTOSAVE)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOSAVE), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOSAVE2), TRUE);	// Sept. 6, 2000 JEPRO 自動保存にしたときだけEnableになるように変更
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_AUTOBACKUP_INTERVAL), TRUE);	//@@@ 2001.03.21 by MIK
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOSAVE), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOSAVE2), FALSE);	// Sept. 6, 2000 JEPRO 同上
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_AUTOBACKUP_INTERVAL), FALSE);	//@@@ 2001.03.21 by MIK
	}

	// 「開こうとしたファイルが大きい場合に警告を出す」
	if (DlgButton_IsChecked(hwndDlg, IDC_CHECK_ALERT_IF_LARGEFILE)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_ALERT_FILESIZE), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_ALERT_FILESIZE), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_ALERT_FILESIZE), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_ALERT_FILESIZE), FALSE);
	}
}
//	To Here Aug. 21, 2000 genta

