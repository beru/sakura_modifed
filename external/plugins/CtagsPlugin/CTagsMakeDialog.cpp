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
#include "resource.h"
#include "CTagsMakeDialog.h"
#include "CPluginService.h"
#include "CommonTools.h"
#include "CPluginDlgCancel.h"
#include "CSqlite.h"

///////////////////////////////////////////////////////////////////////////////
CTagsMakeDialog::CTagsMakeDialog(CtagsOption* lpCtagsOption, CtagsInfo* lpCtagsInfo)
{
	m_nRetCode      = IDCANCEL;
	m_lpCtagsInfo   = lpCtagsInfo;
	m_lpCtagsOption = lpCtagsOption;
}

///////////////////////////////////////////////////////////////////////////////
CTagsMakeDialog::~CTagsMakeDialog()
{
}

///////////////////////////////////////////////////////////////////////////////
/*! モーダルダイアログの表示
*/
INT_PTR CTagsMakeDialog::DoModal(HINSTANCE hInstance, HWND hwndParent)
{
	//とりあえずキャンセル状態にしておく。
	m_nRetCode = IDCANCEL;
	return CPluginDialog::DoModal(hInstance, hwndParent, IDD_CTAGS_MAKE_DIALOG, (LPARAM)NULL);
}

///////////////////////////////////////////////////////////////////////////////
/*! 初期化処理
*/
BOOL CTagsMakeDialog::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	BOOL bResult = CPluginDialog::OnInitDialog(hwndDlg, wParam, lParam);

	HWND hEditFolder = ::GetDlgItem(GetHwnd(), IDC_EDIT_FOLDER);
	HWND hEditOption = ::GetDlgItem(GetHwnd(), IDC_EDIT_CMDLINE);
	HWND hEditResult = ::GetDlgItem(GetHwnd(), IDC_EDIT_RESULT);
	::SendMessage(hEditFolder, EM_LIMITTEXT, (WPARAM)1024, 0);
	::SendMessage(hEditOption, EM_LIMITTEXT, (WPARAM)256, 0);
	::SendMessage(hEditResult, EM_LIMITTEXT, (WPARAM)256, 0);
	SetData();
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CTagsMakeDialog::OnBnClicked(int wID)
{
	switch(wID){
	case IDOK:
		m_nRetCode = wID;
		if(GetData()){
			CloseDialog(IDOK);
		}
		break;

	case IDCANCEL:
		m_nRetCode = wID;
		CloseDialog(IDCANCEL);
		break;

	case IDC_BUTTON_BROWSE:
		{
			HWND hEditFolder = ::GetDlgItem(GetHwnd(), IDC_EDIT_FOLDER);
			WideString strTargetPath = GetWindowText(hEditFolder);
			WideString strTargetPathPrev = strTargetPath;
			WideString strMessage;
			thePluginService.LoadString(IDS_STR_MSG1, strMessage);	//生成ファイル格納フォルダを指定してください。
			if(GetOpenFolderNameDialog(GetHwnd(), strMessage.c_str(), strTargetPath.c_str(), strTargetPath)){
				if((strTargetPathPrev.length() != 0) && (strTargetPathPrev != strTargetPath)){
					thePluginService.LoadString(IDS_STR_MSG2, strMessage);	//生成ファイル格納フォルダを変更されました。\r\nタグファイルの再作成をしてください。
					::MessageBox(GetHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONINFORMATION | MB_OK);
				}
				::SetWindowText(hEditFolder, strTargetPath.c_str());
			}
		}
		break;

	case IDC_BUTTON_UP:
		{
			HWND hEditFolder = ::GetDlgItem(GetHwnd(), IDC_EDIT_FOLDER);
			WideString strTargetPath = GetWindowText(hEditFolder);
			strTargetPath = GetParentFolder(strTargetPath);
			::SetWindowText(hEditFolder, strTargetPath.c_str());
		}
		break;

	case IDC_BUTTON_EXECUTE:
		{
			WideString strMessage;
			HWND hEditFolder = ::GetDlgItem(GetHwnd(), IDC_EDIT_FOLDER);
			HWND hEditOption = ::GetDlgItem(GetHwnd(), IDC_EDIT_CMDLINE);
			HWND hEditResult = ::GetDlgItem(GetHwnd(), IDC_EDIT_RESULT);
			m_lpCtagsInfo->m_strTargetPath = GetWindowText(hEditFolder);
			m_lpCtagsInfo->m_strOption     = GetWindowText(hEditOption);
			m_lpCtagsInfo->m_bSubFolder    = (::IsDlgButtonChecked(GetHwnd(), IDC_CHECKBOX_SUBFOLDER) == BST_CHECKED) ? 1 : 0;
			WideString strUniqID = GetWindowText(hEditResult);
			m_lpCtagsInfo->m_dwUniqID      = thePluginService.GetHexStringToDword(strUniqID.c_str());
			if(m_lpCtagsInfo->m_strTargetPath.length() == 0){
				thePluginService.LoadString(IDS_STR_MSG3, strMessage);	//必須項目が入力されていません。
				::MessageBox(GetHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
				return FALSE;
			}

			//古いファイルを削除する
			WideString strResultFile = thePluginService.GetResultFile(m_lpCtagsInfo->m_dwUniqID);
			WideString strDbName = thePluginService.GetResultDbName(m_lpCtagsInfo->m_dwUniqID);
			::DeleteFile(strResultFile.c_str());
			::DeleteFile(strDbName.c_str());

			HANDLE hProcess = OnExecuteCtags();
			if(hProcess == NULL){
				thePluginService.LoadString(IDS_STR_MSG5, strMessage);	//タグファイルの作成に失敗しました。
				::MessageBox(GetHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
				return FALSE;
			}

			CPluginDlgCancel dlg;
			INT_PTR nRet = dlg.DoModal(thePluginService.GetInstance(), GetHwnd(), IDD_EXECUTE_DIALOG, (LPARAM)hProcess);
			switch(nRet){
			case IDOK:
				break;
			case IDCANCEL:
				thePluginService.LoadString(IDS_STR_MSG5, strMessage);	//タグファイルの作成に失敗しました。
				::MessageBox(GetHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
				return FALSE;
			case IDABORT:
			default:
				thePluginService.LoadString(IDS_STR_MSG4, strMessage);	//タグファイルの作成を中止しました。
				::MessageBox(GetHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
				return FALSE;
			}

			//SQLITEがある場合はDB化する
			if(m_lpCtagsOption->m_strSqliteDllPath.length() != 0){
				if(OnExecuteSqlite() == FALSE){
					thePluginService.LoadString(IDS_STR_MSG5, strMessage);	//タグファイルの作成に失敗しました。
					::MessageBox(GetHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
					return FALSE;
				}
			}

			thePluginService.LoadString(IDS_STR_MSG7, strMessage);	//タグファイルを作成しました。
			::MessageBox(GetHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONINFORMATION | MB_OK);
			OnBnClicked(IDOK);
			return TRUE;
		}
		break;

	default:
		break;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
void CTagsMakeDialog::SetData(void)
{
	if(GetHwnd() == NULL) return;
	HWND hEditFolder = ::GetDlgItem(GetHwnd(), IDC_EDIT_FOLDER);
	HWND hEditOption = ::GetDlgItem(GetHwnd(), IDC_EDIT_CMDLINE);
	HWND hEditResult = ::GetDlgItem(GetHwnd(), IDC_EDIT_RESULT);
	::SetWindowText(hEditFolder, m_lpCtagsInfo->m_strTargetPath.c_str());
	::CheckDlgButton(GetHwnd(), IDC_CHECKBOX_SUBFOLDER, m_lpCtagsInfo->m_bSubFolder ? BST_CHECKED : BST_UNCHECKED);
	::SetWindowText(hEditOption, m_lpCtagsInfo->m_strOption.c_str());
	WideString strUniqID = thePluginService.GetDwordToHexString(m_lpCtagsInfo->m_dwUniqID);
	::SetWindowText(hEditResult, strUniqID.c_str());
}

///////////////////////////////////////////////////////////////////////////////
int CTagsMakeDialog::GetData( void )
{
	switch(m_nRetCode){
	case IDOK:
		{
			HWND hEditFolder = ::GetDlgItem(GetHwnd(), IDC_EDIT_FOLDER);
			HWND hEditOption = ::GetDlgItem(GetHwnd(), IDC_EDIT_CMDLINE);
			HWND hEditResult = ::GetDlgItem(GetHwnd(), IDC_EDIT_RESULT);
			m_lpCtagsInfo->m_strTargetPath = GetWindowText(hEditFolder);
			m_lpCtagsInfo->m_strOption     = GetWindowText(hEditOption);
			m_lpCtagsInfo->m_bSubFolder    = (::IsDlgButtonChecked(GetHwnd(), IDC_CHECKBOX_SUBFOLDER) == BST_CHECKED) ? 1 : 0;
			WideString strUniqID = GetWindowText(hEditResult);
			m_lpCtagsInfo->m_dwUniqID      = thePluginService.GetHexStringToDword(strUniqID.c_str());
			if(m_lpCtagsInfo->m_strTargetPath.length() == 0){
				WideString strMessage;
				thePluginService.LoadString(IDS_STR_MSG3, strMessage);	//必須項目が入力されていません。
				::MessageBox(GetHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
				return FALSE;
			}
		}
		break;

	default:
		break;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
HANDLE CTagsMakeDialog::OnExecuteCtags()
{
	WideString strOption;
	strOption = L"--excmd=number";
	if(m_lpCtagsInfo->m_bSubFolder) strOption += L" --recurse=yes";	//"-R"
	if(m_lpCtagsInfo->m_strOption.length() != 0) strOption += L" " + m_lpCtagsInfo->m_strOption;
	strOption += L" -f \"" + thePluginService.GetResultFile(m_lpCtagsInfo->m_dwUniqID) + L"\"";
	strOption += L" \"" + m_lpCtagsInfo->m_strTargetPath + L"\"";

	WideString strCmdPath = GetSystemDirectory();

	wchar_t szCmdLine[MAX_PATH_LENGTH];
	wsprintf(szCmdLine, L"\"%s\\cmd.exe\" /D /C \"\"%s\" %s\"", strCmdPath.c_str(), m_lpCtagsOption->m_strCtagsExePath.c_str(), strOption.c_str());
	//::MessageBox(GetHwnd(), lpszCmdLine, L"DEBUG", MB_OK);

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
BOOL CTagsMakeDialog::OnExecuteSqlite()
{
	BOOL bResult = TRUE;
	WideString strResultFile = thePluginService.GetResultFile(m_lpCtagsInfo->m_dwUniqID);
	WideString strDbName = thePluginService.GetResultDbName(m_lpCtagsInfo->m_dwUniqID);

	FILE* fp = _wfopen(strResultFile.c_str(), L"r");
	if(fp == NULL){
		return FALSE;
	}

	sqlite3* db = NULL;
	int result = thePluginService.Sqlite.sqlite3_open16(strDbName.c_str(), &db);
	if(result != SQLITE_OK){
		WideString strCause = thePluginService.Sqlite.sqlite3_errmsg16(db);
		//::MessageBox(GetHwnd(), strCause.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
		fclose(fp);
		return FALSE;
	}

	result = thePluginService.Sqlite.sqlite3_exec(db, 
		"CREATE TABLE CTAGS(strKeyword TEXT, strFile TEXT, nLine INTEGER, strType TEXT, strTypeName TEXT);",
		NULL, 0, 0);
	if(result != SQLITE_OK){
		WideString strCause = thePluginService.Sqlite.sqlite3_errmsg16(db);
		//::MessageBox(GetHwnd(), strCause.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
		thePluginService.Sqlite.sqlite3_close(db);
		fclose(fp);
		return FALSE;
	}

	sqlite3_stmt* insert_sql = NULL;
	LPCWSTR sql = L"INSERT INTO CTAGS(strKeyword,strFile,nLine,strType,strTypeName) VALUES (?,?,?,?,?);";
	result = thePluginService.Sqlite.sqlite3_prepare16(db, sql, wcslen(sql) * sizeof(wchar_t), &insert_sql, NULL);
	if(result != SQLITE_OK){
		WideString strCause = thePluginService.Sqlite.sqlite3_errmsg16(db);
		//::MessageBox(GetHwnd(), strCause.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
		thePluginService.Sqlite.sqlite3_close(db);
		fclose(fp);
		return FALSE;
	}

	wchar_t* lpszBuffer   = new wchar_t[MAX_PATH_LENGTH];
	wchar_t* lpszKey      = new wchar_t[MAX_PATH_LENGTH];
	wchar_t* lpszFile     = new wchar_t[MAX_PATH_LENGTH];
	wchar_t* lpszType     = new wchar_t[MAX_PATH_LENGTH];
	wchar_t* lpszTypeName = new wchar_t[MAX_PATH_LENGTH];
	int nLine;

	thePluginService.Sqlite.sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL);
	while(fgetws(lpszBuffer, MAX_PATH_LENGTH, fp)){
		if(lpszBuffer[0] == L'!') continue;
		wcscpy(lpszKey, _T(""));
		wcscpy(lpszFile, _T(""));
		wcscpy(lpszType, _T(""));
		wcscpy(lpszTypeName, _T(""));
		nLine = 0;
		if(swscanf(lpszBuffer, TAG_FORMAT, lpszKey, lpszFile, &nLine, lpszType, lpszTypeName) < 3) continue;

		thePluginService.Sqlite.sqlite3_reset(insert_sql);
		thePluginService.Sqlite.sqlite3_bind_text16(insert_sql, 1, lpszKey,      wcslen(lpszKey) * sizeof(wchar_t),      SQLITE_STATIC);
		thePluginService.Sqlite.sqlite3_bind_text16(insert_sql, 2, lpszFile,     wcslen(lpszFile) * sizeof(wchar_t),     SQLITE_STATIC);
		thePluginService.Sqlite.sqlite3_bind_int(   insert_sql, 3, nLine);
		thePluginService.Sqlite.sqlite3_bind_text16(insert_sql, 4, lpszType,     wcslen(lpszType) * sizeof(wchar_t),     SQLITE_STATIC);
		thePluginService.Sqlite.sqlite3_bind_text16(insert_sql, 5, lpszTypeName, wcslen(lpszTypeName) * sizeof(wchar_t), SQLITE_STATIC);
		result = thePluginService.Sqlite.sqlite3_step(insert_sql);
		if(result != SQLITE_DONE){
			WideString strCause = thePluginService.Sqlite.sqlite3_errmsg16(db);
			//::MessageBox(GetHwnd(), strCause.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
		}
	}
	if(bResult){
		thePluginService.Sqlite.sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
	}else{
		thePluginService.Sqlite.sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
	}
	
	delete[] lpszBuffer;
	delete[] lpszKey;
	delete[] lpszFile;
	delete[] lpszType;
	delete[] lpszTypeName;

	thePluginService.Sqlite.sqlite3_finalize(insert_sql);
	thePluginService.Sqlite.sqlite3_close(db);
	fclose(fp);
	return bResult;
}
