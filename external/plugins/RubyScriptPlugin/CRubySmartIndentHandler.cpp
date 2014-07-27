/*!	@file
	@brief SmartIndent Handler

	SmartIndent Handlerを利用するためのインターフェース

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
#include "CRubySmartIndentHandler.h"
#include "CRubyHandler.h"

///////////////////////////////////////////////////////////////////////////////
CRubySmartIndentHandler::CRubySmartIndentHandler()
{
}

///////////////////////////////////////////////////////////////////////////////
CRubySmartIndentHandler::~CRubySmartIndentHandler()
{
}

///////////////////////////////////////////////////////////////////////////////
VALUE CRubySmartIndentHandler::CommandHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize)
{
	return CRubyHandler::CommandHandler(GetIfObjType(), nFuncID, self, Arguments, nArgSize);
}

///////////////////////////////////////////////////////////////////////////////
VALUE CRubySmartIndentHandler::FunctionHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize)
{
	return CRubyHandler::FunctionHandler(GetIfObjType(), nFuncID, self, Arguments, nArgSize);
}

///////////////////////////////////////////////////////////////////////////////
VALUE CRubySmartIndentHandler::ReadyCommands(CRuby& Ruby)
{
	VALUE SmartIndent = Ruby.rb_define_class("SmartIndent", Ruby.rb_rcObject());
	Ruby.rb_define_singleton_method(SmartIndent, "GetChar", reinterpret_cast<VALUE(__cdecl *)(...)>(SI_GetChar), 0);

	return SmartIndent;
}

///////////////////////////////////////////////////////////////////////////////
//SmartIndent
//Commands
//Functions
VALUE __cdecl CRubySmartIndentHandler::SI_GetChar(VALUE self){
	return FunctionHandler(CExternalSmartIndentIfObj::F_SI_GETCHAR, self, NULL, 0);
}
