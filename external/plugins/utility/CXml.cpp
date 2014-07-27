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
#include "CXml.h"
#include <xmllite.h>
#include <Shlwapi.h>
#include <list>
#include <string>
#include <map>

#pragma comment(lib, "xmllite.lib")	//IXMLReader/IXMLWriter
#pragma comment(lib, "Shlwapi.lib")	//SHCreateStreamOnFile

///////////////////////////////////////////////////////////////////////////////
CXml::CXml()
{
	m_bWrite = FALSE;
	m_pReader = NULL;
	m_pWriter = NULL;
	m_pStream = NULL;
	::CoInitialize(NULL);
}

///////////////////////////////////////////////////////////////////////////////
CXml::~CXml()
{
	Close();
	::CoUninitialize();
}

///////////////////////////////////////////////////////////////////////////////
bool CXml::Open(LPCWSTR lpszFileName, BOOL bWrite)
{
	if((m_pReader != NULL) || (m_pWriter != NULL)){
		return false;
	}

	HRESULT hr;
	m_bWrite = bWrite;
	if(m_bWrite){
		hr = ::CreateXmlWriter(__uuidof(IXmlWriter), reinterpret_cast<void**>(&m_pWriter), 0);
		if(SUCCEEDED(hr)){
			hr = ::SHCreateStreamOnFile(lpszFileName, STGM_CREATE | STGM_WRITE, &m_pStream);
			if(SUCCEEDED(hr)){
				hr = m_pWriter->SetOutput(m_pStream);
				if(SUCCEEDED(hr)){
					//インデントを有効化する
					hr = m_pWriter->SetProperty(XmlWriterProperty_Indent, TRUE);
					if(SUCCEEDED(hr)){
						//<?xml version="1.0" encoding="UTF-8"?>
						hr = m_pWriter->WriteStartDocument(XmlStandalone_Omit);
						if(SUCCEEDED(hr)){
							return true;
						}
					}
				}
			}
		}
	}else{
		hr = ::CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(&m_pReader), 0);
		if(SUCCEEDED(hr)){
			hr = ::SHCreateStreamOnFile(lpszFileName, STGM_READ, &m_pStream);
			if(SUCCEEDED(hr)){
				hr = m_pReader->SetInput(m_pStream);
				if(SUCCEEDED(hr)){
					return true;
				}
			}
		}
	}
	Close();
	return false;
}

///////////////////////////////////////////////////////////////////////////////
void CXml::Close()
{
	if(m_pWriter != NULL){
		m_pWriter->WriteEndDocument();
		m_pWriter->Flush();
	}

	SAFE_RELEASE(m_pStream);
	SAFE_RELEASE(m_pReader);
	SAFE_RELEASE(m_pWriter);
	m_bWrite = FALSE;
	m_Tree.clear();
}

///////////////////////////////////////////////////////////////////////////////
bool CXml::WriteStartElement(LPCWSTR element)
{
	if(m_pWriter != NULL){
		HRESULT hr = m_pWriter->WriteStartElement(NULL, element, NULL);
		if(SUCCEEDED(hr)){
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
bool CXml::WriteEndElement()
{
	if(m_pWriter != NULL){
		HRESULT hr = m_pWriter->WriteFullEndElement();
		if(SUCCEEDED(hr)){
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
bool CXml::WriteElementString(LPCWSTR key, LPCWSTR value)
{
	if(m_pWriter != NULL){
		HRESULT hr = m_pWriter->WriteElementString(NULL, key, NULL, value);
		if(SUCCEEDED(hr)){
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
bool CXml::ReadXml(LPCWSTR lpszFileName)
{
	bool bResult = false;
	m_Tree.clear();
	if(Open(lpszFileName, FALSE) == false){
		Close();
		return false;
	}

	LPCWSTR lpszLocalName;
	LPCWSTR lpszValue;
	XmlNodeType nodeType;
	while(m_pReader->Read(&nodeType) == S_OK){
		switch(nodeType){
		case XmlNodeType_Element:			//要素の開始
			{
				if(FAILED(m_pReader->GetLocalName(&lpszLocalName, NULL))){
					goto EXIT_HANDLER;
				}
				m_Tree.push_back(lpszLocalName);
				OnXmlNodeType_Element(lpszLocalName);
				std::map<std::wstring, std::wstring> mapAttribute;
				HRESULT hr = m_pReader->MoveToFirstAttribute();
				while(hr == S_OK){
					LPCWSTR lpszAttributeName;
					if (FAILED(m_pReader->GetLocalName(&lpszAttributeName, NULL))){
						goto EXIT_HANDLER;
					}
					if (FAILED(m_pReader->GetValue(&lpszValue, NULL))){
						goto EXIT_HANDLER;
					}
					mapAttribute.insert(std::pair<std::wstring, std::wstring>(lpszAttributeName, lpszValue));
					OnXmlNodeType_Element_Attribute(lpszAttributeName, lpszValue);
					hr = m_pReader->MoveToNextAttribute();
				}
				OnXmlNodeType_Element_Attributes(lpszLocalName, mapAttribute);
				mapAttribute.clear();
			}
			break;
		case XmlNodeType_Text:				//テキスト
			if (FAILED(m_pReader->GetValue(&lpszValue, NULL))){
				goto EXIT_HANDLER;
			}
			OnXmlNodeType_Text(lpszLocalName, lpszValue);
			break;
		case XmlNodeType_CDATA:				//CDATAセクション
			{
				if(FAILED(m_pReader->GetValue(&lpszValue, NULL))){
					goto EXIT_HANDLER;
				}
				std::wstring s = lpszValue;
				ReplaceCRLF(s);
				OnXmlNodeType_CDATA(lpszLocalName, s.c_str());
			}
			break;
		case XmlNodeType_EndElement:		//要素の終了
			if (FAILED(m_pReader->GetLocalName(&lpszLocalName, NULL))){
				goto EXIT_HANDLER;
			}
			m_Tree.pop_back();
			OnXmlNodeType_EndElement(lpszLocalName);
			break;
		case XmlNodeType_XmlDeclaration:	//XML宣言
		case XmlNodeType_Whitespace:		//空白
		case XmlNodeType_None:				//なし
		case XmlNodeType_Attribute:			//属性
		case XmlNodeType_ProcessingInstruction:	//XMLプロセッサ処理命令
		case XmlNodeType_Comment:			//コメント
		case XmlNodeType_DocumentType:		//ドキュメントの型宣言
		default:
			break;
		}
	}
	bResult = true;

EXIT_HANDLER:;
	Close();
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////
std::wstring CXml::GetTree(const int nCount)
{
	std::wstring strTree = L"";
	int n = nCount;
	if(nCount == 0){
		n = INT_MAX;
	}
	for(std::list<std::wstring>::reverse_iterator it = m_Tree.rbegin(); it != m_Tree.rend(); ++it){
		if(n <= 0) break;
		n--;
		strTree = L"/" + *it + strTree;
	}
	return strTree;
}

///////////////////////////////////////////////////////////////////////////////
void CXml::ReplaceCRLF(std::wstring& s)
{
	std::wstring t = L"";
	for(DWORD i = 0; i < s.length(); i++){
		wchar_t ch = s.at(i);
		if(ch == L'\n'){
			t += L"\r\n";
		}else{
			t += ch;
		}
	}
	s = t;
}
