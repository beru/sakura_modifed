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

#ifndef CXML_H
#define CXML_H

#include <windows.h>
#include <ole2.h>
#include <xmllite.h>
#include <shlwapi.h>
#include <string>
#include <map>
#include <list>

///////////////////////////////////////////////////////////////////////////////
#define SAFE_RELEASE(ptr) \
	if(ptr != NULL){ \
		ptr->Release(); \
		ptr = NULL; \
	}

///////////////////////////////////////////////////////////////////////////////
class CXml
{
public:
	CXml();
	virtual ~CXml();

public:
	BOOL		m_bWrite;
	IXmlReader*	m_pReader;
	IXmlWriter*	m_pWriter;
	IStream*	m_pStream;
	std::list<std::wstring>	m_Tree;

public:
	bool Open(LPCWSTR lpszFileName, BOOL bWrite = FALSE);
	void Close();
	bool WriteStartElement(LPCWSTR key);
	bool WriteEndElement();
	bool WriteElementString(LPCWSTR key, LPCWSTR value);
	bool ReadXml(LPCWSTR lpszFileName);
	std::wstring GetTree(const int nCount = 0);

	virtual void OnXmlNodeType_Element(LPCWSTR lpszLocalName){};
	virtual void OnXmlNodeType_Element_Attribute(LPCWSTR lpszLocalName, LPCWSTR lpszValue){};
	virtual void OnXmlNodeType_Element_Attributes(LPCWSTR lpszLocalName, std::map<std::wstring, std::wstring>& mapAttribute){};
	virtual void OnXmlNodeType_Text(LPCWSTR lpszLocalName, LPCWSTR lpszValue){};
	virtual void OnXmlNodeType_CDATA(LPCWSTR lpszLocalName, LPCWSTR lpszValue){};
	virtual void OnXmlNodeType_EndElement(LPCWSTR lpszLocalName){};

	void ReplaceCRLF(std::wstring& s);
};

#endif	//CXML_H
