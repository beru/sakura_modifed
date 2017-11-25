#pragma once

BOOL MyWinHelp(HWND hwndCaller, UINT uCommand, DWORD_PTR dwData);

// Shell Interface系(?)
BOOL SelectDir(HWND, const TCHAR*, const TCHAR*, TCHAR*);	// フォルダ選択ダイアログ
BOOL ResolveShortcutLink(HWND hwnd, LPCTSTR lpszLinkFile, LPTSTR lpszPath);	// ショートカット(.lnk)の解決

HWND OpenHtmlHelp(HWND hWnd, LPCTSTR szFile, UINT uCmd, DWORD_PTR data, bool msgflag = true);
DWORD NetConnect (const TCHAR strNetWorkPass[]);

// ヘルプの目次を表示
void ShowWinHelpContents(HWND hwnd);

BOOL GetSpecialFolderPath(int nFolder, LPTSTR pszPath);	// 特殊フォルダのパスを取得する

INT_PTR MyPropertySheet(LPPROPSHEETHEADER lppsph);	// 独自拡張プロパティシート

// フォント選択ダイアログ
BOOL MySelectFont(LOGFONT* plf, INT* piPointSize, HWND hwndDlgOwner, bool);

