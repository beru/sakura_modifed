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
#include "CPluginService.h"
#include "plugin/SakuraPlugin.h"

///////////////////////////////////////////////////////////////////////////////
CPluginService::CPluginService()
{
}

///////////////////////////////////////////////////////////////////////////////
CPluginService::~CPluginService()
{
}

///////////////////////////////////////////////////////////////////////////////
//PP_COMPLEMENT
void CPluginService::OnPluginComplement(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	DoHelloComplement(0);
}

///////////////////////////////////////////////////////////////////////////////
//PP_COMPLEMENT_GLOBAL
void CPluginService::OnPluginComplementGlobal(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	DoHelloComplement(1);
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::DoHelloComplement(const int nType)
{
	switch(nType){
	case 0:
		Complement.AddList(L"Orange");
		Complement.AddList(L"オレンジ");
		Complement.AddList(L"ミカン");
		Complement.AddList(L"みかん");
		Complement.AddList(L"蜜柑");
		break;
	case 1:
		Complement.AddList(L"Cherry blossom");
		Complement.AddList(L"サクラ");
		Complement.AddList(L"さくら");
		Complement.AddList(L"桜");
		break;
	}
}
