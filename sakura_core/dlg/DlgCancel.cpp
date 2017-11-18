#include "StdAfx.h"
#include "dlg/DlgCancel.h"

// �L�����Z���{�^���_�C�A���O�{�b�N�X

DlgCancel::DlgCancel()
{
	bCANCEL = false;	// IDCANCEL�{�^���������ꂽ
	bAutoCleanup = false;
}

/** �W���ȊO�̃��b�Z�[�W��ߑ�����
	@date 2008.05.28 ryoji �V�K�쐬
*/
INT_PTR DlgCancel::DispatchEvent(
	HWND hWnd,
	UINT wMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	INT_PTR result = Dialog::DispatchEvent(hWnd, wMsg, wParam, lParam);
	switch (wMsg) {
	case WM_CLOSE:
		if (bAutoCleanup) {
			::DestroyWindow(GetHwnd());
			return TRUE;
		}
		break;
	case WM_NCDESTROY:
		if (bAutoCleanup) {
			delete this;
			return TRUE;
		}
		break;
	}
	return result;
}

/** �����j����x�����s����
	@date 2008.05.28 ryoji �V�K�쐬
*/
void DlgCancel::DeleteAsync(void)
{
	bAutoCleanup = true;
	::PostMessage(GetHwnd(), WM_CLOSE, 0, 0);
}

// ���[�_���_�C�A���O�̕\��
INT_PTR DlgCancel::DoModal(
	HINSTANCE hInstance,
	HWND hwndParent,
	int nDlgTemplete
	)
{
	bCANCEL = false;	// IDCANCEL�{�^���������ꂽ
	return Dialog::DoModal(hInstance, hwndParent, nDlgTemplete, (LPARAM)NULL);
}

// ���[�h���X�_�C�A���O�̕\��
HWND DlgCancel::DoModeless(
	HINSTANCE hInstance,
	HWND hwndParent,
	int nDlgTemplete
	)
{
	bCANCEL = false;	// IDCANCEL�{�^���������ꂽ
	return Dialog::DoModeless(hInstance, hwndParent, nDlgTemplete, (LPARAM)NULL, SW_SHOW);
}

BOOL DlgCancel::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	_SetHwnd(hwndDlg);
	HICON	hIcon;
	hIcon = ::LoadIcon(NULL, IDI_ASTERISK);
//	hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_GREP));
	::SendMessage(GetHwnd(), WM_SETICON, ICON_SMALL, (LPARAM)NULL);
	::SendMessage(GetHwnd(), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	::SendMessage(GetHwnd(), WM_SETICON, ICON_BIG, (LPARAM)NULL);
	::SendMessage(GetHwnd(), WM_SETICON, ICON_BIG, (LPARAM)hIcon);


	// ���N���X�����o
//	CreateSizeBox();
	return Dialog::OnInitDialog(hwndDlg, wParam, lParam);
}

BOOL DlgCancel::OnBnClicked(int wID)
{
	switch (wID) {
	case IDCANCEL:
		bCANCEL = true;	// IDCANCEL�{�^���������ꂽ
//		CloseDialog(0);
		return TRUE;
	}
	return FALSE;
}

//@@@ 2002.01.18 add start
const DWORD p_helpids[] = {
	0, 0
};

LPVOID DlgCancel::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


