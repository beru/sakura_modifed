/*!	@file
	@brief プラグイン基本クラス
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

#ifndef CEXTERNALIFOBJ_H_779A49AF_0311_4859_9A5F_F58C5E6FA204
#define CEXTERNALIFOBJ_H_779A49AF_0311_4859_9A5F_F58C5E6FA204

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)

#include <Windows.h>
#include <OleAuto.h>
#include <wchar.h>
#include <tchar.h>
#include <list>
#include <string>
#include "CPluginCommon.h"
#include "plugin/SakuraPlugin.h"
#include "Funccode_define.h"

///////////////////////////////////////////////////////////////////////////////
/*
	エディタが提供するプラグインにアクセスするクラス
*/
class CExternalIfObj
{
public:
	SAKURA_DLL_PLUGIN_OBJ*		m_lpDllPluginIfObj;	//!< エディタから渡された情報
	SAKURA_DLL_PLUGIN_IF_OBJ*	m_lpThisObj;		//!< 
	LPVOID						m_lpThisIfObj;		//!< このプラグインのオブジェクト

public:
	CExternalIfObj(){
		m_lpDllPluginIfObj = NULL;
		m_lpThisObj = NULL;
		m_lpThisIfObj = NULL;
	}

	virtual ~CExternalIfObj(){
	}

	//バージョン互換性をチェックする
	virtual BOOL IsCompatibleVersion(const DWORD dwVersion){
		if(HIWORD(dwVersion) == HIWORD(SAKURA_DLL_PLUGIN_VERSION)){
			return TRUE;
		}
		return FALSE;
	}

	//利用可能かチェックする
	virtual BOOL IsAvailable(){
		return (m_lpDllPluginIfObj != NULL) && (m_lpThisIfObj != NULL);
	}

	//自分のプラグインを探して保持する
	virtual BOOL Initialize(SAKURA_DLL_PLUGIN_OBJ* obj){
		m_lpDllPluginIfObj = NULL;
		m_lpThisObj = NULL;
		m_lpThisIfObj = NULL;
		if(obj != NULL){
			if(IsCompatibleVersion(obj->m_dwVersion)){
				m_lpDllPluginIfObj = obj;
				for(DWORD i = 0; i < obj->m_dwIfObjListCount; i++){
					SAKURA_DLL_PLUGIN_IF_OBJ* ifobj = &obj->m_IfObjList[i];
					if(ifobj != NULL){
						if(wcscmp(ifobj->m_szName, GetName()) == 0){
							m_lpThisObj = ifobj;
							m_lpThisIfObj = ifobj->m_lpIfObj;
							return TRUE;
						}
					}
				}
			}
		}
		return FALSE;
	}

	//EditViewを返す
	LPVOID GetEditView(){
		if(m_lpDllPluginIfObj != NULL){
			return m_lpDllPluginIfObj->m_lpEditView;
		}
		return NULL;
	}

	//関数情報を返す
	MACRO_FUNC_INFO* GetFunctionInfo(){
		if(m_lpThisObj != NULL){
			return m_lpThisObj->m_FunctionInfo;
		}
		return NULL;
	}

	//コマンド情報を返す
	MACRO_FUNC_INFO* GetCommandInfo(){
		if(m_lpThisObj != NULL){
			return m_lpThisObj->m_CommandInfo;
		}
		return NULL;
	}

	HWND GetParentHwnd(){
		if(m_lpDllPluginIfObj != NULL){
			return m_lpDllPluginIfObj->m_hParentHwnd;
		}
		return (HWND)NULL;
	}

	MACRO_FUNC_INFO* GetMacroInfoByID(const UINT nFuncID){
		MACRO_FUNC_INFO* info = GetMacroCommandInfo(nFuncID);
		if(info == NULL){
			info = GetMacroFunctionInfo(nFuncID);
		}
		return info;
	}

	MACRO_FUNC_INFO* GetMacroCommandInfo(const UINT nFuncID){
		MACRO_FUNC_INFO* ary = GetCommandInfo();
		if(ary != NULL){
			for(int i = 0; ary[i].m_lpszFuncName != NULL; i++){
				if(ary[i].m_nFuncID == nFuncID){
					return &ary[i];
				}
			}
		}
		return NULL;
	}

	MACRO_FUNC_INFO* GetMacroFunctionInfo(const UINT nFuncID){
		MACRO_FUNC_INFO* ary = GetFunctionInfo();
		if(ary != NULL){
			for(int i = 0; ary[i].m_lpszFuncName != NULL; i++){
				if(ary[i].m_nFuncID == nFuncID){
					return &ary[i];
				}
			}
		}
		return NULL;
	}

public:
	//プラグインの名前を返す "Editor", "Plugin", "Indent", "Complement", "Macro"
	virtual LPCWSTR GetName() = 0;

public:
	virtual BOOL HandleFunction(DWORD ID, const VARIANT* Arguments, const int ArgSize, VARIANT* Result){
		if(IsAvailable()){
			return m_lpDllPluginIfObj->m_fnFunctionHandler(GetName(), m_lpThisIfObj, GetEditView(), ID, Arguments, ArgSize, Result);
		}
		return FALSE;
	}

	virtual void HandleCommand(DWORD ID, LPCWSTR Arguments[], const int ArgLengths[], const int ArgSize){
		if(IsAvailable()){
			m_lpDllPluginIfObj->m_fnCommandHandler(GetName(), m_lpThisIfObj, GetEditView(), ID, Arguments, ArgLengths, ArgSize);
		}
	}

#define DEFINE_PARAMS(Arguments, Result) \
	VARIANT Result; \
	VARIANT Arguments[4]; \
	int ArgLengths[4]; \
	{ \
		VariantInit(&Result); \
		for(int i = 0; i < 4; i++){ \
			VariantInit(&Arguments[i]); \
			ArgLengths[i] = 0; \
		} \
	}

#define CLEAR_PARAMS(Arguments, Result) \
	{ \
		VariantClear(&Result); \
		for(int i = 0; i < 4; i++){ \
			VariantClear(&Arguments[i]); \
			ArgLengths[i] = 0; \
		} \
	}

	static void SET_PARAM(VARIANT& Argument, LPCWSTR lpszValue){
		VariantClear(&Argument);
		Argument.vt = VT_BSTR;
		Argument.bstrVal = SysAllocString(lpszValue);
	}

	static void SET_PARAM(VARIANT& Argument, int nValue){
		VariantClear(&Argument);
		Argument.vt = VT_I4;
		Argument.lVal = nValue;
	}

#define DEFINE_WCHAR_PARAMS(Arguments) \
	LPCWSTR Arguments[4] = { L"", L"", L"", L"" }; \
	int ArgLengths[4] = { 0, 0, 0, 0 };

	static WideString TO_STRING(int nValue){
		wchar_t value[32];
		swprintf(value, L"%ld", nValue);
		WideString strValue = value;
		return strValue;
	}

	static WideString TO_STRING(void* lpValue){
		wchar_t value[32];
		swprintf(value, L"%08x", (DWORD)lpValue);
		WideString strValue = value;
		return strValue;
	}

#define SET_WCHAR_ARG(index, arg) \
	Arguments[index] = arg; \
	ArgLengths[index] = wcslen(arg);

#define SET_INT_ARG(index, arg) \
	WideString __##arg##tmp = TO_STRING(arg); \
	Arguments[index]    = __##arg##tmp.c_str(); \
	ArgLengths[index]   = __##arg##tmp.length();
};

///////////////////////////////////////////////////////////////////////////////
// CommandHandlerを呼び出すときに引数の管理を簡単にする
class CCommandHandlerParam
{
public:
	int			m_nArgs;
	LPCWSTR*	ParamArguments;		//ParamStrings[]のポインタを保持するだけ
	int*		ParamArgLengths;
	WideString*	ParamStrings;

	CCommandHandlerParam(const int nArgs){
		m_nArgs = nArgs;
		ParamArguments  = NULL;
		ParamArgLengths = NULL;
		ParamStrings    = NULL;
		if(nArgs > 0){
			ParamArguments  = new LPCWSTR[nArgs];
			ParamArgLengths = new int[nArgs];
			ParamStrings    = new WideString[nArgs];
			for(int i = 0; i < nArgs; i++){
				ParamArguments[i]  = NULL;
				ParamArgLengths[i] = 0;
			}
		}
	}

	virtual ~CCommandHandlerParam(){
		Clear();
	}

	void Clear(){
		if(ParamArguments  != NULL){
			delete[] ParamArguments;
			ParamArguments = NULL;
		}
		if(ParamArgLengths != NULL){
			delete[] ParamArgLengths;
			ParamArgLengths = NULL;
		}
		if(ParamStrings    != NULL){
			delete[] ParamStrings;
			ParamStrings = NULL;
		}
		m_nArgs = 0;
	}

	void Set(const int nIndex, LPCWSTR wstr){
		ParamStrings[nIndex]    = wstr;
		ParamArguments[nIndex]  = ParamStrings[nIndex].c_str();
		ParamArgLengths[nIndex] = ParamStrings[nIndex].length();
	}
};

///////////////////////////////////////////////////////////////////////////////
// FunctionHandlerを呼び出すときに引数の管理を簡単にする
class CFunctionHandlerParam
{
public:
	int				m_nArgs;
	VARIANT*		vtArguments;
	VARIANT			vtResult;

	CFunctionHandlerParam(const int nArgs){
		m_nArgs = nArgs;
		vtArguments = NULL;
		if(nArgs > 0){
			vtArguments = new VARIANT[nArgs];
			for(int i = 0; i < nArgs; i++){
				::VariantInit(&vtArguments[i]);
			}
		}
		::VariantInit(&vtResult);
	}

	virtual ~CFunctionHandlerParam(){
		Clear();
	}

	void Clear(){
		if(m_nArgs > 0){
			for(int i = 0; i < m_nArgs; i++){
				::VariantClear(&vtArguments[i]);
			}
			delete[] vtArguments;
			vtArguments = NULL;
		}
		::VariantClear(&vtResult);
		m_nArgs = 0;
	}

	void Set(const int nIndex, const long lValue){
		vtArguments[nIndex].vt = VT_I4;
		vtArguments[nIndex].lVal = lValue;
	}

	void Set(const int nIndex, const wchar_t* strValue){
		vtArguments[nIndex].vt = VT_BSTR;
		vtArguments[nIndex].bstrVal = ::SysAllocString(strValue);
	}
};

#endif	//CEXTERNALIFOBJ_H_779A49AF_0311_4859_9A5F_F58C5E6FA204
