// �����F�^�w�i�F����_�C�A���O

#include "StdAfx.h"
#include "DlgSameColor.h"
#include "types/Type.h"
#include "view/colors/EColorIndexType.h"
#include "uiparts/Graphics.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids[] = {
	IDOK,						HIDOK_SAMECOLOR,						// OK
	IDCANCEL,					HIDCANCEL_SAMECOLOR,					// �L�����Z��
	IDC_BUTTON_HELP,			HIDC_BUTTON_SAMECOLOR_HELP,				// �w���v
	IDC_LIST_COLORS,			HIDC_LIST_SAMECOLOR_COLORS,				// �ύX�Ώۂ̐F
	IDC_BUTTON_SELALL,			HIDC_BUTTON_SAMECOLOR_SELALL,			// �S�`�F�b�N
	IDC_BUTTON_SELNOTING,		HIDC_BUTTON_SAMECOLOR_SELNOTING,		// �S����
	IDC_LIST_ITEMINFO,			HIDC_LIST_SAMECOLOR_ITEMINFO,			// �I�𒆂̐F�ɑΉ����鍀�ڂ̃��X�g
	IDC_STATIC_COLOR,			HIDC_STATIC_COLOR,						// ����F
	0, 0
};

LPVOID DlgSameColor::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

DlgSameColor::DlgSameColor() :
	wpColorStaticProc(nullptr),
	wpColorListProc(nullptr),
	wID(0),
	pTypes(nullptr),
	cr(0)
{
	return;
}

DlgSameColor::~DlgSameColor()
{
	return;
}

/*!
	�W���ȊO�̃��b�Z�[�W��ߑ�����
*/
INT_PTR DlgSameColor::DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR result;
	result = Dialog::DispatchEvent(hWnd, wMsg, wParam, lParam);
	switch (wMsg) {
	case WM_COMMAND:
		// �F�I�����X�g�{�b�N�X�̑I�����ύX���ꂽ�ꍇ�̏���
		if (LOWORD(wParam) == IDC_LIST_COLORS && HIWORD(wParam) == LBN_SELCHANGE) {
			OnSelChangeListColors((HWND)lParam);
		}
		break;

	case WM_CTLCOLORLISTBOX:
		{
			// ���ڃ��X�g�̔w�i�F��ݒ肷�鏈��
			HWND hwndLB = (HWND)lParam;
			if (::GetDlgCtrlID(hwndLB) == IDC_LIST_ITEMINFO) {
				HDC hdcLB = (HDC)wParam;
				::SetTextColor(hdcLB, ::GetSysColor(COLOR_WINDOWTEXT));
				::SetBkMode(hdcLB, TRANSPARENT);
				return (INT_PTR)::GetSysColorBrush(COLOR_BTNFACE);
			}
		}
		break;

	default:
		break;
	}
	return result;
}

/*! ���[�_���_�C�A���O�̕\��
	@param wID [in] �^�C�v�ʐݒ�_�C�A���O�ŉ����ꂽ�{�^��ID
	@param pTypes  [in/out] �^�C�v�ʐݒ�f�[�^
	@param cr [in] �w��F
*/
INT_PTR DlgSameColor::DoModal(HINSTANCE hInstance, HWND hwndParent, WORD wID, TypeConfig* pTypes, COLORREF cr)
{
	wID = wID;
	this->pTypes = pTypes;
	cr = cr;

	(void)Dialog::DoModal(hInstance, hwndParent, IDD_SAMECOLOR, (LPARAM)NULL);

	return TRUE;
}

/*! WM_INITDIALOG ���� */
BOOL DlgSameColor::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	BOOL bRet = Dialog::OnInitDialog(hwndDlg, wParam, lParam);

	HWND hwndStatic = GetItemHwnd(IDC_STATIC_COLOR);
	HWND hwndList = GetItemHwnd(IDC_LIST_COLORS);

	// �w��F�X�^�e�B�b�N�A�F�I�����X�g���T�u�N���X��
	::SetWindowLongPtr(hwndStatic, GWLP_USERDATA, (LONG_PTR)this);
	wpColorStaticProc = (WNDPROC)::SetWindowLongPtr(hwndStatic, GWLP_WNDPROC, (LONG_PTR)ColorStatic_SubclassProc);
	::SetWindowLongPtr(hwndList, GWLP_USERDATA, (LONG_PTR)this);
	wpColorListProc = (WNDPROC)::SetWindowLongPtr(hwndList, GWLP_WNDPROC, (LONG_PTR)ColorList_SubclassProc);


	wchar_t szText[30];
	int nItem;

	switch (wID) {	// �^�C�v�ʐݒ�_�C�A���O�ŉ����ꂽ�{�^��ID
	case IDC_BUTTON_SAMETEXTCOLOR:
		// �^�C�v�ʐݒ肩�當���F���d�����Ȃ��悤�Ɏ��o��
		::SetWindowText(GetHwnd(), LS(STR_DLGSMCLR_BTN1));
		for (int i=0; i<COLORIDX_LAST; ++i) {
			if ((g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_TEXT) != 0) {
				continue;
			}
			if (cr != pTypes->colorInfoArr[i].colorAttr.cTEXT) {
				_ultow(pTypes->colorInfoArr[i].colorAttr.cTEXT, szText, 10);
				if (List_FindStringExact(hwndList, -1, szText) == LB_ERR) {
					nItem = ::List_AddString(hwndList, szText);
					List_SetItemData(hwndList, nItem, FALSE); 
				}
			}
		}
		break;

	case IDC_BUTTON_SAMEBKCOLOR:
		// �^�C�v�ʐݒ肩��w�i�F���d�����Ȃ��悤�Ɏ��o��
		::SetWindowText(GetHwnd(), LS(STR_DLGSMCLR_BTN2));
		for (int i=0; i<COLORIDX_LAST; ++i) {
			if ((g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_BACK) != 0) {
				continue;
			}
			if (cr != pTypes->colorInfoArr[i].colorAttr.cBACK) {
				_ultow(pTypes->colorInfoArr[i].colorAttr.cBACK, szText, 10);
				if (List_FindStringExact(hwndList, -1, szText) == LB_ERR) {
					nItem = ::List_AddString(hwndList, szText);
					List_SetItemData(hwndList, nItem, FALSE); 
				}
			}
		}
		break;

	default:
		CloseDialog(IDCANCEL);
		break;
	}

	if (0 < List_GetCount(hwndList)) {
		List_SetCurSel(hwndList, 0);
		OnSelChangeListColors(hwndList);
	}

	return bRet;
}

/*! BN_CLICKED ���� */
BOOL DlgSameColor::OnBnClicked(int wID)
{
	HWND hwndList = GetItemHwnd(IDC_LIST_COLORS);
	int nItemNum = List_GetCount(hwndList);
	BOOL bCheck;

	switch (wID) {
	case IDC_BUTTON_HELP:
		// �w���v
		MyWinHelp(GetHwnd(), HELP_CONTEXT, HLP000316);
		return TRUE;

	case IDC_BUTTON_SELALL:
	case IDC_BUTTON_SELNOTING:
		// �S�I���^�S�����̏���
		bCheck = (wID == IDC_BUTTON_SELALL);
		for (int i=0; i<nItemNum; ++i) {
			List_SetItemData(hwndList, i, bCheck);
		}
		::InvalidateRect(hwndList, NULL, TRUE);
		break;

	case IDOK:
		// �^�C�v�ʐݒ肩��I��F�Ɠ��F�̂��̂����o���Ďw��F�Ɉꊇ�ύX����
		wchar_t szText[30];
		LPWSTR pszStop;
		COLORREF cr;

		for (int i=0; i<nItemNum; ++i) {
			bCheck = (BOOL)List_GetItemData(hwndList, i);
			if (bCheck) {
				List_GetText(hwndList, i, szText);
				cr = wcstoul(szText, &pszStop, 10);

				switch (wID) {
				case IDC_BUTTON_SAMETEXTCOLOR:
					for (int j=0; j<COLORIDX_LAST; ++j) {
						auto& colorAttr = pTypes->colorInfoArr[j].colorAttr.cTEXT;
						if (cr == colorAttr) {
							colorAttr = cr;
						}
					}
					break;

				case IDC_BUTTON_SAMEBKCOLOR:
					for (int j=0; j<COLORIDX_LAST; ++j) {
						auto& colorAttr = pTypes->colorInfoArr[j].colorAttr.cBACK;
						if (cr == colorAttr) {
							colorAttr = cr;
						}
					}
					break;

				default:
					break;
				}
			}
		}
		break;

	case IDCANCEL:
		break;
	}
	return Dialog::OnBnClicked(wID);
}

/*! WM_DRAWITEM ���� */
BOOL DlgSameColor::OnDrawItem(WPARAM wParam, LPARAM lParam)
{
	LPDRAWITEMSTRUCT pDis = (LPDRAWITEMSTRUCT)lParam;	// ���ڕ`����
	if (IDC_LIST_COLORS != pDis->CtlID) {	// �I�[�i�[�`��ɂ��Ă���̂͐F�I�����X�g����
		return TRUE;
	}

	// �`��Ώ�
	Graphics gr(pDis->hDC);

	//
	// �F�I�����X�g�̕`�揈��
	//
	RECT		rc;
	wchar_t		szText[30];
	LPWSTR		pszStop;
	COLORREF	cr;

	List_GetText(pDis->hwndItem, pDis->itemID, szText);
	cr = wcstoul(szText, &pszStop, 10);

	rc = pDis->rcItem;

	// �A�C�e����`�h��Ԃ�
	::FillRect(gr, &pDis->rcItem, ::GetSysColorBrush(COLOR_WINDOW));

	// �A�C�e�����I�����
	if (pDis->itemState & ODS_SELECTED) {
		rc = pDis->rcItem;
		rc.left += (rc.bottom - rc.top);
		::FillRect(gr, &rc, ::GetSysColorBrush(COLOR_HIGHLIGHT));
	}

	// �A�C�e���Ƀt�H�[�J�X������
	if (pDis->itemState & ODS_FOCUS) {
		::DrawFocusRect(gr, &pDis->rcItem);
	}

	// �`�F�b�N�{�b�N�X�\��
	rc = pDis->rcItem;
	rc.top += 2;
	rc.bottom -= 2;
	rc.left += 2;
	rc.right = rc.left + (rc.bottom - rc.top);
	UINT uState =  DFCS_BUTTONCHECK | DFCS_FLAT;
	if ((BOOL)pDis->itemData) {
		uState |= DFCS_CHECKED;		// �`�F�b�N���
	}
	::DrawFrameControl(gr, &rc, DFC_BUTTON, uState);

	// �F���{��`
	rc = pDis->rcItem;
	rc.left += rc.bottom - rc.top + 2;
	rc.top += 2;
	rc.bottom -= 2;
	rc.right -= 2;
	gr.SetBrushColor(cr);
	gr.SetPen(::GetSysColor(COLOR_3DSHADOW));
	::RoundRect(gr, rc.left, rc.top, rc.right, rc.bottom , 5, 5);

	return TRUE;
}

/*! �F�I�����X�g�� LBN_SELCHANGE ���� */
BOOL DlgSameColor::OnSelChangeListColors(HWND hwndCtl)
{
	// �F�I�����X�g�Ō��݃t�H�[�J�X�̂���F�ɂ���
	// �^�C�v�ʐݒ肩�瓯�F�̍��ڂ����o���č��ڃ��X�g�ɕ\������
	HWND hwndListInfo = GetItemHwnd(IDC_LIST_ITEMINFO);
	List_ResetContent(hwndListInfo);

	int i = List_GetCaretIndex(hwndCtl);
	if (i != LB_ERR) {
		wchar_t szText[30];
		List_GetText(hwndCtl, i, szText);
		LPWSTR pszStop;
		COLORREF cr = wcstoul(szText, &pszStop, 10);

		switch (wID) {
		case IDC_BUTTON_SAMETEXTCOLOR:
			for (int j=0; j<COLORIDX_LAST; ++j) {
				if ((g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_TEXT) != 0) {
					continue;
				}
				if (cr == pTypes->colorInfoArr[j].colorAttr.cTEXT) {
					::List_AddString(hwndListInfo, pTypes->colorInfoArr[j].szName);
				}
			}
			break;

		case IDC_BUTTON_SAMEBKCOLOR:
			for (int j=0; j<COLORIDX_LAST; ++j) {
				if ((g_ColorAttributeArr[j].fAttribute & COLOR_ATTRIB_NO_BACK) != 0) {	// 2006.12.18 ryoji �t���O���p�Ŋȑf��
					continue;
				}
				if (cr == pTypes->colorInfoArr[j].colorAttr.cBACK) {
					::List_AddString(hwndListInfo, pTypes->colorInfoArr[j].szName);
				}
			}
			break;

		default:
			break;
		}
	}

	return TRUE;
}

/*! �T�u�N���X�����ꂽ�w��F�X�^�e�B�b�N�̃E�B���h�E�v���V�[�W�� */
LRESULT CALLBACK DlgSameColor::ColorStatic_SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC		hDC;
	RECT	rc;

	DlgSameColor* pDlgSameColor = (DlgSameColor*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (uMsg) {
	case WM_PAINT:
		// �E�B���h�E�`��
		PAINTSTRUCT ps;

		hDC = ::BeginPaint(hwnd, &ps);

		// �F���{��`
		::GetClientRect(hwnd, &rc);
		rc.left += 2;
		rc.top += 2;
		rc.right -=2;
		rc.bottom -= 2;
		{
			Graphics gr(hDC);
			gr.SetBrushColor(pDlgSameColor->cr);
			gr.SetPen(::GetSysColor(COLOR_3DSHADOW));
			::RoundRect(gr, rc.left, rc.top, rc.right, rc.bottom, 5, 5);
		}
		::EndPaint(hwnd, &ps);
		return (LRESULT)0;

	case WM_ERASEBKGND:
		// �w�i�`��
		hDC = (HDC)wParam;
		::GetClientRect(hwnd, &rc);

		// �e��WM_CTLCOLORSTATIC�𑗂��Ĕw�i�u���V���擾���A�w�i�`�悷��
		{
			HBRUSH	hBrush = (HBRUSH)::SendMessage(GetParent(hwnd), WM_CTLCOLORSTATIC, wParam, (LPARAM)hwnd);
			HBRUSH	hBrushOld = (HBRUSH)::SelectObject(hDC, hBrush);
			::FillRect(hDC, &rc, hBrush);
			::SelectObject(hDC, hBrushOld);
		}
		return (LRESULT)1;

	case WM_DESTROY:
		// �T�u�N���X������
		::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)pDlgSameColor->wpColorStaticProc);
		pDlgSameColor->wpColorStaticProc = nullptr;
		return (LRESULT)0;

	default:
		break;
	}

	return CallWindowProc(pDlgSameColor->wpColorStaticProc, hwnd, uMsg, wParam, lParam);
}

/*! �T�u�N���X�����ꂽ�F�I�����X�g�̃E�B���h�E�v���V�[�W�� */
LRESULT CALLBACK DlgSameColor::ColorList_SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT po;
	RECT rcItem;
	RECT rc;
	int nItemNum;

	DlgSameColor* pDlgSameColor = (DlgSameColor*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (uMsg) {
	case WM_LBUTTONUP:
		// �}�E�X�{�^�����ɂ��鍀�ڂ̑I���^�I���������g�O������
		po.x = LOWORD(lParam);	// horizontal position of cursor
		po.y = HIWORD(lParam);	// vertical position of cursor
		nItemNum = List_GetCount(hwnd);
		for (int i=0; i<nItemNum; ++i) {
			List_GetItemRect(hwnd, i, &rcItem);
			rc = rcItem;
			rc.top += 2;
			rc.bottom -= 2;
			rc.left += 2;
			rc.right = rc.left + (rc.bottom - rc.top);
			if (::PtInRect(&rc, po)) {
				BOOL bCheck = !(BOOL)List_GetItemData(hwnd, i);
				List_SetItemData(hwnd, i, bCheck);
				::InvalidateRect(hwnd, &rcItem, TRUE);
				break;
			}
		}
		break;

	case WM_KEYUP:
		// �t�H�[�J�X���ڂ̑I���^�I���������g�O������
		if (wParam == VK_SPACE) {
			int i = List_GetCaretIndex(hwnd);
			if (i != LB_ERR) {
				BOOL bCheck = !(BOOL)List_GetItemData(hwnd, i);
				List_SetItemData(hwnd, i, bCheck);
				::InvalidateRect(hwnd, NULL, TRUE);
			}
		}
		break;

	case WM_DESTROY:
		// �T�u�N���X������
		::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)pDlgSameColor->wpColorListProc);
		pDlgSameColor->wpColorListProc = NULL;
		return (LRESULT)0;

	default:
		break;
	}

	return ::CallWindowProc(pDlgSameColor->wpColorListProc, hwnd, uMsg, wParam, lParam);
}

