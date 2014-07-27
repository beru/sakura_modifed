/*!	@file
	@brief Speech Engine

	Speechを利用するためのインターフェース

	@author Plugins developers
	@date 2013.12.29
*/
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

#include "StdAfx.h"
#include "CPluginService.h"
#include "CSpeechEngine.h"

///////////////////////////////////////////////////////////////////////////////
CSpeechEngine::CSpeechEngine()
{
	m_pVoice = NULL;
	m_pToken = NULL;
	m_bReady = Initialize();
}

///////////////////////////////////////////////////////////////////////////////
CSpeechEngine::~CSpeechEngine()
{
	UnInitialize();
}

///////////////////////////////////////////////////////////////////////////////
bool CSpeechEngine::Initialize()
{
	::CoInitialize(NULL);

	HRESULT hr = ::CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pVoice));
	if(FAILED(hr)){
		m_strErrorMsg = L"Failed to get default voice token.";
		return false;
	}

	hr = m_pVoice->SetVoice(m_pToken);
	if(FAILED(hr)){
		m_strErrorMsg = L"Failed to set default voice.";
		m_pVoice->Release();
		return false;
	}

	m_bReady = TRUE;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
void CSpeechEngine::UnInitialize()
{
	if(m_bReady){
		if(m_pToken != NULL){
			m_pToken->Release();
		}
		if(m_pVoice != NULL){
			m_pVoice->Release();
		}
	}

	::CoUninitialize();

	m_strErrorMsg = L"";
	m_pVoice = NULL;
	m_pToken = NULL;
	m_bReady = FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// テキストを発声する
bool CSpeechEngine::Execute(LPCWSTR code, std::wstring& strVoice, const int nVolume, const int nRate)
{
	m_strErrorMsg = L"";

	if(m_bReady == FALSE){
		m_strErrorMsg = L"Speech engine not ready.";
		return false;
	}
	if((code == NULL) || (wcslen(code) == 0)){
		m_strErrorMsg = L"Empty text.";
		return false;
	}

	SetSpeechEngine(strVoice);
	m_pVoice->SetVolume(nVolume);
	m_pVoice->SetRate(nRate);

	ULONG streamNumber = 0;
	HRESULT hr = m_pVoice->Speak(code, SPF_IS_NOT_XML | SPF_ASYNC | SPF_PURGEBEFORESPEAK, &streamNumber);
	if(FAILED(hr)){
		m_strErrorMsg = L"Speak failed.";
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
bool CSpeechEngine::Purge()
{
	if(m_bReady == FALSE){
		m_strErrorMsg = L"Speech engine not ready.";
		return false;
	}
	HRESULT hr = m_pVoice->Speak(L"", SPF_PURGEBEFORESPEAK, NULL);
	if(FAILED(hr)){
		m_strErrorMsg = L"Purge failed.";
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// 終了を待つ
bool CSpeechEngine::Wait()
{
	if(m_bReady == FALSE){
		m_strErrorMsg = L"Speech engine not ready.";
		return false;
	}
	m_pVoice->WaitUntilDone(INFINITE);
	return true;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT SpGetCategoryFromId(
	LPCWSTR pszCategoryId,
	ISpObjectTokenCategory** ppCategory,
	BOOL fCreateIfNotExist = FALSE)
{
	ISpObjectTokenCategory* cpTokenCategory = NULL;
	HRESULT hr = ::CoCreateInstance(CLSID_SpObjectTokenCategory, NULL, CLSCTX_ALL, IID_ISpObjectTokenCategory, (LPVOID*)&cpTokenCategory);
	if(SUCCEEDED(hr)){
		hr = cpTokenCategory->SetId(pszCategoryId, fCreateIfNotExist);
		if(SUCCEEDED(hr)){
			*ppCategory = cpTokenCategory;
		}
	}

	return hr;
}

///////////////////////////////////////////////////////////////////////////////
HRESULT SpEnumTokens(
	LPCWSTR pszCategoryId,
	LPCWSTR pszReqAttribs,
	LPCWSTR pszOptAttribs,
	IEnumSpObjectTokens** ppEnum)
{
	ISpObjectTokenCategory* cpCategory = NULL;
	HRESULT hr = SpGetCategoryFromId(pszCategoryId, &cpCategory);
	if(SUCCEEDED(hr)){
		hr = cpCategory->EnumTokens(pszReqAttribs, pszOptAttribs, ppEnum);
	}

	return hr;
}

///////////////////////////////////////////////////////////////////////////////
//"Microsoft Haruka Desktop - Japanese"
bool CSpeechEngine::GetSpeechEngine(std::list<std::wstring>& lstTokens)
{
	if(m_bReady == FALSE){
		m_strErrorMsg = L"Speech engine not ready.";
		return false;
	}

//	LPCWSTR pattern = L"language = 411";	//日本語のみリストアップ
	LPCWSTR pattern = L"";
	IEnumSpObjectTokens* pEnum = NULL;
	HRESULT hr = SpEnumTokens(SPCAT_VOICES, pattern, NULL, &pEnum);
	if(FAILED(hr)){
		m_strErrorMsg = L"Failed to enum tokens.";
		return false;
	}

	ISpObjectToken* pToken = NULL;
	while(SUCCEEDED(pEnum->Next(1, &pToken, NULL))){
		if(pToken == NULL) break;
		LPWSTR lpszText = NULL;
		pToken->GetStringValue(L"", &lpszText);
		lstTokens.push_back(lpszText);
		pToken->Release();
		pToken = NULL;
	}

	pEnum->Release();
	return true;
}

///////////////////////////////////////////////////////////////////////////////
bool CSpeechEngine::SetSpeechEngine(std::wstring& strVoice)
{
	if(m_bReady == FALSE){
		m_strErrorMsg = L"Speech engine not ready.";
		return false;
	}

	wchar_t* pattern = L"";
	IEnumSpObjectTokens* pEnum = NULL;
	HRESULT hr = SpEnumTokens(SPCAT_VOICES, pattern, NULL, &pEnum);
	if(FAILED(hr)){
		m_strErrorMsg = L"Failed to enum tokens.";
		return false;
	}

	ISpObjectToken* pToken = NULL;
	while(SUCCEEDED(pEnum->Next(1, &pToken, NULL))){
		if(pToken == NULL) break;
		LPWSTR lpszText = NULL;
		pToken->GetStringValue(L"", &lpszText);
		if(strVoice.compare(lpszText) == 0){
			if(m_pToken != NULL){
				m_pToken->Release();
			}
			m_pToken = pToken;
			pEnum->Release();
			return true;
		}
		pToken->Release();
		pToken = NULL;
	}

	pEnum->Release();
	return false;
}
