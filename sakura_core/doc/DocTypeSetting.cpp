#include "StdAfx.h"
#include "DocTypeSetting.h"


// 色設定(保存用)
struct ColorInfoIni {
	int				nNameId;		// 項目名
	ColorInfoBase	colorInfo;		// 色設定
};

static ColorInfoIni ColorInfo_DEFAULT[] = {
//	項目名,									表示,		太字,		下線,		文字色,					背景色,
	{ STR_COLOR_TEXT,						{ true,		{ false,	false },	{ RGB(  0,   0,   0),	RGB(255, 251, 240) } } },
	{ STR_COLOR_RULER,						{ true,		{ false,	false },	{ RGB(  0,   0,   0),	RGB(239, 239, 239) } } },
	{ STR_COLOR_CURSOR,						{ true,		{ false,	false },	{ RGB(  0,   0,   0),	RGB(255, 251, 240) } } },
	{ STR_COLOR_CURSOR_IMEON,				{ true,		{ false,	false },	{ RGB(255,   0,   0),	RGB(255, 251, 240) } } },
	{ STR_COLOR_CURSOR_LINE_BG,				{ false,	{ false,	false },	{ RGB(  0,   0,   0),	RGB(255, 255, 128) } } },
	{ STR_COLOR_CURSOR_LINE,				{ true,		{ false,	false },	{ RGB(  0,   0, 255),	RGB(255, 251, 240) } } },
	{ STR_COLOR_CURSOR_COLUMN,				{ false,	{ false,	false },	{ RGB(128, 128, 255),	RGB(255, 251, 240) } } },
	{ STR_COLOR_NOTE_LINE,					{ false,	{ false,	false },	{ RGB( 192, 192, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_LINE_NO,					{ true,		{ false,	false },	{ RGB(  0,   0, 255),	RGB(239, 239, 239) } } },
	{ STR_COLOR_LINE_NO_CHANGE,				{ true,		{ true,		false },	{ RGB(  0,   0, 255),	RGB(239, 239, 239) } } },
	{ STR_COLOR_EVEN_LINE_BG,				{ false,	{ false,	false },	{ RGB(  0,   0,   0),	RGB(243, 243, 243) } } },
	{ STR_COLOR_TAB,						{ true,		{ false,	false },	{ RGB(128, 128, 128),	RGB(255, 251, 240) } } },
	{ STR_COLOR_HALF_SPACE,					{ false,	{ false,	false },	{ RGB(192, 192, 192),	RGB(255, 251, 240) } } },
	{ STR_COLOR_FULL_SPACE,					{ true,		{ false,	false },	{ RGB(192, 192, 192),	RGB(255, 251, 240) } } },
	{ STR_COLOR_CTRL_CODE,					{ true,		{ false,	false },	{ RGB(255, 255,   0),	RGB(255, 251, 240) } } },
	{ STR_COLOR_CR,							{ true,		{ false,	false },	{ RGB(  0, 128, 255),	RGB(255, 251, 240) } } },
	{ STR_COLOR_WRAP_MARK,					{ true,		{ false,	false },	{ RGB(255,   0, 255),	RGB(255, 251, 240) } } },
	{ STR_COLOR_VERT_LINE,					{ false,	{ false,	false },	{ RGB(192, 192, 192),	RGB(255, 251, 240) } } },
	{ STR_COLOR_EOF,						{ true,		{ false,	false },	{ RGB(  0, 255, 255),	RGB(  0,   0,   0) } } },
	{ STR_COLOR_NUMBER,						{ false,	{ false,	false },	{ RGB(235,   0,   0),	RGB(255, 251, 240) } } },
	{ STR_COLOR_BRACKET,					{ false,	{ true,		false },	{ RGB(128,   0,   0),	RGB(255, 251, 240) } } },
	{ STR_COLOR_SELECTED_AREA,				{ true,		{ false,	false },	{ RGB( 49, 106, 197),	RGB( 49, 106, 197) } } },
	{ STR_COLOR_SEARCH_WORD1,				{ true,		{ false,	false },	{ RGB(  0,   0,   0),	RGB(255, 255,   0) } } },
	{ STR_COLOR_SEARCH_WORD2,				{ true,		{ false,	false },	{ RGB(  0,   0,   0),	RGB(160, 255, 255) } } },
	{ STR_COLOR_SEARCH_WORD3,				{ true,		{ false,	false },	{ RGB(  0,   0,   0),	RGB(153, 255, 153) } } },
	{ STR_COLOR_SEARCH_WORD4,				{ true,		{ false,	false },	{ RGB(  0,   0,   0),	RGB(255, 153, 153) } } },
	{ STR_COLOR_SEARCH_WORD5,				{ true,		{ false,	false },	{ RGB(  0,   0,   0),	RGB(255, 102, 255) } } },
	{ STR_COLOR_COMMENT,					{ true,		{ false,	false },	{ RGB(  0, 128,   0),	RGB(255, 251, 240) } } },
	{ STR_COLOR_SINGLE_QUOTE,				{ true,		{ false,	false },	{ RGB( 64, 128, 128),	RGB(255, 251, 240) } } },
	{ STR_COLOR_DOUBLE_QUOTE,				{ true,		{ false,	false },	{ RGB(128,   0,  64),	RGB(255, 251, 240) } } },
	{ STR_COLOR_HERE_DOCUMENT,				{ false,	{ false,	false },	{ RGB(128,   0,  64),	RGB(255, 251, 240) } } },
	{ STR_COLOR_URL,						{ true,		{ false,	true  },	{ RGB(  0,   0, 255),	RGB(255, 251, 240) } } },
	{ STR_COLOR_KEYWORD1,					{ true,		{ false,	false },	{ RGB(  0,   0, 255),	RGB(255, 251, 240) } } },
	{ STR_COLOR_KEYWORD2,					{ true,		{ false,	false },	{ RGB(255, 128,   0),	RGB(255, 251, 240) } } },
	{ STR_COLOR_KEYWORD3,					{ true,		{ false,	false },	{ RGB(255, 128,   0),	RGB(255, 251, 240) } } },
	{ STR_COLOR_KEYWORD4,					{ true,		{ false,	false },	{ RGB(255, 128,   0),	RGB(255, 251, 240) } } },
	{ STR_COLOR_KEYWORD5,					{ true,		{ false,	false },	{ RGB(255, 128,   0),	RGB(255, 251, 240) } } },
	{ STR_COLOR_KEYWORD6,					{ true,		{ false,	false },	{ RGB(255, 128,   0),	RGB(255, 251, 240) } } },
	{ STR_COLOR_KEYWORD7,					{ true,		{ false,	false },	{ RGB(255, 128,   0),	RGB(255, 251, 240) } } },
	{ STR_COLOR_KEYWORD8,					{ true,		{ false,	false },	{ RGB(255, 128,   0),	RGB(255, 251, 240) } } },
	{ STR_COLOR_KEYWORD9,					{ true,		{ false,	false },	{ RGB(255, 128,   0),	RGB(255, 251, 240) } } },
	{ STR_COLOR_KEYWORD10,					{ true,		{ false,	false },	{ RGB(255, 128,   0),	RGB(255, 251, 240) } } },
	{ STR_COLOR_REGEX_KEYWORD1,				{ false,	{ false,	false },	{ RGB(  0,   0, 255),	RGB(255, 251, 240) } } },
	{ STR_COLOR_REGEX_KEYWORD2,				{ false,	{ false,	false },	{ RGB(  0,   0, 255),	RGB(255, 251, 240) } } },
	{ STR_COLOR_REGEX_KEYWORD3,				{ false,	{ false,	false },	{ RGB(  0,   0, 255),	RGB(255, 251, 240) } } },
	{ STR_COLOR_REGEX_KEYWORD4,				{ false,	{ false,	false },	{ RGB(  0,   0, 255),	RGB(255, 251, 240) } } },
	{ STR_COLOR_REGEX_KEYWORD5,				{ false,	{ false,	false },	{ RGB(  0,   0, 255),	RGB(255, 251, 240) } } },
	{ STR_COLOR_REGEX_KEYWORD6,				{ false,	{ false,	false },	{ RGB(  0,   0, 255),	RGB(255, 251, 240) } } },
	{ STR_COLOR_REGEX_KEYWORD7,				{ false,	{ false,	false },	{ RGB(  0,   0, 255),	RGB(255, 251, 240) } } },
	{ STR_COLOR_REGEX_KEYWORD8,				{ false,	{ false,	false },	{ RGB(  0,   0, 255),	RGB(255, 251, 240) } } },
	{ STR_COLOR_REGEX_KEYWORD9,				{ false,	{ false,	false },	{ RGB(  0,   0, 255),	RGB(255, 251, 240) } } },
	{ STR_COLOR_REGEX_KEYWORD10,			{ false,	{ false,	false },	{ RGB(  0,   0, 255),	RGB(255, 251, 240) } } },
	{ STR_COLOR_DIFF_ADD,					{ true,		{ false,	false },	{ RGB(  0,   0, 210),	RGB(162, 208, 255) } } },
	{ STR_COLOR_DIFF_CNG,					{ true,		{ false,	false },	{ RGB(  0, 111,   0),	RGB(189, 253, 192) } } },
	{ STR_COLOR_DIFF_DEL,					{ true,		{ false,	false },	{ RGB(213, 106,   0),	RGB(255, 233, 172) } } },
	{ STR_COLOR_BOOKMARK,					{ true ,	{ false,	false },	{ RGB(255, 251, 240),	RGB(  0, 128, 192) } } },
	{ STR_COLOR_PAGEVIEW,					{ true ,	{ false,	false },	{ RGB( 255, 251, 240 ),	RGB( 190, 230, 255 ) } } },
};

void GetDefaultColorInfo(ColorInfo* pColorInfo, int nIndex)
{
	assert(nIndex < _countof(ColorInfo_DEFAULT));

	ColorInfoBase* p = pColorInfo;
	*p = ColorInfo_DEFAULT[nIndex].colorInfo; // ColorInfoBase
	GetDefaultColorInfoName(pColorInfo, nIndex);
	pColorInfo->nColorIdx = nIndex;
}

void GetDefaultColorInfoName(ColorInfo* pColorInfo, int nIndex)
{
	assert(nIndex < _countof(ColorInfo_DEFAULT));
	_tcscpy(pColorInfo->szName, LS(ColorInfo_DEFAULT[nIndex].nNameId));
}

int GetDefaultColorInfoCount()
{
	return _countof(ColorInfo_DEFAULT);
}

