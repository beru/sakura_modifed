/*!	@file
@brief ViewCommander�N���X�̃R�}���h(�J�[�\���ړ��n)�֐��Q

	2012/12/17	CViewCommander.cpp,CViewCommander_New.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, asa-o, hor
	Copyright (C) 2002, hor, YAZAKI, oak
	Copyright (C) 2003, Moca
	Copyright (C) 2004, Moca, genta, fotomo
	Copyright (C) 2006, genta
	Copyright (C) 2007, kobake, maru
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "CMarkMgr.h"/// 2002/2/3 aroka �ǉ�
#include "mem/CMemoryIterator.h"	// @@@ 2002.09.28 YAZAKI


void ViewCommander::Command_MOVECURSOR(LogicPoint pos, int option)
{
	if (pos.GetX2() < 0 || pos.GetY2() < 0) {
		ErrorBeep();
		return;
	}
	LayoutPoint layoutPos;
	GetDocument()->m_cLayoutMgr.LogicToLayout(pos, &layoutPos);
	Command_MOVECURSORLAYOUT(layoutPos, option);
}

void ViewCommander::Command_MOVECURSORLAYOUT(LayoutPoint pos, int option)
{
	if (pos.GetX2() < 0 || pos.GetY2() < 0) {
		ErrorBeep();
		return;
	}

	bool bSelect = (option & 0x01) == 0x01;
	bool bBoxSelect = (option & 0x02) == 0x02;

	auto& si = m_pCommanderView->GetSelectionInfo();

	if (bSelect || bBoxSelect) {
		if (!si.IsTextSelected()) {
			if (bBoxSelect) {
				Command_BEGIN_BOXSELECT();
			}else {
				si.BeginSelectArea();
			}
		}else {
			// 2014.01.08 �ǉ�
			if (bBoxSelect && !si.IsBoxSelecting()) {
				// �ʏ�I������`�I���ɕύX�B���̃R�}���h�ɍ��킹��
				Command_BEGIN_BOXSELECT();
			}
		}
	}else {
		if (si.IsTextSelected()) {
			si.DisableSelectArea(true);
		}else if (si.IsBoxSelecting()) {
			si.SetBoxSelect(false);
		}
	}
	auto& caret = GetCaret();
	caret.GetAdjustCursorPos(&pos);
	// �I��
	if (bSelect || bBoxSelect) {
		si.ChangeSelectAreaByCurrentCursor(pos);
	}
	caret.MoveCursor(pos, true);
	caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
}


/////////////////////////////////// �ȉ��̓R�}���h�Q (Oct. 17, 2000 jepro note) ///////////////////////////////////////////

// �J�[�\����ړ�
int ViewCommander::Command_UP(bool bSelect, bool bRepeat, int lines)
{
	auto& caret = GetCaret();
	// From Here Oct. 24, 2001 genta
	if (lines != 0) {
		caret.Cursor_UPDOWN(LayoutInt(lines), FALSE);
		return 1;
	}
	// To Here Oct. 24, 2001 genta


	int nRepeat = 0;

	// �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩
	auto& csGeneral = GetDllShareData().m_common.m_sGeneral;
	if (!csGeneral.m_nRepeatedScroll_Smooth) {
		LayoutInt i;
		if (!bRepeat) {
			i = LayoutInt(-1);
		}else {
			i = -1 * csGeneral.m_nRepeatedScrollLineNum;	// �L�[���s�[�g���̃X�N���[���s��
		}
		caret.Cursor_UPDOWN(i, bSelect);
		nRepeat = -1 * (Int)i;
	}else {
		++nRepeat;
		if (caret.Cursor_UPDOWN(LayoutInt(-1), bSelect) != 0 && bRepeat) {
			for (int i=0; i<csGeneral.m_nRepeatedScrollLineNum-1; ++i) {		// �L�[���s�[�g���̃X�N���[���s��
				::UpdateWindow(m_pCommanderView->GetHwnd());	// YAZAKI
				caret.Cursor_UPDOWN(LayoutInt(-1), bSelect);
				++nRepeat;
			}
		}
	}
	return nRepeat;
}


// �J�[�\�����ړ�
int ViewCommander::Command_DOWN(bool bSelect, bool bRepeat)
{
	auto& caret = GetCaret();
	int nRepeat = 0;
	auto& csGeneral = GetDllShareData().m_common.m_sGeneral;
	// �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩
	if (!csGeneral.m_nRepeatedScroll_Smooth) {
		LayoutInt i;
		if (!bRepeat) {
			i = LayoutInt(1);
		}else {
			i = csGeneral.m_nRepeatedScrollLineNum;	// �L�[���s�[�g���̃X�N���[���s��
		}
		caret.Cursor_UPDOWN(i, bSelect);
		nRepeat = (Int)i;
	}else {
		++nRepeat;
		if (caret.Cursor_UPDOWN(LayoutInt(1), bSelect) != 0 && bRepeat) {
			for (int i=0; i<csGeneral.m_nRepeatedScrollLineNum-1; ++i) {	// �L�[���s�[�g���̃X�N���[���s��
				// �����ōĕ`��B
				::UpdateWindow(m_pCommanderView->GetHwnd());	// YAZAKI
				caret.Cursor_UPDOWN(LayoutInt(1), bSelect);
				++nRepeat;
			}
		}
	}
	return nRepeat;
}


/*! @brief �J�[�\�����ړ�

	@date 2004.03.28 Moca EOF�����̍s�ȍ~�̓r���ɃJ�[�\��������Ɨ�����o�O�C���D
			pcLayout == NULL���L�����b�g�ʒu���s���ȊO�̏ꍇ��
			2��if�̂ǂ���ɂ����Ă͂܂�Ȃ����C���̂��Ƃ�MoveCursor�ɂēK����
			�ʒu�Ɉړ���������D
	@date 2014.01.10 Moca �L�[���s�[�g���AMoveCursor����x�ɂ܂Ƃ߂�
*/
int ViewCommander::Command_LEFT(bool bSelect, bool bRepeat)
{
	bool bUnderlineDoNotOFF = true;		// �A���_�[���C�����������Ȃ�
	if (bSelect) {
		bUnderlineDoNotOFF = false;		// �I����ԂȂ�A���_�[���C���������s��
	}
	bool bMoveCaretLine = false;
	int nRepeat = bRepeat ? 2 : 1;
	int nRes = 0;
	LayoutPoint ptCaretMove = GetCaret().GetCaretLayoutPos();
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	for (int nRepCount=0; nRepCount<nRepeat; ++nRepCount) {
		if (bSelect && ! selInfo.IsTextSelected()) {
			// ���݂̃J�[�\���ʒu����I�����J�n����
			selInfo.BeginSelectArea();
		}
		if (!bSelect) {
			if (selInfo.IsTextSelected()) {
				this->Command_CANCEL_MODE(1);
				nRes = 1;
				continue; // �I���̃L�����Z���ō��ړ��� 1����B���̌�̈ړ������̓X�L�b�v����B
			}else if (selInfo.IsBoxSelecting()) {
				selInfo.SetBoxSelect(false);
			}
		}
		// (���ꂩ�狁�߂�)�J�[�\���̈ړ���B
		LayoutPoint ptPos(LayoutInt(0), ptCaretMove.GetY2());

		// ���ݍs�̃f�[�^���擾
		const Layout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( ptCaretMove.GetY2() );
		// �J�[�\�������[�ɂ���
		if (ptCaretMove.GetX2() == (pcLayout ? pcLayout->GetIndent() : LayoutInt(0))) {
			if (0 < ptCaretMove.GetY2()
			   && ! selInfo.IsBoxSelecting()
			) {
				// �O�̃��C�A�E�g�s�́A�܂�Ԃ������O�܂��͉��s�����̎�O�Ɉړ�����B
				pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( ptCaretMove.GetY2() - LayoutInt(1) );
				CMemoryIterator it(pcLayout, GetDocument()->m_cLayoutMgr.GetTabSpace());
				while (!it.end()) {
					it.scanNext();
					if (it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL()) {
						ptPos.x += it.getColumnDelta();
						break;
					}
					it.addDelta();
				}
				ptPos.x += it.getColumn() - it.getColumnDelta();
				ptPos.y --;
			}else {
				if (0 < nRepCount) {
					GetCaret().MoveCursor( ptCaretMove, true, _CARETMARGINRATE, bUnderlineDoNotOFF );
					GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
				}
				nRes = 0;
				break; // ����ȏ㍶�ɓ����ʁB
			}
			bUnderlineDoNotOFF = false;	// �s���ς��̂ŃA���_�[���C������������
			bMoveCaretLine = true;
		}
		//  2004.03.28 Moca EOF�����̍s�ȍ~�̓r���ɃJ�[�\��������Ɨ�����o�O�C��
		else if (pcLayout) {
			CMemoryIterator it(pcLayout, GetDocument()->m_cLayoutMgr.GetTabSpace());
			while (!it.end()) {
				it.scanNext();
				if ( it.getColumn() + it.getColumnDelta() > ptCaretMove.GetX2() - 1 ){
					ptPos.x += it.getColumnDelta();
					break;
				}
				it.addDelta();
			}
			ptPos.x += it.getColumn() - it.getColumnDelta();
			// Oct. 18, 2002 YAZAKI
			if (it.getIndex() >= pcLayout->GetLengthWithEOL()) {
				ptPos.x = ptCaretMove.GetX2() - LayoutInt(1);
			}
		}

		GetCaret().GetAdjustCursorPos( &ptPos );
		if (bSelect) {
			/*	���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX�D
				2004.04.02 Moca 
				�L�����b�g�ʒu���s���������ꍇ��MoveCursor�̈ړ����ʂ�
				�����ŗ^�������W�Ƃ͈قȂ邱�Ƃ����邽�߁C
				ptPos�̑���Ɏ��ۂ̈ړ����ʂ��g���悤�ɁD
			*/
			selInfo.ChangeSelectAreaByCurrentCursor(ptPos);
		}
		if (bMoveCaretLine || nRepeat - 1 == nRepCount) {
			GetCaret().MoveCursor( ptPos, true, _CARETMARGINRATE, bUnderlineDoNotOFF );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
		ptCaretMove = ptPos;
		nRes = 1;
	}
	return nRes;
}


/* �J�[�\���E�ړ�
	@date 2014.01.10 Moca �L�[���s�[�g���AMoveCursor����x�ɂ܂Ƃ߂�
*/
void ViewCommander::Command_RIGHT(
	bool bSelect,
	bool bIgnoreCurrentSelection,
	bool bRepeat
	)
{
	bool bUnderlineDoNotOFF = true;	// �A���_�[���C�����������Ȃ�
	if (bSelect) {
		bUnderlineDoNotOFF = false;		// �I����ԂȂ�A���_�[���C���������s��
	}
	bool bMoveCaretLine = false;
	int nRepeat = bRepeat ? 2 : 1; // �ړ������
	LayoutPoint ptCaretMove = GetCaret().GetCaretLayoutPos();
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	for (int nRepCount=0; nRepCount<nRepeat; ++nRepCount) {
		// 2003.06.28 Moca [EOF]�݂̂̍s�ɃJ�[�\��������Ƃ��ɉE�������Ă��I���������ł��Ȃ�����
		// �Ή����邽�߁A���ݍs�̃f�[�^���擾���ړ�
		if (!bIgnoreCurrentSelection) {
			if (bSelect && ! selInfo.IsTextSelected()) {
				// ���݂̃J�[�\���ʒu����I�����J�n����
				selInfo.BeginSelectArea();
			}
			if (!bSelect) {
				if (selInfo.IsTextSelected()) {
					this->Command_CANCEL_MODE(2);
					continue; // �I���̃L�����Z���ŉE�ړ��� 1����B���̌�̈ړ������̓X�L�b�v����B
				}else if (selInfo.IsBoxSelecting()) {
					selInfo.SetBoxSelect(false);
				}
			}
		}
//		2003.06.28 Moca [EOF]�݂̂̍s�ɃJ�[�\��������Ƃ��ɉE�������Ă��I���������ł��Ȃ����ɑΉ�

		// (���ꂩ�狁�߂�)�J�[�\���̈ړ���B
		LayoutPoint ptTo(0, 0);
		const LayoutPoint ptCaret = ptCaretMove;

		// ���ݍs�̃f�[�^���擾
		const Layout* const pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(ptCaret.y);
		// 2004.04.02 EOF�ȍ~�ɃJ�[�\�����������Ƃ��ɉE�������Ă������N���Ȃ������̂��AEOF�Ɉړ�����悤��
		if (pcLayout) {
			// �L�����b�g�ʒu�̃��C�A�E�g�s�ɂ��āB
			const LayoutInt x_wrap = pcLayout->CalcLayoutWidth(GetDocument()->m_cLayoutMgr); // ���s�����A�܂��͐܂�Ԃ��̈ʒu�B
			const bool wrapped = EOL_NONE == pcLayout->GetLayoutEol(); // �܂�Ԃ��Ă��邩�A���s�����ŏI����Ă��邩�B����ɂ�� x_wrap�̈Ӗ����ς��B
			const bool nextline_exists = pcLayout->GetNextLayout() || pcLayout->GetLayoutEol() != EOL_NONE; // EOF�݂̂̍s���܂߁A�L�����b�g���ړ��\�Ȏ��s�����݂��邩�B

			// ���݂̃L�����b�g�̉E�̈ʒu(to_x)�����߂�B
			CMemoryIterator it(pcLayout, GetDocument()->m_cLayoutMgr.GetTabSpace());
			for (; !it.end(); it.scanNext(), it.addDelta()) {
				if (ptCaret.x < it.getColumn()) {
					break;
				}
			}
			const LayoutInt to_x = t_max(it.getColumn(), ptCaret.x + 1);

			// �L�����b�g�̉E�[(x_max)�ƁA�����ł̈���(on_x_max)�����߂�B
			LayoutInt x_max;
			enum {
				STOP,
				MOVE_NEXTLINE_IMMEDIATELY, // �E�[�Ɏ~�܂炸���̍s���Ɉړ�����B(�܂�Ԃ��Ȃ�)
				MOVE_NEXTLINE_NEXTTIME, // �E�[�Ɏ~�܂�A���Ɏ��̍s���Ɉړ�����B(���s�𒴂���Ƃ��Ȃ�)
				MOVE_NEXTLINE_NEXTTIME_AND_MOVE_RIGHT // �E�[�Ɏ~�܂�A���Ɏ��̍s���̈�E�Ɉړ�����B(�܂�Ԃ��Ȃ�)
			} on_x_max;

			if (selInfo.IsBoxSelecting()) {
				x_max = t_max(x_wrap, GetDocument()->m_cLayoutMgr.GetMaxLineKetas());
				on_x_max = STOP;
			}else if (GetDllShareData().m_common.m_sGeneral.m_bIsFreeCursorMode) {
				// �t���[�J�[�\�����[�h�ł͐܂�Ԃ��ʒu�������݂āA���s�����̈ʒu�݂͂Ȃ��B
				if (wrapped) {
					if (nextline_exists) {
						x_max = x_wrap;
						on_x_max = MOVE_NEXTLINE_IMMEDIATELY;
					}else {
						// �f�[�^�̂���EOF�s�͐܂�Ԃ��ł͂Ȃ�
						x_max = t_max(x_wrap, GetDocument()->m_cLayoutMgr.GetMaxLineKetas());
						on_x_max = STOP;
					}
				}else {
					if (x_wrap < GetDocument()->m_cLayoutMgr.GetMaxLineKetas()) {
						x_max = GetDocument()->m_cLayoutMgr.GetMaxLineKetas();
						on_x_max = MOVE_NEXTLINE_IMMEDIATELY;
					}else { // ���s�������Ԃ牺�����Ă���Ƃ��͗�O�B
						x_max = x_wrap;
						on_x_max = MOVE_NEXTLINE_NEXTTIME;
					}
				}
			}else {
				x_max = x_wrap;
				on_x_max = wrapped ? MOVE_NEXTLINE_IMMEDIATELY : MOVE_NEXTLINE_NEXTTIME;
			}

			// �L�����b�g�̈ړ�������߂�B
			if (nextline_exists
				&& (on_x_max == MOVE_NEXTLINE_IMMEDIATELY && x_max <= to_x
					|| on_x_max == MOVE_NEXTLINE_NEXTTIME && x_max < to_x
					|| on_x_max == MOVE_NEXTLINE_NEXTTIME_AND_MOVE_RIGHT && x_max < to_x
				)
			) {
				ptTo.y = ptCaret.y + 1;
				ptTo.x = pcLayout->GetNextLayout() ? pcLayout->GetNextLayout()->GetIndent() : LayoutInt(0);
				if (on_x_max == MOVE_NEXTLINE_NEXTTIME_AND_MOVE_RIGHT) {
					++nRepeat;
				}
				bUnderlineDoNotOFF = false;
				bMoveCaretLine = true;
			}else {
				ptTo.y = ptCaret.y;
				ptTo.x = t_min(to_x, x_max);
			}
		}else {
			// pcLayout��NULL�̏ꍇ��ptPos.x=0�ɒ���
			ptTo.y = ptCaret.y;
			ptTo.x = 0;
		}

		GetCaret().GetAdjustCursorPos( &ptTo );
		if (bSelect) {
			// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
			selInfo.ChangeSelectAreaByCurrentCursor( ptTo );
		}

		if (bMoveCaretLine || nRepeat - 1 == nRepCount) {
			GetCaret().MoveCursor( ptTo, true, _CARETMARGINRATE, bUnderlineDoNotOFF );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
		ptCaretMove = ptTo;
	}
	return;
}


// �J�[�\����ړ�(�Q�s�Â�)
void ViewCommander::Command_UP2(bool bSelect)
{
	GetCaret().Cursor_UPDOWN(LayoutInt(-2), bSelect);
	return;
}


// �J�[�\�����ړ�(�Q�s�Â�)
void ViewCommander::Command_DOWN2(bool bSelect)
{
	GetCaret().Cursor_UPDOWN(LayoutInt(2), bSelect);
	return;
}


// �P��̍��[�Ɉړ�
void ViewCommander::Command_WORDLEFT(bool bSelect)
{
	bool bUnderlineDoNotOFF = true;		// �A���_�[���C�����������Ȃ�
	if (bSelect) {
		bUnderlineDoNotOFF = false;		// �I����ԂȂ�A���_�[���C���������s��
	}
	LogicInt nIdx;
	auto& si = m_pCommanderView->GetSelectionInfo();
	if (bSelect) {
		if (!si.IsTextSelected()) {		// �e�L�X�g���I������Ă��邩
			// ���݂̃J�[�\���ʒu����I�����J�n����
			si.BeginSelectArea();
		}
	}else {
		if (si.IsTextSelected()) {		// �e�L�X�g���I������Ă��邩
			// ���݂̑I��͈͂��I����Ԃɖ߂�
			si.DisableSelectArea(true);
		}else if (si.IsBoxSelecting()) {
			si.SetBoxSelect(false);
		}
	}

	auto& caret = GetCaret();
	auto& csGeneral = GetDllShareData().m_common.m_sGeneral;

	const Layout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().GetY2());
	if (!pcLayout) {
		bool bIsFreeCursorModeOld = csGeneral.m_bIsFreeCursorMode;	// �t���[�J�[�\�����[�h��
		csGeneral.m_bIsFreeCursorMode = false;
		// �J�[�\�����ړ�
		Command_LEFT(bSelect, false);
		csGeneral.m_bIsFreeCursorMode = bIsFreeCursorModeOld;	// �t���[�J�[�\�����[�h��
		return;
	}

	// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
	nIdx = m_pCommanderView->LineColumnToIndex(pcLayout, caret.GetCaretLayoutPos().GetX2());

	// ���݈ʒu�̍��̒P��̐擪�ʒu�𒲂ׂ�
	LayoutPoint ptLayoutNew;
	int nResult = GetDocument()->m_cLayoutMgr.PrevWord(
		caret.GetCaretLayoutPos().GetY2(),
		nIdx,
		&ptLayoutNew,
		csGeneral.m_bStopsBothEndsWhenSearchWord
	);
	if (nResult) {
		// �s���ς����
		if (ptLayoutNew.y != caret.GetCaretLayoutPos().GetY2()) {
			pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(ptLayoutNew.GetY2());
			if (!pcLayout) {
				return;
			}
			bUnderlineDoNotOFF = false;
		}

		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
		// 2007.10.15 kobake ���Ƀ��C�A�E�g�P�ʂȂ̂ŕϊ��͕s�v
		/*
		ptLayoutNew.x = m_pCommanderView->LineIndexToColumn(pcLayout, ptLayoutNew.x);
		*/

		// �J�[�\���ړ�
		caret.GetAdjustCursorPos(&ptLayoutNew);
		if (bSelect) {
			// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
			si.ChangeSelectAreaByCurrentCursor(ptLayoutNew);
		}
		caret.MoveCursor(ptLayoutNew, true, _CARETMARGINRATE, bUnderlineDoNotOFF);
		caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
	}else {
		bool bIsFreeCursorModeOld = csGeneral.m_bIsFreeCursorMode;	// �t���[�J�[�\�����[�h��
		csGeneral.m_bIsFreeCursorMode = false;
		// �J�[�\�����ړ�
		Command_LEFT(bSelect, false);
		csGeneral.m_bIsFreeCursorMode = bIsFreeCursorModeOld;	// �t���[�J�[�\�����[�h��
	}
	return;
}


// �P��̉E�[�Ɉړ�
void ViewCommander::Command_WORDRIGHT(bool bSelect)
{
	bool bUnderlineDoNotOFF = true;	// �A���_�[���C�����������Ȃ�
	if (bSelect) {
		bUnderlineDoNotOFF = false;		// �I����ԂȂ�A���_�[���C���������s��
	}
	LogicInt nIdx;
	LayoutInt nCurLine;
	auto& si = m_pCommanderView->GetSelectionInfo();
	if (bSelect) {
		if (!si.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
			// ���݂̃J�[�\���ʒu����I�����J�n����
			si.BeginSelectArea();
		}
	}else {
		if (si.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
			// ���݂̑I��͈͂��I����Ԃɖ߂�
			si.DisableSelectArea(true);
		}else if (si.IsBoxSelecting()) {
			si.SetBoxSelect(false);
		}
	}
	bool	bTryAgain = false;
try_again:;
	auto& caret = GetCaret();
	nCurLine = caret.GetCaretLayoutPos().GetY2();
	const Layout* pcLayout;
	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(nCurLine);
	if (!pcLayout) {
		return;
	}
	if (bTryAgain) {
		const wchar_t*	pLine = pcLayout->GetPtr();
		if (pLine[0] != L' ' && pLine[0] != WCODE::TAB) {
			return;
		}
	}
	// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
	nIdx = m_pCommanderView->LineColumnToIndex(pcLayout, caret.GetCaretLayoutPos().GetX2());

	auto& csGeneral = GetDllShareData().m_common.m_sGeneral;	
	// ���݈ʒu�̉E�̒P��̐擪�ʒu�𒲂ׂ�
	LayoutPoint ptLayoutNew;
	int nResult = GetDocument()->m_cLayoutMgr.NextWord(
		nCurLine,
		nIdx,
		&ptLayoutNew,
		csGeneral.m_bStopsBothEndsWhenSearchWord
	);
	if (nResult) {
		// �s���ς����
		if (ptLayoutNew.y != nCurLine) {
			pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(ptLayoutNew.GetY2());
			if (!pcLayout) {
				return;
			}
			bUnderlineDoNotOFF = false;
		}
		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
		// 2007.10.15 kobake ���Ƀ��C�A�E�g�P�ʂȂ̂ŕϊ��͕s�v
		/*
		ptLayoutNew.x = m_pCommanderView->LineIndexToColumn(pcLayout, ptLayoutNew.x);
		*/
		// �J�[�\���ړ�
		caret.GetAdjustCursorPos(&ptLayoutNew);
		if (bSelect) {
			// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
			si.ChangeSelectAreaByCurrentCursor(ptLayoutNew);
		}
		caret.MoveCursor(ptLayoutNew, true, _CARETMARGINRATE, bUnderlineDoNotOFF);
		caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
	}else {
		bool bIsFreeCursorModeOld = csGeneral.m_bIsFreeCursorMode;	// �t���[�J�[�\�����[�h��
		csGeneral.m_bIsFreeCursorMode = false;
		// �J�[�\���E�ړ�
		Command_RIGHT(bSelect, false, false);
		csGeneral.m_bIsFreeCursorMode = bIsFreeCursorModeOld;	// �t���[�J�[�\�����[�h��
		if (!bTryAgain) {
			bTryAgain = true;
			goto try_again;
		}
	}
	return;
}


/*! @brief �s���Ɉړ�

	@date Oct. 29, 2001 genta �}�N���p�@�\�g��(�p�����[�^�ǉ�) + goto�r��
	@date May. 15, 2002 oak   ���s�P�ʈړ�
	@date Oct.  7, 2002 YAZAKI �璷�Ȉ��� bLineTopOnly ���폜
	@date Jun. 18, 2007 maru �s������ɑS�p�󔒂̃C���f���g�ݒ���l������
*/
void ViewCommander::Command_GOLINETOP(
	bool	bSelect,	// [in] �I���̗L���Btrue: �I�����Ȃ���ړ��Bfalse: �I�����Ȃ��ňړ��B
	int		lparam		/* [in] �}�N������g�p����g���t���O
								  @li 0: �L�[����Ɠ���(default)
								  @li 1: �J�[�\���ʒu�Ɋ֌W�Ȃ��s���Ɉړ�(������)
								  @li 4: �I�����Ĉړ�(������)
								  @li 8: ���s�P�ʂŐ擪�Ɉړ�(������)
						*/
	)
{
	using namespace WCODE;

	// lparam�̉���
	bool bLineTopOnly = ((lparam & 1) != 0);
	if (lparam & 4) {
		bSelect = true;
	}

	LayoutPoint ptCaretPos;
	auto& caret = GetCaret();
	if (lparam & 8) {
		// ���s�P�ʎw��̏ꍇ�́A�����s���ʒu����ړI�_���ʒu�����߂�
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			LogicPoint(0, caret.GetCaretLogicPos().y),
			&ptCaretPos
		);
	}else {
		const Layout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().GetY2());
		ptCaretPos.x = pcLayout ? pcLayout->GetIndent() : LayoutInt(0);
		ptCaretPos.y = caret.GetCaretLayoutPos().GetY2();
	}
	if (!bLineTopOnly) {
		// �ړI�s�̃f�[�^���擾
		// ���s�P�ʎw��ŁA�擪����󔒂�1�܂�Ԃ��s�ȏ㑱���Ă���ꍇ�͎��̍s�f�[�^���擾
		LayoutInt nPosY_Layout;
		LogicInt  nPosX_Logic;

		nPosY_Layout = ptCaretPos.y - 1;
		const Layout*	pcLayout;
		bool			bZenSpace = m_pCommanderView->m_pTypeData->m_bAutoIndent_ZENSPACE;
		bool			bExtEol = GetDllShareData().m_common.m_sEdit.m_bEnableExtEol;
		
		LogicInt		nLineLen;
		do {
			++nPosY_Layout;
			const wchar_t*	pLine = GetDocument()->m_cLayoutMgr.GetLineStr(nPosY_Layout, &nLineLen, &pcLayout);
			if (!pLine) {
				return;
			}
			for (nPosX_Logic=0; nPosX_Logic<nLineLen; ++nPosX_Logic) {
				if (WCODE::IsIndentChar(pLine[nPosX_Logic], bZenSpace != 0)) continue;
				
				if (WCODE::IsLineDelimiter(pLine[nPosX_Logic], bExtEol)) {
					nPosX_Logic = 0;	// �󔒂܂��̓^�u����щ��s�����̍s������
				}
				break;
			}
		}
		while ((lparam & 8) && (nPosX_Logic >= nLineLen) && (GetDocument()->m_cLayoutMgr.GetLineCount() - 1 > nPosY_Layout));
		
		if (nPosX_Logic >= nLineLen) {
			/* �܂�Ԃ��P�ʂ̍s����T���ĕ����s���܂œ��B����
			�܂��́A�ŏI�s�̂��߉��s�R�[�h�ɑ��������ɍs���ɓ��B���� */
			nPosX_Logic = 0;
		}
		
		if (nPosX_Logic == 0) nPosY_Layout = ptCaretPos.y;	// �����s�̈ړ��Ȃ�
		
		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
		LayoutInt nPosX_Layout = m_pCommanderView->LineIndexToColumn(pcLayout, nPosX_Logic);
		LayoutPoint ptPos(nPosX_Layout, nPosY_Layout);
		if (caret.GetCaretLayoutPos() != ptPos) {
			ptCaretPos = ptPos;
		}
	}

	// 2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
	m_pCommanderView->MoveCursorSelecting(ptCaretPos, bSelect);
}


/*! �s���Ɉړ�(�܂�Ԃ��P��)
	@praram nOption	0x08 ���s�P��(������)
*/
void ViewCommander::Command_GOLINEEND(
	bool bSelect,
	int bIgnoreCurrentSelection,
	int nOption
	)
{
	if (nOption & 4) {
		bSelect = true;
	}
	auto& si = m_pCommanderView->GetSelectionInfo();
	if (!bIgnoreCurrentSelection) {
		if (bSelect) {
			if (!si.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
				// ���݂̃J�[�\���ʒu����I�����J�n����
				si.BeginSelectArea();
			}
		}else {
			if (si.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
				// ���݂̑I��͈͂��I����Ԃɖ߂�
				si.DisableSelectArea(true);
			}else if (si.IsBoxSelecting()) {
				si.SetBoxSelect(false);
			}
		}
	}

	// ���ݍs�̃f�[�^����A���̃��C�A�E�g�����擾
	auto& caret = GetCaret();
	LayoutPoint nPosXY = caret.GetCaretLayoutPos();
	if (nOption & 8) {
		// ���s�P�ʂ̍s���B1�s���̍ŏI���C�A�E�g�s��T��
		const Layout*	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(nPosXY.y);
		const Layout*	pcLayoutNext = pcLayout->GetNextLayout();
		while (pcLayout && pcLayoutNext && pcLayoutNext->GetLogicOffset() != 0) {
			pcLayout = pcLayoutNext;
			pcLayoutNext = pcLayoutNext->GetNextLayout();
			++nPosXY.y;
		}
	}
	nPosXY.x = LayoutInt(0);
	const Layout*	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(nPosXY.y);
	if (pcLayout)
		nPosXY.x = pcLayout->CalcLayoutWidth(GetDocument()->m_cLayoutMgr);

	// �L�����b�g�ړ�
	caret.GetAdjustCursorPos(&nPosXY);
	if (bSelect) {
		// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
		si.ChangeSelectAreaByCurrentCursor(nPosXY);
	}
	caret.MoveCursor(nPosXY, true);
	caret.m_nCaretPosX_Prev = nPosXY.x;
}


// ���y�[�W�A�b�v		// Oct. 6, 2000 JEPRO added (���͏]���̃X�N���[���_�E�����̂���)
void ViewCommander::Command_HalfPageUp(
	bool bSelect,
	LayoutYInt nScrollNum
	)
{
	if (nScrollNum <= 0) {
		nScrollNum = m_pCommanderView->GetTextArea().m_nViewRowNum / 2;
	}
	GetCaret().Cursor_UPDOWN( - (nScrollNum), bSelect );
	return;
}


// ���y�[�W�_�E��		// Oct. 6, 2000 JEPRO added (���͏]���̃X�N���[���A�b�v���̂���)
void ViewCommander::Command_HalfPageDown(
	bool bSelect,
	LayoutYInt nScrollNum
	)
{
	if (nScrollNum <= 0) {
		nScrollNum = m_pCommanderView->GetTextArea().m_nViewRowNum / 2;
	}
	GetCaret().Cursor_UPDOWN( nScrollNum, bSelect );
	return;
}


/*! �P�y�[�W�A�b�v

	@date 2000.10.10 JEPRO �쐬
	@date 2001.12.13 hor ��ʂɑ΂���J�[�\���ʒu�͂��̂܂܂�
		�P�y�[�W�A�b�v�ɓ���ύX
	@date 2014.01.10 Moca �J�[�\���������Ȃ��Ƃ�����ʂ��X�N���[������悤��
*/	// Oct. 10, 2000 JEPRO added
void ViewCommander::Command_1PageUp(
	bool bSelect,
	LayoutYInt nScrollNum
	)
{
// GetCaret().Cursor_UPDOWN(-m_pCommanderView->GetTextArea().m_nViewRowNum, bSelect);

// 2001.12.03 hor
//		���������C�N�ɁA��ʂɑ΂���J�[�\���ʒu�͂��̂܂܂łP�y�[�W�A�b�v
	{
		const bool bDrawSwitchOld = m_pCommanderView->SetDrawSwitch(false);
		auto& textArea = m_pCommanderView->GetTextArea();
		LayoutInt nViewTopLine = textArea.GetViewTopLine();
		if (nScrollNum <= 0) {
			nScrollNum = m_pCommanderView->GetTextArea().m_nViewRowNum - 1;
		}
		GetCaret().Cursor_UPDOWN( -nScrollNum, bSelect );
		// Sep. 11, 2004 genta �����X�N���[�������̂���
		// m_pCommanderView->RedrawAll�ł͂Ȃ�ScrollAt���g���悤��
		m_pCommanderView->SyncScrollV( m_pCommanderView->ScrollAtV( nViewTopLine - nScrollNum ));
		m_pCommanderView->SetDrawSwitch(bDrawSwitchOld);
		m_pCommanderView->RedrawAll();
	}
	return;
}


/*!	�P�y�[�W�_�E��

	@date 2000.10.10 JEPRO �쐬
	@date 2001.12.13 hor ��ʂɑ΂���J�[�\���ʒu�͂��̂܂܂�
		�P�y�[�W�_�E���ɓ���ύX
	@date 2014.01.10 Moca �J�[�\���������Ȃ��Ƃ�����ʂ��X�N���[������悤��
*/
void ViewCommander::Command_1PageDown(
	bool bSelect,
	LayoutYInt nScrollNum
	)
{
// GetCaret().Cursor_UPDOWN(m_pCommanderView->GetTextArea().m_nViewRowNum, bSelect);

// 2001.12.03 hor
//		���������C�N�ɁA��ʂɑ΂���J�[�\���ʒu�͂��̂܂܂łP�y�[�W�_�E��
	{
		const bool bDrawSwitchOld = m_pCommanderView->SetDrawSwitch(false);
		LayoutInt nViewTopLine = m_pCommanderView->GetTextArea().GetViewTopLine();
		if (nScrollNum <= 0) {
			nScrollNum = m_pCommanderView->GetTextArea().m_nViewRowNum - 1;
		}
		GetCaret().Cursor_UPDOWN(nScrollNum, bSelect);
		// Sep. 11, 2004 genta �����X�N���[�������̂���
		// m_pCommanderView->RedrawAll�ł͂Ȃ�ScrollAt���g���悤��
		m_pCommanderView->SyncScrollV(m_pCommanderView->ScrollAtV(nViewTopLine + nScrollNum));
		m_pCommanderView->SetDrawSwitch(bDrawSwitchOld);
		m_pCommanderView->RedrawAll();
	}

	return;
}


// �t�@�C���̐擪�Ɉړ�
void ViewCommander::Command_GOFILETOP(bool bSelect)
{
	// �擪�փJ�[�\�����ړ�
	// Sep. 8, 2000 genta
	m_pCommanderView->AddCurrentLineToHistory();

	// 2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
	LayoutPoint pt(
		!m_pCommanderView->GetSelectionInfo().IsBoxSelecting()? LayoutInt(0): GetCaret().GetCaretLayoutPos().GetX2(),
		LayoutInt(0)
	);
	m_pCommanderView->MoveCursorSelecting(pt, bSelect);	// �ʏ�́A(0, 0)�ֈړ��B�{�b�N�X�I�𒆂́A(GetCaret().GetCaretLayoutPos().GetX2(), 0)�ֈړ�
}


// �t�@�C���̍Ō�Ɉړ�
void ViewCommander::Command_GOFILEEND(bool bSelect)
{
	auto& si = m_pCommanderView->GetSelectionInfo();
// 2001.12.13 hor BOX�I�𒆂Ƀt�@�C���̍Ō�ɃW�����v�����[EOF]�̍s�����]�����܂܂ɂȂ�̏C��
	if (!bSelect) {
		if (si.IsTextSelected()) {
			si.DisableSelectArea(true);	// 2001.12.21 hor Add
		}else if (si.IsBoxSelecting()) {
			si.SetBoxSelect(false);
		}
	}
	m_pCommanderView->AddCurrentLineToHistory();
	auto& caret = GetCaret();
	caret.Cursor_UPDOWN(GetDocument()->m_cLayoutMgr.GetLineCount() , bSelect);
	Command_DOWN(bSelect, true);
	if (!si.IsBoxSelecting()) {							// 2002/04/18 YAZAKI
		/*	2004.04.19 fotomo
			���s�̂Ȃ��ŏI�s�őI�����Ȃ��當�����ֈړ������ꍇ��
			�I��͈͂��������Ȃ��ꍇ��������ɑΉ�
		*/
		Command_GOLINEEND(bSelect, 0, 0);				// 2001.12.21 hor Add
	}
	caret.MoveCursor(caret.GetCaretLayoutPos(), true);	// 2001.12.21 hor Add
	// 2002.02.16 hor ��`�I�𒆂��������O�̃J�[�\���ʒu�����Z�b�g
	if (!(si.IsTextSelected() && si.IsBoxSelecting())) {
		caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
	}

	// �I��͈͏�񃁃b�Z�[�W��\������	// 2009.05.06 ryoji �ǉ�
	if (bSelect) {
		si.PrintSelectionInfoMsg();
	}
}


// �J�[�\���s���E�B���h�E������
void ViewCommander::Command_CURLINECENTER(void)
{
	LayoutInt nViewTopLine;
	auto& textArea = m_pCommanderView->GetTextArea();
	nViewTopLine = GetCaret().GetCaretLayoutPos().GetY2() - (textArea.m_nViewRowNum / 2);

	// sui 02/08/09
	if (0 > nViewTopLine) nViewTopLine = LayoutInt(0);
	
	LayoutInt nScrollLines = nViewTopLine - textArea.GetViewTopLine();	// Sep. 11, 2004 genta �����p�ɍs�����L��
	textArea.SetViewTopLine(nViewTopLine);
	// �t�H�[�J�X�ړ����̍ĕ`��
	m_pCommanderView->RedrawAll();
	// sui 02/08/09

	// Sep. 11, 2004 genta �����X�N���[���̊֐���
	m_pCommanderView->SyncScrollV(nScrollLines);
}


// �ړ�������O�ւ��ǂ�
void ViewCommander::Command_JUMPHIST_PREV(void)
{
	// 2001.12.13 hor
	// �ړ������̍Ō�Ɍ��݂̈ʒu���L������
	// (���̗������擾�ł��Ȃ��Ƃ��͒ǉ����Ė߂�)
	if (!m_pCommanderView->m_cHistory->CheckNext()) {
		m_pCommanderView->AddCurrentLineToHistory();
		m_pCommanderView->m_cHistory->PrevValid();
	}

	if (m_pCommanderView->m_cHistory->CheckPrev()) {
		if (! m_pCommanderView->m_cHistory->PrevValid()) {
			::MessageBox(NULL, _T("Inconsistent Implementation"), _T("PrevValid"), MB_OK);
		}
		LayoutPoint pt;
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			m_pCommanderView->m_cHistory->GetCurrent().GetPosition(),
			&pt
		);
		// 2006.07.09 genta �I�����l��
		m_pCommanderView->MoveCursorSelecting(pt, m_pCommanderView->GetSelectionInfo().m_bSelectingLock);
	}
}


// �ړ����������ւ��ǂ�
void ViewCommander::Command_JUMPHIST_NEXT(void)
{
	if (m_pCommanderView->m_cHistory->CheckNext()) {
		if (!m_pCommanderView->m_cHistory->NextValid()) {
			::MessageBox(NULL, _T("Inconsistent Implementation"), _T("NextValid"), MB_OK);
		}
		LayoutPoint pt;
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			m_pCommanderView->m_cHistory->GetCurrent().GetPosition(),
			&pt
		);
		// 2006.07.09 genta �I�����l��
		m_pCommanderView->MoveCursorSelecting(pt, m_pCommanderView->GetSelectionInfo().m_bSelectingLock);
	}
}


// ���݈ʒu���ړ������ɓo�^����
void ViewCommander::Command_JUMPHIST_SET(void)
{
	m_pCommanderView->AddCurrentLineToHistory();
}


// 2001/06/20 Start by asa-o

// from CViewCommander_New.cpp
// �e�L�X�g���P�s���փX�N���[��
void ViewCommander::Command_WndScrollDown(void)
{
	LayoutInt	nCaretMarginY;

	auto& textArea = m_pCommanderView->GetTextArea();
	nCaretMarginY = textArea.m_nViewRowNum / _CARETMARGINRATE;
	if (nCaretMarginY < 1) {
		nCaretMarginY = LayoutInt(1);
	}

	nCaretMarginY += 2;

	bool bCaretOff = false;
	auto& caret = GetCaret();
	if (caret.GetCaretLayoutPos().GetY() > textArea.m_nViewRowNum + textArea.GetViewTopLine() - (nCaretMarginY + 1)) {
		bCaretOff = true;
	}

	// Sep. 11, 2004 genta �����p�ɍs�����L��
	// Sep. 11, 2004 genta �����X�N���[���̊֐���
	m_pCommanderView->SyncScrollV(m_pCommanderView->ScrollAtV(textArea.GetViewTopLine() - LayoutInt(1)));

	// �e�L�X�g���I������Ă��Ȃ�
	if (!m_pCommanderView->GetSelectionInfo().IsTextSelected()) {
		// �J�[�\������ʊO�ɏo��
		if (caret.GetCaretLayoutPos().GetY() > textArea.m_nViewRowNum + textArea.GetViewTopLine() - nCaretMarginY) {
			if (caret.GetCaretLayoutPos().GetY() > GetDocument()->m_cLayoutMgr.GetLineCount() - nCaretMarginY) {
				caret.Cursor_UPDOWN((GetDocument()->m_cLayoutMgr.GetLineCount() - nCaretMarginY) - caret.GetCaretLayoutPos().GetY2(), FALSE);
			}else {
				caret.Cursor_UPDOWN(LayoutInt(-1), FALSE);
			}
			caret.ShowCaretPosInfo();
		}
	}
	if (bCaretOff) {
		caret.m_cUnderLine.CaretUnderLineOFF(true);
	}
	caret.m_cUnderLine.CaretUnderLineON(true, true);
}


// from CViewCommander_New.cpp
// �e�L�X�g���P�s��փX�N���[��
void ViewCommander::Command_WndScrollUp(void)
{
	LayoutInt	nCaretMarginY;

	auto& textArea = m_pCommanderView->GetTextArea();
	nCaretMarginY = textArea.m_nViewRowNum / _CARETMARGINRATE;
	if (nCaretMarginY < 1)
		nCaretMarginY = 1;

	bool bCaretOff = false;
	if (GetCaret().GetCaretLayoutPos().GetY2() < textArea.GetViewTopLine() + (nCaretMarginY + 1)) {
		bCaretOff = true;
	}

	// Sep. 11, 2004 genta �����p�ɍs�����L��
	// Sep. 11, 2004 genta �����X�N���[���̊֐���
	m_pCommanderView->SyncScrollV(m_pCommanderView->ScrollAtV(textArea.GetViewTopLine() + LayoutInt(1)));

	// �e�L�X�g���I������Ă��Ȃ�
	if (!m_pCommanderView->GetSelectionInfo().IsTextSelected()) {
		// �J�[�\������ʊO�ɏo��
		if (GetCaret().GetCaretLayoutPos().GetY() < textArea.GetViewTopLine() + nCaretMarginY) {
			if (textArea.GetViewTopLine() == 1) {
				GetCaret().Cursor_UPDOWN(nCaretMarginY + 1, FALSE);
			}else {
				GetCaret().Cursor_UPDOWN(LayoutInt(1), FALSE);
			}
			GetCaret().ShowCaretPosInfo();
		}
	}
	if (bCaretOff) {
		GetCaret().m_cUnderLine.CaretUnderLineOFF(true);
	}
	GetCaret().m_cUnderLine.CaretUnderLineON(true, true);
}

// 2001/06/20 End


// from CViewCommander_New.cpp
/* ���̒i���֐i��
	2002/04/26 �i���̗��[�Ŏ~�܂�I�v�V������ǉ�
	2002/04/19 �V�K
*/
void ViewCommander::Command_GONEXTPARAGRAPH(bool bSelect)
{
	CDocLine* pcDocLine;
	int nCaretPointer = 0;
	auto& docLineMgr = GetDocument()->m_cDocLineMgr;
	
	bool nFirstLineIsEmptyLine = false;
	// �܂��́A���݈ʒu����s�i�X�y�[�X�A�^�u�A���s�L���݂̂̍s�j���ǂ�������
	if ((pcDocLine = docLineMgr.GetLine(GetCaret().GetCaretLogicPos().GetY2() + LogicInt(nCaretPointer)))) {
		nFirstLineIsEmptyLine = pcDocLine->IsEmptyLine();
		++nCaretPointer;
	}else {
		// EOF�s�ł����B
		return;
	}

	// ���ɁAnFirstLineIsEmptyLine�ƈقȂ�Ƃ���܂œǂݔ�΂�
	while ((pcDocLine = docLineMgr.GetLine(GetCaret().GetCaretLogicPos().GetY2() + LogicInt(nCaretPointer)))) {
		if (pcDocLine->IsEmptyLine() == nFirstLineIsEmptyLine) {
			++nCaretPointer;
		}else {
			break;
		}
	}

	/*	nFirstLineIsEmptyLine����s��������A�����Ă���Ƃ���͔��s�B���Ȃ킿�����܂��B
		nFirstLineIsEmptyLine�����s��������A�����Ă���Ƃ���͋�s�B
	*/
	if (nFirstLineIsEmptyLine) {
		// �����܂��B
	}else {
		// ���܌��Ă���Ƃ���͋�s��1�s��
		if (GetDllShareData().m_common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph) {	// �i���̗��[�Ŏ~�܂�
		}else {
			// �d�グ�ɁA��s����Ȃ��Ƃ���܂Ői��
			while ((pcDocLine = docLineMgr.GetLine(GetCaret().GetCaretLogicPos().GetY2() + LogicInt(nCaretPointer)))) {
				if (pcDocLine->IsEmptyLine()) {
					++nCaretPointer;
				}else {
					break;
				}
			}
		}
	}

	// EOF�܂ŗ�����A�ړI�̏ꏊ�܂ł����̂ňړ��I���B

	// �ړ��������v�Z
	LayoutPoint ptCaretPos_Layo;

	// �ړ��O�̕����ʒu
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		GetCaret().GetCaretLogicPos(),
		&ptCaretPos_Layo
	);

	// �ړ���̕����ʒu
	LayoutPoint ptCaretPos_Layo_CaretPointer;
	//int nCaretPosY_Layo_CaretPointer;
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		GetCaret().GetCaretLogicPos() + LogicPoint(0, nCaretPointer),
		&ptCaretPos_Layo_CaretPointer
	);

	GetCaret().Cursor_UPDOWN(ptCaretPos_Layo_CaretPointer.y - ptCaretPos_Layo.y, bSelect);
}


// from CViewCommander_New.cpp
/* �O�̒i���֐i��
	2002/04/26 �i���̗��[�Ŏ~�܂�I�v�V������ǉ�
	2002/04/19 �V�K
*/
void ViewCommander::Command_GOPREVPARAGRAPH(bool bSelect)
{
	auto& docLineMgr = GetDocument()->m_cDocLineMgr;
	CDocLine* pcDocLine;
	int nCaretPointer = -1;

	bool nFirstLineIsEmptyLine = false;
	// �܂��́A���݈ʒu����s�i�X�y�[�X�A�^�u�A���s�L���݂̂̍s�j���ǂ�������
	if ((pcDocLine = docLineMgr.GetLine(GetCaret().GetCaretLogicPos().GetY2() + LogicInt(nCaretPointer)))) {
		nFirstLineIsEmptyLine = pcDocLine->IsEmptyLine();
		--nCaretPointer;
	}else {
		nFirstLineIsEmptyLine = true;
		--nCaretPointer;
	}

	// ���ɁAnFirstLineIsEmptyLine�ƈقȂ�Ƃ���܂œǂݔ�΂�
	while ((pcDocLine = docLineMgr.GetLine(GetCaret().GetCaretLogicPos().GetY2() + LogicInt(nCaretPointer)))) {
		if (pcDocLine->IsEmptyLine() == nFirstLineIsEmptyLine) {
			--nCaretPointer;
		}else {
			break;
		}
	}

	/*	nFirstLineIsEmptyLine����s��������A�����Ă���Ƃ���͔��s�B���Ȃ킿�����܂��B
		nFirstLineIsEmptyLine�����s��������A�����Ă���Ƃ���͋�s�B
	*/
	auto& csGeneral = GetDllShareData().m_common.m_sGeneral;	
	if (nFirstLineIsEmptyLine) {
		// �����܂��B
		if (csGeneral.m_bStopsBothEndsWhenSearchParagraph) {	// �i���̗��[�Ŏ~�܂�
			++nCaretPointer;	// ��s�̍ŏ�s�i�i���̖��[�̎��̍s�j�Ŏ~�܂�B
		}else {
			// �d�グ�ɁA��s����Ȃ��Ƃ���܂Ői��
			while ((pcDocLine = docLineMgr.GetLine(GetCaret().GetCaretLogicPos().GetY2() + LogicInt(nCaretPointer)))) {
				if (pcDocLine->IsEmptyLine()) {
					break;
				}else {
					--nCaretPointer;
				}
			}
			++nCaretPointer;	// ��s�̍ŏ�s�i�i���̖��[�̎��̍s�j�Ŏ~�܂�B
		}
	}else {
		// ���܌��Ă���Ƃ���͋�s��1�s��
		if (csGeneral.m_bStopsBothEndsWhenSearchParagraph) {	// �i���̗��[�Ŏ~�܂�
			++nCaretPointer;
		}else {
			++nCaretPointer;
		}
	}

	// EOF�܂ŗ�����A�ړI�̏ꏊ�܂ł����̂ňړ��I���B

	// �ړ��������v�Z
	LayoutPoint ptCaretPos_Layo;

	// �ړ��O�̕����ʒu
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		GetCaret().GetCaretLogicPos(),
		&ptCaretPos_Layo
	);

	// �ړ���̕����ʒu
	LayoutPoint ptCaretPos_Layo_CaretPointer;
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		GetCaret().GetCaretLogicPos() + LogicPoint(0, nCaretPointer),
		&ptCaretPos_Layo_CaretPointer
	);

	GetCaret().Cursor_UPDOWN(ptCaretPos_Layo_CaretPointer.y - ptCaretPos_Layo.y, bSelect);
}

void ViewCommander::Command_AUTOSCROLL()
{
	if (m_pCommanderView->m_nAutoScrollMode == 0) {
		GetCursorPos(&m_pCommanderView->m_cAutoScrollMousePos);
		ScreenToClient(m_pCommanderView->GetHwnd(), &m_pCommanderView->m_cAutoScrollMousePos);
		m_pCommanderView->m_bAutoScrollDragMode = false;
		m_pCommanderView->AutoScrollEnter();
	}else {
		m_pCommanderView->AutoScrollExit();
	}
}

void ViewCommander::Command_WHEELUP(int zDelta)
{
	int zDelta2 = (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	m_pCommanderView->OnMOUSEWHEEL2(wParam, lParam, false, F_WHEELUP);
}

void ViewCommander::Command_WHEELDOWN(int zDelta)
{
	int zDelta2 = -1 * (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	m_pCommanderView->OnMOUSEWHEEL2(wParam, lParam, false, F_WHEELDOWN);
}

void ViewCommander::Command_WHEELLEFT(int zDelta)
{
	int zDelta2 = -1 * (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	m_pCommanderView->OnMOUSEWHEEL2(wParam, lParam, true, F_WHEELLEFT);
}

void ViewCommander::Command_WHEELRIGHT(int zDelta)
{
	int zDelta2 = (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	m_pCommanderView->OnMOUSEWHEEL2(wParam, lParam, true, F_WHEELRIGHT);
}

void ViewCommander::Command_WHEELPAGEUP(int zDelta)
{
	int zDelta2 = (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	m_pCommanderView->OnMOUSEWHEEL2(wParam, lParam, false, F_WHEELPAGEUP);
}

void ViewCommander::Command_WHEELPAGEDOWN(int zDelta)
{
	int zDelta2 = -1 * (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	m_pCommanderView->OnMOUSEWHEEL2(wParam, lParam, false, F_WHEELPAGEDOWN);
}

void ViewCommander::Command_WHEELPAGELEFT(int zDelta)
{
	int zDelta2 = -1 * (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	m_pCommanderView->OnMOUSEWHEEL2(wParam, lParam, true, F_WHEELPAGELEFT);
}

void ViewCommander::Command_WHEELPAGERIGHT(int zDelta)
{
	int zDelta2 = (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	m_pCommanderView->OnMOUSEWHEEL2(wParam, lParam, true, F_WHEELPAGERIGHT);
}

/*! ���̕ύX�s��
	�ύX�s�̃u���b�N�̐擪�s�ƁA�ύX�s�̃u���b�N�̖���(���̍s��)�Ɉړ�����
*/
void ViewCommander::Command_MODIFYLINE_NEXT( bool bSelect )
{
	auto& docLineMgr = GetDocument()->m_cDocLineMgr;
	LogicInt nYOld = GetCaret().GetCaretLogicPos().y;
	LogicPoint ptXY(0, nYOld);
	const CDocLine* pcDocLine = docLineMgr.GetLine(ptXY.GetY2());
	const int nSaveSeq = GetDocument()->m_cDocEditor.m_cOpeBuf.GetNoModifiedSeq();
	bool bModified = false;
	if (docLineMgr.GetLineCount() == 0) {
		return;
	}
	if (pcDocLine) {
		bModified = CModifyVisitor().IsLineModified(pcDocLine, nSaveSeq);
		ptXY.y++;
		pcDocLine = pcDocLine->GetNextLine();
	}
	for (int n=0; n<2; ++n) {
		while (pcDocLine) {
			if (CModifyVisitor().IsLineModified(pcDocLine, nSaveSeq) != bModified
				|| (
					ptXY.y == 0
					&& CModifyVisitor().IsLineModified(pcDocLine, nSaveSeq)
				)
			) {
				LayoutPoint ptLayout;
				GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY, &ptLayout);
				m_pCommanderView->MoveCursorSelecting( ptLayout, bSelect );
				if (nYOld >= ptXY.GetY2()) {
					m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT1));
				}
				return;
			}
			ptXY.y++;
			pcDocLine = pcDocLine->GetNextLine();
		}
		if (n == 0 && bModified) {
			const CDocLine* pcDocLineLast = docLineMgr.GetDocLineBottom();
			bool bSkip = false;
			LogicPoint pos;
			if (pcDocLineLast) {
				if (pcDocLineLast->GetEol() == EOL_NONE) {
					// �Ԃ牺����[EOF]
					pos.x = pcDocLineLast->GetLengthWithoutEOL();
					pos.y = docLineMgr.GetLineCount() - 1;
					if (GetCaret().GetCaretLogicPos() == pos) {
						bSkip = true;
					}
				}else {
					// �P��[EOF]
					pos = ptXY;
				}
			}else {
				bSkip = true;
			}
			if (!bSkip) {
				LayoutPoint ptLayout;
				GetDocument()->m_cLayoutMgr.LogicToLayout(pos, &ptLayout);
				m_pCommanderView->MoveCursorSelecting( ptLayout, bSelect );
				return;
			}
		}
		if (n == 0) {
			ptXY.y = LogicInt(0);
			pcDocLine = docLineMgr.GetLine(ptXY.GetY2());
			bModified = false;
		}
		if (!GetDllShareData().m_common.m_sSearch.m_bSearchAll) {
			break;
		}
	}
	m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT2));
	AlertNotFound( m_pCommanderView->GetHwnd(), false, LS(STR_MODLINE_NEXT_NOT_FOUND));
}

/*! �O�̕ύX�s��
	�ύX�s�̃u���b�N�̐擪�s�ƁA�ύX�s�̃u���b�N�̖���(���̍s��)�Ɉړ�����
	Command_MODIFYLINE_NEXT�Ɠ����ʒu�Ɏ~�܂�
*/
void ViewCommander::Command_MODIFYLINE_PREV( bool bSelect )
{
	auto& docLineMgr = GetDocument()->m_cDocLineMgr;
	LogicInt nYOld = GetCaret().GetCaretLogicPos().y;
	LogicInt nYOld2 = nYOld;
	LogicPoint ptXY(0, nYOld);
	const CDocLine* pcDocLine = docLineMgr.GetLine(ptXY.GetY2());
	const int nSaveSeq = GetDocument()->m_cDocEditor.m_cOpeBuf.GetNoModifiedSeq();
	bool bModified = false;
	bool bLast = false;
	if (!pcDocLine) {
		// [EOF]
		const CDocLine* pcDocLineLast = docLineMgr.GetLine(ptXY.GetY2() - 1);
		if (!pcDocLineLast) {
			// 1�s���Ȃ�
			return;
		}
		bModified = CModifyVisitor().IsLineModified(pcDocLineLast, nSaveSeq);
		ptXY.y--;
		pcDocLine = pcDocLineLast;
		bLast = true;
	}
	if (!bLast) {
		const CDocLine* pcDocLineLast = docLineMgr.GetDocLineBottom();
		if (pcDocLineLast && pcDocLineLast->GetEol() == EOL_NONE) {
			LogicPoint pos;
			pos.x = pcDocLine->GetLengthWithoutEOL();
			pos.y = docLineMgr.GetLineCount() - 1;
			if (GetCaret().GetCaretLogicPos() == pos) {
				// �Ԃ牺����[EOF]�̈ʒu������
				bLast = true;
			}
		}
	}
	assert( pcDocLine );
	bModified = CModifyVisitor().IsLineModified(pcDocLine, nSaveSeq);
	nYOld2 = ptXY.y;
	ptXY.y--;
	pcDocLine = pcDocLine->GetPrevLine();
	if (pcDocLine && !bLast) {
		bModified = CModifyVisitor().IsLineModified(pcDocLine, nSaveSeq);
		nYOld2 = ptXY.y;
		ptXY.y--;
		pcDocLine = pcDocLine->GetPrevLine();
	}
	for (int n=0; n<2; ++n) {
		while (pcDocLine) {
			bool bModifiedTemp = CModifyVisitor().IsLineModified(pcDocLine, nSaveSeq);
			if (bModifiedTemp != bModified) {
				// ���o���ꂽ�ʒu��1�s���(MODIFYLINE_NEXT�Ɠ����ʒu)�Ɏ~�܂�
				ptXY.y = nYOld2;
				LayoutPoint ptLayout;
				GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY, &ptLayout);
				m_pCommanderView->MoveCursorSelecting( ptLayout, bSelect );
				if (n == 1) {
					m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV1));
				}
				return;
			}
			nYOld2 = ptXY.y;
			ptXY.y--;
			pcDocLine = pcDocLine->GetPrevLine();
		}
		if (n == 0) {
			// �擪�s�`�F�b�N
			const CDocLine* pcDocLineTemp = docLineMgr.GetDocLineTop();
			assert( pcDocLineTemp );
			if (CModifyVisitor().IsLineModified(pcDocLineTemp, nSaveSeq) != false) {
				if (GetCaret().GetCaretLogicPos() != LogicPoint(0,0)) {
					ptXY = LogicPoint(0,0);
					LayoutPoint ptLayout;
					GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY, &ptLayout);
					m_pCommanderView->MoveCursorSelecting( ptLayout, bSelect );
					return;
				}
			}
			ptXY.y = docLineMgr.GetLineCount() - LogicInt(1);
			nYOld2 = ptXY.y;
			pcDocLine = docLineMgr.GetLine(ptXY.GetY2());
			bModified = false;
		}
		if (!GetDllShareData().m_common.m_sSearch.m_bSearchAll) {
			break;
		}
		if (n == 0) {
			const CDocLine* pcDocLineTemp = docLineMgr.GetDocLineBottom();
			assert( pcDocLineTemp );
			if (CModifyVisitor().IsLineModified(pcDocLineTemp, nSaveSeq) != false) {
				// �ŏI�s���ύX�s�̏ꍇ�́A[EOF]�Ɏ~�܂�
				LogicPoint pos;
				if (pcDocLineTemp->GetEol() != EOL_NONE) {
					pos.x = 0;
					pos.y = docLineMgr.GetLineCount();
					pos.y++;
				}else {
					pos.x = pcDocLine->GetLengthWithoutEOL();
					pos.y = docLineMgr.GetLineCount() - 1;
				}
				if (GetCaret().GetCaretLogicPos() != pos) {
					ptXY = pos;
					LayoutPoint ptLayout;
					GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY, &ptLayout);
					m_pCommanderView->MoveCursorSelecting( ptLayout, bSelect );
					m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV1));
					return;
				}
			}
		}
	}
	m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV2));
	AlertNotFound( m_pCommanderView->GetHwnd(), false, LS(STR_MODLINE_PREV_NOT_FOUND) );
}

