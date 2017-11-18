#pragma once

#include "Convert.h"

class Converter_SpaceToTab : public Converter {
public:
	Converter_SpaceToTab(int nTabWidth, int nStartColumn, bool bExtEol)
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

