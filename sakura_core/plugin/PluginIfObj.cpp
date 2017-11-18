/*!	@file
	@brief Pluginオブジェクト
*/
#include "stdafx.h"
#include "plugin/PluginIfObj.h"

// コマンド情報
MacroFuncInfo PluginIfObj::macroFuncInfoCommandArr[] = {
	// ID									関数名							引数										戻り値の型	pszData
	{EFunctionCode(F_PL_SETOPTION),			LTEXT("SetOption"),				{VT_BSTR, VT_BSTR, VT_VARIANT, VT_EMPTY},	VT_EMPTY,	NULL }, // オプションファイルに値を書く
	{EFunctionCode(F_PL_ADDCOMMAND),		LTEXT("AddCommand"),			{VT_BSTR, VT_BSTR, VT_BSTR, VT_EMPTY},		VT_EMPTY,	NULL }, // コマンドを追加する
	// 終端
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

// 関数情報
MacroFuncInfo PluginIfObj::macroFuncInfoArr[] = {
	// ID									関数名							引数										戻り値の型	pszData
	{EFunctionCode(F_PL_GETPLUGINDIR),		LTEXT("GetPluginDir"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, // プラグインフォルダパスを取得する
	{EFunctionCode(F_PL_GETDEF),			LTEXT("GetDef"),				{VT_BSTR, VT_BSTR, VT_EMPTY, VT_EMPTY},		VT_BSTR,	NULL }, // 設定ファイルから値を読む
	{EFunctionCode(F_PL_GETOPTION),			LTEXT("GetOption"),				{VT_BSTR, VT_BSTR, VT_EMPTY, VT_EMPTY},		VT_BSTR,	NULL }, // オプションファイルから値を読む
	{EFunctionCode(F_PL_GETCOMMANDNO),		LTEXT("GetCommandNo"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // オプションファイルから値を読む
	{EFunctionCode(F_PL_GETSTRING),			LTEXT("GetString"),				{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, // 設定ファイルから文字列を読む
	// 終端
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

