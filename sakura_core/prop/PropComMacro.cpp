/*!	@file
	���ʐݒ�_�C�A���O�{�b�N�X�A�u�}�N���v�y�[�W

	@author genta
	@date Jun. 2, 2001 genta
*/
/*
	Copyright (C) 2001, genta, MIK
	Copyright (C) 2002, YAZAKI, MIK, genta, novice
	Copyright (C) 2003, Moca, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji

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
#include "prop/PropCommon.h"
#include "env/DllSharedData.h"
#include "util/shell.h"
#include "util/string_ex2.h"
#include "util/module.h"
#include "sakura_rc.h"
#include "sakura.hh"

// Popup Help�pID
//@@@ 2001.12.22 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//11700
	IDC_MACRODIRREF,	HIDC_MACRODIRREF,	// �}�N���f�B���N�g���Q��
	IDC_MACRO_REG,		HIDC_MACRO_REG,		// �}�N���ݒ�
	IDC_COMBO_MACROID,	HIDC_COMBO_MACROID,	// ID
	IDC_MACROPATH,		HIDC_MACROPATH,		// File
	IDC_MACRONAME,		HIDC_MACRONAME,		// �}�N����
	IDC_MACROLIST,		HIDC_MACROLIST,		// �}�N�����X�g
	IDC_MACRODIR,		HIDC_MACRODIR,		// �}�N���ꗗ
	IDC_CHECK_RELOADWHENEXECUTE,	HIDC_CHECK_RELOADWHENEXECUTE,	// �}�N�������s���邽�тɃt�@�C����ǂݍ��݂Ȃ���	// 2006.08.06 ryoji
	IDC_CHECK_MacroOnOpened,		HIDC_CHECK_MacroOnOpened,		// �I�[�v���㎩�����s�}�N��	// 2006.09.01 ryoji
	IDC_CHECK_MacroOnTypeChanged,	HIDC_CHECK_MacroOnTypeChanged,	// �^�C�v�ύX�㎩�����s�}�N��	// 2006.09.01 ryoji
	IDC_CHECK_MacroOnSave,			HIDC_CHECK_MacroOnSave,			// �ۑ��O�������s�}�N��	// 2006.09.01 ryoji
	IDC_MACROCANCELTIMER,			HIDC_MACROCANCELTIMER,			// �}�N����~�_�C�A���O�\���҂�����	// 2011.08.04 syat
//	IDC_STATIC,			-1,
	0, 0
};
//@@@ 2001.12.22 End

/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CALLBACK PropMacro::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropMacro::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}

/*! Macro�y�[�W�̃��b�Z�[�W����
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handlw
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR PropMacro::DispatchEvent(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	NMHDR*		pNMHDR;
	int			idCtrl;

	WORD		wNotifyCode;
	WORD		wID;

	auto& csMacro = common.macro;

	switch (uMsg) {
	case WM_INITDIALOG:
		// �_�C�A���O�f�[�^�̐ݒ� Macro
		InitDialog(hwndDlg);
		SetData(hwndDlg);
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		//	Oct. 5, 2002 genta �G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌�����
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_MACRONAME), _countof(csMacro.macroTable[0].szName) - 1);
		Combo_LimitText(::GetDlgItem(hwndDlg, IDC_MACROPATH), _countof(csMacro.macroTable[0].szFile) - 1);
		// 2003.06.23 Moca
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_MACRODIR), _countof2(csMacro.szMACROFOLDER) - 1);
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_MACROCANCELTIMER), 4);
		return TRUE;
		
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		switch (idCtrl) {
		case IDC_MACROLIST:
			switch (pNMHDR->code) {
			case LVN_ITEMCHANGED:
				CheckListPosition_Macro(hwndDlg);
				break;
			}
			break;
		default:
			switch (pNMHDR->code) {
			case PSN_HELP:
				OnHelp(hwndDlg, IDD_PROP_MACRO);
				return TRUE;
			case PSN_KILLACTIVE:
				// �_�C�A���O�f�[�^�̎擾 Macro
				GetData(hwndDlg);
				return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			case PSN_SETACTIVE:
				nPageNum = ID_PROPCOM_PAGENUM_MACRO;
				return TRUE;
			}
			break;
		}
		break;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// �ʒm�R�[�h
		wID = LOWORD(wParam);			// ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID

		switch (wNotifyCode) {
		// �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ
		case BN_CLICKED:
			switch (wID) {
			case IDC_MACRODIRREF:	// �}�N���f�B���N�g���Q��
				SelectBaseDir_Macro(hwndDlg);
				break;
			case IDC_MACRO_REG:		// �}�N���ݒ�
				SetMacro2List_Macro(hwndDlg);
				break;
			}
			break;
		case CBN_DROPDOWN:
			switch (wID) {
			case IDC_MACROPATH:
				OnFileDropdown_Macro(hwndDlg);
				break;
			}
			break;	// CBN_DROPDOWN
		// From Here 2003.06.23 Moca �}�N���t�H���_�̍Ō��\���Ȃ���Εt����
		case EN_KILLFOCUS:
			switch (wID) {
			case IDC_MACRODIR:
				{
					TCHAR szDir[_MAX_PATH];
					::DlgItem_GetText(hwndDlg, IDC_MACRODIR, szDir, _MAX_PATH);
					if (AddLastChar(szDir, _MAX_PATH, _T('\\')) == 1) {
						::DlgItem_SetText(hwndDlg, IDC_MACRODIR, szDir);
					}
				}
				break;
			}
			break;
		// To Here 2003.06.23 Moca
		}

		break;	// WM_COMMAND
//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids);	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		// NOTREACHED
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids);	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}


/*!
	�_�C�A���O��̃R���g���[���Ƀf�[�^��ݒ肷��

	@param hwndDlg �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��
*/
void PropMacro::SetData(HWND hwndDlg)
{
	int index;
	LVITEM lvItem;

	// �}�N���f�[�^
	HWND hListView = ::GetDlgItem(hwndDlg, IDC_MACROLIST);
	auto& csMacro = pShareData->common.macro;
	
	for (index=0; index<MAX_CUSTMACRO; ++index) {
		auto& macroRec = csMacro.macroTable[index];
		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.iItem = index;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 1;
		lvItem.pszText = macroRec.szName;
		ListView_SetItem(hListView, &lvItem);

		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.iItem = index;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 2;
		lvItem.pszText = macroRec.szFile;
		ListView_SetItem(hListView, &lvItem);

		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.iItem = index;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 3;
		lvItem.pszText = const_cast<TCHAR*>(macroRec.bReloadWhenExecute ? _T("on") : _T("off"));
		ListView_SetItem(hListView, &lvItem);

		// �������s�}�N��	// 2006.09.01 ryoji
		TCHAR szText[8];
		szText[0] = _T('\0');
		if (index == csMacro.nMacroOnOpened) {
			::lstrcat(szText, _T("O"));
		}
		if (index == csMacro.nMacroOnTypeChanged) {
			::lstrcat(szText, _T("T"));
		}
		if (index == csMacro.nMacroOnSave) {
			::lstrcat(szText, _T("S"));
		}
		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.iItem = index;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 4;
		lvItem.pszText = szText;
		ListView_SetItem(hListView, &lvItem);
	}
	
	//	�}�N���f�B���N�g��
	::DlgItem_SetText(hwndDlg, IDC_MACRODIR, /*pShareData->*/common.macro.szMACROFOLDER);

	nLastPos_Macro = -1;
	
	//	���X�g�r���[�̍s�I�����\�ɂ���D
	//	IE 3.x�ȍ~�������Ă���ꍇ�̂ݓ��삷��D
	//	���ꂪ�����Ă��C�ԍ����������I���ł��Ȃ������ő��쎩�͉̂\�D
	DWORD dwStyle;
	dwStyle = ListView_GetExtendedListViewStyle(hListView);
	dwStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle(hListView, dwStyle);
	
	//	�}�N����~�_�C�A���O�\���҂�����
	TCHAR szCancelTimer[16] = {0};
	::DlgItem_SetText(hwndDlg, IDC_MACROCANCELTIMER, _itot(common.macro.nMacroCancelTimer, szCancelTimer, 10));

	return;
}

/*!
	�_�C�A���O��̃R���g���[������f�[�^���擾���ă������Ɋi�[����

	@param hwndDlg �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��
*/

int PropMacro::GetData(HWND hwndDlg)
{
	int index;
	LVITEM lvItem;

	auto& csMacro = common.macro;

	// �������s�}�N���ϐ�������	// 2006.09.01 ryoji
	csMacro.nMacroOnOpened = -1;
	csMacro.nMacroOnTypeChanged = -1;
	csMacro.nMacroOnSave = -1;

	//	�}�N���f�[�^
	HWND hListView = ::GetDlgItem(hwndDlg, IDC_MACROLIST);

	for (index=0; index<MAX_CUSTMACRO; ++index) {
		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.iItem = index;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 1;
		lvItem.cchTextMax = MACRONAME_MAX - 1;
//@@@ 2002.01.03 YAZAKI ���ʐݒ�w�}�N���x���^�u��؂�ւ��邾���Őݒ肪�ۑ�����Ȃ��悤�ɁB
		lvItem.pszText = /*pShareData->*/csMacro.macroTable[index].szName;
		ListView_GetItem(hListView, &lvItem);

		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.iItem = index;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 2;
		lvItem.cchTextMax = _MAX_PATH;
//@@@ 2002.01.03 YAZAKI ���ʐݒ�w�}�N���x���^�u��؂�ւ��邾���Őݒ肪�ۑ�����Ȃ��悤�ɁB
		lvItem.pszText = /*pShareData->*/csMacro.macroTable[index].szFile;
		ListView_GetItem(hListView, &lvItem);

		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.iItem = index;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 3;
		TCHAR buf[MAX_PATH];
		lvItem.pszText = buf;
		lvItem.cchTextMax = MAX_PATH;
		ListView_GetItem(hListView, &lvItem);
		if (_tcscmp(buf, _T("on")) == 0) {
			csMacro.macroTable[index].bReloadWhenExecute = true;
		}else {
			csMacro.macroTable[index].bReloadWhenExecute = false;
		}

		// �������s�}�N��	// 2006.09.01 ryoji
		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.iItem = index;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 4;
		TCHAR szText[8];
		lvItem.pszText = szText;
		lvItem.cchTextMax = _countof(szText);
		ListView_GetItem(hListView, &lvItem);
		int nLen = ::lstrlen(szText);
		for (int i=0; i<nLen; ++i) {
			if (szText[i] == _T('O')) {
				csMacro.nMacroOnOpened = index;
			}
			if (szText[i] == _T('T')) {
				csMacro.nMacroOnTypeChanged = index;
			}
			if (szText[i] == _T('S')) {
				csMacro.nMacroOnSave = index;
			}
		}
	}

	//	�}�N���f�B���N�g��
//@@@ 2002.01.03 YAZAKI ���ʐݒ�w�}�N���x���^�u��؂�ւ��邾���Őݒ肪�ۑ�����Ȃ��悤�ɁB
	::DlgItem_GetText(hwndDlg, IDC_MACRODIR, csMacro.szMACROFOLDER, _MAX_PATH);
	// 2003.06.23 Moca �}�N���t�H���_�̍Ō��\���Ȃ���Εt����
	AddLastChar(csMacro.szMACROFOLDER, _MAX_PATH, _T('\\'));
	
	//	�}�N����~�_�C�A���O�\���҂�����
	TCHAR szCancelTimer[16] = {0};
	::DlgItem_GetText(hwndDlg, IDC_MACROCANCELTIMER, szCancelTimer, _countof(szCancelTimer));
	csMacro.nMacroCancelTimer = _ttoi(szCancelTimer);

	return TRUE;
}

struct ColumnData_CPropMacro_Init {
	int titleId;
	int width;
};

void PropMacro::InitDialog(HWND hwndDlg)
{
	struct ColumnData_CPropMacro_Init ColumnList[] = {
		{ STR_PROPCOMMACR_LIST1, 40 },
		{ STR_PROPCOMMACR_LIST2, 150 },
		{ STR_PROPCOMMACR_LIST3, 150 },
		{ STR_PROPCOMMACR_LIST4, 40 },
		{ STR_PROPCOMMACR_LIST5, 40 },
	};

	//	ListView�̏�����
	HWND hListView = ::GetDlgItem(hwndDlg, IDC_MACROLIST);
	if (!hListView) {
		PleaseReportToAuthor(hwndDlg, _T("PropComMacro::InitDlg::NoListView"));
		return;	//	�悭�킩��񂯂ǎ��s����	
	}

	LVCOLUMN sColumn;
	int pos;
	RECT rc;
	::GetWindowRect(hListView, &rc);
	int width = rc.right - rc.left;
	
	for (pos=0; pos<_countof(ColumnList); ++pos) {
		
		memset_raw(&sColumn, 0, sizeof(sColumn));
		sColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT;
		sColumn.pszText = const_cast<TCHAR*>(LS(ColumnList[pos].titleId));
		sColumn.cx = ColumnList[pos].width * width / 499;
		sColumn.iSubItem = pos;
		sColumn.fmt = LVCFMT_LEFT;
		
		if (ListView_InsertColumn(hListView, pos, &sColumn) < 0) {
			PleaseReportToAuthor(hwndDlg, _T("PropComMacro::InitDlg::ColumnRegistrationFail"));
			return;	//	�悭�킩��񂯂ǎ��s����
		}
	}

	//	�������̊m��
	//	�K�v�Ȑ�������Ɋm�ۂ���D
	ListView_SetItemCount(hListView, MAX_CUSTMACRO);

	//	Index�����̓o�^
	for (pos=0; pos<MAX_CUSTMACRO; ++pos) {
		LVITEM lvItem;
		TCHAR buf[4];
		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.mask = LVIF_TEXT | LVIF_PARAM;
		lvItem.iItem = pos;
		lvItem.iSubItem = 0;
		_itot(pos, buf, 10);
		lvItem.pszText = buf;
		lvItem.lParam = pos;
		ListView_InsertItem(hListView, &lvItem);
	}
	
	// �o�^��w�� ComboBox�̏�����
	HWND hNumCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_MACROID);
	for (pos=0; pos<MAX_CUSTMACRO; ++pos) {
		wchar_t buf[10];
		auto_sprintf(buf, L"%d", pos);
		LRESULT result = Combo_AddString(hNumCombo, buf);
		if (result == CB_ERR) {
			PleaseReportToAuthor(hwndDlg, _T("PropComMacro::InitDlg::AddMacroId"));
			return;	//	�悭�킩��񂯂ǎ��s����
		}else if (result == CB_ERRSPACE) {
			PleaseReportToAuthor(hwndDlg, _T("PropComMacro::InitDlg::AddMacroId/InsufficientSpace"));
			return;	//	�悭�킩��񂯂ǎ��s����
		}
	}
	Combo_SetCurSel(hNumCombo, 0);
}

void PropMacro::SetMacro2List_Macro(HWND hwndDlg)
{
	int index;
	LVITEM lvItem;
	
	HWND hListView = ::GetDlgItem(hwndDlg, IDC_MACROLIST);
	HWND hNum = ::GetDlgItem(hwndDlg, IDC_COMBO_MACROID);

	//	�ݒ��擾
	index = Combo_GetCurSel(hNum);
	if (index == CB_ERR) {
		PleaseReportToAuthor(hwndDlg, _T("PropComMacro::SetMacro2List::GetCurSel"));
		return;	//	�悭�킩��񂯂ǎ��s����
	}

	// �}�N����
	memset_raw(&lvItem, 0, sizeof(lvItem));
	lvItem.iItem = index;
	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	
	TCHAR buf[256];
	::DlgItem_GetText(hwndDlg, IDC_MACRONAME, buf, MACRONAME_MAX);
	lvItem.pszText = buf;
	ListView_SetItem(hListView, &lvItem);

	// �t�@�C����
	memset_raw(&lvItem, 0, sizeof(lvItem));
	lvItem.iItem = index;
	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 2;

	::DlgItem_GetText(hwndDlg, IDC_MACROPATH, buf, _MAX_PATH);
	lvItem.pszText = buf;
	ListView_SetItem(hListView, &lvItem);

	// �`�F�b�N
	memset_raw(&lvItem, 0, sizeof(lvItem));
	lvItem.iItem = index;
	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 3;
	lvItem.pszText = const_cast<TCHAR*>(IsDlgButtonChecked(hwndDlg, IDC_CHECK_RELOADWHENEXECUTE) ? _T("on") : _T("off"));
	ListView_SetItem(hListView, &lvItem);

	// �������s�}�N��	// 2006.09.01 ryoji
	int nMacroOnOpened = -1;
	int nMacroOnTypeChanged = -1;
	int nMacroOnSave = -1;
	TCHAR szText[8];
	int iItem;
	for (iItem=0; iItem<MAX_CUSTMACRO; ++iItem) {
		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.iItem = iItem;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 4;
		lvItem.pszText = szText;
		lvItem.cchTextMax = _countof(szText);
		ListView_GetItem(hListView, &lvItem);
		int nLen = ::lstrlen(szText);
		for (int i=0; i<nLen; ++i) {
			if (szText[i] == _T('O')) {
				nMacroOnOpened = iItem;
			}
			if (szText[i] == _T('T')) {
				nMacroOnTypeChanged = iItem;
			}
			if (szText[i] == _T('S')) {
				nMacroOnSave = iItem;
			}
		}
	}
	if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_MacroOnOpened)) {
		nMacroOnOpened = index;
	}else if (nMacroOnOpened == index) {
		nMacroOnOpened = -1;
	}
	if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_MacroOnTypeChanged)) {
		nMacroOnTypeChanged = index;
	}else if (nMacroOnTypeChanged == index) {
		nMacroOnTypeChanged = -1;
	}
	if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_MacroOnSave)) {
		nMacroOnSave = index;
	}else if (nMacroOnSave == index) {
		nMacroOnSave = -1;
	}
	for (iItem=0; iItem<MAX_CUSTMACRO; ++iItem) {
		szText[0] = _T('\0');
		if (iItem == nMacroOnOpened) {
			::lstrcat(szText, _T("O"));
		}
		if (iItem == nMacroOnTypeChanged) {
			::lstrcat(szText, _T("T"));
		}
		if (iItem == nMacroOnSave) {
			::lstrcat(szText, _T("S"));
		}
		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.iItem = iItem;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 4;
		lvItem.pszText = szText;
		ListView_SetItem(hListView, &lvItem);
	}
}

/*!
	Macro�i�[�p�f�B���N�g����I������

	@param hwndDlg [in] �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��
*/
void PropMacro::SelectBaseDir_Macro(HWND hwndDlg)
{
	TCHAR szDir[_MAX_PATH];

	// �����t�H���_
	::DlgItem_GetText(hwndDlg, IDC_MACRODIR, szDir, _countof(szDir));

	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	if (_IS_REL_PATH(szDir)) {
		TCHAR folder[_MAX_PATH];
		_tcscpy(folder, szDir);
		GetInidirOrExedir(szDir, folder);
	}

	if (SelectDir(hwndDlg, LS(STR_PROPCOMMACR_SEL_DIR), szDir, szDir)) {
		//	������\\�}�[�N��ǉ�����D
		AddLastChar(szDir, _countof(szDir), _T('\\'));
		::DlgItem_SetText(hwndDlg, IDC_MACRODIR, szDir);
	}
}


/*!
	�}�N���t�@�C���w��p�R���{�{�b�N�X�̃h���b�v�_�E�����X�g���J�����Ƃ��ɁC
	�w��f�B���N�g���̃t�@�C���ꗗ������𐶐�����D

	@param hwndDlg [in] �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��
*/
void PropMacro::OnFileDropdown_Macro(HWND hwndDlg)
{
	HANDLE hFind;
	HWND hCombo = ::GetDlgItem(hwndDlg, IDC_MACROPATH);

	TCHAR path[_MAX_PATH * 2];
	::DlgItem_GetText(hwndDlg, IDC_MACRODIR, path, _countof(path));

	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	if (_IS_REL_PATH(path)) {
		TCHAR folder[_MAX_PATH * 2];
		_tcscpy(folder, path);
		GetInidirOrExedir(path, folder);
	}
	_tcscat(path, _T("*.*"));	//	2002/05/01 YAZAKI �ǂ�ȃt�@�C�����ǂ�Ɨ����B

	// ���̏�����
	Combo_ResetContent(hCombo);

	// �t�@�C���̌���
	WIN32_FIND_DATA wf;
	hFind = FindFirstFile(path, &wf);

	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	do {
		//	�R���{�{�b�N�X�ɐݒ�
		//	�ł�.��..�͊��فB
		//if (_tcscmp(wf.cFileName, _T(".")) != 0 && _tcscmp(wf.cFileName, _T("..")) != 0) {
		if ((wf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {	// 2009.02.12 ryoji �t�H���_�����O
			int result = Combo_AddString(hCombo, wf.cFileName);
			if (result == CB_ERR || result == CB_ERRSPACE) {
				break;
			}
		}
	}while (FindNextFile(hFind, &wf));

    FindClose(hFind);
}

void PropMacro::CheckListPosition_Macro(HWND hwndDlg)
{
	HWND hListView = ::GetDlgItem(hwndDlg, IDC_MACROLIST);
	HWND hNum = ::GetDlgItem(hwndDlg, IDC_COMBO_MACROID);
	
	//	���݂�Focus�擾
	int current = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);

	if (current == -1 || current == nLastPos_Macro) {
		return;
	}

	nLastPos_Macro = current;
	
	//	�����l�̐ݒ�
	Combo_SetCurSel(hNum, nLastPos_Macro);
	
	TCHAR buf[MAX_PATH + MACRONAME_MAX];	// MAX_PATH��MACRONAME_MAX�̗������傫���l
	LVITEM lvItem = {0};
	lvItem.iItem = current;
	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	lvItem.pszText = buf;
	lvItem.cchTextMax = MACRONAME_MAX;

	ListView_GetItem(hListView, &lvItem);
	::DlgItem_SetText(hwndDlg, IDC_MACRONAME, buf);

	memset_raw(&lvItem, 0, sizeof(lvItem));
	lvItem.iItem = current;
	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 2;
	lvItem.pszText = buf;
	lvItem.cchTextMax = MAX_PATH;

	ListView_GetItem(hListView, &lvItem);
	::DlgItem_SetText(hwndDlg, IDC_MACROPATH, buf);

	memset_raw(&lvItem, 0, sizeof(lvItem));
	lvItem.iItem = current;
	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 3;
	lvItem.pszText = buf;
	lvItem.cchTextMax = MAX_PATH;
	ListView_GetItem(hListView, &lvItem);
	if (_tcscmp(buf, _T("on")) == 0) {
		::CheckDlgButton(hwndDlg, IDC_CHECK_RELOADWHENEXECUTE, true);
	}else {
		::CheckDlgButton(hwndDlg, IDC_CHECK_RELOADWHENEXECUTE, false);
	}

	// �������s�}�N��	// 2006.09.01 ryoji
	memset_raw(&lvItem, 0, sizeof(lvItem));
	lvItem.iItem = current;
	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 4;
	TCHAR szText[8];
	lvItem.pszText = szText;
	lvItem.cchTextMax = _countof(szText);
	ListView_GetItem(hListView, &lvItem);
	::CheckDlgButton(hwndDlg, IDC_CHECK_MacroOnOpened, false);
	::CheckDlgButton(hwndDlg, IDC_CHECK_MacroOnTypeChanged, false);
	::CheckDlgButton(hwndDlg, IDC_CHECK_MacroOnSave, false);
	int nLen = ::lstrlen(szText);
	for (int i=0; i<nLen; ++i) {
		if (szText[i] == _T('O')) {
			::CheckDlgButton(hwndDlg, IDC_CHECK_MacroOnOpened, true);
		}
		if (szText[i] == _T('T')) {
			::CheckDlgButton(hwndDlg, IDC_CHECK_MacroOnTypeChanged, true);
		}
		if (szText[i] == _T('S')) {
			::CheckDlgButton(hwndDlg, IDC_CHECK_MacroOnSave, true);
		}
	}
}

