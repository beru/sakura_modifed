
#include "StdAfx.h"
#include "BregexpDll2.h"

BregexpDll2::BregexpDll2()
{
}

BregexpDll2::~BregexpDll2()
{
}

/*!
	@date 2001.07.05 genta 引数追加。ただし、ここでは使わない。
	@date 2007.06.25 genta 複数のDLL名に対応
	@date 2007.09.13 genta サーチルールを変更
		@li 指定有りの場合はそれのみを返す
		@li 指定無し(NULLまたは空文字列)の場合はBREGONIG, BREGEXPの順で試みる
*/
LPCTSTR BregexpDll2::GetDllNameImp(int index)
{
	return _T("bregonig.dll");
}


/*!
	DLLの初期化

	関数のアドレスを取得してメンバに保管する．

	@retval 0 成功
	@retval 1 アドレス取得に失敗
*/
bool BregexpDll2::InitDllImp()
{
	// DLL内関数名リスト
	const ImportTable table[] = {
		{ &pBMatch,				"BMatchW" },
		{ &pBSubst,				"BSubstW" },
		{ &pBTrans,				"BTransW" },
		{ &pBSplit,				"BSplitW" },
		{ &pBRegfree,			"BRegfreeW" },
		{ &pBRegexpVersion,		"BRegexpVersionW" },
		{ &pBMatchEx,			"BMatchExW" },
		{ &pBSubstEx,			"BSubstExW" },
		{ NULL, 0 }
	};
	
	if (!RegisterEntries(table)) {
		return false;
	}
	
	return true;
}

