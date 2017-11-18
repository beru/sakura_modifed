/*!	@file
	@brief SmartIndentオブジェクト
*/
#pragma once

#include "macro/WSHIfObj.h"

// スマートインデント用WSHオブジェクト
class SmartIndentIfObj : public WSHIfObj {
	// 型定義
	enum FuncId {
		F_SI_COMMAND_FIRST = 0,					// ↓コマンドは以下に追加する
		F_SI_FUNCTION_FIRST = F_FUNCTION_FIRST,	// ↓関数は以下に追加する
		F_SI_GETCHAR							// 押下したキーを取得する
	};
	typedef std::string string;
	typedef std::wstring wstring;

	// コンストラクタ
public:
	SmartIndentIfObj(wchar_t ch)
		: WSHIfObj(L"Indent", false)
		, wcChar(ch)
	{
	}

	// デストラクタ
public:
	~SmartIndentIfObj() {}

	// 実装
public:
	// コマンド情報を取得する
	MacroFuncInfoArray GetMacroCommandInfo() const {
		static MacroFuncInfo macroFuncInfoArr[] = {
			//	終端
			{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}
		};
		return macroFuncInfoArr;
	}
	// 関数情報を取得する
	MacroFuncInfoArray GetMacroFuncInfo() const {
		static MacroFuncInfo macroFuncInfoNotCommandArr[] = {
			//index									関数名							引数										戻り値の型	pszData
			{EFunctionCode(F_SI_GETCHAR),			LTEXT("GetChar"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	nullptr }, // 押下したキーを取得する
			//	終端
			{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}
		};
		return macroFuncInfoNotCommandArr;
	}
	// 関数を処理する
	bool HandleFunction(
		EditView& view,
		EFunctionCode index,
		const VARIANT* arguments,
		const int argSize,
		VARIANT& result
		)
	{
		switch (LOWORD(index)) {
		case F_SI_GETCHAR:						// 押下したキーを取得する
			{
				wstring value;
				value += wcChar;
				SysString s(value.c_str(), value.size());
				Wrap(&result)->Receive(s);
			}
			return true;
		}
		return false;
	}
	// コマンドを処理する
	bool HandleCommand(
		EditView& view,
		EFunctionCode index,
		const wchar_t* arguments[],
		const int argLengths[],
		const int argSize
		)
	{
		return false;
	}

	// メンバ変数
public:
	wchar_t wcChar;
};

