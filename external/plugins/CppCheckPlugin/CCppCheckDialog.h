/*
	Copyright (C) 2013, Plugins developers

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#ifndef _PLUGIN_CCPPCHECKDIALOG_H__
#define _PLUGIN_CCPPCHECKDIALOG_H__

#include "CPluginDialog.h"
#include "CPluginDlgCancel.h"
#include "CPluginService.h"

///////////////////////////////////////////////////////////////////////////////
typedef struct tagLVCOLUMN_LAYOUT {
	DWORD	m_dwFmt;
	DWORD	m_dwWidth;
	UINT	m_nID;
	//	LPCWSTR	m_lpszText;
} LVCOLUMN_LAYOUT;

///////////////////////////////////////////////////////////////////////////////
class CCppCheckData
{
public:
	WideString	m_strFile;
	WideString	m_strLine;
	WideString	m_strIdentifier;
	WideString	m_strSeverity;
	WideString	m_strMessage;
};

///////////////////////////////////////////////////////////////////////////////
class CCppCheckDialog : public CPluginDialog
{
public:
	CCppCheckDialog();
	virtual ~CCppCheckDialog();

	INT_PTR DoModal(HINSTANCE hInstance, HWND hwndParent);
	HWND DoModeless(HINSTANCE hInstance, HWND hwndParent);
	virtual BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnBnClicked(int wID);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam);
	virtual int GetData(void);
	virtual void SetData(void);

public:
	static LVCOLUMN_LAYOUT	layout[];
	static BOOL				m_bSort[];

	int InsertItem(CCppCheckData* info);
	int InsertItem(HWND hList, int nIndex, CCppCheckData* info);
	void GetItem(HWND hList, int nIndex, CCppCheckData* info);
	static bool Ascending(const CCppCheckData* x, const CCppCheckData* y);
	static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	HANDLE OnExecuteCppCheck();
	void ReadXML(LPCWSTR lpszFileName);
};

#endif	// _PLUGIN_CCPPCHECKDIALOG_H__
