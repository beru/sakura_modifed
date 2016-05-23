#include "StdAfx.h"
#include "Convert_HankataToZenhira.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ���p�J�i���S�p�Ђ炪��
bool Converter_HankataToZenhira::DoConvert(NativeW* pData)
{
	// ���p�J�i���S�p�Ђ炪��
	// �����������邱�Ƃ͂����Ă������邱�Ƃ͖����̂ŁA����łn�j
	std::vector<wchar_t> buf(pData->GetStringLength() + 1);
	wchar_t* pBuf = &buf[0];
	size_t nDstLen = 0;
	Convert_HankataToZenhira(pData->GetStringPtr(), pData->GetStringLength(), pBuf, &nDstLen);
	pData->SetString(pBuf, nDstLen);

	return true;
}

