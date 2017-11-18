#pragma once

#include "view/figures/FigureStrategy.h"

// コントロールコード描画
class Figure_CtrlCode : public FigureSpace {
public:
	// traits
	bool Match(const wchar_t* pText, int nTextLen) const;

	// action
	void DispSpace(Graphics& gr, DispPos* pDispPos, EditView& view, bool bTrans) const;
	EColorIndexType GetColorIdx(void) const { return COLORIDX_CTRLCODE; }
};

// バイナリ半角描画
class Figure_HanBinary : public FigureSpace {
public:
	// traits
	bool Match(const wchar_t* pText, int nTextLen) const;

	// action
	void DispSpace(Graphics& gr, DispPos* pDispPos, EditView& view, bool bTrans) const;
	EColorIndexType GetColorIdx(void) const { return COLORIDX_CTRLCODE; }
};

// バイナリ全角描画
class Figure_ZenBinary : public FigureSpace {
public:
	// traits
	bool Match(const wchar_t* pText, int nTextLen) const;

	// action
	void DispSpace(Graphics& gr, DispPos* pDispPos, EditView& view, bool bTrans) const;
	EColorIndexType GetColorIdx(void) const { return COLORIDX_CTRLCODE; }
};

