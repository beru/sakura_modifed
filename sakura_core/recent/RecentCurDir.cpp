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
		GetShareData()->history.aCurDirs.dataPtr(),
		&GetShareData()->history.aCurDirs._GetSizeRef(),
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
const TCHAR* RecentCurDir::GetItemText(size_t nIndex) const
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

