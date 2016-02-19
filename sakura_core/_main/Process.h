/*!	@file
	@brief プロセス基底クラスヘッダファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 2002, aroka 新規作成
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
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
	void			SetMainWindow(HWND hwnd) { m_hWnd = hwnd; }
#ifdef USE_CRASHDUMP
	int				WriteDump(PEXCEPTION_POINTERS pExceptPtrs);
#endif
public:
	HINSTANCE		GetProcessInstance() const { return m_hInstance; }
	ShareData&		GetShareData()   { return *m_pShareData; }
	HWND			GetMainWindow() const { return m_hWnd; }

private:
	HINSTANCE	m_hInstance;
	HWND		m_hWnd;
#ifdef USE_CRASHDUMP
	BOOL (WINAPI *m_pfnMiniDumpWriteDump)(
		HANDLE hProcess,
		DWORD ProcessId,
		HANDLE hFile,
		MINIDUMP_TYPE DumpType,
		PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
		PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
		PMINIDUMP_CALLBACK_INFORMATION CallbackParam
	);
#endif
	ShareData* m_pShareData;

private:
};

