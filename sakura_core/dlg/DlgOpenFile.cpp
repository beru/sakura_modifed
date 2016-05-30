/*!	@file
	@brief �t�@�C���I�[�v���_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date	1998/08/10 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, jepro, Stonee, genta
	Copyright (C) 2002, MIK, YAZAKI, genta
	Copyright (C) 2003, MIK, KEITA, Moca, ryoji
	Copyright (C) 2004, genta
	Copyright (C) 2005, novice, ryoji
	Copyright (C) 2006, ryoji, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <CdErr.h>
#include <Dlgs.h>
#include "dlg/DlgOpenFile.h"
#include "func/Funccode.h"	// Stonee, 2001/05/18
#include "FileExt.h"
#include "env/DocTypeManager.h"
#include "env/ShareData.h"
#include "EditApp.h"
#include "charset/CodePage.h"
#include "doc/DocListener.h"
#include "recent/Recent.h"
#include "_os/OsVersionInfo.h"
#include "dlg/Dialog.h"
#include "util/window.h"
#include "util/shell.h"
#include "util/fileUtil.h"
#include "util/os.h"
#include "util/module.h"
#include "sakura_rc.h"
#include "sakura.hh"

// �I�[�v���t�@�C�� CDlgOpenFile.cpp	//@@@ 2002.01.07 add start MIK
static const DWORD p_helpids[] = {	//13100
//	IDOK,					HIDOK_OPENDLG,					// Win�̃w���v�ŏ���ɏo�Ă���
//	IDCANCEL,				HIDCANCEL_OPENDLG,				// Win�̃w���v�ŏ���ɏo�Ă���
//	IDC_BUTTON_HELP,		HIDC_OPENDLG_BUTTON_HELP,		// �w���v�{�^��
	IDC_COMBO_CODE,			HIDC_OPENDLG_COMBO_CODE,		// �����R�[�h�Z�b�g
	IDC_COMBO_MRU,			HIDC_OPENDLG_COMBO_MRU,			// �ŋ߂̃t�@�C��
	IDC_COMBO_OPENFOLDER,	HIDC_OPENDLG_COMBO_OPENFOLDER,	// �ŋ߂̃t�H���_
	IDC_COMBO_EOL,			HIDC_OPENDLG_COMBO_EOL,			// ���s�R�[�h
	IDC_CHECK_BOM,			HIDC_OPENDLG_CHECK_BOM,			// BOM	// 2006.08.06 ryoji
	IDC_CHECK_CP,			HIDC_OPENDLG_CHECK_CP,			//CP
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

// 2005.10.29 ryoji
// Windows 2000 version of OPENFILENAME.
// The new version has three extra members.
// See CommDlg.h
#if (_WIN32_WINNT >= 0x0500)
struct OPENFILENAMEZ : public OPENFILENAME {
};
#else
struct OPENFILENAMEZ : public OPENFILENAME {
	void*	pvReserved;
	DWORD	dwReserved;
	DWORD	FlagsEx;
};
#define OPENFILENAME_SIZE_VERSION_400 sizeof(OPENFILENAME)
#endif // (_WIN32_WINNT >= 0x0500)

#ifndef OFN_ENABLESIZING
	#define OFN_ENABLESIZING	0x00800000
#endif

static int AddComboCodePages(HWND hdlg, HWND combo, int nSelCode, bool& bInit);

// 2014.05.22 Moca FileDialog�̍ē��T�|�[�g
class DlgOpenFileMem {
public:
	HINSTANCE		hInstance;	// �A�v���P�[�V�����C���X�^���X�̃n���h��
	HWND			hwndParent;	// �I�[�i�[�E�B���h�E�̃n���h��

	DllSharedData*	pShareData;

	SFilePath		szDefaultWildCard;	// �u�J���v�ł̍ŏ��̃��C���h�J�[�h�i�ۑ����̊g���q�⊮�ł��g�p�����j
	SFilePath		szInitialDir;			// �u�J���v�ł̏����f�B���N�g��

	std::vector<LPCTSTR>	vMRU;
	std::vector<LPCTSTR>	vOPENFOLDER;
};

class DlgOpenFileData {
public:
	DlgOpenFile*	pDlgOpenFile;

	WNDPROC			wpOpenDialogProc;
	int				nHelpTopicID;
	bool			bViewMode;		// �r���[���[�h��
	bool			bIsSaveDialog;	// �ۑ��̃_�C�A���O��
	EncodingType	nCharCode;		// �����R�[�h

	Eol				eol;
	bool			bUseCharCode;
	bool			bUseEol;
	bool			bBom;		// BOM��t���邩�ǂ���	//	Jul. 26, 2003 ryoji BOM
	bool			bUseBom;	// BOM�̗L����I������@�\�𗘗p���邩�ǂ���
	SFilePath		szPath;	// �g���q�̕⊮�����O�ōs�����Ƃ��̃t�@�C���p�X	// 2006.11.10 ryoji

	bool			bInitCodePage;

	ComboBoxItemDeleter		combDelFile;
	RecentFile				recentFile;
	ComboBoxItemDeleter		combDelFolder;
	RecentFolder			recentFolder;

	OPENFILENAME*	pOf;
	OPENFILENAMEZ	ofn;		// 2005.10.29 ryoji OPENFILENAMEZ�u�t�@�C�����J���v�_�C�A���O�p�\����
	HWND			hwndOpenDlg;
	HWND			hwndComboMRU;
	HWND			hwndComboOPENFOLDER;
	HWND			hwndComboCODES;
	HWND			hwndComboEOL;	//	Feb. 9, 2001 genta
	HWND			hwndCheckBOM;	//	Jul. 26, 2003 ryoji BOM�`�F�b�N�{�b�N�X

	DlgOpenFileData()
		:
		pDlgOpenFile(nullptr),
		nHelpTopicID(0)
	{}
};

static const TCHAR* s_pszOpenFileDataName = _T("FileOpenData");


/*
|| 	�J���_�C�A���O�̃T�u�N���X�v���V�[�W��

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
LRESULT APIENTRY OFNHookProcMain(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
//	OFNOTIFY*				pofn;
	DlgOpenFileData* pData = (DlgOpenFileData*)::GetProp(hwnd, s_pszOpenFileDataName);
	WORD wNotifyCode;
	WORD wID;
	static DllSharedData* pShareData;
	switch (uMsg) {
	case WM_MOVE:
		//�u�J���v�_�C�A���O�̃T�C�Y�ƈʒu
		pShareData = &GetDllShareData();
		::GetWindowRect(hwnd, &pShareData->common.others.rcOpenDialog);
//		MYTRACE(_T("WM_MOVE 1\n"));
		break;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// notification code
		wID = LOWORD(wParam);			// item, control, or accelerator identifier
		switch (wNotifyCode) {
//			break;
		// �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ
		case BN_CLICKED:
			switch (wID) {
			case pshHelp:
				// �w���v
				MyWinHelp(hwnd, HELP_CONTEXT, pData->nHelpTopicID);	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
				break;
			case chx1:	// The read-only check box
				pData->bViewMode = DlgButton_IsChecked(hwnd , chx1);
				break;
			}
			break;
		}
		break;
	case WM_NOTIFY:
//		pofn = (OFNOTIFY*) lParam;
//		MYTRACE(_T("=========WM_NOTIFY=========\n"));
//		MYTRACE(_T("pofn->hdr.hwndFrom=%xh\n"), pofn->hdr.hwndFrom);
//		MYTRACE(_T("pofn->hdr.idFrom=%xh(%d)\n"), pofn->hdr.idFrom, pofn->hdr.idFrom);
//		MYTRACE(_T("pofn->hdr.code=%xh(%d)\n"), pofn->hdr.code, pofn->hdr.code);
		break;
	}
//	return ::CallWindowProc((int (__stdcall *)(void))(WNDPROC)wpOpenDialogProc, hwnd, uMsg, wParam, lParam);

	return ::CallWindowProc(pData->wpOpenDialogProc, hwnd, uMsg, wParam, lParam);
}


/*!
	�J���_�C�A���O�̃t�b�N�v���V�[�W��
*/
// Modified by KEITA for WIN64 2003.9.6
// APIENTRY -> CALLBACK Moca 2003.09.09
//UINT APIENTRY OFNHookProc(
UINT_PTR CALLBACK OFNHookProc(
	HWND hdlg,		// handle to child dialog window
	UINT uiMsg,		// message identifier
	WPARAM wParam,	// message parameter
	LPARAM lParam 	// message parameter
	)
{
	POINT		po;
	RECT		rc;
	size_t		i;
	OFNOTIFY*	pofn;
	LRESULT		lRes;
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	LONG_PTR	nIdx;
	LONG_PTR	nIdxSel;
	int			nWidth;
	WPARAM		fCheck;	// Jul. 26, 2003 ryoji BOM��ԗp

	// From Here	Feb. 9, 2001 genta
	static const EolType nEolValueArr[] = {
		EolType::None,
		EolType::CRLF,
		EolType::LF,
		EolType::CR,
	};
	// �������Resource���ɓ����
	static const TCHAR*	const	pEolNameArr[] = {
		_T("�ϊ��Ȃ�"), // �_�~�[
		_T("CR+LF"),
		_T("LF (UNIX)"),
		_T("CR (Mac)"),
	};

// To Here	Feb. 9, 2001 genta
	int	nRightMargin = 24;

	switch (uiMsg) {
	case WM_MOVE:
//		MYTRACE(_T("WM_MOVE 2\n"));
		break;
	case WM_SIZE:
		{
			nWidth = LOWORD(lParam);	// width of client area
			//�u�J���v�_�C�A���O�̃T�C�Y�ƈʒu
			DlgOpenFileData* pData = (DlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
			HWND hwndFrame = ::GetParent(hdlg);
			::GetWindowRect(hwndFrame, &pData->pDlgOpenFile->mem->pShareData->common.others.rcOpenDialog);
			// 2005.10.29 ryoji �ŋ߂̃t�@�C���^�t�H���_ �R���{�̉E�[���q�_�C�A���O�̉E�[�ɍ��킹��
			::GetWindowRect(pData->hwndComboMRU, &rc);
			po.x = rc.left;
			po.y = rc.top;
			::ScreenToClient(hdlg, &po);
			::SetWindowPos(pData->hwndComboMRU, 0, 0, 0, nWidth - po.x - nRightMargin, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
			::SetWindowPos(pData->hwndComboOPENFOLDER, 0, 0, 0, nWidth - po.x - nRightMargin, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
			return 0;
		}
	case WM_INITDIALOG:
		{
			// Save off the long pointer to the OPENFILENAME structure.
			// Modified by KEITA for WIN64 2003.9.6
			OPENFILENAME* pOfn = (OPENFILENAME*)lParam;
			DlgOpenFileData* pData = reinterpret_cast<DlgOpenFileData*>(pOfn->lCustData);
			::SetWindowLongPtr(hdlg, DWLP_USER, (LONG_PTR)pData);
			pData->pOf = pOfn;

			// Explorer�X�^�C���́u�J���v�_�C�A���O�̃n���h��
			pData->hwndOpenDlg = ::GetParent( hdlg );
			// �R���g���[���̃n���h��
			pData->hwndComboCODES = ::GetDlgItem(hdlg, IDC_COMBO_CODE);
			pData->hwndComboMRU = ::GetDlgItem(hdlg, IDC_COMBO_MRU);
			pData->hwndComboOPENFOLDER = ::GetDlgItem(hdlg, IDC_COMBO_OPENFOLDER);
			pData->hwndComboEOL = ::GetDlgItem(hdlg, IDC_COMBO_EOL);
			pData->hwndCheckBOM = ::GetDlgItem(hdlg, IDC_CHECK_BOM);//	Jul. 26, 2003 ryoji BOM�`�F�b�N�{�b�N�X
			pData->bInitCodePage = false;

			// 2005.11.02 ryoji �������C�A�E�g�ݒ�
			DlgOpenFile::InitLayout( pData->hwndOpenDlg, hdlg, pData->hwndComboCODES );

			// �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ���
			Combo_SetExtendedUI(pData->hwndComboCODES, TRUE);
			Combo_SetExtendedUI(pData->hwndComboMRU, TRUE);
			Combo_SetExtendedUI(pData->hwndComboOPENFOLDER, TRUE);
			Combo_SetExtendedUI(pData->hwndComboEOL, TRUE);

			// From Here Feb. 9, 2001 genta
			// ���s�R�[�h�̑I���R���{�{�b�N�X������
			// �K�v�ȂƂ��̂ݗ��p����
			if (pData->bUseEol) {
				// �l�̐ݒ�
				// 2013.05.27 �����l��SaveInfo����ݒ肷��
				nIdxSel = 0;
				for (i=0; i<_countof(pEolNameArr); ++i) {
					if (i == 0) {
						nIdx = Combo_AddString(pData->hwndComboEOL, LS(STR_DLGOPNFL1));
					}else {
						nIdx = Combo_AddString(pData->hwndComboEOL, pEolNameArr[i]);
					}
					Combo_SetItemData(pData->hwndComboEOL, nIdx, (int)nEolValueArr[i]);
					if (nEolValueArr[i] == pData->eol) {
						nIdxSel = nIdx;
					}
				}
				Combo_SetCurSel(pData->hwndComboEOL, nIdxSel);
			}else {
				// �g��Ȃ��Ƃ��͉B��
				::ShowWindow(::GetDlgItem(hdlg, IDC_STATIC_EOL), SW_HIDE);
				::ShowWindow(pData->hwndComboEOL, SW_HIDE);
			}
			// To Here Feb. 9, 2001 genta

			// From Here Jul. 26, 2003 ryoji BOM�`�F�b�N�{�b�N�X�̏�����
			if (pData->bUseBom) {
				// �g���Ƃ��͗L���^������؂�ւ��A�`�F�b�N��Ԃ������l�ɐݒ肷��
				if (CodeTypeName(pData->nCharCode).UseBom()) {
					::EnableWindow(pData->hwndCheckBOM, TRUE);
					fCheck = pData->bBom? BST_CHECKED: BST_UNCHECKED;
				}else {
					::EnableWindow(pData->hwndCheckBOM, FALSE);
					fCheck = BST_UNCHECKED;
				}
				BtnCtl_SetCheck(pData->hwndCheckBOM, fCheck);
			}else {
				// �g��Ȃ��Ƃ��͉B��
				::ShowWindow(pData->hwndCheckBOM, SW_HIDE);
			}
			// To Here Jul. 26, 2003 ryoji BOM�`�F�b�N�{�b�N�X�̏�����

			// Explorer�X�^�C���́u�J���v�_�C�A���O���t�b�N
			::SetProp(pData->hwndOpenDlg, s_pszOpenFileDataName, (HANDLE)pData);
			// Modified by KEITA for WIN64 2003.9.6
			pData->wpOpenDialogProc = (WNDPROC) ::SetWindowLongPtr(pData->hwndOpenDlg, GWLP_WNDPROC, (LONG_PTR) OFNHookProcMain);

			// �����R�[�h�I���R���{�{�b�N�X������
			nIdxSel = -1;
			if (pData->bIsSaveDialog) {	// �ۑ��̃_�C�A���O��
				i = 1; // �u�����I���v��΂�
			}else {
				i = 0;
			}
			CodeTypesForCombobox codeTypes;
			for (/*i = 0*/; i<codeTypes.GetCount(); ++i) {
				nIdx = Combo_AddString(pData->hwndComboCODES, codeTypes.GetName(i));
				Combo_SetItemData(pData->hwndComboCODES, nIdx, codeTypes.GetCode(i));
				if (codeTypes.GetCode(i) == pData->nCharCode) {
					nIdxSel = nIdx;
				}
			}
			if (nIdxSel != -1) {
				Combo_SetCurSel(pData->hwndComboCODES, nIdxSel);
			}else {
				CheckDlgButtonBool( hdlg, IDC_CHECK_CP, true );
				if (AddComboCodePages(hdlg, pData->hwndComboCODES, pData->nCharCode, pData->bInitCodePage) == -1) {
					Combo_SetCurSel(pData->hwndComboCODES, 0);
				}
			}
			if (!pData->bUseCharCode) {
				::ShowWindow(GetDlgItem(hdlg, IDC_STATIC_CHARCODE), SW_HIDE);
				::ShowWindow(pData->hwndComboCODES, SW_HIDE);
			}

			// �r���[���[�h�̏����l�Z�b�g
			::CheckDlgButton(pData->hwndOpenDlg, chx1, pData->bViewMode);
			pData->combDelFile = ComboBoxItemDeleter();
			pData->combDelFile.pRecent = &pData->recentFile;
			Dialog::SetComboBoxDeleter(pData->hwndComboMRU, &pData->combDelFile);
			pData->combDelFolder = ComboBoxItemDeleter();
			pData->combDelFolder.pRecent = &pData->recentFolder;
			Dialog::SetComboBoxDeleter(pData->hwndComboOPENFOLDER, &pData->combDelFolder);
		}
		break;

	case WM_DESTROY:
		// �t�b�N����
		{
			DlgOpenFileData* pData = (DlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
			// Modified by KEITA for WIN64 2003.9.6
			::SetWindowLongPtr(pData->hwndOpenDlg, GWLP_WNDPROC, (LONG_PTR)pData->wpOpenDialogProc);
			::RemoveProp(pData->hwndOpenDlg, s_pszOpenFileDataName);
		}
		return FALSE;

	case WM_NOTIFY:
		pofn = (OFNOTIFY*) lParam;
//		MYTRACE(_T("=========WM_NOTIFY=========\n"));
//		MYTRACE(_T("pofn->hdr.hwndFrom=%xh\n"), pofn->hdr.hwndFrom);
//		MYTRACE(_T("pofn->hdr.idFrom=%xh(%d)\n"), pofn->hdr.idFrom, pofn->hdr.idFrom);
//		MYTRACE(_T("pofn->hdr.code=%xh(%d)\n"), pofn->hdr.code, pofn->hdr.code);

		switch (pofn->hdr.code) {
		case CDN_FILEOK:
			// �g���q�̕⊮�����O�ōs��	// 2006.11.10 ryoji
			{
				DlgOpenFileData* pData = (DlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
				if (pData->bIsSaveDialog) {
					TCHAR szDefExt[_MAX_EXT];	// �⊮����g���q
					TCHAR szBuf[_MAX_PATH + _MAX_EXT];	// ���[�N
					LPTSTR pszCur, pszNext;
					int i;
					CommDlg_OpenSave_GetSpec(pData->hwndOpenDlg, szBuf, _MAX_PATH);	// �t�@�C�������̓{�b�N�X���̕�����
					pszCur = szBuf;
					while (*pszCur == _T(' ')) {	// �󔒂�ǂݔ�΂�
						pszCur = ::CharNext(pszCur);
					}
					if (*pszCur == _T('\"')) {	// ��d���p���Ŏn�܂��Ă���
						::lstrcpyn(pData->szPath, pData->pOf->lpstrFile, _MAX_PATH);
					}else {
						_tsplitpath( pData->pOf->lpstrFile, NULL, NULL, NULL, szDefExt );
						if (szDefExt[0] == _T('.') /* && szDefExt[1] != _T('\0') */) {	// ���Ɋg���q�����Ă���	2�����ڂ̃`�F�b�N�̍폜	2008/6/14 Uchi
						// .�݂̂̏ꍇ�ɂ��g���q�t���Ƃ݂Ȃ��B
							lstrcpyn(pData->szPath, pData->pOf->lpstrFile, _MAX_PATH);
						}else {
							switch (pData->pOf->nFilterIndex) {	// �I������Ă���t�@�C���̎��
							case 1:		// ���[�U�[��`
								pszCur = pData->pDlgOpenFile->mem->szDefaultWildCard;
								while (*pszCur != _T('.') && *pszCur != _T('\0')) {	// '.'�܂œǂݔ�΂�
									pszCur = ::CharNext(pszCur);
								}
								i = 0;
								while (*pszCur != _T(';') && *pszCur != _T('\0')) {	// ';'�܂ŃR�s�[����
									pszNext = ::CharNext(pszCur);
									while (pszCur < pszNext) {
										szDefExt[i++] = *pszCur++;
									}
								}
								szDefExt[i] = _T('\0');
								if (::_tcslen(szDefExt) < 2 || szDefExt[1] == _T('*')) {	// �����Ȋg���q?
									szDefExt[0] = _T('\0');
								}
								break;
							case 2:		// *.txt
								::_tcscpy(szDefExt, _T(".txt"));
								break;
							case 3:		// *.*
							default:	// �s��
								szDefExt[0] = _T('\0');
								break;
							}
							lstrcpyn(szBuf, pData->pOf->lpstrFile, _MAX_PATH + 1);
							::_tcscat(szBuf, szDefExt);
							lstrcpyn(pData->szPath, szBuf, _MAX_PATH);
						}
					}

					// �t�@�C���̏㏑���m�F�����O�ōs��	// 2006.11.10 ryoji
					if (IsFileExists(pData->szPath, true)) {
						TCHAR szText[_MAX_PATH + 100];
						lstrcpyn(szText, pData->szPath, _MAX_PATH);
						::_tcscat(szText, LS(STR_DLGOPNFL2));
						if (::MessageBox(pData->hwndOpenDlg, szText, LS(STR_DLGOPNFL3), MB_YESNO | MB_ICONEXCLAMATION) != IDYES) {
							::SetWindowLongPtr(hdlg, DWLP_MSGRESULT, TRUE);
							return TRUE;
						}
					}
				}

				// �����R�[�h�I���R���{�{�b�N�X �l���擾
				nIdx = Combo_GetCurSel(pData->hwndComboCODES);
				lRes = Combo_GetItemData(pData->hwndComboCODES, nIdx);
				pData->nCharCode = (EncodingType)lRes;	// �����R�[�h
				// Feb. 9, 2001 genta
				if (pData->bUseEol) {
					nIdx = Combo_GetCurSel(pData->hwndComboEOL);
					lRes = Combo_GetItemData(pData->hwndComboEOL, nIdx);
					pData->eol = (EolType)lRes;	// �����R�[�h
				}
				// From Here Jul. 26, 2003 ryoji
				// BOM�`�F�b�N�{�b�N�X�̏�Ԃ��擾
				if (pData->bUseBom) {
					lRes = BtnCtl_GetCheck(pData->hwndCheckBOM);
					pData->bBom = (lRes == BST_CHECKED);	// BOM
				}
				// To Here Jul. 26, 2003 ryoji

//			MYTRACE(_T("�����R�[�h  lRes=%d\n"), lRes);
//			MYTRACE(_T("pofn->hdr.code=CDN_FILEOK        \n"));break;
			}
			break;	// CDN_FILEOK

		case CDN_FOLDERCHANGE:
//			MYTRACE(_T("pofn->hdr.code=CDN_FOLDERCHANGE  \n"));
			{
				wchar_t szFolder[_MAX_PATH];
				DlgOpenFileData* pData = (DlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
				lRes = CommDlg_OpenSave_GetFolderPath(pData->hwndOpenDlg, szFolder, _countof(szFolder));
			}
//			MYTRACE(_T("\tlRes=%d\tszFolder=[%ls]\n"), lRes, szFolder);

			break;
		case CDN_SELCHANGE:
			{
				DlgOpenFileData* pData = (DlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
				// OFN�̍Đݒ��NT�n�ł�Unicode��API�̂ݗL��
				if (1
					&& (pData->ofn.Flags & OFN_ALLOWMULTISELECT)
					&&
#ifdef _UNICODE
						IsWin32NT()
#else
						!IsWin32NT()
#endif
				) {
					DWORD nLength = CommDlg_OpenSave_GetSpec(pData->hwndOpenDlg, NULL, 0);
					nLength += _MAX_PATH + 2;
					if (pData->ofn.nMaxFile < nLength) {
						delete[] pData->ofn.lpstrFile;
						pData->ofn.lpstrFile = new TCHAR[nLength];
						pData->ofn.nMaxFile = nLength;
					}
				}
			}
			// MYTRACE(_T("pofn->hdr.code=CDN_SELCHANGE     \n"));
			break;
//		case CDN_HELP			:	MYTRACE(_T("pofn->hdr.code=CDN_HELP          \n"));break;
//		case CDN_INITDONE		:	MYTRACE(_T("pofn->hdr.code=CDN_INITDONE      \n"));break;
//		case CDN_SHAREVIOLATION	:	MYTRACE(_T("pofn->hdr.code=CDN_SHAREVIOLATION\n"));break;
//		case CDN_TYPECHANGE		:	MYTRACE(_T("pofn->hdr.code=CDN_TYPECHANGE    \n"));break;
//		default:					MYTRACE(_T("pofn->hdr.code=???\n"));break;
		}

//		MYTRACE(_T("=======================\n"));
		break;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// notification code
		wID = LOWORD(wParam);			// item, control, or accelerator identifier
		hwndCtl = (HWND)lParam;		// handle of control
		switch (wNotifyCode) {
		case CBN_SELCHANGE:
			switch ((int) LOWORD(wParam)) {
			// From Here Jul. 26, 2003 ryoji
			// �����R�[�h�̕ύX��BOM�`�F�b�N�{�b�N�X�ɔ��f
			case IDC_COMBO_CODE:
				{
					DlgOpenFileData* pData = (DlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
					nIdx = Combo_GetCurSel((HWND) lParam);
					lRes = Combo_GetItemData((HWND) lParam, nIdx);
					CodeTypeName codeTypeName(lRes);
					if (codeTypeName.UseBom()) {
						::EnableWindow(pData->hwndCheckBOM, TRUE);
						if (lRes == pData->nCharCode){
							fCheck = pData->bBom ? BST_CHECKED: BST_UNCHECKED;
						}else {
							fCheck = codeTypeName.IsBomDefOn() ? BST_CHECKED : BST_UNCHECKED;
						}
					}else {
						::EnableWindow(pData->hwndCheckBOM, FALSE);
						fCheck = BST_UNCHECKED;
					}
					BtnCtl_SetCheck(pData->hwndCheckBOM, fCheck);
				}
				break;
			// To Here Jul. 26, 2003 ryoji
			case IDC_COMBO_MRU:
			case IDC_COMBO_OPENFOLDER:
				{
					DlgOpenFileData* pData = (DlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
					TCHAR szWork[_MAX_PATH + 1];
					nIdx = Combo_GetCurSel((HWND) lParam);
					if (Combo_GetLBText((HWND) lParam, nIdx, szWork) != CB_ERR) {
						// 2005.11.02 ryoji �t�@�C�����w��̃R���g���[�����m�F����
						HWND hwndFilebox = ::GetDlgItem( pData->hwndOpenDlg, cmb13 );		// �t�@�C�����R���{�iWindows 2000�^�C�v�j
						if (!::IsWindow(hwndFilebox))
							hwndFilebox = ::GetDlgItem( pData->hwndOpenDlg, edt1 );	// �t�@�C�����G�f�B�b�g�i���K�V�[�^�C�v�j
						if (::IsWindow(hwndFilebox)) {
							::SetWindowText(hwndFilebox, szWork);
							if (wID == IDC_COMBO_OPENFOLDER)
								::PostMessage(hwndFilebox, WM_KEYDOWN, VK_RETURN, (LPARAM)0);
						}
					}
				}
				break;
			}
			break;	// CBN_SELCHANGE
		case CBN_DROPDOWN:
			{
				DlgOpenFileData* pData = (DlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);

				switch (wID) {
				case IDC_COMBO_MRU:
					if (Combo_GetCount( pData->hwndComboMRU ) == 0) {
						// �ŋߊJ�����t�@�C�� �R���{�{�b�N�X�����l�ݒ�
						//	2003.06.22 Moca vMRU ��NULL�̏ꍇ���l������
						size_t nSize = pData->pDlgOpenFile->mem->vMRU.size();
						for (i=0; i<nSize; ++i) {
							Combo_AddString(pData->hwndComboMRU, pData->pDlgOpenFile->mem->vMRU[i]);
						}
					}
					Dialog::OnCbnDropDown( hwndCtl, true );
					break;

				case IDC_COMBO_OPENFOLDER:
					if (Combo_GetCount( pData->hwndComboOPENFOLDER ) == 0) {
						// �ŋߊJ�����t�H���_ �R���{�{�b�N�X�����l�ݒ�
						//	2003.06.22 Moca vOPENFOLDER ��NULL�̏ꍇ���l������
						size_t nSize = pData->pDlgOpenFile->mem->vOPENFOLDER.size();
						for (i=0; i<nSize; ++i) {
							Combo_AddString(pData->hwndComboOPENFOLDER, pData->pDlgOpenFile->mem->vOPENFOLDER[i]);
						}
					}
					Dialog::OnCbnDropDown(hwndCtl, true);
					break;
				}
				break;	// CBN_DROPDOWN
			}
		case BN_CLICKED:
			switch (wID) {
			case IDC_CHECK_CP:
				{
					DlgOpenFileData* pData = (DlgOpenFileData*)::GetWindowLongPtr(hdlg, DWLP_USER);
					if (IsDlgButtonCheckedBool(hdlg, IDC_CHECK_CP)) {
						AddComboCodePages(hdlg, pData->hwndComboCODES, -1, pData->bInitCodePage);
					}
				}
				break;
			}
			break;	// BN_CLICKED
		}
		break;	// WM_COMMAND

	//@@@ 2002.01.08 add start
	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids);	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;

	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hdlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids);	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
	//@@@ 2002.01.08 add end

	default:
		return FALSE;
	}
	return TRUE;
}

int AddComboCodePages(HWND hdlg, HWND combo, int nSelCode, bool& bInit)
{
	int nSel = -1;
	if (!bInit) {
		::EnableWindow(GetDlgItem(hdlg, IDC_CHECK_CP), FALSE);
		// �R�[�h�y�[�W�ǉ�
		bInit = true;
		nSel = CodePage::AddComboCodePages(hdlg, combo, nSelCode);
	}
	return nSel;
}


/*! �R���X�g���N�^
	@date 2008.05.05 novice GetModuleHandle(NULL) �� NULL�ɕύX
*/
DlgOpenFile::DlgOpenFile()
{
	// �����o�̏�����
	mem = new DlgOpenFileMem();

	mem->hInstance = NULL;		// �A�v���P�[�V�����C���X�^���X�̃n���h��
	mem->hwndParent = NULL;		// �I�[�i�[�E�B���h�E�̃n���h��

	// ���L�f�[�^�\���̂̃A�h���X��Ԃ�
	mem->pShareData = &GetDllShareData();

	TCHAR szFile[_MAX_PATH + 1];
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	::GetModuleFileName(
		NULL,
		szFile, _countof(szFile)
	);
	_tsplitpath(szFile, szDrive, szDir, NULL, NULL);
	_tcscpy(mem->szInitialDir, szDrive);
	_tcscat(mem->szInitialDir, szDir);

	_tcscpy(mem->szDefaultWildCard, _T("*.*"));	//�u�J���v�ł̍ŏ��̃��C���h�J�[�h�i�ۑ����̊g���q�⊮�ł��g�p�����j

	return;
}


DlgOpenFile::~DlgOpenFile()
{
	delete mem;
	mem = NULL;
	return;
}


// ������
void DlgOpenFile::Create(
	HINSTANCE					hInstance,
	HWND						hwndParent,
	const TCHAR*				pszUserWildCard,
	const TCHAR*				pszDefaultPath,
	const std::vector<LPCTSTR>& vMRU,
	const std::vector<LPCTSTR>& vOPENFOLDER
	)
{
	mem->hInstance = hInstance;
	mem->hwndParent = hwndParent;

	// ���[�U�[��`���C���h�J�[�h�i�ۑ����̊g���q�⊮�ł��g�p�����j
	if (pszUserWildCard) {
		_tcscpy(mem->szDefaultWildCard, pszUserWildCard);
	}

	//�u�J���v�ł̏����t�H���_
	if (pszDefaultPath && pszDefaultPath[0] != _T('\0')) {	// ���ݕҏW���̃t�@�C���̃p�X	//@@@ 2002.04.18
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		// Jun. 23, 2002 genta
		my_splitpath_t(pszDefaultPath, szDrive, szDir, NULL, NULL);
		// 2010.08.28 ���΃p�X����
		TCHAR szRelPath[_MAX_PATH];
		auto_sprintf(szRelPath, _T("%ts%ts"), szDrive, szDir);
		const TCHAR* p = szRelPath;
		if (!::GetLongFileName(p, mem->szInitialDir)) {
			auto_strcpy(mem->szInitialDir, p);
		}
	}
	mem->vMRU = vMRU;
	mem->vOPENFOLDER = vOPENFOLDER;
	return;
}


/*! �u�J���v�_�C�A���O ���[�_���_�C�A���O�̕\��

	@param[in,out] pszPath �����t�@�C�����D�I�����ꂽ�t�@�C�����̊i�[�ꏊ
	@param[in] bSetCurDir �J�����g�f�B���N�g����ύX���邩 �f�t�H���g: false
	@date 2002/08/21 �J�����g�f�B���N�g����ύX���邩�ǂ����̃I�v�V������ǉ�
	@date 2003.05.12 MIK �g���q�t�B���^�Ń^�C�v�ʐݒ�̊g���q���g���悤�ɁB
		�g���q�t�B���^�̊Ǘ���FileExt�N���X�ōs���B
	@date 2005.02.20 novice �g���q���ȗ�������⊮����
*/
bool DlgOpenFile::DoModal_GetOpenFileName(TCHAR* pszPath, bool bSetCurDir)
{
	// �J�����g�f�B���N�g����ۑ��B�֐����甲����Ƃ��Ɏ����ŃJ�����g�f�B���N�g���͕��������B
	CurrentDirectoryBackupPoint curDirBackup;

	// 2003.05.12 MIK
	FileExt fileExt;
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME1), mem->szDefaultWildCard);
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME2), _T("*.txt"));
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME3), _T("*.*"));

	// �\���̂̏�����
	auto pData = std::make_unique<DlgOpenFileData>();
	InitOfn(&pData->ofn);		// 2005.10.29 ryoji
	pData->pDlgOpenFile = this;
	pData->ofn.lCustData = (LPARAM)(pData.get());

	pData->ofn.hwndOwner = mem->hwndParent;
	pData->ofn.hInstance = SelectLang::getLangRsrcInstance();
	pData->ofn.lpstrFilter = fileExt.GetExtFilter();
	// From Here Jun. 23, 2002 genta
	//�u�J���v�ł̏����t�H���_�`�F�b�N����
// 2005/02/20 novice �f�t�H���g�̃t�@�C�����͉����ݒ肵�Ȃ�
	{
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		TCHAR szName[_MAX_FNAME];
		TCHAR szExt[_MAX_EXT];

		// Jun. 23, 2002 Thanks to sui
		my_splitpath_t(pszPath, szDrive, szDir, szName, szExt);
	
		// �w�肳�ꂽ�t�@�C�������݂��Ȃ��Ƃ� szName == NULL
		// �t�@�C���̏ꏊ�Ƀf�B���N�g�����w�肷��ƃG���[�ɂȂ�̂�
		// �t�@�C���������ꍇ�͑S���w�肵�Ȃ����Ƃɂ���D
		if (szName[0] == _T('\0')) {
			pszPath[0] = _T('\0');
		}else {
			TCHAR szRelPath[_MAX_PATH];
			auto_sprintf(szRelPath, _T("%ts%ts%ts%ts"), szDrive, szDir, szName, szExt);
			const TCHAR* p = szRelPath;
			if (!::GetLongFileName(p, pszPath)) {
				auto_strcpy(pszPath, p);
			}
		}
	}
	pData->ofn.lpstrFile = pszPath;
	// To Here Jun. 23, 2002 genta
	pData->ofn.nMaxFile = _MAX_PATH;
	pData->ofn.lpstrInitialDir = mem->szInitialDir;
	pData->ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	pData->ofn.lpstrDefExt = _T(""); // 2005/02/20 novice �g���q���ȗ�������⊮����

	// 2010.08.28 Moca DLL���ǂݍ��܂��̂ňړ�
	ChangeCurrentDirectoryToExeDir();
	if (_GetOpenFileNameRecover( &pData->ofn )) {
		return true;
	}else {
		// May 29, 2004 genta �֐��ɂ܂Ƃ߂�
		DlgOpenFail();
		return false;
	}
}


/*! �ۑ��_�C�A���O ���[�_���_�C�A���O�̕\��
	@param pszPath [i/o] �����t�@�C�����D�I�����ꂽ�t�@�C�����̊i�[�ꏊ
	@param bSetCurDir [in] �J�����g�f�B���N�g����ύX���邩 �f�t�H���g: false
	@date 2002/08/21 �J�����g�f�B���N�g����ύX���邩�ǂ����̃I�v�V������ǉ�
	@date 2003.05.12 MIK �g���q�t�B���^�Ń^�C�v�ʐݒ�̊g���q���g���悤�ɁB
		�g���q�t�B���^�̊Ǘ���FileExt�N���X�ōs���B
	@date 2005.02.20 novice �g���q���ȗ�������⊮����
*/
bool DlgOpenFile::DoModal_GetSaveFileName(TCHAR* pszPath, bool bSetCurDir)
{
	// �J�����g�f�B���N�g����ۑ��B�֐����甲����Ƃ��Ɏ����ŃJ�����g�f�B���N�g���͕��������B
	CurrentDirectoryBackupPoint curDirBackup;

	// 2003.05.12 MIK
	FileExt fileExt;
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME1), mem->szDefaultWildCard);
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME2), _T("*.txt"));
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME3), _T("*.*"));
	
	// 2010.08.28 �J�����g�f�B���N�g�����ړ�����̂Ńp�X��������
	if (pszPath[0]) {
		TCHAR szFullPath[_MAX_PATH];
		const TCHAR* pOrg = pszPath;
		if (::GetLongFileName(pOrg, szFullPath)) {
			// �����B�����߂�
			auto_strcpy(pszPath, szFullPath);
		}
	}

	// �\���̂̏�����
	auto pData = std::make_unique<DlgOpenFileData>();
	InitOfn(&pData->ofn);		// 2005.10.29 ryoji
	pData->pDlgOpenFile = this;
	pData->ofn.lCustData = (LPARAM)(pData.get());
	pData->ofn.hwndOwner = mem->hwndParent;
	pData->ofn.hInstance = SelectLang::getLangRsrcInstance();
	pData->ofn.lpstrFilter = fileExt.GetExtFilter();
	pData->ofn.lpstrFile = pszPath; // 2005/02/20 novice �f�t�H���g�̃t�@�C�����͉����ݒ肵�Ȃ�
	pData->ofn.nMaxFile = _MAX_PATH;
	pData->ofn.lpstrInitialDir = mem->szInitialDir;
	pData->ofn.Flags = OFN_CREATEPROMPT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	
	pData->ofn.lpstrDefExt = _T("");	// 2005/02/20 novice �g���q���ȗ�������⊮����
	
	// 2010.08.28 Moca DLL���ǂݍ��܂��̂ňړ�
	ChangeCurrentDirectoryToExeDir();

	if (GetSaveFileNameRecover( &pData->ofn )) {
		return true;
	}else {
		// May 29, 2004 genta �֐��ɂ܂Ƃ߂�
		DlgOpenFail();
		return false;
	}
}


/*! �u�J���v�_�C�A���O ���[�_���_�C�A���O�̕\��
	@date 2003.05.12 MIK �g���q�t�B���^�Ń^�C�v�ʐݒ�̊g���q���g���悤�ɁB
		�g���q�t�B���^�̊Ǘ���FileExt�N���X�ōs���B
	@date 2005.02.20 novice �g���q���ȗ�������⊮����
*/
bool DlgOpenFile::DoModalOpenDlg(
	LoadInfo* pLoadInfo,
	std::vector<std::tstring>* pFileNames,
	bool bOptions
	)
{
	auto pData = std::make_unique<DlgOpenFileData>();
	pData->bIsSaveDialog = FALSE;	// �ۑ��̃_�C�A���O��

	bool bMultiSelect = pFileNames != NULL;

	// �t�@�C���̎��	2003.05.12 MIK
	FileExt fileExt;
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME3), _T("*.*"));
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME2), _T("*.txt"));
	for (int i=0; i<GetDllShareData().nTypesCount; ++i) {
		const TypeConfigMini* type;
		DocTypeManager().GetTypeConfigMini(TypeConfigNum(i), &type);
		fileExt.AppendExt(type->szTypeName, type->szTypeExts);
	}

	// �����o�̏�����
	pData->bViewMode = pLoadInfo->bViewMode;
	pData->nCharCode = pLoadInfo->eCharCode;	// �����R�[�h��������
	pData->nHelpTopicID = ::FuncID_To_HelpContextID(F_FILEOPEN);	//Stonee, 2001/05/18 �@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
	pData->bUseCharCode = true;
	pData->bUseEol = false;	//	Feb. 9, 2001 genta
	pData->bUseBom = false;	//	Jul. 26, 2003 ryoji

	// �t�@�C���p�X�󂯎��o�b�t�@
	TCHAR* pszPathBuf = new TCHAR[2000];
	auto_strcpy(pszPathBuf, pLoadInfo->filePath); // 2013.05.27 �f�t�H���g�t�@�C������ݒ肷��

	// OPENFILENAME�\���̂̏�����
	InitOfn( &pData->ofn );		// 2005.10.29 ryoji
	pData->pDlgOpenFile = this;
	pData->ofn.lCustData = (LPARAM)(pData.get());
	pData->ofn.hwndOwner = mem->hwndParent;
	pData->ofn.hInstance = SelectLang::getLangRsrcInstance();
	pData->ofn.lpstrFilter = fileExt.GetExtFilter();
	pData->ofn.lpstrFile = pszPathBuf;
	pData->ofn.nMaxFile = 2000;
	pData->ofn.lpstrInitialDir = mem->szInitialDir;
	pData->ofn.Flags = OFN_EXPLORER | OFN_CREATEPROMPT | OFN_FILEMUSTEXIST | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK | OFN_SHOWHELP | OFN_ENABLESIZING;
	if (pData->bViewMode) pData->ofn.Flags |= OFN_READONLY;
	if (bMultiSelect) {
		pData->ofn.Flags |= OFN_ALLOWMULTISELECT;
	}
	pData->ofn.lpstrDefExt = _T("");	// 2005/02/20 novice �g���q���ȗ�������⊮����
	if (!bOptions) {
		pData->ofn.Flags |= OFN_HIDEREADONLY;
		pData->bUseCharCode = false;
	}

	// �J�����g�f�B���N�g����ۑ��B�֐��𔲂���Ƃ��Ɏ����ŃJ�����g�f�B���N�g���͕�������܂��B
	CurrentDirectoryBackupPoint curDirBackup;

	// 2010.08.28 Moca DLL���ǂݍ��܂��̂ňړ�
	ChangeCurrentDirectoryToExeDir();

	// �_�C�A���O�\��
	bool bDlgResult = _GetOpenFileNameRecover( &pData->ofn );
	if (bDlgResult) {
		if (bMultiSelect) {
			pLoadInfo->filePath = _T("");
			if (pData->ofn.nFileOffset < _tcslen( pData->ofn.lpstrFile )) {
				pFileNames->emplace_back(pData->ofn.lpstrFile);
			}else {
				std::tstring path;
				TCHAR* pos = pData->ofn.lpstrFile;
				pos += _tcslen(pos) + 1;
				while (*pos != _T('\0')) {
					path = pData->ofn.lpstrFile;
					path.append(_T("\\"));
					path.append(pos);
					pFileNames->push_back(path);
					pos += _tcslen(pos) + 1;
				}
			}
		}else {
			pLoadInfo->filePath = pData->ofn.lpstrFile;
		}
		pLoadInfo->eCharCode = pData->nCharCode;
		pLoadInfo->bViewMode = pData->bViewMode;
	}else {
		DlgOpenFail();
	}
	delete[] pData->ofn.lpstrFile;
	return bDlgResult;
}

/*! �ۑ��_�C�A���O ���[�_���_�C�A���O�̕\��

	@date 2001.02.09 genta	�����ǉ�
	@date 2003.05.12 MIK �g���q�t�B���^�Ń^�C�v�ʐݒ�̊g���q���g���悤�ɁB
		�g���q�t�B���^�̊Ǘ���FileExt�N���X�ōs���B
	@date 2003.07.26 ryoji BOM�p�����[�^�ǉ�
	@date 2005.02.20 novice �g���q���ȗ�������⊮����
	@date 2006.11.10 ryoji �t�b�N���g���ꍇ�͊g���q�̕⊮�����O�ōs��
		Windows�Ŋ֘A�t���������悤�Ȋg���q���w�肵�ĕۑ�����ƁA�����I��
		�g���q���͂��Ă���̂Ƀf�t�H���g�g���q���⊮����Ă��܂����Ƃ�����B
			��jhoge.abc -> hoge.abc.txt
		���O�ŕ⊮���邱�Ƃł�����������B�i���ۂ̏����̓t�b�N�v���V�[�W���̒��j
*/
bool DlgOpenFile::DoModalSaveDlg(
	SaveInfo* pSaveInfo,
	bool bSimpleMode
	)
{
	auto pData = std::make_unique<DlgOpenFileData>();
	pData->bIsSaveDialog = true;	// �ۑ��̃_�C�A���O��

	// 2003.05.12 MIK
	FileExt fileExt;
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME1), mem->szDefaultWildCard);
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME2), _T("*.txt"));
	fileExt.AppendExtRaw(LS(STR_DLGOPNFL_EXTNAME3), _T("*.*"));

	// �t�@�C�����̏����ݒ�	// 2006.11.10 ryoji
	if (pSaveInfo->filePath[0] == _T('\0')) {
		lstrcpyn(pSaveInfo->filePath, LS(STR_NO_TITLE2), _MAX_PATH);	// ����
	}
	// OPENFILENAME�\���̂̏�����
	InitOfn(&pData->ofn);		// 2005.10.29 ryoji
	pData->pDlgOpenFile = this;
	pData->ofn.lCustData = (LPARAM)(pData.get());
	pData->ofn.hwndOwner = mem->hwndParent;
	pData->ofn.hInstance = SelectLang::getLangRsrcInstance();
	pData->ofn.lpstrFilter = fileExt.GetExtFilter();
	pData->ofn.lpstrFile = pSaveInfo->filePath;	// 2005/02/20 novice �f�t�H���g�̃t�@�C�����͉����ݒ肵�Ȃ�
	pData->ofn.nMaxFile = _MAX_PATH;
	pData->ofn.lpstrInitialDir = mem->szInitialDir;
	pData->ofn.Flags = OFN_CREATEPROMPT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_SHOWHELP | OFN_ENABLESIZING;
	if (!bSimpleMode) {
		pData->ofn.Flags = pData->ofn.Flags | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;
		pData->ofn.Flags &= ~OFN_OVERWRITEPROMPT;	// 2006.11.10 ryoji �㏑���m�F���t�b�N�̒��Ŏ��O�ŏ�������
	}

// 2005/02/20 novice �g���q���ȗ�������⊮����
//	pData->ofn.lpstrDefExt = _T("");
	pData->ofn.lpstrDefExt = (pData->ofn.Flags & OFN_ENABLEHOOK) ? NULL: _T("");	// 2006.11.10 ryoji �t�b�N���g���Ƃ��͎��O�Ŋg���q��⊮����

	// �J�����g�f�B���N�g����ۑ��B�֐����甲����Ƃ��Ɏ����ŃJ�����g�f�B���N�g���͕��������B
	CurrentDirectoryBackupPoint curDirBackup;

	// 2010.08.28 Moca DLL���ǂݍ��܂��̂ňړ�
	ChangeCurrentDirectoryToExeDir();

	pData->nCharCode = pSaveInfo->eCharCode;

	// From Here Feb. 9, 2001 genta
	if (!bSimpleMode) {
		pData->eol = pSaveInfo->eol;	//	�����l�́u���s�R�[�h��ۑ��v�ɌŒ� // 2013.05.27 �����l���w��
		pData->bUseEol = true;
	}else {
		pData->bUseEol = false;
	}

	// To Here Feb. 9, 2001 genta
	// Jul. 26, 2003 ryoji BOM�ݒ�
	if (!bSimpleMode) {
		pData->bBom = pSaveInfo->bBomExist;
		pData->bUseBom = true;
		pData->bUseCharCode = true;
	}else {
		pData->bUseBom = false;
		pData->bUseCharCode = false;
	}

	pData->nHelpTopicID = ::FuncID_To_HelpContextID(F_FILESAVEAS_DIALOG);	//Stonee, 2001/05/18 �@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
	if (GetSaveFileNameRecover(&pData->ofn)) {
		pSaveInfo->filePath = pData->ofn.lpstrFile;
		if (pData->ofn.Flags & OFN_ENABLEHOOK) {
			lstrcpyn(pSaveInfo->filePath, pData->szPath, _MAX_PATH);	// ���O�Ŋg���q�̕⊮���s�����Ƃ��̃t�@�C���p�X	// 2006.11.10 ryoji
		}
		pSaveInfo->eCharCode = pData->nCharCode;

		// Feb. 9, 2001 genta
		if (pData->bUseEol) {
			pSaveInfo->eol = pData->eol;
		}
		// Jul. 26, 2003 ryoji BOM�ݒ�
		if (pData->bUseBom) {
			pSaveInfo->bBomExist = pData->bBom;
		}
		return true;
	}else {
		// May 29, 2004 genta �֐��ɂ܂Ƃ߂�
		DlgOpenFail();
		return false;
	}
}

/*! @brief �R�����_�C�A���O�{�b�N�X���s����

	�R�����_�C�A���O�{�b�N�X����FALSE���Ԃ��ꂽ�ꍇ��
	�G���[�����𒲂ׂăG���[�Ȃ烁�b�Z�[�W���o���D
	
	@author genta
	@date 2004.05.29 genta ���X�������������܂Ƃ߂�
*/
void DlgOpenFile::DlgOpenFail(void)
{
	const TCHAR* pszError;
	DWORD dwError = ::CommDlgExtendedError();
	if (dwError == 0) {
		// ���[�U�L�����Z���ɂ��
		return;
	}
	
	switch (dwError) {
	case CDERR_DIALOGFAILURE  : pszError = _T("CDERR_DIALOGFAILURE  "); break;
	case CDERR_FINDRESFAILURE : pszError = _T("CDERR_FINDRESFAILURE "); break;
	case CDERR_NOHINSTANCE    : pszError = _T("CDERR_NOHINSTANCE    "); break;
	case CDERR_INITIALIZATION : pszError = _T("CDERR_INITIALIZATION "); break;
	case CDERR_NOHOOK         : pszError = _T("CDERR_NOHOOK         "); break;
	case CDERR_LOCKRESFAILURE : pszError = _T("CDERR_LOCKRESFAILURE "); break;
	case CDERR_NOTEMPLATE     : pszError = _T("CDERR_NOTEMPLATE     "); break;
	case CDERR_LOADRESFAILURE : pszError = _T("CDERR_LOADRESFAILURE "); break;
	case CDERR_STRUCTSIZE     : pszError = _T("CDERR_STRUCTSIZE     "); break;
	case CDERR_LOADSTRFAILURE : pszError = _T("CDERR_LOADSTRFAILURE "); break;
	case FNERR_BUFFERTOOSMALL : pszError = _T("FNERR_BUFFERTOOSMALL "); break;
	case CDERR_MEMALLOCFAILURE: pszError = _T("CDERR_MEMALLOCFAILURE"); break;
	case FNERR_INVALIDFILENAME: pszError = _T("FNERR_INVALIDFILENAME"); break;
	case CDERR_MEMLOCKFAILURE : pszError = _T("CDERR_MEMLOCKFAILURE "); break;
	case FNERR_SUBCLASSFAILURE: pszError = _T("FNERR_SUBCLASSFAILURE"); break;
	default: pszError = _T("UNKNOWN_ERRORCODE"); break;
	}

	ErrorBeep();
	TopErrorMessage( mem->hwndParent,
		LS(STR_DLGOPNFL_ERR1),
		pszError
	);
}

/*! OPENFILENAME ������

	OPENFILENAME �� DlgOpenFile �N���X�p�̏����K��l��ݒ肷��

	@author ryoji
	@date 2005.10.29
*/
void DlgOpenFile::InitOfn(OPENFILENAMEZ* ofn)
{
	memset_raw(ofn, 0, sizeof(*ofn));

	ofn->lStructSize = IsWinV5forOfn() ? sizeof(OPENFILENAMEZ): OPENFILENAME_SIZE_VERSION_400;
	ofn->lpfnHook = OFNHookProc;
	ofn->lpTemplateName = MAKEINTRESOURCE(IDD_FILEOPEN);	// <-_T("IDD_FILEOPEN"); 2008/7/26 Uchi
	ofn->nFilterIndex = 1;	//Jul. 09, 2001 JEPRO		// �u�J���v�ł̍ŏ��̃��C���h�J�[�h
}

/*! �������C�A�E�g�ݒ菈��

	�ǉ��R���g���[���̃��C�A�E�g��ύX����

	@param hwndOpenDlg [in]		�t�@�C���_�C�A���O�̃E�B���h�E�n���h��
	@param hwndDlg [in]			�q�_�C�A���O�̃E�B���h�E�n���h��
	@param hwndBaseCtrl [in]	�ړ���R���g���[���i�t�@�C�����{�b�N�X�ƍ��[�����킹��R���g���[���j�̃E�B���h�E�n���h��

	@author ryoji
	@date 2005.11.02
*/
void DlgOpenFile::InitLayout(
	HWND hwndOpenDlg,
	HWND hwndDlg,
	HWND hwndBaseCtrl
	)
{
	HWND hwndFilelabel;
	// �t�@�C�������x���ƃt�@�C�����{�b�N�X���擾����
	if (!::IsWindow(hwndFilelabel = ::GetDlgItem(hwndOpenDlg, stc3)))		// �t�@�C�������x��
		return;
	HWND hwndFilebox;
	if (!::IsWindow(hwndFilebox = ::GetDlgItem(hwndOpenDlg, cmb13))) {		// �t�@�C�����R���{�iWindows 2000�^�C�v�j
		if (!::IsWindow(hwndFilebox = ::GetDlgItem(hwndOpenDlg, edt1)))		// �t�@�C�����G�f�B�b�g�i���K�V�[�^�C�v�j
			return;
	}

	// �R���g���[���̊�ʒu�A�ړ��ʂ����肷��
	RECT rcBase;
	RECT rc;
	::GetWindowRect(hwndFilelabel, &rc);
	int nLeft = rc.left;						// ���[�ɑ�����R���g���[���̈ʒu
	::GetWindowRect(hwndFilebox, &rc);
	::GetWindowRect(hwndBaseCtrl, &rcBase);
	int nShift = rc.left - rcBase.left;			// ���[�ȊO�̃R���g���[���̉E�����ւ̑��Έړ���

	// �ǉ��R���g���[�������ׂĈړ�����
	// �E��R���g���[���������ɂ�����̂̓t�@�C�������x���ɍ��킹�č��[�Ɉړ�
	// �E���̑��͈ړ���R���g���[���i�t�@�C�����{�b�N�X�ƍ��[�����킹��R���g���[���j�Ɠ��������E�����֑��Έړ�
	HWND hwndCtrl = ::GetWindow(hwndDlg, GW_CHILD);
	while (hwndCtrl) {
		if (::GetDlgCtrlID(hwndCtrl) != stc32) {
			::GetWindowRect(hwndCtrl, &rc);
			POINT pt;
			pt.x = (rc.right < rcBase.left)? nLeft: rc.left + nShift;
			pt.y = rc.top;
			::ScreenToClient(hwndDlg, &pt);
			::SetWindowPos(hwndCtrl, 0, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
		}
		hwndCtrl = ::GetWindow(hwndCtrl, GW_HWNDNEXT);
	}

	// �W���R���g���[���̃v���[�X�t�H���_�istc32�j�Ǝq�_�C�A���O�̕����I�[�v���_�C�A���O�̕��ɂ��킹��
	//     WM_INITDIALOG �𔲂���Ƃ���ɃI�[�v���_�C�A���O���Ō��݂̈ʒu�֌W���烌�C�A�E�g�������s����
	//     �����ňȉ��̏���������Ă����Ȃ��ƃR���g���[�����Ӑ}���Ȃ��ꏊ�ɓ����Ă��܂����Ƃ�����
	//     �i�Ⴆ�΁ABOM �̃`�F�b�N�{�b�N�X����ʊO�ɔ��ł��܂��Ȃǁj

	// �I�[�v���_�C�A���O�̃N���C�A���g�̈�̕����擾����
	::GetClientRect(hwndOpenDlg, &rc);
	int nWidth = rc.right - rc.left;

	// �W���R���g���[���v���[�X�t�H���_�̕���ύX����
	hwndCtrl = ::GetDlgItem(hwndDlg, stc32);
	::GetWindowRect(hwndCtrl, &rc);
	::SetWindowPos(hwndCtrl, 0, 0, 0, nWidth, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

	// �q�_�C�A���O�̕���ύX����
	// ������ SetWindowPos() �̒��� WM_SIZE ����������
	::GetWindowRect(hwndDlg, &rc);
	::SetWindowPos(hwndDlg, 0, 0, 0, nWidth, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
}


/*! ���g���C�@�\�t�� GetOpenFileName
	@author Moca
	@date 2006.09.03 �V�K�쐬
*/
bool DlgOpenFile::_GetOpenFileNameRecover(OPENFILENAMEZ* ofn)
{
	BOOL bRet = ::GetOpenFileName(ofn);
	if (!bRet) {
		if (::CommDlgExtendedError() == FNERR_INVALIDFILENAME) {
			_tcscpy(ofn->lpstrFile, _T(""));
			ofn->lpstrInitialDir = _T("");
			bRet = ::GetOpenFileName(ofn);
		}
	}
	return bRet != FALSE;
}

/*! ���g���C�@�\�t�� GetSaveFileName
	@author Moca
	@date 2006.09.03 �V�K�쐬
*/
bool DlgOpenFile::GetSaveFileNameRecover(OPENFILENAMEZ* ofn)
{
	BOOL bRet = ::GetSaveFileName(ofn);
	if (!bRet) {
		if (::CommDlgExtendedError() == FNERR_INVALIDFILENAME) {
			_tcscpy(ofn->lpstrFile, _T(""));
			ofn->lpstrInitialDir = _T("");
			bRet = ::GetSaveFileName(ofn);
		}
	}
	return bRet != FALSE;
}

