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
#include <windows.h>
#include <time.h>
#include "CPluginService.h"
#include "plugin/SakuraPlugin.h"
#include "CSpeechDialog.h"

///////////////////////////////////////////////////////////////////////////////
CPluginService::CPluginService()
{
	m_strVoice = L"";
	m_nVolume = 100;
	m_nRate = 0;
}

///////////////////////////////////////////////////////////////////////////////
CPluginService::~CPluginService()
{
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::OnPluginSpeech(SAKURA_DLL_PLUGIN_OBJ* obj, const int nMode)
{
	ReadProfile();

	switch(nMode){
	case 0:	//Dialog
		{
			CSpeechDialog dlg;
			dlg.DoModal(GetInstance(), GetParentHwnd());
		}
		break;
	case 1:	//Word
		{
			Editor.SelectWord();
			WideString strText = Editor.GetSelectedString(0);
			m_SpeechEngine.Execute(strText.c_str(), m_strVoice, m_nVolume, m_nRate);
		}
		break;
	case 2:	//Line
		{
			int nLine = _wtoi(Editor.ExpandParameter(L"$y").c_str());
			WideString strText = Editor.GetLineStr(nLine);
			m_SpeechEngine.Execute(strText.c_str(), m_strVoice, m_nVolume, m_nRate);
		}
		break;
	case 3:	//All
		{
			WideString strText = L"";
			int nLineCount = Editor.GetLineCount(0);
			for(int i = 1; i <= nLineCount; i++){
				strText += Editor.GetLineStr(i);
			}
			m_SpeechEngine.Execute(strText.c_str(), m_strVoice, m_nVolume, m_nRate);
		}
		break;
	case 4:	//Select
		{
			WideString strText = Editor.GetSelectedString(0);
			m_SpeechEngine.Execute(strText.c_str(), m_strVoice, m_nVolume, m_nRate);
		}
		break;
	case 5:	//Purge
		{
			m_SpeechEngine.Purge();
		}
		break;
	default:
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::ReadProfile()
{
	m_nVolume = Plugin.GetOptionInt(GetPluginName(), L"Volume");
	if(m_nVolume < 0) m_nVolume = 0;
	if(m_nVolume > 100) m_nVolume = 100;
	m_nRate = Plugin.GetOptionInt(GetPluginName(), L"Rate");
	if(m_nRate < -10) m_nRate = -10;
	if(m_nRate > 10) m_nRate = 10;
	m_strVoice = Plugin.GetOption(GetPluginName(), L"Voice");
	if(m_strVoice.length() == 0){
		m_strVoice = L"Microsoft Haruka Desktop - Japanese";
		m_nVolume = 100;
		m_nRate = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::WriteProfile()
{
	Plugin.SetOption(GetPluginName(), L"Voice", m_strVoice);
	Plugin.SetOption(GetPluginName(), L"Volume", m_nVolume);
	Plugin.SetOption(GetPluginName(), L"Rate", m_nRate);
}
