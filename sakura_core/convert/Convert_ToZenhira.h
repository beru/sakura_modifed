#pragma once

#include "Convert.h"


// できる限り全角ひらがなにする
class Converter_ToZenhira : public Converter {
public:
	bool DoConvert(NativeW* pData);
};

