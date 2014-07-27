/*!	@file
	@brief 補完プラグインクラス
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
#include "CExternalComplementIfObj.h"

///////////////////////////////////////////////////////////////////////////////
//Command
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//Function
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
WideString CExternalComplementIfObj::GetCurrentWord()
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_CM_GETCURRENTWORD, Arguments, 0, &Result)){
		result = Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
int CExternalComplementIfObj::GetOption()
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_CM_GETOPTION, Arguments, 0, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
int CExternalComplementIfObj::AddList(LPCWSTR arg1)
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	SET_PARAM(Arguments[0], arg1);
	if(HandleFunction(F_CM_ADDLIST, Arguments, 1, &Result)){
		result = Result.lVal;
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
//Function
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
int CExternalComplementIfObj::AddList(const WideString& arg1)
{
	return AddList(arg1.c_str());
}
