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
#include "CPluginShareData.h"
#include "CPluginService.h"
#include "plugin/SakuraPlugin.h"
#include "CSelectDialog.h"
#include "CommonTools.h"
#include "resource.h"
#include <list>

///////////////////////////////////////////////////////////////////////////////
CPluginService::CPluginService()
{
}

///////////////////////////////////////////////////////////////////////////////
CPluginService::~CPluginService()
{
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::OnPluginDiffLauncher(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	ReadProfile();
	m_strFileName1 = Editor.GetFilename();
	if(m_strFileName1.length() == 0) return;	//無題なら何もしない
	m_strFileName2 = L"";
	m_lstFilePath.clear();
	if(GetRecentFiles(m_lstFilePath) == false){
		WideString strMessage;
		LoadString(IDS_STR_ERROR1, strMessage);
		::MessageBox(GetParentHwnd(), strMessage.c_str(), GetPluginName(), MB_OK);
		return;
	}
	CSelectDialog dlg;
	if(dlg.DoModal(GetInstance(), GetParentHwnd()) != IDOK) return;
	if(ExecuteCommand() == false){
		WideString strMessage;
		LoadString(IDS_STR_ERROR2, strMessage);
		::MessageBox(GetParentHwnd(), strMessage.c_str(), GetPluginName(), MB_OK);
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::ReadProfile()
{
	m_strCommandPath = Plugin.GetOption(GetPluginName(), L"CommandPath");
	if(m_strCommandPath.length() == 0){
		m_strCommandPath = DEFAULT_WINMERGE_PATH;
	}
}

///////////////////////////////////////////////////////////////////////////////
bool CPluginService::GetRecentFiles(std::list<std::wstring>& lstFilePath)
{
	bool bResult = false;
	CPluginShareData PluginShareData;
	if(PluginShareData.Initialize(N_SHAREDATA_VERSION)){
		int nMRUArrNum = PluginShareData.m_lpShareData->m_sHistory.m_nMRUArrNum;
		if((nMRUArrNum >= 0) || (nMRUArrNum <= MAX_MRU)){
			for(int i = 0; i < nMRUArrNum; i++){
				std::wstring strPath = PluginShareData.m_lpShareData->m_sHistory.m_fiMRUArr[i].m_szPath;
				if(strPath.length() > 0){
					lstFilePath.push_back(strPath);
				}
			}
			bResult = true;
		}
		PluginShareData.Cleanup();
	}
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////
bool CPluginService::ExecuteCommand()
{
	if((m_strCommandPath.length() == 0) || (m_strFileName1.length() == 0) || (m_strFileName2.length() == 0)){
		return false;
	}
	wchar_t szCmdLine[32765];
	wsprintf(szCmdLine, L"%s \"%s\" \"%s\"", m_strCommandPath.c_str(), m_strFileName1.c_str(), m_strFileName2.c_str());
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
		return false;
	}
	::CloseHandle(pi.hThread);
	::CloseHandle(pi.hProcess);
	return true;
}
