/*!	@file
	@brief プロセス生成クラス
*/

#include "StdAfx.h"
#include "ProcessFactory.h"
#include "ControlProcess.h"
#include "NormalProcess.h"
#include "CommandLine.h"
#include "ControlTray.h"
#include "_os/OsVersionInfo.h"
#include "dlg/DlgProfileMgr.h"
#include "debug/RunningTimer.h"
#include "util/os.h"
#include <io.h>
#include <tchar.h>

class Process;


/*!
	@brief プロセスクラスを生成する
	
	コマンドライン、コントロールプロセスの有無を判定し、
	適当なプロセスクラスを生成する。
	
	@param[in] hInstance インスタンスハンドル
	@param[in] lpCmdLine コマンドライン文字列
*/
Process* ProcessFactory::Create(
	HINSTANCE hInstance,
	LPCTSTR lpCmdLine
	)
{
	if (!ProfileSelect( hInstance, lpCmdLine )) {
		return 0;
	}

	Process* process = 0;
	if (!IsValidVersion()) {
		return 0;
	}

	// プロセスクラスを生成する
	//
	// Note: 以下の処理において使用される IsExistControlProcess() は、コントロールプロセスが
	// 存在しない場合だけでなく、コントロールプロセスが起動して ::CreateMutex() を実行するまで
	// の間も false（コントロールプロセス無し）を返す。
	// 従って、複数のノーマルプロセスが同時に起動した場合などは複数のコントロールプロセスが
	// 起動されることもある。
	// しかし、そのような場合でもミューテックスを最初に確保したコントロールプロセスが唯一生き残る。
	//
	if (IsStartingControlProcess()) {
		if (TestWriteQuit()) {
			return 0;
		}
		if (!IsExistControlProcess()) {
			process = new ControlProcess(hInstance, lpCmdLine);
		}
	}else {
		if (!IsExistControlProcess()) {
			StartControlProcess();
		}
		if (WaitForInitializedControlProcess()) {
			process = new NormalProcess(hInstance, lpCmdLine);
		}
	}
	return process;
}


bool ProcessFactory::ProfileSelect(
	HINSTANCE hInstance,
	LPCTSTR lpCmdLine
	)
{
	ProfileSettings settings;

	DlgProfileMgr::ReadProfSettings(settings);
	SelectLang::InitializeLanguageEnvironment();
	SelectLang::ChangeLang(settings.szDllLanguage);

	auto& cmdLine = CommandLine::getInstance();
	cmdLine.ParseCommandLine(lpCmdLine);

	bool bDialog;
	if (cmdLine.IsProfileMgr()) {
		bDialog = true;
	}else if (cmdLine.IsSetProfile()) {
		bDialog = false;
	}else if (settings.nDefaultIndex == -1) {
		bDialog = true;
	}else {
		ASSERT_GE(settings.nDefaultIndex, 0);
		if (0 < settings.nDefaultIndex) {
			cmdLine.SetProfileName(
				settings.profList[settings.nDefaultIndex - 1].c_str()
			);
		}else {
			cmdLine.SetProfileName(L"");
		}
		bDialog = false;
	}
	if (bDialog) {
		DlgProfileMgr dlgProf;
		if (dlgProf.DoModal(hInstance, NULL, 0)) {
			cmdLine.SetProfileName( to_wchar(dlgProf.strProfileName.c_str()) );
		}else {
			return false; // プロファイルマネージャで「閉じる」を選んだ。プロセス終了
		}
	}
	return true;
}

/*!
	@brief Windowsバージョンのチェック
	
	Windows 95以上，Windows NT4.0以上であることを確認する．
	Windows 95系では残りリソースのチェックも行う．
*/
bool ProcessFactory::IsValidVersion()
{
	// Windowsバージョンのチェック
	OsVersionInfo osVer(true);	// 初期化を行う
	if (osVer.GetVersion()) {
		if (!osVer.OsIsEnableVersion()) {
			InfoMessage(NULL,
				_T("このアプリケーションを実行するには、\n")
#if (WINVER >= _WIN32_WINNT_WIN7)
				_T("Windows7以降のOSが必要です。\n")
#elif (WINVER >= _WIN32_WINNT_VISTA)
				_T("WindowsVista以降 または WindowsServer2008以降のOSが必要です。\n")
#elif (WINVER >= _WIN32_WINNT_WIN2K)
				_T("Windows2000以降のOSが必要です。\n")
#else
				_T("Windows95以上 または WindowsNT4.0以上のOSが必要です。\n")
#endif
				_T("アプリケーションを終了します。")
			);
			return false;
		}
	}else {
		InfoMessage(NULL, _T("OSのバージョンが取得できません。\nアプリケーションを終了します。"));
		return false;
	}

	// 拡張命令セットのチェック
#ifdef USE_SSE2
	if (osVer._SupportSSE2()) {
	}else {
		InfoMessage(NULL,
			_T("このアプリケーションを実行するには、\n")
			_T("SSE2命令セットをサポートしたCPUが必要です。\n")
			_T("アプリケーションを終了します。")
		);
		return false;
	}
#endif

	return true;
}


/*!
	@brief コマンドラインに -NOWIN があるかを判定する。
*/
bool ProcessFactory::IsStartingControlProcess()
{
	return CommandLine::getInstance().IsNoWindow();
}

/*!
	コントロールプロセスの有無を調べる。
*/
bool ProcessFactory::IsExistControlProcess()
{
	std::tstring strProfileName = to_tchar(CommandLine::getInstance().GetProfileName());
	std::tstring strMutexSakuraCp = GSTR_MUTEX_SAKURA_CP;
	strMutexSakuraCp += strProfileName;
 	HANDLE hMutexCP;
	hMutexCP = ::OpenMutex( MUTEX_ALL_ACCESS, FALSE, strMutexSakuraCp.c_str() );
	if (hMutexCP) {
		::CloseHandle(hMutexCP);
		return true;	// コントロールプロセスが見つかった
	}
	return false;	// コントロールプロセスは存在していないか、まだ CreateMutex() してない
}

/*!
	@brief コントロールプロセスを起動する
	
	自分自身に -NOWIN オプションを付けて起動する．
	共有メモリをチェックしてはいけないので，残念ながらCControlTray::OpenNewEditorは使えない．
*/
bool ProcessFactory::StartControlProcess()
{
	MY_RUNNINGTIMER(runningTimer, "StartControlProcess" );

	// プロセスの起動
	STARTUPINFO s;
	s.cb          = sizeof(s);
	s.lpReserved  = NULL;
	s.lpDesktop   = NULL;
	s.lpTitle     = const_cast<TCHAR*>(_T("sakura control process"));
	s.dwFlags     = STARTF_USESHOWWINDOW;
	s.wShowWindow = SW_SHOWDEFAULT;
	s.cbReserved2 = 0;
	s.lpReserved2 = NULL;

	TCHAR szCmdLineBuf[1024];	// コマンドライン
	TCHAR szEXE[MAX_PATH + 1];	// アプリケーションパス名

	::GetModuleFileName(NULL, szEXE, _countof(szEXE));
	if (CommandLine::getInstance().IsSetProfile()) {
		::auto_sprintf( szCmdLineBuf, _T("\"%ts\" -NOWIN -PROF=\"%ls\""),
			szEXE, CommandLine::getInstance().GetProfileName() );
	}else {
		::auto_sprintf( szCmdLineBuf, _T("\"%ts\" -NOWIN"), szEXE ); // ""付加
	}

	// 常駐プロセス起動
	DWORD dwCreationFlag = CREATE_DEFAULT_ERROR_MODE;
#ifdef _DEBUG
//	dwCreationFlag |= DEBUG_PROCESS;
#endif
	PROCESS_INFORMATION p;
	BOOL bCreateResult = ::CreateProcess(
		szEXE,				// 実行可能モジュールの名前
		szCmdLineBuf,		// コマンドラインの文字列
		NULL,				// セキュリティ記述子
		NULL,				// セキュリティ記述子
		FALSE,				// ハンドルの継承オプション
		dwCreationFlag,		// 作成のフラグ
		NULL,				// 新しい環境ブロック
		NULL,				// カレントディレクトリの名前
		&s,					// スタートアップ情報
		&p					// プロセス情報
	);
	if (!bCreateResult) {
		// 失敗
		TCHAR* pMsg;
		::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_IGNORE_INSERTS |
						FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						::GetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPTSTR)&pMsg,
						0,
						NULL
		);
		ErrorMessage(NULL, _T("\'%ts\'\nプロセスの起動に失敗しました。\n%ts"), szEXE, pMsg);
		::LocalFree((HLOCAL)pMsg);	// エラーメッセージバッファを解放
		return false;
	}

	// 起動したプロセスが完全に立ち上がるまでちょっと待つ．
	//
	// Note: この待ちにより、ここで起動したコントロールプロセスが競争に生き残れなかった場合でも、
	// 唯一生き残ったコントロールプロセスが多重起動防止用ミューテックスを作成しているはず。
	//
	int nResult = ::WaitForInputIdle(p.hProcess, 10000);	// 最大10秒間待つ
	if (nResult != 0) {
		ErrorMessage(NULL, _T("\'%ls\'\nコントロールプロセスの起動に失敗しました。"), szEXE);
		::CloseHandle(p.hThread);
		::CloseHandle(p.hProcess);
		return false;
	}

	::CloseHandle(p.hThread);
	::CloseHandle(p.hProcess);
	
	return true;
}

/*!
	@brief コントロールプロセスの初期化完了イベントを待つ。
*/
bool ProcessFactory::WaitForInitializedControlProcess()
{
	// 初期化完了イベントを待つ
	//
	// Note: コントロールプロセス側は多重起動防止用ミューテックスを ::CreateMutex() で
	// 作成するよりも先に初期化完了イベントを ::CreateEvent() で作成する。
	//
	if (!IsExistControlProcess()) {
		// コントロールプロセスが多重起動防止用のミューテックス作成前に異常終了した場合など
		return false;
	}

	std::tstring strProfileName = to_tchar(CommandLine::getInstance().GetProfileName());
	std::tstring strInitEvent = GSTR_EVENT_SAKURA_CP_INITIALIZED;
	strInitEvent += strProfileName;
	HANDLE hEvent;
	hEvent = ::OpenEvent( EVENT_ALL_ACCESS, FALSE, strInitEvent.c_str() );
	if (!hEvent) {
		// 動作中のコントロールプロセスを旧バージョンとみなし、イベントを待たずに処理を進める
		//
		// Note: Ver1.5.9.91以前のバージョンは初期化完了イベントを作らない。
		// このため、コントロールプロセスが常駐していないときに複数ウィンドウをほぼ
		// 同時に起動すると、競争に生き残れなかったコントロールプロセスの親プロセスや、
		// 僅かに出遅れてコントロールプロセスを作成しなかったプロセスでも、
		// コントロールプロセスの初期化処理を追い越してしまい、異常終了したり、
		// 「タブバーが表示されない」のような問題が発生していた。
		//
		return true;
	}
	DWORD dwRet = ::WaitForSingleObject(hEvent, 10000);	// 最大10秒間待つ
	if (dwRet == WAIT_TIMEOUT) {	// コントロールプロセスの初期化が終了しない
		::CloseHandle(hEvent);
		TopErrorMessage(NULL, _T("エディタまたはシステムがビジー状態です。\nしばらく待って開きなおしてください。"));
		return false;
	}
	::CloseHandle(hEvent);
	return true;
}

/*!
	@brief 「設定を保存して終了する」オプション処理（sakuext連携用）
*/
bool ProcessFactory::TestWriteQuit()
{
	if (CommandLine::getInstance().IsWriteQuit()) {
		TCHAR szIniFileIn[_MAX_PATH];
		TCHAR szIniFileOut[_MAX_PATH];
		FileNameManager::getInstance().GetIniFileNameDirect( szIniFileIn, szIniFileOut, _T("") );
		if (szIniFileIn[0] != _T('\0')) {	// マルチユーザ用設定か
			// 既にマルチユーザ用のiniファイルがあればEXE基準のiniファイルに上書き更新して終了
			if (fexist(szIniFileIn)) {
				if (::CopyFile(szIniFileIn, szIniFileOut, FALSE)) {
					return true;
				}
			}
		}else {
			// 既にEXE基準のiniファイルがあれば何もせず終了
			if (fexist(szIniFileOut)) {
				return true;
			}
		}
	}
	return false;
}

