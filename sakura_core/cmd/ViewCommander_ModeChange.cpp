/*!	@file
@brief ViewCommander�N���X�̃R�}���h(���[�h�؂�ւ��n)�֐��Q

	2012/12/15	ViewCommander.cpp,ViewCommander_New.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2003, Moca
	Copyright (C) 2005, genta
	Copyright (C) 2007, Moca

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"


/*! �}���^�㏑�����[�h�؂�ւ�

	@date 2005.10.02 genta InsMode�֐���
*/
void ViewCommander::Command_ChgMod_Ins(void)
{
	// �}�����[�h���H
	view.SetInsMode(!view.IsInsMode());
	// �L�����b�g�̕\���E�X�V
	GetCaret().ShowEditCaret();
	// �L�����b�g�̍s���ʒu��\������
	GetCaret().ShowCaretPosInfo();
}


// from ViewCommander_New.cpp
/*! ���͂�����s�R�[�h��ݒ�

	@author moca
	@date 2003.06.23 �V�K�쐬
*/
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
		int nRet = GetEditWindow().dlgSetCharSet.DoModal(G_AppInstance(), view.GetHwnd(), 
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
		LayoutPoint ptTo;
		LayoutRange rcMoveTo = GetSelect();
		if (selInfo.IsBoxSelecting()) { // ��`�I���ł̓L�����b�g�����s�̌��Ɏ��c����Ȃ��悤�ɁA����B
			bBoxSelect = true;
			// 2�_��Ίp�Ƃ����`�����߂�
			LayoutRange rcSel;
			TwoPointToRange(
				&rcSel,
				GetSelect().GetFrom(),	// �͈͑I���J�n
				GetSelect().GetTo()		// �͈͑I���I��
			);
			// 2013.04.22 Moca ����Œ�͂�߂�
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
		if (ptTo.y >= layoutMgr.GetLineCount()) {
			// �t�@�C���̍Ō�Ɉړ�
			Command_GoFileEnd(false);
		}else {
			if (!GetDllShareData().common.general.bIsFreeCursorMode && bBoxSelect) {
				// 2013.04.22 Moca ��`�I���̂Ƃ�����Œ����߂��̂ő����EOL���E�������ꍇ��EOL�ɕ␳����
				const Layout* pLayout = layoutMgr.SearchLineByLayoutY(ptTo.y);
				if (pLayout) {
					ptTo.x = t_min(ptTo.x, (int)pLayout->CalcLayoutWidth(layoutMgr));
				}
			}

			caret.MoveCursor(ptTo, true);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
		}
	}else {
		// 2011.12.05 Moca �I�𒆂̖��I����Ԃł�Lock�̉����ƕ`�悪�K�v
		if (selInfo.IsTextSelecting()
			|| selInfo.IsBoxSelecting()
		) {
			selInfo.DisableSelectArea(true);
			caret.underLine.CaretUnderLineON(true, false);
			selInfo.PrintSelectionInfoMsg();
		}
	}
}

