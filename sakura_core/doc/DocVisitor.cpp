#include "StdAfx.h"
#include "DocVisitor.h"
#include "doc/EditDoc.h"
#include "cmd/ViewCommander_inline.h"
#include "view/EditView.h"
#include "window/EditWnd.h"
#include "OpeBlk.h"


// 改行コードを統一する
void DocVisitor::SetAllEol(Eol eol)
{
	auto& view = EditWnd::getInstance().GetActiveView();

	// Undo記録開始
	if (!view.m_bDoing_UndoRedo) {
		if (!view.m_commander.GetOpeBlk()) {
			view.m_commander.SetOpeBlk(new OpeBlk());
		}
		view.m_commander.GetOpeBlk()->AddRef();
	}

	// カーソル位置記憶
	LayoutInt		nViewTopLine = view.GetTextArea().GetViewTopLine();
	LayoutInt		nViewLeftCol = view.GetTextArea().GetViewLeftCol();
	LayoutPoint		ptCaretPosXY = view.GetCaret().GetCaretLayoutPos();
	LayoutInt		nCaretPosX_Prev = view.GetCaret().m_nCaretPosX_Prev;

	bool bReplace = false;
	// 改行コードを統一する
	if (eol.IsValid()) {
		LogicInt nLine = LogicInt(0);
		OpeBlk* pOpeBlk = view.m_bDoing_UndoRedo ? NULL : view.m_commander.GetOpeBlk();
		for (;;) {
			DocLine* pDocLine = m_pDocRef->m_docLineMgr.GetLine(nLine); //#######非効率
			if (!pDocLine) {
				break;
			}
			// 改行を置換
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
		// 編集時入力改行コード
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
	// Undo記録
	if (view.m_commander.GetOpeBlk()) {
		if (view.m_commander.GetOpeBlk()->GetNum()>0) {
			// カーソル位置復元
			view.GetTextArea().SetViewTopLine(nViewTopLine);
			view.GetTextArea().SetViewLeftCol(nViewLeftCol);
			view.GetCaret().MoveCursor(ptCaretPosXY, true);
			view.GetCaret().m_nCaretPosX_Prev = nCaretPosX_Prev;
			view.m_commander.GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					view.GetCaret().GetCaretLogicPos()
				)
			);

		}
		view.SetUndoBuffer();
	}
}

