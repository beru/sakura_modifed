/*!	@file
@brief ViewCommander�N���X�̃R�}���h(�W�����v&�u�b�N�}�[�N)�֐��Q

	2012/12/17	ViewCommander.cpp,ViewCommander_New.cpp���番��
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
#include "ViewCommander.h"
#include "ViewCommander_inline.h"
#include "docplus/FuncListManager.h"


// from ViewCommander_New.cpp
/*!	�����J�n�ʒu�֖߂�
	@author	ai
	@date	02/06/26
*/
void ViewCommander::Command_Jump_SrchStartPos(void)
{
	if (view.ptSrchStartPos_PHY.BothNatural()) {
		Point pt;
		// �͈͑I�𒆂�
		GetDocument().layoutMgr.LogicToLayout(
			view.ptSrchStartPos_PHY,
			&pt
		);
		// 2006.07.09 genta �I����Ԃ�ۂ�
		view.MoveCursorSelecting(pt, view.GetSelectionInfo().bSelectingLock);
	}else {
		ErrorBeep();
	}
	return;
}


/*! �w��s�փW�����v�_�C�A���O�̕\��
	2002.2.2 YAZAKI
*/
void ViewCommander::Command_Jump_Dialog(void)
{
	if (!GetEditWindow().dlgJump.DoModal(
			G_AppInstance(), view.GetHwnd(), (LPARAM)&GetDocument()
		)
	) {
		return;
	}
}


// �w��s�w�W�����v
void ViewCommander::Command_Jump(void)
{
	auto& layoutMgr = GetDocument().layoutMgr;
	if (layoutMgr.GetLineCount() == 0) {
		ErrorBeep();
		return;
	}

	int nMode;
	bool bValidLine;
	int nCommentBegin = 0;

	auto& dlgJump = GetEditWindow().dlgJump;
	// �s�ԍ�
	size_t nLineNum = dlgJump.nLineNum; //$$ �P�ʍ���
	if (!dlgJump.bPLSQL) {	// PL/SQL�\�[�X�̗L���s��
		// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
		if (GetDllShareData().bLineNumIsCRLF_ForJump) {
			if (0 == nLineNum) {
				nLineNum = 1;
			}
			/*
			  �J�[�\���ʒu�ϊ�
			  ���W�b�N�ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
			  ��
			  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
			*/
			Point ptPosXY;
			layoutMgr.LogicToLayout(
				Point(0, nLineNum - 1),
				&ptPosXY
			);
			nLineNum = ptPosXY.y + 1;
		}else {
			if (0 == nLineNum) {
				nLineNum = 1;
			}
			if (nLineNum > layoutMgr.GetLineCount()) {
				nLineNum = layoutMgr.GetLineCount();
			}
		}
		// Sep. 8, 2000 genta
		view.AddCurrentLineToHistory();
		// 2006.07.09 genta �I����Ԃ��������Ȃ��悤��
		view.MoveCursorSelecting(
			Point(0, nLineNum - 1),
			view.GetSelectionInfo().bSelectingLock,
			_CARETMARGINRATE / 3
		);
		return;
	}
	if (0 == nLineNum) {
		nLineNum = 1;
	}
	nMode = 0;
	ASSERT_GE(dlgJump.nPLSQL_E2, 1);
	size_t nCurrentLine = dlgJump.nPLSQL_E2 - 1;
	ASSERT_GE(dlgJump.nPLSQL_E1, 1);
	size_t nLineCount = dlgJump.nPLSQL_E1 - 1;; //$$ �P�ʍ���

	// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
	if (!view.pTypeData->bLineNumIsCRLF) { // ���C�A�E�g�P��
		/*
		  �J�[�\���ʒu�ϊ�
		  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		  ��
		  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		*/
		Point ptPosXY;
		layoutMgr.LayoutToLogic(
			Point(0, nLineCount),
			&ptPosXY
		);
		nLineCount = ptPosXY.y;
	}

	auto& lineMgr = GetDocument().docLineMgr;
	for (; nLineCount<lineMgr.GetLineCount(); ++nLineCount) {
		size_t nLineLen;
		const wchar_t* pLine = lineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		bValidLine = false;
		size_t i;
		for (i=0; i<nLineLen; ++i) {
			wchar_t let = pLine[i];
			if (1
				&& let != L' '
				&& let != WCODE::TAB
			) {
				break;
			}
		}
		int nBgn = i;
		wchar_t let = 0;
		wchar_t prevLet;
		auto& csEdit = GetDllShareData().common.edit;
		for (i=nBgn; i<nLineLen; ++i) {
			// �V���O���N�H�[�e�[�V����������ǂݍ��ݒ�
			prevLet = let;
			let = pLine[i];
			if (nMode == 20) {
				bValidLine = true;
				if (let == L'\'') {
					if (i > 0 && prevLet == L'\\') {
					}else {
						nMode = 0;
						continue;
					}
				}else {
				}
			}else
			// �_�u���N�H�[�e�[�V����������ǂݍ��ݒ�
			if (nMode == 21) {
				bValidLine = true;
				if (let == L'"') {
					if (i > 0 && prevLet == L'\\') {
					}else {
						nMode = 0;
						continue;
					}
				}else {
				}
			}else
			// �R�����g�ǂݍ��ݒ�
			if (nMode == 8) {
				if (i < nLineLen - 1 && let == L'*' &&  pLine[i + 1] == L'/') {
					if (/*nCommentBegin != nLineCount &&*/ nCommentBegin != 0) {
						bValidLine = true;
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
					|| WCODE::IsLineDelimiter(pLine[i], csEdit.bEnableExtEol)
				) {
					continue;
				}else
				if (i < nLineLen - 1 && let == L'-' &&  pLine[i + 1] == L'-') {
					bValidLine = true;
					break;
				}else
				if (i < nLineLen - 1 && let == L'/' &&  pLine[i + 1] == L'*') {
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
					bValidLine = true;
				}
			}
		}
		// �R�����g�ǂݍ��ݒ�
		if (nMode == 8) {
			if (nCommentBegin != 0) {
				bValidLine = true;
			}
			// �R�����g�u���b�N���̉��s�����̍s
			if (WCODE::IsLineDelimiter(pLine[nBgn], csEdit.bEnableExtEol)) {
				bValidLine = false;
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
	Point ptPos;
	layoutMgr.LogicToLayout(
		Point(0, nLineCount),
		&ptPos
	);
	// Sep. 8, 2000 genta
	view.AddCurrentLineToHistory();
	// 2006.07.09 genta �I����Ԃ��������Ȃ��悤��
	view.MoveCursorSelecting(ptPos, view.GetSelectionInfo().bSelectingLock, _CARETMARGINRATE / 3);
}


// from ViewCommander_New.cpp
// �u�b�N�}�[�N�̐ݒ�E�������s��(�g�O������)
void ViewCommander::Command_Bookmark_Set(void)
{
	DocLine* pDocLine;
	auto& selInfo = view.GetSelectionInfo();
	auto& select = selInfo.select;
	auto& lineMgr = GetDocument().docLineMgr;
	if (selInfo.IsTextSelected()
		&& select.GetFrom().y < select.GetTo().y
	) {
		Point ptFrom;
		Point ptTo;
		auto& layoutMgr = GetDocument().layoutMgr;
		layoutMgr.LayoutToLogic(
			Point(0, select.GetFrom().y),
			&ptFrom
		);
		layoutMgr.LayoutToLogic(
			Point(0, select.GetTo().y),
			&ptTo
		);
		for (int nY=ptFrom.y; nY<=ptTo.y; ++nY) {
			pDocLine = lineMgr.GetLine(nY);
			if (pDocLine) {
				BookmarkSetter bookmark(pDocLine);
				bookmark.SetBookmark(!bookmark.IsBookmarked());
			}
		}
	}else {
		pDocLine = lineMgr.GetLine(GetCaret().GetCaretLogicPos().y);
		if (pDocLine) {
			BookmarkSetter bookmark(pDocLine);
			bookmark.SetBookmark(!bookmark.IsBookmarked());
		}
	}

	// 2002.01.16 hor ���������r���[���X�V
	GetEditWindow().Views_Redraw();
}


// from ViewCommander_New.cpp
// ���̃u�b�N�}�[�N��T���C����������ړ�����
void ViewCommander::Command_Bookmark_Next(void)
{
	int		nYOld;				// hor
	bool	bFound	=	false;	// hor
	bool	bRedo	=	true;	// hor

	Point ptXY(0, GetCaret().GetCaretLogicPos().y);
	int tmp_y;

	nYOld = ptXY.y;					// hor

re_do:;								// hor
	if (BookmarkManager(GetDocument().docLineMgr).SearchBookMark(ptXY.y, SearchDirection::Forward, &tmp_y)) {
		ptXY.y = tmp_y;
		bFound = true;
		Point ptLayout;
		GetDocument().layoutMgr.LogicToLayout(ptXY, &ptLayout);
		// 2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
		view.MoveCursorSelecting(ptLayout, view.GetSelectionInfo().bSelectingLock);
	}
    // 2002.01.26 hor
	if (GetDllShareData().common.search.bSearchAll) {
		if (!bFound		// ������Ȃ�����
			&& bRedo	// �ŏ��̌���
		) {
			ptXY.y = -1;	// 2002/06/01 MIK
			bRedo = false;
			goto re_do;		// �擪����Č���
		}
	}
	if (bFound) {
		if (nYOld >= ptXY.y) {
			view.SendStatusMessage(LS(STR_ERR_SRNEXT1));
		}
	}else {
		view.SendStatusMessage(LS(STR_ERR_SRNEXT2));
		AlertNotFound(view.GetHwnd(), false, LS(STR_BOOKMARK_NEXT_NOT_FOUND));
	}
	return;
}


// from ViewCommander_New.cpp
// �O�̃u�b�N�}�[�N��T���C����������ړ�����D
void ViewCommander::Command_Bookmark_Prev(void)
{
	int		nYOld;				// hor
	bool	bFound	=	false;	// hor
	bool	bRedo	=	true;	// hor

	Point ptXY(0, GetCaret().GetCaretLogicPos().y);
	int tmp_y;

	nYOld = ptXY.y;						// hor

re_do:;								// hor
	auto& docLineMgr = GetDocument().docLineMgr;
	if (BookmarkManager(docLineMgr).SearchBookMark(ptXY.y, SearchDirection::Backward, &tmp_y)) {
		ptXY.y = tmp_y;
		bFound = true;				// hor
		Point ptLayout;
		GetDocument().layoutMgr.LogicToLayout(ptXY, &ptLayout);
		// 2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
		view.MoveCursorSelecting(ptLayout, view.GetSelectionInfo().bSelectingLock);
	}
    // 2002.01.26 hor
	if (GetDllShareData().common.search.bSearchAll) {
		if (!bFound		// ������Ȃ�����
			&& bRedo	// �ŏ��̌���
		) {
			// 2011.02.02 layoutMgr��docLineMgr
			ptXY.y = docLineMgr.GetLineCount();	// 2002/06/01 MIK
			bRedo = false;
			goto re_do;	// ��������Č���
		}
	}
	if (bFound) {
		if (nYOld <= ptXY.y) {
			view.SendStatusMessage(LS(STR_ERR_SRPREV1));
		}
	}else {
		view.SendStatusMessage(LS(STR_ERR_SRPREV2));
		AlertNotFound(view.GetHwnd(), false, LS(STR_BOOKMARK_PREV_NOT_FOUND));
	}
	return;
}


// from ViewCommander_New.cpp
// �u�b�N�}�[�N���N���A����
void ViewCommander::Command_Bookmark_Reset(void)
{
	BookmarkManager(GetDocument().docLineMgr).ResetAllBookMark();
	// 2002.01.16 hor ���������r���[���X�V
	GetEditWindow().Views_Redraw();
}


// from ViewCommander_New.cpp
// �w��p�^�[���Ɉ�v����s���}�[�N 2002.01.16 hor
// �L�[�}�N���ŋL�^�ł���悤��	2002.02.08 hor
void ViewCommander::Command_Bookmark_Pattern(void)
{
	// ����or�u���_�C�A���O����Ăяo���ꂽ
	if (!view.ChangeCurRegexp(false)) {
		return;
	}
	BookmarkManager(GetDocument().docLineMgr).MarkSearchWord(
		view.searchPattern
	);
	// 2002.01.16 hor ���������r���[���X�V
	GetEditWindow().Views_Redraw();
}



// ���̊֐����X�g�}�[�N��T���C����������ړ�����
void ViewCommander::Command_FuncList_Next(void)
{
	Point ptXY(0, GetCaret().GetCaretLogicPos().y);
	int nYOld = ptXY.y;

	for (int n=0; n<2; ++n) {
		if (FuncListManager().SearchFuncListMark(
				GetDocument().docLineMgr,
				ptXY.y, SearchDirection::Forward, &ptXY.y)) {
			Point ptLayout;
			GetDocument().layoutMgr.LogicToLayout(ptXY,&ptLayout);
			view.MoveCursorSelecting( ptLayout,
				view.GetSelectionInfo().bSelectingLock );
			if (nYOld >= ptXY.y) {
				view.SendStatusMessage(LS(STR_ERR_SRNEXT1));
			}
			return;
		}
		if (!GetDllShareData().common.search.bSearchAll) {
			break;
		}
		ptXY.y = -1;
	}
	view.SendStatusMessage(LS(STR_ERR_SRNEXT2));
	AlertNotFound( view.GetHwnd(), false, LS(STR_FUCLIST_NEXT_NOT_FOUND));
	return;
}



// �O�̃u�b�N�}�[�N��T���C����������ړ�����D
void ViewCommander::Command_FuncList_Prev(void)
{

	Point ptXY(0,GetCaret().GetCaretLogicPos().y);
	int nYOld = ptXY.y;
	auto& docLineMgr = GetDocument().docLineMgr;
	for (int n=0; n<2; ++n) {
		if (FuncListManager().SearchFuncListMark(
			docLineMgr,
			ptXY.y,
			SearchDirection::Backward,
			&ptXY.y
			)
		) {
			Point ptLayout;
			GetDocument().layoutMgr.LogicToLayout(ptXY, &ptLayout);
			view.MoveCursorSelecting(
				ptLayout,
				view.GetSelectionInfo().bSelectingLock
				);
			if (nYOld <= ptXY.y) {
				view.SendStatusMessage(LS(STR_ERR_SRPREV1));
			}
			return;
		}
		if (!GetDllShareData().common.search.bSearchAll) {
			break;
		}
		ptXY.y= docLineMgr.GetLineCount();
	}
	view.SendStatusMessage(LS(STR_ERR_SRPREV2));
	AlertNotFound( view.GetHwnd(), false, LS(STR_FUCLIST_PREV_NOT_FOUND) );
	return;
}

