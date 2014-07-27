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

#include "StdAfx.h"
#include <windows.h>
#include <CommCtrl.h>
#include "resource.h"
#include "CCppCheckDialog.h"
#include "CPluginDlgCancel.h"
#include "CPluginService.h"
#include "CommonTools.h"
#include <algorithm>
#include "CXmlCppCheck.h"

LVCOLUMN_LAYOUT CCppCheckDialog::layout[] = {
	{ LVCFMT_LEFT, 350, IDS_STR_HEADER11 },	//L"error file"
	{ LVCFMT_RIGHT, 60, IDS_STR_HEADER12 },	//L"line"
	{ LVCFMT_LEFT, 100, IDS_STR_HEADER13 },	//L"id"
	{ LVCFMT_LEFT,  70, IDS_STR_HEADER14 },	//L"severity"
	{ LVCFMT_LEFT, 500, IDS_STR_HEADER15 }	//L"msg"
};

BOOL CCppCheckDialog::m_bSort[] = {
	FALSE, FALSE, FALSE, FALSE
};

struct tagCompareInfo {
	HWND	m_hWnd;
	int		m_iSubItem;
	BOOL	m_bAsc;
};

///////////////////////////////////////////////////////////////////////////////
CCppCheckDialog::CCppCheckDialog()
{
	m_nRetCode = IDCANCEL;
}

///////////////////////////////////////////////////////////////////////////////
CCppCheckDialog::~CCppCheckDialog()
{
}

///////////////////////////////////////////////////////////////////////////////
/*! モーダルダイアログの表示
*/
INT_PTR CCppCheckDialog::DoModal(HINSTANCE hInstance, HWND hwndParent)
{
	//とりあえずキャンセル状態にしておく。
	m_nRetCode = IDCANCEL;
	return CPluginDialog::DoModal(hInstance, hwndParent, IDD_CPPCHECK_DIALOG, (LPARAM)NULL);
}

///////////////////////////////////////////////////////////////////////////////
/*! モードレスダイアログの表示
*/
HWND CCppCheckDialog::DoModeless(HINSTANCE hInstance, HWND hwndParent)
{
	//とりあえずキャンセル状態にしておく。
	m_nRetCode = IDCANCEL;
	return CPluginDialog::DoModeless(hInstance, hwndParent, IDD_CPPCHECK_DIALOG, (LPARAM)NULL);
}

///////////////////////////////////////////////////////////////////////////////
/*! 初期化処理
*/
BOOL CCppCheckDialog::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	BOOL bResult = CPluginDialog::OnInitDialog(hwndDlg, wParam, lParam);
	HWND hList = ::GetDlgItem(GetHwnd(), IDC_LIST);

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
	lngStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle(hList, lngStyle);

	SetData();

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CCppCheckDialog::OnBnClicked(int wID)
{
	switch(wID){
	case IDOK:
		m_nRetCode = wID;
		if(GetData()){
			{
				HWND hList = ::GetDlgItem(GetHwnd(), IDC_LIST);
				if(ListView_GetSelectedCount(hList) == 1){
					int nIndex = ListView_GetNextItem(hList, -1, LVIS_SELECTED | LVIS_FOCUSED);
					if(nIndex >= 0){
						CCppCheckData info;
						GetItem(hList, nIndex, &info);
						if (info.m_strFile.length() != 0){
							thePluginService.Editor.S_TagJumpEx(info.m_strFile, _wtoi(info.m_strLine.c_str()), 0, 0);
							CloseDialog(IDOK);
						}
					}
				}
			}
		}
		break;

	case IDCANCEL:
		m_nRetCode = wID;
		CloseDialog(IDCANCEL);
		break;

	case IDC_BUTTON_CPPCHECK:
		{
			if(thePluginService.m_strTargetName.length() == 0){
				return TRUE;	//無題の場合はなにもしない
			}
			HWND hList = ::GetDlgItem(GetHwnd(), IDC_LIST);
			ListView_DeleteAllItems(hList);
			WideString strMessage;
			
			HANDLE hProcess = OnExecuteCppCheck();
			if(hProcess == NULL){
				thePluginService.LoadString(IDS_STR_MSG2, strMessage);	//cppcheckに失敗しました。
				::MessageBox(GetHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
				return FALSE;
			}

			CPluginDlgCancel dlg;
			INT_PTR nRet = dlg.DoModal(thePluginService.GetInstance(), GetHwnd(), IDD_EXECUTE_DIALOG, (LPARAM)hProcess);
			switch(nRet){
			case IDOK:
				break;
			case IDCANCEL:
				thePluginService.LoadString(IDS_STR_MSG1, strMessage);	//cppcheckを中止しました。
				::MessageBox(GetHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
				return FALSE;
			case IDABORT:
			default:
				thePluginService.LoadString(IDS_STR_MSG2, strMessage);	//cppcheckに失敗しました。
				::MessageBox(GetHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
				return FALSE;
			}

			ReadXML(thePluginService.m_strResultFile.c_str());
			thePluginService.LoadString(IDS_STR_MSG3, strMessage);	//cppcheckを実行しました。
			::MessageBox(GetHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONINFORMATION | MB_OK);
			return TRUE;
		}
		break;

	default:
		break;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CCppCheckDialog::OnNotify(WPARAM wParam, LPARAM lParam)
{
	NMHDR* pNMHDR = (NMHDR*)lParam;
	switch(pNMHDR->code){
	case LVN_COLUMNCLICK:
		{
			HWND hList = ::GetDlgItem(GetHwnd(), IDC_LIST);
			NMLISTVIEW* pListView = (NMLISTVIEW*)lParam;
			struct tagCompareInfo param;
			param.m_hWnd = hList;
			param.m_iSubItem = pListView->iSubItem;
			m_bSort[pListView->iSubItem] = ! m_bSort[pListView->iSubItem];
			param.m_bAsc = m_bSort[pListView->iSubItem];
			ListView_SortItems(hList, CompareFunc, (LPARAM)&param);
		}
		break;
	case NM_DBLCLK:
		OnBnClicked(IDOK);
		break;
	default:
		break;
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
int CALLBACK CCppCheckDialog::CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	struct tagCompareInfo* param = (struct tagCompareInfo*)lParamSort;
	LV_FINDINFO lvf;
	lvf.flags = LVFI_PARAM;
	lvf.lParam = lParam1;
	int nItem1 = ListView_FindItem(param->m_hWnd, -1, &lvf);
	lvf.lParam = lParam2;
	int nItem2 = ListView_FindItem(param->m_hWnd, -1, &lvf);
	wchar_t buf1[_MAX_PATH];
	wchar_t buf2[_MAX_PATH];
	ListView_GetItemText(param->m_hWnd, nItem1, param->m_iSubItem, buf1, _countof(buf1));
	ListView_GetItemText(param->m_hWnd, nItem2, param->m_iSubItem, buf2, _countof(buf2));
	switch(param->m_iSubItem){
	case 1:
		if(param->m_bAsc){
			return _wtoi(buf1) - _wtoi(buf2);
		} else{
			return (_wtoi(buf1) - _wtoi(buf2)) * (-1);
		}
	default:
		if(param->m_bAsc){
			return wcscmp(buf1, buf2);
		} else{
			return wcscmp(buf1, buf2) * (-1);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void CCppCheckDialog::SetData()
{
	if(GetHwnd() == NULL) return;
	ReadXML(thePluginService.m_strResultFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////
int CCppCheckDialog::GetData()
{
	switch(m_nRetCode){
	case IDOK:
		break;
	default:
		break;
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
bool CCppCheckDialog::Ascending(const CCppCheckData* x, const CCppCheckData* y)
{
	int result = wcscmp(x->m_strFile.c_str(), y->m_strFile.c_str());
	if(result < 0) return true;
	if(result == 0){
		result = _wtoi(x->m_strLine.c_str()) - _wtoi(y->m_strLine.c_str());
		if(result < 0) return true;
		if (result == 0){
			result = wcscmp(x->m_strIdentifier.c_str(), y->m_strIdentifier.c_str());
			if (result < 0) return true;
			if (result == 0){
				result = wcscmp(x->m_strSeverity.c_str(), y->m_strSeverity.c_str());
				if (result < 0) return true;
			}
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
int CCppCheckDialog::InsertItem(CCppCheckData* info)
{
	HWND hList = ::GetDlgItem(GetHwnd(), IDC_LIST);
	int nIndex = ListView_GetItemCount(hList);
	return InsertItem(hList, nIndex, info);
}

///////////////////////////////////////////////////////////////////////////////
int CCppCheckDialog::InsertItem(HWND hList, int nIndex, CCppCheckData* info)
{
	LV_ITEM lvi;
	lvi.mask     = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem    = nIndex;
	lvi.iSubItem = 0;
	lvi.pszText  = (LPWSTR)info->m_strFile.c_str();
	lvi.lParam   = (LPARAM)nIndex;
	int nResult = ListView_InsertItem(hList, &lvi);
	ListView_SetItemText(hList, nResult, 1, (LPWSTR)info->m_strLine.c_str());
	ListView_SetItemText(hList, nResult, 2, (LPWSTR)info->m_strIdentifier.c_str());
	ListView_SetItemText(hList, nResult, 3, (LPWSTR)info->m_strSeverity.c_str());
	ListView_SetItemText(hList, nResult, 4, (LPWSTR)info->m_strMessage.c_str());
	return nResult;
}

///////////////////////////////////////////////////////////////////////////////
void CCppCheckDialog::GetItem(HWND hList, int nIndex, CCppCheckData* info)
{
	wchar_t szBuffer[MAX_PATH_LENGTH];
	ListView_GetItemText(hList, nIndex, 0, szBuffer, MAX_PATH_LENGTH);
	info->m_strFile = szBuffer;
	ListView_GetItemText(hList, nIndex, 1, szBuffer, MAX_PATH_LENGTH);
	info->m_strLine = szBuffer;
	ListView_GetItemText(hList, nIndex, 2, szBuffer, MAX_PATH_LENGTH);
	info->m_strIdentifier = szBuffer;
	ListView_GetItemText(hList, nIndex, 3, szBuffer, MAX_PATH_LENGTH);
	info->m_strSeverity = szBuffer;
	ListView_GetItemText(hList, nIndex, 4, szBuffer, MAX_PATH_LENGTH);
	info->m_strMessage = szBuffer;
}

///////////////////////////////////////////////////////////////////////////////
HANDLE CCppCheckDialog::OnExecuteCppCheck()
{
	WideString strOption = L" --platform=" + thePluginService.m_strPlatform;
	strOption += L" --enable=all --xml";

	WideString strCmdPath = GetSystemDirectory();

	wchar_t szCmdLine[MAX_PATH_LENGTH];
	wsprintf(szCmdLine, L"\"%s\\cmd.exe\" /D /C \"\"%s\" %s \"%s\" 2> \"%s\"\"", strCmdPath.c_str(), thePluginService.m_strCppCheckExePath.c_str(), strOption.c_str(), thePluginService.m_strTargetName.c_str(), thePluginService.m_strResultFile.c_str());
//	wsprintf(szCmdLine, L"\"%s\\cmd.exe\" /D /C \"\"%s\" %s\" \"%s\" 2> \"%s\"", strCmdPath.c_str(), thePluginService.m_strCppCheckExePath.c_str(), strOption.c_str(), thePluginService.m_strTargetName.c_str(), thePluginService.m_strResultFile.c_str());
//	wsprintf(szCmdLine, L"\"%s\" %s \"%s\" 2> \"%s\"", thePluginService.m_strCppCheckExePath.c_str(), strOption.c_str(), thePluginService.m_strTargetName.c_str(), thePluginService.m_strResultFile.c_str());
//	::MessageBox(GetHwnd(), szCmdLine, L"DEBUG", MB_OK);

	//ctags.exeを実行する
	PROCESS_INFORMATION pi;
	::ZeroMemory(&pi, sizeof(pi));
	STARTUPINFO si;
	::ZeroMemory(&si, sizeof(si));
	si.cb          = sizeof(si);
	si.dwFlags     = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	BOOL bProcessResult = ::CreateProcess(NULL, szCmdLine, NULL, NULL, FALSE, 0, NULL, thePluginService.Plugin.GetPluginDir().c_str(), &si, &pi);
	if(bProcessResult == FALSE){
		return NULL;
	}
	::CloseHandle(pi.hThread);
	return pi.hProcess;
}

///////////////////////////////////////////////////////////////////////////////
void CCppCheckDialog::ReadXML(LPCWSTR lpszFileName)
{
	CXmlCppCheck xml;
	xml.m_lpCppCheckDialog = this;
	xml.ReadXml(lpszFileName);
}
