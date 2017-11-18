/*!	@file
	@brief Cookieマネージャ
*/
#pragma once

#include <map>
#include <string>
#include "_os/OleTypes.h"

class CookieManager {
	typedef std::wstring wstring;

public:
	SysString GetCookie(LPCWSTR scope, LPCWSTR cookieName) const;
	SysString GetCookieDefault(LPCWSTR scope, LPCWSTR cookieName, LPCWSTR defVal, size_t len) const;
	int SetCookie(LPCWSTR scope, LPCWSTR cookieName, LPCWSTR val, int len);
	int DeleteCookie(LPCWSTR scope, LPCWSTR cookieName);
	SysString GetCookieNames(LPCWSTR scope) const;
	int DeleteAll(LPCWSTR scope);

private:
	std::map<wstring, wstring>* SelectCookieType(LPCWSTR scope) const;
	bool ValidateCookieName(LPCWSTR cookieName) const;

	std::map<wstring, wstring> cookieWindow;
	std::map<wstring, wstring> cookieDocument;
};

