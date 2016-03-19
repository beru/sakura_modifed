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
		m_nMode = nMode;
		m_Ext = Ext;
		m_nFlags = flags | FA_FROMMACRO;
		m_nIsMatch = 0;
		m_source = Source;
		m_nIndex = INVALID_MACRO_IDX;
		if (nMode == MACRO_MODE_EXEC) {
			// 呼び出しの直前で設定されている番号を保存する
			m_nIndex = EditApp::getInstance().m_pSMacroMgr->GetCurrentIdx();
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
			// ID									関数名						引数										戻り値の型	m_pszData
			{ EFunctionCode(F_MA_SET_MATCH),		LTEXT("SetMatch"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_I4,	NULL },	// flagsを取得する
			// 終端
			{ F_INVALID, NULL, { VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY }, VT_EMPTY, NULL }
		};
		return macroFuncInfoArr;
	}

	// 関数情報を取得する
	MacroFuncInfoArray GetMacroFuncInfo() const {
		static MacroFuncInfo macroFuncInfoNotCommandArr[] = {
			// ID									関数名						引数										戻り値の型	m_pszData
			{ EFunctionCode(F_MA_GET_MODE),			LTEXT("GetMode"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_I4,	NULL },	// モードを取得する
			{ EFunctionCode(F_MA_GET_FLAGS),		LTEXT("GetFlags"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_I4,	NULL },	// flagsを取得する
			{ EFunctionCode(F_MA_GET_EXT),			LTEXT("GetExt"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_BSTR,	NULL },	// Extを取得する
			{ EFunctionCode(F_MA_GET_SOURCE),		LTEXT("GetSource"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_BSTR,	NULL },	// Sourceを取得する
			{ EFunctionCode(F_MA_GET_INDEX),		LTEXT("GetIndex"),			{ VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY },	VT_I4,	NULL },	// マクロIndexを取得する
			// 終端
			{ F_INVALID, NULL, { VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY }, VT_EMPTY, NULL }
		};
		return macroFuncInfoNotCommandArr;
	}

	// 関数を処理する
	virtual
	bool HandleFunction(
		EditView*		View,
		EFunctionCode	ID,
		const VARIANT*	Arguments,
		const int		ArgSize,
		VARIANT&		Result
		)
	{
		switch (LOWORD(ID)) {
		case F_MA_GET_MODE:
			{
				Wrap(&Result)->Receive(m_nMode);
			}
			return true;
		case F_MA_GET_FLAGS:	// flagsを取得する
			{
				Wrap(&Result)->Receive(m_nFlags);
			}
			return true;
		case F_MA_GET_EXT:
			{
				SysString S(m_Ext.c_str(), m_Ext.length());
				Wrap(&Result)->Receive(S);
			}
			return true;
		case F_MA_GET_SOURCE:
			{
				SysString S(m_source.c_str(), m_source.length());
				Wrap(&Result)->Receive(S);
			}
			return true;
		case F_MA_GET_INDEX:
			{
				Wrap(&Result)->Receive(m_nIndex);
				//Wrap(&Result)->Receive(CEditApp::getInstance()->m_pSMacroMgr->GetCurrentIdx());
			}
			return true;
		}
		return false;
	}

	// コマンドを処理する
	virtual
	bool HandleCommand(
		EditView*		View,
		EFunctionCode	ID,
		const WCHAR*	Arguments[],
		const int		ArgLengths[],
		const int		ArgSize
		)
	{
		switch (LOWORD(ID)) {
		case F_MA_SET_MATCH:
			if (Arguments[0]) {
				m_nIsMatch = _wtol(Arguments[0]);
			}
			return true;
		}
		return false;
	}

	bool IsMatch() {
		return (m_nIsMatch != 0);
	}

	void SetMatch(const int nMatch) {
		m_nIsMatch = nMatch;
	}

	// メンバ変数
public:
	tagModeID m_nMode;
	int m_nIsMatch;
	int m_nFlags;
	std::wstring m_Ext;
	std::wstring m_source;
	int m_nIndex;
};

