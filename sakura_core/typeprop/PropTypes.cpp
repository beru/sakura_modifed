/*!	@file
	@brief �^�C�v�ʐݒ�_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date 1998/12/24  �V�K�쐬
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
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "PropTypes.h"
#include "EditApp.h"
#include "view/colors/EColorIndexType.h"
#include "util/shell.h"
#include "sakura_rc.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      ���b�Z�[�W����                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

typedef INT_PTR (PropTypes::*DISPATCH_EVENT_TYPE)(HWND, UINT, WPARAM, LPARAM);

// ���ʃ_�C�A���O�v���V�[�W��
INT_PTR CALLBACK PropTypesCommonProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam, DISPATCH_EVENT_TYPE pDispatch)
{
	PROPSHEETPAGE* pPsp;
	PropTypes* pPropTypes;
	switch (uMsg) {
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pPropTypes = reinterpret_cast<PropTypes*>(pPsp->lParam);
		if (pPropTypes) {
			return (pPropTypes->*pDispatch)(hwndDlg, uMsg, wParam, pPsp->lParam);
		}else {
			return FALSE;
		}
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pPropTypes = (PropTypes*)::GetWindowLongPtr(hwndDlg, DWLP_USER);
		if (pPropTypes) {
			return (pPropTypes->*pDispatch)(hwndDlg, uMsg, wParam, lParam);
		}else {
			return FALSE;
		}
	}
}

// �e��_�C�A���O�v���V�[�W��
typedef	INT_PTR (PropTypes::*pDispatchPage)(HWND, UINT, WPARAM, LPARAM);
#define GEN_PROPTYPES_CALLBACK(FUNC, CLASS) \
INT_PTR CALLBACK FUNC(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) \
{ \
	return PropTypesCommonProc(hwndDlg, uMsg, wParam, lParam, reinterpret_cast<pDispatchPage>(&CLASS::DispatchEvent)); \
}
GEN_PROPTYPES_CALLBACK(PropTypesScreenDlgProc,		PropTypesScreen)
GEN_PROPTYPES_CALLBACK(PropTypesWindowDlgProc,		PropTypesWindow)
GEN_PROPTYPES_CALLBACK(PropTypesColorDlgProc,		PropTypesColor)
GEN_PROPTYPES_CALLBACK(PropTypesSupportDlgProc,		PropTypesSupport)
GEN_PROPTYPES_CALLBACK(PropTypesRegexDlgProc,		PropTypesRegex)
GEN_PROPTYPES_CALLBACK(PropTypesKeyHelpDlgProc,		PropTypesKeyHelp)



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �����Ɣj��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

PropTypes::PropTypes()
{
	{
		assert(sizeof(PropTypesScreen)  - sizeof(PropTypes) == 0);
		assert(sizeof(PropTypesWindow)  - sizeof(PropTypes) == 0);
		assert(sizeof(PropTypesColor)   - sizeof(PropTypes) == 0);
		assert(sizeof(PropTypesSupport) - sizeof(PropTypes) == 0);
		assert(sizeof(PropTypesRegex)   - sizeof(PropTypes) == 0);
		assert(sizeof(PropTypesKeyHelp) - sizeof(PropTypes) == 0);
	}

	// ���L�f�[�^�\���̂̃A�h���X��Ԃ�
	pShareData = &GetDllShareData();

	// Mar. 31, 2003 genta �������팸�̂��߃|�C���^�ɕύX
	pKeywordSetMgr = &pShareData->common.specialKeyword.keywordSetMgr;

	hInstance = NULL;		// �A�v���P�[�V�����C���X�^���X�̃n���h��
	hwndParent = NULL;	// �I�[�i�[�E�B���h�E�̃n���h��
	hwndThis  = NULL;		// ���̃_�C�A���O�̃n���h��
	nPageNum = ID_PROPTYPE_PAGENUM_SCREEN;

	(static_cast<PropTypesScreen*>(this))->CPropTypes_Screen();
}

PropTypes::~PropTypes()
{
}

// ������
void PropTypes::Create(HINSTANCE hInstApp, HWND hwndParent)
{
	this->hInstance = hInstApp;		// �A�v���P�[�V�����C���X�^���X�̃n���h��
	this->hwndParent = hwndParent;	// �I�[�i�[�E�B���h�E�̃n���h��
}

struct TypePropSheetInfo {
	int nTabNameId;											// TAB�̕\����
	unsigned int resId;										// Property sheet�ɑΉ�����Dialog resource
	INT_PTR (CALLBACK *DProc)(HWND, UINT, WPARAM, LPARAM);	// Dialog Procedure
};

// �L�[���[�h�F�^�C�v�ʐݒ�^�u����(�v���p�e�B�V�[�g)
// �v���p�e�B�V�[�g�̍쐬
INT_PTR PropTypes::DoPropertySheet(int nPageNum)
{
	INT_PTR	nRet;
	int		nIdx;

	// 2001/06/14 Start by asa-o: �^�C�v�ʐݒ�Ɏx���^�u�ǉ�
	// 2001.11.17 add start MIK �^�C�v�ʐݒ�ɐ��K�\���L�[���[�h�^�u�ǉ�
	// 2006.04.10 fon ADD-start �^�C�v�ʐݒ�Ɂu�L�[���[�h�w���v�v�^�u��ǉ�
	// 2013.03.10 aroka ADD-start �^�C�v�ʐݒ�Ɂu�E�B���h�E�v�^�u��ǉ�
	static const TypePropSheetInfo TypePropSheetInfoList[] = {
		{ STR_PROPTYPE_SCREEN,			IDD_PROP_SCREEN,	PropTypesScreenDlgProc },
		{ STR_PROPTYPE_COLOR,			IDD_PROP_COLOR,		PropTypesColorDlgProc },
		{ STR_PROPTYPE_WINDOW,			IDD_PROP_WINDOW,	PropTypesWindowDlgProc },
		{ STR_PROPTYPE_SUPPORT,			IDD_PROP_SUPPORT,	PropTypesSupportDlgProc },
		{ STR_PROPTYPE_REGEX_KEYWORD,	IDD_PROP_REGEX,		PropTypesRegexDlgProc },
		{ STR_PROPTYPE_KEYWORD_HELP,	IDD_PROP_KEYHELP,	PropTypesKeyHelpDlgProc }
	};

	// �J�X�^���F�����L����������擾
	memcpy_raw( dwCustColors, pShareData->dwCustColors, sizeof(dwCustColors) );
	// 2005.11.30 Moca �J�X�^���F�̐擪�Ƀe�L�X�g�F��ݒ肵�Ă���
	dwCustColors[0] = types.colorInfoArr[COLORIDX_TEXT].colorAttr.cTEXT;
	dwCustColors[1] = types.colorInfoArr[COLORIDX_TEXT].colorAttr.cBACK;

	std::tstring sTabname[_countof(TypePropSheetInfoList)];
	bChangeKeywordSet = false;
	PROPSHEETPAGE psp[_countof(TypePropSheetInfoList)];
	for (nIdx=0; nIdx<_countof(TypePropSheetInfoList); ++nIdx) {
		sTabname[nIdx] = LS(TypePropSheetInfoList[nIdx].nTabNameId);

		PROPSHEETPAGE* p = &psp[nIdx];
		memset_raw(p, 0, sizeof_raw(*p));
		p->dwSize      = sizeof_raw(*p);
		p->dwFlags     = PSP_USETITLE | PSP_HASHELP;
		p->hInstance   = SelectLang::getLangRsrcInstance();
		p->pszTemplate = MAKEINTRESOURCE(TypePropSheetInfoList[nIdx].resId);
		p->pszIcon     = NULL;
		p->pfnDlgProc  = TypePropSheetInfoList[nIdx].DProc;
		p->pszTitle    = sTabname[nIdx].c_str();
		p->lParam      = (LPARAM)this;
		p->pfnCallback = nullptr;
	}

	PROPSHEETHEADER	psh = {0};

	//	Jun. 29, 2002 ������
	//	Windows 95�΍�DProperty Sheet�̃T�C�Y��Windows95���F���ł��镨�ɌŒ肷��D
	psh.dwSize = sizeof_old_PROPSHEETHEADER;

	// JEPROtest Sept. 30, 2000 �^�C�v�ʐݒ�̉B��[�K�p]�{�^���̐��̂͂����B�s���̃R�����g�A�E�g�����ւ��Ă݂�΂킩��
	psh.dwFlags    = /*PSH_USEICONID |*/ PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE/* | PSH_HASHELP*/ | PSH_USEPAGELANG;
	psh.hwndParent = hwndParent;
	psh.hInstance  = SelectLang::getLangRsrcInstance();
	psh.pszIcon    = NULL;
	psh.pszCaption = LS(STR_PROPTYPE);	// _T("�^�C�v�ʐݒ�");	// Sept. 8, 2000 jepro �P�Ȃ�u�ݒ�v����ύX
	psh.nPages     = nIdx;

	//- 20020106 aroka # psh.nStartPage �� unsigned �Ȃ̂ŕ��ɂȂ�Ȃ�
	if (nPageNum == -1) {
		psh.nStartPage = nPageNum;
	}else if (0 > nPageNum) {			//- 20020106 aroka
		psh.nStartPage = 0;
	}else {
		psh.nStartPage = nPageNum;
	}
	
	if (psh.nPages - 1 < psh.nStartPage) {
		psh.nStartPage = psh.nPages - 1;
	}
	psh.ppsp = psp;
	psh.pfnCallback = nullptr;

	nRet = MyPropertySheet(&psh);	// 2007.05.24 ryoji �Ǝ��g���v���p�e�B�V�[�g

	if (nRet == -1) {
		TCHAR* pszMsgBuf;
		::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			::GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // �f�t�H���g����
			(LPTSTR)&pszMsgBuf,
			0,
			NULL
		);
		PleaseReportToAuthor(
			NULL,
			LS(STR_PROPTYPE_ERR),
			psh.nStartPage,
			pszMsgBuf
		);
		::LocalFree(pszMsgBuf);
	}

	// �J�X�^���F�����L�������ɐݒ�
	memcpy_raw( pShareData->dwCustColors, dwCustColors, sizeof(dwCustColors) );

	return nRet;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �C�x���g                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �w���v
// 2001.05.18 Stonee �@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
// 2001.07.03 JEPRO  �x���^�u�̃w���v��L����
// 2001.11.17 MIK    IDD_PROP_REGEX
void PropTypes::OnHelp(HWND hwndParent, int nPageID)
{
	int nContextID;
	switch (nPageID) {
	case IDD_PROP_SCREEN:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_SCREEN);			break;
	case IDD_PROP_COLOR:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_COLOR);			break;
	case IDD_PROP_WINDOW:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_WINDOW);			break;
	case IDD_PROP_SUPPORT:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_HELPER);			break;
	case IDD_PROP_REGEX:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_REGEX_KEYWORD);	break;
	case IDD_PROP_KEYHELP:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_KEYHELP);			break;
	default:				nContextID = -1;												break;
	}
	if (nContextID != -1) {
		MyWinHelp(hwndParent, HELP_CONTEXT, nContextID);	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
	}
}


/*!	�R���g���[���Ƀt�H���g�ݒ肷��
	@date 2013.04.24 Uchi
*/
HFONT PropTypes::SetCtrlFont(HWND hwndDlg, int idc_ctrl, const LOGFONT& lf)
{

	// �_���t�H���g���쐬
	HWND hCtrl = ::GetDlgItem(hwndDlg, idc_ctrl);
	HFONT hFont = ::CreateFontIndirect(&lf);
	if (hFont) {
		// �t�H���g�̐ݒ�
		::SendMessage(hCtrl, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
	}

	return hFont;
}


/*!	�t�H���g���x���Ƀt�H���g�ƃt�H���g���ݒ肷��
	@date 2013.04.24 Uchi
*/
HFONT PropTypes::SetFontLabel(HWND hwndDlg, int idc_static, const LOGFONT& lf, int nps, bool bUse)
{
	HFONT	hFont;

	if (bUse) {
		LOGFONT lfTemp = lf;
		// �傫������t�H���g�͏������\��
		if (lfTemp.lfHeight < -16) {
			lfTemp.lfHeight = -16;
		}
		hFont = SetCtrlFont(hwndDlg, idc_static, lfTemp);
		// �t�H���g���̐ݒ�
		TCHAR szFontName[80];
		auto_sprintf_s(szFontName, nps % 10 ? _T("%s(%.1fpt)") : _T("%s(%.0fpt)"),
			lf.lfFaceName, double(nps)/10);
		::DlgItem_SetText(hwndDlg, idc_static, szFontName);
	}else {
		hFont = NULL;
		::DlgItem_SetText(hwndDlg, idc_static, _T(""));
	}

	return hFont;
}

