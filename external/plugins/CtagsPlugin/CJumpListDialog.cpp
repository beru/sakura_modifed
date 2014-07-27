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
#include "CSqlite.h"
#ifndef __MINGW32__
#include <algorithm>
#endif

LVCOLUMN_LAYOUT CJumpListDialog::layout[] = {
	{ LVCFMT_LEFT,   170, IDS_STR_HEADER11 },	//L"キーワード"
	{ LVCFMT_RIGHT,   60, IDS_STR_HEADER12 },	//L"行番号"
	{ LVCFMT_LEFT,    70, IDS_STR_HEADER13 },	//L"種類"
	{ LVCFMT_LEFT,   120, IDS_STR_HEADER14 },	//L"備考"
	{ LVCFMT_LEFT,   500, IDS_STR_HEADER15 }	//L"ファイル名"
};

///////////////////////////////////////////////////////////////////////////////
CJumpListDialog::CJumpListDialog(CtagsOption* lpCtagsOption, std::list<CtagsInfo*>* lpCtagsInfoList)
{
	m_nRetCode        = IDCANCEL;
	m_lpCtagsOption   = lpCtagsOption;
	m_lpCtagsInfoList = lpCtagsInfoList;
	m_nTimerID        = 0;
	m_dwMatchMode     = MATCH_MODE_PERFECT;
	m_bIgnoreCase     = FALSE;
	m_bOperation      = FALSE;
}

///////////////////////////////////////////////////////////////////////////////
CJumpListDialog::~CJumpListDialog()
{
	StopTimer();
	RemoveAllCtagsDataList(m_CtagsDataList);
}


///////////////////////////////////////////////////////////////////////////////
/*! モーダルダイアログの表示
*/
INT_PTR CJumpListDialog::DoModal(HINSTANCE hInstance, HWND hwndParent)
{
	//とりあえずキャンセル状態にしておく。
	m_nRetCode = IDCANCEL;
	return CPluginDialog::DoModal(hInstance, hwndParent, IDD_CTAGS_JUMP_DIALOG, (LPARAM)NULL);
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
						CtagsData info;
						GetItem(hList, nIndex, &info);
						thePluginService.Editor.S_TagJumpEx(info.m_strFile, info.m_nLine, 0, 0);
						RemoveAllCtagsDataList(m_CtagsDataList);
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
		RemoveAllCtagsDataList(m_CtagsDataList);
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
}

///////////////////////////////////////////////////////////////////////////////
void CJumpListDialog::SetDataSub()
{
	HWND hList = ::GetDlgItem(GetHwnd(), IDC_LIST);
	ListView_DeleteAllItems(hList);
	int nIndex = 0;
	for(std::list<CtagsData*>::iterator it = m_CtagsDataList.begin(); it != m_CtagsDataList.end(); ++it){
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
DWORD CJumpListDialog::ReadCtagsFile(LPCWSTR lpszKeyword, const DWORD dwMatchMode, const BOOL bIgnoreCase)
{
	m_strKeyword = lpszKeyword;
	m_dwMatchMode = dwMatchMode;
	m_bIgnoreCase = bIgnoreCase;

	RemoveAllCtagsDataList(m_CtagsDataList);
	
	DWORD dwCount = 0;
	if(m_strKeyword.length() != 0){
		for(std::list<CtagsInfo*>::iterator it = m_lpCtagsInfoList->begin(); it != m_lpCtagsInfoList->end(); ++it){
			CtagsInfo* info = *it;
			if(info->m_bFlag){
				if(m_lpCtagsOption->m_strSqliteDllPath.length() == 0){
					WideString strResultFile = thePluginService.GetResultFile(info->m_dwUniqID);
					dwCount += ReadCtagsFileOne(lpszKeyword, dwMatchMode, bIgnoreCase, strResultFile.c_str(), dwCount);
				}else{
					WideString strSqliteFile = thePluginService.GetResultDbName(info->m_dwUniqID);
					dwCount += ReadSqliteFileOne(lpszKeyword, dwMatchMode, bIgnoreCase, strSqliteFile.c_str(), dwCount);
				}
				if(dwCount >= m_lpCtagsOption->m_dwMaxFind) break;
			}
		}
	}

#ifndef __MINGW32__
	stable_sort(m_CtagsDataList.begin(), m_CtagsDataList.end(), Ascending);
#endif

	return dwCount;
}

///////////////////////////////////////////////////////////////////////////////
bool CJumpListDialog::Ascending(const CtagsData* x, const CtagsData* y)
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
DWORD CJumpListDialog::ReadCtagsFileOne(LPCWSTR lpszKeyword, const DWORD dwMatchMode, const BOOL bIgnoreCase, LPCWSTR lpszFileName, const DWORD dwPrevCount)
{
	DWORD dwCount = 0;
	FILE* fp = _wfopen(lpszFileName, L"r");
	if(fp == NULL) return 0;

	int nLength = wcslen(lpszKeyword);
	wchar_t* lpszKeywordLow = NULL;
	if((dwMatchMode == MATCH_MODE_ANY) && (bIgnoreCase)){
		wchar_t* lpszKeywordLow = new wchar_t[wcslen(lpszKeyword) + 1];
		wcscpy(lpszKeywordLow, lpszKeyword);
		for(int i = 0; lpszKeywordLow[i] != 0; i++){
			if(lpszKeywordLow[i] >= L'A' && lpszKeywordLow[i] <= L'Z'){
				lpszKeywordLow[i] -= L'a' - L'A';
			}
		}
	}

	wchar_t* lpszBuffer   = new wchar_t[MAX_PATH_LENGTH];
	wchar_t* lpszKey      = new wchar_t[MAX_PATH_LENGTH];
	wchar_t* lpszFile     = new wchar_t[MAX_PATH_LENGTH];
	wchar_t* lpszType     = new wchar_t[MAX_PATH_LENGTH];
	wchar_t* lpszTypeName = new wchar_t[MAX_PATH_LENGTH];
	wchar_t* lpszKeyLow   = new wchar_t[MAX_PATH_LENGTH];
	int nLine;
	while(fgetws(lpszBuffer, MAX_PATH_LENGTH, fp)){
		if(lpszBuffer[0] == L'!') continue;
		wcscpy(lpszKey, _T(""));
		wcscpy(lpszFile, _T(""));
		wcscpy(lpszType, _T(""));
		wcscpy(lpszTypeName, _T(""));
		nLine = 0;
		if(swscanf(lpszBuffer, TAG_FORMAT, lpszKey, lpszFile, &nLine, lpszType, lpszTypeName) < 3) continue;
		if(dwMatchMode == MATCH_MODE_PERFECT){
			if(bIgnoreCase){
				if(wcsicmp(lpszKey, lpszKeyword) != 0) continue;
			}else{
				int result = wcscmp(lpszKey, lpszKeyword);
				if(result < 0) continue;
				if(result > 0) break;
			}
		}else if(dwMatchMode == MATCH_MODE_BEGIN){
			if(bIgnoreCase){
				if(wcsnicmp(lpszKey, lpszKeyword, nLength) != 0) continue;
			}else{
				int result = wcsncmp(lpszKey, lpszKeyword, nLength);
				if(result < 0) continue;
				if(result > 0) break;
			}
		}else if(dwMatchMode == MATCH_MODE_ANY){
			if(bIgnoreCase){
				wcscpy(lpszKeyLow, lpszKey);
				for(int i = 0; lpszKeyLow[i] != 0; i++){
					if(lpszKeyLow[i] >= L'A' && lpszKeyLow[i] <= L'Z'){
						lpszKeyLow[i] -= L'a' - L'A';
					}
				}
				if(wcsstr(lpszKey, lpszKeywordLow) == 0) continue;
			}else{
				if(wcsstr(lpszKey, lpszKeyword) == 0) continue;
			}
		}else{
			break;
		}

		CtagsData* info = new CtagsData(lpszKey, lpszFile, nLine, lpszType, lpszTypeName);
		m_CtagsDataList.push_back(info);
		dwCount++;
		if((dwCount + dwPrevCount) >= m_lpCtagsOption->m_dwMaxFind){
			break;
		}
	}
	delete[] lpszBuffer;
	delete[] lpszKey;
	delete[] lpszFile;
	delete[] lpszType;
	delete[] lpszTypeName;
	delete[] lpszKeyLow;
	if(lpszKeywordLow != NULL) delete[] lpszKeywordLow;

	fclose(fp);

	return dwCount;
}

///////////////////////////////////////////////////////////////////////////////
int CJumpListDialog::InsertItem(HWND hList, int nIndex, CtagsData* info)
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
	ListView_SetItemText(hList, nResult, 2, (LPWSTR)info->m_strType.c_str());
	ListView_SetItemText(hList, nResult, 3, (LPWSTR)info->m_strTypeName.c_str());
	ListView_SetItemText(hList, nResult, 4, (LPWSTR)info->m_strFile.c_str());
	return nResult;
}

///////////////////////////////////////////////////////////////////////////////
void CJumpListDialog::GetItem(HWND hList, int nIndex, CtagsData* info)
{
	wchar_t* lpszBuffer = new wchar_t[MAX_PATH_LENGTH];
	if(lpszBuffer != NULL){
		ListView_GetItemText(hList, nIndex, 0, lpszBuffer, MAX_PATH_LENGTH);
		info->m_strKeyword = lpszBuffer;
		ListView_GetItemText(hList, nIndex, 1, lpszBuffer, MAX_PATH_LENGTH);
		info->m_nLine = _wtoi(lpszBuffer);
		ListView_GetItemText(hList, nIndex, 2, lpszBuffer, MAX_PATH_LENGTH);
		info->m_strType = lpszBuffer;
		ListView_GetItemText(hList, nIndex, 3, lpszBuffer, MAX_PATH_LENGTH);
		info->m_strTypeName = lpszBuffer;
		ListView_GetItemText(hList, nIndex, 4, lpszBuffer, MAX_PATH_LENGTH);
		info->m_strFile = lpszBuffer;
		delete[] lpszBuffer;
	}
}

///////////////////////////////////////////////////////////////////////////////
void CJumpListDialog::StartTimer()
{
	StopTimer();
	if(m_lpCtagsOption->m_dwDelay > 0){
		m_nTimerID = ::SetTimer(GetHwnd(), 1, m_lpCtagsOption->m_dwDelay, NULL);
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

	ReadCtagsFile(m_strKeyword.c_str(), m_dwMatchMode, m_bIgnoreCase);

	SetDataSub();

	m_bOperation = FALSE;

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
void CJumpListDialog::RemoveAllCtagsDataList(std::list<CtagsData*>& p)
{
	for(std::list<CtagsData*>::iterator it = p.begin(); it != p.end(); ++it){
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
DWORD CJumpListDialog::ReadSqliteFileOne(LPCWSTR lpszKeyword, const DWORD dwMatchMode, const BOOL bIgnoreCase, LPCWSTR lpszFileName, const DWORD dwPrevCount)
{
	DWORD dwCount = 0;

	int nLength = wcslen(lpszKeyword);
	WideString strKeyword = lpszKeyword;
	WideString strLike = L"%";
	LPCWSTR sql_base;
	wchar_t sql[MAX_PATH_LENGTH];

	if(dwMatchMode == MATCH_MODE_PERFECT){
		strKeyword = lpszKeyword;
		if(bIgnoreCase == FALSE){
			//sql_base = L"SELECT strKeyword,strFile,nLine,strType,strTypeName FROM CTAGS WHERE strKeyword=? ORDER BY strKeyword ASC, nLine ASC, strFile ASC LIMIT %u;";
			sql_base = L"SELECT strKeyword,strFile,nLine,strType,strTypeName FROM CTAGS WHERE strKeyword=? LIMIT %u;";
		}else{
			//sql_base = L"SELECT strKeyword,strFile,nLine,strType,strTypeName FROM CTAGS WHERE strKeyword COLLATE NOCASE=? ORDER BY strKeyword ASC, nLine ASC, strFile ASC LIMIT %u;";
			sql_base = L"SELECT strKeyword,strFile,nLine,strType,strTypeName FROM CTAGS WHERE strKeyword COLLATE NOCASE=? LIMIT %u;";
		}
	}else if(dwMatchMode == MATCH_MODE_BEGIN){
		strKeyword = lpszKeyword + strLike;
		if(bIgnoreCase == FALSE){
			//sql_base = L"SELECT strKeyword,strFile,nLine,strType,strTypeName FROM CTAGS WHERE strKeyword LIKE ? ORDER BY strKeyword ASC, nLine ASC, strFile ASC LIMIT %u;";
			sql_base = L"SELECT strKeyword,strFile,nLine,strType,strTypeName FROM CTAGS WHERE strKeyword LIKE ? LIMIT %u;";
		}else{
			//sql_base = L"SELECT strKeyword,strFile,nLine,strType,strTypeName FROM CTAGS WHERE strKeyword COLLATE NOCASE LIKE ? ORDER BY strKeyword ASC, nLine ASC, strFile ASC LIMIT %u;";
			sql_base = L"SELECT strKeyword,strFile,nLine,strType,strTypeName FROM CTAGS WHERE strKeyword COLLATE NOCASE LIKE ? LIMIT %u;";
		}
	}else if(dwMatchMode == MATCH_MODE_ANY){
		strKeyword = strLike + lpszKeyword + strLike;
		if(bIgnoreCase == FALSE){
			//sql_base = L"SELECT strKeyword,strFile,nLine,strType,strTypeName FROM CTAGS WHERE strKeyword LIKE ? ORDER BY strKeyword ASC, nLine ASC, strFile ASC LIMIT %u;";
			sql_base = L"SELECT strKeyword,strFile,nLine,strType,strTypeName FROM CTAGS WHERE strKeyword LIKE ? LIMIT %u;";
		}else{
			//sql_base = L"SELECT strKeyword,strFile,nLine,strType,strTypeName FROM CTAGS WHERE strKeyword COLLATE NOCASE LIKE ? ORDER BY strKeyword ASC, nLine ASC, strFile ASC LIMIT %u;";
			sql_base = L"SELECT strKeyword,strFile,nLine,strType,strTypeName FROM CTAGS WHERE strKeyword COLLATE NOCASE LIKE ? LIMIT %u;";
		}
	}else{
		return 0;
	}
	swprintf(sql, sql_base, m_lpCtagsOption->m_dwMaxFind);

	sqlite3* db = NULL;
	int result = thePluginService.Sqlite.sqlite3_open16(lpszFileName, &db);
	if(result != SQLITE_OK){
		WideString strCause = thePluginService.Sqlite.sqlite3_errmsg16(db);
		//::MessageBox(GetHwnd(), strCause.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	sqlite3_stmt* select_sql = NULL;
	result = thePluginService.Sqlite.sqlite3_prepare16(db, sql, wcslen(sql) * sizeof(wchar_t), &select_sql, NULL);
	if(result != SQLITE_OK){
		WideString strCause = thePluginService.Sqlite.sqlite3_errmsg16(db);
		//::MessageBox(GetHwnd(), strCause.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
		thePluginService.Sqlite.sqlite3_close(db);
		return 0;
	}

	thePluginService.Sqlite.sqlite3_reset(select_sql);
	thePluginService.Sqlite.sqlite3_bind_text16(select_sql, 1, strKeyword.c_str(), strKeyword.length() * sizeof(wchar_t),      SQLITE_STATIC);
	thePluginService.Sqlite.sqlite3_bind_int(   select_sql, 2, m_lpCtagsOption->m_dwMaxFind);

	while(thePluginService.Sqlite.sqlite3_step(select_sql) == SQLITE_ROW){
		WideString strKeyword  = thePluginService.Sqlite.sqlite3_column_text16(select_sql, 0);
		WideString strFile     = thePluginService.Sqlite.sqlite3_column_text16(select_sql, 1);
		int        nLine       = thePluginService.Sqlite.sqlite3_column_int(select_sql, 2);
		WideString strType     = thePluginService.Sqlite.sqlite3_column_text16(select_sql, 3);
		WideString strTypeName = thePluginService.Sqlite.sqlite3_column_text16(select_sql, 4);

		//SQLITEのLIKE句は常にCOLLATE NOCASEなのでチェックする
		if((dwMatchMode == MATCH_MODE_BEGIN) && (bIgnoreCase == FALSE)){
			if(wcsncmp(strKeyword.c_str(), lpszKeyword, nLength) != 0) continue;
		}else if((dwMatchMode == MATCH_MODE_ANY) && (bIgnoreCase == FALSE)){
			if(wcsstr(strKeyword.c_str(), lpszKeyword) == 0) continue;
		}

		CtagsData* p = new CtagsData(strKeyword.c_str(), strFile.c_str(), nLine, strType.c_str(), strTypeName.c_str());
		m_CtagsDataList.push_back(p);
		dwCount++;
		if((dwCount + dwPrevCount) >= m_lpCtagsOption->m_dwMaxFind){
			break;
		}
	}

	thePluginService.Sqlite.sqlite3_finalize(select_sql);
	thePluginService.Sqlite.sqlite3_close(db);

	return dwCount;
}
