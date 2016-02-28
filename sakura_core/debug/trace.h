#pragma once

class DbgStr
{
public:
	void operator() (const char* pszFormat, ...);
	void operator() (const wchar_t* pszFormat, ...);
};

#define TRACE     DbgStr()

