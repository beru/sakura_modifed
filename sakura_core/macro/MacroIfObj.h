/*!	@file
	@brief マクロオブジェクトクラス
*/
/*
	Copyright (C) 2013, Plugins developers

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

#include "macro/WSHIfObj.h"
#include "EditApp.h"

// マクロWSHオブジェクト
class MacroIfObj : public WSHIfObj {
	// 型定義
	enum FuncId {
		F_MA_COMMAND_FIRST = 0,					//↓コマンドは以下に追加する
		F_MA_SET_MATCH,							//
		F_MA_FUNCTION_FIRST = F_FUNCTION_FIRST,	//↓関数は以下に追加する
		F_MA_GET_MODE,							// モードを取得する
		F_MA_GET_FLAGS,							// flagsを取得する
		F_MA_GET_EXT,							// Extを取得する
		F_MA_GET_SOURCE,						// Sourceを取得する
		F_MA_GET_INDEX,							// マクロインデックス番号を取得する
	};
	typedef std::string string;
	typedef std::wstring wstring;

public:
	enum tagModeID {
		MACRO_MODE_CREATOR = 0,
		MACRO_MODE_EXEC
	};

	// コンストラクタ
public:
	MacroIfObj(tagModeID nMode, LPCWSTR Ext, int flags, LPCWSTR Source)
		: WSHIfObj(L"Macro", false)
	{
		nMode = nMode;
		ext = Ext;
		nFlags = flags | FA_FROMMACRO;
		nIsMatch = 0;
		source = Source;
		nIndex = INVALID_MACRO_IDX;
		if (nMode == MACRO_MODE_EXEC) {
			// 呼び出しの直前で設定されている番号を保存する
			nIndex = EditApp::getInstance().pSMacroMgr->GetCurrentIdx();
		}
	}

	// デストラクタ
public:
	virtual ~MacroIfObj() {}

	// 実装
public:
	// コマンド情報を取得する
	MacroFuncInfoArray GetMacroCommandInfo() const {
		static MacroFuncInfo macroFuncInfoArr[] = {
			// index									関数名						引数										戻り値の型	pszData
			{ EFunctionCode(F_MA_SET_MATCH),		LTEXT("SetMatch"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_I4,	nullptr },	// flagsを取得する
			// 終端
			{ F_INVALID, NULL, { VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY }, VT_EMPTY, nullptr }
		};
		return macroFuncInfoArr;
	}

	// 関数情報を取得する
	MacroFuncInfoArray GetMacroFuncInfo() const {
		static MacroFuncInfo macroFuncInfoNotCommandArr[] = {
			// index									関数名						引数										戻り値の型	pszData
			{ EFunctionCode(F_MA_GET_MODE),			LTEXT("GetMode"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_I4,	nullptr },	// モードを取得する
			{ EFunctionCode(F_MA_GET_FLAGS),		LTEXT("GetFlags"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_I4,	nullptr },	// flagsを取得する
			{ EFunctionCode(F_MA_GET_EXT),			LTEXT("GetExt"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_BSTR,	nullptr },	// Extを取得する
			{ EFunctionCode(F_MA_GET_SOURCE),		LTEXT("GetSource"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_BSTR,	nullptr },	// Sourceを取得する
			{ EFunctionCode(F_MA_GET_INDEX),		LTEXT("GetIndex"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_I4,	nullptr },	// マクロIndexを取得する
			// 終端
			{ F_INVALID, NULL, { VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY }, VT_EMPTY, nullptr }
		};
		return macroFuncInfoNotCommandArr;
	}

	// 関数を処理する
	virtual
	bool HandleFunction(
		EditView&		view,
		EFunctionCode	index,
		const VARIANT*	arguments,
		const int		argSize,
		VARIANT&		result
		)
	{
		switch (LOWORD(index)) {
		case F_MA_GET_MODE:
			{
				Wrap(&result)->Receive(nMode);
			}
			return true;
		case F_MA_GET_FLAGS:	// flagsを取得する
			{
				Wrap(&result)->Receive(nFlags);
			}
			return true;
		case F_MA_GET_EXT:
			{
				SysString s(ext.c_str(), ext.length());
				Wrap(&result)->Receive(s);
			}
			return true;
		case F_MA_GET_SOURCE:
			{
				SysString s(source.c_str(), source.length());
				Wrap(&result)->Receive(s);
			}
			return true;
		case F_MA_GET_INDEX:
			{
				Wrap(&result)->Receive(nIndex);
				//Wrap(&result)->Receive(CEditApp::getInstance()->pSMacroMgr->GetCurrentIdx());
			}
			return true;
		}
		return false;
	}

	// コマンドを処理する
	virtual
	bool HandleCommand(
		EditView&		view,
		EFunctionCode	index,
		const wchar_t*	arguments[],
		const int		argLengths[],
		const int		argSize
		)
	{
		switch (LOWORD(index)) {
		case F_MA_SET_MATCH:
			if (arguments[0]) {
				nIsMatch = _wtol(arguments[0]);
			}
			return true;
		}
		return false;
	}

	bool IsMatch() {
		return (nIsMatch != 0);
	}

	void SetMatch(const int nMatch) {
		nIsMatch = nMatch;
	}

	// メンバ変数
public:
	tagModeID nMode;
	int nIsMatch;
	int nFlags;
	std::wstring ext;
	std::wstring source;
	int nIndex;
};

