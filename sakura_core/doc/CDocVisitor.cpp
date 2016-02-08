#include "StdAfx.h"
#include "CDocVisitor.h"
#include "doc/CEditDoc.h"
#include "cmd/CViewCommander_inline.h"
#include "view/CEditView.h"
#include "window/CEditWnd.h"
#include "COpeBlk.h"


// ���s�R�[�h�𓝈ꂷ��
void CDocVisitor::SetAllEol(CEol cEol)
{
	CEditView* pcView = &CEditWnd::getInstance()->GetActiveView();

	// �A���h�D�L�^�J�n
	if (!pcView->m_bDoing_UndoRedo) {
		if (!pcView->m_cCommander.GetOpeBlk()) {
			pcView->m_cCommander.SetOpeBlk(new OpeBlk());
		}
		pcView->m_cCommander.GetOpeBlk()->AddRef();
	}

	// �J�[�\���ʒu�L��
	LayoutInt		nViewTopLine = pcView->GetTextArea().GetViewTopLine();
	LayoutInt		nViewLeftCol = pcView->GetTextArea().GetViewLeftCol();
	LayoutPoint	ptCaretPosXY = pcView->GetCaret().GetCaretLayoutPos();
	LayoutInt		nCaretPosX_Prev = pcView->GetCaret().m_nCaretPosX_Prev;

	bool bReplace = false;
	// ���s�R�[�h�𓝈ꂷ��
	if (cEol.IsValid()) {
		LogicInt nLine = LogicInt(0);
		OpeBlk* pcOpeBlk = pcView->m_bDoing_UndoRedo ? NULL : pcView->m_cCommander.GetOpeBlk();
		for (;;) {
			CDocLine* pcDocLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLine); //#######�����
			if (!pcDocLine) {
				break;
			}
			// ���s��u��
			if (pcDocLine->GetEol() != EOL_NONE && pcDocLine->GetEol() != cEol) {
				LogicRange sRange;
				sRange.SetFrom(LogicPoint(pcDocLine->GetLengthWithoutEOL(), nLine));
				sRange.SetTo(LogicPoint(pcDocLine->GetLengthWithEOL(), nLine));
				pcView->ReplaceData_CEditView2(
					sRange,
					cEol.GetValue2(),
					cEol.GetLen(),
					false,
					pcOpeBlk,
					true
				);
				bReplace = true;
			}
			++nLine;
		}
		// �ҏW�����͉��s�R�[�h
		EditDoc::GetInstance(0)->m_cDocEditor.SetNewLineCode(cEol);
	}

	if (bReplace) {
		m_pcDocRef->m_cLayoutMgr._DoLayout();
		m_pcDocRef->m_pcEditWnd->ClearViewCaretPosInfo();
		if (m_pcDocRef->m_nTextWrapMethodCur == (int)eTextWrappingMethod::NoWrapping) {
			m_pcDocRef->m_cLayoutMgr.CalculateTextWidth();
		}else {
			m_pcDocRef->m_cLayoutMgr.ClearLayoutLineWidth();
		}
	}
	// �A���h�D�L�^
	if (pcView->m_cCommander.GetOpeBlk()) {
		if (pcView->m_cCommander.GetOpeBlk()->GetNum()>0) {
			// �J�[�\���ʒu����
			pcView->GetTextArea().SetViewTopLine(nViewTopLine);
			pcView->GetTextArea().SetViewLeftCol(nViewLeftCol);
			pcView->GetCaret().MoveCursor(ptCaretPosXY, true);
			pcView->GetCaret().m_nCaretPosX_Prev = nCaretPosX_Prev;
			pcView->m_cCommander.GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					pcView->GetCaret().GetCaretLogicPos()
				)
			);

		}
		pcView->SetUndoBuffer();
	}
}

