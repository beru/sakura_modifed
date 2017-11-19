// �^�C�v�ʐݒ� - �J���[

#include "StdAfx.h"
#include "PropTypes.h"
#include "env/ShareData.h"
#include "typeprop/ImpExpManager.h"
#include "DlgSameColor.h"
#include "DlgKeywordSelect.h"
#include "view/colors/EColorIndexType.h"
#include "uiparts/Graphics.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"
#include "prop/PropCommon.h"

using namespace std;

// �J�X�^���J���[�p�̎��ʕ�����
static const TCHAR* TSTR_PTRCUSTOMCOLORS = _T("ptrCustomColors");

WNDPROC	wpColorListProc;

static const DWORD p_helpids2[] = {	//11400
	IDC_LIST_COLORS,				HIDC_LIST_COLORS,				// �F�w��
	IDC_CHECK_DISP,					HIDC_CHECK_DISP,				// �F�����\��
	IDC_CHECK_BOLD,					HIDC_CHECK_BOLD,				// ����
	IDC_CHECK_UNDERLINE,			HIDC_CHECK_UNDERLINE,			// ����
	IDC_BUTTON_TEXTCOLOR,			HIDC_BUTTON_TEXTCOLOR,			// �����F
	IDC_BUTTON_BACKCOLOR,			HIDC_BUTTON_BACKCOLOR,			// �w�i�F
	IDC_BUTTON_SAMETEXTCOLOR,		HIDC_BUTTON_SAMETEXTCOLOR,		// �����F����
	IDC_BUTTON_SAMEBKCOLOR,			HIDC_BUTTON_SAMEBKCOLOR,		// �w�i�F����
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT_COLOR,		// �C���|�[�g
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT_COLOR,		// �G�N�X�|�[�g
	IDC_COMBO_SET,					HIDC_COMBO_SET_COLOR,			// �����L�[���[�h�P�Z�b�g��
	IDC_BUTTON_KEYWORD_SELECT,		HIDC_BUTTON_KEYWORD_SELECT,		// �����L�[���[�h2�`10
	IDC_EDIT_BLOCKCOMMENT_FROM,		HIDC_EDIT_BLOCKCOMMENT_FROM,	// �u���b�N�R�����g�P�J�n
	IDC_EDIT_BLOCKCOMMENT_TO,		HIDC_EDIT_BLOCKCOMMENT_TO,		// �u���b�N�R�����g�P�I��
	IDC_EDIT_BLOCKCOMMENT_FROM2,	HIDC_EDIT_BLOCKCOMMENT_FROM2,	// �u���b�N�R�����g�Q�J�n
	IDC_EDIT_BLOCKCOMMENT_TO2,		HIDC_EDIT_BLOCKCOMMENT_TO2,		// �u���b�N�R�����g�Q�I��
	IDC_EDIT_LINECOMMENT,			HIDC_EDIT_LINECOMMENT,			// �s�R�����g�P
	IDC_EDIT_LINECOMMENT2,			HIDC_EDIT_LINECOMMENT2,			// �s�R�����g�Q
	IDC_EDIT_LINECOMMENT3,			HIDC_EDIT_LINECOMMENT3,			// �s�R�����g�R
	IDC_EDIT_LINECOMMENTPOS,		HIDC_EDIT_LINECOMMENTPOS,		// �����P
	IDC_EDIT_LINECOMMENTPOS2,		HIDC_EDIT_LINECOMMENTPOS2,		// �����Q
	IDC_EDIT_LINECOMMENTPOS3,		HIDC_EDIT_LINECOMMENTPOS3,		// �����R
	IDC_CHECK_LCPOS,				HIDC_CHECK_LCPOS,				// ���w��P
	IDC_CHECK_LCPOS2,				HIDC_CHECK_LCPOS2,				// ���w��Q
	IDC_CHECK_LCPOS3,				HIDC_CHECK_LCPOS3,				// ���w��R
	IDC_COMBO_STRINGLITERAL,		HIDC_COMBO_STRINGLITERAL,		// ������G�X�P�[�v
	IDC_CHECK_STRINGLINEONLY,		HIDC_CHECK_STRINGLINEONLY,		// ������͍s���̂�
	IDC_CHECK_STRINGENDLINE,		HIDC_CHECK_STRINGENDLINE,		// �I���������Ȃ��ꍇ�s���܂ŐF����
	IDC_EDIT_VERTLINE,				HIDC_EDIT_VERTLINE,				// �c���̌��w��
//	IDC_STATIC,						-1,
	0, 0
};

TYPE_NAME_ID<StringLiteralType> StringLitteralArr[] = {
	{ StringLiteralType::CPP,    STR_STRINGESC_CPP },
	{ StringLiteralType::PLSQL,  STR_STRINGESC_PLSQL },
	{ StringLiteralType::HTML,   STR_STRINGESC_HTML },
	{ StringLiteralType::CSharp, STR_STRINGESC_CSHARP },
	{ StringLiteralType::Python, STR_STRINGESC_PYTHON },
};


// �s�R�����g�Ɋւ�����
struct {
	int nEditID;
	int nCheckBoxID;
	int nTextID;
} const cLineComment[COMMENT_DELIMITER_NUM] = {
	{ IDC_EDIT_LINECOMMENT	, IDC_CHECK_LCPOS , IDC_EDIT_LINECOMMENTPOS },
	{ IDC_EDIT_LINECOMMENT2	, IDC_CHECK_LCPOS2, IDC_EDIT_LINECOMMENTPOS2},
	{ IDC_EDIT_LINECOMMENT3	, IDC_CHECK_LCPOS3, IDC_EDIT_LINECOMMENTPOS3}
};

// �F�̐ݒ���C���|�[�g
bool PropTypesColor::Import(HWND hwndDlg)
{
	ColorInfo colorInfoArr[64];
	ImpExpColors cImpExpColors(colorInfoArr);

	// �F�ݒ� I/O
	for (size_t i=0; i<types.nColorInfoArrNum; ++i) {
		colorInfoArr[i] = types.colorInfoArr[i];
		_tcscpy(colorInfoArr[i].szName, types.colorInfoArr[i].szName);
	}

	// �C���|�[�g
	if (!cImpExpColors.ImportUI(hInstance, hwndDlg)) {
		// �C���|�[�g�����Ă��Ȃ�
		return false;
	}

	// �f�[�^�̃R�s�[
	types.nColorInfoArrNum = COLORIDX_LAST;
	for (size_t i=0; i<types.nColorInfoArrNum; ++i) {
		types.colorInfoArr[i] = colorInfoArr[i];
		_tcscpy(types.colorInfoArr[i].szName, colorInfoArr[i].szName);
	}
	// �_�C�A���O�f�[�^�̐ݒ� color
	SetData(hwndDlg);

	return true;
}


// �F�̐ݒ���G�N�X�|�[�g
bool PropTypesColor::Export(HWND hwndDlg)
{
	ImpExpColors	cImpExpColors(types.colorInfoArr);

	// �G�N�X�|�[�g
	return cImpExpColors.ExportUI(hInstance, hwndDlg);
}



LRESULT APIENTRY ColorList_SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int			xPos = 0;
	int			yPos;
	int			nIndex = -1;
	int			nItemNum;
	RECT		rcItem = {0, 0, 0, 0};
	int			i;
	POINT		poMouse;
	ColorInfo*	pColorInfo;

	switch (uMsg) {
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONUP:
		xPos = LOWORD(lParam);	// horizontal position of cursor
		yPos = HIWORD(lParam);	// vertical position of cursor

		poMouse.x = xPos;
		poMouse.y = yPos;
		nItemNum = List_GetCount(hwnd);
		for (i=0; i<nItemNum; ++i) {
			List_GetItemRect(hwnd, i, &rcItem);
			if (::PtInRect(&rcItem, poMouse)) {
//				MYTRACE(_T("hit at i == %d\n"), i);
//				MYTRACE(_T("\n"));
				nIndex = i;
				break;
			}
		}
		break;
	}
	switch (uMsg) {
	case WM_RBUTTONDOWN:

		if (nIndex == -1) {
			break;
		}
		if (18 <= xPos && xPos <= rcItem.right - 29) {
			List_SetCurSel(hwnd, nIndex);
			::SendMessage(::GetParent(hwnd), WM_COMMAND, MAKELONG(IDC_LIST_COLORS, LBN_SELCHANGE), (LPARAM)hwnd);
			pColorInfo = (ColorInfo*)List_GetItemData(hwnd, nIndex);
			// ����
			if ((g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_NO_UNDERLINE) == 0) {
				pColorInfo->fontAttr.bUnderLine = !pColorInfo->fontAttr.bUnderLine; // toggle true/false
				::CheckDlgButtonBool(::GetParent(hwnd), IDC_CHECK_UNDERLINE, pColorInfo->fontAttr.bUnderLine);
				::InvalidateRect(hwnd, &rcItem, TRUE);
			}
		}
		break;

	case WM_LBUTTONDBLCLK:
		if (nIndex == -1) {
			break;
		}
		if (18 <= xPos && xPos <= rcItem.right - 29) {
			pColorInfo = (ColorInfo*)List_GetItemData(hwnd, nIndex);
			// �����ŕ\��
			if ((g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_NO_BOLD) == 0) {
				pColorInfo->fontAttr.bBoldFont = !pColorInfo->fontAttr.bBoldFont; // toggle true/false
				::CheckDlgButtonBool(::GetParent(hwnd), IDC_CHECK_BOLD, pColorInfo->fontAttr.bBoldFont);
				::InvalidateRect(hwnd, &rcItem, TRUE);
			}
		}
		break;
	case WM_LBUTTONUP:
		if (nIndex == -1) {
			break;
		}
		pColorInfo = (ColorInfo*)List_GetItemData(hwnd, nIndex);
		// �F����/�\�� ����
		if (2 <= xPos && xPos <= 16
			&& ((g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_FORCE_DISP) == 0)
		) {
			if (pColorInfo->bDisp) {	// �F����/�\������
				pColorInfo->bDisp = false;
			}else {
				pColorInfo->bDisp = true;
			}
			if (nIndex == COLORIDX_GYOU) {
				pColorInfo = (ColorInfo*)List_GetItemData(hwnd, nIndex);
			}

			::InvalidateRect(hwnd, &rcItem, TRUE);
		}else
		// �O�i�F���{ ��`
		if (rcItem.right - 27 <= xPos && xPos <= rcItem.right - 27 + 12
			&& ((g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_NO_TEXT) == 0)
		) {
			// �F�I���_�C�A���O
			// 2005.11.30 Moca �J�X�^���F�ێ�
			DWORD* pColors = (DWORD*)::GetProp(hwnd, _T("ptrCustomColors"));
			if (PropTypesColor::SelectColor(hwnd, &pColorInfo->colorAttr.cTEXT, pColors)) {
				::InvalidateRect(hwnd, &rcItem, TRUE);
				::InvalidateRect(::GetDlgItem(::GetParent(hwnd), IDC_BUTTON_TEXTCOLOR), NULL, TRUE);
			}
		}else
		// �O�i�F���{ ��`
		if (rcItem.right - 13 <= xPos && xPos <= rcItem.right - 13 + 12
			&& ((g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_NO_BACK) == 0)
		) {
			// �F�I���_�C�A���O
			DWORD* pColors = (DWORD*)::GetProp(hwnd, _T("ptrCustomColors"));
			if (PropTypesColor::SelectColor(hwnd, &pColorInfo->colorAttr.cBACK, pColors)) {
				::InvalidateRect(hwnd, &rcItem, TRUE);
				::InvalidateRect(::GetDlgItem(::GetParent(hwnd), IDC_BUTTON_BACKCOLOR), NULL, TRUE);
			}
		}
		break;
	case WM_DESTROY:
		if (::GetProp(hwnd, _T("ptrCustomColors"))) {
			::RemoveProp(hwnd, _T("ptrCustomColors"));
		}
		break;
	}
	return CallWindowProc(wpColorListProc, hwnd, uMsg, wParam, lParam);
}


// color ���b�Z�[�W����
INT_PTR PropTypesColor::DispatchEvent(
	HWND				hwndDlg,	// handle to dialog box
	UINT				uMsg,		// message
	WPARAM				wParam,		// first message parameter
	LPARAM				lParam 		// second message parameter
	)
{
	WORD				wNotifyCode;
	WORD				wID;
	HWND				hwndCtl;
	NMHDR*				pNMHDR;
	NM_UPDOWN*			pMNUD;
	int					idCtrl;
	int					nVal;
	int					nIndex;
	static HWND			hwndListColor;
	LPDRAWITEMSTRUCT	pDis;

	switch (uMsg) {
	case WM_INITDIALOG:
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		hwndListColor = ::GetDlgItem(hwndDlg, IDC_LIST_COLORS);

		// �_�C�A���O�f�[�^�̐ݒ� color
		SetData(hwndDlg);

		// �F���X�g���t�b�N
		wpColorListProc = (WNDPROC) ::SetWindowLongPtr(hwndListColor, GWLP_WNDPROC, (LONG_PTR)ColorList_SubclassProc);
		::SetProp(hwndListColor, _T("ptrCustomColors"), dwCustColors);
		
		return TRUE;

	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	// �ʒm�R�[�h
		wID			= LOWORD(wParam);	// ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID
		hwndCtl		= (HWND) lParam;	// �R���g���[���̃n���h��
		if (hwndListColor == hwndCtl) {
			switch (wNotifyCode) {
			case LBN_SELCHANGE:
				nIndex = List_GetCurSel(hwndListColor);
				nCurrentColorType = nIndex;		// ���ݑI������Ă���F�^�C�v

				{
					// �e��R���g���[���̗L���^������؂�ւ���
					unsigned int fAttribute = g_ColorAttributeArr[nIndex].fAttribute;
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_DISP),			((fAttribute & COLOR_ATTRIB_FORCE_DISP) == 0) ? TRUE: FALSE);
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_BOLD),			((fAttribute & COLOR_ATTRIB_NO_BOLD) == 0) ? TRUE: FALSE);
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_UNDERLINE),		((fAttribute & COLOR_ATTRIB_NO_UNDERLINE) == 0) ? TRUE: FALSE);
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_STATIC_MOZI),			((fAttribute & COLOR_ATTRIB_NO_TEXT) == 0) ? TRUE: FALSE);
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_TEXTCOLOR),		((fAttribute & COLOR_ATTRIB_NO_TEXT) == 0) ? TRUE: FALSE);
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_SAMETEXTCOLOR),	((fAttribute & COLOR_ATTRIB_NO_TEXT) == 0) ? TRUE: FALSE);
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_STATIC_HAIKEI),		((fAttribute & COLOR_ATTRIB_NO_BACK) == 0) ? TRUE: FALSE);
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_BACKCOLOR),		((fAttribute & COLOR_ATTRIB_NO_BACK) == 0) ? TRUE: FALSE);
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_SAMEBKCOLOR),	((fAttribute & COLOR_ATTRIB_NO_BACK) == 0) ? TRUE: FALSE);
				}

				// �F����/�\�� ������
				::CheckDlgButtonBool(hwndDlg, IDC_CHECK_DISP, types.colorInfoArr[nCurrentColorType].bDisp);
				// �����ŕ\��
				::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BOLD, types.colorInfoArr[nCurrentColorType].fontAttr.bBoldFont);
				// ������\��
				::CheckDlgButtonBool(hwndDlg, IDC_CHECK_UNDERLINE, types.colorInfoArr[nCurrentColorType].fontAttr.bUnderLine);

				::InvalidateRect(::GetDlgItem(hwndDlg, IDC_BUTTON_TEXTCOLOR), NULL, TRUE);
				::InvalidateRect(::GetDlgItem(hwndDlg, IDC_BUTTON_BACKCOLOR), NULL, TRUE);
				return TRUE;
			}
		}
		switch (wNotifyCode) {
		// �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ
		case BN_CLICKED:
			switch (wID) {
			case IDC_BUTTON_SAMETEXTCOLOR: // �����F����
				{
					// �����F�^�w�i�F����_�C�A���O���g��
					DlgSameColor dlgSameColor;
					COLORREF cr = types.colorInfoArr[nCurrentColorType].colorAttr.cTEXT;
					dlgSameColor.DoModal(::GetModuleHandle(NULL), hwndDlg, wID, &types, cr);
				}
				::InvalidateRect(hwndListColor, NULL, TRUE);
				return TRUE;

			case IDC_BUTTON_SAMEBKCOLOR:	// �w�i�F����
				{
					// �����F�^�w�i�F����_�C�A���O���g��
					DlgSameColor dlgSameColor;
					COLORREF cr = types.colorInfoArr[nCurrentColorType].colorAttr.cBACK;
					dlgSameColor.DoModal(::GetModuleHandle(NULL), hwndDlg, wID, &types, cr);
				}
				::InvalidateRect(hwndListColor, NULL, TRUE);
				return TRUE;

			case IDC_BUTTON_TEXTCOLOR:	// �e�L�X�g�F
				// �F�I���_�C�A���O
				if (SelectColor(hwndDlg, &types.colorInfoArr[nCurrentColorType].colorAttr.cTEXT, dwCustColors)) {
					::InvalidateRect(::GetDlgItem(hwndDlg, IDC_BUTTON_TEXTCOLOR), NULL, TRUE);
				}
				// ���ݑI������Ă���F�^�C�v
				List_SetCurSel(hwndListColor, nCurrentColorType);
				return TRUE;
			case IDC_BUTTON_BACKCOLOR:	// �w�i�F
				// �F�I���_�C�A���O
				if (SelectColor(hwndDlg, &types.colorInfoArr[nCurrentColorType].colorAttr.cBACK, dwCustColors)) {
					::InvalidateRect(::GetDlgItem(hwndDlg, IDC_BUTTON_BACKCOLOR), NULL, TRUE);
				}
				// ���ݑI������Ă���F�^�C�v
				List_SetCurSel(hwndListColor, nCurrentColorType);
				return TRUE;
			case IDC_CHECK_DISP:	// �F����/�\�� ������
				types.colorInfoArr[nCurrentColorType].bDisp = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_DISP);
				// ���ݑI������Ă���F�^�C�v
				List_SetCurSel(hwndListColor, nCurrentColorType);
				types.nRegexKeyMagicNumber = RegexKeyword::GetNewMagicNumber();
				return TRUE;
			case IDC_CHECK_BOLD:	// ������
				types.colorInfoArr[nCurrentColorType].fontAttr.bBoldFont = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_BOLD);
				// ���ݑI������Ă���F�^�C�v
				List_SetCurSel(hwndListColor, nCurrentColorType);
				return TRUE;
			case IDC_CHECK_UNDERLINE:	// ������\��
				types.colorInfoArr[nCurrentColorType].fontAttr.bUnderLine = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_UNDERLINE);
				// ���ݑI������Ă���F�^�C�v
				List_SetCurSel(hwndListColor, nCurrentColorType);
				return TRUE;

			case IDC_BUTTON_IMPORT:	// �F�̐ݒ���C���|�[�g
				Import(hwndDlg);
				types.nRegexKeyMagicNumber = RegexKeyword::GetNewMagicNumber();
				return TRUE;

			case IDC_BUTTON_EXPORT:	// �F�̐ݒ���G�N�X�|�[�g
				Export(hwndDlg);
				return TRUE;

			// �s�R�����g�J�n���w���ON/OFF
			case IDC_CHECK_LCPOS:
			case IDC_CHECK_LCPOS2:
			case IDC_CHECK_LCPOS3:
				EnableTypesPropInput(hwndDlg);
				return TRUE;

			// �����L�[���[�h�̑I��
			case IDC_BUTTON_KEYWORD_SELECT:
				{
					DlgKeywordSelect dlgKeywordSelect;
					// �����L�[���[�h1���擾����B
					HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_SET);
					int nIdx = Combo_GetCurSel(hwndCombo);
					if (nIdx == CB_ERR || nIdx == 0) {
						nSet[0] = -1;
					}else {
						nSet[0] = nIdx - 1;
					}
					dlgKeywordSelect.DoModal(::GetModuleHandle(NULL), hwndDlg, nSet);
					RearrangeKeywordSet(hwndDlg);	// Jan. 23, 2005 genta �L�[���[�h�Z�b�g�Ĕz�u
					// �����L�[���[�h1�𔽉f����B
					if (nSet[0] == -1) {
						Combo_SetCurSel(hwndCombo, 0);
					}else {
						Combo_SetCurSel(hwndCombo, nSet[0] + 1);
					}
				}
				break;
			// �����L�[���[�h�̑I��
			case IDC_BUTTON_EDITKEYWORD:
				{
					auto pPropKeyword = std::make_unique<PropKeyword>();
					PropCommon* pCommon = (PropCommon*)pPropKeyword.get();
					pCommon->hwndParent = ::GetParent(hwndDlg);
					pCommon->InitData();
					pCommon->nKeywordSet1 = nSet[0];
					INT_PTR res = ::DialogBoxParam(
						SelectLang::getLangRsrcInstance(),
						MAKEINTRESOURCE(IDD_PROP_KEYWORD),
						hwndDlg,
						PropKeyword::DlgProc_dialog,
						(LPARAM)pPropKeyword.get()
					);
					if (res == IDOK) {
						ShareDataLockCounter::WaitLock(pCommon->hwndParent);
						pCommon->ApplyData();
						SetData(hwndDlg);
						bChangeKeywordSet = true;
					}
					return TRUE;
				}
			case IDC_CHECK_STRINGLINEONLY:
				{
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_STRINGENDLINE),
						::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_STRINGLINEONLY));
					return TRUE;
				}
			}
			break;	// BN_CLICKED
		}
		break;	// WM_COMMAND
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch (idCtrl) {
		case IDC_SPIN_LCColNum:
			// �s�R�����g���ʒu
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_LINECOMMENTPOS, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 1) {
				nVal = 1;
			}
			if (nVal > 1000) {
				nVal = 1000;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_LINECOMMENTPOS, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_LCColNum2:
			// �s�R�����g���ʒu
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_LINECOMMENTPOS2, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 1) {
				nVal = 1;
			}
			if (nVal > 1000) {
				nVal = 1000;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_LINECOMMENTPOS2, nVal, FALSE);
			return TRUE;

		case IDC_SPIN_LCColNum3:
			// �s�R�����g���ʒu
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_LINECOMMENTPOS3, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 1) {
				nVal = 1;
			}
			if (nVal > 1000) {
				nVal = 1000;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_LINECOMMENTPOS3, nVal, FALSE);
			return TRUE;
		default:
			switch (pNMHDR->code) {
			case PSN_HELP:
				OnHelp(hwndDlg, IDD_PROP_COLOR);
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE(_T("color PSN_KILLACTIVE\n"));
				// �_�C�A���O�f�[�^�̎擾 color
				GetData(hwndDlg);
				return TRUE;
			case PSN_SETACTIVE:
				nPageNum = ID_PROPTYPE_PAGENUM_COLOR;
				return TRUE;
			}
			break;	// default
		}
		break;	// WM_NOTIFY
	case WM_DRAWITEM:
		idCtrl = (UINT) wParam;				// �R���g���[����ID
		pDis = (LPDRAWITEMSTRUCT) lParam;	// ���ڕ`����
		switch (idCtrl) {
		case IDC_BUTTON_TEXTCOLOR:	// �e�L�X�g�F
			DrawColorButton(pDis, types.colorInfoArr[nCurrentColorType].colorAttr.cTEXT);
			return TRUE;
		case IDC_BUTTON_BACKCOLOR:	// �w�i�F
			DrawColorButton(pDis, types.colorInfoArr[nCurrentColorType].colorAttr.cBACK);
			return TRUE;
		case IDC_LIST_COLORS:		// �F��ʃ��X�g
			DrawColorListItem(pDis);
			return TRUE;
		}
		break;

	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids2);
		}
		return TRUE;
		// NOTREACHED
//		break;

	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids2);
		return TRUE;

	}
	return FALSE;
}


// �_�C�A���O�f�[�^�̐ݒ� color
void PropTypesColor::SetData(HWND hwndDlg)
{

	HWND	hwndWork;
	int		nItem;

	nCurrentColorType = 0;	// ���ݑI������Ă���F�^�C�v

	// ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌�����
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_LINECOMMENT)		, COMMENT_DELIMITER_BUFFERSIZE - 1);
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_LINECOMMENT2)		, COMMENT_DELIMITER_BUFFERSIZE - 1);
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_LINECOMMENT3)		, COMMENT_DELIMITER_BUFFERSIZE - 1);
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM)	, BLOCKCOMMENT_BUFFERSIZE - 1);
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO)	, BLOCKCOMMENT_BUFFERSIZE - 1);
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2), BLOCKCOMMENT_BUFFERSIZE - 1);
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2)	, BLOCKCOMMENT_BUFFERSIZE - 1);

	::DlgItem_SetText(hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM	, types.blockComments[0].getBlockCommentFrom());	// �u���b�N�R�����g�f���~�^(From)
	::DlgItem_SetText(hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO		, types.blockComments[0].getBlockCommentTo());		// �u���b�N�R�����g�f���~�^(To)
	::DlgItem_SetText(hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2	, types.blockComments[1].getBlockCommentFrom());	// �u���b�N�R�����g�f���~�^2(From)
	::DlgItem_SetText(hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2	, types.blockComments[1].getBlockCommentTo());		// �u���b�N�R�����g�f���~�^2(To)

	// �s�R�����g�̊J�n���ʒu�ݒ�
	for (int i=0; i<COMMENT_DELIMITER_NUM; ++i) {
		// �e�L�X�g
		::DlgItem_SetText(hwndDlg, cLineComment[i].nEditID, types.lineComment.getLineComment(i));	

		// �����`�F�b�N�ƁA���l
		int nPos = types.lineComment.getLineCommentPos(i);
		if (nPos >= 0) {
			::CheckDlgButton(hwndDlg, cLineComment[i].nCheckBoxID, TRUE);
			::SetDlgItemInt(hwndDlg, cLineComment[i].nTextID, nPos + 1, FALSE);
		}else {
			::CheckDlgButton(hwndDlg, cLineComment[i].nCheckBoxID, FALSE);
			::SetDlgItemInt(hwndDlg, cLineComment[i].nTextID, (~nPos) + 1, FALSE);
		}
	}

	HWND	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_STRINGLITERAL);
	Combo_ResetContent(hwndCombo);
	int		nSelPos = 0;
	for (size_t i=0; i<_countof(StringLitteralArr); ++i) {
		Combo_InsertString(hwndCombo, i, LS(StringLitteralArr[i].nNameId));
		if (StringLitteralArr[i].nMethod == types.stringType) {		// �e�L�X�g�̐܂�Ԃ����@
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_STRINGLINEONLY, types.bStringLineOnly);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_STRINGENDLINE, types.bStringEndLine);
	::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_STRINGENDLINE),
		::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_STRINGLINEONLY));

	// �Z�b�g���R���{�{�b�N�X�̒l�Z�b�g
	hwndWork = ::GetDlgItem(hwndDlg, IDC_COMBO_SET);
	Combo_ResetContent(hwndWork);  // �R���{�{�b�N�X����ɂ���
	// ��s�ڂ͋�
	Combo_AddString(hwndWork, L" ");
	//	Mar. 31, 2003 genta KeywordSetMgr���|�C���^��
	if (0 < pKeywordSetMgr->nKeywordSetNum) {
		for (size_t i=0; i<pKeywordSetMgr->nKeywordSetNum; ++i) {
			Combo_AddString(hwndWork, pKeywordSetMgr->GetTypeName(i));
		}
		if (types.nKeywordSetIdx[0] == -1) {
			// �Z�b�g���R���{�{�b�N�X�̃f�t�H���g�I��
			Combo_SetCurSel(hwndWork, 0);
		}else {
			// �Z�b�g���R���{�{�b�N�X�̃f�t�H���g�I��
			Combo_SetCurSel(hwndWork, types.nKeywordSetIdx[0] + 1);
		}
	}

	// �����L�[���[�h1�`10�̐ݒ�
	for (int i=0; i<MAX_KEYWORDSET_PER_TYPE; ++i) {
		nSet[i] = types.nKeywordSetIdx[i];
	}

	// �F�����镶����ނ̃��X�g
	hwndWork = ::GetDlgItem(hwndDlg, IDC_LIST_COLORS);
	List_ResetContent(hwndWork);  // ���X�g����ɂ���
	// �傫���t�H���g�Ή�
	int nItemHeight = TextWidthCalc(hwndWork).GetTextHeight();
	List_SetItemHeight(hwndWork, 0, nItemHeight + 4);
	for (int i=0; i<COLORIDX_LAST; ++i) {
		GetDefaultColorInfoName(&types.colorInfoArr[i], i);
		nItem = ::List_AddString(hwndWork, types.colorInfoArr[i].szName);
		List_SetItemData(hwndWork, nItem, &types.colorInfoArr[i]);
	}
	// ���ݑI������Ă���F�^�C�v
	List_SetCurSel(hwndWork, nCurrentColorType);
	::SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_LIST_COLORS, LBN_SELCHANGE), (LPARAM)hwndWork);

	// �w��ʒu�c���̐ݒ�
	wchar_t szVertLine[MAX_VERTLINES * 15] = L"";
	int offset = 0;
	for (int i=0; i<MAX_VERTLINES && types.nVertLineIdx[i]!=0; ++i) {
		int nXCol = types.nVertLineIdx[i];
		int nXColEnd = nXCol;
		int nXColAdd = 1;
		if (nXCol < 0) {
			if (i < MAX_VERTLINES - 2) {
				nXCol = -nXCol;
				nXColEnd = types.nVertLineIdx[++i];
				nXColAdd = types.nVertLineIdx[++i];
				if (nXColEnd < nXCol || nXColAdd <= 0) {
					continue;
				}
				if (offset) {
					szVertLine[offset] = ',';
					szVertLine[offset + 1] = '\0';
					offset += 1;
				}
				offset += auto_sprintf(&szVertLine[offset], L"%d(%d,%d)", nXColAdd, nXCol, nXColEnd);
			}
		}else {
			if (offset) {
				szVertLine[offset] = ',';
				szVertLine[offset + 1] = '\0';
				offset += 1;
			}
			offset += auto_sprintf(&szVertLine[offset], L"%d", nXCol);
		}
	}
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_VERTLINE), MAX_VERTLINES * 15);
	::DlgItem_SetText(hwndDlg, IDC_EDIT_VERTLINE, szVertLine);
	return;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �_�C�A���O�f�[�^�̎擾 color
int PropTypesColor::GetData(HWND hwndDlg)
{
	int		nIdx;
	HWND	hwndWork;

	// From Here May 12, 2001 genta
	// �R�����g�̊J�n���ʒu�̎擾
	// May 21, 2001 genta ���ʒu��1���琔����悤��
	wchar_t buffer[COMMENT_DELIMITER_BUFFERSIZE];	// LineComment���擾���邽�߂̃o�b�t�@
	int pos;
	bool en;
	BOOL bTranslated;

	for (int i=0; i<COMMENT_DELIMITER_NUM; ++i) {
		en = DlgButton_IsChecked(hwndDlg, cLineComment[i].nCheckBoxID);
		pos = ::GetDlgItemInt(hwndDlg, cLineComment[i].nTextID, &bTranslated, FALSE);
		if (!bTranslated) {
			en = false;
			pos = 0;
		}
		//	pos == 0�̂Ƃ��͖�������
		if (pos == 0)	en = false;
		else			--pos;
		//	�����̂Ƃ���1�̕␔�Ŋi�[

		::DlgItem_GetText(hwndDlg, cLineComment[i].nEditID		, buffer	, COMMENT_DELIMITER_BUFFERSIZE);		// �s�R�����g�f���~�^
		types.lineComment.CopyTo(i, buffer, en ? pos : ~pos);
	}

	wchar_t szFromBuffer[BLOCKCOMMENT_BUFFERSIZE];
	wchar_t szToBuffer[BLOCKCOMMENT_BUFFERSIZE];

	::DlgItem_GetText(hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM	, szFromBuffer	, BLOCKCOMMENT_BUFFERSIZE);		// �u���b�N�R�����g�f���~�^(From)
	::DlgItem_GetText(hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO		, szToBuffer	, BLOCKCOMMENT_BUFFERSIZE);	// �u���b�N�R�����g�f���~�^(To)
	types.blockComments[0].SetBlockCommentRule(szFromBuffer, szToBuffer);

	::DlgItem_GetText(hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2	, szFromBuffer	, BLOCKCOMMENT_BUFFERSIZE);	// �u���b�N�R�����g�f���~�^(From)
	::DlgItem_GetText(hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2	, szToBuffer	, BLOCKCOMMENT_BUFFERSIZE);	// �u���b�N�R�����g�f���~�^(To)
	types.blockComments[1].SetBlockCommentRule(szFromBuffer, szToBuffer);

	// �������؂�L���G�X�P�[�v���@
	int		nSelPos = Combo_GetCurSel(GetDlgItem(hwndDlg, IDC_COMBO_STRINGLITERAL));
	if (nSelPos >= 0) {
		types.stringType = StringLitteralArr[nSelPos].nMethod;
	}
	types.bStringLineOnly = IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_STRINGLINEONLY);
	types.bStringEndLine = IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_STRINGENDLINE);
	

	// �Z�b�g���R���{�{�b�N�X�̒l�Z�b�g
	hwndWork = ::GetDlgItem(hwndDlg, IDC_COMBO_SET);
	nIdx = Combo_GetCurSel(hwndWork);
	if (nIdx == CB_ERR ||
		nIdx == 0
	) {
		types.nKeywordSetIdx[0] = -1;
	}else {
		types.nKeywordSetIdx[0] = nIdx - 1;
	}

	// �����L�[���[�h2�`10�̎擾(1�͕�)
	for (nIdx=1; nIdx<MAX_KEYWORDSET_PER_TYPE; ++nIdx) {
		types.nKeywordSetIdx[nIdx] = nSet[nIdx];
	}

	// �w��ʒu�c���̐ݒ�
	wchar_t szVertLine[MAX_VERTLINES * 15];
	::DlgItem_GetText(hwndDlg, IDC_EDIT_VERTLINE, szVertLine, MAX_VERTLINES * 15);

	int offset = 0;
	int i = 0;
	while (i < MAX_VERTLINES) {
		int value = 0;
		for (; '0'<=szVertLine[offset] && szVertLine[offset]<='9'; ++offset) {
			value = szVertLine[offset] - '0' + value * 10;
		}
		if (value <= 0) {
			break;
		}
		if (szVertLine[offset] == '(') {
			++offset;
			int valueBegin = 0;
			int valueEnd = 0;
			for (; '0'<=szVertLine[offset] && szVertLine[offset]<='9'; ++offset) {
				valueBegin = szVertLine[offset] - '0' + valueBegin * 10;
			}
			if (valueBegin <= 0) {
				break;
			}
			if (szVertLine[offset] == ',') {
				++offset;
			}else if (szVertLine[offset] != ')') {
				break;
			}
			for (; '0'<=szVertLine[offset] && szVertLine[offset]<='9'; ++offset) {
				valueEnd = szVertLine[offset] - '0' + valueEnd * 10;
			}
			if (valueEnd <= 0) {
				valueEnd = MAXLINEKETAS;
			}
			if (szVertLine[offset] != ')') {
				break;
			}
			++offset;
			if (i + 2 < MAX_VERTLINES) {
				types.nVertLineIdx[i++] = -valueBegin;
				types.nVertLineIdx[i++] = valueEnd;
				types.nVertLineIdx[i++] = value;
			}else {
				break;
			}
		}else {
			types.nVertLineIdx[i++] = value;
		}
		if (szVertLine[offset] != ',') {
			break;
		}
		++offset;
	}
	if (i < MAX_VERTLINES) {
		types.nVertLineIdx[i] = 0;
	}
	return TRUE;
}


// �F�{�^���̕`��
void PropTypesColor::DrawColorButton(DRAWITEMSTRUCT* pDis, COLORREF color)
{
//	MYTRACE(_T("pDis->itemAction = "));

	COLORREF	cBtnHiLight		= (COLORREF)::GetSysColor(COLOR_3DHILIGHT);
	COLORREF	cBtnShadow		= (COLORREF)::GetSysColor(COLOR_3DSHADOW);
	COLORREF	cBtnDkShadow	= (COLORREF)::GetSysColor(COLOR_3DDKSHADOW);
	COLORREF	cBtnFace		= (COLORREF)::GetSysColor(COLOR_3DFACE);
	RECT		rc;
	RECT		rcFocus;

	// �`��Ώ�
	Graphics gr(pDis->hDC);

	// �{�^���̕\�ʂ̐F�œh��Ԃ�
	gr.SetBrushColor(cBtnFace);
	gr.FillMyRect(pDis->rcItem);

	// �g�̕`��
	rc = pDis->rcItem;
	rc.top += 4;
	rc.left += 4;
	rc.right -= 4;
	rc.bottom -= 4;
	rcFocus = rc;
//	rc.right -= 11;

	if (pDis->itemState & ODS_SELECTED) {

		gr.SetPen(cBtnDkShadow);
		::MoveToEx(gr, 0, pDis->rcItem.bottom - 2, NULL);
		::LineTo(gr, 0, 0);
		::LineTo(gr, pDis->rcItem.right - 1, 0);

		gr.SetPen(cBtnShadow);
		::MoveToEx(gr, 1, pDis->rcItem.bottom - 3, NULL);
		::LineTo(gr, 1, 1);
		::LineTo(gr, pDis->rcItem.right - 2, 1);

		gr.SetPen(cBtnHiLight);
		::MoveToEx(gr, 0, pDis->rcItem.bottom - 1, NULL);
		::LineTo(gr, pDis->rcItem.right - 1, pDis->rcItem.bottom - 1);
		::LineTo(gr, pDis->rcItem.right - 1, -1);

		rc.top += 1;
		rc.left += 1;
		rc.right += 1;
		rc.bottom += 1;

		rcFocus.top += 1;
		rcFocus.left += 1;
		rcFocus.right += 1;
		rcFocus.bottom += 1;

	}else {
		gr.SetPen(cBtnHiLight);
		::MoveToEx(gr, 0, pDis->rcItem.bottom - 2, NULL);
		::LineTo(gr, 0, 0);
		::LineTo(gr, pDis->rcItem.right - 1, 0);

		gr.SetPen(cBtnShadow);
		::MoveToEx(gr, 1, pDis->rcItem.bottom - 2, NULL);
		::LineTo(gr, pDis->rcItem.right - 2, pDis->rcItem.bottom - 2);
		::LineTo(gr, pDis->rcItem.right - 2, 0);

		gr.SetPen(cBtnDkShadow);
		::MoveToEx(gr, 0, pDis->rcItem.bottom - 1, NULL);
		::LineTo(gr, pDis->rcItem.right - 1, pDis->rcItem.bottom - 1);
		::LineTo(gr, pDis->rcItem.right - 1, -1);
	}
	
	if ((pDis->itemState & ODS_DISABLED) == 0) {
		// �w��F�œh��Ԃ�
		gr.SetBrushColor(color);
		gr.SetPen(cBtnShadow);
		::RoundRect(gr, rc.left, rc.top, rc.right, rc.bottom , 5, 5);
	}

	// �t�H�[�J�X�̒����`
	if (pDis->itemState & ODS_FOCUS) {
		rcFocus.top -= 3;
		rcFocus.left -= 3;
		rcFocus.right += 2;
		rcFocus.bottom += 2;
		::DrawFocusRect(gr, &rcFocus);
	}
}


//	�`�F�b�N��Ԃɉ����ă_�C�A���O�{�b�N�X�v�f��Enable/Disable��
//	�K�؂ɐݒ肷��
void PropTypesColor::EnableTypesPropInput(HWND hwndDlg)
{
	//	�s�R�����g�J�n���ʒu���̓{�b�N�X��Enable/Disable�ݒ�
	//	1��
	if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_LCPOS)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_LINECOMMENTPOS), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_LCPOS), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_LCColNum), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_LINECOMMENTPOS), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_LCPOS), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_LCColNum), FALSE);
	}
	//	2��
	if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_LCPOS2)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_LINECOMMENTPOS2), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_LCPOS2), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_LCColNum2), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_LINECOMMENTPOS2), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_LCPOS2), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_LCColNum2), FALSE);
	}
	//	3��
	if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_LCPOS3)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_LINECOMMENTPOS3), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_LCPOS3), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_LCColNum3), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_LINECOMMENTPOS3), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_LCPOS3), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_LCColNum3), FALSE);
	}
}


/*!	@brief �L�[���[�h�Z�b�g�̍Ĕz��

	�L�[���[�h�Z�b�g�̐F�����ł͖��w��̃L�[���[�h�Z�b�g�ȍ~�̓`�F�b�N���ȗ�����D
	���̂��߃Z�b�g�̓r���ɖ��w��̂��̂�����ꍇ�͂���ȍ~��O�ɋl�߂邱�Ƃ�
	�w�肳�ꂽ�S�ẴL�[���[�h�Z�b�g���L���ɂȂ�悤�ɂ���D
	���̍ہC�F�����̐ݒ�������Ɉړ�����D

	nSet, types.colorInfoArr[]���ύX�����D

	@param hwndDlg [in] �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��
*/
void PropTypesColor::RearrangeKeywordSet(HWND hwndDlg)
{
	for (int i=0; i<MAX_KEYWORDSET_PER_TYPE; ++i) {
		if (nSet[i] != -1) {
			continue;
		}

		// ���ݒ�̏ꍇ
		int j;
		for (j=i; j<MAX_KEYWORDSET_PER_TYPE; ++j) {
			if (nSet[j] != -1) {
				// ���ɐݒ�ςݍ��ڂ��������ꍇ
				nSet[i] = nSet[j];
				nSet[j] = -1;

				// �F�ݒ�����ւ���
				// �\���̂��Ɠ���ւ���Ɩ��O���ς���Ă��܂��̂Œ���
				ColorInfo &col1 = types.colorInfoArr[COLORIDX_KEYWORD1 + i];
				ColorInfo &col2   = types.colorInfoArr[COLORIDX_KEYWORD1 + j];

				std::swap(col1.bDisp, col2.bDisp);
				std::swap(col1.fontAttr, col2.fontAttr);
				std::swap(col1.colorAttr, col2.colorAttr);

				break;
			}
		}
		if (j == MAX_KEYWORDSET_PER_TYPE) {
			// ���ɂ͐ݒ�ςݍ��ڂ��Ȃ�����
			break;
		}
	}
	
	// ���X�g�{�b�N�X�y�ѐF�ݒ�{�^�����ĕ`��
	::InvalidateRect(::GetDlgItem(hwndDlg, IDC_BUTTON_TEXTCOLOR), NULL, TRUE);
	::InvalidateRect(::GetDlgItem(hwndDlg, IDC_BUTTON_BACKCOLOR), NULL, TRUE);
	::InvalidateRect(::GetDlgItem(hwndDlg, IDC_LIST_COLORS), NULL, TRUE);
}


// �F��ʃ��X�g �I�[�i�[�`��
void PropTypesColor::DrawColorListItem(DRAWITEMSTRUCT* pDis)
{
	ColorInfo*	pColorInfo;
//	RECT		rc0,rc1,rc2;
	RECT		rc1;
	COLORREF	cRim = (COLORREF)::GetSysColor(COLOR_3DSHADOW);

	if (!pDis || pDis->itemData == 0) {
		return;
	}

	// �`��Ώ�
	Graphics gr(pDis->hDC);

//	rc0 = pDis->rcItem;
	rc1 = pDis->rcItem;
//	rc2 = pDis->rcItem;

	// �A�C�e���f�[�^�̎擾
	pColorInfo = (ColorInfo*)pDis->itemData;

	// �A�C�e����`�h��Ԃ�
	gr.SetBrushColor(::GetSysColor(COLOR_WINDOW));
	gr.FillMyRect(pDis->rcItem);
	
	// �A�C�e�����I������Ă���
	if (pDis->itemState & ODS_SELECTED) {
		gr.SetBrushColor(::GetSysColor(COLOR_HIGHLIGHT));
		gr.SetTextForeColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
	}else {
		gr.SetBrushColor(::GetSysColor(COLOR_WINDOW));
		gr.SetTextForeColor(::GetSysColor(COLOR_WINDOWTEXT));
	}

	rc1.left += (2 + 16);
	rc1.top += 2;
	rc1.right -= (2 + 27);
	rc1.bottom -= 2;
	// �I���n�C���C�g��`
	gr.FillMyRect(rc1);
	// �e�L�X�g
	::SetBkMode(gr, TRANSPARENT);
	::TextOut(gr, rc1.left, rc1.top, pColorInfo->szName, (int)_tcslen(pColorInfo->szName));
	if (pColorInfo->fontAttr.bBoldFont) {	// ������
		::TextOut(gr, rc1.left + 1, rc1.top, pColorInfo->szName, (int)_tcslen(pColorInfo->szName));
	}
	if (pColorInfo->fontAttr.bUnderLine) {	// ������
		SIZE	sz;
		::GetTextExtentPoint32(gr, pColorInfo->szName, (int)_tcslen(pColorInfo->szName), &sz);
		::MoveToEx(gr, rc1.left,		rc1.bottom - 2, NULL);
		::LineTo(gr, rc1.left + sz.cx,	rc1.bottom - 2);
		::MoveToEx(gr, rc1.left,		rc1.bottom - 1, NULL);
		::LineTo(gr, rc1.left + sz.cx,	rc1.bottom - 1);
	}

	// �A�C�e���Ƀt�H�[�J�X������
	if (pDis->itemState & ODS_FOCUS) {
		::DrawFocusRect(gr, &pDis->rcItem);
	}

	//�u�F����/�\������v�̃`�F�b�N
	rc1 = pDis->rcItem;
	rc1.left += 2;
	rc1.top += 3;
	rc1.right = rc1.left + 12;
	rc1.bottom = rc1.top + 12;
	if (pColorInfo->bDisp) {	// �F����/�\������
		// �e�L�X�g�F���g���i�u�n�C�R���g���X�g���v�̂悤�Ȑݒ�ł�������悤�Ɂj
		gr.SetPen(::GetSysColor(COLOR_WINDOWTEXT));

		::MoveToEx(gr,	rc1.left + 2, rc1.top + 6, NULL);
		::LineTo(gr,	rc1.left + 5, rc1.bottom - 3);
		::LineTo(gr,	rc1.right - 2, rc1.top + 4);
		rc1.top -= 1;
		rc1.bottom -= 1;
		::MoveToEx(gr,	rc1.left + 2, rc1.top + 6, NULL);
		::LineTo(gr,	rc1.left + 5, rc1.bottom - 3);
		::LineTo(gr,	rc1.right - 2, rc1.top + 4);
		rc1.top -= 1;
		rc1.bottom -= 1;
		::MoveToEx(gr,	rc1.left + 2, rc1.top + 6, NULL);
		::LineTo(gr,	rc1.left + 5, rc1.bottom - 3);
		::LineTo(gr,	rc1.right - 2, rc1.top + 4);
	}
//	return;


	if ((g_ColorAttributeArr[pColorInfo->nColorIdx].fAttribute & COLOR_ATTRIB_NO_BACK) == 0) {
		// �w�i�F ���{��`
		rc1 = pDis->rcItem;
		rc1.left = rc1.right - 13;
		rc1.top += 2;
		rc1.right = rc1.left + 12;
		rc1.bottom -= 2;

		gr.SetBrushColor(pColorInfo->colorAttr.cBACK);
		gr.SetPen(cRim);
		::RoundRect(pDis->hDC, rc1.left, rc1.top, rc1.right, rc1.bottom , 3, 3);
	}

	if ((g_ColorAttributeArr[pColorInfo->nColorIdx].fAttribute & COLOR_ATTRIB_NO_TEXT) == 0) {
		// �O�i�F ���{��`
		rc1 = pDis->rcItem;
		rc1.left = rc1.right - 27;
		rc1.top += 2;
		rc1.right = rc1.left + 12;
		rc1.bottom -= 2;
		gr.SetBrushColor(pColorInfo->colorAttr.cTEXT);
		gr.SetPen(cRim);
		::RoundRect(pDis->hDC, rc1.left, rc1.top, rc1.right, rc1.bottom , 3, 3);
	}
}


// �F�I���_�C�A���O
BOOL PropTypesColor::SelectColor(
	HWND hwndParent,
	COLORREF* pColor,
	DWORD* pCustColors
	)
{
	CHOOSECOLOR cc;
	cc.lStructSize = sizeof_raw(cc);
	cc.hwndOwner = hwndParent;
	cc.hInstance = NULL;
	cc.rgbResult = *pColor;
	cc.lpCustColors = pCustColors;
	cc.Flags = /*CC_PREVENTFULLOPEN |*/ CC_RGBINIT;
	cc.lCustData = 0;
	cc.lpfnHook = nullptr;
	cc.lpTemplateName = NULL;
	if (!::ChooseColor(&cc)) {
		return FALSE;
	}
	*pColor = cc.rgbResult;
	return TRUE;
}

