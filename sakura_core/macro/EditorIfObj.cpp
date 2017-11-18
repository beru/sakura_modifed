/*!	@file
	@brief Editorオブジェクト
*/
#include "StdAfx.h"
#include "EditorIfObj.h"
#include "macro/Macro.h"
#include "macro/SMacroMgr.h"

// コマンド情報を取得する
MacroFuncInfoArray EditorIfObj::GetMacroCommandInfo() const
{
	return SMacroMgr::macroFuncInfoCommandArr;
}

// 関数情報を取得する
MacroFuncInfoArray EditorIfObj::GetMacroFuncInfo() const
{
	return SMacroMgr::macroFuncInfoArr;
}

// 関数を処理する
bool EditorIfObj::HandleFunction(
	EditView& view,
	EFunctionCode index,
	const VARIANT* arguments,
	const int argSize,
	VARIANT& result
	)
{
	return Macro::HandleFunction(view, index, arguments, argSize, result);
}

// コマンドを処理する
bool EditorIfObj::HandleCommand(
	EditView& view,
	EFunctionCode index,
	const wchar_t* arguments[],
	const int argLengths[],
	const int argSize
	)
{
	return Macro::HandleCommand(view, index, arguments, argLengths, argSize);
}

