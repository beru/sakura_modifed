#pragma once

BOOL MyWinHelp(HWND hwndCaller, UINT uCommand, DWORD_PTR dwData);	// WinHelp �̂����� HtmlHelp ���Ăяo��	// 2006.07.22 ryoji

// Shell Interface�n(?)
BOOL SelectDir(HWND, const TCHAR*, const TCHAR*, TCHAR*);	// �t�H���_�I���_�C�A���O
BOOL ResolveShortcutLink(HWND hwnd, LPCTSTR lpszLinkFile, LPTSTR lpszPath);	// �V���[�g�J�b�g(.lnk)�̉���

HWND OpenHtmlHelp(HWND hWnd, LPCTSTR szFile, UINT uCmd, DWORD_PTR data, bool msgflag = true);
DWORD NetConnect (const TCHAR strNetWorkPass[]);

// �w���v�̖ڎ���\��
void ShowWinHelpContents(HWND hwnd);

BOOL GetSpecialFolderPath(int nFolder, LPTSTR pszPath);	// ����t�H���_�̃p�X���擾����	// 2007.05.19 ryoji

INT_PTR MyPropertySheet(LPPROPSHEETHEADER lppsph);	// �Ǝ��g���v���p�e�B�V�[�g	// 2007.05.24 ryoji

// �t�H���g�I���_�C�A���O
BOOL MySelectFont(LOGFONT* plf, INT* piPointSize, HWND hwndDlgOwner, bool);	// 2009.10.01 ryoji �|�C���g�T�C�Y�i1/10�|�C���g�P�ʁj�����ǉ�

