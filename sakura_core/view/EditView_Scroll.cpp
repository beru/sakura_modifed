#include "StdAfx.h"
#include "EditView.h"
#include "Ruler.h"
#include "env/DllSharedData.h"
#include "window/EditWnd.h"
#include "types/TypeSupport.h"
#include <limits.h>

/*! �X�N���[���o�[�쐬 */
BOOL EditView::CreateScrollBar()
{
	// �X�N���[���o�[�̍쐬
	hwndVScrollBar = ::CreateWindowEx(
		0L,									// no extended styles
		_T("SCROLLBAR"),					// scroll bar control class
		NULL,								// text for window title bar
		WS_VISIBLE | WS_CHILD | SBS_VERT,	// scroll bar styles
		0,									// horizontal position
		0,									// vertical position
		200,								// width of the scroll bar
		CW_USEDEFAULT,						// default height
		GetHwnd(),							// handle of main window
		(HMENU) NULL,						// no menu for a scroll bar
		G_AppInstance(),					// instance owning this window
		(LPVOID) NULL						// pointer not needed
	);
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
	si.nMin  = 0;
	si.nMax  = 29;
	si.nPage = 10;
	si.nPos  = 0;
	si.nTrackPos = 1;
	::SetScrollInfo(hwndVScrollBar, SB_CTL, &si, TRUE);
	::ShowScrollBar(hwndVScrollBar, SB_CTL, TRUE);

	// �X�N���[���o�[�̍쐬
	hwndHScrollBar = NULL;
	if (GetDllShareData().common.window.bScrollBarHorz) {	// �����X�N���[���o�[���g��
		hwndHScrollBar = ::CreateWindowEx(
			0L,									// no extended styles
			_T("SCROLLBAR"),					// scroll bar control class
			NULL,								// text for window title bar
			WS_VISIBLE | WS_CHILD | SBS_HORZ,	// scroll bar styles
			0,									// horizontal position
			0,									// vertical position
			200,								// width of the scroll bar
			CW_USEDEFAULT,						// default height
			GetHwnd(),							// handle of main window
			(HMENU) NULL,						// no menu for a scroll bar
			G_AppInstance(),					// instance owning this window
			(LPVOID) NULL						// pointer not needed
		);
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = 29;
		si.nPage = 10;
		si.nPos  = 0;
		si.nTrackPos = 1;
		::SetScrollInfo(hwndHScrollBar, SB_CTL, &si, TRUE);
		::ShowScrollBar(hwndHScrollBar, SB_CTL, TRUE);
	}

	// �T�C�Y�{�b�N�X
	if (GetDllShareData().common.window.nFuncKeyWnd_Place == 0) {	// �t�@���N�V�����L�[�\���ʒu�^0:�� 1:��
		hwndSizeBox = ::CreateWindowEx(
			WS_EX_CONTROLPARENT/*0L*/, 			// no extended styles
			_T("SCROLLBAR"),					// scroll bar control class
			NULL,								// text for window title bar
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, // scroll bar styles
			0,									// horizontal position
			0,									// vertical position
			200,								// width of the scroll ba
			CW_USEDEFAULT,						// default height
			GetHwnd(), 							// handle of main window
			(HMENU) NULL,						// no menu for a scroll bar
			G_AppInstance(),					// instance owning this window
			(LPVOID) NULL						// pointer not needed
		);
	}else {
		hwndSizeBox = ::CreateWindowEx(
			0L, 								// no extended styles
			_T("STATIC"),						// scroll bar control class
			NULL,								// text for window title bar
			WS_VISIBLE | WS_CHILD/* | SBS_SIZEBOX | SBS_SIZEGRIP*/, // scroll bar styles
			0,									// horizontal position
			0,									// vertical position
			200,								// width of the scroll bar
			CW_USEDEFAULT,						// default height
			GetHwnd(), 							// handle of main window
			(HMENU) NULL,						// no menu for a scroll bar
			G_AppInstance(),					// instance owning this window
			(LPVOID) NULL						// pointer not needed
		);
	}
	return TRUE;
}



/*! �X�N���[���o�[�j�� */
void EditView::DestroyScrollBar()
{
	if (hwndVScrollBar) {
		::DestroyWindow(hwndVScrollBar);
		hwndVScrollBar = NULL;
	}

	if (hwndHScrollBar) {
		::DestroyWindow(hwndHScrollBar);
		hwndHScrollBar = NULL;
	}

	if (hwndSizeBox) {
		::DestroyWindow(hwndSizeBox);
		hwndSizeBox = NULL;
	}
}

/*! �����X�N���[���o�[���b�Z�[�W����

	@param nScrollCode [in]	�X�N���[����� (Windows����n��������)
	@param nPos [in]		�X�N���[���ʒu(THUMBTRACK�p)
	@retval	���ۂɃX�N���[�������s��
*/
int EditView::OnVScroll(int nScrollCode, int nPos)
{
	int nScrollVal = 0;

	// nPos 32bit�Ή�
	if (nScrollCode == SB_THUMBTRACK || nScrollCode == SB_THUMBPOSITION) {
		if (hwndVScrollBar) {
			HWND hWndScroll = hwndVScrollBar;
			SCROLLINFO info;
			info.cbSize = sizeof(SCROLLINFO);
			info.fMask = SIF_TRACKPOS;
			::GetScrollInfo(hWndScroll, SB_CTL, &info);
			nPos = info.nTrackPos;
		}
	}

	switch (nScrollCode) {
	case SB_LINEDOWN:
//		for (i=0; i<4; ++i) {
//			ScrollAtV(GetTextArea().GetViewTopLine() + 1);
//		}
		nScrollVal = ScrollAtV(GetTextArea().GetViewTopLine() + GetDllShareData().common.general.nRepeatedScrollLineNum);
		break;
	case SB_LINEUP:
//		for (i=0; i<4; ++i) {
//			ScrollAtV(GetTextArea().GetViewTopLine() - 1);
//		}
		nScrollVal = ScrollAtV(GetTextArea().GetViewTopLine() - GetDllShareData().common.general.nRepeatedScrollLineNum);
		break;
	case SB_PAGEDOWN:
		nScrollVal = ScrollAtV(GetTextArea().GetBottomLine());
		break;
	case SB_PAGEUP:
		nScrollVal = ScrollAtV(GetTextArea().GetViewTopLine() - GetTextArea().nViewRowNum);
		break;
	case SB_THUMBPOSITION:
		nScrollVal = ScrollAtV(nPos);
		break;
	case SB_THUMBTRACK:
		nScrollVal = ScrollAtV(nPos);
		break;
	case SB_TOP:
		nScrollVal = ScrollAtV(0);
		break;
	case SB_BOTTOM:
		nScrollVal = ScrollAtV((int)pEditDoc->layoutMgr.GetLineCount() - GetTextArea().nViewRowNum);
		break;
	default:
		break;
	}
	return nScrollVal;
}

/*! �����X�N���[���o�[���b�Z�[�W����

	@param nScrollCode [in]	�X�N���[����� (Windows����n��������)
	@param nPos [in]		�X�N���[���ʒu(THUMBTRACK�p)
	@retval	���ۂɃX�N���[����������
*/
int EditView::OnHScroll(int nScrollCode, int nPos)
{
	const int nHScrollNum = 4;
	int nScrollVal = 0;

	// nPos 32bit�Ή�
	if (nScrollCode == SB_THUMBTRACK || nScrollCode == SB_THUMBPOSITION) {
		if (hwndHScrollBar) {
			HWND hWndScroll = hwndHScrollBar;
			SCROLLINFO info;
			info.cbSize = sizeof(SCROLLINFO);
			info.fMask = SIF_TRACKPOS;
			::GetScrollInfo(hWndScroll, SB_CTL, &info);
			nPos = info.nTrackPos;
		}
	}

	GetRuler().SetRedrawFlag();
	switch (nScrollCode) {
	case SB_LINELEFT:
		nScrollVal = ScrollAtH(GetTextArea().GetViewLeftCol() - nHScrollNum);
		break;
	case SB_LINERIGHT:
		nScrollVal = ScrollAtH(GetTextArea().GetViewLeftCol() + nHScrollNum);
		break;
	case SB_PAGELEFT:
		nScrollVal = ScrollAtH(GetTextArea().GetViewLeftCol() - GetTextArea().nViewColNum);
		break;
	case SB_PAGERIGHT:
		nScrollVal = ScrollAtH(GetTextArea().GetRightCol());
		break;
	case SB_THUMBPOSITION:
		nScrollVal = ScrollAtH(nPos);
//		MYTRACE(_T("nPos=%d\n"), nPos);
		break;
	case SB_THUMBTRACK:
		nScrollVal = ScrollAtH(nPos);
//		MYTRACE(_T("nPos=%d\n"), nPos);
		break;
	case SB_LEFT:
		nScrollVal = ScrollAtH(0);
		break;
	case SB_RIGHT:
		nScrollVal = ScrollAtH((int)pEditDoc->layoutMgr.GetMaxLineKetas() - GetTextArea().nViewColNum);
		break;
	}
	return nScrollVal;
}

/** �X�N���[���o�[�̏�Ԃ��X�V����

	�^�u�o�[�̃^�u�ؑ֎��� SIF_DISABLENOSCROLL �t���O�ł̗L�����^������������ɓ��삵�Ȃ�
	�i�s���ŃT�C�Y�ύX���Ă��邱�Ƃɂ��e�����H�j�̂� SIF_DISABLENOSCROLL �ŗL���^����
	�̐ؑւɎ��s�����ꍇ�ɂ͋����ؑւ���
*/
void EditView::AdjustScrollBars()
{
	if (!GetDrawSwitch()) {
		return;
	}

	SCROLLINFO	si;
	if (hwndVScrollBar) {
		// �����X�N���[���o�[
		const int nEofMargin = 2; // EOF�Ƃ��̉��̃}�[�W��
		const size_t nAllLines = pEditDoc->layoutMgr.GetLineCount() + nEofMargin;
		int	nVScrollRate = 1;
#ifdef _WIN64
		// nAllLines / nVScrollRate < INT_MAX �ƂȂ鐮��nVScrollRate�����߂�
		// 64bit�ŗp�X�N���[����
		while (nAllLines / nVScrollRate > INT_MAX) {
			++nVScrollRate;
		}
#endif
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = (int)nAllLines / nVScrollRate - 1;	// �S�s��
		si.nPage = GetTextArea().nViewRowNum / nVScrollRate;	// �\����̍s��
		si.nPos  = GetTextArea().GetViewTopLine() / nVScrollRate;	// �\����̈�ԏ�̍s(0�J�n)
		si.nTrackPos = 0;
		::SetScrollInfo(hwndVScrollBar, SB_CTL, &si, TRUE);
		this->nVScrollRate = nVScrollRate;				// �����X�N���[���o�[�̏k��
		
		//	�c�X�N���[���o�[��Disable�ɂȂ����Ƃ��͕K���S�̂���ʓ��Ɏ��܂�悤��
		//	�X�N���[��������
		bool bEnable = (GetTextArea().nViewRowNum < (int)nAllLines);
		if (bEnable != (::IsWindowEnabled(hwndVScrollBar) != 0)) {
			::EnableWindow(hwndVScrollBar, bEnable? TRUE: FALSE);	// SIF_DISABLENOSCROLL �듮�쎞�̋����ؑ�
		}
		if (!bEnable) {
			ScrollAtV(0);
		}
	}
	if (hwndHScrollBar) {
		// �����X�N���[���o�[
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = (int)GetRightEdgeForScrollBar() - 1;		// �X�N���[���o�[����p�̉E�[���W���擾
		si.nPage = GetTextArea().nViewColNum;			// �\����̌���
		si.nPos  = GetTextArea().GetViewLeftCol();		// �\����̈�ԍ��̌�(0�J�n)
		si.nTrackPos = 1;
		::SetScrollInfo(hwndHScrollBar, SB_CTL, &si, TRUE);

		bool bEnable = (GetTextArea().nViewColNum < (int)GetRightEdgeForScrollBar());
		if (bEnable != (::IsWindowEnabled(hwndHScrollBar) != 0)) {
			::EnableWindow(hwndHScrollBar, bEnable? TRUE: FALSE);	// SIF_DISABLENOSCROLL �듮�쎞�̋����ؑ�
		}
		if (!bEnable) {
			ScrollAtH(0);
		}
	}
}

/*! �w���[�s�ʒu�փX�N���[��

	@param nPos [in] �X�N���[���ʒu
	@retval ���ۂɃX�N���[�������s�� (��:������/��:�����)
*/
int EditView::ScrollAtV(int nPos)
{
	int nScrollRowNum;
	if (nPos < 0) {
		nPos = 0;
	}else if (((int)pEditDoc->layoutMgr.GetLineCount() + 2) - GetTextArea().nViewRowNum < nPos) {
		nPos = ((int)pEditDoc->layoutMgr.GetLineCount() + 2) - (int)GetTextArea().nViewRowNum;
		if (nPos < 0) {
			nPos = 0;
		}
	}
	if (GetTextArea().GetViewTopLine() == nPos) {
		return 0;	//	�X�N���[�������B
	}
	// �����X�N���[���ʁi�s���j�̎Z�o
	nScrollRowNum = GetTextArea().GetViewTopLine() - nPos;

	// �X�N���[��
	if (t_abs(nScrollRowNum) >= GetTextArea().nViewRowNum) {
		GetTextArea().SetViewTopLine(nPos);
		::InvalidateRect(GetHwnd(), NULL, TRUE);
	}else {
		RECT rcClip;
		RECT rcScrol;
		rcScrol.left = 0;
		rcScrol.right = GetTextArea().GetAreaRight();
		rcScrol.top = GetTextArea().GetAreaTop();
		rcScrol.bottom = GetTextArea().GetAreaBottom();
		if (nScrollRowNum > 0) {
			rcScrol.bottom =
				GetTextArea().GetAreaBottom() -
				nScrollRowNum * GetTextMetrics().GetHankakuDy();
			GetTextArea().SetViewTopLine(nPos);
			rcClip.left = 0;
			rcClip.right = GetTextArea().GetAreaRight();
			rcClip.top = GetTextArea().GetAreaTop();
			rcClip.bottom =
				GetTextArea().GetAreaTop() + nScrollRowNum * GetTextMetrics().GetHankakuDy();
		}else if (nScrollRowNum < 0) {
			rcScrol.top =
				GetTextArea().GetAreaTop() - nScrollRowNum * GetTextMetrics().GetHankakuDy();
			GetTextArea().SetViewTopLine(nPos);
			rcClip.left = 0;
			rcClip.right = GetTextArea().GetAreaRight();
			rcClip.top =
				GetTextArea().GetAreaBottom() +
				nScrollRowNum * GetTextMetrics().GetHankakuDy();
			rcClip.bottom = GetTextArea().GetAreaBottom();
		}
		if (GetDrawSwitch()) {
			RECT rcClip2 = {0, 0, 0, 0};
			ScrollDraw(nScrollRowNum, 0, rcScrol, rcClip, rcClip2);
			::UpdateWindow(GetHwnd());
		}
	}

	// �X�N���[���o�[�̏�Ԃ��X�V����
	AdjustScrollBars();

	// �L�����b�g�̕\���E�X�V
	GetCaret().ShowEditCaret();

	MiniMapRedraw(false);

	return -nScrollRowNum;	// �������t�Ȃ̂ŕ������]���K�v
}


/*! �w�荶�[���ʒu�փX�N���[��

	@param nPos [in] �X�N���[���ʒu
	@retval ���ۂɃX�N���[���������� (��:�E����/��:������)
*/
int EditView::ScrollAtH(int nPos)
{
	if (nPos < 0) {
		nPos = 0;
	}
	else {
		int nPos2 = (int)(GetRightEdgeForScrollBar() + GetWrapOverhang()) - GetTextArea().nViewColNum;
		if (nPos2 < nPos) {
			nPos = nPos2;
			//	�܂�Ԃ������E�B���h�E�����傫���Ƃ���WM_HSCROLL�������
			//	nPos�����̒l�ɂȂ邱�Ƃ�����C���̏ꍇ�ɃX�N���[���o�[����ҏW�̈悪
			//	����Ă��܂��D
			if (nPos < 0) {
				nPos = 0;
			}
		}
	}
	auto& textArea = GetTextArea();
	if (textArea.GetViewLeftCol() == nPos) {
		return 0;
	}
	// �����X�N���[���ʁi�������j�̎Z�o
	const int nScrollColNum = textArea.GetViewLeftCol() - nPos;

	// �X�N���[��
	if (t_abs(nScrollColNum) >= textArea.nViewColNum /*|| abs(nScrollRowNum) >= textArea.nViewRowNum*/) {
		textArea.SetViewLeftCol(nPos);
		::InvalidateRect(GetHwnd(), NULL, TRUE);
	}else {
		RECT rcClip2;
		RECT rcScrol;
		rcScrol.left = 0;
		rcScrol.right = textArea.GetAreaRight();
		rcScrol.top = textArea.GetAreaTop();
		rcScrol.bottom = textArea.GetAreaBottom();
		int nScrollColPxWidth = nScrollColNum * GetTextMetrics().GetHankakuDx();
		if (nScrollColNum > 0) {
			rcScrol.left = textArea.GetAreaLeft();
			rcScrol.right = textArea.GetAreaRight() - nScrollColPxWidth;
			rcClip2.left = textArea.GetAreaLeft();
			rcClip2.right = textArea.GetAreaLeft() + nScrollColPxWidth;
			rcClip2.top = textArea.GetAreaTop();
			rcClip2.bottom = textArea.GetAreaBottom();
		}else if (nScrollColNum < 0) {
			rcScrol.left = textArea.GetAreaLeft() - nScrollColPxWidth;
			rcClip2.left = textArea.GetAreaRight() + nScrollColPxWidth;
			rcClip2.right = textArea.GetAreaRight();
			rcClip2.top = textArea.GetAreaTop();
			rcClip2.bottom = textArea.GetAreaBottom();
		}
		textArea.SetViewLeftCol(nPos);
		if (GetDrawSwitch()) {
			RECT rcClip = {0, 0, 0, 0};
			ScrollDraw(0, nScrollColNum, rcScrol, rcClip, rcClip2);
			::UpdateWindow(GetHwnd());
		}
	}
	// ���AdjustScrollBars���Ă�ł��܂��ƁA��x�ڂ͂����܂ł��Ȃ��̂ŁA
	// GetRuler().DispRuler���Ă΂�Ȃ��B���̂��߁A���������ւ����B
	GetRuler().SetRedrawFlag(); // ���[���[���ĕ`�悷��B
	HDC hdc = ::GetDC(GetHwnd());
	GetRuler().DispRuler(hdc);
	::ReleaseDC(GetHwnd(), hdc);

	// �X�N���[���o�[�̏�Ԃ��X�V����
	AdjustScrollBars();

	// �L�����b�g�̕\���E�X�V
	GetCaret().ShowEditCaret();

	return -nScrollColNum;	// �������t�Ȃ̂ŕ������]���K�v
}


void EditView::ScrollDraw(
	int nScrollRowNum,
	int nScrollColNum,
	const RECT& rcScroll,
	const RECT& rcClip,
	const RECT& rcClip2
	)
{
	const TextArea& area = GetTextArea();

	// �w�i�͉�ʂɑ΂��ČŒ肩
	bool bBackImgFixed = IsBkBitmap() &&
		(nScrollRowNum != 0 && !pTypeData->backImgScrollY ||
		 nScrollColNum != 0 && !pTypeData->backImgScrollX);
	if (bBackImgFixed) {
		Rect rcBody = area.GetAreaRect();
		rcBody.left = 0; // �s�ԍ����ړ�
		rcBody.top = area.GetRulerHeight();
		InvalidateRect(&rcBody, FALSE);
	}else {
		int nScrollColPxWidth = nScrollColNum * GetTextMetrics().GetHankakuDx();
		ScrollWindowEx(
			nScrollColPxWidth,	// �����X�N���[����
			nScrollRowNum * GetTextMetrics().GetHankakuDy(),	// �����X�N���[����
			&rcScroll,	// �X�N���[�������`�̍\���̂̃A�h���X
			NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
		);
		if (hbmpCompatBMP) {
			// �݊�BMP���X�N���[�������̂��߂�BitBlt�ňړ�������
			::BitBlt(
				hdcCompatDC,
				rcScroll.left + nScrollColPxWidth,
				rcScroll.top  + nScrollRowNum * GetTextMetrics().GetHankakuDy(),
				rcScroll.right - rcScroll.left, rcScroll.bottom - rcScroll.top,
				hdcCompatDC, rcScroll.left, rcScroll.top, SRCCOPY
			);
		}

		if (1
			&& 0 < area.GetTopYohaku()
			&& IsBkBitmap()
			&& (nScrollRowNum != 0 && pTypeData->backImgScrollY || nScrollColNum != 0 && pTypeData->backImgScrollX)
		) {
			// Scroll�̂Ƃ��Ƀ��[���[�]���X�V
			Rect rcTopYohaku;
			if (TypeSupport(*this, COLORIDX_TEXT).GetBackColor() == TypeSupport(*this, COLORIDX_GYOU).GetBackColor()) {
				rcTopYohaku.left = 0;
			}else {
				rcTopYohaku.left = area.GetLineNumberWidth();
			}
			rcTopYohaku.top  = area.GetRulerHeight();
			rcTopYohaku.right  = area.GetAreaRight();
			rcTopYohaku.bottom = area.GetAreaTop();
			HDC hdcSelf = GetDC();
			HDC hdcBgImg = hdcCompatDC ? hdcCompatDC : CreateCompatibleDC(hdcSelf);
			HBITMAP hOldBmp = (HBITMAP)::SelectObject(hdcBgImg, pEditDoc->hBackImg);
			DrawBackImage(hdcSelf, rcTopYohaku, hdcBgImg);
			SelectObject(hdcBgImg, hOldBmp);
			ReleaseDC(hdcSelf);
			if (!hdcCompatDC) {
				DeleteObject(hdcBgImg);
			}
		}
		if (IsBkBitmap()
			&& nScrollColNum != 0
			&& pTypeData->backImgScrollX
		) {
			// �s�ԍ��w�i�̂��߂ɍX�V
			Rect rcLineNum;
			area.GenerateLineNumberRect(&rcLineNum);
			InvalidateRect(&rcLineNum, FALSE);
		}
	}
	// �J�[�\���̏c�����e�L�X�g�ƍs�ԍ��̌��Ԃɂ���Ƃ��A�X�N���[�����ɏc���̈���X�V
	if (nScrollColNum != 0 && nOldCursorLineX == GetTextArea().GetAreaLeft() - 1) {
		RECT rcClip3;
		rcClip3.left   = nOldCursorLineX - (nOldCursorVLineWidth - 1);
		rcClip3.right  = nOldCursorLineX + 1;
		rcClip3.top    = GetTextArea().GetAreaTop();
		rcClip3.bottom = GetTextArea().GetAreaBottom();
		InvalidateRect(&rcClip3, FALSE);
	}

	if (nScrollRowNum != 0) {
		InvalidateRect(&rcClip);
		if (nScrollColNum != 0) {
			RECT lineNumClip;
			GetTextArea().GenerateLineNumberRect(&lineNumClip);
			InvalidateRect(&lineNumClip, FALSE);
		}
	}
	if (nScrollColNum != 0) {
		InvalidateRect(&rcClip2, FALSE);
	}
}


void EditView::MiniMapRedraw(bool bUpdateAll)
{
	if (this == &editWnd.GetActiveView() && editWnd.GetMiniMap().GetHwnd()) {
		EditView& miniMap = editWnd.GetMiniMap();
		int nViewTop = miniMap.nPageViewTop;
		int nViewBottom = miniMap.nPageViewBottom;
		int nDiff = nViewTop - (int)GetTextArea().GetViewTopLine();
		int nDrawTopTop;
		int nDrawTopBottom;
		bool bUpdate = (t_abs(nDiff) > nViewBottom - nViewTop) || bUpdateAll;
		bool bUpdateOne = false;
		if (bUpdate) {
			if (nViewTop == GetTextArea().GetViewTopLine()) {
				// OnSize:�������L�k����
				bUpdateOne = true;
				nDrawTopTop = t_min(nViewBottom, (int)GetTextArea().GetBottomLine());
				nDrawTopBottom = t_max(nViewBottom, (int)GetTextArea().GetBottomLine());
			}else {
				nDrawTopTop = nViewTop;
				nDrawTopBottom = nViewBottom;
			}
		}else {
			if (nDiff < 0) {
				// ��Ɉړ�
				nDrawTopTop = GetTextArea().GetViewTopLine();
				nDrawTopBottom = nViewTop;
			}else {
				// ���Ɉړ�
				nDrawTopTop = nViewTop;
				nDrawTopBottom = GetTextArea().GetViewTopLine();
			}
		}
		RECT rcMiniMap;
		rcMiniMap.left = 0;
		rcMiniMap.right = miniMap.GetTextArea().GetAreaRight();
		rcMiniMap.top = miniMap.GetTextArea().GenerateYPx(nDrawTopTop);
		rcMiniMap.bottom = miniMap.GetTextArea().GenerateYPx(nDrawTopBottom);
		::InvalidateRect(miniMap.GetHwnd(), &rcMiniMap, FALSE);
		::UpdateWindow(miniMap.GetHwnd());

		if (bUpdateOne) {
			return;
		}
		int nDrawBottomTop;
		int nDrawBottomBottom;
		if (bUpdate) {
			nDrawBottomTop = GetTextArea().GetViewTopLine();
			nDrawBottomBottom = GetTextArea().GetBottomLine();
		}else {
			if (nDiff < 0) {
				// ��Ɉړ�
				nDrawBottomTop = GetTextArea().GetBottomLine();
				nDrawBottomBottom = nViewBottom;
			}else {
				// ���Ɉړ�
				nDrawBottomTop = nViewBottom;
				nDrawBottomBottom = GetTextArea().GetBottomLine();
			}
		}
		rcMiniMap.left = 0;
		rcMiniMap.right = miniMap.GetTextArea().GetAreaRight();
		rcMiniMap.top = miniMap.GetTextArea().GenerateYPx(nDrawBottomTop);
		rcMiniMap.bottom = miniMap.GetTextArea().GenerateYPx(nDrawBottomBottom);
		::InvalidateRect(miniMap.GetHwnd(), &rcMiniMap, FALSE);
		::UpdateWindow(miniMap.GetHwnd());
	}
}


/*!	���������X�N���[��

	���������X�N���[����ON�Ȃ�΁C�Ή�����E�B���h�E���w��s�������X�N���[������
	
	@param line [in] �X�N���[���s�� (��:������/��:�����/0:�������Ȃ�)
	
	@note ����̏ڍׂ͐ݒ��@�\�g���ɂ��ύX�ɂȂ�\��������

*/
void EditView::SyncScrollV(int line)
{
	if (GetDllShareData().common.window.bSplitterWndVScroll && line != 0 
		&& editWnd.IsEnablePane(nMyIndex^0x01) 
		&& 0 <= nMyIndex
	) {
		EditView& editView = editWnd.GetView(nMyIndex^0x01);
#if 0
		//	������ۂ����܂܃X�N���[������ꍇ
		editView.ScrollByV(line);
#else
		editView.ScrollAtV(GetTextArea().GetViewTopLine());
#endif
	}
}

/*!	���������X�N���[��

	���������X�N���[����ON�Ȃ�΁C�Ή�����E�B���h�E���w��s�������X�N���[������D
	
	@param col [in] �X�N���[������ (��:�E����/��:������/0:�������Ȃ�)
	
	@note ����̏ڍׂ͐ݒ��@�\�g���ɂ��ύX�ɂȂ�\��������
*/
void EditView::SyncScrollH(int col)
{
	if (GetDllShareData().common.window.bSplitterWndHScroll && col != 0
		&& editWnd.IsEnablePane(nMyIndex^0x02)
		&& 0 <= nMyIndex
	) {
		EditView& editView = editWnd.GetView(nMyIndex^0x02);
		HDC hdc = ::GetDC(editView.GetHwnd());
		
#if 0
		//	������ۂ����܂܃X�N���[������ꍇ
		editView.ScrollByH(col);
#else
		editView.ScrollAtH(GetTextArea().GetViewLeftCol());
#endif
		GetRuler().SetRedrawFlag(); // �X�N���[�������[���[�S�̂�`���Ȃ����B
		GetRuler().DispRuler(hdc);
		::ReleaseDC(GetHwnd(), hdc);
	}
}

/** �܂�Ԃ����Ȍ�̂Ԃ牺���]���v�Z */
size_t EditView::GetWrapOverhang(void) const
{
	size_t nMargin = 1;	// �܂�Ԃ��L��
	if (!pTypeData->bKinsokuHide) {	// �Ԃ牺�����B�����̓X�L�b�v
		if (pTypeData->bKinsokuRet) {
			nMargin += 1;	// ���s�Ԃ牺��
		}
		if (pTypeData->bKinsokuKuto) {
			nMargin += 2;	// ��Ǔ_�Ԃ牺��
		}
	}
	return nMargin;
}

/** �u�E�[�Ő܂�Ԃ��v�p�Ƀr���[�̌�������܂�Ԃ��������v�Z����
	@param nViewColNum	[in] �r���[�̌���
	@retval �܂�Ԃ�����
*/
int EditView::ViewColNumToWrapColNum(int nViewColNum) const
{
	// �Ԃ牺���]������������
	int nWidth = nViewColNum - (int)GetWrapOverhang();

	// MINLINEKETAS�����̎���MINLINEKETAS�Ő܂�Ԃ��Ƃ���
	if (nWidth < MINLINEKETAS) {
		nWidth = MINLINEKETAS;		// �܂�Ԃ����̍ŏ������ɐݒ�
	}
	return nWidth;
}

/*!
	@brief  �X�N���[���o�[����p�ɉE�[���W���擾����

	�u�܂�Ԃ��Ȃ��v
		�t���[�J�[�\����Ԃ̎��̓e�L�X�g�̕������E���փJ�[�\�����ړ��ł���
		�̂ŁA������l�������X�N���[���o�[�̐��䂪�K�v�B
		�{�֐��́A���L�̓��ōł��傫�Ȓl�i�E�[�̍��W�j��Ԃ��B
		�@�E�e�L�X�g�̉E�[
		�@�E�L�����b�g�ʒu
		�@�E�I��͈͂̉E�[
	
	�u�w�茅�Ő܂�Ԃ��v
	�u�E�[�Ő܂�Ԃ��v
		��L�̏ꍇ�܂�Ԃ����Ȍ�̂Ԃ牺���]���v�Z

	@return     �E�[�̃��C�A�E�g���W��Ԃ�

	@note   �u�܂�Ԃ��Ȃ��v�I�����́A�X�N���[����ɃL�����b�g�������Ȃ�
	        �Ȃ�Ȃ��l�ɂ��邽�߂ɉE�}�[�W���Ƃ��Ĕ��p3���Œ�ŉ��Z����B
*/
size_t EditView::GetRightEdgeForScrollBar(void)
{
	// �܂�Ԃ����Ȍ�̂Ԃ牺���]���v�Z
	size_t nWidth = pEditDoc->layoutMgr.GetMaxLineKetas() + GetWrapOverhang();
	
	if (pEditDoc->nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
		int nRightEdge = (int)pEditDoc->layoutMgr.GetMaxTextWidth();	// �e�L�X�g�̍ő啝
		// �I��͈͂��� ���� �͈͂̉E�[���e�L�X�g�̕����E��
		if (GetSelectionInfo().IsTextSelected()) {
			// �J�n�ʒu�E�I���ʒu�̂��E���ɂ�����Ŕ�r
			auto& select = GetSelectionInfo().select;
			if (select.GetFrom().x < select.GetTo().x) {
				if (nRightEdge < select.GetTo().x) {
					nRightEdge = select.GetTo().x;
				}
			}else {
				if (nRightEdge < select.GetFrom().x) {
					nRightEdge = select.GetFrom().x;
				}
			}
		}

		// �t���[�J�[�\�����[�h ���� �L�����b�g�ʒu���e�L�X�g�̕����E��
		if (GetDllShareData().common.general.bIsFreeCursorMode && nRightEdge < GetCaret().GetCaretLayoutPos().x)
			nRightEdge = GetCaret().GetCaretLayoutPos().x;

		// �E�}�[�W�����i3���j���l������nWidth�𒴂��Ȃ��悤�ɂ���
		nWidth = (nRightEdge + 3 < (int)nWidth) ? (nRightEdge + 3) : nWidth;
	}

	return nWidth;
}

