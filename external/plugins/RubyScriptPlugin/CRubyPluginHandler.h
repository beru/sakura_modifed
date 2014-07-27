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

#ifndef _CRUBY_PLUGIN_HANDLER_H_
#define _CRUBY_PLUGIN_HANDLER_H_

#include <stdio.h>
#include "CPluginService.h"
#include "CRuby.h"

///////////////////////////////////////////////////////////////////////////////
/*!	Plugin Handlerをサポートするクラス
*/
class CRubyPluginHandler
{
public:
	CRubyPluginHandler();
	virtual ~CRubyPluginHandler();

	VALUE ReadyCommands(CRuby& Ruby);

protected:
	static DWORD GetIfObjType(){
		return CPluginService::IFOBJ_TYPE_PLUGIN;
	}
	static VALUE CommandHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize);
	static VALUE FunctionHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize);

	//Plugin callback handler
private:
	//Commands
	static VALUE __cdecl PL_SetOption(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3);
//	static VALUE __cdecl PL_SetOptionInt(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3);
	static VALUE __cdecl PL_AddCommand(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3);
	//Functions
	static VALUE __cdecl PL_GetPluginDir(VALUE self);
	static VALUE __cdecl PL_GetDef(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl PL_GetOption(VALUE self, VALUE arg1, VALUE arg2);
//	static VALUE __cdecl PL_GetOptionInt(VALUE self, VALUE arg1, VALUE arg2);
	static VALUE __cdecl PL_GetCommandNo(VALUE self);
	static VALUE __cdecl PL_GetString(VALUE self, VALUE arg1);
};

#endif	//_CRUBY_PLUGIN_HANDLER_H_
