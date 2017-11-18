#pragma once

#include "Convert.h"


// 半角カナ→全角カナ
class Converter_HankataToZenkata : public Converter {
public:
	bool DoConvert(NativeW* pData);
};

