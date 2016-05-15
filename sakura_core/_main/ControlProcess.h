/*!	@file
	@brief コントロールプロセスクラスヘッダファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 2002, aroka 新規作成, YAZAKI
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

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
	
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、Processにひとつあるのみ。
*/
class ControlProcess : public Process {
public:
	ControlProcess(HINSTANCE hInstance, LPCTSTR lpCmdLine) :
		Process(hInstance, lpCmdLine),
		// 2006.04.10 ryoji 同期オブジェクトのハンドルを初期化
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
	HANDLE			hEventCPInitialized;	// コントロールプロセス初期化完了イベント 2006.04.10 ryoji
	ControlTray*	pTray;
};

