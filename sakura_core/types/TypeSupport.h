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
	TypeSupport(const EditView* pEditView, EColorIndexType eColorIdx)
		:
		m_pFontset(&pEditView->GetFontset()),
		m_nColorIdx(ToColorInfoArrIndex(eColorIdx))
	{
		assert(0 <= m_nColorIdx);
		m_pTypes = &pEditView->m_pEditDoc->m_docType.GetDocumentAttribute();
		m_pColorInfoArr = &m_pTypes->colorInfoArr[m_nColorIdx];

		m_gr = NULL;
	}
	virtual ~TypeSupport() {
		if (m_gr) {
			RewindGraphicsState(*m_gr);
		}
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �擾                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �O�i�F(�����F)
	COLORREF GetTextColor() const {
		return m_pColorInfoArr->colorAttr.cTEXT;
	}

	// �w�i�F
	COLORREF GetBackColor() const {
		return m_pColorInfoArr->colorAttr.cBACK;
	}

	// �\�����邩�ǂ���
	bool IsDisp() const {
		return m_pColorInfoArr->bDisp;
	}

	// �������ǂ���
	bool IsBoldFont() const {
		return m_pColorInfoArr->fontAttr.bBoldFont;
	}

	// �����������ǂ���
	bool HasUnderLine() const {
		return m_pColorInfoArr->fontAttr.bUnderLine;
	}

	const ColorInfo& GetColorInfo() const {
		return *m_pColorInfoArr;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �`��                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void FillBack(Graphics& gr, const RECT& rc) {
		gr.FillSolidMyRect(rc, m_pColorInfoArr->colorAttr.cBACK);
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ݒ�                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	Font GetTypeFont() {
		Font sFont;
		sFont.m_fontAttr = m_pColorInfoArr->fontAttr;
		sFont.m_hFont = m_pFontset->ChooseFontHandle( m_pColorInfoArr->fontAttr );
		return sFont;
	}
	
	void SetGraphicsState_WhileThisObj(Graphics& gr) {
		if (m_gr) {
			RewindGraphicsState(*m_gr);
		}

		m_gr = &gr;

		// �e�L�X�g�F
		gr.PushTextBackColor(GetBackColor());
		gr.PushTextForeColor(GetTextColor());

		// �t�H���g
		gr.PushMyFont(GetTypeFont());
	}
	
	void RewindGraphicsState(Graphics& gr) {
		if (m_gr) {
			gr.PopTextBackColor();
			gr.PopTextForeColor();
			gr.PopMyFont();
			m_gr = NULL;
		}
	}

private:
	const ViewFont*	m_pFontset;
	const TypeConfig*	m_pTypes;
	int					m_nColorIdx;
	const ColorInfo*	m_pColorInfoArr;

	Graphics* m_gr;    // �ݒ��ύX����HDC
};

