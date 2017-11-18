#include "StdAfx.h"
#include "ControlProcess.h"
#include "ControlTray.h"
#include "env/DllSharedData.h"
#include "CommandLine.h"
#include "env/ShareData_IO.h"
#include "debug/RunningTimer.h"
#include "sakura_rc.h"

//-------------------------------------------------


/*!
	@brief コントロールプロセスを初期化する
	
	MutexCPを作成・ロックする。
	ControlTrayを作成する。
*/
bool ControlProcess::InitializeProcess()
{
	MY_RUNNINGTIMER(runningTimer, "ControlProcess::InitializeProcess");

	// アプリケーション実行検出用(インストーラで使用)
	hMutex = ::CreateMutex(NULL, FALSE, GSTR_MUTEX_SAKURA);
	if (!hMutex) {
		ErrorBeep();
		TopErrorMessage(NULL, _T("CreateMutex()失敗。\n終了します。"));
		return false;
	}

	std::tstring strProfileName = to_tchar(CommandLine::getInstance().GetProfileName());

	// 初期化完了イベントを作成する
	std::tstring strInitEvent = GSTR_EVENT_SAKURA_CP_INITIALIZED;
	strInitEvent += strProfileName;
	hEventCPInitialized = ::CreateEvent( NULL, TRUE, FALSE, strInitEvent.c_str() );
	if (!hEventCPInitialized) {
		ErrorBeep();
		TopErrorMessage(NULL, _T("CreateEvent()失敗。\n終了します。"));
		return false;
	}

	// コントロールプロセスの目印
	std::tstring strCtrlProcEvent = GSTR_MUTEX_SAKURA_CP;
	strCtrlProcEvent += strProfileName;
	hMutexCP = ::CreateMutex( NULL, TRUE, strCtrlProcEvent.c_str() );
	if (!hMutexCP) {
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
	// 「設定を保存して終了する」オプション処理（sakuext連携用）を追加
	TCHAR szIniFile[_MAX_PATH];
	ShareData_IO::LoadShareData();
	FileNameManager::getInstance().GetIniFileName( szIniFile, strProfileName.c_str() );	// 出力iniファイル名
	if (!fexist(szIniFile) || CommandLine::getInstance().IsWriteQuit()) {
		// レジストリ項目 作成
		ShareData_IO::SaveShareData();
		if (CommandLine::getInstance().IsWriteQuit()) {
			return false;
		}
	}

	// 言語を選択する
	SelectLang::ChangeLang(GetDllShareData().common.window.szLanguageDll);
	RefreshString();

	MY_TRACETIME(runningTimer, "Before new ControlTray");

	// タスクトレイにアイコン作成
	pTray = new ControlTray();

	MY_TRACETIME(runningTimer, "After new ControlTray");

	HWND hwnd = pTray->Create(GetProcessInstance());
	if (!hwnd) {
		ErrorBeep();
		TopErrorMessage(NULL, LS(STR_ERR_CTRLMTX3));
		return false;
	}
	SetMainWindow(hwnd);
	GetDllShareData().handles.hwndTray = hwnd;

	// 初期化完了イベントをシグナル状態にする
	if (!::SetEvent(hEventCPInitialized)) {
		ErrorBeep();
		TopErrorMessage(NULL, LS(STR_ERR_CTRLMTX4));
		return false;
	}
	return true;
}

/*!
	@brief コントロールプロセスのメッセージループ
*/
bool ControlProcess::MainLoop()
{
	if (pTray && GetMainWindow()) {
		pTray->MessageLoop();	// メッセージループ
		return true;
	}
	return false;
}

/*!
	@brief コントロールプロセスを終了する
*/
void ControlProcess::OnExitProcess()
{
	GetDllShareData().handles.hwndTray = NULL;
}

ControlProcess::~ControlProcess()
{
	delete pTray;

	if (hEventCPInitialized) {
		::ResetEvent(hEventCPInitialized);
	}
	::CloseHandle(hEventCPInitialized);
	if (hMutexCP) {
		::ReleaseMutex(hMutexCP);
	}
	::CloseHandle(hMutexCP);
	// 旧バージョン（1.2.104.1以前）との互換性：「異なるバージョン...」が二回出ないように
	if (hMutex) {
		::ReleaseMutex(hMutex);
	}
	::CloseHandle(hMutex);
};


