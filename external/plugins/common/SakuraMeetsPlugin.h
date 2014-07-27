/*!	@file
	@brief DLLプラグインI/F
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

#ifndef SAKURAMEETSPLUGIN_H_0084E6A6_CD9B_4E41_9128_9E90C08BEB6F
#define SAKURAMEETSPLUGIN_H_0084E6A6_CD9B_4E41_9128_9E90C08BEB6F

#include <windows.h>
#include "plugin/SakuraPlugin.h"

#ifdef PLUGIN_EXPORTS
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif

#ifdef PRAGMA_PUSH4
#pragma pack(push, 4)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// DLLインスタンスを設定する
void ProcessAttach(HINSTANCE hInstance);
// DLLインスタンスを取得する
HINSTANCE GetPluginDllInstance();
// DLLリソースを解放する
void ProcessDetach();

//PP_COMMAND
DLL_EXPORT void WINAPI PluginCommand(SAKURA_DLL_PLUGIN_OBJ* obj);
//PP_INSTALL
//DLL_EXPORT void WINAPI PluginInstall(SAKURA_DLL_PLUGIN_OBJ* obj);
//PP_UNINSTALL
//DLL_EXPORT void WINAPI PluginUninstall(SAKURA_DLL_PLUGIN_OBJ* obj);
//PP_APP_START
//DLL_EXPORT void WINAPI PluginAppStart(SAKURA_DLL_PLUGIN_OBJ* obj);
//PP_APP_END
//DLL_EXPORT void WINAPI PluginAppEnd(SAKURA_DLL_PLUGIN_OBJ* obj);
//PP_EDITOR_START
DLL_EXPORT void WINAPI PluginEditorStart(SAKURA_DLL_PLUGIN_OBJ* obj);
//PP_EDITOR_END
DLL_EXPORT void WINAPI PluginEditorEnd(SAKURA_DLL_PLUGIN_OBJ* obj);
//PP_DOCUMENT_OPEN
DLL_EXPORT void WINAPI PluginDocumentOpen(SAKURA_DLL_PLUGIN_OBJ* obj);
//PP_DOCUMENT_CLOSE
DLL_EXPORT void WINAPI PluginDocumentClose(SAKURA_DLL_PLUGIN_OBJ* obj);
//PP_DOCUMENT_BEFORE_SAVE
DLL_EXPORT void WINAPI PluginDocumentBeforeSave(SAKURA_DLL_PLUGIN_OBJ* obj);
//PP_DOCUMENT_AFTER_SAVE
DLL_EXPORT void WINAPI PluginDocumentAfterSave(SAKURA_DLL_PLUGIN_OBJ* obj);
//PP_OUTLINE
DLL_EXPORT void WINAPI PluginOutline(SAKURA_DLL_PLUGIN_OBJ* obj);
//PP_SMARTINDENT
DLL_EXPORT void WINAPI PluginSmartIndent(SAKURA_DLL_PLUGIN_OBJ* obj);
//PP_COMPLEMENT
DLL_EXPORT void WINAPI PluginComplement(SAKURA_DLL_PLUGIN_OBJ* obj);
//PP_COMPLEMENT_GLOBAL
DLL_EXPORT void WINAPI PluginComplementGlobal(SAKURA_DLL_PLUGIN_OBJ* obj);
//PP_MACRO
DLL_EXPORT void WINAPI PluginMacro(SAKURA_DLL_PLUGIN_OBJ* obj);
//PP_COMMAND

#ifdef __cplusplus
}
#endif

#ifdef PRAGMA_PUSH4
#pragma pack(pop)
#endif

#endif	//SAKURAMEETSPLUGIN_H_0084E6A6_CD9B_4E41_9128_9E90C08BEB6F
