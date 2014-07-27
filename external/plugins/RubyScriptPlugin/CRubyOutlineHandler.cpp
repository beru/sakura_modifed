/*!	@file
	@brief Outline Handler

	Outline Handlerを利用するためのインターフェース

	@author Plugins developers
	@date 2013.12.28
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

#include "StdAfx.h"
#include "CPluginService.h"
#include "CRubyOutlineHandler.h"
#include "CRubyHandler.h"

///////////////////////////////////////////////////////////////////////////////
CRubyOutlineHandler::CRubyOutlineHandler()
{
}

///////////////////////////////////////////////////////////////////////////////
CRubyOutlineHandler::~CRubyOutlineHandler()
{
}

///////////////////////////////////////////////////////////////////////////////
VALUE CRubyOutlineHandler::CommandHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize)
{
	return CRubyHandler::CommandHandler(GetIfObjType(), nFuncID, self, Arguments, nArgSize);
}

///////////////////////////////////////////////////////////////////////////////
VALUE CRubyOutlineHandler::FunctionHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize)
{
	return CRubyHandler::FunctionHandler(GetIfObjType(), nFuncID, self, Arguments, nArgSize);
}

///////////////////////////////////////////////////////////////////////////////
VALUE CRubyOutlineHandler::ReadyCommands(CRuby& Ruby)
{
	VALUE Outline = Ruby.rb_define_class("Outline", Ruby.rb_rcObject());
	Ruby.rb_define_singleton_method(Outline, "AddFuncInfo", reinterpret_cast<VALUE(__cdecl *)(...)>(OL_AddFuncInfo), 4);
	Ruby.rb_define_singleton_method(Outline, "AddFuncInfo2", reinterpret_cast<VALUE(__cdecl *)(...)>(OL_AddFuncInfo2), 4);
	Ruby.rb_define_singleton_method(Outline, "SetTitle", reinterpret_cast<VALUE(__cdecl *)(...)>(OL_SetTitle), 1);
	Ruby.rb_define_singleton_method(Outline, "SetListType", reinterpret_cast<VALUE(__cdecl *)(...)>(OL_SetListType), 1);
	Ruby.rb_define_singleton_method(Outline, "SetLabel", reinterpret_cast<VALUE(__cdecl *)(...)>(OL_SetLabel), 2);

	return Outline;
}

///////////////////////////////////////////////////////////////////////////////
//Outline
//Commands
VALUE __cdecl CRubyOutlineHandler::OL_AddFuncInfo(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3, VALUE arg4){
	VALUE args[4] = { arg1, arg2, arg3, arg4 };
	return CommandHandler(CExternalOutlineIfObj::F_OL_ADDFUNCINFO, self, args, 4);
}

VALUE __cdecl CRubyOutlineHandler::OL_AddFuncInfo2(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3, VALUE arg4){
	VALUE args[4] = { arg1, arg2, arg3, arg4 };
	return CommandHandler(CExternalOutlineIfObj::F_OL_ADDFUNCINFO2, self, args, 4);
}

VALUE __cdecl CRubyOutlineHandler::OL_SetTitle(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(CExternalOutlineIfObj::F_OL_SETTITLE, self, args, 1);
}

VALUE __cdecl CRubyOutlineHandler::OL_SetListType(VALUE self, VALUE arg1){
	VALUE args[1] = { arg1 };
	return CommandHandler(CExternalOutlineIfObj::F_OL_SETLISTTYPE, self, args, 1);
}

VALUE __cdecl CRubyOutlineHandler::OL_SetLabel(VALUE self, VALUE arg1, VALUE arg2){
	VALUE args[2] = { arg1, arg2 };
	return CommandHandler(CExternalOutlineIfObj::F_OL_SETLABEL, self, args, 2);
}

//Functions
