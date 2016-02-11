/*!	@file
	@brief �t�@�C����r�_�C�A���O�{�b�N�X

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, JEPRO
	Copyright (C) 2001, Stonee, genta, JEPRO, YAZAKI
	Copyright (C) 2002, aroka, MIK, Moca
	Copyright (C) 2003, MIK
	Copyright (C) 2006, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "dlg/CDlgCompare.h"
#include "window/CEditWnd.h"
#include "func/Funccode.h"		// Stonee, 2001/03/12
#include "util/shell.h"
#include "util/file.h"
#include "util/string_ex2.h"
#include "sakura_rc.h"
#include "sakura.hh"

// �t�@�C�����e��r DlgCompare.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12300
//	IDC_STATIC,						-1,
	IDOK,							HIDOK_CMP,					// OK
	IDCANCEL,						HIDCANCEL_CMP,				// �L�����Z��
	IDC_BUTTON_HELP,				HIDC_CMP_BUTTON_HELP,		// �w���v
	IDC_CHECK_TILE_H,				HIDC_CMP_CHECK_TILE_H,		// ���E�ɕ\��
	IDC_LIST_FILES,					HIDC_CMP_LIST_FILES,		// �t�@�C���ꗗ
	IDC_STATIC_COMPARESRC,			HIDC_CMP_STATIC_COMPARESRC,	// �\�[�X�t�@�C��
	0, 0
};	//@@@ 2002.01.07 add end MIK

static const AnchorListItem anchorList[] = {
	{IDOK,					AnchorStyle::Bottom},
	{IDCANCEL,				AnchorStyle::Bottom},
	{IDC_BUTTON_HELP,		AnchorStyle::Bottom},
	{IDC_CHECK_TILE_H,		AnchorStyle::Left},
	{IDC_LIST_FILES,        AnchorStyle::All},
	{IDC_STATIC_COMPARESRC, AnchorStyle::LeftRight},
};

DlgCompare::DlgCompare()
	:
	Dialog(true)
{
	// �T�C�Y�ύX���Ɉʒu�𐧌䂷��R���g���[����
	assert(_countof(anchorList) == _countof(m_rcItems));

	m_bCompareAndTileHorz = true;	// ���E�ɕ��ׂĕ\��

	m_ptDefaultSize.x = -1;
	m_ptDefaultSize.y = -1;
	return;
}


// ���[�_���_�C�A���O�̕\��
int DlgCompare::DoModal(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	LPARAM			lParam,
	const TCHAR*	pszPath,
	TCHAR*			pszCompareLabel,
	HWND*			phwndCompareWnd
	)
{
	m_pszPath = pszPath;
	m_pszCompareLabel = pszCompareLabel;
	m_phwndCompareWnd = phwndCompareWnd;
	return Dialog::DoModal(hInstance, hwndParent, IDD_COMPARE, lParam);
}

BOOL DlgCompare::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_HELP:
		//�u���e��r�v�̃w���v
		// Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_COMPARE));	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
// From Here Oct. 10, 2000 JEPRO added  Ref. code ��CDlgFind.cpp �� OnBnClicked
// �`�F�b�N�{�b�N�X���{�^��������DlgCompare.cpp�ɒ��ڏ�������ł݂������s
// �_�C�A���O�̃{�^���͉��ɕs�������Ă����Ă���܂��B
// �ȉ��̒ǉ��R�[�h�͑S�������Č��\�ł�����N������Ă��������B�����X�N���[��������Ă����ƂȂ����ꂵ���ł��B
//	case IDC_BUTTON1:	/* �㉺�ɕ\�� */
//		/* �_�C�A���O�f�[�^�̎擾 */
//		return TRUE;
//	case IDOK:			/* ���E�ɕ\�� */
//		/* �_�C�A���O�f�[�^�̎擾 */
//		HWND	hwndCompareWnd;
//		HWND*	phwndArr;
//		int		i;
//		phwndArr = new HWND[2];
//		phwndArr[0] = ::GetParent(m_hwndParent);
//		phwndArr[1] = hwndCompareWnd;
//		for (i=0; i<2; ++i) {
//			if (::IsZoomed(phwndArr[i])) {
//				::ShowWindow(phwndArr[i], SW_RESTORE);
//			}
//		}
//		::TileWindows(NULL, MDITILE_VERTICAL, NULL, 2, phwndArr);
//		delete [] phwndArr;
//		CloseDialog(0);
//		return TRUE;
// To Here Oct. 10, 2000
	case IDOK:			// ���E�ɕ\��
		// �_�C�A���O�f�[�^�̎擾
		::EndDialog(GetHwnd(), GetData());
		return TRUE;
	case IDCANCEL:
		::EndDialog(GetHwnd(), FALSE);
		return TRUE;
	}
	// ���N���X�����o
	return Dialog::OnBnClicked(wID);
}


// �_�C�A���O�f�[�^�̐ݒ�
void DlgCompare::SetData(void)
{
	EditNode*	pEditNodeArr;
	TCHAR		szMenu[512];
	int			selIndex = 0;

	HWND hwndList = GetItemHwnd(IDC_LIST_FILES);
// 2002/2/10 aroka �t�@�C�����Ŕ�r���Ȃ����ߕs�p (2001.12.26 YAZAKI����)
//	//	Oct. 15, 2001 genta �t�@�C��������� stricmp��bcc�ł����Ғʂ蓮��������
//	setlocale (LC_ALL, "C");

	// ���݊J���Ă���ҏW���̃��X�g�����j���[�ɂ���
	int nRowNum = AppNodeManager::getInstance()->GetOpenedWindowArr(&pEditNodeArr, TRUE);
	if (nRowNum > 0) {
		// �����X�N���[�����͎��ۂɕ\�����镶����̕����v�����Č��߂�	// 2009.09.26 ryoji
		TextWidthCalc calc(hwndList);
		int score = 0;
		TCHAR szFile1[_MAX_PATH];
		SplitPath_FolderAndFile(m_pszPath, NULL, szFile1);
		for (int i=0; i<nRowNum; ++i) {
			// �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm
			::SendMessage(pEditNodeArr[i].GetHwnd(), MYWM_GETFILEINFO, 0, 0);
			EditInfo* pfi = (EditInfo*)&m_pShareData->m_workBuffer.m_EditInfo_MYWM_GETFILEINFO;

//@@@ 2001.12.26 YAZAKI �t�@�C�����Ŕ�r�����(����)�������Ƃ��ɖ�蓯�m�̔�r���ł��Ȃ�
			if (pEditNodeArr[i].GetHwnd() == EditWnd::getInstance()->GetHwnd()) {
				// 2010.07.30 �����̖��O����������ݒ肷��
				FileNameManager::getInstance()->GetMenuFullLabel_WinListNoEscape( szMenu, _countof(szMenu), pfi, pEditNodeArr[i].m_nId, -1, calc.GetDC() );
				SetItemText(IDC_STATIC_COMPARESRC, szMenu);
				continue;
			}
			// �ԍ��� �E�B���h�E���X�g�Ɠ����ɂȂ�悤�ɂ���
			FileNameManager::getInstance()->GetMenuFullLabel_WinListNoEscape( szMenu, _countof(szMenu), pfi, pEditNodeArr[i].m_nId, i, calc.GetDC() );

			int nItem = ::List_AddString(hwndList, szMenu);
			List_SetItemData(hwndList, nItem, pEditNodeArr[i].GetHwnd());

			// �������v�Z����
			calc.SetTextWidthIfMax(szMenu);

			// �t�@�C������v�̃X�R�A���v�Z����
			TCHAR szFile2[_MAX_PATH];
			SplitPath_FolderAndFile(pfi->m_szPath, NULL, szFile2);
			int scoreTemp = FileMatchScoreSepExt(szFile1, szFile2);
			if (score < scoreTemp) {
				// �X�R�A�̂������̂�I��
				score = scoreTemp;
				selIndex = nItem;
			}
		}
		delete [] pEditNodeArr;
		// 2002/11/01 Moca �ǉ� ���X�g�r���[�̉�����ݒ�B��������Ȃ��Ɛ����X�N���[���o�[���g���Ȃ�
		List_SetHorizontalExtent( hwndList, calc.GetCx() );
	}
	List_SetCurSel(hwndList, selIndex);

	// ���E�ɕ��ׂĕ\��
	//@@@ 2003.06.12 MIK
	// TAB 1�E�B���h�E�\���̂Ƃ��͕��ׂĔ�r�ł��Ȃ�����
	if (m_pShareData->m_common.m_sTabBar.m_bDispTabWnd
		&& !m_pShareData->m_common.m_sTabBar.m_bDispTabWndMultiWin
	) {
		m_bCompareAndTileHorz = false;
		EnableItem(IDC_CHECK_TILE_H, false);
	}
	CheckButton(IDC_CHECK_TILE_H, m_bCompareAndTileHorz);
	return;
}


// �_�C�A���O�f�[�^�̎擾
// TRUE==����  FALSE==���̓G���[
int DlgCompare::GetData(void)
{
	HWND hwndList = GetItemHwnd(IDC_LIST_FILES);
	int nItem = List_GetCurSel(hwndList);
	if (nItem == LB_ERR) {
		return FALSE;
	}else {
		*m_phwndCompareWnd = (HWND)List_GetItemData(hwndList, nItem);
		// �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm
		::SendMessage(*m_phwndCompareWnd, MYWM_GETFILEINFO, 0, 0);
		EditInfo* pfi = (EditInfo*)&m_pShareData->m_workBuffer.m_EditInfo_MYWM_GETFILEINFO;

		// 2010.07.30 �p�X���͂�߂ĕ\�����ɕύX
		int nId = AppNodeManager::getInstance()->GetEditNode(*m_phwndCompareWnd)->GetId();
		TextWidthCalc calc(hwndList);
		FileNameManager::getInstance()->GetMenuFullLabel_WinListNoEscape( m_pszCompareLabel, _MAX_PATH/*�����s��*/, pfi, nId, -1, calc.GetDC() );

		// ���E�ɕ��ׂĕ\��
		m_bCompareAndTileHorz = IsButtonChecked(IDC_CHECK_TILE_H);

		return TRUE;
	}
}

//@@@ 2002.01.18 add start
LPVOID DlgCompare::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

INT_PTR DlgCompare::DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR result = Dialog::DispatchEvent(hWnd, wMsg, wParam, lParam);
	if (wMsg == WM_GETMINMAXINFO) {
		return OnMinMaxInfo(lParam);
	}
	return result;
}

BOOL DlgCompare::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	_SetHwnd(hwndDlg);

	CreateSizeBox();
	Dialog::OnSize();
	
	RECT rc;
	::GetWindowRect(hwndDlg, &rc);
	m_ptDefaultSize.x = rc.right - rc.left;
	m_ptDefaultSize.y = rc.bottom - rc.top;

	for (int i=0; i<_countof(anchorList); ++i) {
		GetItemClientRect(anchorList[i].id, m_rcItems[i]);
	}

	RECT rcDialog = GetDllShareData().m_common.m_sOthers.m_rcCompareDialog;
	if (rcDialog.left != 0
		|| rcDialog.bottom != 0
	) {
		m_xPos = rcDialog.left;
		m_yPos = rcDialog.top;
		m_nWidth = rcDialog.right - rcDialog.left;
		m_nHeight = rcDialog.bottom - rcDialog.top;
	}

	return Dialog::OnInitDialog(hwndDlg, wParam, lParam);
}

BOOL DlgCompare::OnSize(WPARAM wParam, LPARAM lParam)
{
	// ���N���X�����o
	Dialog::OnSize(wParam, lParam);

	GetWindowRect(&GetDllShareData().m_common.m_sOthers.m_rcCompareDialog);

	RECT rc;
	GetWindowRect(&rc);
	POINT ptNew;
	ptNew.x = rc.right - rc.left;
	ptNew.y = rc.bottom - rc.top;
	for (int i=0; i<_countof(anchorList); ++i) {
		ResizeItem(GetItemHwnd(anchorList[i].id), m_ptDefaultSize, ptNew, m_rcItems[i], anchorList[i].anchor);
	}
	::InvalidateRect(GetHwnd(), NULL, TRUE);
	return TRUE;
}

BOOL DlgCompare::OnMove(WPARAM wParam, LPARAM lParam)
{
	GetWindowRect(&GetDllShareData().m_common.m_sOthers.m_rcCompareDialog);
	return Dialog::OnMove(wParam, lParam);
}

BOOL DlgCompare::OnMinMaxInfo(LPARAM lParam)
{
	LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
	if (m_ptDefaultSize.x < 0) {
		return 0;
	}
	lpmmi->ptMinTrackSize.x = m_ptDefaultSize.x;
	lpmmi->ptMinTrackSize.y = m_ptDefaultSize.y;
	lpmmi->ptMaxTrackSize.x = m_ptDefaultSize.x*2;
	lpmmi->ptMaxTrackSize.y = m_ptDefaultSize.y*3;
	return 0;
}


