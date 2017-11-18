#pragma once

#include "CodeBase.h"
#include "Utf8.h"

class Cesu8 : public CodeBase {
public:

	// CodeBase�C���^�[�t�F�[�X
	CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst) {	// ����R�[�h �� UNICODE    �ϊ�
		return Utf8::CESU8ToUnicode(src, pDst);
	}
	
	CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst) {	// UNICODE    �� ����R�[�h �ϊ�
		return Utf8::UnicodeToCESU8(src, pDst);
	}
	
	void GetBom(Memory* pMemBom);	// BOM�f�[�^�擾
	CodeConvertResult UnicodeToHex(const wchar_t* src, size_t iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar) {	// UNICODE �� Hex �ϊ�
		return Utf8()._UnicodeToHex(src, iSLen, pDst, psStatusbar, true);
	}

};

