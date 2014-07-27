/*!	@file
	@brief プラグインプラグインクラス
*/
/*
	Copyright (C) 2013-2014, Plugins developers

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
#ifndef CEXTERNALPLUGINIFOBJ_H_6154EFC0_3DF4_4F7F_94D3_6CE00477DD2E
#define CEXTERNALPLUGINIFOBJ_H_6154EFC0_3DF4_4F7F_94D3_6CE00477DD2E

#include <Windows.h>
#include "CExternalIfObj.h"

///////////////////////////////////////////////////////////////////////////////
/*
	プラグイン情報にアクセスする
*/
class CExternalPluginIfObj : public CExternalIfObj
{
public:
	CExternalPluginIfObj(){}
	virtual ~CExternalPluginIfObj(){}

public:
	virtual LPCWSTR GetName(){
		return L"Plugin";
	}

public:	//PluginIfObj
	enum FuncId {
		F_PL_COMMAND_FIRST = 0,					//↓コマンドは以下に追加する
		F_PL_SETOPTION,							//オプションファイルに値を書く
		F_PL_ADDCOMMAND,						//コマンドを追加する
		F_PL_FUNCTION_FIRST = F_FUNCTION_FIRST,	//↓関数は以下に追加する
		F_PL_GETPLUGINDIR,						//プラグインフォルダパスを取得する
		F_PL_GETDEF,							//設定ファイルから値を読む
		F_PL_GETOPTION,							//オプションファイルから値を読む
		F_PL_GETCOMMANDNO,						//実行中プラグの番号を取得する
		F_PL_GETSTRING,							//設定ファイルから文字列を読みだす(多言語対応)	//upatchid:259
	};

	//Command
	void SetOption(LPCWSTR arg1, LPCWSTR arg2, LPCWSTR arg3);
	void SetOption(LPCWSTR arg1, LPCWSTR arg2, const int arg3);
	void SetOption(LPCWSTR arg1, LPCWSTR arg2, const bool arg3);
	void AddCommand(LPCWSTR arg1, LPCWSTR arg2, LPCWSTR arg3);
	//Function
	WideString GetPluginDir();
	WideString GetDef(LPCWSTR arg1, LPCWSTR arg2);
	WideString GetOption(LPCWSTR arg1, LPCWSTR arg2);
	int GetOptionInt(LPCWSTR arg1, LPCWSTR arg2);
	int GetCommandNo();
	WideString GetString(const int arg1);

	//Aliases
	//Command
	void SetOption(const WideString& arg1, const WideString& arg2, const WideString& arg3);
	void SetOption(const WideString& arg1, const WideString& arg2, const int arg3);
	void SetOption(const WideString& arg1, const WideString& arg2, const bool arg3);
	void AddCommand(const WideString& arg1, const WideString& arg2, const WideString& arg3);
	//Function
	WideString GetDef(const WideString& arg1, const WideString& arg2);
	WideString GetOption(const WideString& arg1, const WideString& arg2);
	int GetOptionInt(const WideString& arg1, const WideString& arg2);
};

#endif	//CEXTERNALPLUGINIFOBJ_H_6154EFC0_3DF4_4F7F_94D3_6CE00477DD2E
