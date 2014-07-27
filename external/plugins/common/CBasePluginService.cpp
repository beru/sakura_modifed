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

#include "stdafx.h"
#include <windows.h>
#include "CBasePluginService.h"
#include <map>

///////////////////////////////////////////////////////////////////////////////
// CBasePluginInitialize
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
CBasePluginInitialize::CBasePluginInitialize(CBasePluginService* lpService, SAKURA_DLL_PLUGIN_OBJ* obj)
{
	m_lpService = lpService;
	if(m_lpService != NULL){
		m_BackupObject = m_lpService->Plugin.m_lpDllPluginIfObj;
		m_lpService->Initialize(obj);
		//最初のオブジェクトをコピーとして保存する
		Copy(obj);
		if (m_BackupObject == NULL){
			m_BackupObject = m_lpService->m_lpCloneObj;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
CBasePluginInitialize::~CBasePluginInitialize()
{
	if(m_lpService != NULL){
		m_lpService->Initialize(m_BackupObject);
		m_BackupObject = NULL;
		m_lpService = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
/*
	呼び出されたobjのコピーを保持する。
	SAKURA_DLL_PLUGIN_IF_OBJは新規のものがあれば追加保持する。
	そのほかは最初の呼び出しのものを保持する。
	注意：コピーした情報の中には使えないポインタもあるが、DLL側では使わないのでそのまま。
*/
void CBasePluginInitialize::Copy(SAKURA_DLL_PLUGIN_OBJ* src)
{
	if (m_lpService->m_lpCloneObj == NULL){
		//最初なので丸ごとコピーする
		m_lpService->m_lpCloneObj = new SAKURA_DLL_PLUGIN_OBJ;
		if (m_lpService->m_lpCloneObj != NULL){
			memcpy(m_lpService->m_lpCloneObj, src, sizeof(SAKURA_DLL_PLUGIN_OBJ));
			m_lpService->m_lpCloneObj->m_dwIfObjListCount = 0;
			if (src->m_dwIfObjListCount > 0){
				int nCount = src->m_dwIfObjListCount;
				m_lpService->m_lpCloneObj->m_IfObjList = new SAKURA_DLL_PLUGIN_IF_OBJ[nCount];
				if (m_lpService->m_lpCloneObj->m_IfObjList != NULL){
					m_lpService->m_lpCloneObj->m_dwIfObjListCount = src->m_dwIfObjListCount;
					memcpy(m_lpService->m_lpCloneObj->m_IfObjList, src->m_IfObjList, nCount * sizeof(SAKURA_DLL_PLUGIN_IF_OBJ));
				}
			}
		}
	}else{
		//今回新しく渡された情報を追加コピーする
		for (DWORD i = 0; i < src->m_dwIfObjListCount; i++){
			bool result = false;
			for (DWORD j = 0; j < m_lpService->m_lpCloneObj->m_dwIfObjListCount; j++){
				if (wcscmp(src->m_IfObjList[i].m_szName, m_lpService->m_lpCloneObj->m_IfObjList[j].m_szName) == 0){
					result = true;
					break;
				}
			}
			if (result == false){
				int nOldCount = m_lpService->m_lpCloneObj->m_dwIfObjListCount;
				int nNewCount = nOldCount + 1;
				SAKURA_DLL_PLUGIN_IF_OBJ* p = new SAKURA_DLL_PLUGIN_IF_OBJ[nNewCount];
				if (p != NULL){
					memcpy(p, m_lpService->m_lpCloneObj->m_IfObjList, nOldCount * sizeof(SAKURA_DLL_PLUGIN_IF_OBJ));
					memcpy(&p[nOldCount], &src->m_IfObjList[i], 1 * sizeof(SAKURA_DLL_PLUGIN_IF_OBJ));
					m_lpService->m_lpCloneObj->m_dwIfObjListCount = nNewCount;
					delete[] m_lpService->m_lpCloneObj->m_IfObjList;
					m_lpService->m_lpCloneObj->m_IfObjList = p;
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void CBasePluginInitialize::EraseObj(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	if(obj != NULL){
		if(obj->m_IfObjList != NULL){
			delete[] obj->m_IfObjList;
		}
		delete obj;
	}
}

///////////////////////////////////////////////////////////////////////////////
// CBasePluginService
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
CBasePluginService::CBasePluginService()
{
	m_lpCloneObj = NULL;
}

///////////////////////////////////////////////////////////////////////////////
CBasePluginService::~CBasePluginService()
{
	Uninitialize();
}

///////////////////////////////////////////////////////////////////////////////
void CBasePluginService::Uninitialize()
{
	CBasePluginInitialize::EraseObj(m_lpCloneObj);
	m_lpCloneObj = NULL;

	for(std::map<LANGID, HMODULE>::iterator it = m_mapLangModule.begin(); it != m_mapLangModule.end(); ++it){
		if(it->second != NULL){
			::FreeLibrary(it->second);
		}
	}
	m_mapLangModule.clear();
}

///////////////////////////////////////////////////////////////////////////////
//初期化
BOOL CBasePluginService::Initialize(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	//リソースDLLを準備する()
	if(obj != NULL){
		std::map<LANGID, HMODULE>::iterator it = m_mapLangModule.find(obj->m_wLangId);
		if(it == m_mapLangModule.end()){
			wchar_t szModule[_MAX_PATH] = L"";
			if(::GetModuleFileName(GetPluginDllInstance(), szModule, _countof(szModule)) != 0){
				int length = wcslen(szModule);
				if(length >= 4){
					wchar_t szBuffer[16];
					swprintf(szBuffer, L"_%d.dll", obj->m_wLangId);	//"xxx_1033.dll"
					szModule[length - 4] = L'\0';
					wcscat(szModule, szBuffer);
					HMODULE hModule = ::LoadLibrary(szModule);
					if(hModule != NULL){
						m_mapLangModule[obj->m_wLangId] = hModule;
					}
				}
			}
		}
	}

	Plugin.Initialize(obj);
	Editor.Initialize(obj);
	Macro.Initialize(obj);
	SmartIndent.Initialize(obj);
	Outline.Initialize(obj);
	Complement.Initialize(obj);
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
HINSTANCE CBasePluginService::GetInstance()
{
	//言語のリソースDLLのインスタンスを取得する
	if(Plugin.m_lpDllPluginIfObj != NULL){
		std::map<LANGID, HMODULE>::iterator it = m_mapLangModule.find(Plugin.m_lpDllPluginIfObj->m_wLangId);
		if(it != m_mapLangModule.end()){
			return it->second;
		}
	}
	//デフォルトのインスタンスを返す
	return GetPluginDllInstance();
}

///////////////////////////////////////////////////////////////////////////////
BOOL CBasePluginService::LoadString(UINT nID, WideString& strBuffer)
{
	wchar_t szBuffer[1024];
	szBuffer[0] = L'\0';
	::LoadString(GetInstance(), nID, szBuffer, _countof(szBuffer));
	strBuffer = szBuffer;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//PP_COMMAND
void CBasePluginService::OnPluginCommand(SAKURA_DLL_PLUGIN_OBJ* obj)
{
}
/*
///////////////////////////////////////////////////////////////////////////////
//PP_INSTALL
void CBasePluginService::OnPluginInstall(SAKURA_DLL_PLUGIN_OBJ* obj)
{
}

///////////////////////////////////////////////////////////////////////////////
//PP_UNINSTALL
void CBasePluginService::OnPluginUninstall(SAKURA_DLL_PLUGIN_OBJ* obj)
{
}

///////////////////////////////////////////////////////////////////////////////
//PP_APP_START
void CBasePluginService::OnPluginAppStart(SAKURA_DLL_PLUGIN_OBJ* obj)
{
}

///////////////////////////////////////////////////////////////////////////////
//PP_APP_END
void CBasePluginService::OnPluginAppEnd(SAKURA_DLL_PLUGIN_OBJ* obj)
{
}
*/
///////////////////////////////////////////////////////////////////////////////
//PP_EDITOR_START
void CBasePluginService::OnPluginEditorStart(SAKURA_DLL_PLUGIN_OBJ* obj)
{
}

///////////////////////////////////////////////////////////////////////////////
//PP_EDITOR_END
void CBasePluginService::OnPluginEditorEnd(SAKURA_DLL_PLUGIN_OBJ* obj)
{
}

///////////////////////////////////////////////////////////////////////////////
//PP_DOCUMENT_OPEN
void CBasePluginService::OnPluginDocumentOpen(SAKURA_DLL_PLUGIN_OBJ* obj)
{
}

///////////////////////////////////////////////////////////////////////////////
//PP_DOCUMENT_CLOSE
void CBasePluginService::OnPluginDocumentClose(SAKURA_DLL_PLUGIN_OBJ* obj)
{
}

///////////////////////////////////////////////////////////////////////////////
//PP_DOCUMENT_BEFORE_SAVE
void CBasePluginService::OnPluginDocumentBeforeSave(SAKURA_DLL_PLUGIN_OBJ* obj)
{
}

///////////////////////////////////////////////////////////////////////////////
//PP_DOCUMENT_AFTER_SAVE
void CBasePluginService::OnPluginDocumentAfterSave(SAKURA_DLL_PLUGIN_OBJ* obj)
{
}

///////////////////////////////////////////////////////////////////////////////
//PP_OUTLINE
void CBasePluginService::OnPluginOutline(SAKURA_DLL_PLUGIN_OBJ* obj)
{
}

///////////////////////////////////////////////////////////////////////////////
//PP_SMARTINDENT
void CBasePluginService::OnPluginSmartIndent(SAKURA_DLL_PLUGIN_OBJ* obj)
{
}

///////////////////////////////////////////////////////////////////////////////
//PP_COMPLEMENT
void CBasePluginService::OnPluginComplement(SAKURA_DLL_PLUGIN_OBJ* obj)
{
}

///////////////////////////////////////////////////////////////////////////////
//PP_COMPLEMENT_GLOBAL
void CBasePluginService::OnPluginComplementGlobal(SAKURA_DLL_PLUGIN_OBJ* obj)
{
}

///////////////////////////////////////////////////////////////////////////////
//PP_MACRO
void CBasePluginService::OnPluginMacro(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	if(Macro.IsAvailable() == FALSE){
		return;
	}

	switch(Macro.GetMode()){
	case CExternalMacroIfObj::MACRO_MODE_CREATOR:	//MACRO_MODE_CREATOR
		if(wcscmp(Macro.GetExt().c_str(), GetMacroExt()) == 0){
			Macro.SetMatch(1);
		}else{
			Macro.SetMatch(0);
		}
		break;
	case CExternalMacroIfObj::MACRO_MODE_EXEC:	//MACRO_MODE_EXEC
		if(Editor.IsAvailable()){
			Editor.SetFlags(Macro.GetFlags());
			RunMacro(Macro.GetSource().c_str());
		}
		break;
	default:
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
//PP_COMMAND

//TODO: 新しいコマンドI/Fはここに追加します。
