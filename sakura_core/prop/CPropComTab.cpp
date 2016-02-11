/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�^�u�o�[�v�y�[�W

	@author Norio Nakatani
	@date 2007.02.11 genta ���ʐݒ�ɐV�K�^�u��ǉ�
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2007, genta
	Copyright (C) 2001, MIK, genta
	Copyright (C) 2002, YAZAKI, MIK
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, genta, ryoji
	Copyright (C) 2012, Moca
	Copyright (C) 2013, Uchi

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
#include "prop/CPropCommon.h"
#include "CPropertyManager.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"


static const DWORD p_helpids[] = {
	IDC_CHECK_DispTabWnd,			HIDC_CHECK_DispTabWnd,			// �^�u�E�B���h�E�\��	//@@@ 2003.05.31 MIK
	IDC_CHECK_GroupMultiTabWnd,		HIDC_CHECK_GroupMultiTabWnd,	// �E�B���h�E���܂Ƃ߂ăO���[�v������
	IDC_CHECK_RetainEmptyWindow,	HIDC_CHECK_RetainEmptyWindow,	// �Ō�̃t�@�C��������Ƃ�(����)�������c��	// 2007.02.13 ryoji
	IDC_CHECK_CloseOneWin,			HIDC_CHECK_CloseOneWin,			// �E�B���h�E�̕���{�^���͌��݂̃t�@�C���̂ݕ���	// 2007.02.13 ryoji
	IDC_CHECK_OpenNewWin,			HIDC_CHECK_OpenNewWin,			// �O������N������Ƃ��͐V�����E�B���h�E�ŊJ�� 2009.06.19
	IDC_CHECK_DispTabIcon,			HIDC_CHECK_DispTabIcon,			// �A�C�R���\��	// 2006.08.06 ryoji
	IDC_CHECK_SameTabWidth,			HIDC_CHECK_SameTabWidth,		// ����	// 2006.08.06 ryoji
	IDC_CHECK_DispTabClose,			HIDC_CHECK_DispTabClose,		// �^�u�����{�^���\��	// 2012.04.14 syat
	IDC_BUTTON_TABFONT,				HIDC_BUTTON_TABFONT,			// �^�u�t�H���g
	IDC_CHECK_SortTabList,			HIDC_CHECK_SortTabList,			// �^�u�ꗗ�\�[�g	// 2006.08.06 ryoji
	IDC_CHECK_TAB_MULTILINE,		HIDC_CHECK_TAB_MULTILINE,		// �^�u���i
	IDC_COMBO_TAB_POSITION,			HIDC_COMBO_TAB_POSITION,		// �^�u�\���ʒu
	IDC_TABWND_CAPTION,				HIDC_TABWND_CAPTION,			// �^�u�E�B���h�E�L���v�V����	//@@@ 2003.06.15 MIK
	IDC_CHECK_ChgWndByWheel,		HIDC_CHECK_ChgWndByWheel,		// �}�E�X�z�C�[���ŃE�B���h�E�؂�ւ� 2007.04.03 ryoji
	0, 0
};

TYPE_NAME_ID<DispTabCloseType> DispTabCloseArr[] = {
	{ DispTabCloseType::No,		STR_PROPCOMTAB_DISP_NO },
	{ DispTabCloseType::Always,	STR_PROPCOMTAB_DISP_ALLWAYS },
	{ DispTabCloseType::Auto,	STR_PROPCOMTAB_DISP_AUTO },
};

TYPE_NAME_ID<TabPosition> TabPosArr[] = {
	{ TabPosition::Top, STR_PROPCOMTAB_TAB_POS_TOP },
	{ TabPosition::Bottom, STR_PROPCOMTAB_TAB_POS_BOTTOM },
#if 0
	{ TabPosition::Left, STR_PROPCOMTAB_TAB_POS_LEFT },
	{ TabPosition::Right, STR_PROPCOMTAB_TAB_POS_RIGHT },
#endif
};

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg[in] �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg[in] ���b�Z�[�W
	@param wParam[in] �p�����[�^1
	@param lParam[in] �p�����[�^2
*/
INT_PTR CALLBACK PropTab::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropTab::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}
//	To Here Jun. 2, 2001 genta

// ���b�Z�[�W����
INT_PTR PropTab::DispatchEvent(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	NMHDR*		pNMHDR;
//	int			idCtrl;

	switch (uMsg) {
	case WM_INITDIALOG:
		// �_�C�A���O�f�[�^�̐ݒ� Tab
		SetData(hwndDlg);
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌�����
		return TRUE;
		
	case WM_NOTIFY:
//		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
//		switch (idCtrl) {
//		default:
			switch (pNMHDR->code) {
			case PSN_HELP:
				OnHelp(hwndDlg, IDD_PROP_TAB);
				return TRUE;
			case PSN_KILLACTIVE:
				// �_�C�A���O�f�[�^�̎擾 Tab
				GetData(hwndDlg);
				return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			case PSN_SETACTIVE:
				m_nPageNum = ID_PROPCOM_PAGENUM_TAB;
				return TRUE;
			}
//			break;	// default
//		}
		break;	// WM_NOTIFY

	case WM_COMMAND:
		{
			WORD wNotifyCode = HIWORD(wParam);	// �ʒm�R�[�h
			WORD wID = LOWORD(wParam);	// ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID
			if (wNotifyCode == BN_CLICKED) {
				switch (wID) {
				case IDC_CHECK_DispTabWnd:
				case IDC_CHECK_GroupMultiTabWnd:
					EnableTabPropInput(hwndDlg);
					break;
				case IDC_BUTTON_TABFONT:
					auto& csTabBar = m_common.m_tabBar;
					LOGFONT   lf = csTabBar.m_lf;
					INT nPointSize = csTabBar.m_nPointSize;

					if (MySelectFont(&lf, &nPointSize, hwndDlg, false)) {
						csTabBar.m_lf = lf;
						csTabBar.m_nPointSize = nPointSize;
						// �^�u �t�H���g�\��	// 2013/4/24 Uchi
						HFONT hFont = SetFontLabel(hwndDlg, IDC_STATIC_TABFONT, csTabBar.m_lf, csTabBar.m_nPointSize);
						if (m_hTabFont) {
							::DeleteObject(m_hTabFont);
						}
						m_hTabFont = hFont;
					}
					break;
				}
			}
		}
		break;
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

	case WM_DESTROY:
		// �^�u �t�H���g�j��	// 2013/4/24 Uchi
		if (m_hTabFont) {
			::DeleteObject(m_hTabFont);
			m_hTabFont = NULL;
		}
		return TRUE;
	}
	return FALSE;
}


// �_�C�A���O�f�[�^�̐ݒ�
void PropTab::SetData(HWND hwndDlg)
{
	auto& csTabBar = m_common.m_tabBar;

	//	Feb. 11, 2007 genta�u�E�B���h�E�v�V�[�g���ړ�
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_DispTabWnd, csTabBar.m_bDispTabWnd);	//@@@ 2003.05.31 MIK
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_SameTabWidth, csTabBar.m_bSameTabWidth);	//@@@ 2006.01.28 ryoji
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_DispTabIcon, csTabBar.m_bDispTabIcon);	//@@@ 2006.01.28 ryoji
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_SortTabList, csTabBar.m_bSortTabList);			//@@@ 2006.03.23 fon
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_TAB_MULTILINE, csTabBar.m_bTabMultiLine );
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_GroupMultiTabWnd, !csTabBar.m_bDispTabWndMultiWin); //@@@ 2003.05.31 MIK
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_TABWND_CAPTION), _countof(csTabBar.m_szTabWndCaption) - 1);
	::DlgItem_SetText(hwndDlg, IDC_TABWND_CAPTION, csTabBar.m_szTabWndCaption);

	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_CHECK_DispTabClose);
	Combo_ResetContent(hwndCombo);
	int nSelPos = 0;
	for (int i=0; i<_countof(DispTabCloseArr); ++i) {
		Combo_InsertString(hwndCombo, i, LS(DispTabCloseArr[i].nNameId));
		if (DispTabCloseArr[i].nMethod == m_common.m_tabBar.m_dispTabClose) {
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_TAB_POSITION);
	Combo_ResetContent(hwndCombo);
	nSelPos = 0;
	for (int i=0; i<_countof(TabPosArr); ++i) {
		Combo_InsertString(hwndCombo, i, LS(TabPosArr[i].nNameId));
		if (TabPosArr[i].nMethod == m_common.m_tabBar.m_eTabPosition) {
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	//	Feb. 11, 2007 genta �V�K�쐬
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_RetainEmptyWindow, csTabBar.m_bTab_RetainEmptyWin);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_CloseOneWin, csTabBar.m_bTab_CloseOneWin);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_ChgWndByWheel, csTabBar.m_bChgWndByWheel);	// 2007.04.03 ryoji
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_OpenNewWin, csTabBar.m_bNewWindow); // 2009.06.17

	// �^�u �t�H���g	// 2013/4/24 Uchi
	m_hTabFont = SetFontLabel(hwndDlg, IDC_STATIC_TABFONT, csTabBar.m_lf, csTabBar.m_nPointSize);

	EnableTabPropInput(hwndDlg);
}

// �_�C�A���O�f�[�^�̎擾
int PropTab::GetData(HWND hwndDlg)
{
	auto& csTabBar = m_common.m_tabBar;
	//	Feb. 11, 2007 genta�u�E�B���h�E�v�V�[�g���ړ�
	csTabBar.m_bDispTabWnd = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DispTabWnd);
	csTabBar.m_bSameTabWidth = DlgButton_IsChecked(hwndDlg, IDC_CHECK_SameTabWidth);		// 2006.01.28 ryoji
	csTabBar.m_bDispTabIcon = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DispTabIcon);		// 2006.01.28 ryoji
	csTabBar.m_bSortTabList = DlgButton_IsChecked(hwndDlg, IDC_CHECK_SortTabList);		// 2006.03.23 fon
	csTabBar.m_bDispTabWndMultiWin = !DlgButton_IsChecked(hwndDlg, IDC_CHECK_GroupMultiTabWnd);
	::DlgItem_GetText(hwndDlg, IDC_TABWND_CAPTION, csTabBar.m_szTabWndCaption, _countof(csTabBar.m_szTabWndCaption));

	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_CHECK_DispTabClose);
	int nSelPos = Combo_GetCurSel(hwndCombo);
	csTabBar.m_dispTabClose = DispTabCloseArr[nSelPos].nMethod;

	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_TAB_POSITION);
	nSelPos = Combo_GetCurSel(hwndCombo);
	csTabBar.m_eTabPosition = TabPosArr[nSelPos].nMethod;

	//	Feb. 11, 2007 genta �V�K�쐬
	csTabBar.m_bTab_RetainEmptyWin = DlgButton_IsChecked(hwndDlg, IDC_CHECK_RetainEmptyWindow);
	csTabBar.m_bTab_CloseOneWin = DlgButton_IsChecked(hwndDlg, IDC_CHECK_CloseOneWin);
	csTabBar.m_bChgWndByWheel = DlgButton_IsChecked(hwndDlg, IDC_CHECK_ChgWndByWheel);	// 2007.04.03 ryoji
	csTabBar.m_bNewWindow = DlgButton_IsChecked(hwndDlg, IDC_CHECK_OpenNewWin);  // 2009.06.17

	return TRUE;
}

/*! �u�^�u�o�[�v�V�[�g��̃A�C�e���̗L���E������K�؂ɐݒ肷��

	@date 2007.02.12 genta �V�K�쐬
*/
void PropTab::EnableTabPropInput(HWND hwndDlg)
{
	
	bool bTabWnd = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DispTabWnd);
	bool bGroupTabWin = false;
	if (bTabWnd) {
		bGroupTabWin = DlgButton_IsChecked(hwndDlg, IDC_CHECK_GroupMultiTabWnd);
	}
	DlgItem_Enable(hwndDlg, IDC_CHECK_GroupMultiTabWnd,		bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_CHECK_RetainEmptyWindow,	bGroupTabWin);
	DlgItem_Enable(hwndDlg, IDC_CHECK_CloseOneWin,			bGroupTabWin);
	DlgItem_Enable(hwndDlg, IDC_CHECK_OpenNewWin,			bGroupTabWin);	// 2009.06.17
	DlgItem_Enable(hwndDlg, IDC_CHECK_DispTabIcon,			bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_CHECK_SameTabWidth,			bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_CHECK_DispTabClose,			bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_TextTabClose,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_TextTabCaption,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_BUTTON_TABFONT,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_STATIC_TABFONT,				bTabWnd);	// 2013/4/24 Uchi
	DlgItem_Enable(hwndDlg, IDC_CHECK_SortTabList,			bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_CHECK_TAB_MULTILINE,		bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_TAB_POSITION,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_COMBO_TAB_POSITION,			bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_TABWND_CAPTION,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_CHECK_ChgWndByWheel,		bTabWnd);	// 2007.04.03 ryoji
}

