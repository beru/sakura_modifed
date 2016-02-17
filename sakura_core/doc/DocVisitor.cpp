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
	EditView* pView = &EditWnd::getInstance()->GetActiveView();

	// アンドゥ記録開始
	if (!pView->m_bDoing_UndoRedo) {
		if (!pView->m_commander.GetOpeBlk()) {
			pView->m_commander.SetOpeBlk(new OpeBlk());
		}
		pView->m_commander.GetOpeBlk()->AddRef();
	}

	// カーソル位置記憶
	LayoutInt		nViewTopLine = pView->GetTextArea().GetViewTopLine();
	LayoutInt		nViewLeftCol = pView->GetTextArea().GetViewLeftCol();
	LayoutPoint		ptCaretPosXY = pView->GetCaret().GetCaretLayoutPos();
	LayoutInt		nCaretPosX_Prev = pView->GetCaret().m_nCaretPosX_Prev;

	bool bReplace = false;
	// 改行コードを統一する
	if (eol.IsValid()) {
		LogicInt nLine = LogicInt(0);
		OpeBlk* pOpeBlk = pView->m_bDoing_UndoRedo ? NULL : pView->m_commander.GetOpeBlk();
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
	// アンドゥ記録
	if (pView->m_commander.GetOpeBlk()) {
		if (pView->m_commander.GetOpeBlk()->GetNum()>0) {
			// カーソル位置復元
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

