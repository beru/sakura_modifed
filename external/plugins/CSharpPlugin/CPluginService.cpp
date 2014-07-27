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

#include "stdafx.h"
#include <windows.h>
#include "CPluginService.h"
#include "COutputDialog.h"
#include "CPluginDlgCancel.h"
#include "plugin/SakuraPlugin.h"
#include "CommonTools.h"
#include "resource.h"

///////////////////////////////////////////////////////////////////////////////
CPluginService::CPluginService()
{
	m_dwMode = 0;
}

///////////////////////////////////////////////////////////////////////////////
CPluginService::~CPluginService()
{
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::OnPluginCscCompile(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	OnPluginCscCompileSub(0);
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::OnPluginCscCompileAndRun(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	OnPluginCscCompileSub(1);
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::OnPluginCscCompileSub(const int nType)
{
	ReadProfile();

	bool bGet = false;
	WideString strText;
	if(m_dwMode == 1){
		strText = Editor.GetSelectedString(0);
		if(strText.length() != 0){
			bGet = true;
		}
	}
	if(bGet == false){
		Editor.SelectAll();
		strText = Editor.GetSelectedString(0);
	}

	::DeleteFile(m_strCsFileName.c_str());
	::DeleteFile(m_strCompileLogFileName.c_str());
	::DeleteFile(m_strRunLogFileName.c_str());

	WideString strMessage;
	if(SaveCsFile(m_strCsFileName.c_str(), strText.c_str()) == FALSE){
		thePluginService.LoadString(IDS_STR_ERROR1, strMessage);	//スクリプトのコンパイルに失敗しました。
		::MessageBox(GetParentHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	HANDLE hProcess1 = ExecuteCommandCompile(m_strCsFileName.c_str(), m_strCompileLogFileName.c_str());
	if(hProcess1 == NULL){
		WideString strMessage;
		LoadString(IDS_STR_ERROR1, strMessage);
		::MessageBox(GetParentHwnd(), strMessage.c_str(), GetPluginName(), MB_OK);
		return;
	}

	CPluginDlgCancel dlg1;
	INT_PTR nRet1 = dlg1.DoModal(GetInstance(), GetParentHwnd(), IDD_EXECUTE_DIALOG, (LPARAM)hProcess1);
	switch(nRet1){
	case IDOK:
		break;
	case IDCANCEL:
		thePluginService.LoadString(IDS_STR_MSG1, strMessage);	//スクリプトのコンパイルを中止しました。
		::MessageBox(GetParentHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
		return;
	case IDABORT:
	default:
		DisplayResult(m_strCompileLogFileName.c_str());
		//thePluginService.LoadString(IDS_STR_ERROR1, strMessage);	//スクリプトのコンパイルに失敗しました。
		//::MessageBox(GetParentHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	if(nType == 0){
		DisplayResult(m_strCompileLogFileName.c_str());
		return;
	}

	HANDLE hProcess2 = ExecuteCommandRun(m_strRunFileName.c_str(), m_strRunLogFileName.c_str());
	if(hProcess2 == NULL){
		WideString strMessage;
		LoadString(IDS_STR_ERROR2, strMessage);
		::MessageBox(GetParentHwnd(), strMessage.c_str(), GetPluginName(), MB_OK);
		return;
	}

	CPluginDlgCancel dlg2;
	INT_PTR nRet2 = dlg2.DoModal(GetInstance(), GetParentHwnd(), IDD_EXECUTE_DIALOG, (LPARAM)hProcess2);
	switch(nRet2){
	case IDOK:
		break;
	case IDCANCEL:
		thePluginService.LoadString(IDS_STR_MSG2, strMessage);	//スクリプトの実行を中止しました。
		::MessageBox(GetParentHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
		return;
	case IDABORT:
	default:
		DisplayResult(m_strCompileLogFileName.c_str());
		//thePluginService.LoadString(IDS_STR_ERROR2, strMessage);	//スクリプトの実行に失敗しました。
		//::MessageBox(GetParentHwnd(), strMessage.c_str(), thePluginService.GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
	return;
	}

	DisplayResult(m_strRunLogFileName.c_str());
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::ReadProfile()
{
	m_strCommandPath = Plugin.GetOption(GetPluginName(), L"CommandPath");
	if(m_strCommandPath.length() == 0){
		m_strCommandPath = DEFAULT_CSC_PATH;
	}
	m_dwMode = Plugin.GetOptionInt(GetPluginName(), L"SelectMode");

	m_strCsFileName = Plugin.GetPluginDir() + L"\\Result\\csc_test.cs";
	m_strCompileLogFileName = Plugin.GetPluginDir() + L"\\Result\\csc_compile.log";
	m_strRunLogFileName = Plugin.GetPluginDir() + L"\\Result\\csc_run.log";
	m_strRunFileName = Plugin.GetPluginDir() + L"\\Result\\csc_run.exe";

	WideString strResultPath = Plugin.GetPluginDir() + L"\\Result";
	::CreateDirectory(strResultPath.c_str(), NULL);
}

///////////////////////////////////////////////////////////////////////////////
HANDLE CPluginService::ExecuteCommandCompile(LPCWSTR lpszCsFileName, LPCWSTR lpszCompileLogFileName)
{
	if(m_strCommandPath.length() == 0){
		return NULL;
	}
	wchar_t szCmdLine[32768];
	wsprintf(szCmdLine, L"cmd.exe /c \"\"%s\" /target:exe /out:\"%s\" \"%s\" > \"%s\"\"", m_strCommandPath.c_str(), m_strRunFileName.c_str(), lpszCsFileName, lpszCompileLogFileName);
	//::MessageBox(GetParentHwnd(), lpszCmdLine, L"DEBUG", MB_OK);
	//差分コマンドを実行する
	PROCESS_INFORMATION pi;
	::ZeroMemory(&pi, sizeof(pi));
	STARTUPINFO si;
	::ZeroMemory(&si, sizeof(si));
	si.cb          = sizeof(si);
	si.dwFlags     = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;
	BOOL bProcessResult = ::CreateProcess(NULL, szCmdLine, NULL, NULL, FALSE, 0, NULL, Plugin.GetPluginDir().c_str(), &si, &pi);
	if(bProcessResult == FALSE){
		return NULL;
	}
	::CloseHandle(pi.hThread);
	return pi.hProcess;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CPluginService::SaveCsFile(LPCWSTR lpszCsFileName, LPCWSTR lpszText)
{
	AnsiString strText = to_astr(lpszText, CP_UTF8);
	FILE* fp = _wfopen(lpszCsFileName, L"wb");
	if(fp == NULL) return FALSE;
	fputs(strText.c_str(), fp);
	fclose(fp);
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
HANDLE CPluginService::ExecuteCommandRun(LPCWSTR lpszRunFileName, LPCWSTR lpszRunLogFileName)
{
	wchar_t szCmdLine[32768];
	wsprintf(szCmdLine, L"cmd.exe /c \"\"%s\" > \"%s\"\"", lpszRunFileName, lpszRunLogFileName);
	//::MessageBox(GetParentHwnd(), lpszCmdLine, L"DEBUG", MB_OK);
	//差分コマンドを実行する
	PROCESS_INFORMATION pi;
	::ZeroMemory(&pi, sizeof(pi));
	STARTUPINFO si;
	::ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;
	BOOL bProcessResult = ::CreateProcess(NULL, szCmdLine, NULL, NULL, FALSE, 0, NULL, Plugin.GetPluginDir().c_str(), &si, &pi);
	if(bProcessResult == FALSE){
		return NULL;
	}
	::CloseHandle(pi.hThread);
	return pi.hProcess;
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::DisplayResult(LPCWSTR lpszLogFileName)
{
	WideString strText;
	FILE* fp = _wfopen(lpszLogFileName, L"rb");
	if(fp != NULL){
		struct _stat st;
		_wstat(lpszLogFileName, &st);
		char* lpszBuffer = new char[st.st_size + 1];
		fread(lpszBuffer, st.st_size, 1, fp);
		lpszBuffer[st.st_size] = 0;
		fclose(fp);
		strText = to_wstr(lpszBuffer);
		delete[] lpszBuffer;
	}

	COutputDialog dlg;
	dlg.DoModal(GetPluginDllInstance(), GetParentHwnd(), IDD_DIALOG_OUTPUT, strText.c_str());
}