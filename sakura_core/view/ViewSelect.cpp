#include "StdAfx.h"
#include <limits.h>
#include "ViewSelect.h"
#include "EditView.h"
#include "doc/EditDoc.h"
#include "doc/layout/Layout.h"
#include "mem/MemoryIterator.h"
#include "window/EditWnd.h"
#include "charset/CodeBase.h"
#include "charset/CodeFactory.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "types/TypeSupport.h"

ViewSelect::ViewSelect(EditView* pEditView)
	:
	m_pEditView(pEditView)
{
	m_bSelectingLock   = false;	// �I����Ԃ̃��b�N
	m_bBeginSelect     = false;		// �͈͑I��
	m_bBeginBoxSelect  = false;	// ��`�͈͑I��
	m_bBeginLineSelect = false;	// �s�P�ʑI��
	m_bBeginWordSelect = false;	// �P��P�ʑI��

	m_selectBgn.Clear(-1); // �͈͑I��(���_)
	m_select.Clear(-1); // �͈͑I��
	m_selectOld.Clear(0);  // �͈͑I��(Old)
	m_bSelectAreaChanging = false;	// �I��͈͕ύX��
	m_nLastSelectedByteLen = 0;	// �O��I�����̑I���o�C�g��
}

void ViewSelect::CopySelectStatus(ViewSelect* pSelect) const
{
	pSelect->m_bSelectingLock		= m_bSelectingLock;		// �I����Ԃ̃��b�N
	pSelect->m_bBeginSelect			= m_bBeginSelect;		// �͈͑I��
	pSelect->m_bBeginBoxSelect		= m_bBeginBoxSelect;	// ��`�͈͑I��

	pSelect->m_selectBgn			= m_selectBgn;			// �͈͑I��(���_)
	pSelect->m_select				= m_select;				// �͈͑I��
	pSelect->m_selectOld			= m_selectOld;			// �͈͑I��

	pSelect->m_ptMouseRollPosOld	= m_ptMouseRollPosOld;	// �}�E�X�͈͑I��O��ʒu(XY���W)
}

// ���݂̃J�[�\���ʒu����I�����J�n����
void ViewSelect::BeginSelectArea(const LayoutPoint* po)
{
	const EditView* pView = GetEditView();
	LayoutPoint temp;
	if (!po) {
		temp = pView->GetCaret().GetCaretLayoutPos();
		po = &temp;
	}
	m_selectBgn.Set(*po); // �͈͑I��(���_)
	m_select.   Set(*po); // �͈͑I��
}


// ���݂̑I��͈͂��I����Ԃɖ߂�
void ViewSelect::DisableSelectArea(bool bDraw, bool bDrawBracketCursorLine)
{
	const EditView* pView = GetEditView();
	EditView* pView2 = GetEditView();

	m_selectOld = m_select;		// �͈͑I��(Old)
	m_select.Clear(-1);
	m_bSelectingLock	 = false;	// �I����Ԃ̃��b�N

	if (bDraw) {
		DrawSelectArea(bDrawBracketCursorLine);
	}
	m_bDrawSelectArea = false;	// 02/12/13 ai // 2011.12.24 bDraw���ʓ�����ړ�

	m_selectOld.Clear(0);			// �͈͑I��(Old)
	m_bBeginBoxSelect = false;		// ��`�͈͑I��
	m_bBeginLineSelect = false;		// �s�P�ʑI��
	m_bBeginWordSelect = false;		// �P��P�ʑI��
	m_nLastSelectedByteLen = 0;		// �O��I�����̑I���o�C�g��

	// 2002.02.16 hor ���O�̃J�[�\���ʒu�����Z�b�g
	pView2->GetCaret().m_nCaretPosX_Prev = pView->GetCaret().GetCaretLayoutPos().GetX();

}


// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
void ViewSelect::ChangeSelectAreaByCurrentCursor(const LayoutPoint& ptCaretPos)
{
	m_selectOld = m_select; // �͈͑I��(Old)

	//	2002/04/08 YAZAKI �R�[�h�̏d����r��
	ChangeSelectAreaByCurrentCursorTEST(
		ptCaretPos,
		&m_select
	);

	// �I��̈�̕`��
	m_bSelectAreaChanging = true;
	DrawSelectArea(true);
	m_bSelectAreaChanging = false;
}


// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX(�e�X�g�̂�)
void ViewSelect::ChangeSelectAreaByCurrentCursorTEST(
	const LayoutPoint& ptCaretPos,
	LayoutRange* pSelect
)
{
	if (m_selectBgn.GetFrom() == m_selectBgn.GetTo()) {
		if (ptCaretPos == m_selectBgn.GetFrom()) {
			// �I������
			pSelect->Clear(-1);
			m_nLastSelectedByteLen = 0;		// �O��I�����̑I���o�C�g��
		}else if (PointCompare(ptCaretPos, m_selectBgn.GetFrom()) < 0) { // �L�����b�g�ʒu��m_sSelectBgn��from��菬����������
			 pSelect->SetFrom(ptCaretPos);
			 pSelect->SetTo(m_selectBgn.GetFrom());
		}else {
			pSelect->SetFrom(m_selectBgn.GetFrom());
			pSelect->SetTo(ptCaretPos);
		}
	}else {
		// �펞�I��͈͈͓͂̔�
		// �L�����b�g�ʒu�� m_sSelectBgn �� from�ȏ�ŁAto��菬�����ꍇ
		if (PointCompare(ptCaretPos, m_selectBgn.GetFrom()) >= 0 && PointCompare(ptCaretPos, m_selectBgn.GetTo()) < 0) {
			pSelect->SetFrom(m_selectBgn.GetFrom());
			if (ptCaretPos == m_selectBgn.GetFrom()) {
				pSelect->SetTo(m_selectBgn.GetTo());
			}else {
				pSelect->SetTo(ptCaretPos);
			}
		// �L�����b�g�ʒu��m_sSelectBgn��from��菬����������
		}else if (PointCompare(ptCaretPos, m_selectBgn.GetFrom()) < 0) {
			// �펞�I��͈͂̑O����
			pSelect->SetFrom(ptCaretPos);
			pSelect->SetTo(m_selectBgn.GetTo());
		}else {
			// �펞�I��͈͂̌�����
			pSelect->SetFrom(m_selectBgn.GetFrom());
			pSelect->SetTo(ptCaretPos);
		}
	}
}


/*! �I��̈�̕`��

	@date 2006.10.01 Moca �d���R�[�h�폜�D��`�����P�D
	@date 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
		��ʃo�b�t�@���L�����A��ʂƌ݊�BMP�̗����̔��]�������s���B
*/
void ViewSelect::DrawSelectArea(bool bDrawBracketCursorLine)
{
	EditView* pView = GetEditView();

	if (!pView->GetDrawSwitch()) {
		return;
	}
	m_bDrawSelectArea = true;
	
	bool bDispText = TypeSupport(pView, COLORIDX_SELECT).IsDisp();
	if (bDispText) {
		if (m_select != m_selectOld) {
			// �I��F�\���̎��́AWM_PAINT�o�R�ō��
			const int nCharWidth = pView->GetTextMetrics().GetHankakuDx();
			const TextArea& area =  pView->GetTextArea();
			LayoutRect rcOld; // LayoutRect
			TwoPointToRect(&rcOld, m_selectOld.GetFrom(), m_selectOld.GetTo());
			LayoutRect rcNew; // LayoutRect
			TwoPointToRect(&rcNew, m_select.GetFrom(), m_select.GetTo());
			LayoutRect rc; // LayoutRect ������top,bottom�����g��
			LayoutInt drawLeft = LayoutInt(0);
			LayoutInt drawRight = LayoutInt(-1);
			if (!m_select.IsValid()) {
				rc.top    = rcOld.top;
				rc.bottom = rcOld.bottom;
			}else if (!m_selectOld.IsValid()) {
				rc.top    = rcNew.top;
				rc.bottom = rcNew.bottom;
			}else if (1
				&& IsBoxSelecting()
				&& (
					m_select.GetTo().x != m_selectOld.GetTo().x
					|| m_select.GetFrom().x != m_selectOld.GetFrom().x
				)
			) {
				rc.UnionStrictRect(rcOld, rcNew);
			}else if (!IsBoxSelecting() && rcOld.top == rcNew.top && rcOld.bottom == rcNew.bottom) {
				if (m_select.GetFrom() == m_selectOld.GetFrom() && m_select.GetTo().x != m_selectOld.GetTo().x) {
					// GetTo�̍s���Ώ�
					rc.top = rc.bottom = m_select.GetTo().GetY2();
					drawLeft  = t_min(m_select.GetTo().x, m_selectOld.GetTo().x);
					drawRight = t_max(m_select.GetTo().x, m_selectOld.GetTo().x) + 1;
				}else if (m_select.GetTo() == m_selectOld.GetTo() && m_select.GetFrom().x != m_selectOld.GetFrom().x) {
					// GetFrom�̍s���Ώ�
					rc.top = rc.bottom = m_select.GetFrom().GetY2();
					drawLeft  = t_min(m_selectOld.GetFrom().x, m_select.GetFrom().x);
					drawRight = t_max(m_selectOld.GetFrom().x, m_select.GetFrom().x) + 1;
				}else {
					rc.UnionStrictRect(rcOld, rcNew);
				}
			}else if (rcOld.top == rcNew.top) {
				rc.top    = t_min(rcOld.bottom, rcNew.bottom);
				rc.bottom = t_max(rcOld.bottom, rcNew.bottom);
			}else if (rcOld.bottom == rcNew.bottom) {
				rc.top    = t_min(rcOld.top, rcNew.top);
				rc.bottom = t_max(rcOld.top, rcNew.top);
			}else {
				rc.UnionStrictRect(rcOld, rcNew);
			}
			Rect rcPx;
			if (pView->IsBkBitmap() || drawRight == -1) {
				// �w�i�\���̃N���b�s���O���Â��̂ō��E���w�肵�Ȃ�
				rcPx.left   =  0;
				rcPx.right  = SHRT_MAX; 
			}else {
				rcPx.left   =  area.GetAreaLeft() + nCharWidth * (Int)(drawLeft - area.GetViewLeftCol());
				rcPx.right  = area.GetAreaLeft() + nCharWidth * (Int)(drawRight- area.GetViewLeftCol());
			}
			rcPx.top    = area.GenerateYPx(rc.top);
			rcPx.bottom = area.GenerateYPx(rc.bottom + 1);

			Rect rcArea;
			pView->GetTextArea().GenerateTextAreaRect(&rcArea);
			RECT rcUpdate;
			EditView& view = *pView;
			if (::IntersectRect(&rcUpdate, &rcPx, &rcArea)) {
				HDC hdc = view.GetDC();
				PAINTSTRUCT ps;
				ps.rcPaint = rcUpdate;
				// DrawSelectAreaLine�ł̉���OFF�̑���
				view.GetCaret().m_underLine.CaretUnderLineOFF(true, false);
				view.GetCaret().m_underLine.Lock();
				view.OnPaint(hdc, &ps, false);
				view.GetCaret().m_underLine.UnLock();
				view.ReleaseDC(hdc);
			}
			// 2010.10.10 0���I��(����)��Ԃł́A�J�[�\���ʒu���C�����A(���[�W�����O)
			if (bDrawBracketCursorLine) {
				view.GetCaret().m_underLine.CaretUnderLineON(true, false);
			}
		}
	}else {
		if (IsTextSelecting() && (!m_selectOld.IsValid() || m_selectOld.IsOne())) {
			m_bDrawSelectArea = false;
			pView->DrawBracketPair( false );
			m_bDrawSelectArea = true;
		}
		HDC hdc = pView->GetDC();
		DrawSelectArea2(hdc);
		// 2011.12.02 �I��������Ԃł́A�J�[�\���ʒu���C�����A
		if (bDrawBracketCursorLine) {
			pView->GetCaret().m_underLine.CaretUnderLineON(true, false);
		}
		pView->ReleaseDC(hdc);
	}

	// 2011.12.02 �I��������ԂɂȂ�ƑΊ��ʋ������ł��Ȃ��Ȃ�o�O�΍�
	if (!IsTextSelecting()) {
		// �������I�����b�N���͂����ł͋����\������Ȃ�
		m_bDrawSelectArea = false;
		if (bDrawBracketCursorLine) {
			pView->SetBracketPairPos(true);
			pView->DrawBracketPair(true);
		}
	}

	//	Jul. 9, 2005 genta �I��̈�̏���\��
	PrintSelectionInfoMsg();
}

/*!
	���]�p�č�揈���{��
*/
void ViewSelect::DrawSelectArea2(HDC hdc) const
{
	EditView const * const pView = GetEditView();

	// 2006.10.01 Moca �d���R�[�h����
	HBRUSH	hBrush = ::CreateSolidBrush(SELECTEDAREA_RGB);
	HBRUSH	hBrushOld = (HBRUSH)::SelectObject(hdc, hBrush);
	int		nROP_Old = ::SetROP2(hdc, SELECTEDAREA_ROP2);
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	HBRUSH	hBrushCompatOld = 0;
	int		nROPCompatOld = 0;
	bool bCompatBMP = pView->m_hbmpCompatBMP && hdc != pView->m_hdcCompatDC;
	if (bCompatBMP) {
		hBrushCompatOld = (HBRUSH)::SelectObject(pView->m_hdcCompatDC, hBrush);
		nROPCompatOld = ::SetROP2(pView->m_hdcCompatDC, SELECTEDAREA_ROP2);
	}
	// To Here 2007.09.09 Moca

//	MYTRACE(_T("DrawSelectArea()  m_bBeginBoxSelect=%hs\n", m_bBeginBoxSelect?"true":"false"));
	auto& textArea = pView->GetTextArea();
	if (IsBoxSelecting()) {		// ��`�͈͑I��
		// 2001.12.21 hor ��`�G���A��EOF������ꍇ�ARGN_XOR�Ō��������
		// EOF�ȍ~�̃G���A�����]���Ă��܂��̂ŁA���̏ꍇ��Redraw���g��
		// 2002.02.16 hor �������}�~���邽��EOF�ȍ~�̃G���A�����]�����������x���]���Č��ɖ߂����Ƃɂ���
		//if ((GetTextArea().GetViewTopLine()+m_nViewRowNum+1 >= m_pEditDoc->m_layoutMgr.GetLineCount()) &&
		//   (m_select.GetTo().y+1 >= m_pEditDoc->m_layoutMgr.GetLineCount() ||
		//	m_selectOld.GetTo().y+1 >= m_pEditDoc->m_layoutMgr.GetLineCount())) {
		//	Redraw();
		//	return;
		//}

		const int nCharWidth = pView->GetTextMetrics().GetHankakuDx();
		const int nCharHeight = pView->GetTextMetrics().GetHankakuDy();

		// 2�_��Ίp�Ƃ����`�����߂�
		LayoutRect  rcOld;
		TwoPointToRect(
			&rcOld,
			m_selectOld.GetFrom(),	// �͈͑I���J�n
			m_selectOld.GetTo()	// �͈͑I���I��
		);
		rcOld.left   = t_max(rcOld.left  , textArea.GetViewLeftCol() );
		rcOld.right  = t_max(rcOld.right , textArea.GetViewLeftCol() );
		rcOld.right  = t_min(rcOld.right , textArea.GetRightCol() + 1);
		rcOld.top    = t_max(rcOld.top   , textArea.GetViewTopLine() );
		rcOld.bottom = t_max(rcOld.bottom, textArea.GetViewTopLine() - 1);	// 2010.11.02 ryoji �ǉ��i��ʏ�[������ɂ����`�I������������ƃ��[���[�����]�\���ɂȂ���̏C���j
		rcOld.bottom = t_min(rcOld.bottom, textArea.GetBottomLine()  );

		RECT rcOld2;
		rcOld2.left		= (textArea.GetAreaLeft() - (Int)textArea.GetViewLeftCol() * nCharWidth) + (Int)rcOld.left  * nCharWidth;
		rcOld2.right	= (textArea.GetAreaLeft() - (Int)textArea.GetViewLeftCol() * nCharWidth) + (Int)rcOld.right * nCharWidth;
		rcOld2.top		= textArea.GenerateYPx(rcOld.top);
		rcOld2.bottom	= textArea.GenerateYPx(rcOld.bottom + 1);
		HRGN hrgnOld = ::CreateRectRgnIndirect(&rcOld2);

		// 2�_��Ίp�Ƃ����`�����߂�
		LayoutRect  rcNew;
		TwoPointToRect(
			&rcNew,
			m_select.GetFrom(),	// �͈͑I���J�n
			m_select.GetTo()		// �͈͑I���I��
		);
		rcNew.left   = t_max(rcNew.left  , textArea.GetViewLeftCol());
		rcNew.right  = t_max(rcNew.right , textArea.GetViewLeftCol());
		rcNew.right  = t_min(rcNew.right , textArea.GetRightCol() + 1);
		rcNew.top    = t_max(rcNew.top   , textArea.GetViewTopLine());
		rcNew.bottom = t_max(rcNew.bottom, textArea.GetViewTopLine() - 1);	// 2010.11.02 ryoji �ǉ��i��ʏ�[������ɂ����`�I������������ƃ��[���[�����]�\���ɂȂ���̏C���j
		rcNew.bottom = t_min(rcNew.bottom, textArea.GetBottomLine() );

		RECT rcNew2;
		rcNew2.left		= (textArea.GetAreaLeft() - (Int)textArea.GetViewLeftCol() * nCharWidth) + (Int)rcNew.left  * nCharWidth;
		rcNew2.right	= (textArea.GetAreaLeft() - (Int)textArea.GetViewLeftCol() * nCharWidth) + (Int)rcNew.right * nCharWidth;
		rcNew2.top		= textArea.GenerateYPx(rcNew.top);
		rcNew2.bottom	= textArea.GenerateYPx(rcNew.bottom + 1);

		HRGN hrgnNew = ::CreateRectRgnIndirect(&rcNew2);

		// ��`���B
		// ::CombineRgn()�̌��ʂ��󂯎�邽�߂ɁA�K���ȃ��[�W���������
		HRGN hrgnDraw = ::CreateRectRgnIndirect(&rcNew2);
		{
			// ���I����`�ƐV�I����`�̃��[�W������������� �d�Ȃ肠�������������������܂�
			if (::CombineRgn(hrgnDraw, hrgnOld, hrgnNew, RGN_XOR) != NULLREGION) {

				// 2002.02.16 hor
				// ������̃G���A��EOF���܂܂��ꍇ��EOF�ȍ~�̕������������܂�
				// 2006.10.01 Moca ���[�\�[�X���[�N���C��������A�`�����悤�ɂȂ������߁A
				// �}���邽�߂� EOF�ȍ~�����[�W��������폜����1�x�̍��ɂ���

				// 2006.10.01 Moca Start EOF�ʒu�v�Z��GetEndLayoutPos�ɏ��������B
				LayoutPoint ptLast;
				pView->m_pEditDoc->m_layoutMgr.GetEndLayoutPos(&ptLast);
				// 2006.10.01 Moca End
				// 2011.12.26 EOF�̂Ԃ牺����s�͔��]���AEOF�݂̂̍s�͔��]���Ȃ�
				const Layout* pBottom = pView->m_pEditDoc->m_layoutMgr.GetBottomLayout();
				if (pBottom && pBottom->GetLayoutEol() == EolType::None) {
					ptLast.x = 0;
					ptLast.y++;
				}
				if (0
					|| m_select.GetFrom().y >= ptLast.y
					|| m_select.GetTo().y >= ptLast.y
					|| m_selectOld.GetFrom().y >= ptLast.y
					|| m_selectOld.GetTo().y >= ptLast.y
				) {
					//	Jan. 24, 2004 genta nLastLen�͕������Ȃ̂ŕϊ��K�v
					//	�ŏI�s��TAB�������Ă���Ɣ��]�͈͂��s������D
					//	2006.10.01 Moca GetEndLayoutPos�ŏ������邽��ColumnToIndex�͕s�v�ɁB
					RECT rcNew;
					rcNew.left   = textArea.GetAreaLeft() + (Int)(textArea.GetViewLeftCol() + ptLast.x) * nCharWidth;
					rcNew.right  = textArea.GetAreaRight();
					rcNew.top    = textArea.GenerateYPx(ptLast.y);
					rcNew.bottom = rcNew.top + nCharHeight;
					
					// 2006.10.01 Moca GDI(���[�W����)���\�[�X���[�N�C��
					HRGN hrgnEOFNew = ::CreateRectRgnIndirect(&rcNew);
					::CombineRgn(hrgnDraw, hrgnDraw, hrgnEOFNew, RGN_DIFF);
					::DeleteObject(hrgnEOFNew);
				}
				::PaintRgn(hdc, hrgnDraw);
				// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
				if (bCompatBMP) {
					::PaintRgn(pView->m_hdcCompatDC, hrgnDraw);
				}
				// To Here 2007.09.09 Moca
			}
		}

		//////////////////////////////////////////
		// �f�o�b�O�p ���[�W������`�̃_���v
//@@		TraceRgn(hrgnDraw);


		if (hrgnDraw) {
			::DeleteObject(hrgnDraw);
		}
		if (hrgnNew) {
			::DeleteObject(hrgnNew);
		}
		if (hrgnOld) {
			::DeleteObject(hrgnOld);
		}
	}else {
		LayoutRange rangeA;
		LayoutInt nLineNum;

		// ���ݕ`�悳��Ă���͈͂Ǝn�_������
		if (m_select.GetFrom() == m_selectOld.GetFrom()) {
			// �͈͂�����Ɋg�傳�ꂽ
			if (PointCompare(m_select.GetTo(), m_selectOld.GetTo()) > 0) {
				rangeA.SetFrom(m_selectOld.GetTo());
				rangeA.SetTo  (m_select.GetTo());
			}else {
				rangeA.SetFrom(m_select.GetTo());
				rangeA.SetTo  (m_selectOld.GetTo());
			}
			for (nLineNum=rangeA.GetFrom().GetY2(); nLineNum<=rangeA.GetTo().GetY2(); ++nLineNum) {
				if (nLineNum >= textArea.GetViewTopLine() && nLineNum <= textArea.GetBottomLine() + 1) {
					DrawSelectAreaLine(	hdc, nLineNum, rangeA);
				}
			}
		}else if (m_select.GetTo() == m_selectOld.GetTo()) {
			// �͈͂��O���Ɋg�傳�ꂽ
			if (PointCompare(m_select.GetFrom(), m_selectOld.GetFrom()) < 0) {
				rangeA.SetFrom(m_select.GetFrom());
				rangeA.SetTo  (m_selectOld.GetFrom());
			}else {
				rangeA.SetFrom(m_selectOld.GetFrom());
				rangeA.SetTo  (m_select.GetFrom());
			}
			for (nLineNum=rangeA.GetFrom().GetY2(); nLineNum<=rangeA.GetTo().GetY2(); ++nLineNum) {
				if (nLineNum >= textArea.GetViewTopLine() && nLineNum <= textArea.GetBottomLine() + 1) {
					DrawSelectAreaLine(hdc, nLineNum, rangeA);
				}
			}
		}else {
			rangeA = m_selectOld;
			for (nLineNum=rangeA.GetFrom().GetY2(); nLineNum<=rangeA.GetTo().GetY2(); ++nLineNum) {
				if (nLineNum >= textArea.GetViewTopLine() && nLineNum <= textArea.GetBottomLine() + 1) {
					DrawSelectAreaLine(hdc, nLineNum, rangeA);
				}
			}
			rangeA = m_select;
			for (nLineNum=rangeA.GetFrom().GetY2(); nLineNum<=rangeA.GetTo().GetY2(); ++nLineNum) {
				if (nLineNum >= textArea.GetViewTopLine() && nLineNum <= textArea.GetBottomLine() + 1) {
					DrawSelectAreaLine(hdc, nLineNum, rangeA);
				}
			}
		}
	}

	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	if (bCompatBMP) {
		::SetROP2(pView->m_hdcCompatDC, nROPCompatOld);
		::SelectObject(pView->m_hdcCompatDC, hBrushCompatOld);
	}
	// To Here 2007.09.09 Moca

	// 2006.10.01 Moca �d���R�[�h����
	::SetROP2(hdc, nROP_Old);
	::SelectObject(hdc, hBrushOld);
	::DeleteObject(hBrush);
}


/*! �I��̈�̒��̎w��s�̕`��

	�����s�ɓn��I��͈͂̂����CnLineNum�Ŏw�肳�ꂽ1�s��������`�悷��D
	�I��͈͂͌Œ肳�ꂽ�܂�nLineNum�݂̂��K�v�s���ω����Ȃ���Ăт������D

	@date 2006.03.29 Moca 3000��������P�p�D
*/
void ViewSelect::DrawSelectAreaLine(
	HDC					hdc,		// [in] �`��̈��Device Context Handle
	LayoutInt			nLineNum,	// [in] �`��Ώۍs(���C�A�E�g�s)
	const LayoutRange&	range		// [in] �I��͈�(���C�A�E�g�P��)
	) const
{
	EditView const * const pView = m_pEditView;
	bool bCompatBMP = pView->m_hbmpCompatBMP && hdc != pView->m_hdcCompatDC;

	const LayoutMgr& layoutMgr = pView->m_pEditDoc->m_layoutMgr;
	const Layout* pLayout = layoutMgr.SearchLineByLayoutY(nLineNum);
	LayoutRange lineArea;
	GetSelectAreaLineFromRange(lineArea, nLineNum, pLayout, range);
	LayoutInt nSelectFrom = lineArea.GetFrom().GetX2();
	LayoutInt nSelectTo = lineArea.GetTo().GetX2();
	auto& textArea = pView->GetTextArea();
	if (nSelectFrom == INT_MAX || nSelectTo == INT_MAX) {
		LayoutInt nPosX = LayoutInt(0);
		MemoryIterator it = MemoryIterator(pLayout, layoutMgr.GetTabSpace());
		
		while (!it.end()) {
			it.scanNext();
			if (it.getIndex() + it.getIndexDelta() > pLayout->GetLengthWithoutEOL()) {
				nPosX ++;
				break;
			}
			// 2006.03.28 Moca ��ʊO�܂ŋ��߂���ł��؂�
			if (it.getColumn() > textArea.GetRightCol()) {
				break;
			}
			it.addDelta();
		}
		nPosX += it.getColumn();

		if (nSelectFrom == INT_MAX) {
			nSelectFrom = nPosX;
		}
		if (nSelectTo == INT_MAX) {
			nSelectTo = nPosX;
		}
	}
	
	// 2006.03.28 Moca �E�B���h�E�����傫���Ɛ��������]���Ȃ������C��
	if (nSelectFrom < textArea.GetViewLeftCol()) {
		nSelectFrom = textArea.GetViewLeftCol();
	}
	int		nLineHeight = pView->GetTextMetrics().GetHankakuDy();
	int		nCharWidth = pView->GetTextMetrics().GetHankakuDx();
	Rect	rcClip; // px
	rcClip.left		= (textArea.GetAreaLeft() - (Int)textArea.GetViewLeftCol() * nCharWidth) + (Int)nSelectFrom * nCharWidth;
	rcClip.right	= (textArea.GetAreaLeft() - (Int)textArea.GetViewLeftCol() * nCharWidth) + (Int)nSelectTo   * nCharWidth;
	rcClip.top		= textArea.GenerateYPx(nLineNum);
	rcClip.bottom	= rcClip.top + nLineHeight;
	if (rcClip.right > textArea.GetAreaRight()) {
		rcClip.right = textArea.GetAreaRight();
	}
	//	�K�v�ȂƂ������B
	if (rcClip.right != rcClip.left) {
		LayoutRange selectOld = m_select;
		const_cast<LayoutRange*>(&m_select)->Clear(-1);
		pView->GetCaret().m_underLine.CaretUnderLineOFF(true, false, true);
		*(const_cast<LayoutRange*>(&m_select)) = selectOld;
		
		// 2006.03.28 Moca �\������̂ݏ�������
		if (nSelectFrom <= textArea.GetRightCol() && textArea.GetViewLeftCol() < nSelectTo) {
			HRGN hrgnDraw = ::CreateRectRgn(rcClip.left, rcClip.top, rcClip.right, rcClip.bottom);
			::PaintRgn(hdc, hrgnDraw);
			// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
			if (bCompatBMP) {
				::PaintRgn(pView->m_hdcCompatDC, hrgnDraw);
			}
			// To Here 2007.09.09 Moca
			::DeleteObject(hrgnDraw);
		}
	}
}

void ViewSelect::GetSelectAreaLineFromRange(
	LayoutRange& ret,
	LayoutInt nLineNum,
	const Layout* pLayout,
	const LayoutRange&	range
	) const
{
	const EditView& view = *GetEditView();
	if (nLineNum >= range.GetFrom().y && nLineNum <= range.GetTo().y ||
		nLineNum >= range.GetTo().y && nLineNum <= range.GetFrom().y
	) {
		LayoutInt	nSelectFrom = range.GetFrom().GetX2();
		LayoutInt	nSelectTo   = range.GetTo().GetX2();
		if (IsBoxSelecting()) {		// ��`�͈͑I��
			nSelectFrom = range.GetFrom().GetX2();
			nSelectTo   = range.GetTo().GetX2();
			// 2006.09.30 Moca From ��`�I����[EOF]�Ƃ��̉E���͔��]���Ȃ��悤�ɏC���B������ǉ�
			// 2011.12.26 [EOF]�P�ƍs�ȊO�Ȃ甽�]����
			if (view.m_pEditDoc->m_layoutMgr.GetLineCount() <= nLineNum) {
				nSelectFrom = -1;
				nSelectTo = -1;
			}
			// 2006.09.30 Moca To
		}else {
			if (range.IsLineOne()) {
				nSelectFrom = range.GetFrom().GetX2();
				nSelectTo   = range.GetTo().GetX2();
			}else {
				LayoutInt nX_Layout = LayoutInt(INT_MAX);
				if (nLineNum == range.GetFrom().y) {
					nSelectFrom = range.GetFrom().GetX2();
					nSelectTo   = nX_Layout;
				}else if (nLineNum == range.GetTo().GetY2()) {
					nSelectFrom = pLayout ? pLayout->GetIndent() : LayoutInt(0);
					nSelectTo   = range.GetTo().GetX2();
				}else {
					nSelectFrom = pLayout ? pLayout->GetIndent() : LayoutInt(0);
					nSelectTo   = nX_Layout;
				}
			}
		}
		// 2006.05.24 Moca ��`�I��/�t���[�J�[�\���I��(�I���J�n/�I���s)��
		// To < From �ɂȂ邱�Ƃ�����B�K�� From < To �ɂȂ�悤�ɓ���ւ���B
		if (nSelectTo < nSelectFrom) {
			t_swap(nSelectFrom, nSelectTo);
		}
		ret.SetFrom(LayoutPoint(nSelectFrom, nLineNum));
		ret.SetTo(LayoutPoint(nSelectTo, nLineNum));
	}else {
		ret.SetFrom(LayoutPoint(-1, -1));
		ret.SetTo(LayoutPoint(-1, -1));
	}
}

/*!	�I��͈͏�񃁃b�Z�[�W�̕\��

	@author genta
	@date 2005.07.09 genta �V�K�쐬
	@date 2006.06.06 ryoji �I��͈͂̍s�����݂��Ȃ��ꍇ�̑΍��ǉ�
	@date 2006.06.28 syat �o�C�g���J�E���g��ǉ�
*/
void ViewSelect::PrintSelectionInfoMsg() const
{
	const EditView* pView = GetEditView();

	//	�o�͂���Ȃ��Ȃ�v�Z���ȗ�
	if (!pView->m_pEditWnd->m_statusBar.SendStatusMessage2IsEffective())
		return;

	LayoutInt nLineCount = pView->m_pEditDoc->m_layoutMgr.GetLineCount();
	if (!IsTextSelected() || m_select.GetFrom().y >= nLineCount) { // �擪�s�����݂��Ȃ�
		const_cast<EditView*>(pView)->GetCaret().m_bClearStatus = false;
		if (IsBoxSelecting()) {
			pView->m_pEditWnd->m_statusBar.SendStatusMessage2(_T("box selecting"));
		}else if (m_bSelectingLock) {
			pView->m_pEditWnd->m_statusBar.SendStatusMessage2(_T("selecting"));
		}else {
			pView->m_pEditWnd->m_statusBar.SendStatusMessage2(_T(""));
		}
		return;
	}

	TCHAR msg[128];
	//	From here 2006.06.06 ryoji �I��͈͂̍s�����݂��Ȃ��ꍇ�̑΍�

	LayoutInt select_line;
	if (m_select.GetTo().y >= nLineCount) {	// �ŏI�s�����݂��Ȃ�
		select_line = nLineCount - m_select.GetFrom().y + 1;
	}else {
		select_line = m_select.GetTo().y - m_select.GetFrom().y + 1;
	}
	
	//	To here 2006.06.06 ryoji �I��͈͂̍s�����݂��Ȃ��ꍇ�̑΍�
	if (IsBoxSelecting()) {
		//	��`�̏ꍇ�͕��ƍ��������ł��܂���
		LayoutInt select_col = m_select.GetFrom().x - m_select.GetTo().x;
		if (select_col < 0) {
			select_col = -select_col;
		}
		auto_sprintf_s(msg, _T("%d Columns * %d lines selected."),
			select_col, select_line);
			
	}else {
		//	�ʏ�̑I���ł͑I��͈͂̒��g�𐔂���
		int select_sum = 0;	//	�o�C�g�����v
		const wchar_t* pLine;	//	�f�[�^���󂯎��
		LogicInt	nLineLen;		//	�s�̒���
		const Layout*	pLayout;
		ViewSelect* thiz = const_cast<ViewSelect*>(this);	// const�O��this

		// ���ʐݒ�E�I�𕶎����𕶎��P�ʂł͂Ȃ��o�C�g�P�ʂŕ\������
		bool bCountByByteCommon = GetDllShareData().m_common.statusBar.m_bDispSelCountByByte;
		bool bCountByByte = (pView->m_pEditWnd->m_nSelectCountMode == SelectCountMode::Toggle ?
								bCountByByteCommon :
								pView->m_pEditWnd->m_nSelectCountMode == SelectCountMode::ByByte);

		//	1�s��
		pLine = pView->m_pEditDoc->m_layoutMgr.GetLineStr(m_select.GetFrom().GetY2(), &nLineLen, &pLayout);
		if (pLine) {
			if (bCountByByte) {
				//  �o�C�g���ŃJ�E���g
				//  ���������R�[�h���猻�݂̕����R�[�h�ɕϊ����A�o�C�g�����擾����B
				//  �R�[�h�ϊ��͕��ׂ������邽�߁A�I��͈͂̑������݂̂�ΏۂƂ���B

				NativeW memW;
				Memory memCode;

				// ������������̎擾��EditView::GetSelectedData���g���������Am_select����̂��߁A
				// �Ăяo���O��m_select������������B�ďo����Ɍ��ɖ߂��̂ŁAconst�ƌ����Ȃ����Ƃ��Ȃ��B
				LayoutRange rngSelect = m_select;		// �I��̈�̑ޔ�
				bool bSelExtend;						// �I��̈�g��t���O

				// �ŏI�s�̏���
				pLine = pView->m_pEditDoc->m_layoutMgr.GetLineStr(m_select.GetTo().y, &nLineLen, &pLayout);
				if (pLine) {
					if (pView->LineColumnToIndex(pLayout, m_select.GetTo().GetX2()) == 0) {
						//	�ŏI�s�̐擪�ɃL�����b�g������ꍇ��
						//	���̍s���s���Ɋ܂߂Ȃ�
						--select_line;
					}
				}else {
					//	�ŏI�s����s�Ȃ�
					//	���̍s���s���Ɋ܂߂Ȃ�
					--select_line;
				}

				// 2009.07.07 syat m_nLastSelectedByteLen��0�̏ꍇ�́A�����ł͂Ȃ��S�̂�ϊ�����i���[�h�ؑ֎��ɃL���b�V���N���A���邽�߁j

				if (m_bSelectAreaChanging && m_nLastSelectedByteLen && m_select.GetFrom() == m_selectOld.GetFrom()) {
					// �͈͂�����Ɋg�傳�ꂽ
					if (PointCompare(m_select.GetTo(), m_selectOld.GetTo()) < 0) {
						bSelExtend = false;				// �k��
						thiz->m_select = LayoutRange(m_select.GetTo(), m_selectOld.GetTo());
					}else {
						bSelExtend = true;				// �g��
						thiz->m_select = LayoutRange(m_selectOld.GetTo(), m_select.GetTo());
					}

					const_cast<EditView*>(pView)->GetSelectedDataSimple(memW);
					thiz->m_select = rngSelect;		// m_select�����ɖ߂�
				}else if (
					m_bSelectAreaChanging
					&& m_nLastSelectedByteLen
					&& m_select.GetTo() == m_selectOld.GetTo()
				) {
					// �͈͂��O���Ɋg�傳�ꂽ
					if (PointCompare(m_select.GetFrom(), m_selectOld.GetFrom()) < 0) {
						bSelExtend = true;				// �g��
						thiz->m_select = LayoutRange(m_select.GetFrom(), m_selectOld.GetFrom());
					}else {
						bSelExtend = false;				// �k��
						thiz->m_select = LayoutRange(m_selectOld.GetFrom(), m_select.GetFrom());
					}

					const_cast<EditView*>(pView)->GetSelectedDataSimple(memW);
					thiz->m_select = rngSelect;		// m_select�����ɖ߂�
				}else {
					// �I��̈�S�̂��R�[�h�ϊ��Ώۂɂ���
					const_cast<EditView*>(pView)->GetSelectedDataSimple(memW);
					bSelExtend = true;
					thiz->m_nLastSelectedByteLen = 0;
				}
				//  ���݂̕����R�[�h�ɕϊ����A�o�C�g�����擾����
				CodeBase* pCode = CodeFactory::CreateCodeBase(pView->m_pEditDoc->GetDocumentEncoding(), false);
				pCode->UnicodeToCode(memW, &memCode);
				delete pCode;

				if (bSelExtend) {
					select_sum = m_nLastSelectedByteLen + memCode.GetRawLength();
				}else {
					select_sum = m_nLastSelectedByteLen - memCode.GetRawLength();
				}
				thiz->m_nLastSelectedByteLen = select_sum;

			}else {
				//  �������ŃJ�E���g

				// 2009.07.07 syat �J�E���g���@��؂�ւ��Ȃ���I��͈͂��g��E�k������Ɛ�������
				//                �Ƃ�Ȃ��Ȃ邽�߁A���[�h�ؑ֎��ɃL���b�V�����N���A����B
				thiz->m_nLastSelectedByteLen = 0;

				//	1�s�����I������Ă���ꍇ
				if (m_select.IsLineOne()) {
					select_sum =
						pView->LineColumnToIndex(pLayout, m_select.GetTo().GetX2())
						- pView->LineColumnToIndex(pLayout, m_select.GetFrom().GetX2());
				}else {	//	2�s�ȏ�I������Ă���ꍇ
					select_sum =
						pLayout->GetLengthWithoutEOL()
						+ pLayout->GetLayoutEol().GetLen()
						- pView->LineColumnToIndex(pLayout, m_select.GetFrom().GetX2());

					//	GetSelectedData�Ǝ��Ă��邪�C�擪�s�ƍŏI�s�͔r�����Ă���
					//	Aug. 16, 2005 aroka nLineNum��for�ȍ~�ł��g����̂�for�̑O�Ő錾����
					//	VC .NET�ȍ~�ł�Microsoft�g����L���ɂ����W�������VC6�Ɠ������Ƃɒ���
					LayoutInt nLineNum;
					for (nLineNum = m_select.GetFrom().GetY2() + LayoutInt(1);
						nLineNum < m_select.GetTo().GetY2();
						++nLineNum
					) {
						pLine = pView->m_pEditDoc->m_layoutMgr.GetLineStr(nLineNum, &nLineLen, &pLayout);
						//	2006.06.06 ryoji �w��s�̃f�[�^�����݂��Ȃ��ꍇ�̑΍�
						if (!pLine)
							break;
						select_sum += pLayout->GetLengthWithoutEOL() + pLayout->GetLayoutEol().GetLen();
					}

					//	�ŏI�s�̏���
					pLine = pView->m_pEditDoc->m_layoutMgr.GetLineStr(nLineNum, &nLineLen, &pLayout);
					if (pLine) {
						int last_line_chars = pView->LineColumnToIndex(pLayout, m_select.GetTo().GetX2());
						select_sum += last_line_chars;
						if (last_line_chars == 0) {
							//	�ŏI�s�̐擪�ɃL�����b�g������ꍇ��
							//	���̍s���s���Ɋ܂߂Ȃ�
							--select_line;
						}
					}else {
						//	�ŏI�s����s�Ȃ�
						//	���̍s���s���Ɋ܂߂Ȃ�
						--select_line;
					}
				}
			}
		}

#ifdef _DEBUG
		auto_sprintf_s(
			msg, _T("%d %ts (%d lines) selected. [%d:%d]-[%d:%d]"),
			select_sum,
			(bCountByByte ? _T("bytes") : _T("chars")),
			select_line,
			m_select.GetFrom().x, m_select.GetFrom().y,
			m_select.GetTo().x, m_select.GetTo().y
		);
#else
		auto_sprintf_s(
			msg, _T("%d %ts (%d lines) selected."),
			select_sum,
			(bCountByByte ? _T("bytes") : _T("chars")),
			select_line
		);
#endif
	}
	const_cast<EditView*>(pView)->GetCaret().m_bClearStatus = false;
	pView->m_pEditWnd->m_statusBar.SendStatusMessage2(msg);
}

