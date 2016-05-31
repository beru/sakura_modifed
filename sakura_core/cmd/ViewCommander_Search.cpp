/*!	@file
@brief ViewCommander�N���X�̃R�}���h(�����n ��{�`)�֐��Q

	2012/12/17	ViewCommander.cpp,ViewCommander_New.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, hor, YAZAKI
	Copyright (C) 2002, hor, YAZAKI, novice, Azumaiya, Moca
	Copyright (C) 2003, �����
	Copyright (C) 2004, Moca
	Copyright (C) 2005, �����, Moca, D.S.Koba
	Copyright (C) 2006, genta, ryoji, �����, yukihane
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2009, ryoji, genta
	Copyright (C) 2010, ryoji
	Copyright (C) 2011, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "dlg/DlgCancel.h"// 2002/2/8 hor
#include "SearchAgent.h"
#include "util/window.h"
#include "util/string_ex2.h"
#include <limits.h>
#include "sakura_rc.h"

/*!
����(�{�b�N�X)�R�}���h���s.
�c�[���o�[�̌����{�b�N�X�Ƀt�H�[�J�X���ړ�����.
	@date 2006.06.04 yukihane �V�K�쐬
*/
void ViewCommander::Command_Search_Box(void)
{
	GetEditWindow().toolbar.SetFocusSearchBox();
}


// ����(�P�ꌟ���_�C�A���O)
void ViewCommander::Command_Search_Dialog(void)
{
	// ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾
	NativeW memCurText;
	view.GetCurrentTextForSearchDlg(memCurText);	// 2006.08.23 ryoji �_�C�A���O��p�֐��ɕύX

	auto& dlgFind = GetEditWindow().dlgFind;
	// �����������������
	if (0 < memCurText.GetStringLength()) {
		dlgFind.strText = memCurText.GetStringPtr();
	}
	// �����_�C�A���O�̕\��
	if (!dlgFind.GetHwnd()) {
		dlgFind.DoModeless(G_AppInstance(), view.GetHwnd(), (LPARAM)&GetEditWindow().GetActiveView());
	}else {
		// �A�N�e�B�u�ɂ���
		ActivateFrameWindow(dlgFind.GetHwnd());
		dlgFind.SetItemText(IDC_COMBO_TEXT, memCurText.GetStringT());
	}
	return;
}


/*! ��������
	@param bChangeCurRegexp ���L�f�[�^�̌�����������g��
	@date 2003.05.22 ����� �����}�b�`�΍�D�s���E�s�������������D
	@date 2004.05.30 Moca bChangeCurRegexp=true�ŏ]���ʂ�Bfalse�ŁACEditView�̌��ݐݒ肳��Ă��錟���p�^�[�����g��
*/
void ViewCommander::Command_Search_Next(
	bool			bChangeCurRegexp,
	bool			bRedraw,
	bool			bReplaceAll,
	HWND			hwndParent,
	const WCHAR*	pszNotFoundMessage,
	Range*		pSelectLogic		// [out] �I��͈͂̃��W�b�N�ŁB�}�b�`�͈͂�Ԃ��B���ׂĒu��/�������[�h�Ŏg�p
	)
{
	bool	bSelecting;
	bool	bFlag1 = false;
	bool	bSelectingLock_Old = false;
	bool	bFound = false;
	bool	bDisableSelect = false;
	bool	b0Match = false;		// �����O�Ń}�b�`���Ă��邩�H�t���O by �����
	size_t	nIdx = 0;
	int		nLineNum(0);

	Range	rangeA;
	rangeA.Set(GetCaret().GetCaretLayoutPos());

	Range	selectBgn_Old;
	Range	select_Old;
	int nLineNumOld(0);

	// bFastMode
	int nLineNumLogic(0);

	bool bRedo = false;	// hor
	int nIdxOld = 0;	// hor
	auto& layoutMgr = GetDocument().layoutMgr;

	if (pSelectLogic) {
		pSelectLogic->Clear(-1);
	}

	bSelecting = false;
	// 2002.01.16 hor
	// ���ʕ����̂����肾��
	// 2004.05.30 Moca CEditView�̌��ݐݒ肳��Ă��錟���p�^�[�����g����悤��
	if (bChangeCurRegexp && !view.ChangeCurRegexp()) {
		return;
	}
	if (view.strCurSearchKey.size() == 0) {
		goto end_of_func;
	}

	auto& si = view.GetSelectionInfo();
	auto& caret = GetCaret();
	// �����J�n�ʒu�𒲐�
	bFlag1 = false;
	if (!pSelectLogic && si.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// ��`�͈͑I�𒆂łȂ� & �I����Ԃ̃��b�N
		if (!si.IsBoxSelecting() && si.bSelectingLock) {
			bSelecting = true;
			bSelectingLock_Old = si.bSelectingLock;
			selectBgn_Old = si.selectBgn; // �͈͑I��(���_)
			select_Old = GetSelect();

			if (PointCompare(si.selectBgn.GetFrom(), caret.GetCaretLayoutPos()) >= 0) {
				// �J�[�\���ړ�
				caret.SetCaretLayoutPos(GetSelect().GetFrom());
				if (GetSelect().IsOne()) {
					// ���݁A�����O�Ń}�b�`���Ă���ꍇ�͂P�����i�߂�(�����}�b�`�΍�) by �����
					b0Match = true;
				}
				bFlag1 = true;
			}else {
				// �J�[�\���ړ�
				caret.SetCaretLayoutPos(GetSelect().GetTo());
				if (GetSelect().IsOne()) {
					// ���݁A�����O�Ń}�b�`���Ă���ꍇ�͂P�����i�߂�(�����}�b�`�΍�) by �����
					b0Match = true;
				}
			}
		}else {
			// �J�[�\���ړ�
			caret.SetCaretLayoutPos(GetSelect().GetTo());
			if (GetSelect().IsOne()) {
				// ���݁A�����O�Ń}�b�`���Ă���ꍇ�͂P�����i�߂�(�����}�b�`�΍�) by �����
				b0Match = true;
			}

			// ���݂̑I��͈͂��I����Ԃɖ߂�
			si.DisableSelectArea(bRedraw, false);
			bDisableSelect = true;
		}
	}
	if (!pSelectLogic) {
		nLineNum = caret.GetCaretLayoutPos().y;
		size_t nLineLen = 0; // 2004.03.17 Moca pLine == NULL �̂Ƃ��AnLineLen�����ݒ�ɂȂ藎����o�O�΍�
		const Layout*	pLayout;
		const wchar_t*	pLine = layoutMgr.GetLineStr(nLineNum, &nLineLen, &pLayout);

		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
		// 2002.02.08 hor EOF�݂̂̍s��������������Ă��Č����\�� (2/2)
		nIdx = pLayout ? view.LineColumnToIndex(pLayout, caret.GetCaretLayoutPos().x) : 0;
		if (b0Match) {
			// ���݁A�����O�Ń}�b�`���Ă���ꍇ�͕����s�łP�����i�߂�(�����}�b�`�΍�)
			if (nIdx < nLineLen) {
				// 2005-09-02 D.S.Koba GetSizeOfChar
				nIdx += NativeW::GetSizeOfChar(pLine, nLineLen, nIdx) == 2 ? 2 : 1;
			}else {
				// �O�̂��ߍs���͕ʏ���
				++nIdx;
			}
		}
	}else {
		nLineNumLogic = caret.GetCaretLogicPos().y;
		nIdx = caret.GetCaretLogicPos().x;
	}

	nLineNumOld = nLineNum;	// hor
	bRedo		= true;		// hor
	nIdxOld		= (int)nIdx;		// hor

re_do:;
	// ���݈ʒu�����̈ʒu����������
	// 2004.05.30 Moca ������GetShareData()���烁���o�ϐ��ɕύX�B���̃v���Z�X/�X���b�h�ɏ����������Ă��܂�Ȃ��悤�ɁB
	bool bSearchResult;
	if (!pSelectLogic) {
		bSearchResult = layoutMgr.SearchWord(
			nLineNum,						// �����J�n���C�A�E�g�s
			nIdx,							// �����J�n�f�[�^�ʒu
			SearchDirection::Forward,		// 0==�O������ 1==�������
			&rangeA,						// �}�b�`���C�A�E�g�͈�
			view.searchPattern
		);
	}else {
		auto& docLineMgr = GetDocument().docLineMgr;
		bSearchResult = SearchAgent(docLineMgr).SearchWord(
			Point((int)nIdx, nLineNumLogic),
			SearchDirection::Forward,		// 0==�O������ 1==�������
			pSelectLogic,
			view.searchPattern
		);
	}
	if (bSearchResult) {
		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
		if (bFlag1 && rangeA.GetFrom() == caret.GetCaretLayoutPos()) {
			Range logicRange;
			layoutMgr.LayoutToLogic(rangeA, &logicRange);

			nLineNum = rangeA.GetTo().y;
			nIdx     = logicRange.GetTo().x;
			if (logicRange.GetFrom() == logicRange.GetTo()) { // ��0�}�b�`�ł̖������[�v�΍�B
				nIdx += 1; // wchar_t����i�߂邾���ł͑���Ȃ���������Ȃ����B
			}
			goto re_do;
		}

		if (bSelecting) {
			// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
			si.ChangeSelectAreaByCurrentCursor(rangeA.GetTo());
			si.bSelectingLock = bSelectingLock_Old;	// �I����Ԃ̃��b�N
		}else if (!pSelectLogic) {
			// �I��͈͂̕ύX
			// 2005.06.24 Moca
			si.SetSelectArea(rangeA);

			if (bRedraw) {
				// �I��̈�`��
				si.DrawSelectArea();
			}
		}

		// �J�[�\���ړ�
		// Sep. 8, 2000 genta
		if (!bReplaceAll) view.AddCurrentLineToHistory();	// 2002.02.16 hor ���ׂĒu���̂Ƃ��͕s�v
		if (!pSelectLogic) {
			caret.MoveCursor(rangeA.GetFrom(), bRedraw);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
		}else {
			caret.MoveCursorFastMode(pSelectLogic->GetFrom());
		}
		bFound = true;
	}else {
		if (bSelecting) {
			si.bSelectingLock = bSelectingLock_Old;	// �I����Ԃ̃��b�N

			// �I��͈͂̕ύX
			si.selectBgn = selectBgn_Old; // �͈͑I��(���_)
			si.selectOld = select_Old;	// 2011.12.24
			GetSelect().SetFrom(select_Old.GetFrom());
			GetSelect().SetTo(rangeA.GetFrom());

			// �J�[�\���ړ�
			caret.MoveCursor(rangeA.GetFrom(), bRedraw);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

			if (bRedraw) {
				// �I��̈�`��
				si.DrawSelectArea();
			}
		}else {
			if (bDisableSelect) {
				// 2011.12.21 ���W�b�N�J�[�\���ʒu�̏C��/�J�[�\�����E�Ί��ʂ̕\��
				Point ptLogic = layoutMgr.LayoutToLogic(caret.GetCaretLayoutPos());
				caret.SetCaretLogicPos(ptLogic);
				view.DrawBracketCursorLine(bRedraw);
			}
		}
	}

end_of_func:;
// From Here 2002.01.26 hor �擪�i�����j����Č���
	if (GetDllShareData().common.search.bSearchAll) {
		if (!bFound	&&		// ������Ȃ�����
			bRedo	&&		// �ŏ��̌���
			!bReplaceAll	// �S�Ēu���̎��s������Ȃ�
		) {
			nLineNum	= 0;
			nIdx		= 0;
			bRedo		= false;
			goto re_do;		// �擪����Č���
		}
	}

	if (bFound) {
		if (!pSelectLogic && ((nLineNumOld > nLineNum) || (nLineNumOld == nLineNum && nIdxOld > (int)nIdx)))
			view.SendStatusMessage(LS(STR_ERR_SRNEXT1));
	}else {
		caret.ShowEditCaret();	// 2002/04/18 YAZAKI
		caret.ShowCaretPosInfo();	// 2002/04/18 YAZAKI
		if (!bReplaceAll) {
			view.SendStatusMessage(LS(STR_ERR_SRNEXT2));
		}
// To Here 2002.01.26 hor

		// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
		if (!pszNotFoundMessage) {
			NativeW keyName;
			auto& curSearchKey = view.strCurSearchKey;
			LimitStringLength(curSearchKey.c_str(), curSearchKey.size(), _MAX_PATH, keyName);
			if (keyName.GetStringLength() < curSearchKey.size()) {
				keyName.AppendStringLiteral(L"...");
			}
			AlertNotFound(
				hwndParent,
				bReplaceAll,
				LS(STR_ERR_SRNEXT3),
				keyName.GetStringPtr()
			);
		}else {
			AlertNotFound(hwndParent, bReplaceAll, _T("%ls"), pszNotFoundMessage);
		}
	}
}


// �O������
void ViewCommander::Command_Search_Prev(bool bReDraw, HWND hwndParent)
{
	bool		bSelectingLock_Old = false;
	bool		bFound = false;
	bool		bRedo = false;			// hor
	bool		bDisableSelect = false;
	size_t		nLineNumOld = 0;
	size_t		nIdxOld = 0;
	size_t		nLineNum = 0;
	size_t		nIdx = 0;

	auto& caret = GetCaret();
	auto& layoutMgr = GetDocument().layoutMgr;

	Range rangeA;
	rangeA.Set(caret.GetCaretLayoutPos());

	Range selectBgn_Old;
	Range select_Old;

	bool bSelecting = false;
	// 2002.01.16 hor
	// ���ʕ����̂����肾��
	if (!view.ChangeCurRegexp()) {
		return;
	}
	if (view.strCurSearchKey.size() == 0) {
		goto end_of_func;
	}
	auto& si = view.GetSelectionInfo();
	if (si.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		selectBgn_Old = si.selectBgn; // �͈͑I��(���_)
		select_Old = GetSelect();
		
		bSelectingLock_Old = si.bSelectingLock;

		// ��`�͈͑I�𒆂�
		if (!si.IsBoxSelecting() && si.bSelectingLock) {	// �I����Ԃ̃��b�N
			bSelecting = true;
		}else {
			// ���݂̑I��͈͂��I����Ԃɖ߂�
			si.DisableSelectArea(bReDraw, false);
			bDisableSelect = true;
		}
	}

	nLineNum = caret.GetCaretLayoutPos().y;
	const Layout* pLayout = layoutMgr.SearchLineByLayoutY(nLineNum);
	if (!pLayout) {
		// pLayout��NULL�ƂȂ�̂́A[EOF]����O���������ꍇ
		// �P�s�O�Ɉړ����鏈��
		--nLineNum;
		if (nLineNum < 0) {
			goto end_of_func;
		}
		pLayout = layoutMgr.SearchLineByLayoutY(nLineNum);
		if (!pLayout) {
			goto end_of_func;
		}
		// �J�[�\�����ړ��͂�߂� nIdx�͍s�̒����Ƃ��Ȃ���[EOF]������s��O�����������ɍŌ�̉��s�������ł��Ȃ� 2003.05.04 �����
		pLayout = layoutMgr.SearchLineByLayoutY(nLineNum);
		nIdx = pLayout->GetDocLineRef()->GetLengthWithEOL() + 1;		// �s���̃k������(\0)�Ƀ}�b�`�����邽�߂�+1 2003.05.16 �����
	}else {
		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
		nIdx = view.LineColumnToIndex(pLayout, caret.GetCaretLayoutPos().x);
	}

	bRedo		=	true;		// hor
	nLineNumOld	=	nLineNum;	// hor
	nIdxOld		=	nIdx;		// hor
re_do:;							// hor
	// ���݈ʒu���O�̈ʒu����������
	if (layoutMgr.SearchWord(
			nLineNum,								// �����J�n���C�A�E�g�s
			nIdx,									// �����J�n�f�[�^�ʒu
			SearchDirection::Backward,				// 0==�O������ 1==�������
			&rangeA,								// �}�b�`���C�A�E�g�͈�
			view.searchPattern
		)
	) {
		if (bSelecting) {
			// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
			si.ChangeSelectAreaByCurrentCursor(rangeA.GetFrom());
			si.bSelectingLock = bSelectingLock_Old;	// �I����Ԃ̃��b�N
		}else {
			// �I��͈͂̕ύX
			// 2005.06.24 Moca
			si.SetSelectArea(rangeA);

			if (bReDraw) {
				// �I��̈�`��
				si.DrawSelectArea();
			}
		}
		// �J�[�\���ړ�
		// Sep. 8, 2000 genta
		view.AddCurrentLineToHistory();
		caret.MoveCursor(rangeA.GetFrom(), bReDraw);
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
		bFound = true;
	}else {
		if (bSelecting) {
			si.bSelectingLock = bSelectingLock_Old;	// �I����Ԃ̃��b�N
			// �I��͈͂̕ύX
			si.selectBgn = selectBgn_Old;
			GetSelect() = select_Old;

			// �J�[�\���ړ�
			caret.MoveCursor(rangeA.GetFrom(), bReDraw);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
			// �I��̈�`��
			si.DrawSelectArea();
		}else {
			if (bDisableSelect) {
				view.DrawBracketCursorLine(bReDraw);
			}
		}
	}
end_of_func:;
// From Here 2002.01.26 hor �擪�i�����j����Č���
	if (GetDllShareData().common.search.bSearchAll) {
		if (!bFound	&&	// ������Ȃ�����
			bRedo		// �ŏ��̌���
		) {
			nLineNum	= layoutMgr.GetLineCount() - 1;
			nIdx		= MAXLINEKETAS;
			bRedo		= false;
			goto re_do;	// ��������Č���
		}
	}
	if (bFound) {
		if ((nLineNumOld < nLineNum)||(nLineNumOld == nLineNum && nIdxOld < nIdx))
			view.SendStatusMessage(LS(STR_ERR_SRPREV1));
	}else {
		view.SendStatusMessage(LS(STR_ERR_SRPREV2));
// To Here 2002.01.26 hor

		// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
		NativeW keyName;
		auto& curSearchKey = view.strCurSearchKey;
		LimitStringLength(curSearchKey.c_str(), curSearchKey.size(), _MAX_PATH, keyName);
		if (keyName.GetStringLength() < curSearchKey.size()) {
			keyName.AppendStringLiteral(L"...");
		}
		AlertNotFound(
			hwndParent,
			false,
			LS(STR_ERR_SRPREV3),	// Jan. 25, 2001 jepro ���b�Z�[�W���኱�ύX
			keyName.GetStringPtr()
		);
	}
	return;
}


// �u��(�u���_�C�A���O)
void ViewCommander::Command_Replace_Dialog(void)
{
	bool bSelected = false;

	// ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾
	NativeW memCurText;
	view.GetCurrentTextForSearchDlg(memCurText);	// 2006.08.23 ryoji �_�C�A���O��p�֐��ɕύX

	auto& dlgReplace = GetEditWindow().dlgReplace;

	// �����������������
	if (0 < memCurText.GetStringLength()) {
		dlgReplace.strText = memCurText.GetStringPtr();
	}
	if (0 < GetDllShareData().searchKeywords.replaceKeys.size()) {
		if (dlgReplace.nReplaceKeySequence < GetDllShareData().common.search.nReplaceKeySequence) {
			dlgReplace.strText2 = GetDllShareData().searchKeywords.replaceKeys[0];	// 2006.08.23 ryoji �O��̒u���㕶����������p��
		}
	}
	
	if (view.GetSelectionInfo().IsTextSelected() && !GetSelect().IsLineOne()) {
		bSelected = true;	// �I��͈͂��`�F�b�N���ă_�C�A���O�\��
	}else {
		bSelected = false;	// �t�@�C���S�̂��`�F�b�N���ă_�C�A���O�\��
	}
	// �u���I�v�V�����̏�����
	dlgReplace.nReplaceTarget = 0;	// �u���Ώ�
	dlgReplace.bPaste = false;		// �\��t����H
// To Here 2001.12.03 hor

	// �u���_�C�A���O�̕\��
	// From Here Jul. 2, 2001 genta �u���E�B���h�E��2�d�J����}�~
	if (!::IsWindow(dlgReplace.GetHwnd())) {
		dlgReplace.DoModeless(G_AppInstance(), view.GetHwnd(), (LPARAM)&view, bSelected);
	}else {
		// �A�N�e�B�u�ɂ���
		ActivateFrameWindow(dlgReplace.GetHwnd());
		dlgReplace.SetItemText(IDC_COMBO_TEXT, memCurText.GetStringT());
	}
	// To Here Jul. 2, 2001 genta �u���E�B���h�E��2�d�J����}�~
	return;
}


/*! �u�����s
	
	@date 2002/04/08 �e�E�B���h�E���w�肷��悤�ɕύX�B
	@date 2003.05.17 ����� �����O�}�b�`�̖����u������Ȃ�
	@date 2011.12.18 Moca �I�v�V�����E�����L�[��DllShareData����dlgReplace/EditView�x�[�X�ɕύX�B�����񒷐����̓P�p
*/
void ViewCommander::Command_Replace(HWND hwndParent)
{
	if (!hwndParent) {	// �e�E�B���h�E���w�肳��Ă��Ȃ���΁ACEditView���e�B
		hwndParent = view.GetHwnd();
	}
	auto& dlgReplace = GetEditWindow().dlgReplace;
	// 2002.02.10 hor
	int nPaste			=	dlgReplace.bPaste;
	int nReplaceTarget	=	dlgReplace.nReplaceTarget;

	if (nPaste && nReplaceTarget == 3) {
		// �u���ΏہF�s�폜�̂Ƃ��́A�N���b�v�{�[�h����\��t���𖳌��ɂ���
		nPaste = FALSE;
	}

	// From Here 2001.12.03 hor
	if (nPaste && !GetDocument().docEditor.IsEnablePaste()) {
		OkMessage(hwndParent, LS(STR_ERR_CEDITVIEW_CMD10));
		dlgReplace.CheckButton(IDC_CHK_PASTE, false);
		dlgReplace.EnableItem(IDC_COMBO_TEXT2, true);
		return;	// ���sreturn;
	}

	// 2002.01.09 hor
	// �I���G���A������΁A���̐擪�ɃJ�[�\�����ڂ�
	if (view.GetSelectionInfo().IsTextSelected()) {
		if (view.GetSelectionInfo().IsBoxSelecting()) {
			GetCaret().MoveCursor(GetSelect().GetFrom(), true);
		}else {
			Command_Left(false, false);
		}
	}
	// To Here 2002.01.09 hor
	
	// ��`�I���H
//			bBeginBoxSelect = view.GetSelectionInfo().IsBoxSelecting();

	// �J�[�\�����ړ�
	//HandleCommand(F_LEFT, true, 0, 0, 0, 0);	//�H�H�H
	// To Here 2001.12.03 hor

	// �e�L�X�g�I������
	// ���݂̑I��͈͂��I����Ԃɖ߂�
	view.GetSelectionInfo().DisableSelectArea(true);

	// 2004.06.01 Moca �������ɁA���̃v���Z�X�ɂ����replaceKeys�������������Ă����v�Ȃ悤��
	const NativeW memRepKey(dlgReplace.strText2.c_str());

	// ��������
	Command_Search_Next(true, true, false, hwndParent, nullptr);

	bool	bRegularExp = view.curSearchOption.bRegularExp;
	int 	nFlag       = view.curSearchOption.bLoHiCase ? 0x01 : 0x00;
	auto& caret = GetCaret();

	// �e�L�X�g���I������Ă��邩
	if (view.GetSelectionInfo().IsTextSelected()) {
		// From Here 2001.12.03 hor
		Point ptTmp(0, 0);
		if (nPaste || !bRegularExp) {
			// ���K�\������ ����Q��($&)�Ŏ�������̂ŁA���K�\���͏��O
			if (nReplaceTarget == 1) {	// �}���ʒu�ֈړ�
				ptTmp = GetSelect().GetTo() - GetSelect().GetFrom();
				GetSelect().Clear(-1);
			}else if (nReplaceTarget == 2) {	// �ǉ��ʒu�ֈړ�
				// ���K�\�������O�����̂ŁA�u������̕��������s������玟�̍s�̐擪�ֈړ��v�̏������폜
				caret.MoveCursor(GetSelect().GetTo(), false);
				GetSelect().Clear(-1);
			}else {
				// �ʒu�w��Ȃ��̂ŁA�������Ȃ�
			}
		}
		auto& layoutMgr = GetDocument().layoutMgr;
		// �s�폜 �I��͈͂��s�S�̂Ɋg��B�J�[�\���ʒu���s����(���K�\���ł����s)
		if (nReplaceTarget == 3) {
			Point lineHome = layoutMgr.LayoutToLogic(GetSelect().GetFrom());
			lineHome.x = 0; // �s��
			Range selectFix;
			selectFix.SetFrom(layoutMgr.LogicToLayout(lineHome));
			lineHome.y++; // ���s�̍s��
			selectFix.SetTo(layoutMgr.LogicToLayout(lineHome));
			caret.GetAdjustCursorPos(&selectFix.GetTo());
			view.GetSelectionInfo().SetSelectArea(selectFix);
			view.GetSelectionInfo().DrawSelectArea();
			caret.MoveCursor(selectFix.GetFrom(), false);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
		}
		// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
		// �e�L�X�g��\��t��
		if (nPaste) {
			Command_Paste(0);
		} else if (nReplaceTarget == 3) {
			// �s�폜
			Command_InsText( false, L"", 0, true );
		}else if (bRegularExp) { // �����^�u��  1==���K�\��
			// ��ǂ݂ɑΉ����邽�߂ɕ����s���܂ł��g���悤�ɕύX 2005/03/27 �����
			// 2002/01/19 novice ���K�\���ɂ�镶����u��
			Bregexp regexp;

			if (!InitRegexp(view.GetHwnd(), regexp, true)) {
				return;	// ���sreturn;
			}

			// �����s�A�����s���A�����s�ł̌����}�b�`�ʒu
			const Layout* pLayout = layoutMgr.SearchLineByLayoutY(GetSelect().GetFrom().y);
			const wchar_t* pLine = pLayout->GetDocLineRef()->GetPtr();
			size_t nIdx = view.LineColumnToIndex(pLayout, GetSelect().GetFrom().x) + pLayout->GetLogicOffset();
			size_t nLen = pLayout->GetDocLineRef()->GetLengthWithEOL();
			// ���K�\���őI���n�_�E�I�_�ւ̑}�����L�q
			// Jun. 6, 2005 �����
			// ������ł́u�����̌��̕��������s�������玟�̍s���ֈړ��v�������ł��Ȃ�
			// �� Oct. 30, �u�����̌��̕��������s��������E�E�v�̏�������߂�i�N������Ȃ��݂����Ȃ̂Łj
			// Nov. 9, 2005 ����� ���K�\���őI���n�_�E�I�_�ւ̑}�����@��ύX(��)
			NativeW memMatchStr; memMatchStr.SetString(L"$&");
			NativeW memRepKey2;
			if (nReplaceTarget == 1) {	// �I���n�_�֑}��
				memRepKey2 = memRepKey;
				memRepKey2 += memMatchStr;
			}else if (nReplaceTarget == 2) { // �I���I�_�֑}��
				memRepKey2 = memMatchStr;
				memRepKey2 += memRepKey;
			}else {
				memRepKey2 = memRepKey;
			}
			regexp.Compile(view.strCurSearchKey.c_str(), memRepKey2.GetStringPtr(), nFlag);
			if (regexp.Replace(pLine, nLen, nIdx)) {
				// From Here Jun. 6, 2005 �����
				// �����s���܂�INSTEXT������@�́A�L�����b�g�ʒu�𒲐�����K�v������A
				// �L�����b�g�ʒu�̌v�Z�����G�ɂȂ�B�i�u����ɉ��s������ꍇ�ɕs������j
				// �����ŁAINSTEXT���镶���񒷂𒲐�������@�ɕύX����i���͂������̕����킩��₷���j
				size_t matchLen = regexp.GetMatchLen();
				size_t nIdxTo = nIdx + matchLen;		// ����������̖���
				if (matchLen == 0) {
					// �O�����}�b�`�̎�(�����u���ɂȂ�Ȃ��悤�ɂP�����i�߂�)
					if (nIdxTo < nLen) {
						// 2005-09-02 D.S.Koba GetSizeOfChar
						nIdxTo += NativeW::GetSizeOfChar(pLine, nLen, nIdxTo) == 2 ? 2 : 1;
					}
					// �����u�����Ȃ��悤�ɁA�P�������₵���̂łP�����I���ɕύX
					// �I���n�_�E�I�_�ւ̑}���̏ꍇ���O�����}�b�`���͓���͓����ɂȂ�̂�
					GetSelect().SetTo(layoutMgr.LogicToLayout(Point((int)nIdxTo, pLayout->GetLogicLineNo())));	// 2007.01.19 ryoji �s�ʒu���擾����
				}
				// �s�����猟�������񖖔��܂ł̕�����
				ASSERT_GE(nLen, nIdxTo);
				size_t colDiff = nLen - nIdxTo;
				// Oct. 22, 2005 Karoto
				// \r��u������Ƃ��̌���\n�������Ă��܂����̑Ή�
				if (colDiff < pLayout->GetDocLineRef()->GetEol().GetLen()) {
					// ���s�ɂ������Ă�����A�s�S�̂�INSTEXT����B
					colDiff = 0;
					GetSelect().SetTo(layoutMgr.LogicToLayout(Point((int)nLen, pLayout->GetLogicLineNo())));	// 2007.01.19 ryoji �ǉ�
				}
				// �u���㕶����ւ̏�������(�s�����猟�������񖖔��܂ł̕���������)
				Command_InsText(false, regexp.GetString(), regexp.GetStringLen() - colDiff, true);
				// To Here Jun. 6, 2005 �����
			}
		}else {
			Command_InsText(false, memRepKey.GetStringPtr(), memRepKey.GetStringLength(), true);
		}

		// �}����̌����J�n�ʒu�𒲐�
		if (nReplaceTarget == 1) {
			caret.SetCaretLayoutPos(caret.GetCaretLayoutPos() + ptTmp);
		}

		// To Here 2001.12.03 hor
		/* �Ō�܂Œu����������OK�����܂Œu���O�̏�Ԃ��\�������̂ŁA
		** �u����A������������O�ɏ������� 2003.05.17 �����
		*/
		view.Redraw();

		// ��������
		Command_Search_Next(true, true, false, hwndParent, LSW(STR_ERR_CEDITVIEW_CMD11));
	}
}


/*! ���ׂĒu�����s

	@date 2003.05.22 ����� �����}�b�`�΍�D�s���E�s�������Ȃǌ�����
	@date 2006.03.31 ����� �s�u���@�\�ǉ�
	@date 2007.01.16 ryoji �s�u���@�\��S�u���̃I�v�V�����ɕύX
	@date 2009.09.20 genta �����`�E��ŋ�`�I�����ꂽ�̈�̒u�����s���Ȃ�
	@date 2010.09.17 ryoji ���C�����[�h�\��t��������ǉ�
	@date 2011.12.18 Moca �I�v�V�����E�����L�[��DllShareData����dlgReplace/EditView�x�[�X�ɕύX�B�����񒷐����̓P�p
	@date 2013.05.10 Moca fastMode
*/
void ViewCommander::Command_Replace_All()
{
	// sSearchOption�I���̂��߂̐�ɓK�p
	if (!view.ChangeCurRegexp()) {
		return;
	}

	auto& dlgReplace = GetEditWindow().dlgReplace;
	// 2002.02.10 hor
	bool bPaste			= dlgReplace.bPaste;
	BOOL nReplaceTarget	= dlgReplace.nReplaceTarget;
	bool bRegularExp	= view.curSearchOption.bRegularExp;
	bool bSelectedArea	= dlgReplace.bSelectedArea;
	bool bConsecutiveAll = dlgReplace.bConsecutiveAll;	// �u���ׂĒu���v�͒u���̌J�Ԃ�	// 2007.01.16 ryoji
	if (bPaste && nReplaceTarget == 3) {
		// �u���ΏہF�s�폜�̂Ƃ��́A�N���b�v�{�[�h����\��t���𖳌��ɂ���
		bPaste = false;
	}

	dlgReplace.bCanceled = false;
	dlgReplace.nReplaceCnt = 0;

	// From Here 2001.12.03 hor
	if (bPaste && !GetDocument().docEditor.IsEnablePaste()) {
		OkMessage(view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD10));
		dlgReplace.CheckButton(IDC_CHK_PASTE, false);
		dlgReplace.EnableItem(IDC_COMBO_TEXT2, true);
		return;	// TRUE;
	}
	// To Here 2001.12.03 hor
	bool bBeginBoxSelect; // ��`�I���H
	if (view.GetSelectionInfo().IsTextSelected()) {
		bBeginBoxSelect = view.GetSelectionInfo().IsBoxSelecting();
	}else {
		bSelectedArea = false;
		bBeginBoxSelect = false;
	}

	// �\������ON/OFF
	bool bDisplayUpdate = false;
	const bool bDrawSwitchOld = view.SetDrawSwitch(bDisplayUpdate);

	auto& layoutMgr = GetDocument().layoutMgr;
	auto& docLineMgr = GetDocument().docLineMgr;
	bool bFastMode = false;
	if ((docLineMgr.GetLineCount() * 10 < layoutMgr.GetLineCount())
		&& !(bSelectedArea || bPaste)
	) {
		// 1�s������10���C�A�E�g�s�ȏ�ŁA�I���E�y�[�X�g�łȂ��ꍇ
		bFastMode = true;
	}
	size_t nAllLineNum; // $$�P�ʍ���
	if (bFastMode) {
		nAllLineNum = layoutMgr.GetLineCount();
	}else {
		nAllLineNum = docLineMgr.GetLineCount();
	}
	size_t	nAllLineNumOrg = nAllLineNum;
	size_t	nAllLineNumLogicOrg = docLineMgr.GetLineCount();

	// �i���\��&���~�_�C�A���O�̍쐬
	DlgCancel	dlgCancel;
	HWND		hwndCancel = dlgCancel.DoModeless(G_AppInstance(), view.GetHwnd(), IDD_REPLACERUNNING);
	::EnableWindow(view.GetHwnd(), FALSE);
	::EnableWindow(::GetParent(view.GetHwnd()), FALSE);
	::EnableWindow(::GetParent(::GetParent(view.GetHwnd())), FALSE);
	//<< 2002/03/26 Azumaiya
	// ����Z�|���Z�������ɐi���󋵂�\����悤�ɁA�V�t�g���Z������B
	int nShiftCount;
	for (nShiftCount=0; 300<nAllLineNum; ++nShiftCount) {
		nAllLineNum /= 2;
	}
	//>> 2002/03/26 Azumaiya

	// �v���O���X�o�[������
	HWND		hwndProgress = ::GetDlgItem(hwndCancel, IDC_PROGRESS_REPLACE);
	Progress_SetRange(hwndProgress, 0, nAllLineNum + 1);
	int			nNewPos = 0;
	int			nOldPos = -1;
	Progress_SetPos(hwndProgress, nNewPos);

	// �u����������
	int			nReplaceNum = 0;
	HWND		hwndStatic = ::GetDlgItem(hwndCancel, IDC_STATIC_KENSUU);
	TCHAR szLabel[64];
	_itot(nReplaceNum, szLabel, 10);
	::SendMessage(hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel);

	Range rangeA;	// �I��͈�
	Point ptColLineP;

	// From Here 2001.12.03 hor
	auto& caret = GetCaret();
	if (bSelectedArea) {
		// �I��͈͒u��
		// �I��͈͊J�n�ʒu�̎擾
		rangeA = GetSelect();

		// From Here 2007.09.20 genta ��`�͈͂̑I��u�����ł��Ȃ�
		// �����`�E��ƑI�������ꍇ�CnSelectColumnTo < nSelectColumnFrom �ƂȂ邪�C
		// �͈̓`�F�b�N�� colFrom < colTo �����肵�Ă���̂ŁC
		// ��`�I���̏ꍇ�͍���`�E���w��ɂȂ�悤������ꊷ����D
		if (bBeginBoxSelect && rangeA.GetTo().x < rangeA.GetFrom().x)
			t_swap(rangeA.GetFrom().x, rangeA.GetTo().x);
		// To Here 2007.09.20 genta ��`�͈͂̑I��u�����ł��Ȃ�

		ptColLineP = layoutMgr.LayoutToLogic(rangeA.GetTo());
		// �I��͈͊J�n�ʒu�ֈړ�
		caret.MoveCursor(rangeA.GetFrom(), bDisplayUpdate);
	}else {
		// �t�@�C���S�̒u��
		// �t�@�C���̐擪�Ɉړ�
	//	HandleCommand(F_GOFILETOP, bDisplayUpdate, 0, 0, 0, 0);
		Command_GoFileTop(bDisplayUpdate);
	}

	Point ptLast = caret.GetCaretLayoutPos();
	Point ptLastLogic = caret.GetCaretLogicPos();

	// �e�L�X�g�I������
	// ���݂̑I��͈͂��I����Ԃɖ߂�
	view.GetSelectionInfo().DisableSelectArea(bDisplayUpdate);

	Range selectLogic;	// �u��������GetSelect()��Logic�P�ʔ�
	// ��������
	Command_Search_Next(true, bDisplayUpdate, true, 0, NULL, bFastMode ? &selectLogic : nullptr);
	// To Here 2001.12.03 hor

	//<< 2002/03/26 Azumaiya
	// �������������Ƃ��ŗD��ɑg��ł݂܂����B
	// ���[�v�̊O�ŕ�����̒��������ł���̂ŁA�ꎞ�ϐ����B
	const wchar_t* szREPLACEKEY;		// �u���㕶����B
	bool		bColumnSelect = false;	// ��`�\��t�����s�����ǂ����B
	bool		bLineSelect = false;	// ���C�����[�h�\��t�����s�����ǂ���
	NativeW	memClip;				// �u���㕶����̃f�[�^�i�f�[�^���i�[���邾���ŁA���[�v���ł͂��̌`�ł̓f�[�^�������܂���j�B

	// �N���b�v�{�[�h����̃f�[�^�\��t�����ǂ����B
	if (bPaste) {
		// �N���b�v�{�[�h����f�[�^���擾�B
		if (!view.MyGetClipboardData(memClip, &bColumnSelect, GetDllShareData().common.edit.bEnableLineModePaste ? &bLineSelect : nullptr)) {
			ErrorBeep();
			view.SetDrawSwitch(bDrawSwitchOld);

			::EnableWindow(view.GetHwnd(), TRUE);
			::EnableWindow(::GetParent(view.GetHwnd()), TRUE);
			::EnableWindow(::GetParent(::GetParent(view.GetHwnd())), TRUE);
			return;
		}

		// ��`�\��t����������Ă��āA�N���b�v�{�[�h�̃f�[�^����`�I���̂Ƃ��B
		if (GetDllShareData().common.edit.bAutoColumnPaste && bColumnSelect) {
			// �}�E�X�ɂ��͈͑I��
			if (view.GetSelectionInfo().IsMouseSelecting()) {
				ErrorBeep();
				view.SetDrawSwitch(bDrawSwitchOld);
				::EnableWindow(view.GetHwnd(), TRUE);
				::EnableWindow(::GetParent(view.GetHwnd()), TRUE);
				::EnableWindow(::GetParent(::GetParent(view.GetHwnd())), TRUE);
				return;
			}

			// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
			if (!GetDllShareData().common.view.bFontIs_FixedPitch) {
				view.SetDrawSwitch(bDrawSwitchOld);
				::EnableWindow(view.GetHwnd(), TRUE);
				::EnableWindow(::GetParent(view.GetHwnd()), TRUE);
				::EnableWindow(::GetParent(::GetParent(view.GetHwnd())), TRUE);
				return;
			}
		}else { // �N���b�v�{�[�h����̃f�[�^�͕��ʂɈ����B
			bColumnSelect = false;
		}
	}else {
		// 2004.05.14 Moca �S�u���̓r���ő��̃E�B���h�E�Œu�������Ƃ܂����̂ŃR�s�[����
		memClip.SetString(dlgReplace.strText2.c_str());
	}

	size_t nReplaceKey;			// �u���㕶����̒����B
	szREPLACEKEY = memClip.GetStringPtr(&nReplaceKey);

	// �s�R�s�[�iMSDEVLineSelect�`���j�̃e�L�X�g�Ŗ��������s�ɂȂ��Ă��Ȃ���Ή��s��ǉ�����
	// �����C�A�E�g�܂�Ԃ��̍s�R�s�[�������ꍇ�͖��������s�ɂȂ��Ă��Ȃ�
	if (bLineSelect) {
		if (!WCODE::IsLineDelimiter(szREPLACEKEY[nReplaceKey - 1], GetDllShareData().common.edit.bEnableExtEol)) {
			memClip.AppendString(GetDocument().docEditor.GetNewLineCode().GetValue2());
			szREPLACEKEY = memClip.GetStringPtr(&nReplaceKey);
		}
	}

	if (GetDllShareData().common.edit.bConvertEOLPaste) {
		size_t nConvertedTextLen = ConvertEol(szREPLACEKEY, nReplaceKey, NULL);
		std::vector<wchar_t> szConvertedText(nConvertedTextLen);
		wchar_t* pszConvertedText = &szConvertedText[0];
		ConvertEol(szREPLACEKEY, nReplaceKey, pszConvertedText);
		memClip.SetString(pszConvertedText, nConvertedTextLen);
		szREPLACEKEY = memClip.GetStringPtr(&nReplaceKey);
	}

	// �擾�ɃX�e�b�v�������肻���ȕϐ��Ȃǂ��A�ꎞ�ϐ�������B
	// �Ƃ͂����A�����̑�������邱�Ƃɂ���ē�������N���b�N���͍��킹�Ă� 1 ���[�v�Ő��\���Ǝv���܂��B
	// ���S�N���b�N�����[�v�̃I�[�_�[����l���Ă�����Ȃɓ��͂��Ȃ��悤�Ɏv���܂����ǁE�E�E�B
	bool& bCancel = dlgCancel.bCANCEL;

	// �N���X�֌W�����[�v�̒��Ő錾���Ă��܂��ƁA�����[�v���ƂɃR���X�g���N�^�A�f�X�g���N�^��
	// �Ă΂�Ēx���Ȃ�̂ŁA�����Ő錾�B
	Bregexp regexp;
	// �����������l�ɖ����[�v���Ƃɂ��ƒx���̂ŁA�ŏ��ɍς܂��Ă��܂��B
	if (bRegularExp && !bPaste) {
		if (!InitRegexp(view.GetHwnd(), regexp, true)) {
			view.SetDrawSwitch(bDrawSwitchOld);
			::EnableWindow(view.GetHwnd(), TRUE);
			::EnableWindow(::GetParent(view.GetHwnd()), TRUE);
			::EnableWindow(::GetParent(::GetParent(view.GetHwnd())), TRUE);
			return;
		}

		// Nov. 9, 2005 ����� ���K�\���őI���n�_�E�I�_�ւ̑}�����@��ύX(��)
		NativeW memRepKey2;
		NativeW memMatchStr;
		memMatchStr.SetString(L"$&");
		if (nReplaceTarget == 1) {	// �I���n�_�֑}��
			memRepKey2 = memClip;
			memRepKey2 += memMatchStr;
		}else if (nReplaceTarget == 2) { // �I���I�_�֑}��
			memRepKey2 = memMatchStr;
			memRepKey2 += memClip;
		}else {
			memRepKey2 = memClip;
		}
		// ���K�\���I�v�V�����̐ݒ�2006.04.01 �����
		int nFlag = (view.curSearchOption.bLoHiCase ? Bregexp::optCaseSensitive : Bregexp::optNothing);
		nFlag |= (bConsecutiveAll ? Bregexp::optNothing : Bregexp::optGlobal);	// 2007.01.16 ryoji
		regexp.Compile(view.strCurSearchKey.c_str(), memRepKey2.GetStringPtr(), nFlag);
	}

	//$$ �P�ʍ���
	Point ptOld;			// ������̑I��͈�
	size_t lineCnt = 0;		// �u���O�̍s��
	int	linDif = 0;			// �u����̍s����
	int	colDif = 0;			// �u����̌�����
	int	linPrev = 0;		// �O��̌����s(��`) @@@2001.12.31 YAZAKI warning�ގ�
	size_t linOldLen = 0;	// ������̍s�̒���
	int	linNext;			// ����̌����s(��`)

	int nLoopCnt = -1;

	// �e�L�X�g���I������Ă��邩
	while (
		(!bFastMode && view.GetSelectionInfo().IsTextSelected())
		|| (bFastMode && selectLogic.IsValid())
	) {
		// �L�����Z�����ꂽ��
		if (bCancel) {
			break;
		}

		// �������̃��[�U�[������\�ɂ���
		if (!::BlockingHook(hwndCancel)) {
			view.SetDrawSwitch(bDrawSwitchOld);
			::EnableWindow(view.GetHwnd(), TRUE);
			::EnableWindow(::GetParent(view.GetHwnd()), TRUE);
			::EnableWindow(::GetParent(::GetParent(view.GetHwnd())), TRUE);
			return;// -1;
		}

		++nLoopCnt;
		// 128 ���Ƃɕ\���B
		if ((nLoopCnt & 0x7F) == 0) {
			// ���Ԃ��Ƃɐi���󋵕`�悾�Ǝ��Ԏ擾���x���Ȃ�Ǝv�����A������̕������R���Ǝv���̂ŁE�E�E�B
			// �Ǝv�������ǁA�t�ɂ�����̕������R�ł͂Ȃ��̂ŁA��߂�B
		
			if (bFastMode) {
				int nDiff = (int)nAllLineNumOrg - (int)docLineMgr.GetLineCount();
				if (0 <= nDiff) {
					nNewPos = (nDiff + selectLogic.GetFrom().y) >> nShiftCount;
				}else {
					nNewPos = ::MulDiv(selectLogic.GetFrom().GetY(), (int)nAllLineNum, (int)layoutMgr.GetLineCount());
				}
			}else {
				int nDiff = (int)nAllLineNumOrg - (int)layoutMgr.GetLineCount();
				if (0 <= nDiff) {
					nNewPos = (nDiff + GetSelect().GetFrom().y) >> nShiftCount;
				}else {
					nNewPos = ::MulDiv(GetSelect().GetFrom().GetY(), (int)nAllLineNum, (int)layoutMgr.GetLineCount());
				}
			}
			if (nOldPos != nNewPos) {
				Progress_SetPos(hwndProgress, nNewPos +1);
				Progress_SetPos(hwndProgress, nNewPos);
				nOldPos = nNewPos;
			}
			_itot(nReplaceNum, szLabel, 10);
			::SendMessage(hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel);
		}

		// From Here 2001.12.03 hor
		// ������̈ʒu���m�F
		if (bSelectedArea) {
			// ��`�I��
			// o ���C�A�E�g���W���`�F�b�N���Ȃ���u������
			// o �܂�Ԃ�������ƕςɂȂ邩���E�E�E
			//
			if (bBeginBoxSelect) {
				// �������̍s�����L��
				lineCnt = layoutMgr.GetLineCount();
				// ������͈̔͏I�[
				ptOld = GetSelect().GetTo();
				// �O��̌����s�ƈႤ�H
				if (ptOld.y != linPrev) {
					colDif = 0;
				}
				linPrev = ptOld.y;
				// �s�͔͈͓��H
				if (
					(rangeA.GetTo().y + linDif == ptOld.y && rangeA.GetTo().x + colDif < ptOld.x)
					|| (rangeA.GetTo().y + linDif <  ptOld.y)
				) {
					break;
				}
				// ���͔͈͓��H
				if (!(rangeA.GetFrom().x <= GetSelect().GetFrom().x && ptOld.x <= rangeA.GetTo().x + colDif)) {
					if (ptOld.x < rangeA.GetTo().x + colDif) {
						linNext = GetSelect().GetTo().y;
					}else {
						linNext = GetSelect().GetTo().y + 1;
					}
					// ���̌����J�n�ʒu�փV�t�g
					caret.SetCaretLayoutPos(Point(rangeA.GetFrom().x, linNext));
					// 2004.05.30 Moca ���݂̌�����������g���Č�������
					Command_Search_Next(false, bDisplayUpdate, true, 0, nullptr);
					colDif = 0;
					continue;
				}
			}else {
				// ���ʂ̑I��
				// o �������W���`�F�b�N���Ȃ���u������
				//
			
				// �������̍s�����L��
				lineCnt = docLineMgr.GetLineCount();

				// ������͈̔͏I�[
				Point ptOldTmp;
				if (bFastMode) {
					ptOldTmp = selectLogic.GetTo();
				}else {
					ptOldTmp = layoutMgr.LayoutToLogic(GetSelect().GetTo());
				}
				ptOld.x = ptOldTmp.x; //$$ ���C�A�E�g�^�ɖ�����胍�W�b�N�^�����B�C��������
				ptOld.y = ptOldTmp.y;

				// �u���O�̍s�̒���(���s�͂P�����Ɛ�����)��ۑ����Ă����āA�u���O��ōs�ʒu���ς�����ꍇ�Ɏg�p
				linOldLen = docLineMgr.GetLine(ptOldTmp.y)->GetLengthWithoutEOL() + 1;

				// �s�͔͈͓��H
				// 2007.01.19 ryoji �����ǉ�: �I���I�_���s��(ptColLineP.x == 0)�ɂȂ��Ă���ꍇ�͑O�̍s�̍s���܂ł�I��͈͂Ƃ݂Ȃ�
				// �i�I���n�_���s���Ȃ炻�̍s���͑I��͈͂Ɋ܂݁A�I�_���s���Ȃ炻�̍s���͑I��͈͂Ɋ܂܂Ȃ��A�Ƃ���j
				// �_���I�ɏ����ςƎw�E����邩������Ȃ����A���p��͂��̂悤�ɂ����ق����]�܂����P�[�X�������Ǝv����B
				// ���s�I���ōs���܂ł�I��͈͂ɂ�������ł��AUI��͎��̍s�̍s���ɃJ�[�\�����s��
				// ���I�_�̍s�����u^�v�Ƀ}�b�`��������������P�����ȏ�I�����ĂˁA�Ƃ������ƂŁD�D�D
				// $$ �P�ʍ��݂��܂��肾���ǁA���v�H�H
				if (
					(
						ptColLineP.y + linDif == ptOld.y
						&& (
							ptColLineP.x + colDif < ptOld.x
							|| ptColLineP.x == 0
						)
					)
					|| ptColLineP.y + linDif < ptOld.y
				) {
					break;
				}
			}
		}

		Point ptTmp(0, 0);
		Point  ptTmpLogic(0, 0);

		if (bPaste || !bRegularExp) {
			// ���K�\������ ����Q��($&)�Ŏ�������̂ŁA���K�\���͏��O
			if (nReplaceTarget == 1) {	// �}���ʒu�Z�b�g
				if (bFastMode) {
					ptTmpLogic.x = selectLogic.GetTo().x - selectLogic.GetFrom().x;
					ptTmpLogic.y = selectLogic.GetTo().y - selectLogic.GetFrom().y;
					selectLogic.SetTo(selectLogic.GetFrom());
				}else {
					ptTmp.x = GetSelect().GetTo().x - GetSelect().GetFrom().x;
					ptTmp.y = GetSelect().GetTo().y - GetSelect().GetFrom().y;
					GetSelect().Clear(-1);
				}
			}else if (nReplaceTarget == 2) {	// �ǉ��ʒu�Z�b�g
				// ���K�\�������O�����̂ŁA�u������̕��������s������玟�̍s�̐擪�ֈړ��v�̏������폜
				if (bFastMode) {
					caret.MoveCursorFastMode(selectLogic.GetTo());
					selectLogic.SetFrom(selectLogic.GetTo());
				}else {
					caret.MoveCursor(GetSelect().GetTo(), false);
					GetSelect().Clear(-1);
				}
		    }else {
				// �ʒu�w��Ȃ��̂ŁA�������Ȃ�
			}
		}
		// �s�폜 �I��͈͂��s�S�̂Ɋg��B�J�[�\���ʒu���s����(���K�\���ł����s)
		if (nReplaceTarget == 3) {
			if (bFastMode) {
				const int y = selectLogic.GetFrom().y;
				selectLogic.SetFrom(Point(0, y)); // �s��
				selectLogic.SetTo(Point(0, y + 1)); // ���s�̍s��
				if (docLineMgr.GetLineCount() == y + 1) {
					const DocLine* pLine = docLineMgr.GetLine(y);
					if (pLine->GetEol() == EolType::None) {
						// EOF�͍ŏI�f�[�^�s�ɂԂ牺����Ȃ̂ŁA�I���I�[�͍s��
						selectLogic.SetTo(Point((int)pLine->GetLengthWithEOL(), y)); // �Ώۍs�̍s��
					}
				}
				caret.MoveCursorFastMode(selectLogic.GetFrom());
			}else {
				Point lineHome = layoutMgr.LayoutToLogic(GetSelect().GetFrom());
				lineHome.x = 0; // �s��
				Range selectFix;
				selectFix.SetFrom(layoutMgr.LogicToLayout(lineHome));
				lineHome.y++; // ���s�̍s��
				selectFix.SetTo(layoutMgr.LogicToLayout(lineHome));
				caret.GetAdjustCursorPos(&selectFix.GetTo());
				view.GetSelectionInfo().SetSelectArea(selectFix);
				caret.MoveCursor(selectFix.GetFrom(), false);
			}
		}


		// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
		// �e�L�X�g��\��t��
		if (bPaste) {
			if (!bColumnSelect) {
				/* �{���� Command_InsText ���g���ׂ��Ȃ�ł��傤���A���ʂȏ���������邽�߂ɒ��ڂ������B
				** ��nSelectXXX��-1�̎��� view.ReplaceData_CEditView�𒼐ڂ������Ɠ���s�ǂƂȂ邽��
				**   ���ڂ������̂�߂��B2003.05.18 by �����
				*/
				Command_InsText(false, szREPLACEKEY, nReplaceKey, true, bLineSelect);
			}else {
				Command_PasteBox(szREPLACEKEY, nReplaceKey);
				// 2013.06.11 �ĕ`�悵�Ȃ��悤��
				// �ĕ`����s��Ȃ��Ƃǂ�Ȍ��ʂ��N���Ă���̂������炸�݂��Ƃ��Ȃ��̂ŁE�E�E�B
				// view.AdjustScrollBars(); // 2007.07.22 ryoji
				// view.Redraw();
			}
			++nReplaceNum;
		}else if (nReplaceTarget == 3) {
			Command_InsText( false, L"", 0, true, false, bFastMode, bFastMode ? &selectLogic : nullptr );
			++nReplaceNum;
		}else if (bRegularExp) { // �����^�u��  1==���K�\��
			// 2002/01/19 novice ���K�\���ɂ�镶����u��
			// �����s�A�����s���A�����s�ł̌����}�b�`�ʒu
			const DocLine* pDocLine;
			const wchar_t* pLine;
			size_t nLogicLineNum;
			size_t nIdx;
			size_t nLen;
			if (bFastMode) {
				pDocLine = docLineMgr.GetLine(selectLogic.GetFrom().y);
				pLine = pDocLine->GetPtr();
				nLogicLineNum = selectLogic.GetFrom().y;
				nIdx = selectLogic.GetFrom().x;
				nLen = pDocLine->GetLengthWithEOL();
			}else {
				const Layout* pLayout = layoutMgr.SearchLineByLayoutY(GetSelect().GetFrom().y);
				pDocLine = pLayout->GetDocLineRef();
				pLine = pDocLine->GetPtr();
				nLogicLineNum = pLayout->GetLogicLineNo();
				nIdx = view.LineColumnToIndex(pLayout, GetSelect().GetFrom().x) + pLayout->GetLogicOffset();
				nLen = pDocLine->GetLengthWithEOL();
			}
			size_t colDiff = 0;
			if (!bConsecutiveAll) {	// �ꊇ�u��
				// 2007.01.16 ryoji
				// �I��͈͒u���̏ꍇ�͍s���̑I��͈͖����܂Œu���͈͂��k�߁C
				// ���̈ʒu���L������D
				if (bSelectedArea) {
					if (bBeginBoxSelect) {	// ��`�I��
						Point ptWork = layoutMgr.LayoutToLogic(Point(rangeA.GetTo().x, ptOld.y));
						ptColLineP.x = ptWork.x;
						ASSERT_GE(nLen, pDocLine->GetEol().GetLen());
						if ((int)(nLen - pDocLine->GetEol().GetLen()) > ptColLineP.x + colDif) {
							nLen = ptColLineP.x + colDif;
						}
					}else {	// �ʏ�̑I��
						if (ptColLineP.y + linDif == ptOld.y) { //$$ �P�ʍ���
							ASSERT_GE(nLen, pDocLine->GetEol().GetLen());
							if ((int)(nLen - pDocLine->GetEol().GetLen()) > ptColLineP.x + colDif) {
								nLen = ptColLineP.x + colDif;
							}
						}
					}
				}

				if (pDocLine->GetLengthWithoutEOL() < nLen) {
					ptOld.x = (int)pDocLine->GetLengthWithoutEOL() + 1; //$$ �P�ʍ���
				}else {
					ptOld.x = (int)nLen; //$$ �P�ʍ���
				}
			}

			if (int nReplace = regexp.Replace(pLine, nLen, nIdx)) {
				nReplaceNum += nReplace;
				if (!bConsecutiveAll) { // 2006.04.01 �����	// 2007.01.16 ryoji
					// �s�P�ʂł̒u������
					// �I��͈͂𕨗��s���܂łɂ̂΂�
					if (bFastMode) {
						selectLogic.SetTo(Point((int)nLen, (int)nLogicLineNum));
					}else {
						GetSelect().SetTo(layoutMgr.LogicToLayout(Point((int)nLen, (int)nLogicLineNum)));
					}
				}else {
				    // From Here Jun. 6, 2005 �����
				    // �����s���܂�INSTEXT������@�́A�L�����b�g�ʒu�𒲐�����K�v������A
				    // �L�����b�g�ʒu�̌v�Z�����G�ɂȂ�B�i�u����ɉ��s������ꍇ�ɕs������j
				    // �����ŁAINSTEXT���镶���񒷂𒲐�������@�ɕύX����i���͂������̕����킩��₷���j
				    size_t matchLen = regexp.GetMatchLen();
				    size_t nIdxTo = nIdx + matchLen;		// ����������̖���
				    if (matchLen == 0) {
					    // �O�����}�b�`�̎�(�����u���ɂȂ�Ȃ��悤�ɂP�����i�߂�)
					    if (nIdxTo < nLen) {
						    // 2005-09-02 D.S.Koba GetSizeOfChar
						    nIdxTo += NativeW::GetSizeOfChar(pLine, nLen, nIdxTo) == 2 ? 2 : 1;
					    }
					    // �����u�����Ȃ��悤�ɁA�P�������₵���̂łP�����I���ɕύX
					    // �I���n�_�E�I�_�ւ̑}���̏ꍇ���O�����}�b�`���͓���͓����ɂȂ�̂�
						if (bFastMode) {
							selectLogic.SetTo(Point((int)nIdxTo, (int)nLogicLineNum));
						}else {
							// 2007.01.19 ryoji �s�ʒu���擾����
							GetSelect().SetTo(layoutMgr.LogicToLayout(Point((int)nIdxTo, (int)nLogicLineNum)));
						}
				    }
				    // �s�����猟�������񖖔��܂ł̕�����
					ASSERT_GE(nLen, nIdxTo);
					colDiff = nLen - nIdxTo;
					ptOld.x = (int)nIdxTo;	// 2007.01.19 ryoji �ǉ�  // $$ �P�ʍ���
				    // Oct. 22, 2005 Karoto
				    // \r��u������Ƃ��̌���\n�������Ă��܂����̑Ή�
				    if (colDiff < pDocLine->GetEol().GetLen()) {
					    // ���s�ɂ������Ă�����A�s�S�̂�INSTEXT����B
					    colDiff = 0;
						if (bFastMode) {
							selectLogic.SetTo(Point((int)nLen, (int)nLogicLineNum));
						}else {
							// 2007.01.19 ryoji �ǉ�
							GetSelect().SetTo(layoutMgr.LogicToLayout(Point((int)nLen, (int)nLogicLineNum)));
						}
						ptOld.x = (int)pDocLine->GetLengthWithoutEOL() + 1;	// 2007.01.19 ryoji �ǉ� //$$ �P�ʍ���
				    }
				}
				// �u���㕶����ւ̏�������(�s�����猟�������񖖔��܂ł̕���������)
				Command_InsText(false, regexp.GetString(), regexp.GetStringLen() - colDiff, true, false, bFastMode, bFastMode ? &selectLogic : nullptr);
				// To Here Jun. 6, 2005 �����
			}
		}else {
			/* �{���͌��R�[�h���g���ׂ��Ȃ�ł��傤���A���ʂȏ���������邽�߂ɒ��ڂ������B
			** ��nSelectXXX��-1�̎��� view.ReplaceData_CEditView�𒼐ڂ������Ɠ���s�ǂƂȂ邽�ߒ��ڂ������̂�߂��B2003.05.18 �����
			*/
			Command_InsText(false, szREPLACEKEY, nReplaceKey, true, false, bFastMode, bFastMode ? &selectLogic : nullptr);
			++nReplaceNum;
		}

		// �}����̈ʒu����
		if (nReplaceTarget == 1) {
			if (bFastMode) {
				caret.SetCaretLogicPos(caret.GetCaretLogicPos() + ptTmpLogic);
			}else {
				caret.SetCaretLayoutPos(caret.GetCaretLayoutPos() + ptTmp);
				if (!bBeginBoxSelect) {
					Point p = layoutMgr.LayoutToLogic(caret.GetCaretLayoutPos());
					caret.SetCaretLogicPos(p);
				}
			}
		}

		if (!bFastMode && 50 <= nReplaceNum && !(bSelectedArea || bPaste)) {
			bFastMode = true;
			nAllLineNum = docLineMgr.GetLineCount();
			nAllLineNumOrg = nAllLineNumLogicOrg;
			for (nShiftCount=0; 300<nAllLineNum; ++nShiftCount) {
				nAllLineNum /= 2;
			}
			Progress_SetRange( hwndProgress, 0, nAllLineNum + 1 );
			int nDiff = (int)nAllLineNumOrg - (int)docLineMgr.GetLineCount();
			if (0 <= nDiff) {
				nNewPos = (nDiff + selectLogic.GetFrom().y) >> nShiftCount;
			}else {
				nNewPos = ::MulDiv(selectLogic.GetFrom().GetY(), (int)nAllLineNum, (int)docLineMgr.GetLineCount());
			}
			Progress_SetPos( hwndProgress, nNewPos +1 );
			Progress_SetPos( hwndProgress, nNewPos );
		}
		// �Ō�ɒu�������ʒu���L��
		if (bFastMode) {
			ptLastLogic = caret.GetCaretLogicPos();
		}else {
			ptLast = caret.GetCaretLayoutPos();
		}

		// �u����̈ʒu���m�F
		if (bSelectedArea) {
			// �������u���̍s�␳�l�擾
			if (bBeginBoxSelect) {
				colDif += ptLast.x - ptOld.x;
				linDif += (int)layoutMgr.GetLineCount() - (int)lineCnt;
			}else {
				// �u���O�̌���������̍ŏI�ʒu�� ptOld
				// �u����̃J�[�\���ʒu
				Point ptTmp2 = caret.GetCaretLogicPos();
				int linDif_thistime = (int)docLineMgr.GetLineCount() - (int)lineCnt;	// ����u���ł̍s���ω�
				linDif += linDif_thistime;
				if (ptColLineP.y + linDif == ptTmp2.y) {
					// �ŏI�s�Œu���������A���́A�u���̌��ʁA�I���G���A�ŏI�s�܂œ��B������
					// �ŏI�s�Ȃ̂ŁA�u���O��̕������̑����Ō��ʒu�𒲐�����
					colDif += ptTmp2.x - ptOld.x; //$$ �P�ʍ���

					// �A���A�ȉ��̏ꍇ�͒u���O��ōs���قȂ��Ă��܂��̂ŁA�s�̒����ŕ␳����K�v������
					// �P�j�ŏI�s���O�ōs�A�����N����A�s�������Ă���ꍇ�i�s�A���Ȃ̂ŁA���ʒu�͒u����̃J�[�\�����ʒu����������j
					// �@�@ptTmp2.x-ptOld.x���ƁA\r\n �� "" �u���ōs�A�������ꍇ�ɁA���ʒu�����ɂȂ莸�s����i���Ƃ͑O�s�̌��̕��ɂȂ邱�ƂȂ̂ŕ␳����j
					// �@�@����u���ł̍s���̕ω�(linDif_thistime)�ŁA�ŏI�s���s�A�����ꂽ���ǂ��������邱�Ƃɂ���
					// �Q�j���s��u�������iptTmp2.y != ptOld.y�j�ꍇ�A���s��u������ƒu����̌��ʒu�����s�̌��ʒu�ɂȂ��Ă��邽��
					//     ptTmp2.x-ptOld.x���ƁA���̐��ƂȂ�A\r\n �� \n �� \n �� "abc" �ȂǂŌ��ʒu�������
					//     ������O�s�̒����Œ�������K�v������
					if (linDif_thistime < 0 || ptTmp2.y != ptOld.y) { //$$ �P�ʍ���
						colDif += (int)linOldLen;
					}
				}
			}
		}
		// To Here 2001.12.03 hor

		// ��������
		// 2004.05.30 Moca ���݂̌�����������g���Č�������
		Command_Search_Next(false, bDisplayUpdate, true, 0, NULL, bFastMode ? &selectLogic : nullptr);
	}

	if (bFastMode) {
		if (0 < nReplaceNum) {
			// LayoutMgr�̍X�V(�ύX�L�̏ꍇ)
			layoutMgr._DoLayout();
			GetEditWindow().ClearViewCaretPosInfo();
			if (GetDocument().nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
				layoutMgr.CalculateTextWidth();
			}
		}
		ptLast = layoutMgr.LogicToLayout(ptLastLogic);
		caret.MoveCursor(ptLast, true);
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;	// 2009.07.25 ryoji
	}
	//>> 2002/03/26 Azumaiya

	_itot(nReplaceNum, szLabel, 10);
	::SendMessage(hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel);

	if (!dlgCancel.IsCanceled()) {
		nNewPos = (int)nAllLineNum;
		Progress_SetPos(hwndProgress, nNewPos + 1);
		Progress_SetPos(hwndProgress, nNewPos);
	}
	dlgCancel.CloseDialog(0);
	::EnableWindow(view.GetHwnd(), TRUE);
	::EnableWindow(::GetParent(view.GetHwnd()), TRUE);
	::EnableWindow(::GetParent(::GetParent(view.GetHwnd())), TRUE);

	// From Here 2001.12.03 hor

	// �e�L�X�g�I������
	view.GetSelectionInfo().DisableSelectArea(false);

	// �J�[�\���E�I��͈͕���
	if (
		!bSelectedArea			// �t�@�C���S�̒u��
		|| dlgCancel.IsCanceled()
	) {		// �L�����Z�����ꂽ
		// �Ō�ɒu������������̉E��
		if (!bFastMode) {
			caret.MoveCursor(ptLast, true);
		}
	}else {
		if (bBeginBoxSelect) {
			// ��`�I��
			view.GetSelectionInfo().SetBoxSelect(bBeginBoxSelect);
			rangeA.GetTo().y += linDif;
			if (rangeA.GetTo().y < 0) rangeA.SetToY(0);
		}else {
			// ���ʂ̑I��
			ptColLineP.x += colDif;
			if (ptColLineP.x < 0) ptColLineP.x = 0;
			ptColLineP.y += linDif;
			if (ptColLineP.y < 0) ptColLineP.y = 0;
			rangeA.SetTo(layoutMgr.LogicToLayout(ptColLineP));
		}
		if (rangeA.GetFrom().y<rangeA.GetTo().y || rangeA.GetFrom().x<rangeA.GetTo().x) {
			view.GetSelectionInfo().SetSelectArea(rangeA);	// 2009.07.25 ryoji
		}
		caret.MoveCursor(rangeA.GetTo(), true);
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;	// 2009.07.25 ryoji
	}
	// To Here 2001.12.03 hor

	dlgReplace.bCanceled = dlgCancel.IsCanceled();
	dlgReplace.nReplaceCnt = nReplaceNum;
	view.SetDrawSwitch(bDrawSwitchOld);
	ActivateFrameWindow(GetMainWindow());
}


// �����}�[�N�̐ؑւ�	// 2001.12.03 hor �N���A �� �ؑւ� �ɕύX
void ViewCommander::Command_Search_ClearMark(void)
{
// From Here 2001.12.03 hor

	// �����}�[�N�̃Z�b�g

	if (view.GetSelectionInfo().IsTextSelected()) {

		// ����������擾
		NativeW	memCurText;
		view.GetCurrentTextForSearch(memCurText, false);
		auto& csSearch = GetDllShareData().common.search;

		view.strCurSearchKey = memCurText.GetStringPtr();
		if (view.nCurSearchKeySequence < csSearch.nSearchKeySequence) {
			view.curSearchOption = csSearch.searchOption;
		}
		view.curSearchOption.bRegularExp = false;	// ���K�\���g��Ȃ�
		view.curSearchOption.bWordOnly = false;		// �P��Ō������Ȃ�

		// ���L�f�[�^�֓o�^
		if (memCurText.GetStringLength() < _MAX_PATH) {
			SearchKeywordManager().AddToSearchKeys(memCurText.GetStringPtr());
			csSearch.searchOption = view.curSearchOption;
		}
		view.nCurSearchKeySequence = csSearch.nSearchKeySequence;
		view.bCurSearchUpdate = true;

		view.ChangeCurRegexp(false); // 2002.11.11 Moca ���K�\���Ō���������C�F�������ł��Ă��Ȃ�����

		// �ĕ`��
		view.RedrawAll();
		return;
	}
// To Here 2001.12.03 hor

	// �����}�[�N�̃N���A

	view.bCurSrchKeyMark = false;	// ����������̃}�[�N
	// �t�H�[�J�X�ړ����̍ĕ`��
	view.RedrawAll();
	return;
}

// Jun. 16, 2000 genta
// �Ί��ʂ̌���
void ViewCommander::Command_BracketPair(void)
{
	Point ptColLine;
	//int nLine, nCol;

	int mode = 3;
	/*
	bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ�
	bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ�
	bit2(out) : ���������ʒu         0:���      1:�O
	*/
	if (view.SearchBracket(GetCaret().GetCaretLayoutPos(), &ptColLine, &mode)) {	// 02/09/18 ai
		// 2005.06.24 Moca
		// 2006.07.09 genta �\���X�V�R��F�V�K�֐��ɂđΉ�
		view.MoveCursorSelecting(ptColLine, view.GetSelectionInfo().bSelectingLock);
	}else {
		// ���s�����ꍇ�� nCol/nLine�ɂ͗L���Ȓl�������Ă��Ȃ�.
		// �������Ȃ�
	}
}

