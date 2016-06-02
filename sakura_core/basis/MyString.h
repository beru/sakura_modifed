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
#pragma once

#include <string>
#include "util/string_ex.h"

#define delete2(p) { if (p) { delete[] p; p = 0; } }

class String {
public:
	// コンストラクタ・デストラクタ
	String(wchar_t wc)								: wstr(1, wc),		str_cache(NULL) { }
	String(const wchar_t* szData = L"")				: wstr(szData),		str_cache(NULL) { }
	String(const wchar_t* pData, size_t nLength)		: wstr(pData, nLength), str_cache(NULL) { }
	String(const ACHAR* szData)						: wstr(L""), str_cache(NULL) { set(szData); }
	String(const ACHAR* pData, size_t nLength)		: wstr(L""), str_cache(NULL) { set(pData, nLength); }
	String(ACHAR wc)								: wstr(L""), str_cache(NULL) { ACHAR buf[2] = {wc, 0}; set(buf); }
	String(const String& rhs) : wstr(rhs.c_wstr()), str_cache(NULL) { }
	~String();

	// 演算子
	operator const wchar_t* () const { return c_wstr(); }
	operator const char* () const { return c_astr(); }
	String& operator = (const String& rhs) { set(rhs); return *this; }

	// 設定
	void set(const wchar_t* wszData) { wstr = wszData; delete2(str_cache); }
	void set(const wchar_t* wszData, int nLength) { wstr.assign(wszData, nLength); delete2(str_cache); }
	void set(const char* szData);
	void set(const char* szData, size_t nLength);
	void set(const String& szData) { set(szData.c_wstr()); }

	// 取得
	const wchar_t* c_wstr() const { return wstr.c_str(); }
	const char* c_astr() const;
	size_t wlength() const { return wcslen(c_wstr()); }
	size_t alength() const { return strlen(c_astr()); }


	// TCHAR
	const TCHAR* c_tstr() const { return c_wstr(); }

private:
	std::wstring wstr;
	mutable char* str_cache; // c_str用キャッシュ。wstrが変更(set)されたらこれを解放し、NULLにしておくのがルール。
};

// std::string の TCHAR 対応用マクロ定義
#define tstring wstring
#define astring string


// 共通マクロ
#define _FT _T
#include "util/StaticType.h"

// 共通型
typedef StaticString<TCHAR, _MAX_PATH> SFilePath;
class FilePath : public StaticString<TCHAR, _MAX_PATH> {
private:
	typedef StaticString<TCHAR, _MAX_PATH> Super;
public:
	FilePath() : Super() { }
	FilePath(const TCHAR* rhs) : Super(rhs) { }

	bool IsValidPath() const { return At(0) != _T('\0'); }
	std::tstring GetDirPath() const {
		TCHAR	szDirPath[_MAX_PATH];
		TCHAR	szDrive[_MAX_DRIVE];
		TCHAR	szDir[_MAX_DIR];
		_tsplitpath(this->c_str(), szDrive, szDir, NULL, NULL);
		_tcscpy(szDirPath, szDrive);
		_tcscat(szDirPath, szDir);
		return szDirPath;
	}
	// 拡張子を取得する
	LPCTSTR GetExt(bool bWithoutDot = false) const {
		const TCHAR* head = c_str();
		const TCHAR* p = auto_strchr(head, _T('\0')) - 1;
		while (p >= head) {
			if (*p == _T('.')) break;
			if (*p == _T('\\')) break;
			if (*p == _T('/')) break;
			--p;
		}
		if (p >= head && *p == _T('.')) {
			return bWithoutDot ? p + 1 : p;	// bWithoutDot == trueならドットなしを返す
		}else {
			return auto_strchr(head, _T('\0'));
		}
	}
};


//$$ 仮
class CommandLineString {
public:
	CommandLineString() {
		szCmdLine[0] = _T('\0');
		pHead = szCmdLine;
	}
	void AppendF(const TCHAR* szFormat, ...) {
		va_list v;
		va_start(v, szFormat);
		pHead += auto_vsprintf_s(pHead, _countof(szCmdLine) - (pHead - szCmdLine), szFormat, v);
		va_end(v);
	}
	const TCHAR* c_str() const {
		return szCmdLine;
	}
	size_t size() const {
		return pHead - szCmdLine;
	}
	size_t max_size() const {
		return _countof(szCmdLine) - 1;
	}
private:
	TCHAR	szCmdLine[1024];
	TCHAR*	pHead;
};

