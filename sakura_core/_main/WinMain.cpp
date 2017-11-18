/*!	@file
	@brief Entry Point
*/

#include "StdAfx.h"
#include <Ole2.h>
#include "ProcessFactory.h"
#include "Process.h"
#include "util/os.h"
#include "util/module.h"
#include "debug/RunningTimer.h"

/*!
	Windows Entry point

	1�ڂ̃G�f�B�^�v���Z�X�̏ꍇ�́A���̃v���Z�X�̓R���g���[���v���Z�X��
	�Ȃ�A�V�����G�f�B�^�v���Z�X���N������B�����łȂ��Ƃ��̓G�f�B�^�v���Z�X
	�ƂȂ�B

	�R���g���[���v���Z�X��CControlProcess�N���X�̃C���X�^���X�����A
	�G�f�B�^�v���Z�X��CNormalProcess�N���X�̃C���X�^���X�����B
*/
#ifdef __MINGW32__
int WINAPI WinMain(
	HINSTANCE	hInstance,		// handle to current instance
	HINSTANCE	hPrevInstance,	// handle to previous instance
	LPSTR		lpCmdLineA,		// pointer to command line
	int			nCmdShow		// show state of window
	)
#else
int WINAPI _tWinMain(
	HINSTANCE	hInstance,		// handle to current instance
	HINSTANCE	hPrevInstance,	// handle to previous instance
	LPTSTR		lpCmdLine,		// pointer to command line
	int			nCmdShow		// show state of window
	)
#endif
{
#ifdef USE_LEAK_CHECK_WITH_CRTDBG
	::_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
#endif

	MY_RUNNINGTIMER(runningTimer, "WinMain" );
	{
		// DLL�C���W�F�N�V�����΍�
		CurrentDirectoryBackupPoint dirBack;
		ChangeCurrentDirectoryToExeDir();
		
		setlocale(LC_ALL, "Japanese");
		::OleInitialize(NULL);
	}
	
	// �J�����
	DEBUG_TRACE(_T("-- -- WinMain -- --\n"));
	DEBUG_TRACE(_T("sizeof(DllSharedData) = %d\n"), sizeof(DllSharedData));
	
	// �v���Z�X�̐����ƃ��b�Z�[�W���[�v
	ProcessFactory aFactory;
	Process* process = nullptr;

#ifndef _DEBUG
	try {
#endif

#ifdef __MINGW32__
		LPTSTR pszCommandLine;
		pszCommandLine = ::GetCommandLine();
		// ���s�t�@�C�������X�L�b�v����
		if (*pszCommandLine == _T('\"')) {
			++pszCommandLine;
			while (*pszCommandLine != _T('\"') && *pszCommandLine != _T('\0')) {
				++pszCommandLine;
			}
			if (*pszCommandLine == _T('\"')) {
				++pszCommandLine;
			}
		}else {
			while (
				_T(' ') != *pszCommandLine
				&& *pszCommandLine != _T('\t')
				&& *pszCommandLine != _T('\0')
			) {
				++pszCommandLine;
			}
		}
		// ���̃g�[�N���܂Ői�߂�
		while (*pszCommandLine == _T(' ') || *pszCommandLine == _T('\t')) {
			++pszCommandLine;
		}
		process = aFactory.Create(hInstance, pszCommandLine);
#else
		process = aFactory.Create(hInstance, lpCmdLine);
#endif
		MY_TRACETIME(runningTimer, "ProcessObject Created");

#ifndef _DEBUG
	}catch (...) {
		;
	}
#endif

	if (process) {
		process->Run();
		delete process;
	}

	::OleUninitialize();
	return 0;
}

