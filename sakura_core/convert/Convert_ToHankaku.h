#pragma once

#include "Convert.h"


// ”¼Šp‚É‚Å‚«‚é‚à‚Ì‚Í‘S•””¼Šp‚É•ÏŠ·
class Converter_ToHankaku : public Converter {
public:
	bool DoConvert(NativeW* pData);
};

enum class ToHankakuMode {
	Katakana	= 0x01, // ƒJƒ^ƒJƒi‚É‰e‹¿ƒAƒŠ
	Hiragana	= 0x02, // ‚Ğ‚ç‚ª‚È‚É‰e‹¿ƒAƒŠ
	Alnum		= 0x04, // ‰p”š‚É‰e‹¿ƒAƒŠ
};

