#include "StdAfx.h"
#include "CConvert_ZenkataToHankata.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �S�p�J�i�����p�J�i
bool Converter_ZenkataToHankata::DoConvert(NativeW* pcData)
{
	// �S�p�J�^�J�i�����p�J�^�J�i
	// ���_���̉e���ŁA�ő�2�{�ɂ܂Ŗc��オ��\��������̂ŁA2�{�̃o�b�t�@���m��
	std::vector<wchar_t> buf(pcData->GetStringLength() * 2 + 1);
	wchar_t* pBuf = &buf[0];
	int nBufLen = 0;
	Convert_ZenkataToHankata(pcData->GetStringPtr(), pcData->GetStringLength(), pBuf, &nBufLen);
	pcData->SetString(pBuf, nBufLen);

	return true;
}

