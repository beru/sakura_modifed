/*!	@file
	@brief OsVersionInfo
*/
/*
	Copyright (C) 2013, novice

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
#include "_os/OsVersionInfo.h"
#include "util/RegKey.h"
#include "util/window.h"

// OsVersionInfoの内部static変数の定義
// 初期化はIsValidVersion()で行う
BOOL	 		OsVersionInfo::m_bSuccess;
OSVERSIONINFO	OsVersionInfo::m_osVersionInfo;
#ifdef USE_SSE2
bool			COsVersionInfo::m_bSSE2;
#endif
bool			OsVersionInfo::m_bWine;

/*!
	初期化を行う(引数はダミー)
	呼出は基本1回のみ
*/
OsVersionInfo::OsVersionInfo(bool pbStart)
{
	memset_raw(&m_osVersionInfo, 0, sizeof(m_osVersionInfo));
	m_osVersionInfo.dwOSVersionInfoSize = sizeof(m_osVersionInfo);
	m_bSuccess = ::GetVersionEx(&m_osVersionInfo);

#ifdef USE_SSE2
 		int data[4];
#ifdef __MINGW32__
		__cpuid(1, data[0], data[1], data[2], data[3]);
#else
		__cpuid(data, 1);
#endif
		m_bSSE2 = (data[3] & (1<<26)) != 0;
#endif

	RegKey reg;
	m_bWine = (reg.Open(HKEY_CURRENT_USER, _T("Software\\Wine\\Debug"), KEY_READ) == ERROR_SUCCESS);
}

