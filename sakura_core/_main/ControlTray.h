/*!	@file
	@brief 常駐部

	タスクトレイアイコンの管理，タスクトレイメニューのアクション，
	MRU、キー割り当て、共通設定、編集ウィンドウの管理など
*/

#pragma once

#include <Windows.h>
#include "uiparts/MenuDrawer.h"
#include "uiparts/ImageListMgr.h"
#include "dlg/DlgGrep.h"

struct LoadInfo;
struct EditInfo;
struct DllSharedData;
class PropertyManager;

// 常駐部の管理
/*!
	タスクトレイアイコンの管理，タスクトレイメニューのアクション，
	MRU、キー割り当て、共通設定、編集ウィンドウの管理など
*/
class ControlTray {
public:
	/*
	||  Constructors
	*/
	ControlTray();
	~ControlTray();

	/*
	|| メンバ関数
	*/
	HWND Create(HINSTANCE);	// 作成
	bool CreateTrayIcon(HWND);
	LRESULT DispatchEvent(HWND, UINT, WPARAM, LPARAM);	// メッセージ処理
	void MessageLoop(void);	// メッセージループ
	void OnDestroy(void);		// WM_DESTROY 処理
	int	CreatePopUpMenu_L(void);	// ポップアップメニュー(トレイ左ボタン)
	int	CreatePopUpMenu_R(void);	// ポップアップメニュー(トレイ右ボタン)
	void CreateAccelTbl(void);	// アクセラレータテーブル作成
	void DeleteAccelTbl(void);	// アクセラレータテーブル破棄

	// ウィンドウ管理
	static bool OpenNewEditor(							// 新規編集ウィンドウの追加 ver 0
		HINSTANCE			hInstance,					// [in] インスタンスID (実は未使用)
		HWND				hWndParent,					// [in] 親ウィンドウハンドル．エラーメッセージ表示用
		const LoadInfo&		loadInfo,					// [in]
		const TCHAR*		szCmdLineOption	= NULL,		// [in] 追加のコマンドラインオプション
		bool				sync			= false,	// [in] trueなら新規エディタの起動まで待機する
		const TCHAR*		pszCurDir		= NULL,		// [in] 新規エディタのカレントディレクトリ
		bool				bNewWindow		= false		// [in] 新規エディタをウィンドウで開く
	);
	static bool OpenNewEditor2(						// 新規編集ウィンドウの追加 ver 1
		HINSTANCE		hInstance,
		HWND			hWndParent,
		const EditInfo&	editInfo,
		bool			bViewMode,
		bool			sync		= false,
		bool			bNewWindow	= false
	);
	static void ActiveNextWindow(HWND hwndParent);
	static void ActivePrevWindow(HWND hwndParent);

	static bool CloseAllEditor(bool bCheckConfirm, HWND hWndFrom, bool bExit, int nGroup);	// すべてのウィンドウを閉じる
	static void TerminateApplication(HWND hWndFrom);	// サクラエディタの全終了

public:
	HWND GetTrayHwnd() const { return hWnd; }

	/*
	|| 実装ヘルパ系
	*/
	static void DoGrepCreateWindow(HINSTANCE hinst, HWND, DlgGrep& dlgGrep);
protected:
	void DoGrep();
	BOOL TrayMessage(HWND, DWORD, UINT, HICON, const TCHAR*);	// タスクトレイのアイコンに関する処理
	void OnCommand(WORD, WORD, HWND);	// WM_COMMANDメッセージ処理
	void OnNewEditor(bool); // 新規ウィンドウ作成処理

	static INT_PTR CALLBACK ExitingDlgProc(	// 終了ダイアログ用プロシージャ
		HWND	hwndDlg,	// handle to dialog box
		UINT	uMsg,		// message
		WPARAM	wParam,		// first message parameter
		LPARAM	lParam		// second message parameter
	);

	/*
	|| メンバ変数
	*/
private:
	MenuDrawer			menuDrawer;
	PropertyManager*	pPropertyManager;
	bool				bUseTrayMenu;			// トレイメニュー表示中
	HINSTANCE			hInstance;
	HWND				hWnd;
	bool				bCreatedTrayIcon;		// トレイにアイコンを作った

	DllSharedData*		pShareData;
	DlgGrep				dlgGrep;
	int					nCurSearchKeySequence;

	ImageListMgr		hIcons;

	UINT				uCreateTaskBarMsg;	// RegisterMessageで得られるMessage IDの保管場所

	TCHAR				szLanguageDll[MAX_PATH];
};

