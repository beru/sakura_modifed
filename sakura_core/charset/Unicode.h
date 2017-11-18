#pragma once

// IsUtf16SurrogHi()�AIsUtf16SurrogLow() �֐���charset/codechecker.h �Ɉړ�

#include "CodeBase.h"

class Unicode : public CodeBase {
public:
	CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst) {	// ����R�[�h �� UNICODE    �ϊ�
		return UnicodeToUnicode_in(src, pDst);
	}
	CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst) {	// UNICODE    �� ����R�[�h �ϊ�
		return UnicodeToUnicode_out(src, pDst);
	}
	void GetBom(Memory* pMemBom);	// BOM�f�[�^�擾
	void GetEol(Memory* pMemEol, EolType eolType);	// ���s�f�[�^�擾

public:
	// ����
	static CodeConvertResult _UnicodeToUnicode_in(const Memory& src, NativeW* pDstMem, const bool bBigEndian);	// Unicode   �� Unicode (���͑�)
	static CodeConvertResult _UnicodeToUnicode_out(const NativeW& src, Memory* pDstMem, const bool bBigEndian);	// Unicode   �� Unicode (�o�͑�)
	inline static CodeConvertResult UnicodeToUnicode_in(const Memory& src, NativeW* pDst){ return _UnicodeToUnicode_in(src, pDst, false); }
	inline static CodeConvertResult UnicodeToUnicode_out(const NativeW& src, Memory* pDst){ return _UnicodeToUnicode_out(src, pDst, false); }

};

