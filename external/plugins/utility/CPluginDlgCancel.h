/*
	Copyright (C) 2013-2014, Plugins developers

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

#ifndef _PLUGIN_CDLG_CANCEL_H_00A7FC44_66D9_4DEF_B4A4_1E789B38E558
#define _PLUGIN_CDLG_CANCEL_H_00A7FC44_66D9_4DEF_B4A4_1E789B38E558

#include "CPluginDialog.h"

///////////////////////////////////////////////////////////////////////////////
class CPluginDlgCancel : public CPluginDialog
{
public:
	CPluginDlgCancel();
	virtual ~CPluginDlgCancel();

	typedef void (CALLBACK *CancelCallback)(LPARAM lParam);

	INT_PTR DoModal(HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete, LPARAM lParam);
	HWND DoModeless(HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete, LPARAM lParam);
	BOOL IsCanceled();

	BOOL			m_bCancel;
	HANDLE			m_hProcess;

public:
	virtual BOOL OnBnClicked(int wID);
	virtual BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnTimer(WPARAM wParam);
	virtual BOOL OnDestroy(void);

	void CancelCallbackProc();
};

#endif	//_PLUGIN_CDLG_CANCEL_H_00A7FC44_66D9_4DEF_B4A4_1E789B38E558
