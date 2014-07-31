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

#ifndef CPLUGINSERVICE_H
#define CPLUGINSERVICE_H

#include <windows.h>
#include "CBasePluginService.h"
#include "CSqlite.h"

///////////////////////////////////////////////////////////////////////////////
#ifdef PRAGMA_PUSH4
#pragma pack(push, 4)
#endif

#ifdef __cplusplus
extern "C" {
#endif

DLL_EXPORT void WINAPI PluginCtagsJumpDialog(SAKURA_DLL_PLUGIN_OBJ* obj);
DLL_EXPORT void WINAPI PluginCtagsJump(SAKURA_DLL_PLUGIN_OBJ* obj);
DLL_EXPORT void WINAPI PluginCtagsOptionDialog(SAKURA_DLL_PLUGIN_OBJ* obj);

#ifdef __cplusplus
}
#endif

#ifdef PRAGMA_PUSH4
#pragma pack(pop)
#endif

///////////////////////////////////////////////////////////////////////////////
enum {
	MATCH_MODE_PERFECT = 0,	//!< 完全一致
	MATCH_MODE_BEGIN   = 1,	//!< 前方一致
	MATCH_MODE_ANY     = 2	//!< 部分一致
};

#define PLUGIN_INI_FILE					L"CtagsPlugin.ini"
#define PROFILE_SECTION_GENERAL			L"General"
#define PROFILE_KEY_CTAGS_EXE_PATH		L"CtagsExePath"		//!< ctags.exeパス
#define PROFILE_KEY_SQLITE_DLL_PATH		L"SqliteDllPath"	//!< sqlite3.dllパス
#define PROFILE_KEY_MAX_FIND			L"MaxFind"			//!< 最大検索数
#define PROFILE_KEY_DELAY				L"Delay"			//!< 検索までの待ち時間(ms)
#define PROFILE_KEY_MATCH_MODE			L"MatchMode"		//!< 一致モード
#define PROFILE_KEY_IGNORE_CASE			L"IgnoreCase"		//!< 大文字と小文字を無視する

#define PROFILE_SECTION_CTAGS			L"Ctags"
#define PROFILE_KEY_COUNT				L"Count"			//!< 件数
#define PROFILE_KEY_FLAG				L"Flag%d"			//!< 使用するか
#define PROFILE_KEY_TARGET_PATH			L"TargetPath%d"		//!< 対象パス
#define PROFILE_KEY_SUBFOLDER			L"SubFolder%d"		//!< サブフォルダも対象にする
#define PROFILE_KEY_OPTION				L"Option%d"			//!< 追加のオプション
#define PROFILE_KEY_UNIQ_ID				L"UniqID%d"			//!< 識別ID

#define PROFILE_DEF_CTAGS_EXE_PATH		L"ctags.exe"
#define PROFILE_DEF_SQLITE_DLL_PATH		L"sqlite3.dll"
#define PROFILE_DEF_CTAGS_RESULT_PATH	L"Result"			//!< 結果ファイル格納パス
#define PROFILE_DEF_MAXFIND				1000				//!< 最大検索数
#define PROFILE_DEF_DELAY				1000				//!< 検索までの待ち時間
#define PROFILE_DEF_MATCH_MODE			MATCH_MODE_BEGIN	//!< 一致モード
#define PROFILE_DEF_IGNORE_CASE			FALSE				//!< 大文字と小文字を無視する
#define PROFILE_DEF_FLAG				TRUE
#define PROFILE_DEF_SUBFOLDER			TRUE
#define PROFILE_DEF_OPTION				L""	//L"--excmd=number"

#define TAG_FORMAT L"%[^\t\r\n]\t%[^\t\r\n]\t%d;\"\t%[^\t\r\n]\t%[^\t\r\n]"

///////////////////////////////////////////////////////////////////////////////
class CtagsOption
{
public:
	WideString		m_strCtagsExePath;		//!< ctags.exeパス
	WideString		m_strSqliteDllPath;		//!< sqlite3.dllパス
	DWORD			m_dwMaxFind;			//!< 検索最大数
	DWORD			m_dwDelay;				//!< 検索までの待ち時間
	DWORD			m_dwMatchMode;			//!< 一致モード
	BOOL			m_bIgnoreCase;			//!< 大文字と小文字を無視する
	DWORD			m_dwPrevListCount;		//!< INI登録数(前回情報を削除するため)

	CtagsOption() {
		m_strCtagsExePath  = L"";
		m_strSqliteDllPath = L"";
		m_dwMaxFind        = PROFILE_DEF_MAXFIND;
		m_dwDelay          = PROFILE_DEF_DELAY;
		m_dwMatchMode      = PROFILE_DEF_MATCH_MODE;
		m_bIgnoreCase      = PROFILE_DEF_IGNORE_CASE;
		m_dwPrevListCount  = 0;
	}

	CtagsOption& operator = (const CtagsOption& src){
		if (this != &src) {
			m_strCtagsExePath  = src.m_strCtagsExePath;
			m_strSqliteDllPath = src.m_strSqliteDllPath;
			m_dwMaxFind        = src.m_dwMaxFind;
			m_dwDelay          = src.m_dwDelay;
			m_dwMatchMode      = src.m_dwMatchMode;
			m_bIgnoreCase      = src.m_bIgnoreCase;
			m_dwPrevListCount  = src.m_dwPrevListCount;
		}
		return *this;
	}
};

class CtagsInfo
{
public:
	BOOL			m_bFlag;			//!< 使用するかどうか
	WideString		m_strTargetPath;	//!< CTAGS化するフォルダ
	BOOL			m_bSubFolder;		//!< サブフォルダも対象にする
	WideString		m_strOption;		//!< コマンドラインオプション
	DWORD			m_dwUniqID;			//!< 結果ファイルID

	CtagsInfo(){
		m_bFlag         = PROFILE_DEF_FLAG;
		m_strTargetPath = L"";
		m_bSubFolder    = PROFILE_DEF_SUBFOLDER;
		m_strOption     = PROFILE_DEF_OPTION;
		m_dwUniqID      = 0;
	}

	CtagsInfo& operator = (const CtagsInfo& src) {
		if (this != &src) {
			m_bFlag         = src.m_bFlag;
			m_strTargetPath = src.m_strTargetPath;
			m_bSubFolder    = src.m_bSubFolder;
			m_strOption     = src.m_strOption;
			m_dwUniqID      = src.m_dwUniqID;
		}
		return *this;
	}
};

typedef struct tagLVCOLUMN_LAYOUT {
	DWORD	m_dwFmt;
	DWORD	m_dwWidth;
//	LPCWSTR	m_lpszText;
	UINT	m_nID;
} LVCOLUMN_LAYOUT;


///////////////////////////////////////////////////////////////////////////////
class CPluginService : public CBasePluginService
{
public:
	CPluginService();
	virtual ~CPluginService();

	CtagsOption				m_CtagsOption;
	std::list<CtagsInfo*>	m_CtagsInfoList;
	CSqlite					Sqlite;

public:
	virtual LPCWSTR GetPluginName(){ return L"CtagsPlugin"; }

public:
	void OnPluginCtagsJumpDialog(SAKURA_DLL_PLUGIN_OBJ* obj);
	void OnPluginCtagsJump(SAKURA_DLL_PLUGIN_OBJ* obj);
	void OnPluginCtagsOptionDialog(SAKURA_DLL_PLUGIN_OBJ* obj);

	void ReadProfile();
	void WriteProfile();
	void RemoveAllCtagsInfoList(std::list<CtagsInfo*>& p);
	WideString GetResultFile(const DWORD dwUniqID);
	WideString GetResultDbName(const DWORD dwUniqID);
	DWORD GetUniqID();
	WideString GetDwordToString(const DWORD dwValue);
	WideString GetDwordToHexString(const DWORD dwValue);
	DWORD GetHexStringToDword(LPCWSTR lpszValue);
};

///////////////////////////////////////////////////////////////////////////////
extern CPluginService thePluginService;

#endif	//CPLUGINSERVICE_H
