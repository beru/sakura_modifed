/*!	@file
	@brief �A�E�g���C����� �f�[�^�z��

	@author Norio Nakatani
	@date	1998/06/23 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <stdlib.h>
#include "outline/FuncInfoArr.h"
#include "outline/FuncInfo.h"


// FuncInfoArr�N���X�\�z
FuncInfoArr::FuncInfoArr()
{
	nFuncInfoArrNum = 0;		// �z��v�f��
	ppcFuncInfoArr = nullptr;	// �z��
	nAppendTextLenMax = 0;
	return;
}


// FuncInfoArr�N���X����
FuncInfoArr::~FuncInfoArr()
{
	Empty();
	return;
}

void FuncInfoArr::Empty(void)
{
	if (nFuncInfoArrNum > 0 && ppcFuncInfoArr) {
		for (int i=0; i<nFuncInfoArrNum; ++i) {
			delete ppcFuncInfoArr[i];
			ppcFuncInfoArr[i] = nullptr;
		}
		nFuncInfoArrNum = 0;
		free(ppcFuncInfoArr);
		ppcFuncInfoArr = nullptr;
	}
	appendTextArr.clear();
	nAppendTextLenMax = 0;
	return;
}


// 0<=�̎w��ԍ��̃f�[�^��Ԃ�
// �f�[�^���Ȃ��ꍇ��NULL��Ԃ�
FuncInfo* FuncInfoArr::GetAt(int nIdx)
{
	if (nIdx >= nFuncInfoArrNum) {
		return NULL;
	}
	return ppcFuncInfoArr[nIdx];
}

// �z��̍Ō�Ƀf�[�^��ǉ�����
void FuncInfoArr::AppendData(FuncInfo* pFuncInfo)
{
	if (nFuncInfoArrNum == 0) {
		ppcFuncInfoArr = (FuncInfo**)malloc(sizeof(FuncInfo*) * (nFuncInfoArrNum + 1));
	}else {
		ppcFuncInfoArr = (FuncInfo**)realloc(ppcFuncInfoArr, sizeof(FuncInfo*) * (nFuncInfoArrNum + 1));
	}
	ppcFuncInfoArr[nFuncInfoArrNum] = pFuncInfo;
	++nFuncInfoArrNum;
	return;
}


/*! �z��̍Ō�Ƀf�[�^��ǉ�����

	@date 2002.04.01 YAZAKI �[������
*/
void FuncInfoArr::AppendData(
	LogicInt		nFuncLineCRLF,		// �֐��̂���s(CRLF�P��)
	LogicInt		nFuncColCRLF,		// �֐��̂��錅(CRLF�P��)
	LayoutInt		nFuncLineLAYOUT,	// �֐��̂���s(�܂�Ԃ��P��)
	LayoutInt		nFuncColLAYOUT,		// �֐��̂��錅(�܂�Ԃ��P��)
	const TCHAR*	pszFuncName,		// �֐���
	const TCHAR*	pszFileName,		// �t�@�C����
	int				nInfo,				// �t�����
	int				nDepth				// �[��
	)
{
	FuncInfo* pFuncInfo = new FuncInfo(nFuncLineCRLF,
									  nFuncColCRLF,
									  nFuncLineLAYOUT,
									  nFuncColLAYOUT,
									  pszFuncName,
									  pszFileName,
									  nInfo);
	pFuncInfo->m_nDepth = nDepth;
	AppendData(pFuncInfo);
	return;
}

void FuncInfoArr::AppendData(
	LogicInt			nFuncLineCRLF,		// �֐��̂���s(CRLF�P��)
	LogicInt			nFuncColCRLF,		// �֐��̂��錅(CRLF�P��)
	LayoutInt			nFuncLineLAYOUT,	// �֐��̂���s(�܂�Ԃ��P��)
	LayoutInt			nFuncColLAYOUT,		// �֐��̂��錅(�܂�Ԃ��P��)
	const NOT_TCHAR*	pszFuncName,		// �֐���
	const NOT_TCHAR*	pszFileName,		// �t�@�C����
	int					nInfo,				// �t�����
	int					nDepth				// �[��
	)
{
	AppendData(nFuncLineCRLF,
			  nFuncColCRLF,
			  nFuncLineLAYOUT,
			  nFuncColLAYOUT,
			  to_tchar(pszFuncName),
			  (pszFileName ? to_tchar(pszFileName) : NULL),
			  nInfo,
			  nDepth);
}


void FuncInfoArr::AppendData(
	LogicInt		nFuncLineCRLF,		// �֐��̂���s(CRLF�P��)
	LayoutInt		nFuncLineLAYOUT,	// �֐��̂���s(�܂�Ԃ��P��)
	const TCHAR*	pszFuncName,		// �֐���
	int				nInfo,				// �t�����
	int				nDepth				// �[��
	)
{
	AppendData(nFuncLineCRLF,
			  LogicInt(1),
			  nFuncLineLAYOUT,
			  LayoutInt(1),
			  pszFuncName,
			  NULL,
			  nInfo,
			  nDepth);
	return;
}

void FuncInfoArr::AppendData(
	LogicInt			nFuncLineCRLF,		// �֐��̂���s(CRLF�P��)
	LayoutInt			nFuncLineLAYOUT,	// �֐��̂���s(�܂�Ԃ��P��)
	const NOT_TCHAR*	pszFuncName,		// �֐���
	int					nInfo,				// �t�����
	int					nDepth				// �[��
	)
{
	AppendData(nFuncLineCRLF, nFuncLineLAYOUT, to_tchar(pszFuncName), nInfo, nDepth);
}


void FuncInfoArr::DUMP(void)
{
#ifdef _DEBUG
	MYTRACE(_T("=============================\n"));
	for (int i=0; i<nFuncInfoArrNum; ++i) {
		MYTRACE(_T("[%d]------------------\n"), i);
		MYTRACE(_T("m_nFuncLineCRLF	=%d\n"), ppcFuncInfoArr[i]->m_nFuncLineCRLF);
		MYTRACE(_T("m_nFuncLineLAYOUT	=%d\n"), ppcFuncInfoArr[i]->m_nFuncLineLAYOUT);
		MYTRACE(_T("m_memFuncName	=[%ts]\n"), ppcFuncInfoArr[i]->m_memFuncName.GetStringPtr());
		MYTRACE( _T("m_memFileName	=[%ts]\n"),
			(ppcFuncInfoArr[i]->m_memFileName.GetStringPtr() ? ppcFuncInfoArr[i]->m_memFileName.GetStringPtr() : _T("NULL")) );
		MYTRACE(_T("m_nInfo			=%d\n"), ppcFuncInfoArr[i]->m_nInfo);
	}
	MYTRACE(_T("=============================\n"));
#endif
}

void FuncInfoArr::SetAppendText(
	int info,
	const std::wstring& s,
	bool overwrite
	)
{
	if (appendTextArr.find(info) == appendTextArr.end()) {
		// �L�[�����݂��Ȃ��ꍇ�A�ǉ�����
		appendTextArr.emplace(info, s);
		if (nAppendTextLenMax < (int)s.length()) {
			nAppendTextLenMax = s.length();
		}
#ifndef	_UNICODE
		std::tstring t = to_tchar(s.c_str());
		if (m_nAppendTextLenMax < (int)t.length()) {
			m_nAppendTextLenMax = t.length();
		}
#endif
	}else {
		// �L�[�����݂���ꍇ�A�l������������
		if (overwrite) {
			appendTextArr[info] = s;
		}
	}
}

std::wstring FuncInfoArr::GetAppendText(int info)
{
	if (appendTextArr.find(info) == appendTextArr.end()) {
		// �L�[�����݂��Ȃ��ꍇ�A�󕶎����Ԃ�
		return std::wstring();
	}else {
		// �L�[�����݂���ꍇ�A�l��Ԃ�
		return appendTextArr[info];
	}
}

