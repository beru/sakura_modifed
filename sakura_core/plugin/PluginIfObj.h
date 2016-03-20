/*!	@file
	@brief Pluginオブジェクト

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

#include "macro/WSHIfObj.h"
#include "_os/OleTypes.h"
#include "util/ole_convert.h"

// cppへ移動予定
#include "window/EditWnd.h"
#include "view/EditView.h"
#include "Plugin.h"

class PluginIfObj : public WSHIfObj {
	// 型定義
	enum FuncId {
		F_PL_COMMAND_FIRST = 0,					// ↓コマンドは以下に追加する
		F_PL_SETOPTION,							// オプションファイルに値を書く
		F_PL_ADDCOMMAND,						// コマンドを追加する
		F_PL_FUNCTION_FIRST = F_FUNCTION_FIRST,	// ↓関数は以下に追加する
		F_PL_GETPLUGINDIR,						// プラグインフォルダパスを取得する
		F_PL_GETDEF,							// 設定ファイルから値を読む
		F_PL_GETOPTION,							// オプションファイルから値を読む
		F_PL_GETCOMMANDNO,						// 実行中プラグの番号を取得する
		F_PL_GETSTRING,							// 設定ファイルから文字列を読みだす(多言語対応)
	};
	typedef std::string string;
	typedef std::wstring wstring;

	// コンストラクタ
public:
	PluginIfObj(Plugin& plugin)
		:
		WSHIfObj(L"Plugin", false),
		plugin(plugin)
	{
		m_nPlugIndex = -1;
	}

	// デストラクタ
public:
	~PluginIfObj() {}

	// 操作
public:
	void SetPlugIndex(int nIndex) { m_nPlugIndex = nIndex; }
	// 実装
public:
	// コマンド情報を取得する
	MacroFuncInfoArray GetMacroCommandInfo() const {
		return m_macroFuncInfoCommandArr;
	}
	// 関数情報を取得する
	MacroFuncInfoArray GetMacroFuncInfo() const {
		return m_macroFuncInfoArr;
	}
	// 関数を処理する
	bool HandleFunction(EditView* View, EFunctionCode ID, const VARIANT* Arguments, const int ArgSize, VARIANT& Result) {
		Variant varCopy;	// VT_BYREFだと困るのでコピー用

		switch (LOWORD(ID)) {
		case F_PL_GETPLUGINDIR:			// プラグインフォルダパスを取得する
			{
				SysString S(plugin.m_sBaseDir.c_str(), plugin.m_sBaseDir.size());
				Wrap(&Result)->Receive(S);
			}
			return true;
		case F_PL_GETDEF:				// 設定ファイルから値を読む
		case F_PL_GETOPTION:			// オプションファイルから値を読む
			{
				DataProfile profile;
				wstring sSection;
				wstring sKey;
				wstring sValue;
				if (!variant_to_wstr(Arguments[0], sSection)) {
					return false;
				}
				if (!variant_to_wstr(Arguments[1], sKey)) {
					return false;
				}

				profile.SetReadingMode();
				if (LOWORD(ID) == F_PL_GETDEF) {
					profile.ReadProfile(plugin.GetPluginDefPath().c_str());
				}else {
					profile.ReadProfile(plugin.GetOptionPath().c_str());
				}
				if (!profile.IOProfileData(sSection.c_str(), sKey.c_str(), sValue)
					&& LOWORD(ID) == F_PL_GETOPTION
				) {
					// 設定されていなければデフォルトを取得 
					for (auto it=plugin.m_options.begin(); it!=plugin.m_options.end(); ++it) {
						wstring sSectionTmp;
						wstring sKeyTmp;
						(*it)->GetKey(&sSectionTmp, &sKeyTmp);
						if (sSection == sSectionTmp && sKey == sKeyTmp) {
							sValue = (*it)->GetDefaultVal();
							break;
						}
					}
				}

				SysString S(sValue.c_str(), sValue.size());
				Wrap(&Result)->Receive(S);
			}
			return true;
		case F_PL_GETCOMMANDNO:			// 実行中プラグの番号を取得する
			{
				Wrap(&Result)->Receive(m_nPlugIndex);
			}
			return true;
		case F_PL_GETSTRING:
			{
				int num;
				if (!variant_to_int(Arguments[0], num)) {
					return false;
				}
				if (0 < num && num < MAX_PLUG_STRING) {
					std::wstring& str = plugin.m_aStrings[num];
					SysString S(str.c_str(), str.size());
					Wrap(&Result)->Receive(S);
					return true;
				}else if (num == 0) {
					std::wstring str = to_wchar(plugin.m_sLangName.c_str());
					SysString S(str.c_str(), str.size());
					Wrap(&Result)->Receive(S);
					return true;
				}
			}
		}
		return false;
	}
	// コマンドを処理する
	bool HandleCommand(EditView* View, EFunctionCode ID, const WCHAR* Arguments[], const int ArgLengths[], const int ArgSize) {
		switch (LOWORD(ID)) {
		case F_PL_SETOPTION:			// オプションファイルに値を書く
			{
				if (!Arguments[0]) return false;
				if (!Arguments[1]) return false;
				if (!Arguments[2]) return false;
				DataProfile profile;

				profile.ReadProfile(plugin.GetOptionPath().c_str());
				profile.SetWritingMode();
				wstring tmp(Arguments[2]);
				profile.IOProfileData(Arguments[0], Arguments[1], tmp);
				profile.WriteProfile(plugin.GetOptionPath().c_str(), (plugin.sName + L" プラグイン設定ファイル").c_str());
			}
			break;
		case F_PL_ADDCOMMAND:			// コマンドを追加する
			{
				int id = plugin.AddCommand(Arguments[0], Arguments[1], Arguments[2], true);
				View->m_editWnd.RegisterPluginCommand(id);
			}
			break;
		}
		return true;
	}

	// メンバ変数
public:
private:
	Plugin& plugin;
	static MacroFuncInfo m_macroFuncInfoCommandArr[];	// コマンド情報(戻り値なし)
	static MacroFuncInfo m_macroFuncInfoArr[];	// 関数情報(戻り値あり)
	int m_nPlugIndex;	// 実行中プラグの番号
};

