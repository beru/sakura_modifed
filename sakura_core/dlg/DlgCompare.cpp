/*!	@file
	@brief �t�@�C����r�_�C�A���O�{�b�N�X
*/
#include "StdAfx.h"
#include "dlg/DlgCompare.h"
#include "window/EditWnd.h"
#include "func/Funccode.h"
#include "util/shell.h"
#include "util/fileUtil.h"
#include "util/string_ex2.h"
#include "sakura_rc.h"
#include "sakura.hh"

// �t�@�C�����e��r DlgCompare.cpp
const DWORD p_helpids[] = {	//12300
//	IDC_STATIC,						-1,
	IDOK,							HIDOK_CMP,					// OK
	IDCANCEL,						HIDCANCEL_CMP,				// �L�����Z��
	IDC_BUTTON_HELP,				HIDC_CMP_BUTTON_HELP,		// �w���v
	IDC_CHECK_TILE_H,				HIDC_CMP_CHECK_TILE_H,		// ���E�ɕ\��
	IDC_LIST_FILES,					HIDC_CMP_LIST_FILES,		// �t�@�C���ꗗ
	IDC_STATIC_COMPARESRC,			HIDC_CMP_STATIC_COMPARESRC,	// �\�[�X�t�@�C��
	0, 0
};

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
	Dialog(true),
	pszPath(NULL)
{
	// �T�C�Y�ύX���Ɉʒu�𐧌䂷��R���g���[����
	assert(_countof(anchorList) == _countof(rcItems));

	bCompareAndTileHorz = true;	// ���E�ɕ��ׂĕ\��

	ptDefaultSize.x = -1;
	ptDefaultSize.y = -1;
	return;
}


// ���[�_���_�C�A���O�̕\��
INT_PTR DlgCompare::DoModal(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	LPARAM			lParam,
	const TCHAR*	pszPath,
	TCHAR*			pszCompareLabel,
	HWND*			phwndCompareWnd
	)
{
	this->pszPath = pszPath;
	this->pszCompareLabel = pszCompareLabel;
	this->phwndCompareWnd = phwndCompareWnd;
	return Dialog::DoModal(hInstance, hwndParent, IDD_COMPARE, lParam);
}

BOOL DlgCompare::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_HELP:
		//�u���e��r�v�̃w���v
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_COMPARE));
		return TRUE;
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
//		phwndArr[0] = ::GetParent(hwndParent);
//		phwndArr[1] = hwndCompareWnd;
//		for (i=0; i<2; ++i) {
//			if (::IsZoomed(phwndArr[i])) {
//				::ShowWindow(phwndArr[i], SW_RESTORE);
//			}
//		}
//		::TileWindows(NULL, MDITILE_VERTICAL, NULL, 2, phwndArr);
//		delete[] phwndArr;
//		CloseDialog(0);
//		return TRUE;
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

	// ���݊J���Ă���ҏW���̃��X�g�����j���[�ɂ���
	size_t nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true);
	if (nRowNum > 0) {
		// �����X�N���[�����͎��ۂɕ\�����镶����̕����v�����Č��߂�
		TextWidthCalc calc(hwndList);
		size_t score = 0;
		TCHAR szFile1[_MAX_PATH];
		SplitPath_FolderAndFile(pszPath, NULL, szFile1);
		for (size_t i=0; i<nRowNum; ++i) {
			// �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm
			::SendMessage(pEditNodeArr[i].GetHwnd(), MYWM_GETFILEINFO, 0, 0);
			EditInfo* pfi = (EditInfo*)&pShareData->workBuffer.editInfo_MYWM_GETFILEINFO;

			if (pEditNodeArr[i].GetHwnd() == EditWnd::getInstance().GetHwnd()) {
				// �����̖��O����������ݒ肷��
				FileNameManager::getInstance().GetMenuFullLabel_WinListNoEscape(szMenu, _countof(szMenu), pfi, pEditNodeArr[i].nId, -1, calc.GetDC());
				SetItemText(IDC_STATIC_COMPARESRC, szMenu);
				continue;
			}
			// �ԍ��� �E�B���h�E���X�g�Ɠ����ɂȂ�悤�ɂ���
			FileNameManager::getInstance().GetMenuFullLabel_WinListNoEscape(szMenu, _countof(szMenu), pfi, pEditNodeArr[i].nId, i, calc.GetDC());

			LRESULT nItem = ::List_AddString(hwndList, szMenu);
			List_SetItemData(hwndList, nItem, pEditNodeArr[i].GetHwnd());

			// �������v�Z����
			calc.SetTextWidthIfMax(szMenu);

			// �t�@�C������v�̃X�R�A���v�Z����
			TCHAR szFile2[_MAX_PATH];
			SplitPath_FolderAndFile(pfi->szPath, NULL, szFile2);
			size_t scoreTemp = FileMatchScoreSepExt(szFile1, szFile2);
			if (score < scoreTemp) {
				// �X�R�A�̂������̂�I��
				score = scoreTemp;
				selIndex = nItem;
			}
		}
		delete[] pEditNodeArr;
		// ���X�g�r���[�̉�����ݒ�B��������Ȃ��Ɛ����X�N���[���o�[���g���Ȃ�
		List_SetHorizontalExtent( hwndList, calc.GetCx() );
	}
	List_SetCurSel(hwndList, selIndex);

	// ���E�ɕ��ׂĕ\��
	// TAB 1�E�B���h�E�\���̂Ƃ��͕��ׂĔ�r�ł��Ȃ�����
	if (pShareData->common.tabBar.bDispTabWnd
		&& !pShareData->common.tabBar.bDispTabWndMultiWin
	) {
		bCompareAndTileHorz = false;
		EnableItem(IDC_CHECK_TILE_H, false);
	}
	CheckButton(IDC_CHECK_TILE_H, bCompareAndTileHorz);
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
		*phwndCompareWnd = (HWND)List_GetItemData(hwndList, nItem);
		// �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm
		::SendMessage(*phwndCompareWnd, MYWM_GETFILEINFO, 0, 0);
		EditInfo* pfi = (EditInfo*)&pShareData->workBuffer.editInfo_MYWM_GETFILEINFO;

		int nId = AppNodeManager::getInstance().GetEditNode(*phwndCompareWnd)->GetId();
		TextWidthCalc calc(hwndList);
		FileNameManager::getInstance().GetMenuFullLabel_WinListNoEscape(pszCompareLabel, _MAX_PATH/*�����s��*/, pfi, nId, -1, calc.GetDC());

		// ���E�ɕ��ׂĕ\��
		bCompareAndTileHorz = IsButtonChecked(IDC_CHECK_TILE_H);

		return TRUE;
	}
}

LPVOID DlgCompare::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

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
	ptDefaultSize.x = rc.right - rc.left;
	ptDefaultSize.y = rc.bottom - rc.top;

	for (size_t i=0; i<_countof(anchorList); ++i) {
		GetItemClientRect(anchorList[i].id, rcItems[i]);
	}

	RECT rcDialog = GetDllShareData().common.others.rcCompareDialog;
	if (rcDialog.left != 0
		|| rcDialog.bottom != 0
	) {
		xPos = rcDialog.left;
		yPos = rcDialog.top;
		nWidth = rcDialog.right - rcDialog.left;
		nHeight = rcDialog.bottom - rcDialog.top;
	}

	return Dialog::OnInitDialog(hwndDlg, wParam, lParam);
}

BOOL DlgCompare::OnSize(WPARAM wParam, LPARAM lParam)
{
	// ���N���X�����o
	Dialog::OnSize(wParam, lParam);

	GetWindowRect(&GetDllShareData().common.others.rcCompareDialog);

	RECT rc;
	GetWindowRect(&rc);
	POINT ptNew;
	ptNew.x = rc.right - rc.left;
	ptNew.y = rc.bottom - rc.top;
	for (size_t i=0; i<_countof(anchorList); ++i) {
		ResizeItem(GetItemHwnd(anchorList[i].id), ptDefaultSize, ptNew, rcItems[i], anchorList[i].anchor);
	}
	::InvalidateRect(GetHwnd(), NULL, TRUE);
	return TRUE;
}

BOOL DlgCompare::OnMove(WPARAM wParam, LPARAM lParam)
{
	GetWindowRect(&GetDllShareData().common.others.rcCompareDialog);
	return Dialog::OnMove(wParam, lParam);
}

BOOL DlgCompare::OnMinMaxInfo(LPARAM lParam)
{
	LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
	if (ptDefaultSize.x < 0) {
		return 0;
	}
	lpmmi->ptMinTrackSize.x = ptDefaultSize.x;
	lpmmi->ptMinTrackSize.y = ptDefaultSize.y;
	lpmmi->ptMaxTrackSize.x = ptDefaultSize.x*2;
	lpmmi->ptMaxTrackSize.y = ptDefaultSize.y*3;
	return 0;
}


