/*!	@file
	@brief エディタプロセスクラスヘッダファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 2002, aroka 新規作成

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include "global.h"
#include "Process.h"
#include "extmodule/Migemo.h"
#include "EditApp.h"
#include "util/design_template.h"
class EditWnd;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief エディタプロセスクラス
	
	エディタプロセスはEditWndクラスのインスタンスを作る。
*/
class NormalProcess : public Process {
public:
	// コンストラクタ・デストラクタ
	NormalProcess(HINSTANCE hInstance, LPCTSTR lpCmdLine);
	virtual ~NormalProcess();

protected:
	// プロセスハンドラ
	virtual bool InitializeProcess();
	virtual bool MainLoop();
	virtual void OnExitProcess();

protected:
	// 実装補助
	HANDLE _GetInitializeMutex() const; // 2002/2/8 aroka

private:
	EditApp* pEditApp;	// 2007.10.23 kobake
};

