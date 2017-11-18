#pragma once

#include "Convert.h"


// できる限り全角カタカナにする
class Converter_ToZenkata : public Converter {
public:
	bool DoConvert(NativeW* pData);
};

