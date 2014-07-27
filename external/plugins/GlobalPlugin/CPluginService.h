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

///////////////////////////////////////////////////////////////////////////////
#ifdef PRAGMA_PUSH4
#pragma pack(push, 4)
#endif

#ifdef __cplusplus
extern "C" {
#endif

DLL_EXPORT void WINAPI PluginGlobalJumpDialog(SAKURA_DLL_PLUGIN_OBJ* obj);
DLL_EXPORT void WINAPI PluginGlobalJump(SAKURA_DLL_PLUGIN_OBJ* obj);
DLL_EXPORT void WINAPI PluginGlobalOptionDialog(SAKURA_DLL_PLUGIN_OBJ* obj);

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

#define PLUGIN_INI_FILE					L"GlobalPlugin.ini"
#define PROFILE_SECTION_GENERAL			L"General"
#define PROFILE_KEY_GTAGS_EXE_PATH		L"GtagsExePath"		//!< gtags.exe�p�X
#define PROFILE_KEY_GLOBAL_EXE_PATH		L"GlobalExePath"	//!< global.exe�p�X
#define PROFILE_KEY_MAX_FIND			L"MaxFind"			//!< �ő匟����
#define PROFILE_KEY_DELAY				L"Delay"			//!< �����܂ł̑҂�����(ms)
#define PROFILE_KEY_MATCH_MODE			L"MatchMode"		//!< ��v���[�h
#define PROFILE_KEY_IGNORE_CASE			L"IgnoreCase"		//!< �啶���Ə������𖳎�����
#define PROFILE_KEY_SYMBOL				L"Symbol"			//!< �V���{������������
#define PROFILE_KEY_REF					L"Ref"				//!< �Q�Ƃ���������

#define PROFILE_SECTION_GLOBAL			L"Global"
#define PROFILE_KEY_COUNT				L"Count"			//!< ����
#define PROFILE_KEY_FLAG				L"Flag%d"			//!< �g�p���邩
#define PROFILE_KEY_TARGET_PATH			L"TargetPath%d"		//!< �Ώۃp�X
#define PROFILE_KEY_UNIQ_ID				L"UniqID%d"			//!< ����ID

#define PROFILE_DEF_GTAGS_EXE_PATH		L"gtags.exe"
#define PROFILE_DEF_GLOBAL_EXE_PATH		L"global.exe"
#define PROFILE_DEF_GTAGS_RESULT_PATH	L"Result"			//!< ���ʃt�@�C���i�[�p�X
#define PROFILE_DEF_GTAGS_TMP_FILE		L"Result\\global.tmp"
#define PROFILE_DEF_MAXFIND				1000				//!< �ő匟����
#define PROFILE_DEF_DELAY				1000				//!< �����܂ł̑҂�����
#define PROFILE_DEF_MATCH_MODE			MATCH_MODE_BEGIN	//!< ��v���[�h
#define PROFILE_DEF_IGNORE_CASE			FALSE				//!< �啶���Ə������𖳎�����
#define PROFILE_DEF_SYMBOL				FALSE				//!< �V���{������������
#define PROFILE_DEF_REF					FALSE				//!< �Q�Ƃ���������
#define PROFILE_DEF_FLAG				TRUE

//#define TAG_FORMAT L"%[^\t\r\n]\t%[^\t\r\n]\t%d;\"\t%[^\t\r\n]\t%[^\t\r\n]"
#define TAG_FORMAT L"%[^\t\r\n]\t%[^\t\r\n]\t%d"

///////////////////////////////////////////////////////////////////////////////
class CGlobalOption
{
public:
	WideString		m_strGtagsExePath;		//!< gtags.exe�p�X
	WideString		m_strGlobalExePath;		//!< global.exe�p�X
	DWORD			m_dwMaxFind;			//!< �����ő吔
	DWORD			m_dwDelay;				//!< �����܂ł̑҂�����
	DWORD			m_dwMatchMode;			//!< ��v���[�h
	BOOL			m_bIgnoreCase;			//!< �啶���Ə������𖳎�����
	BOOL			m_bSymbol;				//!< �V���{������������
	BOOL			m_bRef;					//!< �Q�Ƃ���������
	DWORD			m_dwPrevListCount;		//!< INI�o�^��(�O������폜���邽��)

	CGlobalOption(){
		m_strGtagsExePath  = L"";
		m_strGlobalExePath = L"";
		m_dwMaxFind        = PROFILE_DEF_MAXFIND;
		m_dwDelay          = PROFILE_DEF_DELAY;
		m_dwMatchMode      = PROFILE_DEF_MATCH_MODE;
		m_bIgnoreCase      = PROFILE_DEF_IGNORE_CASE;
		m_bSymbol          = PROFILE_DEF_SYMBOL;
		m_bRef             = PROFILE_DEF_REF;
		m_dwPrevListCount  = 0;
	}

	CGlobalOption& operator=(const CGlobalOption& src){
		if(this != &src){
			m_strGtagsExePath  = src.m_strGtagsExePath;
			m_strGlobalExePath = src.m_strGlobalExePath;
			m_dwMaxFind        = src.m_dwMaxFind;
			m_dwDelay          = src.m_dwDelay;
			m_dwMatchMode      = src.m_dwMatchMode;
			m_bIgnoreCase      = src.m_bIgnoreCase;
			m_bSymbol          = src.m_bSymbol;
			m_bRef             = src.m_bRef;
			m_dwPrevListCount  = src.m_dwPrevListCount;
		}
		return *this;
	}
};

class CGlobalInfo
{
public:
	BOOL			m_bFlag;			//!< �g�p���邩�ǂ���
	WideString		m_strTargetPath;	//!< GTAGS������t�H���_
	DWORD			m_dwUniqID;			//!< ���ʃt�@�C��ID

	CGlobalInfo(){
		m_bFlag         = PROFILE_DEF_FLAG;
		m_strTargetPath = L"";
		m_dwUniqID      = 0;
	}

	CGlobalInfo& operator=(const CGlobalInfo& src){
		if(this != &src){
			m_bFlag         = src.m_bFlag;
			m_strTargetPath = src.m_strTargetPath;
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

	CGlobalOption			m_GlobalOption;
	std::list<CGlobalInfo*>	m_GlobalInfoList;

public:
	virtual LPCWSTR GetPluginName(){ return L"GlobalPlugin"; }

public:
	void OnPluginGlobalJumpDialog(SAKURA_DLL_PLUGIN_OBJ* obj);
	void OnPluginGlobalJump(SAKURA_DLL_PLUGIN_OBJ* obj);
	void OnPluginGlobalOptionDialog(SAKURA_DLL_PLUGIN_OBJ* obj);

	void ReadProfile();
	void WriteProfile();
	void RemoveAllGlobalInfoList(std::list<CGlobalInfo*>& p);
	WideString GetResultPath(const DWORD dwUniqID);
	DWORD GetUniqID();
	WideString GetDwordToString(const DWORD dwValue);
	WideString GetDwordToHexString(const DWORD dwValue);
	DWORD GetHexStringToDword(LPCWSTR lpszValue);
	void RemoveResultPath(const DWORD dwUniqID, BOOL bFull = TRUE);
};

///////////////////////////////////////////////////////////////////////////////
extern CPluginService thePluginService;

#endif	//CPLUGINSERVICE_H
