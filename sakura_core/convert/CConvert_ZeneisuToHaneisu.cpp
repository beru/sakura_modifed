#include "StdAfx.h"
#include "CConvert_ZeneisuToHaneisu.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �S�p�p�������p�p��
bool Converter_ZeneisuToHaneisu::DoConvert(NativeW* pData)
{
	// �S�p�p�������p�p��
	Convert_ZeneisuToHaneisu(pData->GetStringPtr(), pData->GetStringLength());

	return true;
}

