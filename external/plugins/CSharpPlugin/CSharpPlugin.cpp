// CSharpPlugin.cpp : DLL �A�v���P�[�V�����p�ɃG�N�X�|�[�g�����֐����`���܂��B
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
