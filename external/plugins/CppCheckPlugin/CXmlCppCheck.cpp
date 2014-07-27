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

#include "StdAfx.h"
#include "CXmlCppCheck.h"
#include "CCppCheckDialog.h"

/*
<?xml version="1.0" encoding="UTF-8"?>
<results>
    <error file="CSpeechEngine.cpp" line="117" id="redundantAssignment" severity="performance" msg="Variable &amp;#039;hr&amp;#039; is reassigned a value before the old one has been used."/>
</results>
*/

///////////////////////////////////////////////////////////////////////////////
CXmlCppCheck::CXmlCppCheck()
{
	m_lpCppCheckDialog = NULL;
}

///////////////////////////////////////////////////////////////////////////////
CXmlCppCheck::~CXmlCppCheck()
{
}

///////////////////////////////////////////////////////////////////////////////
void CXmlCppCheck::OnXmlNodeType_Element_Attributes(LPCWSTR lpszLocalName, std::map<std::wstring, std::wstring>& mapAttribute)
{
	if (mapAttribute.size() > 0){
		CCppCheckData info;
		info.m_strFile = mapAttribute[L"file"];
		info.m_strLine = mapAttribute[L"line"];
		info.m_strIdentifier = mapAttribute[L"id"];
		info.m_strSeverity = mapAttribute[L"severity"];
		info.m_strMessage = mapAttribute[L"msg"];
		if(m_lpCppCheckDialog != NULL){
			m_lpCppCheckDialog->InsertItem(&info);
		}
	}
}
