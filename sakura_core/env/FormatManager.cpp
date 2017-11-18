#include "StdAfx.h"
#include "DllSharedData.h"

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
		pShareData->common.format.nDateFormatType,
		pShareData->common.format.szDateFormat
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
		pShareData->common.format.nTimeFormatType,
		pShareData->common.format.szTimeFormat
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

