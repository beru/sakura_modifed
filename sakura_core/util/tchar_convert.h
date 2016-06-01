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

// wchar_t‚É•ÏŠ·
const wchar_t* to_wchar(const ACHAR* src);
const wchar_t* to_wchar(const ACHAR* pSrcData, size_t nSrcLength);
inline
const wchar_t* to_wchar(const wchar_t* src) { return src; }

// ACHAR‚É•ÏŠ·
inline
const ACHAR* to_achar(const ACHAR* src) { return src; }
const ACHAR* to_achar(const wchar_t* src);
const ACHAR* to_achar(const wchar_t* pSrc, size_t nSrcLength);

// TCHAR‚É•ÏŠ·
#ifdef _UNICODE
	#define to_tchar     to_wchar
	#define to_not_tchar to_achar
#else
	#define to_tchar     to_achar
	#define to_not_tchar to_wchar
#endif

// ‚»‚Ì‘¼
const wchar_t* easy_format(const wchar_t* format, ...);

