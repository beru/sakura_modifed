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

ViewSelect::ViewSelect(EditView& editView)
	:
	editView(editView)
{
	bSelectingLock   = false;	// �I����Ԃ̃��b�N
	bBeginSelect     = false;		// �͈͑I��
	bBeginBoxSelect  = false;	// ��`�͈͑I��
	bBeginLineSelect = false;	// �s�P�ʑI��
	bBeginWordSelect = false;	// �P��P�ʑI��

	selectBgn.Clear(-1); // �͈͑I��(���_)
	select.Clear(-1); // �͈͑I��
	selectOld.Clear(0);  // �͈͑I��(Old)
	bSelectAreaChanging = false;	// �I��͈͕ύX��
	nLastSelectedByteLen = 0;	// �O��I�����̑I���o�C�g��
}

void ViewSelect::CopySelectStatus(ViewSelect* pSelect) const
{
	pSelect->bSelectingLock		= bSelectingLock;		// �I����Ԃ̃��b�N
	pSelect->bBeginSelect		= bBeginSelect;		// �͈͑I��
	pSelect->bBeginBoxSelect	= bBeginBoxSelect;	// ��`�͈͑I��

	pSelect->selectBgn			= selectBgn;			// �͈͑I��(���_)
	pSelect->select			= select;				// �͈͑I��
	pSelect->selectOld		= selectOld;			// �͈͑I��

	pSelect->ptMouseRollPosOld	= ptMouseRollPosOld;	// �}�E�X�͈͑I��O��ʒu(XY���W)
}

// ���݂̃J�[�\���ʒu����I�����J�n����
void ViewSelect::BeginSelectArea(const Point* po)
{
	const EditView& view = GetEditView();
	Point temp;
	if (!po) {
		temp = view.GetCaret().GetCaretLayoutPos();
		po = &temp;
	}
	selectBgn.Set(*po); // �͈͑I��(���_)
	select.   Set(*po); // �͈͑I��
}


// ���݂̑I��͈͂��I����Ԃɖ߂�
void ViewSelect::DisableSelectArea(bool bDraw, bool bDrawBracketCursorLine)
{
	const EditView& view = GetEditView();
	EditView& view2 = GetEditView();

	selectOld = select;		// �͈͑I��(Old)
	select.Clear(-1);
	bSelectingLock	 = false;	// �I����Ԃ̃��b�N

	if (bDraw) {
		DrawSelectArea(bDrawBracketCursorLine);
	}
	bDrawSelectArea = false;	// 02/12/13 ai // 2011.12.24 bDraw���ʓ�����ړ�

	selectOld.Clear(0);			// �͈͑I��(Old)
	bBeginBoxSelect = false;		// ��`�͈͑I��
	bBeginLineSelect = false;		// �s�P�ʑI��
	bBeginWordSelect = false;		// �P��P�ʑI��
	nLastSelectedByteLen = 0;		// �O��I�����̑I���o�C�g��

	// 2002.02.16 hor ���O�̃J�[�\���ʒu�����Z�b�g
	view2.GetCaret().nCaretPosX_Prev = view.GetCaret().GetCaretLayoutPos().GetX();

}


// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
void ViewSelect::ChangeSelectAreaByCurrentCursor(const Point& ptCaretPos)
{
	selectOld = select; // �͈͑I��(Old)

	//	2002/04/08 YAZAKI �R�[�h�̏d����r��
	ChangeSelectAreaByCurrentCursorTEST(
		ptCaretPos,
		&select
	);

	// �I��̈�̕`��
	bSelectAreaChanging = true;
	DrawSelectArea(true);
	bSelectAreaChanging = false;
}


// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX(�e�X�g�̂�)
void ViewSelect::ChangeSelectAreaByCurrentCursorTEST(
	const Point& ptCaretPos,
	Range* pSelect
)
{
	if (selectBgn.GetFrom() == selectBgn.GetTo()) {
		if (ptCaretPos == selectBgn.GetFrom()) {
			// �I������
			pSelect->Clear(-1);
			nLastSelectedByteLen = 0;		// �O��I�����̑I���o�C�g��
		}else if (PointCompare(ptCaretPos, selectBgn.GetFrom()) < 0) { // �L�����b�g�ʒu��sSelectBgn��from��菬����������
			 pSelect->SetFrom(ptCaretPos);
			 pSelect->SetTo(selectBgn.GetFrom());
		}else {
			pSelect->SetFrom(selectBgn.GetFrom());
			pSelect->SetTo(ptCaretPos);
		}
	}else {
		// �펞�I��͈͈͓͂̔�
		// �L�����b�g�ʒu�� sSelectBgn �� from�ȏ�ŁAto��菬�����ꍇ
		if (PointCompare(ptCaretPos, selectBgn.GetFrom()) >= 0 && PointCompare(ptCaretPos, selectBgn.GetTo()) < 0) {
			pSelect->SetFrom(selectBgn.GetFrom());
			if (ptCaretPos == selectBgn.GetFrom()) {
				pSelect->SetTo(selectBgn.GetTo());
			}else {
				pSelect->SetTo(ptCaretPos);
			}
		// �L�����b�g�ʒu��sSelectBgn��from��菬����������
		}else if (PointCompare(ptCaretPos, selectBgn.GetFrom()) < 0) {
			// �펞�I��͈͂̑O����
			pSelect->SetFrom(ptCaretPos);
			pSelect->SetTo(selectBgn.GetTo());
		}else {
			// �펞�I��͈͂̌�����
			pSelect->SetFrom(selectBgn.GetFrom());
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
	EditView& view = GetEditView();

	if (!view.GetDrawSwitch()) {
		return;
	}
	bDrawSelectArea = true;
	
	bool bDispText = TypeSupport(view, COLORIDX_SELECT).IsDisp();
	if (bDispText) {
		if (select != selectOld) {
			// �I��F�\���̎��́AWM_PAINT�o�R�ō��
			const size_t nCharWidth = view.GetTextMetrics().GetHankakuDx();
			const TextArea& area =  view.GetTextArea();
			Rect rcOld; // LayoutRect
			TwoPointToRect(&rcOld, selectOld.GetFrom(), selectOld.GetTo());
			Rect rcNew; // LayoutRect
			TwoPointToRect(&rcNew, select.GetFrom(), select.GetTo());
			Rect rc; // LayoutRect ������top,bottom�����g��
			int drawLeft = 0;
			int drawRight = -1;
			if (!select.IsValid()) {
				rc.top    = rcOld.top;
				rc.bottom = rcOld.bottom;
			}else if (!selectOld.IsValid()) {
				rc.top    = rcNew.top;
				rc.bottom = rcNew.bottom;
			}else if (1
				&& IsBoxSelecting()
				&& (
					select.GetTo().x != selectOld.GetTo().x
					|| select.GetFrom().x != selectOld.GetFrom().x
				)
			) {
				rc.Union(rcOld, rcNew);
			}else if (!IsBoxSelecting() && rcOld.top == rcNew.top && rcOld.bottom == rcNew.bottom) {
				if (select.GetFrom() == selectOld.GetFrom() && select.GetTo().x != selectOld.GetTo().x) {
					// GetTo�̍s���Ώ�
					rc.top = rc.bottom = select.GetTo().y;
					drawLeft  = t_min(select.GetTo().x, selectOld.GetTo().x);
					drawRight = t_max(select.GetTo().x, selectOld.GetTo().x) + 1;
				}else if (select.GetTo() == selectOld.GetTo() && select.GetFrom().x != selectOld.GetFrom().x) {
					// GetFrom�̍s���Ώ�
					rc.top = rc.bottom = select.GetFrom().y;
					drawLeft  = t_min(selectOld.GetFrom().x, select.GetFrom().x);
					drawRight = t_max(selectOld.GetFrom().x, select.GetFrom().x) + 1;
				}else {
					rc.Union(rcOld, rcNew);
				}
			}else if (rcOld.top == rcNew.top) {
				rc.top    = t_min(rcOld.bottom, rcNew.bottom);
				rc.bottom = t_max(rcOld.bottom, rcNew.bottom);
			}else if (rcOld.bottom == rcNew.bottom) {
				rc.top    = t_min(rcOld.top, rcNew.top);
				rc.bottom = t_max(rcOld.top, rcNew.top);
			}else {
				rc.Union(rcOld, rcNew);
			}
			Rect rcPx;
			if (view.IsBkBitmap() || drawRight == -1) {
				// �w�i�\���̃N���b�s���O���Â��̂ō��E���w�肵�Ȃ�
				rcPx.left   =  0;
				rcPx.right  = SHRT_MAX; 
			}else {
				rcPx.left   =  area.GetAreaLeft() + nCharWidth * (drawLeft - area.GetViewLeftCol());
				rcPx.right  = area.GetAreaLeft() + nCharWidth * (drawRight- area.GetViewLeftCol());
			}
			rcPx.top    = area.GenerateYPx(rc.top);
			rcPx.bottom = area.GenerateYPx(rc.bottom + 1);

			Rect rcArea;
			view.GetTextArea().GenerateTextAreaRect(&rcArea);
			RECT rcUpdate;
			if (::IntersectRect(&rcUpdate, &rcPx, &rcArea)) {
				HDC hdc = view.GetDC();
				PAINTSTRUCT ps;
				ps.rcPaint = rcUpdate;
				// DrawSelectAreaLine�ł̉���OFF�̑���
				view.GetCaret().underLine.CaretUnderLineOFF(true, false);
				view.GetCaret().underLine.Lock();
				view.OnPaint(hdc, &ps, false);
				view.GetCaret().underLine.UnLock();
				view.ReleaseDC(hdc);
			}
			// 2010.10.10 0���I��(����)��Ԃł́A�J�[�\���ʒu���C�����A(���[�W�����O)
			if (bDrawBracketCursorLine) {
				view.GetCaret().underLine.CaretUnderLineON(true, false);
			}
		}
	}else {
		if (IsTextSelecting() && (!selectOld.IsValid() || selectOld.IsOne())) {
			bDrawSelectArea = false;
			view.DrawBracketPair( false );
			bDrawSelectArea = true;
		}
		HDC hdc = view.GetDC();
		DrawSelectArea2(hdc);
		// 2011.12.02 �I��������Ԃł́A�J�[�\���ʒu���C�����A
		if (bDrawBracketCursorLine) {
			view.GetCaret().underLine.CaretUnderLineON(true, false);
		}
		view.ReleaseDC(hdc);
	}

	// 2011.12.02 �I��������ԂɂȂ�ƑΊ��ʋ������ł��Ȃ��Ȃ�o�O�΍�
	if (!IsTextSelecting()) {
		// �������I�����b�N���͂����ł͋����\������Ȃ�
		bDrawSelectArea = false;
		if (bDrawBracketCursorLine) {
			view.SetBracketPairPos(true);
			view.DrawBracketPair(true);
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
	auto& view = GetEditView();

	// 2006.10.01 Moca �d���R�[�h����
	HBRUSH	hBrush = ::CreateSolidBrush(SELECTEDAREA_RGB);
	HBRUSH	hBrushOld = (HBRUSH)::SelectObject(hdc, hBrush);
	int		nROP_Old = ::SetROP2(hdc, SELECTEDAREA_ROP2);
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	HBRUSH	hBrushCompatOld = 0;
	int		nROPCompatOld = 0;
	bool bCompatBMP = view.hbmpCompatBMP && hdc != view.hdcCompatDC;
	if (bCompatBMP) {
		hBrushCompatOld = (HBRUSH)::SelectObject(view.hdcCompatDC, hBrush);
		nROPCompatOld = ::SetROP2(view.hdcCompatDC, SELECTEDAREA_ROP2);
	}
	// To Here 2007.09.09 Moca

//	MYTRACE(_T("DrawSelectArea()  bBeginBoxSelect=%hs\n", bBeginBoxSelect?"true":"false"));
	auto& textArea = view.GetTextArea();
	if (IsBoxSelecting()) {		// ��`�͈͑I��
		// 2001.12.21 hor ��`�G���A��EOF������ꍇ�ARGN_XOR�Ō��������
		// EOF�ȍ~�̃G���A�����]���Ă��܂��̂ŁA���̏ꍇ��Redraw���g��
		// 2002.02.16 hor �������}�~���邽��EOF�ȍ~�̃G���A�����]�����������x���]���Č��ɖ߂����Ƃɂ���
		//if ((GetTextArea().GetViewTopLine()+nViewRowNum+1 >= pEditDoc->layoutMgr.GetLineCount()) &&
		//   (select.GetTo().y+1 >= pEditDoc->layoutMgr.GetLineCount() ||
		//	selectOld.GetTo().y+1 >= pEditDoc->layoutMgr.GetLineCount())) {
		//	Redraw();
		//	return;
		//}

		const size_t nCharWidth = view.GetTextMetrics().GetHankakuDx();
		const size_t nCharHeight = view.GetTextMetrics().GetHankakuDy();

		// 2�_��Ίp�Ƃ����`�����߂�
		Rect rcOld;
		TwoPointToRect(
			&rcOld,
			selectOld.GetFrom(),	// �͈͑I���J�n
			selectOld.GetTo()	// �͈͑I���I��
		);
		rcOld.left   = t_max((int)rcOld.left  , (int)textArea.GetViewLeftCol() );
		rcOld.right  = t_max((int)rcOld.right , (int)textArea.GetViewLeftCol() );
		rcOld.right  = t_min((int)rcOld.right , (int)textArea.GetRightCol() + 1);
		rcOld.top    = t_max((int)rcOld.top   , (int)textArea.GetViewTopLine() );
		rcOld.bottom = t_max((int)rcOld.bottom, (int)textArea.GetViewTopLine() - 1);	// 2010.11.02 ryoji �ǉ��i��ʏ�[������ɂ����`�I������������ƃ��[���[�����]�\���ɂȂ���̏C���j
		rcOld.bottom = t_min((int)rcOld.bottom, (int)textArea.GetBottomLine()  );

		RECT rcOld2;
		rcOld2.left		= (textArea.GetAreaLeft() - textArea.GetViewLeftCol() * nCharWidth) + rcOld.left  * nCharWidth;
		rcOld2.right	= (textArea.GetAreaLeft() - textArea.GetViewLeftCol() * nCharWidth) + rcOld.right * nCharWidth;
		rcOld2.top		= textArea.GenerateYPx(rcOld.top);
		rcOld2.bottom	= textArea.GenerateYPx(rcOld.bottom + 1);
		HRGN hrgnOld = ::CreateRectRgnIndirect(&rcOld2);

		// 2�_��Ίp�Ƃ����`�����߂�
		Rect rcNew;
		TwoPointToRect(
			&rcNew,
			select.GetFrom(),	// �͈͑I���J�n
			select.GetTo()		// �͈͑I���I��
		);
		rcNew.left   = t_max((int)rcNew.left  , (int)textArea.GetViewLeftCol());
		rcNew.right  = t_max((int)rcNew.right , (int)textArea.GetViewLeftCol());
		rcNew.right  = t_min((int)rcNew.right , (int)textArea.GetRightCol() + 1);
		rcNew.top    = t_max((int)rcNew.top   , (int)textArea.GetViewTopLine());
		rcNew.bottom = t_max((int)rcNew.bottom, (int)textArea.GetViewTopLine() - 1);	// 2010.11.02 ryoji �ǉ��i��ʏ�[������ɂ����`�I������������ƃ��[���[�����]�\���ɂȂ���̏C���j
		rcNew.bottom = t_min((int)rcNew.bottom, (int)textArea.GetBottomLine() );

		RECT rcNew2;
		rcNew2.left		= (textArea.GetAreaLeft() - textArea.GetViewLeftCol() * nCharWidth) + rcNew.left  * nCharWidth;
		rcNew2.right	= (textArea.GetAreaLeft() - textArea.GetViewLeftCol() * nCharWidth) + rcNew.right * nCharWidth;
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
				Point ptLast;
				view.pEditDoc->layoutMgr.GetEndLayoutPos(&ptLast);
				// 2006.10.01 Moca End
				// 2011.12.26 EOF�̂Ԃ牺����s�͔��]���AEOF�݂̂̍s�͔��]���Ȃ�
				const Layout* pBottom = view.pEditDoc->layoutMgr.GetBottomLayout();
				if (pBottom && pBottom->GetLayoutEol() == EolType::None) {
					ptLast.x = 0;
					ptLast.y++;
				}
				if (0
					|| select.GetFrom().y >= ptLast.y
					|| select.GetTo().y >= ptLast.y
					|| selectOld.GetFrom().y >= ptLast.y
					|| selectOld.GetTo().y >= ptLast.y
				) {
					//	Jan. 24, 2004 genta nLastLen�͕������Ȃ̂ŕϊ��K�v
					//	�ŏI�s��TAB�������Ă���Ɣ��]�͈͂��s������D
					//	2006.10.01 Moca GetEndLayoutPos�ŏ������邽��ColumnToIndex�͕s�v�ɁB
					RECT rcNew;
					rcNew.left   = textArea.GetAreaLeft() + (textArea.GetViewLeftCol() + ptLast.x) * nCharWidth;
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
					::PaintRgn(view.hdcCompatDC, hrgnDraw);
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
		Range rangeA;
		int nLineNum;

		// ���ݕ`�悳��Ă���͈͂Ǝn�_������
		if (select.GetFrom() == selectOld.GetFrom()) {
			// �͈͂�����Ɋg�傳�ꂽ
			if (PointCompare(select.GetTo(), selectOld.GetTo()) > 0) {
				rangeA.SetFrom(selectOld.GetTo());
				rangeA.SetTo  (select.GetTo());
			}else {
				rangeA.SetFrom(select.GetTo());
				rangeA.SetTo  (selectOld.GetTo());
			}
			for (nLineNum=rangeA.GetFrom().y; nLineNum<=rangeA.GetTo().y; ++nLineNum) {
				if (nLineNum >= textArea.GetViewTopLine() && nLineNum <= textArea.GetBottomLine() + 1) {
					DrawSelectAreaLine(	hdc, nLineNum, rangeA);
				}
			}
		}else if (select.GetTo() == selectOld.GetTo()) {
			// �͈͂��O���Ɋg�傳�ꂽ
			if (PointCompare(select.GetFrom(), selectOld.GetFrom()) < 0) {
				rangeA.SetFrom(select.GetFrom());
				rangeA.SetTo  (selectOld.GetFrom());
			}else {
				rangeA.SetFrom(selectOld.GetFrom());
				rangeA.SetTo  (select.GetFrom());
			}
			for (nLineNum=rangeA.GetFrom().y; nLineNum<=rangeA.GetTo().y; ++nLineNum) {
				if (nLineNum >= textArea.GetViewTopLine() && nLineNum <= textArea.GetBottomLine() + 1) {
					DrawSelectAreaLine(hdc, nLineNum, rangeA);
				}
			}
		}else {
			rangeA = selectOld;
			for (nLineNum=rangeA.GetFrom().y; nLineNum<=rangeA.GetTo().y; ++nLineNum) {
				if (nLineNum >= textArea.GetViewTopLine() && nLineNum <= textArea.GetBottomLine() + 1) {
					DrawSelectAreaLine(hdc, nLineNum, rangeA);
				}
			}
			rangeA = select;
			for (nLineNum=rangeA.GetFrom().y; nLineNum<=rangeA.GetTo().y; ++nLineNum) {
				if (nLineNum >= textArea.GetViewTopLine() && nLineNum <= textArea.GetBottomLine() + 1) {
					DrawSelectAreaLine(hdc, nLineNum, rangeA);
				}
			}
		}
	}

	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	if (bCompatBMP) {
		::SetROP2(view.hdcCompatDC, nROPCompatOld);
		::SelectObject(view.hdcCompatDC, hBrushCompatOld);
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
	HDC hdc,			// [in] �`��̈��Device Context Handle
	int nLineNum,		// [in] �`��Ώۍs(���C�A�E�g�s)
	const Range& range	// [in] �I��͈�(���C�A�E�g�P��)
	) const
{
	auto& view = editView;
	bool bCompatBMP = view.hbmpCompatBMP && hdc != view.hdcCompatDC;

	auto& layoutMgr = view.pEditDoc->layoutMgr;
	const Layout* pLayout = layoutMgr.SearchLineByLayoutY(nLineNum);
	Range lineArea;
	GetSelectAreaLineFromRange(lineArea, nLineNum, pLayout, range);
	int nSelectFrom = lineArea.GetFrom().x;
	int nSelectTo = lineArea.GetTo().x;
	auto& textArea = view.GetTextArea();
	if (nSelectFrom == INT_MAX || nSelectTo == INT_MAX) {
		int nPosX = 0;
		MemoryIterator it = MemoryIterator(pLayout, layoutMgr.GetTabSpace());
		
		while (!it.end()) {
			it.scanNext();
			if (it.getIndex() + it.getIndexDelta() > pLayout->GetLengthWithoutEOL()) {
				nPosX ++;
				break;
			}
			// 2006.03.28 Moca ��ʊO�܂ŋ��߂���ł��؂�
			if ((int)it.getColumn() > textArea.GetRightCol()) {
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
	size_t nLineHeight = view.GetTextMetrics().GetHankakuDy();
	size_t nCharWidth = view.GetTextMetrics().GetHankakuDx();
	Rect	rcClip; // px
	rcClip.left		= (textArea.GetAreaLeft() - textArea.GetViewLeftCol() * nCharWidth) + nSelectFrom * nCharWidth;
	rcClip.right	= (textArea.GetAreaLeft() - textArea.GetViewLeftCol() * nCharWidth) + nSelectTo   * nCharWidth;
	rcClip.top		= textArea.GenerateYPx(nLineNum);
	rcClip.bottom	= rcClip.top + nLineHeight;
	if (rcClip.right > textArea.GetAreaRight()) {
		rcClip.right = textArea.GetAreaRight();
	}
	//	�K�v�ȂƂ������B
	if (rcClip.right != rcClip.left) {
		Range selectOld = select;
		const_cast<Range*>(&select)->Clear(-1);
		view.GetCaret().underLine.CaretUnderLineOFF(true, false, true);
		*(const_cast<Range*>(&select)) = selectOld;
		
		// 2006.03.28 Moca �\������̂ݏ�������
		if (nSelectFrom <= textArea.GetRightCol() && textArea.GetViewLeftCol() < nSelectTo) {
			HRGN hrgnDraw = ::CreateRectRgn(rcClip.left, rcClip.top, rcClip.right, rcClip.bottom);
			::PaintRgn(hdc, hrgnDraw);
			// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
			if (bCompatBMP) {
				::PaintRgn(view.hdcCompatDC, hrgnDraw);
			}
			// To Here 2007.09.09 Moca
			::DeleteObject(hrgnDraw);
		}
	}
}

void ViewSelect::GetSelectAreaLineFromRange(
	Range& ret,
	int nLineNum,
	const Layout* pLayout,
	const Range& range
	) const
{
	const EditView& view = GetEditView();
	if (nLineNum >= range.GetFrom().y && nLineNum <= range.GetTo().y ||
		nLineNum >= range.GetTo().y && nLineNum <= range.GetFrom().y
	) {
		int nSelectFrom = range.GetFrom().x;
		int nSelectTo   = range.GetTo().x;
		if (IsBoxSelecting()) {		// ��`�͈͑I��
			nSelectFrom = range.GetFrom().x;
			nSelectTo   = range.GetTo().x;
			// 2006.09.30 Moca From ��`�I����[EOF]�Ƃ��̉E���͔��]���Ȃ��悤�ɏC���B������ǉ�
			// 2011.12.26 [EOF]�P�ƍs�ȊO�Ȃ甽�]����
			if ((int)view.pEditDoc->layoutMgr.GetLineCount() <= nLineNum) {
				nSelectFrom = -1;
				nSelectTo = -1;
			}
			// 2006.09.30 Moca To
		}else {
			if (range.IsLineOne()) {
				nSelectFrom = range.GetFrom().x;
				nSelectTo   = range.GetTo().x;
			}else {
				int nX_Layout = INT_MAX;
				if (nLineNum == range.GetFrom().y) {
					nSelectFrom = range.GetFrom().x;
					nSelectTo   = nX_Layout;
				}else if (nLineNum == range.GetTo().y) {
					nSelectFrom = pLayout ? pLayout->GetIndent() : 0;
					nSelectTo   = range.GetTo().x;
				}else {
					nSelectFrom = pLayout ? pLayout->GetIndent() : 0;
					nSelectTo   = nX_Layout;
				}
			}
		}
		// 2006.05.24 Moca ��`�I��/�t���[�J�[�\���I��(�I���J�n/�I���s)��
		// To < From �ɂȂ邱�Ƃ�����B�K�� From < To �ɂȂ�悤�ɓ���ւ���B
		if (nSelectTo < nSelectFrom) {
			t_swap(nSelectFrom, nSelectTo);
		}
		ret.SetFrom(Point(nSelectFrom, nLineNum));
		ret.SetTo(Point(nSelectTo, nLineNum));
	}else {
		ret.SetFrom(Point(-1, -1));
		ret.SetTo(Point(-1, -1));
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
	auto& view = GetEditView();

	//	�o�͂���Ȃ��Ȃ�v�Z���ȗ�
	if (!view.editWnd.statusBar.SendStatusMessage2IsEffective())
		return;

	int nLineCount = view.pEditDoc->layoutMgr.GetLineCount();
	if (!IsTextSelected() || select.GetFrom().y >= nLineCount) { // �擪�s�����݂��Ȃ�
		const_cast<EditView&>(view).GetCaret().bClearStatus = false;
		if (IsBoxSelecting()) {
			view.editWnd.statusBar.SendStatusMessage2(_T("box selecting"));
		}else if (bSelectingLock) {
			view.editWnd.statusBar.SendStatusMessage2(_T("selecting"));
		}else {
			view.editWnd.statusBar.SendStatusMessage2(_T(""));
		}
		return;
	}

	TCHAR msg[128];
	//	From here 2006.06.06 ryoji �I��͈͂̍s�����݂��Ȃ��ꍇ�̑΍�

	int select_line;
	if (select.GetTo().y >= nLineCount) {	// �ŏI�s�����݂��Ȃ�
		select_line = nLineCount - select.GetFrom().y + 1;
	}else {
		select_line = select.GetTo().y - select.GetFrom().y + 1;
	}
	
	//	To here 2006.06.06 ryoji �I��͈͂̍s�����݂��Ȃ��ꍇ�̑΍�
	if (IsBoxSelecting()) {
		//	��`�̏ꍇ�͕��ƍ��������ł��܂���
		int select_col = select.GetFrom().x - select.GetTo().x;
		if (select_col < 0) {
			select_col = -select_col;
		}
		auto_sprintf_s(msg, _T("%d Columns * %d lines selected."),
			select_col, select_line);
			
	}else {
		//	�ʏ�̑I���ł͑I��͈͂̒��g�𐔂���
		size_t select_sum = 0;	//	�o�C�g�����v
		const wchar_t* pLine;	//	�f�[�^���󂯎��
		size_t nLineLen;		//	�s�̒���
		const Layout*	pLayout;
		ViewSelect* thiz = const_cast<ViewSelect*>(this);	// const�O��this

		// ���ʐݒ�E�I�𕶎����𕶎��P�ʂł͂Ȃ��o�C�g�P�ʂŕ\������
		bool bCountByByteCommon = GetDllShareData().common.statusBar.bDispSelCountByByte;
		bool bCountByByte = (view.editWnd.nSelectCountMode == SelectCountMode::Toggle ?
								bCountByByteCommon :
								view.editWnd.nSelectCountMode == SelectCountMode::ByByte);

		//	1�s��
		pLine = view.pEditDoc->layoutMgr.GetLineStr(select.GetFrom().y, &nLineLen, &pLayout);
		if (pLine) {
			if (bCountByByte) {
				//  �o�C�g���ŃJ�E���g
				//  ���������R�[�h���猻�݂̕����R�[�h�ɕϊ����A�o�C�g�����擾����B
				//  �R�[�h�ϊ��͕��ׂ������邽�߁A�I��͈͂̑������݂̂�ΏۂƂ���B

				NativeW memW;
				Memory memCode;

				// ������������̎擾��EditView::GetSelectedData���g���������Aselect����̂��߁A
				// �Ăяo���O��select������������B�ďo����Ɍ��ɖ߂��̂ŁAconst�ƌ����Ȃ����Ƃ��Ȃ��B
				Range rngSelect = select;		// �I��̈�̑ޔ�
				bool bSelExtend;						// �I��̈�g��t���O

				// �ŏI�s�̏���
				pLine = view.pEditDoc->layoutMgr.GetLineStr(select.GetTo().y, &nLineLen, &pLayout);
				if (pLine) {
					if (view.LineColumnToIndex(pLayout, select.GetTo().x) == 0) {
						//	�ŏI�s�̐擪�ɃL�����b�g������ꍇ��
						//	���̍s���s���Ɋ܂߂Ȃ�
						--select_line;
					}
				}else {
					//	�ŏI�s����s�Ȃ�
					//	���̍s���s���Ɋ܂߂Ȃ�
					--select_line;
				}

				// 2009.07.07 syat nLastSelectedByteLen��0�̏ꍇ�́A�����ł͂Ȃ��S�̂�ϊ�����i���[�h�ؑ֎��ɃL���b�V���N���A���邽�߁j

				if (bSelectAreaChanging && nLastSelectedByteLen && select.GetFrom() == selectOld.GetFrom()) {
					// �͈͂�����Ɋg�傳�ꂽ
					if (PointCompare(select.GetTo(), selectOld.GetTo()) < 0) {
						bSelExtend = false;				// �k��
						thiz->select = Range(select.GetTo(), selectOld.GetTo());
					}else {
						bSelExtend = true;				// �g��
						thiz->select = Range(selectOld.GetTo(), select.GetTo());
					}

					const_cast<EditView&>(view).GetSelectedDataSimple(memW);
					thiz->select = rngSelect;		// select�����ɖ߂�
				}else if (
					bSelectAreaChanging
					&& nLastSelectedByteLen
					&& select.GetTo() == selectOld.GetTo()
				) {
					// �͈͂��O���Ɋg�傳�ꂽ
					if (PointCompare(select.GetFrom(), selectOld.GetFrom()) < 0) {
						bSelExtend = true;				// �g��
						thiz->select = Range(select.GetFrom(), selectOld.GetFrom());
					}else {
						bSelExtend = false;				// �k��
						thiz->select = Range(selectOld.GetFrom(), select.GetFrom());
					}

					const_cast<EditView&>(view).GetSelectedDataSimple(memW);
					thiz->select = rngSelect;		// select�����ɖ߂�
				}else {
					// �I��̈�S�̂��R�[�h�ϊ��Ώۂɂ���
					const_cast<EditView&>(view).GetSelectedDataSimple(memW);
					bSelExtend = true;
					thiz->nLastSelectedByteLen = 0;
				}
				//  ���݂̕����R�[�h�ɕϊ����A�o�C�g�����擾����
				CodeBase* pCode = CodeFactory::CreateCodeBase(view.pEditDoc->GetDocumentEncoding(), false);
				pCode->UnicodeToCode(memW, &memCode);
				delete pCode;

				if (bSelExtend) {
					select_sum = nLastSelectedByteLen + memCode.GetRawLength();
				}else {
					select_sum = nLastSelectedByteLen - memCode.GetRawLength();
				}
				thiz->nLastSelectedByteLen = select_sum;

			}else {
				//  �������ŃJ�E���g

				// 2009.07.07 syat �J�E���g���@��؂�ւ��Ȃ���I��͈͂��g��E�k������Ɛ�������
				//                �Ƃ�Ȃ��Ȃ邽�߁A���[�h�ؑ֎��ɃL���b�V�����N���A����B
				thiz->nLastSelectedByteLen = 0;

				//	1�s�����I������Ă���ꍇ
				if (select.IsLineOne()) {
					select_sum =
						view.LineColumnToIndex(pLayout, select.GetTo().x)
						- view.LineColumnToIndex(pLayout, select.GetFrom().x);
				}else {	//	2�s�ȏ�I������Ă���ꍇ
					select_sum =
						pLayout->GetLengthWithoutEOL()
						+ pLayout->GetLayoutEol().GetLen()
						- view.LineColumnToIndex(pLayout, select.GetFrom().x);

					//	GetSelectedData�Ǝ��Ă��邪�C�擪�s�ƍŏI�s�͔r�����Ă���
					//	Aug. 16, 2005 aroka nLineNum��for�ȍ~�ł��g����̂�for�̑O�Ő錾����
					//	VC .NET�ȍ~�ł�Microsoft�g����L���ɂ����W�������VC6�Ɠ������Ƃɒ���
					int nLineNum;
					for (nLineNum = select.GetFrom().y + 1;
						nLineNum < select.GetTo().y;
						++nLineNum
					) {
						pLine = view.pEditDoc->layoutMgr.GetLineStr(nLineNum, &nLineLen, &pLayout);
						//	2006.06.06 ryoji �w��s�̃f�[�^�����݂��Ȃ��ꍇ�̑΍�
						if (!pLine)
							break;
						select_sum += pLayout->GetLengthWithoutEOL() + pLayout->GetLayoutEol().GetLen();
					}

					//	�ŏI�s�̏���
					pLine = view.pEditDoc->layoutMgr.GetLineStr(nLineNum, &nLineLen, &pLayout);
					if (pLine) {
						int last_line_chars = view.LineColumnToIndex(pLayout, select.GetTo().x);
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
			select.GetFrom().x, select.GetFrom().y,
			select.GetTo().x, select.GetTo().y
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
	const_cast<EditView&>(view).GetCaret().bClearStatus = false;
	view.editWnd.statusBar.SendStatusMessage2(msg);
}

