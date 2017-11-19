#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "macro/SMacroMgr.h"
#include "dlg/DlgExec.h"
#include "dlg/DlgOpenFile.h"
#include "EditApp.h"
#include "recent/RecentCurDir.h"
#include "util/module.h"
#include "env/ShareData.h"
#include "env/SakuraEnvironment.h"

// ViewCommander�N���X�̃R�}���h(�}�N���n)�֐��Q

// �L�[�}�N���̋L�^�J�n�^�I��
void ViewCommander::Command_RecKeyMacro(void)
{
	auto& flags = GetDllShareData().flags;
	if (flags.bRecordingKeyMacro) {									// �L�[�{�[�h�}�N���̋L�^��
		flags.bRecordingKeyMacro = false;
		flags.hwndRecordingKeyMacro = NULL;							// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
		// �L�[�}�N�����}�N���p�t�H���_�ɁuRecKey.mac�v�Ƃ������ŕۑ�
		TCHAR szInitDir[MAX_PATH];
		int nRet;
		// �L�^�p�L�[�}�N���̃t���p�X��ShareData�o�R�Ŏ擾
		nRet = ShareData::getInstance().GetMacroFilename(-1, szInitDir, MAX_PATH);
		auto& csMacro = GetDllShareData().common.macro;
		if (nRet <= 0) {
			ErrorMessage(view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD24), nRet);
			return;
		}else {
			_tcscpy(csMacro.szKeyMacroFileName, szInitDir);
		}
		int nSaveResult = pSMacroMgr->Save(
			STAND_KEYMACRO,
			G_AppInstance(),
			csMacro.szKeyMacroFileName
		);
		if (!nSaveResult) {
			ErrorMessage(	view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD25), csMacro.szKeyMacroFileName);
		}
	}else {
		flags.bRecordingKeyMacro = true;
		flags.hwndRecordingKeyMacro = GetMainWindow();	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
		// �L�[�}�N���̃o�b�t�@���N���A����
		pSMacroMgr->Clear(STAND_KEYMACRO);
	}
	// �e�E�B���h�E�̃^�C�g�����X�V
	GetEditWindow().UpdateCaption();

	// �L�����b�g�̍s���ʒu��\������
	GetCaret().ShowCaretPosInfo();
}


// �L�[�}�N���̕ۑ�
void ViewCommander::Command_SaveKeyMacro(void)
{
	auto& flags = GetDllShareData().flags;
	flags.bRecordingKeyMacro = false;
	flags.hwndRecordingKeyMacro = NULL;	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E

	if (!pSMacroMgr->IsSaveOk()) {
		// �ۑ��s��
		ErrorMessage(view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD26));
	}

	TCHAR szPath[_MAX_PATH + 1];
	TCHAR szInitDir[_MAX_PATH + 1];
	szPath[0] = 0;
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
		view.GetHwnd(),
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
	if (!pSMacroMgr->Save(STAND_KEYMACRO, G_AppInstance(), szPath)) {
		ErrorMessage(view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD27), szPath);
	}
}


/*! �L�[�}�N���̓ǂݍ��� */
void ViewCommander::Command_LoadKeyMacro(void)
{
	auto& flags = GetDllShareData().flags;
	flags.bRecordingKeyMacro = false;
	flags.hwndRecordingKeyMacro = NULL;	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E

	TCHAR szPath[_MAX_PATH + 1];
	TCHAR szInitDir[_MAX_PATH + 1];
	const TCHAR* pszFolder = GetDllShareData().common.macro.szMACROFOLDER;
	szPath[0] = 0;
	
	if (_IS_REL_PATH(pszFolder)) {
		GetInidirOrExedir(szInitDir, pszFolder);
	}else {
		_tcscpy_s(szInitDir, pszFolder);	// �}�N���p�t�H���_
	}
	// �t�@�C���I�[�v���_�C�A���O�̏�����
	DlgOpenFile dlgOpenFile;
	dlgOpenFile.Create(
		G_AppInstance(),
		view.GetHwnd(),
		_T("*.*"),
		szInitDir
	);
	if (!dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
		return;
	}

	// �L�[�{�[�h�}�N���̓ǂݍ���
	// �ǂݍ��݂Ƃ������A�t�@�C�������R�s�[���邾���B���s���O�ɓǂݍ���
	_tcscpy(GetDllShareData().common.macro.szKeyMacroFileName, szPath);
//	GetDllShareData().CKeyMacroMgr.LoadKeyMacro(G_AppInstance(), view.GetHwnd(), szPath);
}

// �L�[�}�N���̎��s
void ViewCommander::Command_ExecKeyMacro(void)
{
	auto& flags = GetDllShareData().flags;
	// �L�^���͏I�����Ă�����s
	if (flags.bRecordingKeyMacro) {
		Command_RecKeyMacro();
	}
	flags.bRecordingKeyMacro = false;
	flags.hwndRecordingKeyMacro = NULL;	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E

	// �L�[�{�[�h�}�N���̎��s
	auto& csMacro = GetDllShareData().common.macro;
	if (csMacro.szKeyMacroFileName[0]) {
		// �t�@�C�����ۑ�����Ă�����
		bool bLoadResult = pSMacroMgr->Load(
			view,
			STAND_KEYMACRO,
			G_AppInstance(),
			csMacro.szKeyMacroFileName,
			NULL
		);
		if (!bLoadResult) {
			ErrorMessage(view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD28), csMacro.szKeyMacroFileName);
		}else {
			pSMacroMgr->Exec(STAND_KEYMACRO, G_AppInstance(), view, 0);
		}
	}
}


/*! ���O���w�肵�ă}�N�����s
	@param pszPath	�}�N���̃t�@�C���p�X�A�܂��̓}�N���̃R�[�h�B
	@param pszType	��ʁBNULL�̏ꍇ�t�@�C���w��A����ȊO�̏ꍇ�͌���̊g���q���w��
 */
void ViewCommander::Command_ExecExtMacro(const wchar_t* pszPathW, const wchar_t* pszTypeW)
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
			view.GetHwnd(),
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
		pSMacroMgr->Append(STAND_KEYMACRO, F_EXECEXTMACRO, lparams, view);
		// �L�[�}�N���̋L�^���ꎞ��~����
		flags.bRecordingKeyMacro = false;
		hwndRecordingKeyMacro = flags.hwndRecordingKeyMacro;
		flags.hwndRecordingKeyMacro = NULL;	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
	}

	// �Â��ꎞ�}�N���̑ޔ�
	MacroManagerBase* oldMacro = pSMacroMgr->SetTempMacro(nullptr);
	bool bLoadResult = pSMacroMgr->Load(
		view,
		TEMP_KEYMACRO,
		G_AppInstance(),
		pszPath,
		pszType
	);
	if (!bLoadResult) {
		ErrorMessage(view.GetHwnd(), LS(STR_ERR_MACROERR1), pszPath);
	}else {
		pSMacroMgr->Exec(TEMP_KEYMACRO, G_AppInstance(), view, FA_NONRECORD | FA_FROMMACRO);
	}

	// �I�������J��
	pSMacroMgr->Clear(TEMP_KEYMACRO);
	if (oldMacro) {
		pSMacroMgr->SetTempMacro(oldMacro);
	}

	// �L�[�}�N���L�^���������ꍇ�͍ĊJ����
	if (hwndRecordingKeyMacro) {
		flags.bRecordingKeyMacro = true;
		flags.hwndRecordingKeyMacro = hwndRecordingKeyMacro;	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
	}
}


/*! �O���R�}���h���s�_�C�A���O�\�� */
void ViewCommander::Command_ExecCommand_Dialog(void)
{
	DlgExec dlgExec;

	// ���[�h���X�_�C�A���O�̕\��
	if (!dlgExec.DoModal(G_AppInstance(), view.GetHwnd(), 0)) {
		return;
	}

	view.AddToCmdArr(dlgExec.szCommand);
	const wchar_t* cmd_string = to_wchar(dlgExec.szCommand);
	const wchar_t* curDir = to_wchar(dlgExec.szCurDir);
	const wchar_t* pszDir = curDir;
	if (curDir[0] == L'\0') {
		pszDir = NULL;
	}else {
		RecentCurDir cRecentCurDir;
		cRecentCurDir.AppendItem(dlgExec.szCurDir);
		cRecentCurDir.Terminate();
	}

	//HandleCommand(F_EXECMD, true, (LPARAM)cmd_string, 0, 0, 0);	// �O���R�}���h���s�R�}���h�̔��s
	HandleCommand(F_EXECMD, true, (LPARAM)cmd_string, (LPARAM)(GetDllShareData().nExecFlgOpt), (LPARAM)pszDir, 0);	// �O���R�}���h���s�R�}���h�̔��s
}


// �O���R�}���h���s
void ViewCommander::Command_ExecCommand(
	LPCWSTR cmd_string,
	const int nFlgOpt,
	LPCWSTR pszCurDir
	)
{
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
	view.ExecCmd(buf2.c_str(), nFlgOpt, (pszCurDir ? buf3.c_str() : nullptr));
}

