/*!	@file
	@brief コントロールプロセスクラス

	@author aroka
	@date 2002/01/07 Create
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka Processより分離, YAZAKI
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "StdAfx.h"
#include "ControlProcess.h"
#include "ControlTray.h"
#include "env/DLLSHAREDATA.h"
#include "CommandLine.h"
#include "env/ShareData_IO.h"
#include "debug/RunningTimer.h"
#include "sakura_rc.h" /// IDD_EXITTING 2002/2/10 aroka ヘッダ整理


//-------------------------------------------------


/*!
	@brief コントロールプロセスを初期化する
	
	MutexCPを作成・ロックする。
	ControlTrayを作成する。
	
	@author aroka
	@date 2002/01/07
	@date 2002/02/17 YAZAKI 共有メモリを初期化するのはProcessに移動。
	@date 2006/04/10 ryoji 初期化完了イベントの処理を追加、異常時の後始末はデストラクタに任せる
	@date 2013.03.20 novice コントロールプロセスのカレントディレクトリをシステムディレクトリに変更
*/
bool ControlProcess::InitializeProcess()
{
	MY_RUNNINGTIMER(runningTimer, "ControlProcess::InitializeProcess");

	// アプリケーション実行検出用(インストーラで使用)
	m_hMutex = ::CreateMutex(NULL, FALSE, GSTR_MUTEX_SAKURA);
	if (!m_hMutex) {
		ErrorBeep();
		TopErrorMessage(NULL, _T("CreateMutex()失敗。\n終了します。"));
		return false;
	}

	std::tstring strProfileName = to_tchar(CommandLine::getInstance()->GetProfileName());

	// 初期化完了イベントを作成する
	std::tstring strInitEvent = GSTR_EVENT_SAKURA_CP_INITIALIZED;
	strInitEvent += strProfileName;
	m_hEventCPInitialized = ::CreateEvent( NULL, TRUE, FALSE, strInitEvent.c_str() );
	if (!m_hEventCPInitialized) {
		ErrorBeep();
		TopErrorMessage(NULL, _T("CreateEvent()失敗。\n終了します。"));
		return false;
	}

	// コントロールプロセスの目印
	std::tstring strCtrlProcEvent = GSTR_MUTEX_SAKURA_CP;
	strCtrlProcEvent += strProfileName;
	m_hMutexCP = ::CreateMutex( NULL, TRUE, strCtrlProcEvent.c_str() );
	if (!m_hMutexCP) {
		ErrorBeep();
		TopErrorMessage(NULL, _T("CreateMutex()失敗。\n終了します。"));
		return false;
	}
	if (::GetLastError() == ERROR_ALREADY_EXISTS) {
		return false;
	}
	
	// 共有メモリを初期化
	if (!Process::InitializeProcess()) {
		return false;
	}

	// コントロールプロセスのカレントディレクトリをシステムディレクトリに変更
	TCHAR szDir[_MAX_PATH];
	::GetSystemDirectory(szDir, _countof(szDir));
	::SetCurrentDirectory(szDir);

	// 共有データのロード
	// 2007.05.19 ryoji 「設定を保存して終了する」オプション処理（sakuext連携用）を追加
	TCHAR szIniFile[_MAX_PATH];
	ShareData_IO::LoadShareData();
	FileNameManager::getInstance()->GetIniFileName( szIniFile, strProfileName.c_str() );	// 出力iniファイル名
	if (!fexist(szIniFile) || CommandLine::getInstance()->IsWriteQuit()) {
		// レジストリ項目 作成
		ShareData_IO::SaveShareData();
		if (CommandLine::getInstance()->IsWriteQuit()) {
			return false;
		}
	}

	// 言語を選択する
	SelectLang::ChangeLang(GetDllShareData().m_common.m_window.m_szLanguageDll);
	RefreshString();

	MY_TRACETIME(runningTimer, "Before new ControlTray");

	// タスクトレイにアイコン作成
	m_pTray = new ControlTray();

	MY_TRACETIME(runningTimer, "After new ControlTray");

	HWND hwnd = m_pTray->Create(GetProcessInstance());
	if (!hwnd) {
		ErrorBeep();
		TopErrorMessage(NULL, LS(STR_ERR_CTRLMTX3));
		return false;
	}
	SetMainWindow(hwnd);
	GetDllShareData().m_handles.m_hwndTray = hwnd;

	// 初期化完了イベントをシグナル状態にする
	if (!::SetEvent(m_hEventCPInitialized)) {
		ErrorBeep();
		TopErrorMessage(NULL, LS(STR_ERR_CTRLMTX4));
		return false;
	}
	return true;
}

/*!
	@brief コントロールプロセスのメッセージループ
	
	@author aroka
	@date 2002/01/07
*/
bool ControlProcess::MainLoop()
{
	if (m_pTray && GetMainWindow()) {
		m_pTray->MessageLoop();	// メッセージループ
		return true;
	}
	return false;
}

/*!
	@brief コントロールプロセスを終了する
	
	@author aroka
	@date 2002/01/07
	@date 2006/07/02 ryoji 共有データ保存を ControlTray へ移動
*/
void ControlProcess::OnExitProcess()
{
	GetDllShareData().m_handles.m_hwndTray = NULL;
}

ControlProcess::~ControlProcess()
{
	delete m_pTray;

	if (m_hEventCPInitialized) {
		::ResetEvent(m_hEventCPInitialized);
	}
	::CloseHandle(m_hEventCPInitialized);
	if (m_hMutexCP) {
		::ReleaseMutex(m_hMutexCP);
	}
	::CloseHandle(m_hMutexCP);
	// 旧バージョン（1.2.104.1以前）との互換性：「異なるバージョン...」が二回出ないように
	if (m_hMutex) {
		::ReleaseMutex(m_hMutex);
	}
	::CloseHandle(m_hMutex);
};


