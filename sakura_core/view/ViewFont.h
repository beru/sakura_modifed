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

	HFONT ChooseFontHandle(FontAttr fontAttr) const;		// �t�H���g��I��

	HFONT GetFontHan() const {
		return hFont_HAN;
	}

	const LOGFONT& GetLogfont(int FontNo = 0) const {
		return logFont;
	}

private:
	void CreateFont(const LOGFONT* plf);
	void DeleteFont();

	HFONT	hFont_HAN;			// ���݂̃t�H���g�n���h��
	HFONT	hFont_HAN_BOLD;		// ���݂̃t�H���g�n���h��(����)
	HFONT	hFont_HAN_UL;			// ���݂̃t�H���g�n���h��(����)
	HFONT	hFont_HAN_BOLD_UL;	// ���݂̃t�H���g�n���h��(�����A����)

	LOGFONT	logFont;
	bool	bMiniMap;
};

