// �ݒ�t�@�C�����̃e�L�X�g���o�͂��s�����߂̃N���X�Q�B
// .ini �� .mac �̓��o�͂������Ƃ��Ɏg���Ɨǂ��ł��B
// ���u�ҏW�e�L�X�g�v���������߂ł͂Ȃ��A�����܂ł��A.ini��.mac�̂悤�ȁu�ݒ�t�@�C���v�������ړI�̃N���X�Q�ł��B
//
// ���̂Ƃ����ShiftJIS�œ��o�͂��s�����A
// ������UTF-8���ɂ��邱�Ƃɂ��AUNICODE�f�[�^�̌������N����Ȃ��悤�ɂ������B
#pragma once

#include <string>

#include "Stream.h"
class CodeBase;

// �e�L�X�g���̓X�g���[�� (UTF-8, SJIS)
class TextInputStream : public Stream {
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	TextInputStream(const TCHAR* tszPath);
	TextInputStream();
	virtual ~TextInputStream();

	// ����
	std::wstring ReadLineW(); // 1�s�Ǎ��B���s�͍��

private:
	bool bIsUtf8; // UTF-8�Ȃ�true
};

// �e�L�X�g�o�̓X�g���[��
// 2008.01.26 kobake �o�͕����R�[�h��C�ӂŎw��ł���悤�ɕύX
class TextOutputStream : public OutputStream {
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	TextOutputStream(const TCHAR* tszPath, EncodingType eCodeType = CODE_UTF8, bool bExceptionMode = false, bool bBom = true);
	virtual ~TextOutputStream();

	// �����񏑍��B���s����ꂽ���ꍇ�́A���������'\n'���܂߂邱�ƁB(�N���X���œK�؂ȉ��s�R�[�h�ɕϊ����ďo�͂��܂�)
	void WriteString(const wchar_t* szData, int nLen = -1);
	void WriteF(const wchar_t* format, ...);

	// ���l�����B(�N���X���œK���ɐ��`���ďo�͂��܂�)
	void WriteInt(int n);

private:
	CodeBase* pCodeBase;
};


// �e�L�X�g���̓X�g���[���B���΃p�X�̏ꍇ��INI�t�@�C���̃p�X����̑��΃p�X�Ƃ��ĊJ���B
class TextInputStream_AbsIni : public TextInputStream {
public:
	TextInputStream_AbsIni(const TCHAR* tszPath, bool bOrExedir = true);
};

