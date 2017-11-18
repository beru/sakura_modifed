/*!	@file
	@brief Cookieマネージャ
*/
#include "StdAfx.h"
#include "CookieManager.h"


SysString CookieManager::GetCookie(LPCWSTR scope, LPCWSTR cookieName) const
{
	const std::map<wstring, wstring>* cookies = SelectCookieType(scope);
	if (!cookies) {
		return SysString(L"", 0);
	}
	wstring key = cookieName;
	auto keyVal = cookies->find(key);
	if (keyVal == cookies->end()) {
		return SysString(L"", 0);
	}
	return SysString(keyVal->second.c_str(), keyVal->second.length());
}

SysString CookieManager::GetCookieDefault(
	LPCWSTR scope,
	LPCWSTR cookieName,
	LPCWSTR defVal,
	size_t len
	) const
{
	auto cookies = SelectCookieType(scope);
	if (!cookies) {
		return SysString(L"", 0);
	}
	wstring key = cookieName;
	auto keyVal = cookies->find(key);
	if (keyVal == cookies->end()) {
		return SysString(defVal, len);
	}
	return SysString(keyVal->second.c_str(), keyVal->second.length());
}

int CookieManager::SetCookie(
	LPCWSTR scope,
	LPCWSTR cookieName,
	LPCWSTR val,
	int len
	)
{
	std::map<wstring, wstring>* cookies = SelectCookieType(scope);
	if (!cookies) {
		return 1;
	}
	if (!ValidateCookieName(cookieName)) {
		return 2;
	}
	(*cookies)[cookieName] = wstring(val, len);
	return 0;
}

int CookieManager::DeleteCookie(LPCWSTR scope, LPCWSTR cookieName)
{
	auto cookies = SelectCookieType(scope);
	if (!cookies) {
		return 1;
	}
	if (!ValidateCookieName(cookieName)) {
		return 2;
	}
	wstring key = cookieName;
	auto keyVal = cookies->find(key);
	if (keyVal == cookies->end()) {
		return 5;
	}
	cookies->erase(keyVal);
	return 0;
}

SysString CookieManager::GetCookieNames(LPCWSTR scope) const
{
	auto cookies = SelectCookieType(scope);
	if (!cookies) {
		return SysString(L"", 0);
	}
	auto it = cookies->begin();
	wstring keyNames;
	if (it != cookies->end()) {
		keyNames += it->first;
		++it;
	}
	for (; it!=cookies->end(); ++it) {
		keyNames += L",";
		keyNames += it->first;
	}
	return SysString(keyNames.c_str(), keyNames.length());
}

int CookieManager::DeleteAll(LPCWSTR scope)
{
	auto cookies = SelectCookieType(scope);
	if (!cookies) {
		return 1;
	}
	cookies->clear();
	return 0;
}

std::map<std::wstring, std::wstring>* CookieManager::SelectCookieType(LPCWSTR scope) const
{
	if (wcscmp(scope, L"window") == 0) {
		return const_cast<std::map<std::wstring, std::wstring>*>(&cookieWindow);
	}else if (wcscmp(scope, L"document") == 0) {
		return const_cast<std::map<std::wstring, std::wstring>*>(&cookieDocument);
	}
	return NULL;
}

bool CookieManager::ValidateCookieName(LPCWSTR cookieName) const
{
	for (int i=0; cookieName[i]!=L'\0'; ++i) {
		if (L'0' <= cookieName[i] && cookieName[i] <= L'9' ||
			L'a' <= cookieName[i] && cookieName[i] <= L'z' ||
			L'A' <= cookieName[i] && cookieName[i] <= L'Z' ||
			L'_' <= cookieName[i]
		) {
		}else {
			return false;
		}
	}
	return true;
}

