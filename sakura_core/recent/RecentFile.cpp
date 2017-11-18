#include "StdAfx.h"
#include "recent/RecentFile.h"
#include <string.h>
#include "env/DllSharedData.h"


/*
	アイテムの比較要素を取得する。

	@note	取得後のポインタはユーザ管理の構造体にキャストして参照してください。
*/
const TCHAR* RecentFile::GetItemText(size_t nIndex) const
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
	size_t n = GetItemCount();
	for (size_t i=0; i<n; ++i) {
		if (_tcsicmp(GetItem(i)->szPath, pszPath) == 0) {
			return (int)i;
		}
	}
	return -1;
}

