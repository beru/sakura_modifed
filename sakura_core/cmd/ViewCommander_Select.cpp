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
bool ViewCommander::Command_SELECTWORD(LayoutPoint* pptCaretPos)
{
	auto& si = m_view.GetSelectionInfo();
	if (si.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// ���݂̑I��͈͂��I����Ԃɖ߂�
		si.DisableSelectArea(true);
	}
	auto& caret = GetCaret();
	LayoutPoint ptCaretPos = ((!pptCaretPos) ? caret.GetCaretLayoutPos() : *pptCaretPos);
	auto& layoutMgr = GetDocument()->m_layoutMgr;
	const Layout* pLayout = layoutMgr.SearchLineByLayoutY(ptCaretPos.GetY2());
	if (!pLayout) {
		return false;	// �P��I���Ɏ��s
	}
	// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
	LogicInt nIdx = m_view.LineColumnToIndex(pLayout, ptCaretPos.GetX2());

	// ���݈ʒu�̒P��͈̔͂𒲂ׂ�
	LayoutRange range;
	if (layoutMgr.WhereCurrentWord(ptCaretPos.GetY2(), nIdx, &range, NULL, NULL)) {

		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
		// 2007.10.15 kobake ���Ƀ��C�A�E�g�P�ʂȂ̂ŕϊ��͕s�v
		/*
		pLayout = layoutMgr.SearchLineByLayoutY(range.GetFrom().GetY2());
		range.SetFromX(m_view.LineIndexToColumn(pLayout, range.GetFrom().x));
		pLayout = layoutMgr.SearchLineByLayoutY(range.GetTo().GetY2());
		range.SetToX(m_view.LineIndexToColumn(pLayout, range.GetTo().x));
		*/

		// �I��͈͂̕ύX
		// 2005.06.24 Moca
		si.SetSelectArea(range);
		// �I��̈�`��
		si.DrawSelectArea();

		// �P��̐擪�ɃJ�[�\�����ړ�
		caret.MoveCursor(range.GetTo(), true);
		caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
		return true;	// �P��I���ɐ����B
	}else {
		return false;	// �P��I���Ɏ��s
	}
}


// ���ׂđI��
void ViewCommander::Command_SELECTALL(void)
{
	auto& si = m_view.GetSelectionInfo();
	if (si.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// ���݂̑I��͈͂��I����Ԃɖ߂�
		si.DisableSelectArea(true);
	}

	// �擪�փJ�[�\�����ړ�
	// Sep. 8, 2000 genta
	m_view.AddCurrentLineToHistory();
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	// Jul. 29, 2006 genta �I���ʒu�̖����𐳊m�Ɏ擾����
	// �}�N������擾�����ꍇ�ɐ������͈͂��擾�ł��Ȃ�����
	//int nX, nY;
	LayoutRange range;
	range.SetFrom(LayoutPoint(0, 0));
	GetDocument()->m_layoutMgr.GetEndLayoutPos(range.GetToPointer());
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
void ViewCommander::Command_SELECTLINE(int lparam)
{
	// ���s�P�ʂ�1�s�I������
	Command_GOLINETOP(false, 0x9);	// �����s���Ɉړ�

	auto& si = m_view.GetSelectionInfo();
	si.m_bBeginLineSelect = true;		// �s�P�ʑI��

	LayoutPoint ptCaret;

	auto& layoutMgr = GetDocument()->m_layoutMgr;
	auto& caret = GetCaret();
	// �ŉ��s�i�����s�j�łȂ�
	if (caret.GetCaretLogicPos().y < GetDocument()->m_docLineMgr.GetLineCount()) {
		// 1�s��̕����s���烌�C�A�E�g�s�����߂�
		layoutMgr.LogicToLayout(LogicPoint(0, caret.GetCaretLogicPos().y + 1), &ptCaret);

		// �J�[�\�������̕����s���ֈړ�����
		m_view.MoveCursorSelecting(ptCaret, TRUE);

		// �ړ���̃J�[�\���ʒu���擾����
		ptCaret = caret.GetCaretLayoutPos().Get();
	}else {
		// �J�[�\�����ŉ��s�i���C�A�E�g�s�j�ֈړ�����
		m_view.MoveCursorSelecting(LayoutPoint(LayoutInt(0), layoutMgr.GetLineCount()), TRUE);
		Command_GOLINEEND(true, 0, 0);	// �s���Ɉړ�

		// �I��������̂������i[EOF]�݂̂̍s�j���͑I����ԂƂ��Ȃ�
		if (
			!si.IsTextSelected()
			&& (caret.GetCaretLogicPos().y >= GetDocument()->m_docLineMgr.GetLineCount())
		) {
			// ���݂̑I��͈͂��I����Ԃɖ߂�
			si.DisableSelectArea(true);
		}
	}
	
	if (si.m_bBeginLineSelect) {
		// �͈͑I���J�n�s�E�J�������L��
		si.m_select.SetTo(ptCaret);
		si.m_selectBgn.SetTo(ptCaret);
	}

	return;
}

// �͈͑I���J�n
void ViewCommander::Command_BEGIN_SELECT(void)
{
	auto& si = m_view.GetSelectionInfo();
	if (!si.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// ���݂̃J�[�\���ʒu����I�����J�n����
		si.BeginSelectArea();
	}

	// ���b�N�̉����؂�ւ�
	if (si.m_bSelectingLock) {
		si.m_bSelectingLock = false;	// �I����Ԃ̃��b�N����
	}else {
		si.m_bSelectingLock = true;		// �I����Ԃ̃��b�N
	}
	if (GetSelect().IsOne()) {
		GetCaret().m_underLine.CaretUnderLineOFF(true);
	}
	si.PrintSelectionInfoMsg();
	return;
}


// ��`�͈͑I���J�n
void ViewCommander::Command_BEGIN_BOXSELECT(bool bSelectingLock)
{
	if (!GetDllShareData().common.view.bFontIs_FixedPitch) {	// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
		return;
	}

	auto& si = m_view.GetSelectionInfo();
//@@@ 2002.01.03 YAZAKI �͈͑I�𒆂�Shift+F6�����s����ƑI��͈͂��N���A����Ȃ����ɑΏ�
	if (si.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// ���݂̑I��͈͂��I����Ԃɖ߂�
		si.DisableSelectArea(true);
	}

	// ���݂̃J�[�\���ʒu����I�����J�n����
	si.BeginSelectArea();

	si.m_bSelectingLock = bSelectingLock;	// �I����Ԃ̃��b�N
	si.SetBoxSelect(true);	// ��`�͈͑I��

	si.PrintSelectionInfoMsg();
	GetCaret().m_underLine.CaretUnderLineOFF(true);
	return;
}

