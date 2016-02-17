#include "StdAfx.h"
#include "DocVisitor.h"
#include "doc/EditDoc.h"
#include "cmd/ViewCommander_inline.h"
#include "view/EditView.h"
#include "window/EditWnd.h"
#include "OpeBlk.h"


// ���s�R�[�h�𓝈ꂷ��
void DocVisitor::SetAllEol(Eol eol)
{
	EditView* pView = &EditWnd::getInstance()->GetActiveView();

	// �A���h�D�L�^�J�n
	if (!pView->m_bDoing_UndoRedo) {
		if (!pView->m_commander.GetOpeBlk()) {
			pView->m_commander.SetOpeBlk(new OpeBlk());
		}
		pView->m_commander.GetOpeBlk()->AddRef();
	}

	// �J�[�\���ʒu�L��
	LayoutInt		nViewTopLine = pView->GetTextArea().GetViewTopLine();
	LayoutInt		nViewLeftCol = pView->GetTextArea().GetViewLeftCol();
	LayoutPoint		ptCaretPosXY = pView->GetCaret().GetCaretLayoutPos();
	LayoutInt		nCaretPosX_Prev = pView->GetCaret().m_nCaretPosX_Prev;

	bool bReplace = false;
	// ���s�R�[�h�𓝈ꂷ��
	if (eol.IsValid()) {
		LogicInt nLine = LogicInt(0);
		OpeBlk* pOpeBlk = pView->m_bDoing_UndoRedo ? NULL : pView->m_commander.GetOpeBlk();
		for (;;) {
			DocLine* pDocLine = m_pDocRef->m_docLineMgr.GetLine(nLine); //#######�����
			if (!pDocLine) {
				break;
			}
			// ���s��u��
			if (pDocLine->GetEol() != EolType::None && pDocLine->GetEol() != eol) {
				LogicRange range;
				range.SetFrom(LogicPoint(pDocLine->GetLengthWithoutEOL(), nLine));
				range.SetTo(LogicPoint(pDocLine->GetLengthWithEOL(), nLine));
				pView->ReplaceData_CEditView2(
					range,
					eol.GetValue2(),
					eol.GetLen(),
					false,
					pOpeBlk,
					true
				);
				bReplace = true;
			}
			++nLine;
		}
		// �ҏW�����͉��s�R�[�h
		EditDoc::GetInstance(0)->m_docEditor.SetNewLineCode(eol);
	}

	if (bReplace) {
		m_pDocRef->m_layoutMgr._DoLayout();
		m_pDocRef->m_pEditWnd->ClearViewCaretPosInfo();
		if (m_pDocRef->m_nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
			m_pDocRef->m_layoutMgr.CalculateTextWidth();
		}else {
			m_pDocRef->m_layoutMgr.ClearLayoutLineWidth();
		}
	}
	// �A���h�D�L�^
	if (pView->m_commander.GetOpeBlk()) {
		if (pView->m_commander.GetOpeBlk()->GetNum()>0) {
			// �J�[�\���ʒu����
			pView->GetTextArea().SetViewTopLine(nViewTopLine);
			pView->GetTextArea().SetViewLeftCol(nViewLeftCol);
			pView->GetCaret().MoveCursor(ptCaretPosXY, true);
			pView->GetCaret().m_nCaretPosX_Prev = nCaretPosX_Prev;
			pView->m_commander.GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					pView->GetCaret().GetCaretLogicPos()
				)
			);

		}
		pView->SetUndoBuffer();
	}
}

