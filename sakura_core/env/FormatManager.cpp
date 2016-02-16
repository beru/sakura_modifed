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
#include "DLLSHAREDATA.h"

#include "FormatManager.h"

/*! ���t���t�H�[�}�b�g
	systime�F�����f�[�^
	
	pszDest�F�t�H�[�}�b�g�ς݃e�L�X�g�i�[�p�o�b�t�@
	nDestLen�FpszDest�̒���
	
	pszDateFormat�F
		�J�X�^���̂Ƃ��̃t�H�[�}�b�g
*/
const TCHAR* FormatManager::MyGetDateFormat(const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen)
{
	return MyGetDateFormat(
		systime,
		pszDest,
		nDestLen,
		m_pShareData->m_common.m_format.m_nDateFormatType,
		m_pShareData->m_common.m_format.m_szDateFormat
	);
}

const TCHAR* FormatManager::MyGetDateFormat(
	const SYSTEMTIME&	systime,
	TCHAR*				pszDest,
	int					nDestLen,
	int					nDateFormatType,
	const TCHAR*		szDateFormat
	)
{
	const TCHAR* pszForm;
	DWORD dwFlags;
	if (nDateFormatType == 0) {
		dwFlags = DATE_LONGDATE;
		pszForm = NULL;
	}else {
		dwFlags = 0;
		pszForm = szDateFormat;
	}
	::GetDateFormat( SelectLang::getDefaultLangId(), dwFlags, &systime, pszForm, pszDest, nDestLen );
	return pszDest;
}


// �������t�H�[�}�b�g
const TCHAR* FormatManager::MyGetTimeFormat(const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen)
{
	return MyGetTimeFormat(
		systime,
		pszDest,
		nDestLen,
		m_pShareData->m_common.m_format.m_nTimeFormatType,
		m_pShareData->m_common.m_format.m_szTimeFormat
	);
}

// �������t�H�[�}�b�g
const TCHAR* FormatManager::MyGetTimeFormat(
	const SYSTEMTIME&	systime,
	TCHAR*				pszDest,
	int					nDestLen,
	int					nTimeFormatType,
	const TCHAR*		szTimeFormat
	)
{
	const TCHAR* pszForm;
	DWORD dwFlags;
	if (nTimeFormatType == 0) {
		dwFlags = 0;
		pszForm = NULL;
	}else {
		dwFlags = 0;
		pszForm = szTimeFormat;
	}
	::GetTimeFormat(SelectLang::getDefaultLangId(), dwFlags, &systime, pszForm, pszDest, nDestLen);
	return pszDest;
}

