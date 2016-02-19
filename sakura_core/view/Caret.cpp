/*!	@file
	@brief キャレットの管理

	@author	kobake
*/
/*
	Copyright (C) 2008, kobake, ryoji, Uchi
	Copyright (C) 2009, ryoji, nasukoji
	Copyright (C) 2010, ryoji, Moca
	Copyright (C) 2011, Moca, syat
	Copyright (C) 2012, ryoji, Moca
	Copyright (C) 2013, Moca, Uchi

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include <algorithm>
#include "view/Caret.h"
#include "view/EditView.h"
#include "view/TextArea.h"
#include "view/TextMetrics.h"
#include "view/ViewFont.h"
#include "view/Ruler.h"
#include "doc/EditDoc.h"
#include "doc/layout/Layout.h"
#include "mem/MemoryIterator.h"
#include "charset/charcode.h"
#include "charset/CodePage.h"
#include "charset/CodeFactory.h"
#include "charset/CodeBase.h"
#include "window/EditWnd.h"

using namespace std;

#define SCROLLMARGIN_LEFT 4
#define SCROLLMARGIN_RIGHT 4
#define SCROLLMARGIN_NOMOVE 4

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         外部依存                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


inline int Caret::GetHankakuDx() const
{
	return m_pEditView->GetTextMetrics().GetHankakuDx();
}

inline int Caret::GetHankakuHeight() const
{
	return m_pEditView->GetTextMetrics().GetHankakuHeight();
}

inline int Caret::GetHankakuDy() const
{
	return m_pEditView->GetTextMetrics().GetHankakuDy();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      CaretUnderLine                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// カーソル行アンダーラインのON
void CaretUnderLine::CaretUnderLineON(bool bDraw, bool bPaintDraw)
{
	if (m_nLockCounter) return;	//	ロックされていたら何もできない。
	m_pEditView->CaretUnderLineON(bDraw, bPaintDraw, m_nUnderLineLockCounter != 0);
}

// カーソル行アンダーラインのOFF
void CaretUnderLine::CaretUnderLineOFF(bool bDraw, bool bDrawPaint, bool bResetFlag)
{
	if (m_nLockCounter) return;	//	ロックされていたら何もできない。
	m_pEditView->CaretUnderLineOFF(bDraw, bDrawPaint, bResetFlag, m_nUnderLineLockCounter != 0);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

Caret::Caret(EditView* pEditView, const EditDoc* pEditDoc)
	:
	m_pEditView(pEditView),
	m_pEditDoc(pEditDoc),
	m_ptCaretPos_Layout(0, 0),
	m_ptCaretPos_Logic(0, 0),			// カーソル位置 (改行単位行先頭からのバイト数(0開始), 改行単位行の行番号(0開始))
	m_sizeCaret(0, 0),					// キャレットのサイズ
	m_underLine(pEditView)
{
	m_nCaretPosX_Prev = LayoutInt(0);	// ビュー左端からのカーソル桁直前の位置(０オリジン)

	m_crCaret = -1;				// キャレットの色				// 2006.12.16 ryoji
	m_hbmpCaret = NULL;			// キャレット用ビットマップ		// 2006.11.28 ryoji
	m_bClearStatus = true;
	ClearCaretPosInfoCache();
}

Caret::~Caret()
{
	// キャレット用ビットマップ	// 2006.11.28 ryoji
	if (m_hbmpCaret)
		DeleteObject(m_hbmpCaret);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	@brief 行桁指定によるカーソル移動

	必要に応じて縦/横スクロールもする．
	垂直スクロールをした場合はその行数を返す（正／負）．
	
	@return 縦スクロール行数(負:上スクロール/正:下スクロール)

	@note 不正な位置が指定された場合には適切な座標値に
		移動するため，引数で与えた座標と移動後の座標は
		必ずしも一致しない．
	
	@note bScrollがfalseの場合にはカーソル位置のみ移動する．
		trueの場合にはスクロール位置があわせて変更される

	@note 同じ行の左右移動はアンダーラインを一度消す必要が無いので
		bUnderlineDoNotOFFを指定すると高速化できる.
		同様に同じ桁の上下移動はbVertLineDoNotOFFを指定すると
		カーソル位置縦線の消去を省いて高速化できる.

	@date 2001.10.20 deleted by novice AdjustScrollBar()を呼ぶ位置を変更
	@date 2004.04.02 Moca 行だけ有効な座標に修正するのを厳密に処理する
	@date 2004.09.11 genta bDrawスイッチは動作と名称が一致していないので
		再描画スイッチ→画面位置調整スイッチと名称変更
	@date 2009.08.28 nasukoji	テキスト折り返しの「折り返さない」対応
	@date 2010.11.27 syat アンダーライン、縦線を消去しないフラグを追加
*/
LayoutInt Caret::MoveCursor(
	LayoutPoint	ptWk_CaretPos,		// [in] 移動先レイアウト位置
	bool			bScroll,			// [in] true: 画面位置調整有り  false: 画面位置調整無し
	int				nCaretMarginRate,	// [in] 縦スクロール開始位置を決める値
	bool			bUnderLineDoNotOFF,	// [in] アンダーラインを消去しない
	bool			bVertLineDoNotOFF	// [in] カーソル位置縦線を消去しない
)
{
	// スクロール処理
	LayoutInt	nScrollRowNum = LayoutInt(0);
	LayoutInt	nScrollColNum = LayoutInt(0);
	int		nCaretMarginY;
	LayoutInt		nScrollMarginRight;
	LayoutInt		nScrollMarginLeft;

	auto& textArea = m_pEditView->GetTextArea();
	if (0 >= textArea.m_nViewColNum) {
		return LayoutInt(0);
	}

	if (m_pEditView->GetSelectionInfo().IsMouseSelecting()) {	// 範囲選択中
		nCaretMarginY = 0;
	}else {
		//	2001/10/20 novice
		nCaretMarginY = (Int)textArea.m_nViewRowNum / nCaretMarginRate;
		if (1 > nCaretMarginY) {
			nCaretMarginY = 1;
		}
	}
	// 2004.04.02 Moca 行だけ有効な座標に修正するのを厳密に処理する
	GetAdjustCursorPos(&ptWk_CaretPos);
	m_pEditDoc->m_layoutMgr.LayoutToLogic(
		ptWk_CaretPos,
		&m_ptCaretPos_Logic	// カーソル位置。ロジック単位。
	);
	// キャレット移動
	SetCaretLayoutPos(ptWk_CaretPos);

	// カーソル行アンダーラインのOFF
	bool bDrawPaint = ptWk_CaretPos.GetY2() != m_pEditView->m_nOldUnderLineYBg;
	m_underLine.SetUnderLineDoNotOFF(bUnderLineDoNotOFF);
	m_underLine.SetVertLineDoNotOFF(bVertLineDoNotOFF);
	m_underLine.CaretUnderLineOFF(bScroll, bDrawPaint);	//	YAZAKI
	m_underLine.SetUnderLineDoNotOFF(false);
	m_underLine.SetVertLineDoNotOFF(false);
	
	// 水平スクロール量（文字数）の算出
	nScrollColNum = LayoutInt(0);
	nScrollMarginRight = LayoutInt(SCROLLMARGIN_RIGHT);
	nScrollMarginLeft = LayoutInt(SCROLLMARGIN_LEFT);

	// 2010.08.24 Moca 幅が狭い場合のマージンの調整
	{
		// カーソルが真ん中にあるときに左右にぶれないように
		int nNoMove = SCROLLMARGIN_NOMOVE;
		LayoutInt a = ((textArea.m_nViewColNum) - nNoMove) / 2;
		LayoutInt nMin = (2 <= a ? a : LayoutInt(0)); // 1だと全角移動に支障があるので2以上
		nScrollMarginRight = t_min(nScrollMarginRight, nMin);
		nScrollMarginLeft  = t_min(nScrollMarginLeft,  nMin);
	}
	
	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	if (m_pEditDoc->m_layoutMgr.GetMaxLineKetas() > textArea.m_nViewColNum
		&& ptWk_CaretPos.GetX() > textArea.GetViewLeftCol() + textArea.m_nViewColNum - nScrollMarginRight
	) {
		nScrollColNum =
			(textArea.GetViewLeftCol() + textArea.m_nViewColNum - nScrollMarginRight) - ptWk_CaretPos.GetX2();
	}else if (1
		&& 0 < textArea.GetViewLeftCol()
		&& ptWk_CaretPos.GetX() < textArea.GetViewLeftCol() + nScrollMarginLeft
	) {
		nScrollColNum = textArea.GetViewLeftCol() + nScrollMarginLeft - ptWk_CaretPos.GetX2();
		if (0 > textArea.GetViewLeftCol() - nScrollColNum) {
			nScrollColNum = textArea.GetViewLeftCol();
		}
	}

	// 2013.12.30 bScrollがOFFのときは横スクロールしない
	if (bScroll) {
		textArea.SetViewLeftCol(textArea.GetViewLeftCol() - nScrollColNum);
	}else {
		nScrollColNum = 0;
	}

	//	From Here 2007.07.28 じゅうじ : 表示行数が3行以下の場合の動作改善
	// 垂直スクロール量（行数）の算出
	// 画面が３行以下
	if (textArea.m_nViewRowNum <= 3) {
		// 移動先は、画面のスクロールラインより上か？（up キー）
		if (ptWk_CaretPos.y - textArea.GetViewTopLine() < nCaretMarginY) {
			if (ptWk_CaretPos.y < nCaretMarginY) {	// １行目に移動
				nScrollRowNum = textArea.GetViewTopLine();
			}else if (textArea.m_nViewRowNum <= 1) {	// 画面が１行
				nScrollRowNum = textArea.GetViewTopLine() - ptWk_CaretPos.y;
			}
#if !(0)	// COMMENTにすると、上下の空きを死守しない為、縦移動はgoodだが、横移動の場合上下にぶれる
			else if (textArea.m_nViewRowNum <= 2) {	// 画面が２行
				nScrollRowNum = textArea.GetViewTopLine() - ptWk_CaretPos.y;
			}
#endif
			else {						// 画面が３行
				nScrollRowNum = textArea.GetViewTopLine() - ptWk_CaretPos.y + 1;
			}
		// 移動先は、画面の最大行数−２より下か？（down キー）
		}else if (ptWk_CaretPos.y - textArea.GetViewTopLine() >= (textArea.m_nViewRowNum - nCaretMarginY - 2)) {
			LayoutInt ii = m_pEditDoc->m_layoutMgr.GetLineCount();
			if (1
				&& ii - ptWk_CaretPos.y < nCaretMarginY + 1
				&& ii - textArea.GetViewTopLine() < textArea.m_nViewRowNum
			) {
			}else if (textArea.m_nViewRowNum <= 2) {	// 画面が２行、１行
				nScrollRowNum = textArea.GetViewTopLine() - ptWk_CaretPos.y;
			}else {						// 画面が３行
				nScrollRowNum = textArea.GetViewTopLine() - ptWk_CaretPos.y + 1;
			}
		}
	// 移動先は、画面のスクロールラインより上か？（up キー）
	}else if (ptWk_CaretPos.y - textArea.GetViewTopLine() < nCaretMarginY) {
		if (ptWk_CaretPos.y < nCaretMarginY) {	// １行目に移動
			nScrollRowNum = textArea.GetViewTopLine();
		}else {
			nScrollRowNum = -(ptWk_CaretPos.y - textArea.GetViewTopLine()) + nCaretMarginY;
		}
	// 移動先は、画面の最大行数−２より下か？（down キー）
	}else if (ptWk_CaretPos.y - textArea.GetViewTopLine() >= textArea.m_nViewRowNum - nCaretMarginY - 2) {
		LayoutInt ii = m_pEditDoc->m_layoutMgr.GetLineCount();
		if (1
			&& ii - ptWk_CaretPos.y < nCaretMarginY + 1
			&& ii - textArea.GetViewTopLine() < textArea.m_nViewRowNum
		) {
		}else {
			nScrollRowNum =
				-(ptWk_CaretPos.y - textArea.GetViewTopLine()) + (textArea.m_nViewRowNum - nCaretMarginY - 2);
		}
	}
	//	To Here 2007.07.28 じゅうじ
	if (bScroll) {
		// スクロール
		if (0
			|| t_abs(nScrollColNum) >= textArea.m_nViewColNum
			|| t_abs(nScrollRowNum) >= textArea.m_nViewRowNum
		) {
			textArea.OffsetViewTopLine(-nScrollRowNum);
			if (m_pEditView->GetDrawSwitch()) {
				m_pEditView->InvalidateRect(NULL);
				if (m_pEditView->m_pEditWnd->GetMiniMap().GetHwnd()) {
					m_pEditView->MiniMapRedraw(true);
				}
			}
		}else if (nScrollRowNum != 0 || nScrollColNum != 0) {
			RECT	rcClip;
			RECT	rcClip2;
			RECT	rcScroll;

			textArea.GenerateTextAreaRect(&rcScroll);
			if (nScrollRowNum > 0) {
				rcScroll.bottom = textArea.GetAreaBottom() - (Int)nScrollRowNum * m_pEditView->GetTextMetrics().GetHankakuDy();
				textArea.OffsetViewTopLine(-nScrollRowNum);
				textArea.GenerateTopRect(&rcClip, nScrollRowNum);
			}else if (nScrollRowNum < 0) {
				rcScroll.top = textArea.GetAreaTop() - (Int)nScrollRowNum * m_pEditView->GetTextMetrics().GetHankakuDy();
				textArea.OffsetViewTopLine(-nScrollRowNum);
				textArea.GenerateBottomRect(&rcClip, -nScrollRowNum);
			}

			if (nScrollColNum > 0) {
				rcScroll.left = textArea.GetAreaLeft();
				rcScroll.right = textArea.GetAreaRight() - (Int)nScrollColNum * GetHankakuDx();
				textArea.GenerateLeftRect(&rcClip2, nScrollColNum);
			}else if (nScrollColNum < 0) {
				rcScroll.left = textArea.GetAreaLeft() - (Int)nScrollColNum * GetHankakuDx();
				textArea.GenerateRightRect(&rcClip2, -nScrollColNum);
			}

			if (m_pEditView->GetDrawSwitch()) {
				m_pEditView->ScrollDraw(nScrollRowNum, nScrollColNum, rcScroll, rcClip, rcClip2);
				if (m_pEditView->m_pEditWnd->GetMiniMap().GetHwnd()) {
					m_pEditView->MiniMapRedraw(false);
				}
			}
		}

		// スクロールバーの状態を更新する
		m_pEditView->AdjustScrollBars(); // 2001/10/20 novice
	}

	// 横スクロールが発生したら、ルーラー全体を再描画 2002.02.25 Add By KK
	if (nScrollColNum != 0) {
		// 次回DispRuler呼び出し時に再描画。（bDraw=falseのケースを考慮した。）
		m_pEditView->GetRuler().SetRedrawFlag();
	}

	// カーソル行アンダーラインのON
	//CaretUnderLineON(bDraw); //2002.02.27 Del By KK アンダーラインのちらつきを低減
	if (bScroll) {
		// キャレットの表示・更新
		ShowEditCaret();

		// ルーラの再描画
		HDC		hdc = m_pEditView->GetDC();
		m_pEditView->GetRuler().DispRuler(hdc);
		m_pEditView->ReleaseDC(hdc);

		// アンダーラインの再描画
		m_underLine.CaretUnderLineON(true, bDrawPaint);

		// キャレットの行桁位置を表示する
		ShowCaretPosInfo();

		//	Sep. 11, 2004 genta 同期スクロールの関数化
		//	bScroll == FALSEの時にはスクロールしないので，実行しない
		m_pEditView->SyncScrollV(-nScrollRowNum);	//	方向が逆なので符号反転が必要
		m_pEditView->SyncScrollH(-nScrollColNum);	//	方向が逆なので符号反転が必要

	}

// 02/09/18 対括弧の強調表示 ai Start	03/02/18 ai mod S
	m_pEditView->DrawBracketPair(false);
	m_pEditView->SetBracketPairPos(true);
	m_pEditView->DrawBracketPair(true);
// 02/09/18 対括弧の強調表示 ai End		03/02/18 ai mod E

	return nScrollRowNum;

}


LayoutInt Caret::MoveCursorFastMode(
	const LogicPoint&		ptWk_CaretPosLogic	// [in] 移動先ロジック位置
)
{
	// fastMode
	SetCaretLogicPos(ptWk_CaretPosLogic);
	return LayoutInt(0);
}

/* マウス等による座標指定によるカーソル移動
|| 必要に応じて縦/横スクロールもする
|| 垂直スクロールをした場合はその行数を返す(正／負)
*/
// 2007.09.11 kobake 関数名変更: MoveCursorToPoint→MoveCursorToClientPoint
LayoutInt Caret::MoveCursorToClientPoint(const POINT& ptClientPos, bool test, LayoutPoint* pCaretPosNew)
{
	LayoutPoint	ptLayoutPos;
	m_pEditView->GetTextArea().ClientToLayout(ptClientPos, &ptLayoutPos);

	int	dx = (ptClientPos.x - m_pEditView->GetTextArea().GetAreaLeft()) % (m_pEditView->GetTextMetrics().GetHankakuDx());
	LayoutInt nScrollRowNum = MoveCursorProperly(ptLayoutPos, true, test, pCaretPosNew, 1000, dx);
	if (!test) {
		m_nCaretPosX_Prev = GetCaretLayoutPos().GetX2();
	}
	return nScrollRowNum;
}
//_CARETMARGINRATE_CARETMARGINRATE_CARETMARGINRATE


/*! 正しいカーソル位置を算出する(EOF以降のみ)
	@param pptPosXY [in/out] カーソルのレイアウト座標
	@retval	TRUE 座標を修正した
	@retval	FALSE 座標は修正されなかった
	@note	EOFの直前が改行でない場合は、その行に限りEOF以降にも移動可能
			EOFだけの行は、先頭位置のみ正しい。
	@date 2004.04.02 Moca 関数化
*/
bool Caret::GetAdjustCursorPos(
	LayoutPoint* pptPosXY
	)
{
	// 2004.03.28 Moca EOFのみのレイアウト行は、0桁目のみ有効.EOFより下の行のある場合は、EOF位置にする
	LayoutInt nLayoutLineCount = m_pEditDoc->m_layoutMgr.GetLineCount();

	LayoutPoint ptPosXY2 = *pptPosXY;
	bool ret = false;
	if (ptPosXY2.y >= nLayoutLineCount) {
		if (0 < nLayoutLineCount) {
			ptPosXY2.y = nLayoutLineCount - 1;
			const Layout* pLayout = m_pEditDoc->m_layoutMgr.SearchLineByLayoutY(ptPosXY2.GetY2());
			if (pLayout->GetLayoutEol() == EolType::None) {
				ptPosXY2.x = m_pEditView->LineIndexToColumn(pLayout, (LogicInt)pLayout->GetLengthWithEOL());
				// [EOF]のみ折り返すのはやめる	// 2009.02.17 ryoji
				// 復活するなら ptPosXY2.x に折り返し行インデントを適用するのがよい

				// EOFだけ折り返されているか
				//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
				//if (ptPosXY2.x >= m_pEditDoc->m_layoutMgr.GetMaxLineKetas()) {
				//	ptPosXY2.y++;
				//	ptPosXY2.x = LayoutInt(0);
				//}
			}else {
				// EOFだけの行
				ptPosXY2.y++;
				ptPosXY2.x = LayoutInt(0);
			}
		}else {
			// 空のファイル
			ptPosXY2.Set(LayoutInt(0), LayoutInt(0));
		}
		if (*pptPosXY != ptPosXY2) {
			*pptPosXY = ptPosXY2;
			ret = true;
		}
	}
	return ret;
}

// キャレットの表示・更新
void Caret::ShowEditCaret()
{
	if (m_pEditView->m_bMiniMap) {
		return;
	}
	// 必要なインターフェース
	const LayoutMgr* pLayoutMgr = &m_pEditDoc->m_layoutMgr;
	CommonSetting* pCommon = &GetDllShareData().m_common;
	const TypeConfig* pTypes = &m_pEditDoc->m_docType.GetDocumentAttribute();

	using namespace WCODE;

/*
	フォーカスが無いときに内部的にキャレット作成すると暗黙的にキャレット破棄（※）されても
	キャレットがある（m_nCaretWidth != 0）ということになってしまい、フォーカスを取得しても
	キャレットが出てこなくなる場合がある
	フォーカスが無いときはキャレットを作成／表示しないようにする

	※キャレットはスレッドにひとつだけなので例えばエディットボックスがフォーカス取得すれば
	　別形状のキャレットに暗黙的に差し替えられるしフォーカスを失えば暗黙的に破棄される

	2007.12.11 ryoji
	ドラッグアンドドロップ編集中はキャレットが必要で暗黙破棄の要因も無いので例外的に表示する
*/
	if (::GetFocus() != m_pEditView->GetHwnd() && !m_pEditView->m_bDragMode) {
		m_sizeCaret.cx = 0;
		return;
	}
	// 2014.07.02 GetDrawSwitchを見る
	if (!m_pEditView->GetDrawSwitch()) {
		return;
	}

	// CalcCaretDrawPosのためにCaretサイズを仮設定
	int	nCaretWidth = 0;
	int	nCaretHeight = 0;
	if (pCommon->general.GetCaretType() == 0) {
		nCaretHeight = GetHankakuHeight();
		if (m_pEditView->IsInsMode()) {
			nCaretWidth = 2;
		}else {
			nCaretWidth = GetHankakuDx();
		}
	}else if (pCommon->general.GetCaretType() == 1) {
		if (m_pEditView->IsInsMode()) {
			nCaretHeight = GetHankakuHeight() / 2;
		}else {
			nCaretHeight = GetHankakuHeight();
		}
		nCaretWidth = GetHankakuDx();
	}
	Size caretSizeOld = GetCaretSize();
	SetCaretSize(nCaretWidth, nCaretHeight);
	POINT ptDrawPos = CalcCaretDrawPos(GetCaretLayoutPos());
	SetCaretSize(caretSizeOld.cx, caretSizeOld.cy); // 後で比較するので戻す
	bool bShowCaret = false;
	auto& textArea = m_pEditView->GetTextArea();
	if (1
		&& textArea.GetAreaLeft() <= ptDrawPos.x
		&& textArea.GetAreaTop() <= ptDrawPos.y
		&& ptDrawPos.x < textArea.GetAreaRight()
		&& ptDrawPos.y < textArea.GetAreaBottom()
	) {
		// キャレットの表示
		bShowCaret = true;
	}
	// キャレットの幅、高さを決定
	// カーソルのタイプ = win
	if (pCommon->general.GetCaretType() == 0) {
		nCaretHeight = GetHankakuHeight();					// キャレットの高さ
		if (m_pEditView->IsInsMode() /* Oct. 2, 2005 genta */) {
			nCaretWidth = 2; // 2px
			// 2011.12.22 システムの設定に従う(けど2px以上)
			DWORD dwWidth;
			if (::SystemParametersInfo(SPI_GETCARETWIDTH, 0, &dwWidth, 0) && 2 < dwWidth) {
				nCaretWidth = t_min((int)dwWidth, GetHankakuDx());
			}
		}else {
			nCaretWidth = GetHankakuDx();

			const wchar_t*	pLine = NULL;
			LogicInt		nLineLen = LogicInt(0);
			const Layout*	pLayout = NULL;
			if (bShowCaret) {
				// 画面外のときはGetLineStrを呼ばない
				pLine = pLayoutMgr->GetLineStr(GetCaretLayoutPos().GetY2(), &nLineLen, &pLayout);
			}

			if (pLine) {
				// 指定された桁に対応する行のデータ内の位置を調べる
				int nIdxFrom = GetCaretLogicPos().GetX() - pLayout->GetLogicOffset();
				if (0
					|| nIdxFrom >= nLineLen
					|| WCODE::IsLineDelimiter(pLine[nIdxFrom], GetDllShareData().m_common.edit.m_bEnableExtEol)
					|| pLine[nIdxFrom] == TAB
				) {
					nCaretWidth = GetHankakuDx();
				}else {
					LayoutInt nKeta = NativeW::GetKetaOfChar(pLine, nLineLen, nIdxFrom);
					if (0 < nKeta) {
						nCaretWidth = GetHankakuDx() * (Int)nKeta;
					}
				}
			}
		}
	// カーソルのタイプ = dos
	}else if (pCommon->general.GetCaretType() == 1) {
		if (m_pEditView->IsInsMode() /* Oct. 2, 2005 genta */) {
			nCaretHeight = GetHankakuHeight() / 2;			// キャレットの高さ
		}else {
			nCaretHeight = GetHankakuHeight();				// キャレットの高さ
		}
		nCaretWidth = GetHankakuDx();

		const wchar_t*	pLine = NULL;
		LogicInt		nLineLen = LogicInt(0);
		const Layout*	pLayout = NULL;
		if (bShowCaret) {
			pLine= pLayoutMgr->GetLineStr(GetCaretLayoutPos().GetY2(), &nLineLen, &pLayout);
		}

		if (pLine) {
			// 指定された桁に対応する行のデータ内の位置を調べる
			int nIdxFrom = m_pEditView->LineColumnToIndex(pLayout, GetCaretLayoutPos().GetX2());
			if (0
				|| nIdxFrom >= nLineLen
				|| WCODE::IsLineDelimiter(pLine[nIdxFrom], GetDllShareData().m_common.edit.m_bEnableExtEol)
				|| pLine[nIdxFrom] == TAB
			) {
				nCaretWidth = GetHankakuDx();
			}else {
				LayoutInt nKeta = NativeW::GetKetaOfChar(pLine, nLineLen, nIdxFrom);
				if (0 < nKeta) {
					nCaretWidth = GetHankakuDx() * (Int)nKeta;
				}
			}
		}
	}

	//	キャレット色の取得
	const ColorInfo* ColorInfoArr = pTypes->m_colorInfoArr;
	int nCaretColor = (ColorInfoArr[COLORIDX_CARET_IME].m_bDisp && m_pEditView->IsImeON())? COLORIDX_CARET_IME: COLORIDX_CARET;
	COLORREF crCaret = ColorInfoArr[nCaretColor].m_colorAttr.m_cTEXT;
	COLORREF crBack = ColorInfoArr[COLORIDX_TEXT].m_colorAttr.m_cBACK;

	if (!ExistCaretFocus()) {
		// キャレットがなかった場合
		// キャレットの作成
		CreateEditCaret(crCaret, crBack, nCaretWidth, nCaretHeight);	// 2006.12.07 ryoji
		m_bCaretShowFlag = false; // 2002/07/22 novice
	}else {
		if (
			GetCaretSize() != Size(nCaretWidth, nCaretHeight)
			|| m_crCaret != crCaret
			|| m_pEditView->m_crBack != crBack
		) {
			// キャレットはあるが、大きさや色が変わった場合
			// 現在のキャレットを削除
			::DestroyCaret();

			// キャレットの作成
			CreateEditCaret(crCaret, crBack, nCaretWidth, nCaretHeight);	// 2006.12.07 ryoji
			m_bCaretShowFlag = false; // 2002/07/22 novice
		}else {
			// キャレットはあるし、大きさも変わっていない場合
			// キャレットを隠す
			HideCaret_(m_pEditView->GetHwnd()); // 2002/07/22 novice
		}
	}

	// キャレットサイズ
	SetCaretSize(nCaretWidth, nCaretHeight);

	// キャレットの位置を調整
	// 2007.08.26 kobake キャレットX座標の計算をUNICODE仕様にした。
	::SetCaretPos(ptDrawPos.x, ptDrawPos.y);
	if (bShowCaret) {
		// キャレットの表示
		ShowCaret_(m_pEditView->GetHwnd()); // 2002/07/22 novice
	}

	m_crCaret = crCaret;	//	2006.12.07 ryoji
	m_pEditView->m_crBack2 = crBack;		//	2006.12.07 ryoji
	m_pEditView->SetIMECompFormPos();
}


/*! キャレットの行桁位置およびステータスバーの状態表示の更新

	@note ステータスバーの状態の並び方の変更はメッセージを受信する
		CEditWnd::DispatchEvent()のWM_NOTIFYにも影響があることに注意
	
	@note ステータスバーの出力内容の変更はCEditWnd::OnSize()の
		カラム幅計算に影響があることに注意
*/
// 2007.10.17 kobake 重複するコードを整理
void Caret::ShowCaretPosInfo()
{
	// 必要なインターフェース
	const LayoutMgr* pLayoutMgr = &m_pEditDoc->m_layoutMgr;
	const TypeConfig* pTypes = &m_pEditDoc->m_docType.GetDocumentAttribute();

	if (!m_pEditView->GetDrawSwitch()) {
		return;
	}

	// ステータスバーハンドルを取得
	HWND hwndStatusBar = m_pEditDoc->m_pEditWnd->m_statusBar.GetStatusHwnd();

	// カーソル位置の文字列を取得
	const Layout*	pLayout;
	LogicInt		nLineLen;
	const wchar_t*	pLine = pLayoutMgr->GetLineStr(GetCaretLayoutPos().GetY2(), &nLineLen, &pLayout);

	// -- -- -- -- 文字コード情報 -> pszCodeName -- -- -- -- //
	const TCHAR* pszCodeName;
	NativeT memCodeName;
	if (hwndStatusBar) {
		TCHAR szCodeName[100];
		CodePage::GetNameNormal(szCodeName, m_pEditDoc->GetDocumentEncoding());
		memCodeName.AppendString(szCodeName);
		if (m_pEditDoc->GetDocumentBomExist()) {
			memCodeName.AppendString(LS(STR_CARET_WITHBOM));
		}
	}else {
		TCHAR szCodeName[100];
		CodePage::GetNameShort(szCodeName, m_pEditDoc->GetDocumentEncoding());
		memCodeName.AppendString(szCodeName);
		if (m_pEditDoc->GetDocumentBomExist()) {
			memCodeName.AppendString(_T("#"));		// BOM付(メニューバーなので小さく)	// 2013/4/17 Uchi
		}
	}
	pszCodeName = memCodeName.GetStringPtr();


	// -- -- -- -- 改行モード -> szEolMode -- -- -- -- //
	//	May 12, 2000 genta
	//	改行コードの表示を追加
	Eol cNlType = m_pEditDoc->m_docEditor.GetNewLineCode();
	const TCHAR* szEolMode = cNlType.GetName();


	// -- -- -- -- キャレット位置 -> ptCaret -- -- -- -- //
	//
	Point ptCaret;
	// 行番号をロジック単位で表示
	if (pTypes->m_bLineNumIsCRLF) {
		ptCaret.x = 0;
		ptCaret.y = (Int)GetCaretLogicPos().y;
		if (pLayout) {
			// 2014.01.10 改行のない大きい行があると遅いのでキャッシュする
			LayoutInt offset;
			if (m_nLineLogicNoCache == pLayout->GetLogicLineNo()
				&& m_nLineNoCache == GetCaretLayoutPos().GetY2()
				&& m_nLineLogicModCache == ModifyVisitor().GetLineModifiedSeq( pLayout->GetDocLineRef() )
			) {
				offset = m_nOffsetCache;
			}else if (
				m_nLineLogicNoCache == pLayout->GetLogicLineNo()
				&& m_nLineNoCache < GetCaretLayoutPos().GetY2()
				&& m_nLineLogicModCache == ModifyVisitor().GetLineModifiedSeq( pLayout->GetDocLineRef() )
			) {
				// 下移動
				offset = pLayout->CalcLayoutOffset(*pLayoutMgr, m_nLogicOffsetCache, m_nOffsetCache);
				m_nOffsetCache = offset;
				m_nLogicOffsetCache = pLayout->GetLogicOffset();
				m_nLineNoCache = GetCaretLayoutPos().GetY2();
			}else if (m_nLineLogicNoCache == pLayout->GetLogicLineNo()
				&& m_nLineNo50Cache <= GetCaretLayoutPos().GetY2()
				&& GetCaretLayoutPos().GetY2() <= m_nLineNo50Cache + 50
				&& m_nLineLogicModCache == ModifyVisitor().GetLineModifiedSeq( pLayout->GetDocLineRef() )
			) {
				// 上移動
				offset = pLayout->CalcLayoutOffset(*pLayoutMgr, m_nLogicOffset50Cache, m_nOffset50Cache);
				m_nOffsetCache = offset;
				m_nLogicOffsetCache = pLayout->GetLogicOffset();
				m_nLineNoCache = GetCaretLayoutPos().GetY2();
			}else {
			// 2013.05.11 折り返しなしとして計算する
				const Layout* pLayout50 = pLayout;
				LayoutInt nLineNum = GetCaretLayoutPos().GetY2();
				for (;;) {
					if (pLayout50->GetLogicOffset() == 0) {
						break;
					}
					if (nLineNum + 50 == GetCaretLayoutPos().GetY2()) {
						break;
					}
					pLayout50 = pLayout50->GetPrevLayout();
					--nLineNum;
				}
				m_nOffset50Cache = pLayout50->CalcLayoutOffset(*pLayoutMgr);
				m_nLogicOffset50Cache = pLayout50->GetLogicOffset();
				m_nLineNo50Cache = nLineNum;
				
				offset = pLayout->CalcLayoutOffset(*pLayoutMgr, m_nLogicOffset50Cache, m_nOffset50Cache);
				m_nOffsetCache = offset;
				m_nLogicOffsetCache = pLayout->GetLogicOffset();
				m_nLineLogicNoCache = pLayout->GetLogicLineNo();
				m_nLineNoCache = GetCaretLayoutPos().GetY2();
				m_nLineLogicModCache = ModifyVisitor().GetLineModifiedSeq( pLayout->GetDocLineRef() );
			}
			Layout cLayout(
				pLayout->GetDocLineRef(),
				pLayout->GetLogicPos(),
				pLayout->GetLengthWithEOL(),
				pLayout->GetColorTypePrev(),
				offset,
				NULL
			);
			ptCaret.x = (Int)m_pEditView->LineIndexToColumn(&cLayout, GetCaretLogicPos().x - pLayout->GetLogicPos().x);
		}
	// 行番号をレイアウト単位で表示
	}else {
		ptCaret.x = (Int)GetCaretLayoutPos().GetX();
		ptCaret.y = (Int)GetCaretLayoutPos().GetY();
	}
	// 表示値が1から始まるように補正
	ptCaret.x++;
	ptCaret.y++;


	// -- -- -- -- キャレット位置の文字情報 -> szCaretChar -- -- -- -- //
	//
	TCHAR szCaretChar[32] = _T("");
	if (pLine) {
		// 指定された桁に対応する行のデータ内の位置を調べる
		LogicInt nIdx = GetCaretLogicPos().GetX2() - pLayout->GetLogicOffset();
		if (nIdx < nLineLen) {
			if (nIdx < nLineLen - (pLayout->GetLayoutEol().GetLen() ? 1 : 0)) {
				//auto_sprintf(szCaretChar, _T("%04x"),);
				// 任意の文字コードからUnicodeへ変換する		2008/6/9 Uchi
				CodeBase* pCode = CodeFactory::CreateCodeBase(m_pEditDoc->GetDocumentEncoding(), false);
				CommonSetting_StatusBar* psStatusbar = &GetDllShareData().m_common.statusBar;
				CodeConvertResult ret = pCode->UnicodeToHex(&pLine[nIdx], nLineLen - nIdx, szCaretChar, psStatusbar);
				delete pCode;
				if (ret != CodeConvertResult::Complete) {
					// うまくコードが取れなかった(Unicodeで表示)
					pCode = CodeFactory::CreateCodeBase(CODE_UNICODE, false);
					/* CodeConvertResult ret = */ pCode->UnicodeToHex(&pLine[nIdx], nLineLen - nIdx, szCaretChar, psStatusbar);
					delete pCode;
				}
			}else {
				_tcscpy_s(szCaretChar, _countof(szCaretChar), pLayout->GetLayoutEol().GetName());
			}
		}
	}


	// -- -- -- --  ステータス情報を書き出す -- -- -- -- //
	//
	// ウィンドウ右上に書き出す
	if (!hwndStatusBar) {
		TCHAR	szText[64];
		TCHAR	szFormat[64];
		TCHAR	szLeft[64];
		TCHAR	szRight[64];
		int		nLen;
		{	// メッセージの左側文字列（「行:列」を除いた表示）
			nLen = _tcslen(pszCodeName) + _tcslen(szEolMode) + _tcslen(szCaretChar);
			// これは %s(%s)%6s%s%s 等になる。%6ts表記は使えないので注意
			auto_sprintf_s(
				szFormat,
				_T("%%s(%%s)%%%ds%%s%%s"),	// 「キャレット位置の文字情報」を右詰で配置（足りないときは左詰になって右に伸びる）
				(nLen < 15)? 15 - nLen: 1
			);
			auto_sprintf_s(
				szLeft,
				szFormat,
				pszCodeName,
				szEolMode,
				szCaretChar[0]? _T("["): _T(" "),	// 文字情報無しなら括弧も省略（EOFやフリーカーソル位置）
				szCaretChar,
				szCaretChar[0]? _T("]"): _T(" ")	// 文字情報無しなら括弧も省略（EOFやフリーカーソル位置）
			);
		}
		szRight[0] = _T('\0');
		nLen = MENUBAR_MESSAGE_MAX_LEN - _tcslen(szLeft);	// 右側に残っている文字長
		if (nLen > 0) {	// メッセージの右側文字列（「行:列」表示）
			TCHAR szRowCol[32];
			auto_sprintf_s(
				szRowCol,
				_T("%d:%-4d"),	// 「列」は最小幅を指定して左寄せ（足りないときは右に伸びる）
				ptCaret.y,
				ptCaret.x
			);
			auto_sprintf_s(
				szFormat,
				_T("%%%ds"),	// 「行:列」を右詰で配置（足りないときは左詰になって右に伸びる）
				nLen
			);
			auto_sprintf_s(
				szRight,
				szFormat,
				szRowCol
			);
		}
		auto_sprintf_s(
			szText,
			_T("%s%s"),
			szLeft,
			szRight
		);
		m_pEditDoc->m_pEditWnd->PrintMenubarMessage(szText);
	// ステータスバーに状態を書き出す
	}else {
		TCHAR	szText_1[64];
		auto_sprintf_s(szText_1, LS(STR_STATUS_ROW_COL), ptCaret.y, ptCaret.x);	// Oct. 30, 2000 JEPRO 千万行も要らん

		TCHAR	szText_6[16];
		if (m_pEditView->IsInsMode() /* Oct. 2, 2005 genta */) {
			_tcscpy_s(szText_6, LS(STR_INS_MODE_INS));	// "挿入"
		}else {
			_tcscpy_s(szText_6, LS(STR_INS_MODE_OVR));	// "上書"
		}
		if (m_bClearStatus) {
			::StatusBar_SetText(hwndStatusBar, 0 | SBT_NOBORDERS, _T(""));
		}
		::StatusBar_SetText(hwndStatusBar, 1 | 0,             szText_1);
		//	May 12, 2000 genta
		//	改行コードの表示を追加．後ろの番号を1つずつずらす
		//	From Here
		::StatusBar_SetText(hwndStatusBar, 2 | 0,             szEolMode);
		//	To Here
		::StatusBar_SetText(hwndStatusBar, 3 | 0,             szCaretChar);
		::StatusBar_SetText(hwndStatusBar, 4 | 0,             pszCodeName);
		::StatusBar_SetText(hwndStatusBar, 5 | SBT_OWNERDRAW, _T(""));
		::StatusBar_SetText(hwndStatusBar, 6 | 0,             szText_6);
	}

}

void Caret::ClearCaretPosInfoCache()
{
	m_nOffsetCache = LayoutInt(-1);
	m_nLineNoCache = LayoutInt(-1);
	m_nLogicOffsetCache = LogicInt(-1);
	m_nLineLogicNoCache = LogicInt(-1);
	m_nLineNo50Cache = LayoutInt(-1);
	m_nOffset50Cache = LayoutInt(-1);
	m_nLogicOffset50Cache = LogicInt(-1);
	m_nLineLogicModCache = -1;
}

/* カーソル上下移動処理 */
LayoutInt Caret::Cursor_UPDOWN(LayoutInt nMoveLines, bool bSelect)
{
	// 必要なインターフェース
	const LayoutMgr* const pLayoutMgr = &m_pEditDoc->m_layoutMgr;
	const CommonSetting* const pCommon = &GetDllShareData().m_common;

	const LayoutPoint ptCaret = GetCaretLayoutPos();

	bool	bVertLineDoNotOFF = true;	// カーソル位置縦線を消去しない
	if (bSelect) {
		bVertLineDoNotOFF = false;		// 選択状態ならカーソル位置縦線消去を行う
	}

	auto& selInfo = m_pEditView->GetSelectionInfo();

	// 現在のキャレットY座標 + nMoveLinesが正しいレイアウト行の範囲内に収まるように nMoveLinesを調整する。
	if (nMoveLines > 0) { // 下移動。
		const bool existsEOFOnlyLine = pLayoutMgr->GetBottomLayout() && pLayoutMgr->GetBottomLayout()->GetLayoutEol() != EolType::None
			|| pLayoutMgr->GetLineCount() == 0;
		const LayoutInt maxLayoutLine = pLayoutMgr->GetLineCount() + (existsEOFOnlyLine ? 1 : 0) - 1;
		// 移動先が EOFのみの行を含めたレイアウト行数未満になるように移動量を規正する。
		nMoveLines = t_min(nMoveLines,  maxLayoutLine - ptCaret.y);
		if (1
			&& ptCaret.y + nMoveLines == maxLayoutLine
			&& existsEOFOnlyLine // 移動先が EOFのみの行
			&& selInfo.IsBoxSelecting()
			&& ptCaret.x != 0 // かつ矩形選択中なら、
		) {
			// EOFのみの行には移動しない。下移動でキャレットの X座標を動かしたくないので。
			nMoveLines = t_max(LayoutInt(0), nMoveLines - 1); // うっかり上移動しないように 0以上を守る。
		}
	}else { // 上移動。
		// 移動先が 0行目より小さくならないように移動量を規制。
		nMoveLines = t_max(nMoveLines, - GetCaretLayoutPos().GetY());
	}

	if (bSelect && ! selInfo.IsTextSelected()) {
		// 現在のカーソル位置から選択を開始する
		selInfo.BeginSelectArea();
	}
	if (!bSelect) {
		if (selInfo.IsTextSelected()) {
			// 現在の選択範囲を非選択状態に戻す
			selInfo.DisableSelectArea(true);
		}else if (selInfo.IsBoxSelecting()) {
			selInfo.SetBoxSelect(false);
		}
	}

	// (これから求める)キャレットの移動先。
	LayoutPoint ptTo(LayoutInt(0), ptCaret.y + nMoveLines);

	// 移動先の行のデータを取得
	const Layout* const pLayout = pLayoutMgr->SearchLineByLayoutY(ptTo.y);
	const LogicInt nLineLen = pLayout ? pLayout->GetLengthWithEOL() : LogicInt(0);
	int i = 0; ///< 何？
	if (pLayout) {
		MemoryIterator it(pLayout, pLayoutMgr->GetTabSpace());
		while (!it.end()) {
			it.scanNext();
			if (it.getIndex() + it.getIndexDelta() > pLayout->GetLengthWithoutEOL()) {
				i = nLineLen;
				break;
			}
			if (it.getColumn() + it.getColumnDelta() > m_nCaretPosX_Prev) {
				i = it.getIndex();
				break;
			}
			it.addDelta();
		}
		ptTo.x += it.getColumn();
		if (it.end()) {
			i = it.getIndex();
		}
	}
	if (i >= nLineLen) {
		// フリーカーソルモードと矩形選択中は、キャレットの位置を改行や EOFの前に制限しない
		if (pCommon->general.m_bIsFreeCursorMode
			|| selInfo.IsBoxSelecting()
		) {
			ptTo.x = m_nCaretPosX_Prev;
		}
	}
	if (ptTo.x != GetCaretLayoutPos().GetX()) {
		bVertLineDoNotOFF = false;
	}
	GetAdjustCursorPos(&ptTo);
	if (bSelect) {
		// 現在のカーソル位置によって選択範囲を変更
		selInfo.ChangeSelectAreaByCurrentCursor(ptTo);
	}
	const LayoutInt nScrollLines = MoveCursor(	ptTo,
								m_pEditView->GetDrawSwitch() /* TRUE */,
								_CARETMARGINRATE,
								false,
								bVertLineDoNotOFF);
	return nScrollLines;
}


/*!	キャレットの作成

	@param nCaretColor [in]	キャレットの色種別 (0:通常, 1:IME ON)
	@param nWidth [in]		キャレット幅
	@param nHeight [in]		キャレット高

	@date 2006.12.07 ryoji 新規作成
*/
void Caret::CreateEditCaret(COLORREF crCaret, COLORREF crBack, int nWidth, int nHeight)
{
	//
	// キャレット用のビットマップを作成する
	//
	// Note: ウィンドウ互換のメモリ DC 上で PatBlt を用いてキャレット色と背景色を XOR 結合
	//       することで，目的のビットマップを得る．
	//       ※ 256 色環境では RGB 値を単純に直接演算してもキャレット色を出すための正しい
	//          ビットマップ色は得られない．
	//       参考: [HOWTO] キャレットの色を制御する方法
	//             http://support.microsoft.com/kb/84054/ja
	//

	HBITMAP hbmpCaret;	// キャレット用のビットマップ

	HDC hdc = m_pEditView->GetDC();

	hbmpCaret = ::CreateCompatibleBitmap(hdc, nWidth, nHeight);
	HDC hdcMem = ::CreateCompatibleDC(hdc);
	HBITMAP hbmpOld = (HBITMAP)::SelectObject(hdcMem, hbmpCaret);
	HBRUSH hbrCaret = ::CreateSolidBrush(crCaret);
	HBRUSH hbrBack = ::CreateSolidBrush(crBack);
	HBRUSH hbrOld = (HBRUSH)::SelectObject(hdcMem, hbrCaret);
	::PatBlt(hdcMem, 0, 0, nWidth, nHeight, PATCOPY);
	::SelectObject(hdcMem, hbrBack);
	::PatBlt(hdcMem, 0, 0, nWidth, nHeight, PATINVERT);
	::SelectObject(hdcMem, hbrOld);
	::SelectObject(hdcMem, hbmpOld);
	::DeleteObject(hbrCaret);
	::DeleteObject(hbrBack);
	::DeleteDC(hdcMem);

	m_pEditView->ReleaseDC(hdc);

	// 以前のビットマップを破棄する
	if (m_hbmpCaret)
		::DeleteObject(m_hbmpCaret);
	m_hbmpCaret = hbmpCaret;

	// キャレットを作成する
	m_pEditView->CreateCaret(hbmpCaret, nWidth, nHeight);
	return;
}


// 2002/07/22 novice
/*!
	キャレットの表示
*/
void Caret::ShowCaret_(HWND hwnd)
{
	if (!m_bCaretShowFlag) {
		::ShowCaret(hwnd);
		m_bCaretShowFlag = true;
	}
}


/*!
	キャレットの非表示
*/
void Caret::HideCaret_(HWND hwnd)
{
	if (m_bCaretShowFlag) {
		::HideCaret(hwnd);
		m_bCaretShowFlag = false;
	}
}

// 自分の状態を他のCaretにコピー
void Caret::CopyCaretStatus(Caret* pCaret) const
{
	pCaret->SetCaretLayoutPos(GetCaretLayoutPos());
	pCaret->SetCaretLogicPos(GetCaretLogicPos());
	pCaret->m_nCaretPosX_Prev = m_nCaretPosX_Prev;	// ビュー左端からのカーソル桁位置（０オリジン

	//※ キャレットのサイズはコピーしない。2002/05/12 YAZAKI
}


POINT Caret::CalcCaretDrawPos(const LayoutPoint& ptCaretPos) const
{
	auto& textArea = m_pEditView->GetTextArea();
	int nPosX = textArea.GetAreaLeft()
		+ (Int)(ptCaretPos.x - textArea.GetViewLeftCol()) * GetHankakuDx();

	LayoutYInt nY = ptCaretPos.y - textArea.GetViewTopLine();
	int nPosY;
	if (nY < 0) {
		nPosY = -1;
	}else if (textArea.m_nViewRowNum < nY) {
		nPosY = textArea.GetAreaBottom() + 1;
	}else {
		nPosY = textArea.GetAreaTop()
			+ (Int)(nY) * m_pEditView->GetTextMetrics().GetHankakuDy()
		+ m_pEditView->GetTextMetrics().GetHankakuHeight() - GetCaretSize().cy; // 下寄せ
	}

	return Point(nPosX, nPosY);
}


/*!
	行桁指定によるカーソル移動（座標調整付き）

	@return 縦スクロール行数(負:上スクロール/正:下スクロール)

	@note マウス等による移動で不適切な位置に行かないよう座標調整してカーソル移動する

	@date 2007.08.23 ryoji 関数化（MoveCursorToPoint()から処理を抜き出し）
	@date 2007.09.26 ryoji 半角文字でも中央で左右にカーソルを振り分ける
	@date 2007.10.23 kobake 引数説明の誤りを修正 ([in/out]→[in])
	@date 2009.02.17 ryoji レイアウト行末以後のカラム位置指定なら末尾文字の前ではなく末尾文字の後に移動する
*/
LayoutInt Caret::MoveCursorProperly(
	LayoutPoint	ptNewXY,			// [in] カーソルのレイアウト座標X
	bool			bScroll,			// [in] true: 画面位置調整有り/ false: 画面位置調整有り無し
	bool			test,				// [in] true: カーソル移動はしない
	LayoutPoint*	ptNewXYNew,			// [out] 新しいレイアウト座標
	int				nCaretMarginRate,	// [in] 縦スクロール開始位置を決める値
	int				dx					// [in] ptNewXY.xとマウスカーソル位置との誤差(カラム幅未満のドット数)
)
{
	LogicInt		nLineLen;
	const Layout*	pLayout;

	if (0 > ptNewXY.y) {
		ptNewXY.y = LayoutInt(0);
	}
	
	// 2011.12.26 EOF以下の行だった場合で矩形のときは、最終レイアウト行へ移動する
	auto& layoutMgr = m_pEditDoc->m_layoutMgr;
	auto& selectionInfo = m_pEditView->GetSelectionInfo();
	if (1
		&& ptNewXY.y >= layoutMgr.GetLineCount()
		&& (selectionInfo.IsMouseSelecting() && selectionInfo.IsBoxSelecting())
	) {
		const Layout* layoutEnd = layoutMgr.GetBottomLayout();
		bool bEofOnly = (layoutEnd && layoutEnd->GetLayoutEol() != EolType::None) || !layoutEnd;
	 	// 2012.01.09 ぴったり[EOF]位置にある場合は位置を維持(1つ上の行にしない)
	 	if (1
	 		&& bEofOnly
	 		&& ptNewXY.y == layoutMgr.GetLineCount()
	 		&& ptNewXY.x == 0
	 	) {
	 	}else {
			ptNewXY.y = t_max(LayoutInt(0), layoutMgr.GetLineCount() - 1);
		}
	}
	// カーソルがテキスト最下端行にあるか
	if (ptNewXY.y >= layoutMgr.GetLineCount()) {
		// 2004.04.03 Moca EOFより後ろの座標調整は、MoveCursor内でやってもらうので、削除
	// カーソルがテキスト最上端行にあるか
	}else if (ptNewXY.y < 0) {
		ptNewXY.Set(LayoutInt(0), LayoutInt(0));
	}else {
		// 移動先の行のデータを取得
		layoutMgr.GetLineStr(ptNewXY.GetY2(), &nLineLen, &pLayout);

		int nColWidth = m_pEditView->GetTextMetrics().GetHankakuDx();
		LayoutInt nPosX = LayoutInt(0);
		int i = 0;
		MemoryIterator it(pLayout, layoutMgr.GetTabSpace());
		while (!it.end()) {
			it.scanNext();
			if (it.getIndex() + it.getIndexDelta() > LogicInt(pLayout->GetLengthWithoutEOL())) {
				i = nLineLen;
				break;
			}
			if (it.getColumn() + it.getColumnDelta() > ptNewXY.GetX2()) {
				if (1
					&& ptNewXY.GetX2() >= (pLayout ? pLayout->GetIndent() : LayoutInt(0))
					&& ((ptNewXY.GetX2() - it.getColumn()) * nColWidth + dx) * 2 >= it.getColumnDelta() * nColWidth
				) {
				//if (ptNewXY.GetX2() >= (pLayout ? pLayout->GetIndent() : LayoutInt(0)) && (it.getColumnDelta() > LayoutInt(1)) && ((it.getColumn() + it.getColumnDelta() - ptNewXY.GetX2()) <= it.getColumnDelta() / 2)) {
					nPosX += it.getColumnDelta();
				}
				i = it.getIndex();
				break;
			}
			it.addDelta();
		}
		nPosX += it.getColumn();
		if (it.end()) {
			i = it.getIndex();
			//nPosX -= it.getColumnDelta();	// 2009.02.17 ryoji コメントアウト（末尾文字の後に移動する）
		}

		if (i >= nLineLen) {
			// 2011.12.26 フリーカーソル/矩形でデータ付きEOFの右側へ移動できるように
			// フリーカーソルモードか
			if (0
				|| GetDllShareData().m_common.general.m_bIsFreeCursorMode
				|| (selectionInfo.IsMouseSelecting() && selectionInfo.IsBoxSelecting())	/* マウス範囲選択中 && 矩形範囲選択中 */
				|| (m_pEditView->m_bDragMode && m_pEditView->m_bDragBoxData) /* OLE DropTarget && 矩形データ */
			) {
				// 折り返し幅とレイアウト行桁数（ぶら下げを含む）のどちらか大きいほうまでカーソル移動可能
				//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
				LayoutInt nMaxX = t_max(nPosX, layoutMgr.GetMaxLineKetas());
				nPosX = ptNewXY.GetX2();
				if (nPosX < LayoutInt(0)) {
					nPosX = LayoutInt(0);
				}else if (nPosX > nMaxX) {
					nPosX = nMaxX;
				}
			}
		}
		ptNewXY.SetX(nPosX);
	}
	
	if (ptNewXYNew) {
		*ptNewXYNew = ptNewXY;
		GetAdjustCursorPos(ptNewXYNew);
	}
	if (test) {
		return LayoutInt(0);
	}
	return MoveCursor(ptNewXY, bScroll, nCaretMarginRate);
}

