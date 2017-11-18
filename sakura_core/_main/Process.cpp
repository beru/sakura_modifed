/*!	@file
	@brief �v���Z�X���N���X
*/

#include "StdAfx.h"
#include "Process.h"
#include "util/module.h"

/*!
	@brief �v���Z�X���N���X
*/
Process::Process(
	HINSTANCE	hInstance,		// handle to process instance
	LPCTSTR		lpCmdLine		// pointer to command line
	)
	:
	hInstance(hInstance),
	hWnd(0)
#ifdef USE_CRASHDUMP
	, pfnMiniDumpWriteDump(nullptr)
#endif
{
	pShareData = &ShareData::getInstance();
}

/*!
	@brief �v���Z�X������������

	���L������������������
*/
bool Process::InitializeProcess()
{
	// ���L�f�[�^�\���̂̃A�h���X��Ԃ�
	if (!GetShareData().InitShareData()) {
		// �K�؂ȃf�[�^�𓾂��Ȃ�����
		::MYMESSAGEBOX(NULL, MB_OK | MB_ICONERROR,
			GSTR_APPNAME, _T("�قȂ�o�[�W�����̃G�f�B�^�𓯎��ɋN�����邱�Ƃ͂ł��܂���B"));
		return false;
	}

	return true;
}

/*!
	@brief �v���Z�X���s
*/
bool Process::Run()
{
	if (InitializeProcess()) {
#ifdef USE_CRASHDUMP
		HMODULE hDllDbgHelp = LoadLibraryExedir(_T("dbghelp.dll"));
		pfnMiniDumpWriteDump = nullptr;
		if (hDllDbgHelp) {
			*(FARPROC*)&pfnMiniDumpWriteDump = ::GetProcAddress(hDllDbgHelp, "MiniDumpWriteDump");
		}

		__try {
#endif
			MainLoop() ;
			OnExitProcess();
#ifdef USE_CRASHDUMP
		}
		__except(WriteDump(GetExceptionInformation())) {
		}

		if (hDllDbgHelp) {
			::FreeLibrary(hDllDbgHelp);
			pfnMiniDumpWriteDump = nullptr;
		}
#endif
		return true;
	}
	return false;
}

#ifdef USE_CRASHDUMP
/*!
	@brief �N���b�V���_���v
*/
int Process::WriteDump(PEXCEPTION_POINTERS pExceptPtrs)
{
	if (!pfnMiniDumpWriteDump) {
		return EXCEPTION_CONTINUE_SEARCH;
	}

	static TCHAR szFile[MAX_PATH];
	// �o�͐��ini�Ɠ����iInitializeProcess()��Ɋm��j
	// Vista�ȍ~�ł� C:\Users\(���[�U��)\AppData\Local\CrashDumps �ɏo��
	GetInidirOrExedir(szFile, _APP_NAME_(_T) _T(".dmp"));

	HANDLE hFile = ::CreateFile(
		szFile,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
		NULL);

	if (hFile != INVALID_HANDLE_VALUE) {
		MINIDUMP_EXCEPTION_INFORMATION eInfo;
		eInfo.ThreadId = GetCurrentThreadId();
		eInfo.ExceptionPointers = pExceptPtrs;
		eInfo.ClientPointers = FALSE;

		pfnMiniDumpWriteDump(
			::GetCurrentProcess(),
			::GetCurrentProcessId(),
			hFile,
			MiniDumpNormal,
			pExceptPtrs ? &eInfo : NULL,
			NULL,
			NULL);

		::CloseHandle(hFile);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}
#endif

/*!
	����I����ɋ��L���������̕�������X�V����
*/
void Process::RefreshString()
{
	pShareData->RefreshString();
}

