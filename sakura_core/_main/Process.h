/*!	@file
	@brief プロセス基底クラスヘッダファイル
*/

#pragma once

#include "global.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"

//#define USE_CRASHDUMP

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief プロセス基底クラス
*/
class Process : public TSingleInstance<Process> {
public:
	Process(HINSTANCE hInstance, LPCTSTR lpCmdLine);
	bool Run();
	virtual ~Process() {}
	virtual void RefreshString();
protected:
	Process();
	virtual bool InitializeProcess();
	virtual bool MainLoop() = 0;
	virtual void OnExitProcess() = 0;

protected:
	void			SetMainWindow(HWND hwnd) { hWnd = hwnd; }
#ifdef USE_CRASHDUMP
	int				WriteDump(PEXCEPTION_POINTERS pExceptPtrs);
#endif
public:
	HINSTANCE		GetProcessInstance() const { return hInstance; }
	ShareData&		GetShareData()   { return *pShareData; }
	HWND			GetMainWindow() const { return hWnd; }

private:
	HINSTANCE	hInstance;
	HWND		hWnd;
#ifdef USE_CRASHDUMP
	BOOL (WINAPI *pfnMiniDumpWriteDump)(
		HANDLE hProcess,
		DWORD ProcessId,
		HANDLE hFile,
		MINIDUMP_TYPE DumpType,
		PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
		PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
		PMINIDUMP_CALLBACK_INFORMATION CallbackParam
	);
#endif
	ShareData* pShareData;

private:
};

