/*!	@file
	@brief 印刷Preview管理クラス
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
// ColorStrategyは本来はCEditViewが必要だが、EditWnd.hあたりでinclude済み
#include "view/colors/ColorStrategy.h"
#include "sakura_rc.h"

using namespace std;

#define MIN_PREVIEW_ZOOM 10
#define MAX_PREVIEW_ZOOM 400

#define		LINE_RANGE_X	48		// 水平方向の１回のスクロール幅
#define		LINE_RANGE_Y	24		// 垂直方向の１回のスクロール幅

#define		PAGE_RANGE_X	160		// 水平方向の１回のページスクロール幅
#define		PAGE_RANGE_Y	160		// 垂直方向の１回のページスクロール幅

#define		COMPAT_BMP_BASE     1   // COMPAT_BMP_SCALEピクセル幅を複写する画面ピクセル幅
#define		COMPAT_BMP_SCALE    2   // 互換BMPのCOMPAT_BMP_BASEに対する倍率(1以上の整数倍)

Print PrintPreview::print;		// 現在のプリンタ情報 2003.05.02 かろと

/*! コンストラクタ
	印刷Previewを表示するために必要な情報を初期化、領域確保。
	コントロールも作成する。
*/
PrintPreview::PrintPreview(EditWnd& parentWnd)
	:
	parentWnd(parentWnd),
	hdcCompatDC(NULL),			// 再描画用コンパチブルDC
	hbmpCompatBMP(NULL),			// 再描画用メモリBMP
	hbmpCompatBMPOld(NULL),		// 再描画用メモリBMP(OLD)
	nbmpCompatScale(COMPAT_BMP_BASE),
	nPreviewVScrollPos(0),
	nPreviewHScrollPos(0),
	nPreview_Zoom(100),			// 印刷Preview倍率
	nCurPageNum(0),				// 現在のページ
	bLockSetting(false),
	bDemandUpdateSetting(false)
{
	// 印刷用のレイアウト情報の作成
	pLayoutMgr_Print = new LayoutMgr;

	// 印刷Preview コントロール 作成
	CreatePrintPreviewControls();

	// 再描画用コンパチブルDC
	HDC hdc = ::GetDC(parentWnd.GetHwnd());
	hdcCompatDC = ::CreateCompatibleDC(hdc);
	::ReleaseDC(parentWnd.GetHwnd(), hdc);
}

PrintPreview::~PrintPreview()
{
	// 印刷Preview コントロール 破棄
	DestroyPrintPreviewControls();
	
	// 印刷用のレイアウト情報の削除
	delete pLayoutMgr_Print;
	
	// フォント幅キャッシュを編集モードに戻す
	SelectCharWidthCache(CharWidthFontMode::Edit, CharWidthCacheMode::Neutral);

	// 2006.08.17 Moca CompatDC削除。EditWndから移設
	// 再描画用メモリBMP
	if (hbmpCompatBMP) {
		// 再描画用メモリBMP(OLD)
		::SelectObject(hdcCompatDC, hbmpCompatBMPOld);
		::DeleteObject(hbmpCompatBMP);
	}
	// 再描画用コンパチブルDC
	if (hdcCompatDC) {
		::DeleteDC(hdcCompatDC);
	}
}

/*!	印刷Preview時の、WM_PAINTを処理

	@date 2007.02.11 Moca Previewを滑らかにする機能．
		拡大描画してから縮小することでアンチエイリアス効果を出す．
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
	HDC				hdc = hdcCompatDC;	//	親ウィンドウのComatibleDCに描く

	// 印刷Preview 操作バー
	
	// BMPはあとで縮小コピーするので拡大して作画する必要あり

	// クライアント領域全体をグレーで塗りつぶす
	{
		RECT bmpRc;
		::GetClientRect(hwnd, &bmpRc);
		bmpRc.right  = (bmpRc.right  * nbmpCompatScale) / COMPAT_BMP_BASE;
		bmpRc.bottom = (bmpRc.bottom * nbmpCompatScale) / COMPAT_BMP_BASE;
		::FillRect(hdc, &bmpRc, (HBRUSH)::GetStockObject(GRAY_BRUSH));
	}

	// ツールバー高さ -> nToolBarHeight
	int nToolBarHeight = 0;
	if (hwndPrintPreviewBar) {
		RECT rc;
		::GetWindowRect(hwndPrintPreviewBar, &rc);
		nToolBarHeight = rc.bottom - rc.top;
	}

	// プリンタ情報の表示 -> IDD_PRINTPREVIEWBAR右上のSTATICへ
	TCHAR	szText[1024];
	::DlgItem_SetText(
		hwndPrintPreviewBar,
		IDC_STATIC_PRNDEV,
		pPrintSetting->mdmDevMode.szPrinterDeviceName
	);

	// 要素情報の表示 -> IDD_PRINTPREVIEWBAR右下のSTATICへ
	TCHAR	szPaperName[256];
	Print::GetPaperName(pPrintSetting->mdmDevMode.dmPaperSize , szPaperName);
	auto_sprintf_s(
		szText,
		_T("%ts  %ts"),
		szPaperName,
		(pPrintSetting->mdmDevMode.dmOrientation & DMORIENT_LANDSCAPE) ? LS(STR_ERR_DLGPRNPRVW1) : LS(STR_ERR_DLGPRNPRVW2)
	);
	::DlgItem_SetText(hwndPrintPreviewBar, IDC_STATIC_PAPER, szText);

	// バックグラウンド モードを変更
	::SetBkMode(hdc, TRANSPARENT);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        マッピング                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// マッピングモードの変更
	int nMapModeOld =
	::SetMapMode(hdc, MM_LOMETRIC);
	::SetMapMode(hdc, MM_ANISOTROPIC);

	// 出力倍率の変更
	SIZE sz;
	::GetWindowExtEx(hdc, &sz);
	int nCx = sz.cx;
	int nCy = sz.cy;
	nCx = (int)(((long)nCx) * 100L / ((long)nPreview_Zoom));
	nCy = (int)(((long)nCy) * 100L / ((long)nPreview_Zoom));
	// 作画時は、 COMPAT_BMP_SCALE/COMPAT_BMP_BASE倍の座標 (SetWindowExtExは逆なので反対になる)
	nCx = (nCx * COMPAT_BMP_BASE) / nbmpCompatScale;
	nCy = (nCy * COMPAT_BMP_BASE) / nbmpCompatScale;
	::SetWindowExtEx(hdc, nCx, nCy, &sz);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         フォント                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// フォント作成
	CreateFonts(hdc);
	// 印刷用半角フォントに設定し、以前のフォントを保持
	HFONT hFontOld = (HFONT)::SelectObject(hdc, hFontHan);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           原点                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 操作ウィンドウの下に物理座標原点を移動
	POINT poViewPortOld;
	::SetViewportOrgEx(
		hdc,
		((-1 * nPreviewHScrollPos) * nbmpCompatScale) / COMPAT_BMP_BASE, 
		((nToolBarHeight + nPreviewVScrollPos) * nbmpCompatScale) / COMPAT_BMP_BASE,
		&poViewPortOld
	);


	// 以下 0.1mm座標でレンダリング

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           背景                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 用紙の描画
	int	nDirectY = -1;	//	Y座標の下をプラス方向にするため？
	::Rectangle(hdc,
		nPreview_ViewMarginLeft,
		nDirectY * (nPreview_ViewMarginTop),
		nPreview_ViewMarginLeft + nPreview_PaperAllWidth + 1,
		nDirectY * (nPreview_ViewMarginTop + nPreview_PaperAllHeight + 1)
	);

	// マージン枠の表示
	Graphics gr(hdc);
	gr.SetPen(RGB(128, 128, 128)); // 2006.08.14 Moca 127を128に変更
	::Rectangle(hdc,
		nPreview_ViewMarginLeft + pPrintSetting->nPrintMarginLX,
		nDirectY * (nPreview_ViewMarginTop + pPrintSetting->nPrintMarginTY),
		nPreview_ViewMarginLeft + nPreview_PaperAllWidth - pPrintSetting->nPrintMarginRX + 1,
		nDirectY * (nPreview_ViewMarginTop + nPreview_PaperAllHeight - pPrintSetting->nPrintMarginBY)
	);
	gr.ClearPen();

	::SetTextColor(hdc, RGB(0, 0, 0));

	RECT rect;	// 紙の大きさをあらわすRECT
	rect.left   = nPreview_ViewMarginLeft +                             pPrintSetting->nPrintMarginLX + 5;
	rect.right  = nPreview_ViewMarginLeft + nPreview_PaperAllWidth - (pPrintSetting->nPrintMarginRX + 5);
	rect.top    = nDirectY * (nPreview_ViewMarginTop +                              pPrintSetting->nPrintMarginTY + 5);
	rect.bottom = nDirectY * (nPreview_ViewMarginTop + nPreview_PaperAllHeight - (pPrintSetting->nPrintMarginBY + 5));
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         テキスト                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	int nHeaderHeight = Print::CalcHeaderHeight(*pPrintSetting);

	// ヘッダ
	if (nHeaderHeight) {
		DrawHeaderFooter(hdc, rect, true);
	}

	ColorStrategy* pStrategyStart = DrawPageTextFirst(nCurPageNum);

	// 印刷/印刷Preview ページテキストの描画
	DrawPageText(
		hdc,
		nPreview_ViewMarginLeft + pPrintSetting->nPrintMarginLX,
		nPreview_ViewMarginTop  + pPrintSetting->nPrintMarginTY + nHeaderHeight*2,
		nCurPageNum,
		NULL,
		pStrategyStart
	);

	// フッタ
	if (Print::CalcFooterHeight(*pPrintSetting)) {
		DrawHeaderFooter(hdc, rect, false);
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                          後始末                             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//	印刷前のフォントに戻す
	::SelectObject(hdc, hFontOld);

	// マッピングモードの変更
	::SetMapMode(hdc, nMapModeOld);

	//	印刷用フォント破棄
	DestroyFonts();

	// 物理座標原点をもとに戻す
	::SetViewportOrgEx(hdc, poViewPortOld.x, poViewPortOld.y, NULL);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       実画面へ転送                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// MemoryDCを利用した再描画の場合はMemoryDCに描画した内容を画面へコピーする
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

	// 印刷Preview 操作バー
	int nToolBarHeight = 0;
	if (hwndPrintPreviewBar) {
		RECT			rc;
		::GetWindowRect(hwndPrintPreviewBar, &rc);
		nToolBarHeight = rc.bottom - rc.top;
		::MoveWindow(hwndPrintPreviewBar, 0, 0, cx, nToolBarHeight, TRUE);
	}

	// 印刷Preview 垂直スクロールバーウィンドウ
	int	nCxVScroll = ::GetSystemMetrics(SM_CXVSCROLL);
	int	nCyVScroll = ::GetSystemMetrics(SM_CYVSCROLL);
	if (hwndVScrollBar) {
		::MoveWindow(hwndVScrollBar, cx - nCxVScroll, nToolBarHeight, nCxVScroll, cy - nCyVScroll - nToolBarHeight, TRUE);
	}
	
	// 印刷Preview 水平スクロールバーウィンドウ
	int	nCxHScroll = ::GetSystemMetrics(SM_CXHSCROLL);
	int	nCyHScroll = ::GetSystemMetrics(SM_CYHSCROLL);
	if (hwndHScrollBar) {
		::MoveWindow(hwndHScrollBar, 0, cy - nCyHScroll, cx - nCxVScroll, nCyHScroll, TRUE);
	}
	
	// 印刷Preview サイズボックスウィンドウ
	if (hwndSizeBox) {
		::MoveWindow(hwndSizeBox, cx - nCxVScroll, cy - nCyHScroll, nCxHScroll, nCyVScroll, TRUE);
	}

	HDC hdc = ::GetDC(parentWnd.GetHwnd());
	int nMapModeOld = ::SetMapMode(hdc, MM_LOMETRIC);
	::SetMapMode(hdc, MM_ANISOTROPIC);

	// 出力倍率の変更
	SIZE sz;
	::GetWindowExtEx(hdc, &sz);
	int nCx = sz.cx;
	int nCy = sz.cy;
	nCx = (int)(((long)nCx) * 100L / ((long)nPreview_Zoom));
	nCy = (int)(((long)nCy) * 100L / ((long)nPreview_Zoom));
	::SetWindowExtEx(hdc, nCx, nCy, &sz);

	// ビューのサイズ
	POINT po;
	po.x = nPreview_PaperAllWidth + nPreview_ViewMarginLeft * 2;
	po.y = nPreview_PaperAllHeight + nPreview_ViewMarginTop * 2;
	::LPtoDP(hdc, &po, 1);

	// 再描画用メモリＢＭＰ
	if (hbmpCompatBMP) {
		::SelectObject(hdcCompatDC, hbmpCompatBMPOld);	// 再描画用メモリＢＭＰ(OLD)
		::DeleteObject(hbmpCompatBMP);
	}
	// 2007.02.11 Moca Previewを滑らかにする
	// Win9xでは 巨大なBMPは作成できないことと
	// StretchBltでSTRETCH_HALFTONEが未サポートであるので Win2K 以上のみで有効にする。
	if (IsDlgButtonChecked( hwndPrintPreviewBar, IDC_CHECK_ANTIALIAS )
		&& IsWin2000_or_later()
	) {
		nbmpCompatScale = COMPAT_BMP_SCALE;
	}else {
		// Win9x: BASE = SCALE で 1:1
		nbmpCompatScale = COMPAT_BMP_BASE;
	}
	hbmpCompatBMP = ::CreateCompatibleBitmap(hdc, (cx * nbmpCompatScale + COMPAT_BMP_BASE - 1) / COMPAT_BMP_BASE,
		(cy * nbmpCompatScale + COMPAT_BMP_BASE - 1) / COMPAT_BMP_BASE);
	hbmpCompatBMPOld = (HBITMAP)::SelectObject(hdcCompatDC, hbmpCompatBMP);

	::SetMapMode(hdc, nMapModeOld);

	::ReleaseDC(parentWnd.GetHwnd(), hdc);

	// 印刷Preview：ビュー幅(ピクセル)
	nPreview_ViewWidth = abs(po.x);
	
	// 印刷Preview：ビュー高さ(ピクセル)
	nPreview_ViewHeight = abs(po.y);
	
	// 印刷Preview スクロールバー初期化
	InitPreviewScrollBar();
	
	// 印刷Preview スクロールバーの初期化
	
	parentWnd.SetDragPosOrg(Point(0, 0));
	parentWnd.SetDragMode(true);
	OnMouseMove(0, MAKELONG(0, 0));
	parentWnd.SetDragMode(false);
	//	SizeBox問題テスト
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
	@date 2006.08.14 Moca SB_TOP, SB_BOTTOMへの対応
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
	int nPos = si.nTrackPos; // 2013.05.30 32bit対応
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
	// 2006.08.14 Moca SB_TOP, SB_BOTTOMへの対応
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
		// 描画
		::ScrollWindowEx(parentWnd.GetHwnd(), 0, nMove, NULL, NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE);
	}
	return 0;
}

/*!
	@date 2006.08.14 Moca SB_LEFT, SB_RIGHTへの対応
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
	int nPos = si.nTrackPos; // 2013.05.30 32bit対応
	//nNowPos = GetScrollPosだとロジクールのSetPointで不具合があり、nPos == nNowPosになってしまう
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
	// 2006.08.14 Moca SB_LEFT, SB_RIGHTへの対応
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
		// 描画
		::ScrollWindowEx(parentWnd.GetHwnd(), nMove, 0, NULL, NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE);
	}
	return 0;
}

LRESULT PrintPreview::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	// 手カーソル
	SetHandCursor();		// Hand Cursorを設定 2013/1/29 Uchi
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
	if (!PtInRect(&rc, po)) {	//	Preview内かチェック。
		return 0;
	}

	//	Y軸
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

	//	X軸
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
	// 描画
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
		// 印刷Preview 垂直スクロールバーメッセージ処理 WM_VSCROLL
		::PostMessage(parentWnd.GetHwnd(), WM_VSCROLL, MAKELONG(nScrollCode, 0), (LPARAM)hwndVScrollBar);

		// 処理中のユーザー操作を可能にする
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
	::SetMapMode(hdc, MM_LOMETRIC); // MM_HIMETRIC それぞれの論理単位は、0.01 mm にマップされます
	::SetMapMode(hdc, MM_ANISOTROPIC);

	::EnumFontFamilies(
		hdc,
		NULL,
		(FONTENUMPROC)PrintPreview::MyEnumFontFamProc,
		(LPARAM)this
	);

	bool bLockOld = bLockSetting;
	bLockSetting = true;

	// 2009.08.08 印刷で用紙サイズ、横指定が効かない問題対応 syat
	// DEVMODE構造体が設定されていなかったら既定のプリンタを設定
	if (pPrintSetting->mdmDevMode.szPrinterDeviceName[0] == L'\0') {
		GetDefaultPrinterInfo();
	}

	// 印刷Preview表示情報
	nPreview_LineNumberColumns = 0;	// 行番号エリアの幅(文字数)

	// 行番号を表示するか
	if (pPrintSetting->bPrintLineNumber) {
		// 行番号表示に必要な桁数を計算
		nPreview_LineNumberColumns = parentWnd.GetActiveView().GetTextArea().DetectWidthOfLineNumberArea_calculate(pLayoutMgr_Print);
	}
	// 現在のページ設定の、用紙サイズと用紙方向を反映させる
	pPrintSetting->mdmDevMode.dmPaperSize = pPrintSetting->nPrintPaperSize;
	pPrintSetting->mdmDevMode.dmOrientation = pPrintSetting->nPrintPaperOrientation;
	// 用紙サイズ、用紙方向は変更したのでビットを立てる
	pPrintSetting->mdmDevMode.dmFields |= (DM_ORIENTATION | DM_PAPERSIZE);
	// 用紙の長さ、幅は決まっていないので、ビットを下ろす
	pPrintSetting->mdmDevMode.dmFields &= (~DM_PAPERLENGTH);
	pPrintSetting->mdmDevMode.dmFields &= (~DM_PAPERWIDTH);

	// 印刷/Previewに必要な情報を取得
	TCHAR szErrMsg[1024];
	if (!print.GetPrintMetrics(
		&pPrintSetting->mdmDevMode,	// プリンタ設定 DEVMODE用
		&nPreview_PaperAllWidth,		// 用紙幅
		&nPreview_PaperAllHeight,		// 用紙高さ
		&nPreview_PaperWidth,			// 用紙印刷有効幅
		&nPreview_PaperHeight,		// 用紙印刷有効高さ
		&nPreview_PaperOffsetLeft,	// 印刷可能位置左端
		&nPreview_PaperOffsetTop,		// 印刷可能位置上端
		szErrMsg						// エラーメッセージ格納場所
		)
	) {
		// エラーの場合、A4縦(210mm×297mm)で初期化
		nPreview_PaperAllWidth = 210 * 10;	// 用紙幅
		nPreview_PaperAllHeight = 297 * 10;	// 用紙高さ
		nPreview_PaperWidth = 210 * 10;		// 用紙印刷有効幅
		nPreview_PaperHeight = 297 * 10;		// 用紙印刷有効高さ
		nPreview_PaperOffsetLeft = 0;			// 印刷可能位置左端
		nPreview_PaperOffsetTop = 0;			// 印刷可能位置上端
		// DEVMODE構造体もA4縦で初期化 2003.07.03 かろと
		pPrintSetting->mdmDevMode.dmPaperSize = DMPAPER_A4;
		pPrintSetting->mdmDevMode.dmOrientation = DMORIENT_PORTRAIT;
		pPrintSetting->mdmDevMode.dmPaperLength = nPreview_PaperHeight;
		pPrintSetting->mdmDevMode.dmPaperWidth = nPreview_PaperWidth;
		pPrintSetting->mdmDevMode.dmFields |= (DM_ORIENTATION | DM_PAPERSIZE | DM_PAPERLENGTH | DM_PAPERWIDTH);
	}else {
		if (pPrintSetting->nPrintPaperSize != pPrintSetting->mdmDevMode.dmPaperSize) {
			TCHAR szPaperNameOld[256];
			TCHAR szPaperNameNew[256];
			// 用紙の名前を取得
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
	// 現在のページ設定の、用紙サイズと用紙方向を反映させる(エラーでA4縦になった場合も考慮してif文の外へ移動 2003.07.03 かろと)
	pPrintSetting->nPrintPaperSize = pPrintSetting->mdmDevMode.dmPaperSize;
	pPrintSetting->nPrintPaperOrientation = pPrintSetting->mdmDevMode.dmOrientation;	// 用紙方向の反映忘れを修正 2003/07/03 かろと

	// プリンタ設定はここで変更されるがそれぞれのウィンドウで再設定するので更新メッセージは投げない
	*pPrintSettingOrg = *pPrintSetting;

	nPreview_ViewMarginLeft = 8 * 10;		// 印刷Preview：ビュー左端と用紙の間隔(1/10mm単位)
	nPreview_ViewMarginTop = 8 * 10;		// 印刷Preview：ビュー左端と用紙の間隔(1/10mm単位)

	// 行あたりの文字数(行番号込み)
	bPreview_EnableColumns = Print::CalculatePrintableColumns(*pPrintSetting, nPreview_PaperAllWidth, nPreview_LineNumberColumns);	// 印字可能桁数/ページ
	// 縦方向の行数
	bPreview_EnableLines = Print::CalculatePrintableLines(*pPrintSetting, nPreview_PaperAllHeight);			// 印字可能行数/ページ

	// 印字可能領域がない場合は印刷Previewを終了する 2013.5.10 aroka
	if (bPreview_EnableColumns == 0 || bPreview_EnableLines == 0) {
		parentWnd.PrintPreviewModeONOFF();
		parentWnd.SendStatusMessage(LS(STR_ERR_DLGPRNPRVW3_1));
		return;
	}

	// 印刷用のレイアウト管理情報の初期化
	pLayoutMgr_Print->Create(&parentWnd.GetDocument(), &parentWnd.GetDocument().docLineMgr);

	// 印刷用のレイアウト情報の変更
	// タイプ別設定をコピー
	typePrint = parentWnd.GetDocument().docType.GetDocumentAttribute();
	TypeConfig& ref = typePrint;

	ref.nMaxLineKetas = 	bPreview_EnableColumns;
	ref.bWordWrap =		pPrintSetting->bPrintWordWrap;	// 英文ワードラップをする
	//	Sep. 23, 2002 genta LayoutMgrの値を使う
	ref.nTabSpace =		parentWnd.GetDocument().layoutMgr.GetTabSpace();

	//@@@ 2002.09.22 YAZAKI
	ref.lineComment.CopyTo(0, L"", -1);	// 行コメントデリミタ
	ref.lineComment.CopyTo(1, L"", -1);	// 行コメントデリミタ2
	ref.lineComment.CopyTo(2, L"", -1);	// 行コメントデリミタ3	// Jun. 01, 2001 JEPRO 追加
	ref.blockComments[0].SetBlockCommentRule(L"", L"");	// ブロックコメントデリミタ
	ref.blockComments[1].SetBlockCommentRule(L"", L"");	// ブロックコメントデリミタ2

	ref.stringType = StringLiteralType::CPP;		// 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""]['']
	ref.colorInfoArr[COLORIDX_COMMENT].bDisp = false;
	ref.colorInfoArr[COLORIDX_SSTRING].bDisp = false;
	ref.colorInfoArr[COLORIDX_WSTRING].bDisp = false;
	ref.bKinsokuHead = pPrintSetting->bPrintKinsokuHead,	// 行頭禁則する	//@@@ 2002.04.08 MIK
	ref.bKinsokuTail = pPrintSetting->bPrintKinsokuTail,	// 行末禁則する	//@@@ 2002.04.08 MIK
	ref.bKinsokuRet = pPrintSetting->bPrintKinsokuRet,	// 改行文字をぶら下げる	//@@@ 2002.04.13 MIK
	ref.bKinsokuKuto = pPrintSetting->bPrintKinsokuKuto,	// 句読点をぶら下げる	//@@@ 2002.04.17 MIK
	pLayoutMgr_Print->SetLayoutInfo(true, ref, ref.nTabSpace, ref.nMaxLineKetas);
	nAllPageNum = (WORD)(pLayoutMgr_Print->GetLineCount() / (bPreview_EnableLines * pPrintSetting->nPrintDansuu));		// 全ページ数
	if (0 < pLayoutMgr_Print->GetLineCount() % (bPreview_EnableLines * pPrintSetting->nPrintDansuu)) {
		++nAllPageNum;
	}
	if (nAllPageNum <= nCurPageNum) {	// 現在のページ
		nCurPageNum = 0;
	}

	// WM_SIZE 処理
	RECT rc;
	::GetClientRect(parentWnd.GetHwnd(), &rc);
	OnSize(SIZE_RESTORED, MAKELONG(rc.right - rc.left, rc.bottom - rc.top));
	::ReleaseDC(parentWnd.GetHwnd(), hdc);
	// Preview ページ指定
	OnPreviewGoPage(nCurPageNum);
	bLockSetting = bLockOld;

	// 2014.07.23 レイアウト行番号で行番号幅が合わない時は再計算
	if (pPrintSetting->bPrintLineNumber) {
		// 行番号表示に必要な桁数を計算
		int tempLineNum = parentWnd.GetActiveView().GetTextArea().DetectWidthOfLineNumberArea_calculate(pLayoutMgr_Print);
		if (nPreview_LineNumberColumns != tempLineNum) {
			OnChangeSetting();
		}
	}
	if (bDemandUpdateSetting) {
		// やりなおし
		OnChangeSetting();
	}
	return;
}

/*! @brief ページ番号直接指定によるジャンプ

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
	if (nAllPageNum <= nPage) {	// 現在のページ
		nPage = nAllPageNum - 1;
	}
	if (0 > nPage) {				// 現在のページ
		nPage = 0;
	}
	nCurPageNum = (short)nPage;

	//	2008.01.29 nasukoji	印刷枚数が2枚の時操作できなくなることへの対処（SetFocusを移動）
	//	2008.02.01 genta : ボタンのフォーカスが元の動作になるようにするため，
	//		前ボタンのDisableを後ろへ移動した．
	//		操作できない現象は「次へ」がDisableにも関わらずフォーカスを与えていたため．
	//		次・前どちらも，ボタン有効化→フォーカス移動→ボタン無効化の順にした
	if (0 < nCurPageNum) {
		//	前のページボタンをオン
		::EnableWindow(::GetDlgItem(hwndPrintPreviewBar, IDC_BUTTON_PREVPAGE), TRUE);
	}

	if (nAllPageNum <= nCurPageNum + 1) {
		//	最後のページのときは、次のページボタンをオフ。
		//	Jul. 18, 2001 genta FocusのあるWindowをDisableにすると操作できなくなるのを回避
		//	Mar. 9, 2003 genta 1ページしか無いときは「前へ」ボタンもDisableされているので、
		//	最後のページまで達したら「戻る」にフォーカスを移すように
		::SetFocus(::GetDlgItem(hwndPrintPreviewBar, IDCANCEL));
		::EnableWindow(::GetDlgItem(hwndPrintPreviewBar, IDC_BUTTON_NEXTPAGE), FALSE);
	}else {
		//	次のページボタンをオン。
		::EnableWindow(::GetDlgItem(hwndPrintPreviewBar, IDC_BUTTON_NEXTPAGE), TRUE);
	}

	if (nCurPageNum == 0) {
		//	最初のページのときは、前のページボタンをオフ。
		//	Jul. 18, 2001 genta FocusのあるWindowをDisableにすると操作できなくなるのを回避
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
		nPreview_Zoom += 10;	// 印刷Preview倍率
		if (MAX_PREVIEW_ZOOM < nPreview_Zoom) {
			nPreview_Zoom = MAX_PREVIEW_ZOOM;
		}
	}else {
		// スクロール位置を調整
		nPreviewVScrollPos = 0;
		nPreviewHScrollPos = 0;

		nPreview_Zoom -= 10;	// 印刷Preview倍率
		if (MIN_PREVIEW_ZOOM > nPreview_Zoom) {
			nPreview_Zoom = MIN_PREVIEW_ZOOM;
		}
	}
	
	//	縮小ボタンのON/OFF
	if (nPreview_Zoom == MIN_PREVIEW_ZOOM) {
		// 2013.05.30 FocusがDisableなウィンドウだとマウススクロールできない対策
		HWND focus = ::GetFocus();
		if (focus == GetDlgItem(hwndPrintPreviewBar, IDC_BUTTON_ZOOMDOWN)) {
			::SetFocus(parentWnd.GetHwnd());
		}
		::EnableWindow(::GetDlgItem(hwndPrintPreviewBar, IDC_BUTTON_ZOOMDOWN), FALSE);
	}else {
		::EnableWindow(::GetDlgItem(hwndPrintPreviewBar, IDC_BUTTON_ZOOMDOWN), TRUE);
	}
	//	拡大ボタンのON/OFF
	if (nPreview_Zoom == MAX_PREVIEW_ZOOM) {
		// 2013.05.30 FocusがDisableなウィンドウだとマウススクロールできない対策
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

	// WM_SIZE 処理
	RECT rc1;
	::GetClientRect(parentWnd.GetHwnd(), &rc1);
	OnSize(SIZE_RESTORED, MAKELONG(rc1.right - rc1.left, rc1.bottom - rc1.top));

	// 印刷Preview スクロールバー初期化
	InitPreviewScrollBar();

	// 再描画
	::InvalidateRect(parentWnd.GetHwnd(), NULL, TRUE);
	return;
}


/*!
	滑らか
	チェック時、2倍(COMPAT_BMP_SCALE/COMPAT_BMP_BASE)サイズでレンダリングする
*/
void PrintPreview::OnCheckAntialias(void)
{
	// WM_SIZE 処理
	RECT rc;
	::GetClientRect(parentWnd.GetHwnd(), &rc);
	OnSize(SIZE_RESTORED, MAKELONG(rc.right - rc.left, rc.bottom - rc.top));
}


/*!
	印刷
*/
void PrintPreview::OnPrint(void)
{
	if (nAllPageNum == 0) {
		TopWarningMessage(parentWnd.GetHwnd(), LS(STR_ERR_DLGPRNPRVW7));
		return;
	}

	// プリンタに渡すジョブ名を生成
	TCHAR szJobName[256 + 1];
	if (!parentWnd.GetDocument().docFile.GetFilePathClass().IsValidPath()) {	// 現在編集中のファイルのパス
		_tcscpy_s(szJobName, LS(STR_NO_TITLE2));
	}else {
		TCHAR szFileName[_MAX_FNAME];
		TCHAR szExt[_MAX_EXT];
		_tsplitpath(parentWnd.GetDocument().docFile.GetFilePath(), NULL, NULL, szFileName, szExt);
		auto_snprintf_s(szJobName, _countof(szJobName), _T("%ts%ts"), szFileName, szExt);
	}

	// 印刷範囲を指定できるプリンタダイアログを作成
	//	2003.05.02 かろと
	PRINTDLG pd = {0};
#ifndef _DEBUG
// Debugモードで、hwndOwnerを指定すると、Win2000では落ちるので・・・
	pd.hwndOwner = parentWnd.GetHwnd();
#endif
	pd.nMinPage = 1;
	pd.nMaxPage = nAllPageNum;
	pd.nFromPage = 1;
	pd.nToPage = nAllPageNum;
	pd.Flags = PD_ALLPAGES | PD_NOSELECTION | PD_USEDEVMODECOPIESANDCOLLATE;

	bLockSetting = true; // プリント設定でページ数がきまるのでロックする

	if (!print.PrintDlg(&pd, &pPrintSetting->mdmDevMode)) {
		bLockSetting = false;
		if (bDemandUpdateSetting) {
			OnChangePrintSetting();
		}
		return;
	}
	if (memcmp(&pPrintSettingOrg->mdmDevMode, &pPrintSetting->mdmDevMode, sizeof(pPrintSetting->mdmDevMode)) != 0) {
		pPrintSettingOrg->mdmDevMode = pPrintSetting->mdmDevMode;
		// 自分はLockで更新しない
		AppNodeGroupHandle(0).PostMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0,
			(LPARAM)PM_PrintSetting,
			EditWnd::getInstance().GetHwnd()
		);
	}

	// 印刷開始ページと、印刷ページ数を確認
	WORD nFrom;
	WORD nNum;
	if ((pd.Flags & PD_PAGENUMS) != 0) {	// 2003.05.02 かろと
		nFrom = pd.nFromPage - 1;
		nNum  = pd.nToPage - nFrom;
	}else {
		nFrom = 0;
		nNum  = nAllPageNum;
	}

	// 印刷過程を表示して、キャンセルするためのダイアログを作成
	DlgCancel	dlgPrinting;
	dlgPrinting.DoModeless(EditApp::getInstance().GetAppInstance(), parentWnd.GetHwnd(), IDD_PRINTING);
	dlgPrinting.SetItemText(IDC_STATIC_JOBNAME, szJobName);
	dlgPrinting.SetItemText(IDC_STATIC_PROGRESS, _T(""));	// XPS対応 2013/5/8 Uchi

	// 親ウィンドウを無効化
	::EnableWindow(parentWnd.GetHwnd(), FALSE);

	// 2013.06.10 Moca キーワード強調設定をロックして、印刷中に共通設定を更新されないようにする
	ShareDataLockCounter lock;

	// 印刷 ジョブ開始
	HDC hdc;
	TCHAR szErrMsg[1024];
	if (!print.PrintOpen(
		szJobName,
		&pPrintSetting->mdmDevMode,	// プリンタ設定 DEVMODE用
		&hdc,
		szErrMsg						// エラーメッセージ格納場所
		)
	) {
//		MYTRACE(_T("%ts\n"), szErrMsg);
	}

	// 印刷用半角フォントと、印刷用全角フォントを作成
	CreateFonts(hdc);
	// 現在のフォントを印刷用半角フォントに設定＆以前のフォントを保持
	// OnPrint以前のフォント
	HFONT hFontOld = (HFONT)::SelectObject(hdc, hFontHan);

	// 紙の大きさをあらわすRECTを設定
	int nDirectY = -1;
	RECT rect;
	rect.left   =                             pPrintSetting->nPrintMarginLX - nPreview_PaperOffsetLeft + 5;
	rect.right  = nPreview_PaperAllWidth - (pPrintSetting->nPrintMarginRX + nPreview_PaperOffsetLeft + 5);
	rect.top    = nDirectY * (                             pPrintSetting->nPrintMarginTY - nPreview_PaperOffsetTop + 5);
	rect.bottom = nDirectY * (nPreview_PaperAllHeight - (pPrintSetting->nPrintMarginBY + nPreview_PaperOffsetTop + 5));

	// ヘッダ・フッタの$pを展開するために、nCurPageNumを保持
	WORD nCurPageNumOld = nCurPageNum;
	ColorStrategy* pStrategy = DrawPageTextFirst(nCurPageNum);
	TCHAR szProgress[100];
	for (int i=0; i<nNum; ++i) {
		nCurPageNum = nFrom + (WORD)i;

		// 印刷過程を表示
		//	Jun. 18, 2001 genta ページ番号表示の計算ミス修正
		auto_sprintf_s(szProgress, _T("%d/%d"), i + 1, nNum);
		dlgPrinting.SetItemText(IDC_STATIC_PROGRESS, szProgress);

		// 印刷 ページ開始
		print.PrintStartPage(hdc);

		//	From Here Jun. 26, 2003 かろと / おきた
		//	Windows 95/98ではStartPage()関数の呼び出し時に、属性はリセットされて既定値へ戻ります．
		//	このとき開発者は次のページの印刷を始める前にオブジェクトを選択し直し，
		//	マッピングモードをもう一度設定しなければなりません
		//	Windows NT/2000ではStartPageでも属性はリセットされません．

		// マッピングモードの変更
		::SetMapMode(hdc, MM_LOMETRIC);		// それぞれの論理単位は、0.1 mm にマップされます
		::SetMapMode(hdc, MM_ANISOTROPIC);	// 論理単位は、任意にスケーリングされた軸上の任意の単位にマップされます

		// 現在のフォントを印刷用半角フォントに設定
		::SelectObject(hdc, hFontHan);
		//	To Here Jun. 26, 2003 かろと / おきた

		int nHeaderHeight = Print::CalcHeaderHeight(*pPrintSetting);

		// ヘッダ印刷
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
		// 印刷/印刷Preview ページテキストの描画
		pStrategy = DrawPageText(
			hdc,
			pPrintSetting->nPrintMarginLX - nPreview_PaperOffsetLeft ,
			pPrintSetting->nPrintMarginTY - nPreview_PaperOffsetTop + nHeaderHeight*2,
			nFrom + i,
			&dlgPrinting,
			pStrategy
		);

		// フッタ印刷
		if (Print::CalcFooterHeight(*pPrintSetting)) {
			DrawHeaderFooter(hdc, rect, false);
		}

		// 印刷 ページ終了
		print.PrintEndPage(hdc);

		// 中断ボタン押下チェック
		if (dlgPrinting.IsCanceled()) {
			break;
		}
	}
	//	印刷前のフォントに戻す 2003.05.02 かろと hdc解放の前に処理順序を変更
	::SelectObject(hdc, hFontOld);

	// 印刷 ジョブ終了
	print.PrintClose(hdc);

	//	印刷用フォント破棄
	DestroyFonts();

	::EnableWindow(parentWnd.GetHwnd(), TRUE);
	dlgPrinting.CloseDialog(0);

	nCurPageNum = nCurPageNumOld;

	bLockSetting = false;

	// 印刷が終わったら、Previewから抜ける 2003.05.02 かろと
	parentWnd.PrintPreviewModeONOFF();
	return;
}


// Tab文字をSpace文字に置換え
static void Tab2Space(wchar_t* pTrg)
{
	for (; *pTrg!=L'\0'; ++pTrg) {
		if (*pTrg == L'\t') {
			*pTrg = L' ';
		}
	}
}


/*! 印刷/印刷Preview ヘッダ･フッタの描画
*/
void PrintPreview::DrawHeaderFooter(HDC hdc, const Rect& rect, bool bHeader)
{
	bool		bFontSetting = (bHeader ? pPrintSetting->lfHeader.lfFaceName[0] : pPrintSetting->lfFooter.lfFaceName[0]) != _T('\0');
	const int	nWorkLen = 1024;
	wchar_t		szWork[1024 + 1];
	size_t		nLen;

	if (bFontSetting) {
		// フォント作成
		LOGFONT	lf = (bHeader ? pPrintSetting->lfHeader : pPrintSetting->lfFooter);
		lf.lfHeight = -(bHeader ? pPrintSetting->nHeaderPointSize : pPrintSetting->nFooterPointSize) * 254 / 720;	// フォントのサイズ計算(pt->1/10mm)
		HFONT hFontForce = ::CreateFontIndirect(&lf);

		// フォント設定
		HFONT hFontOld = (HFONT)::SelectObject(hdc, hFontForce);

		// TextMetricの取得
		TEXTMETRIC tm;
		::GetTextMetrics(hdc, &tm);

		// Y座標基準
		int nY = bHeader ? rect.top : (rect.bottom + tm.tmHeight);

		// 左寄せ
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

		// 中央寄せ
		SakuraEnvironment::ExpandParameter(
			bHeader ? pPrintSetting->szHeaderForm[POS_CENTER] : pPrintSetting->szFooterForm[POS_CENTER],
			szWork, nWorkLen);
		Tab2Space(szWork);
		SIZE size;
		nLen = wcslen(szWork);
		::GetTextExtentPoint32W(hdc, szWork, (int)nLen, &size);		// テキスト幅
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

		// 右寄せ
		SakuraEnvironment::ExpandParameter(
			bHeader ? pPrintSetting->szHeaderForm[POS_RIGHT] : pPrintSetting->szFooterForm[POS_RIGHT],
			szWork, nWorkLen);
		Tab2Space(szWork);
		nLen = wcslen(szWork);
		::GetTextExtentPoint32W(hdc, szWork, (int)nLen, &size);		// テキスト幅
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
		// フォントの戻し
		::SelectObject(hdc, hFontOld);
		::DeleteObject(hFontForce);
	}else {
		// 文字間隔
		int nDx = pPrintSetting->nPrintFontWidth;

		// Y座標基準
		int nY = bHeader ? rect.top : (rect.bottom + pPrintSetting->nPrintFontHeight);

		// 左寄せ
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

		// 中央寄せ
		SakuraEnvironment::ExpandParameter(
			bHeader ? pPrintSetting->szHeaderForm[POS_CENTER] : pPrintSetting->szFooterForm[POS_CENTER],
			szWork, nWorkLen);
		nLen = wcslen(szWork);
		size_t nTextWidth = TextMetrics::CalcTextWidth2(szWork, nLen, nDx); // テキスト幅
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

		// 右寄せ
		SakuraEnvironment::ExpandParameter(
			bHeader ? pPrintSetting->szHeaderForm[POS_RIGHT] : pPrintSetting->szFooterForm[POS_RIGHT],
			szWork, nWorkLen);
		nLen = wcslen(szWork);
		nTextWidth = TextMetrics::CalcTextWidth2(szWork, nLen, nDx); // テキスト幅
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

/* 印刷/印刷Preview ページテキストの色分け処理
	最初のページ用
	@date 2013.05.19 Moca 新規追加 
*/
ColorStrategy* PrintPreview::DrawPageTextFirst(int nPageNum)
{
	// ページトップの色指定を取得
	ColorStrategy*	pStrategy = nullptr;
	if (pPrintSetting->bColorPrint) {
		pool = &ColorStrategyPool::getInstance();
		pool->SetCurrentView(&(parentWnd.GetActiveView()));

		const int nPageTopLineNum = (nPageNum * pPrintSetting->nPrintDansuu) * bPreview_EnableLines;
		const Layout*	pPageTopLayout = pLayoutMgr_Print->SearchLineByLayoutY(nPageTopLineNum);

		if (pPageTopLayout) {
			const int nPageTopOff = pPageTopLayout->GetLogicOffset();

			// ページトップの物理行の先頭を検索
			while (pPageTopLayout->GetLogicOffset()) {
				pPageTopLayout = pPageTopLayout->GetPrevLayout();
			}

			// 論理行先頭のColorStrategy取得
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


/* 印刷/印刷Preview ページテキストの描画
	DrawPageTextでは、行番号を（半角フォントで）印刷。
	本文はPrint_DrawLineにお任せ
	@date 2006.08.14 Moca 共通式のくくりだしと、コードの整理 
	@date 2013.05.19 Moca 色分け処理のpStrategyをページをまたいで利用する
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
	// 段と段の間隔の幅
	const int		nDanWidth = bPreview_EnableColumns * pPrintSetting->nPrintFontWidth + pPrintSetting->nPrintDanSpace;
	// 行番号の幅
	const int		nLineNumWidth = nPreview_LineNumberColumns * pPrintSetting->nPrintFontWidth;

	// 半角フォントの情報を取得＆半角フォントに設定

	// ページトップの色指定を取得
	ColorStrategy*	pStrategy = pStrategyStart;

	auto& typeConfig = parentWnd.GetDocument().docType.GetDocumentAttribute();

	// 段数ループ
	for (int nDan=0; nDan<pPrintSetting->nPrintDansuu; ++nDan) {
		// 本文1桁目の左隅の座標(行番号がある場合はこの座標より左側)
		const int nBasePosX = nOffX + nDanWidth * nDan + nLineNumWidth * (nDan + 1);
		
		int i; //	行数カウンタ
		for (i=0; i<bPreview_EnableLines; ++i) {
			if (pDlgCancel) {
				// 処理中のユーザー操作を可能にする
				if (!::BlockingHook(pDlgCancel->GetHwnd())) {
					return nullptr;
				}
			}

			/*	現在描画しようとしている行の物理行数（折り返しごとにカウントした行数）
				関係するものは、
				「ページ数（nPageNum）」
				「段数（pPrintSetting->nPrintDansuu）」
				「段数が1のときに、1ページあたりに何行入るか（bPreview_EnableLines）」
			*/
			const int nLineNum = (nPageNum * pPrintSetting->nPrintDansuu + nDan) * bPreview_EnableLines + i;
			const Layout* pLayout = pLayoutMgr_Print->SearchLineByLayoutY(nLineNum);
			if (!pLayout) {
				break;
			}
			// 行番号を表示するか
			if (pPrintSetting->bPrintLineNumber) {
				wchar_t szLineNum[64];	//	行番号を入れる。
				// 行番号の表示 false=折り返し単位／true=改行単位
				if (typeConfig.bLineNumIsCRLF) {
					// 論理行番号表示モード
					if (pLayout->GetLogicOffset() != 0) { // 折り返しレイアウト行
						wcscpy_s(szLineNum, L" ");
					}else {
						_itow(pLayout->GetLogicLineNo() + 1, szLineNum, 10);	// 対応する論理行番号
					}
				}else {
					// 物理行(レイアウト行)番号表示モード
					_itow(nLineNum + 1, szLineNum, 10);
				}

				// 行番号区切り  0=なし 1=縦線 2=任意
				if (typeConfig.nLineTermType == 2) {
					wchar_t szLineTerm[2];
					szLineTerm[0] = typeConfig.cLineTermChar;	// 行番号区切り文字
					szLineTerm[1] = L'\0';
					wcscat(szLineNum, szLineTerm);
				}else {
					wcscat(szLineNum, L" ");
				}

				// 文字列長
				const size_t nLineCols = wcslen(szLineNum);

				// 文字間隔配列を生成
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

			// 物理行頭の色指定を取得
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
			// 印刷／Preview 行描画
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
				pLayout->GetIndent(), // 2006.05.16 Add Moca. レイアウトインデント分ずらす。
				pPrintSetting->bColorPrint ? pLayout : NULL,
				pStrategy
			);
		}

		// 2006.08.14 Moca 行番号が縦線の場合は1度に引く
		if (pPrintSetting->bPrintLineNumber
			&& typeConfig.nLineTermType == 1
		) {
			// 縦線は本文と行番号の隙間1桁の中心に作画する(画面作画では、右詰め)
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


// 印刷Preview スクロールバー初期化
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
		// 垂直スクロールバー
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		if (nPreview_ViewHeight <= cy - nToolBarHeight) {
			si.nMax  = cy - nToolBarHeight;			// 全幅
			si.nPage = cy - nToolBarHeight;			// 表示域の桁数
			si.nPos  = -1 * nPreviewVScrollPos;	// 表示域の一番左の位置
			si.nTrackPos = 0;
			SCROLLBAR_VERT = FALSE;
		}else {
			si.nMax  = nPreview_ViewHeight;		// 全幅
			si.nPage = cy - nToolBarHeight;			// 表示域の桁数
			si.nPos  = -1 * nPreviewVScrollPos;	// 表示域の一番左の位置
			si.nTrackPos = 100;
			SCROLLBAR_VERT = TRUE;
		}
		::SetScrollInfo(hwndVScrollBar, SB_CTL, &si, TRUE);
	}
	// 印刷Preview 水平スクロールバーウィンドウハンドル
	if (hwndHScrollBar) {
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		// 水平スクロールバー
//		si.cbSize = sizeof(si);
//		si.fMask = SIF_ALL;
		si.nMin  = 0;
		if (nPreview_ViewWidth <= cx) {
			si.nMax  = cx;							// 全幅
			si.nPage = cx;							// 表示域の桁数
			si.nPos  = nPreviewHScrollPos;		// 表示域の一番左の位置
			si.nTrackPos = 0;
			SCROLLBAR_HORZ = FALSE;
		}else {
			si.nMax  = nPreview_ViewWidth;		// 全幅
			si.nPage = cx;							// 表示域の桁数
			si.nPos  = nPreviewHScrollPos;		// 表示域の一番左の位置
			si.nTrackPos = 100;
			SCROLLBAR_HORZ = TRUE;
		}
		::SetScrollInfo(hwndHScrollBar, SB_CTL, &si, TRUE);
	}
	return;
}

/*! 印刷／Preview 行描画
	@param[in] nIndent 行頭折り返しインデント桁数

	@date 2006.08.14 Moca   折り返しインデントが印刷時に反映されるように
	@date 2007.08    kobake 機械的にUNICODE化
	@date 2007.12.12 kobake 全角フォントが反映されていない問題を修正
*/
ColorStrategy* PrintPreview::Print_DrawLine(
	HDC				hdc,
	POINT			ptDraw,		// 描画座標。HDC内部単位。
	const wchar_t*	pLine,
	size_t			nDocLineLen,
	size_t			nLineStart,
	size_t			nLineLen,
	size_t			nIndent,	// 2006.08.14 Moca 追加
	const Layout*	pLayout,	// 色付用Layout
	ColorStrategy*	pStrategyStart
	)
{
	if (nLineLen == 0) {
		return pStrategyStart;
	}

	/*	pLineをスキャンして、半角文字は半角文字でまとめて、全角文字は全角文字でまとめて描画する。
	*/

	// 文字間隔
	int nDx = pPrintSetting->nPrintFontWidth;

	// タブ幅取得
	size_t nTabSpace = parentWnd.GetDocument().layoutMgr.GetTabSpace(); //	Sep. 23, 2002 genta LayoutMgrの値を使う

	// 文字間隔配列を生成
	vector<int> vDxArray;
	const int* pDxArray = TextMetrics::GenerateDxArray(
		&vDxArray,
		pLine + nLineStart,
		nLineLen,
		nDx,
		nTabSpace,
		nIndent
	);

	size_t nBgnLogic = nLineStart;	// TABを展開する前のバイト数で、pLineの何バイト目まで描画したか？
	size_t iLogic;					// pLineの何文字目をスキャン？
	size_t nLayoutX = nIndent;	// TABを展開した後のバイト数で、テキストの何バイト目まで描画したか？

	// 文字種判定フラグ
	int nKind     = 0; // 0:半角 1:全角 2:タブ
	int nKindLast = 2; // 直前のnKind状態

	// 色設定	2012-03-07 ossan
	StringRef cStringLine(pLine, nDocLineLen);
	ColorStrategy* pStrategy = pStrategyStart;
	// 2014.12.30 色はGetColorStrategyで次の色になる前に取得する必要がある
	int nColorIdx = ToColorInfoArrIndex( pStrategy ? pStrategy->GetStrategyColor() : COLORIDX_TEXT );

	for (
		iLogic = nLineStart;
		iLogic < nLineStart + nLineLen; 
		++iLogic, nKindLast = nKind
	) {
		// 文字の種類
		if (pLine[iLogic] == WCODE::TAB) {
			nKind = 2;
		}else if (WCODE::IsHankaku(pLine[iLogic])) {
			nKind = 0;
		}else {
			nKind = 1;
		}

		bool bChange = false;
		pStrategy = pLayout ? GetColorStrategy(cStringLine, iLogic, pStrategy, bChange) : nullptr;

		// タブ文字出現 or 文字種(全角／半角)の境界 or 色指定の境界
		if (nKind != nKindLast || bChange) {
			// iLogicの直前までを描画
			ASSERT_GE(iLogic, nBgnLogic);
			if (0 < iLogic - nBgnLogic) {
				ASSERT_GE(nBgnLogic, nLineStart);
				Print_DrawBlock(
					hdc,
					ptDraw,		// 描画座標。HDC内部単位。
					pLine + nLineStart,
					iLogic - nBgnLogic,
					nKindLast,
					pLayout,	// 色設定用Layout
					nColorIdx,
					nBgnLogic - nLineStart,
					nLayoutX,
					nDx,
					pDxArray
				);

				// 桁進め
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
				// ロジック進め
				nBgnLogic = iLogic;
			}
			if (bChange) {
				// 次のブロックの色
				nColorIdx = ToColorInfoArrIndex( pStrategy ? pStrategy->GetStrategyColor() : COLORIDX_TEXT );
			}
		}
	}

	// 残りを描画
	if (0 < nLineStart + nLineLen - nBgnLogic) {
		Print_DrawBlock(
			hdc,
			ptDraw,		// 描画座標。HDC内部単位。
			pLine + nLineStart,
			nLineStart + nLineLen - nBgnLogic,
			nKindLast,
			pLayout,	// 色設定用Layout
			nColorIdx,
			nBgnLogic - nLineStart,
			nLayoutX,
			nDx,
			pDxArray
		);
	}

	// フォントを元 (半角) に戻す
	::SelectObject(hdc, hFontHan);

	// 色を元に戻す	2012-03-07 ossan
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

/*! 印刷／Preview ブロック描画
	@param[in] 

	@date 2013.05.01 Uchi Print_DrawLineから切り出し
*/
void PrintPreview::Print_DrawBlock(
	HDC				hdc,
	POINT			ptDraw,		// 描画座標。HDC内部単位。
	const wchar_t*	pPhysicalLine,
	int				nBlockLen,	// iLogic - nBgnLogic
	int				nKind,
	const Layout*	pLayout,	// 色設定用Layout
	int				nColorIdx,
	int				nBgnPhysical,	// nBgnLogic - nLineStart
	int				nLayoutX,
	int				nDx,
	const int*		pDxArray
	)
{
	if (nKind == 2 && !pLayout) {
		// TABはカラーで無ければ印字不要
		return;
	}
	HFONT hFont = (nKind == 1) ? hFontZen : hFontHan;
	// 色設定
	if (pLayout) {
		if (nColorIdx != -1) {
			const ColorInfo& info = parentWnd.GetDocument().docType.GetDocumentAttribute().colorInfoArr[nColorIdx];
			if (nKind == 2 && !info.fontAttr.bUnderLine) {
				// TABは下線が無ければ印字不要
				return;
			}
			if (info.fontAttr.bBoldFont) {
				if (info.fontAttr.bUnderLine) {
					hFont = (nKind == 1 ? hFontZen_bu: hFontHan_bu);	// 太字、下線
				}else {
					hFont = (nKind == 1 ? hFontZen_b : hFontHan_b);		// 太字
				}
			}else {
				if (info.fontAttr.bUnderLine) {
					hFont = (nKind == 1 ? hFontZen_u : hFontHan_u);		// 下線
				}
			}
			//	else					hFont = (nKind == 1 ? hFontZen   : hFontHan);		// 標準
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

/*! 指定ロジック位置のColorStrategyを取得
	@param[in] 

	@date 2013.05.01 Uchi 新規作成
	@date 2014.12.30 Moca 正規表現の違う色が並んでいた場合に色替えできてなかったバグを修正
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


/*	印刷Previewフォント（半角）を設定する
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

	//	PrintSettingからコピー
	lfPreviewHan.lfHeight			= pPrintSetting->nPrintFontHeight;
	lfPreviewHan.lfWidth			= pPrintSetting->nPrintFontWidth;
	_tcscpy(lfPreviewHan.lfFaceName, pPrintSetting->szPrintFontFaceHan);

	SelectCharWidthCache(CharWidthFontMode::Print, CharWidthCacheMode::Local);
	InitCharWidthCache(lfPreviewHan, CharWidthFontMode::Print);
}

void PrintPreview::SetPreviewFontZen(const LOGFONT* lf)
{
	lfPreviewZen = *lf;
	//	PrintSettingからコピー
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
	印刷Previewに必要なコントロールを作成する
*/
void PrintPreview::CreatePrintPreviewControls(void)
{
	// 印刷Preview 操作バー
	hwndPrintPreviewBar = ::CreateDialogParam(
		SelectLang::getLangRsrcInstance(),					// handle to application instance
		MAKEINTRESOURCE(IDD_PRINTPREVIEWBAR),				// identifies dialog box template name
		parentWnd.GetHwnd(),							// handle to owner window
		PrintPreview::PrintPreviewBar_DlgProc,	// pointer to dialog box procedure
		(LPARAM)this
	);

	// 縦スクロールバーの作成
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

	// 横スクロールバーの作成
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

	// サイズボックスの作成
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

	// WM_SIZE 処理
	RECT rc1;
	::GetClientRect(parentWnd.GetHwnd(), &rc1);
	OnSize(SIZE_RESTORED, MAKELONG(rc1.right - rc1.left, rc1.bottom - rc1.top));
	return;
}


/*!
	印刷Previewに必要だったコントロールを破棄する
*/
void PrintPreview::DestroyPrintPreviewControls(void)
{
	// 印刷Preview 操作バー 削除
	if (hwndPrintPreviewBar) {
		::DestroyWindow(hwndPrintPreviewBar);
		hwndPrintPreviewBar = NULL;
	}

	// 印刷Preview 垂直スクロールバーウィンドウ 削除
	if (hwndVScrollBar) {
		::DestroyWindow(hwndVScrollBar);
		hwndVScrollBar = NULL;
	}
	// 印刷Preview 水平スクロールバーウィンドウ 削除
	if (hwndHScrollBar) {
		::DestroyWindow(hwndHScrollBar);
		hwndHScrollBar = NULL;
	}
	// 印刷Preview サイズボックスウィンドウ 削除
	if (hwndSizeBox) {
		::DestroyWindow(hwndSizeBox);
		hwndSizeBox = NULL;
	}
}

// ダイアログプロシージャ
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
		// 2007.02.11 Moca WM_INITもDispatchEvent_PPBを呼ぶように
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

// 印刷Preview 操作バーにフォーカスを当てる
void PrintPreview::SetFocusToPrintPreviewBar(void)
{
	if (hwndPrintPreviewBar) {
		::SetFocus(hwndPrintPreviewBar);
	}
}

// 印刷Preview 操作バー ダイアログのメッセージ処理
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
		// 2007.02.11 Moca DWLP_USER設定は不要
		//// Modified by KEITA for WIN64 2003.9.6
		//::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
		{
			if (IsWin2000_or_later()) {
				::EnableWindow( ::GetDlgItem(hwndDlg, IDC_CHECK_ANTIALIAS), TRUE );
			}
		}
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// 通知コード
		wID			= LOWORD(wParam);	// 項目ID、コントロールID またはアクセラレータID
		switch (wNotifyCode) {
		// ボタン／チェックボックスがクリックされた
		case BN_CLICKED:
			switch (wID) {
			case IDC_BUTTON_PRINTERSELECT:
				// From Here 2003.05.03 かろと
				{
					// PRINTDLGを初期化
					PRINTDLG pd = {0};
					pd.Flags = PD_PRINTSETUP | PD_NONETWORKBUTTON;
					pd.hwndOwner = parentWnd.GetHwnd();
					if (print.PrintDlg(&pd, &pPrintSettingOrg->mdmDevMode)) {
						// 用紙サイズと用紙方向を反映させる 2003.05.03 かろと
						pPrintSettingOrg->nPrintPaperSize = pPrintSettingOrg->mdmDevMode.dmPaperSize;
						pPrintSettingOrg->nPrintPaperOrientation = pPrintSettingOrg->mdmDevMode.dmOrientation;
						// 印刷Preview スクロールバー初期化
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
				// To Here 2003.05.03 かろと
				break;
			case IDC_BUTTON_PrintSetting:
				parentWnd.OnPrintPageSetting();
				break;
			case IDC_BUTTON_ZOOMUP:
				// Preview拡大縮小
				OnPreviewZoom(TRUE);
				break;
			case IDC_BUTTON_ZOOMDOWN:
				// Preview拡大縮小
				OnPreviewZoom(FALSE);
				break;
			case IDC_BUTTON_PREVPAGE:
				// 前ページ
				OnPreviewGoPreviousPage();
				break;
			case IDC_BUTTON_NEXTPAGE:
				// 次ページ
				OnPreviewGoNextPage();
				break;
			// From Here 2007.02.11 Moca ダイレクトジャンプおよびアンチエイリアス
			case IDC_BUTTON_DIRECTPAGE:
				OnPreviewGoDirectPage();
				break;
			case IDC_CHECK_ANTIALIAS:
				OnCheckAntialias();
				break;
			// To Here 2007.02.11 Moca
			case IDC_BUTTON_HELP:
				// 印刷Previewのヘルプ
				// Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした
				MyWinHelp(hwndDlg, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PRINT_PREVIEW));	// 2006.10.10 ryoji MyWinHelpに変更に変更
				break;
			case IDOK:
				// 印刷実行
				OnPrint();
				return TRUE;
			case IDCANCEL:
				// 印刷Previewモードのオン/オフ
				parentWnd.PrintPreviewModeONOFF();
				return TRUE;
			}
			break;	// BN_CLICKED
		}
		break;	// WM_COMMAND
	}
	return FALSE;
}


// 印刷用フォントを作成する
void PrintPreview::CreateFonts(HDC hdc)
{
	LOGFONT	lf;
	// 印刷用半角フォントを作成 -> hFontHan
	lfPreviewHan.lfHeight	= pPrintSetting->nPrintFontHeight;
	lfPreviewHan.lfWidth	= pPrintSetting->nPrintFontWidth;
	_tcscpy(lfPreviewHan.lfFaceName, pPrintSetting->szPrintFontFaceHan);
	hFontHan	= CreateFontIndirect(&lfPreviewHan);
	if (pPrintSetting->bColorPrint) {
		lf = lfPreviewHan;	lf.lfWeight = FW_BOLD;
		hFontHan_b	= CreateFontIndirect(&lf);		// 太字
		lf = lfPreviewHan;							lf.lfUnderline = true;
		hFontHan_u	= CreateFontIndirect(&lf);		// 下線
		lf = lfPreviewHan;	lf.lfWeight = FW_BOLD;	lf.lfUnderline = true;
		hFontHan_bu	= CreateFontIndirect(&lf);		// 太字、下線
	}
#ifdef _DEEBUG
	else {
		hFontHan_b  = m_hFontHan_u  = m_hFontHan_bu = NULL;
	}
#endif
	// 半角文字のアセント（文字高）を取得
	::SelectObject(hdc, hFontHan);
	TEXTMETRIC tm;
	::GetTextMetrics(hdc, &tm);
	nAscentHan = tm.tmAscent;

	// 印刷用全角フォントを作成 -> hFontZen
	if (auto_strcmp(pPrintSetting->szPrintFontFaceHan, pPrintSetting->szPrintFontFaceZen)) {
		lfPreviewZen.lfHeight	= pPrintSetting->nPrintFontHeight;
		lfPreviewZen.lfWidth	= pPrintSetting->nPrintFontWidth;
		_tcscpy(lfPreviewZen.lfFaceName, pPrintSetting->szPrintFontFaceZen);
		hFontZen	= CreateFontIndirect(&lfPreviewZen);
		if (pPrintSetting->bColorPrint) {
			lf = lfPreviewZen;	lf.lfWeight = FW_BOLD;
			hFontZen_b	= CreateFontIndirect(&lf);		// 太字
			lf = lfPreviewZen;							lf.lfUnderline = true;
			hFontZen_u	= CreateFontIndirect(&lf);		// 下線
			lf = lfPreviewZen;	lf.lfWeight = FW_BOLD;	lf.lfUnderline = true;
			hFontZen_bu	= CreateFontIndirect(&lf);		// 太字、下線
		}
#ifdef _DEEBUG
		else {
			hFontHan_b  = m_hFontHan_u  = m_hFontHan_bu = NULL;
		}
#endif
		// 全角文字のアセント（文字高）を取得
		::SelectObject(hdc, hFontZen);
		::GetTextMetrics(hdc, &tm);
		nAscentZen = tm.tmAscent;
	}else {
		// 半角全角同じフォント
		hFontZen		= hFontHan;
		hFontZen_b	= hFontHan_b;		// 太字
		hFontZen_u	= hFontHan_u;		// 下線
		hFontZen_bu	= hFontHan_bu;	// 太字、下線
		nAscentZen	= nAscentHan;		// 全角文字のアセント
	}
}

// 印刷用フォントを破棄する
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

