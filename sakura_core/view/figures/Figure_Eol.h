#pragma once

#include "view/figures/FigureStrategy.h"

// â¸çsï`âÊ
class Figure_Eol : public FigureSpace {
public:
	// traits
	bool Match(const wchar_t* pText, int nTextLen) const;
	bool Disp(void) const {
		return true;
	}

	// action
	bool DrawImp(ColorStrategyInfo& csInfo) override;
	void DispSpace(Graphics& gr, DispPos* pDispPos, EditView& view, bool bTrans) const {};
	EColorIndexType GetColorIdx(void) const { return COLORIDX_EOL; }
};

