/*!	@file
	@brief Plugin Handler

	Plugin Handlerを利用するためのインターフェース

	@author Plugins developers
	@date 2013.12.25
	@date 2014.01.10	upatchid:259
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

#include "StdAfx.h"
#include "CPluginService.h"
#include "CRubyPluginHandler.h"
#include "CRubyHandler.h"

///////////////////////////////////////////////////////////////////////////////
CRubyPluginHandler::CRubyPluginHandler()
{
}

///////////////////////////////////////////////////////////////////////////////
CRubyPluginHandler::~CRubyPluginHandler()
{
}

///////////////////////////////////////////////////////////////////////////////
VALUE CRubyPluginHandler::CommandHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize)
{
	return CRubyHandler::CommandHandler(GetIfObjType(), nFuncID, self, Arguments, nArgSize);
}

///////////////////////////////////////////////////////////////////////////////
VALUE CRubyPluginHandler::FunctionHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize)
{
	return CRubyHandler::FunctionHandler(GetIfObjType(), nFuncID, self, Arguments, nArgSize);
}

///////////////////////////////////////////////////////////////////////////////
VALUE CRubyPluginHandler::ReadyCommands(CRuby& Ruby)
{
	VALUE Plugin = Ruby.rb_define_class("Plugin", Ruby.rb_rcObject());
	Ruby.rb_define_singleton_method(Plugin, "SetOption", reinterpret_cast<VALUE(__cdecl *)(...)>(PL_SetOption), 3);
//	Ruby.rb_define_singleton_method(Plugin, "SetOptionInt", reinterpret_cast<VALUE(__cdecl *)(...)>(PL_SetOptionInt), 3);
	Ruby.rb_define_singleton_method(Plugin, "AddCommand", reinterpret_cast<VALUE(__cdecl *)(...)>(PL_AddCommand), 3);
	Ruby.rb_define_singleton_method(Plugin, "GetPluginDir", reinterpret_cast<VALUE(__cdecl *)(...)>(PL_GetPluginDir), 0);
	Ruby.rb_define_singleton_method(Plugin, "GetDef", reinterpret_cast<VALUE(__cdecl *)(...)>(PL_GetDef), 2);
	Ruby.rb_define_singleton_method(Plugin, "GetOption", reinterpret_cast<VALUE(__cdecl *)(...)>(PL_GetOption), 2);
//	Ruby.rb_define_singleton_method(Plugin, "GetOptionInt", reinterpret_cast<VALUE(__cdecl *)(...)>(PL_GetOptionInt), 2);
	Ruby.rb_define_singleton_method(Plugin, "GetCommandNo", reinterpret_cast<VALUE(__cdecl *)(...)>(PL_GetCommandNo), 0);
	Ruby.rb_define_singleton_method(Plugin, "GetString", reinterpret_cast<VALUE(__cdecl *)(...)>(PL_GetString), 1);

	return Plugin;
}

///////////////////////////////////////////////////////////////////////////////
//Plugin
//Commands
VALUE __cdecl CRubyPluginHandler::PL_SetOption(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3){
	VALUE args[3] = { arg1, arg2, arg3 };
	return CommandHandler(CExternalPluginIfObj::F_PL_SETOPTION, self, args, 3);
}

//VALUE __cdecl CRubyPluginHandler::PL_SetOptionInt(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3){
//	VALUE args[3] = { arg1, arg2, arg3 };
//	return CommandHandler(CExternalPluginIfObj::F_PL_SETOPTION, self, args, 3);
//}

VALUE __cdecl CRubyPluginHandler::PL_AddCommand(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3){
	VALUE args[3] = { arg1, arg2, arg3 };
	return CommandHandler(CExternalPluginIfObj::F_PL_ADDCOMMAND, self, args, 3);
}

//Functions
VALUE __cdecl CRubyPluginHandler::PL_GetPluginDir(VALUE self){
	return FunctionHandler(CExternalPluginIfObj::F_PL_GETPLUGINDIR, self, NULL, 0);
}

VALUE __cdecl CRubyPluginHandler::PL_GetDef(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return FunctionHandler(CExternalPluginIfObj::F_PL_GETDEF, self, args, 2);
}

VALUE __cdecl CRubyPluginHandler::PL_GetOption(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return FunctionHandler(CExternalPluginIfObj::F_PL_GETOPTION, self, args, 2);
}

//VALUE __cdecl CRubyPluginHandler::PL_GetOptionInt(VALUE self, VALUE arg1, VALUE arg2){
//	VALUE args[2] = { arg1, arg2 };
//	return FunctionHandler(CExternalPluginIfObj::F_PL_GETOPTION, self, args, 2);
//}

VALUE __cdecl CRubyPluginHandler::PL_GetCommandNo(VALUE self){
	return FunctionHandler(CExternalPluginIfObj::F_PL_GETCOMMANDNO, self, NULL, 0);
}

VALUE __cdecl CRubyPluginHandler::PL_GetString(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return FunctionHandler(CExternalPluginIfObj::F_PL_GETSTRING, self, args, 1);
}
