#pragma once

#include "CodeBase.h"

class Utf7 : public CodeBase {
public:
	// CodeBase�C���^�[�t�F�[�X
	CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst){ return UTF7ToUnicode(src, pDst); }	// ����R�[�h �� UNICODE    �ϊ�
	CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst){ return UnicodeToUTF7(src, pDst); }	// UNICODE    �� ����R�[�h �ϊ�
	void GetBom(Memory* pMemBom);	// BOM�f�[�^�擾
	void GetEol(Memory* pMemEol, EolType eolType);

public:
	// ����
	static CodeConvertResult UTF7ToUnicode(const Memory& src, NativeW* pDstMem);		// UTF-7     �� Unicode�R�[�h�ϊ�
	static CodeConvertResult UnicodeToUTF7(const NativeW& src, Memory* pDstMem);		// Unicode   �� UTF-7�R�[�h�ϊ�

protected:

	static size_t _Utf7SetDToUni_block(const char*, const size_t, wchar_t*);
	static size_t _Utf7SetBToUni_block(const char*, const size_t, wchar_t*, bool*);
	static size_t Utf7ToUni(const char*, const size_t, wchar_t*, bool* pbError);

	static size_t _UniToUtf7SetD_block(const wchar_t* pSrc, const size_t nSrcLen, char* pDst);
	static size_t _UniToUtf7SetB_block(const wchar_t* pSrc, const size_t nSrcLen, char* pDst);
	static size_t UniToUtf7(const wchar_t* pSrc, const size_t nSrcLen, char* pDst);
};

