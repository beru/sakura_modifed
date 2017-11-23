#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

// ViewCommander�N���X�̃R�}���h(�ҏW�n �P��/�s�P��)�֐��Q

// �P��̍��[�܂ō폜
void ViewCommander::Command_WordDeleteToStart(void)
{
	auto& selInfo = view.GetSelectionInfo();
	// ��`�I����Ԃł͎��s�s�\(��������蔲������)
	if (selInfo.IsTextSelected()) {
		// ��`�͈͑I�𒆂�
		if (selInfo.IsBoxSelecting()) {
			ErrorBeep();
			return;
		}
	}

	// �P��̍��[�Ɉړ�
	ViewCommander::Command_WordLeft(true);
	if (!selInfo.IsTextSelected()) {
		ErrorBeep();
		return;
	}

	if (!view.bDoing_UndoRedo) {	// Undo, Redo�̎��s����
		MoveCaretOpe* pOpe = new MoveCaretOpe();
		pOpe->ptCaretPos_PHY_Before = GetDocument().layoutMgr.LayoutToLogic(GetSelect().GetTo());
		pOpe->ptCaretPos_PHY_After = pOpe->ptCaretPos_PHY_Before;	// �����̃L�����b�g�ʒu

		// ����̒ǉ�
		GetOpeBlk()->AppendOpe(pOpe);
	}

	// �폜
	view.DeleteData(true);
}



// �P��̉E�[�܂ō폜
void ViewCommander::Command_WordDeleteToEnd(void)
{
	auto& selInfo = view.GetSelectionInfo();

	// ��`�I����Ԃł͎��s�s�\((��������蔲������))
	if (selInfo.IsTextSelected()) {
		// ��`�͈͑I�𒆂�
		if (selInfo.IsBoxSelecting()) {
			ErrorBeep();
			return;
		}
	}
	// �P��̉E�[�Ɉړ�
	ViewCommander::Command_WordRight(true);
	if (!selInfo.IsTextSelected()) {
		ErrorBeep();
		return;
	}
	if (!view.bDoing_UndoRedo) {	// Undo, Redo�̎��s����
		MoveCaretOpe* pOpe = new MoveCaretOpe();
		pOpe->ptCaretPos_PHY_Before = GetDocument().layoutMgr.LayoutToLogic(GetSelect().GetFrom());
		pOpe->ptCaretPos_PHY_After = pOpe->ptCaretPos_PHY_Before;	// �����̃L�����b�g�ʒu
		// ����̒ǉ�
		GetOpeBlk()->AppendOpe(pOpe);
	}
	// �폜
	view.DeleteData(true);
}


// �P��؂���
void ViewCommander::Command_WordCut(void)
{
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsTextSelected()) {
		// �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
		Command_Cut();
		return;
	}
	// ���݈ʒu�̒P��I��
	Command_SelectWord();
	// �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
	if (!selInfo.IsTextSelected()) {
		// �P��I���őI���ł��Ȃ�������A���̕�����I�Ԃ��Ƃɒ���B
		Command_Right(true, false, false);
	}
	Command_Cut();
	return;
}


// �P��폜
void ViewCommander::Command_WordDelete(void)
{
	if (view.GetSelectionInfo().IsTextSelected()) {
		// �폜
		view.DeleteData(true);
		return;
	}
	// ���݈ʒu�̒P��I��
	Command_SelectWord();
	// �폜
	view.DeleteData(true);
	return;
}


// �s���܂Ő؂���(���s�P��)
void ViewCommander::Command_LineCutToStart(void)
{
	auto& selInfo = view.GetSelectionInfo();
	Layout* pLayout;
	if (selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
		Command_Cut();
		return;
	}
	auto& layoutMgr = GetDocument().layoutMgr;
	auto& caret = GetCaret();
	pLayout = layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().y);	// �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(Layout)�ւ̃|�C���^��Ԃ�
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	Point ptPos = layoutMgr.LogicToLayout(Point(0, pLayout->GetLogicLineNo()));
	if (caret.GetCaretLayoutPos() == ptPos) {
		ErrorBeep();
		return;
	}

	// �I��͈͂̕ύX
	Range range(ptPos, caret.GetCaretLayoutPos());
	selInfo.SetSelectArea(range);

	// �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
	Command_Cut();
}



// �s���܂Ő؂���(���s�P��)
void ViewCommander::Command_LineCutToEnd(void)
{
	auto& selInfo = view.GetSelectionInfo();
	Layout* pLayout;
	if (selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
		Command_Cut();
		return;
	}
	auto& layoutMgr = GetDocument().layoutMgr;
	auto& caret = GetCaret();
	// �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(Layout)�ւ̃|�C���^��Ԃ�
	pLayout = layoutMgr.SearchLineByLayoutY(
		caret.GetCaretLayoutPos().y
	);
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	Point ptPos;
	auto& docLineRef = *pLayout->GetDocLineRef();
	if (docLineRef.GetEol() == EolType::None) {	// ���s�R�[�h�̎��
		ptPos = layoutMgr.LogicToLayout(
			Point(
				(int)docLineRef.GetLengthWithEOL(),
				pLayout->GetLogicLineNo()
			)
		);
	}else {
		ptPos = layoutMgr.LogicToLayout(
			Point(
				(int)docLineRef.GetLengthWithEOL() - (int)docLineRef.GetEol().GetLen(),
				pLayout->GetLogicLineNo()
			)
		);
	}

	if (caret.GetCaretLayoutPos().y == ptPos.y && caret.GetCaretLayoutPos().x >= ptPos.x) {
		ErrorBeep();
		return;
	}

	// �I��͈͂̕ύX
	Range range(caret.GetCaretLayoutPos(), ptPos);
	selInfo.SetSelectArea(range);

	// �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
	Command_Cut();
}


// �s���܂ō폜(���s�P��)
void ViewCommander::Command_LineDeleteToStart(void)
{
	auto& selInfo = view.GetSelectionInfo();
	Layout* pLayout;
	if (selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		view.DeleteData(true);
		return;
	}
	auto& layoutMgr = GetDocument().layoutMgr;
	auto& caret = GetCaret();
	pLayout = layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().y);	// �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(Layout)�ւ̃|�C���^��Ԃ�
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	Point ptPos = layoutMgr.LogicToLayout(Point(0, pLayout->GetLogicLineNo()));
	if (caret.GetCaretLayoutPos() == ptPos) {
		ErrorBeep();
		return;
	}

	// �I��͈͂̕ύX
	Range range(ptPos, caret.GetCaretLayoutPos());
	selInfo.SetSelectArea(range);

	// �I��̈�폜
	view.DeleteData(true);
}


// �s���܂ō폜(���s�P��)
void ViewCommander::Command_LineDeleteToEnd(void)
{
	auto& selInfo = view.GetSelectionInfo();
	Layout* pLayout;
	if (selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		view.DeleteData(true);
		return;
	}

	auto& caretLayoutPos = GetCaret().GetCaretLayoutPos();
	pLayout = GetDocument().layoutMgr.SearchLineByLayoutY(caretLayoutPos.y);	// �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(Layout)�ւ̃|�C���^��Ԃ�
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	Point ptPos;

	auto& docLineRef = *pLayout->GetDocLineRef();
	if (docLineRef.GetEol() == EolType::None) {	// ���s�R�[�h�̎��
		ptPos = GetDocument().layoutMgr.LogicToLayout(
			Point(
				(int)docLineRef.GetLengthWithEOL(),
				pLayout->GetLogicLineNo()
			)
		);
	}else {
		ptPos = GetDocument().layoutMgr.LogicToLayout(
			Point(
				(int)docLineRef.GetLengthWithEOL() - (int)docLineRef.GetEol().GetLen(),
				pLayout->GetLogicLineNo()
			)
		);
	}

	if (caretLayoutPos.y == ptPos.y
		&& caretLayoutPos.x >= ptPos.x
	) {
		ErrorBeep();
		return;
	}

	// �I��͈͂̕ύX
	Range range(caretLayoutPos, ptPos);
	selInfo.SetSelectArea(range);

	// �I��̈�폜
	view.DeleteData(true);
}


// �s�؂���(�܂�Ԃ��P��)
void ViewCommander::Command_Cut_Line(void)
{
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// �}�E�X�ɂ��͈͑I��
		ErrorBeep();
		return;
	}

	if (selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		ErrorBeep();
		return;
	}

	const Layout* pLayout = GetDocument().layoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().y);
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	// �����ȑf��
	view.CopyCurLine(
		GetDllShareData().common.edit.bAddCRLFWhenCopy,
		EolType::Unknown,
		GetDllShareData().common.edit.bEnableLineModePaste
	);
	Command_Delete_Line();
	return;
}


// �s�폜(�܂�Ԃ��P��)
void ViewCommander::Command_Delete_Line(void)
{
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// �}�E�X�ɂ��͈͑I��
		ErrorBeep();
		return;
	}

	if (selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		ErrorBeep();
		return;
	}
	auto& caret = GetCaret();
	const Layout* pLayout = GetDocument().layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().y);
	if (!pLayout) {
		ErrorBeep();
		return;
	}
	GetSelect().SetFrom(Point(0, caret.GetCaretLayoutPos().y  ));	// �͈͑I���J�n�ʒu
	GetSelect().SetTo  (Point(0, caret.GetCaretLayoutPos().y + 1));	// �͈͑I���I���ʒu

	Point ptCaretPos_OLD = caret.GetCaretLayoutPos();

	Command_Delete();
	pLayout = GetDocument().layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().y);
	if (pLayout) {
		// �s�폜������A�t���[�J�[�\���łȂ��̂ɃJ�[�\���ʒu���s�[���E�ɂȂ�s��Ή�
		// �t���[�J�[�\�����[�h�łȂ��ꍇ�́A�J�[�\���ʒu�𒲐�����
		if (!GetDllShareData().common.general.bIsFreeCursorMode) {

			size_t tmp;
			size_t nIndex = view.LineColumnToIndex2(pLayout, ptCaretPos_OLD.x, &tmp);
			ptCaretPos_OLD.x = (LONG)tmp;

			if (ptCaretPos_OLD.x > 0) {
				ptCaretPos_OLD.x--;
			}else {
				ptCaretPos_OLD.x = (LONG)view.LineIndexToColumn(pLayout, nIndex);
			}
		}
		// ����O�̈ʒu�փJ�[�\�����ړ�
		caret.MoveCursor(ptCaretPos_OLD, true);
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
		if (!view.bDoing_UndoRedo) {	// Undo, Redo�̎��s����
			// ����̒ǉ�
			GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					caret.GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
				)
			);
		}
	}
	return;
}


// �s�̓�d��(�܂�Ԃ��P��)
void ViewCommander::Command_DuplicateLine(void)
{
	NativeW		memBuf;
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// ���݂̑I��͈͂��I����Ԃɖ߂�
		selInfo.DisableSelectArea(true);
	}

	auto& caret = GetCaret();
	auto& layoutMgr = GetDocument().layoutMgr;
	const Layout* pLayout = layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().y);
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	if (!view.bDoing_UndoRedo) {	// Undo, Redo�̎��s����
		// ����̒ǉ�
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				caret.GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
			)
		);
	}

	Point ptCaretPosOld = caret.GetCaretLayoutPos() + Point(0, 1);

	// �s���Ɉړ�(�܂�Ԃ��P��)
	Command_GoLineTop(selInfo.bSelectingLock, 0x1 /* �J�[�\���ʒu�Ɋ֌W�Ȃ��s���Ɉړ� */);

	if (!view.bDoing_UndoRedo) {	// Undo, Redo�̎��s����
		// ����̒ǉ�
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				caret.GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
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
	bool bCRLF = (pLayout->GetLayoutEol() != EolType::None);
	bool bAddCRLF = false;
	if (!bCRLF) {
		if (caret.GetCaretLayoutPos().y == layoutMgr.GetLineCount() - 1) {
			bAddCRLF = true;
		}
	}

	memBuf.SetString(pLayout->GetPtr(), pLayout->GetLengthWithoutEOL() + pLayout->GetLayoutEol().GetLen());	// ��pLayout->GetLengthWithEOL()�́AEOL�̒�����K��1�ɂ���̂Ŏg���Ȃ��B
	if (bAddCRLF) {
		// ���݁AEnter�Ȃǂő}��������s�R�[�h�̎�ނ��擾
		Eol cWork = GetDocument().docEditor.GetNewLineCode();
		memBuf.AppendString(cWork.GetValue2(), cWork.GetLen());
	}

	// ���݈ʒu�Ƀf�[�^��}��
	Point ptLayoutNew;
	view.InsertData_CEditView(
		caret.GetCaretLayoutPos(),
		memBuf.GetStringPtr(),
		memBuf.GetStringLength(),
		&ptLayoutNew,
		true
	);

	// �J�[�\�����ړ�
	caret.MoveCursor(ptCaretPosOld, true);
	caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

	if (!view.bDoing_UndoRedo) {	// Undo, Redo�̎��s����
		// ����̒ǉ�
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				caret.GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
			)
		);
	}
	return;
}

