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
		LPCTSTR				lpBaseFolder,
		VGrepEnumKeys&		vecKeys,
		GrepEnumOptions&	option,
		GrepEnumFileBase*	pExceptItems = NULL
	) {
		int found = 0;

		std::vector<TCHAR> path;
		std::vector<TCHAR> name;
		std::vector<TCHAR> fullPath;
		for (int i=0; i<(int)vecKeys.size(); ++i) {
			int baseLen = _tcslen(lpBaseFolder);
			path.resize( baseLen + _tcslen(vecKeys[ i ]) + 2 );
			LPTSTR lpPath = &path[0];
			auto_strcpy(lpPath, lpBaseFolder);
			auto_strcpy(lpPath + baseLen, _T("\\"));
			auto_strcpy(lpPath + baseLen + 1, vecKeys[ i ]);
			// vecKeys[ i ] ==> "subdir\*.h" ���̏ꍇ�Ɍ��(�t�@�C��|�t�H���_)���� "subdir\" ��A������
			const TCHAR* keyDirYen = _tcsrchr(vecKeys[ i ], _T('\\'));
			const TCHAR* keyDirSlash = _tcsrchr(vecKeys[ i ], _T('/'));
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
			int nKeyDirLen = keyDir ? keyDir - vecKeys[ i ] + 1 : 0;

			WIN32_FIND_DATA w32fd;
			HANDLE handle = ::FindFirstFile(lpPath, &w32fd);
			if (handle != INVALID_HANDLE_VALUE) {
				do {
					if (option.bIgnoreHidden && (w32fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {
						continue;
					}
					if (option.bIgnoreReadOnly && (w32fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)) {
						continue;
					}
					if (option.bIgnoreSystem && (w32fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
						continue;
					}
					name.resize( nKeyDirLen + _tcslen(w32fd.cFileName) + 1 );
					LPTSTR lpName = &name[0];
					_tcsncpy(lpName, vecKeys[ i ], nKeyDirLen);
					_tcscpy(lpName + nKeyDirLen, w32fd.cFileName);
					fullPath.resize( baseLen + _tcslen(lpName) + 2 );
					LPTSTR lpFullPath = &fullPath[0];
					auto_strcpy(lpFullPath, lpBaseFolder);
					auto_strcpy(lpFullPath + baseLen, _T("\\"));
					auto_strcpy(lpFullPath + baseLen + 1, lpName);
					if (IsValid(w32fd, lpName)) {
						if (pExceptItems && pExceptItems->IsExist(lpFullPath)) {
						}else {
							m_vpItems.emplace_back(lpName, w32fd.nFileSizeLow);
							++found; // 2011.11.19
							if (pExceptItems && nKeyDirLen) {
								// �t�H���_���܂񂾃p�X�Ȃ猟���ς݂Ƃ��ď��O�w��ɒǉ�����
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

