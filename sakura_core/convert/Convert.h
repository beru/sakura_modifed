#pragma once

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

