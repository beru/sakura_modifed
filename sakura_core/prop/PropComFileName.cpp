/*!	@file
	���ʐݒ�_�C�A���O�{�b�N�X�A�u�t�@�C�����\���v�y�[�W
*/
#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"


static const DWORD p_helpids[] = {	//13400
	IDC_CHECK_SHORTPATH,	HIDC_CHECK_FNAME_SHORTPATH,
	IDC_EDIT_SHORTMAXWIDTH,	HIDC_EDIT_FNAME_SHORTMAXWIDTH,
	IDC_LIST_FNAME,			HIDC_LIST_FNAME, 		// �t�@�C�����u�����X�g
	IDC_EDIT_FNAME_FROM,	HIDC_EDIT_FNAME_FROM,	// �u���O
	IDC_EDIT_FNAME_TO,		HIDC_EDIT_FNAME_TO,		// �u����
	IDC_BUTTON_FNAME_INS,	HIDC_BUTTON_FNAME_INS,	// �}��
	IDC_BUTTON_FNAME_ADD,	HIDC_BUTTON_FNAME_ADD,	// �ǉ�
	IDC_BUTTON_FNAME_UPD,	HIDC_BUTTON_FNAME_UPD,	// �X�V
	IDC_BUTTON_FNAME_DEL,	HIDC_BUTTON_FNAME_DEL,	// �폜
	IDC_BUTTON_FNAME_TOP,	HIDC_BUTTON_FNAME_TOP,	// �擪
	IDC_BUTTON_FNAME_UP,	HIDC_BUTTON_FNAME_UP,	// ���
	IDC_BUTTON_FNAME_DOWN,	HIDC_BUTTON_FNAME_DOWN,	// ����
	IDC_BUTTON_FNAME_LAST,	HIDC_BUTTON_FNAME_LAST,	// �ŏI
//	IDC_CHECK_FNAME,		HIDC_CHECK_FNAME,	// �t�@�C�������ȈՕ\������
	0, 0 // 
};


INT_PTR CALLBACK PropFileName::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropFileName::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}

INT_PTR PropFileName::DispatchEvent(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	HWND hListView;
	int nIndex;
	TCHAR szFrom[_MAX_PATH];
	TCHAR szTo[_MAX_PATH];

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			RECT rc;
			LV_COLUMN	col;
			hListView = GetDlgItem(hwndDlg, IDC_LIST_FNAME);

			::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
			::GetWindowRect(hListView, &rc);
			col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			col.fmt      = LVCFMT_LEFT;
			col.cx       = (rc.right - rc.left) * 60 / 100;
			col.pszText  = const_cast<TCHAR*>(LS(STR_PROPCOMFNM_LIST1));
			col.iSubItem = 0;
			ListView_InsertColumn(hListView, 0, &col);
			col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			col.fmt      = LVCFMT_LEFT;
			col.cx       = (rc.right - rc.left) * 35 / 100;
			col.pszText  = const_cast<TCHAR*>(LS(STR_PROPCOMFNM_LIST2));
			col.iSubItem = 1;
			ListView_InsertColumn(hListView, 1, &col);

			// �_�C�A���O���J�����Ƃ��Ƀ��X�g���I������Ă��Ă��t�B�[���h����̏ꍇ��������
			nLastPos_FILENAME = -1;

			// �_�C�A���O�f�[�^�̐ݒ�
			SetData(hwndDlg);

			// �G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌�����
			EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_SHORTMAXWIDTH ), 4 );
			EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_FNAME_FROM), _MAX_PATH - 1);
			EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_FNAME_TO),   _MAX_PATH - 1);
		}
		return TRUE;

	case WM_NOTIFY:
		{
			NMHDR* pNMHDR = (NMHDR*)lParam;
			int idCtrl = (int)wParam;

			switch (idCtrl) {
			case IDC_LIST_FNAME:
				switch (pNMHDR->code) {
				case LVN_ITEMCHANGED:
					hListView = GetDlgItem(hwndDlg, IDC_LIST_FNAME);
					nIndex = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
					// ���I��
					if (nIndex == -1) {
						::DlgItem_SetText(hwndDlg, IDC_EDIT_FNAME_FROM, _T(""));
						::DlgItem_SetText(hwndDlg, IDC_EDIT_FNAME_TO, _T(""));
					}else if (nIndex != nLastPos_FILENAME) {
						GetListViewItem_FILENAME(hListView, nIndex, szFrom, szTo);
						::DlgItem_SetText(hwndDlg, IDC_EDIT_FNAME_FROM, szFrom);
						::DlgItem_SetText(hwndDlg, IDC_EDIT_FNAME_TO, szTo);
					}else {
						// nIndex == nLastPos_FILENAME�̂Ƃ�
						// ���X�g���G�f�B�b�g�{�b�N�X�Ƀf�[�^���R�s�[�����[�X�V]�����܂����܂����삵�Ȃ�
					}
					nLastPos_FILENAME = nIndex;
					break;
				}
				break;
			default:
				switch (pNMHDR->code) {
				case PSN_HELP:
					OnHelp(hwndDlg, IDD_PROP_FNAME);
					return TRUE;
				case PSN_KILLACTIVE:
					// �_�C�A���O�f�[�^�̎擾
					GetData(hwndDlg);
					return TRUE;
	//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
				case PSN_SETACTIVE:
					nPageNum = ID_PROPCOM_PAGENUM_FILENAME;
					return TRUE;
				}
				break;
			}
		}
		break;

	case WM_COMMAND:
		{
			WORD	wNotifyCode = HIWORD(wParam);	// �ʒm�R�[�h
			WORD	wID = LOWORD(wParam);			// ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID
			int		nCount;

			switch (wNotifyCode) {
			// �{�^�����N���b�N���ꂽ
			case BN_CLICKED:
				hListView = GetDlgItem(hwndDlg, IDC_LIST_FNAME);
				nIndex = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
				switch (wID) {
				case IDC_BUTTON_FNAME_INS:	// �}��
					// �I�𒆂̃L�[��T��
					nCount = ListView_GetItemCount(hListView);
					if (nIndex == -1) {
						// �I�𒆂łȂ���΍Ō�ɒǉ�
						nIndex = nCount;
					}
					::DlgItem_GetText(hwndDlg, IDC_EDIT_FNAME_FROM, szFrom, _MAX_PATH);
					::DlgItem_GetText(hwndDlg, IDC_EDIT_FNAME_TO,   szTo,   _MAX_PATH);

					if (SetListViewItem_FILENAME(hListView, nIndex, szFrom, szTo, true) != -1) {
						return TRUE;
					}
					break;
				case IDC_BUTTON_FNAME_ADD:	// �ǉ�
					nCount = ListView_GetItemCount(hListView);

					::DlgItem_GetText(hwndDlg, IDC_EDIT_FNAME_FROM, szFrom, _MAX_PATH);
					::DlgItem_GetText(hwndDlg, IDC_EDIT_FNAME_TO,   szTo,   _MAX_PATH);
					
					if (SetListViewItem_FILENAME(hListView, nCount, szFrom, szTo, true) != -1) {
						return TRUE;
					}
					break;

				case IDC_BUTTON_FNAME_UPD:	// �X�V
					if (nIndex != -1) {
						::DlgItem_GetText(hwndDlg, IDC_EDIT_FNAME_FROM, szFrom, _MAX_PATH);
						::DlgItem_GetText(hwndDlg, IDC_EDIT_FNAME_TO,   szTo,   _MAX_PATH);
						if (SetListViewItem_FILENAME(hListView, nIndex, szFrom, szTo, false) != -1) {
							return TRUE;
						}
					}else {
						// ���I���Ń��X�g�ɂЂƂ����ڂ��Ȃ��ꍇ�͒ǉ����Ă���
						if (ListView_GetItemCount(hListView) == 0) {
							if (SetListViewItem_FILENAME(hListView, 0, szFrom, szTo, true) != -1) {
								return TRUE;
							}
						}
					}
					break;
				case IDC_BUTTON_FNAME_DEL:	// �폜
					if (nIndex != -1) {
						ListView_DeleteItem(hListView, nIndex);	// �Â��L�[���폜
						ListView_SetItemState(hListView, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
						return TRUE;
					}
					break;
				case IDC_BUTTON_FNAME_TOP:	// �擪
					if (MoveListViewItem_FILENAME(hListView, nIndex, 0) != -1) {
						return TRUE;
					}
					break;
				case IDC_BUTTON_FNAME_UP: 	// ���
					if (MoveListViewItem_FILENAME(hListView, nIndex, nIndex - 1) != -1) {
						return TRUE;
					}
					break;
				case IDC_BUTTON_FNAME_DOWN:	// ����
					if (MoveListViewItem_FILENAME(hListView, nIndex, nIndex + 1) != -1) {
						return TRUE;
					}
					break;
				case IDC_BUTTON_FNAME_LAST:	// �ŏI
					nCount = ListView_GetItemCount(hListView);
					if (MoveListViewItem_FILENAME(hListView, nIndex, nCount - 1) != -1) {
						return TRUE;
					}
					break;
				// default:
				}
				break;
			// default:
			}
		}

		break;	// WM_COMMAND
	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids);
		}
		return TRUE;

	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids);
		return TRUE;

	}
	return FALSE;
}


/*!
	�_�C�A���O��̃R���g���[���Ƀf�[�^��ݒ肷��

	@param hwndDlg �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��
*/
void PropFileName::SetData(HWND hwndDlg)
{
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_SHORTPATH, common.fileName.bTransformShortPath );
	::SetDlgItemInt( hwndDlg, IDC_EDIT_SHORTMAXWIDTH, common.fileName.nTransformShortMaxWidth, FALSE );

	// �t�@�C�����u�����X�g
	HWND hListView = ::GetDlgItem(hwndDlg, IDC_LIST_FNAME);
	ListView_DeleteAllItems(hListView); // ���X�g����ɂ���

	auto& csFileName = common.fileName;
	// ���X�g�Ƀf�[�^���Z�b�g
	int nIndex = 0;
	for (int i=0; i<csFileName.nTransformFileNameArrNum; ++i) {
		if ('\0' == csFileName.szTransformFileNameFrom[i][0]) {
			continue;
		}

		LVITEM lvItem = {0};
		lvItem.mask     = LVIF_TEXT;
		lvItem.iItem    = nIndex;
		lvItem.iSubItem = 0;
		lvItem.pszText  = csFileName.szTransformFileNameFrom[i];
		ListView_InsertItem(hListView, &lvItem);

		::ZeroMemory(&lvItem, sizeof_raw(lvItem));
		lvItem.mask     = LVIF_TEXT;
		lvItem.iItem    = nIndex;
		lvItem.iSubItem = 1;
		lvItem.pszText  = csFileName.szTransformFileNameTo[i];
		ListView_SetItem(hListView, &lvItem);

		++nIndex;
	}

	// ��ԏ��I�����Ă���
	if (nIndex != 0) {
		ListView_SetItemState(hListView, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
	//	���X�g�r���[�̍s�I�����\�ɂ���D
	DWORD dwStyle;
	dwStyle = ListView_GetExtendedListViewStyle(hListView);
	dwStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle(hListView, dwStyle);

	return;
}

/*!
	�_�C�A���O��̃R���g���[������f�[�^���擾���ă������Ɋi�[����

	@param hwndDlg �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��
*/

int PropFileName::GetData(HWND hwndDlg)
{
	auto& csFileName = common.fileName;

	common.fileName.bTransformShortPath = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_SHORTPATH );
	common.fileName.nTransformShortMaxWidth = ::GetDlgItemInt( hwndDlg, IDC_EDIT_SHORTMAXWIDTH, NULL, FALSE );

	// �t�@�C�����u�����X�g
	HWND hListView = ::GetDlgItem(hwndDlg, IDC_LIST_FNAME);
	csFileName.nTransformFileNameArrNum = ListView_GetItemCount(hListView);

	for (int nIndex=0, nCount=0; nIndex<MAX_TRANSFORM_FILENAME; ++nIndex) {
		if (nIndex < csFileName.nTransformFileNameArrNum) {
			ListView_GetItemText(hListView, nIndex, 0, csFileName.szTransformFileNameFrom[nCount], _MAX_PATH);

			// �u���O������NULL��������̂Ă�
			if (L'\0' == csFileName.szTransformFileNameFrom[nCount][0]) {
				csFileName.szTransformFileNameTo[nIndex][0] = L'\0';
			}else {
				ListView_GetItemText(hListView, nIndex, 1, csFileName.szTransformFileNameTo[nCount], _MAX_PATH);
				++nCount;
			}
		}else {
			csFileName.szTransformFileNameFrom[nIndex][0] = L'\0';
			csFileName.szTransformFileNameTo[nIndex][0] = L'\0';
		}
	}

	return TRUE;
}


int PropFileName::SetListViewItem_FILENAME(
	HWND hListView,
	int nIndex,
	LPTSTR szFrom,
	LPTSTR szTo,
	bool bInsMode
	)
{
	if (szFrom[0] == _T('\0') || nIndex == -1) {
		return -1;
	}

	int nCount = ListView_GetItemCount(hListView);

	// ����ȏ�ǉ��ł��Ȃ�
	if (bInsMode && MAX_TRANSFORM_FILENAME <= nCount) {
		ErrorMessage(GetParent(hListView), LS(STR_PROPCOMFNM_ERR_REG));
		return -1;
	}

	LV_ITEM	item = {0};
	item.mask     = LVIF_TEXT;
	item.iItem    = nIndex;
	item.iSubItem = 0;
	item.pszText  = szFrom;
	if (bInsMode) {
		ListView_InsertItem(hListView, &item);
	}else {
		ListView_SetItem(hListView, &item);
	}

	::ZeroMemory(&item, sizeof_raw(item));
	item.mask     = LVIF_TEXT;
	item.iItem    = nIndex;
	item.iSubItem = 1;
	item.pszText  = szTo;
	ListView_SetItem(hListView, &item);

	ListView_SetItemState(hListView, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	return nIndex;
}


void PropFileName::GetListViewItem_FILENAME(
	HWND hListView,
	int nIndex,
	LPTSTR szFrom,
	LPTSTR szTo
	)
{
	ListView_GetItemText(hListView, nIndex, 0, szFrom, _MAX_PATH);
	ListView_GetItemText(hListView, nIndex, 1, szTo, _MAX_PATH);
}


int PropFileName::MoveListViewItem_FILENAME(
	HWND hListView,
	int nIndex,
	int nIndex2
	)
{
	TCHAR szFrom[_MAX_PATH];
	TCHAR szTo[_MAX_PATH];
	int nCount = ListView_GetItemCount(hListView);

	//	2004.03.24 dskoba
	if (nIndex > nCount - 1) {
		nIndex = nCount - 1;
	}
	if (nIndex2 > nCount - 1) {
		nIndex2 = nCount - 1;
	}
	if (nIndex < 0) {
		nIndex = 0;
	}
	if (nIndex2 < 0) {
		nIndex2 = 0;
	}
	
	if (nIndex == nIndex2) {
		return -1;
	}

	GetListViewItem_FILENAME(hListView, nIndex, szFrom, szTo);
	ListView_DeleteItem(hListView, nIndex);	// �Â��L�[���폜
	SetListViewItem_FILENAME(hListView, nIndex2, szFrom, szTo, true);
	return nIndex2;
}

