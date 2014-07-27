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

#ifndef CXML_CPPCHECK_H
#define CXML_CPPCHECK_H

#include "CXml.h"

class CCppCheckDialog;

///////////////////////////////////////////////////////////////////////////////
class CXmlCppCheck : public CXml
{
public:
	CXmlCppCheck();
	virtual ~CXmlCppCheck();

public:
	CCppCheckDialog*	m_lpCppCheckDialog;

public:
	virtual void OnXmlNodeType_Element_Attributes(LPCWSTR lpszLocalName, std::map<std::wstring, std::wstring>& mapAttribute);
};

#endif	//CXML_CPPCHECK_H
