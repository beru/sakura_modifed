/*
	2008.05.18 kobake CShareData から分離
*/
/*
	Copyright (C) 2008, kobake

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
#include "DLLSHAREDATA.h"

#include "FormatManager.h"

/*! 日付をフォーマット
	systime：時刻データ
	
	pszDest：フォーマット済みテキスト格納用バッファ
	nDestLen：pszDestの長さ
	
	pszDateFormat：
		カスタムのときのフォーマット
*/
const TCHAR* FormatManager::MyGetDateFormat(const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen)
{
	return MyGetDateFormat(
		systime,
		pszDest,
		nDestLen,
		m_pShareData->m_common.m_format.m_nDateFormatType,
		m_pShareData->m_common.m_format.m_szDateFormat
	);
}

const TCHAR* FormatManager::MyGetDateFormat(
	const SYSTEMTIME&	systime,
	TCHAR*				pszDest,
	int					nDestLen,
	int					nDateFormatType,
	const TCHAR*		szDateFormat
	)
{
	const TCHAR* pszForm;
	DWORD dwFlags;
	if (nDateFormatType == 0) {
		dwFlags = DATE_LONGDATE;
		pszForm = NULL;
	}else {
		dwFlags = 0;
		pszForm = szDateFormat;
	}
	::GetDateFormat( SelectLang::getDefaultLangId(), dwFlags, &systime, pszForm, pszDest, nDestLen );
	return pszDest;
}


// 時刻をフォーマット
const TCHAR* FormatManager::MyGetTimeFormat(const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen)
{
	return MyGetTimeFormat(
		systime,
		pszDest,
		nDestLen,
		m_pShareData->m_common.m_format.m_nTimeFormatType,
		m_pShareData->m_common.m_format.m_szTimeFormat
	);
}

// 時刻をフォーマット
const TCHAR* FormatManager::MyGetTimeFormat(
	const SYSTEMTIME&	systime,
	TCHAR*				pszDest,
	int					nDestLen,
	int					nTimeFormatType,
	const TCHAR*		szTimeFormat
	)
{
	const TCHAR* pszForm;
	DWORD dwFlags;
	if (nTimeFormatType == 0) {
		dwFlags = 0;
		pszForm = NULL;
	}else {
		dwFlags = 0;
		pszForm = szTimeFormat;
	}
	::GetTimeFormat(SelectLang::getDefaultLangId(), dwFlags, &systime, pszForm, pszDest, nDestLen);
	return pszDest;
}

