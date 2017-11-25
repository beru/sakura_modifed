#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "view/Ruler.h"
#include "uiparts/WaitCursor.h"
#include "plugin/JackManager.h"
#include "plugin/SmartIndentIfObj.h"
#include "debug/RunningTimer.h"

// ViewCommander�N���X�̃R�}���h(�ҏW�n ��{�`)�֐��Q

// wchar_t1���̕��������
void ViewCommander::Command_WCHAR(
	wchar_t wcChar,
	bool bConvertEOL
	)
{
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// �}�E�X�ɂ��͈͑I��
		ErrorBeep();
		return;
	}

	size_t nPos;
	auto& doc = GetDocument();
	doc.docEditor.SetModified(true, true);

	if (view.bHideMouse && 0 <= view.nMousePouse) {
		view.nMousePouse = -1;
		::SetCursor(NULL);
	}

	auto& caret = GetCaret();
	auto& typeData = view.pTypeData;

	// ���݈ʒu�Ƀf�[�^��}��
	NativeW memDataW2;
	memDataW2 = wcChar;
	if (WCODE::IsLineDelimiter(wcChar, GetDllShareData().common.edit.bEnableExtEol)) { 
		// ���݁AEnter�Ȃǂő}��������s�R�[�h�̎�ނ��擾
		if (bConvertEOL) {
			Eol cWork = doc.docEditor.GetNewLineCode();
			memDataW2.SetString(cWork.GetValue2(), cWork.GetLen());
		}

		// �e�L�X�g���I������Ă��邩
		if (selInfo.IsTextSelected()) {
			view.DeleteData(true);
		}
		if (typeData->bAutoIndent) {	// �I�[�g�C���f���g
			const Layout* pLayout;
			size_t nLineLen;
			auto& layoutMgr = doc.layoutMgr;
			const wchar_t* pLine = layoutMgr.GetLineStr(caret.GetCaretLayoutPos().y, &nLineLen, &pLayout);
			if (pLayout) {
				const DocLine* pDocLine = doc.docLineMgr.GetLine(pLayout->GetLogicLineNo());
				pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
				if (pLine) {
					/*
					  �J�[�\���ʒu�ϊ�
					  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
					  ��
					  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
					*/
					Point ptXY = layoutMgr.LayoutToLogic(caret.GetCaretLayoutPos());

					// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
					ASSERT_GE(ptXY.x, 0);
					for (nPos=0; nPos<nLineLen && nPos<(size_t)ptXY.x;) {
						size_t nCharChars = NativeW::GetSizeOfChar(pLine, nLineLen, nPos);

						// ���̑��̃C���f���g����
						if (0 < nCharChars
						 && pLine[nPos] != L'\0'	// ���̑��̃C���f���g������ L'\0' �͊܂܂�Ȃ�
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
				Command_Indent(wcChar);
				return;
			}else {
				view.DeleteData(true);
			}
		}else {
			if (!view.IsInsMode()) {
				DelCharForOverwrite(&wcChar, 1);	// �㏑���p�̈ꕶ���폜
			}
		}
	}

	// �{���ɑ}������
	Point ptLayoutNew;
	view.InsertData_CEditView(
		caret.GetCaretLayoutPos(),
		memDataW2.GetStringPtr(),
		memDataW2.GetStringLength(),
		&ptLayoutNew,
		true
	);

	// �}���f�[�^�̍Ō�փJ�[�\�����ړ�
	caret.MoveCursor(ptLayoutNew, true);
	caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

	// �X�}�[�g�C���f���g
	SmartIndentType nSIndentType = typeData->eSmartIndent;
	switch (nSIndentType) {	// �X�}�[�g�C���f���g���
	case SmartIndentType::None:
		break;
	case SmartIndentType::Cpp:
		// C/C++�X�}�[�g�C���f���g����
		view.SmartIndent_CPP(wcChar);
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
				view.SetUndoBuffer();

				// �L�[���͂Ƃ͕ʂ̑���u���b�N�ɂ���i�������v���O�C�����̑���͂܂Ƃ߂�j
				if (!GetOpeBlk()) {
					SetOpeBlk(new OpeBlk);
				}
				GetOpeBlk()->AddRef();	// ��Release��HandleCommand�̍Ō�ōs��

				// �v���O�C���Ăяo��
				(*plugs.begin())->Invoke(view, params);
				objIndent->Release();
			}
		}
		break;
	}

	// ���s���ɖ����̋󔒂��폜
	if (WCODE::IsLineDelimiter(
			wcChar,
			GetDllShareData().common.edit.bEnableExtEol
		)
		&& typeData->bRTrimPrevLine
	) {	// ���s���ɖ����̋󔒂��폜
		// �O�̍s�ɂ��閖���̋󔒂��폜����
		view.RTrimPrevLine();
	}

	view.PostprocessCommand_hokan();
}


/*!
	@brief 2�o�C�g��������
	
	WM_IME_CHAR�ő����Ă�����������������D
	�������C�}�����[�h�ł�WM_IME_CHAR�ł͂Ȃ�WM_IME_COMPOSITION�ŕ������
	�擾����̂ł����ɂ͗��Ȃ��D

	@param wChar [in] SJIS�����R�[�h�D��ʂ�1�o�C�g�ځC���ʂ�2�o�C�g�ځD
*/
void ViewCommander::Command_IME_CHAR(WORD wChar)
{
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// �}�E�X�ɂ��͈͑I��
		ErrorBeep();
		return;
	}

	// �㉺�t�]
	if ((wChar & 0xff00) == 0) {
		Command_WCHAR(wChar & 0xff);
		return;
	}
	GetDocument().docEditor.SetModified(true, true);

 	if (view.bHideMouse && 0 <= view.nMousePouse) {
		view.nMousePouse = -1;
		::SetCursor(NULL);
	}

	wchar_t szWord[2] = {wChar, 0};
	size_t nWord = 1;
	// �e�L�X�g���I������Ă��邩
	if (selInfo.IsTextSelected()) {
		// ��`�͈͑I�𒆂�
		if (selInfo.IsBoxSelecting()) {
			Command_Indent(szWord, nWord);
			return;
		}else {
			view.DeleteData(true);
		}
	}else {
		if (!view.IsInsMode()) {
			DelCharForOverwrite(szWord, nWord);	// �㏑���p�̈ꕶ���폜
		}
	}

	Point ptLayoutNew;
	auto& caret = GetCaret();
	view.InsertData_CEditView(caret.GetCaretLayoutPos(), szWord, nWord, &ptLayoutNew, true);

	// �}���f�[�^�̍Ō�փJ�[�\�����ړ�
	caret.MoveCursor(ptLayoutNew, true);
	caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

	view.PostprocessCommand_hokan();
}

// from ViewCommander_New.cpp
// Undo ���ɖ߂�
void ViewCommander::Command_Undo(void)
{
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// �}�E�X�ɂ��͈͑I��
		ErrorBeep();
		return;
	}

	{
		OpeBlk* opeBlk = view.commander.GetOpeBlk();
		if (opeBlk) {
			int nCount = opeBlk->GetRefCount();
			opeBlk->SetRefCount(1); // �����I�Ƀ��Z�b�g���邽��1���w��
			view.SetUndoBuffer();
			if (!view.commander.GetOpeBlk() && 0 < nCount) {
				view.commander.SetOpeBlk(new OpeBlk());
				view.commander.GetOpeBlk()->SetRefCount(nCount);
			}
		}
	}

	if (!GetDocument().docEditor.IsEnableUndo()) {	// Undo(���ɖ߂�)�\�ȏ�Ԃ��H
		return;
	}

	MY_RUNNINGTIMER(runningTimer, "ViewCommander::Command_Undo()");

	Ope* pOpe = nullptr;

	OpeBlk*	pOpeBlk;
	size_t nOpeBlkNum;
	bool bIsModified;
//	int nNewLine;	// �}�����ꂽ�����̎��̈ʒu�̍s
//	int nNewPos;	// �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu

	Point ptCaretPos_Before;
	Point ptCaretPos_After;

	// �e�탂�[�h�̎�����
	Command_Cancel_Mode();

	view.bDoing_UndoRedo = true;	// Undo, Redo�̎��s����

	// ���݂�Undo�Ώۂ̑���u���b�N��Ԃ�
	auto& caret = GetCaret();
	auto& docEditor = GetDocument().docEditor;
	if ((pOpeBlk = docEditor.opeBuf.DoUndo(&bIsModified))) {
		nOpeBlkNum = pOpeBlk->GetNum();
		bool bDraw = (nOpeBlkNum < 5) && view.GetDrawSwitch();
		bool bDrawAll = false;
		const bool bDrawSwitchOld = view.SetDrawSwitch(bDraw);	// hor


		WaitCursor waitCursor(view.GetHwnd(), 1000 < nOpeBlkNum);
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if (waitCursor.IsEnable()) {
			hwndProgress = view.StartProgress();
		}

		const bool bFastMode = (100 < nOpeBlkNum);
		auto& layoutMgr = GetDocument().layoutMgr;
		for (int i=(int)nOpeBlkNum-1; i>=0; --i) {
			pOpe = pOpeBlk->GetOpe(i);
			if (bFastMode) {
				caret.MoveCursorFastMode(pOpe->ptCaretPos_PHY_After);
			}else {
				ptCaretPos_After = layoutMgr.LogicToLayout(pOpe->ptCaretPos_PHY_After);
				ptCaretPos_Before = layoutMgr.LogicToLayout(pOpe->ptCaretPos_PHY_Before);
				// �J�[�\�����ړ�
				caret.MoveCursor(ptCaretPos_After, false);
			}

			switch (pOpe->GetCode()) {
			case OpeCode::Insert:
				{
					InsertOpe* pInsertOpe = static_cast<InsertOpe*>(pOpe);

					// �I��͈͂̕ύX
					Range selectLogic;
					selectLogic.SetFrom(pOpe->ptCaretPos_PHY_Before);
					selectLogic.SetTo(pOpe->ptCaretPos_PHY_After);
					if (bFastMode) {
					}else {
						selInfo.selectBgn.SetFrom(ptCaretPos_Before);
						selInfo.selectBgn.SetTo(selInfo.selectBgn.GetFrom());
						selInfo.select.SetFrom(ptCaretPos_Before);
						selInfo.select.SetTo(ptCaretPos_After);
					}

					// �f�[�^�u�� �폜&�}���ɂ��g����
					bDrawAll |= view.ReplaceData_CEditView3(
						selInfo.select,				// �폜�͈�
						&pInsertOpe->opeLineData,	// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
						nullptr,
						bDraw,						// �ĕ`�悷�邩�ۂ�
						nullptr,
						pInsertOpe->nOrgSeq,
						nullptr,
						bFastMode,
						&selectLogic
					);

					// �I��͈͂̕ύX
					selInfo.selectBgn.Clear(-1); // �͈͑I��(���_)
					selInfo.select.Clear(-1);
				}
				break;
			case OpeCode::Delete:
				{
					DeleteOpe* pDeleteOpe = static_cast<DeleteOpe*>(pOpe);

					if (0 < pDeleteOpe->opeLineData.size()) {
						// �f�[�^�u�� �폜&�}���ɂ��g����
						Range range;
						range.Set(ptCaretPos_Before);
						Range cSelectLogic;
						cSelectLogic.Set(pOpe->ptCaretPos_PHY_Before);
						bDrawAll |= view.ReplaceData_CEditView3(
							range,
							nullptr,									// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
							&pDeleteOpe->opeLineData,
							bDraw,										// �ĕ`�悷�邩�ۂ�
							nullptr,
							0,
							&pDeleteOpe->nOrgSeq,
							bFastMode,
							&cSelectLogic
						);
					}
					pDeleteOpe->opeLineData.clear();
				}
				break;
			case OpeCode::Replace:
				{
					ReplaceOpe* pReplaceOpe = static_cast<ReplaceOpe*>(pOpe);

					Range range;
					range.SetFrom(ptCaretPos_Before);
					range.SetTo(ptCaretPos_After);
					Range cSelectLogic;
					cSelectLogic.SetFrom(pOpe->ptCaretPos_PHY_Before);
					cSelectLogic.SetTo(pOpe->ptCaretPos_PHY_After);

					// �f�[�^�u�� �폜&�}���ɂ��g����
					bDrawAll |= view.ReplaceData_CEditView3(
						range,				// �폜�͈�
						&pReplaceOpe->pMemDataIns,	// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
						&pReplaceOpe->pMemDataDel,	// �}������f�[�^
						bDraw,							// �ĕ`�悷�邩�ۂ�
						nullptr,
						pReplaceOpe->nOrgInsSeq,
						&pReplaceOpe->nOrgDelSeq,
						bFastMode,
						&cSelectLogic
					);
					pReplaceOpe->pMemDataDel.clear();
				}
				break;
			case OpeCode::MoveCaret:
				// �J�[�\�����ړ�
				if (bFastMode) {
					caret.MoveCursorFastMode(pOpe->ptCaretPos_PHY_After);
				}else {
					caret.MoveCursor(ptCaretPos_After, false);
				}
				break;
			}

			if (bFastMode) {
				if (i == 0) {
					layoutMgr._DoLayout();
					GetEditWindow().ClearViewCaretPosInfo();
					if (GetDocument().nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
						layoutMgr.CalculateTextWidth();
					}
					ptCaretPos_Before = layoutMgr.LogicToLayout(pOpe->ptCaretPos_PHY_Before);
					caret.MoveCursor(ptCaretPos_Before, true);
					// �ʏ탂�[�h�ł�ReplaceData_CEditView�̒��Őݒ肳���
					caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX();
				}else {
					caret.MoveCursorFastMode(pOpe->ptCaretPos_PHY_Before);
				}
			}else {
				ptCaretPos_Before = layoutMgr.LogicToLayout(pOpe->ptCaretPos_PHY_Before);
				// �J�[�\�����ړ�
				caret.MoveCursor(ptCaretPos_Before, (i == 0));
			}
			if (hwndProgress && (i % 100) == 0) {
				int newPos = ::MulDiv((int)nOpeBlkNum - i, 100, (int)nOpeBlkNum);
				if (newPos != nProgressPos) {
					nProgressPos = newPos;
					Progress_SetPos(hwndProgress, newPos + 1);
					Progress_SetPos(hwndProgress, newPos);
				}
			}
		}
		view.SetDrawSwitch(bDrawSwitchOld);
		view.AdjustScrollBars();

		// Undo��̕ύX�t���O
		docEditor.SetModified(bIsModified, true);

		view.bDoing_UndoRedo = false;	// Undo, Redo�̎��s����

		view.SetBracketPairPos(true);

		// �ĕ`��
		// ���[���[�ĕ`��̕K�v������Ƃ��� DispRuler() �ł͂Ȃ����̕����Ɠ����� Call_OnPaint() �ŕ`�悷��
		// �EDispRuler() �̓��[���[�ƃe�L�X�g�̌��ԁi�����͍s�ԍ��̕��ɍ��킹���сj��`�悵�Ă���Ȃ�
		// �E�s�ԍ��\���ɕK�v�ȕ��� OPE_INSERT/OPE_DELETE �������ōX�V����Ă���ύX������΃��[���[�ĕ`��t���O�ɔ��f����Ă���
		// �E����Scroll�����[���[�ĕ`��t���O�ɔ��f����Ă���
		const bool bRedrawRuler = view.GetRuler().GetRedrawFlag();
		view.Call_OnPaint((int)PaintAreaType::LineNumber | (int)PaintAreaType::Body | (bRedrawRuler? (int)PaintAreaType::Ruler: 0), false);
		if (!bRedrawRuler) {
			// ���[���[�̃L�����b�g�݂̂��ĕ`��
			HDC hdc = view.GetDC();
			view.GetRuler().DispRuler(hdc);
			view.ReleaseDC(hdc);
		}

		caret.ShowCaretPosInfo();	// �L�����b�g�̍s���ʒu��\������

		if (!GetEditWindow().UpdateTextWrap() && bDrawAll) {	// �܂�Ԃ����@�֘A�̍X�V
			GetEditWindow().RedrawAllViews(&view);	// ���̃y�C���̕\�����X�V
		}
		if (hwndProgress) {
			::ShowWindow(hwndProgress, SW_HIDE);
		}
	}

	caret.nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;
	view.bDoing_UndoRedo = false;	// Undo, Redo�̎��s����

	return;
}


// from ViewCommander_New.cpp
// Redo ��蒼��
void ViewCommander::Command_Redo(void)
{
	if (view.GetSelectionInfo().IsMouseSelecting()) {	// �}�E�X�ɂ��͈͑I��
		ErrorBeep();
		return;
	}

	{
		OpeBlk* opeBlk = view.commander.GetOpeBlk();
		if (opeBlk) {
			int nCount = opeBlk->GetRefCount();
			opeBlk->SetRefCount(1); // �����I�Ƀ��Z�b�g���邽��1���w��
			view.SetUndoBuffer();
			if (!view.commander.GetOpeBlk() && 0 < nCount) {
				view.commander.SetOpeBlk(new OpeBlk());
				view.commander.GetOpeBlk()->SetRefCount(nCount);
			}
		}
		// ���ӁFOpe��ǉ������Redo�͂ł��Ȃ��Ȃ�
	}

	auto& docEditor = GetDocument().docEditor;
	if (!docEditor.IsEnableRedo()) {	// Redo(��蒼��)�\�ȏ�Ԃ��H
		return;
	}
	MY_RUNNINGTIMER(runningTimer, "ViewCommander::Command_Redo()");

	Ope*		pOpe = nullptr;
	OpeBlk*	pOpeBlk;
	size_t		nOpeBlkNum;
//	int			nNewLine;	// �}�����ꂽ�����̎��̈ʒu�̍s
//	int			nNewPos;	// �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu
	bool		bIsModified;

	Point ptCaretPos_Before;
	Point ptCaretPos_To;
	Point ptCaretPos_After;

	// �e�탂�[�h�̎�����
	Command_Cancel_Mode();

	view.bDoing_UndoRedo = true;	// Undo, Redo�̎��s����

	// ���݂�Redo�Ώۂ̑���u���b�N��Ԃ�
	auto& caret = GetCaret();
	if ((pOpeBlk = docEditor.opeBuf.DoRedo(&bIsModified))) {
		nOpeBlkNum = pOpeBlk->GetNum();
		bool bDraw = (nOpeBlkNum < 5) && view.GetDrawSwitch();
		bool bDrawAll = false;
		const bool bDrawSwitchOld = view.SetDrawSwitch(bDraw);

		WaitCursor waitCursor(view.GetHwnd(), 1000 < nOpeBlkNum);
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if (waitCursor.IsEnable()) {
			hwndProgress = view.StartProgress();
		}

		const bool bFastMode = (100 < nOpeBlkNum);
		auto& layoutMgr = GetDocument().layoutMgr;
		for (size_t i=0; i<nOpeBlkNum; ++i) {
			pOpe = pOpeBlk->GetOpe(i);
			if (bFastMode) {
				if (i == 0) {
					ptCaretPos_Before = layoutMgr.LogicToLayout(pOpe->ptCaretPos_PHY_Before);
					caret.MoveCursor(ptCaretPos_Before, true);
				}else {
					caret.MoveCursorFastMode(pOpe->ptCaretPos_PHY_Before);
				}
			}else {
				ptCaretPos_Before = layoutMgr.LogicToLayout(pOpe->ptCaretPos_PHY_Before);
				caret.MoveCursor(ptCaretPos_Before, (i == 0));
			}
			switch (pOpe->GetCode()) {
			case OpeCode::Insert:
				{
					InsertOpe* pInsertOpe = static_cast<InsertOpe*>(pOpe);

					if (0 < pInsertOpe->opeLineData.size()) {
						// �f�[�^�u�� �폜&�}���ɂ��g����
						Range range;
						range.Set(ptCaretPos_Before);
						Range cSelectLogic;
						cSelectLogic.Set(pOpe->ptCaretPos_PHY_Before);
						bDrawAll |= view.ReplaceData_CEditView3(
							range,
							nullptr,								// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
							&pInsertOpe->opeLineData,				// �}������f�[�^
							bDraw,									// �ĕ`�悷�邩�ۂ�
							nullptr,
							0,
							&pInsertOpe->nOrgSeq,
							bFastMode,
							&cSelectLogic
						);

					}
					pInsertOpe->opeLineData.clear();
				}
				break;
			case OpeCode::Delete:
				{
					DeleteOpe* pDeleteOpe = static_cast<DeleteOpe*>(pOpe);

					if (bFastMode) {
					}else {
						ptCaretPos_To = layoutMgr.LogicToLayout(pDeleteOpe->ptCaretPos_PHY_To);
					}
					Range cSelectLogic;
					cSelectLogic.SetFrom(pOpe->ptCaretPos_PHY_Before);
					cSelectLogic.SetTo(pDeleteOpe->ptCaretPos_PHY_To);

					// �f�[�^�u�� �폜&�}���ɂ��g����
					bDrawAll |= view.ReplaceData_CEditView3(
						Range(ptCaretPos_Before, ptCaretPos_To),
						&pDeleteOpe->opeLineData,	// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
						nullptr,
						bDraw,
						nullptr,
						pDeleteOpe->nOrgSeq,
						nullptr,
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
						ptCaretPos_To = layoutMgr.LogicToLayout(pReplaceOpe->ptCaretPos_PHY_To);
					}
					Range cSelectLogic;
					cSelectLogic.SetFrom(pOpe->ptCaretPos_PHY_Before);
					cSelectLogic.SetTo(pReplaceOpe->ptCaretPos_PHY_To);

					// �f�[�^�u�� �폜&�}���ɂ��g����
					bDrawAll |= view.ReplaceData_CEditView3(
						Range(ptCaretPos_Before, ptCaretPos_To),
						&pReplaceOpe->pMemDataDel,	// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
						&pReplaceOpe->pMemDataIns,	// �}������f�[�^
						bDraw,
						nullptr,
						pReplaceOpe->nOrgDelSeq,
						&pReplaceOpe->nOrgInsSeq,
						bFastMode,
						&cSelectLogic
					);
					pReplaceOpe->pMemDataIns.clear();
				}
				break;
			case OpeCode::MoveCaret:
				break;
			}
			if (bFastMode) {
				if (i == nOpeBlkNum - 1) {
					layoutMgr._DoLayout();
					GetEditWindow().ClearViewCaretPosInfo();
					if (GetDocument().nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
						layoutMgr.CalculateTextWidth();
					}
					ptCaretPos_After = layoutMgr.LogicToLayout(pOpe->ptCaretPos_PHY_After);
					caret.MoveCursor(ptCaretPos_After, true);
					// �ʏ탂�[�h�ł�ReplaceData_CEditView�̒��Őݒ肳���
					caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX();
				}else {
					caret.MoveCursorFastMode(pOpe->ptCaretPos_PHY_After);
				}
			}else {
				ptCaretPos_After = layoutMgr.LogicToLayout(pOpe->ptCaretPos_PHY_After);
				caret.MoveCursor(ptCaretPos_After, (i == nOpeBlkNum - 1));
			}
			if (hwndProgress && (i % 100) == 0) {
				int newPos = ::MulDiv((int)i + 1, 100, (int)nOpeBlkNum);
				if (newPos != nProgressPos) {
					nProgressPos = newPos;
					Progress_SetPos(hwndProgress, newPos + 1);
					Progress_SetPos(hwndProgress, newPos);
				}
			}
		}
		view.SetDrawSwitch(bDrawSwitchOld);
		view.AdjustScrollBars();

		// Redo��̕ύX�t���O
		docEditor.SetModified(bIsModified, true);

		view.bDoing_UndoRedo = false;	// Undo, Redo�̎��s����

		view.SetBracketPairPos(true);

		// �ĕ`��
		// ���[���[�ĕ`��̕K�v������Ƃ��� DispRuler() �ł͂Ȃ����̕����Ɠ����� Call_OnPaint() �ŕ`�悷��
		// �EDispRuler() �̓��[���[�ƃe�L�X�g�̌��ԁi�����͍s�ԍ��̕��ɍ��킹���сj��`�悵�Ă���Ȃ�
		// �E�s�ԍ��\���ɕK�v�ȕ��� OPE_INSERT/OPE_DELETE �������ōX�V����Ă���ύX������΃��[���[�ĕ`��t���O�ɔ��f����Ă���
		// �E����Scroll�����[���[�ĕ`��t���O�ɔ��f����Ă���
		const bool bRedrawRuler = view.GetRuler().GetRedrawFlag();
		view.Call_OnPaint(
			(int)PaintAreaType::LineNumber | (int)PaintAreaType::Body | (bRedrawRuler? (int)PaintAreaType::Ruler: 0),
			false
		);
		if (!bRedrawRuler) {
			// ���[���[�̃L�����b�g�݂̂��ĕ`��
			HDC hdc = view.GetDC();
			view.GetRuler().DispRuler(hdc);
			view.ReleaseDC(hdc);
		}

		caret.ShowCaretPosInfo();	// �L�����b�g�̍s���ʒu��\������

		if (!GetEditWindow().UpdateTextWrap())	// �܂�Ԃ����@�֘A�̍X�V
			GetEditWindow().RedrawAllViews(&view);	// ���̃y�C���̕\�����X�V

		if (hwndProgress) {
			::ShowWindow(hwndProgress, SW_HIDE);
		}
	}

	caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
	view.bDoing_UndoRedo = false;	// Undo, Redo�̎��s����

	return;
}


// �J�[�\���ʒu�܂��͑I���G���A���폜
void ViewCommander::Command_Delete(void)
{
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {		// �}�E�X�ɂ��͈͑I��
		ErrorBeep();
		return;
	}

	if (!selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		auto& layoutMgr = GetDocument().layoutMgr;
		// �I��͈͂Ȃ���DELETE�����s�����ꍇ�A�J�[�\���ʒu�܂Ŕ��p�X�y�[�X��}����������s���폜���Ď��s�ƘA������
		auto& caret = GetCaret();
		if ((int)layoutMgr.GetLineCount() > caret.GetCaretLayoutPos().y) {
			const Layout* pLayout = layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().y);
			if (pLayout) {
				size_t nLineLen;
				size_t nIndex = view.LineColumnToIndex2(pLayout, caret.GetCaretLayoutPos().x, &nLineLen);
				if (nLineLen != 0) {	// �܂�Ԃ�����s�R�[�h���E�̏ꍇ�ɂ� nLineLen �ɍs�S�̂̕\������������
					if (pLayout->GetLayoutEol().GetType() != EolType::None) {	// �s�I�[�͉��s�R�[�h��?
						Command_InsText(true, L"", 0, false);	// �J�[�\���ʒu�܂Ŕ��p�X�y�[�X�}��
					}else {	// �s�I�[���܂�Ԃ�
						// �܂�Ԃ��s���ł̓X�y�[�X�}����A���̕������폜����

						// �t���[�J�[�\�����̐܂�Ԃ��z���ʒu�ł̍폜�͂ǂ�����̂��Ó����悭�킩��Ȃ���
						// ��t���[�J�[�\�����i���傤�ǃJ�[�\�����܂�Ԃ��ʒu�ɂ���j�ɂ͎��̍s�̐擪�������폜������

						if ((int)nLineLen < caret.GetCaretLayoutPos().x) {	// �܂�Ԃ��s���ƃJ�[�\���̊ԂɌ��Ԃ�����
							Command_InsText(true, L"", 0, false);	// �J�[�\���ʒu�܂Ŕ��p�X�y�[�X�}��
							pLayout = layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().y);
							nIndex = view.LineColumnToIndex2(pLayout, caret.GetCaretLayoutPos().x, &nLineLen);
						}
						if (nLineLen != 0) {	// �i�X�y�[�X�}������j�܂�Ԃ��s���Ȃ玟�������폜���邽�߂Ɏ��s�̐擪�Ɉړ�����K�v������
							if (pLayout->GetNextLayout()) {	// �ŏI�s���ł͂Ȃ�
								Point ptLog(pLayout->GetLogicOffset() + (int)nIndex, pLayout->GetLogicLineNo());
								Point ptLay = layoutMgr.LogicToLayout(ptLog);
								caret.MoveCursor(ptLay, true);
								caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
							}
						}
					}
				}
			}
		}
	}
	view.DeleteData(true);
	return;
}


// �J�[�\���O���폜
void ViewCommander::Command_Delete_Back(void)
{
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// �}�E�X�ɂ��͈͑I��
		ErrorBeep();
		return;
	}

	if (selInfo.IsTextSelected()) {				// �e�L�X�g���I������Ă��邩
		view.DeleteData(true);
	}else {
		auto& caret = GetCaret();
		Point	ptLayoutPos_Old = caret.GetCaretLayoutPos();
		Point ptLogicPos_Old = caret.GetCaretLogicPos();
		BOOL bBool = Command_Left(false, false);
		if (bBool) {
			const Layout* pLayout = GetDocument().layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().y);
			if (pLayout) {
				size_t nLineLen;
				size_t nIdx = view.LineColumnToIndex2(pLayout, caret.GetCaretLayoutPos().x, &nLineLen);
				if (nLineLen == 0) {	// �܂�Ԃ�����s�R�[�h���E�̏ꍇ�ɂ� nLineLen �ɍs�S�̂̕\������������
					// �E����̈ړ��ł͐܂�Ԃ����������͍폜���邪���s�͍폜���Ȃ�
					// ������i���̍s�̍s������j�̈ړ��ł͉��s���폜����
					if (nIdx < pLayout->GetLengthWithoutEOL() || caret.GetCaretLayoutPos().y < ptLayoutPos_Old.y) {
						if (!view.bDoing_UndoRedo) {	// Undo, Redo�̎��s����
							// ����̒ǉ�
							GetOpeBlk()->AppendOpe(
								new MoveCaretOpe(
									ptLogicPos_Old,
									caret.GetCaretLogicPos()
								)
							);
						}
						view.DeleteData(true);
					}
				}
			}
		}
	}
	view.PostprocessCommand_hokan();
}

// 	�㏑���p�̈ꕶ���폜
void ViewCommander::DelCharForOverwrite(
	const wchar_t* pszInput,
	size_t nLen
	)
{
	bool bEol = false;
	bool bDelete = true;
	auto& caret = GetCaret();
	const Layout* pLayout = GetDocument().layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().y);
	int nDelLen = 0;
	int nKetaDiff = 0;
	int nKetaAfterIns = 0;
	if (pLayout) {
		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
		size_t nIdxTo = view.LineColumnToIndex(pLayout, caret.GetCaretLayoutPos().x);
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
				size_t nPos = caret.GetCaretLogicPos().GetX();
				if (line.At(nPos) != WCODE::TAB) {
					size_t nKetaBefore = NativeW::GetKetaOfChar(line, nPos);
					size_t nKetaAfter = NativeW::GetKetaOfChar(pszInput, nLen, 0);
					nKetaDiff = (int)nKetaBefore - (int)nKetaAfter;
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
							size_t nKetaBefore2 = NativeW::GetKetaOfChar(line, nPos);
							nKetaAfterIns = (int)nKetaBefore + (int)nKetaBefore2 - (int)nKetaAfter;
						}
					}
				}
			}
		}
	}
	if (bDelete) {
		// �㏑�����[�h�Ȃ̂ŁA���݈ʒu�̕������P��������
		Point posBefore;
		if (bEol) {
			Command_Delete();	// �s�����ł͍ĕ`�悪�K�v���s���Ȍ�̍폜����������
			posBefore = caret.GetCaretLayoutPos();
		}else {
			// 1�����폜
			view.DeleteData(false);
			posBefore = caret.GetCaretLayoutPos();
			for (int i=1; i<nDelLen; ++i) {
				view.DeleteData(false);
			}
		}
		NativeW tmp;
		for (int i=0; i<nKetaDiff; ++i) {
			tmp.AppendStringLiteral(L" ");
		}
		for (int i=0; i<nKetaAfterIns; ++i) {
			tmp.AppendStringLiteral(L" ");
		}
		if (0 < tmp.GetStringLength()) {
			Command_InsText(false, tmp.GetStringPtr(), tmp.GetStringLength(), false, false);
			caret.MoveCursor(posBefore, false);
		}
	}
}

