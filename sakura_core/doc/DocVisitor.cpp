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
	if (!view.bDoing_UndoRedo) {
		if (!view.commander.GetOpeBlk()) {
			view.commander.SetOpeBlk(new OpeBlk());
		}
		view.commander.GetOpeBlk()->AddRef();
	}

	// カーソル位置記憶
	auto& caret = view.GetCaret();
	auto& textArea = view.GetTextArea();
	LayoutInt		nViewTopLine = textArea.GetViewTopLine();
	LayoutInt		nViewLeftCol = textArea.GetViewLeftCol();
	LayoutPoint		ptCaretPosXY = caret.GetCaretLayoutPos();
	LayoutInt		nCaretPosX_Prev = caret.nCaretPosX_Prev;

	bool bReplace = false;
	// 改行コードを統一する
	if (eol.IsValid()) {
		LogicInt nLine = LogicInt(0);
		OpeBlk* pOpeBlk = view.bDoing_UndoRedo ? NULL : view.commander.GetOpeBlk();
		for (;;) {
			DocLine* pDocLine = doc.docLineMgr.GetLine(nLine); //#######非効率
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
		EditDoc::GetInstance(0)->docEditor.SetNewLineCode(eol);
	}

	if (bReplace) {
		doc.layoutMgr._DoLayout();
		doc.pEditWnd->ClearViewCaretPosInfo();
		if (doc.nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
			doc.layoutMgr.CalculateTextWidth();
		}else {
			doc.layoutMgr.ClearLayoutLineWidth();
		}
	}
	// Undo記録
	if (view.commander.GetOpeBlk()) {
		if (view.commander.GetOpeBlk()->GetNum()>0) {
			// カーソル位置復元
			textArea.SetViewTopLine(nViewTopLine);
			textArea.SetViewLeftCol(nViewLeftCol);
			caret.MoveCursor(ptCaretPosXY, true);
			caret.nCaretPosX_Prev = nCaretPosX_Prev;
			view.commander.GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					caret.GetCaretLogicPos()
				)
			);

		}
		view.SetUndoBuffer();
	}
}

