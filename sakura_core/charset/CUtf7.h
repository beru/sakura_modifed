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

class Utf7 : public CodeBase {
public:
	// CodeBaseインターフェース
	CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst){ return UTF7ToUnicode(src, pDst); }	//!< 特定コード → UNICODE    変換
	CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst){ return UnicodeToUTF7(src, pDst); }	//!< UNICODE    → 特定コード 変換
	void GetBom(Memory* pMemBom);	//!< BOMデータ取得
	void GetEol(Memory* pMemEol, EolType eolType);

public:
	// 実装
	static CodeConvertResult UTF7ToUnicode(const Memory& src, NativeW* pDstMem);		// UTF-7     → Unicodeコード変換 //2007.08.13 kobake 追加
	static CodeConvertResult UnicodeToUTF7(const NativeW& src, Memory* pDstMem);		// Unicode   → UTF-7コード変換
//	static int MemBASE64_Encode(const char*, int, char**, int, int);/* Base64エンコード */  // convert/convert_util2.h へ移動

protected:

	// 2008.11.10 変換ロジックを書き直す
	static int _Utf7SetDToUni_block(const char*, const int, wchar_t*);
	static int _Utf7SetBToUni_block(const char*, const int, wchar_t*, bool*);
	static int Utf7ToUni(const char*, const int, wchar_t*, bool* pbError);

	static int _UniToUtf7SetD_block(const wchar_t* pSrc, const int nSrcLen, char* pDst);
	static int _UniToUtf7SetB_block(const wchar_t* pSrc, const int nSrcLen, char* pDst);
	static int UniToUtf7(const wchar_t* pSrc, const int nSrcLen, char* pDst);


};

