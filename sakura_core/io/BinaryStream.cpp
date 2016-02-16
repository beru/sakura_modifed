#include "StdAfx.h"
#include "BinaryStream.h"


BinaryInputStream::BinaryInputStream(LPCTSTR tszFilePath)
	:
	Stream(tszFilePath, _T("rb"))
{
}

// �X�g���[���́u�c��v�T�C�Y���擾
int BinaryInputStream::GetLength()
{
	long nCur = ftell(GetFp());
	fseek(GetFp(), 0, SEEK_END);
	long nDataLen = ftell(GetFp());
	fseek(GetFp(), nCur, SEEK_SET);
	return nDataLen;
}

// �f�[�^�𖳕ϊ��œǂݍ��ށB�߂�l�͓ǂݍ��񂾃o�C�g���B
int BinaryInputStream::Read(void* pBuffer, int nSizeInBytes)
{
	return fread(pBuffer, 1, nSizeInBytes, GetFp());
}

BinaryOutputStream::BinaryOutputStream(LPCTSTR tszFilePath, bool bExceptionMode)
	:
	OutputStream(tszFilePath, _T("wb"), bExceptionMode)
{
}

