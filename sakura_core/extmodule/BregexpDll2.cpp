
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
	DLL�̏�����

	�֐��̃A�h���X���擾���ă����o�ɕۊǂ���D

	@retval 0 ����
	@retval 1 �A�h���X�擾�Ɏ��s
*/
bool BregexpDll2::InitDllImp()
{
	// DLL���֐������X�g
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

