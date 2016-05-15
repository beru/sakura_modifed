/*!	@file
	@brief オープンダイアログ用ファイル拡張子管理

	@author MIK
	@date 2003.5.12
*/
/*
	Copyright (C) 2003, MIK

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

#include "StdAfx.h"
#include <string.h>
#include <stdlib.h>
#include "FileExt.h"
#include "env/DocTypeManager.h"

FileExt::FileExt()
{
	puFileExtInfo = nullptr;
	nCount = 0;
	vstrFilter.resize( 1 );
	vstrFilter[0] = _T('\0');

//	// テキストエディタとして、既定でリストに載ってほしい拡張子
//	AppendExt("すべてのファイル", "*");
//	AppendExt("テキストファイル", "txt");
}

FileExt::~FileExt()
{
	if (puFileExtInfo) {
		free(puFileExtInfo);
	}
	puFileExtInfo = nullptr;
	nCount = 0;
}

bool FileExt::AppendExt(const TCHAR* pszName, const TCHAR* pszExt)
{
	TCHAR szWork[_countof(puFileExtInfo[0].szExt) + 10];

	if (!DocTypeManager::ConvertTypesExtToDlgExt(pszExt, NULL, szWork)) {
		return false;
	}
	return AppendExtRaw(pszName, szWork);
}

bool FileExt::AppendExtRaw(const TCHAR* pszName, const TCHAR* pszExt)
{
	if (!pszName || pszName[0] == _T('\0')) {
		return false;
	}
	if (!pszExt  || pszExt[0] == _T('\0')) {
		return false;
	}

	FileExtInfoTag* p;
	if (!puFileExtInfo) {
		p = (FileExtInfoTag*)malloc(sizeof(FileExtInfoTag) * 1);
		if (!p) {
			return false;
		}
	}else {
		p = (FileExtInfoTag*)realloc(puFileExtInfo, sizeof(FileExtInfoTag) * (nCount + 1));
		if (!p) {
			return false;
		}
	}
	puFileExtInfo = p;

	_tcscpy(puFileExtInfo[nCount].szName, pszName);
	_tcscpy(puFileExtInfo[nCount].szExt, pszExt);
	++nCount;

	return true;
}

const TCHAR* FileExt::GetName(int nIndex)
{
	if (nIndex < 0 || nIndex >= nCount) {
		return NULL;
	}
	return puFileExtInfo[nIndex].szName;
}

const TCHAR* FileExt::GetExt(int nIndex)
{
	if (nIndex < 0 || nIndex >= nCount) {
		return NULL;
	}
	return puFileExtInfo[nIndex].szExt;
}

const TCHAR* FileExt::GetExtFilter(void)
{
	std::tstring work;

	// 拡張子フィルタの作成
	vstrFilter.resize(0);

	for (int i=0; i<nCount; ++i) {
		// "%ts (%ts)\0%ts\0"
		work = puFileExtInfo[i].szName;
		work.append(_T(" ("));
		work.append(puFileExtInfo[i].szExt);
		work.append(_T(")"));
		work.append(_T("\0"), 1);
		work.append(puFileExtInfo[i].szExt);
		work.append(_T("\0"), 1);

		int sz = (int)vstrFilter.size();
		vstrFilter.resize(sz + work.length());
		auto_memcpy(&vstrFilter[sz], &work[0], work.length());
	}
	if (nCount == 0) {
		vstrFilter.push_back(_T('\0'));
	}
	vstrFilter.push_back(_T('\0'));

	return &vstrFilter[0];

}

