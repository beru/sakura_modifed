#include "StdAfx.h"
#include "Convert_ZenkataToHankata.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �S�p�J�i�����p�J�i
bool Converter_ZenkataToHankata::DoConvert(NativeW* pData)
{
	// �S�p�J�^�J�i�����p�J�^�J�i
	// ���_���̉e���ŁA�ő�2�{�ɂ܂Ŗc��オ��\��������̂ŁA2�{�̃o�b�t�@���m��
	std::vector<wchar_t> buf(pData->GetStringLength() * 2 + 1);
	wchar_t* pBuf = &buf[0];
	size_t nBufLen = 0;
	Convert_ZenkataToHankata(pData->GetStringPtr(), pData->GetStringLength(), pBuf, &nBufLen);
	pData->SetString(pBuf, nBufLen);

	return true;
}

