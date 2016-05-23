/*
	�^�C�v�ʐݒ� - �E�B���h�E

	2008.04.12 kobake CPropTypes.cpp���番��
	2009.02.22 ryoji
*/
#include "StdAfx.h"
#include "PropTypes.h"
#include "env/ShareData.h"
#include "typeprop/ImpExpManager.h"	// 2010/4/23 Uchi
#include "DlgSameColor.h"
#include "DlgKeywordSelect.h"
#include "view/colors/EColorIndexType.h"
#include "charset/CodePage.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

using namespace std;

static const DWORD p_helpids2[] = {	//11400
	IDC_CHECK_DOCICON,				HIDC_CHECK_DOCICON,				// �����A�C�R�����g��	// 2006.08.06 ryoji

	IDC_COMBO_IMESWITCH,			HIDC_COMBO_IMESWITCH,			// IME��ON/OFF���
	IDC_COMBO_IMESTATE,				HIDC_COMBO_IMESTATE,			// IME�̓��̓��[�h

	IDC_COMBO_DEFAULT_CODETYPE,		HIDC_COMBO_DEFAULT_CODETYPE,	// �f�t�H���g�����R�[�h
	IDC_CHECK_CP,					HIDC_CHECK_TYPE_SUPPORT_CP,		// �R�[�h�y�[�W
	IDC_COMBO_DEFAULT_EOLTYPE,		HIDC_COMBO_DEFAULT_EOLTYPE,		// �f�t�H���g���s�R�[�h	// 2011.01.24 ryoji
	IDC_CHECK_DEFAULT_BOM,			HIDC_CHECK_DEFAULT_BOM,			// �f�t�H���gBOM	// 2011.01.24 ryoji
	IDC_CHECK_PRIOR_CESU8,			HIDC_CHECK_PRIOR_CESU8,			// �������ʎ���CESU-8��D�悷��

	IDC_EDIT_BACKIMG_PATH,			HIDC_EDIT_BACKIMG_PATH,			// �w�i�摜
	IDC_BUTTON_BACKIMG_PATH_SEL,	HIDC_BUTTON_BACKIMG_PATH_SEL,	// �w�i�摜�{�^��
	IDC_COMBO_BACKIMG_POS,			HIDC_COMBO_BACKIMG_POS,			// �w�i�摜�ʒu
	IDC_CHECK_BACKIMG_SCR_X,		HIDC_CHECK_BACKIMG_SCR_X,		// �w�i�摜ScrollX
	IDC_CHECK_BACKIMG_SCR_Y,		HIDC_CHECK_BACKIMG_SCR_Y,		// �w�i�摜ScrollY
	IDC_CHECK_BACKIMG_REP_X,		HIDC_CHECK_BACKIMG_REP_X,		// �w�i�摜RepeatX
	IDC_CHECK_BACKIMG_REP_Y,		HIDC_CHECK_BACKIMG_REP_Y,		// �w�i�摜RepeatY
	IDC_EDIT_BACKIMG_OFFSET_X,		HIDC_EDIT_BACKIMG_OFFSET_X,		// �w�i�摜OffsetX
	IDC_EDIT_BACKIMG_OFFSET_Y,		HIDC_EDIT_BACKIMG_OFFSET_Y,		// �w�i�摜OffsetY
	IDC_RADIO_LINENUM_LAYOUT,		HIDC_RADIO_LINENUM_LAYOUT,		// �s�ԍ��̕\���i�܂�Ԃ��P�ʁj
	IDC_RADIO_LINENUM_CRLF,			HIDC_RADIO_LINENUM_CRLF,		// �s�ԍ��̕\���i���s�P�ʁj
	IDC_RADIO_LINETERMTYPE0,		HIDC_RADIO_LINETERMTYPE0,		// �s�ԍ���؂�i�Ȃ��j
	IDC_RADIO_LINETERMTYPE1,		HIDC_RADIO_LINETERMTYPE1,		// �s�ԍ���؂�i�c���j
	IDC_RADIO_LINETERMTYPE2,		HIDC_RADIO_LINETERMTYPE2,		// �s�ԍ���؂�i�C�Ӂj
	IDC_EDIT_LINETERMCHAR,			HIDC_EDIT_LINETERMCHAR,			// �s�ԍ���؂�
	IDC_EDIT_LINENUMWIDTH,			HIDC_EDIT_LINENUMWIDTH,			// �s�ԍ��̍ŏ����� 2014.08.02 katze
//	IDC_STATIC,						-1,
	0, 0
};



TYPE_NAME_ID<int> ImeSwitchArr[] = {
	{ 0, STR_IME_SWITCH_DONTSET },
	{ 1, STR_IME_SWITCH_ON },
	{ 2, STR_IME_SWITCH_OFF },
};

//	Nov. 20, 2000 genta
TYPE_NAME_ID<int> ImeStateArr[] = {
	{ 0, STR_IME_STATE_DEF },
	{ 1, STR_IME_STATE_FULL },
	{ 2, STR_IME_STATE_FULLHIRA },
	{ 3, STR_IME_STATE_FULLKATA },
	{ 4, STR_IME_STATE_NO }
};

static const wchar_t* aszEolStr[] = {
	L"CR+LF",
	L"LF (UNIX)",
	L"CR (Mac)",
	L"NEL",
	L"LS",
	L"PS",
};
static const EolType aeEolType[] = {
	EolType::CRLF,
	EolType::LF,
	EolType::CR,
	EolType::NEL,
	EolType::LS,
	EolType::PS,
};

// window ���b�Z�[�W����
INT_PTR PropTypesWindow::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
	)
{
	WORD	wNotifyCode;
	WORD	wID;
	NMHDR*	pNMHDR;
	NM_UPDOWN* pMNUD;		// �ǉ� 2014.08.02 katze

	switch (uMsg) {
	case WM_INITDIALOG:
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// �_�C�A���O�f�[�^�̐ݒ� color
		SetData(hwndDlg);

		// ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌�����
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_LINETERMCHAR), 1);

		return TRUE;

	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	// �ʒm�R�[�h
		wID			= LOWORD(wParam);	// ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID

		switch (wNotifyCode) {
		case CBN_SELCHANGE:
			{
				int i;
				switch (wID) {
				case IDC_COMBO_DEFAULT_CODETYPE:
					// �����R�[�h�̕ύX��BOM�`�F�b�N�{�b�N�X�ɔ��f
					i = Combo_GetCurSel((HWND) lParam);
					if (CB_ERR != i) {
						CodeTypeName	codeTypeName(Combo_GetItemData((HWND)lParam, i));
						::CheckDlgButton(hwndDlg, IDC_CHECK_DEFAULT_BOM, (codeTypeName.IsBomDefOn() ?  BST_CHECKED : BST_UNCHECKED));
						::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_DEFAULT_BOM), codeTypeName.UseBom());
					}
					break;
				}
			}
			break;

		// �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ
		case BN_CLICKED:
			switch (wID) {
			case IDC_BUTTON_BACKIMG_PATH_SEL:
				{
					Dialog::SelectFile(hwndDlg, GetDlgItem(hwndDlg, IDC_EDIT_BACKIMG_PATH),
						_T("*.bmp;*.jpg;*.jpeg"), true);
				}
				return TRUE;
			//	From Here Sept. 10, 2000 JEPRO
			//	�s�ԍ���؂��C�ӂ̔��p�����ɂ���Ƃ������w�蕶�����͂�Enable�ɐݒ�
			case IDC_RADIO_LINETERMTYPE0: // �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C��
			case IDC_RADIO_LINETERMTYPE1:
			case IDC_RADIO_LINETERMTYPE2:
				EnableTypesPropInput(hwndDlg);
				return TRUE;
			//	To Here Sept. 10, 2000

			}
			case IDC_CHECK_CP:
				{
					::CheckDlgButton(hwndDlg, IDC_CHECK_CP, TRUE);
					::EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_CP), FALSE);
					CodePage::AddComboCodePages(hwndDlg, ::GetDlgItem(hwndDlg, IDC_COMBO_DEFAULT_CODETYPE), -1);
				}
				return TRUE;
			break;	// BN_CLICKED
		}
		break;	// WM_COMMAND
	case WM_NOTIFY:
		pNMHDR = (NMHDR*)lParam;
		switch (pNMHDR->code) {
		case PSN_HELP:
//	Sept. 10, 2000 JEPRO ID�������ۂ̖��O�ɕύX���邽�߈ȉ��̍s�̓R�����g�A�E�g
//				OnHelp(hwndDlg, IDD_PROP1P3);
			OnHelp(hwndDlg, IDD_PROP_WINDOW);
			return TRUE;
		case PSN_KILLACTIVE:
//			MYTRACE(_T("color PSN_KILLACTIVE\n"));
			// �_�C�A���O�f�[�^�̎擾 window
			GetData(hwndDlg);
			return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
		case PSN_SETACTIVE:
			nPageNum = ID_PROPTYPE_PAGENUM_WINDOW;
			return TRUE;
		}

		// switch���ǉ� 2014.08.02 katze
		pMNUD  = (NM_UPDOWN*)lParam;
		switch ((int)wParam) {
		case IDC_SPIN_LINENUMWIDTH:
			// �s�ԍ��̍ŏ�����
//			MYTRACE( _T("IDC_SPIN_LINENUMWIDTH\n") );
			int nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_LINENUMWIDTH, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < LINENUMWIDTH_MIN) {
				nVal = LINENUMWIDTH_MIN;
			}
			if (nVal > LINENUMWIDTH_MAX) {
				nVal = LINENUMWIDTH_MAX;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_LINENUMWIDTH, nVal, FALSE);
			return TRUE;
		}

		break;	// WM_NOTIFY

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids2);	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		// NOTREACHED
//		break;
//@@@ 2001.02.04 End

//@@@ 2001.11.17 add start MIK
	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids2);	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//@@@ 2001.11.17 add end MIK

	}
	return FALSE;
}


void PropTypesWindow::SetCombobox(
	HWND hwndWork,
	const int* nIds,
	int nCount,
	int select
	)
{
	Combo_ResetContent(hwndWork);
	for (int i=0; i<nCount; ++i) {
		Combo_AddString(hwndWork, LS(nIds[i]));
	}
	Combo_SetCurSel(hwndWork, select);
}


// �_�C�A���O�f�[�^�̐ݒ� window
void PropTypesWindow::SetData(HWND hwndDlg)
{
	{
		// �����A�C�R�����g��	// Sep. 10, 2002 genta
		::CheckDlgButtonBool(hwndDlg, IDC_CHECK_DOCICON, types.bUseDocumentIcon);
	}

	// �N������IME(���{����͕ϊ�)	// Nov. 20, 2000 genta
	{
		int ime;
		// ON/OFF���
		HWND	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_IMESWITCH);
		Combo_ResetContent(hwndCombo);
		ime = types.nImeState & 3;
		int		nSelPos = 0;
		for (int i=0; i<_countof(ImeSwitchArr); ++i) {
			Combo_InsertString(hwndCombo, i, LS(ImeSwitchArr[i].nNameId));
			if (ImeSwitchArr[i].nMethod == ime) {	// IME���
				nSelPos = i;
			}
		}
		Combo_SetCurSel(hwndCombo, nSelPos);

		// ���̓��[�h
		hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_IMESTATE);
		Combo_ResetContent(hwndCombo);
		ime = types.nImeState >> 2;
		nSelPos = 0;
		for (int i=0; i<_countof(ImeStateArr); ++i) {
			Combo_InsertString(hwndCombo, i, LS(ImeStateArr[i].nNameId));
			if (ImeStateArr[i].nMethod == ime) {	// IME���
				nSelPos = i;
			}
		}
		Combo_SetCurSel(hwndCombo, nSelPos);
	}

	// �u�����R�[�h�v�O���[�v�̐ݒ�
	{
		HWND hCombo;

		// �u�����F������CESU-8��D��vtypes.encoding.bPriorCesu8 ���`�F�b�N
		::CheckDlgButton(hwndDlg, IDC_CHECK_PRIOR_CESU8, types.encoding.bPriorCesu8);

		// �f�t�H���g�R�[�h�^�C�v�̃R���{�{�b�N�X�ݒ�
		int		nSel= 0;
		int		j = 0;
		hCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_DEFAULT_CODETYPE);
		CodeTypesForCombobox codeTypes;
		for (int i=0; i<codeTypes.GetCount(); ++i) {
			if (CodeTypeName(codeTypes.GetCode(i)).CanDefault()) {
				int idx = Combo_AddString(hCombo, codeTypes.GetName(i));
				Combo_SetItemData(hCombo, idx, codeTypes.GetCode(i));
				if (types.encoding.eDefaultCodetype == codeTypes.GetCode(i)) {
					nSel = j;
				}
				++j;
			}
		}
		if (nSel == -1) {
			::CheckDlgButton(hwndDlg, IDC_CHECK_CP, TRUE);
			::EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_CP), FALSE);
			int nIdx = CodePage::AddComboCodePages(hwndDlg, hCombo, types.encoding.eDefaultCodetype);
			if (nIdx == -1) {
				nSel = 0;
			}else {
				nSel = nIdx;
			}
		}
		Combo_SetCurSel(hCombo, nSel);

		// BOM �`�F�b�N�{�b�N�X�ݒ�
		CodeTypeName	cCodeTypeName(types.encoding.eDefaultCodetype);
		if (!cCodeTypeName.UseBom()) {
			types.encoding.bDefaultBom = false;
		}
		::CheckDlgButton(hwndDlg, IDC_CHECK_DEFAULT_BOM, (types.encoding.bDefaultBom ? BST_CHECKED : BST_UNCHECKED));
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_DEFAULT_BOM), (int)cCodeTypeName.UseBom());

		// �f�t�H���g���s�^�C�v�̃R���{�{�b�N�X�ݒ�
		hCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_DEFAULT_EOLTYPE);
		for (int i=0; i<_countof(aszEolStr); ++i) {
			ApiWrap::Combo_AddString(hCombo, aszEolStr[i]);
		}
		int i;
		for (i=0; i<_countof(aeEolType); ++i) {
			if (types.encoding.eDefaultEoltype == aeEolType[i]) {
				break;
			}
		}
		if (i == _countof(aeEolType)) {
			i = 0;
		}
		Combo_SetCurSel(hCombo, i);
	}

	// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
	if (!types.bLineNumIsCRLF) {
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINENUM_LAYOUT, TRUE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINENUM_CRLF, FALSE);
	}else {
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINENUM_LAYOUT, FALSE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINENUM_CRLF, TRUE);
	}

	{
		// �s�ԍ��̍ŏ�����	// �ǉ� 2014.08.02 katze
		::SetDlgItemInt(hwndDlg, IDC_EDIT_LINENUMWIDTH, types.nLineNumWidth, FALSE);
	}

	// �w�i�摜
	EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_BACKIMG_PATH), _countof2(types.szBackImgPath));
	EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_X), 5);
	EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_Y), 5);

	DlgItem_SetText(hwndDlg, IDC_EDIT_BACKIMG_PATH, types.szBackImgPath);
	{
		static const int posNameId[] ={
			STR_IMAGE_POS1,
			STR_IMAGE_POS2,
			STR_IMAGE_POS3,
			STR_IMAGE_POS4,
			STR_IMAGE_POS5,
			STR_IMAGE_POS6,
			STR_IMAGE_POS7,
			STR_IMAGE_POS8,
			STR_IMAGE_POS9,
		};
		// BGIMAGE_TOP_LEFT ..
		int nCount = _countof(posNameId);
		SetCombobox(::GetDlgItem(hwndDlg, IDC_COMBO_BACKIMG_POS), posNameId, nCount, (int)types.backImgPos);
	}
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKIMG_REP_X, types.backImgRepeatX);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKIMG_REP_Y, types.backImgRepeatY);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKIMG_SCR_X, types.backImgScrollX);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKIMG_SCR_Y, types.backImgScrollY);
	SetDlgItemInt(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_X, types.backImgPosOffset.x, TRUE);
	SetDlgItemInt(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_Y, types.backImgPosOffset.y, TRUE);

	// �s�ԍ���؂�  0=�Ȃ� 1=�c�� 2=�C��
	if (types.nLineTermType == 0) {
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINETERMTYPE0, TRUE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINETERMTYPE1, FALSE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINETERMTYPE2, FALSE);
	}else
	if (types.nLineTermType == 1) {
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINETERMTYPE0, FALSE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINETERMTYPE1, TRUE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINETERMTYPE2, FALSE);
	}else
	if (types.nLineTermType == 2) {
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINETERMTYPE0, FALSE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINETERMTYPE1, FALSE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_LINETERMTYPE2, TRUE);
	}

	// �s�ԍ���؂蕶��
	wchar_t	szLineTermChar[2];
	auto_sprintf_s(szLineTermChar, L"%lc", types.cLineTermChar);
	::DlgItem_SetText(hwndDlg, IDC_EDIT_LINETERMCHAR, szLineTermChar);

	//	From Here Sept. 10, 2000 JEPRO
	//	�s�ԍ���؂��C�ӂ̔��p�����ɂ���Ƃ������w�蕶�����͂�Enable�ɐݒ�
	EnableTypesPropInput(hwndDlg);
	//	To Here Sept. 10, 2000

	return;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �_�C�A���O�f�[�^�̎擾 color
int PropTypesWindow::GetData(HWND hwndDlg)
{
	{
		// �����A�C�R�����g��	// Sep. 10, 2002 genta
		types.bUseDocumentIcon = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DOCICON);
	}

	// �N������IME(���{����͕ϊ�)	Nov. 20, 2000 genta
	{
		// ���̓��[�h
		HWND	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_IMESTATE);
		int		nSelPos = Combo_GetCurSel(hwndCombo);
		types.nImeState = ImeStateArr[nSelPos].nMethod << 2;	//	IME���̓��[�h

		// ON/OFF���
		hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_IMESWITCH);
		nSelPos = Combo_GetCurSel(hwndCombo);
		types.nImeState |= ImeSwitchArr[nSelPos].nMethod;	//	IME ON/OFF
	}

	// �u�����R�[�h�v�O���[�v�̐ݒ�
	{
		// types.bPriorCesu8 ��ݒ�
		types.encoding.bPriorCesu8 = DlgButton_IsChecked(hwndDlg, IDC_CHECK_PRIOR_CESU8);

		// types.eDefaultCodetype ��ݒ�
		HWND hCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_DEFAULT_CODETYPE);
		int i = Combo_GetCurSel(hCombo);
		if (CB_ERR != i) {
			types.encoding.eDefaultCodetype = EncodingType(Combo_GetItemData(hCombo, i));
		}

		// types.bDefaultBom ��ݒ�
		types.encoding.bDefaultBom = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DEFAULT_BOM);

		// types.eDefaultEoltype ��ݒ�
		hCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_DEFAULT_EOLTYPE);
		i = Combo_GetCurSel(hCombo);
		if (CB_ERR != i) {
			types.encoding.eDefaultEoltype = aeEolType[i];
		}
	}

	// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
	types.bLineNumIsCRLF = !DlgButton_IsChecked(hwndDlg, IDC_RADIO_LINENUM_LAYOUT);

	DlgItem_GetText(hwndDlg, IDC_EDIT_BACKIMG_PATH, types.szBackImgPath, _countof2(types.szBackImgPath));
	types.backImgPos = static_cast<BackgroundImagePosType>(Combo_GetCurSel(GetDlgItem(hwndDlg, IDC_COMBO_BACKIMG_POS)));
	types.backImgRepeatX = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKIMG_REP_X);
	types.backImgRepeatY = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKIMG_REP_Y);
	types.backImgScrollX = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKIMG_SCR_X);
	types.backImgScrollY = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKIMG_SCR_Y);
	types.backImgPosOffset.x = GetDlgItemInt(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_X, NULL, TRUE);
	types.backImgPosOffset.y = GetDlgItemInt(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_Y, NULL, TRUE);

	// �s�ԍ���؂�  0=�Ȃ� 1=�c�� 2=�C��
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_LINETERMTYPE0)) {
		types.nLineTermType = 0;
	}else
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_LINETERMTYPE1)) {
		types.nLineTermType = 1;
	}else
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_LINETERMTYPE2)) {
		types.nLineTermType = 2;
	}
	
	// �s�ԍ���؂蕶��
	wchar_t	szLineTermChar[2];
	::DlgItem_GetText(hwndDlg, IDC_EDIT_LINETERMCHAR, szLineTermChar, 2);
	types.cLineTermChar = szLineTermChar[0];

	// �s�ԍ��̍ŏ�����		// �ǉ� 2014.08.02 katze
	types.nLineNumWidth = ::GetDlgItemInt(hwndDlg, IDC_EDIT_LINENUMWIDTH, NULL, FALSE);
	if (types.nLineNumWidth < LINENUMWIDTH_MIN) {
		types.nLineNumWidth = LINENUMWIDTH_MIN;
	}
	if (types.nLineNumWidth > LINENUMWIDTH_MAX) {
		types.nLineNumWidth = LINENUMWIDTH_MAX;
	}

	return TRUE;
}


//	From Here Sept. 10, 2000 JEPRO
//	�`�F�b�N��Ԃɉ����ă_�C�A���O�{�b�N�X�v�f��Enable/Disable��
//	�K�؂ɐݒ肷��
void PropTypesWindow::EnableTypesPropInput(HWND hwndDlg)
{
	//	�s�ԍ���؂��C�ӂ̔��p�����ɂ��邩�ǂ���
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_LINETERMTYPE2)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_LINETERMCHAR), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_LINETERMCHAR), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_LINETERMCHAR), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_LINETERMCHAR), FALSE);
	}
}
//	To Here Sept. 10, 2000

