/*!	@file
@brief ViewCommander�N���X�̃R�}���h(�J�[�\���ړ��n)�֐��Q

	2012/12/17	ViewCommander.cpp,ViewCommander_New.cpp���番��
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
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "MarkMgr.h"/// 2002/2/3 aroka �ǉ�
#include "mem/MemoryIterator.h"	// @@@ 2002.09.28 YAZAKI


void ViewCommander::Command_MoveCursor(Point pos, int option)
{
	if (pos.x < 0 || pos.y < 0) {
		ErrorBeep();
		return;
	}
	Point layoutPos = GetDocument().layoutMgr.LogicToLayout(pos);
	Command_MoveCursorLayout(layoutPos, option);
}

void ViewCommander::Command_MoveCursorLayout(Point pos, int option)
{
	if (pos.x < 0 || pos.y < 0) {
		ErrorBeep();
		return;
	}

	bool bSelect = (option & 0x01) == 0x01;
	bool bBoxSelect = (option & 0x02) == 0x02;

	auto& si = view.GetSelectionInfo();

	if (bSelect || bBoxSelect) {
		if (!si.IsTextSelected()) {
			if (bBoxSelect) {
				Command_Begin_BoxSelect();
			}else {
				si.BeginSelectArea();
			}
		}else {
			// 2014.01.08 �ǉ�
			if (bBoxSelect && !si.IsBoxSelecting()) {
				// �ʏ�I������`�I���ɕύX�B���̃R�}���h�ɍ��킹��
				Command_Begin_BoxSelect();
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
	caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
}


/////////////////////////////////// �ȉ��̓R�}���h�Q (Oct. 17, 2000 jepro note) ///////////////////////////////////////////

// �J�[�\����ړ�
int ViewCommander::Command_Up(bool bSelect, bool bRepeat, int lines)
{
	auto& caret = GetCaret();
	// From Here Oct. 24, 2001 genta
	if (lines != 0) {
		caret.Cursor_UPDOWN(lines, false);
		return 1;
	}
	// To Here Oct. 24, 2001 genta


	int nRepeat = 0;

	// �L�[���s�[�g����Scroll�����炩�ɂ��邩
	auto& csGeneral = GetDllShareData().common.general;
	if (!csGeneral.nRepeatedScroll_Smooth) {
		int i;
		if (!bRepeat) {
			i = -1;
		}else {
			i = -1 * csGeneral.nRepeatedScrollLineNum;	// �L�[���s�[�g����Scroll�s��
		}
		caret.Cursor_UPDOWN(i, bSelect);
		nRepeat = -1 * i;
	}else {
		++nRepeat;
		if (caret.Cursor_UPDOWN(-1, bSelect) != 0 && bRepeat) {
			for (int i=0; i<csGeneral.nRepeatedScrollLineNum-1; ++i) {		// �L�[���s�[�g����Scroll�s��
				::UpdateWindow(view.GetHwnd());	// YAZAKI
				caret.Cursor_UPDOWN(-1, bSelect);
				++nRepeat;
			}
		}
	}
	return nRepeat;
}


// �J�[�\�����ړ�
int ViewCommander::Command_Down(bool bSelect, bool bRepeat)
{
	auto& caret = GetCaret();
	int nRepeat = 0;
	auto& csGeneral = GetDllShareData().common.general;
	// �L�[���s�[�g����Scroll�����炩�ɂ��邩
	if (!csGeneral.nRepeatedScroll_Smooth) {
		int i;
		if (!bRepeat) {
			i = 1;
		}else {
			i = csGeneral.nRepeatedScrollLineNum;	// �L�[���s�[�g����Scroll�s��
		}
		caret.Cursor_UPDOWN(i, bSelect);
		nRepeat = i;
	}else {
		++nRepeat;
		if (caret.Cursor_UPDOWN(1, bSelect) != 0 && bRepeat) {
			for (int i=0; i<csGeneral.nRepeatedScrollLineNum-1; ++i) {	// �L�[���s�[�g����Scroll�s��
				// �����ōĕ`��B
				::UpdateWindow(view.GetHwnd());	// YAZAKI
				caret.Cursor_UPDOWN(1, bSelect);
				++nRepeat;
			}
		}
	}
	return nRepeat;
}


/*! @brief �J�[�\�����ړ�

	@date 2004.03.28 Moca EOF�����̍s�ȍ~�̓r���ɃJ�[�\��������Ɨ�����o�O�C���D
			pLayout == NULL���L�����b�g�ʒu���s���ȊO�̏ꍇ��
			2��if�̂ǂ���ɂ����Ă͂܂�Ȃ����C���̂��Ƃ�MoveCursor�ɂēK����
			�ʒu�Ɉړ���������D
	@date 2014.01.10 Moca �L�[���s�[�g���AMoveCursor����x�ɂ܂Ƃ߂�
*/
int ViewCommander::Command_Left(bool bSelect, bool bRepeat)
{
	bool bUnderlineDoNotOFF = true;		// �A���_�[���C�����������Ȃ�
	if (bSelect) {
		bUnderlineDoNotOFF = false;		// �I����ԂȂ�A���_�[���C���������s��
	}
	bool bMoveCaretLine = false;
	int nRepeat = bRepeat ? 2 : 1;
	int nRes = 0;
	auto& caret = GetCaret();
	Point ptCaretMove = caret.GetCaretLayoutPos();
	auto& selInfo = view.GetSelectionInfo();
	for (int nRepCount=0; nRepCount<nRepeat; ++nRepCount) {
		if (bSelect && ! selInfo.IsTextSelected()) {
			// ���݂̃J�[�\���ʒu����I�����J�n����
			selInfo.BeginSelectArea();
		}
		if (!bSelect) {
			if (selInfo.IsTextSelected()) {
				this->Command_Cancel_Mode(1);
				nRes = 1;
				continue; // �I���̃L�����Z���ō��ړ��� 1����B���̌�̈ړ������̓X�L�b�v����B
			}else if (selInfo.IsBoxSelecting()) {
				selInfo.SetBoxSelect(false);
			}
		}
		// (���ꂩ�狁�߂�)�J�[�\���̈ړ���B
		Point ptPos(0, ptCaretMove.y);

		auto& layoutMgr = GetDocument().layoutMgr;
		// ���ݍs�̃f�[�^���擾
		const Layout* pLayout = layoutMgr.SearchLineByLayoutY(ptCaretMove.y);
		// �J�[�\�������[�ɂ���
		if (ptCaretMove.x == (pLayout ? pLayout->GetIndent() : 0)) {
			if (0 < ptCaretMove.y
			   && ! selInfo.IsBoxSelecting()
			) {
				// �O�̃��C�A�E�g�s�́A�܂�Ԃ������O�܂��͉��s�����̎�O�Ɉړ�����B
				pLayout = layoutMgr.SearchLineByLayoutY(ptCaretMove.y - 1);
				MemoryIterator it(pLayout, layoutMgr.GetTabSpace());
				while (!it.end()) {
					it.scanNext();
					if (it.getIndex() + it.getIndexDelta() > pLayout->GetLengthWithoutEOL()) {
						ptPos.x += it.getColumnDelta();
						break;
					}
					it.addDelta();
				}
				ptPos.x += it.getColumn() - it.getColumnDelta();
				ptPos.y --;
			}else {
				if (0 < nRepCount) {
					caret.MoveCursor( ptCaretMove, true, _CARETMARGINRATE, bUnderlineDoNotOFF );
					caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
				}
				nRes = 0;
				break; // ����ȏ㍶�ɓ����ʁB
			}
			bUnderlineDoNotOFF = false;	// �s���ς��̂ŃA���_�[���C������������
			bMoveCaretLine = true;
		}
		//  2004.03.28 Moca EOF�����̍s�ȍ~�̓r���ɃJ�[�\��������Ɨ�����o�O�C��
		else if (pLayout) {
			MemoryIterator it(pLayout, layoutMgr.GetTabSpace());
			while (!it.end()) {
				it.scanNext();
				if ((int)(it.getColumn() + it.getColumnDelta()) > ptCaretMove.x - 1){
					ptPos.x += it.getColumnDelta();
					break;
				}
				it.addDelta();
			}
			ptPos.x += it.getColumn() - it.getColumnDelta();
			// Oct. 18, 2002 YAZAKI
			if (it.getIndex() >= pLayout->GetLengthWithEOL()) {
				ptPos.x = ptCaretMove.x - 1;
			}
		}

		caret.GetAdjustCursorPos( &ptPos );
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
			caret.MoveCursor( ptPos, true, _CARETMARGINRATE, bUnderlineDoNotOFF );
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
		}
		ptCaretMove = ptPos;
		nRes = 1;
	}
	return nRes;
}


/* �J�[�\���E�ړ�
	@date 2014.01.10 Moca �L�[���s�[�g���AMoveCursor����x�ɂ܂Ƃ߂�
*/
void ViewCommander::Command_Right(
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
	auto& caret = GetCaret();
	Point ptCaretMove = caret.GetCaretLayoutPos();
	auto& selInfo = view.GetSelectionInfo();
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
					this->Command_Cancel_Mode(2);
					continue; // �I���̃L�����Z���ŉE�ړ��� 1����B���̌�̈ړ������̓X�L�b�v����B
				}else if (selInfo.IsBoxSelecting()) {
					selInfo.SetBoxSelect(false);
				}
			}
		}
//		2003.06.28 Moca [EOF]�݂̂̍s�ɃJ�[�\��������Ƃ��ɉE�������Ă��I���������ł��Ȃ����ɑΉ�

		// (���ꂩ�狁�߂�)�J�[�\���̈ړ���B
		Point ptTo(0, 0);
		const Point ptCaret = ptCaretMove;

		auto& layoutMgr = GetDocument().layoutMgr;
		// ���ݍs�̃f�[�^���擾
		const Layout* const pcLayout = layoutMgr.SearchLineByLayoutY(ptCaret.y);
		// 2004.04.02 EOF�ȍ~�ɃJ�[�\�����������Ƃ��ɉE�������Ă������N���Ȃ������̂��AEOF�Ɉړ�����悤��
		if (pcLayout) {
			// �L�����b�g�ʒu�̃��C�A�E�g�s�ɂ��āB
			const size_t x_wrap = pcLayout->CalcLayoutWidth(layoutMgr); // ���s�����A�܂��͐܂�Ԃ��̈ʒu�B
			const bool wrapped = EolType::None == pcLayout->GetLayoutEol(); // �܂�Ԃ��Ă��邩�A���s�����ŏI����Ă��邩�B����ɂ�� x_wrap�̈Ӗ����ς��B
			const bool nextline_exists = pcLayout->GetNextLayout() || pcLayout->GetLayoutEol() != EolType::None; // EOF�݂̂̍s���܂߁A�L�����b�g���ړ��\�Ȏ��s�����݂��邩�B

			// ���݂̃L�����b�g�̉E�̈ʒu(to_x)�����߂�B
			MemoryIterator it(pcLayout, layoutMgr.GetTabSpace());
			for (; !it.end(); it.scanNext(), it.addDelta()) {
				if (ptCaret.x < (int)it.getColumn()) {
					break;
				}
			}
			ASSERT_GE(ptCaret.x, -2);
			const size_t to_x = t_max(it.getColumn(), (size_t)ptCaret.x + 1);

			// �L�����b�g�̉E�[(x_max)�ƁA�����ł̈���(on_x_max)�����߂�B
			size_t x_max;
			enum {
				STOP,
				MOVE_NEXTLINE_IMMEDIATELY, // �E�[�Ɏ~�܂炸���̍s���Ɉړ�����B(�܂�Ԃ��Ȃ�)
				MOVE_NEXTLINE_NEXTTIME, // �E�[�Ɏ~�܂�A���Ɏ��̍s���Ɉړ�����B(���s�𒴂���Ƃ��Ȃ�)
				MOVE_NEXTLINE_NEXTTIME_AND_MOVE_RIGHT // �E�[�Ɏ~�܂�A���Ɏ��̍s���̈�E�Ɉړ�����B(�܂�Ԃ��Ȃ�)
			} on_x_max;

			if (selInfo.IsBoxSelecting()) {
				x_max = t_max(x_wrap, layoutMgr.GetMaxLineKetas());
				on_x_max = STOP;
			}else if (GetDllShareData().common.general.bIsFreeCursorMode) {
				// �t���[�J�[�\�����[�h�ł͐܂�Ԃ��ʒu�������݂āA���s�����̈ʒu�݂͂Ȃ��B
				if (wrapped) {
					if (nextline_exists) {
						x_max = x_wrap;
						on_x_max = MOVE_NEXTLINE_IMMEDIATELY;
					}else {
						// �f�[�^�̂���EOF�s�͐܂�Ԃ��ł͂Ȃ�
						x_max = t_max(x_wrap, layoutMgr.GetMaxLineKetas());
						on_x_max = STOP;
					}
				}else {
					if (x_wrap < layoutMgr.GetMaxLineKetas()) {
						x_max = layoutMgr.GetMaxLineKetas();
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
				ptTo.x = pcLayout->GetNextLayout() ? pcLayout->GetNextLayout()->GetIndent() : 0;
				if (on_x_max == MOVE_NEXTLINE_NEXTTIME_AND_MOVE_RIGHT) {
					++nRepeat;
				}
				bUnderlineDoNotOFF = false;
				bMoveCaretLine = true;
			}else {
				ptTo.y = ptCaret.y;
				ptTo.x = (int)t_min(to_x, x_max);
			}
		}else {
			// pLayout��NULL�̏ꍇ��ptPos.x=0�ɒ���
			ptTo.y = ptCaret.y;
			ptTo.x = 0;
		}

		caret.GetAdjustCursorPos( &ptTo );
		if (bSelect) {
			// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
			selInfo.ChangeSelectAreaByCurrentCursor( ptTo );
		}

		if (bMoveCaretLine || nRepeat - 1 == nRepCount) {
			caret.MoveCursor( ptTo, true, _CARETMARGINRATE, bUnderlineDoNotOFF );
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
		}
		ptCaretMove = ptTo;
	}
	return;
}


// �J�[�\����ړ�(�Q�s�Â�)
void ViewCommander::Command_Up2(bool bSelect)
{
	GetCaret().Cursor_UPDOWN(-2, bSelect);
	return;
}


// �J�[�\�����ړ�(�Q�s�Â�)
void ViewCommander::Command_Down2(bool bSelect)
{
	GetCaret().Cursor_UPDOWN(2, bSelect);
	return;
}


// �P��̍��[�Ɉړ�
void ViewCommander::Command_WordLeft(bool bSelect)
{
	bool bUnderlineDoNotOFF = true;		// �A���_�[���C�����������Ȃ�
	if (bSelect) {
		bUnderlineDoNotOFF = false;		// �I����ԂȂ�A���_�[���C���������s��
	}
	auto& si = view.GetSelectionInfo();
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
	auto& csGeneral = GetDllShareData().common.general;
	auto& layoutMgr = GetDocument().layoutMgr;
	const Layout* pLayout = layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().y);
	if (!pLayout) {
		bool bIsFreeCursorModeOld = csGeneral.bIsFreeCursorMode;	// �t���[�J�[�\�����[�h��
		csGeneral.bIsFreeCursorMode = false;
		// �J�[�\�����ړ�
		Command_Left(bSelect, false);
		csGeneral.bIsFreeCursorMode = bIsFreeCursorModeOld;	// �t���[�J�[�\�����[�h��
		return;
	}

	// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
	size_t nIdx = view.LineColumnToIndex(pLayout, caret.GetCaretLayoutPos().x);
	// ���݈ʒu�̍��̒P��̐擪�ʒu�𒲂ׂ�
	Point ptLayoutNew;
	int nResult = layoutMgr.PrevWord(
		caret.GetCaretLayoutPos().y,
		nIdx,
		&ptLayoutNew,
		csGeneral.bStopsBothEndsWhenSearchWord
	);
	if (nResult) {
		// �s���ς����
		if (ptLayoutNew.y != caret.GetCaretLayoutPos().y) {
			pLayout = layoutMgr.SearchLineByLayoutY(ptLayoutNew.y);
			if (!pLayout) {
				return;
			}
			bUnderlineDoNotOFF = false;
		}

		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
		// 2007.10.15 kobake ���Ƀ��C�A�E�g�P�ʂȂ̂ŕϊ��͕s�v
		/*
		ptLayoutNew.x = view.LineIndexToColumn(pLayout, ptLayoutNew.x);
		*/

		// �J�[�\���ړ�
		caret.GetAdjustCursorPos(&ptLayoutNew);
		if (bSelect) {
			// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
			si.ChangeSelectAreaByCurrentCursor(ptLayoutNew);
		}
		caret.MoveCursor(ptLayoutNew, true, _CARETMARGINRATE, bUnderlineDoNotOFF);
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
	}else {
		bool bIsFreeCursorModeOld = csGeneral.bIsFreeCursorMode;	// �t���[�J�[�\�����[�h��
		csGeneral.bIsFreeCursorMode = false;
		// �J�[�\�����ړ�
		Command_Left(bSelect, false);
		csGeneral.bIsFreeCursorMode = bIsFreeCursorModeOld;	// �t���[�J�[�\�����[�h��
	}
	return;
}


// �P��̉E�[�Ɉړ�
void ViewCommander::Command_WordRight(bool bSelect)
{
	bool bUnderlineDoNotOFF = true;	// �A���_�[���C�����������Ȃ�
	if (bSelect) {
		bUnderlineDoNotOFF = false;		// �I����ԂȂ�A���_�[���C���������s��
	}
	int nCurLine;
	auto& si = view.GetSelectionInfo();
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
	bool bTryAgain = false;
try_again:;
	auto& caret = GetCaret();
	nCurLine = caret.GetCaretLayoutPos().y;
	const Layout* pLayout;
	auto& layoutMgr = GetDocument().layoutMgr;
	pLayout = layoutMgr.SearchLineByLayoutY(nCurLine);
	if (!pLayout) {
		return;
	}
	if (bTryAgain) {
		const wchar_t*	pLine = pLayout->GetPtr();
		if (pLine[0] != L' ' && pLine[0] != WCODE::TAB) {
			return;
		}
	}
	// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
	size_t nIdx = view.LineColumnToIndex(pLayout, caret.GetCaretLayoutPos().x);
	auto& csGeneral = GetDllShareData().common.general;	
	// ���݈ʒu�̉E�̒P��̐擪�ʒu�𒲂ׂ�
	Point ptLayoutNew;
	int nResult = layoutMgr.NextWord(
		nCurLine,
		nIdx,
		&ptLayoutNew,
		csGeneral.bStopsBothEndsWhenSearchWord
	);
	if (nResult) {
		// �s���ς����
		if (ptLayoutNew.y != nCurLine) {
			pLayout = layoutMgr.SearchLineByLayoutY(ptLayoutNew.y);
			if (!pLayout) {
				return;
			}
			bUnderlineDoNotOFF = false;
		}
		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
		// 2007.10.15 kobake ���Ƀ��C�A�E�g�P�ʂȂ̂ŕϊ��͕s�v
		/*
		ptLayoutNew.x = view.LineIndexToColumn(pLayout, ptLayoutNew.x);
		*/
		// �J�[�\���ړ�
		caret.GetAdjustCursorPos(&ptLayoutNew);
		if (bSelect) {
			// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
			si.ChangeSelectAreaByCurrentCursor(ptLayoutNew);
		}
		caret.MoveCursor(ptLayoutNew, true, _CARETMARGINRATE, bUnderlineDoNotOFF);
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
	}else {
		bool bIsFreeCursorModeOld = csGeneral.bIsFreeCursorMode;	// �t���[�J�[�\�����[�h��
		csGeneral.bIsFreeCursorMode = false;
		// �J�[�\���E�ړ�
		Command_Right(bSelect, false, false);
		csGeneral.bIsFreeCursorMode = bIsFreeCursorModeOld;	// �t���[�J�[�\�����[�h��
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
void ViewCommander::Command_GoLineTop(
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

	Point ptCaretPos;
	auto& caret = GetCaret();
	auto& layoutMgr = GetDocument().layoutMgr;
	if (lparam & 8) {
		// ���s�P�ʎw��̏ꍇ�́A�����s���ʒu����ړI�_���ʒu�����߂�
		ptCaretPos = layoutMgr.LogicToLayout(Point(0, caret.GetCaretLogicPos().y));
	}else {
		const Layout* pLayout = layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().y);
		ptCaretPos.x = pLayout ? pLayout->GetIndent() : 0;
		ptCaretPos.y = caret.GetCaretLayoutPos().y;
	}
	if (!bLineTopOnly) {
		// �ړI�s�̃f�[�^���擾
		// ���s�P�ʎw��ŁA�擪����󔒂�1�܂�Ԃ��s�ȏ㑱���Ă���ꍇ�͎��̍s�f�[�^���擾
		int nPosY_Layout;
		size_t nPosX_Logic;
		nPosY_Layout = ptCaretPos.y - 1;
		const Layout* pLayout;
		bool bZenSpace = view.pTypeData->bAutoIndent_ZENSPACE;
		bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
		size_t nLineLen;
		do {
			++nPosY_Layout;
			ASSERT_GE(nPosY_Layout, 0);
			const wchar_t*	pLine = layoutMgr.GetLineStr(nPosY_Layout, &nLineLen, &pLayout);
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
		while ((lparam & 8) && (nPosX_Logic >= nLineLen) && ((int)layoutMgr.GetLineCount() - 1 > nPosY_Layout));
		
		if (nPosX_Logic >= nLineLen) {
			/* �܂�Ԃ��P�ʂ̍s����T���ĕ����s���܂œ��B����
			�܂��́A�ŏI�s�̂��߉��s�R�[�h�ɑ��������ɍs���ɓ��B���� */
			nPosX_Logic = 0;
		}
		
		if (nPosX_Logic == 0) nPosY_Layout = ptCaretPos.y;	// �����s�̈ړ��Ȃ�
		
		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
		size_t nPosX_Layout = view.LineIndexToColumn(pLayout, nPosX_Logic);
		Point ptPos((int)nPosX_Layout, nPosY_Layout);
		if (caret.GetCaretLayoutPos() != ptPos) {
			ptCaretPos = ptPos;
		}
	}

	// 2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
	view.MoveCursorSelecting(ptCaretPos, bSelect);
}


/*! �s���Ɉړ�(�܂�Ԃ��P��)
	@praram nOption	0x08 ���s�P��(������)
*/
void ViewCommander::Command_GoLineEnd(
	bool bSelect,
	int bIgnoreCurrentSelection,
	int nOption
	)
{
	if (nOption & 4) {
		bSelect = true;
	}
	auto& si = view.GetSelectionInfo();
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
	auto& layoutMgr = GetDocument().layoutMgr;
	Point nPosXY = caret.GetCaretLayoutPos();
	if (nOption & 8) {
		// ���s�P�ʂ̍s���B1�s���̍ŏI���C�A�E�g�s��T��
		const Layout*	pLayout = layoutMgr.SearchLineByLayoutY(nPosXY.y);
		const Layout*	pLayoutNext = pLayout->GetNextLayout();
		while (pLayout && pLayoutNext && pLayoutNext->GetLogicOffset() != 0) {
			pLayout = pLayoutNext;
			pLayoutNext = pLayoutNext->GetNextLayout();
			++nPosXY.y;
		}
	}
	nPosXY.x = 0;
	const Layout* pLayout = layoutMgr.SearchLineByLayoutY(nPosXY.y);
	if (pLayout) {
		nPosXY.x = pLayout->CalcLayoutWidth(layoutMgr);
	}

	// �L�����b�g�ړ�
	caret.GetAdjustCursorPos(&nPosXY);
	if (bSelect) {
		// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
		si.ChangeSelectAreaByCurrentCursor(nPosXY);
	}
	caret.MoveCursor(nPosXY, true);
	caret.nCaretPosX_Prev = nPosXY.x;
}


// ���y�[�W�A�b�v		// Oct. 6, 2000 JEPRO added (���͏]����Scroll Down���̂���)
void ViewCommander::Command_HalfPageUp(
	bool bSelect,
	int nScrollNum
	)
{
	if (nScrollNum <= 0) {
		nScrollNum = view.GetTextArea().nViewRowNum / 2;
	}
	GetCaret().Cursor_UPDOWN( - (nScrollNum), bSelect );
	return;
}


// ���y�[�W�_�E��		// Oct. 6, 2000 JEPRO added (���͏]����Scroll Up���̂���)
void ViewCommander::Command_HalfPageDown(
	bool bSelect,
	int nScrollNum
	)
{
	if (nScrollNum <= 0) {
		nScrollNum = view.GetTextArea().nViewRowNum / 2;
	}
	GetCaret().Cursor_UPDOWN( nScrollNum, bSelect );
	return;
}


/*! �P�y�[�W�A�b�v

	@date 2000.10.10 JEPRO �쐬
	@date 2001.12.13 hor ��ʂɑ΂���J�[�\���ʒu�͂��̂܂܂�
		�P�y�[�W�A�b�v�ɓ���ύX
	@date 2014.01.10 Moca �J�[�\���������Ȃ��Ƃ�����ʂ�Scroll����悤��
*/	// Oct. 10, 2000 JEPRO added
void ViewCommander::Command_1PageUp(
	bool bSelect,
	int nScrollNum
	)
{
// GetCaret().Cursor_UPDOWN(-view.GetTextArea().nViewRowNum, bSelect);

// 2001.12.03 hor
//		���������C�N�ɁA��ʂɑ΂���J�[�\���ʒu�͂��̂܂܂łP�y�[�W�A�b�v
	{
		const bool bDrawSwitchOld = view.SetDrawSwitch(false);
		auto& textArea = view.GetTextArea();
		int nViewTopLine = textArea.GetViewTopLine();
		if (nScrollNum <= 0) {
			nScrollNum = view.GetTextArea().nViewRowNum - 1;
		}
		GetCaret().Cursor_UPDOWN( -nScrollNum, bSelect );
		// Sep. 11, 2004 genta ����Scroll�����̂���
		// view.RedrawAll�ł͂Ȃ�ScrollAt���g���悤��
		view.SyncScrollV( view.ScrollAtV( nViewTopLine - nScrollNum ));
		view.SetDrawSwitch(bDrawSwitchOld);
		view.RedrawAll();
	}
	return;
}


/*!	�P�y�[�W�_�E��

	@date 2000.10.10 JEPRO �쐬
	@date 2001.12.13 hor ��ʂɑ΂���J�[�\���ʒu�͂��̂܂܂�
		�P�y�[�W�_�E���ɓ���ύX
	@date 2014.01.10 Moca �J�[�\���������Ȃ��Ƃ�����ʂ�Scroll����悤��
*/
void ViewCommander::Command_1PageDown(
	bool bSelect,
	int nScrollNum
	)
{
// GetCaret().Cursor_UPDOWN(view.GetTextArea().nViewRowNum, bSelect);

// 2001.12.03 hor
//		���������C�N�ɁA��ʂɑ΂���J�[�\���ʒu�͂��̂܂܂łP�y�[�W�_�E��
	{
		const bool bDrawSwitchOld = view.SetDrawSwitch(false);
		int nViewTopLine = view.GetTextArea().GetViewTopLine();
		if (nScrollNum <= 0) {
			nScrollNum = view.GetTextArea().nViewRowNum - 1;
		}
		GetCaret().Cursor_UPDOWN(nScrollNum, bSelect);
		// Sep. 11, 2004 genta ����Scroll�����̂���
		// view.RedrawAll�ł͂Ȃ�ScrollAt���g���悤��
		view.SyncScrollV(view.ScrollAtV(nViewTopLine + nScrollNum));
		view.SetDrawSwitch(bDrawSwitchOld);
		view.RedrawAll();
	}

	return;
}


// �t�@�C���̐擪�Ɉړ�
void ViewCommander::Command_GoFileTop(bool bSelect)
{
	// �擪�փJ�[�\�����ړ�
	// Sep. 8, 2000 genta
	view.AddCurrentLineToHistory();

	// 2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
	Point pt(
		!view.GetSelectionInfo().IsBoxSelecting()? 0: GetCaret().GetCaretLayoutPos().x,
		0
	);
	view.MoveCursorSelecting(pt, bSelect);	// �ʏ�́A(0, 0)�ֈړ��B�{�b�N�X�I�𒆂́A(GetCaret().GetCaretLayoutPos().x, 0)�ֈړ�
}


// �t�@�C���̍Ō�Ɉړ�
void ViewCommander::Command_GoFileEnd(bool bSelect)
{
	auto& si = view.GetSelectionInfo();
// 2001.12.13 hor BOX�I�𒆂Ƀt�@�C���̍Ō�ɃW�����v�����[EOF]�̍s�����]�����܂܂ɂȂ�̏C��
	if (!bSelect) {
		if (si.IsTextSelected()) {
			si.DisableSelectArea(true);	// 2001.12.21 hor Add
		}else if (si.IsBoxSelecting()) {
			si.SetBoxSelect(false);
		}
	}
	view.AddCurrentLineToHistory();
	auto& caret = GetCaret();
	caret.Cursor_UPDOWN((int)GetDocument().layoutMgr.GetLineCount() , bSelect);
	Command_Down(bSelect, true);
	if (!si.IsBoxSelecting()) {							// 2002/04/18 YAZAKI
		/*	2004.04.19 fotomo
			���s�̂Ȃ��ŏI�s�őI�����Ȃ��當�����ֈړ������ꍇ��
			�I��͈͂��������Ȃ��ꍇ��������ɑΉ�
		*/
		Command_GoLineEnd(bSelect, 0, 0);				// 2001.12.21 hor Add
	}
	caret.MoveCursor(caret.GetCaretLayoutPos(), true);	// 2001.12.21 hor Add
	// 2002.02.16 hor ��`�I�𒆂��������O�̃J�[�\���ʒu�����Z�b�g
	if (!(si.IsTextSelected() && si.IsBoxSelecting())) {
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
	}

	// �I��͈͏�񃁃b�Z�[�W��\������	// 2009.05.06 ryoji �ǉ�
	if (bSelect) {
		si.PrintSelectionInfoMsg();
	}
}


// �J�[�\���s���E�B���h�E������
void ViewCommander::Command_CurLineCenter(void)
{
	int nViewTopLine;
	auto& textArea = view.GetTextArea();
	nViewTopLine = GetCaret().GetCaretLayoutPos().y - (textArea.nViewRowNum / 2);

	// sui 02/08/09
	if (0 > nViewTopLine) {
		nViewTopLine = 0;
	}
	
	int nScrollLines = nViewTopLine - textArea.GetViewTopLine();	// Sep. 11, 2004 genta �����p�ɍs�����L��
	textArea.SetViewTopLine(nViewTopLine);
	// �t�H�[�J�X�ړ����̍ĕ`��
	view.RedrawAll();
	// sui 02/08/09

	// Sep. 11, 2004 genta ����Scroll�̊֐���
	view.SyncScrollV(nScrollLines);
}


// �ړ�������O�ւ��ǂ�
void ViewCommander::Command_JumpHist_Prev(void)
{
	// 2001.12.13 hor
	// �ړ������̍Ō�Ɍ��݂̈ʒu���L������
	// (���̗������擾�ł��Ȃ��Ƃ��͒ǉ����Ė߂�)
	if (!view.pHistory->CheckNext()) {
		view.AddCurrentLineToHistory();
		view.pHistory->PrevValid();
	}

	if (view.pHistory->CheckPrev()) {
		if (! view.pHistory->PrevValid()) {
			::MessageBox(NULL, _T("Inconsistent Implementation"), _T("PrevValid"), MB_OK);
		}
		Point pt = GetDocument().layoutMgr.LogicToLayout(view.pHistory->GetCurrent().GetPosition());
		// 2006.07.09 genta �I�����l��
		view.MoveCursorSelecting(pt, view.GetSelectionInfo().bSelectingLock);
	}
}


// �ړ����������ւ��ǂ�
void ViewCommander::Command_JumpHist_Next(void)
{
	if (view.pHistory->CheckNext()) {
		if (!view.pHistory->NextValid()) {
			::MessageBox(NULL, _T("Inconsistent Implementation"), _T("NextValid"), MB_OK);
		}
		Point pt = GetDocument().layoutMgr.LogicToLayout(view.pHistory->GetCurrent().GetPosition());
		// 2006.07.09 genta �I�����l��
		view.MoveCursorSelecting(pt, view.GetSelectionInfo().bSelectingLock);
	}
}


// ���݈ʒu���ړ������ɓo�^����
void ViewCommander::Command_JumpHist_Set(void)
{
	view.AddCurrentLineToHistory();
}


// 2001/06/20 Start by asa-o

// from ViewCommander_New.cpp
// �e�L�X�g���P�s����Scroll
void ViewCommander::Command_WndScrollDown(void)
{
	int nCaretMarginY;

	auto& textArea = view.GetTextArea();
	nCaretMarginY = textArea.nViewRowNum / _CARETMARGINRATE;
	if (nCaretMarginY < 1) {
		nCaretMarginY = 1;
	}

	nCaretMarginY += 2;

	bool bCaretOff = false;
	auto& caret = GetCaret();
	if (caret.GetCaretLayoutPos().GetY() > textArea.nViewRowNum + textArea.GetViewTopLine() - (nCaretMarginY + 1)) {
		bCaretOff = true;
	}

	// Sep. 11, 2004 genta �����p�ɍs�����L��
	// Sep. 11, 2004 genta ����Scroll�̊֐���
	view.SyncScrollV(view.ScrollAtV(textArea.GetViewTopLine() - 1));

	// �e�L�X�g���I������Ă��Ȃ�
	if (!view.GetSelectionInfo().IsTextSelected()) {
		// �J�[�\������ʊO�ɏo��
		if (caret.GetCaretLayoutPos().GetY() > textArea.nViewRowNum + textArea.GetViewTopLine() - nCaretMarginY) {
			if (caret.GetCaretLayoutPos().GetY() > (int)GetDocument().layoutMgr.GetLineCount() - nCaretMarginY) {
				caret.Cursor_UPDOWN((GetDocument().layoutMgr.GetLineCount() - nCaretMarginY) - caret.GetCaretLayoutPos().y, false);
			}else {
				caret.Cursor_UPDOWN(-1, false);
			}
			caret.ShowCaretPosInfo();
		}
	}
	if (bCaretOff) {
		caret.underLine.CaretUnderLineOFF(true);
	}
	caret.underLine.CaretUnderLineON(true, true);
}


// from ViewCommander_New.cpp
// �e�L�X�g���P�s���Scroll
void ViewCommander::Command_WndScrollUp(void)
{
	auto& textArea = view.GetTextArea();
	int nCaretMarginY = textArea.nViewRowNum / _CARETMARGINRATE;
	if (nCaretMarginY < 1)
		nCaretMarginY = 1;

	bool bCaretOff = false;
	auto& caret = GetCaret();
	if (caret.GetCaretLayoutPos().y < textArea.GetViewTopLine() + (nCaretMarginY + 1)) {
		bCaretOff = true;
	}

	// Sep. 11, 2004 genta �����p�ɍs�����L��
	// Sep. 11, 2004 genta ����Scroll�̊֐���
	view.SyncScrollV(view.ScrollAtV(textArea.GetViewTopLine() + 1));

	// �e�L�X�g���I������Ă��Ȃ�
	if (!view.GetSelectionInfo().IsTextSelected()) {
		// �J�[�\������ʊO�ɏo��
		if (caret.GetCaretLayoutPos().GetY() < textArea.GetViewTopLine() + nCaretMarginY) {
			if (textArea.GetViewTopLine() == 1) {
				caret.Cursor_UPDOWN(nCaretMarginY + 1, false);
			}else {
				caret.Cursor_UPDOWN(1, false);
			}
			caret.ShowCaretPosInfo();
		}
	}
	if (bCaretOff) {
		caret.underLine.CaretUnderLineOFF(true);
	}
	caret.underLine.CaretUnderLineON(true, true);
}

// 2001/06/20 End


// from ViewCommander_New.cpp
/* ���̒i���֐i��
	2002/04/26 �i���̗��[�Ŏ~�܂�I�v�V������ǉ�
	2002/04/19 �V�K
*/
void ViewCommander::Command_GoNextParagraph(bool bSelect)
{
	DocLine* pDocLine;
	int nCaretPointer = 0;
	auto& docLineMgr = GetDocument().docLineMgr;
	auto& caret = GetCaret();
	
	bool nFirstLineIsEmptyLine = false;
	// �܂��́A���݈ʒu����s�i�X�y�[�X�A�^�u�A���s�L���݂̂̍s�j���ǂ�������
	if ((pDocLine = docLineMgr.GetLine(caret.GetCaretLogicPos().y + nCaretPointer))) {
		nFirstLineIsEmptyLine = pDocLine->IsEmptyLine();
		++nCaretPointer;
	}else {
		// EOF�s�ł����B
		return;
	}

	// ���ɁAnFirstLineIsEmptyLine�ƈقȂ�Ƃ���܂œǂݔ�΂�
	while ((pDocLine = docLineMgr.GetLine(caret.GetCaretLogicPos().y + nCaretPointer))) {
		if (pDocLine->IsEmptyLine() == nFirstLineIsEmptyLine) {
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
		if (GetDllShareData().common.general.bStopsBothEndsWhenSearchParagraph) {	// �i���̗��[�Ŏ~�܂�
		}else {
			// �d�グ�ɁA��s����Ȃ��Ƃ���܂Ői��
			while ((pDocLine = docLineMgr.GetLine(caret.GetCaretLogicPos().y + nCaretPointer))) {
				if (pDocLine->IsEmptyLine()) {
					++nCaretPointer;
				}else {
					break;
				}
			}
		}
	}

	// EOF�܂ŗ�����A�ړI�̏ꏊ�܂ł����̂ňړ��I���B

	// �ړ��������v�Z
	Point ptCaretPos_Layo;

	// �ړ��O�̕����ʒu
	ptCaretPos_Layo = GetDocument().layoutMgr.LogicToLayout(caret.GetCaretLogicPos());

	// �ړ���̕����ʒu
	Point ptCaretPos_Layo_CaretPointer;
	ptCaretPos_Layo_CaretPointer = GetDocument().layoutMgr.LogicToLayout(caret.GetCaretLogicPos() + Point(0, nCaretPointer));

	caret.Cursor_UPDOWN(ptCaretPos_Layo_CaretPointer.y - ptCaretPos_Layo.y, bSelect);
}


// from ViewCommander_New.cpp
/* �O�̒i���֐i��
	2002/04/26 �i���̗��[�Ŏ~�܂�I�v�V������ǉ�
	2002/04/19 �V�K
*/
void ViewCommander::Command_GoPrevParagraph(bool bSelect)
{
	auto& docLineMgr = GetDocument().docLineMgr;
	DocLine* pDocLine;
	int nCaretPointer = -1;

	bool nFirstLineIsEmptyLine = false;
	auto& caret = GetCaret();
	// �܂��́A���݈ʒu����s�i�X�y�[�X�A�^�u�A���s�L���݂̂̍s�j���ǂ�������
	if ((pDocLine = docLineMgr.GetLine(caret.GetCaretLogicPos().y + nCaretPointer))) {
		nFirstLineIsEmptyLine = pDocLine->IsEmptyLine();
		--nCaretPointer;
	}else {
		nFirstLineIsEmptyLine = true;
		--nCaretPointer;
	}

	// ���ɁAnFirstLineIsEmptyLine�ƈقȂ�Ƃ���܂œǂݔ�΂�
	while ((pDocLine = docLineMgr.GetLine(caret.GetCaretLogicPos().y + nCaretPointer))) {
		if (pDocLine->IsEmptyLine() == nFirstLineIsEmptyLine) {
			--nCaretPointer;
		}else {
			break;
		}
	}

	/*	nFirstLineIsEmptyLine����s��������A�����Ă���Ƃ���͔��s�B���Ȃ킿�����܂��B
		nFirstLineIsEmptyLine�����s��������A�����Ă���Ƃ���͋�s�B
	*/
	auto& csGeneral = GetDllShareData().common.general;	
	if (nFirstLineIsEmptyLine) {
		// �����܂��B
		if (csGeneral.bStopsBothEndsWhenSearchParagraph) {	// �i���̗��[�Ŏ~�܂�
			++nCaretPointer;	// ��s�̍ŏ�s�i�i���̖��[�̎��̍s�j�Ŏ~�܂�B
		}else {
			// �d�グ�ɁA��s����Ȃ��Ƃ���܂Ői��
			while ((pDocLine = docLineMgr.GetLine(caret.GetCaretLogicPos().y + nCaretPointer))) {
				if (pDocLine->IsEmptyLine()) {
					break;
				}else {
					--nCaretPointer;
				}
			}
			++nCaretPointer;	// ��s�̍ŏ�s�i�i���̖��[�̎��̍s�j�Ŏ~�܂�B
		}
	}else {
		// ���܌��Ă���Ƃ���͋�s��1�s��
		if (csGeneral.bStopsBothEndsWhenSearchParagraph) {	// �i���̗��[�Ŏ~�܂�
			++nCaretPointer;
		}else {
			++nCaretPointer;
		}
	}

	// EOF�܂ŗ�����A�ړI�̏ꏊ�܂ł����̂ňړ��I���B

	// �ړ��������v�Z
	
	// �ړ��O�̕����ʒu
	Point ptCaretPos_Layo = GetDocument().layoutMgr.LogicToLayout(caret.GetCaretLogicPos());
	// �ړ���̕����ʒu
	Point ptCaretPos_Layo_CaretPointer = GetDocument().layoutMgr.LogicToLayout(caret.GetCaretLogicPos() + Point(0, nCaretPointer));
	
	caret.Cursor_UPDOWN(ptCaretPos_Layo_CaretPointer.y - ptCaretPos_Layo.y, bSelect);
}

void ViewCommander::Command_AutoScroll()
{
	if (view.nAutoScrollMode == 0) {
		GetCursorPos(&view.autoScrollMousePos);
		ScreenToClient(view.GetHwnd(), &view.autoScrollMousePos);
		view.bAutoScrollDragMode = false;
		view.AutoScrollEnter();
	}else {
		view.AutoScrollExit();
	}
}

void ViewCommander::Command_WheelUp(int zDelta)
{
	int zDelta2 = (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	view.OnMOUSEWHEEL2(wParam, lParam, false, F_WHEELUP);
}

void ViewCommander::Command_WheelDown(int zDelta)
{
	int zDelta2 = -1 * (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	view.OnMOUSEWHEEL2(wParam, lParam, false, F_WHEELDOWN);
}

void ViewCommander::Command_WheelLeft(int zDelta)
{
	int zDelta2 = -1 * (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	view.OnMOUSEWHEEL2(wParam, lParam, true, F_WHEELLEFT);
}

void ViewCommander::Command_WheelRight(int zDelta)
{
	int zDelta2 = (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	view.OnMOUSEWHEEL2(wParam, lParam, true, F_WHEELRIGHT);
}

void ViewCommander::Command_WheelPageUp(int zDelta)
{
	int zDelta2 = (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	view.OnMOUSEWHEEL2(wParam, lParam, false, F_WHEELPAGEUP);
}

void ViewCommander::Command_WheelPageDown(int zDelta)
{
	int zDelta2 = -1 * (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	view.OnMOUSEWHEEL2(wParam, lParam, false, F_WHEELPAGEDOWN);
}

void ViewCommander::Command_WheelPageLeft(int zDelta)
{
	int zDelta2 = -1 * (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	view.OnMOUSEWHEEL2(wParam, lParam, true, F_WHEELPAGELEFT);
}

void ViewCommander::Command_WheelPageRight(int zDelta)
{
	int zDelta2 = (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	view.OnMOUSEWHEEL2(wParam, lParam, true, F_WHEELPAGERIGHT);
}

/*! ���̕ύX�s��
	�ύX�s�̃u���b�N�̐擪�s�ƁA�ύX�s�̃u���b�N�̖���(���̍s��)�Ɉړ�����
*/
void ViewCommander::Command_ModifyLine_Next( bool bSelect )
{
	auto& docLineMgr = GetDocument().docLineMgr;
	int nYOld = GetCaret().GetCaretLogicPos().y;
	Point ptXY(0, nYOld);
	const DocLine* pDocLine = docLineMgr.GetLine(ptXY.y);
	const int nSaveSeq = GetDocument().docEditor.opeBuf.GetNoModifiedSeq();
	bool bModified = false;
	if (docLineMgr.GetLineCount() == 0) {
		return;
	}
	if (pDocLine) {
		bModified = ModifyVisitor().IsLineModified(pDocLine, nSaveSeq);
		ptXY.y++;
		pDocLine = pDocLine->GetNextLine();
	}
	for (int n=0; n<2; ++n) {
		while (pDocLine) {
			if (ModifyVisitor().IsLineModified(pDocLine, nSaveSeq) != bModified
				|| (
					ptXY.y == 0
					&& ModifyVisitor().IsLineModified(pDocLine, nSaveSeq)
				)
			) {
				Point ptLayout = GetDocument().layoutMgr.LogicToLayout(ptXY);
				view.MoveCursorSelecting(ptLayout, bSelect);
				if (nYOld >= ptXY.y) {
					view.SendStatusMessage(LS(STR_ERR_SRNEXT1));
				}
				return;
			}
			ptXY.y++;
			pDocLine = pDocLine->GetNextLine();
		}
		if (n == 0 && bModified) {
			const DocLine* pDocLineLast = docLineMgr.GetDocLineBottom();
			bool bSkip = false;
			Point pos;
			if (pDocLineLast) {
				if (pDocLineLast->GetEol() == EolType::None) {
					// �Ԃ牺����[EOF]
					pos.x = pDocLineLast->GetLengthWithoutEOL();
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
				Point ptLayout = GetDocument().layoutMgr.LogicToLayout(pos);
				view.MoveCursorSelecting( ptLayout, bSelect );
				return;
			}
		}
		if (n == 0) {
			ptXY.y = 0;
			pDocLine = docLineMgr.GetLine(ptXY.y);
			bModified = false;
		}
		if (!GetDllShareData().common.search.bSearchAll) {
			break;
		}
	}
	view.SendStatusMessage(LS(STR_ERR_SRNEXT2));
	AlertNotFound( view.GetHwnd(), false, LS(STR_MODLINE_NEXT_NOT_FOUND));
}

/*! �O�̕ύX�s��
	�ύX�s�̃u���b�N�̐擪�s�ƁA�ύX�s�̃u���b�N�̖���(���̍s��)�Ɉړ�����
	Command_ModifyLine_Next�Ɠ����ʒu�Ɏ~�܂�
*/
void ViewCommander::Command_ModifyLine_Prev( bool bSelect )
{
	auto& docLineMgr = GetDocument().docLineMgr;
	int nYOld = GetCaret().GetCaretLogicPos().y;
	int nYOld2 = nYOld;
	Point ptXY(0, nYOld);
	const DocLine* pDocLine = docLineMgr.GetLine(ptXY.y);
	const int nSaveSeq = GetDocument().docEditor.opeBuf.GetNoModifiedSeq();
	bool bModified = false;
	bool bLast = false;
	if (!pDocLine) {
		// [EOF]
		const DocLine* pDocLineLast = docLineMgr.GetLine(ptXY.y - 1);
		if (!pDocLineLast) {
			// 1�s���Ȃ�
			return;
		}
		bModified = ModifyVisitor().IsLineModified(pDocLineLast, nSaveSeq);
		ptXY.y--;
		pDocLine = pDocLineLast;
		bLast = true;
	}
	if (!bLast) {
		const DocLine* pDocLineLast = docLineMgr.GetDocLineBottom();
		if (pDocLineLast && pDocLineLast->GetEol() == EolType::None) {
			Point pos;
			pos.x = pDocLine->GetLengthWithoutEOL();
			pos.y = docLineMgr.GetLineCount() - 1;
			if (GetCaret().GetCaretLogicPos() == pos) {
				// �Ԃ牺����[EOF]�̈ʒu������
				bLast = true;
			}
		}
	}
	assert( pDocLine );
	bModified = ModifyVisitor().IsLineModified(pDocLine, nSaveSeq);
	nYOld2 = ptXY.y;
	ptXY.y--;
	pDocLine = pDocLine->GetPrevLine();
	if (pDocLine && !bLast) {
		bModified = ModifyVisitor().IsLineModified(pDocLine, nSaveSeq);
		nYOld2 = ptXY.y;
		ptXY.y--;
		pDocLine = pDocLine->GetPrevLine();
	}
	auto& layoutMgr = GetDocument().layoutMgr;
	for (int n=0; n<2; ++n) {
		while (pDocLine) {
			bool bModifiedTemp = ModifyVisitor().IsLineModified(pDocLine, nSaveSeq);
			if (bModifiedTemp != bModified) {
				// ���o���ꂽ�ʒu��1�s���(MODIFYLINE_NEXT�Ɠ����ʒu)�Ɏ~�܂�
				ptXY.y = nYOld2;
				Point ptLayout = layoutMgr.LogicToLayout(ptXY);
				view.MoveCursorSelecting(ptLayout, bSelect);
				if (n == 1) {
					view.SendStatusMessage(LS(STR_ERR_SRPREV1));
				}
				return;
			}
			nYOld2 = ptXY.y;
			ptXY.y--;
			pDocLine = pDocLine->GetPrevLine();
		}
		if (n == 0) {
			// �擪�s�`�F�b�N
			const DocLine* pDocLineTemp = docLineMgr.GetDocLineTop();
			assert( pDocLineTemp );
			if (ModifyVisitor().IsLineModified(pDocLineTemp, nSaveSeq) != false) {
				if (GetCaret().GetCaretLogicPos() != Point(0,0)) {
					ptXY = Point(0,0);
					Point ptLayout = layoutMgr.LogicToLayout(ptXY);
					view.MoveCursorSelecting(ptLayout, bSelect);
					return;
				}
			}
			ptXY.y = docLineMgr.GetLineCount() - 1;
			nYOld2 = ptXY.y;
			pDocLine = docLineMgr.GetLine(ptXY.y);
			bModified = false;
		}
		if (!GetDllShareData().common.search.bSearchAll) {
			break;
		}
		if (n == 0) {
			const DocLine* pDocLineTemp = docLineMgr.GetDocLineBottom();
			assert( pDocLineTemp );
			if (ModifyVisitor().IsLineModified(pDocLineTemp, nSaveSeq) != false) {
				// �ŏI�s���ύX�s�̏ꍇ�́A[EOF]�Ɏ~�܂�
				Point pos;
				if (pDocLineTemp->GetEol() != EolType::None) {
					pos.x = 0;
					pos.y = docLineMgr.GetLineCount();
					pos.y++;
				}else {
					pos.x = pDocLine->GetLengthWithoutEOL();
					pos.y = docLineMgr.GetLineCount() - 1;
				}
				if (GetCaret().GetCaretLogicPos() != pos) {
					ptXY = pos;
					Point ptLayout = layoutMgr.LogicToLayout(ptXY);
					view.MoveCursorSelecting(ptLayout, bSelect);
					view.SendStatusMessage(LS(STR_ERR_SRPREV1));
					return;
				}
			}
		}
	}
	view.SendStatusMessage(LS(STR_ERR_SRPREV2));
	AlertNotFound( view.GetHwnd(), false, LS(STR_MODLINE_PREV_NOT_FOUND) );
}

