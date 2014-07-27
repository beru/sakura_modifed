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

#ifndef _PLUGIN_CDIALOG_H_00980ECC_75A9_485C_B0D1_5F61959C2370
#define _PLUGIN_CDIALOG_H_00980ECC_75A9_485C_B0D1_5F61959C2370

#include "CPluginCommon.h"

///////////////////////////////////////////////////////////////////////////////
class CPluginDialog
{
public:
	CPluginDialog();
	virtual ~CPluginDialog();

	INT_PTR DoModal(HINSTANCE hInstance, HWND hwndDlg, int nDlgTemplete, LPARAM lParam);
	HWND DoModeless(HINSTANCE hInstance, HWND hwndDlg, int nDlgTemplete, LPARAM lParam);
	void CloseDialog(int nRetVal);
	HWND GetHwnd(){ return m_hWnd; }
	int GetRetCode() const { return m_nRetCode; }

	virtual INT_PTR DispatchEvent(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnDestroy(void);
	virtual BOOL OnNcDestroy(void){ return FALSE; }
	virtual BOOL OnClose(void){ return FALSE; }
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnBnClicked(int wID);
	virtual BOOL OnLbnDblclk(int wID){ return FALSE; }
	virtual BOOL OnLbnSelChange(HWND hwndCtl, int wID){ return FALSE; }
	virtual BOOL OnCbnSelChange(HWND hwndCtl, int wID){ return FALSE; }
	virtual BOOL OnEnChange(HWND hwndCtl, int wID){ return FALSE; }
	virtual BOOL OnEnKillFocus(HWND hwndCtl, int wID){ return FALSE; }
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam){ return FALSE; }
	virtual BOOL OnTimer(WPARAM wParam){ return FALSE; }
	virtual int GetData(void);
	virtual void SetData(void);

public:
	WideString GetWindowText(HWND hwndCtl);

public:
	HINSTANCE	m_hInstance;
	HWND		m_hwndParent;
	HWND		m_hWnd;
	LPARAM		m_lParam;
	BOOL		m_bModal;
	int			m_nRetCode;
};

#endif // _PLUGIN_CDIALOG_H_00980ECC_75A9_485C_B0D1_5F61959C2370
