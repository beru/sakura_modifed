#include "StdAfx.h"
#include "Convert_HaneisuToZeneisu.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ���p�p�����S�p�p��
bool Converter_HaneisuToZeneisu::DoConvert(NativeW* pData)
{
	// ���p�p�����S�p�p��
	Convert_HaneisuToZeneisu(pData->GetStringPtr(), pData->GetStringLength());

	return true;
}

