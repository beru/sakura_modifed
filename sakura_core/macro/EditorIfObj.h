/*!	@file
	@brief Editorオブジェクト
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
	bool HandleFunction(EditView& view, EFunctionCode index, const VARIANT* arguments, const int argSize, VARIANT& result);	// 関数を処理する
	bool HandleCommand(EditView& view, EFunctionCode index, const wchar_t* arguments[], const int argLengths[], const int argSize);	// コマンドを処理する
};

