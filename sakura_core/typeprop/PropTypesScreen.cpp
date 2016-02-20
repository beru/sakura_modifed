/*! @file
	@brief �^�C�v�ʐݒ� - �X�N���[��

	@date 2008.04.12 kobake CPropTypes.cpp���番��
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, jepro, genta, MIK, hor, Stonee, asa-o
	Copyright (C) 2002, YAZAKI, aroka, MIK, genta, ������, Moca
	Copyright (C) 2003, MIK, zenryaku, Moca, naoh, KEITA, genta
	Copyright (C) 2005, MIK, genta, Moca, ryoji
	Copyright (C) 2006, ryoji, fon, novice
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2008, nasukoji
	Copyright (C) 2009, ryoji, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "PropTypes.h"
#include "dlg/DlgOpenFile.h"
#include "util/module.h"
#include "util/shell.h"
#include "util/window.h"
#include "util/fileUtil.h" // _IS_REL_PATH
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids1[] = {	//11300
	IDC_EDIT_TYPENAME,				HIDC_EDIT_TYPENAME,			// �ݒ�̖��O
	IDC_EDIT_TYPEEXTS,				HIDC_EDIT_TYPEEXTS,			// �t�@�C���g���q

	IDC_COMBO_WRAPMETHOD,			HIDC_COMBO_WRAPMETHOD,		// �e�L�X�g�̐܂�Ԃ����@		// 2008.05.30 nasukoji
	IDC_EDIT_MAXLINELEN,			HIDC_EDIT_MAXLINELEN,		// �܂�Ԃ�����
	IDC_SPIN_MAXLINELEN,			HIDC_EDIT_MAXLINELEN,
	IDC_EDIT_CHARSPACE,				HIDC_EDIT_CHARSPACE,		// �����̊Ԋu
	IDC_SPIN_CHARSPACE,				HIDC_EDIT_CHARSPACE,
	IDC_EDIT_LINESPACE,				HIDC_EDIT_LINESPACE,		// �s�̊Ԋu
	IDC_SPIN_LINESPACE,				HIDC_EDIT_LINESPACE,
	IDC_EDIT_TABSPACE,				HIDC_EDIT_TABSPACE,			// TAB�� // Sep. 19, 2002 genta
	IDC_SPIN_TABSPACE,				HIDC_EDIT_TABSPACE,
	IDC_EDIT_TABVIEWSTRING,			HIDC_EDIT_TABVIEWSTRING,	// TAB�\��������
	IDC_CHECK_TAB_ARROW,			HIDC_CHECK_TAB_ARROW,		// ���\��	// 2006.08.06 ryoji
	IDC_CHECK_INS_SPACE,			HIDC_CHECK_INS_SPACE,		// �X�y�[�X�̑}��

	IDC_CHECK_INDENT,				HIDC_CHECK_INDENT,			// �����C���f���g	// 2006.08.19 ryoji
	IDC_CHECK_INDENT_WSPACE,		HIDC_CHECK_INDENT_WSPACE,	// �S�p�󔒂��C���f���g	// 2006.08.19 ryoji
	IDC_COMBO_SMARTINDENT,			HIDC_COMBO_SMARTINDENT,		// �X�}�[�g�C���f���g
	IDC_EDIT_INDENTCHARS,			HIDC_EDIT_INDENTCHARS,		// ���̑��̃C���f���g�Ώە���
	IDC_COMBO_INDENTLAYOUT,			HIDC_COMBO_INDENTLAYOUT,	// �܂�Ԃ��s�C���f���g	// 2006.08.06 ryoji
	IDC_CHECK_RTRIM_PREVLINE,		HIDC_CHECK_RTRIM_PREVLINE,	// ���s���ɖ����̋󔒂��폜	// 2006.08.06 ryoji

	IDC_RADIO_OUTLINEDEFAULT,		HIDC_RADIO_OUTLINEDEFAULT,	// �W�����[��	// 2006.08.06 ryoji
	IDC_COMBO_OUTLINES,				HIDC_COMBO_OUTLINES,		// �A�E�g���C����͕��@
	IDC_RADIO_OUTLINERULEFILE,		HIDC_RADIO_OUTLINERULEFILE,	// ���[���t�@�C��	// 2006.08.06 ryoji
	IDC_EDIT_OUTLINERULEFILE,		HIDC_EDIT_OUTLINERULEFILE,	// ���[���t�@�C����	// 2006.08.06 ryoji
	IDC_BUTTON_RULEFILE_REF,		HIDC_BUTTON_RULEFILE_REF,	// ���[���t�@�C���Q��	// 2006/09/09 novice

	IDC_CHECK_USETYPEFONT,			HIDC_CHECK_USETYPEFONT,		// �^�C�v�ʃt�H���g�g�p����
	IDC_BUTTON_TYPEFONT,			HIDC_BUTTON_TYPEFONT,		// �^�C�v�ʃt�H���g

	IDC_CHECK_WORDWRAP,				HIDC_CHECK_WORDWRAP,		// �p�����[�h���b�v
	IDC_CHECK_KINSOKURET,			HIDC_CHECK_KINSOKURET,		// ���s�������Ԃ牺����	//@@@ 2002.04.14 MIK
	IDC_CHECK_KINSOKUHIDE,			HIDC_CHECK_KINSOKUHIDE,		// �Ԃ牺�����B��		// 2012.11.30 Uchi
	IDC_CHECK_KINSOKUKUTO,			HIDC_CHECK_KINSOKUKUTO,		// ��Ǔ_���Ԃ牺����	//@@@ 2002.04.17 MIK
	IDC_EDIT_KINSOKUKUTO,			HIDC_EDIT_KINSOKUKUTO,		// ��Ǔ_�Ԃ牺������	// 2009.08.07 ryoji
	IDC_CHECK_KINSOKUHEAD,			HIDC_CHECK_KINSOKUHEAD,		// �s���֑�	//@@@ 2002.04.08 MIK
	IDC_EDIT_KINSOKUHEAD,			HIDC_EDIT_KINSOKUHEAD,		// �s���֑�	//@@@ 2002.04.08 MIK
	IDC_CHECK_KINSOKUTAIL,			HIDC_CHECK_KINSOKUTAIL,		// �s���֑�	//@@@ 2002.04.08 MIK
	IDC_EDIT_KINSOKUTAIL,			HIDC_EDIT_KINSOKUTAIL,		// �s���֑�	//@@@ 2002.04.08 MIK
//	IDC_STATIC,						-1,
	0, 0
};


// �A�E�g���C����͕��@�E�W�����[��
TYPE_NAME_ID<EOutlineType> OlmArr[] = {
//	{ OUTLINE_C,		_T("C") },
	{ OUTLINE_CPP,		STR_OUTLINE_CPP },
	{ OUTLINE_PLSQL,	STR_OUTLINE_PLSQL },
	{ OUTLINE_JAVA,		STR_OUTLINE_JAVA },
	{ OUTLINE_COBOL,	STR_OUTLINE_COBOL },
	{ OUTLINE_PERL,		STR_OUTLINE_PERL },			// Sep. 8, 2000 genta
	{ OUTLINE_ASM,		STR_OUTLINE_ASM },
	{ OUTLINE_VB,		STR_OUTLINE_VB },			// 2001/06/23 N.Nakatani
	{ OUTLINE_PYTHON,	STR_OUTLINE_PYTHON },		// 2007.02.08 genta
	{ OUTLINE_ERLANG,	STR_OUTLINE_ERLANG },		// 2009.08.10 genta
	{ OUTLINE_WZTXT,	STR_OUTLINE_WZ },			// 2003.05.20 zenryaku, 2003.06.23 Moca ���̕ύX
	{ OUTLINE_HTML,		STR_OUTLINE_HTML },			// 2003.05.20 zenryaku
	{ OUTLINE_TEX,		STR_OUTLINE_TEX },			// 2003.07.20 naoh
	{ OUTLINE_TEXT,		STR_OUTLINE_TEXT }			// Jul. 08, 2001 JEPRO ��ɍŌ���ɂ���
};

TYPE_NAME_ID<TabArrowType> TabArrowArr[] = {
	{ TabArrowType::String,	STR_TAB_SYMBOL_CHARA },			// _T("�����w��")
	{ TabArrowType::Short,	STR_TAB_SYMBOL_SHORT_ARROW },	// _T("�Z�����")
	{ TabArrowType::Long,	STR_TAB_SYMBOL_LONG_ARROW },	// _T("�������")
};

TYPE_NAME_ID<SmartIndentType> SmartIndentArr[] = {
	{ SmartIndentType::None,	STR_SMART_INDENT_NONE },
	{ SmartIndentType::Cpp,	STR_SMART_INDENT_C_CPP },
};

/*!	2�s�ڈȍ~�̃C���f���g���@

	@sa CLayoutMgr::SetLayoutInfo()
	@date Oct. 1, 2002 genta 
*/
TYPE_NAME_ID<int> IndentTypeArr[] = {
	{ 0, STR_WRAP_INDENT_NONE },	// _T("�Ȃ�")
	{ 1, STR_WRAP_INDENT_TX2X },	// _T("tx2x")
	{ 2, STR_WRAP_INDENT_BOL },		// _T("�_���s�擪")
};

// 2008.05.30 nasukoji	�e�L�X�g�̐܂�Ԃ����@
TYPE_NAME_ID<TextWrappingMethod> WrapMethodArr[] = {
	{ TextWrappingMethod::NoWrapping,	STR_WRAP_METHOD_NO_WRAP },		// _T("�܂�Ԃ��Ȃ�")
	{ TextWrappingMethod::SettingWidth,	STR_WRAP_METHOD_SPEC_WIDTH },	// _T("�w�茅�Ő܂�Ԃ�")
	{ TextWrappingMethod::WindowWidth,	STR_WRAP_METHOD_WIN_WIDTH },	// _T("�E�[�Ő܂�Ԃ�")
};

// �ÓI�����o
std::vector<TYPE_NAME_ID2<EOutlineType>> PropTypes::m_OlmArr;	// �A�E�g���C����̓��[���z��
std::vector<TYPE_NAME_ID2<SmartIndentType>> PropTypes::m_SIndentArr;	// �X�}�[�g�C���f���g���[���z��

// �X�N���[���^�u�̏�����
void PropTypesScreen::CPropTypes_Screen()
{
	// �v���O�C�������̏ꍇ�A�����ŐÓI�����o������������B�v���O�C���L���̏ꍇ��AddXXXMethod���ŏ���������B
	if (m_OlmArr.empty()) {
		InitTypeNameId2(m_OlmArr, OlmArr, _countof(OlmArr));	// �A�E�g���C����̓��[��
	}
	if (m_SIndentArr.empty()) {
		InitTypeNameId2(m_SIndentArr, SmartIndentArr, _countof(SmartIndentArr));	// �X�}�[�g�C���f���g���[��
	}
}

// Screen ���b�Z�[�W����
INT_PTR PropTypesScreen::DispatchEvent(
	HWND		hwndDlg,	// handle to dialog box
	UINT		uMsg,		// message
	WPARAM		wParam,		// first message parameter
	LPARAM		lParam 		// second message parameter
	)
{
	WORD		wNotifyCode;
	WORD		wID;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
	int			nVal;

	switch (uMsg) {
	case WM_INITDIALOG:
		m_hwndThis = hwndDlg;
		// �_�C�A���O�f�[�^�̐ݒ� Screen
		SetData(hwndDlg);
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// �G�f�B�b�g�R���g���[���̓��͕���������
		EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_TYPENAME       ), _countof(m_types.szTypeName     ) - 1);
		EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_TYPEEXTS       ), _countof(m_types.szTypeExts     ) - 1);
		EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_INDENTCHARS    ), _countof(m_types.szIndentChars  ) - 1);
		EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_TABVIEWSTRING  ), _countof(m_types.szTabViewString) - 1);
		EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_OUTLINERULEFILE), _countof2(m_types.szOutlineRuleFilename) - 1);	//	Oct. 5, 2002 genta ��ʏ�ł����͐���

		if (m_types.nIdx == 0) {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_TYPENAME), FALSE);	// �ݒ�̖��O
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_TYPEEXTS), FALSE);	// �t�@�C���g���q
		}
		return TRUE;
		
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	// �ʒm�R�[�h
		wID			= LOWORD(wParam);	// ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID
//		hwndCtl		= (HWND) lParam;	// �R���g���[���̃n���h��
		switch (wNotifyCode) {
		case CBN_SELCHANGE:
			switch (wID) {
			case IDC_CHECK_TAB_ARROW:
				{
					// Mar. 31, 2003 genta ���\����ON/OFF��TAB������ݒ�ɘA��������
					HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_CHECK_TAB_ARROW);
					int nSelPos = Combo_GetCurSel(hwndCombo);
					if (TabArrowType::String == TabArrowArr[nSelPos].nMethod) {
						::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_TABVIEWSTRING), TRUE);
					}else {
						::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_TABVIEWSTRING), FALSE);
					}
				}
				break;
			}
			break;

		// �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ
		case BN_CLICKED:
			switch (wID) {
			/*	2002.04.01 YAZAKI �I�[�g�C���f���g���폜�i���Ƃ��ƕs�v�j
				�A�E�g���C����͂Ƀ��[���t�@�C���֘A��ǉ�
			*/
			case IDC_RADIO_OUTLINEDEFAULT:	// �A�E�g���C����́��W�����[��
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_COMBO_OUTLINES), TRUE);
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_OUTLINERULEFILE), FALSE);
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_RULEFILE_REF), FALSE);

				Combo_SetCurSel(::GetDlgItem(hwndDlg, IDC_COMBO_OUTLINES), 0);

				return TRUE;
			case IDC_RADIO_OUTLINERULEFILE:	// �A�E�g���C����́����[���t�@�C��
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_COMBO_OUTLINES), FALSE);
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_OUTLINERULEFILE), TRUE);
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_RULEFILE_REF), TRUE);
				return TRUE;

			case IDC_BUTTON_RULEFILE_REF:	// �A�E�g���C����́����[���t�@�C���́u�Q��...�v�{�^��
				{
					DlgOpenFile	dlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
					// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
					if (_IS_REL_PATH(m_types.szOutlineRuleFilename)) {
						GetInidirOrExedir(szPath, m_types.szOutlineRuleFilename);
					}else {
						_tcscpy(szPath, m_types.szOutlineRuleFilename);
					}
					// �t�@�C���I�[�v���_�C�A���O�̏�����
					dlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						_T("*.*"),
						szPath
					);
					if (dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
						_tcscpy(m_types.szOutlineRuleFilename, szPath);
						::DlgItem_SetText(hwndDlg, IDC_EDIT_OUTLINERULEFILE, m_types.szOutlineRuleFilename);
					}
				}
				return TRUE;

			case IDC_BUTTON_TYPEFONT:
				{
					LOGFONT lf;
					INT nPointSize;

					if (m_types.bUseTypeFont) {
						lf = m_types.lf;
					}else {
						lf = m_pShareData->m_common.view.lf;
					}

					bool bFixedFont = true;
					if (m_pShareData->m_common.view.lf.lfPitchAndFamily & FIXED_PITCH) {
					}else {
						bool bFixedFont = false;
					}

					if (MySelectFont(&lf, &nPointSize, hwndDlg, bFixedFont)) {
						m_types.lf = lf;
						m_types.nPointSize = nPointSize;
						m_types.bUseTypeFont = true;		// �^�C�v�ʃt�H���g�̎g�p
						::CheckDlgButton(hwndDlg, IDC_CHECK_USETYPEFONT, m_types.bUseTypeFont);
						::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_USETYPEFONT), m_types.bUseTypeFont);
						// �t�H���g�\��	// 2013/6/23 Uchi
						HFONT hFont = SetFontLabel(hwndDlg, IDC_STATIC_TYPEFONT, m_types.lf, m_types.nPointSize, m_types.bUseTypeFont);
						if (m_hTypeFont) {
							::DeleteObject(m_hTypeFont);
						}
						m_hTypeFont = hFont;
					}
				}
				return TRUE;
			case IDC_CHECK_USETYPEFONT:	// 2013/6/24 Uchi
				if (!IsDlgButtonChecked(hwndDlg, IDC_CHECK_USETYPEFONT)) {
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_USETYPEFONT), FALSE);
					// �t�H���g�\��
					HFONT hFont = SetFontLabel(hwndDlg, IDC_STATIC_TYPEFONT, m_types.lf, m_types.nPointSize, FALSE);
					if (m_hTypeFont) {
						::DeleteObject(m_hTypeFont);
					}
					m_hTypeFont = hFont;
				}
				return TRUE;
			case IDC_CHECK_KINSOKURET:		// ���s�������Ԃ牺����
			case IDC_CHECK_KINSOKUKUTO:		// ��Ǔ_���Ԃ牺����
				// �Ԃ牺�����B���̗L����	2012/11/30 Uchi
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_KINSOKUHIDE), 
					IsDlgButtonChecked(hwndDlg, IDC_CHECK_KINSOKURET) 
				 || IsDlgButtonChecked(hwndDlg, IDC_CHECK_KINSOKUKUTO));
			}
			break;	// BN_CLICKED
		}
		break;	// WM_COMMAND
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch (idCtrl) {
		case IDC_SPIN_MAXLINELEN:
			// �܂�Ԃ�����
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_MAXLINELEN, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < MINLINEKETAS) {
				nVal = MINLINEKETAS;
			}
			if (nVal > MAXLINEKETAS) {
				nVal = MAXLINEKETAS;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_MAXLINELEN, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_CHARSPACE:
			// �����̌���
//			MYTRACE(_T("IDC_SPIN_CHARSPACE\n"));
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_CHARSPACE, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 0) {
				nVal = 0;
			}
			if (nVal > COLUMNSPACE_MAX) { // Feb. 18, 2003 genta �ő�l�̒萔��
				nVal = COLUMNSPACE_MAX;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_CHARSPACE, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_LINESPACE:
			// �s�̌���
//			MYTRACE(_T("IDC_SPIN_LINESPACE\n"));
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_LINESPACE, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
//	From Here Oct. 8, 2000 JEPRO �s�Ԃ��ŏ�0�܂Őݒ�ł���悤�ɕύX(�̂ɖ߂�������?)
//			if (nVal < 1) {
//				nVal = 1;
//			}
			if (nVal < 0) {
				nVal = 0;
			}
//	To Here  Oct. 8, 2000
			if (nVal > LINESPACE_MAX) { // Feb. 18, 2003 genta �ő�l�̒萔��
				nVal = LINESPACE_MAX;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_LINESPACE, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_TABSPACE:
			//	Sep. 22, 2002 genta
			// TAB��
//			MYTRACE(_T("IDC_SPIN_CHARSPACE\n"));
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_TABSPACE, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 1) {
				nVal = 1;
			}
			if (nVal > 64) {
				nVal = 64;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_TABSPACE, nVal, FALSE);
			return TRUE;

		default:
			switch (pNMHDR->code) {
			case PSN_HELP:
				OnHelp(hwndDlg, IDD_PROP_SCREEN);
				return TRUE;
			case PSN_KILLACTIVE:
				// �_�C�A���O�f�[�^�̎擾 Screen
				GetData(hwndDlg);

				return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			case PSN_SETACTIVE:
				m_nPageNum = ID_PROPTYPE_PAGENUM_SCREEN;
				return TRUE;
			}
			break;
		}

//		MYTRACE(_T("pNMHDR->hwndFrom	=%xh\n"),	pNMHDR->hwndFrom);
//		MYTRACE(_T("pNMHDR->idFrom	=%xh\n"),	pNMHDR->idFrom);
//		MYTRACE(_T("pNMHDR->code		=%xh\n"),	pNMHDR->code);
//		MYTRACE(_T("pMNUD->iPos		=%d\n"),		pMNUD->iPos);
//		MYTRACE(_T("pMNUD->iDelta		=%d\n"),		pMNUD->iDelta);
		break;

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids1);	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		// NOTREACHED
//		break;
//@@@ 2001.02.04 End

//@@@ 2001.11.17 add start MIK
	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids1);	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//@@@ 2001.11.17 add end MIK

	case WM_DESTROY:
		// �^�C�v�t�H���g�j��	// 2013/6/23 Uchi
		if (m_hTypeFont) {
			::DeleteObject(m_hTypeFont);
			m_hTypeFont = NULL;
		}
		return TRUE;
	}
	return FALSE;
}


// �_�C�A���O�f�[�^�̐ݒ� Screen
void PropTypesScreen::SetData(HWND hwndDlg)
{
	::DlgItem_SetText(hwndDlg, IDC_EDIT_TYPENAME, m_types.szTypeName);	// �ݒ�̖��O
	::DlgItem_SetText(hwndDlg, IDC_EDIT_TYPEEXTS, m_types.szTypeExts);	// �t�@�C���g���q

	// ���C�A�E�g
	{
		// 2008.05.30 nasukoji	�e�L�X�g�̐܂�Ԃ����@
		HWND	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_WRAPMETHOD);
		Combo_ResetContent(hwndCombo);
		int		nSelPos = 0;
		for (int i=0; i<_countof(WrapMethodArr); ++i) {
			Combo_InsertString(hwndCombo, i, LS(WrapMethodArr[i].nNameId));
			if (WrapMethodArr[i].nMethod == m_types.nTextWrapMethod) {		// �e�L�X�g�̐܂�Ԃ����@
				nSelPos = i;
			}
		}
		Combo_SetCurSel(hwndCombo, nSelPos);

		::SetDlgItemInt(hwndDlg, IDC_EDIT_MAXLINELEN, (Int)m_types.nMaxLineKetas, FALSE);	// �܂�Ԃ�������
		::SetDlgItemInt(hwndDlg, IDC_EDIT_CHARSPACE, m_types.nColumnSpace, FALSE);			// �����̊Ԋu
		::SetDlgItemInt(hwndDlg, IDC_EDIT_LINESPACE, m_types.nLineSpace, FALSE);			// �s�̊Ԋu
		::SetDlgItemInt(hwndDlg, IDC_EDIT_TABSPACE, (Int)m_types.nTabSpace, FALSE);			// TAB��	//	Sep. 22, 2002 genta
		::DlgItem_SetText(hwndDlg, IDC_EDIT_TABVIEWSTRING, m_types.szTabViewString);		// TAB�\��(8����)
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_TABVIEWSTRING), m_types.bTabArrow == TabArrowType::String);	// Mar. 31, 2003 genta ���\����ON/OFF��TAB������ݒ�ɘA��������

		// ���\��	//@@@ 2003.03.26 MIK
		hwndCombo = ::GetDlgItem(hwndDlg, IDC_CHECK_TAB_ARROW);
		Combo_ResetContent(hwndCombo);
		nSelPos = 0;
		for (int i=0; i<_countof(TabArrowArr); ++i) {
			Combo_InsertString(hwndCombo, i, LS(TabArrowArr[i].nNameId));
			if (TabArrowArr[i].nMethod == m_types.bTabArrow) {
				nSelPos = i;
			}
		}
		Combo_SetCurSel(hwndCombo, nSelPos);

		::CheckDlgButtonBool(hwndDlg, IDC_CHECK_INS_SPACE, m_types.bInsSpace);				// SPACE�̑}�� [�`�F�b�N�{�b�N�X]	// From Here 2001.12.03 hor
	}

	// �C���f���g
	{
		// �����C���f���g
		::CheckDlgButtonBool(hwndDlg, IDC_CHECK_INDENT, m_types.bAutoIndent);

		// ���{��󔒂��C���f���g
		::CheckDlgButtonBool(hwndDlg, IDC_CHECK_INDENT_WSPACE, m_types.bAutoIndent_ZENSPACE);

		// �X�}�[�g�C���f���g���
		HWND	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_SMARTINDENT);
		Combo_ResetContent(hwndCombo);
		int		nSelPos = 0;
		int nSize = (int)m_SIndentArr.size();
		for (int i=0; i<nSize; ++i) {
			if (!m_SIndentArr[i].pszName) {
				Combo_InsertString(hwndCombo, i, LS(m_SIndentArr[i].nNameId));
			}else {
				Combo_InsertString(hwndCombo, i, m_SIndentArr[i].pszName);
			}
			if (m_SIndentArr[i].nMethod == m_types.eSmartIndent) {	// �X�}�[�g�C���f���g���
				nSelPos = i;
			}
		}
		Combo_SetCurSel(hwndCombo, nSelPos);

		// ���̑��̃C���f���g�Ώە���
		::DlgItem_SetText(hwndDlg, IDC_EDIT_INDENTCHARS, m_types.szIndentChars);

		// �܂�Ԃ��s�C���f���g	//	Oct. 1, 2002 genta �R���{�{�b�N�X�ɕύX
		hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_INDENTLAYOUT);
		Combo_ResetContent(hwndCombo);
		nSelPos = 0;
		for (int i=0; i<_countof(IndentTypeArr); ++i) {
			Combo_InsertString(hwndCombo, i, LS(IndentTypeArr[i].nNameId));
			if (IndentTypeArr[i].nMethod == m_types.nIndentLayout) {	// �܂�Ԃ��C���f���g���
				nSelPos = i;
			}
		}
		Combo_SetCurSel(hwndCombo, nSelPos);

		// ���s���ɖ����̋󔒂��폜	// 2005.10.11 ryoji
		::CheckDlgButton(hwndDlg, IDC_CHECK_RTRIM_PREVLINE, m_types.bRTrimPrevLine);
	}

	// �A�E�g���C����͕��@
	// 2002.04.01 YAZAKI ���[���t�@�C���֘A�ǉ�
	{
		// �W�����[���̃R���{�{�b�N�X������
		HWND	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_OUTLINES);
		Combo_ResetContent(hwndCombo);
		int		nSelPos = 0;
		int nSize = (int)m_OlmArr.size();
		for (int i=0; i<nSize; ++i) {
			if (!m_OlmArr[i].pszName) {
				Combo_InsertString(hwndCombo, i, LS(m_OlmArr[i].nNameId));
			}else {
				Combo_InsertString(hwndCombo, i, m_OlmArr[i].pszName);
			}
			if (m_OlmArr[i].nMethod == m_types.eDefaultOutline) {	// �A�E�g���C����͕��@
				nSelPos = i;
			}
		}

		// ���[���t�@�C��	// 2003.06.23 Moca ���[���t�@�C�����͎g��Ȃ��Ă��Z�b�g���Ă���
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_OUTLINERULEFILE), TRUE);
		::DlgItem_SetText(hwndDlg, IDC_EDIT_OUTLINERULEFILE, m_types.szOutlineRuleFilename);

		// �W�����[��
		if (m_types.eDefaultOutline != OUTLINE_FILE) {
			::CheckDlgButton(hwndDlg, IDC_RADIO_OUTLINEDEFAULT, TRUE);
			::CheckDlgButton(hwndDlg, IDC_RADIO_OUTLINERULEFILE, FALSE);

			::EnableWindow(::GetDlgItem(hwndDlg, IDC_COMBO_OUTLINES), TRUE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_OUTLINERULEFILE), FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_RULEFILE_REF), FALSE);

			Combo_SetCurSel(hwndCombo, nSelPos);
		// ���[���t�@�C��
		}else {
			::CheckDlgButton(hwndDlg, IDC_RADIO_OUTLINEDEFAULT, FALSE);
			::CheckDlgButton(hwndDlg, IDC_RADIO_OUTLINERULEFILE, TRUE);

			::EnableWindow(::GetDlgItem(hwndDlg, IDC_COMBO_OUTLINES), FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_RULEFILE_REF), TRUE);
		}
	}

	// �t�H���g
	{
		::CheckDlgButton(hwndDlg, IDC_CHECK_USETYPEFONT, m_types.bUseTypeFont);			// �^�C�v�ʃt�H���g�̎g�p
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_USETYPEFONT), m_types.bUseTypeFont);
		m_hTypeFont = SetFontLabel(hwndDlg, IDC_STATIC_TYPEFONT, m_types.lf, m_types.nPointSize, m_types.bUseTypeFont);
	}

	// ���̑�
	{
		// �p�����[�h���b�v������
		::CheckDlgButtonBool(hwndDlg, IDC_CHECK_WORDWRAP, m_types.bWordWrap);

		// �֑�����
		{	//@@@ 2002.04.08 MIK start
			::CheckDlgButtonBool(hwndDlg, IDC_CHECK_KINSOKUHEAD, m_types.bKinsokuHead);
			::CheckDlgButtonBool(hwndDlg, IDC_CHECK_KINSOKUTAIL, m_types.bKinsokuTail);
			::CheckDlgButtonBool(hwndDlg, IDC_CHECK_KINSOKURET,  m_types.bKinsokuRet );	// ���s�������Ԃ牺����	//@@@ 2002.04.13 MIK
			::CheckDlgButtonBool(hwndDlg, IDC_CHECK_KINSOKUKUTO, m_types.bKinsokuKuto);	// ��Ǔ_���Ԃ牺����	//@@@ 2002.04.17 MIK
			::CheckDlgButtonBool(hwndDlg, IDC_CHECK_KINSOKUHIDE, m_types.bKinsokuHide);	// �Ԃ牺�����B��			// 2011/11/30 Uchi
			EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_KINSOKUHEAD), _countof(m_types.szKinsokuHead) - 1);
			EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_KINSOKUTAIL), _countof(m_types.szKinsokuTail) - 1);
			EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_KINSOKUKUTO), _countof(m_types.szKinsokuKuto) - 1);	// 2009.08.07 ryoji
			::DlgItem_SetText(hwndDlg, IDC_EDIT_KINSOKUHEAD, m_types.szKinsokuHead);
			::DlgItem_SetText(hwndDlg, IDC_EDIT_KINSOKUTAIL, m_types.szKinsokuTail);
			::DlgItem_SetText(hwndDlg, IDC_EDIT_KINSOKUKUTO, m_types.szKinsokuKuto);	// 2009.08.07 ryoji
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_KINSOKUHIDE), (m_types.bKinsokuRet || m_types.bKinsokuKuto) ? TRUE : FALSE);	// �Ԃ牺�����B���̗L����	2012/11/30 Uchi
		}	//@@@ 2002.04.08 MIK end
	}
}


// �_�C�A���O�f�[�^�̎擾 Screen
int PropTypesScreen::GetData(HWND hwndDlg)
{
	::DlgItem_GetText(hwndDlg, IDC_EDIT_TYPENAME, m_types.szTypeName, _countof(m_types.szTypeName));	// �ݒ�̖��O
	::DlgItem_GetText(hwndDlg, IDC_EDIT_TYPEEXTS, m_types.szTypeExts, _countof(m_types.szTypeExts));	// �t�@�C���g���q

	// ���C�A�E�g
	{
		// 2008.05.30 nasukoji	�e�L�X�g�̐܂�Ԃ����@
		HWND	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_WRAPMETHOD);
		int		nSelPos = Combo_GetCurSel(hwndCombo);
		m_types.nTextWrapMethod = WrapMethodArr[nSelPos].nMethod;		// �e�L�X�g�̐܂�Ԃ����@

		// �܂�Ԃ�����
		m_types.nMaxLineKetas = LayoutInt(::GetDlgItemInt(hwndDlg, IDC_EDIT_MAXLINELEN, NULL, FALSE));
		if (m_types.nMaxLineKetas < LayoutInt(MINLINEKETAS)) {
			m_types.nMaxLineKetas = LayoutInt(MINLINEKETAS);
		}
		if (m_types.nMaxLineKetas > LayoutInt(MAXLINEKETAS)) {
			m_types.nMaxLineKetas = LayoutInt(MAXLINEKETAS);
		}

		// �����̊Ԋu
		m_types.nColumnSpace = ::GetDlgItemInt(hwndDlg, IDC_EDIT_CHARSPACE, NULL, FALSE);
		if (m_types.nColumnSpace < 0) {
			m_types.nColumnSpace = 0;
		}
		if (m_types.nColumnSpace > COLUMNSPACE_MAX) { // Feb. 18, 2003 genta �ő�l�̒萔��
			m_types.nColumnSpace = COLUMNSPACE_MAX;
		}

		// �s�̊Ԋu
		m_types.nLineSpace = ::GetDlgItemInt(hwndDlg, IDC_EDIT_LINESPACE, NULL, FALSE);
		if (m_types.nLineSpace < 0) {
			m_types.nLineSpace = 0;
		}
		if (m_types.nLineSpace > LINESPACE_MAX) {	// Feb. 18, 2003 genta �ő�l�̒萔��
			m_types.nLineSpace = LINESPACE_MAX;
		}

		// TAB��
		m_types.nTabSpace = LayoutInt(::GetDlgItemInt(hwndDlg, IDC_EDIT_TABSPACE, NULL, FALSE));
		if (m_types.nTabSpace < LayoutInt(1)) {
			m_types.nTabSpace = LayoutInt(1);
		}
		if (m_types.nTabSpace > LayoutInt(64)) {
			m_types.nTabSpace = LayoutInt(64);
		}

		// TAB�\��������
		WIN_CHAR szTab[8 + 1]; // +1. happy
		::DlgItem_GetText(hwndDlg, IDC_EDIT_TABVIEWSTRING, szTab, _countof(szTab));
		wcscpy_s(m_types.szTabViewString, L"^       ");
		for (int i=0; i<8; ++i) {
			if (!TCODE::IsTabAvailableCode(szTab[i])) {
				break;
			}
			m_types.szTabViewString[i] = szTab[i];
		}

		// �^�u���\��	//@@@ 2003.03.26 MIK
		hwndCombo = ::GetDlgItem(hwndDlg, IDC_CHECK_TAB_ARROW);
		nSelPos = Combo_GetCurSel(hwndCombo);
		m_types.bTabArrow = TabArrowArr[nSelPos].nMethod;		// �e�L�X�g�̐܂�Ԃ����@

		// SPACE�̑}��
		m_types.bInsSpace = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_INS_SPACE);
	}

	// �C���f���g
	{
		// �����C���f���g
		m_types.bAutoIndent = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_INDENT);

		// ���{��󔒂��C���f���g
		m_types.bAutoIndent_ZENSPACE = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_INDENT_WSPACE);

		// �X�}�[�g�C���f���g���
		HWND	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_SMARTINDENT);
		int		nSelPos = Combo_GetCurSel(hwndCombo);
		if (nSelPos >= 0) {
			m_types.eSmartIndent = m_SIndentArr[nSelPos].nMethod;	// �X�}�[�g�C���f���g���
		}

		// ���̑��̃C���f���g�Ώە���
		::DlgItem_GetText(hwndDlg, IDC_EDIT_INDENTCHARS, m_types.szIndentChars, _countof(m_types.szIndentChars));

		// �܂�Ԃ��s�C���f���g	//	Oct. 1, 2002 genta �R���{�{�b�N�X�ɕύX
		hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_INDENTLAYOUT);
		nSelPos = Combo_GetCurSel(hwndCombo);
		m_types.nIndentLayout = IndentTypeArr[nSelPos].nMethod;	// �܂�Ԃ����C���f���g���

		// ���s���ɖ����̋󔒂��폜	// 2005.10.11 ryoji
		m_types.bRTrimPrevLine = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_RTRIM_PREVLINE);
	}

	// �A�E�g���C����͕��@
	// 2002.04.01 YAZAKI ���[���t�@�C���֘A�ǉ�
	{
		// �W�����[��
		if (!IsDlgButtonChecked(hwndDlg, IDC_RADIO_OUTLINERULEFILE)) {
			HWND	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_OUTLINES);
			int		nSelPos = Combo_GetCurSel(hwndCombo);
			if (nSelPos >= 0) {
				m_types.eDefaultOutline = m_OlmArr[nSelPos].nMethod;	// �A�E�g���C����͕��@
			}
		// ���[���t�@�C��
		}else {
			m_types.eDefaultOutline = OUTLINE_FILE;
		}

		// ���[���t�@�C��	// 2003.06.23 Moca ���[�����g���Ă��Ȃ��Ă��t�@�C������ێ�
		::DlgItem_GetText(hwndDlg, IDC_EDIT_OUTLINERULEFILE, m_types.szOutlineRuleFilename, _countof2(m_types.szOutlineRuleFilename));
	}

	// �t�H���g
	{
		LOGFONT lf;
		m_types.bUseTypeFont = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_USETYPEFONT);		// �^�C�v�ʃt�H���g�̎g�p
		if (m_types.bUseTypeFont) {
			lf = m_types.lf;
		}else {
			lf = m_pShareData->m_common.view.lf;
		}
	}

	// ���̑�
	{
		// �p�����[�h���b�v������
		m_types.bWordWrap = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_WORDWRAP);

		// �֑�����
		{	//@@@ 2002.04.08 MIK start
			m_types.bKinsokuHead = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_KINSOKUHEAD);
			m_types.bKinsokuTail = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_KINSOKUTAIL);
			m_types.bKinsokuRet  = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_KINSOKURET );	// ���s�������Ԃ牺����	//@@@ 2002.04.13 MIK
			m_types.bKinsokuKuto = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_KINSOKUKUTO);	// ��Ǔ_���Ԃ牺����	//@@@ 2002.04.17 MIK
			m_types.bKinsokuHide = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_KINSOKUHIDE);	// �Ԃ牺�����B��		// 2011/11/30 Uchi
			::DlgItem_GetText(hwndDlg, IDC_EDIT_KINSOKUHEAD, m_types.szKinsokuHead, _countof(m_types.szKinsokuHead));
			::DlgItem_GetText(hwndDlg, IDC_EDIT_KINSOKUTAIL, m_types.szKinsokuTail, _countof(m_types.szKinsokuTail));
			::DlgItem_GetText(hwndDlg, IDC_EDIT_KINSOKUKUTO, m_types.szKinsokuKuto, _countof(m_types.szKinsokuKuto));	// 2009.08.07 ryoji
		}	//@@@ 2002.04.08 MIK end

	}

	return TRUE;
}

// �A�E�g���C����̓��[���̒ǉ�
void PropTypesScreen::AddOutlineMethod(int nMethod, const WCHAR* szName)
{
	if (m_OlmArr.empty()) {
		InitTypeNameId2(m_OlmArr, OlmArr, _countof(OlmArr));	// �A�E�g���C����̓��[��
	}
	TYPE_NAME_ID2<EOutlineType> method;
	method.nMethod = (EOutlineType)nMethod;
	method.nNameId = 0;
	const TCHAR* tszName = to_tchar(szName);
	TCHAR* pszName = new TCHAR[_tcslen(tszName) + 1];
	_tcscpy(pszName, tszName);
	method.pszName = pszName;
	m_OlmArr.push_back(method);
}

void PropTypesScreen::RemoveOutlineMethod(int nMethod, const WCHAR* szName)
{
	int nSize = (int)m_OlmArr.size();
	for (int i=0; i<nSize; ++i) {
		if (m_OlmArr[i].nMethod == (EOutlineType)nMethod) {
			delete [] m_OlmArr[i].pszName;
			m_OlmArr.erase(m_OlmArr.begin() + i);
			break;
		}
	}
}

// �X�}�[�g�C���f���g���[���̒ǉ�
void PropTypesScreen::AddSIndentMethod(SmartIndentType nMethod, const WCHAR* szName)
{
	if (m_SIndentArr.empty()) {
		InitTypeNameId2(m_SIndentArr, SmartIndentArr, _countof(SmartIndentArr));	// �X�}�[�g�C���f���g���[��
	}
	TYPE_NAME_ID2<SmartIndentType> method;
	method.nMethod = (SmartIndentType)nMethod;
	method.nNameId = 0;
	const TCHAR* tszName = to_tchar(szName);
	TCHAR* pszName = new TCHAR[_tcslen(tszName) + 1];
	_tcscpy(pszName, tszName);
	method.pszName = pszName;
	m_SIndentArr.push_back(method);
}

void PropTypesScreen::RemoveSIndentMethod(SmartIndentType nMethod, const WCHAR* szName)
{
	int nSize = (int)m_SIndentArr.size();
	for (int i=0; i<nSize; ++i) {
		if (m_SIndentArr[i].nMethod == (SmartIndentType)nMethod) {
			delete [] m_SIndentArr[i].pszName;
			m_SIndentArr.erase(m_SIndentArr.begin() + i);
			break;
		}
	}
}

