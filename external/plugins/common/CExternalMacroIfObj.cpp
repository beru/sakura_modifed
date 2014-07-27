/*!	@file
	@brief マクロプラグインクラス
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

#include "stdafx.h"
#include "CExternalMacroIfObj.h"

///////////////////////////////////////////////////////////////////////////////
//Command
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/*!	F_MA_SET_MATCH
	拡張子が一致したことをセットする。
	@param[in]	arg1	0:拡張子不一致, 1:拡張子一致
*/
void CExternalMacroIfObj::SetMatch(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_MA_SET_MATCH, Arguments, ArgLengths, 1);
}

///////////////////////////////////////////////////////////////////////////////
//Function
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/*	F_MA_GET_MODE
	マクロ実行モードを取得する。
	@retval	0	拡張子確認要求　拡張子が一致したらSetMatch(1)を行う。
	@retval	1	マクロ実行要求
*/
int CExternalMacroIfObj::GetMode()
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_MA_GET_MODE, Arguments, 0, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
/*!	F_MA_GET_FLAGS
	@return	マクロ実行時のIDマスク値
	通常はEditorプラグイン内で自動的にセットしてくれるため何もしなくてよい。
*/
int CExternalMacroIfObj::GetFlags()
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_MA_GET_FLAGS, Arguments, 0, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
/*!	F_MA_GET_EXT
	@return	実行マクロの拡張子
*/
WideString CExternalMacroIfObj::GetExt()
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_MA_GET_EXT, Arguments, 0, &Result)){
		result = (LPCTSTR)Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
/*!	F_MA_GET_SOURCE
	@return	実行マクロソースコード
*/
WideString CExternalMacroIfObj::GetSource()
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_MA_GET_SOURCE, Arguments, 0, &Result)){
		result = (LPCTSTR)Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
/*!	F_MA_GET_INDEX
	@return	マクロ実行時のIndex値 (CSMacroMgr.hを参照)
	-1: 標準マクロ(キーマクロ)
	-2: 一時マクロ(名前を指定してマクロ実行)
	-3: 無効なマクロ
	0～: マクロ番号
*/
int CExternalMacroIfObj::GetIndex()
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_MA_GET_INDEX, Arguments, 0, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}
