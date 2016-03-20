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
#pragma once

class EditView;

#include "basis/SakuraBasis.h"
#include "doc/layout/Layout.h"

class ViewSelect {
public:
	EditView& GetEditView() { return m_editView; }
	const EditView& GetEditView() const { return m_editView; }

public:
	ViewSelect(EditView& editView);
	void CopySelectStatus(ViewSelect* pSelect) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      選択範囲の変更                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void DisableSelectArea(bool bDraw, bool bDrawBracketCursorLine = true); // 現在の選択範囲を非選択状態に戻す

	void BeginSelectArea(const LayoutPoint* po = NULL);								// 現在のカーソル位置から選択を開始する
	void ChangeSelectAreaByCurrentCursor(const LayoutPoint& ptCaretPos);			// 現在のカーソル位置によって選択範囲を変更
	void ChangeSelectAreaByCurrentCursorTEST(const LayoutPoint& ptCaretPos, LayoutRange* pSelect);// 現在のカーソル位置によって選択範囲を変更

	// 選択範囲を指定する(原点未選択)
	// 2005.06.24 Moca
	void SetSelectArea(const LayoutRange& range) {
		m_selectBgn.Set(range.GetFrom());
		m_select = range;
	}

	// 単語選択開始
	void SelectBeginWord() {
		m_bBeginSelect     = true;			// 範囲選択中
		m_bBeginBoxSelect  = false;			// 矩形範囲選択中でない
		m_bBeginLineSelect = false;			// 行単位選択中
		m_bBeginWordSelect = true;			// 単語単位選択中
	}

	// 矩形選択開始
	void SelectBeginBox() {
		m_bBeginSelect     = true;		// 範囲選択中
		m_bBeginBoxSelect  = true;		// 矩形範囲選択中
		m_bBeginLineSelect = false;		// 行単位選択中
		m_bBeginWordSelect = false;		// 単語単位選択中
	}

	// 謎の選択開始
	void SelectBeginNazo() {
		m_bBeginSelect     = true;		// 範囲選択中
//		m_bBeginBoxSelect  = false;		// 矩形範囲選択中でない
		m_bBeginLineSelect = false;		// 行単位選択中
		m_bBeginWordSelect = false;		// 単語単位選択中
	}

	// 範囲選択終了
	void SelectEnd() {
		m_bBeginSelect = false;
	}

	// m_bBeginBoxSelectを設定。
	void SetBoxSelect(bool b) {
		m_bBeginBoxSelect = b;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           描画                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void DrawSelectArea(bool bDrawBracketCursorLine = true);		// 指定行の選択領域の描画
private:
	void DrawSelectArea2(HDC) const;	// 指定範囲の選択領域の描画
	void DrawSelectAreaLine(			// 指定行の選択領域の描画
		HDC					hdc,		// [in] 描画領域のDevice Context Handle
		LayoutInt			nLineNum,	// [in] 描画対象行(レイアウト行)
		const LayoutRange&	range		// [in] 選択範囲(レイアウト単位)
	) const;
public:
	void GetSelectAreaLineFromRange(LayoutRange& ret, LayoutInt nLineNum, const Layout* pLayout, const LayoutRange& range) const;
	void GetSelectAreaLine(LayoutRange& ret, LayoutInt nLineNum, const Layout* pLayout) const {
		GetSelectAreaLineFromRange(ret, nLineNum, pLayout, m_select);
	}
	LayoutRange GetSelectAreaLine(LayoutInt nLineNum, const Layout* pLayout) const {
		LayoutRange ret;
		GetSelectAreaLineFromRange(ret, nLineNum, pLayout, m_select);
		return ret;
	}
	// 選択情報データの作成	2005.07.09 genta
	void PrintSelectionInfoMsg() const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         状態取得                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// テキストが選択されているか
	// 2002/03/29 Azumaiya インライン関数化
	bool IsTextSelected() const {
		return m_select.IsValid();
//		return 0 != (
//			~((DWORD)(m_sSelect.nLineFrom | m_sSelect.nLineTo | m_sSelect.nColumnFrom | m_sSelect.nColumnTo)) >> 31
//			);
	}

	// テキストの選択中か
	// 2002/03/29 Azumaiya インライン関数化
	bool IsTextSelecting() const {
		// ジャンプ回数を減らして、一気に判定。
		return m_bSelectingLock || IsTextSelected();
	}

	// マウスで選択中か
	bool IsMouseSelecting() const {
		return m_bBeginSelect;
	}

	// 矩形選択中か
	bool IsBoxSelecting() const {
		return m_bBeginBoxSelect;
	}

private:
	// 参照
	EditView&	m_editView;

public:

	bool	m_bDrawSelectArea;		// 選択範囲を描画したか	// 02/12/13 ai

	// 選択状態
	bool	m_bSelectingLock;		// 選択状態のロック
private:
	bool	m_bBeginSelect;			// 範囲選択中
	bool	m_bBeginBoxSelect;		// 矩形範囲選択中
	bool	m_bSelectAreaChanging;	// 選択範囲変更中
	int		m_nLastSelectedByteLen;	// 前回選択時の選択バイト数

public:
	bool	m_bBeginLineSelect;		// 行単位選択中
	bool	m_bBeginWordSelect;		// 単語単位選択中

	// 選択範囲を保持するための変数群
	// これらはすべて折り返し行と、折り返し桁を保持している。
	LayoutRange m_selectBgn; // 範囲選択(原点)
	LayoutRange m_select;    // 範囲選択
	LayoutRange m_selectOld; // 範囲選択Old

	Point	m_ptMouseRollPosOld;	// マウス範囲選択前回位置(XY座標)
};

/*
m_sSelectOldについて
	DrawSelectArea()に現在の選択範囲を教えて差分のみ描画するためのもの
	現在の選択範囲をOldへコピーした上で新しい選択範囲をSelectに設定して
	DrawSelectArea()を呼びだすことで新しい範囲が描かれる．
*/

