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
	m_bSelectingLock   = false;	// 選択状態のロック
	m_bBeginSelect     = false;		// 範囲選択中
	m_bBeginBoxSelect  = false;	// 矩形範囲選択中
	m_bBeginLineSelect = false;	// 行単位選択中
	m_bBeginWordSelect = false;	// 単語単位選択中

	m_selectBgn.Clear(-1); // 範囲選択(原点)
	m_select.Clear(-1); // 範囲選択
	m_selectOld.Clear(0);  // 範囲選択(Old)
	m_bSelectAreaChanging = false;	// 選択範囲変更中
	m_nLastSelectedByteLen = 0;	// 前回選択時の選択バイト数
}

void ViewSelect::CopySelectStatus(ViewSelect* pSelect) const
{
	pSelect->m_bSelectingLock		= m_bSelectingLock;		// 選択状態のロック
	pSelect->m_bBeginSelect			= m_bBeginSelect;		// 範囲選択中
	pSelect->m_bBeginBoxSelect		= m_bBeginBoxSelect;	// 矩形範囲選択中

	pSelect->m_selectBgn			= m_selectBgn;			// 範囲選択(原点)
	pSelect->m_select				= m_select;				// 範囲選択
	pSelect->m_selectOld			= m_selectOld;			// 範囲選択

	pSelect->m_ptMouseRollPosOld	= m_ptMouseRollPosOld;	// マウス範囲選択前回位置(XY座標)
}

// 現在のカーソル位置から選択を開始する
void ViewSelect::BeginSelectArea(const LayoutPoint* po)
{
	const EditView* pView = GetEditView();
	LayoutPoint temp;
	if (!po) {
		temp = pView->GetCaret().GetCaretLayoutPos();
		po = &temp;
	}
	m_selectBgn.Set(*po); // 範囲選択(原点)
	m_select.   Set(*po); // 範囲選択
}


// 現在の選択範囲を非選択状態に戻す
void ViewSelect::DisableSelectArea(bool bDraw, bool bDrawBracketCursorLine)
{
	const EditView* pView = GetEditView();
	EditView* pView2 = GetEditView();

	m_selectOld = m_select;		// 範囲選択(Old)
	m_select.Clear(-1);
	m_bSelectingLock	 = false;	// 選択状態のロック

	if (bDraw) {
		DrawSelectArea(bDrawBracketCursorLine);
	}
	m_bDrawSelectArea = false;	// 02/12/13 ai // 2011.12.24 bDraw括弧内から移動

	m_selectOld.Clear(0);			// 範囲選択(Old)
	m_bBeginBoxSelect = false;		// 矩形範囲選択中
	m_bBeginLineSelect = false;		// 行単位選択中
	m_bBeginWordSelect = false;		// 単語単位選択中
	m_nLastSelectedByteLen = 0;		// 前回選択時の選択バイト数

	// 2002.02.16 hor 直前のカーソル位置をリセット
	pView2->GetCaret().m_nCaretPosX_Prev = pView->GetCaret().GetCaretLayoutPos().GetX();

}


// 現在のカーソル位置によって選択範囲を変更
void ViewSelect::ChangeSelectAreaByCurrentCursor(const LayoutPoint& ptCaretPos)
{
	m_selectOld = m_select; // 範囲選択(Old)

	//	2002/04/08 YAZAKI コードの重複を排除
	ChangeSelectAreaByCurrentCursorTEST(
		ptCaretPos,
		&m_select
	);

	// 選択領域の描画
	m_bSelectAreaChanging = true;
	DrawSelectArea(true);
	m_bSelectAreaChanging = false;
}


// 現在のカーソル位置によって選択範囲を変更(テストのみ)
void ViewSelect::ChangeSelectAreaByCurrentCursorTEST(
	const LayoutPoint& ptCaretPos,
	LayoutRange* pSelect
)
{
	if (m_selectBgn.GetFrom() == m_selectBgn.GetTo()) {
		if (ptCaretPos == m_selectBgn.GetFrom()) {
			// 選択解除
			pSelect->Clear(-1);
			m_nLastSelectedByteLen = 0;		// 前回選択時の選択バイト数
		}else if (PointCompare(ptCaretPos, m_selectBgn.GetFrom()) < 0) { // キャレット位置がm_sSelectBgnのfromより小さかったら
			 pSelect->SetFrom(ptCaretPos);
			 pSelect->SetTo(m_selectBgn.GetFrom());
		}else {
			pSelect->SetFrom(m_selectBgn.GetFrom());
			pSelect->SetTo(ptCaretPos);
		}
	}else {
		// 常時選択範囲の範囲内
		// キャレット位置が m_sSelectBgn の from以上で、toより小さい場合
		if (PointCompare(ptCaretPos, m_selectBgn.GetFrom()) >= 0 && PointCompare(ptCaretPos, m_selectBgn.GetTo()) < 0) {
			pSelect->SetFrom(m_selectBgn.GetFrom());
			if (ptCaretPos == m_selectBgn.GetFrom()) {
				pSelect->SetTo(m_selectBgn.GetTo());
			}else {
				pSelect->SetTo(ptCaretPos);
			}
		// キャレット位置がm_sSelectBgnのfromより小さかったら
		}else if (PointCompare(ptCaretPos, m_selectBgn.GetFrom()) < 0) {
			// 常時選択範囲の前方向
			pSelect->SetFrom(ptCaretPos);
			pSelect->SetTo(m_selectBgn.GetTo());
		}else {
			// 常時選択範囲の後ろ方向
			pSelect->SetFrom(m_selectBgn.GetFrom());
			pSelect->SetTo(ptCaretPos);
		}
	}
}


/*! 選択領域の描画

	@date 2006.10.01 Moca 重複コード削除．矩形作画改善．
	@date 2007.09.09 Moca 互換BMPによる画面バッファ
		画面バッファが有効時、画面と互換BMPの両方の反転処理を行う。
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
			// 選択色表示の時は、WM_PAINT経由で作画
			const int nCharWidth = pView->GetTextMetrics().GetHankakuDx();
			const TextArea& area =  pView->GetTextArea();
			LayoutRect rcOld; // LayoutRect
			TwoPointToRect(&rcOld, m_selectOld.GetFrom(), m_selectOld.GetTo());
			LayoutRect rcNew; // LayoutRect
			TwoPointToRect(&rcNew, m_select.GetFrom(), m_select.GetTo());
			LayoutRect rc; // LayoutRect ただしtop,bottomだけ使う
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
					// GetToの行が対象
					rc.top = rc.bottom = m_select.GetTo().GetY2();
					drawLeft  = t_min(m_select.GetTo().x, m_selectOld.GetTo().x);
					drawRight = t_max(m_select.GetTo().x, m_selectOld.GetTo().x) + 1;
				}else if (m_select.GetTo() == m_selectOld.GetTo() && m_select.GetFrom().x != m_selectOld.GetFrom().x) {
					// GetFromの行が対象
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
				// 背景表示のクリッピングが甘いので左右を指定しない
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
				// DrawSelectAreaLineでの下線OFFの代わり
				view.GetCaret().m_underLine.CaretUnderLineOFF(true, false);
				view.GetCaret().m_underLine.Lock();
				view.OnPaint(hdc, &ps, false);
				view.GetCaret().m_underLine.UnLock();
				view.ReleaseDC(hdc);
			}
			// 2010.10.10 0幅選択(解除)状態での、カーソル位置ライン復帰(リージョン外)
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
		// 2011.12.02 選択解除状態での、カーソル位置ライン復帰
		if (bDrawBracketCursorLine) {
			pView->GetCaret().m_underLine.CaretUnderLineON(true, false);
		}
		pView->ReleaseDC(hdc);
	}

	// 2011.12.02 選択解除状態になると対括弧強調ができなくなるバグ対策
	if (!IsTextSelecting()) {
		// ただし選択ロック中はここでは強調表示されない
		m_bDrawSelectArea = false;
		if (bDrawBracketCursorLine) {
			pView->SetBracketPairPos(true);
			pView->DrawBracketPair(true);
		}
	}

	//	Jul. 9, 2005 genta 選択領域の情報を表示
	PrintSelectionInfoMsg();
}

/*!
	反転用再作画処理本体
*/
void ViewSelect::DrawSelectArea2(HDC hdc) const
{
	EditView const * const pView = GetEditView();

	// 2006.10.01 Moca 重複コード統合
	HBRUSH	hBrush = ::CreateSolidBrush(SELECTEDAREA_RGB);
	HBRUSH	hBrushOld = (HBRUSH)::SelectObject(hdc, hBrush);
	int		nROP_Old = ::SetROP2(hdc, SELECTEDAREA_ROP2);
	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
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
	if (IsBoxSelecting()) {		// 矩形範囲選択中
		// 2001.12.21 hor 矩形エリアにEOFがある場合、RGN_XORで結合すると
		// EOF以降のエリアも反転してしまうので、この場合はRedrawを使う
		// 2002.02.16 hor ちらつきを抑止するためEOF以降のエリアが反転したらもう一度反転して元に戻すことにする
		//if ((GetTextArea().GetViewTopLine()+m_nViewRowNum+1 >= m_pEditDoc->m_layoutMgr.GetLineCount()) &&
		//   (m_select.GetTo().y+1 >= m_pEditDoc->m_layoutMgr.GetLineCount() ||
		//	m_selectOld.GetTo().y+1 >= m_pEditDoc->m_layoutMgr.GetLineCount())) {
		//	Redraw();
		//	return;
		//}

		const int nCharWidth = pView->GetTextMetrics().GetHankakuDx();
		const int nCharHeight = pView->GetTextMetrics().GetHankakuDy();

		// 2点を対角とする矩形を求める
		LayoutRect  rcOld;
		TwoPointToRect(
			&rcOld,
			m_selectOld.GetFrom(),	// 範囲選択開始
			m_selectOld.GetTo()	// 範囲選択終了
		);
		rcOld.left   = t_max(rcOld.left  , textArea.GetViewLeftCol() );
		rcOld.right  = t_max(rcOld.right , textArea.GetViewLeftCol() );
		rcOld.right  = t_min(rcOld.right , textArea.GetRightCol() + 1);
		rcOld.top    = t_max(rcOld.top   , textArea.GetViewTopLine() );
		rcOld.bottom = t_max(rcOld.bottom, textArea.GetViewTopLine() - 1);	// 2010.11.02 ryoji 追加（画面上端よりも上にある矩形選択を解除するとルーラーが反転表示になる問題の修正）
		rcOld.bottom = t_min(rcOld.bottom, textArea.GetBottomLine()  );

		RECT rcOld2;
		rcOld2.left		= (textArea.GetAreaLeft() - (Int)textArea.GetViewLeftCol() * nCharWidth) + (Int)rcOld.left  * nCharWidth;
		rcOld2.right	= (textArea.GetAreaLeft() - (Int)textArea.GetViewLeftCol() * nCharWidth) + (Int)rcOld.right * nCharWidth;
		rcOld2.top		= textArea.GenerateYPx(rcOld.top);
		rcOld2.bottom	= textArea.GenerateYPx(rcOld.bottom + 1);
		HRGN hrgnOld = ::CreateRectRgnIndirect(&rcOld2);

		// 2点を対角とする矩形を求める
		LayoutRect  rcNew;
		TwoPointToRect(
			&rcNew,
			m_select.GetFrom(),	// 範囲選択開始
			m_select.GetTo()		// 範囲選択終了
		);
		rcNew.left   = t_max(rcNew.left  , textArea.GetViewLeftCol());
		rcNew.right  = t_max(rcNew.right , textArea.GetViewLeftCol());
		rcNew.right  = t_min(rcNew.right , textArea.GetRightCol() + 1);
		rcNew.top    = t_max(rcNew.top   , textArea.GetViewTopLine());
		rcNew.bottom = t_max(rcNew.bottom, textArea.GetViewTopLine() - 1);	// 2010.11.02 ryoji 追加（画面上端よりも上にある矩形選択を解除するとルーラーが反転表示になる問題の修正）
		rcNew.bottom = t_min(rcNew.bottom, textArea.GetBottomLine() );

		RECT rcNew2;
		rcNew2.left		= (textArea.GetAreaLeft() - (Int)textArea.GetViewLeftCol() * nCharWidth) + (Int)rcNew.left  * nCharWidth;
		rcNew2.right	= (textArea.GetAreaLeft() - (Int)textArea.GetViewLeftCol() * nCharWidth) + (Int)rcNew.right * nCharWidth;
		rcNew2.top		= textArea.GenerateYPx(rcNew.top);
		rcNew2.bottom	= textArea.GenerateYPx(rcNew.bottom + 1);

		HRGN hrgnNew = ::CreateRectRgnIndirect(&rcNew2);

		// 矩形作画。
		// ::CombineRgn()の結果を受け取るために、適当なリージョンを作る
		HRGN hrgnDraw = ::CreateRectRgnIndirect(&rcNew2);
		{
			// 旧選択矩形と新選択矩形のリージョンを結合し､ 重なりあう部分だけを除去します
			if (::CombineRgn(hrgnDraw, hrgnOld, hrgnNew, RGN_XOR) != NULLREGION) {

				// 2002.02.16 hor
				// 結合後のエリアにEOFが含まれる場合はEOF以降の部分を除去します
				// 2006.10.01 Moca リーソースリークを修正したら、チラつくようになったため、
				// 抑えるために EOF以降をリージョンから削除して1度の作画にする

				// 2006.10.01 Moca Start EOF位置計算をGetEndLayoutPosに書き換え。
				LayoutPoint ptLast;
				pView->m_pEditDoc->m_layoutMgr.GetEndLayoutPos(&ptLast);
				// 2006.10.01 Moca End
				// 2011.12.26 EOFのぶら下がり行は反転し、EOFのみの行は反転しない
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
					//	Jan. 24, 2004 genta nLastLenは物理桁なので変換必要
					//	最終行にTABが入っていると反転範囲が不足する．
					//	2006.10.01 Moca GetEndLayoutPosで処理するためColumnToIndexは不要に。
					RECT rcNew;
					rcNew.left   = textArea.GetAreaLeft() + (Int)(textArea.GetViewLeftCol() + ptLast.x) * nCharWidth;
					rcNew.right  = textArea.GetAreaRight();
					rcNew.top    = textArea.GenerateYPx(ptLast.y);
					rcNew.bottom = rcNew.top + nCharHeight;
					
					// 2006.10.01 Moca GDI(リージョン)リソースリーク修正
					HRGN hrgnEOFNew = ::CreateRectRgnIndirect(&rcNew);
					::CombineRgn(hrgnDraw, hrgnDraw, hrgnEOFNew, RGN_DIFF);
					::DeleteObject(hrgnEOFNew);
				}
				::PaintRgn(hdc, hrgnDraw);
				// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
				if (bCompatBMP) {
					::PaintRgn(pView->m_hdcCompatDC, hrgnDraw);
				}
				// To Here 2007.09.09 Moca
			}
		}

		//////////////////////////////////////////
		// デバッグ用 リージョン矩形のダンプ
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

		// 現在描画されている範囲と始点が同じ
		if (m_select.GetFrom() == m_selectOld.GetFrom()) {
			// 範囲が後方に拡大された
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
			// 範囲が前方に拡大された
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

	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	if (bCompatBMP) {
		::SetROP2(pView->m_hdcCompatDC, nROPCompatOld);
		::SelectObject(pView->m_hdcCompatDC, hBrushCompatOld);
	}
	// To Here 2007.09.09 Moca

	// 2006.10.01 Moca 重複コード統合
	::SetROP2(hdc, nROP_Old);
	::SelectObject(hdc, hBrushOld);
	::DeleteObject(hBrush);
}


/*! 選択領域の中の指定行の描画

	複数行に渡る選択範囲のうち，nLineNumで指定された1行分だけを描画する．
	選択範囲は固定されたままnLineNumのみが必要行分変化しながら呼びだされる．

	@date 2006.03.29 Moca 3000桁制限を撤廃．
*/
void ViewSelect::DrawSelectAreaLine(
	HDC					hdc,		// [in] 描画領域のDevice Context Handle
	LayoutInt			nLineNum,	// [in] 描画対象行(レイアウト行)
	const LayoutRange&	range		// [in] 選択範囲(レイアウト単位)
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
			// 2006.03.28 Moca 画面外まで求めたら打ち切る
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
	
	// 2006.03.28 Moca ウィンドウ幅が大きいと正しく反転しない問題を修正
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
	//	必要なときだけ。
	if (rcClip.right != rcClip.left) {
		LayoutRange selectOld = m_select;
		const_cast<LayoutRange*>(&m_select)->Clear(-1);
		pView->GetCaret().m_underLine.CaretUnderLineOFF(true, false, true);
		*(const_cast<LayoutRange*>(&m_select)) = selectOld;
		
		// 2006.03.28 Moca 表示域内のみ処理する
		if (nSelectFrom <= textArea.GetRightCol() && textArea.GetViewLeftCol() < nSelectTo) {
			HRGN hrgnDraw = ::CreateRectRgn(rcClip.left, rcClip.top, rcClip.right, rcClip.bottom);
			::PaintRgn(hdc, hrgnDraw);
			// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
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
		if (IsBoxSelecting()) {		// 矩形範囲選択中
			nSelectFrom = range.GetFrom().GetX2();
			nSelectTo   = range.GetTo().GetX2();
			// 2006.09.30 Moca From 矩形選択時[EOF]とその右側は反転しないように修正。処理を追加
			// 2011.12.26 [EOF]単独行以外なら反転する
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
		// 2006.05.24 Moca 矩形選択/フリーカーソル選択(選択開始/終了行)で
		// To < From になることがある。必ず From < To になるように入れ替える。
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

/*!	選択範囲情報メッセージの表示

	@author genta
	@date 2005.07.09 genta 新規作成
	@date 2006.06.06 ryoji 選択範囲の行が実在しない場合の対策を追加
	@date 2006.06.28 syat バイト数カウントを追加
*/
void ViewSelect::PrintSelectionInfoMsg() const
{
	const EditView* pView = GetEditView();

	//	出力されないなら計算を省略
	if (!pView->m_pEditWnd->m_statusBar.SendStatusMessage2IsEffective())
		return;

	LayoutInt nLineCount = pView->m_pEditDoc->m_layoutMgr.GetLineCount();
	if (!IsTextSelected() || m_select.GetFrom().y >= nLineCount) { // 先頭行が実在しない
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
	//	From here 2006.06.06 ryoji 選択範囲の行が実在しない場合の対策

	LayoutInt select_line;
	if (m_select.GetTo().y >= nLineCount) {	// 最終行が実在しない
		select_line = nLineCount - m_select.GetFrom().y + 1;
	}else {
		select_line = m_select.GetTo().y - m_select.GetFrom().y + 1;
	}
	
	//	To here 2006.06.06 ryoji 選択範囲の行が実在しない場合の対策
	if (IsBoxSelecting()) {
		//	矩形の場合は幅と高さだけでごまかす
		LayoutInt select_col = m_select.GetFrom().x - m_select.GetTo().x;
		if (select_col < 0) {
			select_col = -select_col;
		}
		auto_sprintf_s(msg, _T("%d Columns * %d lines selected."),
			select_col, select_line);
			
	}else {
		//	通常の選択では選択範囲の中身を数える
		int select_sum = 0;	//	バイト数合計
		const wchar_t* pLine;	//	データを受け取る
		LogicInt	nLineLen;		//	行の長さ
		const Layout*	pLayout;
		ViewSelect* thiz = const_cast<ViewSelect*>(this);	// const外しthis

		// 共通設定・選択文字数を文字単位ではなくバイト単位で表示する
		bool bCountByByteCommon = GetDllShareData().m_common.statusBar.m_bDispSelCountByByte;
		bool bCountByByte = (pView->m_pEditWnd->m_nSelectCountMode == SelectCountMode::Toggle ?
								bCountByByteCommon :
								pView->m_pEditWnd->m_nSelectCountMode == SelectCountMode::ByByte);

		//	1行目
		pLine = pView->m_pEditDoc->m_layoutMgr.GetLineStr(m_select.GetFrom().GetY2(), &nLineLen, &pLayout);
		if (pLine) {
			if (bCountByByte) {
				//  バイト数でカウント
				//  内部文字コードから現在の文字コードに変換し、バイト数を取得する。
				//  コード変換は負荷がかかるため、選択範囲の増減分のみを対象とする。

				NativeW memW;
				Memory memCode;

				// 増減分文字列の取得にEditView::GetSelectedDataを使いたいが、m_select限定のため、
				// 呼び出し前にm_selectを書き換える。呼出し後に元に戻すので、constと言えないこともない。
				LayoutRange rngSelect = m_select;		// 選択領域の退避
				bool bSelExtend;						// 選択領域拡大フラグ

				// 最終行の処理
				pLine = pView->m_pEditDoc->m_layoutMgr.GetLineStr(m_select.GetTo().y, &nLineLen, &pLayout);
				if (pLine) {
					if (pView->LineColumnToIndex(pLayout, m_select.GetTo().GetX2()) == 0) {
						//	最終行の先頭にキャレットがある場合は
						//	その行を行数に含めない
						--select_line;
					}
				}else {
					//	最終行が空行なら
					//	その行を行数に含めない
					--select_line;
				}

				// 2009.07.07 syat m_nLastSelectedByteLenが0の場合は、差分ではなく全体を変換する（モード切替時にキャッシュクリアするため）

				if (m_bSelectAreaChanging && m_nLastSelectedByteLen && m_select.GetFrom() == m_selectOld.GetFrom()) {
					// 範囲が後方に拡大された
					if (PointCompare(m_select.GetTo(), m_selectOld.GetTo()) < 0) {
						bSelExtend = false;				// 縮小
						thiz->m_select = LayoutRange(m_select.GetTo(), m_selectOld.GetTo());
					}else {
						bSelExtend = true;				// 拡大
						thiz->m_select = LayoutRange(m_selectOld.GetTo(), m_select.GetTo());
					}

					const_cast<EditView*>(pView)->GetSelectedDataSimple(memW);
					thiz->m_select = rngSelect;		// m_selectを元に戻す
				}else if (
					m_bSelectAreaChanging
					&& m_nLastSelectedByteLen
					&& m_select.GetTo() == m_selectOld.GetTo()
				) {
					// 範囲が前方に拡大された
					if (PointCompare(m_select.GetFrom(), m_selectOld.GetFrom()) < 0) {
						bSelExtend = true;				// 拡大
						thiz->m_select = LayoutRange(m_select.GetFrom(), m_selectOld.GetFrom());
					}else {
						bSelExtend = false;				// 縮小
						thiz->m_select = LayoutRange(m_selectOld.GetFrom(), m_select.GetFrom());
					}

					const_cast<EditView*>(pView)->GetSelectedDataSimple(memW);
					thiz->m_select = rngSelect;		// m_selectを元に戻す
				}else {
					// 選択領域全体をコード変換対象にする
					const_cast<EditView*>(pView)->GetSelectedDataSimple(memW);
					bSelExtend = true;
					thiz->m_nLastSelectedByteLen = 0;
				}
				//  現在の文字コードに変換し、バイト長を取得する
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
				//  文字数でカウント

				// 2009.07.07 syat カウント方法を切り替えながら選択範囲を拡大・縮小すると整合性が
				//                とれなくなるため、モード切替時にキャッシュをクリアする。
				thiz->m_nLastSelectedByteLen = 0;

				//	1行だけ選択されている場合
				if (m_select.IsLineOne()) {
					select_sum =
						pView->LineColumnToIndex(pLayout, m_select.GetTo().GetX2())
						- pView->LineColumnToIndex(pLayout, m_select.GetFrom().GetX2());
				}else {	//	2行以上選択されている場合
					select_sum =
						pLayout->GetLengthWithoutEOL()
						+ pLayout->GetLayoutEol().GetLen()
						- pView->LineColumnToIndex(pLayout, m_select.GetFrom().GetX2());

					//	GetSelectedDataと似ているが，先頭行と最終行は排除している
					//	Aug. 16, 2005 aroka nLineNumはfor以降でも使われるのでforの前で宣言する
					//	VC .NET以降でもMicrosoft拡張を有効にした標準動作はVC6と同じことに注意
					LayoutInt nLineNum;
					for (nLineNum = m_select.GetFrom().GetY2() + LayoutInt(1);
						nLineNum < m_select.GetTo().GetY2();
						++nLineNum
					) {
						pLine = pView->m_pEditDoc->m_layoutMgr.GetLineStr(nLineNum, &nLineLen, &pLayout);
						//	2006.06.06 ryoji 指定行のデータが存在しない場合の対策
						if (!pLine)
							break;
						select_sum += pLayout->GetLengthWithoutEOL() + pLayout->GetLayoutEol().GetLen();
					}

					//	最終行の処理
					pLine = pView->m_pEditDoc->m_layoutMgr.GetLineStr(nLineNum, &nLineLen, &pLayout);
					if (pLine) {
						int last_line_chars = pView->LineColumnToIndex(pLayout, m_select.GetTo().GetX2());
						select_sum += last_line_chars;
						if (last_line_chars == 0) {
							//	最終行の先頭にキャレットがある場合は
							//	その行を行数に含めない
							--select_line;
						}
					}else {
						//	最終行が空行なら
						//	その行を行数に含めない
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

