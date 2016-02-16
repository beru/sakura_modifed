#include "StdAfx.h"
#include "Convert_HankataToZenkata.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ���p�J�i���S�p�J�i
bool Converter_HankataToZenkata::DoConvert(NativeW* pData)
{
	// ���p�J�i���S�p�J�i
	// �����������邱�Ƃ͂����Ă������邱�Ƃ͖����̂ŁA����łn�j
	std::vector<wchar_t> buf(pData->GetStringLength() + 1);
	wchar_t* pBuf = &buf[0];
	int nDstLen = 0;
	Convert_HankataToZenkata(pData->GetStringPtr(), pData->GetStringLength(), pBuf, &nDstLen);
	pData->SetString(pBuf, nDstLen);

	return true;
}

