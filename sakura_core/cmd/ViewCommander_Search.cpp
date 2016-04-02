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
void ViewCommander::Command_SEARCH_BOX(void)
{
	GetEditWindow().m_toolbar.SetFocusSearchBox();
}


// ����(�P�ꌟ���_�C�A���O)
void ViewCommander::Command_SEARCH_DIALOG(void)
{
	// ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾
	NativeW memCurText;
	m_view.GetCurrentTextForSearchDlg(memCurText);	// 2006.08.23 ryoji �_�C�A���O��p�֐��ɕύX

	auto& dlgFind = GetEditWindow().m_dlgFind;
	// �����������������
	if (0 < memCurText.GetStringLength()) {
		dlgFind.m_strText = memCurText.GetStringPtr();
	}
	// �����_�C�A���O�̕\��
	if (!dlgFind.GetHwnd()) {
		dlgFind.DoModeless(G_AppInstance(), m_view.GetHwnd(), (LPARAM)&GetEditWindow().GetActiveView());
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
void ViewCommander::Command_SEARCH_NEXT(
	bool			bChangeCurRegexp,
	bool			bRedraw,
	bool			bReplaceAll,
	HWND			hwndParent,
	const WCHAR*	pszNotFoundMessage,
	LogicRange*		pSelectLogic		// [out] �I��͈͂̃��W�b�N�ŁB�}�b�`�͈͂�Ԃ��B���ׂĒu��/�������[�h�Ŏg�p
	)
{
	bool		bSelecting;
	bool		bFlag1 = false;
	bool		bSelectingLock_Old = false;
	bool		bFound = false;
	bool		bDisableSelect = false;
	bool		b0Match = false;		// �����O�Ń}�b�`���Ă��邩�H�t���O by �����
	LogicInt	nIdx(0);
	LayoutInt	nLineNum(0);

	LayoutRange	rangeA;
	rangeA.Set(GetCaret().GetCaretLayoutPos());

	LayoutRange	selectBgn_Old;
	LayoutRange	select_Old;
	LayoutInt	nLineNumOld(0);

	// bFastMode
	LogicInt nLineNumLogic(0);

	bool	bRedo = false;	// hor
	int		nIdxOld = 0;	// hor
	int		nSearchResult;
	auto& layoutMgr = GetDocument().m_layoutMgr;

	if (pSelectLogic) {
		pSelectLogic->Clear(-1);
	}

	bSelecting = false;
	// 2002.01.16 hor
	// ���ʕ����̂����肾��
	// 2004.05.30 Moca CEditView�̌��ݐݒ肳��Ă��錟���p�^�[�����g����悤��
	if (bChangeCurRegexp && !m_view.ChangeCurRegexp()) {
		return;
	}
	if (m_view.m_strCurSearchKey.size() == 0) {
		goto end_of_func;
	}

	auto& si = m_view.GetSelectionInfo();
	auto& caret = GetCaret();
	// �����J�n�ʒu�𒲐�
	bFlag1 = false;
	if (!pSelectLogic && si.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// ��`�͈͑I�𒆂łȂ� & �I����Ԃ̃��b�N
		if (!si.IsBoxSelecting() && si.m_bSelectingLock) {
			bSelecting = true;
			bSelectingLock_Old = si.m_bSelectingLock;

			selectBgn_Old = si.m_selectBgn; // �͈͑I��(���_)
			select_Old = GetSelect();

			if (PointCompare(si.m_selectBgn.GetFrom(), caret.GetCaretLayoutPos()) >= 0) {
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
		nLineNum = caret.GetCaretLayoutPos().GetY2();
		LogicInt nLineLen = LogicInt(0); // 2004.03.17 Moca pLine == NULL �̂Ƃ��AnLineLen�����ݒ�ɂȂ藎����o�O�΍�
		const Layout*	pLayout;
		const wchar_t*	pLine = layoutMgr.GetLineStr(nLineNum, &nLineLen, &pLayout);

		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
		// 2002.02.08 hor EOF�݂̂̍s��������������Ă��Č����\�� (2/2)
		nIdx = pLayout ? m_view.LineColumnToIndex(pLayout, caret.GetCaretLayoutPos().GetX2()) : LogicInt(0);
		if (b0Match) {
			// ���݁A�����O�Ń}�b�`���Ă���ꍇ�͕����s�łP�����i�߂�(�����}�b�`�΍�)
			if (nIdx < nLineLen) {
				// 2005-09-02 D.S.Koba GetSizeOfChar
				nIdx += LogicInt(NativeW::GetSizeOfChar(pLine, nLineLen, nIdx) == 2 ? 2 : 1);
			}else {
				// �O�̂��ߍs���͕ʏ���
				++nIdx;
			}
		}
	}else {
		nLineNumLogic = caret.GetCaretLogicPos().GetY2();
		nIdx = caret.GetCaretLogicPos().GetX2();
	}

	nLineNumOld = nLineNum;	// hor
	bRedo		= true;		// hor
	nIdxOld		= nIdx;		// hor

re_do:;
	// ���݈ʒu�����̈ʒu����������
	// 2004.05.30 Moca ������GetShareData()���烁���o�ϐ��ɕύX�B���̃v���Z�X/�X���b�h�ɏ����������Ă��܂�Ȃ��悤�ɁB
	if (!pSelectLogic) {
		nSearchResult = layoutMgr.SearchWord(
			nLineNum,						// �����J�n���C�A�E�g�s
			nIdx,							// �����J�n�f�[�^�ʒu
			SearchDirection::Forward,		// 0==�O������ 1==�������
			&rangeA,						// �}�b�`���C�A�E�g�͈�
			m_view.m_searchPattern
		);
	}else {
		auto& docLineMgr = GetDocument().m_docLineMgr;
		nSearchResult = SearchAgent(docLineMgr).SearchWord(
			LogicPoint(nIdx, nLineNumLogic),
			SearchDirection::Forward,		// 0==�O������ 1==�������
			pSelectLogic,
			m_view.m_searchPattern
		);
	}
	if (nSearchResult) {
		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
		if (bFlag1 && rangeA.GetFrom() == caret.GetCaretLayoutPos()) {
			LogicRange logicRange;
			layoutMgr.LayoutToLogic(rangeA, &logicRange);

			nLineNum = rangeA.GetTo().GetY2();
			nIdx     = logicRange.GetTo().GetX2();
			if (logicRange.GetFrom() == logicRange.GetTo()) { // ��0�}�b�`�ł̖������[�v�΍�B
				nIdx += 1; // wchar_t����i�߂邾���ł͑���Ȃ���������Ȃ����B
			}
			goto re_do;
		}

		if (bSelecting) {
			// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
			si.ChangeSelectAreaByCurrentCursor(rangeA.GetTo());
			si.m_bSelectingLock = bSelectingLock_Old;	// �I����Ԃ̃��b�N
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
		if (!bReplaceAll) m_view.AddCurrentLineToHistory();	// 2002.02.16 hor ���ׂĒu���̂Ƃ��͕s�v
		if (!pSelectLogic) {
			caret.MoveCursor(rangeA.GetFrom(), bRedraw);
			caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
		}else {
			caret.MoveCursorFastMode(pSelectLogic->GetFrom());
		}
		bFound = true;
	}else {
		if (bSelecting) {
			si.m_bSelectingLock = bSelectingLock_Old;	// �I����Ԃ̃��b�N

			// �I��͈͂̕ύX
			si.m_selectBgn = selectBgn_Old; // �͈͑I��(���_)
			si.m_selectOld = select_Old;	// 2011.12.24
			GetSelect().SetFrom(select_Old.GetFrom());
			GetSelect().SetTo(rangeA.GetFrom());

			// �J�[�\���ړ�
			caret.MoveCursor(rangeA.GetFrom(), bRedraw);
			caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();

			if (bRedraw) {
				// �I��̈�`��
				si.DrawSelectArea();
			}
		}else {
			if (bDisableSelect) {
				// 2011.12.21 ���W�b�N�J�[�\���ʒu�̏C��/�J�[�\�����E�Ί��ʂ̕\��
				LogicPoint ptLogic;
				layoutMgr.LayoutToLogic(caret.GetCaretLayoutPos(), &ptLogic);
				caret.SetCaretLogicPos(ptLogic);
				m_view.DrawBracketCursorLine(bRedraw);
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
			nLineNum	= LayoutInt(0);
			nIdx		= LogicInt(0);
			bRedo		= false;
			goto re_do;		// �擪����Č���
		}
	}

	if (bFound) {
		if (!pSelectLogic && ((nLineNumOld > nLineNum)||(nLineNumOld == nLineNum && nIdxOld > nIdx)))
			m_view.SendStatusMessage(LS(STR_ERR_SRNEXT1));
	}else {
		caret.ShowEditCaret();	// 2002/04/18 YAZAKI
		caret.ShowCaretPosInfo();	// 2002/04/18 YAZAKI
		if (!bReplaceAll) {
			m_view.SendStatusMessage(LS(STR_ERR_SRNEXT2));
		}
// To Here 2002.01.26 hor

		// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
		if (!pszNotFoundMessage) {
			NativeW keyName;
			auto& curSearchKey = m_view.m_strCurSearchKey;
			LimitStringLengthW(curSearchKey.c_str(), curSearchKey.size(), _MAX_PATH, keyName);
			if ((size_t)keyName.GetStringLength() < curSearchKey.size()) {
				keyName.AppendString(L"...");
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
void ViewCommander::Command_SEARCH_PREV(bool bReDraw, HWND hwndParent)
{
	bool		bSelectingLock_Old = false;
	bool		bFound = false;
	bool		bRedo = false;			// hor
	bool		bDisableSelect = false;
	LayoutInt	nLineNumOld(0);
	LogicInt	nIdxOld(0);
	LayoutInt	nLineNum(0);
	LogicInt	nIdx(0);

	auto& caret = GetCaret();
	auto& layoutMgr = GetDocument().m_layoutMgr;

	LayoutRange rangeA;
	rangeA.Set(caret.GetCaretLayoutPos());

	LayoutRange selectBgn_Old;
	LayoutRange select_Old;

	bool bSelecting = false;
	// 2002.01.16 hor
	// ���ʕ����̂����肾��
	if (!m_view.ChangeCurRegexp()) {
		return;
	}
	if (m_view.m_strCurSearchKey.size() == 0) {
		goto end_of_func;
	}
	auto& si = m_view.GetSelectionInfo();
	if (si.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		selectBgn_Old = si.m_selectBgn; // �͈͑I��(���_)
		select_Old = GetSelect();
		
		bSelectingLock_Old = si.m_bSelectingLock;

		// ��`�͈͑I�𒆂�
		if (!si.IsBoxSelecting() && si.m_bSelectingLock) {	// �I����Ԃ̃��b�N
			bSelecting = true;
		}else {
			// ���݂̑I��͈͂��I����Ԃɖ߂�
			si.DisableSelectArea(bReDraw, false);
			bDisableSelect = true;
		}
	}

	nLineNum = caret.GetCaretLayoutPos().GetY2();
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
		Layout* pLayout = layoutMgr.SearchLineByLayoutY(nLineNum);
		nIdx = LogicInt(pLayout->GetDocLineRef()->GetLengthWithEOL() + 1);		// �s���̃k������(\0)�Ƀ}�b�`�����邽�߂�+1 2003.05.16 �����
	}else {
		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
		nIdx = m_view.LineColumnToIndex(pLayout, caret.GetCaretLayoutPos().GetX2());
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
			m_view.m_searchPattern
		)
	) {
		if (bSelecting) {
			// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
			si.ChangeSelectAreaByCurrentCursor(rangeA.GetFrom());
			si.m_bSelectingLock = bSelectingLock_Old;	// �I����Ԃ̃��b�N
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
		m_view.AddCurrentLineToHistory();
		caret.MoveCursor(rangeA.GetFrom(), bReDraw);
		caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
		bFound = true;
	}else {
		if (bSelecting) {
			si.m_bSelectingLock = bSelectingLock_Old;	// �I����Ԃ̃��b�N
			// �I��͈͂̕ύX
			si.m_selectBgn = selectBgn_Old;
			GetSelect() = select_Old;

			// �J�[�\���ړ�
			caret.MoveCursor(rangeA.GetFrom(), bReDraw);
			caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
			// �I��̈�`��
			si.DrawSelectArea();
		}else {
			if (bDisableSelect) {
				m_view.DrawBracketCursorLine(bReDraw);
			}
		}
	}
end_of_func:;
// From Here 2002.01.26 hor �擪�i�����j����Č���
	if (GetDllShareData().common.search.bSearchAll) {
		if (!bFound	&&	// ������Ȃ�����
			bRedo		// �ŏ��̌���
		) {
			nLineNum	= layoutMgr.GetLineCount() - LayoutInt(1);
			nIdx		= LogicInt(MAXLINEKETAS);
			bRedo		= false;
			goto re_do;	// ��������Č���
		}
	}
	if (bFound) {
		if ((nLineNumOld < nLineNum)||(nLineNumOld == nLineNum && nIdxOld < nIdx))
			m_view.SendStatusMessage(LS(STR_ERR_SRPREV1));
	}else {
		m_view.SendStatusMessage(LS(STR_ERR_SRPREV2));
// To Here 2002.01.26 hor

		// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
		NativeW keyName;
		auto& curSearchKey = m_view.m_strCurSearchKey;
		LimitStringLengthW(curSearchKey.c_str(), curSearchKey.size(), _MAX_PATH, keyName);
		if ((size_t)keyName.GetStringLength() < curSearchKey.size()) {
			keyName.AppendString(L"...");
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
void ViewCommander::Command_REPLACE_DIALOG(void)
{
	bool bSelected = false;

	// ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾
	NativeW memCurText;
	m_view.GetCurrentTextForSearchDlg(memCurText);	// 2006.08.23 ryoji �_�C�A���O��p�֐��ɕύX

	auto& dlgReplace = GetEditWindow().m_dlgReplace;

	// �����������������
	if (0 < memCurText.GetStringLength()) {
		dlgReplace.m_strText = memCurText.GetStringPtr();
	}
	if (0 < GetDllShareData().searchKeywords.replaceKeys.size()) {
		if (dlgReplace.nReplaceKeySequence < GetDllShareData().common.search.nReplaceKeySequence) {
			dlgReplace.m_strText2 = GetDllShareData().searchKeywords.replaceKeys[0];	// 2006.08.23 ryoji �O��̒u���㕶����������p��
		}
	}
	
	if (m_view.GetSelectionInfo().IsTextSelected() && !GetSelect().IsLineOne()) {
		bSelected = true;	// �I��͈͂��`�F�b�N���ă_�C�A���O�\��
	}else {
		bSelected = false;	// �t�@�C���S�̂��`�F�b�N���ă_�C�A���O�\��
	}
	// �u���I�v�V�����̏�����
	dlgReplace.m_nReplaceTarget = 0;	// �u���Ώ�
	dlgReplace.m_bPaste = false;		// �\��t����H
// To Here 2001.12.03 hor

	// �u���_�C�A���O�̕\��
	// From Here Jul. 2, 2001 genta �u���E�B���h�E��2�d�J����}�~
	if (!::IsWindow(dlgReplace.GetHwnd())) {
		dlgReplace.DoModeless(G_AppInstance(), m_view.GetHwnd(), (LPARAM)&m_view, bSelected);
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
	@date 2011.12.18 Moca �I�v�V�����E�����L�[��DllShareData����m_dlgReplace/EditView�x�[�X�ɕύX�B�����񒷐����̓P�p
*/
void ViewCommander::Command_REPLACE(HWND hwndParent)
{
	if (!hwndParent) {	// �e�E�B���h�E���w�肳��Ă��Ȃ���΁ACEditView���e�B
		hwndParent = m_view.GetHwnd();
	}
	auto& dlgReplace = GetEditWindow().m_dlgReplace;
	// 2002.02.10 hor
	int nPaste			=	dlgReplace.m_bPaste;
	int nReplaceTarget	=	dlgReplace.m_nReplaceTarget;

	if (nPaste && nReplaceTarget == 3) {
		// �u���ΏہF�s�폜�̂Ƃ��́A�N���b�v�{�[�h����\��t���𖳌��ɂ���
		nPaste = FALSE;
	}

	// From Here 2001.12.03 hor
	if (nPaste && !GetDocument().m_docEditor.IsEnablePaste()) {
		OkMessage(hwndParent, LS(STR_ERR_CEDITVIEW_CMD10));
		dlgReplace.CheckButton(IDC_CHK_PASTE, false);
		dlgReplace.EnableItem(IDC_COMBO_TEXT2, true);
		return;	// ���sreturn;
	}

	// 2002.01.09 hor
	// �I���G���A������΁A���̐擪�ɃJ�[�\�����ڂ�
	if (m_view.GetSelectionInfo().IsTextSelected()) {
		if (m_view.GetSelectionInfo().IsBoxSelecting()) {
			GetCaret().MoveCursor(GetSelect().GetFrom(), true);
		}else {
			Command_LEFT(false, false);
		}
	}
	// To Here 2002.01.09 hor
	
	// ��`�I���H
//			bBeginBoxSelect = m_view.GetSelectionInfo().IsBoxSelecting();

	// �J�[�\�����ړ�
	//HandleCommand(F_LEFT, true, 0, 0, 0, 0);	//�H�H�H
	// To Here 2001.12.03 hor

	// �e�L�X�g�I������
	// ���݂̑I��͈͂��I����Ԃɖ߂�
	m_view.GetSelectionInfo().DisableSelectArea(true);

	// 2004.06.01 Moca �������ɁA���̃v���Z�X�ɂ����replaceKeys�������������Ă����v�Ȃ悤��
	const NativeW memRepKey(dlgReplace.m_strText2.c_str());

	// ��������
	Command_SEARCH_NEXT(true, true, false, hwndParent, nullptr);

	bool	bRegularExp = m_view.m_curSearchOption.bRegularExp;
	int 	nFlag       = m_view.m_curSearchOption.bLoHiCase ? 0x01 : 0x00;
	auto& caret = GetCaret();

	// �e�L�X�g���I������Ă��邩
	if (m_view.GetSelectionInfo().IsTextSelected()) {
		// From Here 2001.12.03 hor
		LayoutPoint ptTmp(0, 0);
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
		auto& layoutMgr = GetDocument().m_layoutMgr;
		// �s�폜 �I��͈͂��s�S�̂Ɋg��B�J�[�\���ʒu���s����(���K�\���ł����s)
		if (nReplaceTarget == 3) {
			LogicPoint lineHome;
			layoutMgr.LayoutToLogic(GetSelect().GetFrom(), &lineHome);
			lineHome.x = LogicXInt(0); // �s��
			LayoutRange selectFix;
			layoutMgr.LogicToLayout(lineHome, selectFix.GetFromPointer());
			lineHome.y++; // ���s�̍s��
			layoutMgr.LogicToLayout(lineHome, selectFix.GetToPointer());
			caret.GetAdjustCursorPos(selectFix.GetToPointer());
			m_view.GetSelectionInfo().SetSelectArea(selectFix);
			m_view.GetSelectionInfo().DrawSelectArea();
			caret.MoveCursor(selectFix.GetFrom(), false);
			caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
		}
		// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
		// �e�L�X�g��\��t��
		if (nPaste) {
			Command_PASTE(0);
		} else if (nReplaceTarget == 3) {
			// �s�폜
			Command_INSTEXT( false, L"", LogicInt(0), true );
		}else if (bRegularExp) { // �����^�u��  1==���K�\��
			// ��ǂ݂ɑΉ����邽�߂ɕ����s���܂ł��g���悤�ɕύX 2005/03/27 �����
			// 2002/01/19 novice ���K�\���ɂ�镶����u��
			Bregexp regexp;

			if (!InitRegexp(m_view.GetHwnd(), regexp, true)) {
				return;	// ���sreturn;
			}

			// �����s�A�����s���A�����s�ł̌����}�b�`�ʒu
			const Layout* pLayout = layoutMgr.SearchLineByLayoutY(GetSelect().GetFrom().GetY2());
			const wchar_t* pLine = pLayout->GetDocLineRef()->GetPtr();
			LogicInt nIdx = m_view.LineColumnToIndex(pLayout, GetSelect().GetFrom().GetX2()) + pLayout->GetLogicOffset();
			LogicInt nLen = pLayout->GetDocLineRef()->GetLengthWithEOL();
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
			regexp.Compile(m_view.m_strCurSearchKey.c_str(), memRepKey2.GetStringPtr(), nFlag);
			if (regexp.Replace(pLine, nLen, nIdx)) {
				// From Here Jun. 6, 2005 �����
				// �����s���܂�INSTEXT������@�́A�L�����b�g�ʒu�𒲐�����K�v������A
				// �L�����b�g�ʒu�̌v�Z�����G�ɂȂ�B�i�u����ɉ��s������ꍇ�ɕs������j
				// �����ŁAINSTEXT���镶���񒷂𒲐�������@�ɕύX����i���͂������̕����킩��₷���j
				LogicInt matchLen = regexp.GetMatchLen();
				LogicInt nIdxTo = nIdx + matchLen;		// ����������̖���
				if (matchLen == 0) {
					// �O�����}�b�`�̎�(�����u���ɂȂ�Ȃ��悤�ɂP�����i�߂�)
					if (nIdxTo < nLen) {
						// 2005-09-02 D.S.Koba GetSizeOfChar
						nIdxTo += LogicInt(NativeW::GetSizeOfChar(pLine, nLen, nIdxTo) == 2 ? 2 : 1);
					}
					// �����u�����Ȃ��悤�ɁA�P�������₵���̂łP�����I���ɕύX
					// �I���n�_�E�I�_�ւ̑}���̏ꍇ���O�����}�b�`���͓���͓����ɂȂ�̂�
					layoutMgr.LogicToLayout(LogicPoint(nIdxTo, pLayout->GetLogicLineNo()), GetSelect().GetToPointer());	// 2007.01.19 ryoji �s�ʒu���擾����
				}
				// �s�����猟�������񖖔��܂ł̕�����
				LogicInt colDiff = nLen - nIdxTo;
				// Oct. 22, 2005 Karoto
				// \r��u������Ƃ��̌���\n�������Ă��܂����̑Ή�
				if (colDiff < pLayout->GetDocLineRef()->GetEol().GetLen()) {
					// ���s�ɂ������Ă�����A�s�S�̂�INSTEXT����B
					colDiff = LogicInt(0);
					layoutMgr.LogicToLayout(LogicPoint(nLen, pLayout->GetLogicLineNo()), GetSelect().GetToPointer());	// 2007.01.19 ryoji �ǉ�
				}
				// �u���㕶����ւ̏�������(�s�����猟�������񖖔��܂ł̕���������)
				Command_INSTEXT(false, regexp.GetString(), regexp.GetStringLen() - colDiff, true);
				// To Here Jun. 6, 2005 �����
			}
		}else {
			Command_INSTEXT(false, memRepKey.GetStringPtr(), memRepKey.GetStringLength(), true);
		}

		// �}����̌����J�n�ʒu�𒲐�
		if (nReplaceTarget == 1) {
			caret.SetCaretLayoutPos(caret.GetCaretLayoutPos() + ptTmp);
		}

		// To Here 2001.12.03 hor
		/* �Ō�܂Œu����������OK�����܂Œu���O�̏�Ԃ��\�������̂ŁA
		** �u����A������������O�ɏ������� 2003.05.17 �����
		*/
		m_view.Redraw();

		// ��������
		Command_SEARCH_NEXT(true, true, false, hwndParent, LSW(STR_ERR_CEDITVIEW_CMD11));
	}
}


/*! ���ׂĒu�����s

	@date 2003.05.22 ����� �����}�b�`�΍�D�s���E�s�������Ȃǌ�����
	@date 2006.03.31 ����� �s�u���@�\�ǉ�
	@date 2007.01.16 ryoji �s�u���@�\��S�u���̃I�v�V�����ɕύX
	@date 2009.09.20 genta �����`�E��ŋ�`�I�����ꂽ�̈�̒u�����s���Ȃ�
	@date 2010.09.17 ryoji ���C�����[�h�\��t��������ǉ�
	@date 2011.12.18 Moca �I�v�V�����E�����L�[��DllShareData����m_dlgReplace/EditView�x�[�X�ɕύX�B�����񒷐����̓P�p
	@date 2013.05.10 Moca fastMode
*/
void ViewCommander::Command_REPLACE_ALL()
{
	// m_sSearchOption�I���̂��߂̐�ɓK�p
	if (!m_view.ChangeCurRegexp()) {
		return;
	}

	auto& dlgReplace = GetEditWindow().m_dlgReplace;
	// 2002.02.10 hor
	bool bPaste			= dlgReplace.m_bPaste;
	BOOL nReplaceTarget	= dlgReplace.m_nReplaceTarget;
	bool bRegularExp	= m_view.m_curSearchOption.bRegularExp;
	bool bSelectedArea	= dlgReplace.bSelectedArea;
	bool bConsecutiveAll = dlgReplace.bConsecutiveAll;	// �u���ׂĒu���v�͒u���̌J�Ԃ�	// 2007.01.16 ryoji
	if (bPaste && nReplaceTarget == 3) {
		// �u���ΏہF�s�폜�̂Ƃ��́A�N���b�v�{�[�h����\��t���𖳌��ɂ���
		bPaste = false;
	}

	dlgReplace.m_bCanceled = false;
	dlgReplace.m_nReplaceCnt = 0;

	// From Here 2001.12.03 hor
	if (bPaste && !GetDocument().m_docEditor.IsEnablePaste()) {
		OkMessage(m_view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD10));
		dlgReplace.CheckButton(IDC_CHK_PASTE, false);
		dlgReplace.EnableItem(IDC_COMBO_TEXT2, true);
		return;	// TRUE;
	}
	// To Here 2001.12.03 hor
	bool bBeginBoxSelect; // ��`�I���H
	if (m_view.GetSelectionInfo().IsTextSelected()) {
		bBeginBoxSelect = m_view.GetSelectionInfo().IsBoxSelecting();
	}else {
		bSelectedArea = false;
		bBeginBoxSelect = false;
	}

	// �\������ON/OFF
	bool bDisplayUpdate = false;
	const bool bDrawSwitchOld = m_view.SetDrawSwitch(bDisplayUpdate);

	auto& layoutMgr = GetDocument().m_layoutMgr;
	auto& docLineMgr = GetDocument().m_docLineMgr;
	bool bFastMode = false;
	if (((Int)docLineMgr.GetLineCount() * 10 < (Int)layoutMgr.GetLineCount())
		&& !(bSelectedArea || bPaste)
	) {
		// 1�s������10���C�A�E�g�s�ȏ�ŁA�I���E�y�[�X�g�łȂ��ꍇ
		bFastMode = true;
	}
	int	nAllLineNum; // $$�P�ʍ���
	if (bFastMode) {
		nAllLineNum = (Int)layoutMgr.GetLineCount();
	}else {
		nAllLineNum = (Int)docLineMgr.GetLineCount();
	}
	int	nAllLineNumOrg = nAllLineNum;
	int	nAllLineNumLogicOrg = (Int)docLineMgr.GetLineCount();

	// �i���\��&���~�_�C�A���O�̍쐬
	DlgCancel	dlgCancel;
	HWND		hwndCancel = dlgCancel.DoModeless(G_AppInstance(), m_view.GetHwnd(), IDD_REPLACERUNNING);
	::EnableWindow(m_view.GetHwnd(), FALSE);
	::EnableWindow(::GetParent(m_view.GetHwnd()), FALSE);
	::EnableWindow(::GetParent(::GetParent(m_view.GetHwnd())), FALSE);
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

	LayoutRange rangeA;	// �I��͈�
	LogicPoint ptColLineP;

	// From Here 2001.12.03 hor
	auto& caret = GetCaret();
	if (bSelectedArea) {
		// �I��͈͒u��
		// �I��͈͊J�n�ʒu�̎擾
		rangeA = GetSelect();

		// From Here 2007.09.20 genta ��`�͈͂̑I��u�����ł��Ȃ�
		// �����`�E��ƑI�������ꍇ�Cm_nSelectColumnTo < m_nSelectColumnFrom �ƂȂ邪�C
		// �͈̓`�F�b�N�� colFrom < colTo �����肵�Ă���̂ŁC
		// ��`�I���̏ꍇ�͍���`�E���w��ɂȂ�悤������ꊷ����D
		if (bBeginBoxSelect && rangeA.GetTo().x < rangeA.GetFrom().x)
			t_swap(rangeA.GetFromPointer()->x, rangeA.GetToPointer()->x);
		// To Here 2007.09.20 genta ��`�͈͂̑I��u�����ł��Ȃ�

		layoutMgr.LayoutToLogic(
			rangeA.GetTo(),
			&ptColLineP
		);
		// �I��͈͊J�n�ʒu�ֈړ�
		caret.MoveCursor(rangeA.GetFrom(), bDisplayUpdate);
	}else {
		// �t�@�C���S�̒u��
		// �t�@�C���̐擪�Ɉړ�
	//	HandleCommand(F_GOFILETOP, bDisplayUpdate, 0, 0, 0, 0);
		Command_GOFILETOP(bDisplayUpdate);
	}

	LayoutPoint ptLast = caret.GetCaretLayoutPos();
	LogicPoint ptLastLogic = caret.GetCaretLogicPos();

	// �e�L�X�g�I������
	// ���݂̑I��͈͂��I����Ԃɖ߂�
	m_view.GetSelectionInfo().DisableSelectArea(bDisplayUpdate);

	LogicRange selectLogic;	// �u��������GetSelect()��Logic�P�ʔ�
	// ��������
	Command_SEARCH_NEXT(true, bDisplayUpdate, true, 0, NULL, bFastMode ? &selectLogic : nullptr);
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
		if (!m_view.MyGetClipboardData(memClip, &bColumnSelect, GetDllShareData().common.edit.bEnableLineModePaste ? &bLineSelect : nullptr)) {
			ErrorBeep();
			m_view.SetDrawSwitch(bDrawSwitchOld);

			::EnableWindow(m_view.GetHwnd(), TRUE);
			::EnableWindow(::GetParent(m_view.GetHwnd()), TRUE);
			::EnableWindow(::GetParent(::GetParent(m_view.GetHwnd())), TRUE);
			return;
		}

		// ��`�\��t����������Ă��āA�N���b�v�{�[�h�̃f�[�^����`�I���̂Ƃ��B
		if (GetDllShareData().common.edit.bAutoColumnPaste && bColumnSelect) {
			// �}�E�X�ɂ��͈͑I��
			if (m_view.GetSelectionInfo().IsMouseSelecting()) {
				ErrorBeep();
				m_view.SetDrawSwitch(bDrawSwitchOld);
				::EnableWindow(m_view.GetHwnd(), TRUE);
				::EnableWindow(::GetParent(m_view.GetHwnd()), TRUE);
				::EnableWindow(::GetParent(::GetParent(m_view.GetHwnd())), TRUE);
				return;
			}

			// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
			if (!GetDllShareData().common.view.bFontIs_FixedPitch) {
				m_view.SetDrawSwitch(bDrawSwitchOld);
				::EnableWindow(m_view.GetHwnd(), TRUE);
				::EnableWindow(::GetParent(m_view.GetHwnd()), TRUE);
				::EnableWindow(::GetParent(::GetParent(m_view.GetHwnd())), TRUE);
				return;
			}
		}else { // �N���b�v�{�[�h����̃f�[�^�͕��ʂɈ����B
			bColumnSelect = false;
		}
	}else {
		// 2004.05.14 Moca �S�u���̓r���ő��̃E�B���h�E�Œu�������Ƃ܂����̂ŃR�s�[����
		memClip.SetString(dlgReplace.m_strText2.c_str());
	}

	LogicInt nReplaceKey;			// �u���㕶����̒����B
	szREPLACEKEY = memClip.GetStringPtr(&nReplaceKey);

	// �s�R�s�[�iMSDEVLineSelect�`���j�̃e�L�X�g�Ŗ��������s�ɂȂ��Ă��Ȃ���Ή��s��ǉ�����
	// �����C�A�E�g�܂�Ԃ��̍s�R�s�[�������ꍇ�͖��������s�ɂȂ��Ă��Ȃ�
	if (bLineSelect) {
		if (!WCODE::IsLineDelimiter(szREPLACEKEY[nReplaceKey - 1], GetDllShareData().common.edit.bEnableExtEol)) {
			memClip.AppendString(GetDocument().m_docEditor.GetNewLineCode().GetValue2());
			szREPLACEKEY = memClip.GetStringPtr(&nReplaceKey);
		}
	}

	if (GetDllShareData().common.edit.bConvertEOLPaste) {
		LogicInt nConvertedTextLen = ConvertEol(szREPLACEKEY, nReplaceKey, NULL);
		std::vector<wchar_t> szConvertedText(nConvertedTextLen);
		wchar_t* pszConvertedText = &szConvertedText[0];
		ConvertEol(szREPLACEKEY, nReplaceKey, pszConvertedText);
		memClip.SetString(pszConvertedText, nConvertedTextLen);
		szREPLACEKEY = memClip.GetStringPtr(&nReplaceKey);
	}

	// �擾�ɃX�e�b�v�������肻���ȕϐ��Ȃǂ��A�ꎞ�ϐ�������B
	// �Ƃ͂����A�����̑�������邱�Ƃɂ���ē�������N���b�N���͍��킹�Ă� 1 ���[�v�Ő��\���Ǝv���܂��B
	// ���S�N���b�N�����[�v�̃I�[�_�[����l���Ă�����Ȃɓ��͂��Ȃ��悤�Ɏv���܂����ǁE�E�E�B
	bool& bCancel = dlgCancel.m_bCANCEL;

	// �N���X�֌W�����[�v�̒��Ő錾���Ă��܂��ƁA�����[�v���ƂɃR���X�g���N�^�A�f�X�g���N�^��
	// �Ă΂�Ēx���Ȃ�̂ŁA�����Ő錾�B
	Bregexp regexp;
	// �����������l�ɖ����[�v���Ƃɂ��ƒx���̂ŁA�ŏ��ɍς܂��Ă��܂��B
	if (bRegularExp && !bPaste) {
		if (!InitRegexp(m_view.GetHwnd(), regexp, true)) {
			m_view.SetDrawSwitch(bDrawSwitchOld);
			::EnableWindow(m_view.GetHwnd(), TRUE);
			::EnableWindow(::GetParent(m_view.GetHwnd()), TRUE);
			::EnableWindow(::GetParent(::GetParent(m_view.GetHwnd())), TRUE);
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
		int nFlag = (m_view.m_curSearchOption.bLoHiCase ? Bregexp::optCaseSensitive : Bregexp::optNothing);
		nFlag |= (bConsecutiveAll ? Bregexp::optNothing : Bregexp::optGlobal);	// 2007.01.16 ryoji
		regexp.Compile(m_view.m_strCurSearchKey.c_str(), memRepKey2.GetStringPtr(), nFlag);
	}

	//$$ �P�ʍ���
	LayoutPoint	ptOld;						// ������̑I��͈�
	/*LogicInt*/int		lineCnt = 0;		// �u���O�̍s��
	/*LayoutInt*/int	linDif = (0);		// �u����̍s����
	/*LayoutInt*/int	colDif = (0);		// �u����̌�����
	/*LayoutInt*/int	linPrev = (0);		// �O��̌����s(��`) @@@2001.12.31 YAZAKI warning�ގ�
	/*LogicInt*/int		linOldLen = (0);	// ������̍s�̒���
	/*LayoutInt*/int	linNext;			// ����̌����s(��`)

	int nLoopCnt = -1;

	// �e�L�X�g���I������Ă��邩
	while (
		(!bFastMode && m_view.GetSelectionInfo().IsTextSelected())
		|| (bFastMode && selectLogic.IsValid())
	) {
		// �L�����Z�����ꂽ��
		if (bCancel) {
			break;
		}

		// �������̃��[�U�[������\�ɂ���
		if (!::BlockingHook(hwndCancel)) {
			m_view.SetDrawSwitch(bDrawSwitchOld);
			::EnableWindow(m_view.GetHwnd(), TRUE);
			::EnableWindow(::GetParent(m_view.GetHwnd()), TRUE);
			::EnableWindow(::GetParent(::GetParent(m_view.GetHwnd())), TRUE);
			return;// -1;
		}

		++nLoopCnt;
		// 128 ���Ƃɕ\���B
		if ((nLoopCnt & 0x7F) == 0) {
			// ���Ԃ��Ƃɐi���󋵕`�悾�Ǝ��Ԏ擾���x���Ȃ�Ǝv�����A������̕������R���Ǝv���̂ŁE�E�E�B
			// �Ǝv�������ǁA�t�ɂ�����̕������R�ł͂Ȃ��̂ŁA��߂�B
		
			if (bFastMode) {
				int nDiff = nAllLineNumOrg - (Int)docLineMgr.GetLineCount();
				if (0 <= nDiff) {
					nNewPos = (nDiff + (Int)selectLogic.GetFrom().GetY2()) >> nShiftCount;
				}else {
					nNewPos = ::MulDiv((Int)selectLogic.GetFrom().GetY(), nAllLineNum, (Int)layoutMgr.GetLineCount());
				}
			}else {
				int nDiff = nAllLineNumOrg - (Int)layoutMgr.GetLineCount();
				if (0 <= nDiff) {
					nNewPos = (nDiff + (Int)GetSelect().GetFrom().GetY2()) >> nShiftCount;
				}else {
					nNewPos = ::MulDiv((Int)GetSelect().GetFrom().GetY(), nAllLineNum, (Int)layoutMgr.GetLineCount());
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
				lineCnt = (Int)layoutMgr.GetLineCount();
				// ������͈̔͏I�[
				ptOld = GetSelect().GetTo();
				// �O��̌����s�ƈႤ�H
				if (ptOld.y != linPrev) {
					colDif = (0);
				}
				linPrev = (Int)ptOld.GetY2();
				// �s�͔͈͓��H
				if (
					(rangeA.GetTo().y + linDif == ptOld.y && rangeA.GetTo().GetX2() + colDif < ptOld.x)
					|| (rangeA.GetTo().y + linDif <  ptOld.y)
				) {
					break;
				}
				// ���͔͈͓��H
				if (!(rangeA.GetFrom().x <= GetSelect().GetFrom().x && ptOld.GetX2() <= rangeA.GetTo().GetX2() + colDif)) {
					if (ptOld.x < rangeA.GetTo().GetX2() + colDif) {
						linNext = (Int)GetSelect().GetTo().GetY2();
					}else {
						linNext = (Int)GetSelect().GetTo().GetY2() + 1;
					}
					// ���̌����J�n�ʒu�փV�t�g
					caret.SetCaretLayoutPos(LayoutPoint(rangeA.GetFrom().x, LayoutInt(linNext)));
					// 2004.05.30 Moca ���݂̌�����������g���Č�������
					Command_SEARCH_NEXT(false, bDisplayUpdate, true, 0, nullptr);
					colDif = (0);
					continue;
				}
			}else {
				// ���ʂ̑I��
				// o �������W���`�F�b�N���Ȃ���u������
				//
			
				// �������̍s�����L��
				lineCnt = docLineMgr.GetLineCount();

				// ������͈̔͏I�[
				LogicPoint ptOldTmp;
				if (bFastMode) {
					ptOldTmp = selectLogic.GetTo();
				}else {
					layoutMgr.LayoutToLogic(
						GetSelect().GetTo(),
						&ptOldTmp
					);
				}
				ptOld.x = (LayoutInt)ptOldTmp.x; //$$ ���C�A�E�g�^�ɖ�����胍�W�b�N�^�����B�C��������
				ptOld.y = (LayoutInt)ptOldTmp.y;

				// �u���O�̍s�̒���(���s�͂P�����Ɛ�����)��ۑ����Ă����āA�u���O��ōs�ʒu���ς�����ꍇ�Ɏg�p
				linOldLen = docLineMgr.GetLine(ptOldTmp.GetY2())->GetLengthWithoutEOL() + LogicInt(1);

				// �s�͔͈͓��H
				// 2007.01.19 ryoji �����ǉ�: �I���I�_���s��(ptColLineP.x == 0)�ɂȂ��Ă���ꍇ�͑O�̍s�̍s���܂ł�I��͈͂Ƃ݂Ȃ�
				// �i�I���n�_���s���Ȃ炻�̍s���͑I��͈͂Ɋ܂݁A�I�_���s���Ȃ炻�̍s���͑I��͈͂Ɋ܂܂Ȃ��A�Ƃ���j
				// �_���I�ɏ����ςƎw�E����邩������Ȃ����A���p��͂��̂悤�ɂ����ق����]�܂����P�[�X�������Ǝv����B
				// ���s�I���ōs���܂ł�I��͈͂ɂ�������ł��AUI��͎��̍s�̍s���ɃJ�[�\�����s��
				// ���I�_�̍s�����u^�v�Ƀ}�b�`��������������P�����ȏ�I�����ĂˁA�Ƃ������ƂŁD�D�D
				// $$ �P�ʍ��݂��܂��肾���ǁA���v�H�H
				if (
					(
						ptColLineP.y + linDif == (Int)ptOld.y
						&& (
							ptColLineP.x + colDif < (Int)ptOld.x
							|| ptColLineP.x == 0
						)
					)
					|| ptColLineP.y + linDif < (Int)ptOld.y
				) {
					break;
				}
			}
		}

		LayoutPoint ptTmp(0, 0);
		LogicPoint  ptTmpLogic(0, 0);

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
				const LogicInt y = selectLogic.GetFrom().y;
				selectLogic.SetFrom(LogicPoint(LogicXInt(0), y)); // �s��
				selectLogic.SetTo(LogicPoint(LogicXInt(0), y + LogicInt(1))); // ���s�̍s��
				if (docLineMgr.GetLineCount() == y + LogicInt(1)) {
					const DocLine* pLine = docLineMgr.GetLine(y);
					if (pLine->GetEol() == EolType::None) {
						// EOF�͍ŏI�f�[�^�s�ɂԂ牺����Ȃ̂ŁA�I���I�[�͍s��
						selectLogic.SetTo(LogicPoint(pLine->GetLengthWithEOL(), y)); // �Ώۍs�̍s��
					}
				}
				caret.MoveCursorFastMode(selectLogic.GetFrom());
			}else {
				LogicPoint lineHome;
				layoutMgr.LayoutToLogic(GetSelect().GetFrom(), &lineHome);
				lineHome.x = LogicXInt(0); // �s��
				LayoutRange selectFix;
				layoutMgr.LogicToLayout(lineHome, selectFix.GetFromPointer());
				lineHome.y++; // ���s�̍s��
				layoutMgr.LogicToLayout(lineHome, selectFix.GetToPointer());
				caret.GetAdjustCursorPos(selectFix.GetToPointer());
				m_view.GetSelectionInfo().SetSelectArea(selectFix);
				caret.MoveCursor(selectFix.GetFrom(), false);
			}
		}


		// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
		// �e�L�X�g��\��t��
		if (bPaste) {
			if (!bColumnSelect) {
				/* �{���� Command_INSTEXT ���g���ׂ��Ȃ�ł��傤���A���ʂȏ���������邽�߂ɒ��ڂ������B
				** ��m_nSelectXXX��-1�̎��� m_view.ReplaceData_CEditView�𒼐ڂ������Ɠ���s�ǂƂȂ邽��
				**   ���ڂ������̂�߂��B2003.05.18 by �����
				*/
				Command_INSTEXT(false, szREPLACEKEY, nReplaceKey, true, bLineSelect);
			}else {
				Command_PASTEBOX(szREPLACEKEY, nReplaceKey);
				// 2013.06.11 �ĕ`�悵�Ȃ��悤��
				// �ĕ`����s��Ȃ��Ƃǂ�Ȍ��ʂ��N���Ă���̂������炸�݂��Ƃ��Ȃ��̂ŁE�E�E�B
				// m_view.AdjustScrollBars(); // 2007.07.22 ryoji
				// m_view.Redraw();
			}
			++nReplaceNum;
		}else if (nReplaceTarget == 3) {
			Command_INSTEXT( false, L"", LogicInt(0), true, false, bFastMode, bFastMode ? &selectLogic : nullptr );
			++nReplaceNum;
		}else if (bRegularExp) { // �����^�u��  1==���K�\��
			// 2002/01/19 novice ���K�\���ɂ�镶����u��
			// �����s�A�����s���A�����s�ł̌����}�b�`�ʒu
			const DocLine* pDocLine;
			const wchar_t* pLine;
			LogicInt nLogicLineNum;
			LogicInt nIdx;
			LogicInt nLen;
			if (bFastMode) {
				pDocLine = docLineMgr.GetLine(selectLogic.GetFrom().GetY2());
				pLine = pDocLine->GetPtr();
				nLogicLineNum = selectLogic.GetFrom().GetY2();
				nIdx = selectLogic.GetFrom().GetX2();
				nLen = pDocLine->GetLengthWithEOL();
			}else {
				const Layout* pLayout = layoutMgr.SearchLineByLayoutY(GetSelect().GetFrom().GetY2());
				pDocLine = pLayout->GetDocLineRef();
				pLine = pDocLine->GetPtr();
				nLogicLineNum = pLayout->GetLogicLineNo();
				nIdx = m_view.LineColumnToIndex(pLayout, GetSelect().GetFrom().GetX2()) + pLayout->GetLogicOffset();
				nLen = pDocLine->GetLengthWithEOL();
			}
			LogicInt colDiff = LogicInt(0);
			if (!bConsecutiveAll) {	// �ꊇ�u��
				// 2007.01.16 ryoji
				// �I��͈͒u���̏ꍇ�͍s���̑I��͈͖����܂Œu���͈͂��k�߁C
				// ���̈ʒu���L������D
				if (bSelectedArea) {
					if (bBeginBoxSelect) {	// ��`�I��
						LogicPoint ptWork;
						layoutMgr.LayoutToLogic(
							LayoutPoint(rangeA.GetTo().x, ptOld.y),
							&ptWork
						);
						ptColLineP.x = ptWork.x;
						if (nLen - pDocLine->GetEol().GetLen() > ptColLineP.x + colDif)
							nLen = ptColLineP.GetX2() + LogicInt(colDif);
					}else {	// �ʏ�̑I��
						if (ptColLineP.y + linDif == (Int)ptOld.y) { //$$ �P�ʍ���
							if (nLen - pDocLine->GetEol().GetLen() > ptColLineP.x + colDif)
								nLen = ptColLineP.GetX2() + LogicInt(colDif);
						}
					}
				}

				if (pDocLine->GetLengthWithoutEOL() < nLen)
					ptOld.x = (LayoutInt)(Int)pDocLine->GetLengthWithoutEOL() + 1; //$$ �P�ʍ���
				else
					ptOld.x = (LayoutInt)(Int)nLen; //$$ �P�ʍ���
			}

			if (int nReplace = regexp.Replace(pLine, nLen, nIdx)) {
				nReplaceNum += nReplace;
				if (!bConsecutiveAll) { // 2006.04.01 �����	// 2007.01.16 ryoji
					// �s�P�ʂł̒u������
					// �I��͈͂𕨗��s���܂łɂ̂΂�
					if (bFastMode) {
						selectLogic.SetTo(LogicPoint(nLen, nLogicLineNum));
					}else {
						layoutMgr.LogicToLayout(LogicPoint(nLen, nLogicLineNum), GetSelect().GetToPointer());
					}
				}else {
				    // From Here Jun. 6, 2005 �����
				    // �����s���܂�INSTEXT������@�́A�L�����b�g�ʒu�𒲐�����K�v������A
				    // �L�����b�g�ʒu�̌v�Z�����G�ɂȂ�B�i�u����ɉ��s������ꍇ�ɕs������j
				    // �����ŁAINSTEXT���镶���񒷂𒲐�������@�ɕύX����i���͂������̕����킩��₷���j
				    LogicInt matchLen = regexp.GetMatchLen();
				    LogicInt nIdxTo = nIdx + matchLen;		// ����������̖���
				    if (matchLen == 0) {
					    // �O�����}�b�`�̎�(�����u���ɂȂ�Ȃ��悤�ɂP�����i�߂�)
					    if (nIdxTo < nLen) {
						    // 2005-09-02 D.S.Koba GetSizeOfChar
						    nIdxTo += LogicInt(NativeW::GetSizeOfChar(pLine, nLen, nIdxTo) == 2 ? 2 : 1);
					    }
					    // �����u�����Ȃ��悤�ɁA�P�������₵���̂łP�����I���ɕύX
					    // �I���n�_�E�I�_�ւ̑}���̏ꍇ���O�����}�b�`���͓���͓����ɂȂ�̂�
						if (bFastMode) {
							selectLogic.SetTo(LogicPoint(nIdxTo, nLogicLineNum));
						}else {
							layoutMgr.LogicToLayout(LogicPoint(nIdxTo, nLogicLineNum), GetSelect().GetToPointer());	// 2007.01.19 ryoji �s�ʒu���擾����
						}
				    }
				    // �s�����猟�������񖖔��܂ł̕�����
					colDiff =  nLen - nIdxTo;
					ptOld.x = (LayoutInt)(Int)nIdxTo;	// 2007.01.19 ryoji �ǉ�  // $$ �P�ʍ���
				    // Oct. 22, 2005 Karoto
				    // \r��u������Ƃ��̌���\n�������Ă��܂����̑Ή�
				    if (colDiff < pDocLine->GetEol().GetLen()) {
					    // ���s�ɂ������Ă�����A�s�S�̂�INSTEXT����B
					    colDiff = LogicInt(0);
						if (bFastMode) {
							selectLogic.SetTo(LogicPoint(nLen, nLogicLineNum));
						}else {
							layoutMgr.LogicToLayout(LogicPoint(nLen, nLogicLineNum), GetSelect().GetToPointer());	// 2007.01.19 ryoji �ǉ�
						}
						ptOld.x = (LayoutInt)(Int)pDocLine->GetLengthWithoutEOL() + 1;	// 2007.01.19 ryoji �ǉ� //$$ �P�ʍ���
				    }
				}
				// �u���㕶����ւ̏�������(�s�����猟�������񖖔��܂ł̕���������)
				Command_INSTEXT(false, regexp.GetString(), regexp.GetStringLen() - colDiff, true, false, bFastMode, bFastMode ? &selectLogic : nullptr);
				// To Here Jun. 6, 2005 �����
			}
		}else {
			/* �{���͌��R�[�h���g���ׂ��Ȃ�ł��傤���A���ʂȏ���������邽�߂ɒ��ڂ������B
			** ��m_nSelectXXX��-1�̎��� m_view.ReplaceData_CEditView�𒼐ڂ������Ɠ���s�ǂƂȂ邽�ߒ��ڂ������̂�߂��B2003.05.18 �����
			*/
			Command_INSTEXT(false, szREPLACEKEY, nReplaceKey, true, false, bFastMode, bFastMode ? &selectLogic : nullptr);
			++nReplaceNum;
		}

		// �}����̈ʒu����
		if (nReplaceTarget == 1) {
			if (bFastMode) {
				caret.SetCaretLogicPos(caret.GetCaretLogicPos() + ptTmpLogic);
			}else {
				caret.SetCaretLayoutPos(caret.GetCaretLayoutPos() + ptTmp);
				if (!bBeginBoxSelect) {
					LogicPoint p;
					layoutMgr.LayoutToLogic(
						caret.GetCaretLayoutPos(),
						&p
					);
					caret.SetCaretLogicPos(p);
				}
			}
		}

		if (!bFastMode && 50 <= nReplaceNum && !(bSelectedArea || bPaste)) {
			bFastMode = true;
			nAllLineNum = (Int)docLineMgr.GetLineCount();
			nAllLineNumOrg = nAllLineNumLogicOrg;
			for (nShiftCount=0; 300<nAllLineNum; ++nShiftCount) {
				nAllLineNum /= 2;
			}
			Progress_SetRange( hwndProgress, 0, nAllLineNum + 1 );
			int nDiff = nAllLineNumOrg - (Int)docLineMgr.GetLineCount();
			if (0 <= nDiff) {
				nNewPos = (nDiff + (Int)selectLogic.GetFrom().GetY2()) >> nShiftCount;
			}else {
				nNewPos = ::MulDiv((Int)selectLogic.GetFrom().GetY(), nAllLineNum, (Int)docLineMgr.GetLineCount());
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
				colDif += (Int)(ptLast.GetX2() - ptOld.GetX2());
				linDif += (Int)(layoutMgr.GetLineCount() - lineCnt);
			}else {
				// �u���O�̌���������̍ŏI�ʒu�� ptOld
				// �u����̃J�[�\���ʒu
				LogicPoint ptTmp2 = caret.GetCaretLogicPos();
				int linDif_thistime = docLineMgr.GetLineCount() - lineCnt;	// ����u���ł̍s���ω�
				linDif += linDif_thistime;
				if (ptColLineP.y + linDif == ptTmp2.y) {
					// �ŏI�s�Œu���������A���́A�u���̌��ʁA�I���G���A�ŏI�s�܂œ��B������
					// �ŏI�s�Ȃ̂ŁA�u���O��̕������̑����Ō��ʒu�𒲐�����
					colDif += (Int)ptTmp2.GetX2() - (Int)ptOld.GetX2(); //$$ �P�ʍ���

					// �A���A�ȉ��̏ꍇ�͒u���O��ōs���قȂ��Ă��܂��̂ŁA�s�̒����ŕ␳����K�v������
					// �P�j�ŏI�s���O�ōs�A�����N����A�s�������Ă���ꍇ�i�s�A���Ȃ̂ŁA���ʒu�͒u����̃J�[�\�����ʒu����������j
					// �@�@ptTmp2.x-ptOld.x���ƁA\r\n �� "" �u���ōs�A�������ꍇ�ɁA���ʒu�����ɂȂ莸�s����i���Ƃ͑O�s�̌��̕��ɂȂ邱�ƂȂ̂ŕ␳����j
					// �@�@����u���ł̍s���̕ω�(linDif_thistime)�ŁA�ŏI�s���s�A�����ꂽ���ǂ��������邱�Ƃɂ���
					// �Q�j���s��u�������iptTmp2.y != ptOld.y�j�ꍇ�A���s��u������ƒu����̌��ʒu�����s�̌��ʒu�ɂȂ��Ă��邽��
					//     ptTmp2.x-ptOld.x���ƁA���̐��ƂȂ�A\r\n �� \n �� \n �� "abc" �ȂǂŌ��ʒu�������
					//     ������O�s�̒����Œ�������K�v������
					if (linDif_thistime < 0 || ptTmp2.y != (Int)ptOld.y) { //$$ �P�ʍ���
						colDif += linOldLen;
					}
				}
			}
		}
		// To Here 2001.12.03 hor

		// ��������
		// 2004.05.30 Moca ���݂̌�����������g���Č�������
		Command_SEARCH_NEXT(false, bDisplayUpdate, true, 0, NULL, bFastMode ? &selectLogic : nullptr);
	}

	if (bFastMode) {
		if (0 < nReplaceNum) {
			// LayoutMgr�̍X�V(�ύX�L�̏ꍇ)
			layoutMgr._DoLayout();
			GetEditWindow().ClearViewCaretPosInfo();
			if (GetDocument().m_nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
				layoutMgr.CalculateTextWidth();
			}
		}
		layoutMgr.LogicToLayout(ptLastLogic, &ptLast);
		caret.MoveCursor(ptLast, true);
		caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();	// 2009.07.25 ryoji
	}
	//>> 2002/03/26 Azumaiya

	_itot(nReplaceNum, szLabel, 10);
	::SendMessage(hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel);

	if (!dlgCancel.IsCanceled()) {
		nNewPos = nAllLineNum;
		Progress_SetPos(hwndProgress, nNewPos + 1);
		Progress_SetPos(hwndProgress, nNewPos);
	}
	dlgCancel.CloseDialog(0);
	::EnableWindow(m_view.GetHwnd(), TRUE);
	::EnableWindow(::GetParent(m_view.GetHwnd()), TRUE);
	::EnableWindow(::GetParent(::GetParent(m_view.GetHwnd())), TRUE);

	// From Here 2001.12.03 hor

	// �e�L�X�g�I������
	m_view.GetSelectionInfo().DisableSelectArea(false);

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
			m_view.GetSelectionInfo().SetBoxSelect(bBeginBoxSelect);
			rangeA.GetToPointer()->y += linDif;
			if (rangeA.GetTo().y < 0) rangeA.SetToY(LayoutInt(0));
		}else {
			// ���ʂ̑I��
			ptColLineP.x += colDif;
			if (ptColLineP.x < 0) ptColLineP.x = 0;
			ptColLineP.y += linDif;
			if (ptColLineP.y < 0) ptColLineP.y = 0;
			layoutMgr.LogicToLayout(
				ptColLineP,
				rangeA.GetToPointer()
			);
		}
		if (rangeA.GetFrom().y<rangeA.GetTo().y || rangeA.GetFrom().x<rangeA.GetTo().x) {
			m_view.GetSelectionInfo().SetSelectArea(rangeA);	// 2009.07.25 ryoji
		}
		caret.MoveCursor(rangeA.GetTo(), true);
		caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();	// 2009.07.25 ryoji
	}
	// To Here 2001.12.03 hor

	dlgReplace.m_bCanceled = dlgCancel.IsCanceled();
	dlgReplace.m_nReplaceCnt = nReplaceNum;
	m_view.SetDrawSwitch(bDrawSwitchOld);
	ActivateFrameWindow(GetMainWindow());
}


// �����}�[�N�̐ؑւ�	// 2001.12.03 hor �N���A �� �ؑւ� �ɕύX
void ViewCommander::Command_SEARCH_CLEARMARK(void)
{
// From Here 2001.12.03 hor

	// �����}�[�N�̃Z�b�g

	if (m_view.GetSelectionInfo().IsTextSelected()) {

		// ����������擾
		NativeW	memCurText;
		m_view.GetCurrentTextForSearch(memCurText, false);
		auto& csSearch = GetDllShareData().common.search;

		m_view.m_strCurSearchKey = memCurText.GetStringPtr();
		if (m_view.m_nCurSearchKeySequence < csSearch.nSearchKeySequence) {
			m_view.m_curSearchOption = csSearch.searchOption;
		}
		m_view.m_curSearchOption.bRegularExp = false;	// ���K�\���g��Ȃ�
		m_view.m_curSearchOption.bWordOnly = false;		// �P��Ō������Ȃ�

		// ���L�f�[�^�֓o�^
		if (memCurText.GetStringLength() < _MAX_PATH) {
			SearchKeywordManager().AddToSearchKeys(memCurText.GetStringPtr());
			csSearch.searchOption = m_view.m_curSearchOption;
		}
		m_view.m_nCurSearchKeySequence = csSearch.nSearchKeySequence;
		m_view.m_bCurSearchUpdate = true;

		m_view.ChangeCurRegexp(false); // 2002.11.11 Moca ���K�\���Ō���������C�F�������ł��Ă��Ȃ�����

		// �ĕ`��
		m_view.RedrawAll();
		return;
	}
// To Here 2001.12.03 hor

	// �����}�[�N�̃N���A

	m_view.m_bCurSrchKeyMark = false;	// ����������̃}�[�N
	// �t�H�[�J�X�ړ����̍ĕ`��
	m_view.RedrawAll();
	return;
}

// Jun. 16, 2000 genta
// �Ί��ʂ̌���
void ViewCommander::Command_BRACKETPAIR(void)
{
	LayoutPoint ptColLine;
	//int nLine, nCol;

	int mode = 3;
	/*
	bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ�
	bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ�
	bit2(out) : ���������ʒu         0:���      1:�O
	*/
	if (m_view.SearchBracket(GetCaret().GetCaretLayoutPos(), &ptColLine, &mode)) {	// 02/09/18 ai
		// 2005.06.24 Moca
		// 2006.07.09 genta �\���X�V�R��F�V�K�֐��ɂđΉ�
		m_view.MoveCursorSelecting(ptColLine, m_view.GetSelectionInfo().m_bSelectingLock);
	}else {
		// ���s�����ꍇ�� nCol/nLine�ɂ͗L���Ȓl�������Ă��Ȃ�.
		// �������Ȃ�
	}
}

