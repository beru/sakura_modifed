/*!	@file
	@brief DLLÇÃÉçÅ[Éh
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

#ifndef _PLUGIN_DLL_LOADER_H_37B3EE4142CE_4F45_B228_DA2BFD80375C
#define _PLUGIN_DLL_LOADER_H_37B3EE4142CE_4F45_B228_DA2BFD80375C

#include <windows.h>
#include <string>

///////////////////////////////////////////////////////////////////////////////
/*
	DLL loader
*/
class CDllLoader
{
public:
	CDllLoader();
	virtual ~CDllLoader();

	BOOL Load(LPCTSTR lpszDllName);
	void Unload();

	LPCTSTR GetDllName() const { return m_strDllName.c_str(); }
	BOOL IsAvailable() const { return m_hInstance != NULL; }
	HINSTANCE GetInstance(){ return m_hInstance; }

	typedef struct tagImportTable {
		void*		proc;
		const char*	name;
	} ImportTable;

	enum tagDllEntryFlags {
		DLL_ENTRY_OPTIONAL = 1
	};

	typedef struct tagImportTable2 {
		void*		proc;
		const char*	name;
		int			flag;
	} ImportTable2;

protected:
	virtual BOOL RegisterEntries() = 0;
	BOOL RegisterEntries(const ImportTable table[]);
	BOOL RegisterEntries(const ImportTable2 table[]);
	virtual void OnLoad(){};
	virtual void OnUnload(){};

private:
	HINSTANCE		m_hInstance;
	std::wstring	m_strDllName;
};

#endif	//_PLUGIN_DLL_LOADER_H_37B3EE4142CE_4F45_B228_DA2BFD80375C
