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
#include <vector>
#include "CPluginService.h"

#define DEFAULT_DELAY_TIMER 1000

class CtagsData
{
public:
	WideString	m_strKeyword;
	WideString	m_strFile;
	int			m_nLine;
	WideString	m_strType;
	WideString	m_strTypeName;

	CtagsData(){
		m_nLine = 0;
	}

	CtagsData(LPCWSTR lpszKeyword, LPCWSTR lpszFile, const int nLine, LPCWSTR lpszType, LPCWSTR lpszTypeName){
		m_strKeyword  = lpszKeyword;
		m_strFile     = lpszFile;
		m_nLine       = nLine;
		m_strType     = lpszType;
		m_strTypeName = lpszTypeName;
	}
};

///////////////////////////////////////////////////////////////////////////////
class CJumpListDialog : public CPluginDialog
{
public:
	CJumpListDialog(CtagsOption* lpCtagsOption, std::list<CtagsInfo*>* lpCtagsInfoList);
	virtual ~CJumpListDialog();

	INT_PTR DoModal(HINSTANCE hInstance, HWND hwndParent);
	virtual BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnBnClicked(int wID);
	virtual BOOL OnEnChange(HWND hwndCtl, int wID);
	virtual int GetData(void);
	virtual void SetData(void);

public:
	CtagsOption*			m_lpCtagsOption;
	std::list<CtagsInfo*>*	m_lpCtagsInfoList;
	std::list<CtagsData*>	m_CtagsDataList;
	static LVCOLUMN_LAYOUT	layout[];
	int						m_nTimerID;
	WideString				m_strKeyword;
	DWORD					m_dwMatchMode;
	BOOL					m_bIgnoreCase;
	BOOL					m_bOperation;

	DWORD ReadCtagsFile(LPCWSTR lpszKeyword, const DWORD dwMatchMode, const BOOL bIgnoreCase);
	DWORD ReadCtagsFileOne(LPCWSTR lpszKeyword, const DWORD dwMatchMode, const BOOL bIgnoreCase, LPCWSTR lpszFileName, const DWORD dwPrevCount);
	DWORD ReadSqliteFileOne(LPCWSTR lpszKeyword, const DWORD dwMatchMode, const BOOL bIgnoreCase, LPCWSTR lpszFileName, const DWORD dwPrevCount);

	int InsertItem(HWND hList, int nIndex, CtagsData* info);
	void GetItem(HWND hList, int nIndex, CtagsData* info);
	void StartTimer();
	void StopTimer();
	BOOL OnTimer(WPARAM wParam);
	void RemoveAllCtagsDataList(std::list<CtagsData*>& p);
	void SetDataSub();
	static bool Ascending(const CtagsData* x, const CtagsData* y);
};

#endif	// _PLUGIN_CJUMPLISTDIALOG_H__
