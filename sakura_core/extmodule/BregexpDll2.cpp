
#include "StdAfx.h"
#include "BregexpDll2.h"

BregexpDll2::BregexpDll2()
{
}

BregexpDll2::~BregexpDll2()
{
}

/*!
	@date 2001.07.05 genta �����ǉ��B�������A�����ł͎g��Ȃ��B
	@date 2007.06.25 genta ������DLL���ɑΉ�
	@date 2007.09.13 genta �T�[�`���[����ύX
		@li �w��L��̏ꍇ�͂���݂̂�Ԃ�
		@li �w�薳��(NULL�܂��͋󕶎���)�̏ꍇ��BREGONIG, BREGEXP�̏��Ŏ��݂�
*/
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

