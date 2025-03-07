#pragma once

// wchar_tに変換
const wchar_t* to_wchar(const char* src);
const wchar_t* to_wchar(const char* pSrcData, size_t nSrcLength);
inline
const wchar_t* to_wchar(const wchar_t* src) { return src; }

// charに変換
inline
const char* to_achar(const char* src) { return src; }
const char* to_achar(const wchar_t* src);
const char* to_achar(const wchar_t* pSrc, size_t nSrcLength);

// TCHARに変換
#define to_tchar     to_wchar
#define to_not_tchar to_achar

// その他
const wchar_t* easy_format(const wchar_t* format, ...);

