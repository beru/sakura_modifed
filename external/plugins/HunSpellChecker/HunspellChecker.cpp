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
// HunspellChecker.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "CPluginService.h"

///////////////////////////////////////////////////////////////////////////////
DLL_EXPORT void WINAPI HunspellChecker(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	PLUGIN_INITIALIZE;
	thePluginService.OnPluginHunspellChecker(obj, 0);
}

///////////////////////////////////////////////////////////////////////////////
DLL_EXPORT void WINAPI HunspellCheckerCurrent(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	PLUGIN_INITIALIZE;
	thePluginService.OnPluginHunspellChecker(obj, 1);
}

///////////////////////////////////////////////////////////////////////////////
DLL_EXPORT void WINAPI HunspellCheckerOneShot(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	PLUGIN_INITIALIZE;
	thePluginService.OnPluginHunspellChecker(obj, 2);
}
