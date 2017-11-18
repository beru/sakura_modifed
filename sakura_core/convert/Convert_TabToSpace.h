#pragma once

#include "Convert.h"

class Converter_TabToSpace : public Converter {
public:
	Converter_TabToSpace(int nTabWidth, int nStartColumn, bool bExtEol)
		:
		nTabWidth(nTabWidth),
		nStartColumn(nStartColumn),
		bExtEol(bExtEol)
	{
	}

	bool DoConvert(NativeW* pData);

private:
	int nTabWidth;
	int nStartColumn;
	bool bExtEol;
};

