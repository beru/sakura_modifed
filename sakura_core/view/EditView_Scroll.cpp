#include "StdAfx.h"
#include "EditView.h"
#include "Ruler.h"
#include "env/DllSharedData.h"
#include "window/EditWnd.h"
#include "types/TypeSupport.h"
#include <limits.h>

/*! スクロールバー作成 */
BOOL EditView::CreateScrollBar()
{
	// スクロールバーの作成
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

	// スクロールバーの作成
	hwndHScrollBar = NULL;
	if (GetDllShareData().common.window.bScrollBarHorz) {	// 水平スクロールバーを使う
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

	// サイズボックス
	if (GetDllShareData().common.window.nFuncKeyWnd_Place == 0) {	// ファンクションキー表示位置／0:上 1:下
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



/*! スクロールバー破棄 */
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

/*! 垂直スクロールバーメッセージ処理

	@param nScrollCode [in]	スクロール種別 (Windowsから渡されるもの)
	@param nPos [in]		スクロール位置(THUMBTRACK用)
	@retval	実際にスクロールした行数
*/
int EditView::OnVScroll(int nScrollCode, int nPos)
{
	int nScrollVal = 0;

	// nPos 32bit対応
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

/*! 水平スクロールバーメッセージ処理

	@param nScrollCode [in]	スクロール種別 (Windowsから渡されるもの)
	@param nPos [in]		スクロール位置(THUMBTRACK用)
	@retval	実際にスクロールした桁数
*/
int EditView::OnHScroll(int nScrollCode, int nPos)
{
	const int nHScrollNum = 4;
	int nScrollVal = 0;

	// nPos 32bit対応
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

/** スクロールバーの状態を更新する

	タブバーのタブ切替時は SIF_DISABLENOSCROLL フラグでの有効化／無効化が正常に動作しない
	（不可視でサイズ変更していることによる影響か？）ので SIF_DISABLENOSCROLL で有効／無効
	の切替に失敗した場合には強制切替する
*/
void EditView::AdjustScrollBars()
{
	if (!GetDrawSwitch()) {
		return;
	}

	SCROLLINFO	si;
	if (hwndVScrollBar) {
		// 垂直スクロールバー
		const int nEofMargin = 2; // EOFとその下のマージン
		const size_t nAllLines = pEditDoc->layoutMgr.GetLineCount() + nEofMargin;
		int	nVScrollRate = 1;
#ifdef _WIN64
		// nAllLines / nVScrollRate < INT_MAX となる整数nVScrollRateを求める
		// 64bit版用スクロール率
		while (nAllLines / nVScrollRate > INT_MAX) {
			++nVScrollRate;
		}
#endif
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = (int)nAllLines / nVScrollRate - 1;	// 全行数
		si.nPage = GetTextArea().nViewRowNum / nVScrollRate;	// 表示域の行数
		si.nPos  = GetTextArea().GetViewTopLine() / nVScrollRate;	// 表示域の一番上の行(0開始)
		si.nTrackPos = 0;
		::SetScrollInfo(hwndVScrollBar, SB_CTL, &si, TRUE);
		this->nVScrollRate = nVScrollRate;				// 垂直スクロールバーの縮尺
		
		//	縦スクロールバーがDisableになったときは必ず全体が画面内に収まるように
		//	スクロールさせる
		bool bEnable = (GetTextArea().nViewRowNum < (int)nAllLines);
		if (bEnable != (::IsWindowEnabled(hwndVScrollBar) != 0)) {
			::EnableWindow(hwndVScrollBar, bEnable? TRUE: FALSE);	// SIF_DISABLENOSCROLL 誤動作時の強制切替
		}
		if (!bEnable) {
			ScrollAtV(0);
		}
	}
	if (hwndHScrollBar) {
		// 水平スクロールバー
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = (int)GetRightEdgeForScrollBar() - 1;		// スクロールバー制御用の右端座標を取得
		si.nPage = GetTextArea().nViewColNum;			// 表示域の桁数
		si.nPos  = GetTextArea().GetViewLeftCol();		// 表示域の一番左の桁(0開始)
		si.nTrackPos = 1;
		::SetScrollInfo(hwndHScrollBar, SB_CTL, &si, TRUE);

		bool bEnable = (GetTextArea().nViewColNum < (int)GetRightEdgeForScrollBar());
		if (bEnable != (::IsWindowEnabled(hwndHScrollBar) != 0)) {
			::EnableWindow(hwndHScrollBar, bEnable? TRUE: FALSE);	// SIF_DISABLENOSCROLL 誤動作時の強制切替
		}
		if (!bEnable) {
			ScrollAtH(0);
		}
	}
}

/*! 指定上端行位置へスクロール

	@param nPos [in] スクロール位置
	@retval 実際にスクロールした行数 (正:下方向/負:上方向)
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
		return 0;	//	スクロール無し。
	}
	// 垂直スクロール量（行数）の算出
	nScrollRowNum = GetTextArea().GetViewTopLine() - nPos;

	// スクロール
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

	// スクロールバーの状態を更新する
	AdjustScrollBars();

	// キャレットの表示・更新
	GetCaret().ShowEditCaret();

	MiniMapRedraw(false);

	return -nScrollRowNum;	// 方向が逆なので符号反転が必要
}


/*! 指定左端桁位置へスクロール

	@param nPos [in] スクロール位置
	@retval 実際にスクロールした桁数 (正:右方向/負:左方向)
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
			//	折り返し幅よりウィンドウ幅が大きいときにWM_HSCROLLが来ると
			//	nPosが負の値になることがあり，その場合にスクロールバーから編集領域が
			//	離れてしまう．
			if (nPos < 0) {
				nPos = 0;
			}
		}
	}
	auto& textArea = GetTextArea();
	if (textArea.GetViewLeftCol() == nPos) {
		return 0;
	}
	// 水平スクロール量（文字数）の算出
	const int nScrollColNum = textArea.GetViewLeftCol() - nPos;

	// スクロール
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
	// 先にAdjustScrollBarsを呼んでしまうと、二度目はここまでこないので、
	// GetRuler().DispRulerが呼ばれない。そのため、順序を入れ替えた。
	GetRuler().SetRedrawFlag(); // ルーラーを再描画する。
	HDC hdc = ::GetDC(GetHwnd());
	GetRuler().DispRuler(hdc);
	::ReleaseDC(GetHwnd(), hdc);

	// スクロールバーの状態を更新する
	AdjustScrollBars();

	// キャレットの表示・更新
	GetCaret().ShowEditCaret();

	return -nScrollColNum;	// 方向が逆なので符号反転が必要
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

	// 背景は画面に対して固定か
	bool bBackImgFixed = IsBkBitmap() &&
		(nScrollRowNum != 0 && !pTypeData->backImgScrollY ||
		 nScrollColNum != 0 && !pTypeData->backImgScrollX);
	if (bBackImgFixed) {
		Rect rcBody = area.GetAreaRect();
		rcBody.left = 0; // 行番号も移動
		rcBody.top = area.GetRulerHeight();
		InvalidateRect(&rcBody, FALSE);
	}else {
		int nScrollColPxWidth = nScrollColNum * GetTextMetrics().GetHankakuDx();
		ScrollWindowEx(
			nScrollColPxWidth,	// 水平スクロール量
			nScrollRowNum * GetTextMetrics().GetHankakuDy(),	// 垂直スクロール量
			&rcScroll,	// スクロール長方形の構造体のアドレス
			NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
		);
		if (hbmpCompatBMP) {
			// 互換BMPもスクロール処理のためにBitBltで移動させる
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
			// Scrollのときにルーラー余白更新
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
			// 行番号背景のために更新
			Rect rcLineNum;
			area.GenerateLineNumberRect(&rcLineNum);
			InvalidateRect(&rcLineNum, FALSE);
		}
	}
	// カーソルの縦線がテキストと行番号の隙間にあるとき、スクロール時に縦線領域を更新
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
				// OnSize:下だけ伸縮する
				bUpdateOne = true;
				nDrawTopTop = t_min(nViewBottom, (int)GetTextArea().GetBottomLine());
				nDrawTopBottom = t_max(nViewBottom, (int)GetTextArea().GetBottomLine());
			}else {
				nDrawTopTop = nViewTop;
				nDrawTopBottom = nViewBottom;
			}
		}else {
			if (nDiff < 0) {
				// 上に移動
				nDrawTopTop = GetTextArea().GetViewTopLine();
				nDrawTopBottom = nViewTop;
			}else {
				// 下に移動
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
				// 上に移動
				nDrawBottomTop = GetTextArea().GetBottomLine();
				nDrawBottomBottom = nViewBottom;
			}else {
				// 下に移動
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


/*!	垂直同期スクロール

	垂直同期スクロールがONならば，対応するウィンドウを指定行数同期スクロールする
	
	@param line [in] スクロール行数 (正:下方向/負:上方向/0:何もしない)
	
	@note 動作の詳細は設定や機能拡張により変更になる可能性がある

*/
void EditView::SyncScrollV(int line)
{
	if (GetDllShareData().common.window.bSplitterWndVScroll && line != 0 
		&& editWnd.IsEnablePane(nMyIndex^0x01) 
		&& 0 <= nMyIndex
	) {
		EditView& editView = editWnd.GetView(nMyIndex^0x01);
#if 0
		//	差分を保ったままスクロールする場合
		editView.ScrollByV(line);
#else
		editView.ScrollAtV(GetTextArea().GetViewTopLine());
#endif
	}
}

/*!	水平同期スクロール

	水平同期スクロールがONならば，対応するウィンドウを指定行数同期スクロールする．
	
	@param col [in] スクロール桁数 (正:右方向/負:左方向/0:何もしない)
	
	@note 動作の詳細は設定や機能拡張により変更になる可能性がある
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
		//	差分を保ったままスクロールする場合
		editView.ScrollByH(col);
#else
		editView.ScrollAtH(GetTextArea().GetViewLeftCol());
#endif
		GetRuler().SetRedrawFlag(); // スクロール時ルーラー全体を描きなおす。
		GetRuler().DispRuler(hdc);
		::ReleaseDC(GetHwnd(), hdc);
	}
}

/** 折り返し桁以後のぶら下げ余白計算 */
size_t EditView::GetWrapOverhang(void) const
{
	size_t nMargin = 1;	// 折り返し記号
	if (!pTypeData->bKinsokuHide) {	// ぶら下げを隠す時はスキップ
		if (pTypeData->bKinsokuRet) {
			nMargin += 1;	// 改行ぶら下げ
		}
		if (pTypeData->bKinsokuKuto) {
			nMargin += 2;	// 句読点ぶら下げ
		}
	}
	return nMargin;
}

/** 「右端で折り返す」用にビューの桁数から折り返し桁数を計算する
	@param nViewColNum	[in] ビューの桁数
	@retval 折り返し桁数
*/
int EditView::ViewColNumToWrapColNum(int nViewColNum) const
{
	// ぶら下げ余白を差し引く
	int nWidth = nViewColNum - (int)GetWrapOverhang();

	// MINLINEKETAS未満の時はMINLINEKETASで折り返しとする
	if (nWidth < MINLINEKETAS) {
		nWidth = MINLINEKETAS;		// 折り返し幅の最小桁数に設定
	}
	return nWidth;
}

/*!
	@brief  スクロールバー制御用に右端座標を取得する

	「折り返さない」
		フリーカーソル状態の時はテキストの幅よりも右側へカーソルが移動できる
		ので、それを考慮したスクロールバーの制御が必要。
		本関数は、下記の内で最も大きな値（右端の座標）を返す。
		　・テキストの右端
		　・キャレット位置
		　・選択範囲の右端
	
	「指定桁で折り返す」
	「右端で折り返す」
		上記の場合折り返し桁以後のぶら下げ余白計算

	@return     右端のレイアウト座標を返す

	@note   「折り返さない」選択時は、スクロール後にキャレットが見えなく
	        ならない様にするために右マージンとして半角3個分固定で加算する。
*/
size_t EditView::GetRightEdgeForScrollBar(void)
{
	// 折り返し桁以後のぶら下げ余白計算
	size_t nWidth = pEditDoc->layoutMgr.GetMaxLineKetas() + GetWrapOverhang();
	
	if (pEditDoc->nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
		int nRightEdge = (int)pEditDoc->layoutMgr.GetMaxTextWidth();	// テキストの最大幅
		// 選択範囲あり かつ 範囲の右端がテキストの幅より右側
		if (GetSelectionInfo().IsTextSelected()) {
			// 開始位置・終了位置のより右側にある方で比較
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

		// フリーカーソルモード かつ キャレット位置がテキストの幅より右側
		if (GetDllShareData().common.general.bIsFreeCursorMode && nRightEdge < GetCaret().GetCaretLayoutPos().x)
			nRightEdge = GetCaret().GetCaretLayoutPos().x;

		// 右マージン分（3桁）を考慮しつつnWidthを超えないようにする
		nWidth = (nRightEdge + 3 < (int)nWidth) ? (nRightEdge + 3) : nWidth;
	}

	return nWidth;
}

