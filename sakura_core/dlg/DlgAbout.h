#pragma once

#include "dlg/Dialog.h"

// バージョン情報ダイアログ

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
	INT_PTR DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
protected:
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnBnClicked(int);
	BOOL OnStnClicked(int);
	LPVOID GetHelpIdTable(void);
private:
	UrlWnd UrlUrWnd;
	UrlWnd UrlOrgWnd;
};

