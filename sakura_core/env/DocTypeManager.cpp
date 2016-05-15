/*
	2008.05.18 kobake CShareData から分離
*/
/*
	Copyright (C) 2008, kobake

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
#include "DocTypeManager.h"
#include "_main/Mutex.h"
#include "FileExt.h"
#include <Shlwapi.h>	// PathMatchSpec

const TCHAR* DocTypeManager::typeExtSeps = _T(" ;,");	// タイプ別拡張子 区切り文字
const TCHAR* DocTypeManager::typeExtWildcards = _T("*?");	// タイプ別拡張子 ワイルドカード

static Mutex g_docTypeMutex(FALSE, GSTR_MUTEX_SAKURA_DOCTYPE);

/*!
	ファイル名から、ドキュメントタイプ（数値）を取得する
	
	@param pszFilePath [in] ファイル名
	
	拡張子を切り出して GetDocumentTypeOfExt に渡すだけ．
	@date 2014.12.06 syat ワイルドカード対応。２重拡張子対応をやめる
*/
TypeConfigNum DocTypeManager::GetDocumentTypeOfPath(const TCHAR* pszFilePath)
{
	// ファイル名を抽出
	const TCHAR* pszFileName = pszFilePath;
	const TCHAR* pszSep = _tcsrchr(pszFilePath, _T('\\'));
	if (pszSep) {
		pszFileName = pszSep + 1;
	}

	for (int i=0; i<pShareData->nTypesCount; ++i) {
		const TypeConfigMini* mini;
		GetTypeConfigMini(TypeConfigNum(i), &mini);
		if (IsFileNameMatch(mini->szTypeExts, pszFileName)) {
			return TypeConfigNum(i);	//	番号
		}
	}
	return TypeConfigNum(0);
}


/*!
	拡張子から、ドキュメントタイプ（数値）を取得する
	
	@param pszExt [in] 拡張子 (先頭の.は含まない)
	
	指定された拡張子の属する文書タイプ番号を返す．
	とりあえず今のところはタイプは拡張子のみに依存すると仮定している．
	ファイル全体の形式に対応させるときは，また考え直す．
	@date 2012.10.22 Moca ２重拡張子, 拡張子なしに対応
	@date 2014.12.06 syat GetDocumentTypeOfPathに統合
*/
TypeConfigNum DocTypeManager::GetDocumentTypeOfExt(const TCHAR* pszExt)
{
	return GetDocumentTypeOfPath(pszExt);
}

TypeConfigNum DocTypeManager::GetDocumentTypeOfId(int id)
{
	for (int i=0; i<pShareData->nTypesCount; ++i) {
		const TypeConfigMini* mini;
		GetTypeConfigMini(TypeConfigNum(i), &mini);
		if (mini->id == id) {
			return TypeConfigNum(i);
		}
	}
	return TypeConfigNum(-1);	// ハズレ
}

bool DocTypeManager::GetTypeConfig(TypeConfigNum documentType, TypeConfig& type)
{
	int n = documentType.GetIndex();
	if (0 <= n && n < pShareData->nTypesCount) {
		if (n == 0) {
			type = pShareData->typeBasis;
			return true;
		}else {
			LockGuard<Mutex> guard(g_docTypeMutex);
			 if (SendMessage(pShareData->handles.hwndTray, MYWM_GET_TYPESETTING, (WPARAM)n, 0)) {
				type = pShareData->workBuffer.typeConfig;
				return true;
			}
		}
	}
	return false;
}

bool DocTypeManager::SetTypeConfig(TypeConfigNum documentType, const TypeConfig& type)
{
	int n = documentType.GetIndex();
	if (0 <= n && n < pShareData->nTypesCount) {
		LockGuard<Mutex> guard(g_docTypeMutex);
		pShareData->workBuffer.typeConfig = type;
		if (SendMessage(pShareData->handles.hwndTray, MYWM_SET_TYPESETTING, (WPARAM)n, 0)) {
			return true;
		}
	}
	return false;
}

bool DocTypeManager::GetTypeConfigMini(TypeConfigNum documentType, const TypeConfigMini** type)
{
	int n = documentType.GetIndex();
	if (0 <= n && n < pShareData->nTypesCount) {
		*type = &pShareData->typesMini[n];
		return true;
	}
	return false;
}

bool DocTypeManager::AddTypeConfig(TypeConfigNum documentType)
{
	LockGuard<Mutex> guard(g_docTypeMutex);
	return SendMessage(pShareData->handles.hwndTray, MYWM_ADD_TYPESETTING, (WPARAM)documentType.GetIndex(), 0) != FALSE;
}

bool DocTypeManager::DelTypeConfig(TypeConfigNum documentType)
{
	LockGuard<Mutex> guard(g_docTypeMutex);
	return SendMessage(pShareData->handles.hwndTray, MYWM_DEL_TYPESETTING, (WPARAM)documentType.GetIndex(), 0) != FALSE;
}

/*!
	タイプ別拡張子にファイル名がマッチするか
	
	@param pszTypeExts [in] タイプ別拡張子（ワイルドカードを含む）
	@param pszFileName [in] ファイル名
*/
bool DocTypeManager::IsFileNameMatch(const TCHAR* pszTypeExts, const TCHAR* pszFileName)
{
	TCHAR szWork[MAX_TYPES_EXTS];

	_tcsncpy(szWork, pszTypeExts, _countof(szWork));
	szWork[_countof(szWork) - 1] = '\0';
	TCHAR* token = _tcstok(szWork, typeExtSeps);
	while (token) {
		if (_tcspbrk(token, typeExtWildcards) == NULL) {
			if (_tcsicmp(token, pszFileName) == 0) {
				return true;
			}
			const TCHAR* pszExt = _tcsrchr(pszFileName, _T('.'));
			if (pszExt && _tcsicmp(token, pszExt + 1) == 0) {
				return true;
			}
		}else {
			if (PathMatchSpec(pszFileName, token) == TRUE) {
				return true;
			}
		}
		token = _tcstok(NULL, typeExtSeps);
	}
	return false;
}

/*!
	タイプ別拡張子の先頭拡張子を取得する
	
	@param pszTypeExts [in] タイプ別拡張子（ワイルドカードを含む）
	@param szFirstExt  [out] 先頭拡張子
	@param nBuffSize   [in] 先頭拡張子のバッファサイズ
*/
void DocTypeManager::GetFirstExt(const TCHAR* pszTypeExts, TCHAR szFirstExt[], int nBuffSize)
{
	TCHAR szWork[MAX_TYPES_EXTS];

	_tcsncpy(szWork, pszTypeExts, _countof(szWork));
	szWork[_countof(szWork) - 1] = '\0';
	TCHAR* token = _tcstok(szWork, typeExtSeps);
	while (token) {
		if (_tcspbrk(token, typeExtWildcards) == NULL) {
			_tcsncpy(szFirstExt, token, nBuffSize);
			szFirstExt[nBuffSize - 1] = _T('\0');
			return;
		}
	}
	szFirstExt[0] = _T('\0');
	return;
}

/*! タイプ別設定の拡張子リストをダイアログ用リストに変換する
	@param pszSrcExt [in]  拡張子リスト 例「.c .cpp;.h」
	@param pszDstExt [out] 拡張子リスト 例「*.c;*.cpp;*.h」
	@param szExt [in] リストの先頭にする拡張子 例「.h」

	@date 2014.12.06 syat CFileExtから移動
*/
bool DocTypeManager::ConvertTypesExtToDlgExt( const TCHAR *pszSrcExt, const TCHAR* szExt, TCHAR *pszDstExt )
{
	TCHAR* token;
	TCHAR* p;

	//	2003.08.14 MIK NULLじゃなくてfalse
	if (!pszSrcExt) return false;
	if (!pszDstExt) return false;

	p = _tcsdup( pszSrcExt );
	_tcscpy( pszDstExt, _T("") );

	if (szExt && szExt[0] != _T('\0')) {
		// ファイルパスがあり、拡張子ありの場合、トップに指定
		_tcscpy(pszDstExt, _T("*"));
		_tcscat(pszDstExt, szExt);
	}

	token = _tcstok(p, typeExtSeps);
	while (token) {
		if (!szExt || szExt[0] == _T('\0') || auto_stricmp(token, szExt + 1) != 0) {
			if (pszDstExt[0] != '\0') _tcscat( pszDstExt, _T(";") );
			// 拡張子指定なし、またはマッチした拡張子でない
			if (_tcspbrk(token, typeExtWildcards) == NULL) {
				if (_T('.') == *token) _tcscat(pszDstExt, _T("*"));
				else                 _tcscat(pszDstExt, _T("*."));
			}
			_tcscat(pszDstExt, token);
		}

		token = _tcstok( NULL, typeExtSeps );
	}
	free( p );	// 2003.05.20 MIK メモリ解放漏れ
	return true;
}
