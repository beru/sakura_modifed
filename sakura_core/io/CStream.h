/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#pragma once

class FileAttribute;

// ��O
class Error_FileOpen {};	// ��O�F�t�@�C���I�[�v���Ɏ��s
class Error_FileWrite {};	// ��O�F�t�@�C���������ݎ��s
class Error_FileRead {};	// ��O�F�t�@�C���ǂݍ��ݎ��s

// �X�g���[�����N���X
class Stream {
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	Stream(const TCHAR* tszPath, const TCHAR* tszMode, bool bExceptionMode = false);
//	Stream();
	virtual ~Stream();

	// ���Z�q
	operator bool() const { return Good(); }

	// �I�[�v���E�N���[�Y
	void Open(const TCHAR* tszPath, const TCHAR* tszMode);
	void Close();

	// ����
	void SeekSet(	// �V�[�N
		long offset	// �X�g���[���擪����̃I�t�Z�b�g 
	);
	void SeekEnd(	// �V�[�N
		long offset // �X�g���[���I�[����̃I�t�Z�b�g
	);

	// ���
	virtual bool Good() const { return m_fp && !Eof(); }
	bool Eof() const { return !m_fp || feof(m_fp); }

	// �t�@�C���n���h��
	FILE* GetFp() const { return m_fp; }

	// ���[�h
	bool IsExceptionMode() const { return m_bExceptionMode; }
private:
	FILE*			m_fp;
	FileAttribute*	m_pcFileAttribute;
	bool			m_bExceptionMode;
};


class OutputStream : public Stream {
public:
	OutputStream(const TCHAR* tszPath, const TCHAR* tszMode, bool bExceptionMode = false)
		:
		Stream(tszPath, tszMode, bExceptionMode)
	{
	}

	// �f�[�^�𖳕ϊ��ŏ������ށB�߂�l�͏������񂾃o�C�g���B
	int Write(const void* pBuffer, int nSizeInBytes) {
		int nRet = fwrite(pBuffer, 1, nSizeInBytes, GetFp());
		if (nRet != nSizeInBytes && IsExceptionMode()) {
			throw Error_FileWrite();
		}
		return nRet;
	}
};

