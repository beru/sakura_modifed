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
	return editView.GetTextMetrics().GetHankakuDx();
}

inline int Caret::GetHankakuHeight() const
{
	return editView.GetTextMetrics().GetHankakuHeight();
}

inline int Caret::GetHankakuDy() const
{
	return editView.GetTextMetrics().GetHankakuDy();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      CaretUnderLine                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// カーソル行アンダーラインのON
void CaretUnderLine::CaretUnderLineON(bool bDraw, bool bPaintDraw)
{
	if (nLockCounter) return;	//	ロックされていたら何もできない。
	editView.CaretUnderLineON(bDraw, bPaintDraw, nUnderLineLockCounter != 0);
}

// カーソル行アンダーラインのOFF
void CaretUnderLine::CaretUnderLineOFF(bool bDraw, bool bDrawPaint, bool bResetFlag)
{
	if (nLockCounter) return;	//	ロックされていたら何もできない。
	editView.CaretUnderLineOFF(bDraw, bDrawPaint, bResetFlag, nUnderLineLockCounter != 0);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

Caret::Caret(EditView& editView, const EditDoc& editDoc)
	:
	editView(editView),
	editDoc(editDoc),
	ptCaretPos_Layout(0, 0),
	ptCaretPos_Logic(0, 0),			// カーソル位置 (改行単位行先頭からのバイト数(0開始), 改行単位行の行番号(0開始))
	sizeCaret(0, 0),					// キャレットのサイズ
	underLine(editView)
{
	nCaretPosX_Prev = 0;	// ビュー左端からのカーソル桁直前の位置(０オリジン)

	crCaret = -1;				// キャレットの色
	hbmpCaret = NULL;			// キャレット用ビットマップ
	bClearStatus = true;
	ClearCaretPosInfoCache();
}

Caret::~Caret()
{
	// キャレット用ビットマップ
	if (hbmpCaret) {
		DeleteObject(hbmpCaret);
	}
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
*/
int Caret::MoveCursor(
	Point	ptWk_CaretPos,		// [in] 移動先レイアウト位置
	bool	bScroll,			// [in] true: 画面位置調整有り  false: 画面位置調整無し
	int		nCaretMarginRate,	// [in] 縦スクロール開始位置を決める値
	bool	bUnderLineDoNotOFF,	// [in] アンダーラインを消去しない
	bool	bVertLineDoNotOFF	// [in] カーソル位置縦線を消去しない
	)
{
	// スクロール処理
	int nScrollRowNum = 0;
	int nScrollColNum = 0;
	int nCaretMarginY;
	int	nScrollMarginRight;
	int	nScrollMarginLeft;

	auto& textArea = editView.GetTextArea();
	if (0 >= textArea.nViewColNum) {
		return 0;
	}

	if (editView.GetSelectionInfo().IsMouseSelecting()) {	// 範囲選択中
		nCaretMarginY = 0;
	}else {
		nCaretMarginY = textArea.nViewRowNum / nCaretMarginRate;
		if (1 > nCaretMarginY) {
			nCaretMarginY = 1;
		}
	}
	GetAdjustCursorPos(&ptWk_CaretPos);
	// カーソル位置。ロジック単位。
	ptCaretPos_Logic = editDoc.layoutMgr.LayoutToLogic(ptWk_CaretPos);
	// キャレット移動
	SetCaretLayoutPos(ptWk_CaretPos);

	// カーソル行アンダーラインのOFF
	bool bDrawPaint = ptWk_CaretPos.y != editView.nOldUnderLineYBg;
	underLine.SetUnderLineDoNotOFF(bUnderLineDoNotOFF);
	underLine.SetVertLineDoNotOFF(bVertLineDoNotOFF);
	underLine.CaretUnderLineOFF(bScroll, bDrawPaint);
	underLine.SetUnderLineDoNotOFF(false);
	underLine.SetVertLineDoNotOFF(false);
	
	// 水平スクロール量（文字数）の算出
	nScrollColNum = 0;
	nScrollMarginRight = SCROLLMARGIN_RIGHT;
	nScrollMarginLeft = SCROLLMARGIN_LEFT;

	// 幅が狭い場合のマージンの調整
	{
		// カーソルが真ん中にあるときに左右にぶれないように
		int nNoMove = SCROLLMARGIN_NOMOVE;
		int a = (textArea.nViewColNum - nNoMove) / 2;
		int nMin = (2 <= a ? a : 0); // 1だと全角移動に支障があるので2以上
		nScrollMarginRight = t_min(nScrollMarginRight, nMin);
		nScrollMarginLeft  = t_min(nScrollMarginLeft,  nMin);
	}
	
	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	if ((int)editDoc.layoutMgr.GetMaxLineKetas() > textArea.nViewColNum
		&& ptWk_CaretPos.GetX() > textArea.GetViewLeftCol() + textArea.nViewColNum - nScrollMarginRight
	) {
		nScrollColNum = (textArea.GetViewLeftCol() + textArea.nViewColNum - nScrollMarginRight) - ptWk_CaretPos.x;
	}else if (1
		&& 0 < textArea.GetViewLeftCol()
		&& ptWk_CaretPos.GetX() < textArea.GetViewLeftCol() + nScrollMarginLeft
	) {
		nScrollColNum = textArea.GetViewLeftCol() + nScrollMarginLeft - ptWk_CaretPos.x;
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

	// 垂直スクロール量（行数）の算出
	// 画面が３行以下
	if (textArea.nViewRowNum <= 3) {
		// 移動先は、画面のスクロールラインより上か？（up キー）
		if (ptWk_CaretPos.y - textArea.GetViewTopLine() < nCaretMarginY) {
			if (ptWk_CaretPos.y < nCaretMarginY) {	// １行目に移動
				nScrollRowNum = textArea.GetViewTopLine();
			}else if (textArea.nViewRowNum <= 1) {	// 画面が１行
				nScrollRowNum = textArea.GetViewTopLine() - ptWk_CaretPos.y;
			}
#if !(0)	// COMMENTにすると、上下の空きを死守しない為、縦移動はgoodだが、横移動の場合上下にぶれる
			else if (textArea.nViewRowNum <= 2) {	// 画面が２行
				nScrollRowNum = textArea.GetViewTopLine() - ptWk_CaretPos.y;
			}
#endif
			else {						// 画面が３行
				nScrollRowNum = textArea.GetViewTopLine() - ptWk_CaretPos.y + 1;
			}
		// 移動先は、画面の最大行数－２より下か？（down キー）
		}else if (ptWk_CaretPos.y - textArea.GetViewTopLine() >= (textArea.nViewRowNum - nCaretMarginY - 2)) {
			int ii = (int)editDoc.layoutMgr.GetLineCount();
			if (1
				&& ii - ptWk_CaretPos.y < nCaretMarginY + 1
				&& ii - textArea.GetViewTopLine() < textArea.nViewRowNum
			) {
			}else if (textArea.nViewRowNum <= 2) {	// 画面が２行、１行
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
			nScrollRowNum = -(ptWk_CaretPos.y - (int)textArea.GetViewTopLine()) + nCaretMarginY;
		}
	// 移動先は、画面の最大行数－２より下か？（down キー）
	}else if (ptWk_CaretPos.y - textArea.GetViewTopLine() >= textArea.nViewRowNum - nCaretMarginY - 2) {
		int ii = (int)editDoc.layoutMgr.GetLineCount();
		if (1
			&& ii - ptWk_CaretPos.y < nCaretMarginY + 1
			&& ii - textArea.GetViewTopLine() < textArea.nViewRowNum
		) {
		}else {
			nScrollRowNum = -(ptWk_CaretPos.y - (int)textArea.GetViewTopLine()) + ((int)textArea.nViewRowNum - nCaretMarginY - 2);
		}
	}
	if (bScroll) {
		// スクロール
		if (0
			|| t_abs(nScrollColNum) >= textArea.nViewColNum
			|| t_abs(nScrollRowNum) >= textArea.nViewRowNum
		) {
			textArea.OffsetViewTopLine(-nScrollRowNum);
			if (editView.GetDrawSwitch()) {
				editView.InvalidateRect(NULL);
				if (editView.editWnd.GetMiniMap().GetHwnd()) {
					editView.MiniMapRedraw(true);
				}
			}
		}else if (nScrollRowNum != 0 || nScrollColNum != 0) {
			RECT	rcClip;
			RECT	rcClip2;
			RECT	rcScroll;

			textArea.GenerateTextAreaRect(&rcScroll);
			if (nScrollRowNum > 0) {
				rcScroll.bottom = textArea.GetAreaBottom() - nScrollRowNum * editView.GetTextMetrics().GetHankakuDy();
				textArea.OffsetViewTopLine(-nScrollRowNum);
				textArea.GenerateTopRect(&rcClip, nScrollRowNum);
			}else if (nScrollRowNum < 0) {
				rcScroll.top = textArea.GetAreaTop() - nScrollRowNum * editView.GetTextMetrics().GetHankakuDy();
				textArea.OffsetViewTopLine(-nScrollRowNum);
				textArea.GenerateBottomRect(&rcClip, -nScrollRowNum);
			}

			if (nScrollColNum > 0) {
				rcScroll.left = textArea.GetAreaLeft();
				rcScroll.right = textArea.GetAreaRight() - nScrollColNum * GetHankakuDx();
				textArea.GenerateLeftRect(&rcClip2, nScrollColNum);
			}else if (nScrollColNum < 0) {
				rcScroll.left = textArea.GetAreaLeft() - nScrollColNum * GetHankakuDx();
				textArea.GenerateRightRect(&rcClip2, -nScrollColNum);
			}

			if (editView.GetDrawSwitch()) {
				editView.ScrollDraw(nScrollRowNum, nScrollColNum, rcScroll, rcClip, rcClip2);
				if (editView.editWnd.GetMiniMap().GetHwnd()) {
					editView.MiniMapRedraw(false);
				}
			}
		}

		// スクロールバーの状態を更新する
		editView.AdjustScrollBars();
	}

	// 横スクロールが発生したら、ルーラー全体を再描画
	if (nScrollColNum != 0) {
		// 次回DispRuler呼び出し時に再描画。（bDraw=falseのケースを考慮した。）
		editView.GetRuler().SetRedrawFlag();
	}

	// カーソル行アンダーラインのON
	if (bScroll) {
		// キャレットの表示・更新
		ShowEditCaret();

		// ルーラの再描画
		HDC		hdc = editView.GetDC();
		editView.GetRuler().DispRuler(hdc);
		editView.ReleaseDC(hdc);

		// アンダーラインの再描画
		underLine.CaretUnderLineON(true, bDrawPaint);

		// キャレットの行桁位置を表示する
		ShowCaretPosInfo();

		//	Sep. 11, 2004 genta 同期スクロールの関数化
		//	bScroll == FALSEの時にはスクロールしないので，実行しない
		editView.SyncScrollV(-nScrollRowNum);	//	方向が逆なので符号反転が必要
		editView.SyncScrollH(-nScrollColNum);	//	方向が逆なので符号反転が必要

	}

	editView.DrawBracketPair(false);
	editView.SetBracketPairPos(true);
	editView.DrawBracketPair(true);

	return nScrollRowNum;
}


int Caret::MoveCursorFastMode(
	const Point& ptWk_CaretPosLogic	// [in] 移動先ロジック位置
	)
{
	// fastMode
	SetCaretLogicPos(ptWk_CaretPosLogic);
	return 0;
}

/* マウス等による座標指定によるカーソル移動
|| 必要に応じて縦/横スクロールもする
|| 垂直スクロールをした場合はその行数を返す(正／負)
*/
int Caret::MoveCursorToClientPoint(
	const POINT& ptClientPos,
	bool test,
	Point* pCaretPosNew
	)
{
	Point	ptLayoutPos;
	editView.GetTextArea().ClientToLayout(ptClientPos, &ptLayoutPos);

	int	dx = (ptClientPos.x - editView.GetTextArea().GetAreaLeft()) % (editView.GetTextMetrics().GetHankakuDx());
	int nScrollRowNum = MoveCursorProperly(ptLayoutPos, true, test, pCaretPosNew, 1000, dx);
	if (!test) {
		nCaretPosX_Prev = GetCaretLayoutPos().x;
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
*/
bool Caret::GetAdjustCursorPos(
	Point* pptPosXY
	)
{
	// EOFのみのレイアウト行は、0桁目のみ有効.EOFより下の行のある場合は、EOF位置にする
	size_t nLayoutLineCount = editDoc.layoutMgr.GetLineCount();

	Point ptPosXY2 = *pptPosXY;
	bool ret = false;
	if (ptPosXY2.y >= (int)nLayoutLineCount) {
		if (0 < nLayoutLineCount) {
			ptPosXY2.y = (int)nLayoutLineCount - 1;
			const Layout* pLayout = editDoc.layoutMgr.SearchLineByLayoutY(ptPosXY2.y);
			if (pLayout->GetLayoutEol() == EolType::None) {
				ptPosXY2.x = (int)editView.LineIndexToColumn(pLayout, pLayout->GetLengthWithEOL());
			}else {
				// EOFだけの行
				ptPosXY2.y++;
				ptPosXY2.x = 0;
			}
		}else {
			// 空のファイル
			ptPosXY2.Set(0, 0);
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
	if (editView.bMiniMap) {
		return;
	}
	// 必要なインターフェース
	auto& layoutMgr = editDoc.layoutMgr;
	auto& csGeneral = GetDllShareData().common.general;
	const TypeConfig* pTypes = &editDoc.docType.GetDocumentAttribute();

	using namespace WCODE;

/*
	フォーカスが無いときに内部的にキャレット作成すると暗黙的にキャレット破棄（※）されても
	キャレットがある（nCaretWidth != 0）ということになってしまい、フォーカスを取得しても
	キャレットが出てこなくなる場合がある
	フォーカスが無いときはキャレットを作成／表示しないようにする

	※キャレットはスレッドにひとつだけなので例えばエディットボックスがフォーカス取得すれば
	　別形状のキャレットに暗黙的に差し替えられるしフォーカスを失えば暗黙的に破棄される

	ドラッグアンドドロップ編集中はキャレットが必要で暗黙破棄の要因も無いので例外的に表示する
*/
	if (::GetFocus() != editView.GetHwnd() && !editView.bDragMode) {
		sizeCaret.cx = 0;
		return;
	}
	// 2014.07.02 GetDrawSwitchを見る
	if (!editView.GetDrawSwitch()) {
		return;
	}

	// CalcCaretDrawPosのためにCaretサイズを仮設定
	int	nCaretWidth = 0;
	int	nCaretHeight = 0;
	if (csGeneral.GetCaretType() == 0) {
		nCaretHeight = GetHankakuHeight();
		if (editView.IsInsMode()) {
			nCaretWidth = 2;
		}else {
			nCaretWidth = GetHankakuDx();
		}
	}else if (csGeneral.GetCaretType() == 1) {
		if (editView.IsInsMode()) {
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
	auto& textArea = editView.GetTextArea();
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
	if (csGeneral.GetCaretType() == 0) {
		nCaretHeight = GetHankakuHeight();					// キャレットの高さ
		if (editView.IsInsMode() /* Oct. 2, 2005 genta */) {
			nCaretWidth = 2; // 2px
			// 2011.12.22 システムの設定に従う(けど2px以上)
			DWORD dwWidth;
			if (::SystemParametersInfo(SPI_GETCARETWIDTH, 0, &dwWidth, 0) && 2 < dwWidth) {
				nCaretWidth = t_min((int)dwWidth, GetHankakuDx());
			}
		}else {
			nCaretWidth = GetHankakuDx();

			const wchar_t*	pLine = NULL;
			size_t			nLineLen = 0;
			const Layout*	pLayout = nullptr;
			if (bShowCaret) {
				// 画面外のときはGetLineStrを呼ばない
				pLine = layoutMgr.GetLineStr(GetCaretLayoutPos().y, &nLineLen, &pLayout);
			}

			if (pLine) {
				// 指定された桁に対応する行のデータ内の位置を調べる
				int nIdxFrom = GetCaretLogicPos().GetX() - pLayout->GetLogicOffset();
				if (0
					|| nIdxFrom >= (int)nLineLen
					|| WCODE::IsLineDelimiter(pLine[nIdxFrom], GetDllShareData().common.edit.bEnableExtEol)
					|| pLine[nIdxFrom] == TAB
				) {
					nCaretWidth = GetHankakuDx();
				}else {
					size_t nKeta = NativeW::GetKetaOfChar(pLine, nLineLen, nIdxFrom);
					if (0 < nKeta) {
						nCaretWidth = GetHankakuDx() * (int)nKeta;
					}
				}
			}
		}
	// カーソルのタイプ = dos
	}else if (csGeneral.GetCaretType() == 1) {
		if (editView.IsInsMode() /* Oct. 2, 2005 genta */) {
			nCaretHeight = GetHankakuHeight() / 2;			// キャレットの高さ
		}else {
			nCaretHeight = GetHankakuHeight();				// キャレットの高さ
		}
		nCaretWidth = GetHankakuDx();

		const wchar_t*	pLine = NULL;
		size_t			nLineLen = 0;
		const Layout*	pLayout = nullptr;
		if (bShowCaret) {
			pLine= layoutMgr.GetLineStr(GetCaretLayoutPos().y, &nLineLen, &pLayout);
		}

		if (pLine) {
			// 指定された桁に対応する行のデータ内の位置を調べる
			size_t nIdxFrom = editView.LineColumnToIndex(pLayout, GetCaretLayoutPos().x);
			if (0
				|| nIdxFrom >= nLineLen
				|| WCODE::IsLineDelimiter(pLine[nIdxFrom], GetDllShareData().common.edit.bEnableExtEol)
				|| pLine[nIdxFrom] == TAB
			) {
				nCaretWidth = GetHankakuDx();
			}else {
				size_t nKeta = NativeW::GetKetaOfChar(pLine, nLineLen, nIdxFrom);
				if (0 < nKeta) {
					nCaretWidth = GetHankakuDx() * (int)nKeta;
				}
			}
		}
	}

	//	キャレット色の取得
	const ColorInfo* colorInfoArr = pTypes->colorInfoArr;
	int nCaretColor = (colorInfoArr[COLORIDX_CARET_IME].bDisp && editView.IsImeON())? COLORIDX_CARET_IME: COLORIDX_CARET;
	COLORREF crCaret = colorInfoArr[nCaretColor].colorAttr.cTEXT;
	COLORREF crBack = colorInfoArr[COLORIDX_TEXT].colorAttr.cBACK;

	if (!ExistCaretFocus()) {
		// キャレットがなかった場合
		// キャレットの作成
		CreateEditCaret(crCaret, crBack, nCaretWidth, nCaretHeight);
		bCaretShowFlag = false;
	}else {
		if (
			GetCaretSize() != Size(nCaretWidth, nCaretHeight)
			|| this->crCaret != crCaret
			|| editView.crBack != crBack
		) {
			// キャレットはあるが、大きさや色が変わった場合
			// 現在のキャレットを削除
			::DestroyCaret();

			// キャレットの作成
			CreateEditCaret(crCaret, crBack, nCaretWidth, nCaretHeight);
			bCaretShowFlag = false;
		}else {
			// キャレットはあるし、大きさも変わっていない場合
			// キャレットを隠す
			HideCaret_(editView.GetHwnd());
		}
	}

	// キャレットサイズ
	SetCaretSize(nCaretWidth, nCaretHeight);

	// キャレットの位置を調整
	::SetCaretPos(ptDrawPos.x, ptDrawPos.y);
	if (bShowCaret) {
		// キャレットの表示
		ShowCaret_(editView.GetHwnd());
	}

	this->crCaret = crCaret;
	editView.crBack2 = crBack;
	editView.SetIMECompFormPos();
}


/*! キャレットの行桁位置およびステータスバーの状態表示の更新

	@note ステータスバーの状態の並び方の変更はメッセージを受信する
		CEditWnd::DispatchEvent()のWM_NOTIFYにも影響があることに注意
	
	@note ステータスバーの出力内容の変更はCEditWnd::OnSize()の
		カラム幅計算に影響があることに注意
*/
void Caret::ShowCaretPosInfo()
{
	// 必要なインターフェース
	auto& layoutMgr = editDoc.layoutMgr;
	const TypeConfig* pTypes = &editDoc.docType.GetDocumentAttribute();

	if (!editView.GetDrawSwitch()) {
		return;
	}

	// ステータスバーハンドルを取得
	HWND hwndStatusBar = editDoc.pEditWnd->statusBar.GetStatusHwnd();

	// カーソル位置の文字列を取得
	const Layout* pLayout;
	size_t nLineLen;
	const wchar_t*	pLine = layoutMgr.GetLineStr(GetCaretLayoutPos().y, &nLineLen, &pLayout);

	// -- -- -- -- 文字コード情報 -> pszCodeName -- -- -- -- //
	const TCHAR* pszCodeName;
	NativeT memCodeName;
	if (hwndStatusBar) {
		TCHAR szCodeName[100];
		CodePage::GetNameNormal(szCodeName, editDoc.GetDocumentEncoding());
		memCodeName.AppendString(szCodeName);
		if (editDoc.GetDocumentBomExist()) {
			memCodeName.AppendString(LS(STR_CARET_WITHBOM));
		}
	}else {
		TCHAR szCodeName[100];
		CodePage::GetNameShort(szCodeName, editDoc.GetDocumentEncoding());
		memCodeName.AppendString(szCodeName);
		if (editDoc.GetDocumentBomExist()) {
			memCodeName.AppendStringLiteral(_T("#"));		// BOM付(メニューバーなので小さく)	// 2013/4/17 Uchi
		}
	}
	pszCodeName = memCodeName.GetStringPtr();


	// -- -- -- -- 改行モード -> szEolMode -- -- -- -- //
	//	May 12, 2000 genta
	//	改行コードの表示を追加
	Eol cNlType = editDoc.docEditor.GetNewLineCode();
	const TCHAR* szEolMode = cNlType.GetName();

	// -- -- -- -- キャレット位置 -> ptCaret -- -- -- -- //
	//
	Point ptCaret;
	// 行番号をロジック単位で表示
	if (pTypes->bLineNumIsCRLF) {
		ptCaret.x = 0;
		ptCaret.y = GetCaretLogicPos().y;
		if (pLayout) {
			// 2014.01.10 改行のない大きい行があると遅いのでキャッシュする
			int offset;
			if (nLineLogicNoCache == pLayout->GetLogicLineNo()
				&& nLineNoCache == GetCaretLayoutPos().y
				&& nLineLogicModCache == ModifyVisitor().GetLineModifiedSeq( pLayout->GetDocLineRef() )
			) {
				offset = nOffsetCache;
			}else if (
				nLineLogicNoCache == pLayout->GetLogicLineNo()
				&& nLineNoCache < GetCaretLayoutPos().y
				&& nLineLogicModCache == ModifyVisitor().GetLineModifiedSeq( pLayout->GetDocLineRef() )
			) {
				// 下移動
				offset = pLayout->CalcLayoutOffset(layoutMgr, nLogicOffsetCache, nOffsetCache);
				nOffsetCache = offset;
				nLogicOffsetCache = pLayout->GetLogicOffset();
				nLineNoCache = GetCaretLayoutPos().y;
			}else if (nLineLogicNoCache == pLayout->GetLogicLineNo()
				&& nLineNo50Cache <= GetCaretLayoutPos().y
				&& GetCaretLayoutPos().y <= nLineNo50Cache + 50
				&& nLineLogicModCache == ModifyVisitor().GetLineModifiedSeq( pLayout->GetDocLineRef() )
			) {
				// 上移動
				offset = pLayout->CalcLayoutOffset(layoutMgr, nLogicOffset50Cache, nOffset50Cache);
				nOffsetCache = offset;
				nLogicOffsetCache = pLayout->GetLogicOffset();
				nLineNoCache = GetCaretLayoutPos().y;
			}else {
			// 2013.05.11 折り返しなしとして計算する
				const Layout* pLayout50 = pLayout;
				int nLineNum = GetCaretLayoutPos().y;
				for (;;) {
					if (pLayout50->GetLogicOffset() == 0) {
						break;
					}
					if (nLineNum + 50 == GetCaretLayoutPos().y) {
						break;
					}
					pLayout50 = pLayout50->GetPrevLayout();
					--nLineNum;
				}
				nOffset50Cache = pLayout50->CalcLayoutOffset(layoutMgr);
				nLogicOffset50Cache = pLayout50->GetLogicOffset();
				nLineNo50Cache = nLineNum;
				
				offset = pLayout->CalcLayoutOffset(layoutMgr, nLogicOffset50Cache, nOffset50Cache);
				nOffsetCache = offset;
				nLogicOffsetCache = pLayout->GetLogicOffset();
				nLineLogicNoCache = pLayout->GetLogicLineNo();
				nLineNoCache = GetCaretLayoutPos().y;
				nLineLogicModCache = ModifyVisitor().GetLineModifiedSeq( pLayout->GetDocLineRef() );
			}
			Layout layout(
				pLayout->GetDocLineRef(),
				pLayout->GetLogicPos(),
				pLayout->GetLengthWithEOL(),
				pLayout->GetColorTypePrev(),
				offset,
				nullptr
			);
			ptCaret.x = (int)editView.LineIndexToColumn(&layout, GetCaretLogicPos().x - pLayout->GetLogicPos().x);
		}
	// 行番号をレイアウト単位で表示
	}else {
		ptCaret.x = GetCaretLayoutPos().GetX();
		ptCaret.y = GetCaretLayoutPos().GetY();
	}
	// 表示値が1から始まるように補正
	ptCaret.x++;
	ptCaret.y++;

	// -- -- -- -- キャレット位置の文字情報 -> szCaretChar -- -- -- -- //
	//
	TCHAR szCaretChar[32] = _T("");
	if (pLine) {
		// 指定された桁に対応する行のデータ内の位置を調べる
		ASSERT_GE(GetCaretLogicPos().x, pLayout->GetLogicOffset());
		int nIdx = GetCaretLogicPos().x - pLayout->GetLogicOffset();
		if (nIdx < (int)nLineLen) {
			if (nIdx < (int)nLineLen - (pLayout->GetLayoutEol().GetLen() ? 1 : 0)) {
				//auto_sprintf(szCaretChar, _T("%04x"),);
				// 任意の文字コードからUnicodeへ変換する		2008/6/9 Uchi
				CodeBase* pCode = CodeFactory::CreateCodeBase(editDoc.GetDocumentEncoding(), false);
				CommonSetting_StatusBar* psStatusbar = &GetDllShareData().common.statusBar;
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
		size_t	nLen;
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
		editDoc.pEditWnd->PrintMenubarMessage(szText);
	// ステータスバーに状態を書き出す
	}else {
		TCHAR	szText_1[64];
		auto_sprintf_s(szText_1, LS(STR_STATUS_ROW_COL), ptCaret.y, ptCaret.x);

		TCHAR	szText_6[16];
		if (editView.IsInsMode() /* Oct. 2, 2005 genta */) {
			_tcscpy_s(szText_6, LS(STR_INS_MODE_INS));	// "挿入"
		}else {
			_tcscpy_s(szText_6, LS(STR_INS_MODE_OVR));	// "上書"
		}
		if (bClearStatus) {
			::StatusBar_SetText(hwndStatusBar, 0 | SBT_NOBORDERS, _T(""));
		}
		::StatusBar_SetText(hwndStatusBar, 1 | 0,             szText_1);
		::StatusBar_SetText(hwndStatusBar, 2 | 0,             szEolMode);
		::StatusBar_SetText(hwndStatusBar, 3 | 0,             szCaretChar);
		::StatusBar_SetText(hwndStatusBar, 4 | 0,             pszCodeName);
		::StatusBar_SetText(hwndStatusBar, 5 | SBT_OWNERDRAW, _T(""));
		::StatusBar_SetText(hwndStatusBar, 6 | 0,             szText_6);
	}

}

void Caret::ClearCaretPosInfoCache()
{
	nOffsetCache = -1;
	nLineNoCache = -1;
	nLogicOffsetCache = -1;
	nLineLogicNoCache = -1;
	nLineNo50Cache = -1;
	nOffset50Cache = -1;
	nLogicOffset50Cache = -1;
	nLineLogicModCache = -1;
}

/* カーソル上下移動処理 */
int Caret::Cursor_UPDOWN(int nMoveLines, bool bSelect)
{
	// 必要なインターフェース
	auto& layoutMgr = editDoc.layoutMgr;
	auto& csGeneral = GetDllShareData().common.general;

	const Point ptCaret = GetCaretLayoutPos();

	bool bVertLineDoNotOFF = true;	// カーソル位置縦線を消去しない
	if (bSelect) {
		bVertLineDoNotOFF = false;		// 選択状態ならカーソル位置縦線消去を行う
	}

	auto& selInfo = editView.GetSelectionInfo();

	// 現在のキャレットY座標 + nMoveLinesが正しいレイアウト行の範囲内に収まるように nMoveLinesを調整する。
	if (nMoveLines > 0) { // 下移動。
		const bool existsEOFOnlyLine = layoutMgr.GetBottomLayout() && layoutMgr.GetBottomLayout()->GetLayoutEol() != EolType::None
			|| layoutMgr.GetLineCount() == 0;
		const size_t maxLayoutLine = layoutMgr.GetLineCount() + (existsEOFOnlyLine ? 1 : 0) - 1;
		// 移動先が EOFのみの行を含めたレイアウト行数未満になるように移動量を規正する。
		nMoveLines = t_min(nMoveLines, ((int)maxLayoutLine - (int)ptCaret.y));
		if (1
			&& ptCaret.y + nMoveLines == maxLayoutLine
			&& existsEOFOnlyLine // 移動先が EOFのみの行
			&& selInfo.IsBoxSelecting()
			&& ptCaret.x != 0 // かつ矩形選択中なら、
		) {
			// EOFのみの行には移動しない。下移動でキャレットの X座標を動かしたくないので。
			nMoveLines = t_max(0, nMoveLines - 1); // うっかり上移動しないように 0以上を守る。
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
	Point ptTo(0, ptCaret.y + nMoveLines);

	// 移動先の行のデータを取得
	const Layout* const pLayout = layoutMgr.SearchLineByLayoutY(ptTo.y);
	const size_t nLineLen = pLayout ? pLayout->GetLengthWithEOL() : 0;
	size_t i = 0; ///< 何？
	if (pLayout) {
		MemoryIterator it(pLayout, layoutMgr.GetTabSpace());
		while (!it.end()) {
			it.scanNext();
			if (it.getIndex() + it.getIndexDelta() > pLayout->GetLengthWithoutEOL()) {
				i = nLineLen;
				break;
			}
			if ((int)(it.getColumn() + it.getColumnDelta()) > nCaretPosX_Prev) {
				i = it.getIndex();
				break;
			}
			it.addDelta();
		}
		ptTo.x += (int)it.getColumn();
		if (it.end()) {
			i = it.getIndex();
		}
	}
	if (i >= nLineLen) {
		// フリーカーソルモードと矩形選択中は、キャレットの位置を改行や EOFの前に制限しない
		if (csGeneral.bIsFreeCursorMode
			|| selInfo.IsBoxSelecting()
		) {
			ptTo.x = nCaretPosX_Prev;
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
	const int nScrollLines = MoveCursor(	ptTo,
								editView.GetDrawSwitch() /* TRUE */,
								_CARETMARGINRATE,
								false,
								bVertLineDoNotOFF);
	return nScrollLines;
}


/*!	キャレットの作成

	@param nCaretColor [in]	キャレットの色種別 (0:通常, 1:IME ON)
	@param nWidth [in]		キャレット幅
	@param nHeight [in]		キャレット高
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

	HDC hdc = editView.GetDC();

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

	editView.ReleaseDC(hdc);

	// 以前のビットマップを破棄する
	if (this->hbmpCaret) {
		::DeleteObject(this->hbmpCaret);
	}
	this->hbmpCaret = hbmpCaret;

	// キャレットを作成する
	editView.CreateCaret(hbmpCaret, nWidth, nHeight);
	return;
}


/*!
	キャレットの表示
*/
void Caret::ShowCaret_(HWND hwnd)
{
	if (!bCaretShowFlag) {
		::ShowCaret(hwnd);
		bCaretShowFlag = true;
	}
}


/*!
	キャレットの非表示
*/
void Caret::HideCaret_(HWND hwnd)
{
	if (bCaretShowFlag) {
		::HideCaret(hwnd);
		bCaretShowFlag = false;
	}
}

// 自分の状態を他のCaretにコピー
void Caret::CopyCaretStatus(Caret* pCaret) const
{
	pCaret->SetCaretLayoutPos(GetCaretLayoutPos());
	pCaret->SetCaretLogicPos(GetCaretLogicPos());
	pCaret->nCaretPosX_Prev = nCaretPosX_Prev;	// ビュー左端からのカーソル桁位置（０オリジン

	//※ キャレットのサイズはコピーしない。
}


POINT Caret::CalcCaretDrawPos(const Point& ptCaretPos) const
{
	auto& textArea = editView.GetTextArea();
	int nPosX = textArea.GetAreaLeft() + (ptCaretPos.x - textArea.GetViewLeftCol()) * GetHankakuDx();
	int nY = ptCaretPos.y - textArea.GetViewTopLine();
	int nPosY;
	if (nY < 0) {
		nPosY = -1;
	}else if (textArea.nViewRowNum < nY) {
		nPosY = textArea.GetAreaBottom() + 1;
	}else {
		nPosY = textArea.GetAreaTop()
			+ nY * editView.GetTextMetrics().GetHankakuDy()
			+ editView.GetTextMetrics().GetHankakuHeight() - GetCaretSize().cy; // 下寄せ
	}

	return Point(nPosX, nPosY);
}


/*!
	行桁指定によるカーソル移動（座標調整付き）

	@return 縦スクロール行数(負:上スクロール/正:下スクロール)

	@note マウス等による移動で不適切な位置に行かないよう座標調整してカーソル移動する
*/
int Caret::MoveCursorProperly(
	Point	ptNewXY,			// [in] カーソルのレイアウト座標X
	bool	bScroll,			// [in] true: 画面位置調整有り/ false: 画面位置調整有り無し
	bool	test,				// [in] true: カーソル移動はしない
	Point*	ptNewXYNew,			// [out] 新しいレイアウト座標
	int		nCaretMarginRate,	// [in] 縦スクロール開始位置を決める値
	int		dx					// [in] ptNewXY.xとマウスカーソル位置との誤差(カラム幅未満のドット数)
	)
{
	size_t			nLineLen;
	const Layout*	pLayout;

	if (0 > ptNewXY.y) {
		ptNewXY.y = 0;
	}
	
	// EOF以下の行だった場合で矩形のときは、最終レイアウト行へ移動する
	auto& layoutMgr = editDoc.layoutMgr;
	auto& selectionInfo = editView.GetSelectionInfo();
	if (1
		&& ptNewXY.y >= (int)layoutMgr.GetLineCount()
		&& (selectionInfo.IsMouseSelecting() && selectionInfo.IsBoxSelecting())
	) {
		const Layout* layoutEnd = layoutMgr.GetBottomLayout();
		bool bEofOnly = (layoutEnd && layoutEnd->GetLayoutEol() != EolType::None) || !layoutEnd;
	 	// ぴったり[EOF]位置にある場合は位置を維持(1つ上の行にしない)
	 	if (1
	 		&& bEofOnly
	 		&& ptNewXY.y == layoutMgr.GetLineCount()
	 		&& ptNewXY.x == 0
	 	) {
	 	}else {
			ptNewXY.y = t_max(0, (int)layoutMgr.GetLineCount() - 1);
		}
	}
	// カーソルがテキスト最下端行にあるか
	if (ptNewXY.y >= (int)layoutMgr.GetLineCount()) {
	// カーソルがテキスト最上端行にあるか
	}else if (ptNewXY.y < 0) {
		ptNewXY.Set(0, 0);
	}else {
		// 移動先の行のデータを取得
		layoutMgr.GetLineStr(ptNewXY.y, &nLineLen, &pLayout);

		size_t nColWidth = editView.GetTextMetrics().GetHankakuDx();
		int nPosX = 0;
		size_t i = 0;
		MemoryIterator it(pLayout, layoutMgr.GetTabSpace());
		while (!it.end()) {
			it.scanNext();
			if (it.getIndex() + it.getIndexDelta() > pLayout->GetLengthWithoutEOL()) {
				i = nLineLen;
				break;
			}
			if ((int)(it.getColumn() + it.getColumnDelta()) > ptNewXY.x) {
				if (1
					&& ptNewXY.x >= (int)(pLayout ? pLayout->GetIndent() : 0)
					&& ((ptNewXY.x - it.getColumn()) * nColWidth + dx) * 2 >= it.getColumnDelta() * nColWidth
				) {
				//if (ptNewXY.x >= (pLayout ? pLayout->GetIndent() : 0) && (it.getColumnDelta() > 1) && ((it.getColumn() + it.getColumnDelta() - ptNewXY.x) <= it.getColumnDelta() / 2)) {
					nPosX += (int)it.getColumnDelta();
				}
				i = it.getIndex();
				break;
			}
			it.addDelta();
		}
		nPosX += (int)it.getColumn();
		if (it.end()) {
			i = it.getIndex();
		}

		if (i >= nLineLen) {
			// 2011.12.26 フリーカーソル/矩形でデータ付きEOFの右側へ移動できるように
			// フリーカーソルモードか
			if (0
				|| GetDllShareData().common.general.bIsFreeCursorMode
				|| (selectionInfo.IsMouseSelecting() && selectionInfo.IsBoxSelecting())	/* マウス範囲選択中 && 矩形範囲選択中 */
				|| (editView.bDragMode && editView.bDragBoxData) /* OLE DropTarget && 矩形データ */
			) {
				// 折り返し幅とレイアウト行桁数（ぶら下げを含む）のどちらか大きいほうまでカーソル移動可能
				//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
				int nMaxX = t_max(nPosX, (int)layoutMgr.GetMaxLineKetas());
				nPosX = ptNewXY.x;
				if (nPosX < 0) {
					nPosX = 0;
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
		return 0;
	}
	return MoveCursor(ptNewXY, bScroll, nCaretMarginRate);
}

