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

#include "stdafx.h"
#include "CExternalOutlineIfObj.h"

///////////////////////////////////////////////////////////////////////////////
//Command
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
void CExternalOutlineIfObj::AddFuncInfo(const int arg1, const int arg2, LPCWSTR arg3, const int arg4)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	SET_INT_ARG(1, arg2);
	SET_WCHAR_ARG(2, arg3);
	SET_INT_ARG(3, arg4);
	HandleCommand(F_OL_ADDFUNCINFO, Arguments, ArgLengths, 4);
}

///////////////////////////////////////////////////////////////////////////////
void CExternalOutlineIfObj::AddFuncInfo2(const int arg1, const int arg2, LPCWSTR arg3, const int arg4)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	SET_INT_ARG(1, arg2);
	SET_WCHAR_ARG(2, arg3);
	SET_INT_ARG(3, arg4);
	HandleCommand(F_OL_ADDFUNCINFO2, Arguments, ArgLengths, 4);
}

///////////////////////////////////////////////////////////////////////////////
void CExternalOutlineIfObj::SetTitle(LPCWSTR arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_WCHAR_ARG(0, arg1);
	HandleCommand(F_OL_SETTITLE, Arguments, ArgLengths, 1);
}

///////////////////////////////////////////////////////////////////////////////
void CExternalOutlineIfObj::SetListType(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_OL_SETLISTTYPE, Arguments, ArgLengths, 1);
}

///////////////////////////////////////////////////////////////////////////////
void CExternalOutlineIfObj::SetLabel(const int arg1, LPCWSTR arg2)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	SET_WCHAR_ARG(1, arg2);
	HandleCommand(F_OL_SETLABEL, Arguments, ArgLengths, 2);
}

///////////////////////////////////////////////////////////////////////////////
//Function
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Aliases
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//Command
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
void CExternalOutlineIfObj::AddFuncInfo(const int arg1, const int arg2, const WideString& arg3, const int arg4)
{
	AddFuncInfo(arg1, arg2, arg3.c_str(), arg4);
}

///////////////////////////////////////////////////////////////////////////////
void CExternalOutlineIfObj::AddFuncInfo2(const int arg1, const int arg2, const WideString& arg3, const int arg4)
{
	AddFuncInfo2(arg1, arg2, arg3.c_str(), arg4);
}

///////////////////////////////////////////////////////////////////////////////
void CExternalOutlineIfObj::SetTitle(const WideString& arg1)
{
	SetTitle(arg1.c_str());
}

///////////////////////////////////////////////////////////////////////////////
void CExternalOutlineIfObj::SetLabel(const int arg1, const WideString& arg2)
{
	SetLabel(arg1, arg2.c_str());
}

///////////////////////////////////////////////////////////////////////////////
//Function
///////////////////////////////////////////////////////////////////////////////

