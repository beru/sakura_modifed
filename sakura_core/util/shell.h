#pragma once

BOOL MyWinHelp(HWND hwndCaller, UINT uCommand, DWORD_PTR dwData);

// Shell Interface�n(?)
BOOL SelectDir(HWND, const TCHAR*, const TCHAR*, TCHAR*);	// �t�H���_�I���_�C�A���O
BOOL ResolveShortcutLink(HWND hwnd, LPCTSTR lpszLinkFile, LPTSTR lpszPath);	// �V���[�g�J�b�g(.lnk)�̉���

HWND OpenHtmlHelp(HWND hWnd, LPCTSTR szFile, UINT uCmd, DWORD_PTR data, bool msgflag = true);
DWORD NetConnect (const TCHAR strNetWorkPass[]);

// �w���v�̖ڎ���\��
void ShowWinHelpContents(HWND hwnd);

BOOL GetSpecialFolderPath(int nFolder, LPTSTR pszPath);	// ����t�H���_�̃p�X���擾����

INT_PTR MyPropertySheet(LPPROPSHEETHEADER lppsph);	// �Ǝ��g���v���p�e�B�V�[�g

// �t�H���g�I���_�C�A���O
BOOL MySelectFont(LOGFONT* plf, INT* piPointSize, HWND hwndDlgOwner, bool);

