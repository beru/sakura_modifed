/*!	@file
@brief ViewCommander�N���X�̃R�}���h(�����n ��{�`)�֐��Q

	2012/12/17	CViewCommander.cpp,CViewCommander_New.cpp���番��
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
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "dlg/CDlgCancel.h"// 2002/2/8 hor
#include "CSearchAgent.h"
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
	GetEditWindow()->m_cToolbar.SetFocusSearchBox();
}


// ����(�P�ꌟ���_�C�A���O)
void ViewCommander::Command_SEARCH_DIALOG(void)
{
	// ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾
	NativeW cmemCurText;
	m_pCommanderView->GetCurrentTextForSearchDlg(cmemCurText);	// 2006.08.23 ryoji �_�C�A���O��p�֐��ɕύX

	auto& dlgFind = GetEditWindow()->m_cDlgFind;
	// �����������������
	if (0 < cmemCurText.GetStringLength()) {
		dlgFind.m_strText = cmemCurText.GetStringPtr();
	}
	// �����_�C�A���O�̕\��
	if (!dlgFind.GetHwnd()) {
		dlgFind.DoModeless(G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)&GetEditWindow()->GetActiveView());
	}else {
		// �A�N�e�B�u�ɂ���
		ActivateFrameWindow(dlgFind.GetHwnd());
		dlgFind.SetItemText(IDC_COMBO_TEXT, cmemCurText.GetStringT());
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
	LogicRange*	pcSelectLogic		// [out] �I��͈͂̃��W�b�N�ŁB�}�b�`�͈͂�Ԃ��B���ׂĒu��/�������[�h�Ŏg�p
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

	LayoutRange	sRangeA;
	sRangeA.Set(GetCaret().GetCaretLayoutPos());

	LayoutRange	sSelectBgn_Old;
	LayoutRange	sSelect_Old;
	LayoutInt	nLineNumOld(0);

	// bFastMode
	LogicInt nLineNumLogic(0);

	bool	bRedo = false;	// hor
	int		nIdxOld = 0;	// hor
	int		nSearchResult;
	auto& layoutMgr = GetDocument()->m_cLayoutMgr;

	if (pcSelectLogic) {
		pcSelectLogic->Clear(-1);
	}

	bSelecting = false;
	// 2002.01.16 hor
	// ���ʕ����̂����肾��
	// 2004.05.30 Moca CEditView�̌��ݐݒ肳��Ă��錟���p�^�[�����g����悤��
	if (bChangeCurRegexp && !m_pCommanderView->ChangeCurRegexp()) return;
	if (m_pCommanderView->m_strCurSearchKey.size() == 0) {
		goto end_of_func;
	}

	auto& si = m_pCommanderView->GetSelectionInfo();
	auto& caret = GetCaret();
	// �����J�n�ʒu�𒲐�
	bFlag1 = false;
	if (!pcSelectLogic && si.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// ��`�͈͑I�𒆂łȂ� & �I����Ԃ̃��b�N
		if (!si.IsBoxSelecting() && si.m_bSelectingLock) {
			bSelecting = true;
			bSelectingLock_Old = si.m_bSelectingLock;

			sSelectBgn_Old = si.m_sSelectBgn; // �͈͑I��(���_)
			sSelect_Old = GetSelect();

			if (PointCompare(si.m_sSelectBgn.GetFrom(), caret.GetCaretLayoutPos()) >= 0) {
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
	if (!pcSelectLogic) {
		nLineNum = caret.GetCaretLayoutPos().GetY2();
		LogicInt nLineLen = LogicInt(0); // 2004.03.17 Moca NULL == pLine�̂Ƃ��AnLineLen�����ݒ�ɂȂ藎����o�O�΍�
		const Layout*	pcLayout;
		const wchar_t*	pLine = layoutMgr.GetLineStr(nLineNum, &nLineLen, &pcLayout);

		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
		// 2002.02.08 hor EOF�݂̂̍s��������������Ă��Č����\�� (2/2)
		nIdx = pcLayout ? m_pCommanderView->LineColumnToIndex(pcLayout, caret.GetCaretLayoutPos().GetX2()) : LogicInt(0);
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
	if (!pcSelectLogic) {
		nSearchResult = layoutMgr.SearchWord(
			nLineNum,						// �����J�n���C�A�E�g�s
			nIdx,							// �����J�n�f�[�^�ʒu
			eSearchDirection::Forward,		// 0==�O������ 1==�������
			&sRangeA,						// �}�b�`���C�A�E�g�͈�
			m_pCommanderView->m_sSearchPattern
		);
	}else {
		nSearchResult = SearchAgent(&GetDocument()->m_cDocLineMgr).SearchWord(
			LogicPoint(nIdx, nLineNumLogic),
			eSearchDirection::Forward,		// 0==�O������ 1==�������
			pcSelectLogic,
			m_pCommanderView->m_sSearchPattern
		);
	}
	if (nSearchResult) {
		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
		if (bFlag1 && sRangeA.GetFrom() == caret.GetCaretLayoutPos()) {
			LogicRange sRange_Logic;
			layoutMgr.LayoutToLogic(sRangeA, &sRange_Logic);

			nLineNum = sRangeA.GetTo().GetY2();
			nIdx     = sRange_Logic.GetTo().GetX2();
			if (sRange_Logic.GetFrom() == sRange_Logic.GetTo()) { // ��0�}�b�`�ł̖������[�v�΍�B
				nIdx += 1; // wchar_t����i�߂邾���ł͑���Ȃ���������Ȃ����B
			}
			goto re_do;
		}

		if (bSelecting) {
			// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
			si.ChangeSelectAreaByCurrentCursor(sRangeA.GetTo());
			si.m_bSelectingLock = bSelectingLock_Old;	// �I����Ԃ̃��b�N
		}else if (!pcSelectLogic) {
			// �I��͈͂̕ύX
			// 2005.06.24 Moca
			si.SetSelectArea(sRangeA);

			if (bRedraw) {
				// �I��̈�`��
				si.DrawSelectArea();
			}
		}

		// �J�[�\���ړ�
		// Sep. 8, 2000 genta
		if (!bReplaceAll) m_pCommanderView->AddCurrentLineToHistory();	// 2002.02.16 hor ���ׂĒu���̂Ƃ��͕s�v
		if (!pcSelectLogic) {
			caret.MoveCursor(sRangeA.GetFrom(), bRedraw);
			caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
		}else {
			caret.MoveCursorFastMode(pcSelectLogic->GetFrom());
		}
		bFound = TRUE;
	}else {
		if (bSelecting) {
			si.m_bSelectingLock = bSelectingLock_Old;	// �I����Ԃ̃��b�N

			// �I��͈͂̕ύX
			si.m_sSelectBgn = sSelectBgn_Old; // �͈͑I��(���_)
			si.m_sSelectOld = sSelect_Old;	// 2011.12.24
			GetSelect().SetFrom(sSelect_Old.GetFrom());
			GetSelect().SetTo(sRangeA.GetFrom());

			// �J�[�\���ړ�
			caret.MoveCursor(sRangeA.GetFrom(), bRedraw);
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
				m_pCommanderView->DrawBracketCursorLine(bRedraw);
			}
		}
	}

end_of_func:;
// From Here 2002.01.26 hor �擪�i�����j����Č���
	if (GetDllShareData().m_common.m_sSearch.m_bSearchAll) {
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
		if (!pcSelectLogic && ((nLineNumOld > nLineNum)||(nLineNumOld == nLineNum && nIdxOld > nIdx)))
			m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT1));
	}else {
		caret.ShowEditCaret();	// 2002/04/18 YAZAKI
		caret.ShowCaretPosInfo();	// 2002/04/18 YAZAKI
		if (!bReplaceAll) {
			m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT2));
		}
// To Here 2002.01.26 hor

		// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
		if (!pszNotFoundMessage) {
			NativeW KeyName;
			LimitStringLengthW(m_pCommanderView->m_strCurSearchKey.c_str(), m_pCommanderView->m_strCurSearchKey.size(), _MAX_PATH, KeyName);
			if ((size_t)KeyName.GetStringLength() < m_pCommanderView->m_strCurSearchKey.size()) {
				KeyName.AppendString(L"...");
			}
			AlertNotFound(
				hwndParent,
				bReplaceAll,
				LS(STR_ERR_SRNEXT3),
				KeyName.GetStringPtr()
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
	auto& layoutMgr = GetDocument()->m_cLayoutMgr;

	LayoutRange sRangeA;
	sRangeA.Set(caret.GetCaretLayoutPos());

	LayoutRange sSelectBgn_Old;
	LayoutRange sSelect_Old;

	bool bSelecting = false;
	// 2002.01.16 hor
	// ���ʕ����̂����肾��
	if (!m_pCommanderView->ChangeCurRegexp()) {
		return;
	}
	if (m_pCommanderView->m_strCurSearchKey.size() == 0) {
		goto end_of_func;
	}
	auto& si = m_pCommanderView->GetSelectionInfo();
	if (si.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		sSelectBgn_Old = si.m_sSelectBgn; // �͈͑I��(���_)
		sSelect_Old = GetSelect();
		
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
	const Layout* pcLayout = layoutMgr.SearchLineByLayoutY(nLineNum);
	if (!pcLayout) {
		// pcLayout��NULL�ƂȂ�̂́A[EOF]����O���������ꍇ
		// �P�s�O�Ɉړ����鏈��
		--nLineNum;
		if (nLineNum < 0) {
			goto end_of_func;
		}
		pcLayout = layoutMgr.SearchLineByLayoutY(nLineNum);
		if (!pcLayout) {
			goto end_of_func;
		}
		// �J�[�\�����ړ��͂�߂� nIdx�͍s�̒����Ƃ��Ȃ���[EOF]������s��O�����������ɍŌ�̉��s�������ł��Ȃ� 2003.05.04 �����
		Layout* pCLayout = layoutMgr.SearchLineByLayoutY(nLineNum);
		nIdx = LogicInt(pCLayout->GetDocLineRef()->GetLengthWithEOL() + 1);		// �s���̃k������(\0)�Ƀ}�b�`�����邽�߂�+1 2003.05.16 �����
	}else {
		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
		nIdx = m_pCommanderView->LineColumnToIndex(pcLayout, caret.GetCaretLayoutPos().GetX2());
	}

	bRedo		=	true;		// hor
	nLineNumOld	=	nLineNum;	// hor
	nIdxOld		=	nIdx;		// hor
re_do:;							// hor
	// ���݈ʒu���O�̈ʒu����������
	if (layoutMgr.SearchWord(
			nLineNum,								// �����J�n���C�A�E�g�s
			nIdx,									// �����J�n�f�[�^�ʒu
			eSearchDirection::Backward,				// 0==�O������ 1==�������
			&sRangeA,								// �}�b�`���C�A�E�g�͈�
			m_pCommanderView->m_sSearchPattern
		)
	) {
		if (bSelecting) {
			// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
			si.ChangeSelectAreaByCurrentCursor(sRangeA.GetFrom());
			si.m_bSelectingLock = bSelectingLock_Old;	// �I����Ԃ̃��b�N
		}else {
			// �I��͈͂̕ύX
			// 2005.06.24 Moca
			si.SetSelectArea(sRangeA);

			if (bReDraw) {
				// �I��̈�`��
				si.DrawSelectArea();
			}
		}
		// �J�[�\���ړ�
		// Sep. 8, 2000 genta
		m_pCommanderView->AddCurrentLineToHistory();
		caret.MoveCursor(sRangeA.GetFrom(), bReDraw);
		caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
		bFound = TRUE;
	}else {
		if (bSelecting) {
			si.m_bSelectingLock = bSelectingLock_Old;	// �I����Ԃ̃��b�N
			// �I��͈͂̕ύX
			si.m_sSelectBgn = sSelectBgn_Old;
			GetSelect() = sSelect_Old;

			// �J�[�\���ړ�
			caret.MoveCursor(sRangeA.GetFrom(), bReDraw);
			caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
			// �I��̈�`��
			si.DrawSelectArea();
		}else {
			if (bDisableSelect) {
				m_pCommanderView->DrawBracketCursorLine(bReDraw);
			}
		}
	}
end_of_func:;
// From Here 2002.01.26 hor �擪�i�����j����Č���
	if (GetDllShareData().m_common.m_sSearch.m_bSearchAll) {
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
			m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV1));
	}else {
		m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV2));
// To Here 2002.01.26 hor

		// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
		NativeW KeyName;
		LimitStringLengthW(m_pCommanderView->m_strCurSearchKey.c_str(), m_pCommanderView->m_strCurSearchKey.size(),
			_MAX_PATH, KeyName);
		if ((size_t)KeyName.GetStringLength() < m_pCommanderView->m_strCurSearchKey.size()) {
			KeyName.AppendString(L"...");
		}
		AlertNotFound(
			hwndParent,
			false,
			LS(STR_ERR_SRPREV3),	// Jan. 25, 2001 jepro ���b�Z�[�W���኱�ύX
			KeyName.GetStringPtr()
		);
	}
	return;
}


// �u��(�u���_�C�A���O)
void ViewCommander::Command_REPLACE_DIALOG(void)
{
	bool bSelected = false;

	// ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾
	NativeW cmemCurText;
	m_pCommanderView->GetCurrentTextForSearchDlg(cmemCurText);	// 2006.08.23 ryoji �_�C�A���O��p�֐��ɕύX

	auto& dlgReplace = GetEditWindow()->m_cDlgReplace;

	// �����������������
	if (0 < cmemCurText.GetStringLength()) {
		dlgReplace.m_strText = cmemCurText.GetStringPtr();
	}
	if (0 < GetDllShareData().m_searchKeywords.m_aReplaceKeys.size()) {
		if (dlgReplace.m_nReplaceKeySequence < GetDllShareData().m_common.m_sSearch.m_nReplaceKeySequence) {
			dlgReplace.m_strText2 = GetDllShareData().m_searchKeywords.m_aReplaceKeys[0];	// 2006.08.23 ryoji �O��̒u���㕶����������p��
		}
	}
	
	if (m_pCommanderView->GetSelectionInfo().IsTextSelected() && !GetSelect().IsLineOne()) {
		bSelected = true;	// �I��͈͂��`�F�b�N���ă_�C�A���O�\��
	}else {
		bSelected = false;	// �t�@�C���S�̂��`�F�b�N���ă_�C�A���O�\��
	}
	// �u���I�v�V�����̏�����
	dlgReplace.m_nReplaceTarget = 0;	// �u���Ώ�
	dlgReplace.m_nPaste = false;		// �\��t����H
// To Here 2001.12.03 hor

	// �u���_�C�A���O�̕\��
	// From Here Jul. 2, 2001 genta �u���E�B���h�E��2�d�J����}�~
	if (!::IsWindow(dlgReplace.GetHwnd())) {
		dlgReplace.DoModeless(G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)m_pCommanderView, bSelected);
	}else {
		// �A�N�e�B�u�ɂ���
		ActivateFrameWindow(dlgReplace.GetHwnd());
		dlgReplace.SetItemText(IDC_COMBO_TEXT, cmemCurText.GetStringT());
	}
	// To Here Jul. 2, 2001 genta �u���E�B���h�E��2�d�J����}�~
	return;
}


/*! �u�����s
	
	@date 2002/04/08 �e�E�B���h�E���w�肷��悤�ɕύX�B
	@date 2003.05.17 ����� �����O�}�b�`�̖����u������Ȃ�
	@date 2011.12.18 Moca �I�v�V�����E�����L�[��DllShareData����m_cDlgReplace/EditView�x�[�X�ɕύX�B�����񒷐����̓P�p
*/
void ViewCommander::Command_REPLACE(HWND hwndParent)
{
	if (!hwndParent) {	// �e�E�B���h�E���w�肳��Ă��Ȃ���΁ACEditView���e�B
		hwndParent = m_pCommanderView->GetHwnd();
	}
	auto& dlgReplace = GetEditWindow()->m_cDlgReplace;
	// 2002.02.10 hor
	int nPaste			=	dlgReplace.m_nPaste;
	int nReplaceTarget	=	dlgReplace.m_nReplaceTarget;

	if (nPaste && nReplaceTarget == 3) {
		// �u���ΏہF�s�폜�̂Ƃ��́A�N���b�v�{�[�h����\��t���𖳌��ɂ���
		nPaste = FALSE;
	}

	// From Here 2001.12.03 hor
	if (nPaste && !GetDocument()->m_cDocEditor.IsEnablePaste()) {
		OkMessage(hwndParent, LS(STR_ERR_CEDITVIEW_CMD10));
		dlgReplace.CheckButton(IDC_CHK_PASTE, false);
		dlgReplace.EnableItem(IDC_COMBO_TEXT2, true);
		return;	// ���sreturn;
	}

	// 2002.01.09 hor
	// �I���G���A������΁A���̐擪�ɃJ�[�\�����ڂ�
	if (m_pCommanderView->GetSelectionInfo().IsTextSelected()) {
		if (m_pCommanderView->GetSelectionInfo().IsBoxSelecting()) {
			GetCaret().MoveCursor(GetSelect().GetFrom(), true);
		}else {
			Command_LEFT(false, false);
		}
	}
	// To Here 2002.01.09 hor
	
	// ��`�I���H
//			bBeginBoxSelect = m_pCommanderView->GetSelectionInfo().IsBoxSelecting();

	// �J�[�\�����ړ�
	//HandleCommand(F_LEFT, true, 0, 0, 0, 0);	//�H�H�H
	// To Here 2001.12.03 hor

	// �e�L�X�g�I������
	// ���݂̑I��͈͂��I����Ԃɖ߂�
	m_pCommanderView->GetSelectionInfo().DisableSelectArea(true);

	// 2004.06.01 Moca �������ɁA���̃v���Z�X�ɂ����m_aReplaceKeys�������������Ă����v�Ȃ悤��
	const NativeW	cMemRepKey(dlgReplace.m_strText2.c_str());

	// ��������
	Command_SEARCH_NEXT(true, true, false, hwndParent, NULL);

	BOOL	bRegularExp = m_pCommanderView->m_curSearchOption.bRegularExp;
	int 	nFlag       = m_pCommanderView->m_curSearchOption.bLoHiCase ? 0x01 : 0x00;

	// �e�L�X�g���I������Ă��邩
	if (m_pCommanderView->GetSelectionInfo().IsTextSelected()) {
		// From Here 2001.12.03 hor
		LayoutPoint ptTmp(0, 0);
		if (nPaste || !bRegularExp) {
			// ���K�\������ ����Q��($&)�Ŏ�������̂ŁA���K�\���͏��O
			if (nReplaceTarget == 1) {	// �}���ʒu�ֈړ�
				ptTmp = GetSelect().GetTo() - GetSelect().GetFrom();
				GetSelect().Clear(-1);
			}else if (nReplaceTarget == 2) {	// �ǉ��ʒu�ֈړ�
				// ���K�\�������O�����̂ŁA�u������̕��������s������玟�̍s�̐擪�ֈړ��v�̏������폜
				GetCaret().MoveCursor(GetSelect().GetTo(), false);
				GetSelect().Clear(-1);
			}else {
				// �ʒu�w��Ȃ��̂ŁA�������Ȃ�
			}
		}
		// �s�폜 �I��͈͂��s�S�̂Ɋg��B�J�[�\���ʒu���s����(���K�\���ł����s)
		if (nReplaceTarget == 3) {
			LogicPoint lineHome;
			GetDocument()->m_cLayoutMgr.LayoutToLogic(GetSelect().GetFrom(), &lineHome);
			lineHome.x = LogicXInt(0); // �s��
			LayoutRange selectFix;
			GetDocument()->m_cLayoutMgr.LogicToLayout(lineHome, selectFix.GetFromPointer());
			lineHome.y++; // ���s�̍s��
			GetDocument()->m_cLayoutMgr.LogicToLayout(lineHome, selectFix.GetToPointer());
			GetCaret().GetAdjustCursorPos(selectFix.GetToPointer());
			m_pCommanderView->GetSelectionInfo().SetSelectArea(selectFix);
			m_pCommanderView->GetSelectionInfo().DrawSelectArea();
			GetCaret().MoveCursor(selectFix.GetFrom(), false);
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
		// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
		// �e�L�X�g��\��t��
		if (nPaste) {
			Command_PASTE(0);
		} else if (nReplaceTarget == 3) {
			// �s�폜
			Command_INSTEXT( false, L"", LogicInt(0), TRUE );
		}else if (bRegularExp) { // �����^�u��  1==���K�\��
			// ��ǂ݂ɑΉ����邽�߂ɕ����s���܂ł��g���悤�ɕύX 2005/03/27 �����
			// 2002/01/19 novice ���K�\���ɂ�镶����u��
			Bregexp cRegexp;

			if (!InitRegexp(m_pCommanderView->GetHwnd(), cRegexp, true)) {
				return;	// ���sreturn;
			}

			// �����s�A�����s���A�����s�ł̌����}�b�`�ʒu
			const Layout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(GetSelect().GetFrom().GetY2());
			const wchar_t* pLine = pcLayout->GetDocLineRef()->GetPtr();
			LogicInt nIdx = m_pCommanderView->LineColumnToIndex(pcLayout, GetSelect().GetFrom().GetX2()) + pcLayout->GetLogicOffset();
			LogicInt nLen = pcLayout->GetDocLineRef()->GetLengthWithEOL();
			// ���K�\���őI���n�_�E�I�_�ւ̑}�����L�q
			// Jun. 6, 2005 �����
			// ������ł́u�����̌��̕��������s�������玟�̍s���ֈړ��v�������ł��Ȃ�
			// �� Oct. 30, �u�����̌��̕��������s��������E�E�v�̏�������߂�i�N������Ȃ��݂����Ȃ̂Łj
			// Nov. 9, 2005 ����� ���K�\���őI���n�_�E�I�_�ւ̑}�����@��ύX(��)
			NativeW cMemMatchStr; cMemMatchStr.SetString(L"$&");
			NativeW cMemRepKey2;
			if (nReplaceTarget == 1) {	// �I���n�_�֑}��
				cMemRepKey2 = cMemRepKey;
				cMemRepKey2 += cMemMatchStr;
			}else if (nReplaceTarget == 2) { // �I���I�_�֑}��
				cMemRepKey2 = cMemMatchStr;
				cMemRepKey2 += cMemRepKey;
			}else {
				cMemRepKey2 = cMemRepKey;
			}
			cRegexp.Compile(m_pCommanderView->m_strCurSearchKey.c_str(), cMemRepKey2.GetStringPtr(), nFlag);
			if (cRegexp.Replace(pLine, nLen, nIdx)) {
				// From Here Jun. 6, 2005 �����
				// �����s���܂�INSTEXT������@�́A�L�����b�g�ʒu�𒲐�����K�v������A
				// �L�����b�g�ʒu�̌v�Z�����G�ɂȂ�B�i�u����ɉ��s������ꍇ�ɕs������j
				// �����ŁAINSTEXT���镶���񒷂𒲐�������@�ɕύX����i���͂������̕����킩��₷���j
				LayoutMgr& rLayoutMgr = GetDocument()->m_cLayoutMgr;
				LogicInt matchLen = cRegexp.GetMatchLen();
				LogicInt nIdxTo = nIdx + matchLen;		// ����������̖���
				if (matchLen == 0) {
					// �O�����}�b�`�̎�(�����u���ɂȂ�Ȃ��悤�ɂP�����i�߂�)
					if (nIdxTo < nLen) {
						// 2005-09-02 D.S.Koba GetSizeOfChar
						nIdxTo += LogicInt(NativeW::GetSizeOfChar(pLine, nLen, nIdxTo) == 2 ? 2 : 1);
					}
					// �����u�����Ȃ��悤�ɁA�P�������₵���̂łP�����I���ɕύX
					// �I���n�_�E�I�_�ւ̑}���̏ꍇ���O�����}�b�`���͓���͓����ɂȂ�̂�
					rLayoutMgr.LogicToLayout(LogicPoint(nIdxTo, pcLayout->GetLogicLineNo()), GetSelect().GetToPointer());	// 2007.01.19 ryoji �s�ʒu���擾����
				}
				// �s�����猟�������񖖔��܂ł̕�����
				LogicInt colDiff = nLen - nIdxTo;
				// Oct. 22, 2005 Karoto
				// \r��u������Ƃ��̌���\n�������Ă��܂����̑Ή�
				if (colDiff < pcLayout->GetDocLineRef()->GetEol().GetLen()) {
					// ���s�ɂ������Ă�����A�s�S�̂�INSTEXT����B
					colDiff = LogicInt(0);
					rLayoutMgr.LogicToLayout(LogicPoint(nLen, pcLayout->GetLogicLineNo()), GetSelect().GetToPointer());	// 2007.01.19 ryoji �ǉ�
				}
				// �u���㕶����ւ̏�������(�s�����猟�������񖖔��܂ł̕���������)
				Command_INSTEXT(false, cRegexp.GetString(), cRegexp.GetStringLen() - colDiff, TRUE);
				// To Here Jun. 6, 2005 �����
			}
		}else {
			Command_INSTEXT(false, cMemRepKey.GetStringPtr(), cMemRepKey.GetStringLength(), TRUE);
		}

		// �}����̌����J�n�ʒu�𒲐�
		if (nReplaceTarget == 1) {
			GetCaret().SetCaretLayoutPos(GetCaret().GetCaretLayoutPos() + ptTmp);
		}

		// To Here 2001.12.03 hor
		/* �Ō�܂Œu����������OK�����܂Œu���O�̏�Ԃ��\�������̂ŁA
		** �u����A������������O�ɏ������� 2003.05.17 �����
		*/
		m_pCommanderView->Redraw();

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
	@date 2011.12.18 Moca �I�v�V�����E�����L�[��DllShareData����m_cDlgReplace/EditView�x�[�X�ɕύX�B�����񒷐����̓P�p
	@date 2013.05.10 Moca fastMode
*/
void ViewCommander::Command_REPLACE_ALL()
{

	// m_sSearchOption�I���̂��߂̐�ɓK�p
	if (!m_pCommanderView->ChangeCurRegexp()) {
		return;
	}

	auto& dlgReplace = GetEditWindow()->m_cDlgReplace;
	// 2002.02.10 hor
	BOOL nPaste			= dlgReplace.m_nPaste;
	BOOL nReplaceTarget	= dlgReplace.m_nReplaceTarget;
	bool bRegularExp	= m_pCommanderView->m_curSearchOption.bRegularExp;
	bool bSelectedArea	= dlgReplace.m_bSelectedArea;
	bool bConsecutiveAll = dlgReplace.m_bConsecutiveAll;	// �u���ׂĒu���v�͒u���̌J�Ԃ�	// 2007.01.16 ryoji
	if (nPaste && nReplaceTarget == 3) {
		// �u���ΏہF�s�폜�̂Ƃ��́A�N���b�v�{�[�h����\��t���𖳌��ɂ���
		nPaste = FALSE;
	}

	dlgReplace.m_bCanceled = false;
	dlgReplace.m_nReplaceCnt = 0;

	// From Here 2001.12.03 hor
	if (nPaste && !GetDocument()->m_cDocEditor.IsEnablePaste()) {
		OkMessage(m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD10));
		dlgReplace.CheckButton(IDC_CHK_PASTE, false);
		dlgReplace.EnableItem(IDC_COMBO_TEXT2, true);
		return;	// TRUE;
	}
	// To Here 2001.12.03 hor
	bool bBeginBoxSelect; // ��`�I���H
	if (m_pCommanderView->GetSelectionInfo().IsTextSelected()) {
		bBeginBoxSelect = m_pCommanderView->GetSelectionInfo().IsBoxSelecting();
	}else {
		bSelectedArea = false;
		bBeginBoxSelect = false;
	}

	// �\������ON/OFF
	bool bDisplayUpdate = false;

	const bool bDrawSwitchOld = m_pCommanderView->SetDrawSwitch(bDisplayUpdate);

	auto& layoutMgr = GetDocument()->m_cLayoutMgr;
	bool bFastMode = false;
	if (((Int)GetDocument()->m_cDocLineMgr.GetLineCount() * 10 < (Int)layoutMgr.GetLineCount())
		&& !(bSelectedArea || nPaste)
	) {
		// 1�s������10���C�A�E�g�s�ȏ�ŁA�I���E�y�[�X�g�łȂ��ꍇ
		bFastMode = true;
	}
	int	nAllLineNum; // $$�P�ʍ���
	if (bFastMode) {
		nAllLineNum = (Int)layoutMgr.GetLineCount();
	}else {
		nAllLineNum = (Int)GetDocument()->m_cDocLineMgr.GetLineCount();
	}
	int	nAllLineNumOrg = nAllLineNum;
	int	nAllLineNumLogicOrg = (Int)GetDocument()->m_cDocLineMgr.GetLineCount();

	// �i���\��&���~�_�C�A���O�̍쐬
	DlgCancel	cDlgCancel;
	HWND		hwndCancel = cDlgCancel.DoModeless(G_AppInstance(), m_pCommanderView->GetHwnd(), IDD_REPLACERUNNING);
	::EnableWindow(m_pCommanderView->GetHwnd(), FALSE);
	::EnableWindow(::GetParent(m_pCommanderView->GetHwnd()), FALSE);
	::EnableWindow(::GetParent(::GetParent(m_pCommanderView->GetHwnd())), FALSE);
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

	LayoutRange sRangeA;	// �I��͈�
	LogicPoint ptColLineP;

	// From Here 2001.12.03 hor
	if (bSelectedArea) {
		// �I��͈͒u��
		// �I��͈͊J�n�ʒu�̎擾
		sRangeA = GetSelect();

		// From Here 2007.09.20 genta ��`�͈͂̑I��u�����ł��Ȃ�
		// �����`�E��ƑI�������ꍇ�Cm_nSelectColumnTo < m_nSelectColumnFrom �ƂȂ邪�C
		// �͈̓`�F�b�N�� colFrom < colTo �����肵�Ă���̂ŁC
		// ��`�I���̏ꍇ�͍���`�E���w��ɂȂ�悤������ꊷ����D
		if (bBeginBoxSelect && sRangeA.GetTo().x < sRangeA.GetFrom().x)
			t_swap(sRangeA.GetFromPointer()->x, sRangeA.GetToPointer()->x);
		// To Here 2007.09.20 genta ��`�͈͂̑I��u�����ł��Ȃ�

		layoutMgr.LayoutToLogic(
			sRangeA.GetTo(),
			&ptColLineP
		);
		// �I��͈͊J�n�ʒu�ֈړ�
		GetCaret().MoveCursor(sRangeA.GetFrom(), bDisplayUpdate);
	}else {
		// �t�@�C���S�̒u��
		// �t�@�C���̐擪�Ɉړ�
	//	HandleCommand(F_GOFILETOP, bDisplayUpdate, 0, 0, 0, 0);
		Command_GOFILETOP(bDisplayUpdate);
	}

	LayoutPoint ptLast = GetCaret().GetCaretLayoutPos();
	LogicPoint ptLastLogic = GetCaret().GetCaretLogicPos();

	// �e�L�X�g�I������
	// ���݂̑I��͈͂��I����Ԃɖ߂�
	m_pCommanderView->GetSelectionInfo().DisableSelectArea(bDisplayUpdate);

	LogicRange cSelectLogic;	// �u��������GetSelect()��Logic�P�ʔ�
	// ��������
	Command_SEARCH_NEXT(true, bDisplayUpdate, true, 0, NULL, bFastMode ? &cSelectLogic : NULL);
	// To Here 2001.12.03 hor

	//<< 2002/03/26 Azumaiya
	// �������������Ƃ��ŗD��ɑg��ł݂܂����B
	// ���[�v�̊O�ŕ�����̒��������ł���̂ŁA�ꎞ�ϐ����B
	const wchar_t* szREPLACEKEY;		// �u���㕶����B
	bool		bColumnSelect = false;	// ��`�\��t�����s�����ǂ����B
	bool		bLineSelect = false;	// ���C�����[�h�\��t�����s�����ǂ���
	NativeW	cmemClip;				// �u���㕶����̃f�[�^�i�f�[�^���i�[���邾���ŁA���[�v���ł͂��̌`�ł̓f�[�^�������܂���j�B

	// �N���b�v�{�[�h����̃f�[�^�\��t�����ǂ����B
	if (nPaste != 0) {
		// �N���b�v�{�[�h����f�[�^���擾�B
		if (!m_pCommanderView->MyGetClipboardData(cmemClip, &bColumnSelect, GetDllShareData().m_common.m_sEdit.m_bEnableLineModePaste ? &bLineSelect : NULL)) {
			ErrorBeep();
			m_pCommanderView->SetDrawSwitch(bDrawSwitchOld);

			::EnableWindow(m_pCommanderView->GetHwnd(), TRUE);
			::EnableWindow(::GetParent(m_pCommanderView->GetHwnd()), TRUE);
			::EnableWindow(::GetParent(::GetParent(m_pCommanderView->GetHwnd())), TRUE);
			return;
		}

		// ��`�\��t����������Ă��āA�N���b�v�{�[�h�̃f�[�^����`�I���̂Ƃ��B
		if (GetDllShareData().m_common.m_sEdit.m_bAutoColumnPaste && bColumnSelect) {
			// �}�E�X�ɂ��͈͑I��
			if (m_pCommanderView->GetSelectionInfo().IsMouseSelecting()) {
				ErrorBeep();
				m_pCommanderView->SetDrawSwitch(bDrawSwitchOld);
				::EnableWindow(m_pCommanderView->GetHwnd(), TRUE);
				::EnableWindow(::GetParent(m_pCommanderView->GetHwnd()), TRUE);
				::EnableWindow(::GetParent(::GetParent(m_pCommanderView->GetHwnd())), TRUE);
				return;
			}

			// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
			if (!GetDllShareData().m_common.m_sView.m_bFontIs_FIXED_PITCH) {
				m_pCommanderView->SetDrawSwitch(bDrawSwitchOld);
				::EnableWindow(m_pCommanderView->GetHwnd(), TRUE);
				::EnableWindow(::GetParent(m_pCommanderView->GetHwnd()), TRUE);
				::EnableWindow(::GetParent(::GetParent(m_pCommanderView->GetHwnd())), TRUE);
				return;
			}
		}else { // �N���b�v�{�[�h����̃f�[�^�͕��ʂɈ����B
			bColumnSelect = false;
		}
	}else {
		// 2004.05.14 Moca �S�u���̓r���ő��̃E�B���h�E�Œu�������Ƃ܂����̂ŃR�s�[����
		cmemClip.SetString(dlgReplace.m_strText2.c_str());
	}

	LogicInt nREPLACEKEY;			// �u���㕶����̒����B
	szREPLACEKEY = cmemClip.GetStringPtr(&nREPLACEKEY);

	// �s�R�s�[�iMSDEVLineSelect�`���j�̃e�L�X�g�Ŗ��������s�ɂȂ��Ă��Ȃ���Ή��s��ǉ�����
	// �����C�A�E�g�܂�Ԃ��̍s�R�s�[�������ꍇ�͖��������s�ɂȂ��Ă��Ȃ�
	if (bLineSelect) {
		if (!WCODE::IsLineDelimiter(szREPLACEKEY[nREPLACEKEY - 1], GetDllShareData().m_common.m_sEdit.m_bEnableExtEol)) {
			cmemClip.AppendString(GetDocument()->m_cDocEditor.GetNewLineCode().GetValue2());
			szREPLACEKEY = cmemClip.GetStringPtr(&nREPLACEKEY);
		}
	}

	if (GetDllShareData().m_common.m_sEdit.m_bConvertEOLPaste) {
		LogicInt nConvertedTextLen = ConvertEol(szREPLACEKEY, nREPLACEKEY, NULL);
		std::vector<wchar_t> szConvertedText(nConvertedTextLen);
		wchar_t* pszConvertedText = &szConvertedText[0];
		ConvertEol(szREPLACEKEY, nREPLACEKEY, pszConvertedText);
		cmemClip.SetString(pszConvertedText, nConvertedTextLen);
		szREPLACEKEY = cmemClip.GetStringPtr(&nREPLACEKEY);
	}

	// �擾�ɃX�e�b�v�������肻���ȕϐ��Ȃǂ��A�ꎞ�ϐ�������B
	// �Ƃ͂����A�����̑�������邱�Ƃɂ���ē�������N���b�N���͍��킹�Ă� 1 ���[�v�Ő��\���Ǝv���܂��B
	// ���S�N���b�N�����[�v�̃I�[�_�[����l���Ă�����Ȃɓ��͂��Ȃ��悤�Ɏv���܂����ǁE�E�E�B
	bool& bCANCEL = cDlgCancel.m_bCANCEL;
	DocLineMgr& rDocLineMgr = GetDocument()->m_cDocLineMgr;

	// �N���X�֌W�����[�v�̒��Ő錾���Ă��܂��ƁA�����[�v���ƂɃR���X�g���N�^�A�f�X�g���N�^��
	// �Ă΂�Ēx���Ȃ�̂ŁA�����Ő錾�B
	Bregexp cRegexp;
	// �����������l�ɖ����[�v���Ƃɂ��ƒx���̂ŁA�ŏ��ɍς܂��Ă��܂��B
	if (bRegularExp && nPaste == 0) {
		if (!InitRegexp(m_pCommanderView->GetHwnd(), cRegexp, true)) {
			m_pCommanderView->SetDrawSwitch(bDrawSwitchOld);
			::EnableWindow(m_pCommanderView->GetHwnd(), TRUE);
			::EnableWindow(::GetParent(m_pCommanderView->GetHwnd()), TRUE);
			::EnableWindow(::GetParent(::GetParent(m_pCommanderView->GetHwnd())), TRUE);
			return;
		}

		// Nov. 9, 2005 ����� ���K�\���őI���n�_�E�I�_�ւ̑}�����@��ύX(��)
		NativeW cMemRepKey2;
		NativeW cMemMatchStr;
		cMemMatchStr.SetString(L"$&");
		if (nReplaceTarget == 1) {	// �I���n�_�֑}��
			cMemRepKey2 = cmemClip;
			cMemRepKey2 += cMemMatchStr;
		}else if (nReplaceTarget == 2) { // �I���I�_�֑}��
			cMemRepKey2 = cMemMatchStr;
			cMemRepKey2 += cmemClip;
		}else {
			cMemRepKey2 = cmemClip;
		}
		// ���K�\���I�v�V�����̐ݒ�2006.04.01 �����
		int nFlag = (m_pCommanderView->m_curSearchOption.bLoHiCase ? Bregexp::optCaseSensitive : Bregexp::optNothing);
		nFlag |= (bConsecutiveAll ? Bregexp::optNothing : Bregexp::optGlobal);	// 2007.01.16 ryoji
		cRegexp.Compile(m_pCommanderView->m_strCurSearchKey.c_str(), cMemRepKey2.GetStringPtr(), nFlag);
	}

	//$$ �P�ʍ���
	LayoutPoint	ptOld;						// ������̑I��͈�
	/*LogicInt*/int		lineCnt = 0;		// �u���O�̍s��
	/*LayoutInt*/int		linDif = (0);		// �u����̍s����
	/*LayoutInt*/int		colDif = (0);		// �u����̌�����
	/*LayoutInt*/int		linPrev = (0);		// �O��̌����s(��`) @@@2001.12.31 YAZAKI warning�ގ�
	/*LogicInt*/int		linOldLen = (0);	// ������̍s�̒���
	/*LayoutInt*/int		linNext;			// ����̌����s(��`)

	int nLoopCnt = -1;

	// �e�L�X�g���I������Ă��邩
	while (
		(!bFastMode && m_pCommanderView->GetSelectionInfo().IsTextSelected())
		|| (bFastMode && cSelectLogic.IsValid())
	) {
		// �L�����Z�����ꂽ��
		if (bCANCEL) {
			break;
		}

		// �������̃��[�U�[������\�ɂ���
		if (!::BlockingHook(hwndCancel)) {
			m_pCommanderView->SetDrawSwitch(bDrawSwitchOld);
			::EnableWindow(m_pCommanderView->GetHwnd(), TRUE);
			::EnableWindow(::GetParent(m_pCommanderView->GetHwnd()), TRUE);
			::EnableWindow(::GetParent(::GetParent(m_pCommanderView->GetHwnd())), TRUE);
			return;// -1;
		}

		++nLoopCnt;
		// 128 ���Ƃɕ\���B
		if ((nLoopCnt & 0x7F) == 0) {
			// ���Ԃ��Ƃɐi���󋵕`�悾�Ǝ��Ԏ擾���x���Ȃ�Ǝv�����A������̕������R���Ǝv���̂ŁE�E�E�B
			// �Ǝv�������ǁA�t�ɂ�����̕������R�ł͂Ȃ��̂ŁA��߂�B
		
			if (bFastMode) {
				int nDiff = nAllLineNumOrg - (Int)GetDocument()->m_cDocLineMgr.GetLineCount();
				if (0 <= nDiff) {
					nNewPos = (nDiff + (Int)cSelectLogic.GetFrom().GetY2()) >> nShiftCount;
				}else {
					nNewPos = ::MulDiv((Int)cSelectLogic.GetFrom().GetY(), nAllLineNum, (Int)layoutMgr.GetLineCount());
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
					(sRangeA.GetTo().y + linDif == ptOld.y && sRangeA.GetTo().GetX2() + colDif < ptOld.x)
					|| (sRangeA.GetTo().y + linDif <  ptOld.y)
				) {
					break;
				}
				// ���͔͈͓��H
				if (!(sRangeA.GetFrom().x <= GetSelect().GetFrom().x && ptOld.GetX2() <= sRangeA.GetTo().GetX2() + colDif)) {
					if (ptOld.x < sRangeA.GetTo().GetX2() + colDif) {
						linNext = (Int)GetSelect().GetTo().GetY2();
					}else {
						linNext = (Int)GetSelect().GetTo().GetY2() + 1;
					}
					// ���̌����J�n�ʒu�փV�t�g
					GetCaret().SetCaretLayoutPos(LayoutPoint(sRangeA.GetFrom().x, LayoutInt(linNext)));
					// 2004.05.30 Moca ���݂̌�����������g���Č�������
					Command_SEARCH_NEXT(false, bDisplayUpdate, true, 0, NULL);
					colDif = (0);
					continue;
				}
			}else {
				// ���ʂ̑I��
				// o �������W���`�F�b�N���Ȃ���u������
				//
			
				// �������̍s�����L��
				lineCnt = rDocLineMgr.GetLineCount();

				// ������͈̔͏I�[
				LogicPoint ptOldTmp;
				if (bFastMode) {
					ptOldTmp = cSelectLogic.GetTo();
				}else {
					layoutMgr.LayoutToLogic(
						GetSelect().GetTo(),
						&ptOldTmp
					);
				}
				ptOld.x = (LayoutInt)ptOldTmp.x; //$$ ���C�A�E�g�^�ɖ�����胍�W�b�N�^�����B�C��������
				ptOld.y = (LayoutInt)ptOldTmp.y;

				// �u���O�̍s�̒���(���s�͂P�����Ɛ�����)��ۑ����Ă����āA�u���O��ōs�ʒu���ς�����ꍇ�Ɏg�p
				linOldLen = rDocLineMgr.GetLine(ptOldTmp.GetY2())->GetLengthWithoutEOL() + LogicInt(1);

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

		if (nPaste || !bRegularExp) {
			// ���K�\������ ����Q��($&)�Ŏ�������̂ŁA���K�\���͏��O
			if (nReplaceTarget == 1) {	// �}���ʒu�Z�b�g
				if (bFastMode) {
					ptTmpLogic.x = cSelectLogic.GetTo().x - cSelectLogic.GetFrom().x;
					ptTmpLogic.y = cSelectLogic.GetTo().y - cSelectLogic.GetFrom().y;
					cSelectLogic.SetTo(cSelectLogic.GetFrom());
				}else {
					ptTmp.x = GetSelect().GetTo().x - GetSelect().GetFrom().x;
					ptTmp.y = GetSelect().GetTo().y - GetSelect().GetFrom().y;
					GetSelect().Clear(-1);
				}
			}else if (nReplaceTarget == 2) {	// �ǉ��ʒu�Z�b�g
				// ���K�\�������O�����̂ŁA�u������̕��������s������玟�̍s�̐擪�ֈړ��v�̏������폜
				if (bFastMode) {
					GetCaret().MoveCursorFastMode(cSelectLogic.GetTo());
					cSelectLogic.SetFrom(cSelectLogic.GetTo());
				}else {
					GetCaret().MoveCursor(GetSelect().GetTo(), false);
					GetSelect().Clear(-1);
				}
		    }else {
				// �ʒu�w��Ȃ��̂ŁA�������Ȃ�
			}
		}
		// �s�폜 �I��͈͂��s�S�̂Ɋg��B�J�[�\���ʒu���s����(���K�\���ł����s)
		if (nReplaceTarget == 3) {
			if (bFastMode) {
				const LogicInt y = cSelectLogic.GetFrom().y;
				cSelectLogic.SetFrom(LogicPoint(LogicXInt(0), y)); // �s��
				cSelectLogic.SetTo(LogicPoint(LogicXInt(0), y + LogicInt(1))); // ���s�̍s��
				if (GetDocument()->m_cDocLineMgr.GetLineCount() == y + LogicInt(1)) {
					const DocLine* pLine = GetDocument()->m_cDocLineMgr.GetLine(y);
					if (pLine->GetEol() == EOL_NONE) {
						// EOF�͍ŏI�f�[�^�s�ɂԂ牺����Ȃ̂ŁA�I���I�[�͍s��
						cSelectLogic.SetTo(LogicPoint(pLine->GetLengthWithEOL(), y)); // �Ώۍs�̍s��
					}
				}
				GetCaret().MoveCursorFastMode(cSelectLogic.GetFrom());
			}else {
				LogicPoint lineHome;
				GetDocument()->m_cLayoutMgr.LayoutToLogic(GetSelect().GetFrom(), &lineHome);
				lineHome.x = LogicXInt(0); // �s��
				LayoutRange selectFix;
				GetDocument()->m_cLayoutMgr.LogicToLayout(lineHome, selectFix.GetFromPointer());
				lineHome.y++; // ���s�̍s��
				GetDocument()->m_cLayoutMgr.LogicToLayout(lineHome, selectFix.GetToPointer());
				GetCaret().GetAdjustCursorPos(selectFix.GetToPointer());
				m_pCommanderView->GetSelectionInfo().SetSelectArea(selectFix);
				GetCaret().MoveCursor(selectFix.GetFrom(), false);
			}
		}


		// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
		// �e�L�X�g��\��t��
		if (nPaste) {
			if (!bColumnSelect) {
				/* �{���� Command_INSTEXT ���g���ׂ��Ȃ�ł��傤���A���ʂȏ���������邽�߂ɒ��ڂ������B
				** ��m_nSelectXXX��-1�̎��� m_pCommanderView->ReplaceData_CEditView�𒼐ڂ������Ɠ���s�ǂƂȂ邽��
				**   ���ڂ������̂�߂��B2003.05.18 by �����
				*/
				Command_INSTEXT(false, szREPLACEKEY, nREPLACEKEY, TRUE, bLineSelect);
			}else {
				Command_PASTEBOX(szREPLACEKEY, nREPLACEKEY);
				// 2013.06.11 �ĕ`�悵�Ȃ��悤��
				// �ĕ`����s��Ȃ��Ƃǂ�Ȍ��ʂ��N���Ă���̂������炸�݂��Ƃ��Ȃ��̂ŁE�E�E�B
				// m_pCommanderView->AdjustScrollBars(); // 2007.07.22 ryoji
				// m_pCommanderView->Redraw();
			}
			++nReplaceNum;
		}else if (nReplaceTarget == 3) {
			Command_INSTEXT( false, L"", LogicInt(0), true, false, bFastMode, bFastMode ? &cSelectLogic : NULL );
			++nReplaceNum;
		}else if (bRegularExp) { // �����^�u��  1==���K�\��
			// 2002/01/19 novice ���K�\���ɂ�镶����u��
			// �����s�A�����s���A�����s�ł̌����}�b�`�ʒu
			const DocLine* pcDocLine;
			const wchar_t* pLine;
			LogicInt nLogicLineNum;
			LogicInt nIdx;
			LogicInt nLen;
			if (bFastMode) {
				pcDocLine = rDocLineMgr.GetLine(cSelectLogic.GetFrom().GetY2());
				pLine = pcDocLine->GetPtr();
				nLogicLineNum = cSelectLogic.GetFrom().GetY2();
				nIdx = cSelectLogic.GetFrom().GetX2();
				nLen = pcDocLine->GetLengthWithEOL();
			}else {
				const Layout* pcLayout = layoutMgr.SearchLineByLayoutY(GetSelect().GetFrom().GetY2());
				pcDocLine = pcLayout->GetDocLineRef();
				pLine = pcDocLine->GetPtr();
				nLogicLineNum = pcLayout->GetLogicLineNo();
				nIdx = m_pCommanderView->LineColumnToIndex(pcLayout, GetSelect().GetFrom().GetX2()) + pcLayout->GetLogicOffset();
				nLen = pcDocLine->GetLengthWithEOL();
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
							LayoutPoint(sRangeA.GetTo().x, ptOld.y),
							&ptWork
						);
						ptColLineP.x = ptWork.x;
						if (nLen - pcDocLine->GetEol().GetLen() > ptColLineP.x + colDif)
							nLen = ptColLineP.GetX2() + LogicInt(colDif);
					}else {	// �ʏ�̑I��
						if (ptColLineP.y + linDif == (Int)ptOld.y) { //$$ �P�ʍ���
							if (nLen - pcDocLine->GetEol().GetLen() > ptColLineP.x + colDif)
								nLen = ptColLineP.GetX2() + LogicInt(colDif);
						}
					}
				}

				if (pcDocLine->GetLengthWithoutEOL() < nLen)
					ptOld.x = (LayoutInt)(Int)pcDocLine->GetLengthWithoutEOL() + 1; //$$ �P�ʍ���
				else
					ptOld.x = (LayoutInt)(Int)nLen; //$$ �P�ʍ���
			}

			if (int nReplace = cRegexp.Replace(pLine, nLen, nIdx)) {
				nReplaceNum += nReplace;
				if (!bConsecutiveAll) { // 2006.04.01 �����	// 2007.01.16 ryoji
					// �s�P�ʂł̒u������
					// �I��͈͂𕨗��s���܂łɂ̂΂�
					if (bFastMode) {
						cSelectLogic.SetTo(LogicPoint(nLen, nLogicLineNum));
					}else {
						layoutMgr.LogicToLayout(LogicPoint(nLen, nLogicLineNum), GetSelect().GetToPointer());
					}
				}else {
				    // From Here Jun. 6, 2005 �����
				    // �����s���܂�INSTEXT������@�́A�L�����b�g�ʒu�𒲐�����K�v������A
				    // �L�����b�g�ʒu�̌v�Z�����G�ɂȂ�B�i�u����ɉ��s������ꍇ�ɕs������j
				    // �����ŁAINSTEXT���镶���񒷂𒲐�������@�ɕύX����i���͂������̕����킩��₷���j
				    LogicInt matchLen = cRegexp.GetMatchLen();
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
							cSelectLogic.SetTo(LogicPoint(nIdxTo, nLogicLineNum));
						}else {
							layoutMgr.LogicToLayout(LogicPoint(nIdxTo, nLogicLineNum), GetSelect().GetToPointer());	// 2007.01.19 ryoji �s�ʒu���擾����
						}
				    }
				    // �s�����猟�������񖖔��܂ł̕�����
					colDiff =  nLen - nIdxTo;
					ptOld.x = (LayoutInt)(Int)nIdxTo;	// 2007.01.19 ryoji �ǉ�  // $$ �P�ʍ���
				    // Oct. 22, 2005 Karoto
				    // \r��u������Ƃ��̌���\n�������Ă��܂����̑Ή�
				    if (colDiff < pcDocLine->GetEol().GetLen()) {
					    // ���s�ɂ������Ă�����A�s�S�̂�INSTEXT����B
					    colDiff = LogicInt(0);
						if (bFastMode) {
							cSelectLogic.SetTo(LogicPoint(nLen, nLogicLineNum));
						}else {
							layoutMgr.LogicToLayout(LogicPoint(nLen, nLogicLineNum), GetSelect().GetToPointer());	// 2007.01.19 ryoji �ǉ�
						}
						ptOld.x = (LayoutInt)(Int)pcDocLine->GetLengthWithoutEOL() + 1;	// 2007.01.19 ryoji �ǉ� //$$ �P�ʍ���
				    }
				}
				// �u���㕶����ւ̏�������(�s�����猟�������񖖔��܂ł̕���������)
				Command_INSTEXT(false, cRegexp.GetString(), cRegexp.GetStringLen() - colDiff, true, false, bFastMode, bFastMode ? &cSelectLogic : NULL);
				// To Here Jun. 6, 2005 �����
			}
		}else {
			/* �{���͌��R�[�h���g���ׂ��Ȃ�ł��傤���A���ʂȏ���������邽�߂ɒ��ڂ������B
			** ��m_nSelectXXX��-1�̎��� m_pCommanderView->ReplaceData_CEditView�𒼐ڂ������Ɠ���s�ǂƂȂ邽�ߒ��ڂ������̂�߂��B2003.05.18 �����
			*/
			Command_INSTEXT(false, szREPLACEKEY, nREPLACEKEY, true, false, bFastMode, bFastMode ? &cSelectLogic : NULL);
			++nReplaceNum;
		}

		// �}����̈ʒu����
		if (nReplaceTarget == 1) {
			if (bFastMode) {
				GetCaret().SetCaretLogicPos(GetCaret().GetCaretLogicPos() + ptTmpLogic);
			}else {
				GetCaret().SetCaretLayoutPos(GetCaret().GetCaretLayoutPos() + ptTmp);
				if (!bBeginBoxSelect) {
					LogicPoint p;
					layoutMgr.LayoutToLogic(
						GetCaret().GetCaretLayoutPos(),
						&p
					);
					GetCaret().SetCaretLogicPos(p);
				}
			}
		}

		if (!bFastMode && 50 <= nReplaceNum && !(bSelectedArea || nPaste)) {
			bFastMode = true;
			nAllLineNum = (Int)GetDocument()->m_cDocLineMgr.GetLineCount();
			nAllLineNumOrg = nAllLineNumLogicOrg;
			for (nShiftCount=0; 300<nAllLineNum; ++nShiftCount) {
				nAllLineNum /= 2;
			}
			Progress_SetRange( hwndProgress, 0, nAllLineNum + 1 );
			int nDiff = nAllLineNumOrg - (Int)GetDocument()->m_cDocLineMgr.GetLineCount();
			if (0 <= nDiff) {
				nNewPos = (nDiff + (Int)cSelectLogic.GetFrom().GetY2()) >> nShiftCount;
			}else {
				nNewPos = ::MulDiv((Int)cSelectLogic.GetFrom().GetY(), nAllLineNum, (Int)GetDocument()->m_cDocLineMgr.GetLineCount());
			}
			Progress_SetPos( hwndProgress, nNewPos +1 );
			Progress_SetPos( hwndProgress, nNewPos );
		}
		// �Ō�ɒu�������ʒu���L��
		if (bFastMode) {
			ptLastLogic = GetCaret().GetCaretLogicPos();
		}else {
			ptLast = GetCaret().GetCaretLayoutPos();
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
				LogicPoint ptTmp2 = GetCaret().GetCaretLogicPos();
				int linDif_thistime = rDocLineMgr.GetLineCount() - lineCnt;	// ����u���ł̍s���ω�
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
		Command_SEARCH_NEXT(false, bDisplayUpdate, true, 0, NULL, bFastMode ? &cSelectLogic : NULL);
	}

	if (bFastMode) {
		if (0 < nReplaceNum) {
			// LayoutMgr�̍X�V(�ύX�L�̏ꍇ)
			layoutMgr._DoLayout();
			GetEditWindow()->ClearViewCaretPosInfo();
			if (GetDocument()->m_nTextWrapMethodCur == (int)eTextWrappingMethod::NoWrapping) {
				layoutMgr.CalculateTextWidth();
			}
		}
		layoutMgr.LogicToLayout(ptLastLogic, &ptLast);
		GetCaret().MoveCursor(ptLast, true);
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();	// 2009.07.25 ryoji
	}
	//>> 2002/03/26 Azumaiya

	_itot(nReplaceNum, szLabel, 10);
	::SendMessage(hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel);

	if (!cDlgCancel.IsCanceled()) {
		nNewPos = nAllLineNum;
		Progress_SetPos(hwndProgress, nNewPos + 1);
		Progress_SetPos(hwndProgress, nNewPos);
	}
	cDlgCancel.CloseDialog(0);
	::EnableWindow(m_pCommanderView->GetHwnd(), TRUE);
	::EnableWindow(::GetParent(m_pCommanderView->GetHwnd()), TRUE);
	::EnableWindow(::GetParent(::GetParent(m_pCommanderView->GetHwnd())), TRUE);

	// From Here 2001.12.03 hor

	// �e�L�X�g�I������
	m_pCommanderView->GetSelectionInfo().DisableSelectArea(false);

	// �J�[�\���E�I��͈͕���
	if (
		!bSelectedArea			// �t�@�C���S�̒u��
		|| cDlgCancel.IsCanceled()
	) {		// �L�����Z�����ꂽ
		// �Ō�ɒu������������̉E��
		if (!bFastMode) {
			GetCaret().MoveCursor(ptLast, true);
		}
	}else {
		if (bBeginBoxSelect) {
			// ��`�I��
			m_pCommanderView->GetSelectionInfo().SetBoxSelect(bBeginBoxSelect);
			sRangeA.GetToPointer()->y += linDif;
			if (sRangeA.GetTo().y < 0) sRangeA.SetToY(LayoutInt(0));
		}else {
			// ���ʂ̑I��
			ptColLineP.x += colDif;
			if (ptColLineP.x < 0) ptColLineP.x = 0;
			ptColLineP.y += linDif;
			if (ptColLineP.y < 0) ptColLineP.y = 0;
			layoutMgr.LogicToLayout(
				ptColLineP,
				sRangeA.GetToPointer()
			);
		}
		if (sRangeA.GetFrom().y<sRangeA.GetTo().y || sRangeA.GetFrom().x<sRangeA.GetTo().x) {
			m_pCommanderView->GetSelectionInfo().SetSelectArea(sRangeA);	// 2009.07.25 ryoji
		}
		GetCaret().MoveCursor(sRangeA.GetTo(), true);
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();	// 2009.07.25 ryoji
	}
	// To Here 2001.12.03 hor

	dlgReplace.m_bCanceled = cDlgCancel.IsCanceled();
	dlgReplace.m_nReplaceCnt = nReplaceNum;
	m_pCommanderView->SetDrawSwitch(bDrawSwitchOld);
	ActivateFrameWindow(GetMainWindow());
}


// �����}�[�N�̐ؑւ�	// 2001.12.03 hor �N���A �� �ؑւ� �ɕύX
void ViewCommander::Command_SEARCH_CLEARMARK(void)
{
// From Here 2001.12.03 hor

	// �����}�[�N�̃Z�b�g

	if (m_pCommanderView->GetSelectionInfo().IsTextSelected()) {

		// ����������擾
		NativeW	cmemCurText;
		m_pCommanderView->GetCurrentTextForSearch(cmemCurText, false);
		auto& csSearch = GetDllShareData().m_common.m_sSearch;

		m_pCommanderView->m_strCurSearchKey = cmemCurText.GetStringPtr();
		if (m_pCommanderView->m_nCurSearchKeySequence < csSearch.m_nSearchKeySequence) {
			m_pCommanderView->m_curSearchOption = csSearch.m_searchOption;
		}
		m_pCommanderView->m_curSearchOption.bRegularExp = false;	// ���K�\���g��Ȃ�
		m_pCommanderView->m_curSearchOption.bWordOnly = false;		// �P��Ō������Ȃ�

		// ���L�f�[�^�֓o�^
		if (cmemCurText.GetStringLength() < _MAX_PATH) {
			SearchKeywordManager().AddToSearchKeyArr(cmemCurText.GetStringPtr());
			csSearch.m_searchOption = m_pCommanderView->m_curSearchOption;
		}
		m_pCommanderView->m_nCurSearchKeySequence = csSearch.m_nSearchKeySequence;
		m_pCommanderView->m_bCurSearchUpdate = true;

		m_pCommanderView->ChangeCurRegexp(false); // 2002.11.11 Moca ���K�\���Ō���������C�F�������ł��Ă��Ȃ�����

		// �ĕ`��
		m_pCommanderView->RedrawAll();
		return;
	}
// To Here 2001.12.03 hor

	// �����}�[�N�̃N���A

	m_pCommanderView->m_bCurSrchKeyMark = false;	// ����������̃}�[�N
	// �t�H�[�J�X�ړ����̍ĕ`��
	m_pCommanderView->RedrawAll();
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
	if (m_pCommanderView->SearchBracket(GetCaret().GetCaretLayoutPos(), &ptColLine, &mode)) {	// 02/09/18 ai
		// 2005.06.24 Moca
		// 2006.07.09 genta �\���X�V�R��F�V�K�֐��ɂđΉ�
		m_pCommanderView->MoveCursorSelecting(ptColLine, m_pCommanderView->GetSelectionInfo().m_bSelectingLock);
	}else {
		// ���s�����ꍇ�� nCol/nLine�ɂ͗L���Ȓl�������Ă��Ȃ�.
		// �������Ȃ�
	}
}

