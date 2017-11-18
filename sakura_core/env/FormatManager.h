#pragma once

// 要先行定義
// #include "DllSharedData.h"

// 書式管理
class FormatManager {
public:
	FormatManager() {
		pShareData = &GetDllShareData();
	}
	// 書式
	// 共有DllSharedData依存
	const TCHAR* MyGetDateFormat(const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen);
	const TCHAR* MyGetTimeFormat(const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen);

	// 共有DllSharedData非依存
	const TCHAR* MyGetDateFormat(const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen, int nDateFormatType, const TCHAR* szDateFormat);
	const TCHAR* MyGetTimeFormat(const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen, int nTimeFormatType, const TCHAR* szTimeFormat);
private:
	DllSharedData* pShareData;
};

