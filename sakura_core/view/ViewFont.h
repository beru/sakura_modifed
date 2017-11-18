#pragma once

#include "doc/DocTypeSetting.h" // ColorInfo !!

class ViewFont {
public:
	ViewFont(const LOGFONT *plf, bool bMiniMap = false) {
		this->bMiniMap = bMiniMap;
		CreateFont(plf);
	}
	virtual ~ViewFont() {
		DeleteFont();
	}

	void UpdateFont(const LOGFONT *plf) {
		DeleteFont();
		CreateFont(plf);
	}

	HFONT ChooseFontHandle(FontAttr fontAttr) const;		// フォントを選ぶ

	HFONT GetFontHan() const {
		return hFont_HAN;
	}

	const LOGFONT& GetLogfont(int FontNo = 0) const {
		return logFont;
	}

private:
	void CreateFont(const LOGFONT* plf);
	void DeleteFont();

	HFONT	hFont_HAN;			// 現在のフォントハンドル
	HFONT	hFont_HAN_BOLD;		// 現在のフォントハンドル(太字)
	HFONT	hFont_HAN_UL;			// 現在のフォントハンドル(下線)
	HFONT	hFont_HAN_BOLD_UL;	// 現在のフォントハンドル(太字、下線)

	LOGFONT	logFont;
	bool	bMiniMap;
};

