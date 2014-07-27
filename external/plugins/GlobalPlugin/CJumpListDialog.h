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

#ifndef _PLUGIN_CJUMPLISTDIALOG_H__
#define _PLUGIN_CJUMPLISTDIALOG_H__

#include "CPluginDialog.h"
#include <string>
#include <list>
#include "CPluginService.h"
#include "CPluginDlgCancel.h"

#define DEFAULT_DELAY_TIMER 1000

class CGlobalData
{
public:
	WideString	m_strKeyword;
	WideString	m_strFile;
	int			m_nLine;

	CGlobalData(){
		m_nLine = 0;
	}

	CGlobalData(LPCWSTR lpszKeyword, LPCWSTR lpszFile, const int nLine){
		m_strKeyword  = lpszKeyword;
		m_strFile     = lpszFile;
		m_nLine       = nLine;
	}
};

///////////////////////////////////////////////////////////////////////////////
class CJumpListDialog : public CPluginDialog
{
public:
	CJumpListDialog(CGlobalOption* lpGlobalOption, std::list<CGlobalInfo*>* lpGlobalInfoList);
	virtual ~CJumpListDialog();

	INT_PTR DoModal(HINSTANCE hInstance, HWND hwndParent);
	virtual BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnBnClicked(int wID);
	virtual BOOL OnEnChange(HWND hwndCtl, int wID);
	virtual int GetData(void);
	virtual void SetData(void);

public:
	CGlobalOption*				m_lpGlobalOption;
	std::list<CGlobalInfo*>*	m_lpGlobalInfoList;
	std::list<CGlobalData*>		m_GlobalDataList;
	static LVCOLUMN_LAYOUT		layout[];
	int							m_nTimerID;
	WideString					m_strKeyword;
	DWORD						m_dwMatchMode;
	BOOL						m_bIgnoreCase;
	BOOL						m_bOperation;
	BOOL						m_bSymbol;

	DWORD ReadGlobalFile(LPCWSTR lpszKeyword, const DWORD dwMatchMode, const BOOL bIgnoreCase, BOOL bSymbol);
	DWORD ReadGlobalFileOne(LPCWSTR lpszFileName, const DWORD dwPrevCount);
	HANDLE OnExecuteGlobal(CGlobalInfo* info, WideString& strTmpFile);

	int InsertItem(HWND hList, int nIndex, CGlobalData* info);
	void GetItem(HWND hList, int nIndex, CGlobalData* info);
	void StartTimer();
	void StopTimer();
	BOOL OnTimer(WPARAM wParam);
	void RemoveAllGlobalDataList(std::list<CGlobalData*>& p);
	void SetDataSub();
	static bool Ascending(const CGlobalData* x, const CGlobalData* y);
};

#endif	// _PLUGIN_CJUMPLISTDIALOG_H__
