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

#include "stdafx.h"
#include <windows.h>
#include "CDllLoader.h"

///////////////////////////////////////////////////////////////////////////////
CDllLoader::CDllLoader()
{
	m_hInstance = NULL;
}

///////////////////////////////////////////////////////////////////////////////
CDllLoader::~CDllLoader()
{
	Unload();
}

///////////////////////////////////////////////////////////////////////////////
BOOL CDllLoader::Load(LPCTSTR lpszDllName)
{
	if(IsAvailable()){
		return TRUE;
	}

	m_strDllName = lpszDllName;
	m_hInstance = LoadLibrary(m_strDllName.c_str());
	if(m_hInstance == NULL){
		return FALSE;
	}

	if(RegisterEntries() == FALSE){
		Unload();
		return FALSE;
	}

	OnLoad();

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
void CDllLoader::Unload()
{
	if(m_hInstance != NULL){
		OnUnload();

		::FreeLibrary( m_hInstance );
		m_hInstance = NULL;
		m_strDllName = L"";
	}
}

///////////////////////////////////////////////////////////////////////////////
BOOL CDllLoader::RegisterEntries(const ImportTable table[])
{
	if(IsAvailable() == FALSE) return FALSE;

	for(int i = 0; table[i].proc != NULL; i++){
		FARPROC proc;
		if((proc = ::GetProcAddress(GetInstance(), table[i].name)) == NULL){
			return FALSE;
		}
		*((FARPROC*)table[i].proc) = proc;
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CDllLoader::RegisterEntries(const ImportTable2 table[])
{
	if(IsAvailable() == FALSE) return FALSE;

	for(int i = 0; table[i].proc != NULL; i++){
		FARPROC proc;
		if((proc = ::GetProcAddress(GetInstance(), table[i].name)) == NULL){
			if(table[i].flag != DLL_ENTRY_OPTIONAL){
				return FALSE;
			}
		}
		*((FARPROC*)table[i].proc) = proc;
	}
	return TRUE;
}
