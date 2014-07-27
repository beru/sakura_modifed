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
#include "CSelectDialog.h"
#include "CPluginService.h"

///////////////////////////////////////////////////////////////////////////////
CSelectDialog::CSelectDialog()
{
	m_nRetCode = IDCANCEL;
}

///////////////////////////////////////////////////////////////////////////////
CSelectDialog::~CSelectDialog()
{
}

///////////////////////////////////////////////////////////////////////////////
/*! モーダルダイアログの表示
*/
INT_PTR CSelectDialog::DoModal(HINSTANCE hInstance, HWND hwndParent)
{
	//とりあえずキャンセル状態にしておく。
	m_nRetCode = IDCANCEL;
	return CPluginDialog::DoModal(hInstance, hwndParent, IDD_SELECT_DIALOG, (LPARAM)NULL);
}

///////////////////////////////////////////////////////////////////////////////
/*! 初期化処理
*/
BOOL CSelectDialog::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	BOOL bResult = CPluginDialog::OnInitDialog(hwndDlg, wParam, lParam);
	WideString strHeader;
	thePluginService.LoadString(IDS_STR_HEADER, strHeader);
	HWND hList = ::GetDlgItem(GetHwnd(), IDC_LIST);
	LV_COLUMN lvc;
	lvc.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt      = LVCFMT_LEFT;
	lvc.cx       = 580;
	lvc.pszText  = (LPWSTR)strHeader.c_str();
	lvc.iSubItem = 0;
	ListView_InsertColumn(hList, 0, &lvc);
	long lngStyle = ListView_GetExtendedListViewStyle(hList);
	lngStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle(hList, lngStyle);
	SetData();
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CSelectDialog::OnBnClicked(int wID)
{
	switch(wID){
	case IDOK:
		m_nRetCode = wID;
		if(GetData()){
			//設定を書き込む
			CloseDialog(IDOK);
		}
		break;
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
BOOL CSelectDialog::OnNotify(WPARAM wParam, LPARAM lParam)
{
	NMHDR* pNMHDR = (NMHDR*)lParam;
	if(pNMHDR != NULL){
		HWND hList = ::GetDlgItem(GetHwnd(), IDC_LIST);
		if(hList == pNMHDR->hwndFrom){
			switch(pNMHDR->code){
			case NM_DBLCLK:
				return OnBnClicked(IDOK);
			default:
				break;
			}
		}
	}
	return CPluginDialog::OnNotify(wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////
void CSelectDialog::SetData(void)
{
	if(GetHwnd() == NULL) return;
	HWND hEdit = ::GetDlgItem(GetHwnd(), IDC_EDIT);
	::SetWindowText(hEdit, thePluginService.m_strFileName1.c_str());
	HWND hList = ::GetDlgItem(GetHwnd(), IDC_LIST);
	int index = 0;
	for(std::list<std::wstring>::iterator it = thePluginService.m_lstFilePath.begin(); it != thePluginService.m_lstFilePath.end(); ++it){
		if(wcsicmp(thePluginService.m_strFileName1.c_str(), it->c_str()) == 0) continue;
		LV_ITEM lvi;
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.iItem    = index;
		lvi.iSubItem = 0;
		lvi.pszText  = (LPWSTR)it->c_str();
		lvi.lParam   = (LPARAM)NULL;
		ListView_InsertItem(hList, &lvi);
		index++;
	}
}

///////////////////////////////////////////////////////////////////////////////
int CSelectDialog::GetData(void)
{
	switch(m_nRetCode){
	case IDOK:
		{
			HWND hList = ::GetDlgItem(GetHwnd(), IDC_LIST);
			if(ListView_GetSelectedCount(hList) == 1){
				int nIndex = ListView_GetNextItem(hList, -1, LVIS_SELECTED | LVIS_FOCUSED);
				if(nIndex >= 0){
					wchar_t szBuffer[32768];
					ListView_GetItemText(hList, nIndex, 0, szBuffer, _countof(szBuffer));
					thePluginService.m_strFileName2 = szBuffer;
					return TRUE;
				}
			}
			return FALSE;
		}
		break;
	case IDCANCEL:
		break;
	default:
		break;
	}
	return TRUE;
}
