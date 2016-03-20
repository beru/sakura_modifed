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
	auto& view = EditWnd::getInstance().GetActiveView();

	// Undo�L�^�J�n
	if (!view.m_bDoing_UndoRedo) {
		if (!view.m_commander.GetOpeBlk()) {
			view.m_commander.SetOpeBlk(new OpeBlk());
		}
		view.m_commander.GetOpeBlk()->AddRef();
	}

	// �J�[�\���ʒu�L��
	auto& caret = view.GetCaret();
	auto& textArea = view.GetTextArea();
	LayoutInt		nViewTopLine = textArea.GetViewTopLine();
	LayoutInt		nViewLeftCol = textArea.GetViewLeftCol();
	LayoutPoint		ptCaretPosXY = caret.GetCaretLayoutPos();
	LayoutInt		nCaretPosX_Prev = caret.m_nCaretPosX_Prev;

	bool bReplace = false;
	// ���s�R�[�h�𓝈ꂷ��
	if (eol.IsValid()) {
		LogicInt nLine = LogicInt(0);
		OpeBlk* pOpeBlk = view.m_bDoing_UndoRedo ? NULL : view.m_commander.GetOpeBlk();
		for (;;) {
			DocLine* pDocLine = m_doc.m_docLineMgr.GetLine(nLine); //#######�����
			if (!pDocLine) {
				break;
			}
			// ���s��u��
			if (pDocLine->GetEol() != EolType::None && pDocLine->GetEol() != eol) {
				LogicRange range;
				range.SetFrom(LogicPoint(pDocLine->GetLengthWithoutEOL(), nLine));
				range.SetTo(LogicPoint(pDocLine->GetLengthWithEOL(), nLine));
				view.ReplaceData_CEditView2(
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
		m_doc.m_layoutMgr._DoLayout();
		m_doc.m_pEditWnd->ClearViewCaretPosInfo();
		if (m_doc.m_nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
			m_doc.m_layoutMgr.CalculateTextWidth();
		}else {
			m_doc.m_layoutMgr.ClearLayoutLineWidth();
		}
	}
	// Undo�L�^
	if (view.m_commander.GetOpeBlk()) {
		if (view.m_commander.GetOpeBlk()->GetNum()>0) {
			// �J�[�\���ʒu����
			textArea.SetViewTopLine(nViewTopLine);
			textArea.SetViewLeftCol(nViewLeftCol);
			caret.MoveCursor(ptCaretPosXY, true);
			caret.m_nCaretPosX_Prev = nCaretPosX_Prev;
			view.m_commander.GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					caret.GetCaretLogicPos()
				)
			);

		}
		view.SetUndoBuffer();
	}
}

