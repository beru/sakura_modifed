/*!	@file
	@brief プロセス基底クラス
*/

#include "StdAfx.h"
#include "Process.h"
#include "util/module.h"

/*!
	@brief プロセス基底クラス
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
	@brief プロセスを初期化する

	共有メモリを初期化する
*/
bool Process::InitializeProcess()
{
	// 共有データ構造体のアドレスを返す
	if (!GetShareData().InitShareData()) {
		// 適切なデータを得られなかった
		::MYMESSAGEBOX(NULL, MB_OK | MB_ICONERROR,
			GSTR_APPNAME, _T("異なるバージョンのエディタを同時に起動することはできません。"));
		return false;
	}

	return true;
}

/*!
	@brief プロセス実行
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
	@brief クラッシュダンプ
*/
int Process::WriteDump(PEXCEPTION_POINTERS pExceptPtrs)
{
	if (!pfnMiniDumpWriteDump) {
		return EXCEPTION_CONTINUE_SEARCH;
	}

	static TCHAR szFile[MAX_PATH];
	// 出力先はiniと同じ（InitializeProcess()後に確定）
	// Vista以降では C:\Users\(ユーザ名)\AppData\Local\CrashDumps に出力
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
	言語選択後に共有メモリ内の文字列を更新する
*/
void Process::RefreshString()
{
	pShareData->RefreshString();
}

