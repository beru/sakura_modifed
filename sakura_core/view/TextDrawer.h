#pragma once

class TextMetrics;
class TextArea;
class ViewFont;
class Eol;
class EditView;
class Layout;
#include "DispPos.h"

class Graphics;

class TextDrawer {
public:
	TextDrawer(const EditView& editView) : editView(editView) { }
	virtual ~TextDrawer() {}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         外部依存                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 領域のインスタンスを求める
	const TextArea& GetTextArea() const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     インターフェース                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// 2007.08.25 kobake 戻り値を void に変更。引数 x, y を DispPos に変更
	// 実際には pX と nX が更新される。
	void DispText(HDC hdc, DispPos* pDispPos, const wchar_t* pData, size_t nLength, bool bTransparent = false) const; // テキスト表示

	// ノート線描画
	void DispNoteLine(Graphics& gr, int nTop, int nBottom, int nLeft, int nRight) const;

	// -- -- 指定桁縦線描画 -- -- //
	// 指定桁縦線描画関数	// 2005.11.08 Moca
	void DispVerticalLines(Graphics& gr, int nTop, int nBottom, int nLeftCol, int nRightCol) const;

	// -- -- 折り返し桁縦線描画 -- -- //
	void DispWrapLine(Graphics& gr, int nTop, int nBottom) const;

	// -- -- 行番号 -- -- //
	void DispLineNumber(Graphics& gr, int nLineNum, int y) const;		// 行番号表示

private:
	const EditView& editView;
};

