#pragma once

// GREP support library

#include <vector>
#include <windows.h>
#include <string.h>
#include <tchar.h>
#include "util/string_ex.h"
#include "util/fileUtil.h"

typedef std::vector<std::tstring> VGrepEnumKeys;

class GrepEnumKeys {
public:
	VGrepEnumKeys vecSearchFileKeys;
	VGrepEnumKeys vecSearchFolderKeys;
	VGrepEnumKeys vecExceptFileKeys;
	VGrepEnumKeys vecExceptFolderKeys;

//	VGrepEnumKeys vecSearchAbsFileKeys;
	VGrepEnumKeys vecExceptAbsFileKeys;
	VGrepEnumKeys vecExceptAbsFolderKeys;

public:
	GrepEnumKeys() {
	}

	~GrepEnumKeys() {
		ClearItems();
	}

	int SetFileKeys(LPCTSTR lpKeys);

private:

	void ClearItems(void) {
		vecExceptFileKeys.clear();
		vecSearchFileKeys.clear();
		vecExceptFolderKeys.clear();
		vecSearchFolderKeys.clear();
		return;
	}

	void push_back_unique(VGrepEnumKeys& keys, LPCTSTR addKey) {
		if (!IsExist(keys, addKey)) {
			keys.push_back(addKey);
		}
	}

	BOOL IsExist(VGrepEnumKeys& keys, LPCTSTR addKey) {
		for (size_t i=0; i<keys.size(); ++i) {
			if (keys[i] == addKey) {
				return TRUE;
			}
		}
		return FALSE;
	}

	/*
		@retval 0 正常終了
		@retval 1 *\file.exe などのフォルダ部分でのワイルドカードはエラー
	*/
	int ValidateKey(LPCTSTR key) {
		// 
		bool wildcard = false;
		for (int i=0; key[i]; ++i) {
			TCHAR c = key[i];
			if (!wildcard && (c == _T('*') || c == _T('?'))) {
				wildcard = true;
			}else if (wildcard && (c == _T('\\') || c == _T('/'))) {
				return 1;
			}
		}
		return 0;
	}
};

