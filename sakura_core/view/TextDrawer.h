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

class TextMetrics;
class TextArea;
class ViewFont;
class Eol;
class EditView;
class Layout;
#include "DispPos.h"

class Graphics;

class TextDrawer {
public:
	TextDrawer(const EditView* pEditView) : m_pEditView(pEditView) { }
	virtual ~TextDrawer() {}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �O���ˑ�                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �̈�̃C���X�^���X�����߂�
	const TextArea* GetTextArea() const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     �C���^�[�t�F�[�X                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// 2007.08.25 kobake �߂�l�� void �ɕύX�B���� x, y �� DispPos �ɕύX
	// ���ۂɂ� pX �� nX ���X�V�����B
	void DispText(HDC hdc, DispPos* pDispPos, const wchar_t* pData, int nLength, bool bTransparent = false) const; // �e�L�X�g�\��

	//!	�m�[�g���`��
	void DispNoteLine( Graphics& gr, int nTop, int nBottom, int nLeft, int nRight ) const;

	// -- -- �w�茅�c���`�� -- -- //
	// �w�茅�c���`��֐�	// 2005.11.08 Moca
	void DispVerticalLines(Graphics& gr, int nTop, int nBottom, LayoutInt nLeftCol, LayoutInt nRightCol) const;

	// -- -- �܂�Ԃ����c���`�� -- -- //
	void DispWrapLine(Graphics& gr, int nTop, int nBottom) const;

	// -- -- �s�ԍ� -- -- //
	void DispLineNumber(Graphics& gr, LayoutInt nLineNum, int y) const;		// �s�ԍ��\��

private:
	const EditView* m_pEditView;
};

