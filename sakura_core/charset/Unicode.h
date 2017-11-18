#pragma once

// IsUtf16SurrogHi()、IsUtf16SurrogLow() 関数をcharset/codechecker.h に移動

#include "CodeBase.h"

class Unicode : public CodeBase {
public:
	CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst) {	// 特定コード → UNICODE    変換
		return UnicodeToUnicode_in(src, pDst);
	}
	CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst) {	// UNICODE    → 特定コード 変換
		return UnicodeToUnicode_out(src, pDst);
	}
	void GetBom(Memory* pMemBom);	// BOMデータ取得
	void GetEol(Memory* pMemEol, EolType eolType);	// 改行データ取得

public:
	// 実装
	static CodeConvertResult _UnicodeToUnicode_in(const Memory& src, NativeW* pDstMem, const bool bBigEndian);	// Unicode   → Unicode (入力側)
	static CodeConvertResult _UnicodeToUnicode_out(const NativeW& src, Memory* pDstMem, const bool bBigEndian);	// Unicode   → Unicode (出力側)
	inline static CodeConvertResult UnicodeToUnicode_in(const Memory& src, NativeW* pDst){ return _UnicodeToUnicode_in(src, pDst, false); }
	inline static CodeConvertResult UnicodeToUnicode_out(const NativeW& src, Memory* pDst){ return _UnicodeToUnicode_out(src, pDst, false); }

};

