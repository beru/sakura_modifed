#pragma once

#include "convert/Decode.h"

class Decode_Base64Decode : public Decode {
public:
	bool DoDecode(const NativeW& data, Memory* pDst);

};

