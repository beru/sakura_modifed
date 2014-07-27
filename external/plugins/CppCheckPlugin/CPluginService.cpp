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

#include "stdafx.h"
#include <windows.h>
#include "CPluginService.h"
#include "plugin/SakuraPlugin.h"
#include "CCppCheckDialog.h"

///////////////////////////////////////////////////////////////////////////////
CPluginService::CPluginService()
{
	m_lpCppCheckDialog = new CCppCheckDialog;
}

///////////////////////////////////////////////////////////////////////////////
CPluginService::~CPluginService()
{
	if (m_lpCppCheckDialog != NULL){
		delete m_lpCppCheckDialog;
		m_lpCppCheckDialog = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::OnPluginCppCheckDialog(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	if (m_lpCppCheckDialog == NULL){
		return;
	}
	//Ý’è‚ð“Ç‚Ýž‚Þ
	thePluginService.ReadProfile();
	if (::IsWindow(m_lpCppCheckDialog->m_hWnd) != FALSE){
		if (::IsWindowVisible(m_lpCppCheckDialog->m_hWnd) != FALSE){
			return;
		}
	}
	m_strTargetName = Editor.GetFilename();
//	m_lpCppCheckDialog->DoModeless(GetInstance(), GetParentHwnd());
	m_lpCppCheckDialog->DoModal(GetInstance(), GetParentHwnd());
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::ReadProfile()
{
	WideString strPluginPath = Plugin.GetPluginDir();

	//cppcheck.exeƒpƒX
	m_strCppCheckExePath = Plugin.GetOption(GetPluginName(), PROFILE_KEY_CPPCHECK_EXE_PATH);
	if(m_strCppCheckExePath.length() == 0){
		m_strCppCheckExePath = strPluginPath + L"\\" + PROFILE_DEF_CPPCHECK_EXE_PATH;
	}else{
		if(m_strCppCheckExePath.substr(0, 2) == L".\\"){
			m_strCppCheckExePath = strPluginPath + L"\\" + m_strCppCheckExePath;
		}else if(m_strCppCheckExePath.find(L"\\") == std::string::npos){
			m_strCppCheckExePath = strPluginPath + L"\\" + m_strCppCheckExePath;
		}
	}
	//platform
	m_strPlatform = Plugin.GetOption(GetPluginName(), PROFILE_KEY_PLATFORM);
	if(m_strPlatform.length() == 0){
		m_strPlatform = PROFILE_DEF_PLATFORM;
	}
	m_strResultFile = strPluginPath + L"\\cppcheck_result.xml";
}
