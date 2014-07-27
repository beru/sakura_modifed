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

#include "stdafx.h"
#include "CExternalIfObj.h"
#include "CExternalPluginIfObj.h"
#include "SakuraMeetsPlugin.h"

///////////////////////////////////////////////////////////////////////////////
//Command
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
void CExternalPluginIfObj::SetOption(LPCWSTR arg1, LPCWSTR arg2, LPCWSTR arg3)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	SET_WCHAR_ARG(1, arg2);
	SET_WCHAR_ARG(2, arg3);
	HandleCommand(F_PL_SETOPTION, Arguments, ArgLengths, 3);
}

///////////////////////////////////////////////////////////////////////////////
void CExternalPluginIfObj::SetOption(LPCWSTR arg1, LPCWSTR arg2, const int arg3)
{
	WideString arg3tmp = TO_STRING(arg3);
	SetOption(arg1, arg2, arg3tmp.c_str());
}

///////////////////////////////////////////////////////////////////////////////
void CExternalPluginIfObj::SetOption(LPCWSTR arg1, LPCWSTR arg2, const bool arg3)
{
	SetOption(arg1, arg2, (arg3 ? 1 : 0));
}

///////////////////////////////////////////////////////////////////////////////
void CExternalPluginIfObj::AddCommand(LPCWSTR arg1, LPCWSTR arg2, LPCWSTR arg3)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	SET_WCHAR_ARG(1, arg2);
	SET_WCHAR_ARG(2, arg3);
	HandleCommand(F_PL_ADDCOMMAND, Arguments, ArgLengths, 3);
}

///////////////////////////////////////////////////////////////////////////////
//Function
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
WideString CExternalPluginIfObj::GetPluginDir()
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_PL_GETPLUGINDIR, Arguments, 0, &Result)){
		result = (LPCTSTR)Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
WideString CExternalPluginIfObj::GetDef(LPCWSTR arg1, LPCWSTR arg2)
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	SET_PARAM(Arguments[1], arg2);
	if(HandleFunction(F_PL_GETDEF, Arguments, 2, &Result)){
		result = (LPCTSTR)Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
WideString CExternalPluginIfObj::GetOption(LPCWSTR arg1, LPCWSTR arg2)
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	SET_PARAM(Arguments[1], arg2);
	if(HandleFunction(F_PL_GETOPTION, Arguments, 2, &Result)){
		result = (LPCTSTR)Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
int CExternalPluginIfObj::GetOptionInt(LPCWSTR arg1, LPCWSTR arg2)
{
	WideString strResult = GetOption(arg1, arg2);
	return _wtoi(strResult.c_str());
}

///////////////////////////////////////////////////////////////////////////////
int CExternalPluginIfObj::GetCommandNo()
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_PL_GETCOMMANDNO, Arguments, 0, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
WideString CExternalPluginIfObj::GetString(const int arg1)
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	if(HandleFunction(F_PL_GETSTRING, Arguments, 1, &Result)){
		result = (LPCTSTR)Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
// Aliases
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//Command
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
void CExternalPluginIfObj::SetOption(const WideString& arg1, const WideString& arg2, const WideString& arg3)
{
	SetOption(arg1.c_str(), arg2.c_str(), arg3.c_str());
}

///////////////////////////////////////////////////////////////////////////////
void CExternalPluginIfObj::SetOption(const WideString& arg1, const WideString& arg2, const int arg3)
{
	SetOption(arg1.c_str(), arg2.c_str(), arg3);
}

///////////////////////////////////////////////////////////////////////////////
void CExternalPluginIfObj::SetOption(const WideString& arg1, const WideString& arg2, const bool arg3)
{
	SetOption(arg1.c_str(), arg2.c_str(), arg3);
}

///////////////////////////////////////////////////////////////////////////////
void CExternalPluginIfObj::AddCommand(const WideString& arg1, const WideString& arg2, const WideString& arg3)
{
	AddCommand(arg1.c_str(), arg2.c_str(), arg3.c_str());
}

///////////////////////////////////////////////////////////////////////////////
//Function
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
WideString CExternalPluginIfObj::GetDef(const WideString& arg1, const WideString& arg2)
{
	return GetDef(arg1.c_str(), arg2.c_str());
}

///////////////////////////////////////////////////////////////////////////////
WideString CExternalPluginIfObj::GetOption(const WideString& arg1, const WideString& arg2)
{
	return GetOption(arg1.c_str(), arg2.c_str());
}

///////////////////////////////////////////////////////////////////////////////
int CExternalPluginIfObj::GetOptionInt(const WideString& arg1, const WideString& arg2)
{
	return GetOptionInt(arg1.c_str(), arg2.c_str());
}
