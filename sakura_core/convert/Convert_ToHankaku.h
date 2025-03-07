#pragma once

#include "Convert.h"


// 半角にできるものは全部半角に変換
class Converter_ToHankaku : public Converter {
public:
	bool DoConvert(NativeW* pData);
};

enum class ToHankakuMode {
	Katakana	= 0x01, // カタカナに影響アリ
	Hiragana	= 0x02, // ひらがなに影響アリ
	Alnum		= 0x04, // 英数字に影響アリ
};

