
#include "StdAfx.h"
#include "BregexpDll2.h"

BregexpDll2::BregexpDll2()
{
}

BregexpDll2::~BregexpDll2()
{
}

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

