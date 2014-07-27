/*!	@file
	@brief Outline Handler

	Outline Handler�𗘗p���邽�߂̃C���^�[�t�F�[�X

	@author Plugins developers
	@date 2013.12.28
	@date 2014.02.08	SetLabel�ǉ�
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

#ifndef _CRUBY_OUTLINE_HANDLER_H_
#define _CRUBY_OUTLINE_HANDLER_H_

#include <stdio.h>
#include "CPluginService.h"
#include "CRuby.h"

///////////////////////////////////////////////////////////////////////////////
/*!	Outline Handler���T�|�[�g����N���X
*/
class CRubyOutlineHandler
{
public:
	CRubyOutlineHandler();
	virtual ~CRubyOutlineHandler();

	VALUE ReadyCommands(CRuby& Ruby);

protected:
	static DWORD GetIfObjType(){
		return CPluginService::IFOBJ_TYPE_OUTLINE;
	}
	static VALUE CommandHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize);
	static VALUE FunctionHandler(const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize);

	//Outline callback handler
private:
	//Commands
	static VALUE __cdecl OL_AddFuncInfo(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3, VALUE arg4);
	static VALUE __cdecl OL_AddFuncInfo2(VALUE self, VALUE arg1, VALUE arg2, VALUE arg3, VALUE arg4);
	static VALUE __cdecl OL_SetTitle(VALUE self, VALUE arg1);
	static VALUE __cdecl OL_SetListType(VALUE self, VALUE arg1);
	static VALUE __cdecl OL_SetLabel(VALUE self, VALUE arg1, VALUE arg2);
	//Functions
};

#endif	//_CRUBY_OUTLINE_HANDLER_H_
