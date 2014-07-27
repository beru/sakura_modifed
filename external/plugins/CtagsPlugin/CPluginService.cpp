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
#include "CTagsMakeDialog.h"
#include "CommonTools.h"
#include "CSqlite.h"
#include "resource.h"

///////////////////////////////////////////////////////////////////////////////
CPluginService::CPluginService()
{
}

///////////////////////////////////////////////////////////////////////////////
CPluginService::~CPluginService()
{
	RemoveAllCtagsInfoList(m_CtagsInfoList);
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::OnPluginCtagsOptionDialog(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	ReadProfile();

	COptionDialog dlg(&m_CtagsOption, &m_CtagsInfoList);
	if(dlg.DoModal(GetInstance(), GetParentHwnd()) == IDOK){
		WriteProfile();
	}
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::OnPluginCtagsJumpDialog(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	ReadProfile();

	CJumpListDialog dlg(&m_CtagsOption, &m_CtagsInfoList);
	dlg.ReadCtagsFile(_T(""), m_CtagsOption.m_dwMatchMode, m_CtagsOption.m_bIgnoreCase);
	dlg.DoModal(GetInstance(), GetParentHwnd());
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::OnPluginCtagsJump(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	ReadProfile();

	Editor.SelectWord();
	WideString strKeyword = Editor.GetSelectedString(0);
	CJumpListDialog dlg(&m_CtagsOption, &m_CtagsInfoList);
	dlg.ReadCtagsFile(strKeyword.c_str(), m_CtagsOption.m_dwMatchMode, m_CtagsOption.m_bIgnoreCase);
	dlg.DoModal(GetInstance(), GetParentHwnd());
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::ReadProfile()
{
	WideString strPluginPath = Plugin.GetPluginDir();
	WideString strIniFile = strPluginPath + L"\\" + PLUGIN_INI_FILE;
	wchar_t szBuffer[MAX_PATH_LENGTH];

	//ctags.exeパス
	memset(szBuffer, 0, _countof(szBuffer) * sizeof(wchar_t));
	GetPrivateProfileString(PROFILE_SECTION_GENERAL, PROFILE_KEY_CTAGS_EXE_PATH, L"", szBuffer, _countof(szBuffer), strIniFile.c_str());
	m_CtagsOption.m_strCtagsExePath = szBuffer;
	if(m_CtagsOption.m_strCtagsExePath.length() == 0){
		m_CtagsOption.m_strCtagsExePath = strPluginPath + L"\\" + PROFILE_DEF_CTAGS_EXE_PATH;
	}

	//sqlite3.dllパス
	memset(szBuffer, 0, _countof(szBuffer) * sizeof(wchar_t));
	GetPrivateProfileString(PROFILE_SECTION_GENERAL, PROFILE_KEY_SQLITE_DLL_PATH, L"", szBuffer, _countof(szBuffer), strIniFile.c_str());
	m_CtagsOption.m_strSqliteDllPath = szBuffer;
	if(m_CtagsOption.m_strSqliteDllPath.length() == 0){
		//m_CtagsOption.m_strSqliteDllPath = strPluginPath + L"\\" + PROFILE_DEF_SQLITE_DLL_PATH;
	}

	//最大検索数
	m_CtagsOption.m_dwMaxFind = GetPrivateProfileInt(PROFILE_SECTION_GENERAL, PROFILE_KEY_MAX_FIND, PROFILE_DEF_MAXFIND, strIniFile.c_str());
	//検索までの待ち時間
	m_CtagsOption.m_dwDelay = GetPrivateProfileInt(PROFILE_SECTION_GENERAL, PROFILE_KEY_DELAY, PROFILE_DEF_DELAY, strIniFile.c_str());
	//一致モード
	m_CtagsOption.m_dwMatchMode = GetPrivateProfileInt(PROFILE_SECTION_GENERAL, PROFILE_KEY_MATCH_MODE, PROFILE_DEF_MATCH_MODE, strIniFile.c_str());
	//大文字小文字を無視する
	m_CtagsOption.m_bIgnoreCase = (BOOL)GetPrivateProfileInt(PROFILE_SECTION_GENERAL, PROFILE_KEY_IGNORE_CASE, (DWORD)PROFILE_DEF_IGNORE_CASE, strIniFile.c_str());

	DWORD dwTotalCount = GetPrivateProfileInt(PROFILE_SECTION_CTAGS, PROFILE_KEY_COUNT, 0, strIniFile.c_str());
	m_CtagsOption.m_dwPrevListCount = dwTotalCount;

	RemoveAllCtagsInfoList(m_CtagsInfoList);
	for(DWORD i = 0; i < dwTotalCount; i++){
		CtagsInfo* info = new CtagsInfo;
		wchar_t szKey[256];

		//使用フラグ
		wsprintf(szKey, PROFILE_KEY_FLAG, (i + 1));
		DWORD dwFlag = GetPrivateProfileInt(PROFILE_SECTION_CTAGS, szKey, 0, strIniFile.c_str());
		info->m_bFlag = (dwFlag != 0) ? TRUE : FALSE;

		//CTAGS対象パス
		wsprintf(szKey, PROFILE_KEY_TARGET_PATH, (i + 1));
		memset(szBuffer, 0, _countof(szBuffer) * sizeof(wchar_t));
		GetPrivateProfileString(PROFILE_SECTION_CTAGS, szKey, L"", szBuffer, _countof(szBuffer), strIniFile.c_str());
		info->m_strTargetPath = szBuffer;

		//サブフォルダも対象にする
		wsprintf(szKey, PROFILE_KEY_SUBFOLDER, (i + 1));
		DWORD dwSubFolder = GetPrivateProfileInt(PROFILE_SECTION_CTAGS, szKey, 0, strIniFile.c_str());
		info->m_bSubFolder = (dwSubFolder != 0) ? TRUE : FALSE;

		//コマンドラインオプション
		wsprintf(szKey, PROFILE_KEY_OPTION, (i + 1));
		memset(szBuffer, 0, _countof(szBuffer) * sizeof(wchar_t));
		GetPrivateProfileString(PROFILE_SECTION_CTAGS, szKey, L"", szBuffer, _countof(szBuffer), strIniFile.c_str());
		info->m_strOption = szBuffer;

		//CTAGS結果ファイル
		wsprintf(szKey, PROFILE_KEY_UNIQ_ID, (i + 1));
		memset(szBuffer, 0, _countof(szBuffer) * sizeof(wchar_t));
		GetPrivateProfileString(PROFILE_SECTION_CTAGS, szKey, L"", szBuffer, _countof(szBuffer), strIniFile.c_str());
		info->m_dwUniqID = thePluginService.GetHexStringToDword(szBuffer);

		m_CtagsInfoList.push_back(info);
	}

	if(m_CtagsOption.m_strSqliteDllPath.length() != 0){
		if(Sqlite.Load(m_CtagsOption.m_strSqliteDllPath.c_str()) == FALSE){
			WideString strMessage;
			LoadString(IDS_STR_ERROR_SQLITE, strMessage);
			::MessageBox(GetParentHwnd(), strMessage.c_str(), GetPluginName(), MB_ICONEXCLAMATION | MB_OK);
			return;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::WriteProfile()
{
	WideString strPluginPath = Plugin.GetPluginDir();
	WideString strIniFile = strPluginPath + L"\\" + PLUGIN_INI_FILE;
	wchar_t szBuffer[MAX_PATH_LENGTH];

	//ctags.exeパス
	WritePrivateProfileString(PROFILE_SECTION_GENERAL, PROFILE_KEY_CTAGS_EXE_PATH, m_CtagsOption.m_strCtagsExePath.c_str(), strIniFile.c_str());
	//sqlite3.dllパス
	WritePrivateProfileString(PROFILE_SECTION_GENERAL, PROFILE_KEY_SQLITE_DLL_PATH, m_CtagsOption.m_strSqliteDllPath.c_str(), strIniFile.c_str());
	//最大検索数
	wsprintf(szBuffer, L"%d", m_CtagsOption.m_dwMaxFind);
	WritePrivateProfileString(PROFILE_SECTION_GENERAL, PROFILE_KEY_MAX_FIND, szBuffer, strIniFile.c_str());
	//検索までの待ち時間
	wsprintf(szBuffer, L"%d", m_CtagsOption.m_dwDelay);
	WritePrivateProfileString(PROFILE_SECTION_GENERAL, PROFILE_KEY_DELAY, szBuffer, strIniFile.c_str());
	//一致モード
	wsprintf(szBuffer, L"%d", m_CtagsOption.m_dwMatchMode);
	WritePrivateProfileString(PROFILE_SECTION_GENERAL, PROFILE_KEY_MATCH_MODE, szBuffer, strIniFile.c_str());
	//大文字小文字を無視する
	wsprintf(szBuffer, L"%d", (DWORD)m_CtagsOption.m_bIgnoreCase);
	WritePrivateProfileString(PROFILE_SECTION_GENERAL, PROFILE_KEY_IGNORE_CASE, szBuffer, strIniFile.c_str());

	DWORD i;
	for(DWORD i = 0; i < m_CtagsOption.m_dwPrevListCount; i++){
		wchar_t szKey[256];
		wsprintf(szKey, PROFILE_KEY_FLAG, (i + 1));
		WritePrivateProfileString(PROFILE_SECTION_CTAGS, szKey, NULL, strIniFile.c_str());
		wsprintf(szKey, PROFILE_KEY_TARGET_PATH, (i + 1));
		WritePrivateProfileString(PROFILE_SECTION_CTAGS, szKey, NULL, strIniFile.c_str());
		wsprintf(szKey, PROFILE_KEY_SUBFOLDER, (i + 1));
		WritePrivateProfileString(PROFILE_SECTION_CTAGS, szKey, NULL, strIniFile.c_str());
		wsprintf(szKey, PROFILE_KEY_OPTION, (i + 1));
		WritePrivateProfileString(PROFILE_SECTION_CTAGS, szKey, NULL, strIniFile.c_str());
		wsprintf(szKey, PROFILE_KEY_UNIQ_ID, (i + 1));
		WritePrivateProfileString(PROFILE_SECTION_CTAGS, szKey, NULL, strIniFile.c_str());
	}

	wsprintf(szBuffer, L"%d", m_CtagsInfoList.size());
	WritePrivateProfileString(PROFILE_SECTION_CTAGS, PROFILE_KEY_COUNT, szBuffer, strIniFile.c_str());
	i = 0;
	for(std::list<CtagsInfo*>::iterator it = m_CtagsInfoList.begin(); it != m_CtagsInfoList.end(); ++it){
		CtagsInfo* info = *it;
		wchar_t szKey[256];

		//使用フラグ
		wsprintf(szKey, PROFILE_KEY_FLAG, (i + 1));
		wsprintf(szBuffer, L"%d", info->m_bFlag ? 1 : 0);
		WritePrivateProfileString(PROFILE_SECTION_CTAGS, szKey, szBuffer, strIniFile.c_str());

		//CTAGS対象パス
		wsprintf(szKey, PROFILE_KEY_TARGET_PATH, (i + 1));
		WritePrivateProfileString(PROFILE_SECTION_CTAGS, szKey, info->m_strTargetPath.c_str(), strIniFile.c_str());

		//サブフォルダも対象にする
		wsprintf(szKey, PROFILE_KEY_SUBFOLDER, (i + 1));
		WideString strSubFolder = GetDwordToString(info->m_bSubFolder ? 1 : 0);
		WritePrivateProfileString(PROFILE_SECTION_CTAGS, szKey, strSubFolder.c_str(), strIniFile.c_str());

		//コマンドラインオプション
		wsprintf(szKey, PROFILE_KEY_OPTION, (i + 1));
		WritePrivateProfileString(PROFILE_SECTION_CTAGS, szKey, info->m_strOption.c_str(), strIniFile.c_str());

		//CTAGS結果ファイル
		wsprintf(szKey, PROFILE_KEY_UNIQ_ID, (i + 1));
		WideString strUniqID = GetDwordToHexString(info->m_dwUniqID);
		WritePrivateProfileString(PROFILE_SECTION_CTAGS, szKey, strUniqID.c_str(), strIniFile.c_str());

		i++;
	}
	m_CtagsOption.m_dwPrevListCount = m_CtagsInfoList.size();
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::RemoveAllCtagsInfoList(std::list<CtagsInfo*>& p)
{
	for(std::list<CtagsInfo*>::iterator it = p.begin(); it != p.end(); ++it){
		delete *it;
	}
	p.clear();
}

///////////////////////////////////////////////////////////////////////////////
WideString CPluginService::GetResultFile(const DWORD dwUniqID)
{
	WideString strResultFile = Plugin.GetPluginDir() + L"\\" + PROFILE_DEF_CTAGS_RESULT_PATH + L"\\" + GetDwordToHexString(dwUniqID) + L".ctags";
	return strResultFile;
}

///////////////////////////////////////////////////////////////////////////////
WideString CPluginService::GetResultDbName(const DWORD dwUniqID)
{
	WideString strResultFile = Plugin.GetPluginDir() + L"\\" + PROFILE_DEF_CTAGS_RESULT_PATH + L"\\" + GetDwordToHexString(dwUniqID) + L".sqlite";
	return strResultFile;
}

///////////////////////////////////////////////////////////////////////////////
DWORD CPluginService::GetUniqID()
{
	DWORD dwUniqID = (DWORD)time(NULL);
	while(1){
		WideString strFile = GetResultFile(dwUniqID);
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
