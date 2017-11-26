/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�c�[���o�[�v�y�[�W
*/
#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "uiparts/MenuDrawer.h"
#include "uiparts/ImageListMgr.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids[] = {	//11000
	IDC_BUTTON_DELETE,				HIDC_BUTTON_DELETE_TOOLBAR,				// �c�[���o�[����@�\�폜
	IDC_BUTTON_INSERTSEPARATOR,		HIDC_BUTTON_INSERTSEPARATOR_TOOLBAR,	// �Z�p���[�^�}��
	IDC_BUTTON_INSERT,				HIDC_BUTTON_INSERT_TOOLBAR,				// �c�[���o�[�֋@�\�}��
	IDC_BUTTON_ADD,					HIDC_BUTTON_ADD_TOOLBAR,				// �c�[���o�[�֋@�\�ǉ�
	IDC_BUTTON_UP,					HIDC_BUTTON_UP_TOOLBAR,					// �c�[���o�[�̋@�\����ֈړ�
	IDC_BUTTON_DOWN,				HIDC_BUTTON_DOWN_TOOLBAR,				// �c�[���o�[�̋@�\�����ֈړ�
	IDC_CHECK_TOOLBARISFLAT,		HIDC_CHECK_TOOLBARISFLAT,				// �t���b�g�ȃ{�^��
	IDC_COMBO_FUNCKIND,				HIDC_COMBO_FUNCKIND_TOOLBAR,			// �@�\�̎��
	IDC_LIST_FUNC,					HIDC_LIST_FUNC_TOOLBAR,					// �@�\�ꗗ
	IDC_LIST_RES,					HIDC_LIST_RES_TOOLBAR,					// �c�[���o�[�ꗗ
	IDC_BUTTON_INSERTWRAP,			HIDC_BUTTON_INSERTWRAP,					// �c�[���o�[�ܕ�
	IDC_LABEL_MENUFUNCKIND,			(DWORD)-1,
	IDC_LABEL_MENUFUNC,				(DWORD)-1,
	IDC_LABEL_TOOLBAR,				(DWORD)-1,
//	IDC_STATIC,						-1,
	0, 0
};

/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CALLBACK PropToolbar::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropToolbar::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}

/*!
	Owner Draw List Box�Ɏw��̒l��}������ (Windows XP�̖����p)
	
	Windows XP + manifest�̎���LB_INSERTSTRING���l0���󂯕t���Ȃ��̂�
	�Ƃ肠����0�ȊO�̒l�����Ă���0�ɐݒ肵�����ĉ������B
	1��ڂ̑}����0�łȂ���Ή��ł������͂��B
	
	@param hWnd [in] ���X�g�{�b�N�X�̃E�B���h�E�n���h��
	@param index [in] �}���ʒu
	@param value [in] �}������l
	@return �}���ʒu�B�G���[�̎���LB_ERR�܂���LB_ERRSPACE
*/
int Listbox_INSERTDATA(
	HWND hWnd,				// handle to destination window 
	int index,				// item index
	int value
	)
{
	int nIndex1 = List_InsertItemData(hWnd, index, 1);
	if (nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE) {
		TopErrorMessage(NULL, LS(STR_PROPCOMTOOL_ERR01), index, nIndex1);
		return nIndex1;
	}else if (List_SetItemData(hWnd, nIndex1, value) == LB_ERR) {
		TopErrorMessage(NULL, LS(STR_PROPCOMTOOL_ERR02), nIndex1);
		return LB_ERR;
	}
	return nIndex1;
}

/*!
	Owner Draw List Box�Ɏw��̒l��ǉ����� (Windows XP�̖����p)
	
	Windows XP + manifest�̎���LB_ADDSTRING���l0���󂯕t���Ȃ��̂�
	�Ƃ肠����0�ȊO�̒l�����Ă���0�ɐݒ肵�����ĉ������B
	1��ڂ̑}����0�łȂ���Ή��ł������͂��B
	
	@param hWnd [in] ���X�g�{�b�N�X�̃E�B���h�E�n���h��
	@param index [in] �}���ʒu
	@param value [in] �}������l
	@return �}���ʒu�B�G���[�̎���LB_ERR�܂���LB_ERRSPACE
*/
int Listbox_ADDDATA(
	HWND hWnd,              // handle to destination window 
	int value
	)
{
	int nIndex1 = List_AddItemData(hWnd, 1);
	if (nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE) {
		TopErrorMessage(NULL, LS(STR_PROPCOMTOOL_ERR03), nIndex1);
		return nIndex1;
	}else if (List_SetItemData(hWnd, nIndex1, value) == LB_ERR) {
		TopErrorMessage(NULL, LS(STR_PROPCOMTOOL_ERR04), nIndex1);
		return LB_ERR;
	}
	return nIndex1;
}


static int nToolBarListBoxTopMargin = 0;


// Toolbar ���b�Z�[�W����
INT_PTR PropToolbar::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
	)
{
	WORD				wNotifyCode;
	WORD				wID;
	HWND				hwndCtl;
	NMHDR*				pNMHDR;
	int					idCtrl;
	static HWND			hwndCombo;
	static HWND			hwndFuncList;
	static HWND			hwndResList;
	LPDRAWITEMSTRUCT	pDis;
	int					nIndex1;
	int					nIndex2;
	int					i;
	int					j;
	static int			nListItemHeight;
	LRESULT				lResult;

	switch (uMsg) {
	case WM_INITDIALOG:
		// �R���g���[���̃n���h�����擾
		hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_FUNCKIND);
		hwndFuncList = ::GetDlgItem(hwndDlg, IDC_LIST_FUNC);
		hwndResList = ::GetDlgItem(hwndDlg, IDC_LIST_RES);

		{
			TextWidthCalc calc(hwndResList);
			int nFontHeight = calc.GetTextHeight();
			nListItemHeight = pIcons->GetCy() + 2;
			if (nListItemHeight < nFontHeight) {
				nListItemHeight = nFontHeight;
				nToolBarListBoxTopMargin = 0;
			}else {
				nToolBarListBoxTopMargin = (nListItemHeight - (nFontHeight + 1)) / 2;
			}
		}
		/* �_�C�A���O�f�[�^�̐ݒ� Toolbar */
		SetData( hwndDlg );
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		// �L�[�I�����̏���
		::SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_COMBO_FUNCKIND, CBN_SELCHANGE), (LPARAM)hwndCombo);

		::SetTimer(hwndDlg, 1, 300, NULL);

		return TRUE;

	case WM_DRAWITEM:
		idCtrl = (UINT) wParam;	// �R���g���[����ID
		pDis = (LPDRAWITEMSTRUCT) lParam;	// ���ڕ`����
		switch (idCtrl) {
		case IDC_LIST_RES:	// �c�[���o�[�{�^�����ʃ��X�g
		case IDC_LIST_FUNC:	// �{�^���ꗗ���X�g
			DrawToolBarItemList(pDis);	// �c�[���o�[�{�^�����X�g�̃A�C�e���`��
			return TRUE;
		}
		return TRUE;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		switch (pNMHDR->code) {
		case PSN_HELP:
			OnHelp(hwndDlg, IDD_PROP_TOOLBAR);
			return TRUE;
		case PSN_KILLACTIVE:
			// �_�C�A���O�f�[�^�̎擾 Toolbar
			GetData(hwndDlg);
			return TRUE;
		case PSN_SETACTIVE:
			nPageNum = ID_PROPCOM_PAGENUM_TOOLBAR;
			return TRUE;
		}
		break;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// �ʒm�R�[�h
		wID = LOWORD(wParam);			// ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID
		hwndCtl = (HWND) lParam;		// �R���g���[���̃n���h��

		if (hwndResList == hwndCtl) {
			switch (wNotifyCode) {
			case LBN_SELCHANGE:
				return TRUE;
			}
		}else
		if (hwndCombo == hwndCtl) {
			switch (wNotifyCode) {
			case CBN_SELCHANGE:
				nIndex2 = Combo_GetCurSel(hwndCombo);
				if (nIndex2 == CB_ERR) {
					return TRUE;
				}
				List_ResetContent(hwndFuncList);
				// �@�\�ꗗ�ɕ�������Z�b�g (���X�g�{�b�N�X)
				size_t nNum = lookup.GetItemCount(nIndex2);
				for (size_t i=0; i<nNum; ++i) {
					nIndex1 = lookup.Pos2FuncCode(nIndex2, i);
					int nbarNo = pMenuDrawer->FindToolbarNoFromCommandId(nIndex1);
					if (nbarNo >= 0) {
						// �c�[���o�[�{�^���̏����Z�b�g (���X�g�{�b�N�X)
						lResult = ::Listbox_ADDDATA(hwndFuncList, (LPARAM)nbarNo);
						if (lResult == LB_ERR || lResult == LB_ERRSPACE) {
							break;
						}
						lResult = List_SetItemHeight(hwndFuncList, lResult, nListItemHeight);
					}

				}
				return TRUE;
			}
		}else {
			switch (wNotifyCode) {
			// �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ
			case BN_CLICKED:
				switch (wID) {
				case IDC_BUTTON_INSERTSEPARATOR:
					nIndex1 = List_GetCurSel(hwndResList);
					if (nIndex1 == LB_ERR) {
						nIndex1 = 0;
					}
					nIndex1 = ::Listbox_INSERTDATA(hwndResList, nIndex1, 0);
					if (nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE) {
						break;
					}
					List_SetCurSel(hwndResList, nIndex1);
					break;

        // �ܕԃ{�^���������ꂽ��A�E�̃��X�g�Ɂu�c�[���o�[�ܕԁv��ǉ�����B
				case IDC_BUTTON_INSERTWRAP:
					nIndex1 = List_GetCurSel(hwndResList);
					if (nIndex1 == LB_ERR) {
						nIndex1 = 0;
					}
					nIndex1 = ::Listbox_INSERTDATA(hwndResList, nIndex1, MenuDrawer::TOOLBAR_BUTTON_F_TOOLBARWRAP);
					if (nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE) {
						break;
					}
					List_SetCurSel(hwndResList, nIndex1);
					break;

				case IDC_BUTTON_DELETE:
					nIndex1 = List_GetCurSel(hwndResList);
					if (nIndex1 == LB_ERR) {
						break;
					}
					i = List_DeleteString(hwndResList, nIndex1);
					if (i == LB_ERR) {
						break;
					}
					if (nIndex1 >= i) {
						if (i == 0) {
							i = List_SetCurSel(hwndResList, 0);
						}else {
							i = List_SetCurSel(hwndResList, i - 1);
						}
					}else {
						i = List_SetCurSel(hwndResList, nIndex1);
					}
					break;

				case IDC_BUTTON_INSERT:
					nIndex1 = List_GetCurSel(hwndResList);
					if (nIndex1 == LB_ERR) {
//						break;
						nIndex1 = 0;
					}
					nIndex2 = List_GetCurSel(hwndFuncList);
					if (nIndex2 == LB_ERR) {
						break;
					}
					i = List_GetItemData(hwndFuncList, nIndex2);
					nIndex1 = ::Listbox_INSERTDATA(hwndResList, nIndex1, i);
					if (nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE) {
						break;
					}
					List_SetCurSel(hwndResList, nIndex1 + 1);
					break;

				case IDC_BUTTON_ADD:
					nIndex1 = List_GetCount(hwndResList);
					nIndex2 = List_GetCurSel(hwndFuncList);
					if (nIndex2 == LB_ERR) {
						break;
					}
					i = List_GetItemData(hwndFuncList, nIndex2);
					//	�����ł� i != 0 ���Ƃ͎v�����ǁA�ꉞ�ی��ł��B
					nIndex1 = ::Listbox_INSERTDATA(hwndResList, nIndex1, i);
					if (nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE) {
						TopErrorMessage(NULL, LS(STR_PROPCOMTOOL_ERR05), nIndex1);
						break;
					}
					List_SetCurSel(hwndResList, nIndex1);
					break;

				case IDC_BUTTON_UP:
					nIndex1 = List_GetCurSel(hwndResList);
					if (nIndex1 == LB_ERR || 0 >= nIndex1) {
						break;
					}
					i = List_GetItemData(hwndResList, nIndex1);

					j = List_DeleteString(hwndResList, nIndex1);
					if (j == LB_ERR) {
						break;
					}
					nIndex1 = ::Listbox_INSERTDATA(hwndResList, nIndex1 - 1, i);
					if (nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE) {
						TopErrorMessage(NULL, LS(STR_PROPCOMTOOL_ERR05), nIndex1);
						break;
					}
					List_SetCurSel(hwndResList, nIndex1);
					break;

				case IDC_BUTTON_DOWN:
					i = List_GetCount(hwndResList);
					nIndex1 = List_GetCurSel(hwndResList);
					if (nIndex1 == LB_ERR || nIndex1 + 1 >= i) {
						break;
					}
					i = List_GetItemData(hwndResList, nIndex1);

					j = List_DeleteString(hwndResList, nIndex1);
					if (j == LB_ERR) {
						break;
					}
					nIndex1 = ::Listbox_INSERTDATA(hwndResList, nIndex1 + 1, i);
					if (nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE) {
						TopErrorMessage(NULL, LS(STR_PROPCOMTOOL_ERR05), nIndex1);
						break;
					}
					List_SetCurSel(hwndResList, nIndex1);
					break;
				}
				break;
			}
		}
		break;

	case WM_TIMER:
		nIndex1 = List_GetCurSel(hwndResList);
		nIndex2 = List_GetCurSel(hwndFuncList);
		i = List_GetCount(hwndResList);
		if (nIndex1 == LB_ERR) {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DELETE), FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_UP), FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DOWN), FALSE);
		}else {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DELETE), TRUE);
			if (nIndex1 <= 0) {
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_UP), FALSE);
			}else {
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_UP), TRUE);
			}
			if (nIndex1 + 1 >= i) {
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DOWN), FALSE);
			}else {
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DOWN), TRUE);
			}
		}
		if (nIndex1 == LB_ERR || nIndex2 == LB_ERR) {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_INSERT), FALSE);
		}else {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_INSERT), TRUE);
		}
		if (nIndex2 == LB_ERR) {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_ADD), FALSE);
		}else {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_ADD), TRUE);
		}
		break;

	case WM_DESTROY:
		::KillTimer(hwndDlg, 1);
		break;

	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids);
		}
		return TRUE;

	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids);
		return TRUE;

	}
	return FALSE;
}

// �_�C�A���O�f�[�^�̐ݒ� Toolbar
void PropToolbar::SetData(HWND hwndDlg)
{
	// �@�\��ʈꗗ�ɕ�������Z�b�g(�R���{�{�b�N�X)
	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_FUNCKIND);
	lookup.SetCategory2Combo(hwndCombo);
	
	// ��ʂ̐擪�̍��ڂ�I��(�R���{�{�b�N�X)
	Combo_SetCurSel(hwndCombo, 0);
	::PostMessage(hwndCombo, WM_COMMAND, MAKELONG(IDC_COMBO_FUNCKIND, CBN_SELCHANGE), (LPARAM)hwndCombo);

  // �R���g���[���̃n���h�����擾
	HWND hwndResList = ::GetDlgItem(hwndDlg, IDC_LIST_RES);
	int nFontHeight = TextWidthCalc(hwndResList).GetTextHeight();
	int nListItemHeight = std::max(pIcons->GetCy() + 2, nFontHeight);
	auto& csToolBar = common.toolBar;
	// �c�[���o�[�{�^���̏����Z�b�g(���X�g�{�b�N�X)
	for (int i=0; i<csToolBar.nToolBarButtonNum; ++i) {
		LRESULT lResult = ::Listbox_ADDDATA(hwndResList, (LPARAM)csToolBar.nToolBarButtonIdxArr[i]);
		if (lResult == LB_ERR || lResult == LB_ERRSPACE) {
			break;
		}
		lResult = List_SetItemHeight(hwndResList, lResult, nListItemHeight);
	}
	// �c�[���o�[�̐擪�̍��ڂ�I��(���X�g�{�b�N�X)
	List_SetCurSel(hwndResList, 0);	// �������R�����g�A�E�g����Ɛ擪���ڂ��I������Ȃ��Ȃ�
	// �t���b�g�c�[���o�[�ɂ���^���Ȃ� 
	::CheckDlgButton(hwndDlg, IDC_CHECK_TOOLBARISFLAT, csToolBar.bToolBarIsFlat);
}

// �_�C�A���O�f�[�^�̎擾 Toolbar
int PropToolbar::GetData(HWND hwndDlg)
{
	HWND hwndResList = ::GetDlgItem(hwndDlg, IDC_LIST_RES);
	auto& csToolBar = common.toolBar;

	// �c�[���o�[�{�^���̐�
	csToolBar.nToolBarButtonNum = List_GetCount(hwndResList);

	// �c�[���o�[�{�^���̏����擾
	int k = 0;
	for (int i=0; i<csToolBar.nToolBarButtonNum; ++i) {
		int j = List_GetItemData(hwndResList, i);
		if (j != LB_ERR) {
			csToolBar.nToolBarButtonIdxArr[k] = j;
			++k;
		}
	}
	csToolBar.nToolBarButtonNum = k;

	// �t���b�g�c�[���o�[�ɂ���^���Ȃ�
	csToolBar.bToolBarIsFlat = DlgButton_IsChecked(hwndDlg, IDC_CHECK_TOOLBARISFLAT);
	return TRUE;
}

/* �c�[���o�[�{�^�����X�g�̃A�C�e���`�� */
void PropToolbar::DrawToolBarItemList(DRAWITEMSTRUCT* pDis)
{
	TBBUTTON	tbb;

	HBRUSH hBrush = ::GetSysColorBrush(COLOR_WINDOW);
	::FillRect(pDis->hDC, &pDis->rcItem, hBrush);

	RECT rc  = pDis->rcItem;
	RECT rc0 = pDis->rcItem;
	rc0.left += 2;
	RECT rc1 = rc0;
	RECT rc2 = rc0;

	if ((int)pDis->itemID < 0) {
	}else {

		tbb = pMenuDrawer->getButton(pDis->itemData);

		// �{�^���ƃZ�p���[�^�Ƃŏ����𕪂���
		wchar_t	szLabel[256];
		if (tbb.fsStyle & TBSTYLE_SEP) {
			// �e�L�X�g�����\������
			if (tbb.idCommand == F_SEPARATOR) {
				auto_strncpy( szLabel, LSW(STR_PROPCOMTOOL_ITEM1), _countof(szLabel) - 1 );
				szLabel[_countof(szLabel) - 1] = L'\0';
			}else if (tbb.idCommand == F_MENU_NOT_USED_FIRST) {
				// �c�[���o�[�ܕ�
				auto_strncpy( szLabel, LSW(STR_PROPCOMTOOL_ITEM2), _countof(szLabel) - 1 );
				szLabel[_countof(szLabel) - 1] = L'\0';
			}else {
				auto_strncpy( szLabel, LSW(STR_PROPCOMTOOL_ITEM3), _countof(szLabel) - 1 );
				szLabel[_countof(szLabel) - 1] = L'\0';
			}
		}else {
			// �A�C�R���ƃe�L�X�g��\������
			pIcons->Draw(tbb.iBitmap, pDis->hDC, rc.left + 2, rc.top + 2, ILD_NORMAL);
			lookup.Funccode2Name(tbb.idCommand, szLabel, _countof(szLabel));
		}

		// �A�C�e�����I������Ă���
		if (pDis->itemState & ODS_SELECTED) {
			hBrush = ::GetSysColorBrush(COLOR_HIGHLIGHT);
			::SetTextColor(pDis->hDC, ::GetSysColor(COLOR_HIGHLIGHTTEXT));
		}else {
			hBrush = ::GetSysColorBrush(COLOR_WINDOW);
			::SetTextColor(pDis->hDC, ::GetSysColor(COLOR_WINDOWTEXT));
		}
		rc1.left += pIcons->GetCx() + 2;
		rc1.top++;
		rc1.right--;
		rc1.bottom--;
		::FillRect(pDis->hDC, &rc1, hBrush);

		::SetBkMode(pDis->hDC, TRANSPARENT);
		TextOutW_AnyBuild( pDis->hDC, rc1.left + 4, rc1.top + nToolBarListBoxTopMargin, szLabel, wcslen( szLabel ) );
	}

	// �A�C�e���Ƀt�H�[�J�X������
	if (pDis->itemState & ODS_FOCUS) {
		::DrawFocusRect(pDis->hDC, &rc2);
	}
	return;
}

