/*!	@file
	
	@brief GREP support library
	
	@author wakura
	@date 2008/04/28
*/
/*
	Copyright (C) 2008, wakura

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

#include <vector>
#include <windows.h>
#include <string.h>
#include <tchar.h>
#include "GrepEnumKeys.h"
#include "util/string_ex.h"

typedef std::pair<std::tstring, DWORD> PairGrepEnumItem;
typedef std::vector<PairGrepEnumItem> VPGrepEnumItem;

class GrepEnumOptions {
public:
	GrepEnumOptions()
		:bIgnoreHidden(false)
		,bIgnoreReadOnly(false)
		,bIgnoreSystem(false)
	{}
	bool	bIgnoreHidden;
	bool	bIgnoreReadOnly;
	bool	bIgnoreSystem;
};

class GrepEnumFileBase {
private:
	VPGrepEnumItem m_vpItems;

public:
	GrepEnumFileBase() {
	}

	virtual ~GrepEnumFileBase(){
		ClearItems();
	}

	void ClearItems(void) {
		m_vpItems.clear();
		return;
	}

	BOOL IsExist(LPCTSTR lpFileName) {
		for (int i=0; i<GetCount(); ++i) {
			if (m_vpItems[i].first == lpFileName) {
				return TRUE;
			}
		}
		return FALSE;
	}

	virtual BOOL IsValid( WIN32_FIND_DATA& w32fd, LPCTSTR pFile = NULL ){
		if (!IsExist(pFile ? pFile : w32fd.cFileName)) {
			return TRUE;
		}
		return FALSE;
	}

	int GetCount(void) {
		return (int)m_vpItems.size();
	}

	LPCTSTR GetFileName(int i) {
		if (i < 0 || i >= GetCount()) {
			return NULL;
		}
		return m_vpItems[ i ].first.c_str();
	}

	DWORD GetFileSizeLow(int i) {
		if (i < 0 || i >= GetCount()) {
			return 0;
		}
		return m_vpItems[ i ].second;
	}

	int Enumerates(
		LPCTSTR					lpBaseFolder,
		const VGrepEnumKeys&	vecKeys,
		const GrepEnumOptions&	option,
		GrepEnumFileBase*	pExceptItems = NULL
	) {
		int found = 0;

		DWORD ignoreFileAttributes = 0;
		if (option.bIgnoreHidden) {
			ignoreFileAttributes |= FILE_ATTRIBUTE_HIDDEN;
		}
		if (option.bIgnoreReadOnly) {
			ignoreFileAttributes |= FILE_ATTRIBUTE_READONLY;
		}
		if (option.bIgnoreSystem) {
			ignoreFileAttributes |= FILE_ATTRIBUTE_SYSTEM;
		}

		size_t baseLen = _tcslen(lpBaseFolder);
		std::vector<TCHAR> path(baseLen + 2);
		LPTSTR lpPath = &path[0];
		auto_strcpy(lpPath, lpBaseFolder);
		auto_strcpy(lpPath + baseLen, _T("\\"));
		std::vector<TCHAR> fullPath(baseLen + 2);
		LPTSTR lpFullPath = &fullPath[0];
		auto_strcpy(lpFullPath, lpBaseFolder);
		auto_strcpy(lpFullPath + baseLen, _T("\\"));
		std::vector<TCHAR> name;
		for (size_t i=0; i<vecKeys.size(); ++i) {
			LPCTSTR key = vecKeys[i].c_str();
			path.resize(baseLen + _tcslen(key) + 2);
			lpPath = &path[0];
			auto_strcpy(lpPath + baseLen + 1, key);
			// vecKeys[ i ] ==> "subdir\*.h" 等の場合に後で(ファイル|フォルダ)名に "subdir\" を連結する
			const TCHAR* keyDirYen = _tcsrchr(key, _T('\\'));
			const TCHAR* keyDirSlash = _tcsrchr(key, _T('/'));
			const TCHAR* keyDir;
			if (!keyDirYen) {
				keyDir = keyDirSlash;
			}else if (!keyDirSlash) {
				keyDir = keyDirYen;
			}else if (keyDirYen < keyDirSlash) {
				keyDir = keyDirSlash;
			}else {
				keyDir = keyDirYen;
			}
			int nKeyDirLen = keyDir ? keyDir - key + 1 : 0;
			name.resize(nKeyDirLen + 1);
			LPTSTR lpName = &name[0];
			_tcsncpy(lpName, key, nKeyDirLen);

			WIN32_FIND_DATA w32fd;
			HANDLE handle = ::FindFirstFile(lpPath, &w32fd);
			if (handle != INVALID_HANDLE_VALUE) {
				do {
					if (w32fd.dwFileAttributes & ignoreFileAttributes) {
						continue;
					}
					size_t fileNameLen = _tcslen(w32fd.cFileName);
					size_t nameLen = nKeyDirLen + fileNameLen;
					name.resize(nameLen + 1);
					lpName = &name[0];
					_tcscpy(lpName + nKeyDirLen, w32fd.cFileName);
					fullPath.resize(baseLen + nameLen + 2);
					lpFullPath = &fullPath[0];
					auto_strcpy(lpFullPath + baseLen + 1, lpName);
					if (IsValid(w32fd, lpName)) {
						if (pExceptItems && pExceptItems->IsExist(lpFullPath)) {
						}else {
							m_vpItems.emplace_back(lpName, w32fd.nFileSizeLow);
							++found; // 2011.11.19
							if (pExceptItems && nKeyDirLen) {
								// フォルダを含んだパスなら検索済みとして除外指定に追加する
								pExceptItems->m_vpItems.emplace_back(lpFullPath, w32fd.nFileSizeLow);
							}
							continue;
						}
					}
				}while (::FindNextFile(handle, &w32fd));
				::FindClose(handle);
			}
		}
		return found;
	}
};

