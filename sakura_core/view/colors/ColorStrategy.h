/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#pragma once

// �v��s��`
// #include "view/EditView.h"
#include "EColorIndexType.h"
#include "uiparts/Graphics.h"

class EditView;

bool _IsPosKeywordHead(const StringRef& str, int nPos);

// ���K�\���L�[���[�h��EColorIndexType�l�����֐�
inline
EColorIndexType ToColorIndexType_RegularExpression(const int nRegexColorIndex)
{
	return (EColorIndexType)(COLORIDX_REGEX_FIRST + nRegexColorIndex);
}

// ���K�\���L�[���[�h��EColorIndexType�l��F�ԍ��ɖ߂��֐�
inline
int ToColorInfoArrIndex_RegularExpression(const EColorIndexType eRegexColorIndex)
{
	return eRegexColorIndex - COLORIDX_REGEX_FIRST;
}

/*! �F�萔��F�ԍ��ɕϊ�����֐�

	@date 2013.05.08 novice �͈͊O�̂Ƃ��̓e�L�X�g��I������
*/
inline
int ToColorInfoArrIndex(const EColorIndexType eColorIndex)
{
	if (eColorIndex >= 0 && eColorIndex < COLORIDX_LAST)
		return eColorIndex;
	else if (eColorIndex & COLORIDX_BLOCK_BIT)
		return COLORIDX_COMMENT;
	else if (eColorIndex & COLORIDX_REGEX_BIT)
		return ToColorInfoArrIndex_RegularExpression(eColorIndex);

	assert(0); // �����ɂ͗��Ȃ�
	return COLORIDX_TEXT;
}

// �J���[�������C���f�b�N�X�ԍ��̕ϊ�	//@@@ 2002.04.30
int GetColorIndexByName(const TCHAR* name);
const TCHAR* GetColorNameByIndex(int index);


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

struct DispPos;
class ColorStrategy;
#include "view/DispPos.h"

class Color_Found;
class Color_Select;

// �F�ݒ�
struct Color3Setting {
	EColorIndexType eColorIndex;    // �I�����܂ތ��݂̐F
	EColorIndexType eColorIndex2;   // �I���ȊO�̌��݂̐F
	EColorIndexType eColorIndexBg;  // �w�i�F
};

struct ColorStrategyInfo {
	ColorStrategyInfo(EditView& view)
		:
		view(view),
		dispPosBegin(0, 0),
		pStrategy(nullptr),
		pStrategyFound(nullptr),
		pStrategySelect(nullptr),
		colorIdxBackLine(COLORIDX_TEXT)
	{
		index.eColorIndex = COLORIDX_TEXT;
		index.eColorIndex2 = COLORIDX_TEXT;
		index.eColorIndexBg = COLORIDX_TEXT;
	}

	// �Q��
	EditView&	view;
	Graphics	gr;	// (SColorInfo�ł͖��g�p)

	// �X�L�����ʒu
	LPCWSTR			pLineOfLogic;
	LogicInt		nPosInLogic;

	// �`��ʒu
	DispPos*		pDispPos;
	DispPos			dispPosBegin;

	// �F�ς�
	ColorStrategy*		pStrategy;
	Color_Found*		pStrategyFound;
	Color_Select*		pStrategySelect;
	EColorIndexType		colorIdxBackLine;
	Color3Setting		index;

	// �F�̐؂�ւ�
	bool CheckChangeColor(const StringRef& lineStr);
	void DoChangeColor(Color3Setting *pcColor);
	EColorIndexType GetCurrentColor() const { return index.eColorIndex; }
	EColorIndexType GetCurrentColor2() const { return index.eColorIndex2; }
	EColorIndexType GetCurrentColorBg() const { return index.eColorIndexBg; }

	// ���݂̃X�L�����ʒu
	LogicInt GetPosInLogic() const {
		return nPosInLogic;
	}
	
	const DocLine* GetDocLine() const {
		return pDispPos->GetLayoutRef()->GetDocLineRef();
	}
	
	const Layout* GetLayout() const {
		return pDispPos->GetLayoutRef();
	}
	
};

class ColorStrategy {
public:
	virtual ~ColorStrategy() {}
	// �F��`
	virtual EColorIndexType GetStrategyColor() const = 0;
	virtual LayoutColorInfo* GetStrategyColorInfo() const {
		return nullptr;
	}
	// �F�؂�ւ��J�n�����o������A���̒��O�܂ł̕`����s���A����ɐF�ݒ���s���B
	virtual void InitStrategyStatus() = 0;
	virtual void SetStrategyColorInfo(const LayoutColorInfo* = nullptr) {};
	virtual bool BeginColor(const StringRef& str, int nPos) { return false; }
	virtual bool EndColor(const StringRef& str, int nPos) { return true; }
	virtual bool Disp() const = 0;
	// �C�x���g
	virtual void OnStartScanLogic() {}

	// �ݒ�X�V
	virtual void Update(void) {
		const EditDoc* pEditDoc = EditDoc::GetInstance(0);
		pTypeData = &pEditDoc->docType.GetDocumentAttribute();
	}

	//#######���b�v
	EColorIndexType GetStrategyColorSafe() const { if (this) return GetStrategyColor(); else return COLORIDX_TEXT; }
	LayoutColorInfo* GetStrategyColorInfoSafe() const {
		if (this) {
			return GetStrategyColorInfo();
		}
		return nullptr;
	}

protected:
	const TypeConfig* pTypeData;
};

#include "util/design_template.h"
#include <vector>
class Color_LineComment;
class Color_BlockComment;
class Color_BlockComment;
class Color_SingleQuote;
class Color_DoubleQuote;
class Color_Heredoc;

class ColorStrategyPool : public TSingleton<ColorStrategyPool> {
	friend class TSingleton<ColorStrategyPool>;
	ColorStrategyPool();
	virtual ~ColorStrategyPool();

public:

	// �擾
	ColorStrategy*	GetStrategy(int nIndex) const { return vStrategiesDisp[nIndex]; }
	int				GetStrategyCount() const { return (int)vStrategiesDisp.size(); }
	ColorStrategy*	GetStrategyByColor(EColorIndexType eColor) const;

	// ����擾
	Color_Found*   GetFoundStrategy() const { return pcFoundStrategy; }
	Color_Select*  GetSelectStrategy() const { return pcSelectStrategy; }

	// �C�x���g
	void NotifyOnStartScanLogic();

	/*
	|| �F����
	*/
	//@@@ 2002.09.22 YAZAKI
	// 2005.11.21 Moca ���p���̐F���������������珜��
	void CheckColorMODE(ColorStrategy** ppColorStrategy, int nPos, const StringRef& lineStr);
	bool IsSkipBeforeLayout();	// ���C�A�E�g���s������`�F�b�N���Ȃ��Ă���������

	// �ݒ�ύX
	void OnChangeSetting(void);

	// �r���[�̐ݒ�E�擾
	EditView* GetCurrentView(void) const { return pView; }
	void SetCurrentView(EditView* pView) { pView = pView; }

private:
	std::vector<ColorStrategy*>	vStrategies;
	std::vector<ColorStrategy*>	vStrategiesDisp;	// �F�����\���Ώ�
	Color_Found*					pcFoundStrategy;
	Color_Select*					pcSelectStrategy;

	Color_LineComment*				pcLineComment;
	Color_BlockComment*				pcBlockComment1;
	Color_BlockComment*				pcBlockComment2;
	Color_SingleQuote*				pcSingleQuote;
	Color_DoubleQuote*				pcDoubleQuote;
	Color_Heredoc*					pcHeredoc;

	EditView*						pView;

	bool	bSkipBeforeLayoutGeneral;
	bool	bSkipBeforeLayoutFound;
};

