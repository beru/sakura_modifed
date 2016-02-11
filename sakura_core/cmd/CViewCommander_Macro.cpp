/*!	@file
@brief ViewCommander�N���X�̃R�}���h(�}�N���n)�֐��Q

	2012/12/20	CViewCommander.cpp���番��
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
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
#include "macro/CSMacroMgr.h"
#include "dlg/CDlgExec.h"
#include "dlg/CDlgOpenFile.h"
#include "CEditApp.h"
#include "recent/CRecentCurDir.h"
#include "util/module.h"
#include "env/CShareData.h"
#include "env/CSakuraEnvironment.h"


// �L�[�}�N���̋L�^�J�n�^�I��
void ViewCommander::Command_RECKEYMACRO(void)
{
	auto& sFlags = GetDllShareData().m_flags;
	if (sFlags.m_bRecordingKeyMacro) {									// �L�[�{�[�h�}�N���̋L�^��
		sFlags.m_bRecordingKeyMacro = FALSE;
		sFlags.m_hwndRecordingKeyMacro = NULL;							// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
		//@@@ 2002.1.24 YAZAKI �L�[�}�N�����}�N���p�t�H���_�ɁuRecKey.mac�v�Ƃ������ŕۑ�
		TCHAR szInitDir[MAX_PATH];
		int nRet;
		// 2003.06.23 Moca �L�^�p�L�[�}�N���̃t���p�X��ShareData�o�R�Ŏ擾
		nRet = ShareData::getInstance()->GetMacroFilename(-1, szInitDir, MAX_PATH);
		auto& csMacro = GetDllShareData().m_common.m_sMacro;
		if (nRet <= 0) {
			ErrorMessage(m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD24), nRet);
			return;
		}else {
			_tcscpy(csMacro.m_szKeyMacroFileName, szInitDir);
		}
		//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
		int nSaveResult = m_pcSMacroMgr->Save(
			STAND_KEYMACRO,
			G_AppInstance(),
			csMacro.m_szKeyMacroFileName
		);
		if (!nSaveResult) {
			ErrorMessage(	m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD25), csMacro.m_szKeyMacroFileName);
		}
	}else {
		sFlags.m_bRecordingKeyMacro = TRUE;
		sFlags.m_hwndRecordingKeyMacro = GetMainWindow();	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
		// �L�[�}�N���̃o�b�t�@���N���A����
		//@@@ 2002.1.24 m_CKeyMacroMgr��CEditDoc�ֈړ�
		//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
		m_pcSMacroMgr->Clear(STAND_KEYMACRO);
//		GetDocument()->m_CKeyMacroMgr.ClearAll();
//		GetDllShareData().m_CKeyMacroMgr.Clear();
	}
	// �e�E�B���h�E�̃^�C�g�����X�V
	GetEditWindow()->UpdateCaption();

	// �L�����b�g�̍s���ʒu��\������
	GetCaret().ShowCaretPosInfo();
}


// �L�[�}�N���̕ۑ�
void ViewCommander::Command_SAVEKEYMACRO(void)
{
	auto& sFlags = GetDllShareData().m_flags;
	sFlags.m_bRecordingKeyMacro = FALSE;
	sFlags.m_hwndRecordingKeyMacro = NULL;	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E

	// Jun. 16, 2002 genta
	if (!m_pcSMacroMgr->IsSaveOk()) {
		// �ۑ��s��
		ErrorMessage(m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD26));
	}

	TCHAR szPath[_MAX_PATH + 1];
	TCHAR szInitDir[_MAX_PATH + 1];
	szPath[0] = 0;
	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	auto& macroFolder = GetDllShareData().m_common.m_sMacro.m_szMACROFOLDER;
	if (_IS_REL_PATH(macroFolder)) {
		GetInidirOrExedir(szInitDir, macroFolder);
	}else {
		_tcscpy(szInitDir, macroFolder);	// �}�N���p�t�H���_
	}
	// �t�@�C���I�[�v���_�C�A���O�̏�����
	DlgOpenFile	cDlgOpenFile;
	cDlgOpenFile.Create(
		G_AppInstance(),
		m_pCommanderView->GetHwnd(),
		_T("*.mac"),
		szInitDir
	);
	if (!cDlgOpenFile.DoModal_GetSaveFileName(szPath)) {
		return;
	}
	// �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ���
	// [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt]
//	::SplitPath_FolderAndFile(szPath, macroFolder, NULL);
//	wcscat(macroFolder, L"\\");

	// �L�[�{�[�h�}�N���̕ۑ�
	//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
	//@@@ 2002.1.24 YAZAKI
	if (!m_pcSMacroMgr->Save(STAND_KEYMACRO, G_AppInstance(), szPath)) {
		ErrorMessage(m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD27), szPath);
	}
	return;
}


/*! �L�[�}�N���̓ǂݍ���
	@date 2005.02.20 novice �f�t�H���g�̊g���q�ύX
 */
void ViewCommander::Command_LOADKEYMACRO(void)
{
	auto& sFlags = GetDllShareData().m_flags;
	sFlags.m_bRecordingKeyMacro = FALSE;
	sFlags.m_hwndRecordingKeyMacro = NULL;	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E

	TCHAR szPath[_MAX_PATH + 1];
	TCHAR szInitDir[_MAX_PATH + 1];
	const TCHAR* pszFolder = GetDllShareData().m_common.m_sMacro.m_szMACROFOLDER;
	szPath[0] = 0;
	
	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	if (_IS_REL_PATH(pszFolder)) {
		GetInidirOrExedir(szInitDir, pszFolder);
	}else {
		_tcscpy_s(szInitDir, pszFolder);	// �}�N���p�t�H���_
	}
	// �t�@�C���I�[�v���_�C�A���O�̏�����
	DlgOpenFile cDlgOpenFile;
	cDlgOpenFile.Create(
		G_AppInstance(),
		m_pCommanderView->GetHwnd(),
// 2005/02/20 novice �f�t�H���g�̊g���q�ύX
// 2005/07/13 novice ���l�ȃ}�N�����T�|�[�g���Ă���̂Ńf�t�H���g�͑S�ĕ\���ɂ���
		_T("*.*"),
		szInitDir
	);
	if (!cDlgOpenFile.DoModal_GetOpenFileName(szPath)) {
		return;
	}

	// �L�[�{�[�h�}�N���̓ǂݍ���
	//@@@ 2002.1.24 YAZAKI �ǂݍ��݂Ƃ������A�t�@�C�������R�s�[���邾���B���s���O�ɓǂݍ���
	_tcscpy(GetDllShareData().m_common.m_sMacro.m_szKeyMacroFileName, szPath);
//	GetDllShareData().m_CKeyMacroMgr.LoadKeyMacro(G_AppInstance(), m_pCommanderView->GetHwnd(), szPath);
	return;
}


// �L�[�}�N���̎��s
void ViewCommander::Command_EXECKEYMACRO(void)
{
	auto& sFlags = GetDllShareData().m_flags;
	//@@@ 2002.1.24 YAZAKI �L�^���͏I�����Ă�����s
	if (sFlags.m_bRecordingKeyMacro) {
		Command_RECKEYMACRO();
	}
	sFlags.m_bRecordingKeyMacro = FALSE;
	sFlags.m_hwndRecordingKeyMacro = NULL;	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E

	// �L�[�{�[�h�}�N���̎��s
	//@@@ 2002.1.24 YAZAKI
	auto& csMacro = GetDllShareData().m_common.m_sMacro;
	if (csMacro.m_szKeyMacroFileName[0]) {
		// �t�@�C�����ۑ�����Ă�����
		//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
		BOOL bLoadResult = m_pcSMacroMgr->Load(
			STAND_KEYMACRO,
			G_AppInstance(),
			csMacro.m_szKeyMacroFileName,
			NULL
		);
		if (!bLoadResult) {
			ErrorMessage(m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD28), csMacro.m_szKeyMacroFileName);
		}else {
			// 2007.07.20 genta : flags�I�v�V�����ǉ�
			m_pcSMacroMgr->Exec(STAND_KEYMACRO, G_AppInstance(), m_pCommanderView, 0);
		}
	}
	return;
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
		const TCHAR* pszFolder = GetDllShareData().m_common.m_sMacro.m_szMACROFOLDER;

		// �t�@�C���I���_�C�A���O�̏����t�H���_
		TCHAR szInitDir[_MAX_PATH + 1];
		if (_IS_REL_PATH(pszFolder)) {
			GetInidirOrExedir(szInitDir, pszFolder);
		}else {
			_tcscpy_s(szInitDir, pszFolder);	// �}�N���p�t�H���_
		}
		// �t�@�C���I�[�v���_�C�A���O�̏�����
		DlgOpenFile cDlgOpenFile;
		cDlgOpenFile.Create(
			G_AppInstance(),
			m_pCommanderView->GetHwnd(),
			_T("*.*"),
			szInitDir
		);
		if (!cDlgOpenFile.DoModal_GetOpenFileName(szPath)) {
			return;
		}
		pszPath = szPath;
		pszType = NULL;
	}

	auto& sFlags = GetDllShareData().m_flags;
	// �L�[�}�N���L�^���̏ꍇ�A�ǉ�����
	if (sFlags.m_bRecordingKeyMacro &&						// �L�[�{�[�h�}�N���̋L�^��
		sFlags.m_hwndRecordingKeyMacro == GetMainWindow()	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
	) {
		LPARAM lparams[] = {(LPARAM)pszPath, 0, 0, 0};
		m_pcSMacroMgr->Append(STAND_KEYMACRO, F_EXECEXTMACRO, lparams, m_pCommanderView);

		// �L�[�}�N���̋L�^���ꎞ��~����
		sFlags.m_bRecordingKeyMacro = FALSE;
		hwndRecordingKeyMacro = sFlags.m_hwndRecordingKeyMacro;
		sFlags.m_hwndRecordingKeyMacro = NULL;	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
	}

	// �Â��ꎞ�}�N���̑ޔ�
	MacroManagerBase* oldMacro = m_pcSMacroMgr->SetTempMacro(NULL);

	BOOL bLoadResult = m_pcSMacroMgr->Load(
		TEMP_KEYMACRO,
		G_AppInstance(),
		pszPath,
		pszType
	);
	if (!bLoadResult) {
		ErrorMessage(m_pCommanderView->GetHwnd(), LS(STR_ERR_MACROERR1), pszPath);
	}else {
		m_pcSMacroMgr->Exec(TEMP_KEYMACRO, G_AppInstance(), m_pCommanderView, FA_NONRECORD | FA_FROMMACRO);
	}

	// �I�������J��
	m_pcSMacroMgr->Clear(TEMP_KEYMACRO);
	if (oldMacro) {
		m_pcSMacroMgr->SetTempMacro(oldMacro);
	}

	// �L�[�}�N���L�^���������ꍇ�͍ĊJ����
	if (hwndRecordingKeyMacro) {
		sFlags.m_bRecordingKeyMacro = TRUE;
		sFlags.m_hwndRecordingKeyMacro = hwndRecordingKeyMacro;	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
	}
	return;
}


/*! �O���R�}���h���s�_�C�A���O�\��
	@date 2002.02.02 YAZAKI.
*/
void ViewCommander::Command_EXECCOMMAND_DIALOG(void)
{
	DlgExec cDlgExec;

	// ���[�h���X�_�C�A���O�̕\��
	if (!cDlgExec.DoModal(G_AppInstance(), m_pCommanderView->GetHwnd(), 0)) {
		return;
	}

	m_pCommanderView->AddToCmdArr(cDlgExec.m_szCommand);
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
	HandleCommand(F_EXECMD, true, (LPARAM)cmd_string, (LPARAM)(GetDllShareData().m_nExecFlgOpt), (LPARAM)pszDir, 0);	// �O���R�}���h���s�R�}���h�̔��s
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
	m_pCommanderView->ExecCmd(buf2.c_str(), nFlgOpt, (pszCurDir ? buf3.c_str() : NULL));
	// To Here Aug. 21, 2001 genta
	return;
}

