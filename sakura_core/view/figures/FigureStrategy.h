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

#include <vector>
#include "view/colors/ColorStrategy.h" // ColorStrategyInfo


//$$���C�A�E�g�\�z�t���[(DoLayout)�� Figure �ōs���Ɛ������₷��
class Figure {
public:
	virtual ~Figure() {}
	virtual bool DrawImp(ColorStrategyInfo& csInfo) = 0;
	virtual bool Match(const wchar_t* pText, int nTextLen) const = 0;

	// �F�����\���Ώ۔���
	virtual bool Disp(void) const = 0;

	// �ݒ�X�V
	virtual void Update(void) {
		EditDoc* pEditDoc = EditDoc::GetInstance(0);
		m_pTypeData = &pEditDoc->m_docType.GetDocumentAttribute();
	}
protected:
	const TypeConfig* m_pTypeData;
};

// �ʏ�e�L�X�g�`��
class Figure_Text : public Figure {
public:
	bool DrawImp(ColorStrategyInfo& csInfo) override;
	bool Match(const wchar_t* pText, int nTextLen) const {
		return true;
	}

	// �F�����\���Ώ۔���
	virtual bool Disp(void) const {
		return true;
	}
};

// �e��󔒁i���p�󔒁^�S�p�󔒁^�^�u�^���s�j�`��p�̊�{�N���X
class FigureSpace : public Figure {
public:
	virtual bool DrawImp(ColorStrategyInfo& csInfo) override;
protected:
	virtual void DispSpace(Graphics& gr, DispPos* pDispPos, EditView& view, bool bTrans) const = 0;
	virtual EColorIndexType GetColorIdx(void) const = 0;

	// �F�����\���Ώ۔���
	virtual bool Disp(void) const {
		EColorIndexType nColorIndex = GetColorIdx();
		return m_pTypeData->colorInfoArr[nColorIndex].bDisp;
	}

	virtual void Update(void) {
		Figure::Update();

		EColorIndexType nColorIndex = GetColorIdx();
		if (m_pTypeData->colorInfoArr[nColorIndex].bDisp) {
			m_nDispColorIndex = nColorIndex;
		}else {
			m_nDispColorIndex = COLORIDX_TEXT;
		}
	}

	EColorIndexType GetDispColorIdx(void) const { return m_nDispColorIndex; }

	// �����⏕
	bool DrawImp_StyleSelect(ColorStrategyInfo& csInfo);
	void DrawImp_StylePop(ColorStrategyInfo& csInfo);
	void DrawImp_DrawUnderline(ColorStrategyInfo& csInfo, DispPos&);

protected:
	EColorIndexType m_nDispColorIndex;
};

