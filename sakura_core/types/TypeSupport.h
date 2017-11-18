#pragma once

#include "uiparts/Graphics.h"
#include "doc/EditDoc.h"
#include "view/EditView.h"
#include "view/ViewFont.h"
#include "view/colors/ColorStrategy.h"

/*!タイプサポートクラス
	今のところタイプ別設定の色情報取得の補助
*/
class TypeSupport {
private:
	static const COLORREF INVALID_COLOR = 0xFFFFFFFF; // 無効な色定数

public:
	TypeSupport(const EditView& editView, EColorIndexType eColorIdx)
		:
		pFontset(&editView.GetFontset()),
		nColorIdx(ToColorInfoArrIndex(eColorIdx))
	{
		ASSERT_GE(nColorIdx, 0);
		pTypes = &editView.pEditDoc->docType.GetDocumentAttribute();
		pColorInfoArr = &pTypes->colorInfoArr[nColorIdx];

		gr = nullptr;
	}
	virtual ~TypeSupport() {
		if (gr) {
			RewindGraphicsState(*gr);
		}
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           取得                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 前景色(文字色)
	COLORREF GetTextColor() const {
		return pColorInfoArr->colorAttr.cTEXT;
	}

	// 背景色
	COLORREF GetBackColor() const {
		return pColorInfoArr->colorAttr.cBACK;
	}

	// 表示するかどうか
	bool IsDisp() const {
		return pColorInfoArr->bDisp;
	}

	// 太字かどうか
	bool IsBoldFont() const {
		return pColorInfoArr->fontAttr.bBoldFont;
	}

	// 下線を持つかどうか
	bool HasUnderLine() const {
		return pColorInfoArr->fontAttr.bUnderLine;
	}

	const ColorInfo& GetColorInfo() const {
		return *pColorInfoArr;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           描画                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void FillBack(Graphics& gr, const RECT& rc) {
		gr.FillSolidMyRect(rc, pColorInfoArr->colorAttr.cBACK);
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           設定                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	Font GetTypeFont() {
		Font font;
		font.fontAttr = pColorInfoArr->fontAttr;
		font.hFont = pFontset->ChooseFontHandle( pColorInfoArr->fontAttr );
		return font;
	}
	
	void SetGraphicsState_WhileThisObj(Graphics& gr) {
		if (this->gr) {
			RewindGraphicsState(*this->gr);
		}

		this->gr = &gr;

		// テキスト色
		gr.PushTextBackColor(GetBackColor());
		gr.PushTextForeColor(GetTextColor());

		// フォント
		gr.PushMyFont(GetTypeFont());
	}
	
	void RewindGraphicsState(Graphics& gr) {
		if (this->gr) {
			gr.PopTextBackColor();
			gr.PopTextForeColor();
			gr.PopMyFont();
			this->gr = nullptr;
		}
	}

private:
	const ViewFont*		pFontset;
	const TypeConfig*	pTypes;
	int					nColorIdx;
	const ColorInfo*	pColorInfoArr;

	Graphics* gr;    // 設定を変更したHDC
};

