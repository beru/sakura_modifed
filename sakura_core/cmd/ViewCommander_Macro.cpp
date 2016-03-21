/*!	@file
@brief ViewCommander�N���X�̃R�}���h(�}�N���n)�֐��Q

	2012/12/20	ViewCommander.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, MIK, Stonee, Misaka, asa-o, novice, hor, YAZAKI
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2003, Moca
	Copyright (C) 2005, novice
	Copyright (C) 2006, maru
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2008, syat

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
#include "macro/SMacroMgr.h"
#include "dlg/DlgExec.h"
#include "dlg/DlgOpenFile.h"
#include "EditApp.h"
#include "recent/RecentCurDir.h"
#include "util/module.h"
#include "env/ShareData.h"
#include "env/SakuraEnvironment.h"


// �L�[�}�N���̋L�^�J�n�^�I��
void ViewCommander::Command_RECKEYMACRO(void)
{
	auto& flags = GetDllShareData().flags;
	if (flags.bRecordingKeyMacro) {									// �L�[�{�[�h�}�N���̋L�^��
		flags.bRecordingKeyMacro = false;
		flags.hwndRecordingKeyMacro = NULL;							// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
		//@@@ 2002.1.24 YAZAKI �L�[�}�N�����}�N���p�t�H���_�ɁuRecKey.mac�v�Ƃ������ŕۑ�
		TCHAR szInitDir[MAX_PATH];
		int nRet;
		// 2003.06.23 Moca �L�^�p�L�[�}�N���̃t���p�X��ShareData�o�R�Ŏ擾
		nRet = ShareData::getInstance().GetMacroFilename(-1, szInitDir, MAX_PATH);
		auto& csMacro = GetDllShareData().common.macro;
		if (nRet <= 0) {
			ErrorMessage(m_view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD24), nRet);
			return;
		}else {
			_tcscpy(csMacro.szKeyMacroFileName, szInitDir);
		}
		//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
		int nSaveResult = m_pSMacroMgr->Save(
			STAND_KEYMACRO,
			G_AppInstance(),
			csMacro.szKeyMacroFileName
		);
		if (!nSaveResult) {
			ErrorMessage(	m_view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD25), csMacro.szKeyMacroFileName);
		}
	}else {
		flags.bRecordingKeyMacro = true;
		flags.hwndRecordingKeyMacro = GetMainWindow();	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
		// �L�[�}�N���̃o�b�t�@���N���A����
		//@@@ 2002.1.24 m_CKeyMacroMgr��CEditDoc�ֈړ�
		//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
		m_pSMacroMgr->Clear(STAND_KEYMACRO);
//		GetDocument()->m_CKeyMacroMgr.ClearAll();
//		GetDllShareData().m_CKeyMacroMgr.Clear();
	}
	// �e�E�B���h�E�̃^�C�g�����X�V
	GetEditWindow().UpdateCaption();

	// �L�����b�g�̍s���ʒu��\������
	GetCaret().ShowCaretPosInfo();
}


// �L�[�}�N���̕ۑ�
void ViewCommander::Command_SAVEKEYMACRO(void)
{
	auto& flags = GetDllShareData().flags;
	flags.bRecordingKeyMacro = false;
	flags.hwndRecordingKeyMacro = NULL;	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E

	// Jun. 16, 2002 genta
	if (!m_pSMacroMgr->IsSaveOk()) {
		// �ۑ��s��
		ErrorMessage(m_view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD26));
	}

	TCHAR szPath[_MAX_PATH + 1];
	TCHAR szInitDir[_MAX_PATH + 1];
	szPath[0] = 0;
	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	auto& macroFolder = GetDllShareData().common.macro.szMACROFOLDER;
	if (_IS_REL_PATH(macroFolder)) {
		GetInidirOrExedir(szInitDir, macroFolder);
	}else {
		_tcscpy(szInitDir, macroFolder);	// �}�N���p�t�H���_
	}
	// �t�@�C���I�[�v���_�C�A���O�̏�����
	DlgOpenFile	dlgOpenFile;
	dlgOpenFile.Create(
		G_AppInstance(),
		m_view.GetHwnd(),
		_T("*.mac"),
		szInitDir
	);
	if (!dlgOpenFile.DoModal_GetSaveFileName(szPath)) {
		return;
	}
	// �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ���
	// [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt]
//	::SplitPath_FolderAndFile(szPath, macroFolder, NULL);
//	wcscat(macroFolder, L"\\");

	// �L�[�{�[�h�}�N���̕ۑ�
	//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
	//@@@ 2002.1.24 YAZAKI
	if (!m_pSMacroMgr->Save(STAND_KEYMACRO, G_AppInstance(), szPath)) {
		ErrorMessage(m_view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD27), szPath);
	}
}


/*! �L�[�}�N���̓ǂݍ���
	@date 2005.02.20 novice �f�t�H���g�̊g���q�ύX
 */
void ViewCommander::Command_LOADKEYMACRO(void)
{
	auto& flags = GetDllShareData().flags;
	flags.bRecordingKeyMacro = false;
	flags.hwndRecordingKeyMacro = NULL;	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E

	TCHAR szPath[_MAX_PATH + 1];
	TCHAR szInitDir[_MAX_PATH + 1];
	const TCHAR* pszFolder = GetDllShareData().common.macro.szMACROFOLDER;
	szPath[0] = 0;
	
	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	if (_IS_REL_PATH(pszFolder)) {
		GetInidirOrExedir(szInitDir, pszFolder);
	}else {
		_tcscpy_s(szInitDir, pszFolder);	// �}�N���p�t�H���_
	}
	// �t�@�C���I�[�v���_�C�A���O�̏�����
	DlgOpenFile dlgOpenFile;
	dlgOpenFile.Create(
		G_AppInstance(),
		m_view.GetHwnd(),
// 2005/02/20 novice �f�t�H���g�̊g���q�ύX
// 2005/07/13 novice ���l�ȃ}�N�����T�|�[�g���Ă���̂Ńf�t�H���g�͑S�ĕ\���ɂ���
		_T("*.*"),
		szInitDir
	);
	if (!dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
		return;
	}

	// �L�[�{�[�h�}�N���̓ǂݍ���
	//@@@ 2002.1.24 YAZAKI �ǂݍ��݂Ƃ������A�t�@�C�������R�s�[���邾���B���s���O�ɓǂݍ���
	_tcscpy(GetDllShareData().common.macro.szKeyMacroFileName, szPath);
//	GetDllShareData().m_CKeyMacroMgr.LoadKeyMacro(G_AppInstance(), m_view.GetHwnd(), szPath);
}

// �L�[�}�N���̎��s
void ViewCommander::Command_EXECKEYMACRO(void)
{
	auto& flags = GetDllShareData().flags;
	//@@@ 2002.1.24 YAZAKI �L�^���͏I�����Ă�����s
	if (flags.bRecordingKeyMacro) {
		Command_RECKEYMACRO();
	}
	flags.bRecordingKeyMacro = false;
	flags.hwndRecordingKeyMacro = NULL;	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E

	// �L�[�{�[�h�}�N���̎��s
	//@@@ 2002.1.24 YAZAKI
	auto& csMacro = GetDllShareData().common.macro;
	if (csMacro.szKeyMacroFileName[0]) {
		// �t�@�C�����ۑ�����Ă�����
		//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
		bool bLoadResult = m_pSMacroMgr->Load(
			m_view,
			STAND_KEYMACRO,
			G_AppInstance(),
			csMacro.szKeyMacroFileName,
			NULL
		);
		if (!bLoadResult) {
			ErrorMessage(m_view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD28), csMacro.szKeyMacroFileName);
		}else {
			// 2007.07.20 genta : flags�I�v�V�����ǉ�
			m_pSMacroMgr->Exec(STAND_KEYMACRO, G_AppInstance(), m_view, 0);
		}
	}
}


/*! ���O���w�肵�ă}�N�����s
	@param pszPath	�}�N���̃t�@�C���p�X�A�܂��̓}�N���̃R�[�h�B
	@param pszType	��ʁBNULL�̏ꍇ�t�@�C���w��A����ȊO�̏ꍇ�͌���̊g���q���w��

	@date 2008.10.23 syat �V�K�쐬
	@date 2008.12.21 syat �����u��ʁv��ǉ�
 */
void ViewCommander::Command_EXECEXTMACRO(const WCHAR* pszPathW, const WCHAR* pszTypeW)
{
	TCHAR			szPath[_MAX_PATH + 1];
	const TCHAR*	pszPath = NULL;				// ��1������TCHAR*�ɕϊ�����������
	const TCHAR*	pszType = NULL;				// ��2������TCHAR*�ɕϊ�����������
	HWND			hwndRecordingKeyMacro = NULL;

	if (pszPathW) {
		// to_tchar()�Ŏ擾�����������delete���Ȃ����ƁB
		pszPath = to_tchar(pszPathW);
		pszType = to_tchar(pszTypeW);

	}else {
		// �t�@�C�����w�肳��Ă��Ȃ��ꍇ�A�_�C�A���O��\������
		szPath[0] = 0;
		// �}�N���t�H���_
		const TCHAR* pszFolder = GetDllShareData().common.macro.szMACROFOLDER;
		// �t�@�C���I���_�C�A���O�̏����t�H���_
		TCHAR szInitDir[_MAX_PATH + 1];
		if (_IS_REL_PATH(pszFolder)) {
			GetInidirOrExedir(szInitDir, pszFolder);
		}else {
			_tcscpy_s(szInitDir, pszFolder);	// �}�N���p�t�H���_
		}
		// �t�@�C���I�[�v���_�C�A���O�̏�����
		DlgOpenFile dlgOpenFile;
		dlgOpenFile.Create(
			G_AppInstance(),
			m_view.GetHwnd(),
			_T("*.*"),
			szInitDir
		);
		if (!dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
			return;
		}
		pszPath = szPath;
		pszType = NULL;
	}

	auto& flags = GetDllShareData().flags;
	// �L�[�}�N���L�^���̏ꍇ�A�ǉ�����
	if (flags.bRecordingKeyMacro &&						// �L�[�{�[�h�}�N���̋L�^��
		flags.hwndRecordingKeyMacro == GetMainWindow()	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
	) {
		LPARAM lparams[] = {(LPARAM)pszPath, 0, 0, 0};
		m_pSMacroMgr->Append(STAND_KEYMACRO, F_EXECEXTMACRO, lparams, m_view);
		// �L�[�}�N���̋L�^���ꎞ��~����
		flags.bRecordingKeyMacro = false;
		hwndRecordingKeyMacro = flags.hwndRecordingKeyMacro;
		flags.hwndRecordingKeyMacro = NULL;	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
	}

	// �Â��ꎞ�}�N���̑ޔ�
	MacroManagerBase* oldMacro = m_pSMacroMgr->SetTempMacro(NULL);
	bool bLoadResult = m_pSMacroMgr->Load(
		m_view,
		TEMP_KEYMACRO,
		G_AppInstance(),
		pszPath,
		pszType
	);
	if (!bLoadResult) {
		ErrorMessage(m_view.GetHwnd(), LS(STR_ERR_MACROERR1), pszPath);
	}else {
		m_pSMacroMgr->Exec(TEMP_KEYMACRO, G_AppInstance(), m_view, FA_NONRECORD | FA_FROMMACRO);
	}

	// �I�������J��
	m_pSMacroMgr->Clear(TEMP_KEYMACRO);
	if (oldMacro) {
		m_pSMacroMgr->SetTempMacro(oldMacro);
	}

	// �L�[�}�N���L�^���������ꍇ�͍ĊJ����
	if (hwndRecordingKeyMacro) {
		flags.bRecordingKeyMacro = true;
		flags.hwndRecordingKeyMacro = hwndRecordingKeyMacro;	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
	}
}


/*! �O���R�}���h���s�_�C�A���O�\��
	@date 2002.02.02 YAZAKI.
*/
void ViewCommander::Command_EXECCOMMAND_DIALOG(void)
{
	DlgExec cDlgExec;

	// ���[�h���X�_�C�A���O�̕\��
	if (!cDlgExec.DoModal(G_AppInstance(), m_view.GetHwnd(), 0)) {
		return;
	}

	m_view.AddToCmdArr(cDlgExec.m_szCommand);
	const WCHAR* cmd_string = to_wchar(cDlgExec.m_szCommand);
	const WCHAR* curDir = to_wchar(cDlgExec.m_szCurDir);
	const WCHAR* pszDir = curDir;
	if (curDir[0] == L'\0') {
		pszDir = NULL;
	}else {
		RecentCurDir cRecentCurDir;
		cRecentCurDir.AppendItem(cDlgExec.m_szCurDir);
		cRecentCurDir.Terminate();
	}

	//HandleCommand(F_EXECMD, true, (LPARAM)cmd_string, 0, 0, 0);	// �O���R�}���h���s�R�}���h�̔��s
	HandleCommand(F_EXECMD, true, (LPARAM)cmd_string, (LPARAM)(GetDllShareData().nExecFlgOpt), (LPARAM)pszDir, 0);	// �O���R�}���h���s�R�}���h�̔��s
}


// �O���R�}���h���s
// Sept. 20, 2000 JEPRO  ����CMMAND��COMMAND�ɕύX
// Oct. 9, 2001   genta  �}�N���Ή��̂��߈����ǉ�
// 2002.2.2       YAZAKI �_�C�A���O�Ăяo�����ƃR�}���h���s���𕪗�
//void CEditView::Command_EXECCOMMAND(const char* cmd_string)
void ViewCommander::Command_EXECCOMMAND(
	LPCWSTR cmd_string,
	const int nFlgOpt,
	LPCWSTR pszCurDir
	)	// 2006.12.03 maru �����̊g��
{
	// From Here Aug. 21, 2001 genta
	// �p�����[�^�u�� (���b��)
	const int bufmax = 1024;
	wchar_t buf[bufmax + 1];
	SakuraEnvironment::ExpandParameter(cmd_string, buf, bufmax);

	// �q�v���Z�X�̕W���o�͂����_�C���N�g����
	std::tstring buf2 = to_tchar(buf);
	std::tstring buf3;
	if (pszCurDir) {
		buf3 = to_tchar(pszCurDir);
	}
	m_view.ExecCmd(buf2.c_str(), nFlgOpt, (pszCurDir ? buf3.c_str() : NULL));
	// To Here Aug. 21, 2001 genta
}

