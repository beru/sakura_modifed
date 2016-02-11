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

class EditView;

#include "basis/SakuraBasis.h"
#include "doc/layout/CLayout.h"

class ViewSelect {
public:
	EditView* GetEditView() { return m_pcEditView; }
	const EditView* GetEditView() const { return m_pcEditView; }

public:
	ViewSelect(EditView* pcEditView);
	void CopySelectStatus(ViewSelect* pSelect) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �I��͈͂̕ύX                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void DisableSelectArea(bool bDraw, bool bDrawBracketCursorLine = true); // ���݂̑I��͈͂��I����Ԃɖ߂�

	void BeginSelectArea(const LayoutPoint* po = NULL);								// ���݂̃J�[�\���ʒu����I�����J�n����
	void ChangeSelectAreaByCurrentCursor(const LayoutPoint& ptCaretPos);			// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
	void ChangeSelectAreaByCurrentCursorTEST(const LayoutPoint& ptCaretPos, LayoutRange* pSelect);// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX

	// �I��͈͂��w�肷��(���_���I��)
	// 2005.06.24 Moca
	void SetSelectArea(const LayoutRange& sRange) {
		m_sSelectBgn.Set(sRange.GetFrom());
		m_sSelect = sRange;
	}

	// �P��I���J�n
	void SelectBeginWord() {
		m_bBeginSelect     = true;			// �͈͑I��
		m_bBeginBoxSelect  = false;			// ��`�͈͑I�𒆂łȂ�
		m_bBeginLineSelect = false;			// �s�P�ʑI��
		m_bBeginWordSelect = true;			// �P��P�ʑI��
	}

	// ��`�I���J�n
	void SelectBeginBox() {
		m_bBeginSelect     = true;		// �͈͑I��
		m_bBeginBoxSelect  = true;		// ��`�͈͑I��
		m_bBeginLineSelect = false;		// �s�P�ʑI��
		m_bBeginWordSelect = false;		// �P��P�ʑI��
	}

	// ��̑I���J�n
	void SelectBeginNazo() {
		m_bBeginSelect     = true;		// �͈͑I��
//		m_bBeginBoxSelect  = false;		// ��`�͈͑I�𒆂łȂ�
		m_bBeginLineSelect = false;		// �s�P�ʑI��
		m_bBeginWordSelect = false;		// �P��P�ʑI��
	}

	// �͈͑I���I��
	void SelectEnd() {
		m_bBeginSelect = false;
	}

	// m_bBeginBoxSelect��ݒ�B
	void SetBoxSelect(bool b) {
		m_bBeginBoxSelect = b;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �`��                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void DrawSelectArea(bool bDrawBracketCursorLine = true);		// �w��s�̑I��̈�̕`��
private:
	void DrawSelectArea2(HDC) const;	// �w��͈͂̑I��̈�̕`��
	void DrawSelectAreaLine(			// �w��s�̑I��̈�̕`��
		HDC					hdc,		// [in] �`��̈��Device Context Handle
		LayoutInt			nLineNum,	// [in] �`��Ώۍs(���C�A�E�g�s)
		const LayoutRange&	sRange		// [in] �I��͈�(���C�A�E�g�P��)
	) const;
public:
	void GetSelectAreaLineFromRange(LayoutRange& ret, LayoutInt nLineNum, const Layout* pcLayout, const LayoutRange& sRange) const;
	void GetSelectAreaLine(LayoutRange& ret, LayoutInt nLineNum, const Layout* pcLayout) const {
		GetSelectAreaLineFromRange(ret, nLineNum, pcLayout, m_sSelect);
	}
	LayoutRange GetSelectAreaLine(LayoutInt nLineNum, const Layout* pcLayout) const {
		LayoutRange ret;
		GetSelectAreaLineFromRange(ret, nLineNum, pcLayout, m_sSelect);
		return ret;
	}
	// �I�����f�[�^�̍쐬	2005.07.09 genta
	void PrintSelectionInfoMsg() const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         ��Ԏ擾                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �e�L�X�g���I������Ă��邩
	// 2002/03/29 Azumaiya �C�����C���֐���
	bool IsTextSelected() const {
		return m_sSelect.IsValid();
//		return 0 != (
//			~((DWORD)(m_sSelect.nLineFrom | m_sSelect.nLineTo | m_sSelect.nColumnFrom | m_sSelect.nColumnTo)) >> 31
//			);
	}

	// �e�L�X�g�̑I�𒆂�
	// 2002/03/29 Azumaiya �C�����C���֐���
	bool IsTextSelecting() const {
		// �W�����v�񐔂����炵�āA��C�ɔ���B
		return m_bSelectingLock || IsTextSelected();
	}

	// �}�E�X�őI�𒆂�
	bool IsMouseSelecting() const {
		return m_bBeginSelect;
	}

	// ��`�I�𒆂�
	bool IsBoxSelecting() const {
		return m_bBeginBoxSelect;
	}

private:
	// �Q��
	EditView*	m_pcEditView;

public:

	bool	m_bDrawSelectArea;		// �I��͈͂�`�悵����	// 02/12/13 ai

	// �I�����
	bool	m_bSelectingLock;		// �I����Ԃ̃��b�N
private:
	bool	m_bBeginSelect;			// �͈͑I��
	bool	m_bBeginBoxSelect;		// ��`�͈͑I��
	bool	m_bSelectAreaChanging;	// �I��͈͕ύX��
	int		m_nLastSelectedByteLen;	// �O��I�����̑I���o�C�g��

public:
	bool	m_bBeginLineSelect;		// �s�P�ʑI��
	bool	m_bBeginWordSelect;		// �P��P�ʑI��

	// �I��͈͂�ێ����邽�߂̕ϐ��Q
	// �����͂��ׂĐ܂�Ԃ��s�ƁA�܂�Ԃ�����ێ����Ă���B
	LayoutRange m_sSelectBgn; // �͈͑I��(���_)
	LayoutRange m_sSelect;    // �͈͑I��
	LayoutRange m_sSelectOld; // �͈͑I��Old

	Point	m_ptMouseRollPosOld;	// �}�E�X�͈͑I��O��ʒu(XY���W)
};

/*
m_sSelectOld�ɂ���
	DrawSelectArea()�Ɍ��݂̑I��͈͂������č����̂ݕ`�悷�邽�߂̂���
	���݂̑I��͈͂�Old�փR�s�[������ŐV�����I��͈͂�Select�ɐݒ肵��
	DrawSelectArea()���Ăт������ƂŐV�����͈͂��`�����D
*/

