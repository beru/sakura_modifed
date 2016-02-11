/*!	@file
@brief ViewCommander�N���X�̃R�}���h(�W�����v&�u�b�N�}�[�N)�֐��Q

	2012/12/17	CViewCommander.cpp,CViewCommander_New.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, hor, YAZAKI, MIK
	Copyright (C) 2006, genta

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"
#include "docplus/CFuncListManager.h"


// from CViewCommander_New.cpp
/*!	�����J�n�ʒu�֖߂�
	@author	ai
	@date	02/06/26
*/
void ViewCommander::Command_JUMP_SRCHSTARTPOS(void)
{
	if (m_pCommanderView->m_ptSrchStartPos_PHY.BothNatural()) {
		LayoutPoint pt;
		// �͈͑I�𒆂�
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			m_pCommanderView->m_ptSrchStartPos_PHY,
			&pt
		);
		// 2006.07.09 genta �I����Ԃ�ۂ�
		m_pCommanderView->MoveCursorSelecting(pt, m_pCommanderView->GetSelectionInfo().m_bSelectingLock);
	}else {
		ErrorBeep();
	}
	return;
}


/*! �w��s�փW�����v�_�C�A���O�̕\��
	2002.2.2 YAZAKI
*/
void ViewCommander::Command_JUMP_DIALOG(void)
{
	if (!GetEditWindow()->m_cDlgJump.DoModal(
			G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)GetDocument()
		)
	) {
		return;
	}
}


// �w��s�w�W�����v
void ViewCommander::Command_JUMP(void)
{
	auto& layoutMgr = GetDocument()->m_cLayoutMgr;
	if (layoutMgr.GetLineCount() == 0) {
		ErrorBeep();
		return;
	}

	int nMode;
	int bValidLine;
	int nCurrentLine;
	int nCommentBegin = 0;

	auto& dlgJump = GetEditWindow()->m_cDlgJump;

	// �s�ԍ�
	int	nLineNum; //$$ �P�ʍ���
	nLineNum = dlgJump.m_nLineNum;

	if (!dlgJump.m_bPLSQL) {	// PL/SQL�\�[�X�̗L���s��
		// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
		if (GetDllShareData().m_bLineNumIsCRLF_ForJump) {
			if (LogicInt(0) >= nLineNum) {
				nLineNum = LogicInt(1);
			}
			/*
			  �J�[�\���ʒu�ϊ�
			  ���W�b�N�ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
			  ��
			  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
			*/
			LayoutPoint ptPosXY;
			layoutMgr.LogicToLayout(
				LogicPoint(0, nLineNum - 1),
				&ptPosXY
			);
			nLineNum = (Int)ptPosXY.y + 1;
		}else {
			if (0 >= nLineNum) {
				nLineNum = 1;
			}
			if (nLineNum > layoutMgr.GetLineCount()) {
				nLineNum = (Int)layoutMgr.GetLineCount();
			}
		}
		// Sep. 8, 2000 genta
		m_pCommanderView->AddCurrentLineToHistory();
		// 2006.07.09 genta �I����Ԃ��������Ȃ��悤��
		m_pCommanderView->MoveCursorSelecting(
			LayoutPoint(0, nLineNum - 1),
			m_pCommanderView->GetSelectionInfo().m_bSelectingLock,
			_CARETMARGINRATE / 3
		);
		return;
	}
	if (0 >= nLineNum) {
		nLineNum = 1;
	}
	nMode = 0;
	nCurrentLine = dlgJump.m_nPLSQL_E2 - 1;

	int	nLineCount; //$$ �P�ʍ���
	nLineCount = dlgJump.m_nPLSQL_E1 - 1;

	// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
	if (!m_pCommanderView->m_pTypeData->m_bLineNumIsCRLF) { // ���C�A�E�g�P��
		/*
		  �J�[�\���ʒu�ϊ�
		  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		  ��
		  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		*/
		LogicPoint ptPosXY;
		layoutMgr.LayoutToLogic(
			LayoutPoint(0, nLineCount),
			&ptPosXY
		);
		nLineCount = ptPosXY.y;
	}

	auto& lineMgr = GetDocument()->m_docLineMgr;
	for (; nLineCount<lineMgr.GetLineCount(); ++nLineCount) {
		LogicInt nLineLen;
		const wchar_t* pLine = lineMgr.GetLine(LogicInt(nLineCount))->GetDocLineStrWithEOL(&nLineLen);
		bValidLine = FALSE;
		LogicInt i;
		for (i=LogicInt(0); i<nLineLen; ++i) {
			wchar_t let = pLine[i];
			if (1
				&& let != L' '
				&& let != WCODE::TAB
			) {
				break;
			}
		}
		LogicInt nBgn = i;
		wchar_t let = 0;
		wchar_t prevLet;
		for (i=nBgn; i<nLineLen; ++i) {
			// �V���O���N�H�[�e�[�V����������ǂݍ��ݒ�
			prevLet = let;
			let = pLine[i];
			if (nMode == 20) {
				bValidLine = TRUE;
				if (let == L'\'') {
					if (i > 0 && L'\\' == prevLet) {
					}else {
						nMode = 0;
						continue;
					}
				}else {
				}
			}else
			// �_�u���N�H�[�e�[�V����������ǂݍ��ݒ�
			if (nMode == 21) {
				bValidLine = TRUE;
				if (let == L'"') {
					if (i > 0 && L'\\' == prevLet) {
					}else {
						nMode = 0;
						continue;
					}
				}else {
				}
			}else
			// �R�����g�ǂݍ��ݒ�
			if (nMode == 8) {
				if (i < nLineLen - 1 && let == L'*' &&  L'/' == pLine[i + 1]) {
					if (/*nCommentBegin != nLineCount &&*/ nCommentBegin != 0) {
						bValidLine = TRUE;
					}
					++i;
					nMode = 0;
					continue;
				}else {
				}
			}else
			// �m�[�}�����[�h
			if (nMode == 0) {
				// �󔒂�^�u�L�������΂�
				if (0
					|| let == L'\t'
					|| let == L' '
					|| WCODE::IsLineDelimiter(pLine[i], GetDllShareData().m_common.m_sEdit.m_bEnableExtEol)
				) {
					continue;
				}else
				if (i < nLineLen - 1 && let == L'-' &&  L'-' == pLine[i + 1]) {
					bValidLine = TRUE;
					break;
				}else
				if (i < nLineLen - 1 && let == L'/' &&  L'*' == pLine[i + 1]) {
					++i;
					nMode = 8;
					nCommentBegin = nLineCount;
					continue;
				}else
				if (let == L'\'') {
					nMode = 20;
					continue;
				}else
				if (let == L'"') {
					nMode = 21;
					continue;
				}else {
					bValidLine = TRUE;
				}
			}
		}
		// �R�����g�ǂݍ��ݒ�
		if (nMode == 8) {
			if (nCommentBegin != 0) {
				bValidLine = TRUE;
			}
			// �R�����g�u���b�N���̉��s�����̍s
			if (WCODE::IsLineDelimiter(pLine[nBgn], GetDllShareData().m_common.m_sEdit.m_bEnableExtEol)) {
				bValidLine = FALSE;
			}
		}
		if (bValidLine) {
			++nCurrentLine;
			if (nCurrentLine >= nLineNum) {
				break;
			}
		}
	}
	/*
	  �J�[�\���ʒu�ϊ�
	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	  ��
	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	*/
	LayoutPoint ptPos;
	layoutMgr.LogicToLayout(
		LogicPoint(0, nLineCount),
		&ptPos
	);
	// Sep. 8, 2000 genta
	m_pCommanderView->AddCurrentLineToHistory();
	// 2006.07.09 genta �I����Ԃ��������Ȃ��悤��
	m_pCommanderView->MoveCursorSelecting(ptPos, m_pCommanderView->GetSelectionInfo().m_bSelectingLock, _CARETMARGINRATE / 3);
}


// from CViewCommander_New.cpp
// �u�b�N�}�[�N�̐ݒ�E�������s��(�g�O������)
void ViewCommander::Command_BOOKMARK_SET(void)
{
	DocLine* pCDocLine;
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	auto& sSelect = selInfo.m_sSelect;
	auto& lineMgr = GetDocument()->m_docLineMgr;
	if (selInfo.IsTextSelected()
		&& sSelect.GetFrom().y < sSelect.GetTo().y
	) {
		LogicPoint ptFrom;
		LogicPoint ptTo;
		auto& layoutMgr = GetDocument()->m_cLayoutMgr;
		layoutMgr.LayoutToLogic(
			LayoutPoint(LayoutInt(0), sSelect.GetFrom().y),
			&ptFrom
		);
		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			LayoutPoint(LayoutInt(0), sSelect.GetTo().y),
			&ptTo
		);
		for (LogicInt nY=ptFrom.GetY2(); nY<=ptTo.y; ++nY) {
			pCDocLine = lineMgr.GetLine(nY);
			BookmarkSetter cBookmark(pCDocLine);
			if (pCDocLine) cBookmark.SetBookmark(!cBookmark.IsBookmarked());
		}
	}else {
		pCDocLine = lineMgr.GetLine(GetCaret().GetCaretLogicPos().GetY2());
		BookmarkSetter cBookmark(pCDocLine);
		if (pCDocLine) cBookmark.SetBookmark(!cBookmark.IsBookmarked());
	}

	// 2002.01.16 hor ���������r���[���X�V
	GetEditWindow()->Views_Redraw();
}


// from CViewCommander_New.cpp
// ���̃u�b�N�}�[�N��T���C����������ړ�����
void ViewCommander::Command_BOOKMARK_NEXT(void)
{
	int		nYOld;				// hor
	BOOL	bFound	=	FALSE;	// hor
	BOOL	bRedo	=	TRUE;	// hor

	LogicPoint	ptXY(0, GetCaret().GetCaretLogicPos().y);
	LogicInt tmp_y;

	nYOld = ptXY.y;					// hor

re_do:;								// hor
	if (BookmarkManager(&GetDocument()->m_docLineMgr).SearchBookMark(ptXY.GetY2(), SearchDirection::Forward, &tmp_y)) {
		ptXY.y = tmp_y;
		bFound = TRUE;
		LayoutPoint ptLayout;
		GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY, &ptLayout);
		// 2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
		m_pCommanderView->MoveCursorSelecting(ptLayout, m_pCommanderView->GetSelectionInfo().m_bSelectingLock);
	}
    // 2002.01.26 hor
	if (GetDllShareData().m_common.m_sSearch.m_bSearchAll) {
		if (!bFound	&&		// ������Ȃ�����
			bRedo			// �ŏ��̌���
		) {
			ptXY.y = -1;	// 2002/06/01 MIK
			bRedo = FALSE;
			goto re_do;		// �擪����Č���
		}
	}
	if (bFound) {
		if (nYOld >= ptXY.y) {
			m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT1));
		}
	}else {
		m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT2));
		AlertNotFound(m_pCommanderView->GetHwnd(), false, LS(STR_BOOKMARK_NEXT_NOT_FOUND));
	}
	return;
}


// from CViewCommander_New.cpp
// �O�̃u�b�N�}�[�N��T���C����������ړ�����D
void ViewCommander::Command_BOOKMARK_PREV(void)
{
	int		nYOld;				// hor
	BOOL	bFound	=	FALSE;	// hor
	BOOL	bRedo	=	TRUE;	// hor

	LogicPoint	ptXY(0, GetCaret().GetCaretLogicPos().y);
	LogicInt tmp_y;

	nYOld = ptXY.y;						// hor

re_do:;								// hor
	if (BookmarkManager(&GetDocument()->m_docLineMgr).SearchBookMark(ptXY.GetY2(), SearchDirection::Backward, &tmp_y)) {
		ptXY.y = tmp_y;
		bFound = TRUE;				// hor
		LayoutPoint ptLayout;
		GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY, &ptLayout);
		// 2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
		m_pCommanderView->MoveCursorSelecting(ptLayout, m_pCommanderView->GetSelectionInfo().m_bSelectingLock);
	}
    // 2002.01.26 hor
	if (GetDllShareData().m_common.m_sSearch.m_bSearchAll) {
		if (!bFound	&&	// ������Ȃ�����
			bRedo		// �ŏ��̌���
		) {
			// 2011.02.02 m_cLayoutMgr��m_docLineMgr
			ptXY.y = GetDocument()->m_docLineMgr.GetLineCount();	// 2002/06/01 MIK
			bRedo = FALSE;
			goto re_do;	// ��������Č���
		}
	}
	if (bFound) {
		if (nYOld <= ptXY.y) {
			m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV1));
		}
	}else {
		m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV2));
		AlertNotFound(m_pCommanderView->GetHwnd(), false, LS(STR_BOOKMARK_PREV_NOT_FOUND));
	}
	return;
}


// from CViewCommander_New.cpp
// �u�b�N�}�[�N���N���A����
void ViewCommander::Command_BOOKMARK_RESET(void)
{
	BookmarkManager(&GetDocument()->m_docLineMgr).ResetAllBookMark();
	// 2002.01.16 hor ���������r���[���X�V
	GetEditWindow()->Views_Redraw();
}


// from CViewCommander_New.cpp
// �w��p�^�[���Ɉ�v����s���}�[�N 2002.01.16 hor
// �L�[�}�N���ŋL�^�ł���悤��	2002.02.08 hor
void ViewCommander::Command_BOOKMARK_PATTERN(void)
{
	// ����or�u���_�C�A���O����Ăяo���ꂽ
	if (!m_pCommanderView->ChangeCurRegexp(false)) {
		return;
	}
	BookmarkManager(&GetDocument()->m_docLineMgr).MarkSearchWord(
		m_pCommanderView->m_sSearchPattern
	);
	// 2002.01.16 hor ���������r���[���X�V
	GetEditWindow()->Views_Redraw();
}



//! ���̊֐����X�g�}�[�N��T���C����������ړ�����
void ViewCommander::Command_FUNCLIST_NEXT(void)
{
	LogicPoint	ptXY(0, GetCaret().GetCaretLogicPos().y);
	int nYOld = ptXY.y;

	for (int n=0; n<2; ++n) {
		if (FuncListManager().SearchFuncListMark(&GetDocument()->m_docLineMgr,
				ptXY.GetY2(), SearchDirection::Forward, &ptXY.y)) {
			LayoutPoint ptLayout;
			GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY,&ptLayout);
			m_pCommanderView->MoveCursorSelecting( ptLayout,
				m_pCommanderView->GetSelectionInfo().m_bSelectingLock );
			if (nYOld >= ptXY.y) {
				m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT1));
			}
			return;
		}
		if (!GetDllShareData().m_common.m_sSearch.m_bSearchAll) {
			break;
		}
		ptXY.y = -1;
	}
	m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT2));
	AlertNotFound( m_pCommanderView->GetHwnd(), false, LS(STR_FUCLIST_NEXT_NOT_FOUND));
	return;
}



//! �O�̃u�b�N�}�[�N��T���C����������ړ�����D
void ViewCommander::Command_FUNCLIST_PREV(void)
{

	LogicPoint	ptXY(0,GetCaret().GetCaretLogicPos().y);
	int nYOld = ptXY.y;

	for (int n=0; n<2; ++n) {
		if (FuncListManager().SearchFuncListMark(
			&GetDocument()->m_docLineMgr,
			ptXY.GetY2(),
			SearchDirection::Backward,
			&ptXY.y
			)
		) {
			LayoutPoint ptLayout;
			GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY, &ptLayout);
			m_pCommanderView->MoveCursorSelecting(
				ptLayout,
				m_pCommanderView->GetSelectionInfo().m_bSelectingLock
				);
			if (nYOld <= ptXY.y) {
				m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV1));
			}
			return;
		}
		if (!GetDllShareData().m_common.m_sSearch.m_bSearchAll) {
			break;
		}
		ptXY.y= GetDocument()->m_docLineMgr.GetLineCount();
	}
	m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV2));
	AlertNotFound( m_pCommanderView->GetHwnd(), false, LS(STR_FUCLIST_PREV_NOT_FOUND) );
	return;
}

