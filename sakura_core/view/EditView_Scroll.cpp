/*!	@file
	@brief 文書ウィンドウの管理

	@author kobake
	@date	2008/04/14 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, jepro
	Copyright (C) 2001, asa-o, MIK, hor, Misaka, Stonee, YAZAKI
	Copyright (C) 2002, genta, hor, YAZAKI, Azumaiya, KK, novice, minfu, ai, aroka, MIK
	Copyright (C) 2003, genta, MIK, Moca
	Copyright (C) 2004, genta, Moca, novice, Kazika, isearch
	Copyright (C) 2005, genta, Moca, MIK, ryoji, maru
	Copyright (C) 2006, genta, aroka, fon, yukihane, ryoji
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, ryoji
	Copyright (C) 2009, nasukoji
	Copyright (C) 2010, Moca
	Copyright (C) 2012, ryoji, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "EditView.h"
#include "Ruler.h"
#include "env/DllSharedData.h"
#include "window/EditWnd.h"
#include "types/TypeSupport.h"
#include <limits.h>

/*! スクロールバー作成
	@date 2006.12.19 ryoji 新規作成（EditView::Createから分離）
*/
BOOL EditView::CreateScrollBar()
{
	// スクロールバーの作成
	m_hwndVScrollBar = ::CreateWindowEx(
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
	::SetScrollInfo(m_hwndVScrollBar, SB_CTL, &si, TRUE);
	::ShowScrollBar(m_hwndVScrollBar, SB_CTL, TRUE);

	// スクロールバーの作成
	m_hwndHScrollBar = NULL;
	if (GetDllShareData().common.window.bScrollBarHorz) {	// 水平スクロールバーを使う
		m_hwndHScrollBar = ::CreateWindowEx(
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
		::SetScrollInfo(m_hwndHScrollBar, SB_CTL, &si, TRUE);
		::ShowScrollBar(m_hwndHScrollBar, SB_CTL, TRUE);
	}

	// サイズボックス
	if (GetDllShareData().common.window.nFuncKeyWnd_Place == 0) {	// ファンクションキー表示位置／0:上 1:下
		m_hwndSizeBox = ::CreateWindowEx(
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
		m_hwndSizeBox = ::CreateWindowEx(
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



/*! スクロールバー破棄
	@date 2006.12.19 ryoji 新規作成
*/
void EditView::DestroyScrollBar()
{
	if (m_hwndVScrollBar) {
		::DestroyWindow(m_hwndVScrollBar);
		m_hwndVScrollBar = NULL;
	}

	if (m_hwndHScrollBar) {
		::DestroyWindow(m_hwndHScrollBar);
		m_hwndHScrollBar = NULL;
	}

	if (m_hwndSizeBox) {
		::DestroyWindow(m_hwndSizeBox);
		m_hwndSizeBox = NULL;
	}
}

/*! 垂直スクロールバーメッセージ処理

	@param nScrollCode [in]	スクロール種別 (Windowsから渡されるもの)
	@param nPos [in]		スクロール位置(THUMBTRACK用)
	@retval	実際にスクロールした行数

	@date 2004.09.11 genta スクロール行数を返すように．
		未使用のhwndScrollBar引数削除．
*/
LayoutInt EditView::OnVScroll(int nScrollCode, int nPos)
{
	LayoutInt nScrollVal = LayoutInt(0);

	// nPos 32bit対応
	if (nScrollCode == SB_THUMBTRACK || nScrollCode == SB_THUMBPOSITION) {
		if (m_hwndVScrollBar) {
			HWND hWndScroll = m_hwndVScrollBar;
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
		nScrollVal = ScrollAtV(GetTextArea().GetViewTopLine() - GetTextArea().m_nViewRowNum);
		break;
	case SB_THUMBPOSITION:
		nScrollVal = ScrollAtV(LayoutInt(nPos));
		break;
	case SB_THUMBTRACK:
		nScrollVal = ScrollAtV(LayoutInt(nPos));
		break;
	case SB_TOP:
		nScrollVal = ScrollAtV(LayoutInt(0));
		break;
	case SB_BOTTOM:
		nScrollVal = ScrollAtV((m_pEditDoc->m_layoutMgr.GetLineCount()) - GetTextArea().m_nViewRowNum);
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

	@date 2004.09.11 genta スクロール桁数を返すように．
		未使用のhwndScrollBar引数削除．
*/
LayoutInt EditView::OnHScroll(int nScrollCode, int nPos)
{
	const LayoutInt nHScrollNum = LayoutInt(4);
	LayoutInt nScrollVal = LayoutInt(0);

	// nPos 32bit対応
	if (nScrollCode == SB_THUMBTRACK || nScrollCode == SB_THUMBPOSITION) {
		if (m_hwndHScrollBar) {
			HWND hWndScroll = m_hwndHScrollBar;
			SCROLLINFO info;
			info.cbSize = sizeof(SCROLLINFO);
			info.fMask = SIF_TRACKPOS;
			::GetScrollInfo(hWndScroll, SB_CTL, &info);
			nPos = info.nTrackPos;
		}
	}

	GetRuler().SetRedrawFlag(); // YAZAKI
	switch (nScrollCode) {
	case SB_LINELEFT:
		nScrollVal = ScrollAtH(GetTextArea().GetViewLeftCol() - nHScrollNum);
		break;
	case SB_LINERIGHT:
		nScrollVal = ScrollAtH(GetTextArea().GetViewLeftCol() + nHScrollNum);
		break;
	case SB_PAGELEFT:
		nScrollVal = ScrollAtH(GetTextArea().GetViewLeftCol() - GetTextArea().m_nViewColNum);
		break;
	case SB_PAGERIGHT:
		nScrollVal = ScrollAtH(GetTextArea().GetRightCol());
		break;
	case SB_THUMBPOSITION:
		nScrollVal = ScrollAtH(LayoutInt(nPos));
//		MYTRACE(_T("nPos=%d\n"), nPos);
		break;
	case SB_THUMBTRACK:
		nScrollVal = ScrollAtH(LayoutInt(nPos));
//		MYTRACE(_T("nPos=%d\n"), nPos);
		break;
	case SB_LEFT:
		nScrollVal = ScrollAtH(LayoutInt(0));
		break;
	case SB_RIGHT:
		//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
		nScrollVal = ScrollAtH(m_pEditDoc->m_layoutMgr.GetMaxLineKetas() - GetTextArea().m_nViewColNum);
		break;
	}
	return nScrollVal;
}

/** スクロールバーの状態を更新する

	タブバーのタブ切替時は SIF_DISABLENOSCROLL フラグでの有効化／無効化が正常に動作しない
	（不可視でサイズ変更していることによる影響か？）ので SIF_DISABLENOSCROLL で有効／無効
	の切替に失敗した場合には強制切替する

	@date 2008.05.24 ryoji 有効／無効の強制切替を追加
	@date 2008.06.08 ryoji 水平スクロール範囲にぶら下げ余白を追加
	@date 2009.08.28 nasukoji	「折り返さない」選択時のスクロールバー調整
*/
void EditView::AdjustScrollBars()
{
	if (!GetDrawSwitch()) {
		return;
	}

	SCROLLINFO	si;

	if (m_hwndVScrollBar) {
		// 垂直スクロールバー
		const LayoutInt	nEofMargin = LayoutInt(2); // EOFとその下のマージン
		const LayoutInt	nAllLines = m_pEditDoc->m_layoutMgr.GetLineCount() + nEofMargin;
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
		si.nMax  = (Int)nAllLines / nVScrollRate - 1;	// 全行数
		si.nPage = (Int)GetTextArea().m_nViewRowNum / nVScrollRate;	// 表示域の行数
		si.nPos  = (Int)GetTextArea().GetViewTopLine() / nVScrollRate;	// 表示域の一番上の行(0開始)
		si.nTrackPos = 0;
		::SetScrollInfo(m_hwndVScrollBar, SB_CTL, &si, TRUE);
		m_nVScrollRate = nVScrollRate;				// 垂直スクロールバーの縮尺
		
		//	Nov. 16, 2002 genta
		//	縦スクロールバーがDisableになったときは必ず全体が画面内に収まるように
		//	スクロールさせる
		//	2005.11.01 aroka 判定条件誤り修正 (バーが消えてもスクロールしない)
		bool bEnable = (GetTextArea().m_nViewRowNum < nAllLines);
		if (bEnable != (::IsWindowEnabled(m_hwndVScrollBar) != 0)) {
			::EnableWindow(m_hwndVScrollBar, bEnable? TRUE: FALSE);	// SIF_DISABLENOSCROLL 誤動作時の強制切替
		}
		if (!bEnable) {
			ScrollAtV(LayoutInt(0));
		}
	}
	if (m_hwndHScrollBar) {
		// 水平スクロールバー
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = (Int)GetRightEdgeForScrollBar() - 1;		// 2009.08.28 nasukoji	スクロールバー制御用の右端座標を取得
		si.nPage = (Int)GetTextArea().m_nViewColNum;		// 表示域の桁数
		si.nPos  = (Int)GetTextArea().GetViewLeftCol();		// 表示域の一番左の桁(0開始)
		si.nTrackPos = 1;
		::SetScrollInfo(m_hwndHScrollBar, SB_CTL, &si, TRUE);

		//	2006.1.28 aroka 判定条件誤り修正 (バーが消えてもスクロールしない)
		bool bEnable = (GetTextArea().m_nViewColNum < GetRightEdgeForScrollBar());
		if (bEnable != (::IsWindowEnabled(m_hwndHScrollBar) != 0)) {
			::EnableWindow(m_hwndHScrollBar, bEnable? TRUE: FALSE);	// SIF_DISABLENOSCROLL 誤動作時の強制切替
		}
		if (!bEnable) {
			ScrollAtH(LayoutInt(0));
		}
	}
}

/*! 指定上端行位置へスクロール

	@param nPos [in] スクロール位置
	@retval 実際にスクロールした行数 (正:下方向/負:上方向)

	@date 2004.09.11 genta 行数を戻り値として返すように．(同期スクロール用)
*/
LayoutInt EditView::ScrollAtV(LayoutInt nPos)
{
	LayoutInt	nScrollRowNum;
	if (nPos < 0) {
		nPos = LayoutInt(0);
	}else if ((m_pEditDoc->m_layoutMgr.GetLineCount() + 2) - GetTextArea().m_nViewRowNum < nPos) {
		nPos = (m_pEditDoc->m_layoutMgr.GetLineCount() + LayoutInt(2)) - GetTextArea().m_nViewRowNum;
		if (nPos < 0) {
			nPos = LayoutInt(0);
		}
	}
	if (GetTextArea().GetViewTopLine() == nPos) {
		return LayoutInt(0);	//	スクロール無し。
	}
	// 垂直スクロール量（行数）の算出
	nScrollRowNum = GetTextArea().GetViewTopLine() - nPos;

	// スクロール
	if (t_abs(nScrollRowNum) >= GetTextArea().m_nViewRowNum) {
		GetTextArea().SetViewTopLine(LayoutInt(nPos));
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
				(Int)nScrollRowNum * GetTextMetrics().GetHankakuDy();
			GetTextArea().SetViewTopLine(LayoutInt(nPos));
			rcClip.left = 0;
			rcClip.right = GetTextArea().GetAreaRight();
			rcClip.top = GetTextArea().GetAreaTop();
			rcClip.bottom =
				GetTextArea().GetAreaTop() + (Int)nScrollRowNum * GetTextMetrics().GetHankakuDy();
		}else if (nScrollRowNum < 0) {
			rcScrol.top =
				GetTextArea().GetAreaTop() - (Int)nScrollRowNum * GetTextMetrics().GetHankakuDy();
			GetTextArea().SetViewTopLine(LayoutInt(nPos));
			rcClip.left = 0;
			rcClip.right = GetTextArea().GetAreaRight();
			rcClip.top =
				GetTextArea().GetAreaBottom() +
				(Int)nScrollRowNum * GetTextMetrics().GetHankakuDy();
			rcClip.bottom = GetTextArea().GetAreaBottom();
		}
		if (GetDrawSwitch()) {
			RECT rcClip2 = {0, 0, 0, 0};
			ScrollDraw(nScrollRowNum, LayoutInt(0), rcScrol, rcClip, rcClip2);
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

	@date 2004.09.11 genta 桁数を戻り値として返すように．(同期スクロール用)
	@date 2008.06.08 ryoji 水平スクロール範囲にぶら下げ余白を追加
	@date 2009.08.28 nasukoji	「折り返さない」選択時右に行き過ぎないようにする
*/
LayoutInt EditView::ScrollAtH(LayoutInt nPos)
{
	if (nPos < 0) {
		nPos = LayoutInt(0);
	}
	//	Aug. 18, 2003 ryoji 変数のミスを修正
	//	ウィンドウの幅をきわめて狭くしたときに編集領域が行番号から離れてしまうことがあった．
	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	else {
		LayoutInt nPos2 = GetRightEdgeForScrollBar() + GetWrapOverhang() - GetTextArea().m_nViewColNum;
		if (nPos2 < nPos) {
			nPos = nPos2;
			//	May 29, 2004 genta 折り返し幅よりウィンドウ幅が大きいときにWM_HSCROLLが来ると
			//	nPosが負の値になることがあり，その場合にスクロールバーから編集領域が
			//	離れてしまう．
			if (nPos < 0) {
				nPos = LayoutInt(0);
			}
		}
	}
	auto& textArea = GetTextArea();
	if (textArea.GetViewLeftCol() == nPos) {
		return LayoutInt(0);
	}
	// 水平スクロール量（文字数）の算出
	const LayoutInt	nScrollColNum = textArea.GetViewLeftCol() - nPos;

	// スクロール
	if (t_abs(nScrollColNum) >= textArea.m_nViewColNum /*|| abs(nScrollRowNum) >= textArea.m_nViewRowNum*/) {
		textArea.SetViewLeftCol(nPos);
		::InvalidateRect(GetHwnd(), NULL, TRUE);
	}else {
		RECT rcClip2;
		RECT rcScrol;
		rcScrol.left = 0;
		rcScrol.right = textArea.GetAreaRight();
		rcScrol.top = textArea.GetAreaTop();
		rcScrol.bottom = textArea.GetAreaBottom();
		int nScrollColPxWidth = (Int)nScrollColNum * GetTextMetrics().GetHankakuDx();
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
			ScrollDraw(LayoutInt(0), nScrollColNum, rcScrol, rcClip, rcClip2);
			::UpdateWindow(GetHwnd());
		}
	}
	//	2006.1.28 aroka 判定条件誤り修正 (バーが消えてもスクロールしない)
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
	LayoutInt nScrollRowNum,
	LayoutInt nScrollColNum,
	const RECT& rcScroll,
	const RECT& rcClip,
	const RECT& rcClip2
	)
{
	const TextArea& area = GetTextArea();

	// 背景は画面に対して固定か
	bool bBackImgFixed = IsBkBitmap() &&
		(nScrollRowNum != 0 && !m_pTypeData->backImgScrollY ||
		 nScrollColNum != 0 && !m_pTypeData->backImgScrollX);
	if (bBackImgFixed) {
		Rect rcBody = area.GetAreaRect();
		rcBody.left = 0; // 行番号も移動
		rcBody.top = area.GetRulerHeight();
		InvalidateRect(&rcBody, FALSE);
	}else {
		int nScrollColPxWidth = (Int)nScrollColNum * GetTextMetrics().GetHankakuDx();
		ScrollWindowEx(
			nScrollColPxWidth,	// 水平スクロール量
			(Int)nScrollRowNum * GetTextMetrics().GetHankakuDy(),	// 垂直スクロール量
			&rcScroll,	// スクロール長方形の構造体のアドレス
			NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
		);
		// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
		if (m_hbmpCompatBMP) {
			// 互換BMPもスクロール処理のためにBitBltで移動させる
			::BitBlt(
				m_hdcCompatDC,
				rcScroll.left + nScrollColPxWidth,
				rcScroll.top  + (Int)nScrollRowNum * GetTextMetrics().GetHankakuDy(),
				rcScroll.right - rcScroll.left, rcScroll.bottom - rcScroll.top,
				m_hdcCompatDC, rcScroll.left, rcScroll.top, SRCCOPY
			);
		}

		if (1
			&& 0 < area.GetTopYohaku()
			&& IsBkBitmap()
			&& (nScrollRowNum != 0 && m_pTypeData->backImgScrollY || nScrollColNum != 0 && m_pTypeData->backImgScrollX)
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
			HDC hdcBgImg = m_hdcCompatDC ? m_hdcCompatDC : CreateCompatibleDC(hdcSelf);
			HBITMAP hOldBmp = (HBITMAP)::SelectObject(hdcBgImg, m_pEditDoc->m_hBackImg);
			DrawBackImage(hdcSelf, rcTopYohaku, hdcBgImg);
			SelectObject(hdcBgImg, hOldBmp);
			ReleaseDC(hdcSelf);
			if (!m_hdcCompatDC) {
				DeleteObject(hdcBgImg);
			}
		}
		if (IsBkBitmap()
			&& nScrollColNum != 0
			&& m_pTypeData->backImgScrollX
		) {
			// 行番号背景のために更新
			Rect rcLineNum;
			area.GenerateLineNumberRect(&rcLineNum);
			InvalidateRect(&rcLineNum, FALSE);
		}
	}
	// カーソルの縦線がテキストと行番号の隙間にあるとき、スクロール時に縦線領域を更新
	if (nScrollColNum != 0 && m_nOldCursorLineX == GetTextArea().GetAreaLeft() - 1) {
		RECT rcClip3;
		rcClip3.left   = m_nOldCursorLineX - (m_nOldCursorVLineWidth - 1);
		rcClip3.right  = m_nOldCursorLineX + 1;
		rcClip3.top    = GetTextArea().GetAreaTop();
		rcClip3.bottom = GetTextArea().GetAreaBottom();
		InvalidateRect(&rcClip3, FALSE);
	}
	// To Here 2007.09.09 Moca

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
	if (this == &m_editWnd.GetActiveView() && m_editWnd.GetMiniMap().GetHwnd()) {
		EditView& miniMap = m_editWnd.GetMiniMap();
		LayoutYInt nViewTop = miniMap.m_nPageViewTop;
		LayoutYInt nViewBottom = miniMap.m_nPageViewBottom;
		LayoutYInt nDiff = nViewTop - GetTextArea().GetViewTopLine();
		LayoutYInt nDrawTopTop;
		LayoutYInt nDrawTopBottom;
		bool bUpdate = (t_abs(nDiff) > nViewBottom - nViewTop) || bUpdateAll;
		bool bUpdateOne = false;
		if (bUpdate) {
			if (nViewTop == GetTextArea().GetViewTopLine()) {
				// OnSize:下だけ伸縮する
				bUpdateOne = true;
				nDrawTopTop = t_min(nViewBottom, GetTextArea().GetBottomLine());
				nDrawTopBottom = t_max(nViewBottom, GetTextArea().GetBottomLine());
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
		LayoutYInt nDrawBottomTop;
		LayoutYInt nDrawBottomBottom;
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
	
	@author asa-o
	@date 2001.06.20 asa-o 新規作成
	@date 2004.09.11 genta 関数化

	@note 動作の詳細は設定や機能拡張により変更になる可能性がある

*/
void EditView::SyncScrollV(LayoutInt line)
{
	if (GetDllShareData().common.window.bSplitterWndVScroll && line != 0 
		&& m_editWnd.IsEnablePane(m_nMyIndex^0x01) 
		&& 0 <= m_nMyIndex
	) {
		EditView&	editView = m_editWnd.GetView(m_nMyIndex^0x01);
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
	
	@author asa-o
	@date 2001.06.20 asa-o 新規作成
	@date 2004.09.11 genta 関数化

	@note 動作の詳細は設定や機能拡張により変更になる可能性がある
*/
void EditView::SyncScrollH(LayoutInt col)
{
	if (GetDllShareData().common.window.bSplitterWndHScroll && col != 0
		&& m_editWnd.IsEnablePane(m_nMyIndex^0x02)
		&& 0 <= m_nMyIndex
	) {
		EditView& editView = m_editWnd.GetView(m_nMyIndex^0x02);
		HDC hdc = ::GetDC(editView.GetHwnd());
		
#if 0
		//	差分を保ったままスクロールする場合
		editView.ScrollByH(col);
#else
		editView.ScrollAtH(GetTextArea().GetViewLeftCol());
#endif
		GetRuler().SetRedrawFlag(); // 2002.02.25 Add By KK スクロール時ルーラー全体を描きなおす。
		GetRuler().DispRuler(hdc);
		::ReleaseDC(GetHwnd(), hdc);
	}
}

/** 折り返し桁以後のぶら下げ余白計算
	@date 2008.06.08 ryoji 新規作成
*/
LayoutInt EditView::GetWrapOverhang(void) const
{
	int nMargin = 1;	// 折り返し記号
	if (!m_pTypeData->bKinsokuHide) {	// ぶら下げを隠す時はスキップ	2012/11/30 Uchi
		if (m_pTypeData->bKinsokuRet)
			nMargin += 1;	// 改行ぶら下げ
		if (m_pTypeData->bKinsokuKuto)
			nMargin += 2;	// 句読点ぶら下げ
	}
	return LayoutInt(nMargin);
}

/** 「右端で折り返す」用にビューの桁数から折り返し桁数を計算する
	@param nViewColNum	[in] ビューの桁数
	@retval 折り返し桁数
	@date 2008.06.08 ryoji 新規作成
*/
LayoutInt EditView::ViewColNumToWrapColNum(LayoutInt nViewColNum) const
{
	// ぶら下げ余白を差し引く
	int nWidth = (Int)(nViewColNum - GetWrapOverhang());

	// MINLINEKETAS未満の時はMINLINEKETASで折り返しとする
	if (nWidth < MINLINEKETAS)
		nWidth = MINLINEKETAS;		// 折り返し幅の最小桁数に設定

	return LayoutInt(nWidth);
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

	@date 2009.08.28 nasukoji	新規作成
*/
LayoutInt EditView::GetRightEdgeForScrollBar(void)
{
	// 折り返し桁以後のぶら下げ余白計算
	LayoutInt nWidth = m_pEditDoc->m_layoutMgr.GetMaxLineKetas() + GetWrapOverhang();
	
	if (m_pEditDoc->m_nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
		LayoutInt nRightEdge = m_pEditDoc->m_layoutMgr.GetMaxTextWidth();	// テキストの最大幅

		// 選択範囲あり かつ 範囲の右端がテキストの幅より右側
		if (GetSelectionInfo().IsTextSelected()) {
			// 開始位置・終了位置のより右側にある方で比較
			auto& select = GetSelectionInfo().m_select;
			if (select.GetFrom().GetX2() < select.GetTo().GetX2()) {
				if (nRightEdge < select.GetTo().GetX2())
					nRightEdge = select.GetTo().GetX2();
			}else {
				if (nRightEdge < select.GetFrom().GetX2())
					nRightEdge = select.GetFrom().GetX2();
			}
		}

		// フリーカーソルモード かつ キャレット位置がテキストの幅より右側
		if (GetDllShareData().common.general.bIsFreeCursorMode && nRightEdge < GetCaret().GetCaretLayoutPos().GetX2())
			nRightEdge = GetCaret().GetCaretLayoutPos().GetX2();

		// 右マージン分（3桁）を考慮しつつnWidthを超えないようにする
		nWidth = (nRightEdge + 3 < nWidth) ? (nRightEdge + LayoutInt(3)) : nWidth;
	}

	return nWidth;
}

