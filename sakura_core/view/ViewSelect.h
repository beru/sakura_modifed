#pragma once

class EditView;

#include "basis/SakuraBasis.h"
#include "doc/layout/Layout.h"

class ViewSelect {
public:
	EditView& GetEditView() { return editView; }
	const EditView& GetEditView() const { return editView; }

public:
	ViewSelect(EditView& editView);
	void CopySelectStatus(ViewSelect* pSelect) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      選択範囲の変更                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void DisableSelectArea(bool bDraw, bool bDrawBracketCursorLine = true); // 現在の選択範囲を非選択状態に戻す

	void BeginSelectArea(const Point* po = nullptr);					// 現在のカーソル位置から選択を開始する
	void ChangeSelectAreaByCurrentCursor(const Point& ptCaretPos);	// 現在のカーソル位置によって選択範囲を変更
	void ChangeSelectAreaByCurrentCursorTEST(
		const Point& ptCaretPos,
		Range* pSelect);	// 現在のカーソル位置によって選択範囲を変更

	// 選択範囲を指定する(原点未選択)
	void SetSelectArea(const Range& range) {
		selectBgn.Set(range.GetFrom());
		select = range;
	}

	// 単語選択開始
	void SelectBeginWord() {
		bBeginSelect     = true;			// 範囲選択中
		bBeginBoxSelect  = false;			// 矩形範囲選択中でない
		bBeginLineSelect = false;			// 行単位選択中
		bBeginWordSelect = true;			// 単語単位選択中
	}

	// 矩形選択開始
	void SelectBeginBox() {
		bBeginSelect     = true;		// 範囲選択中
		bBeginBoxSelect  = true;		// 矩形範囲選択中
		bBeginLineSelect = false;		// 行単位選択中
		bBeginWordSelect = false;		// 単語単位選択中
	}

	// 謎の選択開始
	void SelectBeginNazo() {
		bBeginSelect     = true;		// 範囲選択中
//		bBeginBoxSelect  = false;		// 矩形範囲選択中でない
		bBeginLineSelect = false;		// 行単位選択中
		bBeginWordSelect = false;		// 単語単位選択中
	}

	// 範囲選択終了
	void SelectEnd() {
		bBeginSelect = false;
	}

	// bBeginBoxSelectを設定。
	void SetBoxSelect(bool b) {
		bBeginBoxSelect = b;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           描画                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void DrawSelectArea(bool bDrawBracketCursorLine = true);		// 指定行の選択領域の描画
private:
	void DrawSelectArea2(HDC) const;	// 指定範囲の選択領域の描画
	void DrawSelectAreaLine(			// 指定行の選択領域の描画
		HDC hdc,				// [in] 描画領域のDevice Context Handle
		int nLineNum,			// [in] 描画対象行(レイアウト行)
		const Range& range		// [in] 選択範囲(レイアウト単位)
	) const;
public:
	void GetSelectAreaLineFromRange(
		Range& ret,
		int nLineNum,
		const Layout* pLayout,
		const Range& range) const;
	void GetSelectAreaLine(Range& ret, int nLineNum, const Layout* pLayout) const {
		GetSelectAreaLineFromRange(ret, nLineNum, pLayout, select);
	}
	Range GetSelectAreaLine(int nLineNum, const Layout* pLayout) const {
		Range ret;
		GetSelectAreaLineFromRange(ret, nLineNum, pLayout, select);
		return ret;
	}
	// 選択情報データの作成
	void PrintSelectionInfoMsg() const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         状態取得                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// テキストが選択されているか
	bool IsTextSelected() const {
		return select.IsValid();
//		return 0 != (
//			~((DWORD)(sSelect.nLineFrom | m_sSelect.nLineTo | m_sSelect.nColumnFrom | m_sSelect.nColumnTo)) >> 31
//			);
	}

	// テキストの選択中か
	bool IsTextSelecting() const {
		// ジャンプ回数を減らして、一気に判定。
		return bSelectingLock || IsTextSelected();
	}

	// マウスで選択中か
	bool IsMouseSelecting() const {
		return bBeginSelect;
	}

	// 矩形選択中か
	bool IsBoxSelecting() const {
		return bBeginBoxSelect;
	}

private:
	// 参照
	EditView&	editView;

public:

	bool	bDrawSelectArea;		// 選択範囲を描画したか

	// 選択状態
	bool	bSelectingLock;		// 選択状態のロック
private:
	bool	bBeginSelect;			// 範囲選択中
	bool	bBeginBoxSelect;		// 矩形範囲選択中
	bool	bSelectAreaChanging;	// 選択範囲変更中
	size_t	nLastSelectedByteLen;	// 前回選択時の選択バイト数

public:
	bool	bBeginLineSelect;		// 行単位選択中
	bool	bBeginWordSelect;		// 単語単位選択中

	// 選択範囲を保持するための変数群
	// これらはすべて折り返し行と、折り返し桁を保持している。
	Range selectBgn; // 範囲選択(原点)
	Range select;    // 範囲選択
	Range selectOld; // 範囲選択Old

	Point	ptMouseRollPosOld;	// マウス範囲選択前回位置(XY座標)
};

/*
sSelectOldについて
	DrawSelectArea()に現在の選択範囲を教えて差分のみ描画するためのもの
	現在の選択範囲をOldへコピーした上で新しい選択範囲をSelectに設定して
	DrawSelectArea()を呼びだすことで新しい範囲が描かれる．
*/

