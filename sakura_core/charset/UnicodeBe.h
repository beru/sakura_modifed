#pragma once

#include "CodeBase.h"
#include "Unicode.h"
#include "Eol.h"

class UnicodeBe : public CodeBase {
public:
	// CodeBaseインターフェース
	CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst){ return UnicodeBEToUnicode(src, pDst); }	// 特定コード → UNICODE    変換
	CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst){ return UnicodeToUnicodeBE(src, pDst); }	// UNICODE    → 特定コード 変換
	void GetBom(Memory* pMemBom);	// BOMデータ取得
	void GetEol(Memory* pMemEol, EolType eolType);	// 改行データ取得

public:

	inline static CodeConvertResult UnicodeBEToUnicode(const Memory& src, NativeW* pDst)
		{ return Unicode::_UnicodeToUnicode_in(src, pDst, true); }	// UnicodeBE → Unicodeコード変換
	inline static CodeConvertResult UnicodeToUnicodeBE(const NativeW& src, Memory* pDst)
		{ return Unicode::_UnicodeToUnicode_out(src, pDst, true); }	// Unicode   → UnicodeBEコード変換

};

