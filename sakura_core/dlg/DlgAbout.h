/*!	@file
	@brief バージョン情報ダイアログ

	@author Norio Nakatani
	@date 1998/05/22 作成
	@date 1999/12/05 再作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include "dlg/Dialog.h"
/*!
	@brief About Box管理
	
	DispatchEventを独自に定義することで，Dialogでサポートされていない
	メッセージを捕捉する．
*/

class UrlWnd {
public:
	UrlWnd() { hWnd = NULL; hFont = NULL; bHilighted = FALSE; pOldProc = NULL; }
	virtual ~UrlWnd() { ; }
	BOOL SetSubclassWindow(HWND hWnd);
	HWND GetHwnd() const { return hWnd; }
protected:
	static LRESULT CALLBACK UrlWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
protected:
	HWND hWnd;
	HFONT hFont;
	BOOL bHilighted;
	WNDPROC pOldProc;
};

class DlgAbout : public Dialog {
public:
	INT_PTR DoModal(HINSTANCE, HWND);	// モーダルダイアログの表示
	// Nov. 7, 2000 genta	標準以外のメッセージを捕捉する
	INT_PTR DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
protected:
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnBnClicked(int);
	BOOL OnStnClicked(int);
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
private:
	UrlWnd UrlUrWnd;
	UrlWnd UrlOrgWnd;
};

