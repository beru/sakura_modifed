/*!	@file
	@brief �R���g���[���v���Z�X�N���X

	@author aroka
	@date 2002/01/07 Create
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka Process��蕪��, YAZAKI
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "StdAfx.h"
#include "ControlProcess.h"
#include "ControlTray.h"
#include "env/DLLSHAREDATA.h"
#include "CommandLine.h"
#include "env/ShareData_IO.h"
#include "debug/RunningTimer.h"
#include "sakura_rc.h" /// IDD_EXITTING 2002/2/10 aroka �w�b�_����


//-------------------------------------------------


/*!
	@brief �R���g���[���v���Z�X������������
	
	MutexCP���쐬�E���b�N����B
	ControlTray���쐬����B
	
	@author aroka
	@date 2002/01/07
	@date 2002/02/17 YAZAKI ���L������������������̂�Process�Ɉړ��B
	@date 2006/04/10 ryoji �����������C�x���g�̏�����ǉ��A�ُ펞�̌�n���̓f�X�g���N�^�ɔC����
	@date 2013.03.20 novice �R���g���[���v���Z�X�̃J�����g�f�B���N�g�����V�X�e���f�B���N�g���ɕύX
*/
bool ControlProcess::InitializeProcess()
{
	MY_RUNNINGTIMER(runningTimer, "ControlProcess::InitializeProcess");

	// �A�v���P�[�V�������s���o�p(�C���X�g�[���Ŏg�p)
	m_hMutex = ::CreateMutex(NULL, FALSE, GSTR_MUTEX_SAKURA);
	if (!m_hMutex) {
		ErrorBeep();
		TopErrorMessage(NULL, _T("CreateMutex()���s�B\n�I�����܂��B"));
		return false;
	}

	std::tstring strProfileName = to_tchar(CommandLine::getInstance()->GetProfileName());

	// �����������C�x���g���쐬����
	std::tstring strInitEvent = GSTR_EVENT_SAKURA_CP_INITIALIZED;
	strInitEvent += strProfileName;
	m_hEventCPInitialized = ::CreateEvent( NULL, TRUE, FALSE, strInitEvent.c_str() );
	if (!m_hEventCPInitialized) {
		ErrorBeep();
		TopErrorMessage(NULL, _T("CreateEvent()���s�B\n�I�����܂��B"));
		return false;
	}

	// �R���g���[���v���Z�X�̖ڈ�
	std::tstring strCtrlProcEvent = GSTR_MUTEX_SAKURA_CP;
	strCtrlProcEvent += strProfileName;
	m_hMutexCP = ::CreateMutex( NULL, TRUE, strCtrlProcEvent.c_str() );
	if (!m_hMutexCP) {
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
	// 2007.05.19 ryoji �u�ݒ��ۑ����ďI������v�I�v�V���������isakuext�A�g�p�j��ǉ�
	TCHAR szIniFile[_MAX_PATH];
	ShareData_IO::LoadShareData();
	FileNameManager::getInstance()->GetIniFileName( szIniFile, strProfileName.c_str() );	// �o��ini�t�@�C����
	if (!fexist(szIniFile) || CommandLine::getInstance()->IsWriteQuit()) {
		// ���W�X�g������ �쐬
		ShareData_IO::SaveShareData();
		if (CommandLine::getInstance()->IsWriteQuit()) {
			return false;
		}
	}

	// �����I������
	SelectLang::ChangeLang(GetDllShareData().m_common.m_window.m_szLanguageDll);
	RefreshString();

	MY_TRACETIME(runningTimer, "Before new ControlTray");

	// �^�X�N�g���C�ɃA�C�R���쐬
	m_pTray = new ControlTray();

	MY_TRACETIME(runningTimer, "After new ControlTray");

	HWND hwnd = m_pTray->Create(GetProcessInstance());
	if (!hwnd) {
		ErrorBeep();
		TopErrorMessage(NULL, LS(STR_ERR_CTRLMTX3));
		return false;
	}
	SetMainWindow(hwnd);
	GetDllShareData().m_handles.m_hwndTray = hwnd;

	// �����������C�x���g���V�O�i����Ԃɂ���
	if (!::SetEvent(m_hEventCPInitialized)) {
		ErrorBeep();
		TopErrorMessage(NULL, LS(STR_ERR_CTRLMTX4));
		return false;
	}
	return true;
}

/*!
	@brief �R���g���[���v���Z�X�̃��b�Z�[�W���[�v
	
	@author aroka
	@date 2002/01/07
*/
bool ControlProcess::MainLoop()
{
	if (m_pTray && GetMainWindow()) {
		m_pTray->MessageLoop();	// ���b�Z�[�W���[�v
		return true;
	}
	return false;
}

/*!
	@brief �R���g���[���v���Z�X���I������
	
	@author aroka
	@date 2002/01/07
	@date 2006/07/02 ryoji ���L�f�[�^�ۑ��� ControlTray �ֈړ�
*/
void ControlProcess::OnExitProcess()
{
	GetDllShareData().m_handles.m_hwndTray = NULL;
}

ControlProcess::~ControlProcess()
{
	delete m_pTray;

	if (m_hEventCPInitialized) {
		::ResetEvent(m_hEventCPInitialized);
	}
	::CloseHandle(m_hEventCPInitialized);
	if (m_hMutexCP) {
		::ReleaseMutex(m_hMutexCP);
	}
	::CloseHandle(m_hMutexCP);
	// ���o�[�W�����i1.2.104.1�ȑO�j�Ƃ̌݊����F�u�قȂ�o�[�W����...�v�����o�Ȃ��悤��
	if (m_hMutex) {
		::ReleaseMutex(m_hMutex);
	}
	::CloseHandle(m_hMutex);
};


