
/*!	@file
	@brief 常駐部
	
	タスクトレイアイコンの管理，タスクトレイメニューのアクション，
	MRU、キー割り当て、共通設定、編集ウィンドウの管理など
*/

#include "StdAfx.h"
#define ID_HOTKEY_TRAYMENU	0x1234

#include <HtmlHelp.h>
#include "ControlTray.h"
#include "PropertyManager.h"
#include "typeprop/DlgTypeList.h"
#include "debug/RunningTimer.h"
#include "dlg/DlgOpenFile.h"
#include "dlg/DlgAbout.h"
#include "plugin/PluginManager.h"
#include "plugin/JackManager.h"
#include "io/TextStream.h"
#include "util/module.h"
#include "util/shell.h"
#include "util/window.h"
#include "util/string_ex2.h"
#include "env/ShareData.h"
#include "env/ShareData_IO.h"
#include "env/SakuraEnvironment.h"
#include "env/HelpManager.h"
#include "doc/DocListener.h" // LoadInfo,EditInfo
#include "recent/MRUFile.h"
#include "recent/MRUFolder.h"
#include "_main/CommandLine.h"
#include "sakura_rc.h"

#define IDT_EDITCHECK 2
// 3秒
#define IDT_EDITCHECK_INTERVAL 3000
/////////////////////////////////////////////////////////////////////////
static LRESULT CALLBACK ControlTrayWndProc(HWND, UINT, WPARAM, LPARAM);

static ControlTray* g_pControlTray;

void ControlTray::DoGrep()
{
	// 前回のダイアログがあれば前面に
	if (::IsWindow(dlgGrep.GetHwnd())) {
		::OpenIcon(dlgGrep.GetHwnd());
		::BringWindowToTop(dlgGrep.GetHwnd());
		return;
	}

	auto& searchKeywords = pShareData->searchKeywords;
	auto& csSearch = pShareData->common.search;
	if (0 < searchKeywords.searchKeys.size()
		&& nCurSearchKeySequence < csSearch.nSearchKeySequence
	) {
		dlgGrep.strText = searchKeywords.searchKeys[0];
	}
	if (0 < searchKeywords.grepFiles.size()) {
		_tcscpy(dlgGrep.szFile, searchKeywords.grepFiles[0]);		// 検索ファイル
	}
	if (0 < searchKeywords.grepFolders.size()) {
		_tcscpy(dlgGrep.szFolder, searchKeywords.grepFolders[0]);	// 検索フォルダ
	}

	// Grepダイアログの表示
	INT_PTR nRet = dlgGrep.DoModal(hInstance, NULL, _T(""));
	if (!nRet || !GetTrayHwnd()) {
		return;
	}
	nCurSearchKeySequence = csSearch.nSearchKeySequence;
	DoGrepCreateWindow(hInstance, pShareData->handles.hwndTray, dlgGrep);
}

void ControlTray::DoGrepCreateWindow(HINSTANCE hinst, HWND msgParent, DlgGrep& dlgGrep)
{
	// ======= Grepの実行 =============
	// Grep結果ウィンドウの表示

	NativeT mWork1;
	NativeT mWork2;
	NativeT mWork3;
	mWork1.SetString(dlgGrep.strText.c_str());
	mWork2.SetString(dlgGrep.szFile);
	mWork3.SetString(dlgGrep.szFolder);
	mWork1.Replace(_T("\""), _T("\"\""));
	mWork2.Replace(_T("\""), _T("\"\""));
	mWork3.Replace(_T("\""), _T("\"\""));

	// -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
	NativeT cmdLine;
	cmdLine.AppendStringLiteral(_T("-GREPMODE -GKEY=\""));
	cmdLine.AppendStringT(mWork1.GetStringPtr());
	cmdLine.AppendStringLiteral(_T("\" -GFILE=\""));
	cmdLine.AppendStringT(mWork2.GetStringPtr());
	cmdLine.AppendStringLiteral(_T("\" -GFOLDER=\""));
	cmdLine.AppendStringT(mWork3.GetStringPtr());
	cmdLine.AppendStringLiteral(_T("\" -GCODE="));
	TCHAR szTemp[20];
	auto_sprintf_s(szTemp, _T("%d"), dlgGrep.nGrepCharSet);
	cmdLine.AppendString(szTemp);

	// GOPTオプション
	TCHAR pOpt[64] = _T("");
	if (dlgGrep.bSubFolder			) _tcscat(pOpt, _T("S"));	// サブフォルダからも検索する
	if (dlgGrep.searchOption.bLoHiCase	) _tcscat(pOpt, _T("L"));	// 英大文字と英小文字を区別する
	if (dlgGrep.searchOption.bRegularExp) _tcscat(pOpt, _T("R"));	// 正規表現
	if (dlgGrep.nGrepOutputLineType == 1) _tcscat(pOpt, _T("P"));	// 行を出力する
	if (dlgGrep.nGrepOutputLineType == 2) _tcscat(pOpt, _T("N"));	// 否ヒット行を出力する
	if (dlgGrep.searchOption.bWordOnly	) _tcscat(pOpt, _T("W"));	// 単語単位で探す
	if (dlgGrep.nGrepOutputStyle == 1	) _tcscat(pOpt, _T("1"));	// Grep: 出力形式
	if (dlgGrep.nGrepOutputStyle == 2	) _tcscat(pOpt, _T("2"));	// Grep: 出力形式
	if (dlgGrep.nGrepOutputStyle == 3	) _tcscat(pOpt, _T("3"));
	if (dlgGrep.bGrepOutputFileOnly		) _tcscat(pOpt, _T("F"));
	if (dlgGrep.bGrepOutputBaseFolder	) _tcscat(pOpt, _T("B"));
	if (dlgGrep.bGrepSeparateFolder		) _tcscat(pOpt, _T("D"));
	if (pOpt[0] != _T('\0')) {
		cmdLine.AppendStringLiteral(_T(" -GOPT="));
		cmdLine.AppendString(pOpt);
	}

	// 新規編集ウィンドウの追加 ver 0
	LoadInfo loadInfo;
	loadInfo.filePath = _T("");
	loadInfo.eCharCode = CODE_NONE;
	loadInfo.bViewMode = false;
	OpenNewEditor(
		hinst,
		msgParent,
		loadInfo,
		cmdLine.GetStringPtr(),
		false,
		NULL,
		GetDllShareData().common.tabBar.bNewWindow
	);
}


static LRESULT CALLBACK ControlTrayWndProc(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
	ControlTray* pApp;
	switch (uMsg) {
	case WM_CREATE:
		pApp = (ControlTray*)g_pControlTray;
		return pApp->DispatchEvent(hwnd, uMsg, wParam, lParam);
	default:
		// RELPRINT(_T("dispatch\n"));
		pApp = (ControlTray*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (pApp) {
			return pApp->DispatchEvent(hwnd, uMsg, wParam, lParam);
		}
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}


ControlTray::ControlTray()
	:
	pPropertyManager(nullptr),
	hInstance(NULL),
	hWnd(NULL),
	bCreatedTrayIcon(false),	// トレイにアイコンを作った
	nCurSearchKeySequence(-1),
	uCreateTaskBarMsg(::RegisterWindowMessage(TEXT("TaskbarCreated")))
{
	// 共有データ構造体のアドレスを返す
	pShareData = &GetDllShareData();

	// アクセラレータテーブル作成
	CreateAccelTbl();

	bUseTrayMenu = false;

	return;
}


ControlTray::~ControlTray()
{
	delete pPropertyManager;
	return;
}

/////////////////////////////////////////////////////////////////////////////
// ControlTray メンバ関数


// 作成
HWND ControlTray::Create(HINSTANCE hInstance)
{
	MY_RUNNINGTIMER(runningTimer, "ControlTray::Create");

	// 同名同クラスのウィンドウが既に存在していたら、失敗
	this->hInstance = hInstance;
	std::tstring strProfileName = to_tchar(CommandLine::getInstance().GetProfileName());
	std::tstring strCEditAppName = GSTR_CEDITAPP;
	strCEditAppName += strProfileName;
	HWND hwndWork = ::FindWindow(strCEditAppName.c_str(), strCEditAppName.c_str());
	if (hwndWork) {
		return NULL;
	}

	// ウィンドウクラス登録
	WNDCLASS wc;
	{
		wc.style			=	CS_HREDRAW |
								CS_VREDRAW |
								CS_DBLCLKS |
								CS_BYTEALIGNCLIENT |
								CS_BYTEALIGNWINDOW;
		wc.lpfnWndProc		= ControlTrayWndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= hInstance;
		wc.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= strCEditAppName.c_str();
		ATOM atom = RegisterClass(&wc);
		if (atom == 0) {
			ErrorMessage(NULL, LS(STR_TRAY_CREATE));
		}
	}
	g_pControlTray = this;

	// ウィンドウ作成 (WM_CREATEで、GetHwnd() に HWND が格納される)
	::CreateWindow(
		strCEditAppName.c_str(),			// pointer to registered class name
		strCEditAppName.c_str(),			// pointer to window name
		WS_OVERLAPPEDWINDOW/*WS_VISIBLE *//*| WS_CHILD *//* | WS_CLIPCHILDREN*/	,	// window style
		CW_USEDEFAULT,						// horizontal position of window
		0,									// vertical position of window
		100,								// window width
		100,								// window height
		NULL,								// handle to parent or owner window
		NULL,								// handle to menu or child-window identifier
		hInstance,						// handle to application instance
		NULL								// pointer to window-creation data
	);

	// 最前面にする（トレイからのポップアップウィンドウが最前面になるように）
	::SetWindowPos(
		GetTrayHwnd(),
		HWND_TOPMOST,
		0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE
	);
	
	// タスクトレイアイコン作成
	hIcons.Create(hInstance);
	menuDrawer.Create(SelectLang::getLangRsrcInstance(), GetTrayHwnd(), &hIcons);
	if (GetTrayHwnd()) {
		CreateTrayIcon(GetTrayHwnd());
	}

	pPropertyManager = new PropertyManager();
	pPropertyManager->Create(GetTrayHwnd(), &hIcons, &menuDrawer);

	auto_strcpy(szLanguageDll, pShareData->common.window.szLanguageDll);

	return GetTrayHwnd();
}

// タスクトレイにアイコンを登録する
bool ControlTray::CreateTrayIcon(HWND hWnd)
{
	// タスクトレイのアイコンを作る
	if (pShareData->common.general.bUseTaskTray) {	// タスクトレイのアイコンを使う
		HICON hIcon = GetAppIcon(hInstance, ICON_DEFAULT_APP, FN_APP_ICON, true);
		// バージョン情報
		// UR version no.を設定 (cf. dlgAbout.cpp)
		TCHAR	pszTips[64 + _MAX_PATH];
		// バージョン番号は、プロセスごとに取得する
		DWORD dwVersionMS, dwVersionLS;
		GetAppVersionInfo(
			NULL,
			VS_VERSION_INFO,
			&dwVersionMS,
			&dwVersionLS
		);

		std::wstring profname;
		if (CommandLine::getInstance().GetProfileName()[0] != L'\0') {
			profname = L" ";
			profname += CommandLine::getInstance().GetProfileName();
		}
		auto_snprintf_s(
			pszTips,
			_countof(pszTips),
			_T("%ts %d.%d.%d.%d%ls"),
			GSTR_APPNAME,
			HIWORD(dwVersionMS),
			LOWORD(dwVersionMS),
			HIWORD(dwVersionLS),
			LOWORD( dwVersionLS ),
			profname.c_str()
		);
		TrayMessage(GetTrayHwnd(), NIM_ADD, 0,  hIcon, pszTips);
		bCreatedTrayIcon = true;
	}
	return true;
}


// メッセージループ
void ControlTray::MessageLoop(void)
{
	// 複数プロセス版
	MSG	msg;
	int ret;
	
	while (GetTrayHwnd() && (ret = ::GetMessage(&msg, NULL, 0, 0 )) != 0) {
		if (ret == -1) {
			break;
		}
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
	return;

}


// タスクトレイのアイコンに関する処理
BOOL ControlTray::TrayMessage(
	HWND hDlg,
	DWORD dwMessage,
	UINT uID,
	HICON hIcon,
	const TCHAR* pszTip
	)
{
	NOTIFYICONDATA	tnd;
	tnd.cbSize				= sizeof_raw(tnd);
	tnd.hWnd				= hDlg;
	tnd.uID					= uID;
	tnd.uFlags				= NIF_MESSAGE|NIF_ICON|NIF_TIP;
	tnd.uCallbackMessage	= MYWM_NOTIFYICON;
	tnd.hIcon				= hIcon;
	if (pszTip) {
		lstrcpyn(tnd.szTip, pszTip, _countof(tnd.szTip));
	}else {
		tnd.szTip[0] = _T('\0');
	}
	BOOL res = Shell_NotifyIcon(dwMessage, &tnd);
	if (hIcon) {
		DestroyIcon(hIcon);
	}
	return res;
}


// メッセージ処理
// MRUリストは、CMRUに依頼する
LRESULT ControlTray::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
	int			nId;
	LPHELPINFO	lphi;

	EditNode*	pEditNodeArr;
	static HWND	hwndHtmlHelp;

	static WORD		wHotKeyMods;
	static WORD		wHotKeyCode;
	LPMEASUREITEMSTRUCT	lpmis;	// 項目サイズ情報
	LPDRAWITEMSTRUCT	lpdis;	// 項目描画情報
	int					nItemWidth;
	int					nItemHeight;
	static bool			bLDClick = false;	// 左ダブルクリックをしたか

	switch (uMsg) {
	case WM_MENUCHAR:
		// メニューアクセスキー押下時の処理(WM_MENUCHAR処理)
		return menuDrawer.OnMenuChar(hwnd, uMsg, wParam, lParam);
	case WM_DRAWITEM:
		lpdis = (DRAWITEMSTRUCT*) lParam;	// 項目描画情報
		switch (lpdis->CtlType) {
		case ODT_MENU:	// オーナー描画メニュー
			// メニューアイテム描画
			menuDrawer.DrawItem(lpdis);
			return TRUE;
		}
		return FALSE;
	case WM_MEASUREITEM:
		lpmis = (MEASUREITEMSTRUCT*) lParam;	// item-size information
		switch (lpmis->CtlType) {
		case ODT_MENU:	// オーナー描画メニュー
			// メニューアイテムの描画サイズを計算
			nItemWidth = menuDrawer.MeasureItem(lpmis->itemID, &nItemHeight);
			if (0 < nItemWidth) {
				lpmis->itemWidth = nItemWidth;
				lpmis->itemHeight = nItemHeight;
			}
			return TRUE;
		}
		return FALSE;
	case WM_EXITMENULOOP:
		menuDrawer.EndDrawMenu();
		break;

	// タスクトレイ左クリックメニューへのショートカットキー登録
	case WM_HOTKEY:
		{
			int		idHotKey = (int) wParam;				// identifier of hot key
			UINT	fuModifiers = (UINT) LOWORD(lParam);	// key-modifier flags
			UINT	uVirtKey = (UINT) HIWORD(lParam);		// virtual-key code
			TCHAR	szClassName[100];
			TCHAR	szText[256];

			HWND hwndWork = ::GetForegroundWindow();
			szClassName[0] = L'\0';
			::GetClassName(hwndWork, szClassName, _countof(szClassName) - 1);
			::GetWindowText(hwndWork, szText, _countof(szText) - 1);
			if (_tcscmp(szText, LS(STR_PROPCOMMON)) == 0) {
				return -1;
			}

			if (idHotKey == ID_HOTKEY_TRAYMENU
				&& (wHotKeyMods) == fuModifiers
				&& wHotKeyCode == uVirtKey
			) {
				::PostMessage(GetTrayHwnd(), MYWM_NOTIFYICON, 0, WM_LBUTTONUP);
			}
		}
		return 0;

	case WM_TIMER:
		// タイマメッセージ
		if (wParam == IDT_EDITCHECK) {
			// ウィンドウ存在確認。消えたウィンドウを抹消する
			bool bDelete = false;
			bool bDelFound;
			auto& nodes = pShareData->nodes;
			do {
				bDelFound = false;
				for (size_t i=0; i<nodes.nEditArrNum; ++i) {
					HWND target = nodes.pEditArr[i].GetHwnd();
					if (!IsSakuraMainWindow(target)) {
						AppNodeGroupHandle(nodes.pEditArr[i].nGroup).DeleteEditWndList(target);
						bDelete = bDelFound = true;
						// 1つ削除したらやり直し
						break;
					}
				}
			}while (bDelFound);
			if (bDelete && nodes.nEditArrNum == 0) {
				PostMessage(hwnd, MYWM_DELETE_ME, 0, 0);
			}
		}
		return 0;

	case MYWM_UIPI_CHECK:
		// エディタ−トレイ間でのUI特権分離の確認メッセージ
		::SendMessage((HWND)lParam, MYWM_UIPI_CHECK,  (WPARAM)0, (LPARAM)0);	// 返事を返す
		return 0L;

	case MYWM_HTMLHELP:
		{
			TCHAR* pWork = pShareData->workBuffer.GetWorkBuffer<TCHAR>();
			// szHtmlFile取得
			TCHAR szHtmlHelpFile[1024];
			_tcscpy_s(szHtmlHelpFile, pWork);
			size_t nLen = _tcslen(szHtmlHelpFile);
			hwndHtmlHelp = OpenHtmlHelp(
				NULL,
				szHtmlHelpFile,
				HH_DISPLAY_TOPIC,
				(DWORD_PTR)0,
				true
			);
			HH_AKLINK link;
			link.cbStruct		= sizeof_raw(link);
			link.fReserved		= FALSE;
			link.pszKeywords	= to_tchar(&pWork[nLen + 1]);
			link.pszUrl			= NULL;
			link.pszMsgText		= NULL;
			link.pszMsgTitle	= NULL;
			link.pszWindow		= NULL;
			link.fIndexOnFail	= TRUE;
			hwndHtmlHelp = OpenHtmlHelp(
				NULL,
				szHtmlHelpFile,
				HH_KEYWORD_LOOKUP,
				(DWORD_PTR)&link,
				false
			);
		}
		return (LRESULT)hwndHtmlHelp;

	// 編集ウィンドウオブジェクトからのオブジェクト削除要求
	case MYWM_DELETE_ME:
		{
			auto& csGeneral = pShareData->common.general;
			// タスクトレイのアイコンを常駐しない、または、トレイにアイコンを作っていない
			if (!(csGeneral.bStayTaskTray && csGeneral.bUseTaskTray) || !bCreatedTrayIcon) {
				// 現在開いている編集窓のリスト
				size_t nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true);
				if (0 < nRowNum) {
					delete[] pEditNodeArr;
				}
				// 編集ウィンドウの数が0になったら終了
				if (nRowNum == 0) {
					::SendMessage(hwnd, WM_CLOSE, 0, 0);
				}
			}
		}
		return 0;

	case WM_CREATE:
		{
			auto& csGeneral = pShareData->common.general;
			hWnd = hwnd;
			hwndHtmlHelp = NULL;
			::SetWindowLongPtr(GetTrayHwnd(), GWLP_USERDATA, (LONG_PTR)this);

			// タスクトレイ左クリックメニューへのショートカットキー登録
			wHotKeyMods = 0;
			if (csGeneral.wTrayMenuHotKeyMods & HOTKEYF_SHIFT) {
				wHotKeyMods |= MOD_SHIFT;
			}
			if (csGeneral.wTrayMenuHotKeyMods & HOTKEYF_CONTROL) {
				wHotKeyMods |= MOD_CONTROL;
			}
			if (csGeneral.wTrayMenuHotKeyMods & HOTKEYF_ALT) {
				wHotKeyMods |= MOD_ALT;
			}
			wHotKeyCode = csGeneral.wTrayMenuHotKeyCode;
			if (wHotKeyCode != 0) {
				::RegisterHotKey(
					GetTrayHwnd(),
					ID_HOTKEY_TRAYMENU,
					wHotKeyMods,
					wHotKeyCode
				);
			}

			// 最後の方でシャットダウンするアプリケーションにする
			BOOL (WINAPI *pfnSetProcessShutdownParameters)(DWORD dwLevel, DWORD dwFlags);
			HINSTANCE hDll = ::GetModuleHandle(_T("KERNEL32"));
			if (hDll) {
				*(FARPROC*)&pfnSetProcessShutdownParameters = ::GetProcAddress(hDll, "SetProcessShutdownParameters");
				if (pfnSetProcessShutdownParameters) {
					pfnSetProcessShutdownParameters(0x180, 0);
				}
			}

			// ウィンドウ存在確認
			::SetTimer(hwnd, IDT_EDITCHECK, IDT_EDITCHECK_INTERVAL, NULL);
		}
		return 0L;

//	case WM_QUERYENDSESSION:
	case WM_HELP:
		lphi = (LPHELPINFO) lParam;
		switch (lphi->iContextType) {
		case HELPINFO_MENUITEM:
			MyWinHelp(hwnd, HELP_CONTEXT, FuncID_To_HelpContextID((EFunctionCode)lphi->iCtrlId ));
			break;
		}
		return TRUE;
	case WM_COMMAND:
		OnCommand(HIWORD(wParam), LOWORD(wParam), (HWND) lParam);
		return 0L;

//		case MYWM_SETFILEINFO:
//			return 0L;
	case MYWM_CHANGESETTING:
		if ((e_PM_CHANGESETTING_SELECT)lParam == PM_CHANGESETTING_ALL) {
			{
				auto& csWindow = GetDllShareData().common.window;
				bool bChangeLang = auto_strcmp(csWindow.szLanguageDll, szLanguageDll) != 0;
				auto_strcpy(szLanguageDll, csWindow.szLanguageDll);
				std::vector<std::wstring> values;
				if (bChangeLang) {
					ShareData::getInstance().ConvertLangValues(values, true);
				}
				// 言語を選択する
				SelectLang::ChangeLang(csWindow.szLanguageDll);
				if (bChangeLang) {
					ShareData::getInstance().ConvertLangValues(values, false);
				}
			}

			::UnregisterHotKey(GetTrayHwnd(), ID_HOTKEY_TRAYMENU);
			// タスクトレイ左クリックメニューへのショートカットキー登録
			wHotKeyMods = 0;
			auto& csGeneral = pShareData->common.general;
			if (csGeneral.wTrayMenuHotKeyMods & HOTKEYF_SHIFT) {
				wHotKeyMods |= MOD_SHIFT;
			}
			if (csGeneral.wTrayMenuHotKeyMods & HOTKEYF_CONTROL) {
				wHotKeyMods |= MOD_CONTROL;
			}
			if (csGeneral.wTrayMenuHotKeyMods & HOTKEYF_ALT) {
				wHotKeyMods |= MOD_ALT;
			}
			wHotKeyCode = csGeneral.wTrayMenuHotKeyCode;
			::RegisterHotKey(
				GetTrayHwnd(),
				ID_HOTKEY_TRAYMENU,
				wHotKeyMods,
				wHotKeyCode
			);

			// アクセラレータテーブル破棄
			DeleteAccelTbl();
			// アクセラレータテーブル作成
			CreateAccelTbl();
		}
		return 0L;

	case MYWM_SET_TYPESETTING:
		{
			int nIdx = (int)wParam;
			if (0 <= nIdx && pShareData->nTypesCount) {
				TypeConfig& type = pShareData->workBuffer.typeConfig;
				if (nIdx == 0) {
					pShareData->typeBasis = type;
					pShareData->typeBasis.nIdx = 0;
				}
				*(ShareData::getInstance().GetTypeSettings()[nIdx]) = type;
				ShareData::getInstance().GetTypeSettings()[nIdx]->nIdx = nIdx;
				auto& typeMini = pShareData->typesMini[nIdx];
				auto_strcpy(typeMini.szTypeName, type.szTypeName);
				auto_strcpy(typeMini.szTypeExts, type.szTypeExts);
				typeMini.id = type.id;
				typeMini.encoding = type.encoding;
			}else {
				return FALSE;
			}
		}
		return TRUE;
	case MYWM_GET_TYPESETTING:
		{
			int nIdx = (int)wParam;
			if (0 <= nIdx && pShareData->nTypesCount) {
				pShareData->workBuffer.typeConfig = *(ShareData::getInstance().GetTypeSettings()[nIdx]);
			}else {
				return FALSE;
			}
		}
		return TRUE;
	case MYWM_ADD_TYPESETTING:
		{
			int nInsert = (int)wParam;
			// "共通"の前には入れない
			if (0 < nInsert && nInsert <= pShareData->nTypesCount && nInsert < MAX_TYPES) {
				std::vector<TypeConfig*>& types = ShareData::getInstance().GetTypeSettings();
				TypeConfig* type = new TypeConfig();
				*type = *types[0]; // 基本をコピー
				type->nIdx = nInsert;
				type->id = (::GetTickCount() & 0x3fffffff) + nInsert * 0x10000;
				// 同じ名前のものがあったらその次にする
				int nAddNameNum = nInsert + 1;
				auto_sprintf_s(type->szTypeName, LS(STR_TRAY_TYPE_NAME), nAddNameNum); 
				for (int k=1; k<pShareData->nTypesCount; ++k) {
					if (auto_strcmp(types[k]->szTypeName, type->szTypeName) == 0) {
						++nAddNameNum;
						auto_sprintf_s(type->szTypeName, LS(STR_TRAY_TYPE_NAME), nAddNameNum); 
						k = 0;
					}
				}
				auto_strcpy(type->szTypeExts, _T(""));
				type->nRegexKeyMagicNumber = RegexKeyword::GetNewMagicNumber();
				types.resize(pShareData->nTypesCount + 1);
				int nTypeSizeOld = pShareData->nTypesCount;
				++pShareData->nTypesCount;
				for (int i=nTypeSizeOld; nInsert<i; --i) {
					types[i] = types[i-1];
					types[i]->nIdx = i;
					pShareData->typesMini[i] = pShareData->typesMini[i-1];
				}
				types[nInsert] = type;
				auto& typeMini = pShareData->typesMini[nInsert];
				auto_strcpy(typeMini.szTypeName, type->szTypeName);
				auto_strcpy(typeMini.szTypeExts, type->szTypeExts);
				typeMini.id = type->id;
				typeMini.encoding = type->encoding;
			}else {
				return FALSE;
			}
		}
		return TRUE;
	case MYWM_DEL_TYPESETTING:
		{
			int nDelPos = (int)wParam;
			if (0 < nDelPos && nDelPos < pShareData->nTypesCount && 1 < pShareData->nTypesCount) {
				int nTypeSizeOld = pShareData->nTypesCount;
				auto& types = ShareData::getInstance().GetTypeSettings();
				delete types[nDelPos];
				for (int i=nDelPos; i<nTypeSizeOld-1; ++i) {
					types[i] = types[i + 1];
					types[i]->nIdx = i;
					pShareData->typesMini[i] = pShareData->typesMini[i + 1];
				}
				types.resize(pShareData->nTypesCount - 1);
				pShareData->nTypesCount--;
				pShareData->typesMini[nTypeSizeOld - 1].szTypeName[0] = 0;
				pShareData->typesMini[nTypeSizeOld - 1].szTypeExts[0] = 0;
				pShareData->typesMini[nTypeSizeOld - 1].id = 0;
			}else {
				return FALSE;
			}
		}
		return TRUE;

	case MYWM_NOTIFYICON:
//			MYTRACE(_T("MYWM_NOTIFYICON\n"));
		switch (lParam) {
// キーワード：トレイ右クリックメニュー設定
		case WM_RBUTTONUP:
			::SetActiveWindow(GetTrayHwnd());
			::SetForegroundWindow(GetTrayHwnd());
			// ポップアップメニュー(トレイ右ボタン)
			nId = CreatePopUpMenu_R();
			switch (nId) {
			case F_HELP_CONTENTS:
				// ヘルプ目次
				ShowWinHelpContents(GetTrayHwnd());	// 目次を表示する
				break;
			case F_HELP_SEARCH:
				// ヘルプキーワード検索
				MyWinHelp(GetTrayHwnd(), HELP_KEY, (ULONG_PTR)_T(""));
				break;
			case F_EXTHELP1:
				// 外部ヘルプ１
				do {
					if (HelpManager().ExtWinHelpIsSet()) {	// 共通設定のみ確認
						break;
					}else {
						ErrorBeep();
					}
				}while (::MYMESSAGEBOX(
						NULL, MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST,
						GSTR_APPNAME,
						LS(STR_TRAY_EXTHELP1)
					) == IDYES
				);// do-while

				break;
			case F_EXTHTMLHELP:
				// 外部HTMLヘルプ
				{
//						CEditView::Command_ExtHTMLHelp();
				}
				break;
			case F_TYPE_LIST:	// タイプ別設定一覧
				{
					DlgTypeList			dlgTypeList;
					DlgTypeList::Result	result;
					result.documentType = TypeConfigNum(0);
					result.bTempChange = false;
					if (dlgTypeList.DoModal(G_AppInstance(), GetTrayHwnd(), &result)) {
						// タイプ別設定
						PluginManager::getInstance().LoadAllPlugin();
						pPropertyManager->OpenPropertySheetTypes(NULL, -1, result.documentType);
						PluginManager::getInstance().UnloadAllPlugin();
					}
				}
				break;
			case F_OPTION:	// 共通設定
				{
					PluginManager::getInstance().LoadAllPlugin();
					{
						// アイコンの登録
						const auto& plugs = JackManager::getInstance().GetPlugs(PP_COMMAND);
						menuDrawer.pIcons->ResetExtend();
						for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
							int iBitmap = MenuDrawer::TOOLBAR_ICON_PLUGCOMMAND_DEFAULT - 1;
							const Plug* plug = *it;
							if (!plug->sIcon.empty()) {
								iBitmap = menuDrawer.pIcons->Add(
									to_tchar(plug->plugin.GetFilePath(to_tchar(plug->sIcon.c_str())).c_str()) );
							}
							menuDrawer.AddToolButton( iBitmap, plug->GetFunctionCode() );
						}
					}
					pPropertyManager->OpenPropertySheet(NULL, -1, true);
					PluginManager::getInstance().UnloadAllPlugin();
				}
				break;
			case F_ABOUT:
				// バージョン情報
				{
					DlgAbout dlgAbout;
					dlgAbout.DoModal(hInstance, GetTrayHwnd());
				}
				break;
//				case IDM_EXITALL:
			case F_EXITALL:
				// サクラエディタの全終了
				ControlTray::TerminateApplication(GetTrayHwnd());
				break;
			default:
				break;
			}
			return 0L;

		case WM_LBUTTONDOWN:
			bLDClick = false;
			return 0L;
		case WM_LBUTTONUP:
//				MYTRACE(_T("WM_LBUTTONDOWN\n"));
			if (bLDClick) {
				bLDClick = false;
				return 0L;
			}
			::SetActiveWindow(GetTrayHwnd());
			::SetForegroundWindow(GetTrayHwnd());
			// ポップアップメニュー(トレイ左ボタン)
			nId = CreatePopUpMenu_L();
			switch (nId) {
			case F_FILENEW:	// 新規作成
				// 新規編集ウィンドウの追加
				OnNewEditor(false);
				break;
			case F_FILEOPEN:	// 開く
				{
					// ファイルオープンダイアログの初期化
					LoadInfo loadInfo;
					loadInfo.filePath = _T("");
					loadInfo.eCharCode = CODE_AUTODETECT;	// 文字コード自動判別
					loadInfo.bViewMode = false;
					DlgOpenFile dlgOpenFile;
					dlgOpenFile.Create(
						hInstance,
						NULL,
						_T("*.*"),
						SakuraEnvironment::GetDlgInitialDir(true).c_str(),
						MruFile().GetPathList(),
						MruFolder().GetPathList()	// OPENFOLDERリストのファイルのリスト
					);
					std::vector<std::tstring> files;
					if (!dlgOpenFile.DoModalOpenDlg(&loadInfo, &files)) {
						break;
					}
					if (!GetTrayHwnd()) {
						break;
					}
						
					// 新たな編集ウィンドウを起動
					size_t nSize = files.size();
					for (size_t f=0; f<nSize; ++f) {
						loadInfo.filePath = files[f].c_str();
						ControlTray::OpenNewEditor(
							hInstance,
							GetTrayHwnd(),
							loadInfo,
							NULL,
							true,
							NULL,
							pShareData->common.tabBar.bNewWindow
						);
					}
				}
				break;
			case F_GREP_DIALOG:
				// Grep
				DoGrep();
				break;
			case F_FILESAVEALL:
				AppNodeGroupHandle(0).PostMessageToAllEditors(
					WM_COMMAND,
					MAKELONG(F_FILESAVE_QUIET, 0),
					(LPARAM)0,
					NULL
				);
				break;
			case F_EXITALLEDITORS:
				// 編集の全終了
				ControlTray::CloseAllEditor(true, GetTrayHwnd(), true, 0);
				break;
			case F_EXITALL:
				// サクラエディタの全終了
				ControlTray::TerminateApplication(GetTrayHwnd());
				break;
			default:
				if (nId - IDM_SELWINDOW >= 0
					&& nId - IDM_SELWINDOW < (int)pShareData->nodes.nEditArrNum
				) {
					HWND hwndWork = pShareData->nodes.pEditArr[nId - IDM_SELWINDOW].GetHwnd();
					// アクティブにする
					ActivateFrameWindow(hwndWork);
				}else if (nId - IDM_SELMRU >= 0 && nId-IDM_SELMRU < 999) {

					// 新しい編集ウィンドウを開く
					const MruFile mru;
					EditInfo openEditInfo;
					mru.GetEditInfo(nId - IDM_SELMRU, &openEditInfo);

					if (pShareData->common.file.GetRestoreCurPosition()) {
						ControlTray::OpenNewEditor2(hInstance, GetTrayHwnd(), openEditInfo, false);
					}else {
						LoadInfo loadInfo;
						loadInfo.filePath = openEditInfo.szPath;
						loadInfo.eCharCode = openEditInfo.nCharCode;
						loadInfo.bViewMode = false;
						ControlTray::OpenNewEditor(
							hInstance,
							GetTrayHwnd(),
							loadInfo,
							NULL,
							false,
							NULL,
							pShareData->common.tabBar.bNewWindow
						);

					}
				}else if (
					nId - IDM_SELOPENFOLDER >= 0
					&& nId - IDM_SELOPENFOLDER  < 999
				) {
					// MRUリストのファイルのリスト
					const MruFile mru;
					std::vector<LPCTSTR> vMRU = mru.GetPathList();

					// OPENFOLDERリストのファイルのリスト
					const MruFolder mruFolder;
					std::vector<LPCTSTR> vOPENFOLDER = mruFolder.GetPathList();

					// UNCであれば接続を試みる
					NetConnect(mruFolder.GetPath(nId - IDM_SELOPENFOLDER));

					// ファイルオープンダイアログの初期化
					DlgOpenFile	dlgOpenFile;
					dlgOpenFile.Create(
						hInstance,
						NULL,
						_T("*.*"),
						vOPENFOLDER[nId - IDM_SELOPENFOLDER],
						vMRU,
						vOPENFOLDER
					);
					LoadInfo loadInfo(_T(""), CODE_AUTODETECT, false);
					std::vector<std::tstring> files;
					if (!dlgOpenFile.DoModalOpenDlg(&loadInfo, &files)) {
						break;
					}
					if (!GetTrayHwnd()) {
						break;
					}

					// 新たな編集ウィンドウを起動
					size_t nSize = files.size();
					for (size_t f=0; f<nSize; ++f) {
						loadInfo.filePath = files[f].c_str();
						ControlTray::OpenNewEditor(
							hInstance,
							GetTrayHwnd(),
							loadInfo,
							NULL,
							true,
							NULL,
							pShareData->common.tabBar.bNewWindow
						);
					}
				}
				break;
			}
			return 0L;
		case WM_LBUTTONDBLCLK:
			bLDClick = true;
			// 新規編集ウィンドウの追加
			OnNewEditor(pShareData->common.tabBar.bNewWindow);
			// この後で表示されたメニューは閉じる
			::PostMessage(GetTrayHwnd(), WM_CANCELMODE, 0, 0);
			return 0L;
		case WM_RBUTTONDBLCLK:
			return 0L;
		}
		break;

	case WM_QUERYENDSESSION:
		// すべてのウィンドウを閉じる
		if (CloseAllEditor(false, GetTrayHwnd(), true, 0)) {
			return TRUE;
		}else {
			return FALSE;
		}
	case WM_CLOSE:
		// すべてのウィンドウを閉じる
		if (CloseAllEditor(false, GetTrayHwnd(), true, 0)) {
			::DestroyWindow(hwnd);
		}
		return 0L;

	// Windows終了時の後処理．
	// Windows終了時はWM_CLOSEが呼ばれない上，DestroyWindowを
	// 呼び出す必要もない．また，メッセージループに戻らないので
	// メッセージループの後ろの処理をここで完了させる必要がある．
	case WM_ENDSESSION:
		// もしWindowsの終了が中断されたのなら何もしない
		if (wParam != FALSE) {
			OnDestroy();	// WM_DESTROY と同じ処理をする（トレイアイコンの破棄などもNT系では必要）
		}
		return 0;	// もうこのプロセスに制御が戻ることはない
	case WM_DESTROY:
		OnDestroy();

		// Windows にスレッドの終了を要求します。
		::PostQuitMessage(0);
		return 0L;
	case MYWM_ALLOWACTIVATE:
		::AllowSetForegroundWindow((DWORD)wParam);
		return 0L;
	default:
		if (uMsg == uCreateTaskBarMsg) {
			/* TaskTray Iconの再登録を要求するメッセージ．
				Explorerが再起動したときに送出される．*/
			CreateTrayIcon(GetTrayHwnd()) ;
		}
		break;	// default
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// WM_COMMANDメッセージ処理
void ControlTray::OnCommand(WORD wNotifyCode, WORD wID , HWND hwndCtl)
{
	switch (wNotifyCode) {
	// メニューからのメッセージ
	case 0:
		break;
	}
	return;
}

void ControlTray::OnNewEditor(bool bNewWindow)
{
	// 新規ウィンドウで開くオプションは、タブバー＆グループ化を前提とする
	auto& csTabBar = pShareData->common.tabBar;
	bNewWindow = bNewWindow
				 && csTabBar.bDispTabWnd
				 && !csTabBar.bDispTabWndMultiWin;

	// 編集ウィンドウを開く
	LoadInfo loadInfo;
	loadInfo.filePath = _T("");
	loadInfo.eCharCode = CODE_NONE;
	loadInfo.bViewMode = false;
	std::tstring tstrCurDir = SakuraEnvironment::GetDlgInitialDir(true);
	OpenNewEditor(
		hInstance,
		GetTrayHwnd(),
		loadInfo,
		NULL,
		false,
		tstrCurDir.c_str(),
		bNewWindow
	);
}

/*!
	新規編集ウィンドウの追加 ver 0
*/
bool ControlTray::OpenNewEditor(
	HINSTANCE			hInstance,			// [in] インスタンスID (実は未使用)
	HWND				hWndParent,			// [in] 親ウィンドウハンドル．エラーメッセージ表示用
	const LoadInfo&		loadInfo,			// [in]
	const TCHAR*		szCmdLineOption,	// [in] 追加のコマンドラインオプション
	bool				sync,				// [in] trueなら新規エディタの起動まで待機する
	const TCHAR*		pszCurDir,			// [in] 新規エディタのカレントディレクトリ(NULL可)
	bool				bNewWindow			// [in] 新規エディタを新しいウィンドウで開く
	)
{
	// 共有データ構造体のアドレスを返す
	DllSharedData& shareData = GetDllShareData();

	// 編集ウィンドウの上限チェック
	if (shareData.nodes.nEditArrNum >= MAX_EDITWINDOWS) {	// 最大値修正
		OkMessage(NULL, LS(STR_MAXWINDOW), MAX_EDITWINDOWS);
		return false;
	}

	// -- -- -- -- コマンドライン文字列を生成 -- -- -- -- //
	CommandLineString cmdLineBuf;

	// アプリケーションパス
	TCHAR szEXE[MAX_PATH + 1];
	::GetModuleFileName(NULL, szEXE, _countof(szEXE));
	cmdLineBuf.AppendF(_T("\"%ts\""), szEXE);

	// ファイル名
	if (loadInfo.filePath.c_str()[0] != _T('\0')) {
		cmdLineBuf.AppendF(_T(" \"%ts\""), loadInfo.filePath.c_str());
	}

	// コード指定
	if (IsValidCodeType(loadInfo.eCharCode)) {
		cmdLineBuf.AppendF(_T(" -CODE=%d"), loadInfo.eCharCode);
	}

	// ビューモード指定
	if (loadInfo.bViewMode) {
		cmdLineBuf.AppendF(_T(" -R"));
	}

	// グループID
	if (!bNewWindow) {	// 新規エディタをウィンドウで開く
		// グループIDを親ウィンドウから取得
		HWND hwndAncestor = MyGetAncestor(hWndParent, GA_ROOTOWNER2);
		int nGroup = AppNodeManager::getInstance().GetEditNode(hwndAncestor)->GetGroup();
		if (nGroup > 0) {
			cmdLineBuf.AppendF(_T(" -GROUP=%d"), nGroup);
		}
	}else {
		// 空いているグループIDを使用する
		cmdLineBuf.AppendF(_T(" -GROUP=%d"), AppNodeManager::getInstance().GetFreeGroupId());
	}

	if (CommandLine::getInstance().IsSetProfile()) {
		cmdLineBuf.AppendF( _T(" -PROF=\"%ls\""), CommandLine::getInstance().GetProfileName() );
	}

	// 追加のコマンドラインオプション
	TCHAR szResponseFile[_MAX_PATH] = _T("");
	struct ResponseFileDeleter {
		LPCTSTR fileName;
		ResponseFileDeleter()
			:
			fileName(NULL)
		{}
		~ResponseFileDeleter() {
			if (fileName && fileName[0]) {
				::DeleteFile(fileName);
				fileName = NULL;
			}
		}
	};
	ResponseFileDeleter respDeleter;
	if (szCmdLineOption) {
		// Grepなどで入りきらない場合はレスポンスファイルを利用する
		if (cmdLineBuf.max_size() < cmdLineBuf.size() + auto_strlen(szCmdLineOption)) {
			TCHAR szIniDir[_MAX_PATH];
			GetInidir(szIniDir);
			LPTSTR pszTempFile = _ttempnam(szIniDir, _T("skr_resp"));
			if (!pszTempFile) {
				ErrorMessage(hWndParent, LS(STR_TRAY_RESPONSEFILE));
				return false;
			}
			auto_strcpy(szResponseFile, pszTempFile);
			free(pszTempFile);
			TextOutputStream output(szResponseFile);
			if (!output) {
				ErrorMessage(hWndParent, LS(STR_TRAY_RESPONSEFILE));
				return false;
			}
			respDeleter.fileName = szResponseFile;
			// 出力
			output.WriteString(to_wchar(szCmdLineOption));
			output.Close();
			sync = true;
			cmdLineBuf.AppendF(_T(" -@=\"%ts\""), szResponseFile);
		}else {
			cmdLineBuf.AppendF(_T(" %ts"), szCmdLineOption);
		}
	}
	// -- -- -- -- プロセス生成 -- -- -- -- //

	// 無効なディレクトリのときはNULLに変更
	if (pszCurDir) {
		DWORD attr = GetFileAttributes(pszCurDir);
		if ((attr != -1) && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
		}else {
			pszCurDir = NULL;
		}
	}

	// プロセスの起動
	PROCESS_INFORMATION p;
	STARTUPINFO s;

	s.cb = sizeof_raw(s);
	s.lpReserved = NULL;
	s.lpDesktop = NULL;
	s.lpTitle = NULL;

	s.dwFlags = STARTF_USESHOWWINDOW;
	s.wShowWindow = SW_SHOWDEFAULT;
	s.cbReserved2 = 0;
	s.lpReserved2 = NULL;

	// エディタプロセスを起動
	DWORD dwCreationFlag = CREATE_DEFAULT_ERROR_MODE;
#ifdef _DEBUG
//	dwCreationFlag |= DEBUG_PROCESS;
#endif
	TCHAR szCmdLine[1024]; _tcscpy_s(szCmdLine, cmdLineBuf.c_str());
	BOOL bCreateResult = CreateProcess(
		szEXE,					// 実行可能モジュールの名前
		szCmdLine,				// コマンドラインの文字列
		NULL,					// セキュリティ記述子
		NULL,					// セキュリティ記述子
		FALSE,					// ハンドルの継承オプション
		dwCreationFlag,			// 作成のフラグ
		NULL,					// 新しい環境ブロック
		pszCurDir,				// カレントディレクトリの名前
		&s,						// スタートアップ情報
		&p						// プロセス情報
	);
	if (!bCreateResult) {
		// 失敗
		TCHAR* pMsg;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_IGNORE_INSERTS |
						FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						GetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPTSTR)&pMsg,
						0,
						NULL
		);
		ErrorMessage(
			hWndParent,
			LS(STR_TRAY_CREATEPROC1),
			szEXE,
			pMsg
		);
		::LocalFree((HLOCAL)pMsg);	// エラーメッセージバッファを解放
		return false;
	}

	bool bRet = true;
	if (sync) {
		// 起動したプロセスが完全に立ち上がるまでちょっと待つ．
		int nResult = WaitForInputIdle(p.hProcess, 10000);	// 最大10秒間待つ
		if (nResult != 0) {
			ErrorMessage(
				hWndParent,
				LS(STR_TRAY_CREATEPROC2),
				szEXE
			);
			bRet = false;
		}
	}else {
		// タブまとめ時は起動したプロセスが立ち上がるまでしばらくタイトルバーをアクティブに保つ
		if (shareData.common.tabBar.bDispTabWnd
			&& !shareData.common.tabBar.bDispTabWndMultiWin
		) {
			WaitForInputIdle(p.hProcess, 3000);
			sync = true;
		}
	}

	// MYWM_FIRST_IDLE が届くまでちょっとだけ余分に待つ
	// Note. 起動先プロセスが初期化処理中に COM 関数（SHGetFileInfo API なども含む）を実行すると、
	//       その時点で COM の同期機構が動いて WaitForInputIdle は終了してしまう可能性がある（らしい）。
	if (sync && bRet) {
		for (size_t i=0; i<200; ++i) {
			MSG msg;
			DWORD dwExitCode;
			if (::PeekMessage(&msg, 0, MYWM_FIRST_IDLE, MYWM_FIRST_IDLE, PM_REMOVE)) {
				if (msg.message == WM_QUIT) {	// 指定範囲外でも WM_QUIT は取り出される
					::PostQuitMessage((int)msg.wParam);
					break;
				}
				// 監視対象プロセスからのメッセージなら抜ける
				// そうでなければ破棄して次を取り出す
				if (msg.wParam == p.dwProcessId) {
					break;
				}
			}
			if (::GetExitCodeProcess(p.hProcess, &dwExitCode) && dwExitCode != STILL_ACTIVE) {
				break;	// 監視対象プロセスが終了した
			}
			::Sleep(10);
		}
	}

	CloseHandle(p.hThread);
	CloseHandle(p.hProcess);

	return bRet;
}


/*!	新規編集ウィンドウの追加 ver 2: */
bool ControlTray::OpenNewEditor2(
	HINSTANCE		hInstance,
	HWND			hWndParent,
	const EditInfo&	editInfo,
	bool			bViewMode,
	bool			sync,
	bool			bNewWindow			// [in] 新規エディタを新しいウィンドウで開く
	)
{

	// 共有データ構造体のアドレスを返す
	DllSharedData& shareData = GetDllShareData();

	// 編集ウィンドウの上限チェック
	if (shareData.nodes.nEditArrNum >= MAX_EDITWINDOWS) {	// 最大値修正
		OkMessage(NULL, LS(STR_MAXWINDOW), MAX_EDITWINDOWS);
		return false;
	}

	// 追加のコマンドラインオプション
	CommandLineString cmdLine;
	if (editInfo.ptCursor.x >= 0)   cmdLine.AppendF(_T(" -X=%d"), editInfo.ptCursor.x + 1);
	if (editInfo.ptCursor.y >= 0)   cmdLine.AppendF(_T(" -Y=%d"), editInfo.ptCursor.y + 1);
	if (editInfo.nViewLeftCol >= 0) cmdLine.AppendF(_T(" -VX=%d"), editInfo.nViewLeftCol + 1);
	if (editInfo.nViewTopLine >= 0) cmdLine.AppendF(_T(" -VY=%d"), editInfo.nViewTopLine + 1);
	LoadInfo loadInfo;
	loadInfo.filePath = editInfo.szPath;
	loadInfo.eCharCode = editInfo.nCharCode;
	loadInfo.bViewMode = bViewMode;
	return OpenNewEditor(
		hInstance,
		hWndParent,
		loadInfo,
		cmdLine.c_str(),
		sync,
		NULL,
		bNewWindow
	);
}

void ControlTray::ActiveNextWindow(HWND hwndParent)
{
	// 現在開いている編集窓のリストを得る
	EditNode* pEditNodeArr;
	size_t nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true);
	if (nRowNum > 0) {
		// 自分のウィンドウを調べる
		int nGroup = 0;
		size_t i;
		for (i=0; i<nRowNum; ++i) {
			if (hwndParent == pEditNodeArr[i].GetHwnd()) {
				nGroup = pEditNodeArr[i].nGroup;
				break;
			}
		}
		if (i < nRowNum) {
			// 前のウィンドウ
			int j;
			for (j=(int)i-1; j>=0; --j) {
				if (nGroup == pEditNodeArr[j].nGroup) {
					break;
				}
			}
			if (j < 0) {
				for (j=(int)nRowNum-1; j>(int)i; --j) {
					if (nGroup == pEditNodeArr[j].nGroup) {
						break;
					}
				}
			}
			// 前のウィンドウをアクティブにする
			HWND hwndWork = pEditNodeArr[j].GetHwnd();
			ActivateFrameWindow(hwndWork);
			// 最後のペインをアクティブにする
			::PostMessage(hwndWork, MYWM_SETACTIVEPANE, (WPARAM) - 1, 1);
		}
		delete[] pEditNodeArr;
	}
}

void ControlTray::ActivePrevWindow(HWND hwndParent)
{
	// 現在開いている編集窓のリストを得る
	EditNode* pEditNodeArr;
	size_t nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true);
	if (nRowNum > 0) {
		// 自分のウィンドウを調べる
		int nGroup = 0;
		size_t i;
		for (i=0; i<nRowNum; ++i) {
			if (hwndParent == pEditNodeArr[i].GetHwnd()) {
				nGroup = pEditNodeArr[i].nGroup;
				break;
			}
		}
		if (i < nRowNum) {
			// 次のウィンドウ
			size_t j;
			for (j=i+1; j<nRowNum; ++j) {
				if (nGroup == pEditNodeArr[j].nGroup) {
					break;
				}
			}
			if (j >= nRowNum) {
				for (j=0; j<i; ++j) {
					if (nGroup == pEditNodeArr[j].nGroup) {
						break;
					}
				}
			}
			// 次のウィンドウをアクティブにする
			HWND hwndWork = pEditNodeArr[j].GetHwnd();
			ActivateFrameWindow(hwndWork);
			// 最初のペインをアクティブにする
			::PostMessage(hwndWork, MYWM_SETACTIVEPANE, (WPARAM) - 1, 0);
		}
		delete[] pEditNodeArr;
	}
}

/*!	サクラエディタの全終了 */
void ControlTray::TerminateApplication(
	HWND hWndFrom	// [in] 呼び出し元のウィンドウハンドル
	)
{
	DllSharedData& shareData = GetDllShareData();	// 共有データ構造体のアドレスを返す

	// 現在の編集ウィンドウの数を調べる
	if (shareData.common.general.bExitConfirm) {	// 終了時の確認
		if (0 < AppNodeGroupHandle(0).GetEditorWindowsNum()) {
			if (::MYMESSAGEBOX(
					hWndFrom,
					MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION,
					GSTR_APPNAME,
					LS(STR_TRAY_EXITALL)
				) != IDYES
			) {
				return;
			}
		}
	}
	// 「すべてのウィンドウを閉じる」要求
	bool bCheckConfirm = shareData.common.general.bExitConfirm;
	if (CloseAllEditor(bCheckConfirm, hWndFrom, true, 0)) {
		::PostMessage(shareData.handles.hwndTray, WM_CLOSE, 0, 0);
	}
	return;
}


/*!	すべてのウィンドウを閉じる */
bool ControlTray::CloseAllEditor(
	bool	bCheckConfirm,	// [in] [すべて閉じる]確認オプションに従って問い合わせをするかどうか
	HWND	hWndFrom,		// [in] 呼び出し元のウィンドウハンドル
	bool	bExit,			// [in] true: 編集の全終了 / false: すべて閉じる
	int		nGroup			// [in] グループID
	)
{
	EditNode* pWndArr;
	size_t n = AppNodeManager::getInstance().GetOpenedWindowArr(&pWndArr, false);
	if (n == 0) {
		return true;
	}
	
	// 全編集ウィンドウへ終了要求を出す
	bool bRes = AppNodeGroupHandle(nGroup).RequestCloseEditor(pWndArr, n, bExit, bCheckConfirm, hWndFrom);
	delete[] pWndArr;
	return bRes;
}


// ポップアップメニュー(トレイ左ボタン)
int	ControlTray::CreatePopUpMenu_L(void)
{
	// 本当はセマフォにしないとだめ
	if (bUseTrayMenu) {
		return -1;
	}
	bUseTrayMenu = true;

	menuDrawer.ResetContents();
	FileNameManager::getInstance().TransformFileName_MakeCache();

	// リソースを使わないように
	HMENU hMenuTop = ::CreatePopupMenu();
	HMENU hMenu = ::CreatePopupMenu();
	menuDrawer.MyAppendMenu(hMenuTop, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenu, L"TrayL", L"");

	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW, _T(""), _T("N"), false);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN, _T(""), _T("O"), false);

	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_GREP_DIALOG, _T(""), _T("G"), false);
	menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, false);

	// MRUリストのファイルのリストをメニューにする
	const MruFile mru;
	HMENU hMenuPopUp = mru.CreateMenu(menuDrawer);	// ファイルメニュー
	int nEnable = (mru.MenuLength() > 0 ? 0 : MF_GRAYED);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | nEnable, (UINT_PTR)hMenuPopUp , LS(F_FILE_RCNTFILE_SUBMENU), _T("F"));

	// 最近使ったフォルダのメニューを作成
	const MruFolder mruFolder;
	hMenuPopUp = mruFolder.CreateMenu(menuDrawer);
	nEnable = (mruFolder.MenuLength() > 0 ? 0 : MF_GRAYED);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP| nEnable, (UINT_PTR)hMenuPopUp, LS(F_FILE_RCNTFLDR_SUBMENU), _T("D"));

	menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, false);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVEALL, _T(""), _T("Z"), false);

	// 現在開いている編集窓のリストをメニューにする
	size_t j = 0;
	for (size_t i=0; i<pShareData->nodes.nEditArrNum; ++i) {
		if (IsSakuraMainWindow(pShareData->nodes.pEditArr[i].GetHwnd())) {
			++j;
		}
	}

	if (j > 0) {
		menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, false);

		NONCLIENTMETRICS met;
		met.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, met.cbSize, &met, met.cbSize);
		DCFont dcFont(met.lfMenuFont);

		j = 0;
		TCHAR szMenu[100 + MAX_PATH * 2];
		for (size_t i=0; i<pShareData->nodes.nEditArrNum; ++i) {
			if (IsSakuraMainWindow(pShareData->nodes.pEditArr[i].GetHwnd())) {
				// トレイからエディタへの編集ファイル名要求通知
				::SendMessage(pShareData->nodes.pEditArr[i].GetHwnd(), MYWM_GETFILEINFO, 0, 0);
				EditInfo* pfi = (EditInfo*)&pShareData->workBuffer.editInfo_MYWM_GETFILEINFO;

				// メニューラベル。1からアクセスキーを振る
				FileNameManager::getInstance().GetMenuFullLabel_WinList(
					szMenu,
					_countof(szMenu),
					pfi,
					pShareData->nodes.pEditArr[i].nId,
					i,
					dcFont.GetHDC()
				);
				menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, IDM_SELWINDOW + i, szMenu, _T(""), false);
				++j;
			}
		}
	}
	menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, false);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_EXITALLEDITORS, _T(""), _T("Q"), false);
	if (j == 0) {
		::EnableMenuItem(hMenu, F_EXITALLEDITORS, MF_BYCOMMAND | MF_GRAYED);
		::EnableMenuItem(hMenu, F_FILESAVEALL, MF_BYCOMMAND | MF_GRAYED);
	}

	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL, _T(""), _T("X"), false);

	POINT po;
	po.x = 0;
	po.y = 0;
	::GetCursorPos(&po);
	po.y -= 4;

	RECT rc;
	rc.left = 0;
	rc.right = 0;
	rc.top = 0;
	rc.bottom = 0;

	::SetForegroundWindow(GetTrayHwnd());
	int nId = ::TrackPopupMenu(
		hMenu,
		TPM_BOTTOMALIGN
		| TPM_RIGHTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		/*| TPM_RIGHTBUTTON*/
		,
		po.x,
		po.y,
		0,
		GetTrayHwnd(),
		&rc
	);
	::PostMessage(GetTrayHwnd(), WM_USER + 1, 0, 0);
	::DestroyMenu(hMenuTop);
//	MYTRACE(_T("nId=%d\n"), nId);

	bUseTrayMenu = false;

	return nId;
}

// キーワード：トレイ右クリックメニュー順序

// ポップアップメニュー(トレイ右ボタン)
int	ControlTray::CreatePopUpMenu_R(void)
{
	// 本当はセマフォにしないとだめ
	if (bUseTrayMenu) {
		return -1;
	}
	bUseTrayMenu = true;

	menuDrawer.ResetContents();

	// リソースを使わないように
	HMENU hMenuTop = ::CreatePopupMenu();
	HMENU hMenu = ::CreatePopupMenu();
	menuDrawer.MyAppendMenu(hMenuTop, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenu, L"TrayR", L"");

	// トレイ右クリックの「ヘルプ」メニュー
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_HELP_CONTENTS , _T(""), _T("O"), false);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_HELP_SEARCH , _T(""), _T("S"), false);
	menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, false);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_TYPE_LIST, _T(""), _T("L"), false);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_OPTION, _T(""), _T("C"), false);
	menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, false);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_ABOUT, _T(""), _T("A"), false);
	menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, false);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL, _T(""), _T("X"), false);

	POINT po;
	po.x = 0;
	po.y = 0;
	::GetCursorPos(&po);
	po.y -= 4;

	RECT rc;
	rc.left = 0;
	rc.right = 0;
	rc.top = 0;
	rc.bottom = 0;

	::SetForegroundWindow(GetTrayHwnd());
	int nId = ::TrackPopupMenu(
		hMenu,
		TPM_BOTTOMALIGN
		| TPM_RIGHTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		/*| TPM_RIGHTBUTTON*/
		,
		po.x,
		po.y,
		0,
		GetTrayHwnd(),
		&rc
	);
	::PostMessage(GetTrayHwnd(), WM_USER + 1, 0, 0);
	::DestroyMenu(hMenuTop);
//	MYTRACE(_T("nId=%d\n"), nId);

	bUseTrayMenu = false;

	return nId;
}

/*! アクセラレータテーブル作成 */
void ControlTray::CreateAccelTbl(void)
{
	auto& csKeyBind = pShareData->common.keyBind;
	pShareData->handles.hAccel = KeyBind::CreateAccerelator(
		csKeyBind.nKeyNameArrNum,
		csKeyBind.pKeyNameArr
	);

	if (!pShareData->handles.hAccel) {
		ErrorMessage(
			NULL,
			LS(STR_TRAY_ACCELTABLE)
		);
	}
}

/*! アクセラレータテーブル破棄 */
void ControlTray::DeleteAccelTbl(void)
{
	if (pShareData->handles.hAccel) {
		::DestroyAcceleratorTable(pShareData->handles.hAccel);
		pShareData->handles.hAccel = NULL;
	}
}

/*!
	@brief WM_DESTROY 処理
*/
void ControlTray::OnDestroy()
{
	HWND hwndExitingDlg = 0;

	if (!GetTrayHwnd()) {
		return;	// 既に破棄されている
	}

	// ホットキーの破棄
	::UnregisterHotKey(GetTrayHwnd(), ID_HOTKEY_TRAYMENU);

	// 「タスクトレイに常駐しない」設定でエディタ画面（Normal Process）を立ち上げたまま
	// セッション終了するような場合でも共有データ保存が行われなかったり中断されることが
	// 無いよう、ここでウィンドウが破棄される前に保存する
	//

	// 終了ダイアログを表示する
	if (pShareData->common.general.bDispExitingDialog) {
		// 終了中ダイアログの表示
		hwndExitingDlg = ::CreateDialog(
			hInstance,
			MAKEINTRESOURCE(IDD_EXITING),
			GetTrayHwnd()/*::GetDesktopWindow()*/,
			ExitingDlgProc
		);
		::ShowWindow(hwndExitingDlg, SW_SHOW);
	}

	// 共有データの保存
	ShareData_IO::SaveShareData();

	// 終了ダイアログを表示する
	if (pShareData->common.general.bDispExitingDialog) {
		// 終了中ダイアログの破棄
		::DestroyWindow(hwndExitingDlg);
	}

	if (bCreatedTrayIcon) {	// トレイにアイコンを作った
		TrayMessage(GetTrayHwnd(), NIM_DELETE, 0, NULL, NULL);
	}

	// アクセラレータテーブルの削除
	DeleteAccelTbl();

	hWnd = NULL;
}

/*!
	@brief 終了ダイアログ用プロシージャ
*/
INT_PTR CALLBACK ControlTray::ExitingDlgProc(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam		// second message parameter
	)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		return TRUE;
	}
	return FALSE;
}

