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
#include "resource.h"
#include "CMakeDialog.h"
#include "CPluginService.h"
#include "CommonTools.h"
#include "CPluginDlgCancel.h"

///////////////////////////////////////////////////////////////////////////////
CMakeDialog::CMakeDialog(CGlobalOption* lpGlobalOption, CGlobalInfo* lpGlobalInfo)
{
	m_nRetCode       = IDCANCEL;
	m_lpGlobalOption = lpGlobalOption;
	m_lpGlobalInfo   = lpGlobalInfo;
}

///////////////////////////////////////////////////////////////////////////////
CMakeDialog::~CMakeDialog()
{
}

///////////////////////////////////////////////////////////////////////////////
/*! モーダルダイアログの表示
*/
INT_PTR CMakeDialog::DoModal(HINSTANCE hInstance, HWND hwndParent)
{
	//とりあえずキャンセル状態にしておく。
	m_nRetCode = IDCANCEL;
	return CPluginDialog::DoModal(hInstance, hwndParent, IDD_MAKE_DIALOG, (LPARAM)NULL);
}

///////////////////////////////////////////////////////////////////////////////
/*! 初期化処理
*/
BOOL CMakeDialog::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	BOOL bResult = CPluginDialog::OnInitDialog(hwndDlg, wParam, lParam);

	HWND hEditFolder = ::GetDlgItem(GetHwnd(), IDC_EDIT_FOLDER);
	HWND hEditResult = ::GetDlgItem(GetHwnd(), IDC_EDIT_RESULT);
	::SendMessage(hEditFolder, EM_LIMITTEXT, (WPARAM)1024, 0);
	::SendMessage(hEditResult, EM_LIMITTEXT, (WPARAM)256, 0);
	SetData();
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CMakeDialog::OnBnClicked(int wID)
{
	switch (wID) {
	case IDOK:
		m_nRetCode = wID;
		if (GetData()) {
			CloseDialog(IDOK);
		}
		break;

	case IDCANCEL:
		m_nRetCode = wID;
		CloseDialog(IDCANCEL);
		break;

	case IDC_BUTTON_BROWSE1:
		{
			HWND hEditFolder = ::GetDlgItem(GetHwnd(), IDC_EDIT_FOLDER);
			WideString strTargetPath = GetWindowText(hEditFolder);
			WideString strTargetPathPrev = strTargetPath;
			WideString strMessage;
			thePluginService.LoadString(IDS_STR_MSG4, strMessage);	//生成ファイル格納フォルダを指定してください。
			if (GetOpenFolderNameDialog(GetHwnd(), strMessage.c_str(), strTargetPath.c_str(), strTargetPath)) {
				if ((strTargetPathPrev.length() != 0) && (strTargetPathPrev != strTargetPath)) {
					thePluginService.LoadString(IDS_STR_MSG5, strMessage);	//生成ファイル格納フォルダを変更されました。\r\nタグファイルの再作成をしてください。
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
			HWND hEditResult = ::GetDlgItem(GetHwnd(), IDC_EDIT_RESULT);
			m_lpGlobalInfo->m_strTargetPath = GetWindowText(hEditFolder);
			WideString strUniqID = GetWindowText(hEditResult);
			m_lpGlobalInfo->m_dwUniqID      = thePluginService.GetHexStringToDword(strUniqID.c_str());
			if (m_lpGlobalInfo->m_strTargetPath.length() == 0) {
				thePluginService.LoadString(IDS_STR_MSG6, strMessage);	//必須項目が入力されていません。
				::MessageBox(GetHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
				return FALSE;
			}

			//古いファイルを削除する
			thePluginService.RemoveResultPath(m_lpGlobalInfo->m_dwUniqID, FALSE);
			WideString strResultPath = thePluginService.GetResultPath(m_lpGlobalInfo->m_dwUniqID);
			::CreateDirectory(strResultPath.c_str(), NULL);

			HANDLE hProcess = OnExecuteGtags();
			if (hProcess == NULL) {
				thePluginService.LoadString(IDS_STR_MSG8, strMessage);	//タグファイルの作成に失敗しました。
				::MessageBox(GetHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
				return FALSE;
			}
#if 0
			CPluginDlgCancel dlg;
			INT_PTR nRet = dlg.DoModal(thePluginService.GetInstance(), GetHwnd(), IDD_EXECUTE_DIALOG, (LPARAM)hProcess);
			switch(nRet){
			case IDOK:
				break;
			case IDCANCEL:
				thePluginService.LoadString(IDS_STR_MSG7, strMessage);	//タグファイルの作成を中止しました。
				::MessageBox(GetHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
				return FALSE;
			case IDABORT:
			default:
				thePluginService.LoadString(IDS_STR_MSG8, strMessage);	//タグファイルの作成に失敗しました。
				::MessageBox(GetHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
				return FALSE;
			}
#else
			// TODO: check timeout, error
#endif
			thePluginService.LoadString(IDS_STR_MSG9, strMessage);	//タグファイルを作成しました。
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
void CMakeDialog::SetData(void)
{
	if (GetHwnd() == NULL) return;
	WideString strResultPath = thePluginService.GetDwordToHexString(m_lpGlobalInfo->m_dwUniqID);
	HWND hEditFolder = ::GetDlgItem(GetHwnd(), IDC_EDIT_FOLDER);
	HWND hEditResult = ::GetDlgItem(GetHwnd(), IDC_EDIT_RESULT);
	::SetWindowText(hEditFolder, m_lpGlobalInfo->m_strTargetPath.c_str());
	::SetWindowText(hEditResult, strResultPath.c_str());
}

///////////////////////////////////////////////////////////////////////////////
int CMakeDialog::GetData( void )
{
	switch (m_nRetCode) {
	case IDOK:
		{
			HWND hEditFolder = ::GetDlgItem(GetHwnd(), IDC_EDIT_FOLDER);
			HWND hEditResult = ::GetDlgItem(GetHwnd(), IDC_EDIT_RESULT);
			WideString strUniqID = GetWindowText(hEditResult);
			m_lpGlobalInfo->m_strTargetPath = GetWindowText(hEditFolder);
			m_lpGlobalInfo->m_dwUniqID      = thePluginService.GetHexStringToDword(strUniqID.c_str());
			if (m_lpGlobalInfo->m_strTargetPath.length() == 0) {
				WideString strMessage;
				thePluginService.LoadString(IDS_STR_MSG6, strMessage);	//必須項目が入力されていません。
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
/*
	> cd TARGET-PATH
	> gtags DBPATH
*/
HANDLE CMakeDialog::OnExecuteGtags()
{
	WideString strOption = L" \"" + thePluginService.GetResultPath(m_lpGlobalInfo->m_dwUniqID) + L"\"";
	WideString strCmdPath = GetSystemDirectory();

	wchar_t szCmdLine[MAX_PATH_LENGTH];
	wsprintf(szCmdLine, L"\"%s\\cmd.exe\" /D /C \"\"%s\" %s\"", strCmdPath.c_str(), m_lpGlobalOption->m_strGtagsExePath.c_str(), strOption.c_str());

//	::MessageBox(GetHwnd(), szCmdLine, L"DEBUG", MB_OK);

	//gtags.exeを実行する
	PROCESS_INFORMATION pi;
	::ZeroMemory(&pi, sizeof(pi));
	STARTUPINFO si;
	::ZeroMemory(&si, sizeof(si));
	si.cb          = sizeof(si);
	si.dwFlags     = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	BOOL bProcessResult = ::CreateProcess(NULL, szCmdLine, NULL, NULL, FALSE, 0, NULL, m_lpGlobalInfo->m_strTargetPath.c_str(), &si, &pi);
	if (bProcessResult == FALSE) {
		DWORD err = ::GetLastError();
		TCHAR buff[64];
		_itot(err, buff, 10);
		MessageBox(0, buff, 0, 0);
		return NULL;
	}
	::CloseHandle(pi.hThread);
	return pi.hProcess;
}

