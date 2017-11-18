/*!	@file
	@brief ���Preview�Ǘ��N���X
*/
#include "StdAfx.h"
#include "PrintPreview.h"
#include "uiparts/HandCursor.h"
#include "doc/layout/Layout.h"
#include "window/EditWnd.h"
#include "dlg/DlgCancel.h" /// 2002/2/3 aroka from here
#include "dlg/DlgInput1.h" /// 2007.02.11 Moca
#include "EditApp.h"
#include "util/window.h"
#include "util/shell.h"
#include "env/SakuraEnvironment.h"
// ColorStrategy�͖{����CEditView���K�v�����AEditWnd.h�������include�ς�
#include "view/colors/ColorStrategy.h"
#include "sakura_rc.h"

using namespace std;

#define MIN_PREVIEW_ZOOM 10
#define MAX_PREVIEW_ZOOM 400

#define		LINE_RANGE_X	48		// ���������̂P��̃X�N���[����
#define		LINE_RANGE_Y	24		// ���������̂P��̃X�N���[����

#define		PAGE_RANGE_X	160		// ���������̂P��̃y�[�W�X�N���[����
#define		PAGE_RANGE_Y	160		// ���������̂P��̃y�[�W�X�N���[����

#define		COMPAT_BMP_BASE     1   // COMPAT_BMP_SCALE�s�N�Z�����𕡎ʂ����ʃs�N�Z����
#define		COMPAT_BMP_SCALE    2   // �݊�BMP��COMPAT_BMP_BASE�ɑ΂���{��(1�ȏ�̐����{)

Print PrintPreview::print;		// ���݂̃v�����^��� 2003.05.02 �����

/*! �R���X�g���N�^
	���Preview��\�����邽�߂ɕK�v�ȏ����������A�̈�m�ہB
	�R���g���[�����쐬����B
*/
PrintPreview::PrintPreview(EditWnd& parentWnd)
	:
	parentWnd(parentWnd),
	hdcCompatDC(NULL),			// �ĕ`��p�R���p�`�u��DC
	hbmpCompatBMP(NULL),			// �ĕ`��p������BMP
	hbmpCompatBMPOld(NULL),		// �ĕ`��p������BMP(OLD)
	nbmpCompatScale(COMPAT_BMP_BASE),
	nPreviewVScrollPos(0),
	nPreviewHScrollPos(0),
	nPreview_Zoom(100),			// ���Preview�{��
	nCurPageNum(0),				// ���݂̃y�[�W
	bLockSetting(false),
	bDemandUpdateSetting(false)
{
	// ����p�̃��C�A�E�g���̍쐬
	pLayoutMgr_Print = new LayoutMgr;

	// ���Preview �R���g���[�� �쐬
	CreatePrintPreviewControls();

	// �ĕ`��p�R���p�`�u��DC
	HDC hdc = ::GetDC(parentWnd.GetHwnd());
	hdcCompatDC = ::CreateCompatibleDC(hdc);
	::ReleaseDC(parentWnd.GetHwnd(), hdc);
}

PrintPreview::~PrintPreview()
{
	// ���Preview �R���g���[�� �j��
	DestroyPrintPreviewControls();
	
	// ����p�̃��C�A�E�g���̍폜
	delete pLayoutMgr_Print;
	
	// �t�H���g���L���b�V����ҏW���[�h�ɖ߂�
	SelectCharWidthCache(CharWidthFontMode::Edit, CharWidthCacheMode::Neutral);

	// 2006.08.17 Moca CompatDC�폜�BEditWnd����ڐ�
	// �ĕ`��p������BMP
	if (hbmpCompatBMP) {
		// �ĕ`��p������BMP(OLD)
		::SelectObject(hdcCompatDC, hbmpCompatBMPOld);
		::DeleteObject(hbmpCompatBMP);
	}
	// �ĕ`��p�R���p�`�u��DC
	if (hdcCompatDC) {
		::DeleteDC(hdcCompatDC);
	}
}

/*!	���Preview���́AWM_PAINT������

	@date 2007.02.11 Moca Preview�����炩�ɂ���@�\�D
		�g��`�悵�Ă���k�����邱�ƂŃA���`�G�C���A�X���ʂ��o���D
*/
LRESULT PrintPreview::OnPaint(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
	PAINTSTRUCT		ps;
	HDC				hdcOld = ::BeginPaint(hwnd, &ps);
	HDC				hdc = hdcCompatDC;	//	�e�E�B���h�E��ComatibleDC�ɕ`��

	// ���Preview ����o�[
	
	// BMP�͂��Ƃŏk���R�s�[����̂Ŋg�債�č�悷��K�v����

	// �N���C�A���g�̈�S�̂��O���[�œh��Ԃ�
	{
		RECT bmpRc;
		::GetClientRect(hwnd, &bmpRc);
		bmpRc.right  = (bmpRc.right  * nbmpCompatScale) / COMPAT_BMP_BASE;
		bmpRc.bottom = (bmpRc.bottom * nbmpCompatScale) / COMPAT_BMP_BASE;
		::FillRect(hdc, &bmpRc, (HBRUSH)::GetStockObject(GRAY_BRUSH));
	}

	// �c�[���o�[���� -> nToolBarHeight
	int nToolBarHeight = 0;
	if (hwndPrintPreviewBar) {
		RECT rc;
		::GetWindowRect(hwndPrintPreviewBar, &rc);
		nToolBarHeight = rc.bottom - rc.top;
	}

	// �v�����^���̕\�� -> IDD_PRINTPREVIEWBAR�E���STATIC��
	TCHAR	szText[1024];
	::DlgItem_SetText(
		hwndPrintPreviewBar,
		IDC_STATIC_PRNDEV,
		pPrintSetting->mdmDevMode.szPrinterDeviceName
	);

	// �v�f���̕\�� -> IDD_PRINTPREVIEWBAR�E����STATIC��
	TCHAR	szPaperName[256];
	Print::GetPaperName(pPrintSetting->mdmDevMode.dmPaperSize , szPaperName);
	auto_sprintf_s(
		szText,
		_T("%ts  %ts"),
		szPaperName,
		(pPrintSetting->mdmDevMode.dmOrientation & DMORIENT_LANDSCAPE) ? LS(STR_ERR_DLGPRNPRVW1) : LS(STR_ERR_DLGPRNPRVW2)
	);
	::DlgItem_SetText(hwndPrintPreviewBar, IDC_STATIC_PAPER, szText);

	// �o�b�N�O���E���h ���[�h��ύX
	::SetBkMode(hdc, TRANSPARENT);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �}�b�s���O                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �}�b�s���O���[�h�̕ύX
	int nMapModeOld =
	::SetMapMode(hdc, MM_LOMETRIC);
	::SetMapMode(hdc, MM_ANISOTROPIC);

	// �o�͔{���̕ύX
	SIZE sz;
	::GetWindowExtEx(hdc, &sz);
	int nCx = sz.cx;
	int nCy = sz.cy;
	nCx = (int)(((long)nCx) * 100L / ((long)nPreview_Zoom));
	nCy = (int)(((long)nCy) * 100L / ((long)nPreview_Zoom));
	// ��掞�́A COMPAT_BMP_SCALE/COMPAT_BMP_BASE�{�̍��W (SetWindowExtEx�͋t�Ȃ̂Ŕ��΂ɂȂ�)
	nCx = (nCx * COMPAT_BMP_BASE) / nbmpCompatScale;
	nCy = (nCy * COMPAT_BMP_BASE) / nbmpCompatScale;
	::SetWindowExtEx(hdc, nCx, nCy, &sz);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �t�H���g                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �t�H���g�쐬
	CreateFonts(hdc);
	// ����p���p�t�H���g�ɐݒ肵�A�ȑO�̃t�H���g��ێ�
	HFONT hFontOld = (HFONT)::SelectObject(hdc, hFontHan);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ���_                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// ����E�B���h�E�̉��ɕ������W���_���ړ�
	POINT poViewPortOld;
	::SetViewportOrgEx(
		hdc,
		((-1 * nPreviewHScrollPos) * nbmpCompatScale) / COMPAT_BMP_BASE, 
		((nToolBarHeight + nPreviewVScrollPos) * nbmpCompatScale) / COMPAT_BMP_BASE,
		&poViewPortOld
	);


	// �ȉ� 0.1mm���W�Ń����_�����O

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �w�i                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �p���̕`��
	int	nDirectY = -1;	//	Y���W�̉����v���X�����ɂ��邽�߁H
	::Rectangle(hdc,
		nPreview_ViewMarginLeft,
		nDirectY * (nPreview_ViewMarginTop),
		nPreview_ViewMarginLeft + nPreview_PaperAllWidth + 1,
		nDirectY * (nPreview_ViewMarginTop + nPreview_PaperAllHeight + 1)
	);

	// �}�[�W���g�̕\��
	Graphics gr(hdc);
	gr.SetPen(RGB(128, 128, 128)); // 2006.08.14 Moca 127��128�ɕύX
	::Rectangle(hdc,
		nPreview_ViewMarginLeft + pPrintSetting->nPrintMarginLX,
		nDirectY * (nPreview_ViewMarginTop + pPrintSetting->nPrintMarginTY),
		nPreview_ViewMarginLeft + nPreview_PaperAllWidth - pPrintSetting->nPrintMarginRX + 1,
		nDirectY * (nPreview_ViewMarginTop + nPreview_PaperAllHeight - pPrintSetting->nPrintMarginBY)
	);
	gr.ClearPen();

	::SetTextColor(hdc, RGB(0, 0, 0));

	RECT rect;	// ���̑傫��������킷RECT
	rect.left   = nPreview_ViewMarginLeft +                             pPrintSetting->nPrintMarginLX + 5;
	rect.right  = nPreview_ViewMarginLeft + nPreview_PaperAllWidth - (pPrintSetting->nPrintMarginRX + 5);
	rect.top    = nDirectY * (nPreview_ViewMarginTop +                              pPrintSetting->nPrintMarginTY + 5);
	rect.bottom = nDirectY * (nPreview_ViewMarginTop + nPreview_PaperAllHeight - (pPrintSetting->nPrintMarginBY + 5));
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �e�L�X�g                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	int nHeaderHeight = Print::CalcHeaderHeight(*pPrintSetting);

	// �w�b�_
	if (nHeaderHeight) {
		DrawHeaderFooter(hdc, rect, true);
	}

	ColorStrategy* pStrategyStart = DrawPageTextFirst(nCurPageNum);

	// ���/���Preview �y�[�W�e�L�X�g�̕`��
	DrawPageText(
		hdc,
		nPreview_ViewMarginLeft + pPrintSetting->nPrintMarginLX,
		nPreview_ViewMarginTop  + pPrintSetting->nPrintMarginTY + nHeaderHeight*2,
		nCurPageNum,
		NULL,
		pStrategyStart
	);

	// �t�b�^
	if (Print::CalcFooterHeight(*pPrintSetting)) {
		DrawHeaderFooter(hdc, rect, false);
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                          ��n��                             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//	����O�̃t�H���g�ɖ߂�
	::SelectObject(hdc, hFontOld);

	// �}�b�s���O���[�h�̕ύX
	::SetMapMode(hdc, nMapModeOld);

	//	����p�t�H���g�j��
	DestroyFonts();

	// �������W���_�����Ƃɖ߂�
	::SetViewportOrgEx(hdc, poViewPortOld.x, poViewPortOld.y, NULL);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       ����ʂ֓]��                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// MemoryDC�𗘗p�����ĕ`��̏ꍇ��MemoryDC�ɕ`�悵�����e����ʂփR�s�[����
	RECT rc = ps.rcPaint;
	::DPtoLP(hdc, (POINT*)&rc, 2);
	if ((nbmpCompatScale / COMPAT_BMP_BASE) == 1) {
		::BitBlt(
			hdcOld,
			ps.rcPaint.left,
			ps.rcPaint.top,
			ps.rcPaint.right - ps.rcPaint.left,
			ps.rcPaint.bottom - ps.rcPaint.top,
			hdc,
			ps.rcPaint.left,
			ps.rcPaint.top,
			SRCCOPY
		);
	}else {
		int stretchModeOld = SetStretchBltMode(hdcOld, STRETCH_HALFTONE);
		::StretchBlt(
			hdcOld,
			ps.rcPaint.left,
			ps.rcPaint.top,
			ps.rcPaint.right - ps.rcPaint.left,
			ps.rcPaint.bottom - ps.rcPaint.top,
			hdc,
			(ps.rcPaint.left * nbmpCompatScale) / COMPAT_BMP_BASE,
			(ps.rcPaint.top * nbmpCompatScale) / COMPAT_BMP_BASE,
			((ps.rcPaint.right - ps.rcPaint.left) * nbmpCompatScale) / COMPAT_BMP_BASE,
			((ps.rcPaint.bottom - ps.rcPaint.top) * nbmpCompatScale) / COMPAT_BMP_BASE,
			SRCCOPY
		);
		SetStretchBltMode(hdcOld, stretchModeOld);
	}
	::EndPaint(hwnd, &ps);
	return 0L;
}

LRESULT PrintPreview::OnSize(WPARAM wParam, LPARAM lParam)
{
	int	cx = LOWORD(lParam);
	int	cy = HIWORD(lParam);

	// ���Preview ����o�[
	int nToolBarHeight = 0;
	if (hwndPrintPreviewBar) {
		RECT			rc;
		::GetWindowRect(hwndPrintPreviewBar, &rc);
		nToolBarHeight = rc.bottom - rc.top;
		::MoveWindow(hwndPrintPreviewBar, 0, 0, cx, nToolBarHeight, TRUE);
	}

	// ���Preview �����X�N���[���o�[�E�B���h�E
	int	nCxVScroll = ::GetSystemMetrics(SM_CXVSCROLL);
	int	nCyVScroll = ::GetSystemMetrics(SM_CYVSCROLL);
	if (hwndVScrollBar) {
		::MoveWindow(hwndVScrollBar, cx - nCxVScroll, nToolBarHeight, nCxVScroll, cy - nCyVScroll - nToolBarHeight, TRUE);
	}
	
	// ���Preview �����X�N���[���o�[�E�B���h�E
	int	nCxHScroll = ::GetSystemMetrics(SM_CXHSCROLL);
	int	nCyHScroll = ::GetSystemMetrics(SM_CYHSCROLL);
	if (hwndHScrollBar) {
		::MoveWindow(hwndHScrollBar, 0, cy - nCyHScroll, cx - nCxVScroll, nCyHScroll, TRUE);
	}
	
	// ���Preview �T�C�Y�{�b�N�X�E�B���h�E
	if (hwndSizeBox) {
		::MoveWindow(hwndSizeBox, cx - nCxVScroll, cy - nCyHScroll, nCxHScroll, nCyVScroll, TRUE);
	}

	HDC hdc = ::GetDC(parentWnd.GetHwnd());
	int nMapModeOld = ::SetMapMode(hdc, MM_LOMETRIC);
	::SetMapMode(hdc, MM_ANISOTROPIC);

	// �o�͔{���̕ύX
	SIZE sz;
	::GetWindowExtEx(hdc, &sz);
	int nCx = sz.cx;
	int nCy = sz.cy;
	nCx = (int)(((long)nCx) * 100L / ((long)nPreview_Zoom));
	nCy = (int)(((long)nCy) * 100L / ((long)nPreview_Zoom));
	::SetWindowExtEx(hdc, nCx, nCy, &sz);

	// �r���[�̃T�C�Y
	POINT po;
	po.x = nPreview_PaperAllWidth + nPreview_ViewMarginLeft * 2;
	po.y = nPreview_PaperAllHeight + nPreview_ViewMarginTop * 2;
	::LPtoDP(hdc, &po, 1);

	// �ĕ`��p�������a�l�o
	if (hbmpCompatBMP) {
		::SelectObject(hdcCompatDC, hbmpCompatBMPOld);	// �ĕ`��p�������a�l�o(OLD)
		::DeleteObject(hbmpCompatBMP);
	}
	// 2007.02.11 Moca Preview�����炩�ɂ���
	// Win9x�ł� �����BMP�͍쐬�ł��Ȃ����Ƃ�
	// StretchBlt��STRETCH_HALFTONE�����T�|�[�g�ł���̂� Win2K �ȏ�݂̂ŗL���ɂ���B
	if (IsDlgButtonChecked( hwndPrintPreviewBar, IDC_CHECK_ANTIALIAS )
		&& IsWin2000_or_later()
	) {
		nbmpCompatScale = COMPAT_BMP_SCALE;
	}else {
		// Win9x: BASE = SCALE �� 1:1
		nbmpCompatScale = COMPAT_BMP_BASE;
	}
	hbmpCompatBMP = ::CreateCompatibleBitmap(hdc, (cx * nbmpCompatScale + COMPAT_BMP_BASE - 1) / COMPAT_BMP_BASE,
		(cy * nbmpCompatScale + COMPAT_BMP_BASE - 1) / COMPAT_BMP_BASE);
	hbmpCompatBMPOld = (HBITMAP)::SelectObject(hdcCompatDC, hbmpCompatBMP);

	::SetMapMode(hdc, nMapModeOld);

	::ReleaseDC(parentWnd.GetHwnd(), hdc);

	// ���Preview�F�r���[��(�s�N�Z��)
	nPreview_ViewWidth = abs(po.x);
	
	// ���Preview�F�r���[����(�s�N�Z��)
	nPreview_ViewHeight = abs(po.y);
	
	// ���Preview �X�N���[���o�[������
	InitPreviewScrollBar();
	
	// ���Preview �X�N���[���o�[�̏�����
	
	parentWnd.SetDragPosOrg(Point(0, 0));
	parentWnd.SetDragMode(true);
	OnMouseMove(0, MAKELONG(0, 0));
	parentWnd.SetDragMode(false);
	//	SizeBox���e�X�g
	if (hwndSizeBox) {
		if (wParam == SIZE_MAXIMIZED) {
			::ShowWindow(hwndSizeBox, SW_HIDE);
		}else
		if (wParam == SIZE_RESTORED) {
			if (::IsZoomed(parentWnd.GetHwnd())) {
				::ShowWindow(hwndSizeBox, SW_HIDE);
			}else {
				::ShowWindow(hwndSizeBox, SW_SHOW);
			}
		}else {
			::ShowWindow(hwndSizeBox, SW_SHOW);
		}
	}
	::InvalidateRect(parentWnd.GetHwnd(), NULL, TRUE);
	return 0L;
}

/*!
	@date 2006.08.14 Moca SB_TOP, SB_BOTTOM�ւ̑Ή�
*/
LRESULT PrintPreview::OnVScroll(WPARAM wParam, LPARAM lParam)
{
	int nScrollCode = (int) LOWORD(wParam);
	//nPos = (int) HIWORD(wParam);
	HWND hwndScrollBar = (HWND) lParam;
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	::GetScrollInfo(hwndScrollBar, SB_CTL, &si);
	int nPos = si.nTrackPos; // 2013.05.30 32bit�Ή�
	int nNowPos = -1 * nPreviewVScrollPos;
	int nNewPos = 0;
	int nMove = 0;
	switch (nScrollCode) {
	case SB_LINEUP:
		nMove = -1 * LINE_RANGE_Y;
		break;
	case SB_LINEDOWN:
		nMove = LINE_RANGE_Y;
		break;
	case SB_PAGEUP:
		nMove = -1 * PAGE_RANGE_Y;
		break;
	case SB_PAGEDOWN:
		nMove = PAGE_RANGE_Y;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		nMove = nPos - nNowPos;
		break;
	// 2006.08.14 Moca SB_TOP, SB_BOTTOM�ւ̑Ή�
	case SB_TOP:
		nMove = -1 * nNowPos;
		break;
	case SB_BOTTOM:
		nMove = si.nMax - nNowPos;
		break;
	default:
		return 0;
	}
	nNewPos = nNowPos + nMove;
	if (nNewPos < 0) {
		nNewPos = 0;
	}else
	if (nNewPos > (int)(si.nMax - si.nPage + 1)) {
		nNewPos = (int)(si.nMax - si.nPage + 1);
	}
	nMove = nNowPos - nNewPos;
	int nPreviewVScrollPos = -1 * nNewPos;
	if (this->nPreviewVScrollPos != nPreviewVScrollPos) {
		si.fMask = SIF_POS;
		si.nPos = nNewPos;
		::SetScrollInfo(hwndScrollBar, SB_CTL, &si, TRUE);
		this->nPreviewVScrollPos = nPreviewVScrollPos;
		// �`��
		::ScrollWindowEx(parentWnd.GetHwnd(), 0, nMove, NULL, NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE);
	}
	return 0;
}

/*!
	@date 2006.08.14 Moca SB_LEFT, SB_RIGHT�ւ̑Ή�
*/
LRESULT PrintPreview::OnHScroll(WPARAM wParam, LPARAM lParam)
{
	int nScrollCode = (int) LOWORD(wParam);
	//nPos = (int) HIWORD(wParam);
	HWND hwndScrollBar = (HWND) lParam;
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	::GetScrollInfo(hwndScrollBar, SB_CTL, &si);
	int nPos = si.nTrackPos; // 2013.05.30 32bit�Ή�
	//nNowPos = GetScrollPos���ƃ��W�N�[����SetPoint�ŕs�������AnPos == nNowPos�ɂȂ��Ă��܂�
	int nNowPos = nPreviewHScrollPos;
	int nMove = 0;
	switch (nScrollCode) {
	case SB_LINEUP:
		nMove = -1 * LINE_RANGE_Y;
		break;
	case SB_LINEDOWN:
		nMove = LINE_RANGE_Y;
		break;
	case SB_PAGEUP:
		nMove = -1 * PAGE_RANGE_Y;
		break;
	case SB_PAGEDOWN:
		nMove = PAGE_RANGE_Y;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		nMove = nPos - nNowPos;
		break;
	// 2006.08.14 Moca SB_LEFT, SB_RIGHT�ւ̑Ή�
	case SB_LEFT:
		nMove = -1 * nNowPos;
		break;
	case SB_RIGHT:
		nMove = si.nMax - nNowPos;
		break;
	default:
		return 0;
	}
	int nNewPos = nNowPos + nMove;
	if (nNewPos < 0) {
		nNewPos = 0;
	}else
	if (nNewPos > (int)(si.nMax - si.nPage + 1)) {
		nNewPos = (int)(si.nMax - si.nPage + 1);
	}
	nMove = nNowPos - nNewPos;
	int nPreviewHScrollPos = nNewPos;
	if (this->nPreviewHScrollPos != nPreviewHScrollPos) {
		si.fMask = SIF_POS;
		si.nPos = nNewPos;
		::SetScrollInfo(hwndScrollBar, SB_CTL, &si, TRUE);
		this->nPreviewHScrollPos = nPreviewHScrollPos;
		// �`��
		::ScrollWindowEx(parentWnd.GetHwnd(), nMove, 0, NULL, NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE);
	}
	return 0;
}

LRESULT PrintPreview::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	// ��J�[�\��
	SetHandCursor();		// Hand Cursor��ݒ� 2013/1/29 Uchi
	if (!parentWnd.GetDragMode()) {
		return 0;
	}
//	WPARAM fwKeys = wParam;			// key flags
	int xPos = LOWORD(lParam);	// horizontal position of cursor
	int yPos = HIWORD(lParam);	// vertical position of cursor
	RECT rc;
	GetClientRect(parentWnd.GetHwnd(), &rc);
	POINT po;
	po.x = xPos;
	po.y = yPos;
	if (!PtInRect(&rc, po)) {	//	Preview�����`�F�b�N�B
		return 0;
	}

	//	Y��
	SCROLLINFO siV;
	siV.cbSize = sizeof(siV);
	siV.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	GetScrollInfo(hwndVScrollBar, SB_CTL, &siV);
	int nMoveY;
	if (SCROLLBAR_VERT) {
		int nNowPosY = siV.nTrackPos;
		nMoveY = parentWnd.GetDragPosOrg().y - yPos;

		int nNewPosY = nNowPosY + nMoveY;
		if (nNewPosY < 0) {
			nNewPosY = 0;
		}else
		if (nNewPosY > (int)(siV.nMax - siV.nPage + 1)) {
			nNewPosY = (int)(siV.nMax - siV.nPage + 1);
		}
		nMoveY = nNowPosY - nNewPosY;
		siV.fMask = SIF_POS;
		siV.nPos = nNewPosY;
		SetScrollInfo(hwndVScrollBar, SB_CTL, &siV, TRUE);
		nPreviewVScrollPos = -1 * nNewPosY;
	}else {
		nMoveY = 0;
	}

	//	X��
	SCROLLINFO siH;
	siH.cbSize = sizeof(siH);
	siH.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	GetScrollInfo(hwndHScrollBar, SB_CTL, &siH);
	int nMoveX;
	if (SCROLLBAR_HORZ) {
		int nNowPosX = siH.nTrackPos;
		nMoveX = parentWnd.GetDragPosOrg().x - xPos;
		
		int nNewPosX = nNowPosX + nMoveX;
		if (nNewPosX < 0) {
			nNewPosX = 0;
		}else
		if (nNewPosX > (int)(siH.nMax - siH.nPage + 1)) {
			nNewPosX = (int)(siH.nMax - siH.nPage + 1);
		}
		nMoveX = nNowPosX - nNewPosX;
		siH.fMask = SIF_POS;
		siH.nPos = nNewPosX;
		SetScrollInfo(hwndHScrollBar, SB_CTL, &siH, TRUE);
		nPreviewHScrollPos = nNewPosX;
	}else {
		nMoveX = 0;
	}

	parentWnd.SetDragPosOrg(Point(xPos, yPos));
	// �`��
	ScrollWindowEx(parentWnd.GetHwnd(), nMoveX, nMoveY, NULL, NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE);
	return 0;
}

LRESULT PrintPreview::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
//	WORD	fwKeys = LOWORD(wParam);			// key flags
	short	zDelta = (short) HIWORD(wParam);	// wheel rotation
//	short	xPos = (short) LOWORD(lParam);		// horizontal position of pointer
//	short	yPos = (short) HIWORD(lParam);		// vertical position of pointer

	int nScrollCode;
	if (0 < zDelta) {
		nScrollCode = SB_LINEUP;
	}else {
		nScrollCode = SB_LINEDOWN;
	}

	for (int i=0; i<3; ++i) {
		// ���Preview �����X�N���[���o�[���b�Z�[�W���� WM_VSCROLL
		::PostMessage(parentWnd.GetHwnd(), WM_VSCROLL, MAKELONG(nScrollCode, 0), (LPARAM)hwndVScrollBar);

		// �������̃��[�U�[������\�ɂ���
		if (!::BlockingHook(NULL)) {
			return -1;
		}
	}
	return 0;
}

void PrintPreview::OnChangeSetting()
{
	if (bLockSetting) {
		bDemandUpdateSetting = true;
		return;
	}
	bDemandUpdateSetting = false;
	*pPrintSetting = *pPrintSettingOrg;
	OnChangePrintSetting();
}

void PrintPreview::OnChangePrintSetting(void)
{
	HDC hdc = ::GetDC(parentWnd.GetHwnd());
	::SetMapMode(hdc, MM_LOMETRIC); // MM_HIMETRIC ���ꂼ��̘_���P�ʂ́A0.01 mm �Ƀ}�b�v����܂�
	::SetMapMode(hdc, MM_ANISOTROPIC);

	::EnumFontFamilies(
		hdc,
		NULL,
		(FONTENUMPROC)PrintPreview::MyEnumFontFamProc,
		(LPARAM)this
	);

	bool bLockOld = bLockSetting;
	bLockSetting = true;

	// 2009.08.08 ����ŗp���T�C�Y�A���w�肪�����Ȃ����Ή� syat
	// DEVMODE�\���̂��ݒ肳��Ă��Ȃ����������̃v�����^��ݒ�
	if (pPrintSetting->mdmDevMode.szPrinterDeviceName[0] == L'\0') {
		GetDefaultPrinterInfo();
	}

	// ���Preview�\�����
	nPreview_LineNumberColumns = 0;	// �s�ԍ��G���A�̕�(������)

	// �s�ԍ���\�����邩
	if (pPrintSetting->bPrintLineNumber) {
		// �s�ԍ��\���ɕK�v�Ȍ������v�Z
		nPreview_LineNumberColumns = parentWnd.GetActiveView().GetTextArea().DetectWidthOfLineNumberArea_calculate(pLayoutMgr_Print);
	}
	// ���݂̃y�[�W�ݒ�́A�p���T�C�Y�Ɨp�������𔽉f������
	pPrintSetting->mdmDevMode.dmPaperSize = pPrintSetting->nPrintPaperSize;
	pPrintSetting->mdmDevMode.dmOrientation = pPrintSetting->nPrintPaperOrientation;
	// �p���T�C�Y�A�p�������͕ύX�����̂Ńr�b�g�𗧂Ă�
	pPrintSetting->mdmDevMode.dmFields |= (DM_ORIENTATION | DM_PAPERSIZE);
	// �p���̒����A���͌��܂��Ă��Ȃ��̂ŁA�r�b�g�����낷
	pPrintSetting->mdmDevMode.dmFields &= (~DM_PAPERLENGTH);
	pPrintSetting->mdmDevMode.dmFields &= (~DM_PAPERWIDTH);

	// ���/Preview�ɕK�v�ȏ����擾
	TCHAR szErrMsg[1024];
	if (!print.GetPrintMetrics(
		&pPrintSetting->mdmDevMode,	// �v�����^�ݒ� DEVMODE�p
		&nPreview_PaperAllWidth,		// �p����
		&nPreview_PaperAllHeight,		// �p������
		&nPreview_PaperWidth,			// �p������L����
		&nPreview_PaperHeight,		// �p������L������
		&nPreview_PaperOffsetLeft,	// ����\�ʒu���[
		&nPreview_PaperOffsetTop,		// ����\�ʒu��[
		szErrMsg						// �G���[���b�Z�[�W�i�[�ꏊ
		)
	) {
		// �G���[�̏ꍇ�AA4�c(210mm�~297mm)�ŏ�����
		nPreview_PaperAllWidth = 210 * 10;	// �p����
		nPreview_PaperAllHeight = 297 * 10;	// �p������
		nPreview_PaperWidth = 210 * 10;		// �p������L����
		nPreview_PaperHeight = 297 * 10;		// �p������L������
		nPreview_PaperOffsetLeft = 0;			// ����\�ʒu���[
		nPreview_PaperOffsetTop = 0;			// ����\�ʒu��[
		// DEVMODE�\���̂�A4�c�ŏ����� 2003.07.03 �����
		pPrintSetting->mdmDevMode.dmPaperSize = DMPAPER_A4;
		pPrintSetting->mdmDevMode.dmOrientation = DMORIENT_PORTRAIT;
		pPrintSetting->mdmDevMode.dmPaperLength = nPreview_PaperHeight;
		pPrintSetting->mdmDevMode.dmPaperWidth = nPreview_PaperWidth;
		pPrintSetting->mdmDevMode.dmFields |= (DM_ORIENTATION | DM_PAPERSIZE | DM_PAPERLENGTH | DM_PAPERWIDTH);
	}else {
		if (pPrintSetting->nPrintPaperSize != pPrintSetting->mdmDevMode.dmPaperSize) {
			TCHAR szPaperNameOld[256];
			TCHAR szPaperNameNew[256];
			// �p���̖��O���擾
			Print::GetPaperName(pPrintSetting->nPrintPaperSize , szPaperNameOld);
			Print::GetPaperName(pPrintSetting->mdmDevMode.dmPaperSize , szPaperNameNew);

			TopWarningMessage(
				parentWnd.GetHwnd(),
				LS(STR_ERR_DLGPRNPRVW3),
				pPrintSetting->mdmDevMode.szPrinterDeviceName,
				szPaperNameOld,
				szPaperNameNew
			);
		}
	}
	// ���݂̃y�[�W�ݒ�́A�p���T�C�Y�Ɨp�������𔽉f������(�G���[��A4�c�ɂȂ����ꍇ���l������if���̊O�ֈړ� 2003.07.03 �����)
	pPrintSetting->nPrintPaperSize = pPrintSetting->mdmDevMode.dmPaperSize;
	pPrintSetting->nPrintPaperOrientation = pPrintSetting->mdmDevMode.dmOrientation;	// �p�������̔��f�Y����C�� 2003/07/03 �����

	// �v�����^�ݒ�͂����ŕύX����邪���ꂼ��̃E�B���h�E�ōĐݒ肷��̂ōX�V���b�Z�[�W�͓����Ȃ�
	*pPrintSettingOrg = *pPrintSetting;

	nPreview_ViewMarginLeft = 8 * 10;		// ���Preview�F�r���[���[�Ɨp���̊Ԋu(1/10mm�P��)
	nPreview_ViewMarginTop = 8 * 10;		// ���Preview�F�r���[���[�Ɨp���̊Ԋu(1/10mm�P��)

	// �s������̕�����(�s�ԍ�����)
	bPreview_EnableColumns = Print::CalculatePrintableColumns(*pPrintSetting, nPreview_PaperAllWidth, nPreview_LineNumberColumns);	// �󎚉\����/�y�[�W
	// �c�����̍s��
	bPreview_EnableLines = Print::CalculatePrintableLines(*pPrintSetting, nPreview_PaperAllHeight);			// �󎚉\�s��/�y�[�W

	// �󎚉\�̈悪�Ȃ��ꍇ�͈��Preview���I������ 2013.5.10 aroka
	if (bPreview_EnableColumns == 0 || bPreview_EnableLines == 0) {
		parentWnd.PrintPreviewModeONOFF();
		parentWnd.SendStatusMessage(LS(STR_ERR_DLGPRNPRVW3_1));
		return;
	}

	// ����p�̃��C�A�E�g�Ǘ����̏�����
	pLayoutMgr_Print->Create(&parentWnd.GetDocument(), &parentWnd.GetDocument().docLineMgr);

	// ����p�̃��C�A�E�g���̕ύX
	// �^�C�v�ʐݒ���R�s�[
	typePrint = parentWnd.GetDocument().docType.GetDocumentAttribute();
	TypeConfig& ref = typePrint;

	ref.nMaxLineKetas = 	bPreview_EnableColumns;
	ref.bWordWrap =		pPrintSetting->bPrintWordWrap;	// �p�����[�h���b�v������
	//	Sep. 23, 2002 genta LayoutMgr�̒l���g��
	ref.nTabSpace =		parentWnd.GetDocument().layoutMgr.GetTabSpace();

	//@@@ 2002.09.22 YAZAKI
	ref.lineComment.CopyTo(0, L"", -1);	// �s�R�����g�f���~�^
	ref.lineComment.CopyTo(1, L"", -1);	// �s�R�����g�f���~�^2
	ref.lineComment.CopyTo(2, L"", -1);	// �s�R�����g�f���~�^3	// Jun. 01, 2001 JEPRO �ǉ�
	ref.blockComments[0].SetBlockCommentRule(L"", L"");	// �u���b�N�R�����g�f���~�^
	ref.blockComments[1].SetBlockCommentRule(L"", L"");	// �u���b�N�R�����g�f���~�^2

	ref.stringType = StringLiteralType::CPP;		// �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""]['']
	ref.colorInfoArr[COLORIDX_COMMENT].bDisp = false;
	ref.colorInfoArr[COLORIDX_SSTRING].bDisp = false;
	ref.colorInfoArr[COLORIDX_WSTRING].bDisp = false;
	ref.bKinsokuHead = pPrintSetting->bPrintKinsokuHead,	// �s���֑�����	//@@@ 2002.04.08 MIK
	ref.bKinsokuTail = pPrintSetting->bPrintKinsokuTail,	// �s���֑�����	//@@@ 2002.04.08 MIK
	ref.bKinsokuRet = pPrintSetting->bPrintKinsokuRet,	// ���s�������Ԃ牺����	//@@@ 2002.04.13 MIK
	ref.bKinsokuKuto = pPrintSetting->bPrintKinsokuKuto,	// ��Ǔ_���Ԃ牺����	//@@@ 2002.04.17 MIK
	pLayoutMgr_Print->SetLayoutInfo(true, ref, ref.nTabSpace, ref.nMaxLineKetas);
	nAllPageNum = (WORD)(pLayoutMgr_Print->GetLineCount() / (bPreview_EnableLines * pPrintSetting->nPrintDansuu));		// �S�y�[�W��
	if (0 < pLayoutMgr_Print->GetLineCount() % (bPreview_EnableLines * pPrintSetting->nPrintDansuu)) {
		++nAllPageNum;
	}
	if (nAllPageNum <= nCurPageNum) {	// ���݂̃y�[�W
		nCurPageNum = 0;
	}

	// WM_SIZE ����
	RECT rc;
	::GetClientRect(parentWnd.GetHwnd(), &rc);
	OnSize(SIZE_RESTORED, MAKELONG(rc.right - rc.left, rc.bottom - rc.top));
	::ReleaseDC(parentWnd.GetHwnd(), hdc);
	// Preview �y�[�W�w��
	OnPreviewGoPage(nCurPageNum);
	bLockSetting = bLockOld;

	// 2014.07.23 ���C�A�E�g�s�ԍ��ōs�ԍ���������Ȃ����͍Čv�Z
	if (pPrintSetting->bPrintLineNumber) {
		// �s�ԍ��\���ɕK�v�Ȍ������v�Z
		int tempLineNum = parentWnd.GetActiveView().GetTextArea().DetectWidthOfLineNumberArea_calculate(pLayoutMgr_Print);
		if (nPreview_LineNumberColumns != tempLineNum) {
			OnChangeSetting();
		}
	}
	if (bDemandUpdateSetting) {
		// ���Ȃ���
		OnChangeSetting();
	}
	return;
}

/*! @brief �y�[�W�ԍ����ڎw��ɂ��W�����v

	@author Moca
**/
void PrintPreview::OnPreviewGoDirectPage(void)
{
	const int INPUT_PAGE_NUM_LEN = 12;

	DlgInput1 dlgInputPage;
	TCHAR szMessage[512];
	TCHAR szPageNum[INPUT_PAGE_NUM_LEN];
	
	auto_sprintf(szMessage, LS(STR_ERR_DLGPRNPRVW4) , nAllPageNum);
	auto_sprintf(szPageNum, _T("%d"), nCurPageNum + 1);

	BOOL bDlgInputPageResult = dlgInputPage.DoModal(
		EditApp::getInstance().GetAppInstance(),
		hwndPrintPreviewBar, 
		LS(STR_ERR_DLGPRNPRVW5),
		szMessage,
		INPUT_PAGE_NUM_LEN,
		szPageNum
	);
	if (bDlgInputPageResult) {
		size_t nPageNumLen = _tcslen(szPageNum);
		for (size_t i=0; i<nPageNumLen; ++i) {
			if (!(_T('0') <= szPageNum[i] &&  szPageNum[i] <= _T('9'))) {
				return;
			}
		}
		int nPage = _ttoi(szPageNum);
		OnPreviewGoPage(nPage - 1);
	}
}

void PrintPreview::OnPreviewGoPage(int nPage)
{
	if (nAllPageNum <= nPage) {	// ���݂̃y�[�W
		nPage = nAllPageNum - 1;
	}
	if (0 > nPage) {				// ���݂̃y�[�W
		nPage = 0;
	}
	nCurPageNum = (short)nPage;

	//	2008.01.29 nasukoji	���������2���̎�����ł��Ȃ��Ȃ邱�Ƃւ̑Ώ��iSetFocus���ړ��j
	//	2008.02.01 genta : �{�^���̃t�H�[�J�X�����̓���ɂȂ�悤�ɂ��邽�߁C
	//		�O�{�^����Disable�����ֈړ������D
	//		����ł��Ȃ����ۂ́u���ցv��Disable�ɂ��ւ�炸�t�H�[�J�X��^���Ă������߁D
	//		���E�O�ǂ�����C�{�^���L�������t�H�[�J�X�ړ����{�^���������̏��ɂ���
	if (0 < nCurPageNum) {
		//	�O�̃y�[�W�{�^�����I��
		::EnableWindow(::GetDlgItem(hwndPrintPreviewBar, IDC_BUTTON_PREVPAGE), TRUE);
	}

	if (nAllPageNum <= nCurPageNum + 1) {
		//	�Ō�̃y�[�W�̂Ƃ��́A���̃y�[�W�{�^�����I�t�B
		//	Jul. 18, 2001 genta Focus�̂���Window��Disable�ɂ���Ƒ���ł��Ȃ��Ȃ�̂����
		//	Mar. 9, 2003 genta 1�y�[�W���������Ƃ��́u�O�ցv�{�^����Disable����Ă���̂ŁA
		//	�Ō�̃y�[�W�܂ŒB������u�߂�v�Ƀt�H�[�J�X���ڂ��悤��
		::SetFocus(::GetDlgItem(hwndPrintPreviewBar, IDCANCEL));
		::EnableWindow(::GetDlgItem(hwndPrintPreviewBar, IDC_BUTTON_NEXTPAGE), FALSE);
	}else {
		//	���̃y�[�W�{�^�����I���B
		::EnableWindow(::GetDlgItem(hwndPrintPreviewBar, IDC_BUTTON_NEXTPAGE), TRUE);
	}

	if (nCurPageNum == 0) {
		//	�ŏ��̃y�[�W�̂Ƃ��́A�O�̃y�[�W�{�^�����I�t�B
		//	Jul. 18, 2001 genta Focus�̂���Window��Disable�ɂ���Ƒ���ł��Ȃ��Ȃ�̂����
		::SetFocus(::GetDlgItem(hwndPrintPreviewBar, IDC_BUTTON_NEXTPAGE));
		::EnableWindow(::GetDlgItem(hwndPrintPreviewBar, IDC_BUTTON_PREVPAGE), FALSE);
	}
	wchar_t	szEdit[1024];
	auto_sprintf(szEdit, LSW(STR_ERR_DLGPRNPRVW6), nCurPageNum + 1, nAllPageNum);
	::DlgItem_SetText(hwndPrintPreviewBar, IDC_STATIC_PAGENUM, szEdit);

	auto_sprintf(szEdit, L"%d %%", nPreview_Zoom);
	::DlgItem_SetText(hwndPrintPreviewBar, IDC_STATIC_ZOOM, szEdit);

	::InvalidateRect(parentWnd.GetHwnd(), NULL, TRUE);
	return;
}

void PrintPreview::OnPreviewZoom(BOOL bZoomUp)
{
	if (bZoomUp) {
		nPreview_Zoom += 10;	// ���Preview�{��
		if (MAX_PREVIEW_ZOOM < nPreview_Zoom) {
			nPreview_Zoom = MAX_PREVIEW_ZOOM;
		}
	}else {
		// �X�N���[���ʒu�𒲐�
		nPreviewVScrollPos = 0;
		nPreviewHScrollPos = 0;

		nPreview_Zoom -= 10;	// ���Preview�{��
		if (MIN_PREVIEW_ZOOM > nPreview_Zoom) {
			nPreview_Zoom = MIN_PREVIEW_ZOOM;
		}
	}
	
	//	�k���{�^����ON/OFF
	if (nPreview_Zoom == MIN_PREVIEW_ZOOM) {
		// 2013.05.30 Focus��Disable�ȃE�B���h�E���ƃ}�E�X�X�N���[���ł��Ȃ��΍�
		HWND focus = ::GetFocus();
		if (focus == GetDlgItem(hwndPrintPreviewBar, IDC_BUTTON_ZOOMDOWN)) {
			::SetFocus(parentWnd.GetHwnd());
		}
		::EnableWindow(::GetDlgItem(hwndPrintPreviewBar, IDC_BUTTON_ZOOMDOWN), FALSE);
	}else {
		::EnableWindow(::GetDlgItem(hwndPrintPreviewBar, IDC_BUTTON_ZOOMDOWN), TRUE);
	}
	//	�g��{�^����ON/OFF
	if (nPreview_Zoom == MAX_PREVIEW_ZOOM) {
		// 2013.05.30 Focus��Disable�ȃE�B���h�E���ƃ}�E�X�X�N���[���ł��Ȃ��΍�
		HWND focus = ::GetFocus();
		if (focus == GetDlgItem(hwndPrintPreviewBar, IDC_BUTTON_ZOOMUP)) {
			::SetFocus(parentWnd.GetHwnd());
		}
		::EnableWindow(::GetDlgItem(hwndPrintPreviewBar, IDC_BUTTON_ZOOMUP), FALSE);
	}else {
		::EnableWindow(::GetDlgItem(hwndPrintPreviewBar, IDC_BUTTON_ZOOMUP), TRUE);
	}

	wchar_t	szEdit[1024];
	auto_sprintf(szEdit, L"%d %%", nPreview_Zoom);
	::DlgItem_SetText(hwndPrintPreviewBar, IDC_STATIC_ZOOM, szEdit);

	// WM_SIZE ����
	RECT rc1;
	::GetClientRect(parentWnd.GetHwnd(), &rc1);
	OnSize(SIZE_RESTORED, MAKELONG(rc1.right - rc1.left, rc1.bottom - rc1.top));

	// ���Preview �X�N���[���o�[������
	InitPreviewScrollBar();

	// �ĕ`��
	::InvalidateRect(parentWnd.GetHwnd(), NULL, TRUE);
	return;
}


/*!
	���炩
	�`�F�b�N���A2�{(COMPAT_BMP_SCALE/COMPAT_BMP_BASE)�T�C�Y�Ń����_�����O����
*/
void PrintPreview::OnCheckAntialias(void)
{
	// WM_SIZE ����
	RECT rc;
	::GetClientRect(parentWnd.GetHwnd(), &rc);
	OnSize(SIZE_RESTORED, MAKELONG(rc.right - rc.left, rc.bottom - rc.top));
}


/*!
	���
*/
void PrintPreview::OnPrint(void)
{
	if (nAllPageNum == 0) {
		TopWarningMessage(parentWnd.GetHwnd(), LS(STR_ERR_DLGPRNPRVW7));
		return;
	}

	// �v�����^�ɓn���W���u���𐶐�
	TCHAR szJobName[256 + 1];
	if (!parentWnd.GetDocument().docFile.GetFilePathClass().IsValidPath()) {	// ���ݕҏW���̃t�@�C���̃p�X
		_tcscpy_s(szJobName, LS(STR_NO_TITLE2));
	}else {
		TCHAR szFileName[_MAX_FNAME];
		TCHAR szExt[_MAX_EXT];
		_tsplitpath(parentWnd.GetDocument().docFile.GetFilePath(), NULL, NULL, szFileName, szExt);
		auto_snprintf_s(szJobName, _countof(szJobName), _T("%ts%ts"), szFileName, szExt);
	}

	// ����͈͂��w��ł���v�����^�_�C�A���O���쐬
	//	2003.05.02 �����
	PRINTDLG pd = {0};
#ifndef _DEBUG
// Debug���[�h�ŁAhwndOwner���w�肷��ƁAWin2000�ł͗�����̂ŁE�E�E
	pd.hwndOwner = parentWnd.GetHwnd();
#endif
	pd.nMinPage = 1;
	pd.nMaxPage = nAllPageNum;
	pd.nFromPage = 1;
	pd.nToPage = nAllPageNum;
	pd.Flags = PD_ALLPAGES | PD_NOSELECTION | PD_USEDEVMODECOPIESANDCOLLATE;

	bLockSetting = true; // �v�����g�ݒ�Ńy�[�W�������܂�̂Ń��b�N����

	if (!print.PrintDlg(&pd, &pPrintSetting->mdmDevMode)) {
		bLockSetting = false;
		if (bDemandUpdateSetting) {
			OnChangePrintSetting();
		}
		return;
	}
	if (memcmp(&pPrintSettingOrg->mdmDevMode, &pPrintSetting->mdmDevMode, sizeof(pPrintSetting->mdmDevMode)) != 0) {
		pPrintSettingOrg->mdmDevMode = pPrintSetting->mdmDevMode;
		// ������Lock�ōX�V���Ȃ�
		AppNodeGroupHandle(0).PostMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0,
			(LPARAM)PM_PrintSetting,
			EditWnd::getInstance().GetHwnd()
		);
	}

	// ����J�n�y�[�W�ƁA����y�[�W�����m�F
	WORD nFrom;
	WORD nNum;
	if ((pd.Flags & PD_PAGENUMS) != 0) {	// 2003.05.02 �����
		nFrom = pd.nFromPage - 1;
		nNum  = pd.nToPage - nFrom;
	}else {
		nFrom = 0;
		nNum  = nAllPageNum;
	}

	// ����ߒ���\�����āA�L�����Z�����邽�߂̃_�C�A���O���쐬
	DlgCancel	dlgPrinting;
	dlgPrinting.DoModeless(EditApp::getInstance().GetAppInstance(), parentWnd.GetHwnd(), IDD_PRINTING);
	dlgPrinting.SetItemText(IDC_STATIC_JOBNAME, szJobName);
	dlgPrinting.SetItemText(IDC_STATIC_PROGRESS, _T(""));	// XPS�Ή� 2013/5/8 Uchi

	// �e�E�B���h�E�𖳌���
	::EnableWindow(parentWnd.GetHwnd(), FALSE);

	// 2013.06.10 Moca �L�[���[�h�����ݒ�����b�N���āA������ɋ��ʐݒ���X�V����Ȃ��悤�ɂ���
	ShareDataLockCounter lock;

	// ��� �W���u�J�n
	HDC hdc;
	TCHAR szErrMsg[1024];
	if (!print.PrintOpen(
		szJobName,
		&pPrintSetting->mdmDevMode,	// �v�����^�ݒ� DEVMODE�p
		&hdc,
		szErrMsg						// �G���[���b�Z�[�W�i�[�ꏊ
		)
	) {
//		MYTRACE(_T("%ts\n"), szErrMsg);
	}

	// ����p���p�t�H���g�ƁA����p�S�p�t�H���g���쐬
	CreateFonts(hdc);
	// ���݂̃t�H���g������p���p�t�H���g�ɐݒ聕�ȑO�̃t�H���g��ێ�
	// OnPrint�ȑO�̃t�H���g
	HFONT hFontOld = (HFONT)::SelectObject(hdc, hFontHan);

	// ���̑傫��������킷RECT��ݒ�
	int nDirectY = -1;
	RECT rect;
	rect.left   =                             pPrintSetting->nPrintMarginLX - nPreview_PaperOffsetLeft + 5;
	rect.right  = nPreview_PaperAllWidth - (pPrintSetting->nPrintMarginRX + nPreview_PaperOffsetLeft + 5);
	rect.top    = nDirectY * (                             pPrintSetting->nPrintMarginTY - nPreview_PaperOffsetTop + 5);
	rect.bottom = nDirectY * (nPreview_PaperAllHeight - (pPrintSetting->nPrintMarginBY + nPreview_PaperOffsetTop + 5));

	// �w�b�_�E�t�b�^��$p��W�J���邽�߂ɁAnCurPageNum��ێ�
	WORD nCurPageNumOld = nCurPageNum;
	ColorStrategy* pStrategy = DrawPageTextFirst(nCurPageNum);
	TCHAR szProgress[100];
	for (int i=0; i<nNum; ++i) {
		nCurPageNum = nFrom + (WORD)i;

		// ����ߒ���\��
		//	Jun. 18, 2001 genta �y�[�W�ԍ��\���̌v�Z�~�X�C��
		auto_sprintf_s(szProgress, _T("%d/%d"), i + 1, nNum);
		dlgPrinting.SetItemText(IDC_STATIC_PROGRESS, szProgress);

		// ��� �y�[�W�J�n
		print.PrintStartPage(hdc);

		//	From Here Jun. 26, 2003 ����� / ������
		//	Windows 95/98�ł�StartPage()�֐��̌Ăяo�����ɁA�����̓��Z�b�g����Ċ���l�֖߂�܂��D
		//	���̂Ƃ��J���҂͎��̃y�[�W�̈�����n�߂�O�ɃI�u�W�F�N�g��I���������C
		//	�}�b�s���O���[�h��������x�ݒ肵�Ȃ���΂Ȃ�܂���
		//	Windows NT/2000�ł�StartPage�ł������̓��Z�b�g����܂���D

		// �}�b�s���O���[�h�̕ύX
		::SetMapMode(hdc, MM_LOMETRIC);		// ���ꂼ��̘_���P�ʂ́A0.1 mm �Ƀ}�b�v����܂�
		::SetMapMode(hdc, MM_ANISOTROPIC);	// �_���P�ʂ́A�C�ӂɃX�P�[�����O���ꂽ����̔C�ӂ̒P�ʂɃ}�b�v����܂�

		// ���݂̃t�H���g������p���p�t�H���g�ɐݒ�
		::SelectObject(hdc, hFontHan);
		//	To Here Jun. 26, 2003 ����� / ������

		int nHeaderHeight = Print::CalcHeaderHeight(*pPrintSetting);

		// �w�b�_���
		if (nHeaderHeight) {
			DrawHeaderFooter(hdc, rect, true);
		}

		const int nPageTopLineNum = ((nFrom + i) * pPrintSetting->nPrintDansuu) * bPreview_EnableLines;
		const Layout*		pPageTopLayout = pLayoutMgr_Print->SearchLineByLayoutY(nPageTopLineNum);
		if (pPrintSetting->bColorPrint
			&& !(i == 0)
			&& pPageTopLayout->GetLogicOffset() == 0
		) {
			pStrategy = pool->GetStrategyByColor(pPageTopLayout->GetColorTypePrev());
			pool->NotifyOnStartScanLogic();
			if (pStrategy) {
				pStrategy->InitStrategyStatus();
				pStrategy->SetStrategyColorInfo(pPageTopLayout->GetColorInfo());
			}
		}
		// ���/���Preview �y�[�W�e�L�X�g�̕`��
		pStrategy = DrawPageText(
			hdc,
			pPrintSetting->nPrintMarginLX - nPreview_PaperOffsetLeft ,
			pPrintSetting->nPrintMarginTY - nPreview_PaperOffsetTop + nHeaderHeight*2,
			nFrom + i,
			&dlgPrinting,
			pStrategy
		);

		// �t�b�^���
		if (Print::CalcFooterHeight(*pPrintSetting)) {
			DrawHeaderFooter(hdc, rect, false);
		}

		// ��� �y�[�W�I��
		print.PrintEndPage(hdc);

		// ���f�{�^�������`�F�b�N
		if (dlgPrinting.IsCanceled()) {
			break;
		}
	}
	//	����O�̃t�H���g�ɖ߂� 2003.05.02 ����� hdc����̑O�ɏ���������ύX
	::SelectObject(hdc, hFontOld);

	// ��� �W���u�I��
	print.PrintClose(hdc);

	//	����p�t�H���g�j��
	DestroyFonts();

	::EnableWindow(parentWnd.GetHwnd(), TRUE);
	dlgPrinting.CloseDialog(0);

	nCurPageNum = nCurPageNumOld;

	bLockSetting = false;

	// ������I�������APreview���甲���� 2003.05.02 �����
	parentWnd.PrintPreviewModeONOFF();
	return;
}


// Tab������Space�����ɒu����
static void Tab2Space(wchar_t* pTrg)
{
	for (; *pTrg!=L'\0'; ++pTrg) {
		if (*pTrg == L'\t') {
			*pTrg = L' ';
		}
	}
}


/*! ���/���Preview �w�b�_��t�b�^�̕`��
*/
void PrintPreview::DrawHeaderFooter(HDC hdc, const Rect& rect, bool bHeader)
{
	bool		bFontSetting = (bHeader ? pPrintSetting->lfHeader.lfFaceName[0] : pPrintSetting->lfFooter.lfFaceName[0]) != _T('\0');
	const int	nWorkLen = 1024;
	wchar_t		szWork[1024 + 1];
	size_t		nLen;

	if (bFontSetting) {
		// �t�H���g�쐬
		LOGFONT	lf = (bHeader ? pPrintSetting->lfHeader : pPrintSetting->lfFooter);
		lf.lfHeight = -(bHeader ? pPrintSetting->nHeaderPointSize : pPrintSetting->nFooterPointSize) * 254 / 720;	// �t�H���g�̃T�C�Y�v�Z(pt->1/10mm)
		HFONT hFontForce = ::CreateFontIndirect(&lf);

		// �t�H���g�ݒ�
		HFONT hFontOld = (HFONT)::SelectObject(hdc, hFontForce);

		// TextMetric�̎擾
		TEXTMETRIC tm;
		::GetTextMetrics(hdc, &tm);

		// Y���W�
		int nY = bHeader ? rect.top : (rect.bottom + tm.tmHeight);

		// ����
		SakuraEnvironment::ExpandParameter(
			bHeader ? pPrintSetting->szHeaderForm[POS_LEFT] : pPrintSetting->szFooterForm[POS_LEFT],
			szWork, nWorkLen);
		Tab2Space(szWork);
		::ExtTextOutW_AnyBuild(
			hdc,
			rect.left,
			nY,
			0,
			NULL,
			szWork,
			wcslen(szWork),
			NULL
		);

		// ������
		SakuraEnvironment::ExpandParameter(
			bHeader ? pPrintSetting->szHeaderForm[POS_CENTER] : pPrintSetting->szFooterForm[POS_CENTER],
			szWork, nWorkLen);
		Tab2Space(szWork);
		SIZE size;
		nLen = wcslen(szWork);
		::GetTextExtentPoint32W(hdc, szWork, (int)nLen, &size);		// �e�L�X�g��
		::ExtTextOutW_AnyBuild(
			hdc,
			(rect.right + rect.left - size.cx) / 2,
			nY,
			0,
			NULL,
			szWork,
			nLen,
			NULL
		);

		// �E��
		SakuraEnvironment::ExpandParameter(
			bHeader ? pPrintSetting->szHeaderForm[POS_RIGHT] : pPrintSetting->szFooterForm[POS_RIGHT],
			szWork, nWorkLen);
		Tab2Space(szWork);
		nLen = wcslen(szWork);
		::GetTextExtentPoint32W(hdc, szWork, (int)nLen, &size);		// �e�L�X�g��
		::ExtTextOutW_AnyBuild(
			hdc,
			rect.right - size.cx,
			nY,
			0,
			NULL,
			szWork,
			nLen,
			NULL
		);
		// �t�H���g�̖߂�
		::SelectObject(hdc, hFontOld);
		::DeleteObject(hFontForce);
	}else {
		// �����Ԋu
		int nDx = pPrintSetting->nPrintFontWidth;

		// Y���W�
		int nY = bHeader ? rect.top : (rect.bottom + pPrintSetting->nPrintFontHeight);

		// ����
		SakuraEnvironment::ExpandParameter(
			bHeader ? pPrintSetting->szHeaderForm[POS_LEFT] : pPrintSetting->szFooterForm[POS_LEFT],
			szWork, nWorkLen);
		nLen = wcslen(szWork);
		Print_DrawLine(
			hdc,
			Point(
				rect.left,
				nY
			),
			szWork,
			nLen,
			0,
			nLen,
			0
		);

		// ������
		SakuraEnvironment::ExpandParameter(
			bHeader ? pPrintSetting->szHeaderForm[POS_CENTER] : pPrintSetting->szFooterForm[POS_CENTER],
			szWork, nWorkLen);
		nLen = wcslen(szWork);
		size_t nTextWidth = TextMetrics::CalcTextWidth2(szWork, nLen, nDx); // �e�L�X�g��
		Print_DrawLine(
			hdc,
			Point(
				(rect.right + rect.left - (int)nTextWidth) / 2,
				nY
			),
			szWork,
			nLen,
			0,
			nLen,
			0
		);

		// �E��
		SakuraEnvironment::ExpandParameter(
			bHeader ? pPrintSetting->szHeaderForm[POS_RIGHT] : pPrintSetting->szFooterForm[POS_RIGHT],
			szWork, nWorkLen);
		nLen = wcslen(szWork);
		nTextWidth = TextMetrics::CalcTextWidth2(szWork, nLen, nDx); // �e�L�X�g��
		Print_DrawLine(
			hdc,
			Point(
				rect.right - (int)nTextWidth,
				nY
			),
			szWork,
			nLen,
			0,
			nLen,
			0
		);
	}
}

/* ���/���Preview �y�[�W�e�L�X�g�̐F��������
	�ŏ��̃y�[�W�p
	@date 2013.05.19 Moca �V�K�ǉ� 
*/
ColorStrategy* PrintPreview::DrawPageTextFirst(int nPageNum)
{
	// �y�[�W�g�b�v�̐F�w����擾
	ColorStrategy*	pStrategy = nullptr;
	if (pPrintSetting->bColorPrint) {
		pool = &ColorStrategyPool::getInstance();
		pool->SetCurrentView(&(parentWnd.GetActiveView()));

		const int nPageTopLineNum = (nPageNum * pPrintSetting->nPrintDansuu) * bPreview_EnableLines;
		const Layout*	pPageTopLayout = pLayoutMgr_Print->SearchLineByLayoutY(nPageTopLineNum);

		if (pPageTopLayout) {
			const int nPageTopOff = pPageTopLayout->GetLogicOffset();

			// �y�[�W�g�b�v�̕����s�̐擪������
			while (pPageTopLayout->GetLogicOffset()) {
				pPageTopLayout = pPageTopLayout->GetPrevLayout();
			}

			// �_���s�擪��ColorStrategy�擾
			pStrategy = pool->GetStrategyByColor(pPageTopLayout->GetColorTypePrev());
			pool->NotifyOnStartScanLogic();
			if (pStrategy) {
				pStrategy->InitStrategyStatus();
				pStrategy->SetStrategyColorInfo(pPageTopLayout->GetColorInfo());
			}
			if (nPageTopOff) {
				StringRef	csr = pPageTopLayout->GetDocLineRef()->GetStringRefWithEOL();
				int iLogic;
				for (iLogic=0; iLogic<nPageTopOff; ++iLogic) {
					bool bChange;
					pStrategy = GetColorStrategy(csr, iLogic, pStrategy, bChange);
				}
			}
		}
	}
	return pStrategy;
}


/* ���/���Preview �y�[�W�e�L�X�g�̕`��
	DrawPageText�ł́A�s�ԍ����i���p�t�H���g�Łj����B
	�{����Print_DrawLine�ɂ��C��
	@date 2006.08.14 Moca ���ʎ��̂����肾���ƁA�R�[�h�̐��� 
	@date 2013.05.19 Moca �F����������pStrategy���y�[�W���܂����ŗ��p����
*/
ColorStrategy* PrintPreview::DrawPageText(
	HDC				hdc,
	int				nOffX,
	int				nOffY,
	int				nPageNum,
	DlgCancel*		pDlgCancel,
	ColorStrategy*	pStrategyStart
	)
{
	int				nDirectY = -1;

	const int		nLineHeight = pPrintSetting->nPrintFontHeight + (pPrintSetting->nPrintFontHeight * pPrintSetting->nPrintLineSpacing / 100);
	// �i�ƒi�̊Ԋu�̕�
	const int		nDanWidth = bPreview_EnableColumns * pPrintSetting->nPrintFontWidth + pPrintSetting->nPrintDanSpace;
	// �s�ԍ��̕�
	const int		nLineNumWidth = nPreview_LineNumberColumns * pPrintSetting->nPrintFontWidth;

	// ���p�t�H���g�̏����擾�����p�t�H���g�ɐݒ�

	// �y�[�W�g�b�v�̐F�w����擾
	ColorStrategy*	pStrategy = pStrategyStart;

	auto& typeConfig = parentWnd.GetDocument().docType.GetDocumentAttribute();

	// �i�����[�v
	for (int nDan=0; nDan<pPrintSetting->nPrintDansuu; ++nDan) {
		// �{��1���ڂ̍����̍��W(�s�ԍ�������ꍇ�͂��̍��W��荶��)
		const int nBasePosX = nOffX + nDanWidth * nDan + nLineNumWidth * (nDan + 1);
		
		int i; //	�s���J�E���^
		for (i=0; i<bPreview_EnableLines; ++i) {
			if (pDlgCancel) {
				// �������̃��[�U�[������\�ɂ���
				if (!::BlockingHook(pDlgCancel->GetHwnd())) {
					return nullptr;
				}
			}

			/*	���ݕ`�悵�悤�Ƃ��Ă���s�̕����s���i�܂�Ԃ����ƂɃJ�E���g�����s���j
				�֌W������̂́A
				�u�y�[�W���inPageNum�j�v
				�u�i���ipPrintSetting->nPrintDansuu�j�v
				�u�i����1�̂Ƃ��ɁA1�y�[�W������ɉ��s���邩�ibPreview_EnableLines�j�v
			*/
			const int nLineNum = (nPageNum * pPrintSetting->nPrintDansuu + nDan) * bPreview_EnableLines + i;
			const Layout* pLayout = pLayoutMgr_Print->SearchLineByLayoutY(nLineNum);
			if (!pLayout) {
				break;
			}
			// �s�ԍ���\�����邩
			if (pPrintSetting->bPrintLineNumber) {
				wchar_t szLineNum[64];	//	�s�ԍ�������B
				// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
				if (typeConfig.bLineNumIsCRLF) {
					// �_���s�ԍ��\�����[�h
					if (pLayout->GetLogicOffset() != 0) { // �܂�Ԃ����C�A�E�g�s
						wcscpy_s(szLineNum, L" ");
					}else {
						_itow(pLayout->GetLogicLineNo() + 1, szLineNum, 10);	// �Ή�����_���s�ԍ�
					}
				}else {
					// �����s(���C�A�E�g�s)�ԍ��\�����[�h
					_itow(nLineNum + 1, szLineNum, 10);
				}

				// �s�ԍ���؂�  0=�Ȃ� 1=�c�� 2=�C��
				if (typeConfig.nLineTermType == 2) {
					wchar_t szLineTerm[2];
					szLineTerm[0] = typeConfig.cLineTermChar;	// �s�ԍ���؂蕶��
					szLineTerm[1] = L'\0';
					wcscat(szLineNum, szLineTerm);
				}else {
					wcscat(szLineNum, L" ");
				}

				// ������
				const size_t nLineCols = wcslen(szLineNum);

				// �����Ԋu�z��𐶐�
				vector<int> vDxArray;
				const int* pDxArray = TextMetrics::GenerateDxArray(&vDxArray, szLineNum, nLineCols, pPrintSetting->nPrintFontWidth);

				ApiWrap::ExtTextOutW_AnyBuild(
					hdc,
					nBasePosX - nLineCols * pPrintSetting->nPrintFontWidth,
					nDirectY * (nOffY + nLineHeight * i + (pPrintSetting->nPrintFontHeight - nAscentHan)),
					0,
					NULL,
					szLineNum,
					nLineCols,
					pDxArray
				);
			}

			const size_t nLineLen = pLayout->GetLengthWithoutEOL();
			if (nLineLen == 0) {
				continue;
			}

			// �����s���̐F�w����擾
			if (pPrintSetting->bColorPrint
				&& !(nDan == 0 && i == 0)
				&& pLayout->GetLogicOffset() == 0
			) {
				pStrategy = pool->GetStrategyByColor(pLayout->GetColorTypePrev());
				pool->NotifyOnStartScanLogic();
				if (pStrategy) {
					pStrategy->InitStrategyStatus();
					pStrategy->SetStrategyColorInfo(pLayout->GetColorInfo());
				}
			}
			// ����^Preview �s�`��
			pStrategy = Print_DrawLine(
				hdc,
				Point(
					nBasePosX,
					nDirectY * (nOffY + nLineHeight * i)
				),
				pLayout->GetDocLineRef()->GetPtr(),	// pLayout->GetPtr(),
				pLayout->GetDocLineRef()->GetLengthWithEOL(),
				pLayout->GetLogicOffset(),
				nLineLen,
				pLayout->GetIndent(), // 2006.05.16 Add Moca. ���C�A�E�g�C���f���g�����炷�B
				pPrintSetting->bColorPrint ? pLayout : NULL,
				pStrategy
			);
		}

		// 2006.08.14 Moca �s�ԍ����c���̏ꍇ��1�x�Ɉ���
		if (pPrintSetting->bPrintLineNumber
			&& typeConfig.nLineTermType == 1
		) {
			// �c���͖{���ƍs�ԍ��̌���1���̒��S�ɍ�悷��(��ʍ��ł́A�E�l��)
			::MoveToEx(hdc,
				nBasePosX - (pPrintSetting->nPrintFontWidth / 2),
				nDirectY * nOffY,
				NULL);
			::LineTo(hdc,
				nBasePosX - (pPrintSetting->nPrintFontWidth / 2),
				nDirectY * (nOffY + nLineHeight * i)
			);
		}
	}
	return pStrategy;
}


// ���Preview �X�N���[���o�[������
void PrintPreview::InitPreviewScrollBar(void)
{
	RECT rc;
	int nToolBarHeight = 0;
	if (hwndPrintPreviewBar) {
		::GetWindowRect(hwndPrintPreviewBar, &rc);
		nToolBarHeight = rc.bottom - rc.top;
	}
	::GetClientRect(parentWnd.GetHwnd(), &rc);
	int cx = rc.right - rc.left;
	int cy = rc.bottom - rc.top - nToolBarHeight;
	
	SCROLLINFO si;
	if (hwndVScrollBar) {
		// �����X�N���[���o�[
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		if (nPreview_ViewHeight <= cy - nToolBarHeight) {
			si.nMax  = cy - nToolBarHeight;			// �S��
			si.nPage = cy - nToolBarHeight;			// �\����̌���
			si.nPos  = -1 * nPreviewVScrollPos;	// �\����̈�ԍ��̈ʒu
			si.nTrackPos = 0;
			SCROLLBAR_VERT = FALSE;
		}else {
			si.nMax  = nPreview_ViewHeight;		// �S��
			si.nPage = cy - nToolBarHeight;			// �\����̌���
			si.nPos  = -1 * nPreviewVScrollPos;	// �\����̈�ԍ��̈ʒu
			si.nTrackPos = 100;
			SCROLLBAR_VERT = TRUE;
		}
		::SetScrollInfo(hwndVScrollBar, SB_CTL, &si, TRUE);
	}
	// ���Preview �����X�N���[���o�[�E�B���h�E�n���h��
	if (hwndHScrollBar) {
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		// �����X�N���[���o�[
//		si.cbSize = sizeof(si);
//		si.fMask = SIF_ALL;
		si.nMin  = 0;
		if (nPreview_ViewWidth <= cx) {
			si.nMax  = cx;							// �S��
			si.nPage = cx;							// �\����̌���
			si.nPos  = nPreviewHScrollPos;		// �\����̈�ԍ��̈ʒu
			si.nTrackPos = 0;
			SCROLLBAR_HORZ = FALSE;
		}else {
			si.nMax  = nPreview_ViewWidth;		// �S��
			si.nPage = cx;							// �\����̌���
			si.nPos  = nPreviewHScrollPos;		// �\����̈�ԍ��̈ʒu
			si.nTrackPos = 100;
			SCROLLBAR_HORZ = TRUE;
		}
		::SetScrollInfo(hwndHScrollBar, SB_CTL, &si, TRUE);
	}
	return;
}

/*! ����^Preview �s�`��
	@param[in] nIndent �s���܂�Ԃ��C���f���g����

	@date 2006.08.14 Moca   �܂�Ԃ��C���f���g��������ɔ��f�����悤��
	@date 2007.08    kobake �@�B�I��UNICODE��
	@date 2007.12.12 kobake �S�p�t�H���g�����f����Ă��Ȃ������C��
*/
ColorStrategy* PrintPreview::Print_DrawLine(
	HDC				hdc,
	POINT			ptDraw,		// �`����W�BHDC�����P�ʁB
	const wchar_t*	pLine,
	size_t			nDocLineLen,
	size_t			nLineStart,
	size_t			nLineLen,
	size_t			nIndent,	// 2006.08.14 Moca �ǉ�
	const Layout*	pLayout,	// �F�t�pLayout
	ColorStrategy*	pStrategyStart
	)
{
	if (nLineLen == 0) {
		return pStrategyStart;
	}

	/*	pLine���X�L�������āA���p�����͔��p�����ł܂Ƃ߂āA�S�p�����͑S�p�����ł܂Ƃ߂ĕ`�悷��B
	*/

	// �����Ԋu
	int nDx = pPrintSetting->nPrintFontWidth;

	// �^�u���擾
	size_t nTabSpace = parentWnd.GetDocument().layoutMgr.GetTabSpace(); //	Sep. 23, 2002 genta LayoutMgr�̒l���g��

	// �����Ԋu�z��𐶐�
	vector<int> vDxArray;
	const int* pDxArray = TextMetrics::GenerateDxArray(
		&vDxArray,
		pLine + nLineStart,
		nLineLen,
		nDx,
		nTabSpace,
		nIndent
	);

	size_t nBgnLogic = nLineStart;	// TAB��W�J����O�̃o�C�g���ŁApLine�̉��o�C�g�ڂ܂ŕ`�悵�����H
	size_t iLogic;					// pLine�̉������ڂ��X�L�����H
	size_t nLayoutX = nIndent;	// TAB��W�J������̃o�C�g���ŁA�e�L�X�g�̉��o�C�g�ڂ܂ŕ`�悵�����H

	// �����픻��t���O
	int nKind     = 0; // 0:���p 1:�S�p 2:�^�u
	int nKindLast = 2; // ���O��nKind���

	// �F�ݒ�	2012-03-07 ossan
	StringRef cStringLine(pLine, nDocLineLen);
	ColorStrategy* pStrategy = pStrategyStart;
	// 2014.12.30 �F��GetColorStrategy�Ŏ��̐F�ɂȂ�O�Ɏ擾����K�v������
	int nColorIdx = ToColorInfoArrIndex( pStrategy ? pStrategy->GetStrategyColor() : COLORIDX_TEXT );

	for (
		iLogic = nLineStart;
		iLogic < nLineStart + nLineLen; 
		++iLogic, nKindLast = nKind
	) {
		// �����̎��
		if (pLine[iLogic] == WCODE::TAB) {
			nKind = 2;
		}else if (WCODE::IsHankaku(pLine[iLogic])) {
			nKind = 0;
		}else {
			nKind = 1;
		}

		bool bChange = false;
		pStrategy = pLayout ? GetColorStrategy(cStringLine, iLogic, pStrategy, bChange) : nullptr;

		// �^�u�����o�� or ������(�S�p�^���p)�̋��E or �F�w��̋��E
		if (nKind != nKindLast || bChange) {
			// iLogic�̒��O�܂ł�`��
			ASSERT_GE(iLogic, nBgnLogic);
			if (0 < iLogic - nBgnLogic) {
				ASSERT_GE(nBgnLogic, nLineStart);
				Print_DrawBlock(
					hdc,
					ptDraw,		// �`����W�BHDC�����P�ʁB
					pLine + nLineStart,
					iLogic - nBgnLogic,
					nKindLast,
					pLayout,	// �F�ݒ�pLayout
					nColorIdx,
					nBgnLogic - nLineStart,
					nLayoutX,
					nDx,
					pDxArray
				);

				// ���i��
				if (nKindLast == 2) {
					ASSERT_GE(iLogic, nBgnLogic);
					nLayoutX += (nTabSpace - (nLayoutX % nTabSpace)) + nTabSpace * (iLogic - nBgnLogic - 1);
				}else {
					int nIncrement = 0;
					ASSERT_GE(nBgnLogic, nLineStart);
					ASSERT_GE(iLogic, nLineStart);
					for (size_t i=nBgnLogic-nLineStart; i<iLogic-nLineStart; ++i) {
						nIncrement += pDxArray[i];
					}
					nLayoutX += nIncrement / nDx;
				}
				// ���W�b�N�i��
				nBgnLogic = iLogic;
			}
			if (bChange) {
				// ���̃u���b�N�̐F
				nColorIdx = ToColorInfoArrIndex( pStrategy ? pStrategy->GetStrategyColor() : COLORIDX_TEXT );
			}
		}
	}

	// �c���`��
	if (0 < nLineStart + nLineLen - nBgnLogic) {
		Print_DrawBlock(
			hdc,
			ptDraw,		// �`����W�BHDC�����P�ʁB
			pLine + nLineStart,
			nLineStart + nLineLen - nBgnLogic,
			nKindLast,
			pLayout,	// �F�ݒ�pLayout
			nColorIdx,
			nBgnLogic - nLineStart,
			nLayoutX,
			nDx,
			pDxArray
		);
	}

	// �t�H���g���� (���p) �ɖ߂�
	::SelectObject(hdc, hFontHan);

	// �F�����ɖ߂�	2012-03-07 ossan
	if (pLayout) {
		int nColorIdx = ToColorInfoArrIndex(COLORIDX_TEXT);
		if (nColorIdx != -1) {
			const ColorInfo& info = parentWnd.GetDocument().docType.GetDocumentAttribute().colorInfoArr[nColorIdx];
			::SetTextColor(hdc, info.colorAttr.cTEXT);
//			::SetBkColor(hdc, info.colBACK);
		}
	}

	return pStrategy;
}

/*! ����^Preview �u���b�N�`��
	@param[in] 

	@date 2013.05.01 Uchi Print_DrawLine����؂�o��
*/
void PrintPreview::Print_DrawBlock(
	HDC				hdc,
	POINT			ptDraw,		// �`����W�BHDC�����P�ʁB
	const wchar_t*	pPhysicalLine,
	int				nBlockLen,	// iLogic - nBgnLogic
	int				nKind,
	const Layout*	pLayout,	// �F�ݒ�pLayout
	int				nColorIdx,
	int				nBgnPhysical,	// nBgnLogic - nLineStart
	int				nLayoutX,
	int				nDx,
	const int*		pDxArray
	)
{
	if (nKind == 2 && !pLayout) {
		// TAB�̓J���[�Ŗ�����Έ󎚕s�v
		return;
	}
	HFONT hFont = (nKind == 1) ? hFontZen : hFontHan;
	// �F�ݒ�
	if (pLayout) {
		if (nColorIdx != -1) {
			const ColorInfo& info = parentWnd.GetDocument().docType.GetDocumentAttribute().colorInfoArr[nColorIdx];
			if (nKind == 2 && !info.fontAttr.bUnderLine) {
				// TAB�͉�����������Έ󎚕s�v
				return;
			}
			if (info.fontAttr.bBoldFont) {
				if (info.fontAttr.bUnderLine) {
					hFont = (nKind == 1 ? hFontZen_bu: hFontHan_bu);	// �����A����
				}else {
					hFont = (nKind == 1 ? hFontZen_b : hFontHan_b);		// ����
				}
			}else {
				if (info.fontAttr.bUnderLine) {
					hFont = (nKind == 1 ? hFontZen_u : hFontHan_u);		// ����
				}
			}
			//	else					hFont = (nKind == 1 ? hFontZen   : hFontHan);		// �W��
			::SetTextColor(hdc, info.colorAttr.cTEXT);
//			::SetBkColor(hdc, info.colBACK);
		}
	}
	::SelectObject(hdc, hFont);
	::ExtTextOutW_AnyBuild(
		hdc,
		ptDraw.x + nLayoutX * nDx,
		ptDraw.y - (pPrintSetting->nPrintFontHeight - (nKind == 1 ? nAscentZen : nAscentHan)),
		0,
		NULL,
		&pPhysicalLine[nBgnPhysical],
		nBlockLen,
		&pDxArray[nBgnPhysical]
	);
}

/*! �w�胍�W�b�N�ʒu��ColorStrategy���擾
	@param[in] 

	@date 2013.05.01 Uchi �V�K�쐬
	@date 2014.12.30 Moca ���K�\���̈Ⴄ�F������ł����ꍇ�ɐF�ւ��ł��ĂȂ������o�O���C��
*/
ColorStrategy* PrintPreview::GetColorStrategy(
	const StringRef&	stringLine,
	size_t				iLogic,
	ColorStrategy*		pStrategy,
	bool&				bChange
	)
{
	if (pStrategy) {
		if (pStrategy->EndColor(stringLine, iLogic)) {
			pStrategy = nullptr;
			bChange = true;
		}
	}
	if (!pStrategy) {
		for (size_t i=0; i<pool->GetStrategyCount(); ++i) {
			if (pool->GetStrategy(i)->BeginColor(stringLine, iLogic)) {
				pStrategy = pool->GetStrategy(i);
				bChange = true;
				break;
			}
		}
	}

	return pStrategy;
}


/*	���Preview�t�H���g�i���p�j��ݒ肷��
	typedef struct tagLOGFONT {
	   LONG lfHeight; 
	   LONG lfWidth; 
	   LONG lfEscapement; 
	   LONG lfOrientation; 
	   LONG lfWeight; 
	   BYTE lfItalic; 
	   BYTE lfUnderline; 
	   BYTE lfStrikeOut; 
	   BYTE lfCharSet; 
	   BYTE lfOutPrecision; 
	   BYTE lfClipPrecision; 
	   BYTE lfQuality; 
	   BYTE lfPitchAndFamily; 
	   TCHAR lfFaceName[LF_FACESIZE]; 
	} LOGFONT;
*/
void PrintPreview::SetPreviewFontHan(const LOGFONT* lf)
{
	lfPreviewHan = *lf;

	//	PrintSetting����R�s�[
	lfPreviewHan.lfHeight			= pPrintSetting->nPrintFontHeight;
	lfPreviewHan.lfWidth			= pPrintSetting->nPrintFontWidth;
	_tcscpy(lfPreviewHan.lfFaceName, pPrintSetting->szPrintFontFaceHan);

	SelectCharWidthCache(CharWidthFontMode::Print, CharWidthCacheMode::Local);
	InitCharWidthCache(lfPreviewHan, CharWidthFontMode::Print);
}

void PrintPreview::SetPreviewFontZen(const LOGFONT* lf)
{
	lfPreviewZen = *lf;
	//	PrintSetting����R�s�[
	lfPreviewZen.lfHeight	= pPrintSetting->nPrintFontHeight;
	lfPreviewZen.lfWidth	= pPrintSetting->nPrintFontWidth;
	_tcscpy(lfPreviewZen.lfFaceName, pPrintSetting->szPrintFontFaceZen);
}

int CALLBACK PrintPreview::MyEnumFontFamProc(
	ENUMLOGFONT*	pelf,		// pointer to logical-font data
	NEWTEXTMETRIC*	pntm,		// pointer to physical-font data
	int				nFontType,	// type of font
	LPARAM			lParam 		// address of application-defined data
	)
{
	PrintPreview* pPrintPreview = (PrintPreview*)lParam;
	if (_tcscmp(pelf->elfLogFont.lfFaceName, pPrintPreview->pPrintSetting->szPrintFontFaceHan) == 0) {
		pPrintPreview->SetPreviewFontHan(&pelf->elfLogFont);
	}
	if (_tcscmp(pelf->elfLogFont.lfFaceName, pPrintPreview->pPrintSetting->szPrintFontFaceZen) == 0) {
		pPrintPreview->SetPreviewFontZen(&pelf->elfLogFont);
	}

	return 1;
}

/*!
	���Preview�ɕK�v�ȃR���g���[�����쐬����
*/
void PrintPreview::CreatePrintPreviewControls(void)
{
	// ���Preview ����o�[
	hwndPrintPreviewBar = ::CreateDialogParam(
		SelectLang::getLangRsrcInstance(),					// handle to application instance
		MAKEINTRESOURCE(IDD_PRINTPREVIEWBAR),				// identifies dialog box template name
		parentWnd.GetHwnd(),							// handle to owner window
		PrintPreview::PrintPreviewBar_DlgProc,	// pointer to dialog box procedure
		(LPARAM)this
	);

	// �c�X�N���[���o�[�̍쐬
	hwndVScrollBar = ::CreateWindowEx(
		0L,									// no extended styles
		_T("SCROLLBAR"),					// scroll bar control class
		NULL,								// text for window title bar
		WS_VISIBLE | WS_CHILD | SBS_VERT,	// scroll bar styles
		0,									// horizontal position
		0,									// vertical position
		200,								// width of the scroll bar
		CW_USEDEFAULT,						// default height
		parentWnd.GetHwnd(),			// handle of main window
		(HMENU) NULL,						// no menu for a scroll bar
		EditApp::getInstance().GetAppInstance(),		// instance owning this window
		(LPVOID) NULL						// pointer not needed
	);
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
	si.nMin	 = 0;
	si.nMax	 = 29;
	si.nPage = 10;
	si.nPos	 = 0;
	si.nTrackPos = 1;
	::SetScrollInfo(hwndVScrollBar, SB_CTL, &si, TRUE);
	::ShowScrollBar(hwndVScrollBar, SB_CTL, TRUE);

	// ���X�N���[���o�[�̍쐬
	hwndHScrollBar = ::CreateWindowEx(
		0L,									// no extended styles
		_T("SCROLLBAR"),					// scroll bar control class
		NULL,								// text for window title bar
		WS_VISIBLE | WS_CHILD | SBS_HORZ,	// scroll bar styles
		0,									// horizontal position
		0,									// vertical position
		200,								// width of the scroll bar
		CW_USEDEFAULT,						// default height
		parentWnd.GetHwnd(),			// handle of main window
		(HMENU) NULL,						// no menu for a scroll bar
		EditApp::getInstance().GetAppInstance(),	// instance owning this window
		(LPVOID) NULL						// pointer not needed
	);
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
	si.nMin	 = 0;
	si.nMax	 = 29;
	si.nPage = 10;
	si.nPos	 = 0;
	si.nTrackPos = 1;
	::SetScrollInfo(hwndHScrollBar, SB_CTL, &si, TRUE);
	::ShowScrollBar(hwndHScrollBar, SB_CTL, TRUE);

	// �T�C�Y�{�b�N�X�̍쐬
	hwndSizeBox = ::CreateWindowEx(
		WS_EX_CONTROLPARENT/*0L*/, 							// no extended styles
		_T("SCROLLBAR"),									// scroll bar control class
		NULL,												// text for window title bar
		WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, // scroll bar styles
		0,													// horizontal position
		0,													// vertical position
		200,												// width of the scroll bar
		CW_USEDEFAULT,										// default height
		parentWnd.GetHwnd(), 							// handle of main window
		(HMENU) NULL,										// no menu for a scroll bar
		EditApp::getInstance().GetAppInstance(),			// instance owning this window
		(LPVOID) NULL										// pointer not needed
	);
	::ShowWindow(hwndPrintPreviewBar, SW_SHOW);

	// WM_SIZE ����
	RECT rc1;
	::GetClientRect(parentWnd.GetHwnd(), &rc1);
	OnSize(SIZE_RESTORED, MAKELONG(rc1.right - rc1.left, rc1.bottom - rc1.top));
	return;
}


/*!
	���Preview�ɕK�v�������R���g���[����j������
*/
void PrintPreview::DestroyPrintPreviewControls(void)
{
	// ���Preview ����o�[ �폜
	if (hwndPrintPreviewBar) {
		::DestroyWindow(hwndPrintPreviewBar);
		hwndPrintPreviewBar = NULL;
	}

	// ���Preview �����X�N���[���o�[�E�B���h�E �폜
	if (hwndVScrollBar) {
		::DestroyWindow(hwndVScrollBar);
		hwndVScrollBar = NULL;
	}
	// ���Preview �����X�N���[���o�[�E�B���h�E �폜
	if (hwndHScrollBar) {
		::DestroyWindow(hwndHScrollBar);
		hwndHScrollBar = NULL;
	}
	// ���Preview �T�C�Y�{�b�N�X�E�B���h�E �폜
	if (hwndSizeBox) {
		::DestroyWindow(hwndSizeBox);
		hwndSizeBox = NULL;
	}
}

// �_�C�A���O�v���V�[�W��
INT_PTR CALLBACK PrintPreview::PrintPreviewBar_DlgProc(
	HWND hwndDlg,	// handle to dialog box
	UINT uMsg,		// message
	WPARAM wParam,	// first message parameter
	LPARAM lParam 	// second message parameter
	)
{
	PrintPreview* pPrintPreview;
	switch (uMsg) {
	case WM_INITDIALOG:
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
		// 2007.02.11 Moca WM_INIT��DispatchEvent_PPB���ĂԂ悤��
		pPrintPreview = (PrintPreview*)lParam;
		if (pPrintPreview) {
			return pPrintPreview->DispatchEvent_PPB(hwndDlg, uMsg, wParam, lParam);
		}
		return TRUE;
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pPrintPreview = (PrintPreview*)::GetWindowLongPtr(hwndDlg, DWLP_USER);
		if (pPrintPreview) {
			return pPrintPreview->DispatchEvent_PPB(hwndDlg, uMsg, wParam, lParam);
		}else {
			return FALSE;
		}
	}
}

// ���Preview ����o�[�Ƀt�H�[�J�X�𓖂Ă�
void PrintPreview::SetFocusToPrintPreviewBar(void)
{
	if (hwndPrintPreviewBar) {
		::SetFocus(hwndPrintPreviewBar);
	}
}

// ���Preview ����o�[ �_�C�A���O�̃��b�Z�[�W����
// IDD_PRINTPREVIEWBAR
INT_PTR PrintPreview::DispatchEvent_PPB(
	HWND				hwndDlg,	// handle to dialog box
	UINT				uMsg,		// message
	WPARAM				wParam,		// first message parameter
	LPARAM				lParam 		// second message parameter
	)
{
	WORD wNotifyCode;
	WORD wID;

	switch (uMsg) {
	case WM_INITDIALOG:
		// 2007.02.11 Moca DWLP_USER�ݒ�͕s�v
		//// Modified by KEITA for WIN64 2003.9.6
		//::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
		{
			if (IsWin2000_or_later()) {
				::EnableWindow( ::GetDlgItem(hwndDlg, IDC_CHECK_ANTIALIAS), TRUE );
			}
		}
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// �ʒm�R�[�h
		wID			= LOWORD(wParam);	// ����ID�A�R���g���[��ID �܂��̓A�N�Z�����[�^ID
		switch (wNotifyCode) {
		// �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ
		case BN_CLICKED:
			switch (wID) {
			case IDC_BUTTON_PRINTERSELECT:
				// From Here 2003.05.03 �����
				{
					// PRINTDLG��������
					PRINTDLG pd = {0};
					pd.Flags = PD_PRINTSETUP | PD_NONETWORKBUTTON;
					pd.hwndOwner = parentWnd.GetHwnd();
					if (print.PrintDlg(&pd, &pPrintSettingOrg->mdmDevMode)) {
						// �p���T�C�Y�Ɨp�������𔽉f������ 2003.05.03 �����
						pPrintSettingOrg->nPrintPaperSize = pPrintSettingOrg->mdmDevMode.dmPaperSize;
						pPrintSettingOrg->nPrintPaperOrientation = pPrintSettingOrg->mdmDevMode.dmOrientation;
						// ���Preview �X�N���[���o�[������
						AppNodeGroupHandle(0).SendMessageToAllEditors(
							MYWM_CHANGESETTING,
							(WPARAM)0,
							(LPARAM)PM_PrintSetting,
							EditWnd::getInstance().GetHwnd()
						);
						// OnChangePrintSetting();
						// ::InvalidateRect(parentWnd.GetHwnd(), NULL, TRUE);
					}
				}
				// To Here 2003.05.03 �����
				break;
			case IDC_BUTTON_PrintSetting:
				parentWnd.OnPrintPageSetting();
				break;
			case IDC_BUTTON_ZOOMUP:
				// Preview�g��k��
				OnPreviewZoom(TRUE);
				break;
			case IDC_BUTTON_ZOOMDOWN:
				// Preview�g��k��
				OnPreviewZoom(FALSE);
				break;
			case IDC_BUTTON_PREVPAGE:
				// �O�y�[�W
				OnPreviewGoPreviousPage();
				break;
			case IDC_BUTTON_NEXTPAGE:
				// ���y�[�W
				OnPreviewGoNextPage();
				break;
			// From Here 2007.02.11 Moca �_�C���N�g�W�����v����уA���`�G�C���A�X
			case IDC_BUTTON_DIRECTPAGE:
				OnPreviewGoDirectPage();
				break;
			case IDC_CHECK_ANTIALIAS:
				OnCheckAntialias();
				break;
			// To Here 2007.02.11 Moca
			case IDC_BUTTON_HELP:
				// ���Preview�̃w���v
				// Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
				MyWinHelp(hwndDlg, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PRINT_PREVIEW));	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
				break;
			case IDOK:
				// ������s
				OnPrint();
				return TRUE;
			case IDCANCEL:
				// ���Preview���[�h�̃I��/�I�t
				parentWnd.PrintPreviewModeONOFF();
				return TRUE;
			}
			break;	// BN_CLICKED
		}
		break;	// WM_COMMAND
	}
	return FALSE;
}


// ����p�t�H���g���쐬����
void PrintPreview::CreateFonts(HDC hdc)
{
	LOGFONT	lf;
	// ����p���p�t�H���g���쐬 -> hFontHan
	lfPreviewHan.lfHeight	= pPrintSetting->nPrintFontHeight;
	lfPreviewHan.lfWidth	= pPrintSetting->nPrintFontWidth;
	_tcscpy(lfPreviewHan.lfFaceName, pPrintSetting->szPrintFontFaceHan);
	hFontHan	= CreateFontIndirect(&lfPreviewHan);
	if (pPrintSetting->bColorPrint) {
		lf = lfPreviewHan;	lf.lfWeight = FW_BOLD;
		hFontHan_b	= CreateFontIndirect(&lf);		// ����
		lf = lfPreviewHan;							lf.lfUnderline = true;
		hFontHan_u	= CreateFontIndirect(&lf);		// ����
		lf = lfPreviewHan;	lf.lfWeight = FW_BOLD;	lf.lfUnderline = true;
		hFontHan_bu	= CreateFontIndirect(&lf);		// �����A����
	}
#ifdef _DEEBUG
	else {
		hFontHan_b  = m_hFontHan_u  = m_hFontHan_bu = NULL;
	}
#endif
	// ���p�����̃A�Z���g�i�������j���擾
	::SelectObject(hdc, hFontHan);
	TEXTMETRIC tm;
	::GetTextMetrics(hdc, &tm);
	nAscentHan = tm.tmAscent;

	// ����p�S�p�t�H���g���쐬 -> hFontZen
	if (auto_strcmp(pPrintSetting->szPrintFontFaceHan, pPrintSetting->szPrintFontFaceZen)) {
		lfPreviewZen.lfHeight	= pPrintSetting->nPrintFontHeight;
		lfPreviewZen.lfWidth	= pPrintSetting->nPrintFontWidth;
		_tcscpy(lfPreviewZen.lfFaceName, pPrintSetting->szPrintFontFaceZen);
		hFontZen	= CreateFontIndirect(&lfPreviewZen);
		if (pPrintSetting->bColorPrint) {
			lf = lfPreviewZen;	lf.lfWeight = FW_BOLD;
			hFontZen_b	= CreateFontIndirect(&lf);		// ����
			lf = lfPreviewZen;							lf.lfUnderline = true;
			hFontZen_u	= CreateFontIndirect(&lf);		// ����
			lf = lfPreviewZen;	lf.lfWeight = FW_BOLD;	lf.lfUnderline = true;
			hFontZen_bu	= CreateFontIndirect(&lf);		// �����A����
		}
#ifdef _DEEBUG
		else {
			hFontHan_b  = m_hFontHan_u  = m_hFontHan_bu = NULL;
		}
#endif
		// �S�p�����̃A�Z���g�i�������j���擾
		::SelectObject(hdc, hFontZen);
		::GetTextMetrics(hdc, &tm);
		nAscentZen = tm.tmAscent;
	}else {
		// ���p�S�p�����t�H���g
		hFontZen		= hFontHan;
		hFontZen_b	= hFontHan_b;		// ����
		hFontZen_u	= hFontHan_u;		// ����
		hFontZen_bu	= hFontHan_bu;	// �����A����
		nAscentZen	= nAscentHan;		// �S�p�����̃A�Z���g
	}
}

// ����p�t�H���g��j������
void PrintPreview::DestroyFonts()
{
	if (hFontZen != hFontHan) {
		::DeleteObject(hFontZen);
		if (hFontZen_b) {
			::DeleteObject(hFontZen_b);
			::DeleteObject(hFontZen_u);
			::DeleteObject(hFontZen_bu);
		}
	}
	::DeleteObject(hFontHan);
	if (hFontHan_b) {
		::DeleteObject(hFontHan_b);
		::DeleteObject(hFontHan_u);
		::DeleteObject(hFontHan_bu);
	}
#ifdef _DEEBUG
	hFontHan = m_hFontHan_b = m_hFontHan_u = m_hFontHan_bu =
	hFontZen = m_hFontZen_b = m_hFontZen_u = m_hFontZen_bu = NULL;
#endif
}

