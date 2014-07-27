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

#ifndef CBASEPLUGINSERVICE_H
#define CBASEPLUGINSERVICE_H

#include <windows.h>
#include "CPluginCommon.h"
#include "plugin/SakuraPlugin.h"
#include "CExternalPluginIfObj.h"
#include "CExternalEditorIfObj.h"
#include "CExternalMacroIfObj.h"
#include "CExternalSmartIndentIfObj.h"
#include "CExternalOutlineIfObj.h"
#include "CExternalComplementIfObj.h"
#include "SakuraMeetsPlugin.h"
#include <map>

///////////////////////////////////////////////////////////////////////////////
//!	プラグインベースクラス
class CBasePluginService
{
public:
	CExternalPluginIfObj		Plugin;
	CExternalEditorIfObj		Editor;
	CExternalOutlineIfObj		Outline;
	CExternalSmartIndentIfObj	SmartIndent;
	CExternalComplementIfObj	Complement;
	CExternalMacroIfObj			Macro;
	std::map<LANGID, HMODULE>	m_mapLangModule;	//!< 言語とリソースDLLハンドル
	SAKURA_DLL_PLUGIN_OBJ*		m_lpCloneObj;		//!< 呼び出された情報のコピー

	enum tagIfObjType {
		IFOBJ_TYPE_PLUGIN = 0,
		IFOBJ_TYPE_EDITOR,
		IFOBJ_TYPE_OUTLINE,
		IFOBJ_TYPE_SMARTINDENT,
		IFOBJ_TYPE_COMPLEMENT,
		IFOBJ_TYPE_MACRO
	};

public:
	CBasePluginService();
	virtual ~CBasePluginService();

public:
	virtual BOOL Initialize(SAKURA_DLL_PLUGIN_OBJ* obj);
	virtual void Uninitialize();
	virtual LPCWSTR GetPluginName(){ return L""; }
	virtual LPCWSTR GetMacroExt(){ return L""; }
	virtual void RunMacro(LPCWSTR Source){}

	void ProcessAttach(HINSTANCE hInstance);
	void ProcessDettach();
	HINSTANCE GetInstance();
	HWND GetParentHwnd(){
		if(Plugin.m_lpDllPluginIfObj != NULL){
			return Plugin.m_lpDllPluginIfObj->m_hParentHwnd;
		}
		return NULL;
	}

	CExternalIfObj* GetIfObj(const DWORD dwIfObjType){
		switch(dwIfObjType){
		case IFOBJ_TYPE_PLUGIN:			return &Plugin;
		case IFOBJ_TYPE_EDITOR:			return &Editor;
		case IFOBJ_TYPE_MACRO:			return &Macro;
		case IFOBJ_TYPE_SMARTINDENT:	return &SmartIndent;
		case IFOBJ_TYPE_OUTLINE:		return &Outline;
		case IFOBJ_TYPE_COMPLEMENT:		return &Complement;
		default: return NULL;
		}
	}

	BOOL LoadString(UINT nID, WideString& strBuffer);

public:
	//PP_COMMAND
	virtual void OnPluginCommand(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_INSTALL
	//virtual void OnPluginInstall(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_UNINSTALL
	//virtual void OnPluginUninstall(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_APP_START
	//virtual void OnPluginAppStart(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_APP_END
	//virtual void OnPluginAppEnd(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_EDITOR_START
	virtual void OnPluginEditorStart(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_EDITOR_END
	virtual void OnPluginEditorEnd(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_DOCUMENT_OPEN
	virtual void OnPluginDocumentOpen(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_DOCUMENT_CLOSE
	virtual void OnPluginDocumentClose(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_DOCUMENT_BEFORE_SAVE
	virtual void OnPluginDocumentBeforeSave(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_DOCUMENT_AFTER_SAVE
	virtual void OnPluginDocumentAfterSave(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_OUTLINE
	virtual void OnPluginOutline(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_SMARTINDENT
	virtual void OnPluginSmartIndent(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_COMPLEMENT
	virtual void OnPluginComplement(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_COMPLEMENT_GLOBAL
	virtual void OnPluginComplementGlobal(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_MACRO
	virtual void OnPluginMacro(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_COMMAND
//TODO: 新しいコマンドI/Fはここに追加します。

public:	//library
	//SJIS(ACP) to UTF16
	static WideString to_wstr(const char* astr, const UINT nCodePage = CP_ACP){
		int nLength = ::MultiByteToWideChar(nCodePage, 0, astr, -1, NULL, 0);
		wchar_t* buffer = new wchar_t[nLength+1];
		int nRet = ::MultiByteToWideChar(nCodePage, 0, astr, -1, buffer, nLength);
		WideString wstr = buffer;
		delete[] buffer;
		return wstr;
	}

	static WideString to_wstr(const AnsiString& astr, const UINT nCodePage = CP_ACP){
		int nLength = ::MultiByteToWideChar(nCodePage, 0, astr.c_str(), -1, NULL, 0);
		wchar_t* buffer = new wchar_t[nLength+1];
		int nRet = ::MultiByteToWideChar(nCodePage, 0, astr.c_str(), -1, buffer, nLength);
		WideString wstr = buffer;
		delete[] buffer;
		return wstr;
	}

	//UTF16 to SJIS(ACP)
	static AnsiString to_astr(const wchar_t* wstr, const UINT nCodePage = CP_ACP){
		int nLength = ::WideCharToMultiByte(nCodePage, 0, wstr, -1, NULL, 0, NULL, NULL);
		char* buffer = new char[nLength+1];
		int nRet = ::WideCharToMultiByte(nCodePage, 0, wstr, -1, buffer, nLength, NULL, NULL);
		AnsiString astr = buffer;
		delete[] buffer;
		return astr;
	}

	static AnsiString to_astr(const WideString& wstr, const UINT nCodePage = CP_ACP){
		int nLength = ::WideCharToMultiByte(nCodePage, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
		char* buffer = new char[nLength+1];
		int nRet = ::WideCharToMultiByte(nCodePage, 0, wstr.c_str(), -1, buffer, nLength, NULL, NULL);
		AnsiString astr = buffer;
		delete[] buffer;
		return astr;
	}

	// 引数の型を取得する(拡張情報対応)
	static DWORD GetType(const MACRO_FUNC_INFO* info, const int nIndex){
		if(info != NULL){
			if(nIndex < 4){
				return info->m_varArguments[nIndex];
			}
			MACRO_FUNC_INFO_EX* exp = (MACRO_FUNC_INFO_EX*)info->m_lpExData;
			if(exp != NULL){
				if(nIndex < exp->m_nArgSize){
					return exp->m_lpVarArgEx[nIndex - 4];
				}
			}
		}
		return VT_EMPTY;
	}

	// 引数の数を取得する(拡張情報対応)
	static int GetNumArgs(const MACRO_FUNC_INFO* info){
		int nArgs = 0;
		if(info != NULL){
			MACRO_FUNC_INFO_EX* exp = (MACRO_FUNC_INFO_EX*)info->m_lpExData;
			if(exp != NULL){
				nArgs = exp->m_nArgSize;
			}else{
				for(nArgs = 0; nArgs < 4; nArgs++){
					if(info->m_varArguments[nArgs] == VT_EMPTY) break;
				}
			}
		}
		return nArgs;
	}
};

/*
	多重呼び出しに対応するため呼び出し情報をこのクラスが生成されてから消滅するまで管理する
	objは呼び出しが終了した時点でsakura.exe側で解放されるため、モードレスダイアログ等で
	非同期で操作してはならない。
	非同期で操作したい場合はコピーを取るようにしなければならない。
*/
#define PLUGIN_INITIALIZE CBasePluginInitialize PluginInitialize(&thePluginService, obj);
class CBasePluginInitialize
{
public:
	CBasePluginInitialize(CBasePluginService* lpService, SAKURA_DLL_PLUGIN_OBJ* obj);
	virtual ~CBasePluginInitialize();

	CBasePluginService*		m_lpService;
	SAKURA_DLL_PLUGIN_OBJ*	m_BackupObject;

public:
	void Copy(SAKURA_DLL_PLUGIN_OBJ* src);
	static void EraseObj(SAKURA_DLL_PLUGIN_OBJ* obj);
};

#ifdef __MINGW32__
#define _countof(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#endif

#endif	//CBASEPLUGINSERVICE_H
