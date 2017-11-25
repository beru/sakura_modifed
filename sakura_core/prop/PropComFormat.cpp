/*!	@file
	共通設定ダイアログボックス、「書式」ページ
*/

#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "util/shell.h"
#include "env/DllSharedData.h" // FormatManager.hより前に必要
#include "env/FormatManager.h"
#include "sakura_rc.h"
#include "sakura.hh"


// Popup Help
static const DWORD p_helpids[] = {	//10400
	IDC_EDIT_DFORM,						HIDC_EDIT_DFORM,		// 日付書式
	IDC_EDIT_TFORM,						HIDC_EDIT_TFORM,		// 時刻書式
	IDC_EDIT_DFORM_EX,					HIDC_EDIT_DFORM_EX,		// 日付書式（表示例）
	IDC_EDIT_TFORM_EX,					HIDC_EDIT_TFORM_EX,		// 時刻書式（表示例）
	IDC_EDIT_MIDASHIKIGOU,				HIDC_EDIT_MIDASHIKIGOU,	// 見出し記号
	IDC_EDIT_INYOUKIGOU,				HIDC_EDIT_INYOUKIGOU,	// 引用符
	IDC_RADIO_DFORM_0,					HIDC_RADIO_DFORM_0,		// 日付書式（標準）
	IDC_RADIO_DFORM_1,					HIDC_RADIO_DFORM_1,		// 日付書式（カスタム）
	IDC_RADIO_TFORM_0,					HIDC_RADIO_TFORM_0,		// 時刻書式（標準）
	IDC_RADIO_TFORM_1,					HIDC_RADIO_TFORM_1,		// 時刻書式（カスタム）
//	IDC_STATIC,							-1,
	0, 0
};
//@@@ 2001.02.04 End

//@@@ 2002.01.12 add start
static const char* p_date_form[] = {
	"yyyy'年'M'月'd'日'",
	"yyyy'年'M'月'd'日('dddd')'",
	"yyyy'年'MM'月'dd'日'",
	"yyyy'年'M'月'd'日' dddd",
	"yyyy'年'MM'月'dd'日' dddd",
	"yyyy/MM/dd",
	"yy/MM/dd",
	"yy/M/d",
	"yyyy/M/d",
	"yy/MM/dd' ('ddd')'",
	"yy/M/d' ('ddd')'",
	"yyyy/MM/dd' ('ddd')'",
	"yyyy/M/d' ('ddd')'",
	"yyyy/M/d' ('ddd')'",
	NULL
};

static const char* p_time_form[] = {
	"hh:mm:ss",
	"tthh'時'mm'分'ss'秒'",
	"H:mm:ss",
	"HH:mm:ss",
	"tt h:mm:ss",
	"tt hh:mm:ss",
	NULL
};
//@@@ 2002.01.12 add end

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK PropFormat::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropFormat::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}
//	To Here Jun. 2, 2001 genta

void PropFormat::ChangeDateExample(HWND hwndDlg)
{
	auto& csFormat = common.format;
	// ダイアログデータの取得 Format
	GetData(hwndDlg);

	// 日付をフォーマット
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime(&systime);
	FormatManager().MyGetDateFormat(systime, szText, _countof(szText) - 1, csFormat.nDateFormatType, csFormat.szDateFormat);
	::DlgItem_SetText(hwndDlg, IDC_EDIT_DFORM_EX, szText);
	return;
}

void PropFormat::ChangeTimeExample(HWND hwndDlg)
{
	auto& csFormat = common.format;
	// ダイアログデータの取得 Format
	GetData(hwndDlg);

	// 時刻をフォーマット
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime(&systime);
	FormatManager().MyGetTimeFormat(systime, szText, _countof(szText) - 1, csFormat.nTimeFormatType, csFormat.szTimeFormat);
	::DlgItem_SetText(hwndDlg, IDC_EDIT_TFORM_EX, szText);
	return;
}


// Format メッセージ処理
INT_PTR PropFormat::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,	// message
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
	WORD		wNotifyCode;
	WORD		wID;
	NMHDR*		pNMHDR;
//	NM_UPDOWN*	pMNUD;
//	int			idCtrl;
//	int			nVal;
	auto& csFormat = common.format;

	switch (uMsg) {
	case WM_INITDIALOG:
		// ダイアログデータの設定 Format
		SetData(hwndDlg);
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		ChangeDateExample(hwndDlg);
		ChangeTimeExample(hwndDlg);

		// 見出し記号
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_MIDASHIKIGOU), _countof(csFormat.szMidashiKigou) - 1);

		// 引用符
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_INYOUKIGOU), _countof(csFormat.szInyouKigou) - 1);

		// 日付書式
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_DFORM), _countof(csFormat.szDateFormat) - 1);

		// 時刻書式
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_TFORM), _countof(csFormat.szTimeFormat) - 1);

		return TRUE;
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	// 通知コード
		wID			= LOWORD(wParam);	// 項目ID､ コントロールID､ またはアクセラレータID
		switch (wNotifyCode) {
		case EN_CHANGE:
			if (wID == IDC_EDIT_DFORM) {
				ChangeDateExample(hwndDlg);
				return 0;
			}
			if (wID == IDC_EDIT_TFORM) {
				ChangeTimeExample(hwndDlg);
				return 0;
			}
			break;

		// ボタン／チェックボックスがクリックされた
		case BN_CLICKED:
			switch (wID) {
			case IDC_RADIO_DFORM_0:
			case IDC_RADIO_DFORM_1:
				ChangeDateExample(hwndDlg);
			//	From Here Sept. 10, 2000 JEPRO
			//	日付書式 0=標準 1=カスタム
			//	日付書式をカスタムにするときだけ書式指定文字入力をEnableに設定
				EnableFormatPropInput(hwndDlg);
			//	To Here Sept. 10, 2000
				return 0;
			case IDC_RADIO_TFORM_0:
			case IDC_RADIO_TFORM_1:
				ChangeTimeExample(hwndDlg);
			//	From Here Sept. 10, 2000 JEPRO
			//	時刻書式 0=標準 1=カスタム
			//	時刻書式をカスタムにするときだけ書式指定文字入力をEnableに設定
				EnableFormatPropInput(hwndDlg);
			//	To Here Sept. 10, 2000
				return 0;
			}
			break;	// BN_CLICKED
		}
		break;	// WM_COMMAND
	case WM_NOTIFY:
//		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
//		pMNUD  = (NM_UPDOWN*)lParam;
//		switch (idCtrl) {
//		case ???????:
//			return 0L;
//		default:
			switch (pNMHDR->code) {
			case PSN_HELP:
				OnHelp(hwndDlg, IDD_PROP_FORMAT);
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE(_T("Format PSN_KILLACTIVE\n"));
				// ダイアログデータの取得 Format
				GetData(hwndDlg);
				return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			case PSN_SETACTIVE:
				nPageNum = ID_PROPCOM_PAGENUM_FORMAT;
				return TRUE;
			}
//			break;	// default
//		}

//		MYTRACE(_T("pNMHDR->hwndFrom=%xh\n"), pNMHDR->hwndFrom);
//		MYTRACE(_T("pNMHDR->idFrom  =%xh\n"), pNMHDR->idFrom);
//		MYTRACE(_T("pNMHDR->code    =%xh\n"), pNMHDR->code);
//		MYTRACE(_T("pMNUD->iPos    =%d\n"), pMNUD->iPos);
//		MYTRACE(_T("pMNUD->iDelta  =%d\n"), pMNUD->iDelta);
		break;	// WM_NOTIFY

	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids);
		}
		return TRUE;
		// NOTREACHED
		break;

	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids);
		return TRUE;

	}
	return FALSE;
}


// ダイアログデータの設定 Format
void PropFormat::SetData(HWND hwndDlg)
{
	auto& csFormat = common.format;
	
	// 見出し記号
	::DlgItem_SetText(hwndDlg, IDC_EDIT_MIDASHIKIGOU, csFormat.szMidashiKigou);

	// 引用符
	::DlgItem_SetText(hwndDlg, IDC_EDIT_INYOUKIGOU, csFormat.szInyouKigou);

	// 日付書式のタイプ
	if (csFormat.nDateFormatType == 0) {
		::CheckDlgButton(hwndDlg, IDC_RADIO_DFORM_0, BST_CHECKED);
	}else {
		::CheckDlgButton(hwndDlg, IDC_RADIO_DFORM_1, BST_CHECKED);
	}
	// 日付書式
	::DlgItem_SetText(hwndDlg, IDC_EDIT_DFORM, csFormat.szDateFormat);

	// 時刻書式のタイプ
	if (csFormat.nTimeFormatType == 0) {
		::CheckDlgButton(hwndDlg, IDC_RADIO_TFORM_0, BST_CHECKED);
	}else {
		::CheckDlgButton(hwndDlg, IDC_RADIO_TFORM_1, BST_CHECKED);
	}
	// 時刻書式
	::DlgItem_SetText(hwndDlg, IDC_EDIT_TFORM, csFormat.szTimeFormat);

	//	From Here Sept. 10, 2000 JEPRO
	//	日付/時刻書式 0=標準 1=カスタム
	//	日付/時刻書式をカスタムにするときだけ書式指定文字入力をEnableに設定
	EnableFormatPropInput(hwndDlg);
	//	To Here Sept. 10, 2000

	return;
}


// ダイアログデータの取得 Format
int PropFormat::GetData(HWND hwndDlg)
{
	auto& csFormat = common.format;
	// 見出し記号
	::DlgItem_GetText(hwndDlg, IDC_EDIT_MIDASHIKIGOU, csFormat.szMidashiKigou, _countof(csFormat.szMidashiKigou));

//	// 外部ヘルプ１
//	::DlgItem_GetText(hwndDlg, IDC_EDIT_EXTHELP1, csFormat.m_szExtHelp1, MAX_PATH - 1);
//
//	// 外部HTMLヘルプ
//	::DlgItem_GetText(hwndDlg, IDC_EDIT_EXTHTMLHELP, csFormat.szExtHtmlHelp, MAX_PATH - 1);

	// 引用符
	::DlgItem_GetText(hwndDlg, IDC_EDIT_INYOUKIGOU, csFormat.szInyouKigou, _countof(csFormat.szInyouKigou));


	// 日付書式のタイプ
	if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_DFORM_0)) {
		csFormat.nDateFormatType = 0;
	}else {
		csFormat.nDateFormatType = 1;
	}
	// 日付書式
	::DlgItem_GetText(hwndDlg, IDC_EDIT_DFORM, csFormat.szDateFormat, _countof(csFormat.szDateFormat));

	// 時刻書式のタイプ
	if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_TFORM_0)) {
		csFormat.nTimeFormatType = 0;
	}else {
		csFormat.nTimeFormatType = 1;
	}

	// 時刻書式
	::DlgItem_GetText(hwndDlg, IDC_EDIT_TFORM, csFormat.szTimeFormat, _countof(csFormat.szTimeFormat));

	return TRUE;
}


//	From Here Sept. 10, 2000 JEPRO
//	チェック状態に応じてダイアログボックス要素のEnable/Disableを
//	適切に設定する
void PropFormat::EnableFormatPropInput(HWND hwndDlg)
{
	//	日付書式をカスタムにするかどうか
	if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_DFORM_1)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_DFORM), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_DFORM), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_DFORM), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_DFORM), FALSE);
	}

	//	時刻書式をカスタムにするかどうか
	if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_TFORM_1)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_TFORM), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_TFORM), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_TFORM), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_TFORM), FALSE);
	}
}
//	To Here Sept. 10, 2000

