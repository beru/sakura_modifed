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

#ifndef _CRUBY_SMARTINDENT_HANDLER_H_
#define _CRUBY_SMARTINDENT_HANDLER_H_

#include <stdio.h>
#include "CPluginService.h"
#include "CRuby.h"

///////////////////////////////////////////////////////////////////////////////
/*!	SmartIndent Handlerをサポートするクラス
*/
class CRubySmartIndentHandler
{
public:
	CRubySmartIndentHandler();
	virtual ~CRubySmartIndentHandler();

	VALUE ReadyCommands(CRuby& Ruby);

protected:
	static DWORD GetIfObjType(){
		return CPluginService::IFOBJ_TYPE_SMARTINDENT;
	}
	static VALUE CommandHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize);
	static VALUE FunctionHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize);

	//SmartIndent callback handler
private:
	//Commands
	//Functions
	static VALUE __cdecl SI_GetChar(VALUE self);
};

#endif	//_CRUBY_SMARTINDENT_HANDLER_H_
