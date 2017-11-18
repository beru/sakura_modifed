#pragma once

#include "dlg/Dialog.h"

class DlgCancel;

// キャンセルボタンダイアログ



/*!
	@brief キャンセルボタンダイアログ
*/
class DlgCancel : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgCancel();
//	void Create(HINSTANCE, HWND);	// 初期化

	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal(HINSTANCE, HWND, int);		// モードレスダイアログの表示
	HWND DoModeless(HINSTANCE, HWND, int);	// モードレスダイアログの表示

//	HWND Open(LPCTSTR);
//	void Close(void);	// モードレスダイアログの削除
	bool IsCanceled(void) { return bCANCEL; } // IDCANCELボタンが押されたか？
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);	// ダイアログのメッセージ処理 BOOL->INT_PTR 2008/7/18 Uchi
	void DeleteAsync(void);	// 自動破棄を遅延実行する	// 2008.05.28 ryoji

//	HINSTANCE	hInstance;	// アプリケーションインスタンスのハンドル
//	HWND		hwndParent;	// オーナーウィンドウのハンドル
//	HWND		hWnd;			// このダイアログのハンドル
	bool		bCANCEL;		// IDCANCELボタンが押された
	bool		bAutoCleanup;	// 自動後処理型	// 2008.05.28 ryoji

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnBnClicked(int);
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
};

