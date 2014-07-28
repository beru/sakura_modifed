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

#include "CCodeBase.h"

class CUnicode : public CCodeBase{
public:
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){	//!< 特定コード → UNICODE    変換
		*pDst->_GetMemory() = cSrc;
		return UnicodeToUnicode_in(pDst->_GetMemory());
	}
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){	//!< UNICODE    → 特定コード 変換
		*pDst=*cSrc._GetMemory();
		return UnicodeToUnicode_out(pDst);
	}
	void GetBom(CMemory* pcmemBom);	//!< BOMデータ取得
	void GetEol(CMemory* pcmemEol, EEolType eEolType);	//!< 改行データ取得

public:
	//実装
	static EConvertResult _UnicodeToUnicode_in(CMemory* pMem, const bool bBigEndian);		// Unicode   → Unicode (入力側)
	static EConvertResult _UnicodeToUnicode_out(CMemory* pMem, const bool bBigEndian);	// Unicode   → Unicode (出力側)
	inline static EConvertResult UnicodeToUnicode_in(CMemory* pMem){ return _UnicodeToUnicode_in(pMem,false); }
	inline static EConvertResult UnicodeToUnicode_out(CMemory* pMem){ return _UnicodeToUnicode_out(pMem,false); }

};

