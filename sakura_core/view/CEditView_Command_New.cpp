/*!	@file
	@brief EditView�N���X�̃R�}���h�����n�֐��Q

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, genta, asa-o, hor
	Copyright (C) 2002, YAZAKI, hor, genta. aroka, MIK, minfu, KK, �����
	Copyright (C) 2003, MIK, Moca
	Copyright (C) 2004, genta, Moca
	Copyright (C) 2005, ryoji, genta, D.S.Koba
	Copyright (C) 2006, genta, Moca, fon
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "view/CEditView.h"
#include "CSearchAgent.h"
#include "uiparts/CWaitCursor.h"
#include "charset/charcode.h"
#include "COpe.h" ///	2002/2/3 aroka from here
#include "COpeBlk.h" ///
#include "doc/CEditDoc.h"	//	2002/5/13 YAZAKI �w�b�_����
#include "doc/CDocReader.h"
#include "doc/layout/CLayout.h"
#include "doc/logic/CDocLine.h"
#include "cmd/CViewCommander_inline.h"
#include "window/CEditWnd.h"
#include "dlg/CDlgCtrlCode.h"	// �R���g���[���R�[�h�̓���(�_�C�A���O)
#include "dlg/CDlgFavorite.h"	// �����̊Ǘ�	//@@@ 2003.04.08 MIK
#include "debug/CRunningTimer.h"

using namespace std; // 2002/2/3 aroka

static void StringToOpeLineData(const wchar_t* pLineData, int nLineDataLen, OpeLineData& lineData, int opeSeq)
{
	int nBegin = 0;
	int i;
	bool bExtEol = GetDllShareData().m_common.m_sEdit.m_bEnableExtEol;
	for (i=0; i<nLineDataLen; ++i) {
		if (WCODE::IsLineDelimiter(pLineData[i], bExtEol)) {
			if (i + 1 < nLineDataLen && WCODE::CR == pLineData[i] && WCODE::LF == pLineData[i + 1]) {
				++i;
			}
			LineData tmp;
			lineData.push_back(tmp);
			LineData& insertLine = lineData[lineData.size()-1];
			insertLine.cmemLine.SetString(&pLineData[nBegin], i - nBegin + 1);
			insertLine.nSeq = opeSeq;
			nBegin = i + 1;
		}
	}
	if (nBegin < i) {
		LineData tmp;
		lineData.push_back(tmp);
		LineData& insertLine = lineData[lineData.size()-1];
		insertLine.cmemLine.SetString(&pLineData[nBegin], nLineDataLen - nBegin);
		insertLine.nSeq = opeSeq;
	}
}


/*!	���݈ʒu�Ƀf�[�^��}�� Ver0

	@date 2002/03/24 YAZAKI bUndo�폜
*/
void EditView::InsertData_CEditView(
	LayoutPoint	ptInsertPos,	// [in] �}���ʒu
	const wchar_t*	pData,			// [in] �}���e�L�X�g
	int				nDataLen,		// [in] �}���e�L�X�g���B�����P�ʁB
	LayoutPoint*	pptNewPos,		// [out] �}�����ꂽ�����̎��̈ʒu�̃��C�A�E�g�ʒu
	bool			bRedraw
)
{
#ifdef _DEBUG
	MY_RUNNINGTIMER(cRunningTimer, "EditView::InsertData_CEditView");
#endif

	// 2007.10.18 kobake COpe�����������ɂ܂Ƃ߂�
	InsertOpe* pcOpe = NULL;
	int opeSeq;
	if (!m_bDoing_UndoRedo) {	// �A���h�D�E���h�D�̎��s����
		pcOpe = new InsertOpe();
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			ptInsertPos,
			&pcOpe->m_ptCaretPos_PHY_Before
		);
		opeSeq = GetDocument()->m_cDocEditor.m_cOpeBuf.GetNextSeq();
	}else {
		opeSeq = 0;
	}

	pptNewPos->y = 0;			// �}�����ꂽ�����̎��̈ʒu�̃��C�A�E�g�s
	pptNewPos->x = 0;			// �}�����ꂽ�����̎��̈ʒu�̃��C�A�E�g�ʒu

	// �e�L�X�g���I������Ă��邩
	if (GetSelectionInfo().IsTextSelected()) {
		DeleteData(bRedraw);
		ptInsertPos = GetCaret().GetCaretLayoutPos();
	}

	// �e�L�X�g�擾 -> pLine, nLineLen, pcLayout
	bool			bHintPrev = false;	// �X�V���O�s����ɂȂ�\�������邱�Ƃ���������
	bool			bHintNext = false;	// �X�V�����s����ɂȂ�\�������邱�Ƃ���������
	LogicInt		nLineLen;
	const Layout*	pcLayout;
	const wchar_t*	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr(ptInsertPos.GetY2(), &nLineLen, &pcLayout);
	bool			bLineModifiedChange = (pLine)? !ModifyVisitor().IsLineModified(pcLayout->GetDocLineRef(),
		GetDocument()->m_cDocEditor.m_cOpeBuf.GetNoModifiedSeq()): true;

	// �֑��̗L��
	// �֑�������ꍇ��1�s�O����ĕ`����s��	@@@ 2002.04.19 MIK
	bool bKinsoku = 0
			|| (m_pTypeData->m_bWordWrap
			|| m_pTypeData->m_bKinsokuHead	//@@@ 2002.04.19 MIK
			|| m_pTypeData->m_bKinsokuTail	//@@@ 2002.04.19 MIK
			|| m_pTypeData->m_bKinsokuRet	//@@@ 2002.04.19 MIK
			|| m_pTypeData->m_bKinsokuKuto);	//@@@ 2002.04.19 MIK

	LayoutInt	nLineAllColLen;
	LogicInt	nIdxFrom = LogicInt(0);
	LayoutInt	nColumnFrom = ptInsertPos.GetX2();
	NativeW	cMem(L"");
	OpeLineData insData;
	if (pLine) {
		// �X�V���O�s����ɂȂ�\���𒲂ׂ�	// 2009.02.17 ryoji
		// ���܂�Ԃ��s���ւ̋�Ǔ_���͂őO�̍s�������X�V�����ꍇ������
		// ���}���ʒu�͍s�r���ł���Ǔ_���́{���[�h���b�v�őO�̕����񂩂瑱���đO�s�ɉ�荞�ޏꍇ������
		if (pcLayout->GetLogicOffset() && bKinsoku) {	// �܂�Ԃ����C�A�E�g�s���H
			bHintPrev = true;	// �X�V���O�s����ɂȂ�\��������
		}

		// �X�V�����s����ɂȂ�\���𒲂ׂ�	// 2009.02.17 ryoji
		// ���܂�Ԃ��s���ւ̕������͂╶����\��t���Ō��ݍs�͍X�V���ꂸ���s�Ȍオ�X�V�����ꍇ������
		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
		nIdxFrom = LineColumnToIndex2(pcLayout, ptInsertPos.GetX2(), &nLineAllColLen);

		// �s�I�[���E�ɑ}�����悤�Ƃ���
		if (nLineAllColLen > 0) {
			// �I�[���O����}���ʒu�܂ŋ󔒂𖄂߂�ׂ̏���
			// �s�I�[�����炩�̉��s�R�[�h��?
			if (EOL_NONE != pcLayout->GetLayoutEol()) {
				nIdxFrom = nLineLen - LogicInt(1);
				cMem.AllocStringBuffer((Int)(ptInsertPos.GetX2() - nLineAllColLen + 1) + nDataLen);
				for (int i=0; i<ptInsertPos.GetX2()-nLineAllColLen+1; ++i) {
					cMem += L' ';
				}
				cMem.AppendString(pData, nDataLen);
			}else {
				nIdxFrom = nLineLen;
				cMem.AllocStringBuffer((Int)(ptInsertPos.GetX2() - nLineAllColLen) + nDataLen);
				for (int i=0; i<ptInsertPos.GetX2()-nLineAllColLen; ++i) {
					cMem += L' ';
				}
				cMem.AppendString(pData, nDataLen);
				// 1�s�����X�V����K�v������\��������
				bHintNext = true;
			}
			StringToOpeLineData(cMem.GetStringPtr(), cMem.GetStringLength(), insData, opeSeq);
			cMem.Clear();
			nColumnFrom = LineIndexToColumn(pcLayout, nIdxFrom);
		}else {
			StringToOpeLineData(pData, nDataLen, insData, opeSeq);
		}
	}else {
		// �X�V���O�s����ɂȂ�\���𒲂ׂ�	// 2009.02.17 ryoji
		const Layout* pcLayoutWk = m_pcEditDoc->m_cLayoutMgr.GetBottomLayout();
		if (pcLayoutWk && pcLayoutWk->GetLayoutEol() == EOL_NONE && bKinsoku) {	// �܂�Ԃ����C�A�E�g�s���H�i�O�s�̏I�[�Œ����j
			bHintPrev = true;	// �X�V���O�s����ɂȂ�\��������
		}
		if (0 < ptInsertPos.GetX2()) {
			cMem.AllocStringBuffer((Int)ptInsertPos.GetX2() + nDataLen);
			for (LayoutInt i=LayoutInt(0); i<ptInsertPos.GetX2(); ++i) {
				cMem += L' ';
			}
			cMem.AppendString(pData, nDataLen);
			StringToOpeLineData(cMem.GetStringPtr(), cMem.GetStringLength(), insData, opeSeq);
			cMem.Clear();
		}else {
			StringToOpeLineData(pData, nDataLen, insData, opeSeq);
		}
		nColumnFrom = 0;
	}


	if (!m_bDoing_UndoRedo && pcOpe) {	// �A���h�D�E���h�D�̎��s����
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			LayoutPoint(nColumnFrom, ptInsertPos.y),
			&pcOpe->m_ptCaretPos_PHY_Before
		);
	}

	// ������}��
	LayoutInt	nModifyLayoutLinesOld = LayoutInt(0);
	LayoutInt	nInsLineNum;		// �}���ɂ���đ��������C�A�E�g�s�̐�
	int	nInsSeq;
	{
		LayoutReplaceArg arg;
		arg.sDelRange.Set(LayoutPoint(nColumnFrom, ptInsertPos.y));
		arg.pcmemDeleted = NULL;
		arg.pInsData = &insData;
		arg.nDelSeq = opeSeq;
		m_pcEditDoc->m_cLayoutMgr.ReplaceData_CLayoutMgr(&arg);
		nInsLineNum = arg.nAddLineNum;
		nModifyLayoutLinesOld = arg.nModLineTo - arg.nModLineFrom + 1;
		*pptNewPos = arg.ptLayoutNew;
		nInsSeq = arg.nInsSeq;
	}

	// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
	LogicInt nLineLen2;
	const wchar_t* pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr(pptNewPos->GetY2(), &nLineLen2, &pcLayout);
	if (pLine2) {
		// 2007.10.15 kobake ���Ƀ��C�A�E�g�P�ʂȂ̂ŕϊ��͕s�v
		pptNewPos->x = pptNewPos->GetX2(); //LineIndexToColumn(pcLayout, pptNewPos->GetX2());
	}

	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	if (pptNewPos->x >= m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas()) {
		if (m_pTypeData->m_bKinsokuRet
		 || m_pTypeData->m_bKinsokuKuto
		) {	//@@@ 2002.04.16 MIK
			if (m_pcEditDoc->m_cLayoutMgr.IsEndOfLine(*pptNewPos)) {	//@@@ 2002.04.18
				pptNewPos->x = 0;
				pptNewPos->y++;
			}
		}else {
			// Oct. 7, 2002 YAZAKI
			pptNewPos->x = pcLayout->GetNextLayout() ? pcLayout->GetNextLayout()->GetIndent() : LayoutInt(0);
			pptNewPos->y++;
		}
	}

	// ��ԑJ��
	if (!m_bDoing_UndoRedo) {	// �A���h�D�E���h�D�̎��s����
		m_pcEditDoc->m_cDocEditor.SetModified(true, bRedraw);	//	Jan. 22, 2002 genta
	}

	// �ĕ`��
	// �s�ԍ��\���ɕK�v�ȕ���ݒ�
	if (m_pcEditWnd->DetectWidthOfLineNumberAreaAllPane(bRedraw)) {
		// �L�����b�g�̕\���E�X�V
		GetCaret().ShowEditCaret();
	}else {
		PAINTSTRUCT ps;

		if (bRedraw) {
			LayoutInt nStartLine(ptInsertPos.y);
			// 2013.05.08 �܂�Ԃ��s��EOF���O�ŉ��s�����Ƃ�EOF���ĕ`�悳��Ȃ��o�O�̏C��
			if (nModifyLayoutLinesOld < 1) {
				nModifyLayoutLinesOld = LayoutInt(1);
			}
			// 2011.12.26 ���K�\���L�[���[�h�E����������Ȃǂ́A���W�b�N�s���܂ł����̂ڂ��čX�V����K�v������
			{
				const Layout* pcLayoutLineFirst = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY(ptInsertPos.GetY2());
				while (pcLayoutLineFirst && pcLayoutLineFirst->GetLogicOffset() != 0) {
					pcLayoutLineFirst = pcLayoutLineFirst->GetPrevLayout();
					if (bHintPrev) {
						bHintPrev = false;
					}
					--nStartLine;
					++nModifyLayoutLinesOld;
				}
			}
			LayoutYInt nLayoutTop;
			LayoutYInt nLayoutBottom;
			if (nInsLineNum != 0) {
				// �X�N���[���o�[�̏�Ԃ��X�V����
				AdjustScrollBars();

				// �`��J�n�s�ʒu�𒲐�����	// 2009.02.17 ryoji
				if (bHintPrev) {	// �X�V���O�s����ɂȂ�\��������
					--nStartLine;
				}

				ps.rcPaint.left = 0;
				ps.rcPaint.right = GetTextArea().GetAreaRight();
				ps.rcPaint.top = GetTextArea().GenerateYPx(nStartLine);
				ps.rcPaint.bottom = GetTextArea().GetAreaBottom();
				nLayoutTop = nStartLine;
				nLayoutBottom = LayoutYInt(-1);
			}else {
				// �`��J�n�s�ʒu�ƕ`��s���𒲐�����	// 2009.02.17 ryoji
				if (bHintPrev) {	// �X�V���O�s����ɂȂ�\��������
					--nStartLine;
					++nModifyLayoutLinesOld;
				}
				if (bHintNext) {	// �X�V�����s����ɂȂ�\��������
					++nModifyLayoutLinesOld;
				}

	//			ps.rcPaint.left = GetTextArea().GetAreaLeft();
				ps.rcPaint.left = 0;
				ps.rcPaint.right = GetTextArea().GetAreaRight();

				// 2002.02.25 Mod By KK ���s (ptInsertPos.y - GetTextArea().GetViewTopLine() - 1); => (ptInsertPos.y - GetTextArea().GetViewTopLine());
				//ps.rcPaint.top = GetTextArea().GetAreaTop() + GetTextMetrics().GetHankakuDy() * (ptInsertPos.y - GetTextArea().GetViewTopLine() - 1);
				ps.rcPaint.top = GetTextArea().GenerateYPx(nStartLine);
				ps.rcPaint.bottom = GetTextArea().GenerateYPx(nStartLine + nModifyLayoutLinesOld);
				nLayoutTop = nStartLine;
				nLayoutBottom = nStartLine + nModifyLayoutLinesOld;
			}
			HDC hdc = this->GetDC();
			OnPaint(hdc, &ps, FALSE);
			this->ReleaseDC(hdc);
			// 2014.07.16 ���̃r���[(�~�j�}�b�v)�̍ĕ`���}������
			if (nInsLineNum == 0) {
				for (int i=0; i<m_pcEditWnd->GetAllViewCount(); ++i) {
					EditView* pcView = &m_pcEditWnd->GetView(i);
					if (pcView == this) {
						continue;
					}
					pcView->RedrawLines(nLayoutTop, nLayoutBottom);
				}
				m_pcEditWnd->GetMiniMap().RedrawLines(nLayoutTop, nLayoutBottom);
				if (!m_bDoing_UndoRedo && pcOpe) {
					GetDocument()->m_cDocEditor.m_nOpeBlkRedawCount++;
				}
			}

#if 0 // ���łɍs������`��ς�
			// �s�ԍ��i�ύX�s�j�\���͉��s�P�ʂ̍s������X�V����K�v������	// 2009.03.26 ryoji
			if (bLineModifiedChange) {	// ���ύX�������s���ύX���ꂽ
				const Layout* pcLayoutWk = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY(nStartLine);
				if (pcLayoutWk && pcLayoutWk->GetLogicOffset()) {	// �܂�Ԃ����C�A�E�g�s���H
					Call_OnPaint(PAINT_LINENUMBER, false);
				}
			}
#endif
		}
	}

	// 2007.10.18 kobake ������COpe�������܂Ƃ߂�
	if (!m_bDoing_UndoRedo) {
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			*pptNewPos,
			&pcOpe->m_ptCaretPos_PHY_After
		);
		pcOpe->m_nOrgSeq = nInsSeq;

		// ����̒ǉ�
		m_cCommander.GetOpeBlk()->AppendOpe(pcOpe);
	}
}


/*!	�w��ʒu�̎w�蒷�f�[�^�폜

	@param _ptCaretPos [in]  �폜�f�[�^�̈ʒu
	@param nDelLen [out] �폜�f�[�^�̃T�C�Y
	@param pcMem [out]  �폜�����f�[�^(NULL�\)

	@date 2002/03/24 YAZAKI bUndo�폜
	@date 2002/05/12 YAZAKI bRedraw, bRedraw2�폜�i���FALSE������j
	@date 2007/10/17 kobake (�d�v)pcMem�̏��L�҂������ɂ��COpe�Ɉڂ�����ڂ�Ȃ������肷��U�镑����
	                        ���ɂ�₱���������̌��ɂȂ邽�߁A��ɁApcMem�̏��L�҂͈ڂ��Ȃ��悤�Ɏd�l�ύX�B
*/
void EditView::DeleteData2(
	const LayoutPoint& _ptCaretPos,
	LogicInt			nDelLen,
	NativeW*			pcMem
)
{
#ifdef _DEBUG
	MY_RUNNINGTIMER(cRunningTimer, "EditView::DeleteData(1)");
#endif
	LogicInt nLineLen;
	const Layout* pcLayout;
	const wchar_t* pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr(_ptCaretPos.GetY2(), &nLineLen, &pcLayout);
	if (!pLine) {
		return;
	}
	LogicInt nIdxFrom = LineColumnToIndex(pcLayout, _ptCaretPos.GetX2());

	// 2007.10.18 kobake COpe�̐����������ɂ܂Ƃ߂�
	DeleteOpe*	pcOpe = NULL;
	LayoutInt columnFrom = LineIndexToColumn(pcLayout, nIdxFrom);
	LayoutInt columnTo = LineIndexToColumn(pcLayout, nIdxFrom + nDelLen);
	if (!m_bDoing_UndoRedo) {
		pcOpe = new DeleteOpe();
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			LayoutPoint(columnFrom, _ptCaretPos.GetY2()),
			&pcOpe->m_ptCaretPos_PHY_Before
		);
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			LayoutPoint(columnTo, _ptCaretPos.GetY2()),
			&pcOpe->m_ptCaretPos_PHY_To
		);
	}
	OpeLineData memDeleted;
	OpeLineData* pmemDeleted = NULL;
	if (pcMem || pcOpe) {
		pmemDeleted = &memDeleted;
	}

	// �f�[�^�폜
	{
		LayoutReplaceArg arg;
		arg.sDelRange.SetFrom(_ptCaretPos);
		arg.sDelRange.SetTo(LayoutPoint(columnTo, _ptCaretPos.GetY2()));
		arg.pcmemDeleted = pmemDeleted;
		arg.pInsData = NULL;
		arg.nDelSeq = GetDocument()->m_cDocEditor.m_cOpeBuf.GetNextSeq();
		m_pcEditDoc->m_cLayoutMgr.ReplaceData_CLayoutMgr(&arg);
	}

	// �I���G���A�̐擪�փJ�[�\�����ړ�
	GetCaret().MoveCursor(_ptCaretPos, false);
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();

	if (pcMem) {
		int size = (int)memDeleted.size();
		size_t bufSize = 0;
		for (int i=0; i<size; ++i) {
			bufSize += memDeleted[i].cmemLine.GetStringLength();
		}
		pcMem->SetString(L"");
		pcMem->AllocStringBuffer(bufSize);
		for (int i=0; i<size; ++i) {
			pcMem->AppendNativeData(memDeleted[i].cmemLine);
		}
	}
	// 2007.10.18 kobake COpe�̒ǉ��������ɂ܂Ƃ߂�
	if (pcOpe) {
		pcOpe->m_cOpeLineData.swap(memDeleted);
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			_ptCaretPos,
			&pcOpe->m_ptCaretPos_PHY_After
		);
		// ����̒ǉ�
		m_cCommander.GetOpeBlk()->AppendOpe(pcOpe);
	}

}


/*!	�J�[�\���ʒu�܂��͑I���G���A���폜

	@date 2002/03/24 YAZAKI bUndo�폜
*/
void EditView::DeleteData(
	bool	bRedraw
//	BOOL	bUndo	// Undo���삩�ǂ���
)
{
#ifdef _DEBUG
	MY_RUNNINGTIMER(cRunningTimer, "EditView::DeleteData(2)");
#endif
	LogicInt	nLineLen;
	LayoutInt	nLineNum;
	LogicInt	nCurIdx;
	LogicInt	nIdxFrom;
	LogicInt	nIdxTo;
	LogicInt	nDelLen;
	LogicInt	nDelLenNext;
	LayoutRect		rcSel;
	const Layout*	pcLayout;

	auto& selInfo = GetSelectionInfo();
	// �e�L�X�g�̑��݂��Ȃ��G���A�̍폜�́A�I��͈͂̃L�����Z���ƃJ�[�\���ړ��݂̂Ƃ���	// 2008.08.05 ryoji
	if (selInfo.IsTextSelected()) {		// �e�L�X�g���I������Ă��邩
		if (IsEmptyArea(selInfo.m_sSelect.GetFrom(), selInfo.m_sSelect.GetTo(), true, selInfo.IsBoxSelecting())) {
			// �J�[�\����I��͈͂̍���Ɉړ�
			GetCaret().MoveCursor(
				LayoutPoint(
					selInfo.m_sSelect.GetFrom().GetX2() < selInfo.m_sSelect.GetTo().GetX2() ? selInfo.m_sSelect.GetFrom().GetX2() : selInfo.m_sSelect.GetTo().GetX2(),
					selInfo.m_sSelect.GetFrom().GetY2() < selInfo.m_sSelect.GetTo().GetY2() ? selInfo.m_sSelect.GetFrom().GetY2() : selInfo.m_sSelect.GetTo().GetY2()
				), bRedraw
			);
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			selInfo.DisableSelectArea(bRedraw);
			return;
		}
	}else {
		if (IsEmptyArea(GetCaret().GetCaretLayoutPos())) {
			return;
		}
	}

	LayoutPoint ptCaretPosOld = GetCaret().GetCaretLayoutPos();

	// �e�L�X�g���I������Ă��邩
	if (selInfo.IsTextSelected()) {
		WaitCursor cWaitCursor(this->GetHwnd());  // 2002.02.05 hor
		if (!m_bDoing_UndoRedo) {	// �A���h�D�E���h�D�̎��s����
			// ����̒ǉ�
			m_cCommander.GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					GetCaret().GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
				)
			);
		}

		// ��`�͈͑I�𒆂�
		if (selInfo.IsBoxSelecting()) {
			m_pcEditDoc->m_cDocEditor.SetModified(true, bRedraw);	//	2002/06/04 YAZAKI ��`�I�����폜�����Ƃ��ɕύX�}�[�N�����Ȃ��B

			SetDrawSwitch(false);	// 2002.01.25 hor
			// �I��͈͂̃f�[�^���擾
			// ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ�
			// �Q�_��Ίp�Ƃ����`�����߂�
			TwoPointToRect(
				&rcSel,
				selInfo.m_sSelect.GetFrom(),	// �͈͑I���J�n
				selInfo.m_sSelect.GetTo()		// �͈͑I���I��
			);
			// ���݂̑I��͈͂��I����Ԃɖ߂�
			selInfo.DisableSelectArea(bRedraw);

			nIdxFrom = LogicInt(0);
			nIdxTo = LogicInt(0);
			for (nLineNum=rcSel.bottom; nLineNum>=rcSel.top-1; --nLineNum) {
				nDelLenNext	= nIdxTo - nIdxFrom;
				const wchar_t* pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr(nLineNum, &nLineLen, &pcLayout);
				if (pLine) {
					using namespace WCODE;

					// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
					nIdxFrom = LineColumnToIndex(pcLayout, rcSel.left);
					nIdxTo	 = LineColumnToIndex(pcLayout, rcSel.right);

					bool bExtEol = GetDllShareData().m_common.m_sEdit.m_bEnableExtEol;
					for (LogicInt i=nIdxFrom; i<=nIdxTo; ++i) {
						if (WCODE::IsLineDelimiter(pLine[i], bExtEol)) {
							nIdxTo = i;
							break;
						}
					}
				}else {
					nIdxFrom = LogicInt(0);
					nIdxTo	 = LogicInt(0);
				}
				nDelLen	= nDelLenNext;
				if (nLineNum < rcSel.bottom && 0 < nDelLen) {
					// �w��ʒu�̎w�蒷�f�[�^�폜
					DeleteData2(
						LayoutPoint(rcSel.left, nLineNum + 1),
						nDelLen,
						NULL
					);
				}
			}
			SetDrawSwitch(true);	// 2002.01.25 hor

			// �s�ԍ��\���ɕK�v�ȕ���ݒ�
			if (m_pcEditWnd->DetectWidthOfLineNumberAreaAllPane(true)) {
				// �L�����b�g�̕\���E�X�V
				GetCaret().ShowEditCaret();
			}
			if (bRedraw) {
				// �X�N���[���o�[�̏�Ԃ��X�V����
				AdjustScrollBars();

				// �ĕ`��
				Call_OnPaint(PAINT_LINENUMBER | PAINT_BODY, false);
			}
			// �I���G���A�̐擪�փJ�[�\�����ړ�
			this->UpdateWindow();
			
			LayoutPoint caretOld = LayoutPoint(rcSel.left, rcSel.top);
			m_pcEditDoc->m_cLayoutMgr.GetLineStr(rcSel.top, &nLineLen, &pcLayout);
			if (rcSel.left <= pcLayout->CalcLayoutWidth(m_pcEditDoc->m_cLayoutMgr)) {
				// EOL��荶�Ȃ當���̒P�ʂɂ��낦��
				LogicInt nIdxCaret = LineColumnToIndex(pcLayout, rcSel.left);
				caretOld.SetX(LineIndexToColumn(pcLayout, nIdxCaret));
			}
			GetCaret().MoveCursor(caretOld, bRedraw);
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();
			if (!m_bDoing_UndoRedo) {	// �A���h�D�E���h�D�̎��s����
				MoveCaretOpe*		pcOpe = new MoveCaretOpe();
				m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
					ptCaretPosOld,
					&pcOpe->m_ptCaretPos_PHY_Before
				);

				pcOpe->m_ptCaretPos_PHY_After = GetCaret().GetCaretLogicPos();	// �����̃L�����b�g�ʒu
				// ����̒ǉ�
				m_cCommander.GetOpeBlk()->AppendOpe(pcOpe);
			}
		}else {
			// �f�[�^�u�� �폜&�}���ɂ��g����
			ReplaceData_CEditView(
				selInfo.m_sSelect,
				L"",					// �}������f�[�^
				LogicInt(0),			// �}������f�[�^�̒���
				bRedraw,
				m_bDoing_UndoRedo ? NULL : m_cCommander.GetOpeBlk()
			);
		}
	}else {
		// ���ݍs�̃f�[�^���擾
		const wchar_t* pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr(GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout);
		if (!pLine) {
			goto end_of_func;
//			return;
		}
		// �Ō�̍s�ɃJ�[�\�������邩�ǂ���
		bool bLastLine = (GetCaret().GetCaretLayoutPos().GetY() == m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1);

		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
		nCurIdx = LineColumnToIndex(pcLayout, GetCaret().GetCaretLayoutPos().GetX2());
//		MYTRACE(_T("nLineLen=%d nCurIdx=%d \n"), nLineLen, nCurIdx);
		if (nCurIdx == nLineLen && bLastLine) {	// �S�e�L�X�g�̍Ō�
			goto end_of_func;
//			return;
		}

		// �w�肳�ꂽ���̕����̃o�C�g���𒲂ׂ�
		LogicInt	nNxtIdx;
		LayoutInt	nNxtPos;
		bool bExtEol = GetDllShareData().m_common.m_sEdit.m_bEnableExtEol;
		if (WCODE::IsLineDelimiter(pLine[nCurIdx], bExtEol)) {
			// ���s
			nNxtIdx = nCurIdx + pcLayout->GetLayoutEol().GetLen();
			nNxtPos = GetCaret().GetCaretLayoutPos().GetX() + LayoutInt((Int)pcLayout->GetLayoutEol().GetLen()); // �����s�R�[�h�̕������𕶎����ƌ��Ȃ�
		}else {
			nNxtIdx = LogicInt(NativeW::GetCharNext(pLine, nLineLen, &pLine[nCurIdx]) - pLine);
			// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
			nNxtPos = LineIndexToColumn(pcLayout, nNxtIdx);
		}

		// �f�[�^�u�� �폜&�}���ɂ��g����
		LayoutRange sDelRange;
		sDelRange.SetFrom(GetCaret().GetCaretLayoutPos());
		sDelRange.SetTo(LayoutPoint(nNxtPos, GetCaret().GetCaretLayoutPos().GetY()));
		LogicRange sDelRangeLogic;
		sDelRangeLogic.SetFrom(GetCaret().GetCaretLogicPos());
		sDelRangeLogic.SetTo(LogicPoint(nNxtIdx + pcLayout->GetLogicOffset(), GetCaret().GetCaretLogicPos().GetY()));
		ReplaceData_CEditView(
			sDelRange,
			L"",				// �}������f�[�^
			LogicInt(0),		// �}������f�[�^�̒���
			bRedraw,
			m_bDoing_UndoRedo ? NULL : m_cCommander.GetOpeBlk(),
			false,
			&sDelRangeLogic
		);
	}

	m_pcEditDoc->m_cDocEditor.SetModified(true, bRedraw);	//	Jan. 22, 2002 genta

	if (m_pcEditDoc->m_cLayoutMgr.GetLineCount() > 0) {
		if (GetCaret().GetCaretLayoutPos().GetY() > m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1) {
			// ���ݍs�̃f�[�^���擾
			const Layout*	pcLayout;
			const wchar_t* pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr(m_pcEditDoc->m_cLayoutMgr.GetLineCount() - LayoutInt(1), &nLineLen, &pcLayout);
			if (!pLine) {
				goto end_of_func;
			}
			// ���s�ŏI����Ă��邩
			if (EOL_NONE != pcLayout->GetLayoutEol()) {
				goto end_of_func;
			}
			// �t�@�C���̍Ō�Ɉړ�
			GetCommander().Command_GOFILEEND(false);
		}
	}
end_of_func:;

	return;
}


void EditView::ReplaceData_CEditView(
	const LayoutRange&	sDelRange,			//!< [in]  �폜�͈̓��C�A�E�g�P��
	const wchar_t*		pInsData,			//!< [in]  �}������f�[�^
	LogicInt			nInsDataLen,		//!< [in]  �}������f�[�^�̒���
	bool				bRedraw,
	OpeBlk*			pcOpeBlk,
	bool				bFastMode,
	const LogicRange*	psDelRangeLogicFast
)
{
	auto& opeBuf = GetDocument()->m_cDocEditor.m_cOpeBuf;
	int opeSeq = m_bDoing_UndoRedo ? opeBuf.GetCurrentPointer() : opeBuf.GetNextSeq();
	if (nInsDataLen == 0) {
		ReplaceData_CEditView3(sDelRange, NULL, NULL, bRedraw, pcOpeBlk, opeSeq, NULL, bFastMode, psDelRangeLogicFast);
	}else {
		OpeLineData insData;
		StringToOpeLineData(pInsData, nInsDataLen, insData, opeSeq);
		ReplaceData_CEditView3(sDelRange, NULL, &insData, bRedraw, pcOpeBlk, opeSeq, NULL, bFastMode, psDelRangeLogicFast);
	}
}

void EditView::ReplaceData_CEditView2(
	const LogicRange&	sDelRange,			// �폜�͈́B���W�b�N�P�ʁB
	const wchar_t*		pInsData,			// �}������f�[�^
	LogicInt			nInsDataLen,		// �}������f�[�^�̒���
	bool				bRedraw,
	OpeBlk*			pcOpeBlk,
	bool				bFastMode
)
{
	LayoutRange sDelRangeLayout;
	if (!bFastMode) {
		this->m_pcEditDoc->m_cLayoutMgr.LogicToLayout(sDelRange, &sDelRangeLayout);
	}
	ReplaceData_CEditView(sDelRangeLayout, pInsData, nInsDataLen, bRedraw, pcOpeBlk, bFastMode, &sDelRange);
}


// �f�[�^�u�� �폜&�}���ɂ��g����
// Jun 23, 2000 genta �ϐ��������������Y��Ă����̂��C��
// Jun. 1, 2000 genta DeleteData����ړ�����
bool EditView::ReplaceData_CEditView3(
	LayoutRange	sDelRange,			//!< [in]  �폜�͈̓��C�A�E�g�P��
	OpeLineData*	pcmemCopyOfDeleted,	//!< [out] �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
	OpeLineData*	pInsData,			//!< [in]  �}������f�[�^
	bool			bRedraw,
	OpeBlk*		pcOpeBlk,
	int				nDelSeq,
	int*			pnInsSeq,
	bool			bFastMode,			//!< [in] CDocLineMgr���X�V���Ȃ�,�s���`�F�b�N���ȗ�����BbRedraw==false�̕K�v����
	const LogicRange*	psDelRangeLogicFast
)
{
	assert((bFastMode && !bRedraw) || (!bFastMode)); // bFastMode�̂Ƃ��� bReadraw == false
	bool bLineModifiedChange;
	bool bUpdateAll = true;
	auto& layoutMgr = m_pcEditDoc->m_cLayoutMgr;
	bool bDelRangeUpdate = false;
	{
		//	May. 29, 2000 genta
		//	From Here
		//	�s�̌�낪�I������Ă����Ƃ��̕s���������邽�߁C
		//	�I��̈悩��s���ȍ~�̕�������菜���D

		//	�擪
		const Layout* pcLayout;
		LogicInt len;
		const wchar_t* line = NULL;
		if (!bFastMode) {
			line = layoutMgr.GetLineStr(sDelRange.GetFrom().GetY2(), &len, &pcLayout);
		}
		bLineModifiedChange = (line)? !ModifyVisitor().IsLineModified(pcLayout->GetDocLineRef(), GetDocument()->m_cDocEditor.m_cOpeBuf.GetNoModifiedSeq()): true;
		if (line) {
			LogicInt pos = LineColumnToIndex(pcLayout, sDelRange.GetFrom().GetX2());
			//	Jun. 1, 2000 genta
			//	����s�̍s���ȍ~�݂̂��I������Ă���ꍇ���l������

			//	Aug. 22, 2000 genta
			//	�J�n�ʒu��EOF�̌��̂Ƃ��͎��s�ɑ��鏈�����s��Ȃ�
			//	���������Ă��܂��Ƒ��݂��Ȃ��s��Point���ė�����D
			if (sDelRange.GetFrom().y < layoutMgr.GetLineCount() - 1 && pos >= len) {
				if (sDelRange.GetFrom().y == sDelRange.GetTo().y) {
					//	GetSelectionInfo().m_sSelect.GetFrom().y <= GetSelectionInfo().m_sSelect.GetTo().y �̓`�F�b�N���Ȃ�
					LayoutPoint tmp = sDelRange.GetFrom();
					tmp.y++;
					tmp.x = LayoutInt(0);
					sDelRange.Set(tmp);
				}else {
					sDelRange.GetFromPointer()->y++;
					sDelRange.SetFromX(LayoutInt(0));
				}
				bDelRangeUpdate = true;
			}
		}

		//	����
		if (!bFastMode) {
			line = layoutMgr.GetLineStr(sDelRange.GetTo().GetY2(), &len, &pcLayout);
			if (line) {
				LayoutInt p = LineIndexToColumn(pcLayout, len);

				if (sDelRange.GetTo().x > p) {
					sDelRange.SetToX(p);
					bDelRangeUpdate = true;
				}
			}
		}
		//	To Here
	}

	// �폜�͈̓��W�b�N�P�� sDelRange -> sDelRangeLogic
	LogicRange sDelRangeLogic;
	if (!bDelRangeUpdate && psDelRangeLogicFast) {
		sDelRangeLogic = *psDelRangeLogicFast;
	}else {
		layoutMgr.LayoutToLogic(
			sDelRange,
			&sDelRangeLogic
		);
	}

	LogicPoint ptCaretPos_PHY_Old = GetCaret().GetCaretLogicPos();
	if (pcOpeBlk) {	// �A���h�D�E���h�D�̎��s����
		// ����̒ǉ�
		if (sDelRangeLogic.GetFrom() != GetCaret().GetCaretLogicPos()) {
			pcOpeBlk->AppendOpe(
				new MoveCaretOpe(
					GetCaret().GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
				)
			);
		}
	}

	ReplaceOpe* pcReplaceOpe = NULL;	// �ҏW����v�f COpe
	if (pcOpeBlk) {
		pcReplaceOpe = new ReplaceOpe();
		pcReplaceOpe->m_ptCaretPos_PHY_Before = sDelRangeLogic.GetFrom();
		pcReplaceOpe->m_ptCaretPos_PHY_To = sDelRangeLogic.GetTo();
		pcReplaceOpe->m_ptCaretPos_PHY_After = pcReplaceOpe->m_ptCaretPos_PHY_Before;	// �����̃L�����b�g�ʒu
	}

	OpeLineData* pcMemDeleted = NULL;
	OpeLineData opeData;
	if (pcOpeBlk || pcmemCopyOfDeleted) {
		pcMemDeleted = &opeData;
	}


	// ���݂̑I��͈͂��I����Ԃɖ߂�
	// 2009.07.18 ryoji �u���と�u���O�Ɉʒu��ύX�i�u���ゾ�Ɣ��]���s���ɂȂ��ĉ��� Wiki BugReport/43�j
	GetSelectionInfo().DisableSelectArea(bRedraw);

	// ������u��
	LayoutReplaceArg LRArg;
	DocLineReplaceArg DLRArg;
	if (bFastMode) {
		DLRArg.sDelRange = sDelRangeLogic;
		DLRArg.pcmemDeleted = pcMemDeleted;
		DLRArg.pInsData = pInsData;
		DLRArg.nDelSeq = nDelSeq;
		// DLRArg.ptNewPos;
		SearchAgent(&GetDocument()->m_cDocLineMgr).ReplaceData(&DLRArg);
	}else {
		LRArg.sDelRange    = sDelRange;		//!< �폜�͈̓��C�A�E�g
		LRArg.pcmemDeleted = pcMemDeleted;	//!< [out] �폜���ꂽ�f�[�^
		LRArg.pInsData     = pInsData;		//!< �}������f�[�^
		LRArg.nDelSeq      = nDelSeq;	//!< �}������f�[�^�̒���
		layoutMgr.ReplaceData_CLayoutMgr(&LRArg);
	}

	//	Jan. 30, 2001 genta
	//	�ĕ`��̎��_�Ńt�@�C���X�V�t���O���K�؂ɂȂ��Ă��Ȃ��Ƃ����Ȃ��̂�
	//	�֐��̖������炱���ֈړ�
	// ��ԑJ��
	if (pcOpeBlk) {	// �A���h�D�E���h�D�̎��s����
		m_pcEditDoc->m_cDocEditor.SetModified(true, bRedraw);	//	Jan. 22, 2002 genta
	}

	// �s�ԍ��\���ɕK�v�ȕ���ݒ�
	if (m_pcEditWnd->DetectWidthOfLineNumberAreaAllPane(bRedraw)) {
		// �L�����b�g�̕\���E�X�V
		GetCaret().ShowEditCaret();
	}else {
		// �ĕ`��
		if (bRedraw) {
			// �ĕ`��q���g ���C�A�E�g�s�̑���
			//	Jan. 30, 2001 genta	�\��t���ōs��������ꍇ�̍l���������Ă���
			if (LRArg.nAddLineNum != 0) {
				Call_OnPaint(PAINT_LINENUMBER | PAINT_BODY, false);
			}else {
				// �����������s�Ȃ�������ɕω�������				// 2009.11.11 ryoji
				// EOF�̂ݍs���ǉ��ɂȂ�̂ŁA1�s�]���ɕ`�悷��B
				// �i�����������s���聨�Ȃ��ɕω�����ꍇ�̖���EOF�����͕`��֐����ōs����j
				int nAddLine = (LRArg.ptLayoutNew.GetY2() > LRArg.sDelRange.GetTo().GetY2())? 1: 0;

				PAINTSTRUCT ps;

				ps.rcPaint.left = 0;
				ps.rcPaint.right = GetTextArea().GetAreaRight();

				/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��) */
				ps.rcPaint.top = GetTextArea().GenerateYPx(LRArg.nModLineFrom);
				// 2011.12.26 ���K�\���L�[���[�h�E����������Ȃǂ́A���W�b�N�s���܂ł����̂ڂ��čX�V����K�v������
				{
					const Layout* pcLayoutLineFirst = layoutMgr.SearchLineByLayoutY(LRArg.nModLineFrom);
					while (pcLayoutLineFirst && pcLayoutLineFirst->GetLogicOffset() != 0) {
						pcLayoutLineFirst = pcLayoutLineFirst->GetPrevLayout();
						ps.rcPaint.top -= GetTextMetrics().GetHankakuDy();
						if (ps.rcPaint.top < 0) {
							break;
						}
					}
				}
				if (ps.rcPaint.top < 0) {
					ps.rcPaint.top = 0;
				}
				ps.rcPaint.bottom = GetTextArea().GenerateYPx(LRArg.nModLineTo + 1 + nAddLine);
				if (GetTextArea().GetAreaBottom() < ps.rcPaint.bottom) {
					ps.rcPaint.bottom = GetTextArea().GetAreaBottom();
				}

				HDC hdc = this->GetDC();
				OnPaint(hdc, &ps, FALSE);
				this->ReleaseDC(hdc);

				LayoutYInt nLayoutTop = LRArg.nModLineFrom;
				LayoutYInt nLayoutBottom = LRArg.nModLineTo + 1 + nAddLine;
				for (int i=0; i<m_pcEditWnd->GetAllViewCount(); ++i) {
					EditView* pcView = &m_pcEditWnd->GetView(i);
					if (pcView == this) {
						continue;
					}
					pcView->RedrawLines(nLayoutTop, nLayoutBottom);
				}
				m_pcEditWnd->GetMiniMap().RedrawLines(nLayoutTop, nLayoutBottom);
				if (!m_bDoing_UndoRedo && pcOpeBlk) {
					GetDocument()->m_cDocEditor.m_nOpeBlkRedawCount++;
				}
				bUpdateAll = false;
#if 0 // ���ł�1�s�܂Ƃ߂ĕ`��ς�
				// �s�ԍ��i�ύX�s�j�\���͉��s�P�ʂ̍s������X�V����K�v������	// 2009.03.26 ryoji
				if (bLineModifiedChange) {	// ���ύX�������s���ύX���ꂽ
					const Layout* pcLayoutWk = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( LRArg.nModLineFrom );
					if (pcLayoutWk && pcLayoutWk->GetLogicOffset()) {	// �܂�Ԃ����C�A�E�g�s���H
						Call_OnPaint(PAINT_LINENUMBER, false);
					}
				}
#endif
			}
		}
	}

	// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
	if (pcMemDeleted && 0 < pcMemDeleted->size()) {
		if (pcmemCopyOfDeleted) {
			if (pcOpeBlk) {
				pcReplaceOpe->m_pcmemDataDel = *pcMemDeleted;
			}
			pcmemCopyOfDeleted->swap(*pcMemDeleted);
		}else if (pcOpeBlk) {
			pcReplaceOpe->m_pcmemDataDel.swap(*pcMemDeleted);
		}
	}

	if (pcOpeBlk) {
		if (bFastMode) {
			pcReplaceOpe->m_ptCaretPos_PHY_After = DLRArg.ptNewPos;
			pcReplaceOpe->m_nOrgInsSeq = DLRArg.nInsSeq;
		}else {
			layoutMgr.LayoutToLogic(LRArg.ptLayoutNew,   &pcReplaceOpe->m_ptCaretPos_PHY_After);
			pcReplaceOpe->m_nOrgInsSeq = LRArg.nInsSeq;
		}
		// ����̒ǉ�
		pcOpeBlk->AppendOpe(pcReplaceOpe);
	}

	// �}������ʒu�փJ�[�\�����ړ�
	if (bFastMode) {
		GetCaret().MoveCursorFastMode(DLRArg.ptNewPos);
	}else {
		GetCaret().MoveCursor(
			LRArg.ptLayoutNew,	// �}�����ꂽ�����̎��̈ʒu
			bRedraw
		);
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();
	}

// 2013.06.29 MoveCaretOpe�͕s�v�BReplaceOpe�݂̂ɂ���
	if (pnInsSeq) {
		*pnInsSeq = bFastMode ? DLRArg.nInsSeq : LRArg.nInsSeq;
	}

	//	Jan. 30, 2001 genta
	//	�t�@�C���S�̂̍X�V�t���O�������Ă��Ȃ��Ɗe�s�̍X�V��Ԃ��\������Ȃ��̂�
	//	�t���O�X�V�������ĕ`����O�Ɉړ�����
	return  bUpdateAll;
}


// 2005.10.11 ryoji �O�̍s�ɂ��閖���̋󔒂��폜
void EditView::RTrimPrevLine(void)
{
	LogicPoint ptCaretPos_PHY = GetCaret().GetCaretLogicPos();

	if (GetCaret().GetCaretLogicPos().y > 0) {
		int nLineLen;
		const wchar_t*	pLine = DocReader(m_pcEditDoc->m_cDocLineMgr).GetLineStrWithoutEOL(GetCaret().GetCaretLogicPos().GetY2() - LogicInt(1), &nLineLen);
		if (pLine && nLineLen > 0) {
			int i = 0;
			int j = 0;
			while (i < nLineLen) {
				int nCharChars = NativeW::GetSizeOfChar(pLine, nLineLen, i);
				if (!WCODE::IsBlank(pLine[i])) {
					j = i + nCharChars;
				}
				i += nCharChars;
			}
			auto& layoutMgr = m_pcEditDoc->m_cLayoutMgr;
			if (j < nLineLen) {
				LayoutRange sRangeA;
				layoutMgr.LogicToLayout(LogicPoint(j, GetCaret().GetCaretLogicPos().y - 1), sRangeA.GetFromPointer());
				layoutMgr.LogicToLayout(LogicPoint(nLineLen, GetCaret().GetCaretLogicPos().y - 1), sRangeA.GetToPointer());
				if (!(sRangeA.GetFrom().x >= sRangeA.GetTo().x && sRangeA.GetFrom().y == sRangeA.GetTo().y)) {
					ReplaceData_CEditView(
						sRangeA,
						NULL,		// �}������f�[�^
						LogicInt(0),			// �}������f�[�^�̒���
						true,
						m_bDoing_UndoRedo ? NULL : m_cCommander.GetOpeBlk()
					);
					LayoutPoint ptCP;
					layoutMgr.LogicToLayout(ptCaretPos_PHY, &ptCP);
					GetCaret().MoveCursor(ptCP, true);

					if (!m_bDoing_UndoRedo) {	// �A���h�D�E���h�D�̎��s����
						// ����̒ǉ�
						m_cCommander.GetOpeBlk()->AppendOpe(
							new MoveCaretOpe(
								GetCaret().GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
							)
						);
					}
				}
			}
		}
	}
}


