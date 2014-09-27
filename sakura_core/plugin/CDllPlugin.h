/*!	@file
	@brief DLLプラグインクラス

*/
/*
	Copyright (C) 2009, syat

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
#pragma once

#include "CPlugin.h"
#include "plugin/SakuraPlugin.h"

#define	PII_DLL							L"Dll"			// DLL情報
#define	PII_DLL_NAME					L"Name"			// 名前

//typedef void (WINAPI *DllPlugHandler)(SAKURA_DLL_PLUGIN_OBJ* obj);

class CDllPlug : public CPlug {
public:
	CDllPlug( CPlugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel )
		:
		CPlug( plugin, id, sJack, sHandler, sLabel ),
		m_handler( NULL )
	{
	}
public:
	DllPlugHandler m_handler;
};

class CDllPlugin :
	public CPlugin,
	public CDllImp
{
	// コンストラクタ
public:
	CDllPlugin( const tstring& sBaseDir )
		:
		CPlugin( sBaseDir ),
		CDllImp()
	{
	}

	// デストラクタ
public:
	~CDllPlugin(void);

	// 実装
public:
	bool ReadPluginDef( CDataProfile *cProfile, CDataProfile *cProfileMlang );
	bool ReadPluginOption( CDataProfile *cProfile ) {
		return true;
	}
	CPlug* CreatePlug( CPlugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel );
	CPlug::Array GetPlugs() const{
		return m_plugs;
	}
	bool InvokePlug( CEditView* view, CPlug& plug, CWSHIfObj::List& params );

	bool InitDllImp() {
		return true;
	}
	LPCTSTR GetDllNameImp(int nIndex) {
		return _T("");
	}

	// メンバ変数
private:
	wstring m_sDllName;

	// DLLプラグイン
private:
	static SAKURA_DLL_PLUGIN_OBJ* CreateIfObj(CEditView* view, CWSHIfObj::List& params);
	static void EraseIfObj(SAKURA_DLL_PLUGIN_OBJ* obj);
	static BOOL WINAPI HandleFunctionCallback(LPCWSTR lpszName, LPVOID lpIfObj, LPVOID lpEditView, const DWORD ID, const VARIANT* Arguments, const int ArgSize, VARIANT* Result);
	static void WINAPI HandleCommandCallback(LPCWSTR lpszName, LPVOID lpIfObj, LPVOID lpEditView, const DWORD ID, LPCWSTR Arguments[], const int ArgLengths[], const int ArgSize);
};

