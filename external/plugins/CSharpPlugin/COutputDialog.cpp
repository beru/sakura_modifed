/*
	Copyright (C) 2014, Plugins developers

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
#include <CommCtrl.h>
#include "resource.h"
#include "COutputDialog.h"
#include "CPluginService.h"

///////////////////////////////////////////////////////////////////////////////
COutputDialog::COutputDialog()
{
	m_nRetCode = IDCANCEL;
}

///////////////////////////////////////////////////////////////////////////////
COutputDialog::~COutputDialog()
{
}

///////////////////////////////////////////////////////////////////////////////
/*! モーダルダイアログの表示
*/
INT_PTR COutputDialog::DoModal(HINSTANCE hInstance, HWND hwndParent, int nType, LPCWSTR lpszText)
{
	//とりあえずキャンセル状態にしておく。
	m_nRetCode = IDCANCEL;
	m_strText = lpszText;
	return CPluginDialog::DoModal(hInstance, hwndParent, IDD_DIALOG_OUTPUT, (LPARAM)NULL);
}

///////////////////////////////////////////////////////////////////////////////
/*! 初期化処理
*/
BOOL COutputDialog::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	BOOL bResult = CPluginDialog::OnInitDialog(hwndDlg, wParam, lParam);
	SetData();
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////
BOOL COutputDialog::OnBnClicked(int wID)
{
	switch(wID){
	case IDCANCEL:
		//設定を書き込む
		m_nRetCode = wID;
		CloseDialog(IDCANCEL);
		break;
	default:
		break;
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
void COutputDialog::SetData(void)
{
	if(GetHwnd() == NULL) return;
	HWND hEdit = ::GetDlgItem(GetHwnd(), IDC_EDIT_OUTPUT);
	SetWindowText(hEdit, m_strText.c_str());
}

///////////////////////////////////////////////////////////////////////////////
int COutputDialog::GetData(void)
{
	switch(m_nRetCode){
	case IDCANCEL:
		break;
	default:
		break;
	}
	return TRUE;
}
