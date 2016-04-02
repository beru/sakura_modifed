/*
	Copyright (C) 2008, kobake
	Copyright (C) 2013, Moca

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
#include "RecentCurDir.h"
#include "config/maxdata.h"
#include "env/DllSharedData.h"
#include <string.h>


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

RecentCurDir::RecentCurDir()
{
	Create(
		GetShareData()->history.m_aCurDirs.dataPtr(),
		&GetShareData()->history.m_aCurDirs._GetSizeRef(),
		nullptr,
		MAX_CMDARR,
		nullptr
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      オーバーライド                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	アイテムの比較要素を取得する。

	@note	取得後のポインタはユーザ管理の構造体にキャストして参照してください。
*/
const TCHAR* RecentCurDir::GetItemText(int nIndex) const
{
	return *GetItem(nIndex);
}

bool RecentCurDir::DataToReceiveType(LPCTSTR* dst, const CurDirString* src) const
{
	*dst = *src;
	return true;
}

bool RecentCurDir::TextToDataType(CurDirString* dst, LPCTSTR pszText) const
{
	CopyItem(dst, pszText);
	return true;
}

int RecentCurDir::CompareItem(const CurDirString* p1, LPCTSTR p2) const
{
	return _tcscmp(*p1, p2);
}

void RecentCurDir::CopyItem(CurDirString* dst, LPCTSTR src) const
{
	_tcscpy(*dst, src);
}

