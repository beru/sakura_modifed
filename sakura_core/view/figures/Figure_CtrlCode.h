#pragma once

#include "view/figures/FigureStrategy.h"

// �R���g���[���R�[�h�`��
class Figure_CtrlCode : public FigureSpace {
public:
	// traits
	bool Match(const wchar_t* pText, int nTextLen) const;

	// action
	void DispSpace(Graphics& gr, DispPos* pDispPos, EditView& view, bool bTrans) const;
	EColorIndexType GetColorIdx(void) const { return COLORIDX_CTRLCODE; }
};

// �o�C�i�����p�`��
class Figure_HanBinary : public FigureSpace {
public:
	// traits
	bool Match(const wchar_t* pText, int nTextLen) const;

	// action
	void DispSpace(Graphics& gr, DispPos* pDispPos, EditView& view, bool bTrans) const;
	EColorIndexType GetColorIdx(void) const { return COLORIDX_CTRLCODE; }
};

// �o�C�i���S�p�`��
class Figure_ZenBinary : public FigureSpace {
public:
	// traits
	bool Match(const wchar_t* pText, int nTextLen) const;

	// action
	void DispSpace(Graphics& gr, DispPos* pDispPos, EditView& view, bool bTrans) const;
	EColorIndexType GetColorIdx(void) const { return COLORIDX_CTRLCODE; }
};

