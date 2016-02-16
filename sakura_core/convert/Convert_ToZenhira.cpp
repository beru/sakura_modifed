#include "StdAfx.h"
#include "Convert_ToZenhira.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �ł������S�p�Ђ炪�Ȃɂ���
bool Converter_ToZenhira::DoConvert(NativeW* pData)
{
	// ���J�i���S�p�J�i
	// �����������邱�Ƃ͂����Ă������邱�Ƃ͖����̂ŁA����łn�j
	std::vector<wchar_t> buf(pData->GetStringLength() + 1);
	wchar_t* pBuf = &buf[0];
	int nBufLen = 0;
	Convert_HankataToZenkata(pData->GetStringPtr(), pData->GetStringLength(), pBuf, &nBufLen);

	// �S�J�i���S�p�Ђ炪��
	Convert_ZenkataToZenhira(pBuf, nBufLen);

	// ���p�p�����S�p�p��
	Convert_HaneisuToZeneisu(pBuf, nBufLen);

	// �ݒ�
	pData->SetString(pBuf, nBufLen);

	return true;
}

