/*!	@file
	@brief �}�E�X�C�x���g�̏���

	@author Norio Nakatani
	@date	1998/03/13 �쐬
	@date   2008/04/13 CEditView.cpp���番��
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, genta, JEPRO, MIK
	Copyright (C) 2001, genta, GAE, MIK, hor, asa-o, Stonee, Misaka, novice, YAZAKI
	Copyright (C) 2002, YAZAKI, hor, aroka, MIK, Moca, minfu, KK, novice, ai, Azumaiya, genta
	Copyright (C) 2003, MIK, ai, ryoji, Moca, wmlhq, genta
	Copyright (C) 2004, genta, Moca, novice, naoh, isearch, fotomo
	Copyright (C) 2005, genta, MIK, novice, aroka, D.S.Koba, �����, Moca
	Copyright (C) 2006, Moca, aroka, ryoji, fon, genta
	Copyright (C) 2007, ryoji, ���イ��, maru

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <process.h> // _beginthreadex
#include <limits.h>
#include "EditView.h"
#include "_main/AppMode.h"
#include "EditApp.h"
#include "GrepAgent.h" // use EditApp.h
#include "window/EditWnd.h"
#include "_os/DropTarget.h" // DataObject
#include "_os/Clipboard.h"
#include "OpeBlk.h"
#include "doc/layout/Layout.h"
#include "cmd/ViewCommander_inline.h"
#include "uiparts/WaitCursor.h"
#include "uiparts/HandCursor.h"
#include "util/input.h"
#include "util/os.h"
#include "sakura_rc.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �}�E�X�C�x���g                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �}�E�X���{�^������
void EditView::OnLBUTTONDOWN(WPARAM fwKeys, int _xPos , int _yPos)
{
	Point ptMouse(_xPos, _yPos);

	if (m_bHokan) {
		m_pEditWnd->m_hokanMgr.Hide();
		m_bHokan = FALSE;
	}

	// isearch 2004.10.22 isearch���L�����Z������
	if (m_nISearchMode > 0) {
		ISearchExit();
	}
	if (m_nAutoScrollMode) {
		AutoScrollExit();
	}
	if (m_bMiniMap) {
		::SetFocus(GetHwnd());
		::SetCapture(GetHwnd());
		m_bMiniMapMouseDown = true;
		OnMOUSEMOVE(fwKeys, _xPos, _yPos);
		return;
	}

	NativeW	memCurText;
	const wchar_t*	pLine;
	LogicInt		nLineLen;

	LayoutRange range;

	LogicInt	nIdx;
	int			nWork;
	int			nFuncID = 0;				// 2007.12.02 nasukoji	�}�E�X���N���b�N�ɑΉ�����@�\�R�[�h

	if (m_pEditDoc->m_layoutMgr.GetLineCount() == 0) {
		return;
	}
	if (!GetCaret().ExistCaretFocus()) { // �t�H�[�J�X���Ȃ��Ƃ�
		return;
	}

	// ����Tip���N������Ă���
	if (m_dwTipTimer == 0) {
		// ����Tip������
		m_tipWnd.Hide();
		m_dwTipTimer = ::GetTickCount();	// ����Tip�N���^�C�}�[
	}else {
		m_dwTipTimer = ::GetTickCount();		// ����Tip�N���^�C�}�[
	}

	// 2007.10.02 nasukoji	�g���v���N���b�N�ł��邱�Ƃ�����
	// 2007.12.02 nasukoji	�g���v���N���b�N���`�F�b�N
	bool tripleClickMode = CheckTripleClick(ptMouse);
	if (tripleClickMode) {
		// �}�E�X���g���v���N���b�N�ɑΉ�����@�\�R�[�h��m_common.m_pKeyNameArr[5]�ɓ����Ă���
		nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::TripleClick].nFuncCodeArr[GetCtrlKeyState()];
		if (nFuncID == 0) {
			tripleClickMode = false;	// ���蓖�ċ@�\�����̎��̓g���v���N���b�N OFF
		}
	}else {
		m_dwTripleClickCheck = 0;	// �g���v���N���b�N�`�F�b�N OFF
	}

	// ���݂̃}�E�X�J�[�\���ʒu�����C�A�E�g�ʒu
	LayoutPoint ptNew;
	auto& textArea = GetTextArea();
	textArea.ClientToLayout(ptMouse, &ptNew);

	// 2010.07.15 Moca �}�E�X�_�E�����̍��W���o���ė��p����
	m_mouseDownPos = ptMouse;

	// OLE�ɂ��h���b�O & �h���b�v���g��
	// 2007.12.02 nasukoji	�g���v���N���b�N���̓h���b�O���J�n���Ȃ�
	if (!tripleClickMode && GetDllShareData().common.edit.bUseOLE_DragDrop) {
		if (GetDllShareData().common.edit.bUseOLE_DropSource) {		// OLE�ɂ��h���b�O���ɂ��邩
			// �s�I���G���A���h���b�O����
			if (ptMouse.x < textArea.GetAreaLeft() - GetTextMetrics().GetHankakuDx()) {
				goto normal_action;
			}
			// �w��J�[�\���ʒu���I���G���A���ɂ��邩
			if (IsCurrentPositionSelected(ptNew) == 0) {
				POINT ptWk = {ptMouse.x, ptMouse.y};
				::ClientToScreen(GetHwnd(), &ptWk);
				if (!::DragDetect(GetHwnd(), ptWk)) {
					// �h���b�O�J�n�����𖞂����Ȃ������̂ŃN���b�N�ʒu�ɃJ�[�\���ړ�����
					if (GetSelectionInfo().IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
						// ���݂̑I��͈͂��I����Ԃɖ߂�
						GetSelectionInfo().DisableSelectArea(true);
					}
//@@@ 2002.01.08 YAZAKI �t���[�J�[�\��OFF�ŕ����s�I�����A�s�̌����N���b�N����Ƃ����ɃL�����b�g���u����Ă��܂��o�O�C��
					// �J�[�\���ړ��B
					if (ptMouse.y >= textArea.GetAreaTop() && ptMouse.y < textArea.GetAreaBottom()) {
						if (ptMouse.x >= textArea.GetAreaLeft() && ptMouse.x < textArea.GetAreaRight()
						) {
							GetCaret().MoveCursorToClientPoint(ptMouse);
						}else if (ptMouse.x < textArea.GetAreaLeft()) {
							GetCaret().MoveCursorToClientPoint(Point(textArea.GetDocumentLeftClientPointX(), ptMouse.y));
						}
					}
					return;
				}
				// �I��͈͂̃f�[�^���擾
				if (GetSelectedData(&memCurText, false, NULL, false, GetDllShareData().common.edit.bAddCRLFWhenCopy)) {
					DWORD dwEffects;
					DWORD dwEffectsSrc = (!m_pEditDoc->IsEditable()) ?
											DROPEFFECT_COPY: DROPEFFECT_COPY | DROPEFFECT_MOVE;
					int nOpe = m_pEditDoc->m_docEditor.m_opeBuf.GetCurrentPointer();
					m_pEditWnd->SetDragSourceView(this);
					DataObject data(memCurText.GetStringPtr(), memCurText.GetStringLength(), GetSelectionInfo().IsBoxSelecting());
					dwEffects = data.DragDrop(TRUE, dwEffectsSrc);
					m_pEditWnd->SetDragSourceView(NULL);
					if (m_pEditDoc->m_docEditor.m_opeBuf.GetCurrentPointer() == nOpe) {	// �h�L�������g�ύX�Ȃ����H	// 2007.12.09 ryoji
						m_pEditWnd->SetActivePane(m_nMyIndex);
						if ((dwEffectsSrc & dwEffects) == DROPEFFECT_MOVE) {
							// �ړ��͈͂��폜����
							// �h���b�v�悪�ړ����������������h�L�������g�ɂ����܂ŕύX������
							// ���h���b�v��͊O���̃E�B���h�E�ł���
							if (!m_commander.GetOpeBlk()) {
								m_commander.SetOpeBlk(new OpeBlk);
							}
							m_commander.GetOpeBlk()->AddRef();

							// �I��͈͂��폜
							DeleteData(true);

							// �A���h�D�o�b�t�@�̏���
							SetUndoBuffer();
						}
					}
				}
				return;
			}
		}
	}

normal_action:;

	// ALT�L�[��������Ă���A���g���v���N���b�N�łȂ�		// 2007.11.15 nasukoji	�g���v���N���b�N�Ή�
	if (GetKeyState_Alt() && !tripleClickMode) {
		if (GetSelectionInfo().IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
			// ���݂̑I��͈͂��I����Ԃɖ߂�
			GetSelectionInfo().DisableSelectArea(true);
		}
		if (ptMouse.y >= textArea.GetAreaTop()  && ptMouse.y < textArea.GetAreaBottom()) {
			if (ptMouse.x >= textArea.GetAreaLeft() && ptMouse.x < textArea.GetAreaRight()) {
				GetCaret().MoveCursorToClientPoint(ptMouse);
			}else if (ptMouse.x < textArea.GetAreaLeft()) {
				GetCaret().MoveCursorToClientPoint(Point(textArea.GetDocumentLeftClientPointX(), ptMouse.y));
			}else {
				return;
			}
		}
		GetSelectionInfo().m_ptMouseRollPosOld = ptMouse;	// �}�E�X�͈͑I��O��ʒu(XY���W)

		// �͈͑I���J�n & �}�E�X�L���v�`���[
		GetSelectionInfo().SelectBeginBox();

		::SetCapture(GetHwnd());
		GetCaret().HideCaret_(GetHwnd()); // 2002/07/22 novice
		// ���݂̃J�[�\���ʒu����I�����J�n����
		GetSelectionInfo().BeginSelectArea();
		GetCaret().m_underLine.CaretUnderLineOFF(true);
		GetCaret().m_underLine.UnderLineLock();
		if (ptMouse.x < textArea.GetAreaLeft()) {
			// �J�[�\�����ړ�
			GetCommander().Command_DOWN(true, false);
		}
	}else {
		// �J�[�\���ړ�
		if (ptMouse.y >= textArea.GetAreaTop() && ptMouse.y < textArea.GetAreaBottom()) {
			if (ptMouse.x >= textArea.GetAreaLeft() && ptMouse.x < textArea.GetAreaRight()) {
			}else if (ptMouse.x < textArea.GetAreaLeft()) {
			}else {
				return;
			}
		}else if (ptMouse.y < textArea.GetAreaTop()) {
			//	���[���N���b�N
			return;
		}else {
			return;
		}

		// �}�E�X�̃L���v�`���Ȃ�
		GetSelectionInfo().m_ptMouseRollPosOld = ptMouse;	// �}�E�X�͈͑I��O��ʒu(XY���W)
		
		// �͈͑I���J�n & �}�E�X�L���v�`���[
		GetSelectionInfo().SelectBeginNazo();
		::SetCapture(GetHwnd());
		GetCaret().HideCaret_(GetHwnd()); // 2002/07/22 novice

		LayoutPoint ptNewCaret = GetCaret().GetCaretLayoutPos();
		bool bSetPtNewCaret = false;
		if (tripleClickMode) {		// 2007.11.15 nasukoji	�g���v���N���b�N����������
			// 1�s�I���łȂ��ꍇ�͑I�𕶎��������
			// �g���v���N���b�N��1�s�I���łȂ��Ă��N�A�h���v���N���b�N��L���Ƃ���
			if (nFuncID != F_SELECTLINE) {
				OnLBUTTONUP(fwKeys, ptMouse.x, ptMouse.y);	// �����ō��{�^���A�b�v�������Ƃɂ���

				if (GetSelectionInfo().IsTextSelected())		// �e�L�X�g���I������Ă��邩
					GetSelectionInfo().DisableSelectArea(true);	// ���݂̑I��͈͂��I����Ԃɖ߂�
			}

			// �P��̓r���Ő܂�Ԃ���Ă���Ɖ��̍s���I������Ă��܂����Ƃւ̑Ώ�
			if (nFuncID != F_SELECTLINE) {
				GetCaret().MoveCursorToClientPoint(ptMouse);	// �J�[�\���ړ�
			}else {
				GetCaret().MoveCursorToClientPoint(ptMouse, true, &ptNewCaret);	// �J�[�\���ړ�
				bSetPtNewCaret = true;
			}

			// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
			// �}�E�X����̃��b�Z�[�W��CMD_FROM_MOUSE����ʃr�b�g�ɓ���đ���
			::SendMessage(::GetParent(m_hwndParent), WM_COMMAND, MAKELONG(nFuncID, CMD_FROM_MOUSE), (LPARAM)NULL);

			// 1�s�I���łȂ��ꍇ�͂����Ŕ�����i���̑I���R�}���h�̎����ƂȂ邩���j
			if (nFuncID != F_SELECTLINE)
				return;
			ptNewCaret = GetCaret().GetCaretLayoutPos();

			// �I��������̂������i[EOF]�݂̂̍s�j���͒ʏ�N���b�N�Ɠ�������
			if (1
				&& !GetSelectionInfo().IsTextSelected()
				&& GetCaret().GetCaretLogicPos().y >= m_pEditDoc->m_docLineMgr.GetLineCount()
			) {
				GetSelectionInfo().BeginSelectArea();				// ���݂̃J�[�\���ʒu����I�����J�n����
				GetSelectionInfo().m_bBeginLineSelect = false;		// �s�P�ʑI�� OFF
			}
		// �I���J�n����
		// SHIFT�L�[��������Ă�����
		}else if (GetKeyState_Shift()) {
			if (GetSelectionInfo().IsTextSelected()) {		// �e�L�X�g���I������Ă��邩
				if (GetSelectionInfo().IsBoxSelecting()) {	// ��`�͈͑I��
					// ���݂̑I��͈͂��I����Ԃɖ߂�
					GetSelectionInfo().DisableSelectArea(true);

					// ���݂̃J�[�\���ʒu����I�����J�n����
					GetSelectionInfo().BeginSelectArea();
				}else {
				}
			}else {
				// ���݂̃J�[�\���ʒu����I�����J�n����
				GetSelectionInfo().BeginSelectArea();
			}

			// �J�[�\���ړ�
			if (ptMouse.y >= textArea.GetAreaTop() && ptMouse.y < textArea.GetAreaBottom()) {
				if (ptMouse.x >= textArea.GetAreaLeft() && ptMouse.x < textArea.GetAreaRight()) {
					GetCaret().MoveCursorToClientPoint(ptMouse, true, &ptNewCaret);
				}else if (ptMouse.x < textArea.GetAreaLeft()) {
					GetCaret().MoveCursorToClientPoint(Point(textArea.GetDocumentLeftClientPointX(), ptMouse.y), true, &ptNewCaret);
				}
				bSetPtNewCaret = true;
			}
		}else {
			if (GetSelectionInfo().IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
				// ���݂̑I��͈͂��I����Ԃɖ߂�
				GetSelectionInfo().DisableSelectArea(true);
			}
			// �J�[�\���ړ�
			if (ptMouse.y >= textArea.GetAreaTop() && ptMouse.y < textArea.GetAreaBottom()) {
				if (ptMouse.x >= textArea.GetAreaLeft() && ptMouse.x < textArea.GetAreaRight()) {
					GetCaret().MoveCursorToClientPoint(ptMouse, true, &ptNewCaret);
				}else if (ptMouse.x < textArea.GetAreaLeft()) {
					GetCaret().MoveCursorToClientPoint(Point(textArea.GetDocumentLeftClientPointX(), ptMouse.y), true, &ptNewCaret);
				}
				bSetPtNewCaret = true;
			}
			// ���݂̃J�[�\���ʒu����I�����J�n����
			GetSelectionInfo().BeginSelectArea(&ptNewCaret);
		}


		/******* ���̎��_�ŕK�� true == GetSelectionInfo().IsTextSelected() �̏�ԂɂȂ� ****:*/
		if (!GetSelectionInfo().IsTextSelected()) {
			WarningMessage(GetHwnd(), LS(STR_VIEW_MOUSE_BUG));
			return;
		}

		int	nWorkRel;
		nWorkRel = IsCurrentPositionSelected(
			ptNewCaret	// �J�[�\���ʒu
		);


		// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
		GetSelectionInfo().ChangeSelectAreaByCurrentCursor(ptNewCaret);

		bool bSelectWord = false;
		// CTRL�L�[��������Ă���A���g���v���N���b�N�łȂ�		// 2007.11.15 nasukoji	�g���v���N���b�N�Ή�
		if (GetKeyState_Control() && !tripleClickMode) {
			GetSelectionInfo().m_bBeginWordSelect = true;		// �P��P�ʑI��
			if (!GetSelectionInfo().IsTextSelected()) {
				// ���݈ʒu�̒P��I��
				if (GetCommander().Command_SELECTWORD(&ptNewCaret)) {
					bSelectWord = true;
					GetSelectionInfo().m_selectBgn = GetSelectionInfo().m_select;
				}
			}else {

				// �I��̈�`��
				GetSelectionInfo().DrawSelectArea();

				// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
				const Layout* pLayout;
				pLine = m_pEditDoc->m_layoutMgr.GetLineStr(
					GetSelectionInfo().m_select.GetFrom().GetY2(),
					&nLineLen,
					&pLayout
				);
				if (pLine) {
					nIdx = LineColumnToIndex(pLayout, GetSelectionInfo().m_select.GetFrom().GetX2());
					// ���݈ʒu�̒P��͈̔͂𒲂ׂ�
					bool bWhareResult = m_pEditDoc->m_layoutMgr.WhereCurrentWord(
						GetSelectionInfo().m_select.GetFrom().GetY2(),
						nIdx,
						&range,
						NULL,
						NULL
					);
					if (bWhareResult) {
						// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�B
						// 2007.10.15 kobake ���Ƀ��C�A�E�g�P�ʂȂ̂ŕϊ��͕s�v
						/*
						pLine            = m_pEditDoc->m_layoutMgr.GetLineStr(range.GetFrom().GetY2(), &nLineLen, &pLayout);
						range.SetFromX(LineIndexToColumn(pLayout, range.GetFrom().x));
						pLine            = m_pEditDoc->m_layoutMgr.GetLineStr(range.GetTo().GetY2(), &nLineLen, &pLayout);
						range.SetToX(LineIndexToColumn(pLayout, range.GetTo().x));
						*/

						nWork = IsCurrentPositionSelected(
							range.GetFrom()	// �J�[�\���ʒu
						);
						if (nWork == -1 || nWork == 0) {
							GetSelectionInfo().m_select.SetFrom(range.GetFrom());
							if (nWorkRel == 1) {
								GetSelectionInfo().m_selectBgn = range;
							}
						}
					}
				}
				pLine = m_pEditDoc->m_layoutMgr.GetLineStr(GetSelectionInfo().m_select.GetTo().GetY2(), &nLineLen, &pLayout);
				if (pLine) {
					nIdx = LineColumnToIndex(pLayout, GetSelectionInfo().m_select.GetTo().GetX2());
					// ���݈ʒu�̒P��͈̔͂𒲂ׂ�
					if (m_pEditDoc->m_layoutMgr.WhereCurrentWord(
						GetSelectionInfo().m_select.GetTo().GetY2(), nIdx, &range, NULL, NULL)
					) {
						// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
						// 2007.10.15 kobake ���Ƀ��C�A�E�g�P�ʂȂ̂ŕϊ��͕s�v
						/*
						pLine = m_pEditDoc->m_layoutMgr.GetLineStr(range.GetFrom().GetY2(), &nLineLen, &pLayout);
						range.SetFromX(LineIndexToColumn(pLayout, range.GetFrom().x));
						pLine = m_pEditDoc->m_layoutMgr.GetLineStr(range.GetTo().GetY2(), &nLineLen, &pLayout);
						range.SetToX(LineIndexToColumn(pLayout, range.GetTo().x));
						*/

						nWork = IsCurrentPositionSelected(range.GetFrom());
						if (nWork == -1 || nWork == 0) {
							GetSelectionInfo().m_select.SetTo(range.GetFrom());
						}
						if (IsCurrentPositionSelected(range.GetTo()) == 1) {
							GetSelectionInfo().m_select.SetTo(range.GetTo());
						}
						if (nWorkRel == -1 || nWorkRel == 0) {
							GetSelectionInfo().m_selectBgn=range;
						}
					}
				}

				if (0 < nWorkRel) {

				}
				// �I��̈�`��
				GetSelectionInfo().DrawSelectArea();
			}
		}
		// �s�ԍ��G���A���N���b�N����
		// 2008.05.22 nasukoji	�V�t�g�L�[�������Ă���ꍇ�͍s���N���b�N�Ƃ��Ĉ���
		if (ptMouse.x < textArea.GetAreaLeft() && !GetKeyState_Shift()) {
			// ���݂̃J�[�\���ʒu����I�����J�n����
			GetSelectionInfo().m_bBeginLineSelect = true;

			// 2009.02.22 ryoji 
			// Command_GOLINEEND()/Command_RIGHT()�ł͂Ȃ����̃��C�A�E�g�𒲂ׂĈړ��I��������@�ɕύX
			// ��Command_GOLINEEND()/Command_RIGHT()��[�܂�Ԃ����������̉E�ֈړ�]�{[���s�̐擪�����̉E�Ɉړ�]�̎d�l���Ƃm�f
			const Layout* pLayout = m_pEditDoc->m_layoutMgr.SearchLineByLayoutY(ptNewCaret.GetY2());
			if (pLayout) {
				LayoutPoint ptCaret;
				const Layout* pNext = pLayout->GetNextLayout();
				if (pNext) {
					ptCaret.x = pNext->GetIndent();
				}else {
					ptCaret.x = LayoutInt(0);
				}
				ptCaret.y = ptNewCaret.GetY2() + 1;	// ���s����EOF�s�ł� MoveCursor() ���L���ȍ��W�ɒ������Ă����
				GetCaret().GetAdjustCursorPos(&ptCaret);
				GetSelectionInfo().ChangeSelectAreaByCurrentCursor(ptCaret);
				GetCaret().MoveCursor(ptCaret, true);
				GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			}else {
				// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
				if (bSetPtNewCaret) {
					GetSelectionInfo().ChangeSelectAreaByCurrentCursor(ptNewCaret);
					GetCaret().MoveCursor(ptNewCaret, true, 1000);
					GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
				}
			}

			//	Apr. 14, 2003 genta
			//	�s�ԍ��̉����N���b�N���ăh���b�O���J�n����Ƃ��������Ȃ�̂��C��
			//	�s�ԍ����N���b�N�����ꍇ�ɂ�GetSelectionInfo().ChangeSelectAreaByCurrentCursor()�ɂ�
			//	GetSelectionInfo().m_select.GetTo().x/GetSelectionInfo().m_select.GetTo().y��-1���ݒ肳��邪�A���
			//	GetCommander().Command_GOLINEEND(), Command_RIGHT()�ɂ���čs�I�����s����B
			//	�������L�����b�g�������ɂ���ꍇ�ɂ̓L�����b�g���ړ����Ȃ��̂�
			//	GetSelectionInfo().m_select.GetTo().x/GetSelectionInfo().m_select.GetTo().y��-1�̂܂܎c���Ă��܂��A���ꂪ
			//	���_�ɐݒ肳��邽�߂ɂ��������Ȃ��Ă����B
			//	�Ȃ̂ŁA�͈͑I�����s���Ă��Ȃ��ꍇ�͋N�_�����̐ݒ���s��Ȃ��悤�ɂ���
			if (GetSelectionInfo().IsTextSelected()) {
				GetSelectionInfo().m_selectBgn.SetTo(GetSelectionInfo().m_select.GetTo());
			}
		}else {
			// URL���N���b�N���ꂽ��I�����邩
			if (GetDllShareData().common.edit.bSelectClickedURL) {

				LogicRange cUrlRange;	// URL�͈�
				// �J�[�\���ʒu��URL���L��ꍇ�̂��͈̔͂𒲂ׂ�
				bool bIsUrl = IsCurrentPositionURL(
					ptNewCaret,	// �J�[�\���ʒu
					&cUrlRange,						// URL�͈�
					NULL							// URL�󂯎���
				);
				if (bIsUrl) {
					// ���݂̑I��͈͂��I����Ԃɖ߂�
					GetSelectionInfo().DisableSelectArea(true);

					/*
					  �J�[�\���ʒu�ϊ�
					  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
					  �����C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
						2002/04/08 YAZAKI �����ł��킩��₷���B
					*/
					LayoutRange rangeB;
					m_pEditDoc->m_layoutMgr.LogicToLayout(cUrlRange, &rangeB);
					/*
					m_pEditDoc->m_layoutMgr.LogicToLayout(LogicPoint(nUrlIdxBgn          , nUrlLine), rangeB.GetFromPointer());
					m_pEditDoc->m_layoutMgr.LogicToLayout(LogicPoint(nUrlIdxBgn + nUrlLen, nUrlLine), rangeB.GetToPointer());
					*/

					GetSelectionInfo().m_selectBgn = rangeB;
					GetSelectionInfo().m_select = rangeB;

					// �I��̈�`��
					GetSelectionInfo().DrawSelectArea();
				}
			}
			if (bSetPtNewCaret && !bSelectWord) {
				// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
				GetCaret().MoveCursor(ptNewCaret, true, 1000);
				GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			}
		}
	}
}


/*!	�g���v���N���b�N�̃`�F�b�N
	@brief �g���v���N���b�N�𔻒肷��
	
	2��ڂ̃N���b�N����3��ڂ̃N���b�N�܂ł̎��Ԃ��_�u���N���b�N���Ԉȓ��ŁA
	�����̎��̃N���b�N�ʒu�̂��ꂪ�V�X�e�����g���b�N�iX:SM_CXDOUBLECLK,
	Y:SM_CYDOUBLECLK�j�̒l�i�s�N�Z���j�ȉ��̎��g���v���N���b�N�Ƃ���B
	
	@param[in] xPos		�}�E�X�N���b�NX���W
	@param[in] yPos		�}�E�X�N���b�NY���W
	@return		�g���v���N���b�N�̎���TRUE��Ԃ�
	�g���v���N���b�N�łȂ�����FALSE��Ԃ�

	@note	m_dwTripleClickCheck��0�łȂ����Ƀ`�F�b�N���[�h�Ɣ��肷�邪�APC��
			�A���ғ����Ă���ꍇ49.7�����ɃJ�E���^��0�ɂȂ�ׁA�킸���ȉ\��
			�ł��邪�g���v���N���b�N������ł��Ȃ���������B
			�s�ԍ��\���G���A�̃g���v���N���b�N�͒ʏ�N���b�N�Ƃ��Ĉ����B
	
	@date 2007.11.15 nasukoji	�V�K�쐬
*/
bool EditView::CheckTripleClick(Point ptMouse)
{

	// �g���v���N���b�N�`�F�b�N�L���łȂ��i�������Z�b�g����Ă��Ȃ��j
	if (!m_dwTripleClickCheck)
		return false;

	bool result = false;

	// �O��N���b�N�Ƃ̃N���b�N�ʒu�̂�����Z�o
	Point dpos(GetSelectionInfo().m_ptMouseRollPosOld.x - ptMouse.x,
				   GetSelectionInfo().m_ptMouseRollPosOld.y - ptMouse.y);

	if (dpos.x < 0)
		dpos.x = -dpos.x;	// ��Βl��

	if (dpos.y < 0)
		dpos.y = -dpos.y;	// ��Βl��

	// �s�ԍ��\���G���A�łȂ��A���N���b�N�v���X����_�u���N���b�N���Ԉȓ��A
	// ���_�u���N���b�N�̋��e����s�N�Z���ȉ��̂���̎��g���v���N���b�N�Ƃ���
	//	2007.10.12 genta/dskoba �V�X�e���̃_�u���N���b�N���x�C���ꋖ�e�ʂ��擾
	if ((ptMouse.x >= GetTextArea().GetAreaLeft())&&
		(::GetTickCount() - m_dwTripleClickCheck <= GetDoubleClickTime())&&
		(dpos.x <= GetSystemMetrics(SM_CXDOUBLECLK)) &&
		(dpos.y <= GetSystemMetrics(SM_CYDOUBLECLK))
	) {
		result = true;
	}else {
		m_dwTripleClickCheck = 0;	// �g���v���N���b�N�`�F�b�N OFF
	}
	
	return result;
}

// �}�E�X�E�{�^������
void EditView::OnRBUTTONDOWN(WPARAM fwKeys, int xPos , int yPos)
{
	if (m_nAutoScrollMode) {
		AutoScrollExit();
	}
	if (m_bMiniMap) {
		return;
	}
	// ���݂̃}�E�X�J�[�\���ʒu�����C�A�E�g�ʒu

	LayoutPoint ptNew;
	GetTextArea().ClientToLayout(Point(xPos, yPos), &ptNew);
	/*
	ptNew.x = GetTextArea().GetViewLeftCol() + (xPos - GetTextArea().GetAreaLeft()) / GetTextMetrics().GetHankakuDx();
	ptNew.y = GetTextArea().GetViewTopLine() + (yPos - GetTextArea().GetAreaTop()) / GetTextMetrics().GetHankakuDy();
	*/
	// �w��J�[�\���ʒu���I���G���A���ɂ��邩
	if (IsCurrentPositionSelected(
			ptNew		// �J�[�\���ʒu
		) == 0
	) {
		return;
	}
	OnLBUTTONDOWN(fwKeys, xPos , yPos);
	return;
}

// �}�E�X�E�{�^���J��
void EditView::OnRBUTTONUP(WPARAM fwKeys, int xPos , int yPos)
{
	if (GetSelectionInfo().IsMouseSelecting()) {	// �͈͑I��
		// �}�E�X���{�^���J���̃��b�Z�[�W����
		OnLBUTTONUP(fwKeys, xPos, yPos);
	}

	int		nIdx;
	int		nFuncID;
// novice 2004/10/10
	// Shift,Ctrl,Alt�L�[��������Ă�����
	nIdx = GetCtrlKeyState();
	// �}�E�X�E�N���b�N�ɑΉ�����@�\�R�[�h��m_common.m_pKeyNameArr[1]�ɓ����Ă���
	nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::RightClick].nFuncCodeArr[nIdx];
	if (nFuncID != 0) {
		// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
		//	May 19, 2006 genta �}�E�X����̃��b�Z�[�W��CMD_FROM_MOUSE����ʃr�b�g�ɓ���đ���
		::PostMessage(::GetParent(m_hwndParent), WM_COMMAND, MAKELONG(nFuncID, CMD_FROM_MOUSE), (LPARAM)NULL);
	}
//	// �E�N���b�N���j���[
//	GetCommander().Command_MENU_RBUTTON();
	return;
}


// novice 2004/10/11 �}�E�X���{�^���Ή�
/*!
	@brief �}�E�X���{�^�����������Ƃ��̏���

	@param fwKeys [in] first message parameter
	@param xPos [in] �}�E�X�J�[�\��X���W
	@param yPos [in] �}�E�X�J�[�\��Y���W
	@date 2004.10.11 novice �V�K�쐬
	@date 2008.10.06 nasukoji	�}�E�X���{�^���������̃z�C�[������Ή�
	@date 2009.01.17 nasukoji	�{�^��UP�ŃR�}���h���N������悤�ɕύX
*/
void EditView::OnMBUTTONDOWN(WPARAM fwKeys, int xPos , int yPos)
{
	int nIdx = GetCtrlKeyState();
	if (GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::CenterClick].nFuncCodeArr[nIdx] == F_AUTOSCROLL) {
		if (m_nAutoScrollMode) {
			AutoScrollExit();
			return;
		}else {
			m_nAutoScrollMode = 1;
			m_autoScrollMousePos = Point(xPos, yPos);
			::SetCapture(GetHwnd());
		}
	}
}


/*!
	@brief �}�E�X���{�^�����J�������Ƃ��̏���

	@param fwKeys [in] first message parameter
	@param xPos [in] �}�E�X�J�[�\��X���W
	@param yPos [in] �}�E�X�J�[�\��Y���W
	
	@date 2009.01.17 nasukoji	�V�K�쐬�i�{�^��UP�ŃR�}���h���N������悤�ɕύX�j
*/
void EditView::OnMBUTTONUP(WPARAM fwKeys, int xPos , int yPos)
{
	int		nIdx;
	int		nFuncID;

	// �z�C�[������ɂ��y�[�W�X�N���[������
	if (GetDllShareData().common.general.nPageScrollByWheel == (int)MouseFunctionType::CenterClick &&
	    m_pEditWnd->IsPageScrollByWheel()
	) {
		m_pEditWnd->SetPageScrollByWheel(FALSE);
		return;
	}

	// �z�C�[������ɂ��y�[�W�X�N���[������
	if (GetDllShareData().common.general.nHorizontalScrollByWheel == (int)MouseFunctionType::CenterClick &&
	    m_pEditWnd->IsHScrollByWheel()
	) {
		m_pEditWnd->SetHScrollByWheel(FALSE);
		return;
	}

	// Shift,Ctrl,Alt�L�[��������Ă�����
	nIdx = GetCtrlKeyState();
	// �}�E�X���T�C�h�{�^���ɑΉ�����@�\�R�[�h��m_common.m_pKeyNameArr[2]�ɓ����Ă���
	nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::CenterClick].nFuncCodeArr[nIdx];
	if (nFuncID == F_AUTOSCROLL) {
		if (m_nAutoScrollMode == 1) {
			m_bAutoScrollDragMode = false;
			AutoScrollEnter();
			return;
		}else if (m_nAutoScrollMode == 2 && m_bAutoScrollDragMode) {
			AutoScrollExit();
			return;
		}
	}else if (nFuncID != 0) {
		// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
		//	May 19, 2006 genta �}�E�X����̃��b�Z�[�W��CMD_FROM_MOUSE����ʃr�b�g�ɓ���đ���
		::PostMessage(::GetParent(m_hwndParent), WM_COMMAND, MAKELONG(nFuncID, CMD_FROM_MOUSE),  (LPARAM)NULL);
	}
	if (m_nAutoScrollMode) {
		AutoScrollExit();
	}
}

void CALLBACK AutoScrollTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	EditView*	pEditView;
	pEditView = (EditView*)::GetWindowLongPtr(hwnd, 0);
	if (pEditView) {
		pEditView->AutoScrollOnTimer();
	}
}

void EditView::AutoScrollEnter()
{
	m_bAutoScrollVertical = GetTextArea().m_nViewRowNum < m_pEditDoc->m_layoutMgr.GetLineCount() + 2;
	m_bAutoScrollHorizontal = GetTextArea().m_nViewColNum < GetRightEdgeForScrollBar();
	if (m_bMiniMap) {
		m_bAutoScrollHorizontal = false;
	}
	if (!m_bAutoScrollHorizontal && !m_bAutoScrollVertical) {
		m_nAutoScrollMode = 0;
		return;
	}
	m_nAutoScrollMode = 2;
	m_autoScrollWnd.Create(G_AppInstance(), GetHwnd(), m_bAutoScrollVertical, m_bAutoScrollHorizontal, m_autoScrollMousePos, this);
	::SetTimer(GetHwnd(), 2, 200, AutoScrollTimerProc);
	HCURSOR hCursor;
	hCursor = ::LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_CURSOR_AUTOSCROLL_CENTER));
	::SetCursor(hCursor);
}

void EditView::AutoScrollExit()
{
	if (m_nAutoScrollMode) {
		::ReleaseCapture();
	}
	if (m_nAutoScrollMode == 2) {
		KillTimer(GetHwnd(), 2);
		m_autoScrollWnd.Close();
	}
	m_nAutoScrollMode = 0;
}

void EditView::AutoScrollMove(Point& point)
{
	const Point relPos = point - m_autoScrollMousePos;
	int idcX, idcY;
	if (!m_bAutoScrollHorizontal || abs(relPos.x) < 16) {
		idcX = 0;
	}else if (relPos.x < 0) {
		idcX = 1;
	}else {
		idcX = 2;
	}
	if (!m_bAutoScrollVertical || abs(relPos.y) < 16) {
		idcY = 0;
	}else if (relPos.y < 0) {
		idcY = 1;
	}else {
		idcY = 2;
	}
	const int idcs[3][3] = {
		{IDC_CURSOR_AUTOSCROLL_CENTER, IDC_CURSOR_AUTOSCROLL_UP,       IDC_CURSOR_AUTOSCROLL_DOWN},
		{IDC_CURSOR_AUTOSCROLL_LEFT,   IDC_CURSOR_AUTOSCROLL_UP_LEFT,  IDC_CURSOR_AUTOSCROLL_DOWN_LEFT},
		{IDC_CURSOR_AUTOSCROLL_RIGHT,  IDC_CURSOR_AUTOSCROLL_UP_RIGHT, IDC_CURSOR_AUTOSCROLL_DOWN_RIGHT}};
	int cursor = idcs[idcX][idcY];
	if (cursor == IDC_CURSOR_AUTOSCROLL_CENTER) {
		if (!m_bAutoScrollVertical) {
			cursor = IDC_CURSOR_AUTOSCROLL_HORIZONTAL;
		}else if (!m_bAutoScrollHorizontal) {
			cursor = IDC_CURSOR_AUTOSCROLL_VERTICAL;
		}
	}
	const HCURSOR hCursor = ::LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(cursor));
	::SetCursor(hCursor);
}

void EditView::AutoScrollOnTimer()
{
	Point cursorPos;
	::GetCursorPos(&cursorPos);
	::ScreenToClient(GetHwnd(), &cursorPos);
	
	const Point relPos = cursorPos - m_autoScrollMousePos;
	Point scrollPos = relPos / 8;
	if (m_bAutoScrollHorizontal) {
		if (scrollPos.x < 0) {
			scrollPos.x += 1;
		}else if (scrollPos.x > 0) {
			scrollPos.x -= 1;
		}
		SyncScrollH(ScrollAtH(GetTextArea().GetViewLeftCol() + scrollPos.x));
	}
	if (m_bAutoScrollVertical) {
		if (scrollPos.y < 0) {
			scrollPos.y += 1;
		}else if (scrollPos.y > 0) {
			scrollPos.y -= 1;
		}
		SyncScrollV(ScrollAtV(GetTextArea().GetViewTopLine() + scrollPos.y));
	}
}

// novice 2004/10/10 �}�E�X�T�C�h�{�^���Ή�
/*!
	@brief �}�E�X�T�C�h�{�^��1���������Ƃ��̏���

	@param fwKeys [in] first message parameter
	@param xPos [in] �}�E�X�J�[�\��X���W
	@param yPos [in] �}�E�X�J�[�\��Y���W
	@date 2004.10.10 novice �V�K�쐬
	@date 2004.10.11 novice �}�E�X���{�^���Ή��̂��ߕύX
	@date 2009.01.17 nasukoji	�{�^��UP�ŃR�}���h���N������悤�ɕύX
*/
void EditView::OnXLBUTTONDOWN(WPARAM fwKeys, int xPos , int yPos)
{
	if (m_nAutoScrollMode) {
		AutoScrollExit();
	}
}


/*!
	@brief �}�E�X�T�C�h�{�^��1���J�������Ƃ��̏���

	@param fwKeys [in] first message parameter
	@param xPos [in] �}�E�X�J�[�\��X���W
	@param yPos [in] �}�E�X�J�[�\��Y���W

	@date 2009.01.17 nasukoji	�V�K�쐬�i�{�^��UP�ŃR�}���h���N������悤�ɕύX�j
*/
void EditView::OnXLBUTTONUP(WPARAM fwKeys, int xPos , int yPos)
{
	int		nIdx;
	int		nFuncID;

	// �z�C�[������ɂ��y�[�W�X�N���[������
	if (GetDllShareData().common.general.nPageScrollByWheel == (int)MouseFunctionType::LeftSideClick &&
	    m_pEditWnd->IsPageScrollByWheel()
	) {
		m_pEditWnd->SetPageScrollByWheel(FALSE);
		return;
	}

	// �z�C�[������ɂ��y�[�W�X�N���[������
	if (GetDllShareData().common.general.nHorizontalScrollByWheel == (int)MouseFunctionType::LeftSideClick &&
	    m_pEditWnd->IsHScrollByWheel()
	) {
		m_pEditWnd->SetHScrollByWheel(FALSE);
		return;
	}

	// Shift,Ctrl,Alt�L�[��������Ă�����
	nIdx = GetCtrlKeyState();
	// �}�E�X�T�C�h�{�^��1�ɑΉ�����@�\�R�[�h��m_common.m_pKeyNameArr[3]�ɓ����Ă���
	nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::LeftSideClick].nFuncCodeArr[nIdx];
	if (nFuncID != 0) {
		// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
		//	May 19, 2006 genta �}�E�X����̃��b�Z�[�W��CMD_FROM_MOUSE����ʃr�b�g�ɓ���đ���
		::PostMessage(::GetParent(m_hwndParent), WM_COMMAND, MAKELONG(nFuncID, CMD_FROM_MOUSE),  (LPARAM)NULL);
	}

	return;
}


/*!
	@brief �}�E�X�T�C�h�{�^��2���������Ƃ��̏���

	@param fwKeys [in] first message parameter
	@param xPos [in] �}�E�X�J�[�\��X���W
	@param yPos [in] �}�E�X�J�[�\��Y���W
	@date 2004.10.10 novice �V�K�쐬
	@date 2004.10.11 novice �}�E�X���{�^���Ή��̂��ߕύX
	@date 2009.01.17 nasukoji	�{�^��UP�ŃR�}���h���N������悤�ɕύX
*/
void EditView::OnXRBUTTONDOWN(WPARAM fwKeys, int xPos , int yPos)
{
	if (m_nAutoScrollMode) {
		AutoScrollExit();
	}
}


/*!
	@brief �}�E�X�T�C�h�{�^��2���J�������Ƃ��̏���

	@param fwKeys [in] first message parameter
	@param xPos [in] �}�E�X�J�[�\��X���W
	@param yPos [in] �}�E�X�J�[�\��Y���W

	@date 2009.01.17 nasukoji	�V�K�쐬�i�{�^��UP�ŃR�}���h���N������悤�ɕύX�j
*/
void EditView::OnXRBUTTONUP(WPARAM fwKeys, int xPos , int yPos)
{
	int		nIdx;
	int		nFuncID;

	// �z�C�[������ɂ��y�[�W�X�N���[������
	if (GetDllShareData().common.general.nPageScrollByWheel == (int)MouseFunctionType::RightSideClick &&
	    m_pEditWnd->IsPageScrollByWheel()
	) {
		// �z�C�[������ɂ��y�[�W�X�N���[�������OFF
		m_pEditWnd->SetPageScrollByWheel(FALSE);
		return;
	}

	// �z�C�[������ɂ��y�[�W�X�N���[������
	if (GetDllShareData().common.general.nHorizontalScrollByWheel == (int)MouseFunctionType::RightSideClick &&
	    m_pEditWnd->IsHScrollByWheel()
	) {
		// �z�C�[������ɂ�鉡�X�N���[�������OFF
		m_pEditWnd->SetHScrollByWheel(FALSE);
		return;
	}

	// Shift,Ctrl,Alt�L�[��������Ă�����
	nIdx = GetCtrlKeyState();
	// �}�E�X�T�C�h�{�^��2�ɑΉ�����@�\�R�[�h��m_common.m_pKeyNameArr[4]�ɓ����Ă���
	nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::RightSideClick].nFuncCodeArr[nIdx];
	if (nFuncID != 0) {
		// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
		//	May 19, 2006 genta �}�E�X����̃��b�Z�[�W��CMD_FROM_MOUSE����ʃr�b�g�ɓ���đ���
		::PostMessage(::GetParent(m_hwndParent), WM_COMMAND, MAKELONG(nFuncID, CMD_FROM_MOUSE),  (LPARAM)NULL);
	}

	return;
}

// �}�E�X�ړ��̃��b�Z�[�W����
void EditView::OnMOUSEMOVE(WPARAM fwKeys, int xPos_, int yPos_)
{
	Point ptMouse(xPos_, yPos_);

	if (m_mousePousePos != ptMouse) {
		m_mousePousePos = ptMouse;
		if (m_nMousePouse < 0) {
			m_nMousePouse = 0;
		}
	}

	LayoutRange selectionOld    = GetSelectionInfo().m_select;

	// �I�[�g�X�N���[��
	if (m_nAutoScrollMode == 1) {
		if (::GetSystemMetrics(SM_CXDOUBLECLK) < abs(ptMouse.x - m_autoScrollMousePos.x) ||
		    ::GetSystemMetrics(SM_CYDOUBLECLK) < abs(ptMouse.y - m_autoScrollMousePos.y)
		) {
			m_bAutoScrollDragMode = true;
			AutoScrollEnter();
		}
		return;
	}else if (m_nAutoScrollMode == 2) {
		AutoScrollMove(ptMouse);
		return;
	}

	TextArea& textArea = GetTextArea();
	if (m_bMiniMap) {
		POINT po;
		::GetCursorPos(&po);
		// ����Tip���N������Ă���
		if (m_dwTipTimer == 0) {
			if ((m_poTipCurPos.x != po.x || m_poTipCurPos.y != po.y )) {
				m_tipWnd.Hide();
				m_dwTipTimer = ::GetTickCount();
			}
		}else {
			m_dwTipTimer = ::GetTickCount();
		}
		if (m_bMiniMapMouseDown) {
			LayoutPoint ptNew;
			textArea.ClientToLayout( ptMouse, &ptNew );
			// �~�j�}�b�v�̏㉺�X�N���[��
			if (ptNew.y < 0) {
				ptNew.y = LayoutYInt(0);
			}
			LayoutYInt nScrollRow = LayoutYInt(0);
			LayoutYInt nScrollMargin = LayoutYInt(15);
			nScrollMargin  = t_min(nScrollMargin,  (textArea.m_nViewRowNum) / 2);
			if (m_pEditDoc->m_layoutMgr.GetLineCount() > textArea.m_nViewRowNum
				&& ptNew.y > textArea.GetViewTopLine() + textArea.m_nViewRowNum - nScrollMargin
			) {
				nScrollRow = (textArea.GetViewTopLine() + textArea.m_nViewRowNum - nScrollMargin) - ptNew.y;
			}else if (0 < textArea.GetViewTopLine() && ptNew.y < textArea.GetViewTopLine() + nScrollMargin) {
				nScrollRow = textArea.GetViewTopLine() + nScrollMargin - ptNew.y;
				if (0 > textArea.GetViewTopLine() - nScrollRow) {
					nScrollRow = textArea.GetViewTopLine();
				}
			}
			if (nScrollRow != 0) {
				ScrollAtV( textArea.GetViewTopLine() - nScrollRow );
			}

			textArea.ClientToLayout( ptMouse, &ptNew );
			if (ptNew.y < 0) {
				ptNew.y = LayoutYInt(0);
			}
			EditView& view = m_pEditWnd->GetActiveView();
			ptNew.x = 0;
			LogicPoint ptNewLogic;
			view.GetCaret().GetAdjustCursorPos(&ptNew);
			GetDocument().m_layoutMgr.LayoutToLogic(ptNew, &ptNewLogic);
			GetDocument().m_layoutMgr.LogicToLayout(ptNewLogic, &ptNew, ptNew.y);
			if (GetKeyState_Shift()) {
				if (view.GetSelectionInfo().IsTextSelected()) {
					if (view.GetSelectionInfo().IsBoxSelecting()) {
						view.GetSelectionInfo().DisableSelectArea(true);
						view.GetSelectionInfo().BeginSelectArea();
					}
				}else {
					view.GetSelectionInfo().BeginSelectArea();
				}
				view.GetSelectionInfo().ChangeSelectAreaByCurrentCursor(ptNew);
			}else {
				if (view.GetSelectionInfo().IsTextSelected()) {
					view.GetSelectionInfo().DisableSelectArea(true);
				}
			}
			view.GetCaret().MoveCursor(ptNew, true);
			view.GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		GetSelectionInfo().m_ptMouseRollPosOld = ptMouse; // �}�E�X�͈͑I��O��ʒu(XY���W)
		return;
	}

	if (!GetSelectionInfo().IsMouseSelecting()) {
		// �}�E�X�ɂ��͈͑I�𒆂łȂ��ꍇ
		POINT		po;
		::GetCursorPos(&po);
		//	2001/06/18 asa-o: �⊮�E�B���h�E���\������Ă��Ȃ�
		if (!m_bHokan) {
			// ����Tip���N������Ă���
			if (m_dwTipTimer == 0) {
				if ((m_poTipCurPos.x != po.x || m_poTipCurPos.y != po.y)) {
					// ����Tip������
					m_tipWnd.Hide();
					m_dwTipTimer = ::GetTickCount();	// ����Tip�N���^�C�}�[
				}
			}else {
				m_dwTipTimer = ::GetTickCount();		// ����Tip�N���^�C�}�[
			}
		}
		// ���݂̃}�E�X�J�[�\���ʒu�����C�A�E�g�ʒu
		LayoutPoint ptNew;
		textArea.ClientToLayout(ptMouse, &ptNew);

		LogicRange	cUrlRange;	// URL�͈�

		// �I���e�L�X�g�̃h���b�O����
		if (m_bDragMode) {
			if (GetDllShareData().common.edit.bUseOLE_DragDrop) {	// OLE�ɂ��h���b�O & �h���b�v���g��
				// ���W�w��ɂ��J�[�\���ړ�
				GetCaret().MoveCursorToClientPoint(ptMouse);
			}
		}else {
			// �s�I���G���A?
			if (ptMouse.x < textArea.GetAreaLeft() || ptMouse.y < textArea.GetAreaTop()) {	//	2002/2/10 aroka
				// ���J�[�\��
				if (ptMouse.y >= textArea.GetAreaTop())
					::SetCursor(::LoadCursor(G_AppInstance(), MAKEINTRESOURCE(IDC_CURSOR_RVARROW)));
				else
					::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			}else if (1
				&& GetDllShareData().common.edit.bUseOLE_DragDrop		// OLE�ɂ��h���b�O & �h���b�v���g��
				&& GetDllShareData().common.edit.bUseOLE_DropSource		// OLE�ɂ��h���b�O���ɂ��邩
				&& IsCurrentPositionSelected(ptNew) == 0				// �w��J�[�\���ʒu���I���G���A���ɂ��邩
			) {
				// ���J�[�\��
				::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			// �J�[�\���ʒu��URL���L��ꍇ
			}else if (
				IsCurrentPositionURL(
					ptNew,			// �J�[�\���ʒu
					&cUrlRange,		// URL�͈�
					NULL			// URL�󂯎���
				)
			) {
				// ��J�[�\��
				SetHandCursor();		// Hand Cursor��ݒ� 2013/1/29 Uchi
			}else {
				// migemo isearch 2004.10.22
				if (m_nISearchMode > 0) {
					if (m_nISearchDirection == SearchDirection::Forward) {
						::SetCursor(::LoadCursor(G_AppInstance(), MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_F)));
					}else {
						::SetCursor(::LoadCursor(G_AppInstance(), MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_B)));
					}
				// �A�C�r�[��
				}else if (0 <= m_nMousePouse) {
					::SetCursor(::LoadCursor(NULL, IDC_IBEAM));
				}
			}
		}
		return;
	}
	// �ȉ��A�}�E�X�ł̑I��(�h���b�O��)

	if (0 <= m_nMousePouse) {
		::SetCursor(::LoadCursor(NULL, IDC_IBEAM));
	}

	// 2010.07.15 Moca �h���b�O�J�n�ʒu����ړ����Ă��Ȃ��ꍇ��MOVE�Ƃ݂Ȃ��Ȃ�
	// �V�т� 2px�Œ�Ƃ���
	Point ptMouseMove = ptMouse - m_mouseDownPos;
	if (m_mouseDownPos.x != -INT_MAX && abs(ptMouseMove.x) <= 2 && abs(ptMouseMove.y) <= 2) {
		return;
	}
	// ��x�ړ�������߂��Ă����Ƃ����A�ړ��Ƃ݂Ȃ��悤�ɐݒ�
	m_mouseDownPos.Set(-INT_MAX, -INT_MAX);
	
	LayoutPoint ptNewCursor(LayoutInt(-1), LayoutInt(-1));
	if (GetSelectionInfo().IsBoxSelecting()) {	// ��`�͈͑I��
		// ���W�w��ɂ��J�[�\���ړ�
		GetCaret().MoveCursorToClientPoint(ptMouse, true, &ptNewCursor);
		GetSelectionInfo().ChangeSelectAreaByCurrentCursor(ptNewCursor);
		GetCaret().MoveCursorToClientPoint(ptMouse);
		// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
		GetSelectionInfo().m_ptMouseRollPosOld = ptMouse; // �}�E�X�͈͑I��O��ʒu(XY���W)
	}else {
		// ���W�w��ɂ��J�[�\���ړ�
		if ((ptMouse.x < textArea.GetAreaLeft() || m_dwTripleClickCheck)&& GetSelectionInfo().m_bBeginLineSelect) {	// �s�P�ʑI��
			// 2007.11.15 nasukoji	������̍s�I�������}�E�X�J�[�\���̈ʒu�̍s���I�������悤�ɂ���
			Point nNewPos(0, ptMouse.y);

			// 1�s�̍���
			int nLineHeight = GetTextMetrics().GetHankakuHeight() + m_pTypeData->nLineSpace;

			// �I���J�n�s�ȉ��ւ̃h���b�O����1�s���ɃJ�[�\�����ړ�����
			if (textArea.GetViewTopLine() + (ptMouse.y - textArea.GetAreaTop()) / nLineHeight >= GetSelectionInfo().m_selectBgn.GetTo().y)
				nNewPos.y += nLineHeight;

			// �J�[�\�����ړ�
			nNewPos.x = textArea.GetAreaLeft() - Int(textArea.GetViewLeftCol()) * (GetTextMetrics().GetHankakuWidth() + m_pTypeData->nColumnSpace);
			GetCaret().MoveCursorToClientPoint(nNewPos, false, &ptNewCursor);

			// 2.5�N���b�N�ɂ��s�P�ʂ̃h���b�O
			if (m_dwTripleClickCheck) {
				// �I���J�n�s�ȏ�Ƀh���b�O����
				if (ptNewCursor.GetY() <= GetSelectionInfo().m_selectBgn.GetTo().y) {
					// GetCommander().Command_GOLINETOP(true, 0x09);		// ���s�P�ʂ̍s���ֈړ�
					LogicInt nLineLen;
					const Layout*	pLayout;
					const wchar_t*	pLine = m_pEditDoc->m_layoutMgr.GetLineStr(ptNewCursor.GetY2(), &nLineLen, &pLayout);
					ptNewCursor.x = LayoutInt(0);
					if (pLine) {
						while (pLayout->GetLogicOffset()) {
							ptNewCursor.y--;
							pLayout = pLayout->GetPrevLayout();
						}
					}
				}else {
					LayoutPoint ptCaret;

					LogicPoint ptCaretPrevLog(0, GetCaret().GetCaretLogicPos().y);

					// �I���J�n�s��艺�ɃJ�[�\�������鎞��1�s�O�ƕ����s�ԍ��̈Ⴂ���`�F�b�N����
					// �I���J�n�s�ɃJ�[�\�������鎞�̓`�F�b�N�s�v
					if (ptNewCursor.GetY() > GetSelectionInfo().m_selectBgn.GetTo().y) {
						// 1�s�O�̕����s���擾����
						m_pEditDoc->m_layoutMgr.LayoutToLogic(LayoutPoint(LayoutInt(0), ptNewCursor.GetY() - 1), &ptCaretPrevLog);
					}

					LogicPoint ptNewCursorLogic;
					m_pEditDoc->m_layoutMgr.LayoutToLogic(ptNewCursor, &ptNewCursorLogic);
					// �O�̍s�Ɠ��������s
					if (ptCaretPrevLog.y == ptNewCursorLogic.y) {
						// 1�s��̕����s���烌�C�A�E�g�s�����߂�
						m_pEditDoc->m_layoutMgr.LogicToLayout(LogicPoint(0, GetCaret().GetCaretLogicPos().y + 1), &ptCaret);

						// �J�[�\�������̕����s���ֈړ�����
						ptNewCursor = ptCaret;
					}
				}
			}
		}else {
			GetCaret().MoveCursorToClientPoint(ptMouse, true, &ptNewCursor);
		}
		GetSelectionInfo().m_ptMouseRollPosOld = ptMouse; // �}�E�X�͈͑I��O��ʒu(XY���W)

		// CTRL�L�[��������Ă�����
//		if (GetKeyState_Control()) {
		if (!GetSelectionInfo().m_bBeginWordSelect) {
			// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
			GetSelectionInfo().ChangeSelectAreaByCurrentCursor(ptNewCursor);
			GetCaret().MoveCursor(ptNewCursor, true, 1000);
		}else {
			LayoutRange select;
			
			// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX(�e�X�g�̂�)
			GetSelectionInfo().ChangeSelectAreaByCurrentCursorTEST(
				GetCaret().GetCaretLayoutPos(),
				&select
			);
			// �I��͈͂ɕύX�Ȃ�
			if (selectionOld == select) {
				GetSelectionInfo().ChangeSelectAreaByCurrentCursor(
					GetCaret().GetCaretLayoutPos()
				);
				GetCaret().MoveCursor(ptNewCursor, true, 1000);
				return;
			}
			LogicInt nLineLen;
			const Layout* pLayout;
			if (m_pEditDoc->m_layoutMgr.GetLineStr(GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pLayout)) {
				LogicInt	nIdx = LineColumnToIndex(pLayout, GetCaret().GetCaretLayoutPos().GetX2());
				LayoutRange range;

				// ���݈ʒu�̒P��͈̔͂𒲂ׂ�
				bool bResult = m_pEditDoc->m_layoutMgr.WhereCurrentWord(
					GetCaret().GetCaretLayoutPos().GetY2(),
					nIdx,
					&range,
					NULL,
					NULL
				);
				if (bResult) {
					// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
					// 2007.10.15 kobake ���Ƀ��C�A�E�g�P�ʂȂ̂ŕϊ��͕s�v
					/*
					pLine     = m_pEditDoc->m_layoutMgr.GetLineStr(range.GetFrom().GetY2(), &nLineLen, &pLayout);
					range.SetFromX(LineIndexToColumn(pLayout, range.GetFrom().x));
					pLine     = m_pEditDoc->m_layoutMgr.GetLineStr(range.GetTo().GetY2(), &nLineLen, &pLayout);
					range.SetToX(LineIndexToColumn(pLayout, range.GetTo().x));
					*/
					int nWorkF = IsCurrentPositionSelectedTEST(
						range.GetFrom(), // �J�[�\���ʒu
						select
					);
					int nWorkT = IsCurrentPositionSelectedTEST(
						range.GetTo(),	// �J�[�\���ʒu
						select
					);
					if (nWorkF == -1) {
						// �n�_���O���Ɉړ��B���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
						GetSelectionInfo().ChangeSelectAreaByCurrentCursor(range.GetFrom());
					}else if (nWorkT == 1) {
						// �I�_������Ɉړ��B���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
						GetSelectionInfo().ChangeSelectAreaByCurrentCursor(range.GetTo());
					}else if (selectionOld.GetFrom() == select.GetFrom()) {
						// �n�_�����ύX���O���ɏk�����ꂽ
						// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
						GetSelectionInfo().ChangeSelectAreaByCurrentCursor(range.GetTo());
					}else if (selectionOld.GetTo() == select.GetTo()) {
						// �I�_�����ύX������ɏk�����ꂽ
						// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
						GetSelectionInfo().ChangeSelectAreaByCurrentCursor(range.GetFrom());
					}
				}else {
					// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
					GetSelectionInfo().ChangeSelectAreaByCurrentCursor(GetCaret().GetCaretLayoutPos());
				}
			}else {
				// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
				GetSelectionInfo().ChangeSelectAreaByCurrentCursor(GetCaret().GetCaretLayoutPos());
			}
			GetCaret().MoveCursor(ptNewCursor, true, 1000);
		}
	}
	return;
}
//m_dwTipTimerm_dwTipTimerm_dwTipTimer


#ifndef SPI_GETWHEELSCROLLCHARS
#define SPI_GETWHEELSCROLLCHARS 0x006C
#endif


/* �}�E�X�z�C�[���̃��b�Z�[�W����
	2009.01.17 nasukoji	�z�C�[���X�N���[���𗘗p�����y�[�W�X�N���[���E���X�N���[���Ή�
	2011.11.16 Moca �X�N���[���ω��ʂւ̑Ή�
	2013.09.10 Moca �X�y�V�����X�N���[���̕s��̏C��
*/
LRESULT EditView::OnMOUSEWHEEL2(
	WPARAM wParam,
	LPARAM lParam,
	bool bHorizontalMsg,
	EFunctionCode nCmdFuncID
	)
{
//	WORD	fwKeys;
	short	zDelta;
//	short	xPos;
//	short	yPos;
	int		i;
	int		nScrollCode;
	int		nRollLineNum;

//	fwKeys = LOWORD(wParam);			// key flags
	zDelta = (short) HIWORD(wParam);	// wheel rotation
//	xPos = (short) LOWORD(lParam);		// horizontal position of pointer
//	yPos = (short) HIWORD(lParam);		// vertical position of pointer
//	MYTRACE(_T("EditView::DispatchEvent() WM_MOUSEWHEEL fwKeys=%xh zDelta=%d xPos=%d yPos=%d \n"), fwKeys, zDelta, xPos, yPos);

	if (bHorizontalMsg) {
		if (0 < zDelta) {
			nScrollCode = SB_LINEDOWN; // �E
		}else {
			nScrollCode = SB_LINEUP; // ��
		}
		zDelta *= -1; // ���΂ɂ���
	}else {
		if (0 < zDelta) {
			nScrollCode = SB_LINEUP;
		}else {
			nScrollCode = SB_LINEDOWN;
		}
	}

	{
		// 2009.01.17 nasukoji	�L�[/�}�E�X�{�^�� + �z�C�[���X�N���[���ŉ��X�N���[������
		bool bHorizontal = false;
		bool bKeyPageScroll = false;
		if (nCmdFuncID == F_0) {
			// �ʏ�X�N���[���̎������K�p
			bHorizontal = IsSpecialScrollMode(GetDllShareData().common.general.nHorizontalScrollByWheel);
			bKeyPageScroll = IsSpecialScrollMode(GetDllShareData().common.general.nPageScrollByWheel);
		}

		// 2013.05.30 Moca �z�C�[���X�N���[���ɃL�[���蓖��
		int nIdx = GetCtrlKeyState();
		EFunctionCode nFuncID = nCmdFuncID;
		if (nFuncID != F_0) {
		}else if (bHorizontalMsg) {
			if (nScrollCode == SB_LINEUP) {
				nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::WheelLeft].nFuncCodeArr[nIdx];
			}else {
				nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::WheelRight].nFuncCodeArr[nIdx];
			}
		}else {
			if (nScrollCode == SB_LINEUP) {
				nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::WheelUp].nFuncCodeArr[nIdx];
			}else {
				nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::WheelDown].nFuncCodeArr[nIdx];
			}
		}
		bool bExecCmd = false;
		{
			if (nFuncID < F_WHEEL_FIRST || F_WHEEL_LAST < nFuncID) {
				bExecCmd = true;
			}
			if (nFuncID == F_WHEELLEFT || nFuncID == F_WHEELRIGHT
				|| nFuncID == F_WHEELPAGELEFT ||  nFuncID == F_WHEELPAGERIGHT
			) {
				bHorizontal = true;
			}
			if (nFuncID == F_WHEELPAGEUP || nFuncID == F_WHEELPAGEDOWN
				|| nFuncID == F_WHEELPAGELEFT ||  nFuncID == F_WHEELPAGERIGHT
			) {
				bKeyPageScroll = true;
			}
			if (nFuncID == F_WHEELUP || nFuncID == F_WHEELLEFT
				|| nFuncID == F_WHEELPAGEUP || nFuncID == F_WHEELPAGELEFT
			) {
				if (nScrollCode != SB_LINEUP) {
					zDelta *= -1;
					nScrollCode = SB_LINEUP;
				}
			}else if (nFuncID == F_WHEELDOWN || nFuncID == F_WHEELRIGHT
				|| nFuncID == F_WHEELPAGEDOWN || nFuncID == F_WHEELPAGERIGHT
			) {
				if (nScrollCode != SB_LINEDOWN) {
					zDelta *= -1;
					nScrollCode = SB_LINEDOWN;
				}
			}
		}

		// �}�E�X�z�C�[���ɂ��X�N���[���s�������W�X�g������擾
		nRollLineNum = 3;

		// ���W�X�g���̑��݃`�F�b�N
		// 2006.06.03 Moca ReadRegistry �ɏ�������
		unsigned int uDataLen;	// size of value data
		TCHAR szValStr[256];
		uDataLen = _countof(szValStr) - 1;
		if (!bExecCmd) {
			bool bGetParam = false;
			if (bHorizontal) {
				int nScrollChars = 3;
				if (::SystemParametersInfo(SPI_GETWHEELSCROLLCHARS, 0, &nScrollChars, 0)) {
					bGetParam = true;
					nRollLineNum = nScrollChars;
					if (nRollLineNum != -1 && m_bMiniMap) {
						nRollLineNum *= 10;
					}
				}
			}
			if (!bGetParam) {
				if (ReadRegistry(HKEY_CURRENT_USER, _T("Control Panel\\desktop"), _T("WheelScrollLines"), szValStr, uDataLen)) {
					nRollLineNum = ::_ttoi(szValStr);
					if (nRollLineNum != -1 && m_bMiniMap) {
						nRollLineNum *= 10;
					}
				}
			}
		}

		if (nRollLineNum == -1 || bKeyPageScroll) {
			// �u1��ʕ��X�N���[������v
			if (bHorizontal) {
				nRollLineNum = (Int)GetTextArea().m_nViewColNum - 1;	// �\����̌���
			}else {
				nRollLineNum = (Int)GetTextArea().m_nViewRowNum - 1;	// �\����̍s��
			}
		}else {
			if (nRollLineNum > 30) {	//@@@ YAZAKI 2001.12.31 10��30�ցB
				nRollLineNum = 30;
			}
		}
		if (nRollLineNum < 1 || bExecCmd) {
			nRollLineNum = 1;
		}

		// �X�N���[������̎��(�ʏ���@�̃y�[�W�X�N���[����NORMAL����)
		if (bKeyPageScroll) {
			if (bHorizontal) {
				// �z�C�[������ɂ�鉡�X�N���[������
				m_pEditWnd->SetHScrollByWheel(TRUE);
			}
			// �z�C�[������ɂ��y�[�W�X�N���[������
			m_pEditWnd->SetPageScrollByWheel(TRUE);
		}else {
			if (bHorizontal) {
				// �z�C�[������ɂ�鉡�X�N���[������
				m_pEditWnd->SetHScrollByWheel(TRUE);
			}
		}

		if (0
			|| nFuncID != m_eWheelScroll
			|| (zDelta < 0 && 0 < m_nWheelDelta)
			|| (0 < zDelta && m_nWheelDelta < 0)
		) {
			m_nWheelDelta = 0;
			m_eWheelScroll = nFuncID;
		}
		m_nWheelDelta += zDelta;

		// 2011.05.18 API�̃X�N���[���ʂɏ]��
		int nRollNum = abs(m_nWheelDelta) * nRollLineNum / 120;
		// ���񎝉z���̕ω���(��L��Delta�̂��܂�B�X�N���[��������zDelta�͕���������)
		m_nWheelDelta = (abs(m_nWheelDelta) - nRollNum * 120 / nRollLineNum) * ((nScrollCode == SB_LINEUP) ? 1 : -1);

		if (bExecCmd) {
			if (nFuncID != F_0) {
				// �X�N���[���ω��ʕ��R�}���h���s(zDelta��120�������1��)
				for (int i=0; i<nRollNum; ++i) {
					::PostMessage(::GetParent(m_hwndParent), WM_COMMAND, MAKELONG(nFuncID, CMD_FROM_MOUSE),  (LPARAM)NULL);
				}
			}
			return bHorizontalMsg ? TRUE: 0;
		}

		const bool bSmooth = !! GetDllShareData().common.general.nRepeatedScroll_Smooth;
		const int nRollActions = bSmooth ? nRollNum : 1;
		const LayoutInt nCount = LayoutInt(((nScrollCode == SB_LINEUP) ? -1 : 1) * (bSmooth ? 1 : nRollNum));

		for (i=0; i<nRollActions; ++i) {
			//	Sep. 11, 2004 genta �����X�N���[���s��
			if (bHorizontal) {
				SyncScrollH(ScrollAtH(GetTextArea().GetViewLeftCol() + nCount));
			}else {
				SyncScrollV(ScrollAtV(GetTextArea().GetViewTopLine() + nCount));
			}
		}
	}
	return bHorizontalMsg ? TRUE: 0;
}


// �����}�E�X�X�N���[��
LRESULT EditView::OnMOUSEWHEEL(WPARAM wParam, LPARAM lParam)
{
	return OnMOUSEWHEEL2(wParam, lParam, false, F_0);
}

/*! �����}�E�X�X�N���[��
	@note http://msdn.microsoft.com/en-us/library/ms997498.aspx
	Best Practices for Supporting Microsoft Mouse and Keyboard Devices
	�ɂ��ƁAWM_MOUSEHWHEEL�����������ꍇ��TRUE��Ԃ��K�v�����邻���ł��B
	MSDN��WM_MOUSEHWHEEL Message�̃y�[�W�͊Ԉ���Ă���̂Œ��ӁB
*/
LRESULT EditView::OnMOUSEHWHEEL(WPARAM wParam, LPARAM lParam)
{
	return OnMOUSEWHEEL2(wParam, lParam, true, F_0);
}

/*!
	@brief �L�[�E�}�E�X�{�^����Ԃ��X�N���[�����[�h�𔻒肷��

	�}�E�X�z�C�[�����A�s�X�N���[�����ׂ����y�[�W�X�N���[���E���X�N���[��
	���ׂ����𔻒肷��B
	���݂̃L�[�܂��̓}�E�X��Ԃ������Ŏw�肳�ꂽ�g�ݍ��킹�ɍ��v����ꍇ
	true��Ԃ��B

	@param nSelect	[in] �L�[�E�}�E�X�{�^���̑g�ݍ��킹�w��ԍ�

	@return �y�[�W�X�N���[���܂��͉��X�N���[�����ׂ���Ԃ̎�true��Ԃ�
	        �ʏ�̍s�X�N���[�����ׂ���Ԃ̎�false��Ԃ�

	@date 2009.01.17 nasukoji	�V�K�쐬
*/
bool EditView::IsSpecialScrollMode(int nSelect)
{
	bool bSpecialScrollMode;

	switch (nSelect) {
	case 0:		// �w��̑g�ݍ��킹�Ȃ�
		bSpecialScrollMode = false;
		break;

	case MouseFunctionType::CenterClick:		// �}�E�X���{�^��
		bSpecialScrollMode = ((::GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0);
		break;

	case MouseFunctionType::LeftSideClick:	// �}�E�X�T�C�h�{�^��1
		bSpecialScrollMode = ((::GetAsyncKeyState(VK_XBUTTON1) & 0x8000) != 0);
		break;

	case MouseFunctionType::RightSideClick:	// �}�E�X�T�C�h�{�^��2
		bSpecialScrollMode = ((::GetAsyncKeyState(VK_XBUTTON2) & 0x8000) != 0);
		break;

	case VK_CONTROL:	// Control�L�[
		bSpecialScrollMode = GetKeyState_Control();
		break;

	case VK_SHIFT:		// Shift�L�[
		bSpecialScrollMode = GetKeyState_Shift();
		break;

	default:	// ��L�ȊO�i�����ɂ͗��Ȃ��j
		bSpecialScrollMode = false;
		break;
	}

	return bSpecialScrollMode;
}


// �}�E�X���{�^���J���̃��b�Z�[�W����
void EditView::OnLBUTTONUP(WPARAM fwKeys, int xPos , int yPos)
{
//	MYTRACE(_T("OnLBUTTONUP()\n"));

	// �͈͑I���I�� & �}�E�X�L���v�`���[�����
	if (GetSelectionInfo().IsMouseSelecting()) {	// �͈͑I��
		// �}�E�X �L���v�`�������
		::ReleaseCapture();
		GetCaret().ShowCaret_(GetHwnd()); // 2002/07/22 novice

		GetSelectionInfo().SelectEnd();

		// 20100715 Moca �}�E�X�N���b�N���W�����Z�b�g
		m_mouseDownPos.Set(-INT_MAX, -INT_MAX);

		GetCaret().m_underLine.UnderLineUnLock();
		if (GetSelectionInfo().m_select.IsOne()) {
			// ���݂̑I��͈͂��I����Ԃɖ߂�
			GetSelectionInfo().DisableSelectArea(true);
		}
	}
	if (m_bMiniMapMouseDown) {
		m_bMiniMapMouseDown = false;
		::ReleaseCapture();
	}
	return;
}

// ShellExecute���Ăяo���v���V�[�W��
//   �Ăяo���O�� lpParameter �� new ���Ă�������
static unsigned __stdcall ShellExecuteProc(LPVOID lpParameter)
{
	LPTSTR pszFile = (LPTSTR)lpParameter;
	::ShellExecute(NULL, _T("open"), pszFile, NULL, NULL, SW_SHOW);
	delete []pszFile;
	return 0;
}


// �}�E�X���{�^���_�u���N���b�N
// 2007.01.18 kobake IsCurrentPositionURL�d�l�ύX�ɔ����A�����̏�������
void EditView::OnLBUTTONDBLCLK(WPARAM fwKeys, int _xPos , int _yPos)
{
	Point ptMouse(_xPos, _yPos);

	LogicRange		cUrlRange;	// URL�͈�
	std::wstring	wstrURL;
	const wchar_t*	pszMailTo = L"mailto:";

	// 2007.10.06 nasukoji	�N�A�h���v���N���b�N���̓`�F�b�N���Ȃ�
	if (! m_dwTripleClickCheck) {
		// �J�[�\���ʒu��URL���L��ꍇ�̂��͈̔͂𒲂ׂ�
		if (
			IsCurrentPositionURL(
				GetCaret().GetCaretLayoutPos(),	// �J�[�\���ʒu
				&cUrlRange,				// URL�͈�
				&wstrURL				// URL�󂯎���
			)
		) {
			std::wstring wstrOPEN;

			// URL���J��
		 	// ���݈ʒu�����[���A�h���X�Ȃ�΁ANULL�ȊO�ƁA���̒�����Ԃ�
			if (IsMailAddress(wstrURL.c_str(), wstrURL.length(), NULL)) {
				wstrOPEN = pszMailTo + wstrURL;
			}else {
				if (wcsnicmp(wstrURL.c_str(), L"ttp://", 6) == 0) {	// �}�~URL
					wstrOPEN = L"h" + wstrURL;
				}else if (wcsnicmp(wstrURL.c_str(), L"tp://", 5) == 0) {	// �}�~URL
					wstrOPEN = L"ht" + wstrURL;
				}else {
					wstrOPEN = wstrURL;
				}
			}
			{
				// URL���J��
				// 2009.05.21 syat UNC�p�X����1���ȏ㖳�����ɂȂ邱�Ƃ�����̂ŃX���b�h��
				WaitCursor waitCursor(GetHwnd());	// �J�[�\���������v�ɂ���

				unsigned int nThreadId;
				LPCTSTR szUrl = to_tchar(wstrOPEN.c_str());
				LPTSTR pszUrlDup = new TCHAR[_tcslen(szUrl) + 1];
				_tcscpy(pszUrlDup, szUrl);
				HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, ShellExecuteProc, (LPVOID)pszUrlDup, 0, &nThreadId);
				if (hThread != INVALID_HANDLE_VALUE) {
					// ���[�U�[��URL�N���w���ɔ��������ڈ�Ƃ��Ă�����Ƃ̎��Ԃ��������v�J�[�\����\�����Ă���
					// ShellExecute �͑����ɃG���[�I�����邱�Ƃ����傭���傭����̂� WaitForSingleObject �ł͂Ȃ� Sleep ���g�p�iex.���݂��Ȃ��p�X�̋N���j
					// �y�⑫�z������� API �ł��҂��𒷂߁i2�`3�b�j�ɂ���ƂȂ��� Web �u���E�U���N������̋N�����d���Ȃ�͗l�iPC�^�C�v, XP/Vista, IE/FireFox �Ɋ֌W�Ȃ��j
					::Sleep(200);
					::CloseHandle(hThread);
				}else {
					// �X���b�h�쐬���s
					delete[] pszUrlDup;
				}
			}
			return;
		}

		// GREP�o�̓��[�h�܂��̓f�o�b�O���[�h ���� �}�E�X���{�^���_�u���N���b�N�Ń^�O�W�����v �̏ꍇ
		//	2004.09.20 naoh �O���R�}���h�̏o�͂���Tagjump�ł���悤��
		if (1
			&& (EditApp::getInstance().m_pGrepAgent->m_bGrepMode || AppMode::getInstance().IsDebugMode())
			&& GetDllShareData().common.search.bGTJW_DoubleClick
		) {
			// �^�O�W�����v�@�\
			if (GetCommander().Command_TAGJUMP()) {
				// 2013.05.27 �^�O�W�����v���s���͒ʏ�̏��������s����
				return;
			}
		}
	}

// novice 2004/10/10
	// Shift,Ctrl,Alt�L�[��������Ă�����
	int	nIdx = GetCtrlKeyState();

	// �}�E�X���N���b�N�ɑΉ�����@�\�R�[�h��m_common.m_pKeyNameArr[?]�ɓ����Ă��� 2007.11.15 nasukoji
	EFunctionCode nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[
		(int)(m_dwTripleClickCheck ? MouseFunctionType::QuadrapleClick : MouseFunctionType::DoubleClick)
	].nFuncCodeArr[nIdx];
	if (m_dwTripleClickCheck) {
		// ��I����Ԃɂ����㍶�N���b�N�������Ƃɂ���
		// ���ׂđI���̏ꍇ�́A3.5�N���b�N���̑I����ԕێ��ƃh���b�O�J�n����
		// �͈͕ύX�̂��߁B
		// �N�A�h���v���N���b�N�@�\�����蓖�Ă��Ă��Ȃ��ꍇ�́A�_�u���N���b�N
		// �Ƃ��ď������邽�߁B
		if (GetSelectionInfo().IsTextSelected())		// �e�L�X�g���I������Ă��邩
			GetSelectionInfo().DisableSelectArea(true);		// ���݂̑I��͈͂��I����Ԃɖ߂�

		if (!nFuncID) {
			m_dwTripleClickCheck = 0;	// �g���v���N���b�N�`�F�b�N OFF
			nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::DoubleClick].nFuncCodeArr[nIdx];
			OnLBUTTONDOWN(fwKeys, ptMouse.x , ptMouse.y);	// �J�[�\�����N���b�N�ʒu�ֈړ�����
		}
	}

	if (nFuncID != 0) {
		// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
		//	May 19, 2006 genta �}�E�X����̃��b�Z�[�W��CMD_FROM_MOUSE����ʃr�b�g�ɓ���đ���
		::SendMessage(::GetParent(m_hwndParent), WM_COMMAND, MAKELONG(nFuncID, CMD_FROM_MOUSE),  (LPARAM)NULL);
	}

	// 2007.10.06 nasukoji	�N�A�h���v���N���b�N���������Ŕ�����
	if (m_dwTripleClickCheck) {
		m_dwTripleClickCheck = 0;	// �g���v���N���b�N�`�F�b�N OFF�i����͒ʏ�N���b�N�j
		return;
	}

	// 2007.11.06 nasukoji	�_�u���N���b�N���P��I���łȂ��Ă��g���v���N���b�N��L���Ƃ���
	// 2007.10.02 nasukoji	�g���v���N���b�N�`�F�b�N�p�Ɏ������擾
	m_dwTripleClickCheck = ::GetTickCount();

	// �_�u���N���b�N�ʒu�Ƃ��ċL��
	GetSelectionInfo().m_ptMouseRollPosOld = ptMouse;	// �}�E�X�͈͑I��O��ʒu(XY���W)

	/*	2007.07.09 maru �@�\�R�[�h�̔����ǉ�
		�_�u���N���b�N����̃h���b�O�ł͒P��P�ʂ͈̔͑I��(�G�f�B�^�̈�ʓI����)�ɂȂ邪
		���̓���́A�_�u���N���b�N���P��I����O��Ƃ������́B
		�L�[���蓖�Ă̕ύX�ɂ��A�_�u���N���b�N���P��I���̂Ƃ��ɂ� GetSelectionInfo().m_bBeginWordSelect = true
		�ɂ���ƁA�����̓��e�ɂ���Ă͕\�������������Ȃ�̂ŁA�����Ŕ�����悤�ɂ���B
	*/
	if (F_SELECTWORD != nFuncID) return;

	// �͈͑I���J�n & �}�E�X�L���v�`���[
	GetSelectionInfo().SelectBeginWord();

	if (GetDllShareData().common.view.bFontIs_FixedPitch) {	// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
		// ALT�L�[��������Ă�����
		if (GetKeyState_Alt()) {
			GetSelectionInfo().SetBoxSelect(true);	// ��`�͈͑I��
		}
	}
	::SetCapture(GetHwnd());
	GetCaret().HideCaret_(GetHwnd()); // 2002/07/22 novice
	if (GetSelectionInfo().IsTextSelected()) {
		// �펞�I��͈͈͂̔�
		GetSelectionInfo().m_selectBgn.SetTo(GetSelectionInfo().m_select.GetTo());
	}else {
		// ���݂̃J�[�\���ʒu����I�����J�n����
		GetSelectionInfo().BeginSelectArea();
	}

	return;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           D&D                               //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

STDMETHODIMP EditView::DragEnter(
	LPDATAOBJECT pDataObject,
	DWORD dwKeyState,
	POINTL pt,
	LPDWORD pdwEffect
	)
{
	DEBUG_TRACE(_T("EditView::DragEnter()\n"));
	//�uOLE�ɂ��h���b�O & �h���b�v���g���v�I�v�V�����������̏ꍇ�ɂ̓h���b�v���󂯕t���Ȃ�
	if (!GetDllShareData().common.edit.bUseOLE_DragDrop) return E_UNEXPECTED;

	// �ҏW�֎~�̏ꍇ�̓h���b�v���󂯕t���Ȃ�
	if (!m_pEditDoc->IsEditable()) return E_UNEXPECTED;


	if (!pDataObject || !pdwEffect)
		return E_INVALIDARG;

	m_cfDragData = GetAvailableClipFormat(pDataObject);
	if (m_cfDragData == 0)
		return E_INVALIDARG;
	else if (m_cfDragData == CF_HDROP) {
		// �E�{�^���œ����Ă����Ƃ������t�@�C�����r���[�Ŏ�舵��
		if (!(MK_RBUTTON & dwKeyState))
			return E_INVALIDARG;
	}
	
	// �������A�N�e�B�u�y�C���ɂ���
	m_pEditWnd->SetActivePane(m_nMyIndex);

	// ���݂̃J�[�\���ʒu���L������	// 2007.12.09 ryoji
	m_ptCaretPos_DragEnter = GetCaret().GetCaretLayoutPos();
	m_nCaretPosX_Prev_DragEnter = GetCaret().m_nCaretPosX_Prev;

	// �h���b�O�f�[�^�͋�`��
	m_bDragBoxData = IsDataAvailable(pDataObject, (CLIPFORMAT)::RegisterClipboardFormat(_T("MSDEVColumnSelect")));

	// �I���e�L�X�g�̃h���b�O����
	_SetDragMode(TRUE);

	DragOver(dwKeyState, pt, pdwEffect);
	return S_OK;
}

STDMETHODIMP EditView::DragOver(DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect)
{
	DEBUG_TRACE(_T("EditView::DragOver()\n"));

	// �}�E�X�ړ��̃��b�Z�[�W����
	::ScreenToClient(GetHwnd(), (LPPOINT)&pt);
	OnMOUSEMOVE(dwKeyState, pt.x , pt.y);

	if (!pdwEffect)
		return E_INVALIDARG;

	*pdwEffect = TranslateDropEffect(m_cfDragData, dwKeyState, pt, *pdwEffect);

	EditView* pDragSourceView = m_pEditWnd->GetDragSourceView();

	// �h���b�O�������r���[�ŁA���̃r���[�̃J�[�\�����h���b�O���̑I��͈͓��̏ꍇ�͋֎~�}�[�N�ɂ���
	// �����r���[�̂Ƃ��͋֎~�}�[�N�ɂ��Ȃ��i���A�v���ł������͂����Ȃ��Ă���͗l�j	// 2009.06.09 ryoji
	if (pDragSourceView && !IsDragSource() &&
		!pDragSourceView->IsCurrentPositionSelected(GetCaret().GetCaretLayoutPos())
	) {
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

STDMETHODIMP EditView::DragLeave(void)
{
	DEBUG_TRACE(_T("EditView::DragLeave()\n"));
	// �I���e�L�X�g�̃h���b�O����
	_SetDragMode(FALSE);

	// DragEnter���̃J�[�\���ʒu�𕜌�	// 2007.12.09 ryoji
	// ���͈͑I�𒆂̂Ƃ��ɑI��͈͂ƃJ�[�\������������ƕς�����
	GetCaret().MoveCursor(m_ptCaretPos_DragEnter, false);
	GetCaret().m_nCaretPosX_Prev = m_nCaretPosX_Prev_DragEnter;
	RedrawAll();	// ���[���[�A�A���_�[���C���A�J�[�\���ʒu�\���X�V

	// ��A�N�e�B�u���͕\����Ԃ��A�N�e�B�u�ɖ߂�	// 2007.12.09 ryoji
	if (!::GetActiveWindow())
		OnKillFocus();

	return S_OK;
}

STDMETHODIMP EditView::Drop(LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect)
{
	DEBUG_TRACE(_T("EditView::Drop()\n"));
	BOOL		bBoxData;
	BOOL		bMove;
	bool		bMoveToPrev = false;
	RECT		rcSel;
	NativeW		memBuf;
	bool		bBeginBoxSelect_Old = false;

	LayoutRange selectBgn_Old;
	LayoutRange select_Old;

	// �I���e�L�X�g�̃h���b�O����
	_SetDragMode(FALSE);

	// ��A�N�e�B�u���͕\����Ԃ��A�N�e�B�u�ɖ߂�	// 2007.12.09 ryoji
	if (!::GetActiveWindow())
		OnKillFocus();

	if (!pDataObject || !pdwEffect)
		return E_INVALIDARG;

	CLIPFORMAT cf;
	cf = GetAvailableClipFormat(pDataObject);
	if (cf == 0)
		return E_INVALIDARG;

	*pdwEffect = TranslateDropEffect(cf, dwKeyState, pt, *pdwEffect);
	if (*pdwEffect == DROPEFFECT_NONE)
		return E_INVALIDARG;

	// �t�@�C���h���b�v�� PostMyDropFiles() �ŏ�������
	if (cf == CF_HDROP)
		return PostMyDropFiles(pDataObject);

	// �O������̃h���b�v�͈Ȍ�̏����ł̓R�s�[�Ɠ��l�Ɉ���
	EditView* pDragSourceView = m_pEditWnd->GetDragSourceView();
	bMove = (*pdwEffect == DROPEFFECT_MOVE) && pDragSourceView;
	bBoxData = m_bDragBoxData;

	// �J�[�\�����I��͈͓��ɂ���Ƃ��̓R�s�[�^�ړ����Ȃ�	// 2009.06.09 ryoji
	if (pDragSourceView &&
		!pDragSourceView->IsCurrentPositionSelected(GetCaret().GetCaretLayoutPos())
	) {
		// DragEnter���̃J�[�\���ʒu�𕜌�
		// Note. �h���b�O�������r���[�ł��}�E�X�ړ��������ƋH�ɂ����ɂ���\�������肻��
		*pdwEffect = DROPEFFECT_NONE;
		GetCaret().MoveCursor(m_ptCaretPos_DragEnter, false);
		GetCaret().m_nCaretPosX_Prev = m_nCaretPosX_Prev_DragEnter;
		if (!IsDragSource())	// �h���b�O���̏ꍇ�͂����ł͍ĕ`��s�v�iDragDrop�㏈����SetActivePane�ōĕ`�悳���j
			RedrawAll();	// ����ɈȌ�̔�A�N�e�B�u���ɔ����A���_�[���C�������̂��߂Ɉ�x�X�V���Đ������Ƃ�
		return S_OK;
	}

	// �h���b�v�f�[�^�̎擾
	HGLOBAL hData = GetGlobalData(pDataObject, cf);
	if (!hData)
		return E_INVALIDARG;
	LPVOID pData = ::GlobalLock(hData);
	SIZE_T nSize = ::GlobalSize(hData);
	if (cf == Clipboard::GetSakuraFormat()) {
		if (nSize > sizeof(int)) {
			wchar_t* pszData = (wchar_t*)((BYTE*)pData + sizeof(int));
			memBuf.SetString(pszData, t_min((SIZE_T)*(int*)pData, nSize / sizeof(wchar_t)));	// �r����NUL�������܂߂�
		}
	}else if (cf == CF_UNICODETEXT) {
		memBuf.SetString((wchar_t*)pData, wcsnlen((wchar_t*)pData, nSize / sizeof(wchar_t)));
	}else {
		memBuf.SetStringOld((char*)pData, strnlen((char*)pData, nSize / sizeof(char)));
	}

	// �A���h�D�o�b�t�@�̏���
	if (!m_commander.GetOpeBlk()) {
		m_commander.SetOpeBlk(new OpeBlk);
	}
	m_commander.GetOpeBlk()->AddRef();

	// �ړ��̏ꍇ�A�ʒu�֌W���Z�o
	if (bMove) {
		if (bBoxData) {
			// 2�_��Ίp�Ƃ����`�����߂�
			TwoPointToRect(
				&rcSel,
				pDragSourceView->GetSelectionInfo().m_select.GetFrom(),	// �͈͑I���J�n
				pDragSourceView->GetSelectionInfo().m_select.GetTo()		// �͈͑I���I��
			);
			++rcSel.bottom;
			if (GetCaret().GetCaretLayoutPos().GetY() >= rcSel.bottom) {
				bMoveToPrev = false;
			}else if (GetCaret().GetCaretLayoutPos().GetY() + rcSel.bottom - rcSel.top < rcSel.top) {
				bMoveToPrev = true;
			}else if (GetCaret().GetCaretLayoutPos().GetX2() < rcSel.left) {
				bMoveToPrev = true;
			}else {
				bMoveToPrev = false;
			}
		}else {
			if (pDragSourceView->GetSelectionInfo().m_select.GetFrom().y > GetCaret().GetCaretLayoutPos().GetY()) {
				bMoveToPrev = true;
			}else if (pDragSourceView->GetSelectionInfo().m_select.GetFrom().y == GetCaret().GetCaretLayoutPos().GetY()) {
				if (pDragSourceView->GetSelectionInfo().m_select.GetFrom().x > GetCaret().GetCaretLayoutPos().GetX2()) {
					bMoveToPrev = true;
				}else {
					bMoveToPrev = false;
				}
			}else {
				bMoveToPrev = false;
			}
		}
	}

	LayoutPoint ptCaretPos_Old = GetCaret().GetCaretLayoutPos();
	if (!bMove) {
		// �R�s�[���[�h
		// ���݂̑I��͈͂��I����Ԃɖ߂�
		GetSelectionInfo().DisableSelectArea(true);
	}else {
		bBeginBoxSelect_Old = pDragSourceView->GetSelectionInfo().IsBoxSelecting();
		selectBgn_Old = pDragSourceView->GetSelectionInfo().m_selectBgn;
		select_Old = pDragSourceView->GetSelectionInfo().m_select;
		if (bMoveToPrev) {
			// �ړ����[�h & �O�Ɉړ�
			// �I���G���A���폜
			if (this != pDragSourceView) {
				pDragSourceView->GetSelectionInfo().DisableSelectArea(true);
				GetSelectionInfo().DisableSelectArea(true);
				GetSelectionInfo().SetBoxSelect(bBeginBoxSelect_Old);
				GetSelectionInfo().m_selectBgn = selectBgn_Old;
				GetSelectionInfo().m_select = select_Old;
			}
			DeleteData(true);
			GetCaret().MoveCursor(ptCaretPos_Old, true);
		}else {
			// ���݂̑I��͈͂��I����Ԃɖ߂�
			pDragSourceView->GetSelectionInfo().DisableSelectArea(true);
			if (this != pDragSourceView)
				GetSelectionInfo().DisableSelectArea(true);
		}
	}
	if (!bBoxData) {	// ��`�f�[�^
		//	2004,05.14 Moca �����ɕ����񒷂�ǉ�

		// �}���O�̃L�����b�g�ʒu���L������
		// �i�L�����b�g���s�I�[���E�̏ꍇ�͖��ߍ��܂��󔒕��������ʒu���V�t�g�j
		LogicPoint ptCaretLogicPos_Old = GetCaret().GetCaretLogicPos();
		const Layout* pLayout;
		LogicInt nLineLen;
		LayoutPoint ptCaretLayoutPos_Old = GetCaret().GetCaretLayoutPos();
		if (m_pEditDoc->m_layoutMgr.GetLineStr(ptCaretLayoutPos_Old.GetY2(), &nLineLen, &pLayout)) {
			LayoutInt nLineAllColLen;
			LineColumnToIndex2(pLayout, ptCaretLayoutPos_Old.GetX2(), &nLineAllColLen);
			if (nLineAllColLen > LayoutInt(0)) {	// �s�I�[���E�̏ꍇ�ɂ� nLineAllColLen �ɍs�S�̂̕\�������������Ă���
				ptCaretLogicPos_Old.SetX(
					ptCaretLogicPos_Old.GetX2()
					+ (Int)(ptCaretLayoutPos_Old.GetX2() - nLineAllColLen)
				);
			}
		}

		GetCommander().Command_INSTEXT(true, memBuf.GetStringPtr(), memBuf.GetStringLength(), FALSE);

		// �}���O�̃L�����b�g�ʒu����}����̃L�����b�g�ʒu�܂ł�I��͈͂ɂ���
		LayoutPoint ptSelectFrom;
		m_pEditDoc->m_layoutMgr.LogicToLayout(
			ptCaretLogicPos_Old,
			&ptSelectFrom
		);
		GetSelectionInfo().SetSelectArea(LayoutRange(ptSelectFrom, GetCaret().GetCaretLayoutPos()));	// 2009.07.25 ryoji
	}else {
		// 2004.07.12 Moca �N���b�v�{�[�h�����������Ȃ��悤��
		// bBoxSelected == TRUE
		// GetSelectionInfo().IsBoxSelecting() == FALSE
		// �\��t���i�N���b�v�{�[�h����\��t���j
		GetCommander().Command_PASTEBOX(memBuf.GetStringPtr(), memBuf.GetStringLength());
		AdjustScrollBars(); // 2007.07.22 ryoji
		Redraw();
	}
	if (bMove) {
		if (bMoveToPrev) {
		}else {
			// �ړ����[�h & ���Ɉړ�

			// ���݂̑I��͈͂��L������	// 2008.03.26 ryoji
			LogicRange selLogic;
			m_pEditDoc->m_layoutMgr.LayoutToLogic(
				GetSelectionInfo().m_select,
				&selLogic
			);

			// �ȑO�̑I��͈͂��L������	// 2008.03.26 ryoji
			LogicRange delLogic;
			m_pEditDoc->m_layoutMgr.LayoutToLogic(
				select_Old,
				&delLogic
			);

			// ���݂̍s�����L������	// 2008.03.26 ryoji
			int nLines_Old = m_pEditDoc->m_docLineMgr.GetLineCount();

			// �ȑO�̑I��͈͂�I������
			GetSelectionInfo().SetBoxSelect(bBeginBoxSelect_Old);
			GetSelectionInfo().m_selectBgn = selectBgn_Old;
			GetSelectionInfo().m_select = select_Old;

			// �I���G���A���폜
			DeleteData(true);

			// �폜�O�̑I��͈͂𕜌�����	// 2008.03.26 ryoji
			if (!bBoxData) {
				// �폜���ꂽ�͈͂��l�����đI��͈͂𒲐�����
				if (selLogic.GetFrom().GetY2() == delLogic.GetTo().GetY2()) {	// �I���J�n���폜�����Ɠ���s
					selLogic.SetFromX(
						selLogic.GetFrom().GetX2()
						- (delLogic.GetTo().GetX2() - delLogic.GetFrom().GetX2())
					);
				}
				if (selLogic.GetTo().GetY2() == delLogic.GetTo().GetY2()) {	// �I���I�����폜�����Ɠ���s
					selLogic.SetToX(
						selLogic.GetTo().GetX2()
						- (delLogic.GetTo().GetX2() - delLogic.GetFrom().GetX2())
					);
				}
				// Note.
				// (delLogic.GetTo().GetY2() - delLogic.GetFrom().GetY2()) ���Ǝ��ۂ̍폜�s���Ɠ����ɂȂ�
				// ���Ƃ����邪�A�i�폜�s���|�P�j�ɂȂ邱�Ƃ�����D
				// ��j�t���[�J�[�\���ł̍s�ԍ��N���b�N���̂P�s�I��
				int nLines = m_pEditDoc->m_docLineMgr.GetLineCount();
				selLogic.SetFromY(selLogic.GetFrom().GetY2() - (nLines_Old - nLines));
				selLogic.SetToY(selLogic.GetTo().GetY2() - (nLines_Old - nLines));

				// ������̑I��͈͂�ݒ肷��
				LayoutRange select;
				m_pEditDoc->m_layoutMgr.LogicToLayout(
					selLogic,
					&select
				);
				GetSelectionInfo().SetSelectArea(select);	// 2009.07.25 ryoji
				ptCaretPos_Old = GetSelectionInfo().m_select.GetTo();
			}

			// �L�����b�g���ړ�����
			GetCaret().MoveCursor(ptCaretPos_Old, true);
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

			// �폜�ʒu����ړ���ւ̃J�[�\���ړ���Undo����ɒǉ�����	// 2008.03.26 ryoji
			LogicPoint ptBefore;
			m_pEditDoc->m_layoutMgr.LayoutToLogic(
				GetSelectionInfo().m_select.GetFrom(),
				&ptBefore
			);
			m_commander.GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					delLogic.GetFrom(),
					GetCaret().GetCaretLogicPos()
				)
			);
		}
	}
	GetSelectionInfo().DrawSelectArea();

	// �A���h�D�o�b�t�@�̏���
	SetUndoBuffer();

	::GlobalUnlock(hData);
	// 2004.07.12 fotomo/���� �������[���[�N�̏C��
	if ((GMEM_LOCKCOUNT & ::GlobalFlags(hData)) == 0) {
		::GlobalFree(hData);
	}

	return S_OK;
}


/** �Ǝ��h���b�v�t�@�C�����b�Z�[�W���|�X�g����
	@date 2008.06.20 ryoji �V�K�쐬
*/
STDMETHODIMP EditView::PostMyDropFiles(LPDATAOBJECT pDataObject)
{
	HGLOBAL hData = GetGlobalData(pDataObject, CF_HDROP);
	if (!hData)
		return E_INVALIDARG;
	LPVOID pData = ::GlobalLock(hData);
	SIZE_T nSize = ::GlobalSize(hData);

	// �h���b�v�f�[�^���R�s�[���Ă��ƂœƎ��̃h���b�v�t�@�C���������s��
	HGLOBAL hDrop = ::GlobalAlloc(GHND | GMEM_DDESHARE, nSize);
	memcpy_raw(::GlobalLock(hDrop), pData, nSize);
	::GlobalUnlock(hDrop);
	::PostMessage(
		GetHwnd(),
		MYWM_DROPFILES,
		(WPARAM)hDrop,
		0
	);

	::GlobalUnlock(hData);
	if ((GMEM_LOCKCOUNT & ::GlobalFlags(hData)) == 0) {
		::GlobalFree(hData);
	}

	return S_OK;
}

/** �Ǝ��h���b�v�t�@�C�����b�Z�[�W����
	@date 2008.06.20 ryoji �V�K�쐬
*/
void EditView::OnMyDropFiles(HDROP hDrop)
{
	// ���ʂɃ��j���[���삪�ł���悤�ɓ��͏�Ԃ��t�H�A�O�����h�E�B���h�E�ɃA�^�b�`����
	int nTid2 = ::GetWindowThreadProcessId(::GetForegroundWindow(), NULL);
	int nTid1 = ::GetCurrentThreadId();
	if (nTid1 != nTid2) ::AttachThreadInput(nTid1, nTid2, TRUE);
	
	// �_�~�[�� STATIC ������ăt�H�[�J�X�𓖂Ă�i�G�f�B�^���O�ʂɏo�Ȃ��悤�Ɂj
	HWND hwnd = ::CreateWindow(_T("STATIC"), _T(""), 0, 0, 0, 0, 0, NULL, NULL, G_AppInstance(), NULL);
	::SetFocus(hwnd);

	// ���j���[���쐬����
	POINT pt;
	::GetCursorPos(&pt);
	RECT rcWork;
	GetMonitorWorkRect(pt, &rcWork);	// ���j�^�̃��[�N�G���A
	HMENU hMenu = ::CreatePopupMenu();
	::InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, 100, LS(STR_VIEW_MOUSE_MENU_PATH));
	::InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING, 101, LS(STR_VIEW_MOUSE_MENU_FILE));
	::InsertMenu(hMenu, 2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);	// �Z�p���[�^
	::InsertMenu(hMenu, 3, MF_BYPOSITION | MF_STRING, 110, LS(STR_VIEW_MOUSE_MENU_OPEN));
	::InsertMenu(hMenu, 4, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);	// �Z�p���[�^
	::InsertMenu(hMenu, 5, MF_BYPOSITION | MF_STRING, IDCANCEL, LS(STR_VIEW_MOUSE_MENU_CANCEL));
	int nId = ::TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
									(pt.x > rcWork.left)? pt.x: rcWork.left,
									(pt.y < rcWork.bottom)? pt.y: rcWork.bottom,
								0, hwnd, NULL);
	::DestroyMenu(hMenu);

	::DestroyWindow(hwnd);

	// ���͏�Ԃ��f�^�b�`����
	if (nTid1 != nTid2) ::AttachThreadInput(nTid1, nTid2, FALSE);

	// �I�����ꂽ���j���[�ɑΉ����鏈�������s����
	switch (nId) {
	case 110:	// �t�@�C�����J��
		// �ʏ�̃h���b�v�t�@�C���������s��
		::SendMessage(m_pEditWnd->GetHwnd(), WM_DROPFILES, (WPARAM)hDrop, 0);
		break;

	case 100:	// �p�X����\��t����
	case 101:	// �t�@�C������\��t����
		NativeW memBuf;
		UINT nFiles;
		TCHAR szPath[_MAX_PATH];
		TCHAR szExt[_MAX_EXT];
		TCHAR szWork[_MAX_PATH];

		nFiles = ::DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
		for (UINT i=0; i<nFiles; ++i) {
			::DragQueryFile(hDrop, i, szPath, sizeof(szPath)/sizeof(TCHAR));
			if (!::GetLongFileName(szPath, szWork))
				continue;
			if (nId == 100) {	// �p�X��
				::lstrcpy(szPath, szWork);
			}else if (nId == 101) {	// �t�@�C����
				_tsplitpath(szWork, NULL, NULL, szPath, szExt);
				::lstrcat(szPath, szExt);
			}
#ifdef _UNICODE
			memBuf.AppendString(szPath);
#else
			memBuf.AppendStringOld(szPath);
#endif
			if (nFiles > 1) {
				memBuf.AppendString(m_pEditDoc->m_docEditor.GetNewLineCode().GetValue2());
			}
		}
		::DragFinish(hDrop);

		// �I��͈͂̑I������
		if (GetSelectionInfo().IsTextSelected()) {
			GetSelectionInfo().DisableSelectArea(true);
		}

		// �}���O�̃L�����b�g�ʒu���L������
		// �i�L�����b�g���s�I�[���E�̏ꍇ�͖��ߍ��܂��󔒕��������ʒu���V�t�g�j
		LogicPoint ptCaretLogicPos_Old = GetCaret().GetCaretLogicPos();
		const Layout* pLayout;
		LogicInt nLineLen;
		LayoutPoint ptCaretLayoutPos_Old = GetCaret().GetCaretLayoutPos();
		if (m_pEditDoc->m_layoutMgr.GetLineStr(ptCaretLayoutPos_Old.GetY2(), &nLineLen, &pLayout)) {
			LayoutInt nLineAllColLen;
			LineColumnToIndex2(pLayout, ptCaretLayoutPos_Old.GetX2(), &nLineAllColLen);
			if (nLineAllColLen > LayoutInt(0)) {	// �s�I�[���E�̏ꍇ�ɂ� nLineAllColLen �ɍs�S�̂̕\�������������Ă���
				ptCaretLogicPos_Old.SetX(
					ptCaretLogicPos_Old.GetX2()
					+ (Int)(ptCaretLayoutPos_Old.GetX2() - nLineAllColLen)
				);
			}
		}

		// �e�L�X�g�}��
		GetCommander().HandleCommand(F_INSTEXT_W, true, (LPARAM)memBuf.GetStringPtr(), memBuf.GetStringLength(), TRUE, 0);

		// �}���O�̃L�����b�g�ʒu����}����̃L�����b�g�ʒu�܂ł�I��͈͂ɂ���
		LayoutPoint ptSelectFrom;
		m_pEditDoc->m_layoutMgr.LogicToLayout(
			ptCaretLogicPos_Old,
			&ptSelectFrom
		);
		GetSelectionInfo().SetSelectArea(LayoutRange(ptSelectFrom, GetCaret().GetCaretLayoutPos()));	// 2009.07.25 ryoji
		GetSelectionInfo().DrawSelectArea();
		break;
	}

	// ���������
	::GlobalFree(hDrop);
}

CLIPFORMAT EditView::GetAvailableClipFormat(LPDATAOBJECT pDataObject)
{
	CLIPFORMAT cf = 0;
	CLIPFORMAT cfSAKURAClip = Clipboard::GetSakuraFormat();

	if (IsDataAvailable(pDataObject, cfSAKURAClip))
		cf = cfSAKURAClip;
	else if (IsDataAvailable(pDataObject, CF_UNICODETEXT))
		cf = CF_UNICODETEXT;
	else if (IsDataAvailable(pDataObject, CF_TEXT))
		cf = CF_TEXT;
	else if (IsDataAvailable(pDataObject, CF_HDROP))	// 2008.06.20 ryoji
		cf = CF_HDROP;

	return cf;
}

DWORD EditView::TranslateDropEffect(CLIPFORMAT cf, DWORD dwKeyState, POINTL pt, DWORD dwEffect)
{
	if (cf == CF_HDROP)	// 2008.06.20 ryoji
		return DROPEFFECT_LINK;

	EditView* pDragSourceView = m_pEditWnd->GetDragSourceView();

	// 2008.06.21 ryoji
	// Win 98/Me ���ł͊O������̃h���b�O���� GetKeyState() �ł̓L�[��Ԃ𐳂����擾�ł��Ȃ����߁A
	// Drag & Drop �C���^�[�t�F�[�X�œn����� dwKeyState ��p���Ĕ��肷��B
#if 1
	// �h���b�O�����O���E�B���h�E���ǂ����ɂ���Ď󂯕���ς���
	// ���ėp�e�L�X�g�G�f�B�^�ł͂����炪�嗬���ۂ�
	if (pDragSourceView) {
#else
	// �h���b�O�����ړ����������ǂ����ɂ���Ď󂯕���ς���
	// ��MS ���i�iMS Office, Visual Studio�Ȃǁj�ł͂����炪�嗬���ۂ�
	if (dwEffect & DROPEFFECT_MOVE) {
#endif
		dwEffect &= (dwKeyState & MK_CONTROL)? DROPEFFECT_COPY: DROPEFFECT_MOVE;
	}else {
		dwEffect &= (dwKeyState & MK_SHIFT)? DROPEFFECT_MOVE: DROPEFFECT_COPY;
	}
	return dwEffect;
}

bool EditView::IsDragSource(void)
{
	return (this == m_pEditWnd->GetDragSourceView());
}


