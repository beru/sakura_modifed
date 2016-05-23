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

class ViewFont;
class EditView;
class LayoutMgr;
#include "DispPos.h"

class TextArea {
public:
	TextArea(EditView& editView);
	
	virtual
	~TextArea();
	
	void CopyTextAreaStatus(TextArea* pDst) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     ビュー情報を取得                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// 表示される最初の行
	int GetViewTopLine() const {
		return nViewTopLine;
	}
	void SetViewTopLine(int nLine) {
		nViewTopLine = nLine;
	}

	// 表示域の一番左の桁
	int GetViewLeftCol() const {
		return nViewLeftCol;
	}
	void SetViewLeftCol(int nLeftCol) {
		nViewLeftCol = nLeftCol;
	}

	// 右にはみ出した最初の列を返す
	int GetRightCol() const {
		return nViewLeftCol + nViewColNum;
	}

	// 下にはみ出した最初の行を返す
	int GetBottomLine() const {
		return nViewTopLine + nViewRowNum;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                   領域を取得(ピクセル)                      //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	int GetAreaLeft() const {
		return nViewAlignLeft;
	}
	int GetAreaTop() const {
		return nViewAlignTop;
	}
	int GetAreaRight() const {
		return nViewAlignLeft + nViewCx;
	}
	int GetAreaBottom() const {
		return nViewAlignTop + nViewCy;
	}
	Rect GetAreaRect() const {
		return Rect(GetAreaLeft(), GetAreaTop(), GetAreaRight(), GetAreaBottom());
	}

	int GetAreaWidth() const {
		return nViewCx;
	}
	int GetAreaHeight() const {
		return nViewCy;
	}

	int GetTopYohaku() const {
		return nTopYohaku;
	}
	void SetTopYohaku(int nPixel) {
		nTopYohaku = nPixel;
	}
	int GetLeftYohaku() const {
		return nLeftYohaku;
	}
	void SetLeftYohaku(int nPixel) {
		nLeftYohaku = nPixel;
	}
	// 行番号の幅(余白なし)
	int GetLineNumberWidth() const {
		return nViewAlignLeft - nLeftYohaku;
	}

	// クライアントサイズ更新
	void TextArea_OnSize(
		const Size& sizeClient,		// ウィンドウのクライアントサイズ
		int nCxVScroll,				// 垂直スクロールバーの横幅
		int nCyHScroll				// 水平スクロールバーの縦幅
	);

	// 行番号表示に必要な幅を設定
	bool DetectWidthOfLineNumberArea(bool bRedraw);

	// 行番号表示に必要な桁数を計算
	int  DetectWidthOfLineNumberArea_calculate(const LayoutMgr*, bool bLayout=false) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           判定                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	bool IsRectIntersected(const RECT& rc) const {
		// rcが無効またはゼロ領域の場合はfalse
		if (rc.left >= rc.right) return false;
		if (rc.top  >= rc.bottom) return false;

		if (rc.left >= this->GetAreaRight()) return false; // 右外
		if (rc.right <= this->GetAreaLeft()) return false; // 左外
		if (rc.top >= this->GetAreaBottom()) return false; // 下外
		if (rc.bottom <= this->GetAreaTop()) return false; // 上外
		
		return true;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        その他取得                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	int GetRulerHeight() const {
		return nViewAlignTop - GetTopYohaku();
	}
	// ドキュメント左端のクライアント座標を取得 (つまり、スクロールされた状態であれば、マイナスを返す)
	int GetDocumentLeftClientPointX() const;

	// 計算
	// ! クライアント座標からレイアウト位置に変換する
	void ClientToLayout(Point ptClient, LayoutPoint* pptLayout) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           設定                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void UpdateAreaMetrics(HDC hdc);
	void SetAreaLeft(int nAreaLeft) {
		nViewAlignLeft = nAreaLeft;
	}
	void SetAreaTop(int nAreaTop) {
		nViewAlignTop = nAreaTop;
	}
	void OffsetViewTopLine(int nOff) {
		nViewTopLine += nOff;
	}
protected:
	void UpdateViewColRowNums();

public:


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         サポート                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//$ Generateなんていう大げさな名前じゃなくて、Get～で良い気がしてきた
	// クリッピング矩形を作成。表示範囲外だった場合はfalseを返す。
	void GenerateCharRect(RECT* rc, const DispPos& pos, int nHankakuNum) const;
	bool TrimRectByArea(RECT* rc) const;
	bool GenerateClipRect(RECT* rc, const DispPos& pos, int nHankakuNum) const;
	bool GenerateClipRectRight(RECT* rc, const DispPos& pos) const; // 右端まで全部
	bool GenerateClipRectLine(RECT* rc, const DispPos& pos) const;  // 行全部

	void GenerateTopRect   (RECT* rc, int nLineCount) const;
	void GenerateBottomRect(RECT* rc, int nLineCount) const;
	void GenerateLeftRect  (RECT* rc, int nColCount) const;
	void GenerateRightRect (RECT* rc, int nColCount) const;

	void GenerateLineNumberRect(RECT* rc) const;

	void GenerateTextAreaRect(RECT* rc) const;

	int GenerateYPx(int nLineNum) const;

private:
	// 参照
	EditView&	editView;

public:
	// 画面情報
	// ピクセル
private:
	int		nViewAlignLeft;		// 表示域の左端座標
	int		nViewAlignTop;		// 表示域の上端座標
private:
	int		nTopYohaku;
	int		nLeftYohaku;
private:
	int		nViewCx;				// 表示域の幅
	int		nViewCy;				// 表示域の高さ

	// テキスト
private:
	int	nViewTopLine;		// 表示域の一番上の行(0開始)
public:
	int	nViewRowNum;		// 表示域の行数

private:
	int	nViewLeftCol;		// 表示域の一番左の桁(0開始)
public:
	int	nViewColNum;		// 表示域の桁数

	// その他
	int		nViewAlignLeftCols;	// 行番号域の桁数
};

