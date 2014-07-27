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
#include <stdio.h>
#include <windows.h>
#include <CommCtrl.h>
#include "resource.h"
#include "CJumpListDialog.h"
#include "CPluginService.h"
#include "CommonTools.h"
#include <algorithm>

LVCOLUMN_LAYOUT CJumpListDialog::layout[] = {
	{ LVCFMT_LEFT,   170, IDS_STR_HEADER11 },	//L"キーワード"
	{ LVCFMT_RIGHT,   60, IDS_STR_HEADER12 },	//L"行番号"
	{ LVCFMT_LEFT,   500, IDS_STR_HEADER13 }	//L"ファイル名"
};

///////////////////////////////////////////////////////////////////////////////
CJumpListDialog::CJumpListDialog(CGlobalOption* lpGlobalOption, std::list<CGlobalInfo*>* lpGlobalInfoList)
{
	m_nRetCode         = IDCANCEL;
	m_lpGlobalOption   = lpGlobalOption;
	m_lpGlobalInfoList = lpGlobalInfoList;
	m_nTimerID         = 0;
	m_dwMatchMode      = MATCH_MODE_PERFECT;
	m_bIgnoreCase      = FALSE;
	m_bSymbol          = FALSE;
	m_bOperation       = FALSE;
}

///////////////////////////////////////////////////////////////////////////////
CJumpListDialog::~CJumpListDialog()
{
	StopTimer();
	RemoveAllGlobalDataList(m_GlobalDataList);
}


///////////////////////////////////////////////////////////////////////////////
/*! モーダルダイアログの表示
*/
INT_PTR CJumpListDialog::DoModal(HINSTANCE hInstance, HWND hwndParent)
{
	//とりあえずキャンセル状態にしておく。
	m_nRetCode = IDCANCEL;
	return CPluginDialog::DoModal(hInstance, hwndParent, IDD_JUMP_DIALOG, (LPARAM)NULL);
}

///////////////////////////////////////////////////////////////////////////////
/*! 初期化処理
*/
BOOL CJumpListDialog::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	m_bOperation = TRUE;
	BOOL bResult = CPluginDialog::OnInitDialog(hwndDlg, wParam, lParam);
	HWND hList = ::GetDlgItem(GetHwnd(), IDC_LIST);
	HWND hEditKeyword = ::GetDlgItem(GetHwnd(), IDC_EDIT_KEYWORD);

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

	::SendMessage(hEditKeyword, EM_LIMITTEXT, (WPARAM)256, 0);
	::CheckDlgButton(GetHwnd(), IDC_CHECKBOX_CASE, BST_CHECKED);
	::CheckDlgButton(GetHwnd(), IDC_RADIO_ALL, BST_CHECKED);

	long lngStyle = ListView_GetExtendedListViewStyle(hList);
	lngStyle |= LVS_EX_FULLROWSELECT;// | LVS_SHOWSELALWAYS;
	ListView_SetExtendedListViewStyle(hList, lngStyle);

	SetData();
	SetDataSub();

	m_bOperation = FALSE;
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CJumpListDialog::OnBnClicked(int wID)
{
	switch(wID){
	case IDOK:
		m_bOperation = TRUE;
		m_nRetCode = wID;
		if(GetData()){
			StopTimer();
			{
				HWND hList = ::GetDlgItem(GetHwnd(), IDC_LIST);
				if(ListView_GetSelectedCount(hList) == 1){
					int nIndex = ListView_GetNextItem(hList, -1, LVIS_SELECTED | LVIS_FOCUSED);
					if(nIndex >= 0){
						CGlobalData info;
						GetItem(hList, nIndex, &info);
						thePluginService.Editor.S_TagJumpEx(info.m_strFile, info.m_nLine, 0, 0);
						RemoveAllGlobalDataList(m_GlobalDataList);
						CloseDialog(IDOK);
					}
				}
			}
		}
		m_bOperation = FALSE;
		break;

	case IDCANCEL:
		m_bOperation = TRUE;
		StopTimer();
		RemoveAllGlobalDataList(m_GlobalDataList);
		m_nRetCode = wID;
		CloseDialog(IDCANCEL);
		m_bOperation = FALSE;
		break;

	case IDC_RADIO_ALL:
	case IDC_RADIO_BEGIN:
	case IDC_RADIO_ANY:
	case IDC_CHECKBOX_CASE:
		if(m_bOperation == FALSE){
			StartTimer();
		}
		break;

	case IDC_BUTTON_FIND:
		StopTimer();
		OnTimer((WPARAM)0);
		break;

	default:
		break;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
void CJumpListDialog::SetData()
{
	if(GetHwnd() == NULL) return;

	HWND hEditKeyword = ::GetDlgItem(GetHwnd(), IDC_EDIT_KEYWORD);
	::SetWindowText(hEditKeyword, m_strKeyword.c_str());
	::CheckDlgButton(GetHwnd(), IDC_RADIO_ALL,   (m_dwMatchMode == MATCH_MODE_PERFECT) ? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(GetHwnd(), IDC_RADIO_BEGIN, (m_dwMatchMode == MATCH_MODE_BEGIN  ) ? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(GetHwnd(), IDC_RADIO_ANY,   (m_dwMatchMode == MATCH_MODE_ANY    ) ? BST_CHECKED : BST_UNCHECKED);
	::CheckDlgButton(GetHwnd(), IDC_CHECKBOX_CASE, m_bIgnoreCase ? BST_UNCHECKED : BST_CHECKED);
	::CheckDlgButton(GetHwnd(), IDC_CHECKBOX_SYMBOL, m_bSymbol ? BST_CHECKED : BST_UNCHECKED);
}

///////////////////////////////////////////////////////////////////////////////
void CJumpListDialog::SetDataSub()
{
	HWND hList = ::GetDlgItem(GetHwnd(), IDC_LIST);
	ListView_DeleteAllItems(hList);
	int nIndex = 0;
	for(std::list<CGlobalData*>::iterator it = m_GlobalDataList.begin(); it != m_GlobalDataList.end(); ++it){
		InsertItem(hList, nIndex, *it);
		nIndex++;
	}
}

///////////////////////////////////////////////////////////////////////////////
int CJumpListDialog::GetData()
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
DWORD CJumpListDialog::ReadGlobalFile(LPCWSTR lpszKeyword, const DWORD dwMatchMode, const BOOL bIgnoreCase, BOOL bSymbol)
{
	m_strKeyword = lpszKeyword;
	m_dwMatchMode = dwMatchMode;
	m_bIgnoreCase = bIgnoreCase;

	RemoveAllGlobalDataList(m_GlobalDataList);

	WideString strTmpFile = thePluginService.Plugin.GetPluginDir() + L"\\" + PROFILE_DEF_GTAGS_TMP_FILE;
	
	DWORD dwCount = 0;
	if(m_strKeyword.length() != 0){
		for(std::list<CGlobalInfo*>::iterator it = m_lpGlobalInfoList->begin(); it != m_lpGlobalInfoList->end(); ++it){
			CGlobalInfo* info = *it;
			if(info->m_bFlag){
				::DeleteFile(strTmpFile.c_str());
				WideString strMessage;
				WideString strResultPath = thePluginService.GetResultPath(info->m_dwUniqID);
				HANDLE hProcess = OnExecuteGlobal(info, strTmpFile);
				if(hProcess != NULL){
					CPluginDlgCancel dlg;
					INT_PTR nRet = dlg.DoModal(thePluginService.GetInstance(), GetHwnd(), IDD_EXECUTE_DIALOG, (LPARAM)hProcess);
					switch(nRet){
					case IDOK:
						break;
					case IDCANCEL:
						thePluginService.LoadString(IDS_STR_MSG11, strMessage);	//タグファイルの検索を中止しました。
						::MessageBox(GetHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
						return dwCount;
					case IDABORT:
					default:
					thePluginService.LoadString(IDS_STR_MSG10, strMessage);	//タグファイルの検索に失敗しました。
						::MessageBox(GetHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
						return dwCount;
					}
					dwCount += ReadGlobalFileOne(strTmpFile.c_str(), dwCount);
				}
				::DeleteFile(strTmpFile.c_str());
				if(dwCount >= m_lpGlobalOption->m_dwMaxFind) break;
			}
		}
	}

#ifndef __MINGW32__
	std::stable_sort(m_GlobalDataList.begin(), m_GlobalDataList.end(), Ascending);
#endif

	return dwCount;
}

///////////////////////////////////////////////////////////////////////////////
bool CJumpListDialog::Ascending(const CGlobalData* x, const CGlobalData* y)
{
	int result = wcscmp(x->m_strKeyword.c_str(), y->m_strKeyword.c_str());
	if(result < 0) return true;
	if(result == 0){
		result = x->m_nLine - y->m_nLine;
		if(result < 0) return true;
		if(result == 0){
			result = wcscmp(x->m_strFile.c_str(), y->m_strFile.c_str());
			if(result < 0) return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
DWORD CJumpListDialog::ReadGlobalFileOne(LPCWSTR lpszFileName, const DWORD dwPrevCount)
{
	DWORD dwCount = 0;
	FILE* fp = _wfopen(lpszFileName, L"r");
	if(fp == NULL) return 0;

	wchar_t* lpszBuffer   = new wchar_t[MAX_PATH_LENGTH];
	wchar_t* lpszKey      = new wchar_t[MAX_PATH_LENGTH];
	wchar_t* lpszFile     = new wchar_t[MAX_PATH_LENGTH];
	int nLine;
	while(fgetws(lpszBuffer, MAX_PATH_LENGTH, fp)){
		if(lpszBuffer[0] == L'!') continue;
		wcscpy(lpszKey, _T(""));
		wcscpy(lpszFile, _T(""));
		nLine = 0;
		if(swscanf(lpszBuffer, TAG_FORMAT, lpszKey, lpszFile, &nLine) < 3) continue;

		CGlobalData* info = new CGlobalData(lpszKey, lpszFile, nLine);
		m_GlobalDataList.push_back(info);
		dwCount++;
		if((dwCount + dwPrevCount) >= m_lpGlobalOption->m_dwMaxFind){
			break;
		}
	}
	delete[] lpszBuffer;
	delete[] lpszKey;
	delete[] lpszFile;

	fclose(fp);

	return dwCount;
}

///////////////////////////////////////////////////////////////////////////////
int CJumpListDialog::InsertItem(HWND hList, int nIndex, CGlobalData* info)
{
	LV_ITEM lvi;
	lvi.mask     = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem    = nIndex;
	lvi.iSubItem = 0;
	lvi.pszText  = (LPWSTR)info->m_strKeyword.c_str();
	lvi.lParam   = (LPARAM)info;
	int nResult = ListView_InsertItem(hList, &lvi);
	WideString strLine = thePluginService.GetDwordToString(info->m_nLine);
	ListView_SetItemText(hList, nResult, 1, (LPWSTR)strLine.c_str());
	ListView_SetItemText(hList, nResult, 2, (LPWSTR)info->m_strFile.c_str());
	return nResult;
}

///////////////////////////////////////////////////////////////////////////////
void CJumpListDialog::GetItem(HWND hList, int nIndex, CGlobalData* info)
{
	wchar_t* lpszBuffer = new wchar_t[MAX_PATH_LENGTH];
	if(lpszBuffer != NULL){
		ListView_GetItemText(hList, nIndex, 0, lpszBuffer, MAX_PATH_LENGTH);
		info->m_strKeyword = lpszBuffer;
		ListView_GetItemText(hList, nIndex, 1, lpszBuffer, MAX_PATH_LENGTH);
		info->m_nLine = _wtoi(lpszBuffer);
		ListView_GetItemText(hList, nIndex, 2, lpszBuffer, MAX_PATH_LENGTH);
		info->m_strFile = lpszBuffer;
		delete[] lpszBuffer;
	}
}

///////////////////////////////////////////////////////////////////////////////
void CJumpListDialog::StartTimer()
{
	StopTimer();
	if(m_lpGlobalOption->m_dwDelay > 0){
		m_nTimerID = ::SetTimer(GetHwnd(), 1, m_lpGlobalOption->m_dwDelay, NULL);
	}
}

///////////////////////////////////////////////////////////////////////////////
void CJumpListDialog::StopTimer()
{
	if(m_nTimerID != 0){
		::KillTimer(GetHwnd(), m_nTimerID);
		m_nTimerID = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
BOOL CJumpListDialog::OnTimer(WPARAM wParam)
{
	StopTimer();

	m_bOperation = TRUE;

	HWND hEditKeyword = ::GetDlgItem(GetHwnd(), IDC_EDIT_KEYWORD);
	m_strKeyword = GetWindowText(hEditKeyword);
	if(::IsDlgButtonChecked(GetHwnd(), IDC_RADIO_ALL) == BST_CHECKED){
		m_dwMatchMode = MATCH_MODE_PERFECT;
	}else if(::IsDlgButtonChecked(GetHwnd(), IDC_RADIO_BEGIN) == BST_CHECKED){
		m_dwMatchMode = MATCH_MODE_BEGIN;
	}else if(::IsDlgButtonChecked(GetHwnd(), IDC_RADIO_ANY) == BST_CHECKED){
		m_dwMatchMode = MATCH_MODE_ANY;
	}else{
		m_dwMatchMode = MATCH_MODE_PERFECT;
	}
	m_bIgnoreCase = (::IsDlgButtonChecked(GetHwnd(), IDC_CHECKBOX_CASE) == BST_CHECKED) ? FALSE : TRUE;
	m_bSymbol = (::IsDlgButtonChecked(GetHwnd(), IDC_CHECKBOX_SYMBOL) == BST_CHECKED) ? TRUE : FALSE;

	ReadGlobalFile(m_strKeyword.c_str(), m_dwMatchMode, m_bIgnoreCase, m_bSymbol);

	SetDataSub();

	m_bOperation = FALSE;

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
void CJumpListDialog::RemoveAllGlobalDataList(std::list<CGlobalData*>& p)
{
	for(std::list<CGlobalData*>::iterator it = p.begin(); it != p.end(); ++it){
		delete *it;
	}
	p.clear();
}

///////////////////////////////////////////////////////////////////////////////
BOOL CJumpListDialog::OnEnChange(HWND hwndCtl, int wID)
{
	if(m_bOperation == FALSE){
		StartTimer();
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
/*
	GTAGSDBPATH=DBのあるパス
	GTAGSROOT=対象ソースパス

	global -x		
	       -a		フルパス
	       -t		ctagsフォーマット
	       -i		大文字小文字を無視
	       -s		シンボルを検索
	       "x"		完全一致
	       "^x"		前方一致
	       ".*x.*"	部分一致
*/
HANDLE CJumpListDialog::OnExecuteGlobal(CGlobalInfo* info, WideString& strTmpFile)
{
	WideString strOption = L"-xat";
	if(m_bIgnoreCase) strOption += L"i";
	if(m_bSymbol) strOption += L"s";
	if(m_dwMatchMode == MATCH_MODE_PERFECT){
		strOption += L" \"" + m_strKeyword + L"\"";
	}else if(m_dwMatchMode == MATCH_MODE_BEGIN){
		strOption += L" \"^" + m_strKeyword + L"\"";
	}else if(m_dwMatchMode == MATCH_MODE_ANY){
		strOption += L" \".*" + m_strKeyword + L"\".*";
	}else{
		return NULL;
	}

	WideString strResultPath = thePluginService.GetResultPath(info->m_dwUniqID);
	WideString strCmdPath = GetSystemDirectory();
	wchar_t szEnvironment[MAX_PATH_LENGTH];
	swprintf(szEnvironment, L"GTAGSDBPATH=%s|GTAGSROOT=%s|", strResultPath.c_str(), info->m_strTargetPath.c_str());
	//::MessageBox(GetHwnd(), szEnvironment, L"DEBUG", MB_OK);
	for(int i = 0; szEnvironment[i] != 0; i++){
		if(szEnvironment[i] == L'|') szEnvironment[i] = L'\0';
	}

	wchar_t* lpszCmdLine = new wchar_t[MAX_PATH_LENGTH];
	wsprintf(lpszCmdLine, L"\"%s\\cmd.exe\" /D /C \"\"%s\" %s > \"%s\"\"", strCmdPath.c_str(), m_lpGlobalOption->m_strGlobalExePath.c_str(), strOption.c_str(), strTmpFile.c_str());
	//::MessageBox(GetHwnd(), lpszCmdLine, L"DEBUG", MB_OK);

	//gtags.exeを実行する
	PROCESS_INFORMATION pi;
	::ZeroMemory(&pi, sizeof(pi));
	STARTUPINFO si;
	::ZeroMemory(&si, sizeof(si));
	si.cb          = sizeof(si);
	si.dwFlags     = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	BOOL bProcessResult = ::CreateProcess(NULL, lpszCmdLine, NULL, NULL, FALSE, CREATE_UNICODE_ENVIRONMENT, szEnvironment, info->m_strTargetPath.c_str(), &si, &pi);
	if(bProcessResult == FALSE){
		return NULL;
	}
	::CloseHandle(pi.hThread);
	return pi.hProcess;
}
