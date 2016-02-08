/*!	@file
	
	@brief GREP support library
	
	@author wakura, Moca
	@date 2008/04/28
*/
/*
	Copyright (C) 2008, wakura
	Copyright (C) 2011, Moca

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
#include "util/string_ex.h"
#include "util/file.h"

typedef std::vector< LPCTSTR > VGrepEnumKeys;

class GrepEnumKeys {
public:
	VGrepEnumKeys m_vecSearchFileKeys;
	VGrepEnumKeys m_vecSearchFolderKeys;
	VGrepEnumKeys m_vecExceptFileKeys;
	VGrepEnumKeys m_vecExceptFolderKeys;

//	VGrepEnumKeys m_vecSearchAbsFileKeys;
	VGrepEnumKeys m_vecExceptAbsFileKeys;
	VGrepEnumKeys m_vecExceptAbsFolderKeys;

public:
	GrepEnumKeys() {
	}

	~GrepEnumKeys() {
		ClearItems();
	}

	int SetFileKeys(LPCTSTR lpKeys);

private:

	void ClearItems(void) {
		ClearEnumKeys(m_vecExceptFileKeys);
		ClearEnumKeys(m_vecSearchFileKeys);
		ClearEnumKeys(m_vecExceptFolderKeys);
		ClearEnumKeys(m_vecSearchFolderKeys);
		return;
	}
	void ClearEnumKeys(VGrepEnumKeys& keys) {
		for (int i=0; i<(int)keys.size(); ++i) {
			delete[] keys[ i ];
		}
		keys.clear();
	}

	void push_back_unique(VGrepEnumKeys& keys, LPCTSTR addKey) {
		if (!IsExist(keys, addKey)) {
			TCHAR* newKey = new TCHAR[ _tcslen(addKey) + 1 ];
			_tcscpy(newKey, addKey);
			keys.push_back(newKey);
		}
	}

	BOOL IsExist(VGrepEnumKeys& keys, LPCTSTR addKey) {
		for (int i=0; i<(int)keys.size(); ++i) {
			if (_tcscmp(keys[i], addKey) == 0) {
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
			if (!wildcard && (key[i] == _T('*') || key[i] == _T('?'))) {
				wildcard = true;
			}else if (wildcard && (key[i] == _T('\\') || key[i] == _T('/'))) {
				return 1;
			}
		}
		return 0;
	}
};

