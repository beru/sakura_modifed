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
#include "CUtf8.h"

class Cesu8 : public CodeBase {
public:

	// CodeBaseインターフェース
	CodeConvertResult CodeToUnicode(const Memory& cSrc, NativeW* pDst) {	//!< 特定コード → UNICODE    変換
		return Utf8::CESU8ToUnicode(cSrc, pDst);
	}
	
	CodeConvertResult UnicodeToCode(const NativeW& cSrc, Memory* pDst) {	//!< UNICODE    → 特定コード 変換
		return Utf8::UnicodeToCESU8(cSrc, pDst);
	}
	
	void GetBom(Memory* pcmemBom);	//!< BOMデータ取得
	// GetEolはCodeBaseに移動	2010/6/13 Uchi
	CodeConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst, const CommonSetting_Statusbar* psStatusbar) {			//!< UNICODE → Hex 変換
		return Utf8()._UnicodeToHex(cSrc, iSLen, pDst, psStatusbar, true);
	}

};

