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
	virtual bool Good() const { return fp && !Eof(); }
	bool Eof() const { return !fp || feof(fp); }

	// �t�@�C���n���h��
	FILE* GetFp() const { return fp; }

	// ���[�h
	bool IsExceptionMode() const { return bExceptionMode; }
private:
	FILE*			fp;
	FileAttribute*	pFileAttribute;
	bool			bExceptionMode;
};


class OutputStream : public Stream {
public:
	OutputStream(const TCHAR* tszPath, const TCHAR* tszMode, bool bExceptionMode = false)
		:
		Stream(tszPath, tszMode, bExceptionMode)
	{
	}

	// �f�[�^�𖳕ϊ��ŏ������ށB�߂�l�͏������񂾃o�C�g���B
	size_t Write(const void* pBuffer, size_t nSizeInBytes) {
		size_t nRet = fwrite(pBuffer, 1, nSizeInBytes, GetFp());
		if (nRet != nSizeInBytes && IsExceptionMode()) {
			throw Error_FileWrite();
		}
		return nRet;
	}
};

