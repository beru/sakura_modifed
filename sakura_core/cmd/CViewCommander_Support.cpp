/*!	@file
@brief ViewCommander�N���X�̃R�}���h(�x��)�֐��Q

	2012/12/16	CViewCommander_Hokan.cpp��cppCViewCommander.cpp����x���֘A�𕪗���CViewCommander_Support.cpp�ɖ��̕ύX
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, asa-o
	Copyright (C) 2002, genta, aroka
	Copyright (C) 2003, Moca
	Copyright (C) 2005, genta
	Copyright (C) 2006, ryoji, fon
	Copyright (C) 2007, kobake, ryoji
	Copyright (C) 2011, Moca
	Copyright (C) 2012, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <HtmlHelp.h>
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "CPropertyManager.h"
#include "CEditApp.h"
#include "dlg/CDlgAbout.h"	// Dec. 24, 2000 JEPRO �ǉ�
#include "env/CHelpManager.h"
#include "util/module.h"
#include "util/shell.h"

/*!	���͕⊮
	Ctrl+Space�ł����ɓ����B
	CEditView::m_bHokan�F ���ݕ⊮�E�B���h�E���\������Ă��邩��\���t���O�B
	m_common.m_helper.m_bUseHokan�F���ݕ⊮�E�B���h�E���\������Ă���ׂ����ۂ�������킷�t���O�B

    @date 2001/06/19 asa-o �p�啶���������𓯈ꎋ����
                     ��₪1�̂Ƃ��͂���Ɋm�肷��
	@date 2001/06/14 asa-o �Q�ƃf�[�^�ύX
	                 �J���v���p�e�B�V�[�g���^�C�v�ʂɕύXy
	@date 2000/09/15 JEPRO [Esc]�L�[��[x]�{�^���ł����~�ł���悤�ɕύX
	@date 2005/01/10 genta CEditView_Command����ړ�
*/
void ViewCommander::Command_HOKAN(void)
{
	if (!GetDllShareData().m_common.m_helper.m_bUseHokan) {
		GetDllShareData().m_common.m_helper.m_bUseHokan = TRUE;
	}
#if 0
// 2011.06.24 Moca Plugin�����ɏ]�����ݒ�̊m�F����߂�
retry:;
	// �⊮���ꗗ�t�@�C�����ݒ肳��Ă��Ȃ��Ƃ��́A�ݒ肷��悤�ɑ����B
	// 2003.06.22 Moca �t�@�C�������猟������ꍇ�ɂ͕⊮�t�@�C���̐ݒ�͕K�{�ł͂Ȃ�
	if (!m_pCommanderView->m_pTypeData->m_bUseHokanByFile &&
		!m_pCommanderView->m_pTypeData->m_bUseHokanByKeyword &&
		m_pCommanderView->m_pTypeData->m_szHokanFile[0] == _T('\0')
	) {
		ConfirmBeep();
		if (IDYES == ::ConfirmMessage(
				GetMainWindow(),
				LS(STR_ERR_DLGEDITVWHOKAN1)
			) 
		) {
			// �^�C�v�ʐݒ� �v���p�e�B�V�[�g
			if (!CEditApp::getInstance()->m_pPropertyManager->OpenPropertySheetTypes(2, GetDocument()->m_docType.GetDocumentType())) {
				return;
			}
			goto retry;
		}
	}
#endif
	NativeW cmemData;
	// �J�[�\�����O�̒P����擾
	if (0 < m_pCommanderView->GetParser().GetLeftWord(&cmemData, 100)) {
		m_pCommanderView->ShowHokanMgr(cmemData, true);
	}else {
		InfoBeep(); // 2010.04.03 Error��Info
		m_pCommanderView->SendStatusMessage(LS(STR_SUPPORT_NOT_COMPLITE)); // 2010.05.29 �X�e�[�^�X�ŕ\��
		GetDllShareData().m_common.m_helper.m_bUseHokan = FALSE;	// ���͕⊮�I���̂��m�点
	}
	return;
}


/*! �L�����b�g�ʒu�̒P�����������ON-OFF

	@date 2006.03.24 fon �V�K�쐬
*/
void ViewCommander::Command_ToggleKeySearch(int option)
{	// ���ʐݒ�_�C�A���O�̐ݒ���L�[���蓖�Ăł��؂�ւ�����悤��
	auto& bUseCaretKeyword = GetDllShareData().m_common.m_search.m_bUseCaretKeyWord;
	if (option == 0) {
		bUseCaretKeyword = !bUseCaretKeyword;
	}else if (option == 1) {
		bUseCaretKeyword = TRUE;
	}else if (option == 2) {
		bUseCaretKeyword = FALSE;
	}
}


// �w���v�ڎ�
void ViewCommander::Command_HELP_CONTENTS(void)
{
	ShowWinHelpContents(m_pCommanderView->GetHwnd());	// �ڎ���\������
	return;
}


// �w���v�L�[���[�h����
void ViewCommander::Command_HELP_SEARCH(void)
{
	MyWinHelp(m_pCommanderView->GetHwnd(), HELP_KEY, (ULONG_PTR)_T(""));	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
	return;
}


// �R�}���h�ꗗ
void ViewCommander::Command_MENU_ALLFUNC(void)
{
	POINT	po;
	RECT	rc;

// From Here Sept. 15, 2000 JEPRO
// �T�u���j���[�A���Ɂu���̑��v�̃R�}���h�ɑ΂��ăX�e�[�^�X�o�[�ɕ\�������L�[�A�T�C�����
// ���j���[�ŉB��Ȃ��悤�ɉE�ɂ��炵��
// (�{���͂��́u�R�}���h�ꗗ�v���j���[���_�C�A���O�ɕύX���o�[���܂�Ŏ��R�Ɉړ��ł���悤�ɂ�����)
//	po.x = 0;
	po.x = 540;
//	To Here Sept. 15, 2000 (Oct. 7, 2000 300��500; Nov. 3, 2000 500��540)
	po.y = 0;

	EditWnd* pCEditWnd = GetEditWindow();	// Sep. 10, 2002 genta
	::GetClientRect(pCEditWnd->GetHwnd(), &rc);
	po.x = t_min(po.x, rc.right);
	::ClientToScreen(pCEditWnd->GetHwnd(), &po);
	::GetWindowRect(pCEditWnd->m_splitterWnd.GetHwnd() , &rc);
	po.y = rc.top;

	pCEditWnd->GetMenuDrawer().ResetContents();

	// Oct. 3, 2001 genta
	FuncLookup& FuncLookup = GetDocument()->m_funcLookup;
	HMENU hMenu = ::CreatePopupMenu();
// Oct. 14, 2000 JEPRO �u--����`--�v��\�������Ȃ��悤�ɕύX�������Ƃ�1��(�J�[�\���ړ��n)���O�ɃV�t�g���ꂽ(���̕ύX�ɂ���� i=1��i=0 �ƕύX)
	// Oct. 3, 2001 genta
	for (int i=0; i<FuncLookup.GetCategoryCount(); ++i) {
		HMENU hMenuPopUp = ::CreatePopupMenu();
		for (int j=0; j<FuncLookup.GetItemCount(i); ++j) {
			// Oct. 3, 2001 genta
			int code = FuncLookup.Pos2FuncCode(i, j, false);	// 2007.11.02 ryoji ���o�^�}�N����\���𖾎��w��
			if (code != 0) {
				WCHAR	szLabel[300];
				FuncLookup.Pos2FuncName(i, j, szLabel, 256);
				UINT uFlags = MF_BYPOSITION | MF_STRING | MF_ENABLED;
				// Oct. 3, 2001 genta
				pCEditWnd->GetMenuDrawer().MyAppendMenu(hMenuPopUp, uFlags, code, szLabel, L"");
			}
		}
		// Oct. 3, 2001 genta
		pCEditWnd->GetMenuDrawer().MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , FuncLookup.Category2Name(i) , _T(""));
//		pCEditWnd->GetMenuDrawer().MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , nsFuncCode::ppszFuncKind[i]);
	}
	int nId = ::TrackPopupMenu(
		hMenu,
		TPM_TOPALIGN
		| TPM_LEFTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		,
		po.x,
		po.y,
		0,
		GetMainWindow()/*GetHwnd()*/,
		NULL
	);
	::DestroyMenu(hMenu);
	if (nId != 0) {
		// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
//		HandleCommand(nFuncID, true, 0, 0, 0, 0);
		::PostMessage(GetMainWindow(), WM_COMMAND, MAKELONG(nId, 0), (LPARAM)NULL);
	}
	return;
}


/* �O���w���v�P
	@date 2012.09.26 Moca HTMLHELP�Ή�
*/
void ViewCommander::Command_EXTHELP1(void)
{
retry:;
	if (!HelpManager().ExtWinHelpIsSet(&(GetDocument()->m_docType.GetDocumentAttribute()))) {
//	if (wcslen(GetDllShareData().m_common.m_szExtHelp1) == 0) {
		ErrorBeep();
// From Here Sept. 15, 2000 JEPRO
//		[Esc]�L�[��[x]�{�^���ł����~�ł���悤�ɕύX
		if (IDYES == ::MYMESSAGEBOX(
				NULL,
				MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST,
				GSTR_APPNAME,
// To Here Sept. 15, 2000
				LS(STR_ERR_CEDITVIEW_CMD01)
			)
		) {
			// ���ʐݒ� �v���p�e�B�V�[�g
			if (!EditApp::getInstance()->OpenPropertySheet(ID_PROPCOM_PAGENUM_HELPER)) {
				return;
			}
			goto retry;
		}else {
		// Jun. 15, 2000 genta
			return;
		}
	}

	NativeW cmemCurText;
	const TCHAR* helpfile = HelpManager().GetExtWinHelp(&(GetDocument()->m_docType.GetDocumentAttribute()));

	// ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾
	m_pCommanderView->GetCurrentTextForSearch(cmemCurText, false);
	TCHAR path[_MAX_PATH];
	if (_IS_REL_PATH(helpfile)) {
		// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
		// 2007.05.21 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
		GetInidirOrExedir(path, helpfile);
	}else {
		auto_strcpy(path, helpfile);
	}
	// 2012.09.26 Moca HTMLHELP�Ή�
	TCHAR	szExt[_MAX_EXT];
	_tsplitpath(path, NULL, NULL, NULL, szExt);
	if (_tcsicmp(szExt, _T(".chi")) == 0 || _tcsicmp(szExt, _T(".chm")) == 0 || _tcsicmp(szExt, _T(".col")) == 0) {
		std::wstring pathw = to_wchar(path);
		Command_EXTHTMLHELP(pathw.c_str(), cmemCurText.GetStringPtr());
	}else {
		::WinHelp(m_pCommanderView->m_hwndParent, path, HELP_KEY, (ULONG_PTR)cmemCurText.GetStringPtr());
	}
	return;
}


/*!
	�O��HTML�w���v
	
	@param helpfile [in] HTML�w���v�t�@�C�����DNULL�̂Ƃ��̓^�C�v�ʂɐݒ肳�ꂽ�t�@�C���D
	@param kwd [in] �����L�[���[�h�DNULL�̂Ƃ��̓J�[�\���ʒuor�I�����ꂽ���[�h
	@date 2002.07.05 genta �C�ӂ̃t�@�C���E�L�[���[�h�̎w�肪�ł���悤�����ǉ�
*/
void ViewCommander::Command_EXTHTMLHELP(const WCHAR* _helpfile, const WCHAR* kwd)
{
	std::tstring helpfile;
	if (_helpfile) {
		helpfile = to_tchar(_helpfile);
	}

	HWND hwndHtmlHelp;

	DEBUG_TRACE(_T("helpfile=%ts\n"), helpfile.c_str());

	// From Here Jul. 5, 2002 genta
	const TCHAR* filename = NULL;
	if (helpfile.length() == 0) {
		while (!HelpManager().ExtHTMLHelpIsSet(&(GetDocument()->m_docType.GetDocumentAttribute()))) {
			ErrorBeep();
	// From Here Sept. 15, 2000 JEPRO
	//		[Esc]�L�[��[x]�{�^���ł����~�ł���悤�ɕύX
			if (IDYES != ::MYMESSAGEBOX(
					NULL,
					MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST,
					GSTR_APPNAME,
	// To Here Sept. 15, 2000
					LS(STR_ERR_CEDITVIEW_CMD02)
				)
			) {
				return;
			}
			// ���ʐݒ� �v���p�e�B�V�[�g
			if (!EditApp::getInstance()->OpenPropertySheet(ID_PROPCOM_PAGENUM_HELPER)) {
				return;
			}
		}
		filename = HelpManager().GetExtHTMLHelp(&(GetDocument()->m_docType.GetDocumentAttribute()));
	}else {
		filename = helpfile.c_str();
	}
	// To Here Jul. 5, 2002 genta

	// Jul. 5, 2002 genta
	// �L�[���[�h�̊O���w����\��
	NativeW	cmemCurText;
	if (kwd && kwd[0] != _T('\0')) {
		cmemCurText.SetString(kwd);
	}else {
		// ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾
		m_pCommanderView->GetCurrentTextForSearch(cmemCurText);
	}

	// HtmlHelp�r���[�A�͂ЂƂ�
	if (HelpManager().HTMLHelpIsSingle(&(GetDocument()->m_docType.GetDocumentAttribute()))) {
		// �^�X�N�g���C�̃v���Z�X��HtmlHelp���N��������
		// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
		// 2007.05.21 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
		TCHAR* pWork = GetDllShareData().m_workBuffer.GetWorkBuffer<TCHAR>();
		if (_IS_REL_PATH(filename)) {
			GetInidirOrExedir(pWork, filename);
		}else {
			_tcscpy(pWork, filename); // Jul. 5, 2002 genta
		}
		int nLen = _tcslen(pWork);
		_tcscpy(&pWork[nLen + 1], cmemCurText.GetStringT());
		hwndHtmlHelp = (HWND)::SendMessage(
			GetDllShareData().m_handles.m_hwndTray,
			MYWM_HTMLHELP,
			(WPARAM)GetMainWindow(),
			0
		);
	}else {
		// ������HtmlHelp���N��������
		HH_AKLINK	link;
		link.cbStruct = sizeof(link) ;
		link.fReserved = FALSE ;
		link.pszKeywords = cmemCurText.GetStringT();
		link.pszUrl = NULL;
		link.pszMsgText = NULL;
		link.pszMsgTitle = NULL;
		link.pszWindow = NULL;
		link.fIndexOnFail = TRUE;

		// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
		// 2007.05.21 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
		if (_IS_REL_PATH(filename)) {
			TCHAR path[_MAX_PATH];
			GetInidirOrExedir(path, filename);
			// Jul. 6, 2001 genta HtmlHelp�̌Ăяo�����@�ύX
			hwndHtmlHelp = OpenHtmlHelp(
				NULL/*GetDllShareData().m_handles.m_hwndTray*/,
				path, // Jul. 5, 2002 genta
				HH_KEYWORD_LOOKUP,
				(DWORD_PTR)&link
			);
		}else {
			// Jul. 6, 2001 genta HtmlHelp�̌Ăяo�����@�ύX
			hwndHtmlHelp = OpenHtmlHelp(
				NULL/*GetDllShareData().m_handles.m_hwndTray*/,
				filename, // Jul. 5, 2002 genta
				HH_KEYWORD_LOOKUP,
				(DWORD_PTR)&link
			);
		}
	}

	// Jul. 6, 2001 genta hwndHtmlHelp�̃`�F�b�N��ǉ�
	if (hwndHtmlHelp) {
		::BringWindowToTop(hwndHtmlHelp);
	}

	return;
}


// �o�[�W�������
void ViewCommander::Command_ABOUT(void)
{
	DlgAbout cDlgAbout;
	cDlgAbout.DoModal(G_AppInstance(), m_pCommanderView->GetHwnd());
	return;
}

