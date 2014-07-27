/*!	@file
	@brief Ruby Library Handler

	Rubyを利用するためのインターフェース

	@author Sakura-Editor collaborators
	@date 2013.12.16
*/
/*
	Copyright (C) 2013, Sakura-Editor collaborators

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
#include "CRuby.h"
#include "CRubyHandler.h"
#include "CRubyPluginHandler.h"
#include "CRubyEditorHandler.h"
#include "CRubyOutlineHandler.h"
#include "CRubySmartIndentHandler.h"
#include "CRubyComplementHandler.h"
#include "CRubyMacroHandler.h"
#include <map>

///////////////////////////////////////////////////////////////////////////////
std::map<VALUE, struct CRubyHandler::RubyExecInfo*> CRubyHandler::m_mapEngine;

///////////////////////////////////////////////////////////////////////////////
CRubyHandler::CRubyHandler()
{
	m_valPlugin      = 0;
	m_valEditor      = 0;
	m_valOutline     = 0;
	m_valSmartIndent = 0;
	m_valComplement  = 0;
	m_valMacro       = 0;
}

///////////////////////////////////////////////////////////////////////////////
CRubyHandler::~CRubyHandler()
{
	thePluginService.EnterCriticalSection();
	m_mapEngine.erase(m_valPlugin);
	m_mapEngine.erase(m_valEditor);
	m_mapEngine.erase(m_valOutline);
	m_mapEngine.erase(m_valSmartIndent);
	m_mapEngine.erase(m_valComplement);
	m_mapEngine.erase(m_valMacro);
	thePluginService.LeaveCriticalSection();
}

///////////////////////////////////////////////////////////////////////////////
struct CRubyHandler::RubyExecInfo* CRubyHandler::GetRubyExecInfo(VALUE self)
{
	thePluginService.EnterCriticalSection();
	struct RubyExecInfo* lpRubyExecInfo = (struct RubyExecInfo*)m_mapEngine[self];
	thePluginService.LeaveCriticalSection();
	return lpRubyExecInfo;
}

///////////////////////////////////////////////////////////////////////////////
VALUE CRubyHandler::CommandHandler(const DWORD dwIfObjType, const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize)
{
	BOOL bRet = FALSE;
	RubyExecInfo* lpRubyExecInfo = GetRubyExecInfo(self);
	if(lpRubyExecInfo == NULL){
		return RUBY_NULL;
	}
	CCommandHandlerParam param(nArgSize);
	const MACRO_FUNC_INFO* info = NULL;
	CExternalIfObj* lpIfObj = lpRubyExecInfo->m_lpPluginService->GetIfObj(dwIfObjType);
	if(lpIfObj == NULL) goto ExitHandler;
	info = lpIfObj->GetMacroCommandInfo(nFuncID);
	if (info == NULL) goto ExitHandler;
	if (CPluginService::GetNumArgs(info) != nArgSize) goto ExitHandler;

	for(int i = 0; i < nArgSize; i++){
		switch(CPluginService::GetType(info, i)){
		case VT_I4:
			{
				lpRubyExecInfo->m_lpRuby->rb_check_type(Arguments[i], RUBY_TYPE_FIXNUM);	//raise error
				int nValue = lpRubyExecInfo->m_lpRuby->rb_fix2int(Arguments[i]);
				wchar_t szBuffer[32];
				swprintf(szBuffer, L"%d", nValue);
				param.Set(i, szBuffer);
			}
			break;
		case VT_BSTR:
		case VT_VARIANT:
			{
				lpRubyExecInfo->m_lpRuby->rb_check_type(Arguments[i], RUBY_TYPE_STRING);	//raise error
				const char* utf8str = lpRubyExecInfo->m_lpRuby->StringValueCStr(Arguments[i]);
				WideString wideStr = CPluginService::to_wstr(utf8str, CP_UTF8);
				param.Set(i, wideStr.c_str());
			}
			break;
		default:
			goto ExitHandler;
		}
	}

	lpIfObj->HandleCommand(nFuncID, (LPCWSTR*)param.ParamArguments, param.ParamArgLengths, nArgSize);
	bRet = TRUE;

ExitHandler:
	if(!bRet){
		param.Clear();
		char szBuffer[256];
		sprintf(szBuffer, "Failed to call CommandHandler(ID=%d).", nFuncID);
		lpRubyExecInfo->m_lpRuby->rb_raise(lpRubyExecInfo->m_lpRuby->rb_eRuntimeError(), szBuffer);
		//not reached?
		return RUBY_NULL;
	}

	return RUBY_NULL;
}

///////////////////////////////////////////////////////////////////////////////
VALUE CRubyHandler::FunctionHandler(const DWORD dwIfObjType, const DWORD nFuncID, VALUE self, VALUE Arguments[], int nArgSize)
{
	BOOL bRet = FALSE;
	VALUE retObj = RUBY_NULL;
	RubyExecInfo* lpRubyExecInfo = GetRubyExecInfo(self);
	if(lpRubyExecInfo == NULL){
		return RUBY_NULL;
	}
	CFunctionHandlerParam param(nArgSize);
	const MACRO_FUNC_INFO* info = NULL;
	CExternalIfObj* lpIfObj = lpRubyExecInfo->m_lpPluginService->GetIfObj(dwIfObjType);
	if(lpIfObj == NULL) goto ExitHandler;
	info = lpIfObj->GetMacroFunctionInfo(nFuncID);
	if(info == NULL) goto ExitHandler;
	if(CPluginService::GetNumArgs(info) != nArgSize) goto ExitHandler;

	for(int i = 0; i < nArgSize; i++){
		switch(CPluginService::GetType(info, i)){
		case VT_I4:
			lpRubyExecInfo->m_lpRuby->rb_check_type(Arguments[i], RUBY_TYPE_FIXNUM);	//raise error
			param.Set(i, lpRubyExecInfo->m_lpRuby->rb_fix2int(Arguments[i]));
			break;
		case VT_BSTR:
		case VT_VARIANT:
			{
				lpRubyExecInfo->m_lpRuby->rb_check_type(Arguments[i], RUBY_TYPE_STRING);	//raise error
				const char* utf8str = lpRubyExecInfo->m_lpRuby->StringValueCStr(Arguments[i]);
				WideString wideStr = CPluginService::to_wstr(utf8str, CP_UTF8);
				param.Set(i, wideStr.c_str());
			}
			break;
		default:
			goto ExitHandler;
		}
	}

	bRet = lpIfObj->HandleFunction(nFuncID, param.vtArguments, nArgSize, &param.vtResult);

	if(bRet){
		switch(info->m_varResult){
		case VT_I4:
			if(param.vtResult.vt == VT_I4) retObj = lpRubyExecInfo->m_lpRuby->rb_int2inum(param.vtResult.lVal);
			else if(param.vtResult.vt == VT_INT) retObj = lpRubyExecInfo->m_lpRuby->rb_int2inum(param.vtResult.intVal);
			else if(param.vtResult.vt == VT_UINT) retObj = lpRubyExecInfo->m_lpRuby->rb_uint2inum(param.vtResult.uintVal);
			else bRet = FALSE;
			break;
		case VT_BSTR:
		case VT_VARIANT:
			{
				AnsiString utf8str = CPluginService::to_astr(param.vtResult.bstrVal, CP_UTF8);
				retObj = lpRubyExecInfo->m_lpRuby->rb_str_new2(utf8str.c_str());
			}
			break;
		default:
			bRet = FALSE;
			break;
		}
	}

ExitHandler:
	if(!bRet){
		param.Clear();
		char szBuffer[256];
		sprintf(szBuffer, "Failed to call FunctionHandler(ID=%d).", nFuncID);
		lpRubyExecInfo->m_lpRuby->rb_raise(lpRubyExecInfo->m_lpRuby->rb_eRuntimeError(), szBuffer);
		//not reached?
		return RUBY_NULL;
	}

	return retObj;
}

///////////////////////////////////////////////////////////////////////////////
void CRubyHandler::ReadyCommands(CRuby& Ruby, struct RubyExecInfo* info)
{
	m_valPlugin      = PluginHandler.ReadyCommands(Ruby);
	m_valEditor      = EditorHandler.ReadyCommands(Ruby);
	m_valOutline     = OutlineHandler.ReadyCommands(Ruby);
	m_valSmartIndent = SmartIndentHandler.ReadyCommands(Ruby);
	m_valComplement  = ComplementHandler.ReadyCommands(Ruby);
	m_valMacro       = MacroHandler.ReadyCommands(Ruby);

	thePluginService.EnterCriticalSection();
	m_mapEngine[m_valPlugin]      = info;
	m_mapEngine[m_valEditor]      = info;
	m_mapEngine[m_valOutline]     = info;
	m_mapEngine[m_valSmartIndent] = info;
	m_mapEngine[m_valComplement]  = info;
	m_mapEngine[m_valMacro]       = info;
	thePluginService.LeaveCriticalSection();
}

/*
///////////////////////////////////////////////////////////////////////////////
//文字列は""で囲む、必ず改行する
VALUE __cdecl CRubyHandler::func_p(int argc, VALUE* argv, VALUE self)
{
	RubyExecInfo* lpRubyExecInfo = GetRubyExecInfo(self);
	VALUE str = lpRubyExecInfo->m_lpRuby->rb_str_new2("");
	for(int i = 0; i < argc; i++){
		//TODO: 文字列は""で囲む
		lpRubyExecInfo->m_lpRuby->rb_str_concat(str, lpRubyExecInfo->m_lpRuby->rb_inspect(argv[i]));
		lpRubyExecInfo->m_lpRuby->rb_str_cat2(str, "\r\n");
	}
	S_InsText(self, str);
	return RUBY_NULL;
}

///////////////////////////////////////////////////////////////////////////////
//文字列は囲まない、改行する
VALUE __cdecl CRubyHandler::func_puts(int argc, VALUE* argv, VALUE self)
{
	RubyExecInfo* lpRubyExecInfo = GetRubyExecInfo(self);
	VALUE str = lpRubyExecInfo->m_lpRuby->rb_str_new2("");
	for(int i = 0; i < argc; i++){
		lpRubyExecInfo->m_lpRuby->rb_str_concat(str, lpRubyExecInfo->m_lpRuby->rb_inspect(argv[i]));
		lpRubyExecInfo->m_lpRuby->rb_str_cat2(str, "\r\n");
	}
	S_InsText(self, str);
	return RUBY_NULL;
}

///////////////////////////////////////////////////////////////////////////////
//改行しない
VALUE __cdecl CRubyHandler::func_print(int argc, VALUE* argv, VALUE self)
{
	RubyExecInfo* lpRubyExecInfo = GetRubyExecInfo(self);
	VALUE str = lpRubyExecInfo->m_lpRuby->rb_str_new2("");
	for(int i = 0; i < argc; i++){
		lpRubyExecInfo->m_lpRuby->rb_str_concat(str, lpRubyExecInfo->m_lpRuby->rb_inspect(argv[i]));
	}
	S_InsText(self, str);
	return RUBY_NULL;
}

///////////////////////////////////////////////////////////////////////////////
void CRubyHandler::OverridePcommand()
{
	if(m_nPcommand & RUBY_OVERRIDE_P){
		rb_define_global_function("p", reinterpret_cast<VALUE(__cdecl *)(...)>(func_p), -1);
	}
	if(m_nPcommand & RUBY_OVERRIDE_PUTS){
		rb_define_global_function("puts", reinterpret_cast<VALUE(__cdecl *)(...)>(func_puts), -1);
	}
	if(m_nPcommand & RUBY_OVERRIDE_PRINT){
		rb_define_global_function("print", reinterpret_cast<VALUE(__cdecl *)(...)>(func_print), -1);
	}
}
*/
