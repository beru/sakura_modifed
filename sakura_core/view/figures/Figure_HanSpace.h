#pragma once

#include "view/figures/FigureStrategy.h"

// ���p�X�y�[�X�`��
class Figure_HanSpace : public FigureSpace {
public:
	// traits
	bool Match(const wchar_t* pText, int nTextLen) const;

	// action
	void DispSpace(Graphics& gr, DispPos* pDispPos, EditView& view, bool trans) const;
	EColorIndexType GetColorIdx(void) const { return COLORIDX_SPACE; }
};

