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

#ifndef _PLUGIN_COPTIONDIALOG_H__
#define _PLUGIN_COPTIONDIALOG_H__

#include "CPluginDialog.h"
#include "CPluginService.h"
#include <string>
#include <list>

///////////////////////////////////////////////////////////////////////////////
class COptionDialog : public CPluginDialog
{
public:
	COptionDialog(CtagsOption* lpCtagsOption, std::list<CtagsInfo*>* lpCtagsInfoList);
	virtual ~COptionDialog();

	INT_PTR DoModal(HINSTANCE hInstance, HWND hwndParent);
	virtual BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnBnClicked(int wID);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam);
	virtual int GetData(void);
	virtual void SetData(void);

private:
	CtagsOption*			m_lpCtagsOption;
	std::list<CtagsInfo*>*	m_lpCtagsInfoList;
	std::list<DWORD>		m_DeleteList;
	static LVCOLUMN_LAYOUT	layout[];

	int InsertItem(HWND hList, int nIndex, CtagsInfo* info);
	void GetItem(HWND hList, int nIndex, CtagsInfo* info);
	void ControlButton();
	void Replace(wchar_t* szFilter, wchar_t c1, wchar_t c2);
};

#endif	// _PLUGIN_COPTIONDIALOG_H__
