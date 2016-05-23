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

class ViewFont;
class EditView;
class LayoutMgr;
#include "DispPos.h"

class TextArea {
public:
	TextArea(EditView& editView);
	
	virtual
	~TextArea();
	
	void CopyTextAreaStatus(TextArea* pDst) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     �r���[�����擾                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// �\�������ŏ��̍s
	int GetViewTopLine() const {
		return nViewTopLine;
	}
	void SetViewTopLine(int nLine) {
		nViewTopLine = nLine;
	}

	// �\����̈�ԍ��̌�
	int GetViewLeftCol() const {
		return nViewLeftCol;
	}
	void SetViewLeftCol(int nLeftCol) {
		nViewLeftCol = nLeftCol;
	}

	// �E�ɂ͂ݏo�����ŏ��̗��Ԃ�
	int GetRightCol() const {
		return nViewLeftCol + nViewColNum;
	}

	// ���ɂ͂ݏo�����ŏ��̍s��Ԃ�
	int GetBottomLine() const {
		return nViewTopLine + nViewRowNum;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                   �̈���擾(�s�N�Z��)                      //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	int GetAreaLeft() const {
		return nViewAlignLeft;
	}
	int GetAreaTop() const {
		return nViewAlignTop;
	}
	int GetAreaRight() const {
		return nViewAlignLeft + nViewCx;
	}
	int GetAreaBottom() const {
		return nViewAlignTop + nViewCy;
	}
	Rect GetAreaRect() const {
		return Rect(GetAreaLeft(), GetAreaTop(), GetAreaRight(), GetAreaBottom());
	}

	int GetAreaWidth() const {
		return nViewCx;
	}
	int GetAreaHeight() const {
		return nViewCy;
	}

	int GetTopYohaku() const {
		return nTopYohaku;
	}
	void SetTopYohaku(int nPixel) {
		nTopYohaku = nPixel;
	}
	int GetLeftYohaku() const {
		return nLeftYohaku;
	}
	void SetLeftYohaku(int nPixel) {
		nLeftYohaku = nPixel;
	}
	// �s�ԍ��̕�(�]���Ȃ�)
	int GetLineNumberWidth() const {
		return nViewAlignLeft - nLeftYohaku;
	}

	// �N���C�A���g�T�C�Y�X�V
	void TextArea_OnSize(
		const Size& sizeClient,		// �E�B���h�E�̃N���C�A���g�T�C�Y
		int nCxVScroll,				// �����X�N���[���o�[�̉���
		int nCyHScroll				// �����X�N���[���o�[�̏c��
	);

	// �s�ԍ��\���ɕK�v�ȕ���ݒ�
	bool DetectWidthOfLineNumberArea(bool bRedraw);

	// �s�ԍ��\���ɕK�v�Ȍ������v�Z
	int  DetectWidthOfLineNumberArea_calculate(const LayoutMgr*, bool bLayout=false) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ����                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	bool IsRectIntersected(const RECT& rc) const {
		// rc�������܂��̓[���̈�̏ꍇ��false
		if (rc.left >= rc.right) return false;
		if (rc.top  >= rc.bottom) return false;

		if (rc.left >= this->GetAreaRight()) return false; // �E�O
		if (rc.right <= this->GetAreaLeft()) return false; // ���O
		if (rc.top >= this->GetAreaBottom()) return false; // ���O
		if (rc.bottom <= this->GetAreaTop()) return false; // ��O
		
		return true;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        ���̑��擾                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	int GetRulerHeight() const {
		return nViewAlignTop - GetTopYohaku();
	}
	// �h�L�������g���[�̃N���C�A���g���W���擾 (�܂�A�X�N���[�����ꂽ��Ԃł���΁A�}�C�i�X��Ԃ�)
	int GetDocumentLeftClientPointX() const;

	// �v�Z
	// ! �N���C�A���g���W���烌�C�A�E�g�ʒu�ɕϊ�����
	void ClientToLayout(Point ptClient, LayoutPoint* pptLayout) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ݒ�                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void UpdateAreaMetrics(HDC hdc);
	void SetAreaLeft(int nAreaLeft) {
		nViewAlignLeft = nAreaLeft;
	}
	void SetAreaTop(int nAreaTop) {
		nViewAlignTop = nAreaTop;
	}
	void OffsetViewTopLine(int nOff) {
		nViewTopLine += nOff;
	}
protected:
	void UpdateViewColRowNums();

public:


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �T�|�[�g                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//$ Generate�Ȃ�Ă����傰���Ȗ��O����Ȃ��āAGet�`�ŗǂ��C�����Ă���
	// �N���b�s���O��`���쐬�B�\���͈͊O�������ꍇ��false��Ԃ��B
	void GenerateCharRect(RECT* rc, const DispPos& pos, int nHankakuNum) const;
	bool TrimRectByArea(RECT* rc) const;
	bool GenerateClipRect(RECT* rc, const DispPos& pos, int nHankakuNum) const;
	bool GenerateClipRectRight(RECT* rc, const DispPos& pos) const; // �E�[�܂őS��
	bool GenerateClipRectLine(RECT* rc, const DispPos& pos) const;  // �s�S��

	void GenerateTopRect   (RECT* rc, int nLineCount) const;
	void GenerateBottomRect(RECT* rc, int nLineCount) const;
	void GenerateLeftRect  (RECT* rc, int nColCount) const;
	void GenerateRightRect (RECT* rc, int nColCount) const;

	void GenerateLineNumberRect(RECT* rc) const;

	void GenerateTextAreaRect(RECT* rc) const;

	int GenerateYPx(int nLineNum) const;

private:
	// �Q��
	EditView&	editView;

public:
	// ��ʏ��
	// �s�N�Z��
private:
	int		nViewAlignLeft;		// �\����̍��[���W
	int		nViewAlignTop;		// �\����̏�[���W
private:
	int		nTopYohaku;
	int		nLeftYohaku;
private:
	int		nViewCx;				// �\����̕�
	int		nViewCy;				// �\����̍���

	// �e�L�X�g
private:
	int	nViewTopLine;		// �\����̈�ԏ�̍s(0�J�n)
public:
	int	nViewRowNum;		// �\����̍s��

private:
	int	nViewLeftCol;		// �\����̈�ԍ��̌�(0�J�n)
public:
	int	nViewColNum;		// �\����̌���

	// ���̑�
	int		nViewAlignLeftCols;	// �s�ԍ���̌���
};

