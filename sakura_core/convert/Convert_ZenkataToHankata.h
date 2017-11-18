#pragma once

#include "Convert.h"

// 全角カナ→半角カナ
class Converter_ZenkataToHankata : public Converter {
public:
	bool DoConvert(NativeW* pData);
};

