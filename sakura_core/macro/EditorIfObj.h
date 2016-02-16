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
#pragma once

#include "_os/OleTypes.h"
#include "macro/WSHIfObj.h"


class EditorIfObj : public WSHIfObj {
	// コンストラクタ
public:
	EditorIfObj() : WSHIfObj(L"Editor", true) {}

	// 実装
	MacroFuncInfoArray GetMacroCommandInfo() const;	// コマンド情報を取得する
	MacroFuncInfoArray GetMacroFuncInfo() const;	// 関数情報を取得する
	bool HandleFunction(EditView* pView, EFunctionCode id, const VARIANT* arguments, const int argSize, VARIANT& result);	// 関数を処理する
	bool HandleCommand(EditView* pView, EFunctionCode id, const WCHAR* arguments[], const int argLengths[], const int argSize);	// コマンドを処理する
};

