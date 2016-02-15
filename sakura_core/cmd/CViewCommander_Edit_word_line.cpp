/*!	@file
@brief ViewCommander�N���X�̃R�}���h(�ҏW�n �P��/�s�P��)�֐��Q

	2012/12/16	CViewCommander.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2003, �����
	Copyright (C) 2005, Moca
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"


// �P��̍��[�܂ō폜
void ViewCommander::Command_WordDeleteToStart(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	// ��`�I����Ԃł͎��s�s�\(��������蔲������)
	if (selInfo.IsTextSelected()) {
		// ��`�͈͑I�𒆂�
		if (selInfo.IsBoxSelecting()) {
			ErrorBeep();
			return;
		}
	}

	// �P��̍��[�Ɉړ�
	ViewCommander::Command_WORDLEFT(true);
	if (!selInfo.IsTextSelected()) {
		ErrorBeep();
		return;
	}

	if (!m_pCommanderView->m_bDoing_UndoRedo) {	// �A���h�D�E���h�D�̎��s����
		MoveCaretOpe*	pOpe = new MoveCaretOpe();
		GetDocument()->m_layoutMgr.LayoutToLogic(
			GetSelect().GetTo(),
			&pOpe->m_ptCaretPos_PHY_Before
		);
		pOpe->m_ptCaretPos_PHY_After = pOpe->m_ptCaretPos_PHY_Before;	// �����̃L�����b�g�ʒu

		// ����̒ǉ�
		GetOpeBlk()->AppendOpe(pOpe);
	}

	// �폜
	m_pCommanderView->DeleteData(true);
}



// �P��̉E�[�܂ō폜
void ViewCommander::Command_WordDeleteToEnd(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();

	// ��`�I����Ԃł͎��s�s�\((��������蔲������))
	if (selInfo.IsTextSelected()) {
		// ��`�͈͑I�𒆂�
		if (selInfo.IsBoxSelecting()) {
			ErrorBeep();
			return;
		}
	}
	// �P��̉E�[�Ɉړ�
	ViewCommander::Command_WORDRIGHT(true);
	if (!selInfo.IsTextSelected()) {
		ErrorBeep();
		return;
	}
	if (!m_pCommanderView->m_bDoing_UndoRedo) {	// �A���h�D�E���h�D�̎��s����
		MoveCaretOpe*	pOpe = new MoveCaretOpe();
		GetDocument()->m_layoutMgr.LayoutToLogic(
			GetSelect().GetFrom(),
			&pOpe->m_ptCaretPos_PHY_Before
		);
		pOpe->m_ptCaretPos_PHY_After = pOpe->m_ptCaretPos_PHY_Before;	// �����̃L�����b�g�ʒu
		// ����̒ǉ�
		GetOpeBlk()->AppendOpe(pOpe);
	}
	// �폜
	m_pCommanderView->DeleteData(true);
}


// �P��؂���
void ViewCommander::Command_WordCut(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	if (selInfo.IsTextSelected()) {
		// �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
		Command_CUT();
		return;
	}
	// ���݈ʒu�̒P��I��
	Command_SELECTWORD();
	// �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
	if (!selInfo.IsTextSelected()) {
		// �P��I���őI���ł��Ȃ�������A���̕�����I�Ԃ��Ƃɒ���B
		Command_RIGHT(true, false, false);
	}
	Command_CUT();
	return;
}


// �P��폜
void ViewCommander::Command_WordDelete(void)
{
	if (m_pCommanderView->GetSelectionInfo().IsTextSelected()) {
		// �폜
		m_pCommanderView->DeleteData(true);
		return;
	}
	// ���݈ʒu�̒P��I��
	Command_SELECTWORD();
	// �폜
	m_pCommanderView->DeleteData(true);
	return;
}


// �s���܂Ő؂���(���s�P��)
void ViewCommander::Command_LineCutToStart(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	Layout* pLayout;
	if (selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
		Command_CUT();
		return;
	}
	pLayout = GetDocument()->m_layoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().GetY2());	// �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(Layout)�ւ̃|�C���^��Ԃ�
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	LayoutPoint ptPos;
	GetDocument()->m_layoutMgr.LogicToLayout(LogicPoint(0, pLayout->GetLogicLineNo()), &ptPos);
	if (GetCaret().GetCaretLayoutPos() == ptPos) {
		ErrorBeep();
		return;
	}

	// �I��͈͂̕ύX
	// 2005.06.24 Moca
	LayoutRange range(ptPos, GetCaret().GetCaretLayoutPos());
	selInfo.SetSelectArea(range);

	// �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
	Command_CUT();
}



// �s���܂Ő؂���(���s�P��)
void ViewCommander::Command_LineCutToEnd(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	Layout* pLayout;
	if (selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
		Command_CUT();
		return;
	}
	// �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(Layout)�ւ̃|�C���^��Ԃ�
	pLayout = GetDocument()->m_layoutMgr.SearchLineByLayoutY(
		GetCaret().GetCaretLayoutPos().GetY2()
	);
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	LayoutPoint ptPos;
	auto& docLineRef = *pLayout->GetDocLineRef();
	if (docLineRef.GetEol() == EolType::None) {	// ���s�R�[�h�̎��
		GetDocument()->m_layoutMgr.LogicToLayout(
			LogicPoint(
				docLineRef.GetLengthWithEOL(),
				pLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}else {
		GetDocument()->m_layoutMgr.LogicToLayout(
			LogicPoint(
				docLineRef.GetLengthWithEOL() - docLineRef.GetEol().GetLen(),
				pLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}

	if (GetCaret().GetCaretLayoutPos().GetY2() == ptPos.y && GetCaret().GetCaretLayoutPos().GetX2() >= ptPos.x) {
		ErrorBeep();
		return;
	}

	// �I��͈͂̕ύX
	// 2005.06.24 Moca
	LayoutRange range(GetCaret().GetCaretLayoutPos(), ptPos);
	selInfo.SetSelectArea(range);

	// �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
	Command_CUT();
}


// �s���܂ō폜(���s�P��)
void ViewCommander::Command_LineDeleteToStart(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	Layout* pLayout;
	if (selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		m_pCommanderView->DeleteData(true);
		return;
	}
	pLayout = GetDocument()->m_layoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().GetY2());	// �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(Layout)�ւ̃|�C���^��Ԃ�
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	LayoutPoint ptPos;

	GetDocument()->m_layoutMgr.LogicToLayout(LogicPoint(0, pLayout->GetLogicLineNo()), &ptPos);
	if (GetCaret().GetCaretLayoutPos() == ptPos) {
		ErrorBeep();
		return;
	}

	// �I��͈͂̕ύX
	// 2005.06.24 Moca
	LayoutRange range(ptPos, GetCaret().GetCaretLayoutPos());
	selInfo.SetSelectArea(range);

	// �I��̈�폜
	m_pCommanderView->DeleteData(true);
}


// �s���܂ō폜(���s�P��)
void ViewCommander::Command_LineDeleteToEnd(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	Layout* pLayout;
	if (selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		m_pCommanderView->DeleteData(true);
		return;
	}

	auto& caretLayoutPos = GetCaret().GetCaretLayoutPos();
	pLayout = GetDocument()->m_layoutMgr.SearchLineByLayoutY(caretLayoutPos.GetY2());	// �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(Layout)�ւ̃|�C���^��Ԃ�
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	LayoutPoint ptPos;

	auto& docLineRef = *pLayout->GetDocLineRef();
	if (docLineRef.GetEol() == EolType::None) {	// ���s�R�[�h�̎��
		GetDocument()->m_layoutMgr.LogicToLayout(
			LogicPoint(
				docLineRef.GetLengthWithEOL(),
				pLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}else {
		GetDocument()->m_layoutMgr.LogicToLayout(
			LogicPoint(
				docLineRef.GetLengthWithEOL() - docLineRef.GetEol().GetLen(),
				pLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}

	if (caretLayoutPos.GetY2() == ptPos.y
		&& caretLayoutPos.GetX2() >= ptPos.x
	) {
		ErrorBeep();
		return;
	}

	// �I��͈͂̕ύX
	// 2005.06.24 Moca
	LayoutRange range(caretLayoutPos, ptPos);
	selInfo.SetSelectArea(range);

	// �I��̈�폜
	m_pCommanderView->DeleteData(true);
}


// �s�؂���(�܂�Ԃ��P��)
void ViewCommander::Command_CUT_LINE(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// �}�E�X�ɂ��͈͑I��
		ErrorBeep();
		return;
	}

	if (selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		ErrorBeep();
		return;
	}

	const Layout* pLayout = GetDocument()->m_layoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().y);
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	// 2007.10.04 ryoji �����ȑf��
	m_pCommanderView->CopyCurLine(
		GetDllShareData().m_common.m_edit.m_bAddCRLFWhenCopy,
		EolType::Unknown,
		GetDllShareData().m_common.m_edit.m_bEnableLineModePaste
	);
	Command_DELETE_LINE();
	return;
}


// �s�폜(�܂�Ԃ��P��)
void ViewCommander::Command_DELETE_LINE(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// �}�E�X�ɂ��͈͑I��
		ErrorBeep();
		return;
	}

	if (selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		ErrorBeep();
		return;
	}
	const Layout* pLayout = GetDocument()->m_layoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().GetY2());
	if (!pLayout) {
		ErrorBeep();
		return;
	}
	GetSelect().SetFrom(LayoutPoint(LayoutInt(0), GetCaret().GetCaretLayoutPos().GetY2()  ));	// �͈͑I���J�n�ʒu
	GetSelect().SetTo  (LayoutPoint(LayoutInt(0), GetCaret().GetCaretLayoutPos().GetY2() + 1));	// �͈͑I���I���ʒu

	LayoutPoint ptCaretPos_OLD = GetCaret().GetCaretLayoutPos();

	Command_DELETE();
	pLayout = GetDocument()->m_layoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().GetY2());
	if (pLayout) {
		// 2003-04-30 �����
		// �s�폜������A�t���[�J�[�\���łȂ��̂ɃJ�[�\���ʒu���s�[���E�ɂȂ�s��Ή�
		// �t���[�J�[�\�����[�h�łȂ��ꍇ�́A�J�[�\���ʒu�𒲐�����
		if (!GetDllShareData().m_common.m_general.m_bIsFreeCursorMode) {
			LogicInt nIndex;

			LayoutInt tmp;
			nIndex = m_pCommanderView->LineColumnToIndex2(pLayout, ptCaretPos_OLD.GetX2(), &tmp);
			ptCaretPos_OLD.x = tmp;

			if (ptCaretPos_OLD.x > 0) {
				ptCaretPos_OLD.x--;
			}else {
				ptCaretPos_OLD.x = m_pCommanderView->LineIndexToColumn(pLayout, nIndex);
			}
		}
		// ����O�̈ʒu�փJ�[�\�����ړ�
		GetCaret().MoveCursor(ptCaretPos_OLD, true);
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if (!m_pCommanderView->m_bDoing_UndoRedo) {	// �A���h�D�E���h�D�̎��s����
			// ����̒ǉ�
			GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					GetCaret().GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
				)
			);
		}
	}
	return;
}


// �s�̓�d��(�܂�Ԃ��P��)
void ViewCommander::Command_DUPLICATELINE(void)
{
	int			bCRLF;
	int			bAddCRLF;
	NativeW		memBuf;

	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	if (selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// ���݂̑I��͈͂��I����Ԃɖ߂�
		selInfo.DisableSelectArea(true);
	}

	const Layout* pLayout = GetDocument()->m_layoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().GetY2());
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	if (!m_pCommanderView->m_bDoing_UndoRedo) {	// �A���h�D�E���h�D�̎��s����
		// ����̒ǉ�
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				GetCaret().GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
			)
		);
	}

	LayoutPoint ptCaretPosOld = GetCaret().GetCaretLayoutPos() + LayoutPoint(0, 1);

	// �s���Ɉړ�(�܂�Ԃ��P��)
	Command_GOLINETOP(selInfo.m_bSelectingLock, 0x1 /* �J�[�\���ʒu�Ɋ֌W�Ȃ��s���Ɉړ� */);

	if (!m_pCommanderView->m_bDoing_UndoRedo) {	// �A���h�D�E���h�D�̎��s����
		// ����̒ǉ�
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				GetCaret().GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
			)
		);
	}


	/* ��d���������s�𒲂ׂ�
	||	�E���s�ŏI����Ă���
	||	�E���s�ŏI����Ă��Ȃ�
	||	�E�ŏI�s�ł���
	||	���܂�Ԃ��łȂ�
	||	�E�ŏI�s�łȂ�
	||	���܂�Ԃ��ł���
	*/
	bCRLF = (EolType::None == pLayout->GetLayoutEol()) ? FALSE : TRUE;

	bAddCRLF = FALSE;
	if (!bCRLF) {
		if (GetCaret().GetCaretLayoutPos().GetY2() == GetDocument()->m_layoutMgr.GetLineCount() - 1) {
			bAddCRLF = TRUE;
		}
	}

	memBuf.SetString(pLayout->GetPtr(), pLayout->GetLengthWithoutEOL() + pLayout->GetLayoutEol().GetLen());	// ��pLayout->GetLengthWithEOL()�́AEOL�̒�����K��1�ɂ���̂Ŏg���Ȃ��B
	if (bAddCRLF) {
		// ���݁AEnter�Ȃǂő}��������s�R�[�h�̎�ނ��擾
		Eol cWork = GetDocument()->m_docEditor.GetNewLineCode();
		memBuf.AppendString(cWork.GetValue2(), cWork.GetLen());
	}

	// ���݈ʒu�Ƀf�[�^��}��
	LayoutPoint ptLayoutNew;
	m_pCommanderView->InsertData_CEditView(
		GetCaret().GetCaretLayoutPos(),
		memBuf.GetStringPtr(),
		memBuf.GetStringLength(),
		&ptLayoutNew,
		true
	);

	// �J�[�\�����ړ�
	GetCaret().MoveCursor(ptCaretPosOld, true);
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	if (!m_pCommanderView->m_bDoing_UndoRedo) {	// �A���h�D�E���h�D�̎��s����
		// ����̒ǉ�
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				GetCaret().GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
			)
		);
	}
	return;
}

