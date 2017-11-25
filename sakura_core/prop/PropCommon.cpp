/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�S�ʁv�y�[�W
*/
#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "env/DocTypeManager.h"
#include "EditApp.h"
#include "util/shell.h"
#include "sakura_rc.h"

int	PropCommon::SearchIntArr(int nKey, int* pnArr, int nArrNum)
{
	for (int i=0; i<nArrNum; ++i) {
		if (nKey == pnArr[i]) {
			return i;
		}
	}
	return -1;
}


/*!
	�v���p�e�B�y�[�W���Ƃ�Window Procedure�������Ɏ�邱�Ƃ�
	�����̋��ʉ���_�����D

	@param DispatchPage �^��Window Procedure�̃����o�֐��|�C���^
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handlw
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR PropCommon::DlgProc(
	INT_PTR (PropCommon::*DispatchPage)(HWND, UINT, WPARAM, LPARAM),
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam
)
{
	PROPSHEETPAGE* pPsp;
	PropCommon*	pPropCommon;
	switch (uMsg) {
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pPropCommon = (PropCommon*)(pPsp->lParam);
		if (pPropCommon) {
			return (pPropCommon->*DispatchPage)(hwndDlg, uMsg, wParam, pPsp->lParam);
		}else {
			return FALSE;
		}
	default:
		pPropCommon = (PropCommon*)::GetWindowLongPtr(hwndDlg, DWLP_USER);
		if (pPropCommon) {
			return (pPropCommon->*DispatchPage)(hwndDlg, uMsg, wParam, lParam);
		}else {
			return FALSE;
		}
	}
}

// �Ɨ��E�B���h�E�p
INT_PTR PropCommon::DlgProc2(
	INT_PTR (PropCommon::*DispatchPage)(HWND, UINT, WPARAM, LPARAM),
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam
)
{
	PropCommon*	pPropCommon;
	switch (uMsg) {
	case WM_INITDIALOG:
		pPropCommon = (PropCommon*)(lParam);
		if (pPropCommon) {
			return (pPropCommon->*DispatchPage)(hwndDlg, uMsg, IDOK, lParam);
		}else {
			return FALSE;
		}
	default:
		pPropCommon = (PropCommon*)::GetWindowLongPtr(hwndDlg, DWLP_USER);
		if (pPropCommon) {
			return (pPropCommon->*DispatchPage)(hwndDlg, uMsg, wParam, lParam);
		}else {
			return FALSE;
		}
	}
}

PropCommon::PropCommon()
{
	{
		assert(sizeof(PropGeneral)   - sizeof(PropCommon) == 0);
		assert(sizeof(PropWin)       - sizeof(PropCommon) == 0);
		assert(sizeof(PropMainMenu)  - sizeof(PropCommon) == 0);
		assert(sizeof(PropToolbar)   - sizeof(PropCommon) == 0);
		assert(sizeof(PropTab)       - sizeof(PropCommon) == 0);
		assert(sizeof(PropStatusbar) - sizeof(PropCommon) == 0);
		assert(sizeof(PropEdit)      - sizeof(PropCommon) == 0);
		assert(sizeof(PropFile)      - sizeof(PropCommon) == 0);
		assert(sizeof(PropFileName)  - sizeof(PropCommon) == 0);
		assert(sizeof(PropBackup)    - sizeof(PropCommon) == 0);
		assert(sizeof(PropFormat)    - sizeof(PropCommon) == 0);
		assert(sizeof(PropGrep)      - sizeof(PropCommon) == 0);
		assert(sizeof(PropKeybind)   - sizeof(PropCommon) == 0);
		assert(sizeof(PropCustmenu)  - sizeof(PropCommon) == 0);
		assert(sizeof(PropKeyword)   - sizeof(PropCommon) == 0);
		assert(sizeof(PropHelper)    - sizeof(PropCommon) == 0);
		assert(sizeof(PropMacro)     - sizeof(PropCommon) == 0);
		assert(sizeof(PropPlugin)    - sizeof(PropCommon) == 0);
	}

	// ���L�f�[�^�\���̂̃A�h���X��Ԃ�
	pShareData = &GetDllShareData();

	hwndParent = NULL;	// �I�[�i�[�E�B���h�E�̃n���h��
	hwndThis  = NULL;		// ���̃_�C�A���O�̃n���h��
	nPageNum = ID_PROPCOM_PAGENUM_GENERAL;
	nKeywordSet1 = -1;

	return;
}


PropCommon::~PropCommon()
{
}


// ������
void PropCommon::Create(HWND hwndParent, ImageListMgr* pIcons, MenuDrawer* pMenuDrawer)
{
	this->hwndParent = hwndParent;	// �I�[�i�[�E�B���h�E�̃n���h��
	this->pIcons = pIcons;

	// �}�N���ݒ��ύX�������ƁA��ʂ���Ȃ��ŃJ�X�^�����j���[�A�c�[���o�[�A
	// �L�[���蓖�Ẳ�ʂɐ؂�ւ������Ɋe��ʂŃ}�N���ݒ�̕ύX�����f�����悤�A
	// common.macro.macroTable�i���[�J�������o�j��lookup������������
	lookup.Init(common.macro.macroTable, &common);	//	�@�\���E�ԍ�resolve�N���X�D

	this->pMenuDrawer = pMenuDrawer;

	return;
}


/*!
	�u���ʐݒ�v�v���p�e�B�V�[�g�̍쐬���ɕK�v�ȏ���
	�ێ�����\����
*/
struct ComPropSheetInfo {
	int nTabNameId;											// TAB�̕\����
	unsigned int resId;										// Property sheet�ɑΉ�����Dialog resource
	INT_PTR (CALLBACK *DProc)(HWND, UINT, WPARAM, LPARAM);	// Dialog Procedure
};

//	�L�[���[�h�F���ʐݒ�^�u����(�v���p�e�B�V�[�g)
/*! �v���p�e�B�V�[�g�̍쐬 */
INT_PTR PropCommon::DoPropertySheet(int nPageNum, bool bTrayProc)
{
	INT_PTR	nRet;
	size_t nIdx;

	this->bTrayProc = bTrayProc;

	// �u���ʐݒ�v�v���p�e�B�V�[�g�̍쐬���ɕK�v�ȏ��̔z��D
	static const ComPropSheetInfo ComPropSheetInfoList[] = {
		{ STR_PROPCOMMON_GENERAL,	IDD_PROP_GENERAL,	PropGeneral::DlgProc_page },
		{ STR_PROPCOMMON_WINDOW,	IDD_PROP_WIN,		PropWin::DlgProc_page },
		{ STR_PROPCOMMON_MAINMENU,	IDD_PROP_MAINMENU,	PropMainMenu::DlgProc_page },
		{ STR_PROPCOMMON_TOOLBAR,	IDD_PROP_TOOLBAR,	PropToolbar::DlgProc_page },
		{ STR_PROPCOMMON_TABS,		IDD_PROP_TAB,		PropTab::DlgProc_page },
		{ STR_PROPCOMMON_STATBAR,	IDD_PROP_STATUSBAR,	PropStatusbar::DlgProc_page },
		{ STR_PROPCOMMON_EDITING,	IDD_PROP_EDIT,		PropEdit::DlgProc_page },
		{ STR_PROPCOMMON_FILE,		IDD_PROP_FILE,		PropFile::DlgProc_page },
		{ STR_PROPCOMMON_FILENAME,	IDD_PROP_FNAME,		PropFileName::DlgProc_page },
		{ STR_PROPCOMMON_BACKUP,	IDD_PROP_BACKUP,	PropBackup::DlgProc_page },
		{ STR_PROPCOMMON_FORMAT,	IDD_PROP_FORMAT,	PropFormat::DlgProc_page },
		{ STR_PROPCOMMON_SEARCH,	IDD_PROP_GREP,		PropGrep::DlgProc_page },
		{ STR_PROPCOMMON_KEYS,		IDD_PROP_KEYBIND,	PropKeybind::DlgProc_page },
		{ STR_PROPCOMMON_CUSTMENU,	IDD_PROP_CUSTMENU,	PropCustmenu::DlgProc_page },
		{ STR_PROPCOMMON_KEYWORD,	IDD_PROP_KEYWORD,	PropKeyword::DlgProc_page },
		{ STR_PROPCOMMON_SUPPORT,	IDD_PROP_HELPER,	PropHelper::DlgProc_page },
		{ STR_PROPCOMMON_MACRO,		IDD_PROP_MACRO,		PropMacro::DlgProc_page },
		{ STR_PROPCOMMON_PLUGIN,	IDD_PROP_PLUGIN,	PropPlugin::DlgProc_page },
	};

	std::tstring		sTabname[_countof(ComPropSheetInfoList)];
	PROPSHEETPAGE		psp[_countof(ComPropSheetInfoList)];
	for (nIdx=0; nIdx<_countof(ComPropSheetInfoList); ++nIdx) {
		sTabname[nIdx] = LS(ComPropSheetInfoList[nIdx].nTabNameId);

		PROPSHEETPAGE* p = &psp[nIdx];
		memset_raw(p, 0, sizeof_raw(*p));
		p->dwSize      = sizeof_raw(*p);
		p->dwFlags     = PSP_USETITLE | PSP_HASHELP;
		p->hInstance   = SelectLang::getLangRsrcInstance();
		p->pszTemplate = MAKEINTRESOURCE(ComPropSheetInfoList[nIdx].resId);
		p->pszIcon     = NULL;
		p->pfnDlgProc  = ComPropSheetInfoList[nIdx].DProc;
		p->pszTitle    = sTabname[nIdx].c_str();
		p->lParam      = (LPARAM)this;
		p->pfnCallback = nullptr;
	}

	PROPSHEETHEADER psh;
	memset_raw(&psh, 0, sizeof_raw(psh));
	
	//	Windows 95�΍�DProperty Sheet�̃T�C�Y��Windows95���F���ł��镨�ɌŒ肷��D
	psh.dwSize = sizeof_old_PROPSHEETHEADER;

	//	���ʐݒ�̉B��[�K�p]�{�^���̐��̂͂����B�s���̃R�����g�A�E�g�����ւ��Ă݂�΂킩��
	psh.dwFlags    = PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE | PSH_USEPAGELANG;
	psh.hwndParent = hwndParent;
	psh.hInstance  = SelectLang::getLangRsrcInstance();
	psh.pszIcon    = NULL;
	psh.pszCaption = LS(STR_PROPCOMMON);	//_T("���ʐݒ�");
	psh.nPages     = nIdx;

	if (nPageNum == -1) {
		psh.nStartPage = nPageNum;
	}else
	if (0 > nPageNum) {
		psh.nStartPage = 0;
	}else {
		psh.nStartPage = nPageNum;
	}
	if (psh.nPages - 1 < psh.nStartPage) {
		psh.nStartPage = psh.nPages - 1;
	}

	psh.ppsp = psp;
	psh.pfnCallback = nullptr;

	nRet = MyPropertySheet(&psh);
	if (nRet == -1) {
		TCHAR*	pszMsgBuf;
		::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			::GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	// �f�t�H���g����
			(LPTSTR)&pszMsgBuf,
			0,
			NULL
		);
		PleaseReportToAuthor(
			NULL,
			LS(STR_ERR_DLGPROPCOMMON24),
			psh.nStartPage,
			pszMsgBuf
		);
		::LocalFree(pszMsgBuf);
	}

	return nRet;
}

/*!	ShareData����ꎞ�̈�֐ݒ���R�s�[���� */
void PropCommon::InitData(void)
{
	common = pShareData->common;

	// TypeConfig�S�̂�ێ�����K�v�͂Ȃ�
	for (int i=0; i<GetDllShareData().nTypesCount; ++i) {
		KeywordSetIndex indexs;
		TypeConfig type;
		DocTypeManager().GetTypeConfig(TypeConfigNum(i), type);
		indexs.typeId = type.id;
		for (int j=0; j<MAX_KEYWORDSET_PER_TYPE; ++j) {
			indexs.index[j] = type.nKeywordSetIdx[j];
		}
		types_nKeywordSetIdx.push_back(indexs);
	}
}

/*!	ShareData �� �ݒ��K�p�E�R�s�[����
	@note ShareData�ɃR�s�[���邾���Ȃ̂ŁC�X�V�v���Ȃǂ́C���p���鑤�ŏ������Ă��炤
*/
void PropCommon::ApplyData(void)
{
	pShareData->common = common;

	const int nSize = (int)types_nKeywordSetIdx.size();
	for (int i=0; i<nSize; ++i) {
		TypeConfigNum configIdx = DocTypeManager().GetDocumentTypeOfId(types_nKeywordSetIdx[i].typeId);
		if (configIdx.IsValidType()) {
			TypeConfig type;
			DocTypeManager().GetTypeConfig(configIdx, type);
			// �ύX���ꂽ�ݒ�l�̃R�s�[
			for (int j = 0; j < MAX_KEYWORDSET_PER_TYPE; ++j) {
				type.nKeywordSetIdx[j] = types_nKeywordSetIdx[i].index[j];
			}
			DocTypeManager().SetTypeConfig(configIdx, type);
		}
	}
}


// �w���v
void PropCommon::OnHelp(HWND hwndParent, int nPageID)
{
	int		nContextID;
	switch (nPageID) {
	case IDD_PROP_GENERAL:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_GENERAL);
		break;
	case IDD_PROP_FORMAT:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_FORMAT);
		break;
	case IDD_PROP_FILE:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_FILE);
		break;
	case IDD_PROP_TOOLBAR:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_TOOLBAR);
		break;
	case IDD_PROP_KEYWORD:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_KEYWORD);
		break;
	case IDD_PROP_CUSTMENU:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_CUSTMENU);
		break;
	case IDD_PROP_HELPER:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_HELPER);
		break;

	case IDD_PROP_EDIT:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_EDIT);
		break;
	case IDD_PROP_BACKUP:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_BACKUP);
		break;
	case IDD_PROP_WIN:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_WINDOW);
		break;
	case IDD_PROP_TAB:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_TAB);
		break;
	case IDD_PROP_STATUSBAR:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_STATUSBAR);
		break;
	case IDD_PROP_GREP:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_GREP);
		break;
	case IDD_PROP_KEYBIND:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_KEYBIND);
		break;
	case IDD_PROP_MACRO:	//@@@ 2002.01.02
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_MACRO);
		break;
	case IDD_PROP_FNAME:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_FNAME);
		break;
	case IDD_PROP_PLUGIN:	//@@@ 2002.01.02
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_PLUGIN);
		break;
	case IDD_PROP_MAINMENU:	//@@@ 2010/6/2 Uchi
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_MAINMENU);
		break;

	default:
		nContextID = -1;
		break;
	}
	if (nContextID != -1) {
		MyWinHelp(hwndParent, HELP_CONTEXT, nContextID);
	}
	return;
}


/*!	�R���g���[���Ƀt�H���g�ݒ肷�� */
HFONT PropCommon::SetCtrlFont(HWND hwndDlg, int idc_ctrl, const LOGFONT& lf)
{
	HFONT	hFont;
	HWND	hCtrl;

	// �_���t�H���g���쐬
	hCtrl = ::GetDlgItem(hwndDlg, idc_ctrl);
	hFont = ::CreateFontIndirect(&lf);
	if (hFont) {
		// �t�H���g�̐ݒ�
		::SendMessage(hCtrl, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
	}
	return hFont;
}


/*!	�t�H���g���x���Ƀt�H���g�ƃt�H���g���ݒ肷�� */
HFONT PropCommon::SetFontLabel(HWND hwndDlg, int idc_static, const LOGFONT& lf, int nps)
{
	HFONT	hFont;
	TCHAR	szFontName[80];
	LOGFONT lfTemp;
	lfTemp = lf;
	// �傫������t�H���g�͏������\��
	if (lfTemp.lfHeight < -16) {
		lfTemp.lfHeight = -16;
	}

	hFont = SetCtrlFont(hwndDlg, idc_static, lfTemp);

	// �t�H���g���̐ݒ�
	auto_sprintf_s(szFontName, (nps % 10) ? _T("%s(%.1fpt)") : _T("%s(%.0fpt)"),
		lf.lfFaceName, double(nps)/10);
	::DlgItem_SetText(hwndDlg, idc_static, szFontName);

	return hFont;
}

