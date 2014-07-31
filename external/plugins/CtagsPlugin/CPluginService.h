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
	MATCH_MODE_PERFECT = 0,	//!< ���S��v
	MATCH_MODE_BEGIN   = 1,	//!< �O����v
	MATCH_MODE_ANY     = 2	//!< ������v
};

#define PLUGIN_INI_FILE					L"CtagsPlugin.ini"
#define PROFILE_SECTION_GENERAL			L"General"
#define PROFILE_KEY_CTAGS_EXE_PATH		L"CtagsExePath"		//!< ctags.exe�p�X
#define PROFILE_KEY_SQLITE_DLL_PATH		L"SqliteDllPath"	//!< sqlite3.dll�p�X
#define PROFILE_KEY_MAX_FIND			L"MaxFind"			//!< �ő匟����
#define PROFILE_KEY_DELAY				L"Delay"			//!< �����܂ł̑҂�����(ms)
#define PROFILE_KEY_MATCH_MODE			L"MatchMode"		//!< ��v���[�h
#define PROFILE_KEY_IGNORE_CASE			L"IgnoreCase"		//!< �啶���Ə������𖳎�����

#define PROFILE_SECTION_CTAGS			L"Ctags"
#define PROFILE_KEY_COUNT				L"Count"			//!< ����
#define PROFILE_KEY_FLAG				L"Flag%d"			//!< �g�p���邩
#define PROFILE_KEY_TARGET_PATH			L"TargetPath%d"		//!< �Ώۃp�X
#define PROFILE_KEY_SUBFOLDER			L"SubFolder%d"		//!< �T�u�t�H���_���Ώۂɂ���
#define PROFILE_KEY_OPTION				L"Option%d"			//!< �ǉ��̃I�v�V����
#define PROFILE_KEY_UNIQ_ID				L"UniqID%d"			//!< ����ID

#define PROFILE_DEF_CTAGS_EXE_PATH		L"ctags.exe"
#define PROFILE_DEF_SQLITE_DLL_PATH		L"sqlite3.dll"
#define PROFILE_DEF_CTAGS_RESULT_PATH	L"Result"			//!< ���ʃt�@�C���i�[�p�X
#define PROFILE_DEF_MAXFIND				1000				//!< �ő匟����
#define PROFILE_DEF_DELAY				1000				//!< �����܂ł̑҂�����
#define PROFILE_DEF_MATCH_MODE			MATCH_MODE_BEGIN	//!< ��v���[�h
#define PROFILE_DEF_IGNORE_CASE			FALSE				//!< �啶���Ə������𖳎�����
#define PROFILE_DEF_FLAG				TRUE
#define PROFILE_DEF_SUBFOLDER			TRUE
#define PROFILE_DEF_OPTION				L""	//L"--excmd=number"

#define TAG_FORMAT L"%[^\t\r\n]\t%[^\t\r\n]\t%d;\"\t%[^\t\r\n]\t%[^\t\r\n]"

///////////////////////////////////////////////////////////////////////////////
class CtagsOption
{
public:
	WideString		m_strCtagsExePath;		//!< ctags.exe�p�X
	WideString		m_strSqliteDllPath;		//!< sqlite3.dll�p�X
	DWORD			m_dwMaxFind;			//!< �����ő吔
	DWORD			m_dwDelay;				//!< �����܂ł̑҂�����
	DWORD			m_dwMatchMode;			//!< ��v���[�h
	BOOL			m_bIgnoreCase;			//!< �啶���Ə������𖳎�����
	DWORD			m_dwPrevListCount;		//!< INI�o�^��(�O������폜���邽��)

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
	BOOL			m_bFlag;			//!< �g�p���邩�ǂ���
	WideString		m_strTargetPath;	//!< CTAGS������t�H���_
	BOOL			m_bSubFolder;		//!< �T�u�t�H���_���Ώۂɂ���
	WideString		m_strOption;		//!< �R�}���h���C���I�v�V����
	DWORD			m_dwUniqID;			//!< ���ʃt�@�C��ID

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
