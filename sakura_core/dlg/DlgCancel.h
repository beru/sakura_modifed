/*!	@file
	@brief キャンセルボタンダイアログ

	@author Norio Nakatani
	@date 1998/09/09 作成
    @date 1999/12/02 再作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2008, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class DlgCancel;

#pragma once

#include "dlg/Dialog.h"


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

