#include "StdAfx.h"
#include "RecentFolder.h"
#include <string.h>
#include "env/DllSharedData.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

RecentFolder::RecentFolder()
{
	Create(
		&GetShareData()->history.szOPENFOLDERArr[0],
		&GetShareData()->history.nOPENFOLDERArrNum,
		GetShareData()->history.bOPENFOLDERArrFavorite,
		MAX_OPENFOLDER,
		&(GetShareData()->common.general.nOPENFOLDERArrNum_MAX)
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      オーバーライド                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	アイテムの比較要素を取得する。

	@note	取得後のポインタはユーザ管理の構造体にキャストして参照してください。
*/
const TCHAR* RecentFolder::GetItemText(size_t nIndex) const
{
	return *GetItem(nIndex);
}

bool RecentFolder::DataToReceiveType(LPCTSTR* dst, const PathString* src) const
{
	*dst = *src;
	return true;
}

bool RecentFolder::TextToDataType(PathString* dst, LPCTSTR pszText) const
{
	CopyItem(dst, pszText);
	return true;
}

int RecentFolder::CompareItem(const PathString* p1, LPCTSTR p2) const
{
	return _tcsicmp(*p1, p2);
}

void RecentFolder::CopyItem(PathString* dst, LPCTSTR src) const
{
	_tcscpy(*dst, src);
}

