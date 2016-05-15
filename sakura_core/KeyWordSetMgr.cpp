/*!	@file
	@brief �����L�[���[�h�Ǘ�

	@author Norio Nakatani
	
	@date 2000.12.01 MIK binary search
	@date 2004.07.29-2005.01.27 Moca �L�[���[�h�̉ϒ��L��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, MIK
	Copyright (C) 2002, genta, Moca
	Copyright (C) 2004, Moca
	Copyright (C) 2005, Moca, genta

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
#include "KeywordSetMgr.h"
#include <limits>

// 1�u���b�N������̃L�[���[�h��
static const int nKeywordSetBlockSize = 50;

// �u���b�N�T�C�Y�Ő���
inline int GetAlignmentSize(int nSize)
{
	return (nKeywordSetBlockSize - 1 + nSize) / nKeywordSetBlockSize * nKeywordSetBlockSize;
}

/*!
	@note KeywordSetMgr�͋��L�������\���̂ɖ��ߍ��܂�Ă��邽�߁C
	���̂܂܂ł̓R���X�g���N�^�������Ȃ����Ƃɒ��ӁD
*/
KeywordSetMgr::KeywordSetMgr(void)
{
	nCurrentKeywordSetIdx = 0;
	nKeywordSetNum = 0;
	nStartIdx[0] = 0;
	nStartIdx[1] = 0;
	nStartIdx[MAX_SETNUM] = 0;
	return;
}

KeywordSetMgr::~KeywordSetMgr(void)
{
	nKeywordSetNum = 0;
	return;
}

/*!
	@brief �S�L�[���[�h�Z�b�g�̍폜�Ə�����

	�L�[���[�h�Z�b�g�̃C���f�b�N�X��S��0�Ƃ���D
	
	@date 2004.07.29 Moca �ϒ��L��
*/
void KeywordSetMgr::ResetAllKeywordSet(void)
{
	nKeywordSetNum = 0;
	for (int i=0; i<MAX_SETNUM+1; ++i) {
		nStartIdx[i] = 0;
	}
	for (int i=0; i<MAX_SETNUM; ++i) {
		nKeywordNumArr[i] = 0;
	}
}

const KeywordSetMgr& KeywordSetMgr::operator = (KeywordSetMgr& KeywordSetMgr)
{
//	int		nDataLen;
//	char*	pData;
//	int		i;
	if (this == &KeywordSetMgr) {
		return *this;
	}
	nCurrentKeywordSetIdx = KeywordSetMgr.nCurrentKeywordSetIdx;
	nKeywordSetNum = KeywordSetMgr.nKeywordSetNum;
	// �z��܂邲�ƃR�s�[
	memcpy_raw(szSetNameArr   , KeywordSetMgr.szSetNameArr   , sizeof(szSetNameArr)		);
	memcpy_raw(bKeywordCaseArr, KeywordSetMgr.bKeywordCaseArr, sizeof(bKeywordCaseArr)	);
	memcpy_raw(nStartIdx      , KeywordSetMgr.nStartIdx      , sizeof(nStartIdx	)		); // 2004.07.29 Moca
	memcpy_raw(nKeywordNumArr , KeywordSetMgr.nKeywordNumArr , sizeof(nKeywordNumArr)		);
	memcpy_raw(szKeywordArr   , KeywordSetMgr.szKeywordArr   , sizeof(szKeywordArr)		);
	memcpy_raw(isSorted       , KeywordSetMgr.isSorted       , sizeof(isSorted)			); // MIK 2000.12.01 binary search
	memcpy_raw(nKeywordMaxLenArr, KeywordSetMgr.nKeywordMaxLenArr, sizeof(nKeywordMaxLenArr) ); //2014.05.04 Moca
	return *this;
}


/*! @brief �L�[���[�h�Z�b�g�̒ǉ�

	@date 2005.01.26 Moca �V�K�쐬
	@date 2005.01.29 genta �T�C�Y0�ō쐬��realloc����悤��
*/
bool KeywordSetMgr::AddKeywordSet(
	const wchar_t*	pszSetName,		// [in] �Z�b�g��
	bool			bKeywordCase,	// [in] �啶���������̋�ʁDtrue:����, false:����
	int				nSize			// [in] �ŏ��ɗ̈���m�ۂ���T�C�Y�D
	)
{
	if (nSize < 0) {
		nSize = nKeywordSetBlockSize;
	}
	if (MAX_SETNUM <= nKeywordSetNum) {
		return false;
	}
	int nIdx = nKeywordSetNum;	// �ǉ��ʒu
	nStartIdx[++nKeywordSetNum] = nStartIdx[nIdx];// �T�C�Y0�ŃZ�b�g�ǉ�

	if (!KeywordReAlloc(nIdx, nSize)) {
		--nKeywordSetNum;	//	�L�[���[�h�Z�b�g�̒ǉ����L�����Z������
		return false;
	}
	wcsncpy( szSetNameArr[nIdx], pszSetName, _countof(szSetNameArr[nIdx]) - 1 );
	szSetNameArr[nIdx][_countof(szSetNameArr[nIdx]) - 1] = L'\0';
	bKeywordCaseArr[nIdx] = bKeywordCase;
	nKeywordNumArr[nIdx] = 0;
	isSorted[nIdx] = 0;	// MIK 2000.12.01 binary search
	return true;
}

// ���Ԗڂ̃Z�b�g���폜
bool KeywordSetMgr::DelKeywordSet(int nIdx)
{
	if (nKeywordSetNum <= nIdx ||
		0 > nIdx
	) {
		return false;
	}
	// �L�[���[�h�̈���J��
	KeywordReAlloc(nIdx, 0);
	
	for (int i=nIdx; i<nKeywordSetNum-1; ++i) {
		// �z��܂邲�ƃR�s�[
		memcpy_raw(szSetNameArr[i], szSetNameArr[i + 1], sizeof(szSetNameArr[0]));
		bKeywordCaseArr[i] = bKeywordCaseArr[i + 1];
		nKeywordNumArr[i] = nKeywordNumArr[i + 1];
		nStartIdx[i] = nStartIdx[i + 1];	//	2004.07.29 Moca �ϒ��L��
		isSorted[i] = isSorted[i + 1];	// MIK 2000.12.01 binary search
		nKeywordMaxLenArr[i] = nKeywordMaxLenArr[i+1];	// 2014.05.04 Moca
	}
	nStartIdx[nKeywordSetNum - 1] = nStartIdx[nKeywordSetNum];	// 2007.07.14 ryoji ���ꂪ�����Ɩ������ŏI�Z�b�g�̐擪�ɂȂ��Ă��܂�
	--nKeywordSetNum;
	if (nKeywordSetNum <= nCurrentKeywordSetIdx) {
		nCurrentKeywordSetIdx = nKeywordSetNum - 1;
// �Z�b�g�������Ȃ����Ƃ��AnCurrentKeywordSetIdx���킴��-1�ɂ��邽�߁A�R�����g��
//		if (0 > nCurrentKeywordSetIdx) {
//			nCurrentKeywordSetIdx = 0;
//		}
	}
	return true;
}



/*! ���Ԗڂ̃Z�b�g�̃Z�b�g����Ԃ�

	@param nIdx [in] �Z�b�g�ԍ� 0�`�L�[���[�h�Z�b�g��-1
*/
const wchar_t* KeywordSetMgr::GetTypeName(int nIdx)
{
	if (nIdx < 0 || nKeywordSetNum <= nIdx) {
		return NULL;
	}
	return szSetNameArr[nIdx];
}

/*! ���Ԗڂ̃Z�b�g�̃Z�b�g�����Đݒ�

	@date 2005.01.26 Moca �V�K�쐬
*/
const wchar_t* KeywordSetMgr::SetTypeName(int nIdx, const wchar_t* name)
{
	if (!name || nIdx < 0 || nKeywordSetNum <= nIdx) {
		return NULL;
	}
	wcsncpy(szSetNameArr[nIdx], name, MAX_SETNAMELEN);
	szSetNameArr[nIdx][MAX_SETNAMELEN] = L'\0';
	return szSetNameArr[nIdx];
}

// ���Ԗڂ̃Z�b�g�̃L�[���[�h�̐���Ԃ�
int KeywordSetMgr::GetKeywordNum(int nIdx)
{
	if (nIdx < 0 || nKeywordSetNum <= nIdx) {
		return 0;
	}
	return nKeywordNumArr[nIdx];
}

/*! ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h��Ԃ�

	@param nIdx [in] �L�[���[�h�Z�b�g�ԍ�
	@param nIdx2 [in] �L�[���[�h�ԍ�
*/
const wchar_t* KeywordSetMgr::GetKeyword(int nIdx, int nIdx2)
{
	if (nIdx < 0 || nKeywordSetNum <= nIdx) {
		return NULL;
	}
	if (nIdx2 < 0 || nKeywordNumArr[nIdx] <= nIdx2) {
		return NULL;
	}
	return szKeywordArr[nStartIdx[nIdx] + nIdx2];
}

// ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h��ҏW
const wchar_t* KeywordSetMgr::UpdateKeyword(
	int				nIdx,		// [in] �L�[���[�h�Z�b�g�ԍ�
	int				nIdx2,		// [in] �L�[���[�h�ԍ�
	const WCHAR*	pszKeyword	// [in] �ݒ肷��L�[���[�h
	)
{
	if (nIdx < 0 || nKeywordSetNum <= nIdx) {
		return NULL;
	}
	if (nIdx2 < 0 || nKeywordNumArr[nIdx] <= nIdx2) {
		return NULL;
	}
	// 0�o�C�g�̒����̃L�[���[�h�͕ҏW���Ȃ�
	if (pszKeyword[0] == L'\0') {
		return NULL;
	}
	// �d�������L�[���[�h�͕ҏW���Ȃ�
	for (int i=nStartIdx[nIdx]; i<nStartIdx[nIdx]+nKeywordNumArr[nIdx]; ++i) {
		if (wcscmp(szKeywordArr[i], pszKeyword) == 0) {
			return NULL;
		}
	}
	isSorted[nIdx] = 0;	// MIK 2000.12.01 binary search
	wchar_t* p = szKeywordArr[nStartIdx[nIdx] + nIdx2];
	wcsncpy( p, pszKeyword, MAX_KEYWORDLEN );
	p[MAX_KEYWORDLEN] = L'\0';
	return p;
}


/*! ���Ԗڂ̃Z�b�g�ɃL�[���[�h��ǉ�

	@param nIdx [in] �Z�b�g�ԍ�
	@param pszKeyword [in] �L�[���[�h������
	
	@return 0: ����, 1: �Z�b�g�ԍ��G���[�C2: �������m�ۃG���[
		3: �L�[���[�h�s���C4: �L�[���[�h�d��

*/
int KeywordSetMgr::AddKeyword(int nIdx, const wchar_t* pszKeyword)
{
	if (nKeywordSetNum <= nIdx) {
		return 1;
	}
// 2004.07.29 Moca
	if (!KeywordReAlloc(nIdx, nKeywordNumArr[nIdx] + 1)) {
		return 2;
	}
//	if (MAX_KEYWORDNUM <= nKeywordNumArr[nIdx]) {
//		return FALSE;
//	}

	// 0�o�C�g�̒����̃L�[���[�h�͓o�^���Ȃ�
	if (pszKeyword[0] == L'\0') {
		return 3;
	}
	// �d�������L�[���[�h�͓o�^���Ȃ�
	for (int i=nStartIdx[nIdx]; i<nStartIdx[nIdx]+nKeywordNumArr[nIdx]; ++i) {
		if (wcscmp(szKeywordArr[i], pszKeyword) == 0) {
			return 4;
		}
	}
	// MAX_KEYWORDLEN��蒷���L�[���[�h�͐؂�̂Ă�
	if (MAX_KEYWORDLEN < wcslen(pszKeyword)) {
		wmemcpy(szKeywordArr[nStartIdx[nIdx] + nKeywordNumArr[nIdx]], pszKeyword, MAX_KEYWORDLEN);
		szKeywordArr[nStartIdx[nIdx] + nKeywordNumArr[nIdx]][MAX_KEYWORDLEN] = L'\0';
	}else {
		wcscpy(szKeywordArr[nStartIdx[nIdx] + nKeywordNumArr[nIdx]], pszKeyword);
	}
	nKeywordNumArr[nIdx]++;
	isSorted[nIdx] = 0;	// MIK 2000.12.01 binary search
	return 0;
}


/*! ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h���폜

	@param nIdx [in] �L�[���[�h�Z�b�g�ԍ�
	@param nIdx2 [in] �L�[���[�h�ԍ�
*/
int KeywordSetMgr::DelKeyword(int nIdx, int nIdx2)
{
	if (nIdx < 0 || nKeywordSetNum <= nIdx) {
		return 1;
	}
	if (nIdx2 < 0 ||  nKeywordNumArr[nIdx] <= nIdx2) {
		return 2;
	}
	if (0 >= nKeywordNumArr[nIdx]) {
		return 3;	//	�o�^����0�Ȃ��̏����ň���������̂ł����ɂ͗��Ȃ��H
	}
	int nDelKeywordLen = wcslen( szKeywordArr[nStartIdx[nIdx] + nIdx2] );
	int endPos = nStartIdx[nIdx] + nKeywordNumArr[nIdx] - 1;
	for (int i=nStartIdx[nIdx]+nIdx2; i<endPos; ++i) {
		wcscpy(szKeywordArr[i], szKeywordArr[i + 1]);
	}
	nKeywordNumArr[nIdx]--;

	// 2005.01.26 Moca 1���炷�����Ȃ̂ŁA�\�[�g�̏�Ԃ͕ێ������
	// isSorted[nIdx] = 0;	// MIK 2000.12.01 binary search
	KeywordReAlloc(nIdx, nKeywordNumArr[nIdx]);	// 2004.07.29 Moca

	// 2014.05.04 Moca �L�[���[�h���̍Čv�Z
	if (nDelKeywordLen == nKeywordMaxLenArr[nIdx]) {
		KeywordMaxLen(nIdx);
	}
	return 0;
}


// MIK START 2000.12.01 binary search
/*!	�L�[���[�h�̃\�[�g�ƃL�[���[�h���̍ő�l�v�Z

	@param nIdx [in] �L�[���[�h�Z�b�g�ԍ�

*/
typedef int (__cdecl *qsort_callback)(const void *, const void *);
void KeywordSetMgr::SortKeyword(int nIdx)
{
	// nIdx�̃Z�b�g���\�[�g����B
	if (bKeywordCaseArr[nIdx]) {
		qsort(
			szKeywordArr[nStartIdx[nIdx]],
			nKeywordNumArr[nIdx],
			sizeof(szKeywordArr[0]),
			(qsort_callback)wcscmp
		);
	}else {
		qsort(
			szKeywordArr[nStartIdx[nIdx]],
			nKeywordNumArr[nIdx],
			sizeof(szKeywordArr[0]),
			(qsort_callback)wcsicmp
		);
	}
	KeywordMaxLen(nIdx);
	isSorted[nIdx] = 1;
	return;
}

void KeywordSetMgr::KeywordMaxLen(int nIdx)
{
	int nMaxLen = 0;
	const int nEnd = nStartIdx[nIdx] + nKeywordNumArr[nIdx];
	for (int i=nStartIdx[nIdx]; i<nEnd; ++i) {
		int len = wcslen( szKeywordArr[i] );
		if (nMaxLen < len) {
			nMaxLen = len;
		}
	}
	nKeywordMaxLenArr[nIdx] = nMaxLen;
}


/** nIdx�Ԗڂ̃L�[���[�h�Z�b�g���� pszKeyword��T���B
	������� 0�ȏ���A������Ȃ���Ε��̐���Ԃ��B
	@retval 0�ȏ� ���������B
	@retval -1     ������Ȃ������B
	@retval -2     ������Ȃ��������ApszKeyword����n�܂�L�[���[�h�����݂��Ă���B
	@retval intmax �����������ApszKeyword����n�܂�A��蒷���L�[���[�h�����݂��Ă���B
*/
int KeywordSetMgr::SearchKeyword2(
	int nIdx,
	const wchar_t* pszKeyword,
	int nKeywordLen
	)
{
	// sort
	if (isSorted[nIdx] == 0) {
		SortKeyword(nIdx);
	}

	if (nKeywordMaxLenArr[nIdx] < nKeywordLen) {
		return -1; // �����I�[�o�[�B
	}

	int result = -1;
	int pl = nStartIdx[nIdx];
	int pr = nStartIdx[nIdx] + nKeywordNumArr[nIdx] - 1;
	int pc = (pr + 1 - pl) / 2 + pl;
	int (*const cmp)(const wchar_t*, const wchar_t*, size_t) = bKeywordCaseArr[nIdx] ? wcsncmp : wcsnicmp;
	while (pl <= pr) {
		const int ret = cmp(pszKeyword, szKeywordArr[pc], nKeywordLen);
		if (0 < ret) {
			pl = pc + 1;
		}else if (ret < 0) {
			pr = pc - 1;
		}else {
			if (wcslen(szKeywordArr[pc]) > static_cast<size_t>(nKeywordLen)) {
				// �n�܂�͈�v����������������Ȃ��B
				if (0 <= result) {
					result = std::numeric_limits<int>::max();
					break;
				}
				result = -2;
				// �҂������v����L�[���[�h��T�����߂ɑ�����B
				pr = pc - 1;
			}else {
				// ��v����L�[���[�h�����������B
				if (result == -2) {
					result = std::numeric_limits<int>::max();
					break;
				}
				result = pc - nStartIdx[nIdx];
				// ��蒷���L�[���[�h��T�����߂ɑ�����B
				pl = pc + 1;
			}
		}
		pc = (pr + 1 - pl) / 2 + pl;
	}
	return result;
}
// MIK END

// MIK START 2000.12.01 START
void KeywordSetMgr::SetKeywordCase(int nIdx, bool bCase)
{
	// �啶�����������f�͂P�r�b�g����Ύ����ł���B
	// ����int�^(sizeof(int) * �Z�b�g�� = 4 * 100 = 400)����,
	// char�^(sizeof(char) * �Z�b�g�� = 1 * 100 = 100)�ŏ\������
	// �r�b�g���삵�Ă������B
	bKeywordCaseArr[nIdx] = bCase;
	isSorted[nIdx] = 0;
	return;
}

bool KeywordSetMgr::GetKeywordCase(int nIdx)
{
	return bKeywordCaseArr[nIdx];
}
// MIK END

// From Here 2004.07.29 Moca �ϒ��L��
/*!	@brief \\0�܂���TAB�ŋ�؂�ꂽ�����񂩂�L�[���[�h��ݒ�

	@return �o�^�ɐ��������L�[���[�h��
	
	@author Moca
	@date 2004.07.29 Moca CShareData::ShareData_IO_2���̃R�[�h�����Ɉڒz�E�쐬
*/
int KeywordSetMgr::SetKeywordArr(
	int				nIdx,			// [in] �L�[���[�h�Z�b�g�ԍ�
	int				nSize,			// [in] �L�[���[�h��
	const wchar_t*	pszKeywordArr	// [in]�ukey\\tword\\t\\0�v���́ukey\\0word\\0\\0�v�̌`��
	)
{
	if (!KeywordReAlloc(nIdx, nSize)) {
		return 0;
	}
	int cnt, i;
	const wchar_t* ptr = pszKeywordArr;
	for (cnt = 0, i = nStartIdx[nIdx];
		i < nStartIdx[nIdx] + nSize && *ptr != L'\0';
		++cnt, ++i
	) {
		//	May 25, 2003 �L�[���[�h�̋�؂�Ƃ���\0�ȊO��TAB���󂯕t����悤�ɂ���
		const wchar_t* pTop = ptr;	// �L�[���[�h�̐擪�ʒu��ۑ�
		while (*ptr != L'\t' && *ptr != L'\0') {
			++ptr;
		}
		int kwlen = ptr - pTop;
		wmemcpy(szKeywordArr[i], pTop, kwlen);
		szKeywordArr[i][kwlen] = L'\0';
		++ptr;
	}
	nKeywordNumArr[nIdx] = cnt;
	return nSize;
}

/*!
	�L�[���[�h���X�g��ݒ�

	@return �o�^�����L�[���[�h���D0�͎��s�D
*/
int KeywordSetMgr::SetKeywordArr(
	int				nIdx,				// [in] �L�[���[�h�Z�b�g�ԍ�
	int				nSize,				// [in] ppszKeywordArr�̗v�f��
	const wchar_t*	ppszKeywordArr[]	// [in] �L�[���[�h�̔z��(�d���E�����������A�l���ς݂ł��邱��)
	)
{
	if (!KeywordReAlloc(nIdx, nSize)) {
		return 0;
	}
	for (int cnt = 0, i = nStartIdx[nIdx];
		i < nStartIdx[nIdx] + nSize;
		++cnt, ++i
	) {
		wcscpy_s(szKeywordArr[i], ppszKeywordArr[cnt]);
	}
	nKeywordNumArr[nIdx] = nSize;
	return nSize;
}

/*!	@brief �L�[���[�h�̐���

	�d����g�p�s�̃L�[���[�h����菜��

	@param nIdx [in] �L�[���[�h�Z�b�g�ԍ�
	
	@return �폜�����L�[���[�h��
*/
int KeywordSetMgr::CleanKeywords(int nIdx)
{
	// ��Ƀ\�[�g���Ă����Ȃ��ƁA��ŏ��Ԃ��ς��Ɠs��������
	if (isSorted[nIdx] == 0) {
		SortKeyword(nIdx);
	}

	int nDelCount = 0;	// �폜�L�[���[�h��
	int i = 0;
	while (i < GetKeywordNum(nIdx) - 1) {
		const wchar_t* p = GetKeyword(nIdx, i);
		bool bDelKey = false;	// true�Ȃ�폜�Ώ�
		// �d������L�[���[�h��
		const wchar_t* r = GetKeyword(nIdx, i + 1);
		unsigned int nKeywordLen = wcslen(p);
		if (nKeywordLen == wcslen(r)) {
			if (bKeywordCaseArr[nIdx]) {
				if (auto_memcmp(p, r, nKeywordLen) == 0) {
					bDelKey = true;
				}
			}else {
				if (auto_memicmp(p, r, nKeywordLen) == 0) {
					bDelKey = true;
				}
			}
		}
		if (bDelKey) {
			DelKeyword(nIdx, i);
			++nDelCount;
			// ��낪�����̂ŁAi�𑝂₳�Ȃ�
		}else {
			++i;
		}
	}
	return nDelCount;
}

/*!	@brief �L�[���[�h�ǉ��]�n�̖₢���킹

	@param nIdx [in] �L�[���[�h�Z�b�g�ԍ�
	@return true: ����1�ǉ��\, false: �ǉ��s�\

	@date 2005.01.26 Moca �V�K�쐬
	@date 2005.01.29 genta ���蓖�čς݂̗̈�ɋ󂫂�����Ίg���s�\�ł��ǉ��\
*/
bool KeywordSetMgr::CanAddKeyword(int nIdx)
{
	//	���蓖�čς݂̗̈�̋󂫂��܂����ׂ�
	int nSizeOld = GetAllocSize(nIdx);
	if (nKeywordNumArr[nIdx] < nSizeOld) {
		return true;
	}

	//	���蓖�čςݗ̈悪�����ς��Ȃ�΁C���蓖�ĉ\�̈�̗L�����m�F
	//	�ꉞ���蓖�čŏ��P�ʕ��c���Ă��邱�Ƃ��m�F�D
	if (GetFreeSize() >= nKeywordSetBlockSize) {
		return true;
	}

	//	����ł����߂�
	return false;
}

#if 0
/*!	�V�����L�[���[�h�Z�b�g�̃L�[���[�h�̈���m�ۂ���
	nKeywordSetNum�́A�Ăяo�������A�Ăяo������� + 1����
*/
bool KeywordSetMgr::KeywordAlloc(int nSize)
{
	// assert(nKeywordSetNum < MAX_SETNUM);
	// assert(0 <= nSize);

	// �u���b�N�̃T�C�Y�Ő���
	int nAllocSize = GetAlignmentSize(nSize);

	if (GetFreeSize() < nAllocSize) {
		// �������s��
		return false;
	}
	nStartIdx[nKeywordSetNum + 1] = m_nStartIdx[m_nKeywordSetNum] + nAllocSize;
	for (int i=nKeywordSetNum+1; i<MAX_SETNUM; ++i) {
		nStartIdx[i + 1] = nStartIdx[i];
	}
	return true;
}
#endif

/*!	�������ς݂̃L�[���[�h�Z�b�g�̃L�[���[�h�̈�̍Ċ��蓖�āA������s��

	@param nIdx [in] �L�[���[�h�Z�b�g�ԍ�
	@param nSize [in] �K�v�ȃL�[���[�h�� (0�`)
*/
bool KeywordSetMgr::KeywordReAlloc(int nIdx, int nSize)
{
	// assert(0 <= nIdx && nIdx < nKeywordSetNum);

	// �u���b�N�̃T�C�Y�Ő���
	int nAllocSize = GetAlignmentSize(nSize);
	int nSizeOld = GetAllocSize(nIdx);

	if (nSize < 0) {
		return false;
	}
	if (nAllocSize == nSizeOld) {
		// �T�C�Y�ύX�Ȃ�
		return true;
	}

	int nDiffSize = nAllocSize - nSizeOld;
	if (GetFreeSize() < nDiffSize) {
		// �������s��
		return false;
	}
	// ���̃L�[���[�h�Z�b�g�̃L�[���[�h�����ׂĈړ�����
	if (nIdx + 1 < nKeywordSetNum) {
		int nKeywordIdx = nStartIdx[nIdx + 1];
		int nKeywordNum = nStartIdx[nKeywordSetNum] - nStartIdx[nIdx + 1];
		memmove(
			szKeywordArr[nKeywordIdx + nDiffSize],
			szKeywordArr[nKeywordIdx],
			nKeywordNum * sizeof(szKeywordArr[0])
		);
	}
	for (int i=nIdx+1; i<=nKeywordSetNum; ++i) {
		nStartIdx[i] += nDiffSize;
	}
	return true;
}

/*!	@brief ���蓖�čς݃L�[���[�h�� 

	@param nIdx [in] �L�[���[�h�Z�b�g�ԍ�
	@return �L�[���[�h�Z�b�g�Ɋ��蓖�čς݂̃L�[���[�h��
*/
int KeywordSetMgr::GetAllocSize(int nIdx) const
{
	return nStartIdx[nIdx + 1] - nStartIdx[nIdx];
}

/*! ���L�󂫃X�y�[�X

	@date 2004.07.29 Moca �V�K�쐬
	
	@return ���L�󂫗̈�(�L�[���[�h��)
*/
int KeywordSetMgr::GetFreeSize(void) const 
{
	return MAX_KEYWORDNUM - nStartIdx[nKeywordSetNum];
}

// To Here 2004.07.29 Moca

// �L�[���[�h�Z�b�g������Z�b�g�ԍ����擾�B������Ȃ���� -1
//	Uchi 2010/4/14
int  KeywordSetMgr::SearchKeywordSet(const wchar_t* pszKeyword)
{
	int nIdx = -1;
	for (int i=0; i<nKeywordSetNum; ++i) {
		if (wcscmp(szSetNameArr[i], pszKeyword) == 0) {
			nIdx = i;
			break;
		}
	}
	return nIdx;
}


