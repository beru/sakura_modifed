#pragma once

// wchar_t‚É•ÏŠ·
const wchar_t* to_wchar(const char* src);
const wchar_t* to_wchar(const char* pSrcData, size_t nSrcLength);
inline
const wchar_t* to_wchar(const wchar_t* src) { return src; }

// char‚É•ÏŠ·
inline
const char* to_achar(const char* src) { return src; }
const char* to_achar(const wchar_t* src);
const char* to_achar(const wchar_t* pSrc, size_t nSrcLength);

// TCHAR‚É•ÏŠ·
#define to_tchar     to_wchar
#define to_not_tchar to_achar

// ‚»‚Ì‘¼
const wchar_t* easy_format(const wchar_t* format, ...);

