#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          色設定                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// フォント属性
struct FontAttr {
	bool		bBoldFont;		// 太字
	bool		bUnderLine;		// 下線
};

// 色属性
struct ColorAttr {
	COLORREF	cTEXT;			// 文字色
	COLORREF	cBACK;			// 背景色
};

// 色設定
struct ColorInfoBase {
	bool		bDisp;			// 表示
	FontAttr	fontAttr;		// フォント属性
	ColorAttr	colorAttr;		// 色属性
};

// 名前とインデックス付き色設定
struct ColorInfo : public ColorInfoBase {
	int			nColorIdx;		// インデックス
	TCHAR		szName[64];		// 名前
};


// デフォルト色設定
void GetDefaultColorInfo(ColorInfo* pColorInfo, int nIndex);
void GetDefaultColorInfoName(ColorInfo* pColorInfo, int nIndex);
int GetDefaultColorInfoCount();


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           辞書                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
const int DICT_ABOUT_LEN = 50; // 辞書の説明の最大長 -1
struct KeyHelpInfo {
	bool		bUse;						// 辞書を 使用する/しない
	TCHAR		szAbout[DICT_ABOUT_LEN];	// 辞書の説明(辞書ファイルの1行目から生成)
	SFilePath	szPath;					// ファイルパス
};

