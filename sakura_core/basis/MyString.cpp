#include "StdAfx.h"
#include "MyString.h"
#include "charset/charcode.h"
#include "charset/CharPointer.h"

void String::set(const char* szData)
{
	wchar_t* wszData = mbstowcs_new(szData);
	set(wszData);
	delete[] wszData;
}

void String::set(const char* szData, size_t nLength)
{
	int nLen;
	wchar_t* wszData = mbstowcs_new(szData, nLength, &nLen);
	set(wszData, nLen);
	delete[] wszData;
}

const char* String::c_astr() const
{
	if (!str_cache) {
		str_cache = wcstombs_new(wstr.c_str());
	}
	return str_cache;
}

String::~String()
{
	delete2(str_cache);
}

/*
CFilePath::CFilePath(const char* rhs)
{
	// 現時点ではNULLを受け付けない
	assert(rhs);
	_mbstotcs(tszPath, _countof(tszPath), rhs);
}
CFilePath::CFilePath(const wchar_t* rhs)
{
	// 現時点ではNULLを受け付けない
	assert(rhs);
	_wcstotcs(tszPath, _countof(tszPath), rhs);
}
*/

