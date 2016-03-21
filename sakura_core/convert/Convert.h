/*
	Copyright (C) 2007, kobake

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

// 2007.10.02 kobake CEditViewから分離

#include "Funccode_enum.h"	// EFunctionCode

class ConvertMediator {
public:
	// 機能種別によるバッファの変換
	static void ConvMemory(NativeW* pMemory, EFunctionCode nFuncCode, int nTabWidth, int nStartColumn);

protected:
	static void Command_TRIM2(NativeW* pMemory, bool bLeft);
};

class Converter {
public:
	virtual ~Converter() {}

	// インターフェース
	void CallConvert(NativeW* pData)
	{
		bool bRet = DoConvert(pData);
		if (!bRet) {
			ErrorMessage(NULL, LS(STR_CONVERT_ERR));
		}
	}

	// 実装
	virtual bool DoConvert(NativeW* pData) = 0;
};

