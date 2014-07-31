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
#include "ControlResizer.h"

#define DEFAULT_DELAY_TIMER 1000

class CGlobalData
{
public:
	WideString	m_strFile;
	int			m_lineNum;
	WideString	m_strLine;

	CGlobalData() {
		m_lineNum	= 0;
	}

	CGlobalData(LPCWSTR lpszFile, int lineNum, LPCWSTR lpszLine) {
		m_strFile	= lpszFile;
		m_lineNum	= lineNum;
		m_strLine	= lpszLine;
	}
};

///////////////////////////////////////////////////////////////////////////////
class CJumpListDialog : public CPluginDialog
{
public:
	CJumpListDialog(CGlobalOption* lpGlobalOption, std::list<CGlobalInfo*>* lpGlobalInfoList);
	virtual ~CJumpListDialog();

	INT_PTR DoModal(HINSTANCE hInstance, HWND hwndParent);
	
	virtual INT_PTR DispatchEvent(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
	virtual void CloseDialog(int nRetVal);
	virtual BOOL OnClose(void);
	virtual BOOL OnBnClicked(int wID);
	virtual BOOL OnEnChange(HWND hwndCtl, int wID);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam);
	virtual int GetData(void);
	virtual void SetData(void);
	void SetDataSub();
	DWORD ReadGlobalFile(LPCWSTR lpszKeyword, DWORD dwMatchMode, BOOL bIgnoreCase, BOOL bSymbol, BOOL bRef);

	BY_HANDLE_FILE_INFORMATION	m_fileInfo;
	int							m_lineNo;

private:
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
	BOOL						m_bRef;

	ControlResizer				m_ctrlResizer;
	POINT						m_initialSize;

	DWORD ReadGlobalFileOne(LPSTR buff, DWORD dwPrevCount);
	bool OnExecuteGlobal(CGlobalInfo* info, char* buff, size_t nBytes);

	const CGlobalData* GetItem(HWND hList, int nIndex);
	void StartTimer();
	void StopTimer();
	BOOL OnTimer(WPARAM wParam);
	bool InsertItem(HWND hList, int nIndex, CGlobalData* info);
	void RemoveAllGlobalDataList(std::list<CGlobalData*>& p);
};

#endif	// _PLUGIN_CJUMPLISTDIALOG_H__
