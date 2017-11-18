#pragma once

#include "CodeBase.h"
#include "Unicode.h"
#include "Eol.h"

class UnicodeBe : public CodeBase {
public:
	// CodeBase�C���^�[�t�F�[�X
	CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst){ return UnicodeBEToUnicode(src, pDst); }	// ����R�[�h �� UNICODE    �ϊ�
	CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst){ return UnicodeToUnicodeBE(src, pDst); }	// UNICODE    �� ����R�[�h �ϊ�
	void GetBom(Memory* pMemBom);	// BOM�f�[�^�擾
	void GetEol(Memory* pMemEol, EolType eolType);	// ���s�f�[�^�擾

public:

	inline static CodeConvertResult UnicodeBEToUnicode(const Memory& src, NativeW* pDst)
		{ return Unicode::_UnicodeToUnicode_in(src, pDst, true); }	// UnicodeBE �� Unicode�R�[�h�ϊ�
	inline static CodeConvertResult UnicodeToUnicodeBE(const NativeW& src, Memory* pDst)
		{ return Unicode::_UnicodeToUnicode_out(src, pDst, true); }	// Unicode   �� UnicodeBE�R�[�h�ϊ�

};

