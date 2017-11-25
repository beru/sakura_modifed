#include "StdAfx.h"
#include "EditView_Paint.h"
#include "window/EditWnd.h"
#include "doc/EditDoc.h"
#include "types/TypeSupport.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           括弧                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@param flag [in] モード(true:登録, false:解除)
*/
void EditView::SetBracketPairPos(bool flag)
{
	if (bDoing_UndoRedo || !GetDrawSwitch()) {
		return;
	}

	if (!pTypeData->colorInfoArr[COLORIDX_BRACKET_PAIR].bDisp) {
		return;
	}

	// 対括弧の検索&登録
	/*
	bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる
	bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
	bit2(out) : 見つかった位置         0:後ろ      1:前
	*/
	int	mode = 2;

	Point ptColLine;
	auto& caret = GetCaret();

	if (1
		&& flag
		&& !GetSelectionInfo().IsTextSelected()
		&& !GetSelectionInfo().bDrawSelectArea
		&& SearchBracket(caret.GetCaretLayoutPos(), &ptColLine, &mode)
	) {
		// 登録指定(flag=true)			&&
		// テキストが選択されていない	&&
		// 選択範囲を描画していない		&&
		// 対応する括弧が見つかった		場合
		if (1
			&& (ptColLine.x >= GetTextArea().GetViewLeftCol())
			&& (ptColLine.x <= GetTextArea().GetRightCol())
			&& (ptColLine.y >= GetTextArea().GetViewTopLine())
			&& (ptColLine.y <= GetTextArea().GetBottomLine())
		) {
			// 表示領域内の場合

			// レイアウト位置から物理位置へ変換(強調表示位置を登録)
			ptBracketPairPos_PHY = pEditDoc->layoutMgr.LayoutToLogic(ptColLine);
			ptBracketCaretPos_PHY.y = caret.GetCaretLogicPos().y;
			if ((mode & 4) == 0) {
				// カーソルの後方文字位置
				ptBracketCaretPos_PHY.x = caret.GetCaretLogicPos().x;
			}else {
				// カーソルの前方文字位置
				ptBracketCaretPos_PHY.x = caret.GetCaretLogicPos().x - 1;
			}
			return;
		}
	}

	// 括弧の強調表示位置情報初期化
	ptBracketPairPos_PHY.Set(-1, -1);
	ptBracketCaretPos_PHY.Set(-1, -1);

	return;
}

/*!
	対括弧の強調表示
*/
void EditView::DrawBracketPair(bool bDraw)
{
	if (bDoing_UndoRedo || !GetDrawSwitch()) {
		return;
	}

	if (!pTypeData->colorInfoArr[COLORIDX_BRACKET_PAIR].bDisp) {
		return;
	}

	// 括弧の強調表示位置が未登録の場合は終了
	if (ptBracketPairPos_PHY.HasNegative() || ptBracketCaretPos_PHY.HasNegative()) {
		return;
	}

	// 描画指定(bDraw=true)				かつ
	// (テキストが選択されている		又は
	//   選択範囲を描画している			又は
	//   フォーカスを持っていない		又は
	//   アクティブなペインではない)	場合は終了
	if (bDraw
		&& (0
			|| GetSelectionInfo().IsTextSelected()
			|| GetSelectionInfo().bDrawSelectArea
			|| !bDrawBracketPairFlag
			|| (editWnd.GetActivePane() != nMyIndex)
		)
	) {
		return;
	}
	
	Graphics gr;
	gr.Init(::GetDC(GetHwnd()));
	bool bCaretChange = false;
	gr.SetTextBackTransparent(true);
	auto& caret = GetCaret();

	for (int i=0; i<2; ++i) {
		// i=0:対括弧,i=1:カーソル位置の括弧
		Point ptColLine;
		if (i == 0) {
			ptColLine = pEditDoc->layoutMgr.LogicToLayout(ptBracketPairPos_PHY);
		}else {
			ptColLine = pEditDoc->layoutMgr.LogicToLayout(ptBracketCaretPos_PHY);
		}

		if (1
			&& (ptColLine.x >= GetTextArea().GetViewLeftCol())
			&& (ptColLine.x <= GetTextArea().GetRightCol())
			&& (ptColLine.y >= GetTextArea().GetViewTopLine())
			&& (ptColLine.y <= GetTextArea().GetBottomLine()) 
		) {	// 表示領域内の場合
			if (1
				&& !bDraw
				&& GetSelectionInfo().bDrawSelectArea
				&& (IsCurrentPositionSelected(ptColLine) == 0)
			) {	// 選択範囲描画済みで消去対象の括弧が選択範囲内の場合
				continue;
			}
			const Layout* pLayout;
			size_t nLineLen;
			const wchar_t* pLine = pEditDoc->layoutMgr.GetLineStr(ptColLine.y, &nLineLen, &pLayout);
			if (pLine) {
				EColorIndexType nColorIndex;
				size_t OutputX = LineColumnToIndex(pLayout, ptColLine.x);
				if (bDraw) {
					nColorIndex = COLORIDX_BRACKET_PAIR;
				}else {
					if (IsBracket(pLine, OutputX, 1)) {
						DispPos pos(0, 0); // 注意：この値はダミー。CheckChangeColorでの参照位置は不正確
						ColorStrategyInfo csInfo(*this);
						csInfo.pDispPos = &pos;

						// 03/10/24 ai 折り返し行のColorIndexが正しく取得できない問題に対応
						Color3Setting cColor = GetColorIndex(pLayout, ptColLine.y, OutputX, csInfo);
						nColorIndex = cColor.eColorIndex2;
					}else {
						SetBracketPairPos(false);
						break;
					}
				}
				TypeSupport curetLineBg(*this, COLORIDX_CARETLINEBG);
				EColorIndexType nColorIndexBg = (curetLineBg.IsDisp() && ptColLine.y == caret.GetCaretLayoutPos().y
					? COLORIDX_CARETLINEBG
					: TypeSupport(*this, COLORIDX_EVENLINEBG).IsDisp() && ptColLine.y % 2 == 1
						? COLORIDX_EVENLINEBG
						: COLORIDX_TEXT);
				// 03/03/03 ai カーソルの左に括弧があり括弧が強調表示されている状態でShift+←で選択開始すると
				//             選択範囲内に反転表示されない部分がある問題の修正
				int caretX = caret.GetCaretLayoutPos().x;
				bool bCaretHide = (!bCaretChange && (ptColLine.x == caretX || ptColLine.x + 1 == caretX) && caret.GetCaretShowFlag());
				if (bCaretHide) {
					bCaretChange = true;
					caret.HideCaret_(GetHwnd());	// キャレットが一瞬消えるのを防止
				}
				
				{
					int nWidth  = GetTextMetrics().GetHankakuDx();
					int nHeight = GetTextMetrics().GetHankakuDy();
					int nLeft = (GetTextArea().GetDocumentLeftClientPointX()) + ptColLine.x * nWidth;
					int nTop  = (ptColLine.y - GetTextArea().GetViewTopLine()) * nHeight + GetTextArea().GetAreaTop();
					size_t charsWidth = NativeW::GetKetaOfChar(pLine, nLineLen, OutputX);

					// 色設定
					TypeSupport textType(*this, COLORIDX_TEXT);
					textType.SetGraphicsState_WhileThisObj(gr);
					// 2013.05.24 背景色がテキストの背景色と同じならカーソル行の背景色を適用
					TypeSupport colorIndexType(*this, nColorIndex);
					TypeSupport colorIndexBgType(*this, nColorIndexBg);
					TypeSupport* pColorBack = &colorIndexType;
					if (colorIndexType.GetBackColor() == textType.GetBackColor() && nColorIndexBg != COLORIDX_TEXT) {
						pColorBack = &colorIndexBgType;
					}

					SetCurrentColor(gr, nColorIndex, nColorIndex, nColorIndexBg);
					bool bTrans = false;
					// DEBUG_TRACE(_T("DrawBracket %d %d ") , ptColLine.y, ptColLine.x);
					if (1
						&& IsBkBitmap()
						&& textType.GetBackColor() == pColorBack->GetBackColor()
					) {
						bTrans = true;
						RECT rcChar;
						rcChar.left  = nLeft;
						rcChar.top = nTop;
						rcChar.right = nLeft + charsWidth * nWidth;
						rcChar.bottom = nTop + nHeight;
						HDC hdcBgImg = ::CreateCompatibleDC(gr);
						HBITMAP hBmpOld = (HBITMAP)::SelectObject(hdcBgImg, pEditDoc->hBackImg);
						DrawBackImage(gr, rcChar, hdcBgImg);
						::SelectObject(hdcBgImg, hBmpOld);
						::DeleteDC(hdcBgImg);
					}
					DispPos pos(nWidth, nHeight);
					pos.InitDrawPos(Point(nLeft, nTop));
					GetTextDrawer().DispText(gr, &pos,  &pLine[OutputX], 1, bTrans);
					GetTextDrawer().DispNoteLine(gr, nTop, nTop + nHeight, nLeft, nLeft + charsWidth * nWidth);
					// 対括弧の縦線対応
					GetTextDrawer().DispVerticalLines(gr, nTop, nTop + nHeight, ptColLine.x, ptColLine.x + charsWidth); // ※括弧が全角幅である場合を考慮
					textType.RewindGraphicsState(gr);
				}

				if (1
					&& (editWnd.GetActivePane() == nMyIndex)
					&& (0
						|| (ptColLine.y == caret.GetCaretLayoutPos().GetY())
						|| (ptColLine.y - 1 == caret.GetCaretLayoutPos().GetY())
					) 
				) {	// 03/02/27 ai 行の間隔が"0"の時にアンダーラインが欠ける事がある為修正
					caret.underLine.CaretUnderLineON(true, false);
				}
			}
		}
	}
	if (bCaretChange) {
		caret.ShowCaret_(GetHwnd());	// キャレットが一瞬消えるのを防止
	}

	::ReleaseDC(GetHwnd(), gr);
}


//======================================================================
// 対括弧の対応表
struct KAKKO_T {
	const wchar_t* sStr;
	const wchar_t* eStr;
};
static const KAKKO_T g_aKakkos[] = {
	// 半角
	{ L"(", L")", },
	{ L"[", L"]", },
	{ L"{", L"}", },
	{ L"<", L">", },
	{ L"｢", L"｣", },
	// 全角
	{ L"【", L"】", },
	{ L"『", L"』", },
	{ L"「", L"」", },
	{ L"＜", L"＞", },
	{ L"≪", L"≫", },
	{ L"《", L"》", },
	{ L"（", L"）", },
	{ L"〈", L"〉", },
	{ L"｛", L"｝", },
	{ L"〔", L"〕", },
	{ L"［", L"］", },
	{ L"“", L"”", },
	{ L"〝", L"〟", },
	// 終端
	{ NULL, NULL, },
};


//	Jun. 16, 2000 genta
/*!
	@brief 対括弧の検索

	カーソル位置の括弧に対応する括弧を探す。カーソル位置が括弧でない場合は
	カーソルの後ろの文字が括弧かどうかを調べる。

	カーソルの前後いずれもが括弧でない場合は何もしない。

	括弧が半角か全角か、及び始まりか終わりかによってこれに続く4つの関数に
	制御を移す。

	@param ptLayout [in] 検索開始点の物理座標
	@param pptLayoutNew [out] 移動先のレイアウト座標
	@param mode [in/out] bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる
						 bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる (このbitを参照)
						 bit2(out) : 見つかった位置         0:後ろ      1:前     (このbitを更新)

	@retval true 成功
	@retval false 失敗
*/
bool EditView::SearchBracket(
	const Point&	ptLayout,
	Point*		pptLayoutNew,
	int*				mode
	)
{
	size_t len;	//	行の長さ
	Point ptPos = pEditDoc->layoutMgr.LayoutToLogic(ptLayout);
	const wchar_t* cline = pEditDoc->docLineMgr.GetLine(ptPos.y)->GetDocLineStrWithEOL(&len);

	//	Jun. 19, 2000 genta
	if (!cline)	//	最後の行に本文がない場合
		return false;

	// 括弧処理
	for (const KAKKO_T* p=g_aKakkos; p->sStr; ++p) {
		if (wcsncmp(p->sStr, &cline[ptPos.x], 1) == 0) {
			return SearchBracketForward(ptPos, pptLayoutNew, p->sStr, p->eStr, *mode);
		}else if (wcsncmp(p->eStr, &cline[ptPos.x], 1) == 0) {
			return SearchBracketBackward(ptPos, pptLayoutNew, p->sStr, p->eStr, *mode);
		}
	}

	// 02/09/18 ai Start
	if ((*mode & 2) == 0) {
		// カーソルの前方を調べない場合
		return false;
	}
	*mode |= 4;
	// 02/09/18 ai End

	//	括弧が見つからなかったら，カーソルの直前の文字を調べる

	if (ptPos.x <= 0) {
		return false;	//	前の文字はない
	}

	const wchar_t* bPos = NativeW::GetCharPrev(cline, ptPos.x, cline + ptPos.x);
	ptrdiff_t nCharSize = cline + ptPos.x - bPos;
	// 括弧処理 2007.10.16 kobake
	if (nCharSize == 1) {
		ptPos.x = bPos - cline;
		for (const KAKKO_T* p=g_aKakkos; p->sStr; ++p) {
			if (wcsncmp(p->sStr, &cline[ptPos.x], 1) == 0) {
				return SearchBracketForward(ptPos, pptLayoutNew, p->sStr, p->eStr, *mode);
			}else if (wcsncmp(p->eStr, &cline[ptPos.x], 1) == 0) {
				return SearchBracketBackward(ptPos, pptLayoutNew, p->sStr, p->eStr, *mode);
			}
		}
	}
	return false;
}

/*!
	@brief 半角対括弧の検索:順方向

	@param ptLayout [in] 検索開始点の物理座標
	@param pptLayoutNew [out] 移動先のレイアウト座標
	@param upChar [in] 括弧の始まりの文字
	@param dnChar [in] 括弧を閉じる文字列
	@param mode   [in] bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる (このbitを参照)
					 bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
					 bit2(out) : 見つかった位置         0:後ろ      1:前

	@retval true 成功
	@retval false 失敗
*/
// 03/01/08 ai
bool EditView::SearchBracketForward(
	Point	ptPos,
	Point*	pptLayoutNew,
	const wchar_t*	upChar,
	const wchar_t*	dnChar,
	int				mode
	)
{
	size_t len;
	int level = 0;


	// 初期位置の設定
	Point ptColLine = pEditDoc->layoutMgr.LogicToLayout(ptPos);	// 02/09/19 ai
	int nSearchNum = (GetTextArea().GetBottomLine()) - ptColLine.y;					// 02/09/19 ai
	DocLine* ci = pEditDoc->docLineMgr.GetLine(ptPos.y);
	const wchar_t* cline = ci->GetDocLineStrWithEOL(&len);
	const wchar_t* lineend = cline + len;
	const wchar_t* cPos = cline + ptPos.x;

//	auto typeData = *pTypeData;
//	auto lineComment = typeData.lineComment;

	do {
		while (cPos < lineend) {
			const wchar_t* nPos = NativeW::GetCharNext(cline, len, cPos);
			if (nPos - cPos > 1) {
				// skip
				cPos = nPos;
				continue;
			}
			// 03/01/08 ai Start
			if (wcsncmp(upChar, cPos, 1) == 0) {
				++level;
			}else if (wcsncmp(dnChar, cPos, 1) == 0) {
				--level;
			}// 03/01/08 ai End

			if (level == 0) {	// 見つかった！
				ptPos.x = cPos - cline;
				*pptLayoutNew = pEditDoc->layoutMgr.LogicToLayout(ptPos);
				return true;
				// Happy Ending
			}
			cPos = nPos;	// 次の文字へ
		}

		// 02/09/19 ai Start
		--nSearchNum;
		if (0 > nSearchNum && (mode & 1) == 0) {
			// 表示領域外を調べないモードで表示領域の終端の場合
			//SendStatusMessage("対括弧の検索を中断しました");
			break;
		}
		// 02/09/19 ai End

		//	次の行へ
		ptPos.y++;
		ci = ci->GetNextLine();	//	次のアイテム
		if (!ci)
			break;	//	終わりに達した

		cline = ci->GetDocLineStrWithEOL(&len);
		cPos = cline;
		lineend = cline + len;
	}while (cline);

	return false;
}

/*!
	@brief 半角対括弧の検索:逆方向

	@param ptLayout [in] 検索開始点の物理座標
	@param pptLayoutNew [out] 移動先のレイアウト座標
	@param upChar [in] 括弧の始まりの文字
	@param dnChar [in] 括弧を閉じる文字列
	@param mode [in] bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる (このbitを参照)
					 bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
					 bit2(out) : 見つかった位置         0:後ろ      1:前

	@retval true 成功
	@retval false 失敗
*/
bool EditView::SearchBracketBackward(
	Point	ptPos,
	Point*	pptLayoutNew,
	const wchar_t*	dnChar,
	const wchar_t*	upChar,
	int				mode
	)
{
	size_t len;
	int level = 1;

	// 初期位置の設定
	Point ptColLine = pEditDoc->layoutMgr.LogicToLayout(ptPos);	// 02/09/19 ai
	int nSearchNum = ptColLine.y - GetTextArea().GetViewTopLine();										// 02/09/19 ai
	DocLine* ci = pEditDoc->docLineMgr.GetLine(ptPos.y);
	const wchar_t* cline = ci->GetDocLineStrWithEOL(&len);
	const wchar_t* cPos = cline + ptPos.x;

	do {
		while (cPos > cline) {
			const wchar_t* pPos = NativeW::GetCharPrev(cline, len, cPos);
			if (cPos - pPos > 1) {
				//	skip
				cPos = pPos;
				continue;
			}
			// 03/01/08 ai Start
			if (wcsncmp(upChar, pPos, 1) == 0) {
				++level;
			}else if (wcsncmp(dnChar, pPos, 1) == 0) {
				--level;
			}// 03/01/08 ai End

			if (level == 0) {	// 見つかった！
				ptPos.x = pPos - cline;
				*pptLayoutNew = pEditDoc->layoutMgr.LogicToLayout(ptPos);
				return true;
				// Happy Ending
			}
			cPos = pPos;	// 次の文字へ
		}

		// 02/09/19 ai Start
		--nSearchNum;
		if (0 > nSearchNum && (mode & 1) == 0) {
			// 表示領域外を調べないモードで表示領域の先頭の場合
			//SendStatusMessage("対括弧の検索を中断しました");
			break;
		}
		// 02/09/19 ai End

		//	次の行へ
		ptPos.y--;
		ci = ci->GetPrevLine();	//	次のアイテム
		if (!ci)
			break;	//	終わりに達した

		cline = ci->GetDocLineStrWithEOL(&len);
		cPos = cline + len;
	}while (cline);
	
	return false;
}

//@@@ 2003.01.09 Start by ai:
/*!
	@brief 括弧判定

	@param pLine [in] 
	@param x
	@param size

	@retval true 括弧
	@retval false 非括弧
*/
bool EditView::IsBracket(
	const wchar_t* pLine,
	int x,
	int size
	)
{
	// 括弧処理
	if (size == 1) {
		for (const KAKKO_T* p=g_aKakkos; p->sStr; ++p) {
			if (wcsncmp(p->sStr, &pLine[x], 1) == 0) {
				return true;
			}else if (wcsncmp(p->eStr, &pLine[x], 1) == 0) {
				return true;
			}
		}
	}
	return false;
}
//@@@ 2003.01.09 End

