#pragma once

bool GetDateTimeFormat(TCHAR* szResult, int size, const TCHAR* format, const SYSTEMTIME& systime);
UINT32 ParseVersion(const TCHAR* ver);	// バージョン番号の解析
int CompareVersion(const TCHAR* verA, const TCHAR* verB);	// バージョン番号の比較

