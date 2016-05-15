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
#include "recent/RecentFile.h"
#include <string.h>
#include "env/DllSharedData.h"


/*
	アイテムの比較要素を取得する。

	@note	取得後のポインタはユーザ管理の構造体にキャストして参照してください。
*/
const TCHAR* RecentFile::GetItemText(int nIndex) const
{
	return GetItem(nIndex)->szPath;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

RecentFile::RecentFile()
{
	Create(
		GetShareData()->history.fiMRUArr,
		&GetShareData()->history.nMRUArrNum,
		GetShareData()->history.bMRUArrFavorite,
		MAX_MRU,
		&(GetShareData()->common.general.nMRUArrNum_MAX)
	);
}

bool RecentFile::DataToReceiveType(const EditInfo** dst, const EditInfo* src) const
{
	*dst = src;
	return true;
}

bool RecentFile::TextToDataType(EditInfo* dst, LPCTSTR pszText) const
{
	if (_countof(dst->szPath) < auto_strlen(pszText) + 1) {
		return false;
	}
	_tcscpy_s(dst->szPath, pszText);
	return true;
}

int RecentFile::CompareItem(const EditInfo* p1, const EditInfo* p2) const
{
	return _tcsicmp(p1->szPath, p2->szPath);
}

void RecentFile::CopyItem(EditInfo* dst, const EditInfo* src) const
{
	memcpy_raw(dst, src, sizeof(*dst));
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   固有インターフェース                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

int RecentFile::FindItemByPath(const TCHAR* pszPath) const
{
	int n = GetItemCount();
	for (int i=0; i<n; ++i) {
		if (_tcsicmp(GetItem(i)->szPath, pszPath) == 0) {
			return i;
		}
	}
	return -1;
}

