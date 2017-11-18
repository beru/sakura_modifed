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

	1つ目のエディタプロセスの場合は、このプロセスはコントロールプロセスと
	なり、新しいエディタプロセスを起動する。そうでないときはエディタプロセス
	となる。

	コントロールプロセスはCControlProcessクラスのインスタンスを作り、
	エディタプロセスはCNormalProcessクラスのインスタンスを作る。
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
		// DLLインジェクション対策
		CurrentDirectoryBackupPoint dirBack;
		ChangeCurrentDirectoryToExeDir();
		
		setlocale(LC_ALL, "Japanese");
		::OleInitialize(NULL);
	}
	
	// 開発情報
	DEBUG_TRACE(_T("-- -- WinMain -- --\n"));
	DEBUG_TRACE(_T("sizeof(DllSharedData) = %d\n"), sizeof(DllSharedData));
	
	// プロセスの生成とメッセージループ
	ProcessFactory aFactory;
	Process* process = nullptr;

#ifndef _DEBUG
	try {
#endif

#ifdef __MINGW32__
		LPTSTR pszCommandLine;
		pszCommandLine = ::GetCommandLine();
		// 実行ファイル名をスキップする
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
		// 次のトークンまで進める
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

