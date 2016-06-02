/*!	@file
	@brief �v���O�C���ݒ�_�C�A���O�{�b�N�X

	@author Uchi
	@date 2010/3/22
*/
/*
	Copyright (C) 2010, Uchi

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
#include "dlg/DlgPluginOption.h"
#include "prop/PropCommon.h"
#include "util/shell.h"
#include "util/window.h"
#include "util/string_ex2.h"
#include "util/module.h"
#include "sakura_rc.h"
#include "sakura.hh"

// BOOL�ϐ��̕\��
#define	BOOL_DISP_TRUE	_T("\u2611")
#define	BOOL_DISP_FALSE	_T("\u2610")

// �ҏW�̈��\���A��\���ɂ���
static inline void CtrlShow(
	HWND hwndDlg,
	int id,
	BOOL bShow
	)
{
	HWND hWnd = ::GetDlgItem(hwndDlg, id);
	::ShowWindow(hWnd, bShow ? SW_SHOW: SW_HIDE);
	::EnableWindow(hWnd, bShow);
}

const DWORD p_helpids[] = {
	IDC_LIST_PLUGIN_OPTIONS,		HIDC_LIST_PLUGIN_OPTIONS,		// �I�v�V�������X�g
	IDC_EDIT_PLUGIN_OPTION,			HIDC_EDIT_PLUGIN_OPTION,		// �I�v�V�����ҏW
	IDC_EDIT_PLUGIN_OPTION_DIR,		HIDC_EDIT_PLUGIN_OPTION,		// �I�v�V�����ҏW
	IDC_BUTTON_PLUGIN_OPTION_DIR,	HIDC_EDIT_PLUGIN_OPTION,		// �I�v�V�����ҏW
	IDC_EDIT_PLUGIN_OPTION_NUM,		HIDC_EDIT_PLUGIN_OPTION,		// �I�v�V�����ҏW
	IDC_SPIN_PLUGIN_OPTION,			HIDC_EDIT_PLUGIN_OPTION,		// �I�v�V�����ҏW
	IDC_CHECK_PLUGIN_OPTION,		HIDC_EDIT_PLUGIN_OPTION,		// �I�v�V�����ҏW
	IDC_COMBO_PLUGIN_OPTION,		HIDC_EDIT_PLUGIN_OPTION,		// �I�v�V�����ҏW
	IDOK,							HIDC_FAVORITE_IDOK,				// OK
	IDCANCEL,						HIDC_FAVORITE_IDCANCEL,			// �L�����Z��
	IDC_PLUGIN_README,				HIDC_PLUGIN_README,				// ReadMe
	IDC_BUTTON_HELP,				HIDC_BUTTON_FAVORITE_HELP,		// �w���v
//	IDC_STATIC,						-1,
	0, 0
};

DlgPluginOption::DlgPluginOption(PropPlugin& propPlugin)
	:
	propPlugin(propPlugin)
{

}

DlgPluginOption::~DlgPluginOption()
{

}

// ���[�_���_�C�A���O�̕\��
INT_PTR DlgPluginOption::DoModal(
	HINSTANCE			hInstance,
	HWND				hwndParent,
	int 				ID
	)
{
	// �v���O�C���ԍ��i�G�f�B�^���ӂ�ԍ��j
	id = ID;
	plugin = PluginManager::getInstance().GetPlugin(id);
	if (!plugin) {
		::ErrorMessage(hwndParent, LS(STR_DLGPLUGINOPT_LOAD));
		return 0;
	}

	return Dialog::DoModal(hInstance, hwndParent, IDD_PLUGIN_OPTION, (LPARAM)NULL);
}

// �_�C�A���O�f�[�^�̐ݒ�
void DlgPluginOption::SetData(void)
{
	HWND	hwndList;
	int		i;
	LV_ITEM	lvi;
	TCHAR	buf[MAX_LENGTH_VALUE + 1];
	bool bLoadDefault = false;

	// �^�C�g��
	auto_sprintf(buf, LS(STR_DLGPLUGINOPT_TITLE), plugin->sName.c_str());
	::SetWindowText(GetHwnd(), buf);

	// ���X�g
	hwndList = GetItemHwnd(IDC_LIST_PLUGIN_OPTIONS);
	ListView_DeleteAllItems(hwndList);	// ���X�g����ɂ���
	line = -1;							// �s��I��

	auto profile = std::make_unique<DataProfile>();
	profile->SetReadingMode();
	profile->ReadProfile(plugin->GetOptionPath().c_str());

	PluginOption* pOpt;
	PluginOption::ArrayIter it;
	for (i=0, it=plugin->options.begin(); it!=plugin->options.end(); ++i, ++it) {
		pOpt = *it;

		auto_snprintf_s(buf, _countof(buf), _T("%ls"), pOpt->GetLabel().c_str());
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = buf;
		lvi.iItem    = i;
		lvi.iSubItem = 0;
		lvi.lParam   = pOpt->GetIndex();
		ListView_InsertItem(hwndList, &lvi);

		wstring sSection;
		wstring sKey;
		wstring sValue;
		wstring sType;

		pOpt->GetKey(&sSection, &sKey);
		if (sSection.empty() || sKey.empty()) {
			sValue = L"";
		}else {
			if (!profile->IOProfileData(sSection.c_str(), sKey.c_str(), sValue)) {
				// Option��������Ȃ�������Default�l��ݒ�
				sValue = pOpt->GetDefaultVal();
				if (sValue != wstring(L"")) {
					bLoadDefault = true;
					profile->SetWritingMode();
					profile->IOProfileData(sSection.c_str(), sKey.c_str(), sValue);
					profile->SetReadingMode();
				}
			}
		}

		if (pOpt->GetType() == OPTION_TYPE_BOOL) {
			_tcscpy(buf, sValue == wstring(L"0") || sValue == wstring(L"") ? BOOL_DISP_FALSE : BOOL_DISP_TRUE);
		}else if (pOpt->GetType() == OPTION_TYPE_INT) {
			// ���l�֐��K��
			auto_sprintf( buf, _T("%d"), _wtoi(sValue.c_str()));
		}else if (pOpt->GetType() == OPTION_TYPE_SEL) {
			// �l����\����
			wstring	sView;
			wstring	sTrg;
			std::vector<wstring> selects = pOpt->GetSelects();
			buf[0] = 0;
			for (auto it=selects.begin(); it!=selects.end(); ++it) {
				SepSelect(*it, &sView, &sTrg);
				if (sValue == sTrg) {
					auto_snprintf_s(buf, _countof(buf), _T("%ls"), sView.c_str());
					break;
				}
			}
		}else {
			auto_snprintf_s(buf, _countof(buf), _T("%ls"), sValue.c_str());
		}
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = i;
		lvi.iSubItem = 1;
		lvi.pszText  = buf;
		ListView_SetItem(hwndList, &lvi);
		ListView_SetItemState(hwndList, i, 0, LVIS_SELECTED | LVIS_FOCUSED);
	}

	if (bLoadDefault) {
		profile->SetWritingMode();
		profile->WriteProfile(plugin->GetOptionPath().c_str(), (plugin->sName + LSW(STR_DLGPLUGINOPT_INIHEAD)).c_str());
	}

	if (i == 0) {
		// �I�v�V����������
		EnableItem(IDC_LIST_PLUGIN_OPTIONS, false);
		EnableItem(IDOK, false);
	
		SetItemText(IDC_STATIC_MSG, LS(STR_DLGPLUGINOPT_OPTION));
	}

	// ReadMe Button
	sReadMeName = propPlugin.GetReadMeFile(to_tchar(pShareData->common.plugin.pluginTable[id].szName));
	EnableItem(IDC_PLUGIN_README, !sReadMeName.empty());
	return;
}

// �_�C�A���O�f�[�^�̎擾
// TRUE==����  FALSE==���̓G���[
int DlgPluginOption::GetData(void)
{
	// .ini �t�@�C���ւ̏�������
	LV_ITEM	lvi;

	// ���X�g
	HWND hwndList = GetItemHwnd(IDC_LIST_PLUGIN_OPTIONS);

	auto profile = std::make_unique<DataProfile>();
	profile->SetReadingMode();
	profile->ReadProfile(plugin->GetOptionPath().c_str());
	profile->SetWritingMode();

	PluginOption* pOpt;
	TCHAR	buf[MAX_LENGTH_VALUE + 1];
	PluginOption::ArrayIter it;
	int i;
	for (i=0, it=plugin->options.begin(); it!=plugin->options.end(); ++i, ++it) {
		pOpt = *it;

		memset_raw(&lvi, 0, sizeof(lvi));
		lvi.mask       = LVIF_TEXT;
		lvi.iItem      = i;
		lvi.iSubItem   = 1;
		lvi.pszText    = buf;
		lvi.cchTextMax = MAX_LENGTH_VALUE + 1;
		ListView_GetItem(hwndList, &lvi);

		if (pOpt->GetType() == OPTION_TYPE_BOOL) {
			if (_tcscmp(buf,  BOOL_DISP_FALSE) == 0) {
				_tcscpy (buf, _T("0"));
			}else {
				_tcscpy (buf, _T("1"));
			}
		}else if (pOpt->GetType() == OPTION_TYPE_SEL) {
			// �\������l��
			wstring	sView;
			wstring	sTrg;
			std::vector<wstring>	selects;
			selects = pOpt->GetSelects();
			wstring sWbuf = to_wchar(buf);

			for (auto it=selects.begin(); it!=selects.end(); ++it) {
				SepSelect(*it, &sView, &sTrg);
				if (sView == sWbuf) {
					auto_sprintf( buf, _T("%ls"), sTrg.c_str());
					break;
				}
			}
		}

		wstring sSection;
		wstring sKey;
		wstring sValue;

		pOpt->GetKey(&sSection, &sKey);
		if (sSection.empty() || sKey.empty()) {
			continue;
		}

		sValue = to_wchar(buf);

		profile->IOProfileData(sSection.c_str(), sKey.c_str(), sValue);
	}

	profile->WriteProfile(
		plugin->GetOptionPath().c_str(),
		(plugin->sName + LSW(STR_DLGPLUGINOPT_INIHEAD)).c_str()
		);

	return TRUE;
}

BOOL DlgPluginOption::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	LV_COLUMN	col;
	RECT		rc;
	long		lngStyle;

	_SetHwnd(hwndDlg);

	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_PLUGIN_OPTIONS);
	::GetWindowRect(hwndList, &rc);

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 40 / 100;
	col.pszText  = const_cast<TCHAR*>(LS(STR_DLGPLUGINOPT_LIST1));
	col.iSubItem = 0;
	ListView_InsertColumn(hwndList, 0, &col);

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 55 / 100;
	col.pszText  = const_cast<TCHAR*>(LS(STR_DLGPLUGINOPT_LIST2));
	col.iSubItem = 1;
	ListView_InsertColumn(hwndList, 1, &col);

	// �s�I��
	lngStyle = ListView_GetExtendedListViewStyle(hwndList);
	lngStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle(hwndList, lngStyle);

	// �ҏW�̈�̔�A�N�e�B�u��
	::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_PLUGIN_OPTION), FALSE);
	CtrlShow(hwndDlg, IDC_EDIT_PLUGIN_OPTION_DIR,  FALSE);
	CtrlShow(hwndDlg, IDC_BUTTON_PLUGIN_OPTION_DIR,FALSE);
	CtrlShow(hwndDlg, IDC_EDIT_PLUGIN_OPTION_NUM,  FALSE);
	CtrlShow(hwndDlg, IDC_SPIN_PLUGIN_OPTION,      FALSE);
	CtrlShow(hwndDlg, IDC_CHECK_PLUGIN_OPTION,     FALSE);
	CtrlShow(hwndDlg, IDC_COMBO_PLUGIN_OPTION,     FALSE);

	// ��������
	EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_PLUGIN_OPTION   ), MAX_LENGTH_VALUE);
	EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_PLUGIN_OPTION_DIR), _MAX_PATH);
	EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_PLUGIN_OPTION_NUM), 11);

	// ���N���X�����o
	return Dialog::OnInitDialog(GetHwnd(), wParam, lParam);
}

BOOL DlgPluginOption::OnNotify(WPARAM wParam, LPARAM lParam)
{
	NMHDR* pNMHDR;
	int idCtrl = (int)wParam;
	switch (idCtrl) {
	case IDC_LIST_PLUGIN_OPTIONS:
		pNMHDR = (NMHDR*)lParam;
		switch (pNMHDR->code) {
		case LVN_ITEMCHANGED:
			ChangeListPosition();
			break;
		case NM_DBLCLK:
			// ���X�g�r���[�ւ̃_�u���N���b�N�ŕҏW�̈�ֈړ�	2013/5/23 Uchi
			MoveFocusToEdit();
			break;
		}
		return TRUE;

	case IDC_SPIN_PLUGIN_OPTION:
		int			nVal;
		NM_UPDOWN*	pMNUD;
		
		pMNUD  = (NM_UPDOWN*)lParam;

		nVal = GetItemInt(IDC_EDIT_PLUGIN_OPTION_NUM, NULL, TRUE);
		if (pMNUD->iDelta < 0) {
			if (nVal < INT_MAX) ++nVal;
		}else if (pMNUD->iDelta > 0) {
			// INT_MIN�� SetDlgItemInt �ň����Ȃ�
			if (nVal > -INT_MAX) --nVal;
		}
		SetItemInt(IDC_EDIT_PLUGIN_OPTION_NUM, nVal, TRUE);

		// �ҏW���̃f�[�^�̖߂�
		SetFromEdit(line);
		return TRUE;
	}

	// ���N���X�����o
	return Dialog::OnNotify(wParam, lParam);
}


BOOL DlgPluginOption::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_CHECK_PLUGIN_OPTION:
		// �ҏW���̃f�[�^�̖߂�
		SetFromEdit(line);
		return TRUE;

	case IDC_BUTTON_PLUGIN_OPTION_DIR:	// 2013/05/22 Uchi
		// �f�B���N�g���I��
		SelectDirectory(line);
		return TRUE;

	case IDC_PLUGIN_README:		// 2012/12/22 Uchi
		// ReadMe
		{
			if (!sReadMeName.empty()) {
				if (!propPlugin.BrowseReadMe(sReadMeName)) {
					WarningMessage(GetHwnd(), LS(STR_PROPCOMPLG_ERR2));
				}
			}else {
				WarningMessage(GetHwnd(), LS(STR_PROPCOMPLG_ERR3));
			}
		}
		return TRUE;

	case IDC_BUTTON_HELP:
		// �w���v
		MyWinHelp(GetHwnd(), HELP_CONTEXT, HLP000153);	// �w�v���O�C���ݒ�xHelp�̎w�� 	2011/11/26 Uchi
		return TRUE;

	case IDOK:
		// �ҏW���̃f�[�^�̖߂�
		SetFromEdit(line);
		// �_�C�A���O�f�[�^�̎擾
		::EndDialog(GetHwnd(), (BOOL)GetData());
		return TRUE;

	case IDCANCEL:
		::EndDialog(GetHwnd(), FALSE);
		return TRUE;
	}

	// ���N���X�����o
	return Dialog::OnBnClicked(wID);
}

BOOL DlgPluginOption::OnCbnSelChange(HWND hwndCtl, int wID)
{
	switch (wID) {
	case IDC_COMBO_PLUGIN_OPTION:
		// �ҏW���̃f�[�^�̖߂�
		SetFromEdit(line);
		return TRUE;
	}

	// ���N���X�����o
	return Dialog::OnCbnSelChange(hwndCtl, wID);
}


BOOL DlgPluginOption::OnEnChange(HWND hwndCtl, int wID)
{
	switch (wID) {
	case IDC_EDIT_PLUGIN_OPTION:
	case IDC_EDIT_PLUGIN_OPTION_DIR:
	case IDC_EDIT_PLUGIN_OPTION_NUM:
		// �ҏW���̃f�[�^�̖߂�
		SetFromEdit(line);
		return TRUE;
	}

	// ���N���X�����o
	return Dialog::OnEnChange(hwndCtl, wID);
}


BOOL DlgPluginOption::OnActivate(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam)) {
	case WA_INACTIVE:
		SetFromEdit(line);
		break;
	case WA_ACTIVE:
	case WA_CLICKACTIVE:
	default:
		break;
	}

	// ���N���X�����o
	return Dialog::OnActivate(wParam, lParam);
}


LPVOID DlgPluginOption::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}


void DlgPluginOption::ChangeListPosition(void)
{
	HWND hwndList = GetItemHwnd(IDC_LIST_PLUGIN_OPTIONS);

	// ���݂�Focus�擾
	int current = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);
	if (current == -1 || current == line) {
		return;
	}

	TCHAR	buf[MAX_LENGTH_VALUE + 1];
	LVITEM	lvi;

	// �߂�
	if (line >= 0) {
		SetFromEdit(line);
	}

	line = current;

	// �ҏW�̈�ɏ�������
	SetToEdit(current);

	memset_raw(&lvi, 0, sizeof(lvi));
	lvi.mask       = LVIF_TEXT;
	lvi.iItem      = current;
	lvi.iSubItem   = 1;
	lvi.pszText    = buf;
	lvi.cchTextMax = MAX_LENGTH_VALUE + 1;

	ListView_GetItem(hwndList, &lvi);
	SetItemText(IDC_EDIT_PLUGIN_OPTION, buf);
}

void DlgPluginOption::MoveFocusToEdit(void)
{
	// ���݂�Focus�擾
	int iLine = ListView_GetNextItem(GetItemHwnd(IDC_LIST_PLUGIN_OPTIONS), -1, LVNI_SELECTED);
	wstring	sType;
	HWND hwndCtrl;

	if (iLine >= 0) {
		// Focus�̐؂�ւ�
		sType = plugin->options[iLine]->GetType();
		transform(sType.begin(), sType.end(), sType.begin(), my_towlower2);
		if (sType == OPTION_TYPE_BOOL) {
			hwndCtrl = GetItemHwnd(IDC_CHECK_PLUGIN_OPTION);
		}else if (sType == OPTION_TYPE_INT) {
			hwndCtrl = GetItemHwnd(IDC_EDIT_PLUGIN_OPTION_NUM);
		}else if (sType == OPTION_TYPE_SEL) {
			hwndCtrl = GetItemHwnd(IDC_COMBO_PLUGIN_OPTION);
		}else if (sType == OPTION_TYPE_DIR) {
			hwndCtrl = GetItemHwnd(IDC_EDIT_PLUGIN_OPTION_DIR);
		}else {
			hwndCtrl = GetItemHwnd(IDC_EDIT_PLUGIN_OPTION);
		}
		::SetFocus(hwndCtrl);
	}
}


// �ҏW�̈�ɏ�������
void DlgPluginOption::SetToEdit(int iLine)
{
	HWND hwndList = GetItemHwnd(IDC_LIST_PLUGIN_OPTIONS);

	TCHAR buf[MAX_LENGTH_VALUE + 1];
	LVITEM lvi;
	wstring	sType;

	if (iLine >= 0) {
		GetItemText(IDC_EDIT_PLUGIN_OPTION, buf, MAX_LENGTH_VALUE + 1);
		memset_raw(&lvi, 0, sizeof(lvi));
		lvi.mask       = LVIF_TEXT;
		lvi.iItem      = iLine;
		lvi.iSubItem   = 1;
		lvi.pszText    = buf;
		lvi.cchTextMax = MAX_LENGTH_VALUE + 1;
		ListView_GetItem(hwndList, &lvi);

		sType = plugin->options[iLine]->GetType();
		transform(sType.begin(), sType.end(), sType.begin(), my_towlower2);
		if (sType == OPTION_TYPE_BOOL) {
			CheckButton(IDC_CHECK_PLUGIN_OPTION, _tcscmp(buf,  BOOL_DISP_FALSE) != 0);
			SetItemText(IDC_CHECK_PLUGIN_OPTION, plugin->options[iLine]->GetLabel().c_str());

			// �ҏW�̈�̐؂�ւ�
			SelectEdit(IDC_CHECK_PLUGIN_OPTION);
		}else if (sType == OPTION_TYPE_INT) {
			SetItemText(IDC_EDIT_PLUGIN_OPTION_NUM, buf);

			// �ҏW�̈�̐؂�ւ�
			SelectEdit(IDC_EDIT_PLUGIN_OPTION_NUM);
		}else if (sType == OPTION_TYPE_SEL) {
			// CONBO �ݒ�
			std::vector<wstring> selects;
			selects = plugin->options[iLine]->GetSelects();

			HWND hwndCombo = GetItemHwnd(IDC_COMBO_PLUGIN_OPTION);
			Combo_ResetContent(hwndCombo);

			wstring	sView;
			wstring	sValue;
			wstring	sWbuf = to_wchar(buf);
			int nSelIdx = -1;		// �I��
			int i = 0;
			for (auto it=selects.begin(); it!=selects.end(); ++it) {
				SepSelect(*it, &sView, &sValue);
				LONG_PTR nItemIdx = Combo_AddString(hwndCombo, sView.c_str());
				if (sView == sWbuf) {
					nSelIdx = i;
				}
				Combo_SetItemData(hwndCombo, nItemIdx, i++);
			}
			Combo_SetCurSel(hwndCombo, nSelIdx);

			// �ҏW�̈�̐؂�ւ�
			SelectEdit(IDC_COMBO_PLUGIN_OPTION);
		}else if (sType == OPTION_TYPE_DIR) {
			SetItemText(IDC_EDIT_PLUGIN_OPTION_DIR, buf);

			// �ҏW�̈�̐؂�ւ�
			SelectEdit(IDC_EDIT_PLUGIN_OPTION_DIR);
		}else {
			SetItemText(IDC_EDIT_PLUGIN_OPTION, buf);

			// �ҏW�̈�̐؂�ւ�
			SelectEdit(IDC_EDIT_PLUGIN_OPTION);
		}
	}
}

// �ҏW�̈�̐؂�ւ�
void DlgPluginOption::SelectEdit(int IDCenable)
{
	CtrlShow(GetHwnd(), IDC_EDIT_PLUGIN_OPTION,        (IDCenable == IDC_EDIT_PLUGIN_OPTION));
	CtrlShow(GetHwnd(), IDC_EDIT_PLUGIN_OPTION_DIR,    (IDCenable == IDC_EDIT_PLUGIN_OPTION_DIR));
	CtrlShow(GetHwnd(), IDC_BUTTON_PLUGIN_OPTION_DIR,  (IDCenable == IDC_EDIT_PLUGIN_OPTION_DIR));
	CtrlShow(GetHwnd(), IDC_EDIT_PLUGIN_OPTION_NUM,    (IDCenable == IDC_EDIT_PLUGIN_OPTION_NUM));
	CtrlShow(GetHwnd(), IDC_SPIN_PLUGIN_OPTION,        (IDCenable == IDC_EDIT_PLUGIN_OPTION_NUM));
	CtrlShow(GetHwnd(), IDC_CHECK_PLUGIN_OPTION,       (IDCenable == IDC_CHECK_PLUGIN_OPTION));
	CtrlShow(GetHwnd(), IDC_COMBO_PLUGIN_OPTION,       (IDCenable == IDC_COMBO_PLUGIN_OPTION));
}

// �ҏW�̈悩��߂�
void DlgPluginOption::SetFromEdit(int iLine)
{
	HWND hwndList = GetItemHwnd(IDC_LIST_PLUGIN_OPTIONS);

	TCHAR	buf[MAX_LENGTH_VALUE + 1];
	int		nVal;
	LVITEM	lvi;
	wstring	sType;

	if (iLine >= 0) {
		sType = plugin->options[iLine]->GetType();
		transform(sType.begin (), sType.end (), sType.begin (), my_towlower2);
		if (sType == OPTION_TYPE_BOOL) {
			if (IsButtonChecked(IDC_CHECK_PLUGIN_OPTION)) {
				_tcscpy(buf, BOOL_DISP_TRUE);
			}else {
				_tcscpy(buf, BOOL_DISP_FALSE);
			}
			lvi.mask     = LVIF_TEXT;
			lvi.iItem    = iLine;
			lvi.iSubItem = 1;
			lvi.pszText  = buf;
			ListView_SetItem(hwndList, &lvi);
		}else if (sType == OPTION_TYPE_INT) {
			nVal = GetItemInt(IDC_EDIT_PLUGIN_OPTION_NUM, NULL, TRUE);
			auto_sprintf( buf, _T("%d"), nVal);
		}else if (sType == OPTION_TYPE_SEL) {
			GetItemText(IDC_COMBO_PLUGIN_OPTION, buf, MAX_LENGTH_VALUE + 1);
		}else if (sType == OPTION_TYPE_DIR) {
			GetItemText(IDC_EDIT_PLUGIN_OPTION_DIR, buf, MAX_LENGTH_VALUE + 1);
		}else {
			GetItemText(IDC_EDIT_PLUGIN_OPTION, buf, MAX_LENGTH_VALUE + 1);
		}
		memset_raw(&lvi, 0, sizeof(lvi));
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = iLine;
		lvi.iSubItem = 1;
		lvi.pszText  = buf;
		ListView_SetItem(hwndList, &lvi);
	}
}

// �I��p�����񕪉�
void DlgPluginOption::SepSelect(
	wstring sTrg,
	wstring* spView,
	wstring* spValue
	)
{
	auto ix = sTrg.find(L':');
	if (ix == std::wstring::npos) {
		*spView = *spValue = sTrg;
	}else {
#ifdef _DEBUG
		*spView  = sTrg;
#else
		*spView  = sTrg.substr(0, ix);
#endif
		*spValue = sTrg.substr(ix + 1);
	}
}

// �f�B���N�g����I������
void DlgPluginOption::SelectDirectory(int iLine)
{
	TCHAR szDir[_MAX_PATH + 1];

	// �����t�H���_
	GetItemText(IDC_EDIT_PLUGIN_OPTION_DIR, szDir, _countof(szDir));
	if (_IS_REL_PATH(szDir)) {
		TCHAR	folder[_MAX_PATH];
		_tcscpy(folder, szDir);
		GetInidirOrExedir(szDir, folder);
	}

	// ���ږ��̎擾
	HWND hwndList = GetItemHwnd(IDC_LIST_PLUGIN_OPTIONS);
	LVITEM	lvi;
	TCHAR buf[MAX_LENGTH_VALUE + 1];
	memset_raw( &lvi, 0, sizeof( lvi ));
	lvi.mask       = LVIF_TEXT;
	lvi.iItem      = iLine;
	lvi.iSubItem   = 0;
	lvi.pszText    = buf;
	lvi.cchTextMax = MAX_LENGTH_VALUE + 1;
	ListView_GetItem(hwndList, &lvi);

	TCHAR sTitle[MAX_LENGTH_VALUE + 10];
	auto_sprintf(sTitle, LS(STR_DLGPLUGINOPT_SELECT), buf);
	if (SelectDir(GetHwnd(), (const TCHAR*)sTitle /*_T("�f�B���N�g���̑I��")*/, szDir, szDir)) {
		// ������\�}�[�N��ǉ�����D
		AddLastChar(szDir, _countof(szDir), _T('\\'));
		SetItemText(IDC_EDIT_PLUGIN_OPTION_DIR, szDir);
	}
}

