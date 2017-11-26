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
	// �I�����f�[�^�̍쐬
	void PrintSelectionInfoMsg() const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         ��Ԏ擾                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �e�L�X�g���I������Ă��邩
	bool IsTextSelected() const {
		return select.IsValid();
//		return 0 != (
//			~((DWORD)(sSelect.nLineFrom | m_sSelect.nLineTo | m_sSelect.nColumnFrom | m_sSelect.nColumnTo)) >> 31
//			);
	}

	// �e�L�X�g�̑I�𒆂�
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

	bool	bDrawSelectArea;		// �I��͈͂�`�悵����

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

