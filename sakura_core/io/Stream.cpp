#include "StdAfx.h"
#include "Stream.h"
#include <string>

//	::fflush(hFile);
// �l�b�g���[�N��̃t�@�C���������Ă���ꍇ�A
// �������݌��Flush���s���ƃf�b�g���b�N���������邱�Ƃ�����̂ŁA
// Close����::fflush���Ăяo���Ă͂����܂���B
// �ڍׁFhttp://www.microsoft.com/japan/support/faq/KBArticles2.asp?URL=/japan/support/kb/articles/jp288/7/94.asp


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  �t�@�C����������N���X                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class FileAttribute {
public:
	FileAttribute(const TCHAR* tszPath)
		:
		strPath(tszPath),
		bAttributeChanged(false),
		dwAttribute(0)
	{
	}

	// �w�葮������菜��
	void PopAttribute(DWORD dwPopAttribute)
	{
		if (bAttributeChanged) {
			return; // ���Ɏ�菜���ς�
		}
		dwAttribute = ::GetFileAttributes(strPath.c_str());
		if (dwAttribute != (DWORD)-1) {
			if (dwAttribute & dwPopAttribute) {
				DWORD dwNewAttribute = dwAttribute & ~dwPopAttribute;
				::SetFileAttributes(strPath.c_str(), dwNewAttribute);
				bAttributeChanged = true;
			}
		}
	}
	
	// ���������ɖ߂�
	void RestoreAttribute()
	{
		if (bAttributeChanged) {
			::SetFileAttributes(strPath.c_str(), dwAttribute);
		}
		bAttributeChanged = false;
		dwAttribute = 0;
	}
private:
	std::tstring	strPath;
	bool			bAttributeChanged;
	DWORD			dwAttribute;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

Stream::Stream(
	const TCHAR* tszPath,
	const TCHAR* tszMode,
	bool bExceptionMode
	)
{
	fp = nullptr;
	pFileAttribute = NULL;
	this->bExceptionMode = bExceptionMode;
	Open(tszPath, tszMode);
}

/*
Stream::Stream()
{
	fp = nullptr;
	pFileAttribute = NULL;
	bExceptionMode = false;
}
*/

Stream::~Stream()
{
	Close();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    �I�[�v���E�N���[�Y                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
void Stream::Open(
	const TCHAR* tszPath,
	const TCHAR* tszMode
	)
{
	Close(); // ���ɊJ���Ă�����A��x����

	// �����ύX�F�B��or�V�X�e���t�@�C����C�̊֐��œǂݏ����ł��Ȃ��̂ő�����ύX����
	pFileAttribute = new FileAttribute(tszPath);
	pFileAttribute->PopAttribute(FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

	// �I�[�v��
	fp = _tfopen(tszPath, tszMode);
	if (!fp) {
		Close(); // ��������
	}

	// �G���[����
	if (!fp && IsExceptionMode()) {
		throw Error_FileOpen();
	}
}

void Stream::Close()
{
	// �N���[�Y
	if (fp) {
		fclose(fp);
		fp = nullptr;
	}

	// ��������
	if (pFileAttribute) {
		pFileAttribute->RestoreAttribute();
		SAFE_DELETE(pFileAttribute);
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void Stream::SeekSet(	// �V�[�N
	long offset	// �X�g���[���擪����̃I�t�Z�b�g 
	)
{
	fseek(fp, offset, SEEK_SET);
}

void Stream::SeekEnd(  // �V�[�N
	long offset // �X�g���[���I�[����̃I�t�Z�b�g
	)
{
	fseek(fp, offset, SEEK_END);
}

