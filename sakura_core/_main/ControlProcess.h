#pragma once

#include "global.h"
#include "Process.h"

class ControlTray;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief コントロールプロセスクラス
	
	コントロールプロセスはControlTrayクラスのインスタンスを作る。
*/
class ControlProcess : public Process {
public:
	ControlProcess(HINSTANCE hInstance, LPCTSTR lpCmdLine) :
		Process(hInstance, lpCmdLine),
		hMutex(NULL),
		hMutexCP(NULL),
		hEventCPInitialized(NULL),
		pTray(0)
	{
	}

	virtual ~ControlProcess();
protected:
	ControlProcess();
	virtual bool InitializeProcess();
	virtual bool MainLoop();
	virtual void OnExitProcess();

private:
	HANDLE			hMutex;					// アプリケーション実行検出用ミューテックス
	HANDLE			hMutexCP;				// コントロールプロセスミューテックス
	HANDLE			hEventCPInitialized;	// コントロールプロセス初期化完了イベント
	ControlTray*	pTray;
};

