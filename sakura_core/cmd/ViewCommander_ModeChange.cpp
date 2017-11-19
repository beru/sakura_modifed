#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

// ViewCommander�N���X�̃R�}���h(���[�h�؂�ւ��n)�֐��Q

/*! �}���^�㏑�����[�h�؂�ւ� */
void ViewCommander::Command_ChgMod_Ins(void)
{
	// �}�����[�h���H
	view.SetInsMode(!view.IsInsMode());
	// �L�����b�g�̕\���E�X�V
	GetCaret().ShowEditCaret();
	// �L�����b�g�̍s���ʒu��\������
	GetCaret().ShowCaretPosInfo();
}


/*! ���͂�����s�R�[�h��ݒ� */
void ViewCommander::Command_ChgMod_EOL(EolType e)
{
	if (EolType::None < e && e < EolType::CodeMax) {
		GetDocument().docEditor.SetNewLineCode(e);
		// �X�e�[�^�X�o�[���X�V���邽��
		// �L�����b�g�̍s���ʒu��\������֐����Ăяo��
		GetCaret().ShowCaretPosInfo();
	}
}


// �����R�[�h�Z�b�g�w��
void ViewCommander::Command_Chg_Charset(
	EncodingType	eCharSet,	// [in] �ݒ肷�镶���R�[�h�Z�b�g
	bool		bBom		// [in] �ݒ肷��BOM(Unicode�n�ȊO�͖���)
	)
{
	if (eCharSet == CODE_NONE || eCharSet ==  CODE_AUTODETECT) {
		// �����R�[�h���w�肳��Ă��Ȃ��Ȃ��
		// �����R�[�h�̊m�F
		eCharSet = GetDocument().GetDocumentEncoding();	// �ݒ肷�镶���R�[�h�Z�b�g
		bBom     = GetDocument().GetDocumentBomExist();	// �ݒ肷��BOM
		INT_PTR nRet = GetEditWindow().dlgSetCharSet.DoModal(G_AppInstance(), view.GetHwnd(), 
						&eCharSet, &bBom);
		if (!nRet) {
			return;
		}
	}

	// �����R�[�h�̐ݒ�
	GetDocument().docFile.SetCodeSetChg(eCharSet, CodeTypeName(eCharSet).UseBom() & bBom);

	// �X�e�[�^�X�\��
	GetCaret().ShowCaretPosInfo();
}


/** �e�탂�[�h�̎�����
	@param whereCursorIs �I�����L�����Z��������A�L�����b�g���ǂ��ɒu�����B0=�������Ȃ��B1=����B2=�E���B
*/
void ViewCommander::Command_Cancel_Mode(int whereCursorIs)
{
	bool bBoxSelect = false;
	auto& selInfo = view.GetSelectionInfo();
	auto& caret = GetCaret();
	if (selInfo.IsTextSelected()) {
		// �I��������̃J�[�\���ʒu�����߂�B
		Point ptTo;
		Range rcMoveTo = GetSelect();
		if (selInfo.IsBoxSelecting()) { // ��`�I���ł̓L�����b�g�����s�̌��Ɏ��c����Ȃ��悤�ɁA����B
			bBoxSelect = true;
			// 2�_��Ίp�Ƃ����`�����߂�
			Range rcSel;
			TwoPointToRange(
				&rcSel,
				GetSelect().GetFrom(),	// �͈͑I���J�n
				GetSelect().GetTo()		// �͈͑I���I��
			);
			// ����Œ�͂�߂�
			rcMoveTo = rcSel;
		}
		if (whereCursorIs == 1) { // ����
			ptTo = rcMoveTo.GetFrom();
		}else if (whereCursorIs == 2) { // �E��
			ptTo = rcMoveTo.GetTo();
		}else {
			ptTo = caret.GetCaretLayoutPos();
		}

		// ���݂̑I��͈͂��I����Ԃɖ߂�
		selInfo.DisableSelectArea(true);

		// �J�[�\�����ړ�
		auto& layoutMgr = GetDocument().layoutMgr;
		if (ptTo.y >= (int)layoutMgr.GetLineCount()) {
			// �t�@�C���̍Ō�Ɉړ�
			Command_GoFileEnd(false);
		}else {
			if (!GetDllShareData().common.general.bIsFreeCursorMode && bBoxSelect) {
				// ��`�I���̂Ƃ�����Œ����߂��̂ő����EOL���E�������ꍇ��EOL�ɕ␳����
				const Layout* pLayout = layoutMgr.SearchLineByLayoutY(ptTo.y);
				if (pLayout) {
					ptTo.x = t_min((int)ptTo.x, (int)pLayout->CalcLayoutWidth(layoutMgr));
				}
			}

			caret.MoveCursor(ptTo, true);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
		}
	}else {
		// �I�𒆂̖��I����Ԃł�Lock�̉����ƕ`�悪�K�v
		if (selInfo.IsTextSelecting()
			|| selInfo.IsBoxSelecting()
		) {
			selInfo.DisableSelectArea(true);
			caret.underLine.CaretUnderLineON(true, false);
			selInfo.PrintSelectionInfoMsg();
		}
	}
}

