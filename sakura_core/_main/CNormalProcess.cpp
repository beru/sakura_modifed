/*!	@file
	@brief エディタプロセスクラス

	@author aroka
	@date 2002/01/07 Create
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka Processより分離
	Copyright (C) 2002, YAZAKI, Moca, genta
	Copyright (C) 2003, genta, Moca, MIK
	Copyright (C) 2004, Moca, naoh
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, Uchi
	Copyright (C) 2009, syat, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "StdAfx.h"
#include "CNormalProcess.h"
#include "CCommandLine.h"
#include "CControlTray.h"
#include "window/CEditWnd.h" // 2002/2/3 aroka
#include "CGrepAgent.h"
#include "doc/CEditDoc.h"
#include "doc/logic/CDocLine.h" // 2003/03/28 MIK
#include "debug/CRunningTimer.h"
#include "util/window.h"
#include "util/file.h"
#include "plugin/CPluginManager.h"
#include "plugin/CJackManager.h"
#include "CAppMode.h"
#include "env/CDocTypeManager.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

NormalProcess::NormalProcess(HINSTANCE hInstance, LPCTSTR lpCmdLine)
	:
	Process(hInstance, lpCmdLine),
	m_pcEditApp(NULL)
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
	
	CEditWndを作成する。
	
	@author aroka
	@date 2002/01/07

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、Processにひとつあるのみ。
	@date 2004.05.13 Moca CEditWnd::Create()に失敗した場合にfalseを返すように．
	@date 2007.06.26 ryoji グループIDを指定して編集ウィンドウを作成する
	@date 2012.02.25 novice 複数ファイル読み込み
*/
bool NormalProcess::InitializeProcess()
{
	MY_RUNNINGTIMER(cRunningTimer, "NormalProcess::Init");

	// プロセス初期化の目印
	HANDLE	hMutex = _GetInitializeMutex();	// 2002/2/8 aroka 込み入っていたので分離
	if (!hMutex) {
		return false;
	}

	// 共有メモリを初期化する
	if (!Process::InitializeProcess()) {
		return false;
	}

	// 言語を選択する
	SelectLang::ChangeLang(GetDllShareData().m_common.m_sWindow.m_szLanguageDll);

	// コマンドラインオプション
	bool			bViewMode = false;
	bool			bDebugMode;
	bool			bGrepMode;
	bool			bGrepDlg;
	GrepInfo		gi;
	EditInfo		fi;
	
	auto& cmdLine = *CommandLine::getInstance();
	// コマンドラインで受け取ったファイルが開かれている場合は
	// その編集ウィンドウをアクティブにする
	cmdLine.GetEditInfo(&fi); // 2002/2/8 aroka ここに移動
	if (fi.m_szPath[0] != _T('\0')) {
		// Oct. 27, 2000 genta
		// MRUからカーソル位置を復元する操作はCEditDoc::FileLoadで
		// 行われるのでここでは必要なし．

		// 指定ファイルが開かれているか調べる
		// 2007.03.13 maru 文字コードが異なるときはワーニングを出すように
		HWND hwndOwner;
		if (GetShareData().ActiveAlreadyOpenedWindow(fi.m_szPath, &hwndOwner, fi.m_nCharCode)) {
			// From Here Oct. 19, 2001 genta
			// カーソル位置が引数に指定されていたら指定位置にジャンプ
			if (fi.m_ptCursor.y >= 0) {	// 行の指定があるか
				CLogicPoint& pt = GetDllShareData().m_workBuffer.m_LogicPoint;
				if (fi.m_ptCursor.x < 0) {
					// 桁の指定が無い場合
					::SendMessage(hwndOwner, MYWM_GETCARETPOS, 0, 0);
				}else {
					pt.x = fi.m_ptCursor.x;
				}
				pt.y = fi.m_ptCursor.y;
				::SendMessage(hwndOwner, MYWM_SETCARETPOS, 0, 0);
			}
			// To Here Oct. 19, 2001 genta
			// アクティブにする
			ActivateFrameWindow(hwndOwner);
			::ReleaseMutex(hMutex);
			::CloseHandle(hMutex);
			return false;
		}
	}


	// プラグイン読み込み
	MY_TRACETIME(cRunningTimer, "Before Init Jack");
	// ジャック初期化
	CJackManager::getInstance();
	MY_TRACETIME(cRunningTimer, "After Init Jack");

	MY_TRACETIME(cRunningTimer, "Before Load Plugins");
	// プラグイン読み込み
	CPluginManager::getInstance()->LoadAllPlugin();
	MY_TRACETIME(cRunningTimer, "After Load Plugins");

	// エディタアプリケーションを作成。2007.10.23 kobake
	// グループIDを取得
	int nGroupId = cmdLine.GetGroupId();
	if (GetDllShareData().m_common.m_sTabBar.m_bNewWindow && nGroupId == -1) {
		nGroupId = CAppNodeManager::getInstance()->GetFreeGroupId();
	}
	// CEditAppを作成
	m_pcEditApp = CEditApp::getInstance();
	m_pcEditApp->Create(GetProcessInstance(), nGroupId);
	CEditWnd* pEditWnd = m_pcEditApp->GetEditWindow();
	auto& activeView = pEditWnd->GetActiveView();
	if (!pEditWnd->GetHwnd()) {
		::ReleaseMutex(hMutex);
		::CloseHandle(hMutex);
		return false;	// 2009.06.23 ryoji CEditWnd::Create()失敗のため終了
	}

	// コマンドラインの解析		2002/2/8 aroka ここに移動
	bDebugMode = cmdLine.IsDebugMode();
	bGrepMode  = cmdLine.IsGrepMode();
	bGrepDlg   = cmdLine.IsGrepDlg();

	MY_TRACETIME(cRunningTimer, "CheckFile");

	// -1: SetDocumentTypeWhenCreate での強制指定なし
	const CTypeConfig nType = (fi.m_szDocType[0] == '\0' ? CTypeConfig(-1) : CDocTypeManager().GetDocumentTypeOfExt(fi.m_szDocType));

	if (bDebugMode) {
		// デバッグモニタモードに設定
		pEditWnd->GetDocument()->SetCurDirNotitle();
		AppMode::getInstance()->SetDebugModeON();
		if (!AppMode::getInstance()->IsDebugMode()) {
			// デバッグではなくて(無題)
			CAppNodeManager::getInstance()->GetNoNameNumber(pEditWnd->GetHwnd());
			pEditWnd->UpdateCaption();
		}
		// 2004.09.20 naoh アウトプット用タイプ別設定
		// 文字コードを有効とする Uchi 2008/6/8
		// 2010.06.16 Moca アウトプットは CCommnadLineで -TYPE=output 扱いとする
		pEditWnd->SetDocumentTypeWhenCreate(fi.m_nCharCode, false, nType);
		pEditWnd->m_cDlgFuncList.Refresh();	// アウトラインを表示する
	}else if (bGrepMode) {
		// GREP
		// 2010.06.16 Moca Grepでもオプション指定を適用
		pEditWnd->SetDocumentTypeWhenCreate(fi.m_nCharCode, false, nType);
		pEditWnd->m_cDlgFuncList.Refresh();	// アウトラインを予め表示しておく
		HWND hEditWnd = pEditWnd->GetHwnd();
		if (!::IsIconic(hEditWnd) && pEditWnd->m_cDlgFuncList.GetHwnd()) {
			RECT rc;
			::GetClientRect(hEditWnd, &rc);
			::SendMessage(hEditWnd, WM_SIZE, ::IsZoomed(hEditWnd) ? SIZE_MAXIMIZED: SIZE_RESTORED, MAKELONG(rc.right - rc.left, rc.bottom - rc.top));
		}
		cmdLine.GetGrepInfo(&gi); // 2002/2/8 aroka ここに移動
		if (!bGrepDlg) {
			// Grepでは対象パス解析に現在のカレントディレクトリを必要とする
			// pEditWnd->GetDocument()->SetCurDirNotitle();
			// 2003.06.23 Moca GREP実行前にMutexを開放
			// こうしないとGrepが終わるまで新しいウィンドウを開けない
			SetMainWindow(pEditWnd->GetHwnd());
			::ReleaseMutex(hMutex);
			::CloseHandle(hMutex);
			this->m_pcEditApp->m_pcGrepAgent->DoGrep(
				&activeView,
				gi.bGrepReplace,
				&gi.cmGrepKey,
				&gi.cmGrepRep,
				&gi.cmGrepFile,
				&gi.cmGrepFolder,
				gi.bGrepCurFolder,
				gi.bGrepSubFolder,
				gi.bGrepStdout,
				gi.bGrepHeader,
				gi.grepSearchOption,
				gi.nGrepCharSet,	// 2002/09/21 Moca
				gi.nGrepOutputLineType,
				gi.nGrepOutputStyle,
				gi.bGrepOutputFileOnly,
				gi.bGrepOutputBaseFolder,
				gi.bGrepSeparateFolder,
				gi.bGrepPaste,
				gi.bGrepBackup
			);
			pEditWnd->m_cDlgFuncList.Refresh();	// アウトラインを再解析する
			//return true; // 2003.06.23 Moca
		}else {
			CAppNodeManager::getInstance()->GetNoNameNumber(pEditWnd->GetHwnd());
			pEditWnd->UpdateCaption();
			
			//-GREPDLGでダイアログを出す。　引数も反映（2002/03/24 YAZAKI）
			if (gi.cmGrepKey.GetStringLength() < _MAX_PATH) {
				CSearchKeywordManager().AddToSearchKeyArr(gi.cmGrepKey.GetStringPtr());
			}
			if (gi.cmGrepFile.GetStringLength() < _MAX_PATH) {
				CSearchKeywordManager().AddToGrepFileArr(gi.cmGrepFile.GetStringPtr());
			}
			CNativeT cmemGrepFolder = gi.cmGrepFolder;
			if (gi.cmGrepFolder.GetStringLength() < _MAX_PATH) {
				CSearchKeywordManager().AddToGrepFolderArr(gi.cmGrepFolder.GetStringPtr());
				// 2013.05.21 指定なしの場合はカレントフォルダにする
				if (cmemGrepFolder.GetStringLength() == 0) {
					TCHAR szCurDir[_MAX_PATH];
					::GetCurrentDirectory(_countof(szCurDir), szCurDir);
					cmemGrepFolder.SetString(szCurDir);
				}
			}
			auto& csSearch = GetDllShareData().m_common.m_sSearch;
			csSearch.m_bGrepSubFolder = gi.bGrepSubFolder;
			csSearch.m_searchOption = gi.grepSearchOption;
			csSearch.m_nGrepCharSet = gi.nGrepCharSet;
			csSearch.m_nGrepOutputLineType = gi.nGrepOutputLineType;
			csSearch.m_nGrepOutputStyle = gi.nGrepOutputStyle;
			// 2003.06.23 Moca GREPダイアログ表示前にMutexを開放
			// こうしないとGrepが終わるまで新しいウィンドウを開けない
			SetMainWindow(pEditWnd->GetHwnd());
			::ReleaseMutex(hMutex);
			::CloseHandle(hMutex);
			hMutex = NULL;
			
			// Oct. 9, 2003 genta コマンドラインからGERPダイアログを表示させた場合に
			// 引数の設定がBOXに反映されない
			pEditWnd->m_cDlgGrep.m_strText = gi.cmGrepKey.GetStringPtr();		// 検索文字列
			pEditWnd->m_cDlgGrep.m_bSetText = true;
			int nSize = _countof2(pEditWnd->m_cDlgGrep.m_szFile);
			_tcsncpy(pEditWnd->m_cDlgGrep.m_szFile, gi.cmGrepFile.GetStringPtr(), nSize);	// 検索ファイル
			pEditWnd->m_cDlgGrep.m_szFile[nSize - 1] = _T('\0');
			nSize = _countof2(pEditWnd->m_cDlgGrep.m_szFolder);
			_tcsncpy(pEditWnd->m_cDlgGrep.m_szFolder, cmemGrepFolder.GetStringPtr(), nSize);	// 検索フォルダ
			pEditWnd->m_cDlgGrep.m_szFolder[nSize - 1] = _T('\0');

			
			// Feb. 23, 2003 Moca Owner windowが正しく指定されていなかった
			int nRet = pEditWnd->m_cDlgGrep.DoModal(GetProcessInstance(), pEditWnd->GetHwnd(), NULL);
			if (FALSE != nRet) {
				activeView.GetCommander().HandleCommand(F_GREP, true, 0, 0, 0, 0);
			}else {
				// 自分はGrepでない
				pEditWnd->GetDocument()->SetCurDirNotitle();
			}
			pEditWnd->m_cDlgFuncList.Refresh();	// アウトラインを再解析する
			//return true; // 2003.06.23 Moca
		}

		// プラグイン：EditorStartイベント実行
		CPlug::Array plugs;
		CWSHIfObj::List params;
		CJackManager::getInstance()->GetUsablePlug(PP_EDITOR_START, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(&activeView, params);
		}

		// プラグイン：DocumentOpenイベント実行
		plugs.clear();
		CJackManager::getInstance()->GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(&activeView, params);
		}

		if (!bGrepDlg && gi.bGrepStdout) {
			// 即時終了
			PostMessage( pEditWnd->GetHwnd(), MYWM_CLOSE, PM_CLOSE_GREPNOCONFIRM | PM_CLOSE_EXIT, (LPARAM)NULL );
		}

		return true; // 2003.06.23 Moca
	}else {
		// 2004.05.13 Moca さらにif分の中から前に移動
		// ファイル名が与えられなくてもReadOnly指定を有効にするため．
		bViewMode = cmdLine.IsViewMode(); // 2002/2/8 aroka ここに移動
		if (fi.m_szPath[0] != _T('\0')) {
			// Mar. 9, 2002 genta 文書タイプ指定
			pEditWnd->OpenDocumentWhenStart(
				LoadInfo(
					fi.m_szPath,
					fi.m_nCharCode,
					bViewMode,
					nType
				)
			);
			// 読み込み中断して「(無題)」になった時（他プロセスからのロックなど）もオプション指定を有効にする
			// Note. fi.m_nCharCode で文字コードが明示指定されていても、読み込み中断しない場合は別の文字コードが選択されることがある。
			//       以前は「(無題)」にならない場合でも無条件に SetDocumentTypeWhenCreate() を呼んでいたが、
			//       「前回と異なる文字コード」の問い合わせで前回の文字コードが選択された場合におかしくなっていた。
			if (!pEditWnd->GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath()) {
				// 読み込み中断して「(無題)」になった
				// ---> 無効になったオプション指定を有効にする
				pEditWnd->SetDocumentTypeWhenCreate(
					fi.m_nCharCode,
					bViewMode,
					nType
				);
			}
			// Nov. 6, 2000 genta
			// キャレット位置の復元のため
			// オプション指定がないときは画面移動を行わないようにする
			// Oct. 19, 2001 genta
			// 未設定＝-1になるようにしたので，安全のため両者が指定されたときだけ
			// 移動するようにする． || → &&
			if (
				(CLayoutInt(0) <= fi.m_nViewTopLine && CLayoutInt(0) <= fi.m_nViewLeftCol)
				&& fi.m_nViewTopLine < pEditWnd->GetDocument()->m_cLayoutMgr.GetLineCount()
			) {
				activeView.GetTextArea().SetViewTopLine(fi.m_nViewTopLine);
				activeView.GetTextArea().SetViewLeftCol(fi.m_nViewLeftCol);
			}

			// オプション指定がないときはカーソル位置設定を行わないようにする
			// Oct. 19, 2001 genta
			// 0も位置としては有効な値なので判定に含めなくてはならない
			if (0 <= fi.m_ptCursor.x || 0 <= fi.m_ptCursor.y) {
				/*
				  カーソル位置変換
				  物理位置(行頭からのバイト数、折り返し無し行位置)
				  →
				  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
				*/
				CLayoutPoint ptPos;
				pEditWnd->GetDocument()->m_cLayoutMgr.LogicToLayout(
					fi.m_ptCursor,
					&ptPos
				);

				// From Here Mar. 28, 2003 MIK
				// 改行の真ん中にカーソルが来ないように。
				// 2008.08.20 ryoji 改行単位の行番号を渡すように修正
				const CDocLine* pTmpDocLine = pEditWnd->GetDocument()->m_cDocLineMgr.GetLine(fi.m_ptCursor.GetY2());
				if (pTmpDocLine) {
					if (pTmpDocLine->GetLengthWithoutEOL() < fi.m_ptCursor.x) {
						ptPos.x--;
					}
				}
				// To Here Mar. 28, 2003 MIK

				activeView.GetCaret().MoveCursor(ptPos, true);
				activeView.GetCaret().m_nCaretPosX_Prev =
					activeView.GetCaret().GetCaretLayoutPos().GetX2();
			}
			activeView.RedrawAll();
		}else {
			pEditWnd->GetDocument()->SetCurDirNotitle();	// (無題)ウィンドウ
			// 2004.05.13 Moca ファイル名が与えられなくてもReadOnlyとタイプ指定を有効にする
			pEditWnd->SetDocumentTypeWhenCreate(
				fi.m_nCharCode,
				bViewMode,	// ビューモードか
				nType
			);
		}
		if (!pEditWnd->GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath()) {
			pEditWnd->GetDocument()->SetCurDirNotitle();	// (無題)ウィンドウ
			CAppNodeManager::getInstance()->GetNoNameNumber(pEditWnd->GetHwnd());
			pEditWnd->UpdateCaption();
		}
	}

	SetMainWindow(pEditWnd->GetHwnd());

	// YAZAKI 2002/05/30 IMEウィンドウの位置がおかしいのを修正。
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
	CPlug::Array plugs;
	CWSHIfObj::List params;
	CJackManager::getInstance()->GetUsablePlug(
			PP_EDITOR_START,
			0,
			&plugs
		);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		(*it)->Invoke(&activeView, params);
	}

	// 2006.09.03 ryoji オープン後自動実行マクロを実行する
	if (!(bDebugMode || bGrepMode)) {
		pEditWnd->GetDocument()->RunAutoMacro(GetDllShareData().m_common.m_sMacro.m_nMacroOnOpened);
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
	int fileNum = cmdLine.GetFileNum();
	if (fileNum > 0) {
		int nDropFileNumMax = GetDllShareData().m_common.m_sFile.m_nDropFileNumMax - 1;
		// ファイルドロップ数の上限に合わせる
		if (fileNum > nDropFileNumMax) {
			fileNum = nDropFileNumMax;
		}
		EditInfo openFileInfo = fi;
		for (int i=0; i<fileNum; ++i) {
			// ファイル名差し替え
			_tcscpy_s(openFileInfo.m_szPath, cmdLine.GetFileName(i));
			bool ret = ControlTray::OpenNewEditor2(GetProcessInstance(), pEditWnd->GetHwnd(), &openFileInfo, bViewMode);
			if (!ret) {
				break;
			}
		}
		// 用済みなので削除
		cmdLine.ClearFile();
	}

	// プラグイン：DocumentOpenイベント実行
	plugs.clear();
	CJackManager::getInstance()->GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		(*it)->Invoke(&activeView, params);
	}

	return pEditWnd->GetHwnd() != NULL;
}

/*!
	@brief エディタプロセスのメッセージループ
	
	@author aroka
	@date 2002/01/07
*/
bool NormalProcess::MainLoop()
{
	if (GetMainWindow()) {
		m_pcEditApp->GetEditWindow()->MessageLoop();	// メッセージループ
		return true;
	}
	return false;
}

/*!
	@brief エディタプロセスを終了する
	
	@author aroka
	@date 2002/01/07
	こいつはなにもしない。後始末はdtorで。
*/
void NormalProcess::OnExitProcess()
{
	// プラグイン解放
	CPluginManager::getInstance()->UnloadAllPlugin();		// Mpve here	2010/7/11 Uchi
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@brief Mutex(プロセス初期化の目印)を取得する

	多数同時に起動するとウィンドウが表に出てこないことがある。
	
	@date 2002/2/8 aroka InitializeProcessから移動
	@retval Mutex のハンドルを返す
	@retval 失敗した時はリリースしてから NULL を返す
*/
HANDLE NormalProcess::_GetInitializeMutex() const
{
	MY_RUNNINGTIMER(cRunningTimer, "NormalProcess::_GetInitializeMutex");
	HANDLE hMutex;
	std::tstring strProfileName = to_tchar(CommandLine::getInstance()->GetProfileName());
	std::tstring strMutexInitName = GSTR_MUTEX_SAKURA_INIT;
	strMutexInitName += strProfileName;
	hMutex = ::CreateMutex( NULL, TRUE, strMutexInitName.c_str() );
	if (!hMutex) {
		ErrorBeep();
		TopErrorMessage(NULL, _T("CreateMutex()失敗。\n終了します。"));
		return NULL;
	}
	if (::GetLastError() == ERROR_ALREADY_EXISTS) {
		DWORD dwRet = ::WaitForSingleObject(hMutex, 15000);	// 2002/2/8 aroka 少し長くした
		if (WAIT_TIMEOUT == dwRet) { // 別の誰かが起動中
			TopErrorMessage(NULL, _T("エディタまたはシステムがビジー状態です。\nしばらく待って開きなおしてください。"));
			::CloseHandle(hMutex);
			return NULL;
		}
	}
	return hMutex;
}

