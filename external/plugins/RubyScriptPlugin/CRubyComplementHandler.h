/*!	@file
	@brief Complement Handler

	Complement Handler�𗘗p���邽�߂̃C���^�[�t�F�[�X

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

#ifndef _CRUBY_COMPLEMENT_HANDLER_H_
#define _CRUBY_COMPLEMENT_HANDLER_H_

#include <stdio.h>
#include "CPluginService.h"
#include "CRuby.h"

///////////////////////////////////////////////////////////////////////////////
/*!	Complement Handler���T�|�[�g����N���X
*/
class CRubyComplementHandler
{
public:
	CRubyComplementHandler();
	virtual ~CRubyComplementHandler();

	VALUE ReadyCommands(CRuby& Ruby);

protected:
	static DWORD GetIfObjType(){
		return CPluginService::IFOBJ_TYPE_COMPLEMENT;
	}
	static VALUE CommandHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize);
	static VALUE FunctionHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize);

	//Complement callback handler
private:
	//Commands
	//Functions
	static VALUE __cdecl CM_GetCurrentWord(VALUE self);
	static VALUE __cdecl CM_GetOption(VALUE self);
	static VALUE __cdecl CM_AddList(VALUE self, VALUE arg1);
};

#endif	//_CRUBY_COMPLEMENT_HANDLER_H_
