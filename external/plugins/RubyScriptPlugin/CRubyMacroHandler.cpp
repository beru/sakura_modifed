/*!	@file
	@brief Macro Handler

	Macro Handlerを利用するためのインターフェース

	@author Plugins developers
	@date 2013.12.25
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
#include "CRubyMacroHandler.h"
#include "CRubyHandler.h"

///////////////////////////////////////////////////////////////////////////////
CRubyMacroHandler::CRubyMacroHandler()
{
}

///////////////////////////////////////////////////////////////////////////////
CRubyMacroHandler::~CRubyMacroHandler()
{
}

///////////////////////////////////////////////////////////////////////////////
VALUE CRubyMacroHandler::CommandHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize)
{
	return CRubyHandler::CommandHandler(GetIfObjType(), nFuncID, self, Arguments, nArgSize);
}

///////////////////////////////////////////////////////////////////////////////
VALUE CRubyMacroHandler::FunctionHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize)
{
	return CRubyHandler::FunctionHandler(GetIfObjType(), nFuncID, self, Arguments, nArgSize);
}

///////////////////////////////////////////////////////////////////////////////
VALUE CRubyMacroHandler::ReadyCommands(CRuby& Ruby)
{
	VALUE Macro = Ruby.rb_define_class("Macro", Ruby.rb_rcObject());
	Ruby.rb_define_singleton_method(Macro, "SetMatch", reinterpret_cast<VALUE(__cdecl *)(...)>(MA_SetMatch), 1);
	Ruby.rb_define_singleton_method(Macro, "GetMode", reinterpret_cast<VALUE(__cdecl *)(...)>(MA_GetMode), 0);
	Ruby.rb_define_singleton_method(Macro, "GetFlags", reinterpret_cast<VALUE(__cdecl *)(...)>(MA_GetFlags), 0);
	Ruby.rb_define_singleton_method(Macro, "GetExt", reinterpret_cast<VALUE(__cdecl *)(...)>(MA_GetExt), 0);
	Ruby.rb_define_singleton_method(Macro, "GetSource", reinterpret_cast<VALUE(__cdecl *)(...)>(MA_GetSource), 0);
	Ruby.rb_define_singleton_method(Macro, "GetIndex", reinterpret_cast<VALUE(__cdecl *)(...)>(MA_GetIndex), 0);

	return Macro;
}

///////////////////////////////////////////////////////////////////////////////
//Macro
//Commands
VALUE __cdecl CRubyMacroHandler::MA_SetMatch(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(CExternalMacroIfObj::F_MA_SET_MATCH, self, args, 1);
}

//Functions
VALUE __cdecl CRubyMacroHandler::MA_GetMode(VALUE self){
	return FunctionHandler(CExternalMacroIfObj::F_MA_GET_MODE, self, NULL, 0);
}

VALUE __cdecl CRubyMacroHandler::MA_GetFlags(VALUE self){
	return FunctionHandler(CExternalMacroIfObj::F_MA_GET_FLAGS, self, NULL, 0);
}

VALUE __cdecl CRubyMacroHandler::MA_GetExt(VALUE self){
	return FunctionHandler(CExternalMacroIfObj::F_MA_GET_EXT, self, NULL, 0);
}

VALUE __cdecl CRubyMacroHandler::MA_GetSource(VALUE self){
	return FunctionHandler(CExternalMacroIfObj::F_MA_GET_SOURCE, self, NULL, 0);
}

VALUE __cdecl CRubyMacroHandler::MA_GetIndex(VALUE self){
	return FunctionHandler(CExternalMacroIfObj::F_MA_GET_INDEX, self, NULL, 0);
}
