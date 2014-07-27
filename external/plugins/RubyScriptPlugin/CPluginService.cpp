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

#include "stdafx.h"
#include "CPluginService.h"
#include "plugin/SakuraPlugin.h"
#include "CRubyEngine.h"
#include "resource.h"

///////////////////////////////////////////////////////////////////////////////
CPluginService::CPluginService()
{
	m_strRubyModulePath = L"";
	::InitializeCriticalSection(&m_CriticalSection);
}

///////////////////////////////////////////////////////////////////////////////
CPluginService::~CPluginService()
{
	::DeleteCriticalSection(&m_CriticalSection);
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::EnterCriticalSection()
{
	::EnterCriticalSection(&m_CriticalSection);
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::LeaveCriticalSection()
{
	::LeaveCriticalSection(&m_CriticalSection);
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::ReadProfile()
{
	// ê›íËÇì«Ç›çûÇ›Ç‹Ç∑
	WideString strPluginPath = Plugin.GetPluginDir();

	//RubyModulePath
	m_strRubyModulePath = Plugin.GetOption(GetPluginName(), RUBY_PROFILE_MODULE_PATH);
	if(m_strRubyModulePath.length() == 0){
		m_strRubyModulePath = strPluginPath + L"\\" + RUBY_DEFAULT_MODULE_NAME;
	}else{
		if(m_strRubyModulePath.substr(0, 2) == L".\\"){
			m_strRubyModulePath = strPluginPath + L"\\" + m_strRubyModulePath;
		}else if(m_strRubyModulePath.find(L"\\") == std::string::npos){
			m_strRubyModulePath = strPluginPath + L"\\" + m_strRubyModulePath;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::RunMacro(LPCWSTR Source)
{
	//ê›íËÇì«Ç›çûÇﬁ
	ReadProfile();

	CRubyEngine RubyEngine;
	if(RubyEngine.Load(m_strRubyModulePath.c_str()) == false){
		WideString strMessage;
		LoadString(IDS_STR_ERROR1, strMessage);
		::MessageBox(GetParentHwnd(), strMessage.c_str(), GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	AnsiString utf8str = to_astr(Source, CP_UTF8);
	if(RubyEngine.Execute(utf8str.c_str()) == false){
		WideString strMessage;
		LoadString(IDS_STR_ERROR2, strMessage);
		strMessage += RubyEngine.GetLastMessage();
		::MessageBox(GetParentHwnd(), strMessage.c_str(), GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
		return;
	}
}
