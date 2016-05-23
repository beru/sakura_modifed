/*!	@file
@brief ViewCommander�N���X�̃R�}���h(�I���n/��`�I���n)�֐��Q

	2012/12/20	ViewCommander.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2005, Moca
	Copyright (C) 2007, kobake, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"


// ���݈ʒu�̒P��I��
bool ViewCommander::Command_SelectWord(const Point* pptCaretPos)
{
	auto& si = view.GetSelectionInfo();
	if (si.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// ���݂̑I��͈͂��I����Ԃɖ߂�
		si.DisableSelectArea(true);
	}
	auto& caret = GetCaret();
	Point ptCaretPos = ((!pptCaretPos) ? caret.GetCaretLayoutPos() : *pptCaretPos);
	auto& layoutMgr = GetDocument().layoutMgr;
	const Layout* pLayout = layoutMgr.SearchLineByLayoutY(ptCaretPos.y);
	if (!pLayout) {
		return false;	// �P��I���Ɏ��s
	}
	// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
	int nIdx = view.LineColumnToIndex(pLayout, ptCaretPos.x);

	// ���݈ʒu�̒P��͈̔͂𒲂ׂ�
	Range range;
	if (layoutMgr.WhereCurrentWord(ptCaretPos.y, nIdx, &range, NULL, NULL)) {

		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
		// 2007.10.15 kobake ���Ƀ��C�A�E�g�P�ʂȂ̂ŕϊ��͕s�v
		/*
		pLayout = layoutMgr.SearchLineByLayoutY(range.GetFrom().y);
		range.SetFromX(view.LineIndexToColumn(pLayout, range.GetFrom().x));
		pLayout = layoutMgr.SearchLineByLayoutY(range.GetTo().y);
		range.SetToX(view.LineIndexToColumn(pLayout, range.GetTo().x));
		*/

		// �I��͈͂̕ύX
		// 2005.06.24 Moca
		si.SetSelectArea(range);
		// �I��̈�`��
		si.DrawSelectArea();

		// �P��̐擪�ɃJ�[�\�����ړ�
		caret.MoveCursor(range.GetTo(), true);
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
		return true;	// �P��I���ɐ����B
	}else {
		return false;	// �P��I���Ɏ��s
	}
}


// ���ׂđI��
void ViewCommander::Command_SelectAll(void)
{
	auto& si = view.GetSelectionInfo();
	if (si.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// ���݂̑I��͈͂��I����Ԃɖ߂�
		si.DisableSelectArea(true);
	}

	// �擪�փJ�[�\�����ړ�
	// Sep. 8, 2000 genta
	view.AddCurrentLineToHistory();
	GetCaret().nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;

	// Jul. 29, 2006 genta �I���ʒu�̖����𐳊m�Ɏ擾����
	// �}�N������擾�����ꍇ�ɐ������͈͂��擾�ł��Ȃ�����
	//int nX, nY;
	Range range;
	range.SetFrom(Point(0, 0));
	GetDocument().layoutMgr.GetEndLayoutPos(range.GetToPointer());
	si.SetSelectArea(range);

	// �I��̈�`��
	si.DrawSelectArea(false);
}


/*!	1�s�I��
	@brief �J�[�\���ʒu��1�s�I������
	@param lparam [in] �}�N������g�p����g���t���O�i�g���p�ɗ\��j

	note ���s�P�ʂőI�����s���B

	@date 2007.11.15 nasukoji	�V�K�쐬
*/
void ViewCommander::Command_SelectLine(int lparam)
{
	// ���s�P�ʂ�1�s�I������
	Command_GoLineTop(false, 0x9);	// �����s���Ɉړ�

	auto& si = view.GetSelectionInfo();
	si.bBeginLineSelect = true;		// �s�P�ʑI��

	Point ptCaret;

	auto& layoutMgr = GetDocument().layoutMgr;
	auto& caret = GetCaret();
	// �ŉ��s�i�����s�j�łȂ�
	if (caret.GetCaretLogicPos().y < GetDocument().docLineMgr.GetLineCount()) {
		// 1�s��̕����s���烌�C�A�E�g�s�����߂�
		layoutMgr.LogicToLayout(Point(0, caret.GetCaretLogicPos().y + 1), &ptCaret);

		// �J�[�\�������̕����s���ֈړ�����
		view.MoveCursorSelecting(ptCaret, true);

		// �ړ���̃J�[�\���ʒu���擾����
		ptCaret = caret.GetCaretLayoutPos().Get();
	}else {
		// �J�[�\�����ŉ��s�i���C�A�E�g�s�j�ֈړ�����
		view.MoveCursorSelecting(Point(0, layoutMgr.GetLineCount()), true);
		Command_GoLineEnd(true, 0, 0);	// �s���Ɉړ�

		// �I��������̂������i[EOF]�݂̂̍s�j���͑I����ԂƂ��Ȃ�
		if (
			!si.IsTextSelected()
			&& (caret.GetCaretLogicPos().y >= GetDocument().docLineMgr.GetLineCount())
		) {
			// ���݂̑I��͈͂��I����Ԃɖ߂�
			si.DisableSelectArea(true);
		}
	}
	
	if (si.bBeginLineSelect) {
		// �͈͑I���J�n�s�E�J�������L��
		si.select.SetTo(ptCaret);
		si.selectBgn.SetTo(ptCaret);
	}

	return;
}

// �͈͑I���J�n
void ViewCommander::Command_Begin_Select(void)
{
	auto& si = view.GetSelectionInfo();
	if (!si.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// ���݂̃J�[�\���ʒu����I�����J�n����
		si.BeginSelectArea();
	}

	// ���b�N�̉����؂�ւ�
	if (si.bSelectingLock) {
		si.bSelectingLock = false;	// �I����Ԃ̃��b�N����
	}else {
		si.bSelectingLock = true;		// �I����Ԃ̃��b�N
	}
	if (GetSelect().IsOne()) {
		GetCaret().underLine.CaretUnderLineOFF(true);
	}
	si.PrintSelectionInfoMsg();
	return;
}


// ��`�͈͑I���J�n
void ViewCommander::Command_Begin_BoxSelect(bool bSelectingLock)
{
	if (!GetDllShareData().common.view.bFontIs_FixedPitch) {	// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
		return;
	}

	auto& si = view.GetSelectionInfo();
//@@@ 2002.01.03 YAZAKI �͈͑I�𒆂�Shift+F6�����s����ƑI��͈͂��N���A����Ȃ����ɑΏ�
	if (si.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// ���݂̑I��͈͂��I����Ԃɖ߂�
		si.DisableSelectArea(true);
	}

	// ���݂̃J�[�\���ʒu����I�����J�n����
	si.BeginSelectArea();

	si.bSelectingLock = bSelectingLock;	// �I����Ԃ̃��b�N
	si.SetBoxSelect(true);	// ��`�͈͑I��

	si.PrintSelectionInfoMsg();
	GetCaret().underLine.CaretUnderLineOFF(true);
	return;
}

