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
	bSelectingLock   = false;	// 選択状態のロック
	bBeginSelect     = false;		// 範囲選択中
	bBeginBoxSelect  = false;	// 矩形範囲選択中
	bBeginLineSelect = false;	// 行単位選択中
	bBeginWordSelect = false;	// 単語単位選択中

	selectBgn.Clear(-1); // 範囲選択(原点)
	select.Clear(-1); // 範囲選択
	selectOld.Clear(0);  // 範囲選択(Old)
	bSelectAreaChanging = false;	// 選択範囲変更中
	nLastSelectedByteLen = 0;	// 前回選択時の選択バイト数
}

void ViewSelect::CopySelectStatus(ViewSelect* pSelect) const
{
	pSelect->bSelectingLock		= bSelectingLock;		// 選択状態のロック
	pSelect->bBeginSelect		= bBeginSelect;		// 範囲選択中
	pSelect->bBeginBoxSelect	= bBeginBoxSelect;	// 矩形範囲選択中

	pSelect->selectBgn			= selectBgn;			// 範囲選択(原点)
	pSelect->select			= select;				// 範囲選択
	pSelect->selectOld		= selectOld;			// 範囲選択

	pSelect->ptMouseRollPosOld	= ptMouseRollPosOld;	// マウス範囲選択前回位置(XY座標)
}

// 現在のカーソル位置から選択を開始する
void ViewSelect::BeginSelectArea(const Point* po)
{
	const EditView& view = GetEditView();
	Point temp;
	if (!po) {
		temp = view.GetCaret().GetCaretLayoutPos();
		po = &temp;
	}
	selectBgn.Set(*po); // 範囲選択(原点)
	select.   Set(*po); // 範囲選択
}


// 現在の選択範囲を非選択状態に戻す
void ViewSelect::DisableSelectArea(bool bDraw, bool bDrawBracketCursorLine)
{
	const EditView& view = GetEditView();
	EditView& view2 = GetEditView();

	selectOld = select;		// 範囲選択(Old)
	select.Clear(-1);
	bSelectingLock	 = false;	// 選択状態のロック

	if (bDraw) {
		DrawSelectArea(bDrawBracketCursorLine);
	}
	bDrawSelectArea = false;	// 02/12/13 ai // 2011.12.24 bDraw括弧内から移動

	selectOld.Clear(0);			// 範囲選択(Old)
	bBeginBoxSelect = false;		// 矩形範囲選択中
	bBeginLineSelect = false;		// 行単位選択中
	bBeginWordSelect = false;		// 単語単位選択中
	nLastSelectedByteLen = 0;		// 前回選択時の選択バイト数

	// 2002.02.16 hor 直前のカーソル位置をリセット
	view2.GetCaret().nCaretPosX_Prev = view.GetCaret().GetCaretLayoutPos().GetX();

}


// 現在のカーソル位置によって選択範囲を変更
void ViewSelect::ChangeSelectAreaByCurrentCursor(const Point& ptCaretPos)
{
	selectOld = select; // 範囲選択(Old)

	//	2002/04/08 YAZAKI コードの重複を排除
	ChangeSelectAreaByCurrentCursorTEST(
		ptCaretPos,
		&select
	);

	// 選択領域の描画
	bSelectAreaChanging = true;
	DrawSelectArea(true);
	bSelectAreaChanging = false;
}


// 現在のカーソル位置によって選択範囲を変更(テストのみ)
void ViewSelect::ChangeSelectAreaByCurrentCursorTEST(
	const Point& ptCaretPos,
	Range* pSelect
)
{
	if (selectBgn.GetFrom() == selectBgn.GetTo()) {
		if (ptCaretPos == selectBgn.GetFrom()) {
			// 選択解除
			pSelect->Clear(-1);
			nLastSelectedByteLen = 0;		// 前回選択時の選択バイト数
		}else if (PointCompare(ptCaretPos, selectBgn.GetFrom()) < 0) { // キャレット位置がsSelectBgnのfromより小さかったら
			 pSelect->SetFrom(ptCaretPos);
			 pSelect->SetTo(selectBgn.GetFrom());
		}else {
			pSelect->SetFrom(selectBgn.GetFrom());
			pSelect->SetTo(ptCaretPos);
		}
	}else {
		// 常時選択範囲の範囲内
		// キャレット位置が sSelectBgn の from以上で、toより小さい場合
		if (PointCompare(ptCaretPos, selectBgn.GetFrom()) >= 0 && PointCompare(ptCaretPos, selectBgn.GetTo()) < 0) {
			pSelect->SetFrom(selectBgn.GetFrom());
			if (ptCaretPos == selectBgn.GetFrom()) {
				pSelect->SetTo(selectBgn.GetTo());
			}else {
				pSelect->SetTo(ptCaretPos);
			}
		// キャレット位置がsSelectBgnのfromより小さかったら
		}else if (PointCompare(ptCaretPos, selectBgn.GetFrom()) < 0) {
			// 常時選択範囲の前方向
			pSelect->SetFrom(ptCaretPos);
			pSelect->SetTo(selectBgn.GetTo());
		}else {
			// 常時選択範囲の後ろ方向
			pSelect->SetFrom(selectBgn.GetFrom());
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
	EditView& view = GetEditView();

	if (!view.GetDrawSwitch()) {
		return;
	}
	bDrawSelectArea = true;
	
	bool bDispText = TypeSupport(view, COLORIDX_SELECT).IsDisp();
	if (bDispText) {
		if (select != selectOld) {
			// 選択色表示の時は、WM_PAINT経由で作画
			const size_t nCharWidth = view.GetTextMetrics().GetHankakuDx();
			const TextArea& area =  view.GetTextArea();
			Rect rcOld; // LayoutRect
			TwoPointToRect(&rcOld, selectOld.GetFrom(), selectOld.GetTo());
			Rect rcNew; // LayoutRect
			TwoPointToRect(&rcNew, select.GetFrom(), select.GetTo());
			Rect rc; // LayoutRect ただしtop,bottomだけ使う
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
					// GetToの行が対象
					rc.top = rc.bottom = select.GetTo().y;
					drawLeft  = t_min(select.GetTo().x, selectOld.GetTo().x);
					drawRight = t_max(select.GetTo().x, selectOld.GetTo().x) + 1;
				}else if (select.GetTo() == selectOld.GetTo() && select.GetFrom().x != selectOld.GetFrom().x) {
					// GetFromの行が対象
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
				// 背景表示のクリッピングが甘いので左右を指定しない
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
				// DrawSelectAreaLineでの下線OFFの代わり
				view.GetCaret().underLine.CaretUnderLineOFF(true, false);
				view.GetCaret().underLine.Lock();
				view.OnPaint(hdc, &ps, false);
				view.GetCaret().underLine.UnLock();
				view.ReleaseDC(hdc);
			}
			// 2010.10.10 0幅選択(解除)状態での、カーソル位置ライン復帰(リージョン外)
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
		// 2011.12.02 選択解除状態での、カーソル位置ライン復帰
		if (bDrawBracketCursorLine) {
			view.GetCaret().underLine.CaretUnderLineON(true, false);
		}
		view.ReleaseDC(hdc);
	}

	// 2011.12.02 選択解除状態になると対括弧強調ができなくなるバグ対策
	if (!IsTextSelecting()) {
		// ただし選択ロック中はここでは強調表示されない
		bDrawSelectArea = false;
		if (bDrawBracketCursorLine) {
			view.SetBracketPairPos(true);
			view.DrawBracketPair(true);
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
	auto& view = GetEditView();

	// 2006.10.01 Moca 重複コード統合
	HBRUSH	hBrush = ::CreateSolidBrush(SELECTEDAREA_RGB);
	HBRUSH	hBrushOld = (HBRUSH)::SelectObject(hdc, hBrush);
	int		nROP_Old = ::SetROP2(hdc, SELECTEDAREA_ROP2);
	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
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
	if (IsBoxSelecting()) {		// 矩形範囲選択中
		// 2001.12.21 hor 矩形エリアにEOFがある場合、RGN_XORで結合すると
		// EOF以降のエリアも反転してしまうので、この場合はRedrawを使う
		// 2002.02.16 hor ちらつきを抑止するためEOF以降のエリアが反転したらもう一度反転して元に戻すことにする
		//if ((GetTextArea().GetViewTopLine()+nViewRowNum+1 >= pEditDoc->layoutMgr.GetLineCount()) &&
		//   (select.GetTo().y+1 >= pEditDoc->layoutMgr.GetLineCount() ||
		//	selectOld.GetTo().y+1 >= pEditDoc->layoutMgr.GetLineCount())) {
		//	Redraw();
		//	return;
		//}

		const size_t nCharWidth = view.GetTextMetrics().GetHankakuDx();
		const size_t nCharHeight = view.GetTextMetrics().GetHankakuDy();

		// 2点を対角とする矩形を求める
		Rect rcOld;
		TwoPointToRect(
			&rcOld,
			selectOld.GetFrom(),	// 範囲選択開始
			selectOld.GetTo()	// 範囲選択終了
		);
		rcOld.left   = t_max((int)rcOld.left  , (int)textArea.GetViewLeftCol() );
		rcOld.right  = t_max((int)rcOld.right , (int)textArea.GetViewLeftCol() );
		rcOld.right  = t_min((int)rcOld.right , (int)textArea.GetRightCol() + 1);
		rcOld.top    = t_max((int)rcOld.top   , (int)textArea.GetViewTopLine() );
		rcOld.bottom = t_max((int)rcOld.bottom, (int)textArea.GetViewTopLine() - 1);	// 2010.11.02 ryoji 追加（画面上端よりも上にある矩形選択を解除するとルーラーが反転表示になる問題の修正）
		rcOld.bottom = t_min((int)rcOld.bottom, (int)textArea.GetBottomLine()  );

		RECT rcOld2;
		rcOld2.left		= (textArea.GetAreaLeft() - textArea.GetViewLeftCol() * nCharWidth) + rcOld.left  * nCharWidth;
		rcOld2.right	= (textArea.GetAreaLeft() - textArea.GetViewLeftCol() * nCharWidth) + rcOld.right * nCharWidth;
		rcOld2.top		= textArea.GenerateYPx(rcOld.top);
		rcOld2.bottom	= textArea.GenerateYPx(rcOld.bottom + 1);
		HRGN hrgnOld = ::CreateRectRgnIndirect(&rcOld2);

		// 2点を対角とする矩形を求める
		Rect rcNew;
		TwoPointToRect(
			&rcNew,
			select.GetFrom(),	// 範囲選択開始
			select.GetTo()		// 範囲選択終了
		);
		rcNew.left   = t_max((int)rcNew.left  , (int)textArea.GetViewLeftCol());
		rcNew.right  = t_max((int)rcNew.right , (int)textArea.GetViewLeftCol());
		rcNew.right  = t_min((int)rcNew.right , (int)textArea.GetRightCol() + 1);
		rcNew.top    = t_max((int)rcNew.top   , (int)textArea.GetViewTopLine());
		rcNew.bottom = t_max((int)rcNew.bottom, (int)textArea.GetViewTopLine() - 1);	// 2010.11.02 ryoji 追加（画面上端よりも上にある矩形選択を解除するとルーラーが反転表示になる問題の修正）
		rcNew.bottom = t_min((int)rcNew.bottom, (int)textArea.GetBottomLine() );

		RECT rcNew2;
		rcNew2.left		= (textArea.GetAreaLeft() - textArea.GetViewLeftCol() * nCharWidth) + rcNew.left  * nCharWidth;
		rcNew2.right	= (textArea.GetAreaLeft() - textArea.GetViewLeftCol() * nCharWidth) + rcNew.right * nCharWidth;
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
				Point ptLast;
				view.pEditDoc->layoutMgr.GetEndLayoutPos(&ptLast);
				// 2006.10.01 Moca End
				// 2011.12.26 EOFのぶら下がり行は反転し、EOFのみの行は反転しない
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
					//	Jan. 24, 2004 genta nLastLenは物理桁なので変換必要
					//	最終行にTABが入っていると反転範囲が不足する．
					//	2006.10.01 Moca GetEndLayoutPosで処理するためColumnToIndexは不要に。
					RECT rcNew;
					rcNew.left   = textArea.GetAreaLeft() + (textArea.GetViewLeftCol() + ptLast.x) * nCharWidth;
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
					::PaintRgn(view.hdcCompatDC, hrgnDraw);
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
		Range rangeA;
		int nLineNum;

		// 現在描画されている範囲と始点が同じ
		if (select.GetFrom() == selectOld.GetFrom()) {
			// 範囲が後方に拡大された
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
			// 範囲が前方に拡大された
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

	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	if (bCompatBMP) {
		::SetROP2(view.hdcCompatDC, nROPCompatOld);
		::SelectObject(view.hdcCompatDC, hBrushCompatOld);
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
	HDC hdc,			// [in] 描画領域のDevice Context Handle
	int nLineNum,		// [in] 描画対象行(レイアウト行)
	const Range& range	// [in] 選択範囲(レイアウト単位)
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
			// 2006.03.28 Moca 画面外まで求めたら打ち切る
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
	
	// 2006.03.28 Moca ウィンドウ幅が大きいと正しく反転しない問題を修正
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
	//	必要なときだけ。
	if (rcClip.right != rcClip.left) {
		Range selectOld = select;
		const_cast<Range*>(&select)->Clear(-1);
		view.GetCaret().underLine.CaretUnderLineOFF(true, false, true);
		*(const_cast<Range*>(&select)) = selectOld;
		
		// 2006.03.28 Moca 表示域内のみ処理する
		if (nSelectFrom <= textArea.GetRightCol() && textArea.GetViewLeftCol() < nSelectTo) {
			HRGN hrgnDraw = ::CreateRectRgn(rcClip.left, rcClip.top, rcClip.right, rcClip.bottom);
			::PaintRgn(hdc, hrgnDraw);
			// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
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
		if (IsBoxSelecting()) {		// 矩形範囲選択中
			nSelectFrom = range.GetFrom().x;
			nSelectTo   = range.GetTo().x;
			// 2006.09.30 Moca From 矩形選択時[EOF]とその右側は反転しないように修正。処理を追加
			// 2011.12.26 [EOF]単独行以外なら反転する
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
		// 2006.05.24 Moca 矩形選択/フリーカーソル選択(選択開始/終了行)で
		// To < From になることがある。必ず From < To になるように入れ替える。
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

/*!	選択範囲情報メッセージの表示

	@author genta
	@date 2005.07.09 genta 新規作成
	@date 2006.06.06 ryoji 選択範囲の行が実在しない場合の対策を追加
	@date 2006.06.28 syat バイト数カウントを追加
*/
void ViewSelect::PrintSelectionInfoMsg() const
{
	auto& view = GetEditView();

	//	出力されないなら計算を省略
	if (!view.editWnd.statusBar.SendStatusMessage2IsEffective())
		return;

	int nLineCount = view.pEditDoc->layoutMgr.GetLineCount();
	if (!IsTextSelected() || select.GetFrom().y >= nLineCount) { // 先頭行が実在しない
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
	//	From here 2006.06.06 ryoji 選択範囲の行が実在しない場合の対策

	int select_line;
	if (select.GetTo().y >= nLineCount) {	// 最終行が実在しない
		select_line = nLineCount - select.GetFrom().y + 1;
	}else {
		select_line = select.GetTo().y - select.GetFrom().y + 1;
	}
	
	//	To here 2006.06.06 ryoji 選択範囲の行が実在しない場合の対策
	if (IsBoxSelecting()) {
		//	矩形の場合は幅と高さだけでごまかす
		int select_col = select.GetFrom().x - select.GetTo().x;
		if (select_col < 0) {
			select_col = -select_col;
		}
		auto_sprintf_s(msg, _T("%d Columns * %d lines selected."),
			select_col, select_line);
			
	}else {
		//	通常の選択では選択範囲の中身を数える
		size_t select_sum = 0;	//	バイト数合計
		const wchar_t* pLine;	//	データを受け取る
		size_t nLineLen;		//	行の長さ
		const Layout*	pLayout;
		ViewSelect* thiz = const_cast<ViewSelect*>(this);	// const外しthis

		// 共通設定・選択文字数を文字単位ではなくバイト単位で表示する
		bool bCountByByteCommon = GetDllShareData().common.statusBar.bDispSelCountByByte;
		bool bCountByByte = (view.editWnd.nSelectCountMode == SelectCountMode::Toggle ?
								bCountByByteCommon :
								view.editWnd.nSelectCountMode == SelectCountMode::ByByte);

		//	1行目
		pLine = view.pEditDoc->layoutMgr.GetLineStr(select.GetFrom().y, &nLineLen, &pLayout);
		if (pLine) {
			if (bCountByByte) {
				//  バイト数でカウント
				//  内部文字コードから現在の文字コードに変換し、バイト数を取得する。
				//  コード変換は負荷がかかるため、選択範囲の増減分のみを対象とする。

				NativeW memW;
				Memory memCode;

				// 増減分文字列の取得にEditView::GetSelectedDataを使いたいが、select限定のため、
				// 呼び出し前にselectを書き換える。呼出し後に元に戻すので、constと言えないこともない。
				Range rngSelect = select;		// 選択領域の退避
				bool bSelExtend;						// 選択領域拡大フラグ

				// 最終行の処理
				pLine = view.pEditDoc->layoutMgr.GetLineStr(select.GetTo().y, &nLineLen, &pLayout);
				if (pLine) {
					if (view.LineColumnToIndex(pLayout, select.GetTo().x) == 0) {
						//	最終行の先頭にキャレットがある場合は
						//	その行を行数に含めない
						--select_line;
					}
				}else {
					//	最終行が空行なら
					//	その行を行数に含めない
					--select_line;
				}

				// 2009.07.07 syat nLastSelectedByteLenが0の場合は、差分ではなく全体を変換する（モード切替時にキャッシュクリアするため）

				if (bSelectAreaChanging && nLastSelectedByteLen && select.GetFrom() == selectOld.GetFrom()) {
					// 範囲が後方に拡大された
					if (PointCompare(select.GetTo(), selectOld.GetTo()) < 0) {
						bSelExtend = false;				// 縮小
						thiz->select = Range(select.GetTo(), selectOld.GetTo());
					}else {
						bSelExtend = true;				// 拡大
						thiz->select = Range(selectOld.GetTo(), select.GetTo());
					}

					const_cast<EditView&>(view).GetSelectedDataSimple(memW);
					thiz->select = rngSelect;		// selectを元に戻す
				}else if (
					bSelectAreaChanging
					&& nLastSelectedByteLen
					&& select.GetTo() == selectOld.GetTo()
				) {
					// 範囲が前方に拡大された
					if (PointCompare(select.GetFrom(), selectOld.GetFrom()) < 0) {
						bSelExtend = true;				// 拡大
						thiz->select = Range(select.GetFrom(), selectOld.GetFrom());
					}else {
						bSelExtend = false;				// 縮小
						thiz->select = Range(selectOld.GetFrom(), select.GetFrom());
					}

					const_cast<EditView&>(view).GetSelectedDataSimple(memW);
					thiz->select = rngSelect;		// selectを元に戻す
				}else {
					// 選択領域全体をコード変換対象にする
					const_cast<EditView&>(view).GetSelectedDataSimple(memW);
					bSelExtend = true;
					thiz->nLastSelectedByteLen = 0;
				}
				//  現在の文字コードに変換し、バイト長を取得する
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
				//  文字数でカウント

				// 2009.07.07 syat カウント方法を切り替えながら選択範囲を拡大・縮小すると整合性が
				//                とれなくなるため、モード切替時にキャッシュをクリアする。
				thiz->nLastSelectedByteLen = 0;

				//	1行だけ選択されている場合
				if (select.IsLineOne()) {
					select_sum =
						view.LineColumnToIndex(pLayout, select.GetTo().x)
						- view.LineColumnToIndex(pLayout, select.GetFrom().x);
				}else {	//	2行以上選択されている場合
					select_sum =
						pLayout->GetLengthWithoutEOL()
						+ pLayout->GetLayoutEol().GetLen()
						- view.LineColumnToIndex(pLayout, select.GetFrom().x);

					//	GetSelectedDataと似ているが，先頭行と最終行は排除している
					//	Aug. 16, 2005 aroka nLineNumはfor以降でも使われるのでforの前で宣言する
					//	VC .NET以降でもMicrosoft拡張を有効にした標準動作はVC6と同じことに注意
					int nLineNum;
					for (nLineNum = select.GetFrom().y + 1;
						nLineNum < select.GetTo().y;
						++nLineNum
					) {
						pLine = view.pEditDoc->layoutMgr.GetLineStr(nLineNum, &nLineLen, &pLayout);
						//	2006.06.06 ryoji 指定行のデータが存在しない場合の対策
						if (!pLine)
							break;
						select_sum += pLayout->GetLengthWithoutEOL() + pLayout->GetLayoutEol().GetLen();
					}

					//	最終行の処理
					pLine = view.pEditDoc->layoutMgr.GetLineStr(nLineNum, &nLineLen, &pLayout);
					if (pLine) {
						int last_line_chars = view.LineColumnToIndex(pLayout, select.GetTo().x);
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

