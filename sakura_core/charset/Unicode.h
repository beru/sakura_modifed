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

