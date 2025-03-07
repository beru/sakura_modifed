#include "StdAfx.h"
#include "types/Type.h"
#include "view/colors/EColorIndexType.h"

// リッチテキスト
void CType_Rich::InitTypeConfigImp(TypeConfig& type)
{
	// 名前と拡張子
	_tcscpy(type.szTypeName, _T("リッチテキスト"));
	_tcscpy(type.szTypeExts, _T("rtf"));

	// 設定
	type.eDefaultOutline = OutlineType::Text;				// アウトライン解析方法
	type.nKeywordSetIdx[0]  = 15;							// キーワードセット
	type.colorInfoArr[COLORIDX_DIGIT].bDisp = true;		// 半角数値を色分け表示
	type.colorInfoArr[COLORIDX_SSTRING].bDisp = false;	// シングルクォーテーション文字列を色分け表示しない
	type.colorInfoArr[COLORIDX_WSTRING].bDisp = false;	// ダブルクォーテーション文字列を色分け表示しない
	type.colorInfoArr[COLORIDX_URL].bDisp = false;		// URLにアンダーラインを引かない
}

const wchar_t* g_ppszKeywordsRTF[] = {
	L"\\ansi",
	L"\\b",
	L"\\bin",
	L"\\box",
	L"\\brdrb",
	L"\\brdrbar",
	L"\\brdrdb",
	L"\\brdrdot",
	L"\\brdrl",
	L"\\brdrr",
	L"\\brdrs",
	L"\\brdrsh",
	L"\\brdrt",
	L"\\brdrth",
	L"\\cell",
	L"\\cellx",
	L"\\cf",
	L"\\chftn",
	L"\\clmgf",
	L"\\clmrg",
	L"\\colortbl",
	L"\\deff",
	L"\\f",
	L"\\fi",
	L"\\field",
	L"\\fldrslt",
	L"\\fonttbl",
	L"\\footnote",
	L"\\fs",
	L"\\i",
	L"\\intbl",
	L"\\keep",
	L"\\keepn",
	L"\\li",
	L"\\line",
	L"\\mac",
	L"\\page",
	L"\\par",
	L"\\pard",
	L"\\pc",
	L"\\pich",
	L"\\pichgoal",
	L"\\picscalex",
	L"\\picscaley",
	L"\\pict",
	L"\\picw",
	L"\\picwgoal",
	L"\\plain",
	L"\\qc",
	L"\\ql",
	L"\\qr",
	L"\\ri",
	L"\\row",
	L"\\rtf",
	L"\\sa",
	L"\\sb",
	L"\\scaps",
	L"\\sect",
	L"\\sl",
	L"\\strike",
	L"\\tab",
	L"\\tqc",
	L"\\tqr",
	L"\\trgaph",
	L"\\trleft",
	L"\\trowd",
	L"\\trqc",
	L"\\trql",
	L"\\tx",
	L"\\ul",
	L"\\uldb",
	L"\\v",
	L"\\wbitmap",
	L"\\wbmbitspixel",
	L"\\wbmplanes",
	L"\\wbmwidthbytes",
	L"\\wmetafile",
	L"bmc",
	L"bml",
	L"bmr",
	L"emc",
	L"eml",
	L"emr"
};
size_t g_nKeywordsRTF = _countof(g_ppszKeywordsRTF);

