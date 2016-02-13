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

#include "CCodeBase.h"
#include "CUnicode.h"
#include "CEol.h"

class UnicodeBe : public CodeBase {
public:
	// CodeBaseインターフェース
	CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst){ return UnicodeBEToUnicode(src, pDst); }	//!< 特定コード → UNICODE    変換
	CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst){ return UnicodeToUnicodeBE(src, pDst); }	//!< UNICODE    → 特定コード 変換
	void GetBom(Memory* pMemBom);	//!< BOMデータ取得
	void GetEol(Memory* pMemEol, EolType eolType);	//!< 改行データ取得

public:

	inline static CodeConvertResult UnicodeBEToUnicode(const Memory& src, NativeW* pDst)
		{ return Unicode::_UnicodeToUnicode_in(src, pDst, true); }	// UnicodeBE → Unicodeコード変換 //2007.08.13 kobake 追加
	inline static CodeConvertResult UnicodeToUnicodeBE(const NativeW& src, Memory* pDst)
		{ return Unicode::_UnicodeToUnicode_out(src, pDst, true); }	// Unicode   → UnicodeBEコード変換

};

