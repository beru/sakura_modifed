/*!	@file
@brief ViewCommander�N���X�̃R�}���h(�ҏW�n ��{�`)�֐��Q

	2012/12/16	ViewCommander.cpp,ViewCommander_New.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, genta
	Copyright (C) 2003, MIK, genta, �����, zenryaku, Moca, ryoji, naoh, KEITA, ���イ��
	Copyright (C) 2005, genta, D.S.Koba, ryoji
	Copyright (C) 2007, ryoji, kobake
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2010, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "view/Ruler.h"
#include "uiparts/WaitCursor.h"
#include "plugin/JackManager.h"
#include "plugin/SmartIndentIfObj.h"
#include "debug/RunningTimer.h"

// wchar_t1���̕��������
void ViewCommander::Command_WCHAR(
	wchar_t wcChar,
	bool bConvertEOL
	)
{
	auto& selInfo = m_view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// �}�E�X�ɂ��͈͑I��
		ErrorBeep();
		return;
	}

	LogicInt nPos;
	auto& doc = GetDocument();
	doc.m_docEditor.SetModified(true, true);	// Jan. 22, 2002 genta

	if (m_view.m_bHideMouse && 0 <= m_view.m_nMousePouse) {
		m_view.m_nMousePouse = -1;
		::SetCursor(NULL);
	}

	auto& caret = GetCaret();
	auto& typeData = m_view.m_pTypeData;

	// ���݈ʒu�Ƀf�[�^��}��
	NativeW memDataW2;
	memDataW2 = wcChar;
	if (WCODE::IsLineDelimiter(wcChar, GetDllShareData().common.edit.bEnableExtEol)) { 
		// ���݁AEnter�Ȃǂő}��������s�R�[�h�̎�ނ��擾
		if (bConvertEOL) {
			Eol cWork = doc.m_docEditor.GetNewLineCode();
			memDataW2.SetString(cWork.GetValue2(), cWork.GetLen());
		}

		// �e�L�X�g���I������Ă��邩
		if (selInfo.IsTextSelected()) {
			m_view.DeleteData(true);
		}
		if (typeData->bAutoIndent) {	// �I�[�g�C���f���g
			const Layout* pLayout;
			LogicInt nLineLen;
			auto& layoutMgr = doc.m_layoutMgr;
			const wchar_t* pLine = layoutMgr.GetLineStr(caret.GetCaretLayoutPos().GetY2(), &nLineLen, &pLayout);
			if (pLayout) {
				const DocLine* pDocLine = doc.m_docLineMgr.GetLine(pLayout->GetLogicLineNo());
				pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
				if (pLine) {
					/*
					  �J�[�\���ʒu�ϊ�
					  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
					  ��
					  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
					*/
					LogicPoint ptXY;
					layoutMgr.LayoutToLogic(
						caret.GetCaretLayoutPos(),
						&ptXY
					);

					// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
					for (nPos=LogicInt(0); nPos<nLineLen && nPos<ptXY.GetX2();) {
						// 2005-09-02 D.S.Koba GetSizeOfChar
						LogicInt nCharChars = NativeW::GetSizeOfChar(pLine, nLineLen, nPos);

						// ���̑��̃C���f���g����
						if (0 < nCharChars
						 && pLine[nPos] != L'\0'	// ���̑��̃C���f���g������ L'\0' �͊܂܂�Ȃ�	// 2009.02.04 ryoji L'\0'���C���f���g����Ă��܂����C��
						 && typeData->szIndentChars[0] != L'\0'
						) {
							wchar_t szCurrent[10];
							wmemcpy(szCurrent, &pLine[nPos], nCharChars);
							szCurrent[nCharChars] = L'\0';
							// ���̑��̃C���f���g�Ώە���
							if (wcsstr(
									typeData->szIndentChars,
									szCurrent
								)
							) {
								goto end_of_for;
							}
						}
						
						{
							bool bZenSpace = typeData->bAutoIndent_ZENSPACE;
							if (nCharChars == 1 && WCODE::IsIndentChar(pLine[nPos], bZenSpace)) {
								// ���֐i��
							}
							else break;
						}

end_of_for:;
						nPos += nCharChars;
					}

					// �C���f���g�擾
					//NativeW memIndent;
					//memIndent.SetString(pLine, nPos);

					// �C���f���g�t��
					memDataW2.AppendString(pLine, nPos);
				}
			}
		}
	}else {
		// �e�L�X�g���I������Ă��邩
		if (selInfo.IsTextSelected()) {
			// ��`�͈͑I�𒆂�
			if (selInfo.IsBoxSelecting()) {
				Command_INDENT(wcChar);
				return;
			}else {
				m_view.DeleteData(true);
			}
		}else {
			if (!m_view.IsInsMode() /* Oct. 2, 2005 genta */) {
				DelCharForOverwrite(&wcChar, 1);	// �㏑���p�̈ꕶ���폜	// 2009.04.11 ryoji
			}
		}
	}

	// �{���ɑ}������
	LayoutPoint ptLayoutNew;
	m_view.InsertData_CEditView(
		caret.GetCaretLayoutPos(),
		memDataW2.GetStringPtr(),
		memDataW2.GetStringLength(),
		&ptLayoutNew,
		true
	);

	// �}���f�[�^�̍Ō�փJ�[�\�����ړ�
	caret.MoveCursor(ptLayoutNew, true);
	caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();

	// �X�}�[�g�C���f���g
	SmartIndentType nSIndentType = typeData->eSmartIndent;
	switch (nSIndentType) {	// �X�}�[�g�C���f���g���
	case SmartIndentType::None:
		break;
	case SmartIndentType::Cpp:
		// C/C++�X�}�[�g�C���f���g����
		m_view.SmartIndent_CPP(wcChar);
		break;
	default:
		// �v���O�C�����猟������
		{
			Plug::Array plugs;
			JackManager::getInstance().GetUsablePlug(PP_SMARTINDENT, (PlugId)nSIndentType, &plugs);

			if (plugs.size() > 0) {
				assert_warning(plugs.size() == 1);
				// �C���^�t�F�[�X�I�u�W�F�N�g����
				WSHIfObj::List params;
				SmartIndentIfObj* objIndent = new SmartIndentIfObj(wcChar);	// �X�}�[�g�C���f���g�I�u�W�F�N�g
				objIndent->AddRef();
				params.push_back(objIndent);

				// �L�[���͂��A���h�D�o�b�t�@�ɔ��f
				m_view.SetUndoBuffer();

				// �L�[���͂Ƃ͕ʂ̑���u���b�N�ɂ���i�������v���O�C�����̑���͂܂Ƃ߂�j
				if (!GetOpeBlk()) {
					SetOpeBlk(new OpeBlk);
				}
				GetOpeBlk()->AddRef();	// ��Release��HandleCommand�̍Ō�ōs��

				// �v���O�C���Ăяo��
				(*plugs.begin())->Invoke(&m_view, params);
				objIndent->Release();
			}
		}
		break;
	}

	// 2005.10.11 ryoji ���s���ɖ����̋󔒂��폜
	if (WCODE::IsLineDelimiter(
			wcChar,
			GetDllShareData().common.edit.bEnableExtEol
		)
		&& typeData->bRTrimPrevLine
	) {	// ���s���ɖ����̋󔒂��폜
		// �O�̍s�ɂ��閖���̋󔒂��폜����
		m_view.RTrimPrevLine();
	}

	m_view.PostprocessCommand_hokan();	// Jan. 10, 2005 genta �֐���
}


/*!
	@brief 2�o�C�g��������
	
	WM_IME_CHAR�ő����Ă�����������������D
	�������C�}�����[�h�ł�WM_IME_CHAR�ł͂Ȃ�WM_IME_COMPOSITION�ŕ������
	�擾����̂ł����ɂ͗��Ȃ��D

	@param wChar [in] SJIS�����R�[�h�D��ʂ�1�o�C�g�ځC���ʂ�2�o�C�g�ځD
	
	@date 2002.10.06 genta �����̏㉺�o�C�g�̈Ӗ����t�]�D
		WM_IME_CHAR��wParam�ɍ��킹���D
*/
void ViewCommander::Command_IME_CHAR(WORD wChar)
{
	auto& selInfo = m_view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// �}�E�X�ɂ��͈͑I��
		ErrorBeep();
		return;
	}

	// Oct. 6 ,2002 genta �㉺�t�]
	if ((wChar & 0xff00) == 0) {
		Command_WCHAR(wChar & 0xff);
		return;
	}
	GetDocument().m_docEditor.SetModified(true, true);	// Jan. 22, 2002 genta

 	if (m_view.m_bHideMouse && 0 <= m_view.m_nMousePouse) {
		m_view.m_nMousePouse = -1;
		::SetCursor(NULL);
	}

	// Oct. 6 ,2002 genta �o�b�t�@�Ɋi�[����
	// Aug. 15, 2007 kobake WCHAR�o�b�t�@�ɕϊ�����
#ifdef _UNICODE
	wchar_t szWord[2] = {wChar, 0};
#else
	ACHAR szAnsiWord[3] = {(wChar >> 8) & 0xff, wChar & 0xff, 0};
	const wchar_t* pUniData = to_wchar(szAnsiWord);
	wchar_t szWord[2] = {pUniData[0], 0};
#endif
	LogicInt nWord = LogicInt(1);

	// �e�L�X�g���I������Ă��邩
	if (selInfo.IsTextSelected()) {
		// ��`�͈͑I�𒆂�
		if (selInfo.IsBoxSelecting()) {
			Command_INDENT(szWord, nWord);	// Oct. 6 ,2002 genta 
			return;
		}else {
			m_view.DeleteData(true);
		}
	}else {
		if (!m_view.IsInsMode()) {	// Oct. 2, 2005 genta
			DelCharForOverwrite(szWord, nWord);	// �㏑���p�̈ꕶ���폜	// 2009.04.11 ryoji
		}
	}

	// Oct. 6 ,2002 genta 
	LayoutPoint ptLayoutNew;
	auto& caret = GetCaret();
	m_view.InsertData_CEditView(caret.GetCaretLayoutPos(), szWord, nWord, &ptLayoutNew, true);

	// �}���f�[�^�̍Ō�փJ�[�\�����ړ�
	caret.MoveCursor(ptLayoutNew, true);
	caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();

	m_view.PostprocessCommand_hokan();	// Jan. 10, 2005 genta �֐���
}


// from ViewCommander_New.cpp
// Undo ���ɖ߂�
void ViewCommander::Command_UNDO(void)
{
	auto& selInfo = m_view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// �}�E�X�ɂ��͈͑I��
		ErrorBeep();
		return;
	}

	{
		OpeBlk* opeBlk = m_view.m_commander.GetOpeBlk();
		if (opeBlk) {
			int nCount = opeBlk->GetRefCount();
			opeBlk->SetRefCount(1); // �����I�Ƀ��Z�b�g���邽��1���w��
			m_view.SetUndoBuffer();
			if (!m_view.m_commander.GetOpeBlk() && 0 < nCount) {
				m_view.m_commander.SetOpeBlk(new OpeBlk());
				m_view.m_commander.GetOpeBlk()->SetRefCount(nCount);
			}
		}
	}

	if (!GetDocument().m_docEditor.IsEnableUndo()) {	// Undo(���ɖ߂�)�\�ȏ�Ԃ��H
		return;
	}

	MY_RUNNINGTIMER(runningTimer, "ViewCommander::Command_UNDO()");

	Ope*		pOpe = NULL;

	OpeBlk*	pOpeBlk;
	int			nOpeBlkNum;
	bool		bIsModified;
//	int			nNewLine;	// �}�����ꂽ�����̎��̈ʒu�̍s
//	int			nNewPos;	// �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu

	LayoutPoint ptCaretPos_Before;
	LayoutPoint ptCaretPos_After;

	// �e�탂�[�h�̎�����
	Command_CANCEL_MODE();

	m_view.m_bDoing_UndoRedo = true;	// Undo, Redo�̎��s����

	// ���݂�Undo�Ώۂ̑���u���b�N��Ԃ�
	auto& caret = GetCaret();
	auto& docEditor = GetDocument().m_docEditor;
	if ((pOpeBlk = docEditor.m_opeBuf.DoUndo(&bIsModified))) {
		nOpeBlkNum = pOpeBlk->GetNum();
		bool bDraw = (nOpeBlkNum < 5) && m_view.GetDrawSwitch();
		bool bDrawAll = false;
		const bool bDrawSwitchOld = m_view.SetDrawSwitch(bDraw);	// hor


		WaitCursor waitCursor(m_view.GetHwnd(), 1000 < nOpeBlkNum);
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if (waitCursor.IsEnable()) {
			hwndProgress = m_view.StartProgress();
		}

		const bool bFastMode = (100 < nOpeBlkNum);
		auto& layoutMgr = GetDocument().m_layoutMgr;
		for (int i=nOpeBlkNum-1; i>=0; --i) {
			Ope* pOpe = pOpeBlk->GetOpe(i);
			if (bFastMode) {
				caret.MoveCursorFastMode(pOpe->m_ptCaretPos_PHY_After);
			}else {
				layoutMgr.LogicToLayout(
					pOpe->m_ptCaretPos_PHY_After,
					&ptCaretPos_After
				);
				layoutMgr.LogicToLayout(
					pOpe->m_ptCaretPos_PHY_Before,
					&ptCaretPos_Before
				);

				// �J�[�\�����ړ�
				caret.MoveCursor(ptCaretPos_After, false);
			}

			switch (pOpe->GetCode()) {
			case OpeCode::Insert:
				{
					InsertOpe* pInsertOpe = static_cast<InsertOpe*>(pOpe);

					// �I��͈͂̕ύX
					LogicRange selectLogic;
					selectLogic.SetFrom(pOpe->m_ptCaretPos_PHY_Before);
					selectLogic.SetTo(pOpe->m_ptCaretPos_PHY_After);
					if (bFastMode) {
					}else {
						selInfo.m_selectBgn.SetFrom(ptCaretPos_Before);
						selInfo.m_selectBgn.SetTo(selInfo.m_selectBgn.GetFrom());
						selInfo.m_select.SetFrom(ptCaretPos_Before);
						selInfo.m_select.SetTo(ptCaretPos_After);
					}

					// �f�[�^�u�� �폜&�}���ɂ��g����
					bDrawAll |= m_view.ReplaceData_CEditView3(
						selInfo.m_select,				// �폜�͈�
						&pInsertOpe->m_opeLineData,	// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
						NULL,
						bDraw,						// �ĕ`�悷�邩�ۂ�
						NULL,
						pInsertOpe->m_nOrgSeq,
						NULL,
						bFastMode,
						&selectLogic
					);

					// �I��͈͂̕ύX
					selInfo.m_selectBgn.Clear(-1); // �͈͑I��(���_)
					selInfo.m_select.Clear(-1);
				}
				break;
			case OpeCode::Delete:
				{
					DeleteOpe* pDeleteOpe = static_cast<DeleteOpe*>(pOpe);

					// 2007.10.17 kobake ���������[�N���Ă܂����B�C���B
					if (0 < pDeleteOpe->m_opeLineData.size()) {
						// �f�[�^�u�� �폜&�}���ɂ��g����
						LayoutRange range;
						range.Set(ptCaretPos_Before);
						LogicRange cSelectLogic;
						cSelectLogic.Set(pOpe->m_ptCaretPos_PHY_Before);
						bDrawAll |= m_view.ReplaceData_CEditView3(
							range,
							NULL,										// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
							&pDeleteOpe->m_opeLineData,
							bDraw,										// �ĕ`�悷�邩�ۂ�
							NULL,
							0,
							&pDeleteOpe->m_nOrgSeq,
							bFastMode,
							&cSelectLogic
						);
					}
					pDeleteOpe->m_opeLineData.clear();
				}
				break;
			case OpeCode::Replace:
				{
					ReplaceOpe* pReplaceOpe = static_cast<ReplaceOpe*>(pOpe);

					LayoutRange range;
					range.SetFrom(ptCaretPos_Before);
					range.SetTo(ptCaretPos_After);
					LogicRange cSelectLogic;
					cSelectLogic.SetFrom(pOpe->m_ptCaretPos_PHY_Before);
					cSelectLogic.SetTo(pOpe->m_ptCaretPos_PHY_After);

					// �f�[�^�u�� �폜&�}���ɂ��g����
					bDrawAll |= m_view.ReplaceData_CEditView3(
						range,				// �폜�͈�
						&pReplaceOpe->m_pMemDataIns,	// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
						&pReplaceOpe->m_pMemDataDel,	// �}������f�[�^
						bDraw,						// �ĕ`�悷�邩�ۂ�
						NULL,
						pReplaceOpe->m_nOrgInsSeq,
						&pReplaceOpe->m_nOrgDelSeq,
						bFastMode,
						&cSelectLogic
					);
					pReplaceOpe->m_pMemDataDel.clear();
				}
				break;
			case OpeCode::MoveCaret:
				// �J�[�\�����ړ�
				if (bFastMode) {
					caret.MoveCursorFastMode(pOpe->m_ptCaretPos_PHY_After);
				}else {
					caret.MoveCursor(ptCaretPos_After, false);
				}
				break;
			}

			if (bFastMode) {
				if (i == 0) {
					layoutMgr._DoLayout();
					GetEditWindow()->ClearViewCaretPosInfo();
					if (GetDocument().m_nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
						layoutMgr.CalculateTextWidth();
					}
					layoutMgr.LogicToLayout(
						pOpe->m_ptCaretPos_PHY_Before,
						&ptCaretPos_Before
					);
					caret.MoveCursor(ptCaretPos_Before, true);
					// �ʏ탂�[�h�ł�ReplaceData_CEditView�̒��Őݒ肳���
					caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX();
				}else {
					caret.MoveCursorFastMode(pOpe->m_ptCaretPos_PHY_Before);
				}
			}else {
				layoutMgr.LogicToLayout(
					pOpe->m_ptCaretPos_PHY_Before,
					&ptCaretPos_Before
				);
				// �J�[�\�����ړ�
				caret.MoveCursor(ptCaretPos_Before, (i == 0));
			}
			if (hwndProgress && (i % 100) == 0) {
				int newPos = ::MulDiv(nOpeBlkNum - i, 100, nOpeBlkNum);
				if (newPos != nProgressPos) {
					nProgressPos = newPos;
					Progress_SetPos(hwndProgress, newPos + 1);
					Progress_SetPos(hwndProgress, newPos);
				}
			}
		}
		m_view.SetDrawSwitch(bDrawSwitchOld);	// hor
		m_view.AdjustScrollBars(); // 2007.07.22 ryoji

		// Undo��̕ύX�t���O
		docEditor.SetModified(bIsModified, true);	// Jan. 22, 2002 genta

		m_view.m_bDoing_UndoRedo = false;	// Undo, Redo�̎��s����

		m_view.SetBracketPairPos(true);	// 03/03/07 ai

		// �ĕ`��
		// ���[���[�ĕ`��̕K�v������Ƃ��� DispRuler() �ł͂Ȃ����̕����Ɠ����� Call_OnPaint() �ŕ`�悷��	// 2010.08.20 ryoji
		// �EDispRuler() �̓��[���[�ƃe�L�X�g�̌��ԁi�����͍s�ԍ��̕��ɍ��킹���сj��`�悵�Ă���Ȃ�
		// �E�s�ԍ��\���ɕK�v�ȕ��� OPE_INSERT/OPE_DELETE �������ōX�V����Ă���ύX������΃��[���[�ĕ`��t���O�ɔ��f����Ă���
		// �E����Scroll�����[���[�ĕ`��t���O�ɔ��f����Ă���
		const bool bRedrawRuler = m_view.GetRuler().GetRedrawFlag();
		m_view.Call_OnPaint((int)PaintAreaType::LineNumber | (int)PaintAreaType::Body | (bRedrawRuler? (int)PaintAreaType::Ruler: 0), false);
		if (!bRedrawRuler) {
			// ���[���[�̃L�����b�g�݂̂��ĕ`��
			HDC hdc = m_view.GetDC();
			m_view.GetRuler().DispRuler(hdc);
			m_view.ReleaseDC(hdc);
		}

		caret.ShowCaretPosInfo();	// �L�����b�g�̍s���ʒu��\������	// 2007.10.19 ryoji

		if (!GetEditWindow()->UpdateTextWrap() && bDrawAll) {	// �܂�Ԃ����@�֘A�̍X�V	// 2008.06.10 ryoji
			GetEditWindow()->RedrawAllViews(&m_view);	// ���̃y�C���̕\�����X�V
		}
		if (hwndProgress) {
			::ShowWindow(hwndProgress, SW_HIDE);
		}
	}

	caret.m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;	// 2007.10.11 ryoji �ǉ�
	m_view.m_bDoing_UndoRedo = false;	// Undo, Redo�̎��s����

	return;
}


// from ViewCommander_New.cpp
// Redo ��蒼��
void ViewCommander::Command_REDO(void)
{
	if (m_view.GetSelectionInfo().IsMouseSelecting()) {	// �}�E�X�ɂ��͈͑I��
		ErrorBeep();
		return;
	}

	{
		OpeBlk* opeBlk = m_view.m_commander.GetOpeBlk();
		if (opeBlk) {
			int nCount = opeBlk->GetRefCount();
			opeBlk->SetRefCount(1); // �����I�Ƀ��Z�b�g���邽��1���w��
			m_view.SetUndoBuffer();
			if (!m_view.m_commander.GetOpeBlk() && 0 < nCount) {
				m_view.m_commander.SetOpeBlk(new OpeBlk());
				m_view.m_commander.GetOpeBlk()->SetRefCount(nCount);
			}
		}
		// ���ӁFOpe��ǉ������Redo�͂ł��Ȃ��Ȃ�
	}

	auto& docEditor = GetDocument().m_docEditor;
	if (!docEditor.IsEnableRedo()) {	// Redo(��蒼��)�\�ȏ�Ԃ��H
		return;
	}
	MY_RUNNINGTIMER(runningTimer, "ViewCommander::Command_REDO()");

	Ope*		pOpe = NULL;
	OpeBlk*	pOpeBlk;
	int			nOpeBlkNum;
//	int			nNewLine;	// �}�����ꂽ�����̎��̈ʒu�̍s
//	int			nNewPos;	// �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu
	bool		bIsModified;

	LayoutPoint ptCaretPos_Before;
	LayoutPoint ptCaretPos_To;
	LayoutPoint ptCaretPos_After;

	// �e�탂�[�h�̎�����
	Command_CANCEL_MODE();

	m_view.m_bDoing_UndoRedo = true;	// Undo, Redo�̎��s����

	// ���݂�Redo�Ώۂ̑���u���b�N��Ԃ�
	auto& caret = GetCaret();
	if ((pOpeBlk = docEditor.m_opeBuf.DoRedo(&bIsModified))) {
		nOpeBlkNum = pOpeBlk->GetNum();
		bool bDraw = (nOpeBlkNum < 5) && m_view.GetDrawSwitch();
		bool bDrawAll = false;
		const bool bDrawSwitchOld = m_view.SetDrawSwitch(bDraw);	// 2007.07.22 ryoji

		WaitCursor waitCursor(m_view.GetHwnd(), 1000 < nOpeBlkNum);
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if (waitCursor.IsEnable()) {
			hwndProgress = m_view.StartProgress();
		}

		const bool bFastMode = (100 < nOpeBlkNum);
		auto& layoutMgr = GetDocument().m_layoutMgr;
		for (int i=0; i<nOpeBlkNum; ++i) {
			pOpe = pOpeBlk->GetOpe(i);
			if (bFastMode) {
				if (i == 0) {
					layoutMgr.LogicToLayout(
						pOpe->m_ptCaretPos_PHY_Before,
						&ptCaretPos_Before
					);
					caret.MoveCursor(ptCaretPos_Before, true);
				}else {
					caret.MoveCursorFastMode(pOpe->m_ptCaretPos_PHY_Before);
				}
			}else {
				layoutMgr.LogicToLayout(
					pOpe->m_ptCaretPos_PHY_Before,
					&ptCaretPos_Before
				);
				caret.MoveCursor(ptCaretPos_Before, (i == 0));
			}
			switch (pOpe->GetCode()) {
			case OpeCode::Insert:
				{
					InsertOpe* pInsertOpe = static_cast<InsertOpe*>(pOpe);

					// 2007.10.17 kobake ���������[�N���Ă܂����B�C���B
					if (0 < pInsertOpe->m_opeLineData.size()) {
						// �f�[�^�u�� �폜&�}���ɂ��g����
						LayoutRange range;
						range.Set(ptCaretPos_Before);
						LogicRange cSelectLogic;
						cSelectLogic.Set(pOpe->m_ptCaretPos_PHY_Before);
						bDrawAll |= m_view.ReplaceData_CEditView3(
							range,
							NULL,										// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
							&pInsertOpe->m_opeLineData,				// �}������f�[�^
							bDraw,										// �ĕ`�悷�邩�ۂ�
							NULL,
							0,
							&pInsertOpe->m_nOrgSeq,
							bFastMode,
							&cSelectLogic
						);

					}
					pInsertOpe->m_opeLineData.clear();
				}
				break;
			case OpeCode::Delete:
				{
					DeleteOpe* pDeleteOpe = static_cast<DeleteOpe*>(pOpe);

					if (bFastMode) {
					}else {
						layoutMgr.LogicToLayout(
							pDeleteOpe->m_ptCaretPos_PHY_To,
							&ptCaretPos_To
						);
					}
					LogicRange cSelectLogic;
					cSelectLogic.SetFrom(pOpe->m_ptCaretPos_PHY_Before);
					cSelectLogic.SetTo(pDeleteOpe->m_ptCaretPos_PHY_To);

					// �f�[�^�u�� �폜&�}���ɂ��g����
					bDrawAll |= m_view.ReplaceData_CEditView3(
						LayoutRange(ptCaretPos_Before, ptCaretPos_To),
						&pDeleteOpe->m_opeLineData,	// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
						NULL,
						bDraw,
						NULL,
						pDeleteOpe->m_nOrgSeq,
						NULL,
						bFastMode,
						&cSelectLogic
					);
				}
				break;
			case OpeCode::Replace:
				{
					ReplaceOpe* pReplaceOpe = static_cast<ReplaceOpe*>(pOpe);

					if (bFastMode) {
					}else {
						layoutMgr.LogicToLayout(
							pReplaceOpe->m_ptCaretPos_PHY_To,
							&ptCaretPos_To
						);
					}
					LogicRange cSelectLogic;
					cSelectLogic.SetFrom(pOpe->m_ptCaretPos_PHY_Before);
					cSelectLogic.SetTo(pReplaceOpe->m_ptCaretPos_PHY_To);

					// �f�[�^�u�� �폜&�}���ɂ��g����
					bDrawAll |= m_view.ReplaceData_CEditView3(
						LayoutRange(ptCaretPos_Before, ptCaretPos_To),
						&pReplaceOpe->m_pMemDataDel,	// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
						&pReplaceOpe->m_pMemDataIns,	// �}������f�[�^
						bDraw,
						NULL,
						pReplaceOpe->m_nOrgDelSeq,
						&pReplaceOpe->m_nOrgInsSeq,
						bFastMode,
						&cSelectLogic
					);
					pReplaceOpe->m_pMemDataIns.clear();
				}
				break;
			case OpeCode::MoveCaret:
				break;
			}
			if (bFastMode) {
				if (i == nOpeBlkNum - 1) {
					layoutMgr._DoLayout();
					GetEditWindow()->ClearViewCaretPosInfo();
					if (GetDocument().m_nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
						layoutMgr.CalculateTextWidth();
					}
					layoutMgr.LogicToLayout(
						pOpe->m_ptCaretPos_PHY_After, &ptCaretPos_After);
					caret.MoveCursor(ptCaretPos_After, true);
					// �ʏ탂�[�h�ł�ReplaceData_CEditView�̒��Őݒ肳���
					caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX();
				}else {
					caret.MoveCursorFastMode(pOpe->m_ptCaretPos_PHY_After);
				}
			}else {
				layoutMgr.LogicToLayout(
					pOpe->m_ptCaretPos_PHY_After, &ptCaretPos_After);
				caret.MoveCursor(ptCaretPos_After, (i == nOpeBlkNum - 1));
			}
			if (hwndProgress && (i % 100) == 0) {
				int newPos = ::MulDiv(i + 1, 100, nOpeBlkNum);
				if (newPos != nProgressPos) {
					nProgressPos = newPos;
					Progress_SetPos(hwndProgress, newPos + 1);
					Progress_SetPos(hwndProgress, newPos);
				}
			}
		}
		m_view.SetDrawSwitch(bDrawSwitchOld); // 2007.07.22 ryoji
		m_view.AdjustScrollBars(); // 2007.07.22 ryoji

		// Redo��̕ύX�t���O
		docEditor.SetModified(bIsModified, true);	// Jan. 22, 2002 genta

		m_view.m_bDoing_UndoRedo = false;	// Undo, Redo�̎��s����

		m_view.SetBracketPairPos(true);	// 03/03/07 ai

		// �ĕ`��
		// ���[���[�ĕ`��̕K�v������Ƃ��� DispRuler() �ł͂Ȃ����̕����Ɠ����� Call_OnPaint() �ŕ`�悷��	// 2010.08.20 ryoji
		// �EDispRuler() �̓��[���[�ƃe�L�X�g�̌��ԁi�����͍s�ԍ��̕��ɍ��킹���сj��`�悵�Ă���Ȃ�
		// �E�s�ԍ��\���ɕK�v�ȕ��� OPE_INSERT/OPE_DELETE �������ōX�V����Ă���ύX������΃��[���[�ĕ`��t���O�ɔ��f����Ă���
		// �E����Scroll�����[���[�ĕ`��t���O�ɔ��f����Ă���
		const bool bRedrawRuler = m_view.GetRuler().GetRedrawFlag();
		m_view.Call_OnPaint(
			(int)PaintAreaType::LineNumber | (int)PaintAreaType::Body | (bRedrawRuler? (int)PaintAreaType::Ruler: 0),
			false
		);
		if (!bRedrawRuler) {
			// ���[���[�̃L�����b�g�݂̂��ĕ`��
			HDC hdc = m_view.GetDC();
			m_view.GetRuler().DispRuler(hdc);
			m_view.ReleaseDC(hdc);
		}

		caret.ShowCaretPosInfo();	// �L�����b�g�̍s���ʒu��\������	// 2007.10.19 ryoji

		if (!GetEditWindow()->UpdateTextWrap())	// �܂�Ԃ����@�֘A�̍X�V	// 2008.06.10 ryoji
			GetEditWindow()->RedrawAllViews(&m_view);	// ���̃y�C���̕\�����X�V

		if (hwndProgress) {
			::ShowWindow(hwndProgress, SW_HIDE);
		}
	}

	caret.m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;	// 2007.10.11 ryoji �ǉ�
	m_view.m_bDoing_UndoRedo = false;	// Undo, Redo�̎��s����

	return;
}


// �J�[�\���ʒu�܂��͑I���G���A���폜
void ViewCommander::Command_DELETE(void)
{
	auto& selInfo = m_view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {		// �}�E�X�ɂ��͈͑I��
		ErrorBeep();
		return;
	}

	if (!selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		auto& layoutMgr = GetDocument().m_layoutMgr;
		// 2008.08.03 nasukoji	�I��͈͂Ȃ���DELETE�����s�����ꍇ�A�J�[�\���ʒu�܂Ŕ��p�X�y�[�X��}����������s���폜���Ď��s�ƘA������
		auto& caret = GetCaret();
		if (layoutMgr.GetLineCount() > caret.GetCaretLayoutPos().GetY2()) {
			const Layout* pLayout = layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().GetY2());
			if (pLayout) {
				LayoutInt nLineLen;
				LogicInt nIndex;
				nIndex = m_view.LineColumnToIndex2(pLayout, caret.GetCaretLayoutPos().GetX2(), &nLineLen);
				if (nLineLen != 0) {	// �܂�Ԃ�����s�R�[�h���E�̏ꍇ�ɂ� nLineLen �ɍs�S�̂̕\������������
					if (pLayout->GetLayoutEol().GetType() != EolType::None) {	// �s�I�[�͉��s�R�[�h��?
						Command_INSTEXT(true, L"", LogicInt(0), FALSE);	// �J�[�\���ʒu�܂Ŕ��p�X�y�[�X�}��
					}else {	// �s�I�[���܂�Ԃ�
						// �܂�Ԃ��s���ł̓X�y�[�X�}����A���̕������폜����	// 2009.02.19 ryoji

						// �t���[�J�[�\�����̐܂�Ԃ��z���ʒu�ł̍폜�͂ǂ�����̂��Ó����悭�킩��Ȃ���
						// ��t���[�J�[�\�����i���傤�ǃJ�[�\�����܂�Ԃ��ʒu�ɂ���j�ɂ͎��̍s�̐擪�������폜������

						if (nLineLen < caret.GetCaretLayoutPos().GetX2()) {	// �܂�Ԃ��s���ƃJ�[�\���̊ԂɌ��Ԃ�����
							Command_INSTEXT(true, L"", LogicInt(0), FALSE);	// �J�[�\���ʒu�܂Ŕ��p�X�y�[�X�}��
							pLayout = layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().GetY2());
							nIndex = m_view.LineColumnToIndex2(pLayout, caret.GetCaretLayoutPos().GetX2(), &nLineLen);
						}
						if (nLineLen != 0) {	// �i�X�y�[�X�}������j�܂�Ԃ��s���Ȃ玟�������폜���邽�߂Ɏ��s�̐擪�Ɉړ�����K�v������
							if (pLayout->GetNextLayout()) {	// �ŏI�s���ł͂Ȃ�
								LayoutPoint ptLay;
								LogicPoint ptLog(pLayout->GetLogicOffset() + nIndex, pLayout->GetLogicLineNo());
								layoutMgr.LogicToLayout(ptLog, &ptLay);
								caret.MoveCursor(ptLay, true);
								caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
							}
						}
					}
				}
			}
		}
	}
	m_view.DeleteData(true);
	return;
}


// �J�[�\���O���폜
void ViewCommander::Command_DELETE_BACK(void)
{
	auto& selInfo = m_view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// �}�E�X�ɂ��͈͑I��
		ErrorBeep();
		return;
	}

	// May 29, 2004 genta ���ۂɍ폜���ꂽ�������Ȃ��Ƃ��̓t���O�����ĂȂ��悤��
	//GetDocument().m_docEditor.SetModified(true, true);	// Jan. 22, 2002 genta
	if (selInfo.IsTextSelected()) {				// �e�L�X�g���I������Ă��邩
		m_view.DeleteData(true);
	}else {
		auto& caret = GetCaret();
		LayoutPoint	ptLayoutPos_Old = caret.GetCaretLayoutPos();
		LogicPoint		ptLogicPos_Old = caret.GetCaretLogicPos();
		BOOL bBool = Command_LEFT(false, false);
		if (bBool) {
			const Layout* pLayout = GetDocument().m_layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().GetY2());
			if (pLayout) {
				LayoutInt nLineLen;
				LogicInt nIdx = m_view.LineColumnToIndex2(pLayout, caret.GetCaretLayoutPos().GetX2(), &nLineLen);
				if (nLineLen == 0) {	// �܂�Ԃ�����s�R�[�h���E�̏ꍇ�ɂ� nLineLen �ɍs�S�̂̕\������������
					// �E����̈ړ��ł͐܂�Ԃ����������͍폜���邪���s�͍폜���Ȃ�
					// ������i���̍s�̍s������j�̈ړ��ł͉��s���폜����
					if (nIdx < pLayout->GetLengthWithoutEOL() || caret.GetCaretLayoutPos().GetY2() < ptLayoutPos_Old.GetY2()) {
						if (!m_view.m_bDoing_UndoRedo) {	// Undo, Redo�̎��s����
							// ����̒ǉ�
							GetOpeBlk()->AppendOpe(
								new MoveCaretOpe(
									ptLogicPos_Old,
									caret.GetCaretLogicPos()
								)
							);
						}
						m_view.DeleteData(true);
					}
				}
			}
		}
	}
	m_view.PostprocessCommand_hokan();	// Jan. 10, 2005 genta �֐���
}


// 	�㏑���p�̈ꕶ���폜	2009.04.11 ryoji
void ViewCommander::DelCharForOverwrite(
	const wchar_t* pszInput,
	int nLen
	)
{
	bool bEol = false;
	bool bDelete = true;
	const Layout* pLayout = GetDocument().m_layoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().GetY2());
	int nDelLen = LogicInt(0);
	LayoutInt nKetaDiff = LayoutInt(0);
	LayoutInt nKetaAfterIns = LayoutInt(0);
	if (pLayout) {
		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
		LogicInt nIdxTo = m_view.LineColumnToIndex(pLayout, GetCaret().GetCaretLayoutPos().GetX2());
		if (nIdxTo >= pLayout->GetLengthWithoutEOL()) {
			bEol = true;	// ���݈ʒu�͉��s�܂��͐܂�Ԃ��Ȍ�
			if (pLayout->GetLayoutEol() != EolType::None) {
				if (GetDllShareData().common.edit.bNotOverWriteCRLF) {	// ���s�͏㏑�����Ȃ�
					// ���݈ʒu�����s�Ȃ�΍폜���Ȃ�
					bDelete = false;
				}
			}
		}else {
			// �������ɍ��킹�ăX�y�[�X���l�߂�
			if (GetDllShareData().common.edit.bOverWriteFixMode) {
				const StringRef line = pLayout->GetDocLineRef()->GetStringRefWithEOL();
				LogicInt nPos = GetCaret().GetCaretLogicPos().GetX();
				if (line.At(nPos) != WCODE::TAB) {
					LayoutInt nKetaBefore = NativeW::GetKetaOfChar(line, nPos);
					LayoutInt nKetaAfter = NativeW::GetKetaOfChar(pszInput, nLen, 0);
					nKetaDiff = nKetaBefore - nKetaAfter;
					nPos += NativeW::GetSizeOfChar(line.GetPtr(), line.GetLength(), nPos);
					nDelLen = 1;
					if (nKetaDiff < 0 && nPos < line.GetLength()) {
						wchar_t c = line.At(nPos);
						if (c != WCODE::TAB
							&& !WCODE::IsLineDelimiter(
								c,
								GetDllShareData().common.edit.bEnableExtEol
							)
						) {
							nDelLen = 2;
							LayoutInt nKetaBefore2 = NativeW::GetKetaOfChar(line, nPos);
							nKetaAfterIns = nKetaBefore + nKetaBefore2 - nKetaAfter;
						}
					}
				}
			}
		}
	}
	if (bDelete) {
		// �㏑�����[�h�Ȃ̂ŁA���݈ʒu�̕������P��������
		LayoutPoint posBefore;
		if (bEol) {
			Command_DELETE();	// �s�����ł͍ĕ`�悪�K�v���s���Ȍ�̍폜����������
			posBefore = GetCaret().GetCaretLayoutPos();
		}else {
			// 1�����폜
			m_view.DeleteData(false);
			posBefore = GetCaret().GetCaretLayoutPos();
			for (int i=1; i<nDelLen; ++i) {
				m_view.DeleteData(false);
			}
		}
		NativeW tmp;
		for (LayoutInt i=LayoutInt(0); i<nKetaDiff; ++i) {
			tmp.AppendString(L" ");
		}
		for (LayoutInt i=LayoutInt(0); i<nKetaAfterIns; ++i) {
			tmp.AppendString(L" ");
		}
		if (0 < tmp.GetStringLength()) {
			Command_INSTEXT(false, tmp.GetStringPtr(), tmp.GetStringLength(), false, false);
			GetCaret().MoveCursor(posBefore, false);
		}
	}
}

