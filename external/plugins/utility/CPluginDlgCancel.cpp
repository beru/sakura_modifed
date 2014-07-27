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

#include "stdafx.h"
#include "CPluginDlgCancel.h"

///////////////////////////////////////////////////////////////////////////////
CPluginDlgCancel::CPluginDlgCancel()
{
	m_bCancel = FALSE;
	m_hProcess = NULL;
}

///////////////////////////////////////////////////////////////////////////////
CPluginDlgCancel::~CPluginDlgCancel()
{
}

///////////////////////////////////////////////////////////////////////////////
INT_PTR CPluginDlgCancel::DoModal(HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete, LPARAM lParam)
{
	m_hProcess = (HANDLE)lParam;
	return CPluginDialog::DoModal(hInstance, hwndParent, nDlgTemplete, (LPARAM)NULL);
}

///////////////////////////////////////////////////////////////////////////////
HWND CPluginDlgCancel::DoModeless(HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete, LPARAM lParam)
{
	m_hProcess = (HANDLE)lParam;
	return CPluginDialog::DoModeless(hInstance, hwndParent, nDlgTemplete, (LPARAM)NULL);
}

///////////////////////////////////////////////////////////////////////////////
BOOL CPluginDlgCancel::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	BOOL bRet = CPluginDialog::OnInitDialog(hwndDlg, wParam, lParam);
	if(m_hProcess != NULL){
		::SetTimer(m_hWnd, (UINT_PTR)m_hProcess, 100, NULL);
	}
	return bRet;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CPluginDlgCancel::OnDestroy(void)
{
	if(m_hProcess != NULL){
		::KillTimer(m_hWnd, (UINT_PTR)m_hProcess);
		::CloseHandle(m_hProcess);
	}
	return CPluginDialog::OnDestroy();
}

///////////////////////////////////////////////////////////////////////////////
BOOL CPluginDlgCancel::OnBnClicked(int wID)
{
	switch(wID){
	case IDABORT:
		break;
	case IDCANCEL:
		m_bCancel = TRUE;
		CancelCallbackProc();
		break;
	}
	return CPluginDialog::OnBnClicked(wID);
}
	
///////////////////////////////////////////////////////////////////////////////
BOOL CPluginDlgCancel::IsCanceled()
{
	return m_bCancel;
}

///////////////////////////////////////////////////////////////////////////////
void CPluginDlgCancel::CancelCallbackProc()
{
	if(m_hProcess != NULL){
		::KillTimer(m_hWnd, (UINT_PTR)m_hProcess);
		::TerminateProcess(m_hProcess, 1);
		::CloseHandle(m_hProcess);
		m_hProcess = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
BOOL CPluginDlgCancel::OnTimer(WPARAM wParam)
{
	DWORD dwRet = ::WaitForSingleObject(m_hProcess, 0);
	switch(dwRet){
	case WAIT_TIMEOUT:
		::KillTimer(m_hWnd, (UINT_PTR)m_hProcess);
		::SetTimer(m_hWnd, (UINT_PTR)m_hProcess, 100, NULL);
		break;
	case WAIT_OBJECT_0:
		{
			::KillTimer(m_hWnd, (UINT_PTR)m_hProcess);
			DWORD dwExitCode = 0;
			BOOL bRet = ::GetExitCodeProcess(m_hProcess, &dwExitCode);
			::CloseHandle(m_hProcess);
			m_hProcess = NULL;
			int wID = IDCANCEL;
			if(bRet){
				if(dwExitCode == 0){
					wID = IDOK;
				} else{
					wID = IDABORT;
				}
			}
			OnBnClicked(wID);
		}
		break;
	case WAIT_ABANDONED:
	default:
		OnBnClicked(IDCANCEL);
		break;
	}
	return TRUE;
}
