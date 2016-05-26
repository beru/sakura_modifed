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
#include "doc/layout/Layout.h"

class ViewSelect {
public:
	EditView& GetEditView() { return editView; }
	const EditView& GetEditView() const { return editView; }

public:
	ViewSelect(EditView& editView);
	void CopySelectStatus(ViewSelect* pSelect) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �I��͈͂̕ύX                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void DisableSelectArea(bool bDraw, bool bDrawBracketCursorLine = true); // ���݂̑I��͈͂��I����Ԃɖ߂�

	void BeginSelectArea(const Point* po = nullptr);					// ���݂̃J�[�\���ʒu����I�����J�n����
	void ChangeSelectAreaByCurrentCursor(const Point& ptCaretPos);	// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
	void ChangeSelectAreaByCurrentCursorTEST(
		const Point& ptCaretPos,
		Range* pSelect);	// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX

	// �I��͈͂��w�肷��(���_���I��)
	// 2005.06.24 Moca
	void SetSelectArea(const Range& range) {
		selectBgn.Set(range.GetFrom());
		select = range;
	}

	// �P��I���J�n
	void SelectBeginWord() {
		bBeginSelect     = true;			// �͈͑I��
		bBeginBoxSelect  = false;			// ��`�͈͑I�𒆂łȂ�
		bBeginLineSelect = false;			// �s�P�ʑI��
		bBeginWordSelect = true;			// �P��P�ʑI��
	}

	// ��`�I���J�n
	void SelectBeginBox() {
		bBeginSelect     = true;		// �͈͑I��
		bBeginBoxSelect  = true;		// ��`�͈͑I��
		bBeginLineSelect = false;		// �s�P�ʑI��
		bBeginWordSelect = false;		// �P��P�ʑI��
	}

	// ��̑I���J�n
	void SelectBeginNazo() {
		bBeginSelect     = true;		// �͈͑I��
//		bBeginBoxSelect  = false;		// ��`�͈͑I�𒆂łȂ�
		bBeginLineSelect = false;		// �s�P�ʑI��
		bBeginWordSelect = false;		// �P��P�ʑI��
	}

	// �͈͑I���I��
	void SelectEnd() {
		bBeginSelect = false;
	}

	// bBeginBoxSelect��ݒ�B
	void SetBoxSelect(bool b) {
		bBeginBoxSelect = b;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �`��                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void DrawSelectArea(bool bDrawBracketCursorLine = true);		// �w��s�̑I��̈�̕`��
private:
	void DrawSelectArea2(HDC) const;	// �w��͈͂̑I��̈�̕`��
	void DrawSelectAreaLine(			// �w��s�̑I��̈�̕`��
		HDC hdc,				// [in] �`��̈��Device Context Handle
		int nLineNum,			// [in] �`��Ώۍs(���C�A�E�g�s)
		const Range& range		// [in] �I��͈�(���C�A�E�g�P��)
	) const;
public:
	void GetSelectAreaLineFromRange(
		Range& ret,
		int nLineNum,
		const Layout* pLayout,
		const Range& range) const;
	void GetSelectAreaLine(Range& ret, int nLineNum, const Layout* pLayout) const {
		GetSelectAreaLineFromRange(ret, nLineNum, pLayout, select);
	}
	Range GetSelectAreaLine(int nLineNum, const Layout* pLayout) const {
		Range ret;
		GetSelectAreaLineFromRange(ret, nLineNum, pLayout, select);
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
		return select.IsValid();
//		return 0 != (
//			~((DWORD)(sSelect.nLineFrom | m_sSelect.nLineTo | m_sSelect.nColumnFrom | m_sSelect.nColumnTo)) >> 31
//			);
	}

	// �e�L�X�g�̑I�𒆂�
	// 2002/03/29 Azumaiya �C�����C���֐���
	bool IsTextSelecting() const {
		// �W�����v�񐔂����炵�āA��C�ɔ���B
		return bSelectingLock || IsTextSelected();
	}

	// �}�E�X�őI�𒆂�
	bool IsMouseSelecting() const {
		return bBeginSelect;
	}

	// ��`�I�𒆂�
	bool IsBoxSelecting() const {
		return bBeginBoxSelect;
	}

private:
	// �Q��
	EditView&	editView;

public:

	bool	bDrawSelectArea;		// �I��͈͂�`�悵����	// 02/12/13 ai

	// �I�����
	bool	bSelectingLock;		// �I����Ԃ̃��b�N
private:
	bool	bBeginSelect;			// �͈͑I��
	bool	bBeginBoxSelect;		// ��`�͈͑I��
	bool	bSelectAreaChanging;	// �I��͈͕ύX��
	size_t	nLastSelectedByteLen;	// �O��I�����̑I���o�C�g��

public:
	bool	bBeginLineSelect;		// �s�P�ʑI��
	bool	bBeginWordSelect;		// �P��P�ʑI��

	// �I��͈͂�ێ����邽�߂̕ϐ��Q
	// �����͂��ׂĐ܂�Ԃ��s�ƁA�܂�Ԃ�����ێ����Ă���B
	Range selectBgn; // �͈͑I��(���_)
	Range select;    // �͈͑I��
	Range selectOld; // �͈͑I��Old

	Point	ptMouseRollPosOld;	// �}�E�X�͈͑I��O��ʒu(XY���W)
};

/*
sSelectOld�ɂ���
	DrawSelectArea()�Ɍ��݂̑I��͈͂������č����̂ݕ`�悷�邽�߂̂���
	���݂̑I��͈͂�Old�փR�s�[������ŐV�����I��͈͂�Select�ɐݒ肵��
	DrawSelectArea()���Ăт������ƂŐV�����͈͂��`�����D
*/

