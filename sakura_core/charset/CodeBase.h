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

// �萔
enum class CodeConvertResult {
	Complete, // �f�[�^���������ƂȂ��ϊ������������B
	LoseSome, // �ϊ��������������A�ꕔ�̃f�[�^������ꂽ�B
	Failure,  // ���炩�̌����ɂ�莸�s�����B
};

class Memory;
class NativeW;
struct CommonSetting_StatusBar;
enum class EolType;

/*!
	�����R�[�h���N���X�B
	
	�����Ō����u����R�[�h�v�Ƃ́A
	CodeBase���p�������q�N���X����߂�A��ӂ̕����R�[�h�̂��Ƃł��B
*/
class CodeBase {
public:
	virtual ~CodeBase() {}
//	virtual bool IsCode(const Memory* pMem) {return false;}  // ����R�[�h�ł����true

	// �����R�[�h�ϊ�
	virtual CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst) = 0;	// ����R�[�h �� UNICODE    �ϊ�
	virtual CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst) = 0;	// UNICODE    �� ����R�[�h �ϊ�
	// UNICODE    �� ����R�[�h �ϊ�
	virtual CodeConvertResult UnicodeToCode(const StringRef& src, Memory* pDst) {
		NativeW mem(src.GetPtr(), src.GetLength());
		return UnicodeToCode(mem, pDst);
	}

	// �t�@�C���`��
	virtual void GetBom(Memory* pMemBom);											// BOM�f�[�^�擾
	void GetEol(Memory* pMemEol, EolType eEolType) { S_GetEol(pMemEol, eEolType); }	// ���s�f�[�^�擾 virtual������̂�	2010/6/13 Uchi

	// �����R�[�h�\���p		2008/6/9 Uchi
	virtual CodeConvertResult UnicodeToHex(const wchar_t* pSrc, const int iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar);			// UNICODE �� Hex �ϊ�

	// �ϊ��G���[�����i�P�o�C�g <-> U+D800 ���� U+D8FF�j
	static int BinToText(const unsigned char*, const int, unsigned short*);
	static int TextToBin(const unsigned short);

	// MIME Header �f�R�[�_
	static bool MIMEHeaderDecode(const char*, const int, Memory*, const EncodingType);

	// CShiftJis���ړ� 2010/6/13 Uchi
	static void S_GetEol(Memory* pMemEol, EolType eEolType);	// ���s�f�[�^�擾
	
protected:

};

/*!
	�o�C�i���P�o�C�g�� U+DC00 ���� U+DCFF �܂łɑΉ��t����
*/
inline int CodeBase::BinToText(const unsigned char* pSrc, const int nLen, unsigned short* pDst)
{
	int i;

	for (i=0; i<nLen; ++i) {
		pDst[i] = static_cast<unsigned short>(pSrc[i]) + 0xdc00;
	}

	return i;
}


/*!
	U+DC00 ���� U+DCFF ����o�C�i��1�o�C�g�𕜌�
*/
inline int CodeBase::TextToBin(const unsigned short cSrc)
{
	return static_cast<int>((cSrc - 0xdc00) & 0x00ff);
}

