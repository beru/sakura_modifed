/*!	@file
	���ʐݒ�_�C�A���O�{�b�N�X�A�u�����v�y�[�W
*/

#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "util/shell.h"
#include "env/DllSharedData.h" // FormatManager.h���O�ɕK�v
#include "env/FormatManager.h"
#include "sakura_rc.h"
#include "sakura.hh"


// Popup Help
static const DWORD p_helpids[] = {	//10400
	IDC_EDIT_DFORM,						HIDC_EDIT_DFORM,		// ���t����
	IDC_EDIT_TFORM,						HIDC_EDIT_TFORM,		// ��������
	IDC_EDIT_DFORM_EX,					HIDC_EDIT_DFORM_EX,		// ���t�����i�\����j
	IDC_EDIT_TFORM_EX,					HIDC_EDIT_TFORM_EX,		// ���������i�\����j
	IDC_EDIT_MIDASHIKIGOU,				HIDC_EDIT_MIDASHIKIGOU,	// ���o���L��
	IDC_EDIT_INYOUKIGOU,				HIDC_EDIT_INYOUKIGOU,	// ���p��
	IDC_RADIO_DFORM_0,					HIDC_RADIO_DFORM_0,		// ���t�����i�W���j
	IDC_RADIO_DFORM_1,					HIDC_RADIO_DFORM_1,		// ���t�����i�J�X�^���j
	IDC_RADIO_TFORM_0,					HIDC_RADIO_TFORM_0,		// ���������i�W���j
	IDC_RADIO_TFORM_1,					HIDC_RADIO_TFORM_1,		// ���������i�J�X�^���j
//	IDC_STATIC,							-1,
	0, 0
};
//@@@ 2001.02.04 End

//@@@ 2002.01.12 add start
static const char* p_date_form[] = {
	"yyyy'�N'M'��'d'��'",
	"yyyy'�N'M'��'d'��('dddd')'",
	"yyyy'�N'MM'��'dd'��'",
	"yyyy'�N'M'��'d'��' dddd",
	"yyyy'�N'MM'��'dd'��' dddd",
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
	"tthh'��'mm'��'ss'�b'",
	"H:mm:ss",
	"HH:mm:ss",
	"tt h:mm:ss",
	"tt hh:mm:ss",
	NULL
};
//@@@ 2002.01.12 add end

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
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
	// �_�C�A���O�f�[�^�̎擾 Format
	GetData(hwndDlg);

	// ���t���t�H�[�}�b�g
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
	// �_�C�A���O�f�[�^�̎擾 Format
	GetData(hwndDlg);

	// �������t�H�[�}�b�g
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime(&systime);
	FormatManager().MyGetTimeFormat(systime, szText, _countof(szText) - 1, csFormat.nTimeFormatType, csFormat.szTimeFormat);
	::DlgItem_SetText(hwndDlg, IDC_EDIT_TFORM_EX, szText);
	return;
}


// Format ���b�Z�[�W����
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
		// �_�C�A���O�f�[�^�̐ݒ� Format
		SetData(hwndDlg);
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		ChangeDateExample(hwndDlg);
		ChangeTimeExample(hwndDlg);

		// ���o���L��
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_MIDASHIKIGOU), _countof(csFormat.szMidashiKigou) - 1);

		// ���p��
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_INYOUKIGOU), _countof(csFormat.szInyouKigou) - 1);

		// ���t����
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_DFORM), _countof(csFormat.szDateFormat) - 1);

		// ��������
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_TFORM), _countof(csFormat.szTimeFormat) - 1);

		return TRUE;
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	// �ʒm�R�[�h
		wID			= LOWORD(wParam);	// ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID
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

		// �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ
		case BN_CLICKED:
			switch (wID) {
			case IDC_RADIO_DFORM_0:
			case IDC_RADIO_DFORM_1:
				ChangeDateExample(hwndDlg);
			//	From Here Sept. 10, 2000 JEPRO
			//	���t���� 0=�W�� 1=�J�X�^��
			//	���t�������J�X�^���ɂ���Ƃ����������w�蕶�����͂�Enable�ɐݒ�
				EnableFormatPropInput(hwndDlg);
			//	To Here Sept. 10, 2000
				return 0;
			case IDC_RADIO_TFORM_0:
			case IDC_RADIO_TFORM_1:
				ChangeTimeExample(hwndDlg);
			//	From Here Sept. 10, 2000 JEPRO
			//	�������� 0=�W�� 1=�J�X�^��
			//	�����������J�X�^���ɂ���Ƃ����������w�蕶�����͂�Enable�ɐݒ�
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
				// �_�C�A���O�f�[�^�̎擾 Format
				GetData(hwndDlg);
				return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
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


// �_�C�A���O�f�[�^�̐ݒ� Format
void PropFormat::SetData(HWND hwndDlg)
{
	auto& csFormat = common.format;
	
	// ���o���L��
	::DlgItem_SetText(hwndDlg, IDC_EDIT_MIDASHIKIGOU, csFormat.szMidashiKigou);

	// ���p��
	::DlgItem_SetText(hwndDlg, IDC_EDIT_INYOUKIGOU, csFormat.szInyouKigou);

	// ���t�����̃^�C�v
	if (csFormat.nDateFormatType == 0) {
		::CheckDlgButton(hwndDlg, IDC_RADIO_DFORM_0, BST_CHECKED);
	}else {
		::CheckDlgButton(hwndDlg, IDC_RADIO_DFORM_1, BST_CHECKED);
	}
	// ���t����
	::DlgItem_SetText(hwndDlg, IDC_EDIT_DFORM, csFormat.szDateFormat);

	// ���������̃^�C�v
	if (csFormat.nTimeFormatType == 0) {
		::CheckDlgButton(hwndDlg, IDC_RADIO_TFORM_0, BST_CHECKED);
	}else {
		::CheckDlgButton(hwndDlg, IDC_RADIO_TFORM_1, BST_CHECKED);
	}
	// ��������
	::DlgItem_SetText(hwndDlg, IDC_EDIT_TFORM, csFormat.szTimeFormat);

	//	From Here Sept. 10, 2000 JEPRO
	//	���t/�������� 0=�W�� 1=�J�X�^��
	//	���t/�����������J�X�^���ɂ���Ƃ����������w�蕶�����͂�Enable�ɐݒ�
	EnableFormatPropInput(hwndDlg);
	//	To Here Sept. 10, 2000

	return;
}


// �_�C�A���O�f�[�^�̎擾 Format
int PropFormat::GetData(HWND hwndDlg)
{
	auto& csFormat = common.format;
	// ���o���L��
	::DlgItem_GetText(hwndDlg, IDC_EDIT_MIDASHIKIGOU, csFormat.szMidashiKigou, _countof(csFormat.szMidashiKigou));

//	// �O���w���v�P
//	::DlgItem_GetText(hwndDlg, IDC_EDIT_EXTHELP1, csFormat.m_szExtHelp1, MAX_PATH - 1);
//
//	// �O��HTML�w���v
//	::DlgItem_GetText(hwndDlg, IDC_EDIT_EXTHTMLHELP, csFormat.szExtHtmlHelp, MAX_PATH - 1);

	// ���p��
	::DlgItem_GetText(hwndDlg, IDC_EDIT_INYOUKIGOU, csFormat.szInyouKigou, _countof(csFormat.szInyouKigou));


	// ���t�����̃^�C�v
	if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_DFORM_0)) {
		csFormat.nDateFormatType = 0;
	}else {
		csFormat.nDateFormatType = 1;
	}
	// ���t����
	::DlgItem_GetText(hwndDlg, IDC_EDIT_DFORM, csFormat.szDateFormat, _countof(csFormat.szDateFormat));

	// ���������̃^�C�v
	if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_TFORM_0)) {
		csFormat.nTimeFormatType = 0;
	}else {
		csFormat.nTimeFormatType = 1;
	}

	// ��������
	::DlgItem_GetText(hwndDlg, IDC_EDIT_TFORM, csFormat.szTimeFormat, _countof(csFormat.szTimeFormat));

	return TRUE;
}


//	From Here Sept. 10, 2000 JEPRO
//	�`�F�b�N��Ԃɉ����ă_�C�A���O�{�b�N�X�v�f��Enable/Disable��
//	�K�؂ɐݒ肷��
void PropFormat::EnableFormatPropInput(HWND hwndDlg)
{
	//	���t�������J�X�^���ɂ��邩�ǂ���
	if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_DFORM_1)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_DFORM), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_DFORM), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_DFORM), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_DFORM), FALSE);
	}

	//	�����������J�X�^���ɂ��邩�ǂ���
	if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_TFORM_1)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_TFORM), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_TFORM), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_TFORM), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_TFORM), FALSE);
	}
}
//	To Here Sept. 10, 2000

