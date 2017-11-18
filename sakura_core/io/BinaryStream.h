#pragma once

#include "Stream.h"

class BinaryInputStream : public Stream {
public:
	BinaryInputStream(LPCTSTR tszFilePath);

public:
	// �X�g���[���́u�c��v�T�C�Y���擾
	size_t GetLength();

	// �f�[�^�𖳕ϊ��œǂݍ��ށB�߂�l�͓ǂݍ��񂾃o�C�g���B
	size_t Read(void* pBuffer, size_t nSizeInBytes);
};

class BinaryOutputStream : public OutputStream {
public:
	BinaryOutputStream(LPCTSTR tszFilePath, bool bExceptionMode = false);
};

