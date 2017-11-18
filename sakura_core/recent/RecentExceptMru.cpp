#include "stdafx.h"
#include "RecentExceptMRU.h"
#include <string.h>
#include "env/DllSharedData.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

RecentExceptMRU::RecentExceptMRU()
{
	auto& exceptMRU = GetShareData()->history.aExceptMRU;
	Create(
		exceptMRU.dataPtr(),
		&exceptMRU._GetSizeRef(),
		nullptr,
		MAX_MRU,
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
const TCHAR* RecentExceptMRU::GetItemText(size_t nIndex) const
{
	return *GetItem(nIndex);
}

bool RecentExceptMRU::DataToReceiveType(LPCTSTR* dst, const MetaPath* src) const
{
	*dst = *src;
	return true;
}

bool RecentExceptMRU::TextToDataType(MetaPath* dst, LPCTSTR pszText) const
{
	CopyItem(dst, pszText);
	return true;
}

int RecentExceptMRU::CompareItem(const MetaPath* p1, LPCTSTR p2) const
{
	return _tcsicmp(*p1, p2);
}

void RecentExceptMRU::CopyItem(MetaPath* dst, LPCTSTR src) const
{
	_tcscpy(*dst, src);
}

