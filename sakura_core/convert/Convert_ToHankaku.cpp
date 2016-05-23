#include "StdAfx.h"
#include "Convert_ToHankaku.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ���p�ɂł�����̂͑S�����p�ɕϊ�
bool Converter_ToHankaku::DoConvert(NativeW* pData)
{
	// �S�p�����p
	// ���_���̉e���ŁA�ő�2�{�ɂ܂Ŗc��オ��\��������̂ŁA2�{�̃o�b�t�@���m��
	std::vector<wchar_t> buf(pData->GetStringLength() * 2 + 1);
	wchar_t* pBuf = &buf[0];
	size_t nDstLen = 0;
	Convert_ToHankaku(pData->GetStringPtr(), pData->GetStringLength(), pBuf, &nDstLen);
	pData->SetString(pBuf, nDstLen);

	return true;
}

