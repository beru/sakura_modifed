/*!	@file
	@brief エディタプロセスクラスヘッダファイル
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
	HANDLE _GetInitializeMutex() const;

private:
	EditApp* pEditApp;
};

