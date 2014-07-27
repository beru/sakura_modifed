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

#include "StdAfx.h"
#include <windows.h>
#include <tchar.h>
#include "CPluginDialog.h"
#include "CommonTools.h"

///////////////////////////////////////////////////////////////////////////////
// ダイアログプロシージャ
INT_PTR CALLBACK MyPluginDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR nResult = (INT_PTR)FALSE;
	CPluginDialog* lpCPluginDialog = NULL;

	switch(uMsg){
	case WM_INITDIALOG:
		lpCPluginDialog = reinterpret_cast<CPluginDialog*>(lParam);
		break;
	default:
		lpCPluginDialog = reinterpret_cast<CPluginDialog*>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
		break;
	}

	if(lpCPluginDialog != NULL){
		nResult = lpCPluginDialog->DispatchEvent(hwndDlg, uMsg, wParam, lParam);
	}

	return nResult;
}

///////////////////////////////////////////////////////////////////////////////
// コンストラクタ
CPluginDialog::CPluginDialog()
{
	m_hInstance  = NULL;
	m_hwndParent = NULL;
	m_hWnd       = NULL;
	m_lParam     = (LPARAM)NULL;
	m_nRetCode   = IDCANCEL;
	m_bModal     = TRUE;

	return;
}

///////////////////////////////////////////////////////////////////////////////
// デストラクタ
CPluginDialog::~CPluginDialog()
{
	CloseDialog(0);
	return;
}

///////////////////////////////////////////////////////////////////////////////
// モーダルダイアログの表示
INT_PTR CPluginDialog::DoModal(HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete, LPARAM lParam)
{
	m_bModal     = TRUE;
	m_hInstance  = hInstance;
	m_hwndParent = hwndParent;
	m_lParam     = lParam;
	m_nRetCode   = IDCANCEL;
	return ::DialogBoxParam(m_hInstance, MAKEINTRESOURCE(nDlgTemplete), m_hwndParent, MyPluginDialogProc, (LPARAM)this);
}

///////////////////////////////////////////////////////////////////////////////
// モードレスダイアログの表示
HWND CPluginDialog::DoModeless(HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete, LPARAM lParam)
{
	m_bModal     = FALSE;
	m_hInstance  = hInstance;
	m_hwndParent = hwndParent;
	m_lParam     = lParam;
	m_nRetCode   = IDCANCEL;
	m_hWnd = ::CreateDialogParam(m_hInstance, MAKEINTRESOURCE(nDlgTemplete), m_hwndParent, MyPluginDialogProc, (LPARAM)this);
	if(m_hWnd != NULL){
		::ShowWindow(m_hWnd, SW_SHOW);
	}
	return m_hWnd;
}

///////////////////////////////////////////////////////////////////////////////
void CPluginDialog::CloseDialog(int nRetVal)
{
	if(m_hWnd != NULL){
		if(m_bModal){
			::EndDialog(m_hWnd, nRetVal);
		}else{
			::DestroyWindow(m_hWnd);
		}
		m_hWnd = NULL;
	}
	return;
}

///////////////////////////////////////////////////////////////////////////////
INT_PTR CPluginDialog::DispatchEvent(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_INITDIALOG:	return OnInitDialog(hwndDlg, wParam, lParam);
	case WM_DESTROY:	return OnDestroy();
	case WM_NCDESTROY:	return OnNcDestroy();
	case WM_CLOSE:		return OnClose();
	case WM_COMMAND:	return OnCommand(wParam, lParam);
	case WM_NOTIFY:		return OnNotify(wParam, lParam);
	case WM_TIMER:		return OnTimer(wParam);
	//TODO: 必要に応じてここに追加する
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CPluginDialog::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	m_hWnd = hwndDlg;
	::SetWindowLongPtr(m_hWnd, DWLP_USER, lParam);
	//SetData();
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CPluginDialog::OnDestroy(void)
{
	m_hWnd = NULL;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CPluginDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);	//通知コード
	WORD wID         = LOWORD(wParam);	//項目ID, コントロールID, アクセラレータID
	HWND hwndCtl     = (HWND)lParam;

	if((wID == IDOK) || (wID == IDCANCEL)){
		return OnBnClicked(wID);
	}

	if(hwndCtl != NULL){
		TCHAR szClass[64];
		::GetClassName(hwndCtl, szClass, 64);

		if(_tcsicmp(szClass, _T("Button")) == 0){
			switch(wNotifyCode){
			case BN_CLICKED:	return OnBnClicked(wID);
			}
		}else if(_tcsicmp(szClass, _T("ListBox")) == 0){
			switch(wNotifyCode){
			case LBN_SELCHANGE:	return OnLbnSelChange(hwndCtl, wID);
			case LBN_DBLCLK:	return OnLbnDblclk(wID);
			}
		}else if(_tcsicmp(szClass, _T("ComboBox")) == 0){
			switch(wNotifyCode){
			case CBN_SELCHANGE:	return OnCbnSelChange(hwndCtl, wID);
			}
		}else if(_tcsicmp(szClass, _T("Edit")) == 0){
			switch(wNotifyCode){
			case EN_CHANGE:		return OnEnChange(hwndCtl, wID);
			case EN_KILLFOCUS:	return OnEnKillFocus(hwndCtl, wID);
			}
		}
		//TODO: 必要に応じてここに追加する
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CPluginDialog::OnBnClicked(int wID)
{
	switch(wID){
	case IDABORT:
	case IDCANCEL:
	case IDOK:
	default:
		m_nRetCode = wID;
		CloseDialog(wID);
		return TRUE;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
int CPluginDialog::GetData()
{
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
void CPluginDialog::SetData()
{
	return;
}

///////////////////////////////////////////////////////////////////////////////
WideString CPluginDialog::GetWindowText(HWND hwndCtl)
{
	WideString strResult = L"";
	wchar_t* lpszBuffer = new wchar_t[MAX_PATH_LENGTH];
	::ZeroMemory(lpszBuffer, MAX_PATH_LENGTH);
	::GetWindowText(hwndCtl, lpszBuffer, MAX_PATH_LENGTH);
	strResult = lpszBuffer;
	return strResult;
}
