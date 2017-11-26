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
	VPGrepEnumItem vpItems;

public:
	GrepEnumFileBase() {
	}

	virtual ~GrepEnumFileBase(){
		ClearItems();
	}

	void ClearItems(void) {
		vpItems.clear();
		return;
	}

	bool IsExist(LPCTSTR lpFileName) {
		for (size_t i=0; i<GetCount(); ++i) {
			if (vpItems[i].first == lpFileName) {
				return true;
			}
		}
		return false;
	}

	virtual bool IsValid( WIN32_FIND_DATA& w32fd, LPCTSTR pFile = NULL ){
		if (!IsExist(pFile ? pFile : w32fd.cFileName)) {
			return true;
		}
		return false;
	}

	size_t GetCount(void) const {
		return vpItems.size();
	}

	LPCTSTR GetFileName(size_t i) {
		if (i < 0 || i >= GetCount()) {
			return NULL;
		}
		return vpItems[i].first.c_str();
	}

	DWORD GetFileSizeLow(size_t i) {
		if (i < 0 || i >= GetCount()) {
			return 0;
		}
		return vpItems[i].second;
	}

	int Enumerates(
		LPCTSTR					lpBaseFolder,
		const VGrepEnumKeys&	vecKeys,
		const GrepEnumOptions&	option,
		GrepEnumFileBase*		pExceptItems = nullptr
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
			ptrdiff_t nKeyDirLen = keyDir ? keyDir - key + 1 : 0;
			name.resize(nKeyDirLen + 1);
			LPTSTR lpName = &name[0];
			_tcsncpy(lpName, key, nKeyDirLen);

			WIN32_FIND_DATA w32fd;
			HANDLE handle = ::FindFirstFile(lpPath, &w32fd);
//			HANDLE handle = ::FindFirstFileEx(lpPath, FindExInfoBasic, &w32fd, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
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
							vpItems.emplace_back(lpName, w32fd.nFileSizeLow);
							++found;
							if (pExceptItems && nKeyDirLen) {
								// フォルダを含んだパスなら検索済みとして除外指定に追加する
								pExceptItems->vpItems.emplace_back(lpFullPath, w32fd.nFileSizeLow);
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

