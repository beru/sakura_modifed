#pragma once

#include "CodeBase.h"
#include "Utf8.h"

class Cesu8 : public CodeBase {
public:

	// CodeBaseインターフェース
	CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst) {	// 特定コード → UNICODE    変換
		return Utf8::CESU8ToUnicode(src, pDst);
	}
	
	CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst) {	// UNICODE    → 特定コード 変換
		return Utf8::UnicodeToCESU8(src, pDst);
	}
	
	void GetBom(Memory* pMemBom);	// BOMデータ取得
	CodeConvertResult UnicodeToHex(const wchar_t* src, size_t iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar) {	// UNICODE → Hex 変換
		return Utf8()._UnicodeToHex(src, iSLen, pDst, psStatusbar, true);
	}

};

