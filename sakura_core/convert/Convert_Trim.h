#pragma once

#include "Convert.h"


class Converter_Trim : public Converter {
public:
	Converter_Trim(bool bLeft, bool bExtEol) : bLeft(bLeft), bExtEol(bExtEol) { }

public:
	bool DoConvert(NativeW* pData);

private:
	bool bLeft;
	bool bExtEol;
};

