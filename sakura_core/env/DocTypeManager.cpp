/*
	2008.05.18 kobake CShareData ���番��
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

const TCHAR* DocTypeManager::typeExtSeps = _T(" ;,");	// �^�C�v�ʊg���q ��؂蕶��
const TCHAR* DocTypeManager::typeExtWildcards = _T("*?");	// �^�C�v�ʊg���q ���C���h�J�[�h

static Mutex g_docTypeMutex(FALSE, GSTR_MUTEX_SAKURA_DOCTYPE);

/*!
	�t�@�C��������A�h�L�������g�^�C�v�i���l�j���擾����
	
	@param pszFilePath [in] �t�@�C����
	
	�g���q��؂�o���� GetDocumentTypeOfExt �ɓn�������D
	@date 2014.12.06 syat ���C���h�J�[�h�Ή��B�Q�d�g���q�Ή�����߂�
*/
TypeConfigNum DocTypeManager::GetDocumentTypeOfPath(const TCHAR* pszFilePath)
{
	// �t�@�C�����𒊏o
	const TCHAR* pszFileName = pszFilePath;
	const TCHAR* pszSep = _tcsrchr(pszFilePath, _T('\\'));
	if (pszSep) {
		pszFileName = pszSep + 1;
	}

	for (int i=0; i<pShareData->nTypesCount; ++i) {
		const TypeConfigMini* mini;
		GetTypeConfigMini(TypeConfigNum(i), &mini);
		if (IsFileNameMatch(mini->szTypeExts, pszFileName)) {
			return TypeConfigNum(i);	//	�ԍ�
		}
	}
	return TypeConfigNum(0);
}


/*!
	�g���q����A�h�L�������g�^�C�v�i���l�j���擾����
	
	@param pszExt [in] �g���q (�擪��.�͊܂܂Ȃ�)
	
	�w�肳�ꂽ�g���q�̑����镶���^�C�v�ԍ���Ԃ��D
	�Ƃ肠�������̂Ƃ���̓^�C�v�͊g���q�݂̂Ɉˑ�����Ɖ��肵�Ă���D
	�t�@�C���S�̂̌`���ɑΉ�������Ƃ��́C�܂��l�������D
	@date 2012.10.22 Moca �Q�d�g���q, �g���q�Ȃ��ɑΉ�
	@date 2014.12.06 syat GetDocumentTypeOfPath�ɓ���
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
	return TypeConfigNum(-1);	// �n�Y��
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
	�^�C�v�ʊg���q�Ƀt�@�C�������}�b�`���邩
	
	@param pszTypeExts [in] �^�C�v�ʊg���q�i���C���h�J�[�h���܂ށj
	@param pszFileName [in] �t�@�C����
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
	�^�C�v�ʊg���q�̐擪�g���q���擾����
	
	@param pszTypeExts [in] �^�C�v�ʊg���q�i���C���h�J�[�h���܂ށj
	@param szFirstExt  [out] �擪�g���q
	@param nBuffSize   [in] �擪�g���q�̃o�b�t�@�T�C�Y
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

/*! �^�C�v�ʐݒ�̊g���q���X�g���_�C�A���O�p���X�g�ɕϊ�����
	@param pszSrcExt [in]  �g���q���X�g ��u.c .cpp;.h�v
	@param pszDstExt [out] �g���q���X�g ��u*.c;*.cpp;*.h�v
	@param szExt [in] ���X�g�̐擪�ɂ���g���q ��u.h�v

	@date 2014.12.06 syat CFileExt����ړ�
*/
bool DocTypeManager::ConvertTypesExtToDlgExt( const TCHAR *pszSrcExt, const TCHAR* szExt, TCHAR *pszDstExt )
{
	TCHAR* token;
	TCHAR* p;

	//	2003.08.14 MIK NULL����Ȃ���false
	if (!pszSrcExt) return false;
	if (!pszDstExt) return false;

	p = _tcsdup( pszSrcExt );
	_tcscpy( pszDstExt, _T("") );

	if (szExt && szExt[0] != _T('\0')) {
		// �t�@�C���p�X������A�g���q����̏ꍇ�A�g�b�v�Ɏw��
		_tcscpy(pszDstExt, _T("*"));
		_tcscat(pszDstExt, szExt);
	}

	token = _tcstok(p, typeExtSeps);
	while (token) {
		if (!szExt || szExt[0] == _T('\0') || auto_stricmp(token, szExt + 1) != 0) {
			if (pszDstExt[0] != '\0') _tcscat( pszDstExt, _T(";") );
			// �g���q�w��Ȃ��A�܂��̓}�b�`�����g���q�łȂ�
			if (_tcspbrk(token, typeExtWildcards) == NULL) {
				if (_T('.') == *token) _tcscat(pszDstExt, _T("*"));
				else                 _tcscat(pszDstExt, _T("*."));
			}
			_tcscat(pszDstExt, token);
		}

		token = _tcstok( NULL, typeExtSeps );
	}
	free( p );	// 2003.05.20 MIK ����������R��
	return true;
}
