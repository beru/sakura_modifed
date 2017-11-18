#include "StdAfx.h"
#include "RecentCmd.h"
#include "config/maxdata.h"
#include "env/DllSharedData.h"
#include <string.h>


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

RecentCmd::RecentCmd()
{
	Create(
		GetShareData()->history.aCommands.dataPtr(),
		&GetShareData()->history.aCommands._GetSizeRef(),
		NULL,
		MAX_CMDARR,
		NULL
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      オーバーライド                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	アイテムの比較要素を取得する。

	@note	取得後のポインタはユーザ管理の構造体にキャストして参照してください。
*/
const TCHAR* RecentCmd::GetItemText(size_t nIndex) const
{
	return *GetItem(nIndex);
}

bool RecentCmd::DataToReceiveType(LPCTSTR* dst, const CmdString* src) const
{
	*dst = *src;
	return true;
}

bool RecentCmd::TextToDataType(CmdString* dst, LPCTSTR pszText) const
{
	CopyItem(dst, pszText);
	return true;
}

int RecentCmd::CompareItem(const CmdString* p1, LPCTSTR p2) const
{
	return _tcscmp(*p1, p2);
}

void RecentCmd::CopyItem(CmdString* dst, LPCTSTR src) const
{
	_tcscpy(*dst, src);
}

