#include "StdAfx.h"
#include "ControlProcess.h"
#include "ControlTray.h"
#include "env/DllSharedData.h"
#include "CommandLine.h"
#include "env/ShareData_IO.h"
#include "debug/RunningTimer.h"
#include "sakura_rc.h"

//-------------------------------------------------


/*!
	@brief �R���g���[���v���Z�X������������
	
	MutexCP���쐬�E���b�N����B
	ControlTray���쐬����B
*/
bool ControlProcess::InitializeProcess()
{
	MY_RUNNINGTIMER(runningTimer, "ControlProcess::InitializeProcess");

	// �A�v���P�[�V�������s���o�p(�C���X�g�[���Ŏg�p)
	hMutex = ::CreateMutex(NULL, FALSE, GSTR_MUTEX_SAKURA);
	if (!hMutex) {
		ErrorBeep();
		TopErrorMessage(NULL, _T("CreateMutex()���s�B\n�I�����܂��B"));
		return false;
	}

	std::tstring strProfileName = to_tchar(CommandLine::getInstance().GetProfileName());

	// �����������C�x���g���쐬����
	std::tstring strInitEvent = GSTR_EVENT_SAKURA_CP_INITIALIZED;
	strInitEvent += strProfileName;
	hEventCPInitialized = ::CreateEvent( NULL, TRUE, FALSE, strInitEvent.c_str() );
	if (!hEventCPInitialized) {
		ErrorBeep();
		TopErrorMessage(NULL, _T("CreateEvent()���s�B\n�I�����܂��B"));
		return false;
	}

	// �R���g���[���v���Z�X�̖ڈ�
	std::tstring strCtrlProcEvent = GSTR_MUTEX_SAKURA_CP;
	strCtrlProcEvent += strProfileName;
	hMutexCP = ::CreateMutex( NULL, TRUE, strCtrlProcEvent.c_str() );
	if (!hMutexCP) {
		ErrorBeep();
		TopErrorMessage(NULL, _T("CreateMutex()���s�B\n�I�����܂��B"));
		return false;
	}
	if (::GetLastError() == ERROR_ALREADY_EXISTS) {
		return false;
	}
	
	// ���L��������������
	if (!Process::InitializeProcess()) {
		return false;
	}

	// �R���g���[���v���Z�X�̃J�����g�f�B���N�g�����V�X�e���f�B���N�g���ɕύX
	TCHAR szDir[_MAX_PATH];
	::GetSystemDirectory(szDir, _countof(szDir));
	::SetCurrentDirectory(szDir);

	// ���L�f�[�^�̃��[�h
	// �u�ݒ��ۑ����ďI������v�I�v�V���������isakuext�A�g�p�j��ǉ�
	TCHAR szIniFile[_MAX_PATH];
	ShareData_IO::LoadShareData();
	FileNameManager::getInstance().GetIniFileName( szIniFile, strProfileName.c_str() );	// �o��ini�t�@�C����
	if (!fexist(szIniFile) || CommandLine::getInstance().IsWriteQuit()) {
		// ���W�X�g������ �쐬
		ShareData_IO::SaveShareData();
		if (CommandLine::getInstance().IsWriteQuit()) {
			return false;
		}
	}

	// �����I������
	SelectLang::ChangeLang(GetDllShareData().common.window.szLanguageDll);
	RefreshString();

	MY_TRACETIME(runningTimer, "Before new ControlTray");

	// �^�X�N�g���C�ɃA�C�R���쐬
	pTray = new ControlTray();

	MY_TRACETIME(runningTimer, "After new ControlTray");

	HWND hwnd = pTray->Create(GetProcessInstance());
	if (!hwnd) {
		ErrorBeep();
		TopErrorMessage(NULL, LS(STR_ERR_CTRLMTX3));
		return false;
	}
	SetMainWindow(hwnd);
	GetDllShareData().handles.hwndTray = hwnd;

	// �����������C�x���g���V�O�i����Ԃɂ���
	if (!::SetEvent(hEventCPInitialized)) {
		ErrorBeep();
		TopErrorMessage(NULL, LS(STR_ERR_CTRLMTX4));
		return false;
	}
	return true;
}

/*!
	@brief �R���g���[���v���Z�X�̃��b�Z�[�W���[�v
*/
bool ControlProcess::MainLoop()
{
	if (pTray && GetMainWindow()) {
		pTray->MessageLoop();	// ���b�Z�[�W���[�v
		return true;
	}
	return false;
}

/*!
	@brief �R���g���[���v���Z�X���I������
*/
void ControlProcess::OnExitProcess()
{
	GetDllShareData().handles.hwndTray = NULL;
}

ControlProcess::~ControlProcess()
{
	delete pTray;

	if (hEventCPInitialized) {
		::ResetEvent(hEventCPInitialized);
	}
	::CloseHandle(hEventCPInitialized);
	if (hMutexCP) {
		::ReleaseMutex(hMutexCP);
	}
	::CloseHandle(hMutexCP);
	// ���o�[�W�����i1.2.104.1�ȑO�j�Ƃ̌݊����F�u�قȂ�o�[�W����...�v�����o�Ȃ��悤��
	if (hMutex) {
		::ReleaseMutex(hMutex);
	}
	::CloseHandle(hMutex);
};


