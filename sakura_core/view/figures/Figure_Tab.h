#pragma once

#include "view/figures/FigureStrategy.h"

// ƒ^ƒu•`‰æ
class Figure_Tab : public FigureSpace {
public:
	// traits
	bool Match(const wchar_t* pText, int nTextLen) const;
	bool Disp(void) const {
		return true;
	}

	// action
	void DispSpace(Graphics& gr, DispPos* pDispPos, EditView& view, bool bTrans) const;
	EColorIndexType GetColorIdx(void) const { return COLORIDX_TAB; }
};

