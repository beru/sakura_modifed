/*
	Copyright (C) 2014, Plugins developers

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
#include "CPluginShareData.h"

///////////////////////////////////////////////////////////////////////////////
CPluginShareData::CPluginShareData()
{
	m_hFileMap = NULL;
	m_lpShareData = NULL;
}

///////////////////////////////////////////////////////////////////////////////
CPluginShareData::~CPluginShareData()
{
	Cleanup();
}

///////////////////////////////////////////////////////////////////////////////
bool CPluginShareData::Initialize(const DWORD dwVersion)
{
	if(IsReady()) return true;

	wchar_t szVersion[256];
#ifdef _DEBUG
	swprintf(szVersion, L"SakuraShareDataW_DEBUG%d", dwVersion);
#else
	swprintf(szVersion, L"SakuraShareDataW%d", dwVersion);
#endif

	m_hFileMap = ::OpenFileMapping(FILE_MAP_READ, FALSE, szVersion);
	if(m_hFileMap != NULL){
		m_lpShareData = (DLLSHAREDATA*)::MapViewOfFile(m_hFileMap, FILE_MAP_READ, 0, 0, 0);
		if(m_lpShareData != NULL){
			return true;
		}
		Cleanup();
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
void CPluginShareData::Cleanup()
{
	if(m_lpShareData !=NULL){
		::UnmapViewOfFile(m_lpShareData);
		m_lpShareData = NULL;
	}
	if(m_hFileMap != NULL){
		::CloseHandle(m_hFileMap);
		m_hFileMap = NULL;
	}
}
