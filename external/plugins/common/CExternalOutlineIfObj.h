/*!	@file
	@brief アウトラインプラグインクラス
	
	@date 2014.02.08	SetLabel追加
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
#ifndef CEXTERNALOUTLINEIFOBJ_H_366C3BF9_4627_4DA3_A802_C684C5E6FF98
#define CEXTERNALOUTLINEIFOBJ_H_366C3BF9_4627_4DA3_A802_C684C5E6FF98

#include <Windows.h>
#include "CExternalIfObj.h"

///////////////////////////////////////////////////////////////////////////////
/*
	プラグイン情報にアクセスする
*/
class CExternalOutlineIfObj : public CExternalIfObj
{
public:
	CExternalOutlineIfObj(){}
	virtual ~CExternalOutlineIfObj(){}

public:
	virtual LPCWSTR GetName(){
		return L"Outline";
	}

public:	//COutlineIfObj
	enum FuncId {
		F_OL_COMMAND_FIRST = 0,					//↓コマンドは以下に追加する
		F_OL_ADDFUNCINFO,						//アウトライン解析に追加する
		F_OL_ADDFUNCINFO2,						//アウトライン解析に追加する（深さ指定）
		F_OL_SETTITLE,							//アウトラインダイアログタイトルを指定
		F_OL_SETLISTTYPE,						//アウトラインリスト種別を指定
		F_OL_SETLABEL,							//ラベル文字列を指定
		F_OL_FUNCTION_FIRST = F_FUNCTION_FIRST	//↓関数は以下に追加する
	};

	//Command
	void AddFuncInfo(const int arg1, const int arg2, LPCWSTR arg3, const int arg4);
	void AddFuncInfo2(const int arg1, const int arg2, LPCWSTR arg3, const int arg4);
	void SetTitle(LPCWSTR arg1);
	void SetListType(const int arg1);
	void SetLabel(const int arg1, LPCWSTR arg2);
	//Function

	//Aliases
	void AddFuncInfo(const int arg1, const int arg2, const WideString& arg3, const int arg4);
	void AddFuncInfo2(const int arg1, const int arg2, const WideString& arg3, const int arg4);
	void SetTitle(const WideString& arg1);
	void SetLabel(const int arg1, const WideString& arg2);
};

#endif	//CEXTERNALOUTLINEIFOBJ_H_366C3BF9_4627_4DA3_A802_C684C5E6FF98
