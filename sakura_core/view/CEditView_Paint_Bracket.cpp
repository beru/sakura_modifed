/*
	Copyright (C) 2008, kobake

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
#include "CEditView_Paint.h"
#include "window/CEditWnd.h"
#include "doc/CEditDoc.h"
#include "types/CTypeSupport.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           括弧                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@date 2003/02/18 ai
	@param flag [in] モード(true:登録, false:解除)
*/
void EditView::SetBracketPairPos(bool flag)
{
	// 03/03/06 ai すべて置換、すべて置換後のUndo&Redoがかなり遅い問題に対応
	if (m_bDoing_UndoRedo || !GetDrawSwitch()) {
		return;
	}

	if (!m_pTypeData->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp) {
		return;
	}

	// 対括弧の検索&登録
	/*
	bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる
	bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
	bit2(out) : 見つかった位置         0:後ろ      1:前
	*/
	int	mode = 2;

	LayoutPoint ptColLine;

	if (1
		&& flag
		&& !GetSelectionInfo().IsTextSelected()
		&& !GetSelectionInfo().m_bDrawSelectArea
		&& SearchBracket(GetCaret().GetCaretLayoutPos(), &ptColLine, &mode)
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
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(ptColLine, &m_ptBracketPairPos_PHY);
			m_ptBracketCaretPos_PHY.y = GetCaret().GetCaretLogicPos().y;
			if ((mode & 4) == 0) {
				// カーソルの後方文字位置
				m_ptBracketCaretPos_PHY.x = GetCaret().GetCaretLogicPos().x;
			}else {
				// カーソルの前方文字位置
				m_ptBracketCaretPos_PHY.x = GetCaret().GetCaretLogicPos().x - 1;
			}
			return;
		}
	}

	// 括弧の強調表示位置情報初期化
	m_ptBracketPairPos_PHY.Set(LogicInt(-1), LogicInt(-1));
	m_ptBracketCaretPos_PHY.Set(LogicInt(-1), LogicInt(-1));

	return;
}

/*!
	対括弧の強調表示
	@date 2002/09/18 ai
	@date 2003/02/18 ai 再描画対応の為大改造
*/
void EditView::DrawBracketPair(bool bDraw)
{
	// 03/03/06 ai すべて置換、すべて置換後のUndo&Redoがかなり遅い問題に対応
	if (m_bDoing_UndoRedo || !GetDrawSwitch()) {
		return;
	}

	if (!m_pTypeData->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp) {
		return;
	}

	// 括弧の強調表示位置が未登録の場合は終了
	if (m_ptBracketPairPos_PHY.HasNegative() || m_ptBracketCaretPos_PHY.HasNegative()) {
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
			|| GetSelectionInfo().m_bDrawSelectArea
			|| !m_bDrawBracketPairFlag
			|| (m_pcEditWnd->GetActivePane() != m_nMyIndex)
		)
	) {
		return;
	}
	
	Graphics gr;
	gr.Init(::GetDC(GetHwnd()));
	bool bCaretChange = false;
	gr.SetTextBackTransparent(true);

	for (int i=0; i<2; ++i) {
		// i=0:対括弧,i=1:カーソル位置の括弧
		// 2011.11.23 ryoji 対括弧 -> カーソル位置の括弧 の順に処理順序を変更
		//   ＃ { と } が異なる行にある場合に { を BS で消すと } の強調表示が解除されない問題（Wiki BugReport/89）の対策
		//   ＃ この順序変更によりカーソル位置が括弧でなくなっていても対括弧があれば対括弧側の強調表示は解除される

		LayoutPoint	ptColLine;

		if (i == 0) {
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(m_ptBracketPairPos_PHY,  &ptColLine);
		}else {
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(m_ptBracketCaretPos_PHY, &ptColLine);
		}

		if (1
			&& (ptColLine.x >= GetTextArea().GetViewLeftCol())
			&& (ptColLine.x <= GetTextArea().GetRightCol())
			&& (ptColLine.y >= GetTextArea().GetViewTopLine())
			&& (ptColLine.y <= GetTextArea().GetBottomLine()) 
		) {	// 表示領域内の場合
			if (1
				&& !bDraw
				&& GetSelectionInfo().m_bDrawSelectArea
				&& (IsCurrentPositionSelected(ptColLine) == 0)
			) {	// 選択範囲描画済みで消去対象の括弧が選択範囲内の場合
				continue;
			}
			const Layout* pcLayout;
			LogicInt		nLineLen;
			const wchar_t*	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr(ptColLine.GetY2(), &nLineLen, &pcLayout);
			if (pLine) {
				EColorIndexType nColorIndex;
				LogicInt	OutputX = LineColumnToIndex(pcLayout, ptColLine.GetX2());
				if (bDraw) {
					nColorIndex = COLORIDX_BRACKET_PAIR;
				}else {
					if (IsBracket(pLine, OutputX, LogicInt(1))) {
						DispPos _sPos(0, 0); // 注意：この値はダミー。CheckChangeColorでの参照位置は不正確
						ColorStrategyInfo _sInfo;
						ColorStrategyInfo* pInfo = &_sInfo;
						pInfo->m_pDispPos = &_sPos;
						pInfo->m_pcView = this;

						// 03/10/24 ai 折り返し行のColorIndexが正しく取得できない問題に対応
						// 2009.02.07 ryoji GetColorIndex に渡すインデックスの仕様変更（元はこっちの仕様だった模様）
						Color3Setting cColor = GetColorIndex(pcLayout, ptColLine.GetY2(), OutputX, pInfo);
						nColorIndex = cColor.eColorIndex2;
					}else {
						SetBracketPairPos(false);
						break;
					}
				}
				CTypeSupport    cCuretLineBg(this, COLORIDX_CARETLINEBG);
				EColorIndexType nColorIndexBg = (cCuretLineBg.IsDisp() && ptColLine.GetY2() == GetCaret().GetCaretLayoutPos().GetY2()
					? COLORIDX_CARETLINEBG
					: CTypeSupport(this, COLORIDX_EVENLINEBG).IsDisp() && ptColLine.GetY2() % 2 == 1
						? COLORIDX_EVENLINEBG
						: COLORIDX_TEXT);
				// 03/03/03 ai カーソルの左に括弧があり括弧が強調表示されている状態でShift+←で選択開始すると
				//             選択範囲内に反転表示されない部分がある問題の修正
				LayoutInt caretX = GetCaret().GetCaretLayoutPos().GetX2();
				bool bCaretHide = (!bCaretChange && (ptColLine.x == caretX || ptColLine.x + 1 == caretX) && GetCaret().GetCaretShowFlag());
				if (bCaretHide) {
					bCaretChange = true;
					GetCaret().HideCaret_(GetHwnd());	// キャレットが一瞬消えるのを防止
				}
				
				{
					int nWidth  = GetTextMetrics().GetHankakuDx();
					int nHeight = GetTextMetrics().GetHankakuDy();
					int nLeft = (GetTextArea().GetDocumentLeftClientPointX()) + (Int)ptColLine.x * nWidth;
					int nTop  = (Int)(ptColLine.GetY2() - GetTextArea().GetViewTopLine()) * nHeight + GetTextArea().GetAreaTop();
					LayoutInt charsWidth = NativeW::GetKetaOfChar(pLine, nLineLen, OutputX);

					// 色設定
					CTypeSupport cTextType(this, COLORIDX_TEXT);
					cTextType.SetGraphicsState_WhileThisObj(gr);
					// 2013.05.24 背景色がテキストの背景色と同じならカーソル行の背景色を適用
					CTypeSupport cColorIndexType(this, nColorIndex);
					CTypeSupport cColorIndexBgType(this, nColorIndexBg);
					CTypeSupport* pcColorBack = &cColorIndexType;
					if (cColorIndexType.GetBackColor() == cTextType.GetBackColor() && nColorIndexBg != COLORIDX_TEXT) {
						pcColorBack = &cColorIndexBgType;
					}

					SetCurrentColor(gr, nColorIndex, nColorIndex, nColorIndexBg);
					bool bTrans = false;
					// DEBUG_TRACE(_T("DrawBracket %d %d ") , ptColLine.y, ptColLine.x);
					if (1
						&& IsBkBitmap()
						&& cTextType.GetBackColor() == pcColorBack->GetBackColor()
					) {
						bTrans = true;
						RECT rcChar;
						rcChar.left  = nLeft;
						rcChar.top = nTop;
						rcChar.right = nLeft + (Int)charsWidth * nWidth;
						rcChar.bottom = nTop + nHeight;
						HDC hdcBgImg = ::CreateCompatibleDC(gr);
						HBITMAP hBmpOld = (HBITMAP)::SelectObject(hdcBgImg, m_pcEditDoc->m_hBackImg);
						DrawBackImage(gr, rcChar, hdcBgImg);
						::SelectObject(hdcBgImg, hBmpOld);
						::DeleteDC(hdcBgImg);
					}
					DispPos sPos(nWidth, nHeight);
					sPos.InitDrawPos(Point(nLeft, nTop));
					GetTextDrawer().DispText(gr, &sPos,  &pLine[OutputX], 1, bTrans);
					GetTextDrawer().DispNoteLine(gr, nTop, nTop + nHeight, nLeft, nLeft + (Int)charsWidth * nWidth);
					// 2006.04.30 Moca 対括弧の縦線対応
					GetTextDrawer().DispVerticalLines(gr, nTop, nTop + nHeight, ptColLine.x, ptColLine.x + charsWidth); // ※括弧が全角幅である場合を考慮
					cTextType.RewindGraphicsState(gr);
				}

				if (1
					&& (m_pcEditWnd->GetActivePane() == m_nMyIndex)
					&& (0
						|| (ptColLine.y == GetCaret().GetCaretLayoutPos().GetY())
						|| (ptColLine.y - 1 == GetCaret().GetCaretLayoutPos().GetY())
					) 
				) {	// 03/02/27 ai 行の間隔が"0"の時にアンダーラインが欠ける事がある為修正
					GetCaret().m_cUnderLine.CaretUnderLineON(true, false);
				}
			}
		}
	}
	if (bCaretChange) {
		GetCaret().ShowCaret_(GetHwnd());	// キャレットが一瞬消えるのを防止
	}

	::ReleaseDC(GetHwnd(), gr);
}


//======================================================================
// 対括弧の対応表
// 2007.10.16 kobake
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

	@author genta
	@date Jun. 16, 2000 genta
	@date Feb. 03, 2001 MIK 全角括弧に対応
	@date Sep. 18, 2002 ai modeの追加
*/
bool EditView::SearchBracket(
	const LayoutPoint&	ptLayout,
	LayoutPoint*		pptLayoutNew,
	int*				mode
)
{
	LogicInt len;	//	行の長さ

	LogicPoint ptPos;

	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(ptLayout, &ptPos);
	const wchar_t* cline = m_pcEditDoc->m_cDocLineMgr.GetLine(ptPos.GetY2())->GetDocLineStrWithEOL(&len);

	//	Jun. 19, 2000 genta
	if (!cline)	//	最後の行に本文がない場合
		return false;

	// 括弧処理 2007.10.16 kobake
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
	int nCharSize = cline + ptPos.x - bPos;
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

	@author genta

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
	LogicPoint		ptPos,
	LayoutPoint*	pptLayoutNew,
	const wchar_t*	upChar,
	const wchar_t*	dnChar,
	int				mode
)
{
	int len;
	int level = 0;

	LayoutPoint ptColLine;

	// 初期位置の設定
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(ptPos, &ptColLine);	// 02/09/19 ai
	LayoutInt nSearchNum = (GetTextArea().GetBottomLine()) - ptColLine.y;					// 02/09/19 ai
	DocLine* ci = m_pcEditDoc->m_cDocLineMgr.GetLine(ptPos.GetY2());
	const wchar_t* cline = ci->GetDocLineStrWithEOL(&len);
	const wchar_t* lineend = cline + len;
	const wchar_t* cPos = cline + ptPos.x;

//	auto typeData = *m_pTypeData;
//	auto lineComment = typeData.m_cLineComment;

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
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout(ptPos, pptLayoutNew);
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

	@author genta

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
	LogicPoint		ptPos,
	LayoutPoint*	pptLayoutNew,
	const wchar_t*	dnChar,
	const wchar_t*	upChar,
	int				mode
)
{
	int			len;
	int			level = 1;
	LayoutPoint ptColLine;

	// 初期位置の設定
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(ptPos, &ptColLine);	// 02/09/19 ai
	LayoutInt nSearchNum = ptColLine.y - GetTextArea().GetViewTopLine();										// 02/09/19 ai
	DocLine* ci = m_pcEditDoc->m_cDocLineMgr.GetLine(ptPos.GetY2());
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
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout(ptPos, pptLayoutNew);
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

	@author ai

	@param pLine [in] 
	@param x
	@param size

	@retval true 括弧
	@retval false 非括弧
*/
bool EditView::IsBracket(const wchar_t* pLine, LogicInt x, LogicInt size)
{
	// 括弧処理 2007.10.16 kobake
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

