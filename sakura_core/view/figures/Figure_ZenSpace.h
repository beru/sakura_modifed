#pragma once

#include "view/figures/FigureStrategy.h"

// �S�p�X�y�[�X�`��
class Figure_ZenSpace : public FigureSpace {
public:
	// traits
	bool Match(const wchar_t* pText, int nTextLen) const;

	// action
	void DispSpace(Graphics& gr, DispPos* pDispPos, EditView& view, bool bTrans) const;
	EColorIndexType GetColorIdx(void) const { return COLORIDX_ZENSPACE; }
};

