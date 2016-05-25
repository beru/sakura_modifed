/*!	@file
	@brief DicMgr�N���X

	@author Norio Nakatani
	@date	1998/11/05 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka, Moca
	Copyright (C) 2003, Moca
	Copyright (C) 2006, fon
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <stdio.h>
#include "DicMgr.h"
#include "mem/Memory.h" // 2002/2/10 aroka �w�b�_����
#include "debug/RunningTimer.h"
#include "io/TextStream.h"
using namespace std;

DicMgr::DicMgr()
{
	return;
}


DicMgr::~DicMgr()
{
	return;
}


/*!
	�L�[���[�h�̌���
	�ŏ��Ɍ��������L�[���[�h�̈Ӗ���Ԃ�

	@date 2006.04.10 fon �����q�b�g�s��Ԃ�����pLine��ǉ�
*/
BOOL DicMgr::Search(
	const wchar_t*	pszKey,				// �����L�[���[�h
	const size_t	nCmpLen,			// �����L�[���[�h�̒���
	NativeW**		ppMemKey,			// ���������L�[���[�h�D�Ăяo�����̐ӔC�ŉ������D
	NativeW**		ppMemMean,			// ���������L�[���[�h�ɑΉ����鎫�����e�D�Ăяo�����̐ӔC�ŉ������D
	const TCHAR*	pszKeywordHelpFile,	// �L�[���[�h�w���v�t�@�C���̃p�X��
	int*			pLine				// ���������L�[���[�h�̃L�[���[�h�w���v�t�@�C�����ł̍s�ԍ�
	)
{
#ifdef _DEBUG
	RunningTimer runningTimer("DicMgr::Search");
#endif
	const wchar_t*	pszDelimit = L" /// ";
	const wchar_t*	pszKeySeps = L",\0";

	// �����t�@�C��
	if (pszKeywordHelpFile[0] == _T('\0')) {
		return FALSE;
	}
	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	TextInputStream_AbsIni in(pszKeywordHelpFile);
	if (!in) {
		return FALSE;
	}

	wchar_t	szLine[LINEREADBUFSIZE];
	for (int line=1; in; ++line) {	// 2006.04.10 fon
		// 1�s�ǂݍ���
		{
			wstring tmp = in.ReadLineW(); //fgetws(szLine, _countof(szLine), pFile) != NULL;
			wcsncpy_s(szLine, _countof(szLine), tmp.c_str(), _TRUNCATE);
			// auto_strlcpy(szLine, tmp.c_str(), _countof(szLine));
		}

		wchar_t* pszWork = wcsstr(szLine, pszDelimit);
		if (pszWork && szLine[0] != L';') {
			*pszWork = L'\0';
			pszWork += wcslen(pszDelimit);

			// �ŏ��̃g�[�N�����擾���܂��B
			wchar_t* pszToken = wcstok(szLine, pszKeySeps);
			while (pszToken) {
				int nRes = _wcsnicmp(pszKey, pszToken, nCmpLen);	// 2006.04.10 fon
				if (nRes == 0) {
					int nLen = (int)wcslen(pszWork);
					for (int i=0; i<nLen; ++i) {
						if (WCODE::IsLineDelimiterBasic(pszWork[i])) {
							pszWork[i] = L'\0';
							break;
						}
					}
					// �L�[���[�h�̃Z�b�g
					*ppMemKey = new NativeW;	// 2006.04.10 fon
					(*ppMemKey)->SetString(pszToken);
					// �Ӗ��̃Z�b�g
					*ppMemMean = new NativeW;
					(*ppMemMean)->SetString(pszWork);

					*pLine = line;	// 2006.04.10 fon
					return TRUE;
				}
				pszToken = wcstok(NULL, pszKeySeps);
			}
		}
	}
	return FALSE;
}


/*
||  ���͕⊮�L�[���[�h�̌���
||
||  �E�w�肳�ꂽ���̍ő吔�𒴂���Ə����𒆒f����
||  �E������������Ԃ�
||
*/
int DicMgr::HokanSearch(
	const wchar_t*	pszKey,
	bool			bHokanLoHiCase,	// �p�啶���������𓯈ꎋ����
	vector_ex<std::wstring>&		vKouho,	// [out] ��⃊�X�g
	int				nMaxKouho,		// Max��␔(0==������)
	const TCHAR*	pszKeywordFile
	)
{
	if (pszKeywordFile[0] == _T('\0')) {
		return 0;
	}

	TextInputStream_AbsIni in(pszKeywordFile);
	if (!in) {
		return 0;
	}
	size_t nKeyLen = wcslen(pszKey);
	while (in) {
		wstring szLine = in.ReadLineW();
		if (nKeyLen > (int)szLine.length()) {
			continue;
		}

		// �R�����g����
		if (szLine[0] == L';') {
			continue;
		}

		// ��s����
		if (szLine.length() == 0) {
			continue;
		}

		int nRet;
		if (bHokanLoHiCase) {	// �p�啶���������𓯈ꎋ����
			nRet = auto_memicmp(pszKey, szLine.c_str(), nKeyLen);
		}else {
			nRet = auto_memcmp(pszKey, szLine.c_str(), nKeyLen);
		}
		if (nRet == 0) {
			vKouho.push_back(szLine);
			if (nMaxKouho != 0 && nMaxKouho <= (int)vKouho.size()) {
				break;
			}
		}
	}
	in.Close();
	return (int)vKouho.size();
}

