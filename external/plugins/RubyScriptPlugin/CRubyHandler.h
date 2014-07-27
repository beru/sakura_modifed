/*!	@file
	@brief Ruby Handler

	Ruby Handlerを利用するためのインターフェース

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

#ifndef _CRUBY_HANDLER_H_
#define _CRUBY_HANDLER_H_

#include "CPluginService.h"
#include "CRubyPluginHandler.h"
#include "CRubyEditorHandler.h"
#include "CRubyOutlineHandler.h"
#include "CRubySmartIndentHandler.h"
#include "CRubyComplementHandler.h"
#include "CRubyMacroHandler.h"
#include "CRuby.h"
#include <map>

///////////////////////////////////////////////////////////////////////////////
/*!	Ruby Handlerをサポートするクラス
*/
class CRubyHandler
{
public:
	CRubyHandler();
	virtual ~CRubyHandler();

public:
	struct RubyExecInfo {
		CRuby*			m_lpRuby;			//!< CRuby
		CPluginService*	m_lpPluginService;	//!< 親サービス
		int				m_argc;
		const char*		m_argv[2];
	};

	static std::map<VALUE, struct RubyExecInfo*>	m_mapEngine;
	CRubyPluginHandler		PluginHandler;
	CRubyEditorHandler		EditorHandler;
	CRubyOutlineHandler		OutlineHandler;
	CRubySmartIndentHandler	SmartIndentHandler;
	CRubyComplementHandler	ComplementHandler;
	CRubyMacroHandler		MacroHandler;
	VALUE				m_valPlugin;
	VALUE				m_valEditor;
	VALUE				m_valOutline;
	VALUE				m_valSmartIndent;
	VALUE				m_valComplement;
	VALUE				m_valMacro;

public:
	void ReadyCommands(CRuby& Ruby, struct RubyExecInfo* info);
	static struct RubyExecInfo* GetRubyExecInfo(VALUE self);
	static VALUE CommandHandler(const DWORD dwIfObjType, const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize);
	static VALUE FunctionHandler(const DWORD dwIfObjType, const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize);

private:
//	void OverridePcommand();
//	static VALUE __cdecl func_p(int argc, VALUE* argv, VALUE self);
//	static VALUE __cdecl func_puts(int argc, VALUE* argv, VALUE self);
//	static VALUE __cdecl func_print(int argc, VALUE* argv, VALUE self);

};

#endif	//_CRUBY_HANDLER_H_
