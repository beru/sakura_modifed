/*!	@file
	@brief Editorオブジェクト

*/
/*
	Copyright (C) 2009, syat

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

