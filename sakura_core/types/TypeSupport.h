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

#include "uiparts/Graphics.h"
#include "doc/EditDoc.h"
#include "view/EditView.h"
#include "view/ViewFont.h"
#include "view/colors/ColorStrategy.h"

// 2007.08.28 kobake �ǉ�
/*!�^�C�v�T�|�[�g�N���X
	���̂Ƃ���^�C�v�ʐݒ�̐F���擾�̕⏕
*/
class TypeSupport {
private:
	static const COLORREF INVALID_COLOR = 0xFFFFFFFF; // �����ȐF�萔

public:
	TypeSupport(const EditView& editView, EColorIndexType eColorIdx)
		:
		pFontset(&editView.GetFontset()),
		nColorIdx(ToColorInfoArrIndex(eColorIdx))
	{
		ASSERT_GE(nColorIdx, 0);
		pTypes = &editView.pEditDoc->docType.GetDocumentAttribute();
		pColorInfoArr = &pTypes->colorInfoArr[nColorIdx];

		gr = nullptr;
	}
	virtual ~TypeSupport() {
		if (gr) {
			RewindGraphicsState(*gr);
		}
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �擾                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �O�i�F(�����F)
	COLORREF GetTextColor() const {
		return pColorInfoArr->colorAttr.cTEXT;
	}

	// �w�i�F
	COLORREF GetBackColor() const {
		return pColorInfoArr->colorAttr.cBACK;
	}

	// �\�����邩�ǂ���
	bool IsDisp() const {
		return pColorInfoArr->bDisp;
	}

	// �������ǂ���
	bool IsBoldFont() const {
		return pColorInfoArr->fontAttr.bBoldFont;
	}

	// �����������ǂ���
	bool HasUnderLine() const {
		return pColorInfoArr->fontAttr.bUnderLine;
	}

	const ColorInfo& GetColorInfo() const {
		return *pColorInfoArr;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �`��                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void FillBack(Graphics& gr, const RECT& rc) {
		gr.FillSolidMyRect(rc, pColorInfoArr->colorAttr.cBACK);
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ݒ�                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	Font GetTypeFont() {
		Font font;
		font.fontAttr = pColorInfoArr->fontAttr;
		font.hFont = pFontset->ChooseFontHandle( pColorInfoArr->fontAttr );
		return font;
	}
	
	void SetGraphicsState_WhileThisObj(Graphics& gr) {
		if (this->gr) {
			RewindGraphicsState(*this->gr);
		}

		this->gr = &gr;

		// �e�L�X�g�F
		gr.PushTextBackColor(GetBackColor());
		gr.PushTextForeColor(GetTextColor());

		// �t�H���g
		gr.PushMyFont(GetTypeFont());
	}
	
	void RewindGraphicsState(Graphics& gr) {
		if (this->gr) {
			gr.PopTextBackColor();
			gr.PopTextForeColor();
			gr.PopMyFont();
			this->gr = nullptr;
		}
	}

private:
	const ViewFont*		pFontset;
	const TypeConfig*	pTypes;
	int					nColorIdx;
	const ColorInfo*	pColorInfoArr;

	Graphics* gr;    // �ݒ��ύX����HDC
};

