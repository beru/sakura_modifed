/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�^�u�o�[�v�y�[�W
*/
#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "PropertyManager.h"
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
				nPageNum = ID_PROPCOM_PAGENUM_TAB;
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
					auto& csTabBar = common.tabBar;
					LOGFONT   lf = csTabBar.lf;
					INT nPointSize = csTabBar.nPointSize;

					if (MySelectFont(&lf, &nPointSize, hwndDlg, false)) {
						csTabBar.lf = lf;
						csTabBar.nPointSize = nPointSize;
						// �^�u �t�H���g�\��	// 2013/4/24 Uchi
						HFONT hFont = SetFontLabel(hwndDlg, IDC_STATIC_TABFONT, csTabBar.lf, csTabBar.nPointSize);
						if (hTabFont) {
							::DeleteObject(hTabFont);
						}
						hTabFont = hFont;
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
		if (hTabFont) {
			::DeleteObject(hTabFont);
			hTabFont = NULL;
		}
		return TRUE;
	}
	return FALSE;
}


// �_�C�A���O�f�[�^�̐ݒ�
void PropTab::SetData(HWND hwndDlg)
{
	auto& csTabBar = common.tabBar;

	//	Feb. 11, 2007 genta�u�E�B���h�E�v�V�[�g���ړ�
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_DispTabWnd, csTabBar.bDispTabWnd);	//@@@ 2003.05.31 MIK
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_SameTabWidth, csTabBar.bSameTabWidth);	//@@@ 2006.01.28 ryoji
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_DispTabIcon, csTabBar.bDispTabIcon);	//@@@ 2006.01.28 ryoji
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_SortTabList, csTabBar.bSortTabList);			//@@@ 2006.03.23 fon
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_TAB_MULTILINE, csTabBar.bTabMultiLine );
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_GroupMultiTabWnd, !csTabBar.bDispTabWndMultiWin); //@@@ 2003.05.31 MIK
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_TABWND_CAPTION), _countof(csTabBar.szTabWndCaption) - 1);
	::DlgItem_SetText(hwndDlg, IDC_TABWND_CAPTION, csTabBar.szTabWndCaption);

	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_CHECK_DispTabClose);
	Combo_ResetContent(hwndCombo);
	int nSelPos = 0;
	for (size_t i=0; i<_countof(DispTabCloseArr); ++i) {
		Combo_InsertString(hwndCombo, i, LS(DispTabCloseArr[i].nNameId));
		if (DispTabCloseArr[i].nMethod == common.tabBar.dispTabClose) {
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_TAB_POSITION);
	Combo_ResetContent(hwndCombo);
	nSelPos = 0;
	for (size_t i=0; i<_countof(TabPosArr); ++i) {
		Combo_InsertString(hwndCombo, i, LS(TabPosArr[i].nNameId));
		if (TabPosArr[i].nMethod == common.tabBar.eTabPosition) {
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	//	Feb. 11, 2007 genta �V�K�쐬
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_RetainEmptyWindow, csTabBar.bTab_RetainEmptyWin);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_CloseOneWin, csTabBar.bTab_CloseOneWin);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_ChgWndByWheel, csTabBar.bChgWndByWheel);	// 2007.04.03 ryoji
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_OpenNewWin, csTabBar.bNewWindow); // 2009.06.17

	// �^�u �t�H���g	// 2013/4/24 Uchi
	hTabFont = SetFontLabel(hwndDlg, IDC_STATIC_TABFONT, csTabBar.lf, csTabBar.nPointSize);

	EnableTabPropInput(hwndDlg);
}

// �_�C�A���O�f�[�^�̎擾
int PropTab::GetData(HWND hwndDlg)
{
	auto& csTabBar = common.tabBar;
	//	Feb. 11, 2007 genta�u�E�B���h�E�v�V�[�g���ړ�
	csTabBar.bDispTabWnd = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DispTabWnd);
	csTabBar.bSameTabWidth = DlgButton_IsChecked(hwndDlg, IDC_CHECK_SameTabWidth);		// 2006.01.28 ryoji
	csTabBar.bDispTabIcon = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DispTabIcon);		// 2006.01.28 ryoji
	csTabBar.bSortTabList = DlgButton_IsChecked(hwndDlg, IDC_CHECK_SortTabList);		// 2006.03.23 fon
	csTabBar.bDispTabWndMultiWin = !DlgButton_IsChecked(hwndDlg, IDC_CHECK_GroupMultiTabWnd);
	::DlgItem_GetText(hwndDlg, IDC_TABWND_CAPTION, csTabBar.szTabWndCaption, _countof(csTabBar.szTabWndCaption));

	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_CHECK_DispTabClose);
	int nSelPos = Combo_GetCurSel(hwndCombo);
	csTabBar.dispTabClose = DispTabCloseArr[nSelPos].nMethod;

	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_TAB_POSITION);
	nSelPos = Combo_GetCurSel(hwndCombo);
	csTabBar.eTabPosition = TabPosArr[nSelPos].nMethod;

	csTabBar.bTab_RetainEmptyWin = DlgButton_IsChecked(hwndDlg, IDC_CHECK_RetainEmptyWindow);
	csTabBar.bTab_CloseOneWin = DlgButton_IsChecked(hwndDlg, IDC_CHECK_CloseOneWin);
	csTabBar.bChgWndByWheel = DlgButton_IsChecked(hwndDlg, IDC_CHECK_ChgWndByWheel);
	csTabBar.bNewWindow = DlgButton_IsChecked(hwndDlg, IDC_CHECK_OpenNewWin);

	return TRUE;
}

/*! �u�^�u�o�[�v�V�[�g��̃A�C�e���̗L���E������K�؂ɐݒ肷�� */
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
	DlgItem_Enable(hwndDlg, IDC_CHECK_OpenNewWin,			bGroupTabWin);
	DlgItem_Enable(hwndDlg, IDC_CHECK_DispTabIcon,			bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_CHECK_SameTabWidth,			bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_CHECK_DispTabClose,			bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_TextTabClose,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_TextTabCaption,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_BUTTON_TABFONT,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_STATIC_TABFONT,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_CHECK_SortTabList,			bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_CHECK_TAB_MULTILINE,		bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_TAB_POSITION,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_COMBO_TAB_POSITION,			bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_TABWND_CAPTION,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_CHECK_ChgWndByWheel,		bTabWnd);
}

