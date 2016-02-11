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
#include "CEditorIfObj.h"
#include "macro/CMacro.h"
#include "macro/CSMacroMgr.h"

// コマンド情報を取得する
MacroFuncInfoArray EditorIfObj::GetMacroCommandInfo() const
{
	return SMacroMgr::m_MacroFuncInfoCommandArr;
}

// 関数情報を取得する
MacroFuncInfoArray EditorIfObj::GetMacroFuncInfo() const
{
	return SMacroMgr::m_MacroFuncInfoArr;
}

// 関数を処理する
bool EditorIfObj::HandleFunction(
	EditView* View,
	EFunctionCode ID,
	const VARIANT* Arguments,
	const int ArgSize,
	VARIANT& Result
	)
{
	return Macro::HandleFunction(View, ID, Arguments, ArgSize, Result);
}

// コマンドを処理する
bool EditorIfObj::HandleCommand(
	EditView* View,
	EFunctionCode ID,
	const WCHAR* Arguments[],
	const int ArgLengths[],
	const int ArgSize
	)
{
	return Macro::HandleCommand(View, ID, Arguments, ArgLengths, ArgSize);
}

