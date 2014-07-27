// CSharpPlugin.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "CPluginService.h"

///////////////////////////////////////////////////////////////////////////////
DLL_EXPORT void WINAPI PluginCscCompile(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	PLUGIN_INITIALIZE;
	thePluginService.OnPluginCscCompile(obj);
}

///////////////////////////////////////////////////////////////////////////////
DLL_EXPORT void WINAPI PluginCscCompileAndRun(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	PLUGIN_INITIALIZE;
	thePluginService.OnPluginCscCompileAndRun(obj);
}
