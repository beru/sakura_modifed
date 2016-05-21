/*!	@file
@brief ViewCommander�N���X�̃R�}���h(�N���b�v�{�[�h�n)�֐��Q

	2012/12/20	ViewCommander.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, novice
	Copyright (C) 2002, hor, genta, Azumaiya, ���Ȃӂ�
	Copyright (C) 2004, Moca
	Copyright (C) 2005, genta
	Copyright (C) 2007, ryoji
	Copyright (C) 2010, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"
#include "view/colors/ColorStrategy.h"
#include "view/colors/Color_Found.h"
#include "uiparts/WaitCursor.h"
#include "util/os.h"


/** �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)

	@date 2007.11.18 ryoji �u�I���Ȃ��ŃR�s�[���\�ɂ���v�I�v�V���������ǉ�
*/
void ViewCommander::Command_Cut(void)
{
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// �}�E�X�ɂ��͈͑I��
		ErrorBeep();
		return;
	}

	auto& csEdit = GetDllShareData().common.edit;

	// �͈͑I��������Ă��Ȃ�
	if (!selInfo.IsTextSelected()) {
		// ��I�����́A�J�[�\���s��؂���
		if (!csEdit.bEnableNoSelectCopy) {	// 2007.11.18 ryoji
			return;	// �������Ȃ��i�����炳�Ȃ��j
		}
		// �s�؂���(�܂�Ԃ��P��)
		Command_Cut_Line();
		return;
	}
	bool bBeginBoxSelect = selInfo.IsBoxSelecting();

	// �I��͈͂̃f�[�^���擾
	// ���펞�� true, �͈͖��I���̏ꍇ�� false ��Ԃ�
	NativeW memBuf;
	if (!view.GetSelectedData(&memBuf, false, NULL, false, csEdit.bAddCRLFWhenCopy)) {
		ErrorBeep();
		return;
	}
	// �N���b�v�{�[�h�Ƀf�[�^��ݒ�
	if (!view.MySetClipboardData(memBuf.GetStringPtr(), memBuf.GetStringLength(), bBeginBoxSelect)) {
		ErrorBeep();
		return;
	}
	memBuf.Clear();

	// �J�[�\���ʒu�܂��͑I���G���A���폜
	view.DeleteData(true);
	return;
}


/**	�I��͈͂��N���b�v�{�[�h�ɃR�s�[

	@date 2007.11.18 ryoji �u�I���Ȃ��ŃR�s�[���\�ɂ���v�I�v�V���������ǉ�
*/
void ViewCommander::Command_Copy(
	bool	bIgnoreLockAndDisable,	// [in] �I��͈͂��������邩�H
	bool	bAddCRLFWhenCopy,		// [in] �܂�Ԃ��ʒu�ɉ��s�R�[�h��}�����邩�H
	EolType	neweol					// [in] �R�s�[����Ƃ���EOL�B
	)
{
	NativeW memBuf;
	auto& selInfo = view.GetSelectionInfo();
	auto& csEdit = GetDllShareData().common.edit;
	// �N���b�v�{�[�h�ɓ����ׂ��e�L�X�g�f�[�^���AmemBuf�Ɋi�[����
	if (!selInfo.IsTextSelected()) {
		// ��I�����́A�J�[�\���s���R�s�[����
		if (!csEdit.bEnableNoSelectCopy) {	// 2007.11.18 ryoji
			return;	// �������Ȃ��i�����炳�Ȃ��j
		}
		view.CopyCurLine(
			bAddCRLFWhenCopy,
			neweol,
			csEdit.bEnableLineModePaste
		);
	}else {
		// �e�L�X�g���I������Ă���Ƃ��́A�I��͈͂̃f�[�^���擾
		bool bBeginBoxSelect = selInfo.IsBoxSelecting();
		// �I��͈͂̃f�[�^���擾
		// ���펞��true,�͈͖��I���̏ꍇ��false��Ԃ�
		if (!view.GetSelectedData(&memBuf, false, NULL, false, bAddCRLFWhenCopy, neweol)) {
			ErrorBeep();
			return;
		}

		// �N���b�v�{�[�h�Ƀf�[�^memBuf�̓��e��ݒ�
		if (!view.MySetClipboardData(
			memBuf.GetStringPtr(),
			memBuf.GetStringLength(),
			bBeginBoxSelect,
			false
			)
		) {
			ErrorBeep();
			return;
		}
	}
	memBuf.Clear();

	// �I��͈͂̌�Еt��
	if (!bIgnoreLockAndDisable) {
		// �I����Ԃ̃��b�N
		if (selInfo.bSelectingLock) {
			selInfo.bSelectingLock = false;
			selInfo.PrintSelectionInfoMsg();
			if (!selInfo.IsTextSelected()) {
				GetCaret().underLine.CaretUnderLineON(true, false);
			}
		}
	}
	if (csEdit.bCopyAndDisablSelection) {	// �R�s�[������I������
		// �e�L�X�g���I������Ă��邩
		if (selInfo.IsTextSelected()) {
			// ���݂̑I��͈͂��I����Ԃɖ߂�
			selInfo.DisableSelectArea(true);
		}
	}
	return;
}


/** �\��t��(�N���b�v�{�[�h����\��t��)
	@param [in] option �\��t�����̃I�v�V����
	@li 0x01 ���s�R�[�h�ϊ��L��
	@li 0x02 ���s�R�[�h�ϊ�����
	@li 0x04 ���C�����[�h�\��t���L��
	@li 0x08 ���C�����[�h�\��t������
	@li 0x10 ��`�R�s�[�͏�ɋ�`�\��t��
	@li 0x20 ��`�R�s�[�͏�ɒʏ�\��t��

	@date 2007.10.04 ryoji MSDEVLineSelect�`���̍s�R�s�[�Ή�������ǉ��iVS2003/2005�̃G�f�B�^�Ɨގ��̋����Ɂj
*/
void ViewCommander::Command_Paste(int option)
{
	auto& selInfo = view.GetSelectionInfo();

	if (selInfo.IsMouseSelecting()) {	// �}�E�X�ɂ��͈͑I��
		ErrorBeep();
		return;
	}

	auto& commonSetting = GetDllShareData().common;
	// �N���b�v�{�[�h����f�[�^���擾 -> memClip, bColumnSelect
	NativeW	memClip;
	bool	bColumnSelect;
	bool	bLineSelect = false;
	bool	bLineSelectOption = 
		((option & 0x04) == 0x04) ? true :
		((option & 0x08) == 0x08) ? false :
		commonSetting.edit.bEnableLineModePaste;
	
	if (!view.MyGetClipboardData(memClip, &bColumnSelect, bLineSelectOption ? &bLineSelect: nullptr)) {
		ErrorBeep();
		return;
	}

	// �N���b�v�{�[�h�f�[�^�擾 -> pszText, nTextLen
	LogicInt nTextLen;
	const wchar_t*	pszText = memClip.GetStringPtr(&nTextLen);

	bool bConvertEol = 
		((option & 0x01) == 0x01) ? true :
		((option & 0x02) == 0x02) ? false :
		commonSetting.edit.bConvertEOLPaste;

	bool bAutoColumnPaste = 
		((option & 0x10) == 0x10) ? true :
		((option & 0x20) == 0x20) ? false :
		commonSetting.edit.bAutoColumnPaste;
	
	// ��`�R�s�[�̃e�L�X�g�͏�ɋ�`�\��t��
	if (bAutoColumnPaste) {
		// ��`�R�s�[�̃f�[�^�Ȃ��`�\��t��
		if (bColumnSelect) {
			if (selInfo.IsMouseSelecting()) {
				ErrorBeep();
				return;
			}
			if (!commonSetting.view.bFontIs_FixedPitch) {
				return;
			}
			Command_PasteBox(pszText, nTextLen);
			view.AdjustScrollBars();
			view.Redraw();
			return;
		}
	}

	// 2007.10.04 ryoji
	// �s�R�s�[�iMSDEVLineSelect�`���j�̃e�L�X�g�Ŗ��������s�ɂȂ��Ă��Ȃ���Ή��s��ǉ�����
	// �����C�A�E�g�܂�Ԃ��̍s�R�s�[�������ꍇ�͖��������s�ɂȂ��Ă��Ȃ�
	if (bLineSelect) {
		if (!WCODE::IsLineDelimiter(pszText[nTextLen-1], GetDllShareData().common.edit.bEnableExtEol)) {
			memClip.AppendString(GetDocument().docEditor.GetNewLineCode().GetValue2());
			pszText = memClip.GetStringPtr(&nTextLen);
		}
	}

	if (bConvertEol) {
		LogicInt nConvertedTextLen = ConvertEol(pszText, nTextLen, NULL);
		std::vector<wchar_t> szConvertedText(nConvertedTextLen);
		wchar_t* pszConvertedText = &szConvertedText[0];
		ConvertEol(pszText, nTextLen, pszConvertedText);
		// �e�L�X�g��\��t��
		Command_InsText(true, pszConvertedText, nConvertedTextLen, true, bLineSelect);	// 2010.09.17 ryoji
	}else {
		// �e�L�X�g��\��t��
		Command_InsText(true, pszText, nTextLen, true, bLineSelect);	// 2010.09.17 ryoji
	}

	return;
}



//<< 2002/03/28 Azumaiya
// �������f�[�^����`�\��t���p�̃f�[�^�Ɖ��߂��ď�������B
//  �Ȃ��A���̊֐��� Command_PasteBox(void) �ƁA
// 2769 : GetDocument().docEditor.SetModified(true, true);	// Jan. 22, 2002 genta
// ����A
// 3057 : view.SetDrawSwitch(true);	// 2002.01.25 hor
// �Ԃ܂ŁA�ꏏ�ł��B
//  �ł����A�R�����g���������A#if 0 �̂Ƃ����������肵�Ă��܂��̂ŁACommand_PasteBox(void) ��
// �c���悤�ɂ��܂���(���ɂ��̊֐����g�����g�����o�[�W�������R�����g�ŏ����Ă����܂���)�B
//  �Ȃ��A�ȉ��ɂ�����悤�� Command_PasteBox(void) �ƈႤ�Ƃ��낪����̂Œ��ӂ��Ă��������B
// > �Ăяo�������ӔC�������āA
// �E�}�E�X�ɂ��͈͑I�𒆂ł���B
// �E���݂̃t�H���g�͌Œ蕝�t�H���g�ł���B
// �� 2 �_���`�F�b�N����B
// > �ĕ`����s��Ȃ�
// �ł��B
//  �Ȃ��A�������Ăяo�����Ɋ��҂���킯�́A�u���ׂĒu���v�̂悤�ȉ�����A���ŌĂяo��
// �Ƃ��ɁA�ŏ��Ɉ��`�F�b�N����΂悢���̂�������`�F�b�N����͖̂��ʂƔ��f�������߂ł��B
// @note 2004.06.30 ���݁A���ׂĒu���ł͎g�p���Ă��Ȃ�
void ViewCommander::Command_PasteBox(
	const wchar_t* szPaste,
	int nPasteSize
	)
{
	/* �����̓���͎c���Ă��������̂����A�Ăяo�����ŐӔC�������Ă���Ă��炤���ƂɕύX�B
	if (view.GetSelectionInfo().IsMouseSelecting())	// �}�E�X�ɂ��͈͑I��
	{
		ErrorBeep();
		return;
	}
	if (!GetDllShareData().common.bFontIs_FixedPitch)	// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
	{
		return;
	}
	*/

	GetDocument().docEditor.SetModified(true, true);	// Jan. 22, 2002 genta

	bool bDrawSwitchOld = view.SetDrawSwitch(false);	// 2002.01.25 hor

	// �Ƃ肠�����I��͈͂��폜
	// 2004.06.30 Moca view.GetSelectionInfo().IsTextSelected()���Ȃ��Ɩ��I�����A�ꕶ�������Ă��܂�
	if (view.GetSelectionInfo().IsTextSelected()) {
		view.DeleteData(false/*true 2002.01.25 hor*/);
	}

	WaitCursor waitCursor(view.GetHwnd(), 10000 < nPasteSize);
	HWND hwndProgress = NULL;
	int nProgressPos = 0;
	if (waitCursor.IsEnable()) {
		hwndProgress = view.StartProgress();
	}

	auto& caret = GetCaret();
	LayoutPoint ptCurOld = caret.GetCaretLayoutPos();

	LayoutInt nCount = LayoutInt(0);
	bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;

	// Jul. 10, 2005 genta �\��t���f�[�^�̍Ō��CR/LF�������ꍇ�̑΍�
	// �f�[�^�̍Ō�܂ŏ��� i.e. nBgn��nPasteSize�𒴂�����I��
	//for (nPos = 0; nPos < nPasteSize;)
	int nPos;
	LayoutPoint	ptLayoutNew;	// �}�����ꂽ�����̎��̈ʒu
	for (int nBgn=nPos=0; nBgn<nPasteSize;) {
		// Jul. 10, 2005 genta �\��t���f�[�^�̍Ō��CR/LF��������
		// �ŏI�s��Paste�����������Ȃ��̂ŁC
		// �f�[�^�̖����ɗ����ꍇ�͋����I�ɏ�������悤�ɂ���
		if (WCODE::IsLineDelimiter(szPaste[nPos], bExtEol) || nPos == nPasteSize) {
			// ���݈ʒu�Ƀf�[�^��}��
			if (nPos - nBgn > 0) {
				view.InsertData_CEditView(
					ptCurOld + LayoutPoint(LayoutInt(0), nCount),
					&szPaste[nBgn],
					nPos - nBgn,
					&ptLayoutNew,
					false
				);
			}

			// ���̍s�̑}���ʒu�փJ�[�\�����ړ�
			caret.MoveCursor(ptCurOld + LayoutPoint(LayoutInt(0), nCount), false);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
			// �J�[�\���s���Ō�̍s���s���ɉ��s�������A�}�����ׂ��f�[�^���܂�����ꍇ
			bool bAddLastCR = false;
			const Layout*	pLayout;
			LogicInt		nLineLen = LogicInt(0);
			const wchar_t* pLine = GetDocument().layoutMgr.GetLineStr(caret.GetCaretLayoutPos().GetY2(), &nLineLen, &pLayout);

			if (pLine && 1 <= nLineLen) {
				if (WCODE::IsLineDelimiter(pLine[nLineLen - 1], bExtEol)) {
				}else {
					bAddLastCR = true;
				}
			}else { // 2001/10/02 novice
				bAddLastCR = true;
			}

			if (bAddLastCR) {
//				MYTRACE(_T(" �J�[�\���s���Ō�̍s���s���ɉ��s�������A\n�}�����ׂ��f�[�^���܂�����ꍇ�͍s���ɉ��s��}���B\n"));
				LayoutInt nInsPosX = view.LineIndexToColumn(pLayout, nLineLen);

				view.InsertData_CEditView(
					LayoutPoint(nInsPosX, caret.GetCaretLayoutPos().GetY2()),
					GetDocument().docEditor.GetNewLineCode().GetValue2(),
					GetDocument().docEditor.GetNewLineCode().GetLen(),
					&ptLayoutNew,
					false
				);
			}

			if (
				(nPos + 1 < nPasteSize) &&
				(szPaste[nPos] == L'\r' && szPaste[nPos + 1] == L'\n')
			) {
				nBgn = nPos + 2;
			}else {
				nBgn = nPos + 1;
			}

			nPos = nBgn;
			++nCount;
		}else {
			++nPos;
		}
		if ((nPos % 100) == 0 && hwndProgress) {
			int newPos = ::MulDiv(nPos, 100, nPasteSize);
			if (newPos != nProgressPos) {
				nProgressPos = newPos;
				Progress_SetPos(hwndProgress, newPos + 1);
				Progress_SetPos(hwndProgress, newPos);
			}
		}
	}

	if (hwndProgress) {
		::ShowWindow(hwndProgress, SW_HIDE);
	}

	// �}���f�[�^�̐擪�ʒu�փJ�[�\�����ړ�
	caret.MoveCursor(ptCurOld, true);
	caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();

	if (!view.bDoing_UndoRedo) {	// Undo, Redo�̎��s����
		// ����̒ǉ�
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				caret.GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
			)
		);
	}

	view.SetDrawSwitch(bDrawSwitchOld);	// 2002.01.25 hor
	return;
}


/** ��`�\��t��(�N���b�v�{�[�h�����`�\��t��)
	@param [in] option ���g�p

	@date 2004.06.29 Moca ���g�p���������̂�L���ɂ���
	�I���W�i����Command_PasteBox(void)�͂΂�����폜 (genta)
*/
void ViewCommander::Command_PasteBox(int option)
{
	if (view.GetSelectionInfo().IsMouseSelecting()) {	// �}�E�X�ɂ��͈͑I��
		ErrorBeep();
		return;
	}

	if (!GetDllShareData().common.view.bFontIs_FixedPitch) {	// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
		return;
	}

	// �N���b�v�{�[�h����f�[�^���擾
	NativeW memClip;
	if (!view.MyGetClipboardData(memClip, NULL)) {
		ErrorBeep();
		return;
	}
	// 2004.07.13 Moca \0�R�s�[�΍�
	int nstrlen;
	const wchar_t* lptstr = memClip.GetStringPtr(&nstrlen);

	Command_PasteBox(lptstr, nstrlen);
	view.AdjustScrollBars(); // 2007.07.22 ryoji
	view.Redraw();			// 2002.01.25 hor
}


// ��`������}��
void ViewCommander::Command_InsBoxText(
	const wchar_t* pszPaste,
	int nPasteSize
	)
{
	if (view.GetSelectionInfo().IsMouseSelecting()) {	// �}�E�X�ɂ��͈͑I��
		ErrorBeep();
		return;
	}

	if (!GetDllShareData().common.view.bFontIs_FixedPitch) {	// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
		return;
	}

	Command_PasteBox(pszPaste, nPasteSize);
	view.AdjustScrollBars(); // 2007.07.22 ryoji
	view.Redraw();			// 2002.01.25 hor
}


/*! �e�L�X�g��\��t��
	@date 2004.05.14 Moca '\\0'���󂯓����悤�ɁA�����ɒ�����ǉ�
	@date 2010.09.17 ryoji ���C�����[�h�\��t���I�v�V������ǉ����ĈȑO�� Command_Paste() �Ƃ̏d�����𐮗��E����
	@date 2013.05.10 Moca �������[�h
*/
void ViewCommander::Command_InsText(
	bool				bRedraw,		// 
	const wchar_t*		pszText,		// [in] �\��t���镶����B
	LogicInt			nTextLen,		// [in] pszText�̒����B-1���w�肷��ƁApszText��NUL�I�[������Ƃ݂Ȃ��Ē����������v�Z����
	bool				bNoWaitCursor,	// 
	bool				bLinePaste,		// [in] ���C�����[�h�\��t��
	bool				bFastMode,		// [in] �������[�h(���C�A�E�g���W�͖�������)
	const LogicRange*	pSelectLogic	// [in] �I�v�V�����B�������[�h�̂Ƃ��̍폜�͈̓��W�b�N�P��
	)
{
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// �}�E�X�ɂ��͈͑I��
		ErrorBeep();
		return;
	}

	if (nTextLen < 0) {
		nTextLen = LogicInt(wcslen(pszText));
	}

	WaitCursor waitCursor(
		view.GetHwnd(),
		10000 < nTextLen && !selInfo.IsBoxSelecting()
	);

	GetDocument().docEditor.SetModified(true, bRedraw);	// Jan. 22, 2002 genta

	auto& caret = GetCaret();

	// �e�L�X�g���I������Ă��邩
	if (selInfo.IsTextSelected() || bFastMode) {
		// ��`�͈͑I�𒆂�
		if (selInfo.IsBoxSelecting()) {
			// ���s�܂ł𔲂��o��
			LogicInt i;
			bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
			for (i=LogicInt(0); i<nTextLen; ++i) {
				if (WCODE::IsLineDelimiter(pszText[i], bExtEol)) {
					break;
				}
			}
			Command_Indent(pszText, i);
			goto end_of_func;
		}else {
			// Jun. 23, 2000 genta
			// ����s�̍s���ȍ~�݂̂��I������Ă���ꍇ�ɂ͑I�𖳂��ƌ��Ȃ�
			bool bAfterEOLSelect = false;
			if (!bFastMode) {
				LogicInt len;
				const Layout* pLayout;
				const wchar_t* line = GetDocument().layoutMgr.GetLineStr(GetSelect().GetFrom().GetY2(), &len, &pLayout);
				int pos = (!line) ? 0 : view.LineColumnToIndex(pLayout, GetSelect().GetFrom().GetX2());

				// �J�n�ʒu���s�������ŁA�I���ʒu������s
				if (pos >= len && GetSelect().IsLineOne()) {
					caret.SetCaretLayoutPos(LayoutPoint(GetSelect().GetFrom().x, caret.GetCaretLayoutPos().y)); // �L�����b�gX�ύX
					selInfo.DisableSelectArea(false);
					bAfterEOLSelect = true;
				}
			}
			if (!bAfterEOLSelect) {
				// �f�[�^�u�� �폜&�}���ɂ��g����
				// �s�R�s�[�̓\��t���ł͑I��͈͍͂폜�i��ōs���ɓ\��t����j	// 2007.10.04 ryoji
				view.ReplaceData_CEditView(
					GetSelect(),				// �I��͈�
					bLinePaste? L"": pszText,	// �}������f�[�^
					bLinePaste? LogicInt(0): nTextLen,	// �}������f�[�^�̒���
					bRedraw,
					view.bDoing_UndoRedo ? NULL : GetOpeBlk(),
					bFastMode,
					pSelectLogic
				);
				if (!bLinePaste)	// 2007.10.04 ryoji
					goto end_of_func;
			}
		}
	}

	{	// ��I�����̏��� or ���C�����[�h�\��t�����̎c��̏���
		LogicInt nPosX_PHY_Delta(0);
		if (bLinePaste) {	// 2007.10.04 ryoji
			// �}���|�C���g�i�܂�Ԃ��P�ʍs���j�ɃJ�[�\�����ړ�
			LogicPoint ptCaretBefore = caret.GetCaretLogicPos();	// ����O�̃L�����b�g�ʒu
			Command_GoLineTop(false, 1);								// �s���Ɉړ�(�܂�Ԃ��P��)
			LogicPoint ptCaretAfter = caret.GetCaretLogicPos();	// �����̃L�����b�g�ʒu

			// �}���|�C���g�ƌ��̈ʒu�Ƃ̍���������
			nPosX_PHY_Delta = ptCaretBefore.x - ptCaretAfter.x;

			// UNDO�p�L�^
			if (!view.bDoing_UndoRedo) {
				GetOpeBlk()->AppendOpe(
					new MoveCaretOpe(
						ptCaretBefore,	// ����O�̃L�����b�g�ʒu
						ptCaretAfter	// �����̃L�����b�g�ʒu
					)
				);
			}
		}

		// ���݈ʒu�Ƀf�[�^��}��
		LayoutPoint ptLayoutNew; // �}�����ꂽ�����̎��̈ʒu
		view.InsertData_CEditView(
			caret.GetCaretLayoutPos(),
			pszText,
			nTextLen,
			&ptLayoutNew,
			bRedraw
		);

		// �}���f�[�^�̍Ō�փJ�[�\�����ړ�
		caret.MoveCursor(ptLayoutNew, bRedraw);
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();

		if (bLinePaste) {	// 2007.10.04 ryoji
			// ���̈ʒu�փJ�[�\�����ړ�
			LogicPoint ptCaretBefore = caret.GetCaretLogicPos();	// ����O�̃L�����b�g�ʒu
			LayoutPoint ptLayout;
			GetDocument().layoutMgr.LogicToLayout(
				ptCaretBefore + LogicPoint(nPosX_PHY_Delta, LogicInt(0)),
				&ptLayout
			);
			caret.MoveCursor(ptLayout, bRedraw);					// �J�[�\���ړ�
			LogicPoint ptCaretAfter = caret.GetCaretLogicPos();	// �����̃L�����b�g�ʒu
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

			// UNDO�p�L�^
			if (!view.bDoing_UndoRedo) {
				GetOpeBlk()->AppendOpe(
					new MoveCaretOpe(
						ptCaretBefore,	// ����O�̃L�����b�g�ʒu�w
						ptCaretAfter	// �����̃L�����b�g�ʒu�w
					)
				);
			}
		}
	}

end_of_func:

	return;
}


// �Ō�Ƀe�L�X�g��ǉ�
void ViewCommander::Command_AddTail(
	const wchar_t*	pszData,	// �ǉ�����e�L�X�g
	int				nDataLen	// �ǉ�����e�L�X�g�̒����B�����P�ʁB-1���w�肷��ƁA�e�L�X�g�I�[�܂ŁB
	)
{
	// �e�L�X�g�������v�Z
	if (nDataLen == -1 && pszData) {
		nDataLen = wcslen(pszData);
	}

	GetDocument().docEditor.SetModified(true, true);	// Jan. 22, 2002 genta

	// �t�@�C���̍Ō�Ɉړ�
	Command_GoFileEnd(false);

	auto& caret = GetCaret();
	// ���݈ʒu�Ƀf�[�^��}��
	LayoutPoint ptLayoutNew;	// �}�����ꂽ�����̎��̈ʒu
	view.InsertData_CEditView(
		caret.GetCaretLayoutPos(),
		pszData,
		nDataLen,
		&ptLayoutNew,
		true
	);

	// �}���f�[�^�̍Ō�փJ�[�\�����ړ�
	// Sep. 2, 2002 ���Ȃӂ� �A���_�[���C���̕\�����c���Ă��܂������C��
	caret.MoveCursor(ptLayoutNew, true);
	caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
}


// �I��͈͓��S�s�R�s�[
void ViewCommander::Command_CopyLines(void)
{
	// �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[����
	view.CopySelectedAllLines(
		NULL,	// ���p��
		false	// �s�ԍ���t�^����
	);
	return;
}


// �I��͈͓��S�s���p���t���R�s�[
void ViewCommander::Command_CopyLinesAsPassage(void)
{
	// �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[����
	view.CopySelectedAllLines(
		GetDllShareData().common.format.szInyouKigou,	// ���p��
		false 									// �s�ԍ���t�^����
	);
	return;
}


// �I��͈͓��S�s�s�ԍ��t���R�s�[
void ViewCommander::Command_CopyLinesWithLineNumber(void)
{
	// �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[����
	view.CopySelectedAllLines(
		NULL,	// ���p��
		true	// �s�ԍ���t�^����
	);
	return;
}


static bool AppendHTMLColor(
	const ColorAttr& colorAttrLast, ColorAttr& colorAttrLast2,
	const FontAttr& fontAttrLast, FontAttr& fontAttrLast2,
	const WCHAR* pAppendStr, int nLen,
	NativeW& memClip
	)
{
	if (fontAttrLast.bBoldFont != fontAttrLast2.bBoldFont
		|| fontAttrLast.bUnderLine != fontAttrLast2.bUnderLine
		|| colorAttrLast.cTEXT != colorAttrLast2.cTEXT
		|| colorAttrLast.cBACK != colorAttrLast2.cBACK
	) {
		if (fontAttrLast2.bBoldFont) {
			memClip.AppendStringLiteral(L"</b>");
		}
		if (fontAttrLast2.bUnderLine) {
			if (colorAttrLast.cTEXT != colorAttrLast2.cTEXT
				|| colorAttrLast.cBACK != colorAttrLast2.cBACK
				|| fontAttrLast.bUnderLine != fontAttrLast2.bUnderLine
			) {
				memClip.AppendStringLiteral(L"</u>");
			}
		}
		if (colorAttrLast.cTEXT != colorAttrLast2.cTEXT
			|| colorAttrLast.cBACK != colorAttrLast2.cBACK
		) {
			if (colorAttrLast2.cTEXT != (COLORREF)-1) {
				memClip.AppendStringLiteral(L"</span>");
			}
			if (colorAttrLast.cTEXT != (COLORREF)-1) {
				if (colorAttrLast.cTEXT != colorAttrLast2.cTEXT
					|| colorAttrLast.cBACK != colorAttrLast2.cBACK
				) {
					WCHAR szColor[60];
					DWORD dwTEXTColor = (GetRValue(colorAttrLast.cTEXT) << 16) + (GetGValue(colorAttrLast.cTEXT) << 8) + GetBValue(colorAttrLast.cTEXT);
					DWORD dwBACKColor = (GetRValue(colorAttrLast.cBACK) << 16) + (GetGValue(colorAttrLast.cBACK) << 8) + GetBValue(colorAttrLast.cBACK);
					swprintf(szColor, L"<span style=\"color:#%06x;background-color:#%06x\">", dwTEXTColor, dwBACKColor);
					memClip.AppendString(szColor);
				}
			}
		}
		if (fontAttrLast.bUnderLine) {
			if (colorAttrLast.cTEXT != colorAttrLast2.cTEXT
				|| colorAttrLast.cBACK != colorAttrLast2.cBACK
				|| fontAttrLast.bUnderLine != fontAttrLast2.bUnderLine
			) {
				memClip.AppendStringLiteral(L"<u>");
			}
		}
		if (fontAttrLast.bBoldFont) {
			memClip.AppendStringLiteral(L"<b>");
		}
		colorAttrLast2 = colorAttrLast;
		fontAttrLast2  = fontAttrLast;
	}
	NativeW memBuf(pAppendStr, nLen);
	memBuf.Replace(L"&", L"&amp;");
	memBuf.Replace(L"<", L"&lt;");
	memBuf.Replace(L">", L"&gt;");
	memClip.AppendNativeData(memBuf);
	if (0 < nLen) {
		return WCODE::IsLineDelimiter(
			pAppendStr[nLen-1],
			GetDllShareData().common.edit.bEnableExtEol
		);
	}
	return false;
}


// �I��͈͓��F�t��HTML�R�s�[
void ViewCommander::Command_Copy_Color_HTML(bool bLineNumber)
{
	auto& selInfo = view.GetSelectionInfo();
	if (!selInfo.IsTextSelected()
	  || GetSelect().GetFrom() == GetSelect().GetTo()
	) {
		return;
	}
	const TypeConfig& type = GetDocument().docType.GetDocumentAttribute();
	bool bLineNumLayout = GetDllShareData().common.edit.bAddCRLFWhenCopy
		|| selInfo.IsBoxSelecting();
	LayoutRect rcSel;
	TwoPointToRect(
		&rcSel,
		GetSelect().GetFrom(),	// �͈͑I���J�n
		GetSelect().GetTo()		// �͈͑I���I��
	);
	// �C�������������o�b�t�@�̒��������������Ōv�Z
	LogicRange sSelectLogic;
	sSelectLogic.Clear(-1);
	int nBuffSize = 0;
	const Layout* pLayoutTop = nullptr;
	{
		const Layout* pLayout;
		{
			LogicInt nLineLenTmp;
			GetDocument().layoutMgr.GetLineStr(rcSel.top, &nLineLenTmp, &pLayout);
		}
		pLayoutTop = pLayout;
		LayoutInt i = rcSel.top;
		for (; pLayout && i <= rcSel.bottom; ++i, pLayout = pLayout->GetNextLayout()) {
			// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
			LogicInt nIdxFrom;
			LogicInt nIdxTo;
			if (selInfo.IsBoxSelecting()) {
				nIdxFrom = view.LineColumnToIndex(pLayout, rcSel.left);
				nIdxTo   = view.LineColumnToIndex(pLayout, rcSel.right);
				// ���s�͏���
				if (nIdxTo - nIdxFrom > 0) {
					const WCHAR* pLine = pLayout->GetPtr();
					if (pLine[nIdxTo - 1] == L'\n' || pLine[nIdxTo - 1] == L'\r') {
						--nIdxTo;
					}
				}
				if (i == rcSel.top) {
					sSelectLogic.SetFromY(pLayout->GetLogicLineNo());
					sSelectLogic.SetFromX(nIdxFrom);
				}
				if (i == rcSel.bottom) {
					sSelectLogic.SetToY(pLayout->GetLogicLineNo());
					sSelectLogic.SetToX(nIdxTo);
				}
			}else {
				if (i == rcSel.top) {
					nIdxFrom = view.LineColumnToIndex(pLayout, rcSel.left);
					sSelectLogic.SetFromY(pLayout->GetLogicLineNo());
					sSelectLogic.SetFromX(nIdxFrom);
				}else {
					nIdxFrom = LogicInt(0);
				}
				if (i == rcSel.bottom) {
					nIdxTo = view.LineColumnToIndex(pLayout, rcSel.right);
					sSelectLogic.SetToY(pLayout->GetLogicLineNo());
					sSelectLogic.SetToX(nIdxTo);
				}else {
					nIdxTo = pLayout->GetLengthWithoutEOL();
				}
			}
			nBuffSize += nIdxTo - nIdxFrom;
			if (bLineNumLayout) {
				nBuffSize += 2;
			}else {
				nBuffSize += pLayout->GetLayoutEol().GetLen();
			}
		}
		if (sSelectLogic.GetTo().x == -1) {
			sSelectLogic.SetToY(GetDocument().docLineMgr.GetLineCount());
			sSelectLogic.SetToX(LogicInt(0));
		}
	}
	// �s�ԍ��̕����v�Z
	int nLineNumberMaxLen = 0;
	WCHAR szLineFormat[10];
	szLineFormat[0] = L'\0';
	NativeW memNullLine;
	if (bLineNumber) {
		int nLineNumberMax;
		if (type.bLineNumIsCRLF) {
			nLineNumberMax = sSelectLogic.GetTo().GetY();
		}else {
			nLineNumberMax = (Int)rcSel.bottom;
		}
		int nWork = 10;
		int i;
		memNullLine.AppendStringLiteral(L" ");
		for (i=1; i<12; ++i) {
			if (nWork > nLineNumberMax) {
				break;
			}
			nWork *= 10;
			memNullLine.AppendStringLiteral(L" ");
		}
		nLineNumberMaxLen = i + 1; // "%d:"
		memNullLine.AppendStringLiteral(L":");
		swprintf(szLineFormat, L"%%%dd:", i);
	}
	if (bLineNumLayout) {
		nBuffSize += (Int)(nLineNumberMaxLen * (rcSel.bottom - rcSel.top + 1));
	}else {
		nBuffSize += (Int)(nLineNumberMaxLen * (sSelectLogic.GetTo().y - sSelectLogic.GetFrom().y + 1));
	}
	NativeW memClip;
	memClip.AllocStringBuffer(nBuffSize + 11);
	{
		COLORREF cBACK = type.colorInfoArr[COLORIDX_TEXT].colorAttr.cBACK;
		DWORD dwBACKColor = (GetRValue(cBACK) << 16) + (GetGValue(cBACK) << 8) + GetBValue(cBACK);
		WCHAR szBuf[50];
		swprintf(szBuf, L"<pre style=\"background-color:#%06x\">", dwBACKColor);
		memClip.AppendString(szBuf);
	}
	LayoutInt nLayoutLineNum = rcSel.top;
	const LogicInt nLineNumLast = sSelectLogic.GetTo().y;
	const DocLine* pDocLine = pLayoutTop->GetDocLineRef();
	const Layout* pLayout = pLayoutTop;
	while (pLayout && pLayout->GetLogicOffset()) {
		pLayout = pLayout->GetPrevLayout();
	}
	ColorAttr colorAttr = { (COLORREF)-1, (COLORREF)-1 };
	ColorAttr colorAttrNext = { (COLORREF)-1, (COLORREF)-1 };
	ColorAttr colorAttrLast = { (COLORREF)-1, (COLORREF)-1 };
	ColorAttr colorAttrLast2 = { (COLORREF)-1, (COLORREF)-1 };
	FontAttr fontAttr = { false, false };
	FontAttr fontAttrNext = { false, false };
	FontAttr fontAttrLast = { false, false };
	FontAttr fontAttrLast2 = { false, false };
	auto& pool = ColorStrategyPool::getInstance();
	pool.SetCurrentView(&view);
	for (auto nLineNum = sSelectLogic.GetFrom().y;
		nLineNum <= nLineNumLast;
		++nLineNum, pDocLine = pDocLine->GetNextLine()
	) {
		if (!pDocLine) {
			break;
		}
		pool.NotifyOnStartScanLogic();
		ColorStrategy* pStrategyNormal = nullptr;
		ColorStrategy* pStrategyFound = nullptr;
		ColorStrategy* pStrategy = nullptr;
		StringRef cStringLine(pDocLine->GetPtr(), pDocLine->GetLengthWithEOL());
		{
			pStrategy = pStrategyNormal = pool.GetStrategyByColor(pLayout->GetColorTypePrev());
			if (pStrategy) {
				pStrategy->InitStrategyStatus();
				pStrategy->SetStrategyColorInfo(pLayout->GetColorInfo());
			}
			int nColorIdx = ToColorInfoArrIndex(pLayout->GetColorTypePrev());
			if (nColorIdx != -1) {
				const ColorInfo& info = type.colorInfoArr[nColorIdx];
				fontAttr = info.fontAttr;
				colorAttr = info.colorAttr;
			}
		}
		const WCHAR* pLine = pDocLine->GetPtr();
		for (;
			pLayout->GetLogicLineNo() == nLineNum;
			++nLayoutLineNum, pLayout = pLayout->GetNextLayout()
		) {
			LogicInt nIdxFrom;
			LogicInt nIdxTo;
			const int nLineLen = pLayout->GetLengthWithoutEOL() + pLayout->GetLayoutEol().GetLen();
			if (nLayoutLineNum < rcSel.top) {
				nIdxTo = nIdxFrom = LogicInt(-1);
			}else {
				if (selInfo.IsBoxSelecting()) {
					nIdxFrom = view.LineColumnToIndex(pLayout, rcSel.left);
					nIdxTo   = view.LineColumnToIndex(pLayout, rcSel.right);
					// ���s�͏���
					if (nIdxTo - nIdxFrom > 0) {
						const WCHAR* pLine = pLayout->GetPtr();
						if (pLine[nIdxTo - 1] == L'\n' || pLine[nIdxTo - 1] == L'\r') {
							--nIdxTo;
						}
					}
				}else {
					if (nLayoutLineNum == rcSel.top) {
						nIdxFrom = sSelectLogic.GetFrom().x;
					}else {
						nIdxFrom = LogicInt(0);
					}
					if (nLayoutLineNum == rcSel.bottom) {
						nIdxTo = sSelectLogic.GetTo().x;
					}else {
						nIdxTo = nLineLen;
					}
				}
			}
			// �Ō�̉��s�̎��̍s�ԍ���\�����Ȃ��悤��
			if (nIdxTo == 0 && nLayoutLineNum == rcSel.bottom) {
				break;
			}
			if (bLineNumber) {
				WCHAR szLineNum[14];
				if (type.bLineNumIsCRLF) {
					if (pLayout->GetLogicOffset() != 0) {
						if (bLineNumLayout) {
							memClip.AppendNativeData(memNullLine);
						}
					}else {
						int ret = swprintf(szLineNum, szLineFormat, nLineNum + 1);
						memClip.AppendString(szLineNum, ret);
					}
				}else {
					if (bLineNumLayout || pLayout->GetLogicOffset() == 0) {
						int ret = swprintf(szLineNum, szLineFormat, nLayoutLineNum + 1);
						memClip.AppendString(szLineNum, ret);
					}
				}
			}
			const int nLineStart = pLayout->GetLogicOffset();
			int nBgnLogic = nIdxFrom + nLineStart;
			int iLogic = nLineStart;
			bool bAddCRLF = false;
			for (; iLogic<nLineStart+nLineLen; ++iLogic) {
				bool bChange = false;
				pStrategy = GetColorStrategyHTML(cStringLine, iLogic, &pool, &pStrategyNormal, &pStrategyFound, bChange);
				if (bChange) {
					int nColorIdx = ToColorInfoArrIndex(pStrategy ? pStrategy->GetStrategyColor() : COLORIDX_TEXT);
					if (nColorIdx != -1) {
						const ColorInfo& info = type.colorInfoArr[nColorIdx];
						colorAttrNext = info.colorAttr;
						fontAttrNext  = info.fontAttr;
					}
				}
				if (nIdxFrom != -1 && nIdxFrom + nLineStart <= iLogic && iLogic <= nIdxTo + nLineStart) {
					if (nIdxFrom + nLineStart == iLogic) {
						colorAttrLast = colorAttrNext;
						fontAttrLast  = fontAttrNext;
					}else if (
						nIdxFrom + nLineStart < iLogic
						&& (
							fontAttr.bBoldFont != fontAttrNext.bBoldFont
						  	|| fontAttr.bUnderLine != fontAttrNext.bUnderLine
							|| colorAttr.cTEXT != colorAttrNext.cTEXT
						  	|| colorAttr.cBACK != colorAttrNext.cBACK
						)
					) {
						bAddCRLF = AppendHTMLColor(colorAttrLast, colorAttrLast2,
							fontAttrLast, fontAttrLast2, pLine + nBgnLogic, iLogic - nBgnLogic, memClip);
						colorAttrLast = colorAttrNext;
						fontAttrLast  = fontAttrNext;
						nBgnLogic = iLogic;
					}else if (nIdxTo + nLineStart == iLogic) {
						bAddCRLF = AppendHTMLColor(colorAttrLast, colorAttrLast2,
							fontAttrLast, fontAttrLast2, pLine + nBgnLogic, iLogic - nBgnLogic, memClip);
						nBgnLogic = iLogic;
					}
				}
				colorAttr = colorAttrNext;
				fontAttr = fontAttrNext;
			}
			if (nIdxFrom != -1 && nIdxTo + nLineStart == iLogic) {
				bAddCRLF = AppendHTMLColor(colorAttrLast, colorAttrLast2,
					fontAttrLast, fontAttrLast2, pLine + nBgnLogic, iLogic - nBgnLogic, memClip);
			}
			if (bLineNumber) {
				bool bAddLineNum = true;
				const Layout* pLayoutNext = pLayout->GetNextLayout();
				if (pLayoutNext) {
					if (type.bLineNumIsCRLF) {
						if (bLineNumLayout && pLayoutNext->GetLogicOffset() != 0) {
							bAddLineNum = true;
						}else {
							bAddLineNum = true;
						}
					}else {
						if (bLineNumLayout || pLayoutNext->GetLogicOffset() == 0) {
							bAddLineNum = true;
						}
					}
				}
				if (bAddLineNum) {
					if (fontAttrLast2.bBoldFont) {
						memClip.AppendStringLiteral(L"</b>");
					}
					if (fontAttrLast2.bUnderLine) {
						memClip.AppendStringLiteral(L"</u>");
					}
					if (colorAttrLast2.cTEXT != (COLORREF)-1) {
						memClip.AppendStringLiteral(L"</span>");
					}
					fontAttrLast.bBoldFont = fontAttrLast2.bBoldFont = false;
					fontAttrLast.bUnderLine = fontAttrLast2.bUnderLine = false;
					colorAttrLast.cTEXT = colorAttrLast2.cTEXT = (COLORREF)-1;
					colorAttrLast.cBACK = colorAttrLast2.cBACK = (COLORREF)-1;
				}
			}
			if (bLineNumLayout && !bAddCRLF) {
				memClip.AppendStringLiteral(WCODE::CRLF);
			}
			// 2014.06.25 �o�b�t�@�g��
			if (memClip.capacity() < memClip.GetStringLength() + 100) {
				memClip.AllocStringBuffer( memClip.capacity() + memClip.capacity() / 2 );
			}
		}
	}
	if (fontAttrLast2.bBoldFont) {
		memClip.AppendStringLiteral(L"</b>");
	}
	if (fontAttrLast2.bUnderLine) {
		memClip.AppendStringLiteral(L"</u>");
	}
	if (colorAttrLast2.cTEXT != (COLORREF)-1) {
		memClip.AppendStringLiteral(L"</span>");
	}
	memClip.AppendStringLiteral(L"</pre>");

	Clipboard clipboard(GetEditWindow().GetHwnd());
	if (!clipboard) {
		return;
	}
	clipboard.Empty();
	clipboard.SetHtmlText(memClip);
	clipboard.SetText(memClip.GetStringPtr(), memClip.GetStringLength(), false, false);
}



/*!
	@date 2014.12.30 Moca ����ColorStrategy�ňႤ�F�ɐ؂�ւ�����Ƃ��ɑΉ�
*/
ColorStrategy* ViewCommander::GetColorStrategyHTML(
	const StringRef&	stringLine,
	int					iLogic,
	const ColorStrategyPool*	pool,
	ColorStrategy**	ppStrategy,
	ColorStrategy**	ppStrategyFound,		// [in,out]
	bool& bChange
	)
{
	// �����F�I��
	if (*ppStrategyFound) {
		if ((*ppStrategyFound)->EndColor(stringLine, iLogic)) {
			*ppStrategyFound = nullptr;
			bChange = true;
		}
	}

	// �����F�J�n
	if (!*ppStrategyFound) {
		Color_Found*  pcFound  = pool->GetFoundStrategy();
		if (pcFound->BeginColor(stringLine, iLogic)) {
			*ppStrategyFound = pcFound;
			bChange = true;
		}
	}

	// �F�I��
	if (*ppStrategy) {
		if ((*ppStrategy)->EndColor(stringLine, iLogic)) {
			*ppStrategy = nullptr;
			bChange = true;
		}
	}

	// �F�J�n
	if (!*ppStrategy) {
		int size = pool->GetStrategyCount();
		for (int i=0; i<size; ++i) {
			if (pool->GetStrategy(i)->BeginColor(stringLine, iLogic)) {
				*ppStrategy = pool->GetStrategy(i);
				bChange = true;
				break;
			}
		}
	}
	if (*ppStrategyFound) {
		return *ppStrategyFound;
	}
	return *ppStrategy;
}

// �I��͈͓��s�ԍ��F�t��HTML�R�s�[
void ViewCommander::Command_Copy_Color_HTML_LineNumber()
{
	Command_Copy_Color_HTML(true);
}


/*!	���ݕҏW���̃t�@�C�������N���b�v�{�[�h�ɃR�s�[
	2002/2/3 aroka
*/
void ViewCommander::Command_CopyFileName(void)
{
	if (GetDocument().docFile.GetFilePathClass().IsValidPath()) {
		// �N���b�v�{�[�h�Ƀf�[�^��ݒ�
		const WCHAR* pszFile = to_wchar(GetDocument().docFile.GetFileName());
		view.MySetClipboardData(pszFile , wcslen(pszFile), false);
	}else {
		ErrorBeep();
	}
}


// ���ݕҏW���̃t�@�C���̃p�X�����N���b�v�{�[�h�ɃR�s�[
void ViewCommander::Command_CopyPath(void)
{
	if (GetDocument().docFile.GetFilePathClass().IsValidPath()) {
		// �N���b�v�{�[�h�Ƀf�[�^��ݒ�
		const TCHAR* szPath = GetDocument().docFile.GetFilePath();
		view.MySetClipboardData(szPath, _tcslen(szPath), false);
	}else {
		ErrorBeep();
	}
}


// May 9, 2000 genta
// ���ݕҏW���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���N���b�v�{�[�h�ɃR�s�[
void ViewCommander::Command_CopyTag(void)
{
	if (GetDocument().docFile.GetFilePathClass().IsValidPath()) {
		wchar_t	buf[MAX_PATH + 20];

		LogicPoint ptColLine;

		// �_���s�ԍ��𓾂�
		GetDocument().layoutMgr.LayoutToLogic(GetCaret().GetCaretLayoutPos(), &ptColLine);

		// �N���b�v�{�[�h�Ƀf�[�^��ݒ�
		auto_sprintf( buf, L"%ts (%d,%d): ", GetDocument().docFile.GetFilePath(), ptColLine.y+1, ptColLine.x+1 );
		view.MySetClipboardData(buf, wcslen(buf), false);
	}else {
		ErrorBeep();
	}
}


//// �L�[���蓖�Ĉꗗ���R�s�[
// Dec. 26, 2000 JEPRO // Jan. 24, 2001 JEPRO debug version (directed by genta)
void ViewCommander::Command_CreateKeyBindList(void)
{
	NativeW memKeyList;
	auto& csKeyBind = GetDllShareData().common.keyBind;
	KeyBind::CreateKeyBindList(
		G_AppInstance(),
		csKeyBind.nKeyNameArrNum,
		csKeyBind.pKeyNameArr,
		memKeyList,
		&GetDocument().funcLookup,	// Oct. 31, 2001 genta �ǉ�
		false	// 2007.02.22 ryoji �ǉ�
	);

	// Windows�N���b�v�{�[�h�ɃR�s�[
	// 2004.02.17 Moca �֐���
	SetClipboardText(
		EditWnd::getInstance().splitterWnd.GetHwnd(),
		memKeyList.GetStringPtr(),
		memKeyList.GetStringLength()
	);
}

