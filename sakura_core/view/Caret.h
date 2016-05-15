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

#define _CARETMARGINRATE 20
class TextArea;
class EditView;
class EditDoc;
class TextMetrics;
class Caret;
class EditWnd;

class CaretUnderLine {
public:
	CaretUnderLine(EditView& editView)
		:
		editView(editView)
	{
		nLockCounter = 0;
		nUnderLineLockCounter = 0;
	}
	// 表示非表示を切り替えられないようにする
	void Lock() {
		++nLockCounter;
	}
	// 表示非表示を切り替えられるようにする
	void UnLock() {
		--nLockCounter;
		if (nLockCounter < 0) {
			nLockCounter = 0;
		}
	}
	void UnderLineLock() {
		++nUnderLineLockCounter;
	}
	// 表示非表示を切り替えられるようにする
	void UnderLineUnLock() {
		--nUnderLineLockCounter;
		if (nUnderLineLockCounter < 0) {
			nUnderLineLockCounter = 0;
		}
	}
	void CaretUnderLineON(bool, bool);	// カーソル行アンダーラインのON
	void CaretUnderLineOFF(bool, bool = true, bool = false);	// カーソル行アンダーラインのOFF
	void SetUnderLineDoNotOFF(bool flag) { if (!nLockCounter) bUnderLineDoNotOFF = flag; }
	void SetVertLineDoNotOFF(bool flag) { if (!nLockCounter) bVertLineDoNotOFF = flag; }
	inline bool GetUnderLineDoNotOFF()const { return bUnderLineDoNotOFF; }
	inline bool GetVertLineDoNotOFF()const { return bVertLineDoNotOFF; }
private:
	// ロックカウンタ。0のときは、ロックされていない。UnLockが呼ばれすぎても負にはならない
	int nLockCounter;
	int nUnderLineLockCounter;
	EditView& editView;
	bool bUnderLineDoNotOFF;
	bool bVertLineDoNotOFF;
};


class Caret {
public:
	Caret(EditView& editView, const EditDoc& editDoc);
	
	virtual
	~Caret();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         外部依存                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	int GetHankakuDx() const;
	int GetHankakuDy() const;
	int GetHankakuHeight() const;

	// ドキュメントのインスタンスを求める
	const EditDoc& GetDocument() const { return editDoc; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         実装補助                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	POINT CalcCaretDrawPos(const LayoutPoint& ptCaretPos) const;


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                   初期化・終了処理など                      //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// キャレットの作成。2006.12.07 ryoji
	void CreateEditCaret(
		COLORREF crCaret,
		COLORREF crBack,
		int nWidth,
		int nHeight
	);
	
	// キャレットを破棄する（内部的にも破棄）
	void DestroyCaret() {
		::DestroyCaret();
		sizeCaret.cx = 0;
	}

	// コピー
	void CopyCaretStatus(Caret* pDestCaret) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           移動                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// 設定
	LayoutInt MoveCursorToClientPoint(const POINT& ptClientPos, bool = false, LayoutPoint* = nullptr);		// マウス等による座標指定によるカーソル移動
	LayoutInt Cursor_UPDOWN(LayoutInt nMoveLines, bool bSelect);		// カーソル上下移動処理
	LayoutInt MoveCursor(												// 行桁指定によるカーソル移動
		LayoutPoint		ptWk_CaretPos,									// [in] 移動先レイアウト位置
		bool			bScroll,										// [in] true: 画面位置調整有り  false: 画面位置調整無し
		int				nCaretMarginRate	= _CARETMARGINRATE,			// [in] 縦スクロール開始位置を決める値
		bool			bUnderlineDoNotOFF	= false,					// [in] アンダーラインを消去しない
		bool			bVertLineDoNotOFF	= false						// [in] カーソル位置縦線を消去しない
	);
	LayoutInt MoveCursorFastMode(
		const LogicPoint&	pptWk_CaretPosLogic							// [in] 移動先ロジック位置
	);
	LayoutInt MoveCursorProperly(LayoutPoint ptNewXY, bool, bool = false, LayoutPoint* = nullptr, int = _CARETMARGINRATE, int = 0);	// 行桁指定によるカーソル移動（座標調整付き）

	//$ 設計思想的に微妙
	void SetCaretLayoutPos(const LayoutPoint& pt) { ptCaretPos_Layout = pt; }	// キャレット位置(レイアウト)を設定
	void SetCaretLogicPos(const LogicPoint pt) { ptCaretPos_Logic = pt; }		// キャレット位置(ロジック)を設定

	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        サイズ変更                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void SetCaretSize(int nW, int nH) { sizeCaret.Set(nW, nH); }						// キャレットサイズを設定

	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           計算                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 計算
	bool GetAdjustCursorPos(LayoutPoint* pptPosXY); // 正しいカーソル位置を算出する

	void ClearCaretPosInfoCache();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           表示                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// 描画？
	void ShowEditCaret();    // キャレットの表示・更新
	void ShowCaretPosInfo(); // キャレットの行桁位置を表示する

	// API呼び出し
	void ShowCaret_(HWND hwnd);
	void HideCaret_(HWND hwnd);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           取得                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	LayoutPoint GetCaretLayoutPos() const	{ return ptCaretPos_Layout; }	// キャレット位置(レイアウト)を取得
	Size GetCaretSize() const				{ return sizeCaret; }			// キャレットサイズを取得。※正確には高さは違うらしい (この半分のこともある？)
	bool ExistCaretFocus() const			{ return sizeCaret.cx > 0; }	// キャレットのフォーカスがあるか。※横幅値で判定してるらしい。
	LogicPoint GetCaretLogicPos() const		{ return ptCaretPos_Logic; }	// キャレット位置(ロジック)を取得


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                  低頻度インターフェース                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	bool GetCaretShowFlag() const { return bCaretShowFlag; }


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        メンバ変数                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
private:
	// 参照
	EditView&		editView;
	const EditDoc&	editDoc;

	// キャレット位置
	LayoutPoint	ptCaretPos_Layout;		// ビュー左上端からのカーソル位置。レイアウト単位。
	LogicPoint	ptCaretPos_Logic;		// カーソル位置。ロジック単位。データ内文字単位。

	// カーソル位置計算キャッシュ
	LayoutInt nOffsetCache;
	LayoutInt nLineNoCache;
	LogicInt  nLogicOffsetCache;
	LogicInt  nLineLogicNoCache;
	LayoutInt nLineNo50Cache;
	LayoutInt nOffset50Cache;
	LogicInt  nLogicOffset50Cache;
	int nLineLogicModCache;
	
public:
	LayoutInt	nCaretPosX_Prev;	// 直前のX座標記憶用。レイアウト単位。このソースの下部に詳細説明があります。

	// キャレット見た目
private:
	Size		sizeCaret;		// キャレットのサイズ。ピクセル単位。
	COLORREF	crCaret;			// キャレットの色				// 2006.12.07 ryoji
	HBITMAP		hbmpCaret;		// キャレットのビットマップ		// 2006.11.28 ryoji
	bool		bCaretShowFlag;

	// アンダーライン
public:
	mutable CaretUnderLine underLine;
	
	bool	bClearStatus;
};


/*!	@brief Caret::nCaretPosX_Prev
	直前のX座標記憶用

	フリーカーソルモードでない場合にカーソルを上下に移動させた場合
	カーソル位置より短い行では行末にカーソルを移動するが，
	さらに移動を続けた場合に長い行で移動起点のX位置を復元できるように
	するための変数．
	
	@par 使い方
	読み出しはEditView::Cursor_UPDOWN()のみで行う．
	カーソル上下移動以外でカーソル移動を行った場合には
	直ちにnCaretPosXの値を設定する．そうしないと
	その直後のカーソル上下移動で移動前のX座標に戻ってしまう．

	ビュー左端からのカーソル桁位置(０開始)
	
	@date 2004.04.09 genta 説明文追加
*/

