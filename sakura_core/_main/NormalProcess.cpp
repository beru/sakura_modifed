/*!	@file
	@brief エディタプロセスクラス
*/

#include "StdAfx.h"
#include "NormalProcess.h"
#include "CommandLine.h"
#include "ControlTray.h"
#include "window/EditWnd.h"
#include "GrepAgent.h"
#include "doc/EditDoc.h"
#include "doc/logic/DocLine.h"
#include "debug/RunningTimer.h"
#include "util/window.h"
#include "util/fileUtil.h"
#include "plugin/PluginManager.h"
#include "plugin/JackManager.h"
#include "AppMode.h"
#include "env/DocTypeManager.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

NormalProcess::NormalProcess(HINSTANCE hInstance, LPCTSTR lpCmdLine)
	:
	Process(hInstance, lpCmdLine),
	pEditApp(nullptr)
{
}

NormalProcess::~NormalProcess()
{
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     プロセスハンドラ                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@brief エディタプロセスを初期化する
	
	EditWndを作成する。
*/
bool NormalProcess::InitializeProcess()
{
	MY_RUNNINGTIMER(runningTimer, "NormalProcess::Init");

	// プロセス初期化の目印
	HANDLE hMutex = _GetInitializeMutex();
	if (!hMutex) {
		return false;
	}

	// 共有メモリを初期化する
	if (!Process::InitializeProcess()) {
		return false;
	}

	// 言語を選択する
	SelectLang::ChangeLang(GetDllShareData().common.window.szLanguageDll);

	// コマンドラインオプション
	bool bViewMode = false;
	
	auto& cmdLine = CommandLine::getInstance();
	// コマンドラインで受け取ったファイルが開かれている場合は
	// その編集ウィンドウをアクティブにする
	EditInfo fi = cmdLine.GetEditInfo();
	if (fi.szPath[0] != _T('\0')) {
		// MRUからカーソル位置を復元する操作はCEditDoc::FileLoadで
		// 行われるのでここでは必要なし．

		// 指定ファイルが開かれているか調べる
		// 文字コードが異なるときはワーニングを出すように
		HWND hwndOwner;
		if (GetShareData().ActiveAlreadyOpenedWindow(fi.szPath, &hwndOwner, fi.nCharCode)) {
			// カーソル位置が引数に指定されていたら指定位置にジャンプ
			if (fi.ptCursor.y >= 0) {	// 行の指定があるか
				Point& pt = GetDllShareData().workBuffer.logicPoint;
				if (fi.ptCursor.x < 0) {
					// 桁の指定が無い場合
					::SendMessage(hwndOwner, MYWM_GETCARETPOS, 0, 0);
				}else {
					pt.x = fi.ptCursor.x;
				}
				pt.y = fi.ptCursor.y;
				::SendMessage(hwndOwner, MYWM_SETCARETPOS, 0, 0);
			}
			// アクティブにする
			ActivateFrameWindow(hwndOwner);
			::ReleaseMutex(hMutex);
			::CloseHandle(hMutex);
			return false;
		}
	}


	// プラグイン読み込み
	MY_TRACETIME(runningTimer, "Before Init Jack");
	// ジャック初期化
	JackManager::getInstance();
	MY_TRACETIME(runningTimer, "After Init Jack");

	MY_TRACETIME(runningTimer, "Before Load Plugins");
	// プラグイン読み込み
	PluginManager::getInstance().LoadAllPlugin();
	MY_TRACETIME(runningTimer, "After Load Plugins");

	// エディタアプリケーションを作成
	// グループIDを取得
	int nGroupId = cmdLine.GetGroupId();
	if (GetDllShareData().common.tabBar.bNewWindow && nGroupId == -1) {
		nGroupId = AppNodeManager::getInstance().GetFreeGroupId();
	}
	// CEditAppを作成
	pEditApp = &EditApp::getInstance();
	pEditApp->Create(GetProcessInstance(), nGroupId);
	EditWnd* pEditWnd = pEditApp->GetEditWindow();
	auto& activeView = pEditWnd->GetActiveView();
	if (!pEditWnd->GetHwnd()) {
		::ReleaseMutex(hMutex);
		::CloseHandle(hMutex);
		return false;	// EditWnd::Create()失敗のため終了
	}

	// コマンドラインの解析
	bool bDebugMode = cmdLine.IsDebugMode();
	bool bGrepMode = cmdLine.IsGrepMode();
	bool bGrepDlg = cmdLine.IsGrepDlg();

	MY_TRACETIME(runningTimer, "CheckFile");

	// -1: SetDocumentTypeWhenCreate での強制指定なし
	const TypeConfigNum nType = (fi.szDocType[0] == '\0' ? TypeConfigNum(-1) : DocTypeManager().GetDocumentTypeOfExt(fi.szDocType));

	auto& editDoc = pEditWnd->GetDocument();
	if (bDebugMode) {
		// デバッグモニタモードに設定
		editDoc.SetCurDirNotitle();
		AppMode::getInstance().SetDebugModeON();
		if (!AppMode::getInstance().IsDebugMode()) {
			// デバッグではなくて(無題)
			AppNodeManager::getInstance().GetNoNameNumber(pEditWnd->GetHwnd());
			pEditWnd->UpdateCaption();
		}
		// アウトプット用タイプ別設定
		pEditWnd->SetDocumentTypeWhenCreate(fi.nCharCode, false, nType);
		pEditWnd->dlgFuncList.Refresh();	// アウトラインを表示する
	}else if (bGrepMode) {
		// GREP
		pEditWnd->SetDocumentTypeWhenCreate(fi.nCharCode, false, nType);
		pEditWnd->dlgFuncList.Refresh();	// アウトラインを予め表示しておく
		HWND hEditWnd = pEditWnd->GetHwnd();
		if (!::IsIconic(hEditWnd) && pEditWnd->dlgFuncList.GetHwnd()) {
			RECT rc;
			::GetClientRect(hEditWnd, &rc);
			::SendMessage(hEditWnd, WM_SIZE, ::IsZoomed(hEditWnd) ? SIZE_MAXIMIZED: SIZE_RESTORED, MAKELONG(rc.right - rc.left, rc.bottom - rc.top));
		}
		GrepInfo gi = cmdLine.GetGrepInfo();
		if (!bGrepDlg) {
			SetMainWindow(pEditWnd->GetHwnd());
			::ReleaseMutex(hMutex);
			::CloseHandle(hMutex);
			this->pEditApp->pGrepAgent->DoGrep(
				activeView,
				gi.bGrepReplace,
				&gi.mGrepKey,
				&gi.mGrepRep,
				&gi.mGrepFile,
				&gi.mGrepFolder,
				gi.bGrepCurFolder,
				gi.bGrepSubFolder,
				gi.bGrepStdout,
				gi.bGrepHeader,
				gi.grepSearchOption,
				gi.charEncoding,
				gi.nGrepOutputLineType,
				gi.nGrepOutputStyle,
				gi.bGrepOutputFileOnly,
				gi.bGrepOutputBaseFolder,
				gi.bGrepSeparateFolder,
				gi.bGrepPaste,
				gi.bGrepBackup
			);
			pEditWnd->dlgFuncList.Refresh();	// アウトラインを再解析する
		}else {
			AppNodeManager::getInstance().GetNoNameNumber(pEditWnd->GetHwnd());
			pEditWnd->UpdateCaption();
			
			if (gi.mGrepKey.GetStringLength() < _MAX_PATH) {
				SearchKeywordManager().AddToSearchKeys(gi.mGrepKey.GetStringPtr());
			}
			if (gi.mGrepFile.GetStringLength() < _MAX_PATH) {
				SearchKeywordManager().AddToGrepFiles(gi.mGrepFile.GetStringPtr());
			}
			NativeT memGrepFolder = gi.mGrepFolder;
			if (gi.mGrepFolder.GetStringLength() < _MAX_PATH) {
				SearchKeywordManager().AddToGrepFolders(gi.mGrepFolder.GetStringPtr());
				// 指定なしの場合はカレントフォルダにする
				if (memGrepFolder.GetStringLength() == 0) {
					TCHAR szCurDir[_MAX_PATH];
					::GetCurrentDirectory(_countof(szCurDir), szCurDir);
					memGrepFolder.SetString(szCurDir);
				}
			}
			auto& csSearch = GetDllShareData().common.search;
			csSearch.bGrepSubFolder = gi.bGrepSubFolder;
			csSearch.searchOption = gi.grepSearchOption;
			csSearch.nGrepCharSet = gi.charEncoding;
			csSearch.nGrepOutputLineType = gi.nGrepOutputLineType;
			csSearch.nGrepOutputStyle = gi.nGrepOutputStyle;
			SetMainWindow(pEditWnd->GetHwnd());
			::ReleaseMutex(hMutex);
			::CloseHandle(hMutex);
			hMutex = NULL;
			
			pEditWnd->dlgGrep.strText = gi.mGrepKey.GetStringPtr();		// 検索文字列
			pEditWnd->dlgGrep.bSetText = true;
			int nSize = _countof2(pEditWnd->dlgGrep.szFile);
			_tcsncpy(pEditWnd->dlgGrep.szFile, gi.mGrepFile.GetStringPtr(), nSize);	// 検索ファイル
			pEditWnd->dlgGrep.szFile[nSize - 1] = _T('\0');
			nSize = _countof2(pEditWnd->dlgGrep.szFolder);
			_tcsncpy(pEditWnd->dlgGrep.szFolder, memGrepFolder.GetStringPtr(), nSize);	// 検索フォルダ
			pEditWnd->dlgGrep.szFolder[nSize - 1] = _T('\0');

			INT_PTR nRet = pEditWnd->dlgGrep.DoModal(GetProcessInstance(), pEditWnd->GetHwnd(), NULL);
			if (nRet != FALSE) {
				activeView.GetCommander().HandleCommand(F_GREP, true, 0, 0, 0, 0);
			}else {
				// 自分はGrepでない
				editDoc.SetCurDirNotitle();
			}
			pEditWnd->dlgFuncList.Refresh();	// アウトラインを再解析する
		}

		// プラグイン：EditorStartイベント実行
		Plug::Array plugs;
		WSHIfObj::List params;
		JackManager::getInstance().GetUsablePlug(PP_EDITOR_START, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(activeView, params);
		}

		// プラグイン：DocumentOpenイベント実行
		plugs.clear();
		JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(activeView, params);
		}

		if (!bGrepDlg && gi.bGrepStdout) {
			// 即時終了
			PostMessage( pEditWnd->GetHwnd(), MYWM_CLOSE, PM_CLOSE_GREPNOCONFIRM | PM_CLOSE_EXIT, (LPARAM)NULL );
		}

		return true;
	}else {
		bViewMode = cmdLine.IsViewMode();
		if (fi.szPath[0] != _T('\0')) {
			// 文書タイプ指定
			pEditWnd->OpenDocumentWhenStart(
				LoadInfo(
					fi.szPath,
					fi.nCharCode,
					bViewMode,
					nType
				)
			);
			// 読み込み中断して「(無題)」になった時（他プロセスからのロックなど）もオプション指定を有効にする
			// Note. fi.nCharCode で文字コードが明示指定されていても、読み込み中断しない場合は別の文字コードが選択されることがある。
			//       以前は「(無題)」にならない場合でも無条件に SetDocumentTypeWhenCreate() を呼んでいたが、
			//       「前回と異なる文字コード」の問い合わせで前回の文字コードが選択された場合におかしくなっていた。
			if (!editDoc.docFile.GetFilePathClass().IsValidPath()) {
				// 読み込み中断して「(無題)」になった
				// ---> 無効になったオプション指定を有効にする
				pEditWnd->SetDocumentTypeWhenCreate(
					fi.nCharCode,
					bViewMode,
					nType
				);
			}
			if (
				(0 <= fi.nViewTopLine && 0 <= fi.nViewLeftCol)
				&& fi.nViewTopLine < (int)editDoc.layoutMgr.GetLineCount()
			) {
				activeView.GetTextArea().SetViewTopLine(fi.nViewTopLine);
				activeView.GetTextArea().SetViewLeftCol(fi.nViewLeftCol);
			}

			// オプション指定がないときはカーソル位置設定を行わないようにする
			// 0も位置としては有効な値なので判定に含めなくてはならない
			if (0 <= fi.ptCursor.x || 0 <= fi.ptCursor.y) {
				/*
				  カーソル位置変換
				  物理位置(行頭からのバイト数、折り返し無し行位置)
				  →
				  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
				*/
				Point ptPos = editDoc.layoutMgr.LogicToLayout(fi.ptCursor);
				// 改行の真ん中にカーソルが来ないように。
				const DocLine* pTmpDocLine = editDoc.docLineMgr.GetLine(fi.ptCursor.y);
				if (pTmpDocLine) {
					if ((int)pTmpDocLine->GetLengthWithoutEOL() < fi.ptCursor.x) {
						ptPos.x--;
					}
				}

				activeView.GetCaret().MoveCursor(ptPos, true);
				activeView.GetCaret().nCaretPosX_Prev =
					activeView.GetCaret().GetCaretLayoutPos().x;
			}
			activeView.RedrawAll();
		}else {
			editDoc.SetCurDirNotitle();	// (無題)ウィンドウ
			// ファイル名が与えられなくてもReadOnlyとタイプ指定を有効にする
			pEditWnd->SetDocumentTypeWhenCreate(
				fi.nCharCode,
				bViewMode,	// ビューモードか
				nType
			);
		}
		if (!editDoc.docFile.GetFilePathClass().IsValidPath()) {
			editDoc.SetCurDirNotitle();	// (無題)ウィンドウ
			AppNodeManager::getInstance().GetNoNameNumber(pEditWnd->GetHwnd());
			pEditWnd->UpdateCaption();
		}
	}

	SetMainWindow(pEditWnd->GetHwnd());

	activeView.SetIMECompFormPos();

	// WM_SIZEをポスト
	{	// ファイル読み込みしなかった場合にはこの WM_SIZE がアウトライン画面を配置する
		HWND hEditWnd = pEditWnd->GetHwnd();
		if (!::IsIconic(hEditWnd)) {
			RECT rc;
			::GetClientRect(hEditWnd, &rc);
			::PostMessage(hEditWnd, WM_SIZE, ::IsZoomed(hEditWnd) ? SIZE_MAXIMIZED: SIZE_RESTORED, MAKELONG(rc.right - rc.left, rc.bottom - rc.top));
		}
	}

	// 再描画
	::InvalidateRect(pEditWnd->GetHwnd(), NULL, TRUE);

	if (hMutex) {
		::ReleaseMutex(hMutex);
		::CloseHandle(hMutex);
	}

	// プラグイン：EditorStartイベント実行
	Plug::Array plugs;
	WSHIfObj::List params;
	JackManager::getInstance().GetUsablePlug(
			PP_EDITOR_START,
			0,
			&plugs
		);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		(*it)->Invoke(activeView, params);
	}

	// オープン後自動実行マクロを実行する
	if (!(bDebugMode || bGrepMode)) {
		editDoc.RunAutoMacro(GetDllShareData().common.macro.nMacroOnOpened);
	}

	// 起動時マクロオプション
	LPCWSTR pszMacro = cmdLine.GetMacro();
	if (pEditWnd->GetHwnd() && pszMacro && pszMacro[0] != L'\0') {
		LPCWSTR pszMacroType = cmdLine.GetMacroType();
		if (!pszMacroType || pszMacroType[0] == L'\0' || wcsicmp(pszMacroType, L"file") == 0) {
			pszMacroType = NULL;
		}
		activeView.GetCommander().HandleCommand(F_EXECEXTMACRO, true, (LPARAM)pszMacro, (LPARAM)pszMacroType, 0, 0);
	}

	// 複数ファイル読み込み
	size_t fileNum = cmdLine.GetFileNum();
	if (fileNum > 0) {
		size_t nDropFileNumMax = GetDllShareData().common.file.nDropFileNumMax - 1;
		// ファイルドロップ数の上限に合わせる
		if (fileNum > nDropFileNumMax) {
			fileNum = nDropFileNumMax;
		}
		EditInfo openFileInfo = fi;
		for (size_t i=0; i<fileNum; ++i) {
			// ファイル名差し替え
			_tcscpy_s(openFileInfo.szPath, cmdLine.GetFileName(i));
			bool ret = ControlTray::OpenNewEditor2(GetProcessInstance(), pEditWnd->GetHwnd(), openFileInfo, bViewMode);
			if (!ret) {
				break;
			}
		}
		// 用済みなので削除
		cmdLine.ClearFile();
	}

	// プラグイン：DocumentOpenイベント実行
	plugs.clear();
	JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		(*it)->Invoke(activeView, params);
	}

	return pEditWnd->GetHwnd() != NULL;
}

/*!
	@brief エディタプロセスのメッセージループ
*/
bool NormalProcess::MainLoop()
{
	if (GetMainWindow()) {
		pEditApp->GetEditWindow()->MessageLoop();	// メッセージループ
		return true;
	}
	return false;
}

/*!
	@brief エディタプロセスを終了する
	こいつはなにもしない。後始末はdtorで。
*/
void NormalProcess::OnExitProcess()
{
	// プラグイン解放
	PluginManager::getInstance().UnloadAllPlugin();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@brief Mutex(プロセス初期化の目印)を取得する

	多数同時に起動するとウィンドウが表に出てこないことがある。
*/
HANDLE NormalProcess::_GetInitializeMutex() const
{
	MY_RUNNINGTIMER(runningTimer, "NormalProcess::_GetInitializeMutex");
	HANDLE hMutex;
	std::tstring strProfileName = to_tchar(CommandLine::getInstance().GetProfileName());
	std::tstring strMutexInitName = GSTR_MUTEX_SAKURA_INIT;
	strMutexInitName += strProfileName;
	hMutex = ::CreateMutex( NULL, TRUE, strMutexInitName.c_str() );
	if (!hMutex) {
		ErrorBeep();
		TopErrorMessage(NULL, _T("CreateMutex()失敗。\n終了します。"));
		return NULL;
	}
	if (::GetLastError() == ERROR_ALREADY_EXISTS) {
		DWORD dwRet = ::WaitForSingleObject(hMutex, 15000);
		if (dwRet == WAIT_TIMEOUT) { // 別の誰かが起動中
			TopErrorMessage(NULL, _T("エディタまたはシステムがビジー状態です。\nしばらく待って開きなおしてください。"));
			::CloseHandle(hMutex);
			return NULL;
		}
	}
	return hMutex;
}

