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
#include <time.h>
#include "CPluginService.h"
#include "plugin/SakuraPlugin.h"
#include "COptionDialog.h"
#include "CJumpListDialog.h"
#include "CMakeDialog.h"
#include "CommonTools.h"

///////////////////////////////////////////////////////////////////////////////
CPluginService::CPluginService()
{
}

///////////////////////////////////////////////////////////////////////////////
CPluginService::~CPluginService()
{
	RemoveAllGlobalInfoList(m_GlobalInfoList);
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::OnPluginGlobalOptionDialog(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	//設定を読み込む
	thePluginService.ReadProfile();

	COptionDialog dlg(&m_GlobalOption, &m_GlobalInfoList);
	if(dlg.DoModal(GetInstance(), GetParentHwnd()) == IDOK){
		WriteProfile();
	}
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::OnPluginGlobalJumpDialog(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	//設定を読み込む
	thePluginService.ReadProfile();

	CJumpListDialog dlg(&m_GlobalOption, &m_GlobalInfoList);
	dlg.ReadGlobalFile(_T(""), m_GlobalOption.m_dwMatchMode, m_GlobalOption.m_bIgnoreCase, m_GlobalOption.m_bSymbol, m_GlobalOption.m_bRef);
	
	dlg.DoModal(GetInstance(), GetParentHwnd());
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::OnPluginGlobalJump(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	//設定を読み込む
	thePluginService.ReadProfile();

	Editor.SelectWord();
	WideString strKeyword = Editor.GetSelectedString(0);

	CJumpListDialog dlg(&m_GlobalOption, &m_GlobalInfoList);
	dlg.ReadGlobalFile(strKeyword.c_str(), m_GlobalOption.m_dwMatchMode, m_GlobalOption.m_bIgnoreCase, m_GlobalOption.m_bSymbol, m_GlobalOption.m_bRef);

	dlg.DoModal(GetInstance(), GetParentHwnd());
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::ReadProfile()
{
	WideString strPluginPath = Plugin.GetPluginDir();
	WideString strIniFile = strPluginPath + L"\\" + PLUGIN_INI_FILE;
	wchar_t szBuffer[MAX_PATH_LENGTH];

	//gtags.exeパス
	memset(szBuffer, 0, sizeof(szBuffer));
	GetPrivateProfileString(PROFILE_SECTION_GENERAL, PROFILE_KEY_GTAGS_EXE_PATH, L"", szBuffer, _countof(szBuffer), strIniFile.c_str());
	m_GlobalOption.m_strGtagsExePath = szBuffer;
	if(m_GlobalOption.m_strGtagsExePath.length() == 0){
		m_GlobalOption.m_strGtagsExePath = strPluginPath + L"\\" + PROFILE_DEF_GTAGS_EXE_PATH;
	}

	//global.exeパス
	memset(szBuffer, 0, sizeof(szBuffer));
	GetPrivateProfileString(PROFILE_SECTION_GENERAL, PROFILE_KEY_GLOBAL_EXE_PATH, L"", szBuffer, _countof(szBuffer), strIniFile.c_str());
	m_GlobalOption.m_strGlobalExePath = szBuffer;
	if(m_GlobalOption.m_strGlobalExePath.length() == 0){
		m_GlobalOption.m_strGlobalExePath = strPluginPath + L"\\" + PROFILE_DEF_GLOBAL_EXE_PATH;
	}

	//最大検索数
	m_GlobalOption.m_dwMaxFind = GetPrivateProfileInt(PROFILE_SECTION_GENERAL, PROFILE_KEY_MAX_FIND, PROFILE_DEF_MAXFIND, strIniFile.c_str());
	//検索までの待ち時間
	m_GlobalOption.m_dwDelay = GetPrivateProfileInt(PROFILE_SECTION_GENERAL, PROFILE_KEY_DELAY, PROFILE_DEF_DELAY, strIniFile.c_str());
	//一致モード
	m_GlobalOption.m_dwMatchMode = GetPrivateProfileInt(PROFILE_SECTION_GENERAL, PROFILE_KEY_MATCH_MODE, PROFILE_DEF_MATCH_MODE, strIniFile.c_str());
	//大文字小文字を無視する
	m_GlobalOption.m_bIgnoreCase = (BOOL)GetPrivateProfileInt(PROFILE_SECTION_GENERAL, PROFILE_KEY_IGNORE_CASE, (DWORD)PROFILE_DEF_IGNORE_CASE, strIniFile.c_str());
	//シンボルを検索する
	m_GlobalOption.m_bSymbol = (BOOL)GetPrivateProfileInt(PROFILE_SECTION_GENERAL, PROFILE_KEY_SYMBOL, (DWORD)PROFILE_DEF_SYMBOL, strIniFile.c_str());
	//参照を検索する
	m_GlobalOption.m_bRef = (BOOL)GetPrivateProfileInt(PROFILE_SECTION_GENERAL, PROFILE_KEY_REF, (DWORD)PROFILE_DEF_REF, strIniFile.c_str());

	DWORD dwTotalCount = GetPrivateProfileInt(PROFILE_SECTION_GLOBAL, PROFILE_KEY_COUNT, 0, strIniFile.c_str());
	m_GlobalOption.m_dwPrevListCount = dwTotalCount;

	RemoveAllGlobalInfoList(m_GlobalInfoList);
	for(DWORD i = 0; i < dwTotalCount; i++){
		CGlobalInfo* info = new CGlobalInfo;
		wchar_t szKey[256];

		//使用フラグ
		wsprintf(szKey, PROFILE_KEY_FLAG, (i + 1));
		memset(szBuffer, 0, sizeof(szBuffer));
		DWORD dwFlag = GetPrivateProfileInt(PROFILE_SECTION_GLOBAL, szKey, 0, strIniFile.c_str());
		info->m_bFlag = (dwFlag != 0) ? TRUE : FALSE;

		//GTAGS対象パス
		wsprintf(szKey, PROFILE_KEY_TARGET_PATH, (i + 1));
		memset(szBuffer, 0, sizeof(szBuffer));
		GetPrivateProfileString(PROFILE_SECTION_GLOBAL, szKey, L"", szBuffer, _countof(szBuffer), strIniFile.c_str());
		info->m_strTargetPath = szBuffer;

		//GTAGS結果ファイル
		wsprintf(szKey, PROFILE_KEY_UNIQ_ID, (i + 1));
		memset(szBuffer, 0, sizeof(szBuffer));
		GetPrivateProfileString(PROFILE_SECTION_GLOBAL, szKey, L"", szBuffer, _countof(szBuffer), strIniFile.c_str());
		info->m_dwUniqID = thePluginService.GetHexStringToDword(szBuffer);

		m_GlobalInfoList.push_back(info);
	}
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::WriteProfile()
{
	WideString strPluginPath = Plugin.GetPluginDir();
	WideString strIniFile = strPluginPath + L"\\" + PLUGIN_INI_FILE;
	wchar_t szBuffer[MAX_PATH_LENGTH];

	//gtags.exeパス
	WritePrivateProfileString(PROFILE_SECTION_GENERAL, PROFILE_KEY_GTAGS_EXE_PATH, m_GlobalOption.m_strGtagsExePath.c_str(), strIniFile.c_str());
	//global.exeパス
	WritePrivateProfileString(PROFILE_SECTION_GENERAL, PROFILE_KEY_GLOBAL_EXE_PATH, m_GlobalOption.m_strGlobalExePath.c_str(), strIniFile.c_str());
	//最大検索数
	wsprintf(szBuffer, L"%d", m_GlobalOption.m_dwMaxFind);
	WritePrivateProfileString(PROFILE_SECTION_GENERAL, PROFILE_KEY_MAX_FIND, szBuffer, strIniFile.c_str());
	//検索までの待ち時間
	wsprintf(szBuffer, L"%d", m_GlobalOption.m_dwDelay);
	WritePrivateProfileString(PROFILE_SECTION_GENERAL, PROFILE_KEY_DELAY, szBuffer, strIniFile.c_str());
	//一致モード
	wsprintf(szBuffer, L"%d", m_GlobalOption.m_dwMatchMode);
	WritePrivateProfileString(PROFILE_SECTION_GENERAL, PROFILE_KEY_MATCH_MODE, szBuffer, strIniFile.c_str());
	//大文字小文字を無視する
	wsprintf(szBuffer, L"%d", (DWORD)m_GlobalOption.m_bIgnoreCase);
	WritePrivateProfileString(PROFILE_SECTION_GENERAL, PROFILE_KEY_IGNORE_CASE, szBuffer, strIniFile.c_str());
	//シンボルを検索する
	wsprintf(szBuffer, L"%d", (DWORD)m_GlobalOption.m_bSymbol);
	WritePrivateProfileString(PROFILE_SECTION_GENERAL, PROFILE_KEY_SYMBOL, szBuffer, strIniFile.c_str());
	//参照を検索する
	wsprintf(szBuffer, L"%d", (DWORD)m_GlobalOption.m_bRef);
	WritePrivateProfileString(PROFILE_SECTION_GENERAL, PROFILE_KEY_REF, szBuffer, strIniFile.c_str());

	DWORD i;
	for(DWORD i = 0; i < m_GlobalOption.m_dwPrevListCount; i++){
		wchar_t szKey[256];
		wsprintf(szKey, PROFILE_KEY_FLAG, (i + 1));
		WritePrivateProfileString(PROFILE_SECTION_GLOBAL, szKey, NULL, strIniFile.c_str());
		wsprintf(szKey, PROFILE_KEY_TARGET_PATH, (i + 1));
		WritePrivateProfileString(PROFILE_SECTION_GLOBAL, szKey, NULL, strIniFile.c_str());
		wsprintf(szKey, PROFILE_KEY_UNIQ_ID, (i + 1));
		WritePrivateProfileString(PROFILE_SECTION_GLOBAL, szKey, NULL, strIniFile.c_str());
	}

	wsprintf(szBuffer, L"%d", m_GlobalInfoList.size());
	WritePrivateProfileString(PROFILE_SECTION_GLOBAL, PROFILE_KEY_COUNT, szBuffer, strIniFile.c_str());
	i = 0;
	for(std::list<CGlobalInfo*>::iterator it = m_GlobalInfoList.begin(); it != m_GlobalInfoList.end(); ++it){
		CGlobalInfo* info = *it;
		wchar_t szKey[256];

		//使用フラグ
		wsprintf(szKey, PROFILE_KEY_FLAG, (i + 1));
		wsprintf(szBuffer, L"%d", info->m_bFlag ? 1 : 0);
		WritePrivateProfileString(PROFILE_SECTION_GLOBAL, szKey, szBuffer, strIniFile.c_str());

		//GTAGS対象パス
		wsprintf(szKey, PROFILE_KEY_TARGET_PATH, (i + 1));
		WritePrivateProfileString(PROFILE_SECTION_GLOBAL, szKey, info->m_strTargetPath.c_str(), strIniFile.c_str());

		//GTAGS結果ファイル
		wsprintf(szKey, PROFILE_KEY_UNIQ_ID, (i + 1));
		WideString strUniqID = GetDwordToHexString(info->m_dwUniqID);
		WritePrivateProfileString(PROFILE_SECTION_GLOBAL, szKey, strUniqID.c_str(), strIniFile.c_str());

		i++;
	}
	m_GlobalOption.m_dwPrevListCount = m_GlobalInfoList.size();
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::RemoveAllGlobalInfoList(std::list<CGlobalInfo*>& p)
{
	for(std::list<CGlobalInfo*>::iterator it = p.begin(); it != p.end(); ++it){
		delete *it;
	}
	p.clear();
}

///////////////////////////////////////////////////////////////////////////////
WideString CPluginService::GetResultPath(const DWORD dwUniqID)
{
	WideString strResultPath = Plugin.GetPluginDir() + L"\\" + PROFILE_DEF_GTAGS_RESULT_PATH + L"\\" + GetDwordToHexString(dwUniqID);
	return strResultPath;
}

///////////////////////////////////////////////////////////////////////////////
DWORD CPluginService::GetUniqID()
{
	DWORD dwUniqID = (DWORD)time(NULL);
	while(1){
		WideString strFile = GetResultPath(dwUniqID);
		struct _stat st;
		if(_wstat(strFile.c_str(), &st) != 0){
			return dwUniqID;
		}
		dwUniqID++;
	}
}

///////////////////////////////////////////////////////////////////////////////
WideString CPluginService::GetDwordToString(const DWORD dwValue)
{
	wchar_t szBuffer[64];
	wsprintf(szBuffer, L"%u", dwValue);
	WideString strResult = szBuffer;
	return strResult;
}

///////////////////////////////////////////////////////////////////////////////
WideString CPluginService::GetDwordToHexString(const DWORD dwValue)
{
	wchar_t szBuffer[64];
	wsprintf(szBuffer, L"%08x", dwValue);
	WideString strResult = szBuffer;
	return strResult;
}

///////////////////////////////////////////////////////////////////////////////
DWORD CPluginService::GetHexStringToDword(LPCWSTR lpszValue)
{
	unsigned int unValue = 0;
	swscanf(lpszValue, L"%08x", &unValue);
	return (DWORD)unValue;
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::RemoveResultPath(const DWORD dwUniqID, BOOL bFull)
{
	WideString strResultPath = thePluginService.GetResultPath(dwUniqID);
	WideString strResultFile;
	strResultFile = strResultPath + L"\\GPATH";
	::DeleteFile(strResultFile.c_str());
	strResultFile = strResultPath + L"\\GRTAGS";
	::DeleteFile(strResultFile.c_str());
	strResultFile = strResultPath + L"\\GTAGS";
	::DeleteFile(strResultFile.c_str());
	if(bFull){
		::RemoveDirectory(strResultPath.c_str());
	}
}

