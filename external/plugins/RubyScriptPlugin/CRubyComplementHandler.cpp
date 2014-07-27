/*!	@file
	@brief Complement Handler

	Complement Handlerを利用するためのインターフェース

	@author Plugins developers
	@date 2013.12.28
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

#include "StdAfx.h"
#include "CPluginService.h"
#include "CRubyComplementHandler.h"
#include "CRubyHandler.h"

///////////////////////////////////////////////////////////////////////////////
CRubyComplementHandler::CRubyComplementHandler()
{
}

///////////////////////////////////////////////////////////////////////////////
CRubyComplementHandler::~CRubyComplementHandler()
{
}

///////////////////////////////////////////////////////////////////////////////
VALUE CRubyComplementHandler::CommandHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize)
{
	return CRubyHandler::CommandHandler(GetIfObjType(), nFuncID, self, Arguments, nArgSize);
}

///////////////////////////////////////////////////////////////////////////////
VALUE CRubyComplementHandler::FunctionHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize)
{
	return CRubyHandler::FunctionHandler(GetIfObjType(), nFuncID, self, Arguments, nArgSize);
}

///////////////////////////////////////////////////////////////////////////////
VALUE CRubyComplementHandler::ReadyCommands(CRuby& Ruby)
{
	VALUE Complement = Ruby.rb_define_class("Complement", Ruby.rb_rcObject());
	Ruby.rb_define_singleton_method(Complement, "GetCurrentWord", reinterpret_cast<VALUE(__cdecl *)(...)>(CM_GetCurrentWord), 0);
	Ruby.rb_define_singleton_method(Complement, "GetOption", reinterpret_cast<VALUE(__cdecl *)(...)>(CM_GetOption), 0);
	Ruby.rb_define_singleton_method(Complement, "AddList", reinterpret_cast<VALUE(__cdecl *)(...)>(CM_AddList), 1);

	return Complement;
}

///////////////////////////////////////////////////////////////////////////////
//Macro
//Commands
//Functions
VALUE __cdecl CRubyComplementHandler::CM_GetCurrentWord(VALUE self){
	return FunctionHandler(CExternalComplementIfObj::F_CM_GETCURRENTWORD, self, NULL, 0);
}

VALUE __cdecl CRubyComplementHandler::CM_GetOption(VALUE self){
	return FunctionHandler(CExternalComplementIfObj::F_CM_GETOPTION, self, NULL, 0);
}

VALUE __cdecl CRubyComplementHandler::CM_AddList(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return FunctionHandler(CExternalComplementIfObj::F_CM_ADDLIST, self, args, 1);
}
