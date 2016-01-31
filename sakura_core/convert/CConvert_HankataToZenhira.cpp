#include "StdAfx.h"
#include "CConvert_HankataToZenhira.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ���p�J�i���S�p�Ђ炪��
bool CConvert_HankataToZenhira::DoConvert(CNativeW* pcData)
{
	// ���p�J�i���S�p�Ђ炪��
	// �����������邱�Ƃ͂����Ă������邱�Ƃ͖����̂ŁA����łn�j
	std::vector<wchar_t> buf(pcData->GetStringLength() + 1);
	wchar_t* pBuf = &buf[0];
	int nDstLen = 0;
	Convert_HankataToZenhira(pcData->GetStringPtr(), pcData->GetStringLength(), pBuf, &nDstLen);
	pcData->SetString(pBuf, nDstLen);

	return true;
}

