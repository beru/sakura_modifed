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
		MoveCaretOpe*	pcOpe = new MoveCaretOpe();
		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			GetSelect().GetTo(),
			&pcOpe->m_ptCaretPos_PHY_Before
		);
		pcOpe->m_ptCaretPos_PHY_After = pcOpe->m_ptCaretPos_PHY_Before;	// �����̃L�����b�g�ʒu

		// ����̒ǉ�
		GetOpeBlk()->AppendOpe(pcOpe);
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
		MoveCaretOpe*	pcOpe = new MoveCaretOpe();
		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			GetSelect().GetFrom(),
			&pcOpe->m_ptCaretPos_PHY_Before
		);
		pcOpe->m_ptCaretPos_PHY_After = pcOpe->m_ptCaretPos_PHY_Before;	// �����̃L�����b�g�ʒu
		// ����̒ǉ�
		GetOpeBlk()->AppendOpe(pcOpe);
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
	Layout* pCLayout;
	if (selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
		Command_CUT();
		return;
	}
	pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().GetY2());	// �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(Layout)�ւ̃|�C���^��Ԃ�
	if (!pCLayout) {
		ErrorBeep();
		return;
	}

	LayoutPoint ptPos;
	GetDocument()->m_cLayoutMgr.LogicToLayout(LogicPoint(0, pCLayout->GetLogicLineNo()), &ptPos);
	if (GetCaret().GetCaretLayoutPos() == ptPos) {
		ErrorBeep();
		return;
	}

	// �I��͈͂̕ύX
	// 2005.06.24 Moca
	LayoutRange sRange(ptPos, GetCaret().GetCaretLayoutPos());
	selInfo.SetSelectArea(sRange);

	// �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
	Command_CUT();
}



// �s���܂Ő؂���(���s�P��)
void ViewCommander::Command_LineCutToEnd(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	Layout* pCLayout;
	if (selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
		Command_CUT();
		return;
	}
	// �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(Layout)�ւ̃|�C���^��Ԃ�
	pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(
		GetCaret().GetCaretLayoutPos().GetY2()
	);
	if (!pCLayout) {
		ErrorBeep();
		return;
	}

	LayoutPoint ptPos;
	auto& docLineRef = *pCLayout->GetDocLineRef();
	if (docLineRef.GetEol() == EOL_NONE) {	// ���s�R�[�h�̎��
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			LogicPoint(
				docLineRef.GetLengthWithEOL(),
				pCLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}else {
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			LogicPoint(
				docLineRef.GetLengthWithEOL() - docLineRef.GetEol().GetLen(),
				pCLayout->GetLogicLineNo()
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
	LayoutRange sRange(GetCaret().GetCaretLayoutPos(), ptPos);
	selInfo.SetSelectArea(sRange);

	// �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
	Command_CUT();
}


// �s���܂ō폜(���s�P��)
void ViewCommander::Command_LineDeleteToStart(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	Layout* pCLayout;
	if (selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		m_pCommanderView->DeleteData(true);
		return;
	}
	pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().GetY2());	// �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(Layout)�ւ̃|�C���^��Ԃ�
	if (!pCLayout) {
		ErrorBeep();
		return;
	}

	LayoutPoint ptPos;

	GetDocument()->m_cLayoutMgr.LogicToLayout(LogicPoint(0, pCLayout->GetLogicLineNo()), &ptPos);
	if (GetCaret().GetCaretLayoutPos() == ptPos) {
		ErrorBeep();
		return;
	}

	// �I��͈͂̕ύX
	// 2005.06.24 Moca
	LayoutRange sRange(ptPos, GetCaret().GetCaretLayoutPos());
	selInfo.SetSelectArea(sRange);

	// �I��̈�폜
	m_pCommanderView->DeleteData(true);
}


// �s���܂ō폜(���s�P��)
void ViewCommander::Command_LineDeleteToEnd(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	Layout* pCLayout;
	if (selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		m_pCommanderView->DeleteData(true);
		return;
	}

	auto& caretLayoutPos = GetCaret().GetCaretLayoutPos();
	pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(caretLayoutPos.GetY2());	// �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(Layout)�ւ̃|�C���^��Ԃ�
	if (!pCLayout) {
		ErrorBeep();
		return;
	}

	LayoutPoint ptPos;

	auto& docLineRef = *pCLayout->GetDocLineRef();
	if (docLineRef.GetEol() == EOL_NONE) {	// ���s�R�[�h�̎��
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			LogicPoint(
				docLineRef.GetLengthWithEOL(),
				pCLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}else {
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			LogicPoint(
				docLineRef.GetLengthWithEOL() - docLineRef.GetEol().GetLen(),
				pCLayout->GetLogicLineNo()
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
	LayoutRange sRange(caretLayoutPos, ptPos);
	selInfo.SetSelectArea(sRange);

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

	const Layout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().y);
	if (!pcLayout) {
		ErrorBeep();
		return;
	}

	// 2007.10.04 ryoji �����ȑf��
	m_pCommanderView->CopyCurLine(
		GetDllShareData().m_common.m_sEdit.m_bAddCRLFWhenCopy,
		EOL_UNKNOWN,
		GetDllShareData().m_common.m_sEdit.m_bEnableLineModePaste
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
	const Layout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().GetY2());
	if (!pcLayout) {
		ErrorBeep();
		return;
	}
	GetSelect().SetFrom(LayoutPoint(LayoutInt(0), GetCaret().GetCaretLayoutPos().GetY2()  ));	// �͈͑I���J�n�ʒu
	GetSelect().SetTo  (LayoutPoint(LayoutInt(0), GetCaret().GetCaretLayoutPos().GetY2() + 1));	// �͈͑I���I���ʒu

	LayoutPoint ptCaretPos_OLD = GetCaret().GetCaretLayoutPos();

	Command_DELETE();
	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().GetY2());
	if (pcLayout) {
		// 2003-04-30 �����
		// �s�폜������A�t���[�J�[�\���łȂ��̂ɃJ�[�\���ʒu���s�[���E�ɂȂ�s��Ή�
		// �t���[�J�[�\�����[�h�łȂ��ꍇ�́A�J�[�\���ʒu�𒲐�����
		if (!GetDllShareData().m_common.m_sGeneral.m_bIsFreeCursorMode) {
			LogicInt nIndex;

			LayoutInt tmp;
			nIndex = m_pCommanderView->LineColumnToIndex2(pcLayout, ptCaretPos_OLD.GetX2(), &tmp);
			ptCaretPos_OLD.x = tmp;

			if (ptCaretPos_OLD.x > 0) {
				ptCaretPos_OLD.x--;
			}else {
				ptCaretPos_OLD.x = m_pCommanderView->LineIndexToColumn(pcLayout, nIndex);
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
	int				bCRLF;
	int				bAddCRLF;
	NativeW		cmemBuf;

	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	if (selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// ���݂̑I��͈͂��I����Ԃɖ߂�
		selInfo.DisableSelectArea(true);
	}

	const Layout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().GetY2());
	if (!pcLayout) {
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
	bCRLF = (EOL_NONE == pcLayout->GetLayoutEol()) ? FALSE : TRUE;

	bAddCRLF = FALSE;
	if (!bCRLF) {
		if (GetCaret().GetCaretLayoutPos().GetY2() == GetDocument()->m_cLayoutMgr.GetLineCount() - 1) {
			bAddCRLF = TRUE;
		}
	}

	cmemBuf.SetString(pcLayout->GetPtr(), pcLayout->GetLengthWithoutEOL() + pcLayout->GetLayoutEol().GetLen());	// ��pcLayout->GetLengthWithEOL()�́AEOL�̒�����K��1�ɂ���̂Ŏg���Ȃ��B
	if (bAddCRLF) {
		// ���݁AEnter�Ȃǂő}��������s�R�[�h�̎�ނ��擾
		Eol cWork = GetDocument()->m_cDocEditor.GetNewLineCode();
		cmemBuf.AppendString(cWork.GetValue2(), cWork.GetLen());
	}

	// ���݈ʒu�Ƀf�[�^��}��
	LayoutPoint ptLayoutNew;
	m_pCommanderView->InsertData_CEditView(
		GetCaret().GetCaretLayoutPos(),
		cmemBuf.GetStringPtr(),
		cmemBuf.GetStringLength(),
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

