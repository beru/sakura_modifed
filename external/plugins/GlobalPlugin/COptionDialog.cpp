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
#include <CommCtrl.h>
#include "resource.h"
#include "COptionDialog.h"
#include "CMakeDialog.h"
#include "CPluginService.h"
#include "CommonTools.h"

LVCOLUMN_LAYOUT COptionDialog::layout[] = {
	{ LVCFMT_LEFT,   500, IDS_STR_HEADER21 },	//L"タグ作成対象フォルダ"
	{ LVCFMT_LEFT,    70, IDS_STR_HEADER22 }	//L"タグ識別子"
};

///////////////////////////////////////////////////////////////////////////////
COptionDialog::COptionDialog(CGlobalOption* lpGlobalOption, std::list<CGlobalInfo*>* lpGlobalInfoList)
{
	m_nRetCode         = IDCANCEL;
	m_lpGlobalOption   = lpGlobalOption;
	m_lpGlobalInfoList = lpGlobalInfoList;
}

///////////////////////////////////////////////////////////////////////////////
COptionDialog::~COptionDialog()
{
}

///////////////////////////////////////////////////////////////////////////////
/*! モーダルダイアログの表示
*/
INT_PTR COptionDialog::DoModal(HINSTANCE hInstance, HWND hwndParent)
{
	//とりあえずキャンセル状態にしておく。
	m_nRetCode = IDCANCEL;
	return CPluginDialog::DoModal(hInstance, hwndParent, IDD_OPTION_DIALOG, (LPARAM)NULL);
}

///////////////////////////////////////////////////////////////////////////////
/*! 初期化処理
*/
BOOL COptionDialog::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	BOOL bResult = CPluginDialog::OnInitDialog(hwndDlg, wParam, lParam);

	HWND hList          = ::GetDlgItem(GetHwnd(), IDC_LIST);
	HWND hEditGtagsExe  = ::GetDlgItem(GetHwnd(), IDC_EDIT_GTAGS);
	HWND hEditGlobalExe = ::GetDlgItem(GetHwnd(), IDC_EDIT_GLOBAL);
	HWND hEditMaxFind   = ::GetDlgItem(GetHwnd(), IDC_EDIT_MAXFIND);
	HWND hEditDelay     = ::GetDlgItem(GetHwnd(), IDC_EDIT_DELAY);
	::SendMessage(hEditGtagsExe,  EM_LIMITTEXT, (WPARAM)1024, 0);
	::SendMessage(hEditGlobalExe, EM_LIMITTEXT, (WPARAM)1024, 0);
	::SendMessage(hEditMaxFind,   EM_LIMITTEXT, (WPARAM)6, 0);
	::SendMessage(hEditDelay,     EM_LIMITTEXT, (WPARAM)6, 0);

	for(int i = 0; i < _countof(layout); i++){
		WideString strText;
		thePluginService.LoadString(layout[i].m_nID, strText);
		LV_COLUMN lvc;
		lvc.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.fmt      = layout[i].m_dwFmt;
		lvc.cx       = layout[i].m_dwWidth;
		lvc.pszText  = (LPWSTR)strText.c_str();
		lvc.iSubItem = i;
		ListView_InsertColumn(hList, i, &lvc);
	}

	long lngStyle = ListView_GetExtendedListViewStyle(hList);
	lngStyle |= LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES;// | LVS_SHOWSELALWAYS;
	ListView_SetExtendedListViewStyle(hList, lngStyle);

	SetData();

	ControlButton();

	//念のため作成する
	WideString strResultPath = thePluginService.Plugin.GetPluginDir() + L"\\" + PROFILE_DEF_GTAGS_RESULT_PATH;
	::CreateDirectory(strResultPath.c_str(), NULL);

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////
BOOL COptionDialog::OnBnClicked(int wID)
{
	switch(wID){
	case IDOK:
		m_nRetCode = wID;
		if(GetData()){
			for(std::list<DWORD>::iterator it = m_DeleteList.begin(); it != m_DeleteList.end(); ++it){
				thePluginService.RemoveResultPath(*it);
			}
			m_DeleteList.clear();
			//設定を書き込む
			thePluginService.WriteProfile();
			CloseDialog(IDOK);
		}
		break;

	case IDCANCEL:
		m_DeleteList.clear();
		m_nRetCode = wID;
		CloseDialog(IDCANCEL);
		break;

	case IDC_BUTTON_ADD:
		{
			HWND hList = ::GetDlgItem(GetHwnd(), IDC_LIST);
			CGlobalInfo info;
			info.m_dwUniqID = thePluginService.GetUniqID();
			WideString strResultPath = thePluginService.GetResultPath(info.m_dwUniqID);
			CMakeDialog dlg(m_lpGlobalOption, &info);
			INT_PTR nRet = dlg.DoModal(thePluginService.GetInstance(), GetHwnd());
			if(nRet == IDOK){
				int nIndex = InsertItem(hList, ListView_GetItemCount(hList), &info);
				ListView_SetItemState(hList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				ListView_EnsureVisible(hList, nIndex, FALSE);
				ControlButton();
			}
		}
		break;

	case IDC_BUTTON_UPDATE:
		{
			HWND hList = ::GetDlgItem(GetHwnd(), IDC_LIST);
			if(ListView_GetSelectedCount(hList) == 1){
				int nIndex = ListView_GetNextItem(hList, -1, LVIS_SELECTED | LVIS_FOCUSED);
				CGlobalInfo info;
				GetItem(hList, nIndex, &info);
				CMakeDialog dlg(m_lpGlobalOption, &info);
				INT_PTR nRet = dlg.DoModal(thePluginService.GetInstance(), GetHwnd());
				if(nRet == IDOK){
					ListView_SetItemText(hList, nIndex, 0, (LPWSTR)info.m_strTargetPath.c_str());
					ListView_SetCheckState(hList, nIndex, info.m_bFlag);
					WideString strUniqID = thePluginService.GetDwordToHexString(info.m_dwUniqID);
					ListView_SetItemText(hList, nIndex, 3, (LPWSTR)strUniqID.c_str());
					ControlButton();
				}
			}
		}
		break;

	case IDC_BUTTON_DELETE:
		{
			HWND hList = ::GetDlgItem(GetHwnd(), IDC_LIST);
			if(ListView_GetSelectedCount(hList) == 1){
				WideString strMessage;
				thePluginService.LoadString(IDS_STR_MSG1, strMessage);	//選択された項目を削除してもよろしいですか？
				int nIndex = ListView_GetNextItem(hList, -1, LVIS_SELECTED | LVIS_FOCUSED);
				INT_PTR nRet = ::MessageBox(GetHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1);
				if(nRet == IDYES){
					CGlobalInfo prev;
					GetItem(hList, nIndex, &prev);
					m_DeleteList.push_back(prev.m_dwUniqID);
					ListView_DeleteItem(hList, nIndex);
					ControlButton();
				}
			}
		}
		break;

	case IDC_BUTTON_UP:
		{
			HWND hList = ::GetDlgItem(GetHwnd(), IDC_LIST);
			if(ListView_GetSelectedCount(hList) == 1){
				int nIndex = ListView_GetNextItem(hList, -1, LVIS_SELECTED);
				//int nCount = ListView_GetItemCount(hList);
				if(nIndex > 0){
					CGlobalInfo prev;
					GetItem(hList, nIndex, &prev);
					ListView_DeleteItem(hList, nIndex);
					InsertItem(hList, nIndex - 1, &prev);
					ListView_SetItemState(hList, nIndex - 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					ListView_EnsureVisible(hList, nIndex - 1, FALSE);
					ControlButton();
				}
			}
		}
		break;

	case IDC_BUTTON_DOWN:
		{
			HWND hList = ::GetDlgItem(GetHwnd(), IDC_LIST);
			if(ListView_GetSelectedCount(hList) == 1){
				int nIndex = ListView_GetNextItem(hList, -1, LVIS_SELECTED);
				int nCount = ListView_GetItemCount(hList);
				if(nIndex < (nCount - 1)){
					CGlobalInfo prev;
					GetItem(hList, nIndex, &prev);
					InsertItem(hList, nIndex + 2, &prev);
					ListView_SetItemState(hList, nIndex + 2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					ListView_EnsureVisible(hList, nIndex + 2, FALSE);
					ListView_DeleteItem(hList, nIndex);
					ControlButton();
				}
			}
		}
		break;

	case IDC_BUTTON_BROWSE1:	//gtags.exeパス
		{
			HWND hEditGtagsExe = ::GetDlgItem(GetHwnd(), IDC_EDIT_GTAGS);
			WideString strGtagsExeFile = GetWindowText(hEditGtagsExe);
			WideString strGtagsExePath = GetParentFolder(strGtagsExeFile);
			WideString strFilter;
			thePluginService.LoadString(IDS_STR_FILTER1, strFilter);	//実行ファイル\0*.exe\0All files\0*.*\0
			wchar_t szFilter[256];
			wcscpy(szFilter, strFilter.c_str());
			Replace(szFilter, L'|', L'\0');
			WideString strCaption;
			thePluginService.LoadString(IDS_STR_CAPTION1, strCaption);	//gtags.exeパスの指定
			if(GetOpenFileNameDialog(thePluginService.GetInstance(), GetHwnd(), strCaption.c_str(), strGtagsExePath.c_str(), szFilter, strGtagsExeFile)){
				::SetWindowText(hEditGtagsExe, strGtagsExeFile.c_str());
			}
		}
		break;

	case IDC_BUTTON_BROWSE2:	//global.exeパス
		{
			HWND hEditGlobalExe = ::GetDlgItem(GetHwnd(), IDC_EDIT_GLOBAL);
			WideString strGlobalExeFile = GetWindowText(hEditGlobalExe);
			WideString strGlobalExePath = GetParentFolder(strGlobalExeFile);
			WideString strFilter;
			thePluginService.LoadString(IDS_STR_FILTER1, strFilter);	//実行ファイル\0*.exe\0All files\0*.*\0
			wchar_t szFilter[256];
			wcscpy(szFilter, strFilter.c_str());
			Replace(szFilter, L'|', L'\0');
			WideString strCaption;
			thePluginService.LoadString(IDS_STR_CAPTION2, strCaption);	//global.exeパスの指定
			if(GetOpenFileNameDialog(thePluginService.GetInstance(), GetHwnd(), strCaption.c_str(), strGlobalExePath.c_str(), szFilter, strGlobalExeFile)){
				::SetWindowText(hEditGlobalExe, strGlobalExeFile.c_str());
			}
		}
		break;

	default:
		break;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
BOOL COptionDialog::OnNotify(WPARAM wParam, LPARAM lParam)
{
	switch(wParam){
	case IDC_LIST:
		{
			NMHDR* pNMHDR = (NMHDR*)lParam;
			switch(pNMHDR->code){
			case LVN_ITEMCHANGED:
				ControlButton();
				break;
			case NM_DBLCLK:
				OnBnClicked(IDC_BUTTON_UPDATE);
				break;
			}
			return TRUE;
		}
		break;
	}
	return CPluginDialog::OnNotify(wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////
void COptionDialog::SetData(void)
{
	if(GetHwnd() == NULL) return;
	HWND hEditGtagsExe  = ::GetDlgItem(GetHwnd(), IDC_EDIT_GTAGS);
	HWND hEditGlobalExe = ::GetDlgItem(GetHwnd(), IDC_EDIT_GLOBAL);
	HWND hEditMaxFind   = ::GetDlgItem(GetHwnd(), IDC_EDIT_MAXFIND);
	HWND hEditDelay     = ::GetDlgItem(GetHwnd(), IDC_EDIT_DELAY);

	::SetWindowText(hEditGtagsExe,  m_lpGlobalOption->m_strGtagsExePath.c_str());
	::SetWindowText(hEditGlobalExe, m_lpGlobalOption->m_strGlobalExePath.c_str());
	WideString strMaxFind = thePluginService.GetDwordToString(m_lpGlobalOption->m_dwMaxFind);
	::SetWindowText(hEditMaxFind,   strMaxFind.c_str());
	WideString strDelay = thePluginService.GetDwordToString(m_lpGlobalOption->m_dwDelay);
	::SetWindowText(hEditDelay,     strDelay.c_str());

	::CheckDlgButton(GetHwnd(), IDC_RADIO_ALL,   (m_lpGlobalOption->m_dwMatchMode == MATCH_MODE_PERFECT) ? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(GetHwnd(), IDC_RADIO_BEGIN, (m_lpGlobalOption->m_dwMatchMode == MATCH_MODE_BEGIN  ) ? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(GetHwnd(), IDC_RADIO_ANY,   (m_lpGlobalOption->m_dwMatchMode == MATCH_MODE_ANY    ) ? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(GetHwnd(), IDC_CHECKBOX_CASE, m_lpGlobalOption->m_bIgnoreCase ? BST_UNCHECKED : BST_CHECKED);

	HWND hList = ::GetDlgItem(GetHwnd(), IDC_LIST);
	ListView_DeleteAllItems(hList);
	int nIndex = 0;
	for(std::list<CGlobalInfo*>::iterator it = thePluginService.m_GlobalInfoList.begin(); it != thePluginService.m_GlobalInfoList.end(); ++it){
		InsertItem(hList, nIndex, *it);
		nIndex++;
	}
}

///////////////////////////////////////////////////////////////////////////////
int COptionDialog::GetData(void)
{
	switch(m_nRetCode){
	case IDOK:
		{
			HWND hEditGtagsExe  = ::GetDlgItem(GetHwnd(), IDC_EDIT_GTAGS);
			HWND hEditGlobalExe = ::GetDlgItem(GetHwnd(), IDC_EDIT_GLOBAL);
			HWND hEditMaxFind   = ::GetDlgItem(GetHwnd(), IDC_EDIT_MAXFIND);
			HWND hEditDelay     = ::GetDlgItem(GetHwnd(), IDC_EDIT_DELAY);
			m_lpGlobalOption->m_strGtagsExePath  = GetWindowText(hEditGtagsExe);
			m_lpGlobalOption->m_strGlobalExePath = GetWindowText(hEditGlobalExe);
			WideString strMaxFind = GetWindowText(hEditMaxFind);
			m_lpGlobalOption->m_dwMaxFind        = _wtoi(strMaxFind.c_str());
			if((m_lpGlobalOption->m_dwMaxFind < 1) || (m_lpGlobalOption->m_dwMaxFind > 100000)){
				WideString strMessage;
				thePluginService.LoadString(IDS_STR_MSG2, strMessage);	//1 から 100,000 の範囲で指定してください。
				::MessageBox(GetHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
				return FALSE;
			}
			WideString strDelay = GetWindowText(hEditDelay);
			m_lpGlobalOption->m_dwDelay = _wtoi(strDelay.c_str());
			if(/*(m_lpGlobalOption->m_dwDelay < 0) ||*/ (m_lpGlobalOption->m_dwDelay > 60000)){
				WideString strMessage;
				thePluginService.LoadString(IDS_STR_MSG2, strMessage);	//0 から 60,000 ミリ秒の範囲で指定してください。
				::MessageBox(GetHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
				return FALSE;
			}
			if(::IsDlgButtonChecked(GetHwnd(), IDC_RADIO_ALL) == BST_CHECKED){
				m_lpGlobalOption->m_dwMatchMode = MATCH_MODE_PERFECT;
			}else if(::IsDlgButtonChecked(GetHwnd(), IDC_RADIO_BEGIN) == BST_CHECKED){
				m_lpGlobalOption->m_dwMatchMode = MATCH_MODE_BEGIN;
			}else if(::IsDlgButtonChecked(GetHwnd(), IDC_RADIO_ANY) == BST_CHECKED){
				m_lpGlobalOption->m_dwMatchMode = MATCH_MODE_ANY;
			}else{
				m_lpGlobalOption->m_dwMatchMode = MATCH_MODE_PERFECT;
			}
			m_lpGlobalOption->m_bIgnoreCase = (::IsDlgButtonChecked(GetHwnd(), IDC_CHECKBOX_CASE) == BST_CHECKED) ? FALSE : TRUE;
			thePluginService.RemoveAllGlobalInfoList(*m_lpGlobalInfoList);
			HWND hList = ::GetDlgItem(GetHwnd(), IDC_LIST);
			DWORD dwCount = ListView_GetItemCount(hList);
			for(DWORD nIndex = 0; nIndex < dwCount; nIndex++){
				CGlobalInfo* info = new CGlobalInfo;
				GetItem(hList, nIndex, info);
				m_lpGlobalInfoList->push_back(info);
			}
		}
		break;

	default:
		break;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
int COptionDialog::InsertItem(HWND hList, int nIndex, CGlobalInfo* info)
{
	WideString strUniqID = thePluginService.GetDwordToHexString(info->m_dwUniqID);
	LV_ITEM lvi;
	lvi.mask     = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem    = nIndex;
	lvi.iSubItem = 0;
	lvi.pszText  = (LPWSTR)info->m_strTargetPath.c_str();
	lvi.lParam   = (LPARAM)info;
	int nResult = ListView_InsertItem(hList, &lvi);
	ListView_SetCheckState(hList, nResult, info->m_bFlag);
	ListView_SetItemText(hList, nResult, 1, (LPWSTR)strUniqID.c_str());
	return nResult;
}

///////////////////////////////////////////////////////////////////////////////
void COptionDialog::GetItem(HWND hList, int nIndex, CGlobalInfo* info)
{
	wchar_t* lpszBuffer = new wchar_t[MAX_PATH_LENGTH];
	if(lpszBuffer != NULL){
		info->m_bFlag = ListView_GetCheckState(hList, nIndex);
		ListView_GetItemText(hList, nIndex, 0, lpszBuffer, MAX_PATH_LENGTH);
		info->m_strTargetPath = lpszBuffer;
		ListView_GetItemText(hList, nIndex, 1, lpszBuffer, MAX_PATH_LENGTH);
		info->m_dwUniqID = thePluginService.GetHexStringToDword(lpszBuffer);
		delete[] lpszBuffer;
	}
}

///////////////////////////////////////////////////////////////////////////////
void COptionDialog::ControlButton()
{
	HWND hButtonUpdate = ::GetDlgItem(GetHwnd(), IDC_BUTTON_UPDATE);
	HWND hButtonDelete = ::GetDlgItem(GetHwnd(), IDC_BUTTON_DELETE);
	HWND hButtonUp     = ::GetDlgItem(GetHwnd(), IDC_BUTTON_UP);
	HWND hButtonDown   = ::GetDlgItem(GetHwnd(), IDC_BUTTON_DOWN);
	HWND hList = ::GetDlgItem(GetHwnd(), IDC_LIST);

	int nCount = ListView_GetItemCount(hList);
	int nIndex = -1;
	if(nCount >= 1) nIndex = ListView_GetNextItem(hList, -1, LVIS_SELECTED | LVIS_FOCUSED);
	BOOL bSelected = (nIndex >= 0) ? TRUE : FALSE;
	::EnableWindow(hButtonUpdate, bSelected);
	::EnableWindow(hButtonDelete, bSelected);

	BOOL bUp = FALSE;
	BOOL bDown = FALSE;
	if(nCount > 1){
		int nIndex = ListView_GetNextItem(hList, -1, LVIS_SELECTED | LVIS_FOCUSED);
		if(nIndex > 0){
			bUp = TRUE;
		}
		if(nIndex < (nCount - 1)){
			bDown = TRUE;
		}
	}
	::EnableWindow(hButtonUp, bUp);
	::EnableWindow(hButtonDown, bDown);
}

///////////////////////////////////////////////////////////////////////////////
void COptionDialog::Replace(wchar_t* szFilter, wchar_t c1, wchar_t c2)
{
	for(int i = 0; szFilter[i] != L'\0'; i++){
		if(szFilter[i] == c1){
			szFilter[i] = c2;
		}
	}
}
