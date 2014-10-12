/*!	@file
@brief CViewCommander�N���X�̃R�}���h(�I���n/��`�I���n)�֐��Q

	2012/12/20	CViewCommander.cpp���番��
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
#include "CViewCommander.h"
#include "CViewCommander_inline.h"


// ���݈ʒu�̒P��I��
bool CViewCommander::Command_SELECTWORD(CLayoutPoint* pptCaretPos)
{
	CLayoutRange sRange;
	CLogicInt	nIdx;
	auto& si = m_pCommanderView->GetSelectionInfo();
	if (si.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// ���݂̑I��͈͂��I����Ԃɖ߂�
		si.DisableSelectArea(true);
	}
	CLayoutPoint ptCaretPos = ((!pptCaretPos) ? GetCaret().GetCaretLayoutPos() : *pptCaretPos);
	const CLayout*	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(ptCaretPos.GetY2());
	if (!pcLayout) {
		return false;	//	�P��I���Ɏ��s
	}
	// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
	nIdx = m_pCommanderView->LineColumnToIndex(pcLayout, ptCaretPos.GetX2());

	// ���݈ʒu�̒P��͈̔͂𒲂ׂ�
	if (GetDocument()->m_cLayoutMgr.WhereCurrentWord(	ptCaretPos.GetY2(), nIdx, &sRange, NULL, NULL)) {

		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
		// 2007.10.15 kobake ���Ƀ��C�A�E�g�P�ʂȂ̂ŕϊ��͕s�v
		/*
		pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(sRange.GetFrom().GetY2());
		sRange.SetFromX(m_pCommanderView->LineIndexToColumn(pcLayout, sRange.GetFrom().x));
		pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(sRange.GetTo().GetY2());
		sRange.SetToX(m_pCommanderView->LineIndexToColumn(pcLayout, sRange.GetTo().x));
		*/

		// �I��͈͂̕ύX
		//	2005.06.24 Moca
		si.SetSelectArea(sRange);
		// �I��̈�`��
		si.DrawSelectArea();

		// �P��̐擪�ɃJ�[�\�����ړ�
		GetCaret().MoveCursor(sRange.GetTo(), true);
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

		return true;	//	�P��I���ɐ����B
	}else {
		return false;	//	�P��I���Ɏ��s
	}
}


// ���ׂđI��
void CViewCommander::Command_SELECTALL(void)
{
	auto& si = m_pCommanderView->GetSelectionInfo();
	if (si.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// ���݂̑I��͈͂��I����Ԃɖ߂�
		si.DisableSelectArea(true);
	}

	// �擪�փJ�[�\�����ړ�
	//	Sep. 8, 2000 genta
	m_pCommanderView->AddCurrentLineToHistory();
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	//	Jul. 29, 2006 genta �I���ʒu�̖����𐳊m�Ɏ擾����
	//	�}�N������擾�����ꍇ�ɐ������͈͂��擾�ł��Ȃ�����
	//int nX, nY;
	CLayoutRange sRange;
	sRange.SetFrom(CLayoutPoint(0,0));
	GetDocument()->m_cLayoutMgr.GetEndLayoutPos(sRange.GetToPointer());
	si.SetSelectArea(sRange);

	// �I��̈�`��
	si.DrawSelectArea(false);
}


/*!	1�s�I��
	@brief �J�[�\���ʒu��1�s�I������
	@param lparam [in] �}�N������g�p����g���t���O�i�g���p�ɗ\��j

	note ���s�P�ʂőI�����s���B

	@date 2007.11.15 nasukoji	�V�K�쐬
*/
void CViewCommander::Command_SELECTLINE(int lparam)
{
	// ���s�P�ʂ�1�s�I������
	Command_GOLINETOP(false, 0x9);	// �����s���Ɉړ�

	auto& si = m_pCommanderView->GetSelectionInfo();
	si.m_bBeginLineSelect = true;		// �s�P�ʑI��

	CLayoutPoint ptCaret;

	// �ŉ��s�i�����s�j�łȂ�
	if (GetCaret().GetCaretLogicPos().y < GetDocument()->m_cDocLineMgr.GetLineCount()) {
		// 1�s��̕����s���烌�C�A�E�g�s�����߂�
		GetDocument()->m_cLayoutMgr.LogicToLayout(CLogicPoint(0, GetCaret().GetCaretLogicPos().y + 1), &ptCaret);

		// �J�[�\�������̕����s���ֈړ�����
		m_pCommanderView->MoveCursorSelecting(ptCaret, TRUE);

		// �ړ���̃J�[�\���ʒu���擾����
		ptCaret = GetCaret().GetCaretLayoutPos().Get();
	}else {
		// �J�[�\�����ŉ��s�i���C�A�E�g�s�j�ֈړ�����
		m_pCommanderView->MoveCursorSelecting(CLayoutPoint(CLayoutInt(0), GetDocument()->m_cLayoutMgr.GetLineCount()), TRUE);
		Command_GOLINEEND(true, 0, 0);	// �s���Ɉړ�

		// �I��������̂������i[EOF]�݂̂̍s�j���͑I����ԂƂ��Ȃ�
		if (
			!si.IsTextSelected()
			&& (GetCaret().GetCaretLogicPos().y >= GetDocument()->m_cDocLineMgr.GetLineCount())
		) {
			// ���݂̑I��͈͂��I����Ԃɖ߂�
			si.DisableSelectArea(true);
		}
	}
	
	if (si.m_bBeginLineSelect) {
		// �͈͑I���J�n�s�E�J�������L��
		si.m_sSelect.SetTo(ptCaret);
		si.m_sSelectBgn.SetTo(ptCaret);
	}

	return;
}

// �͈͑I���J�n
void CViewCommander::Command_BEGIN_SELECT(void)
{
	auto& si = m_pCommanderView->GetSelectionInfo();
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
		GetCaret().m_cUnderLine.CaretUnderLineOFF(true);
	}
	si.PrintSelectionInfoMsg();
	return;
}


// ��`�͈͑I���J�n
void CViewCommander::Command_BEGIN_BOXSELECT(bool bSelectingLock)
{
	if (!GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH) {	// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
		return;
	}

	auto& si = m_pCommanderView->GetSelectionInfo();
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
	GetCaret().m_cUnderLine.CaretUnderLineOFF(true);
	return;
}

