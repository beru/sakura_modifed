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
//@@@ 2006.04.10 fon ADD-start
const int DICT_ABOUT_LEN = 50; // 辞書の説明の最大長 -1
struct KeyHelpInfo {
	bool		bUse;						// 辞書を 使用する/しない
	TCHAR		szAbout[DICT_ABOUT_LEN];	// 辞書の説明(辞書ファイルの1行目から生成)
	SFilePath	szPath;					// ファイルパス
};
//@@@ 2006.04.10 fon ADD-end

