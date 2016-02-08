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
#include "outline/CFuncInfoArr.h"
#include "outline/CFuncInfo.h"


// CFuncInfoArr�N���X�\�z
CFuncInfoArr::CFuncInfoArr()
{
	m_nFuncInfoArrNum = 0;		// �z��v�f��
	m_ppcFuncInfoArr = NULL;	// �z��
	m_nAppendTextLenMax = 0;
	return;
}


// CFuncInfoArr�N���X����
CFuncInfoArr::~CFuncInfoArr()
{
	Empty();
	return;
}

void CFuncInfoArr::Empty(void)
{
	if (m_nFuncInfoArrNum > 0 && m_ppcFuncInfoArr) {
		for (int i=0; i<m_nFuncInfoArrNum; ++i) {
			delete m_ppcFuncInfoArr[i];
			m_ppcFuncInfoArr[i] = NULL;
		}
		m_nFuncInfoArrNum = 0;
		free(m_ppcFuncInfoArr);
		m_ppcFuncInfoArr = NULL;
	}
	m_AppendTextArr.clear();
	m_nAppendTextLenMax = 0;
	return;
}


// 0<=�̎w��ԍ��̃f�[�^��Ԃ�
// �f�[�^���Ȃ��ꍇ��NULL��Ԃ�
FuncInfo* CFuncInfoArr::GetAt(int nIdx)
{
	if (nIdx >= m_nFuncInfoArrNum) {
		return NULL;
	}
	return m_ppcFuncInfoArr[nIdx];
}

//! �z��̍Ō�Ƀf�[�^��ǉ�����
void CFuncInfoArr::AppendData(FuncInfo* pcFuncInfo)
{
	if (m_nFuncInfoArrNum == 0) {
		m_ppcFuncInfoArr = (FuncInfo**)malloc(sizeof(FuncInfo*) * (m_nFuncInfoArrNum + 1));
	}else {
		m_ppcFuncInfoArr = (FuncInfo**)realloc(m_ppcFuncInfoArr, sizeof(FuncInfo*) * (m_nFuncInfoArrNum + 1));
	}
	m_ppcFuncInfoArr[m_nFuncInfoArrNum] = pcFuncInfo;
	++m_nFuncInfoArrNum;
	return;
}


/*! �z��̍Ō�Ƀf�[�^��ǉ�����

	@date 2002.04.01 YAZAKI �[������
*/
void CFuncInfoArr::AppendData(
	LogicInt		nFuncLineCRLF,		//!< �֐��̂���s(CRLF�P��)
	LogicInt		nFuncColCRLF,		//!< �֐��̂��錅(CRLF�P��)
	LayoutInt		nFuncLineLAYOUT,	//!< �֐��̂���s(�܂�Ԃ��P��)
	LayoutInt		nFuncColLAYOUT,		//!< �֐��̂��錅(�܂�Ԃ��P��)
	const TCHAR*	pszFuncName,		//!< �֐���
	const TCHAR*	pszFileName,		//!< �t�@�C����
	int				nInfo,				//!< �t�����
	int				nDepth				//!< �[��
	)
{
	FuncInfo* pcFuncInfo = new FuncInfo(nFuncLineCRLF,
										  nFuncColCRLF,
										  nFuncLineLAYOUT,
										  nFuncColLAYOUT,
										  pszFuncName,
										  pszFileName,
										  nInfo);
	pcFuncInfo->m_nDepth = nDepth;
	AppendData(pcFuncInfo);
	return;
}

void CFuncInfoArr::AppendData(
	LogicInt			nFuncLineCRLF,		//!< �֐��̂���s(CRLF�P��)
	LogicInt			nFuncColCRLF,		//!< �֐��̂��錅(CRLF�P��)
	LayoutInt			nFuncLineLAYOUT,	//!< �֐��̂���s(�܂�Ԃ��P��)
	LayoutInt			nFuncColLAYOUT,		//!< �֐��̂��錅(�܂�Ԃ��P��)
	const NOT_TCHAR*	pszFuncName,		//!< �֐���
	const NOT_TCHAR*	pszFileName,		//!< �t�@�C����
	int					nInfo,				//!< �t�����
	int					nDepth				//!< �[��
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


void CFuncInfoArr::AppendData(
	LogicInt		nFuncLineCRLF,		//!< �֐��̂���s(CRLF�P��)
	LayoutInt		nFuncLineLAYOUT,	//!< �֐��̂���s(�܂�Ԃ��P��)
	const TCHAR*	pszFuncName,		//!< �֐���
	int				nInfo,				//!< �t�����
	int				nDepth				//!< �[��
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

void CFuncInfoArr::AppendData(
	LogicInt			nFuncLineCRLF,		//!< �֐��̂���s(CRLF�P��)
	LayoutInt			nFuncLineLAYOUT,	//!< �֐��̂���s(�܂�Ԃ��P��)
	const NOT_TCHAR*	pszFuncName,		//!< �֐���
	int					nInfo,				//!< �t�����
	int					nDepth				//!< �[��
	)
{
	AppendData(nFuncLineCRLF, nFuncLineLAYOUT, to_tchar(pszFuncName), nInfo, nDepth);
}


void CFuncInfoArr::DUMP(void)
{
#ifdef _DEBUG
	MYTRACE(_T("=============================\n"));
	for (int i=0; i<m_nFuncInfoArrNum; ++i) {
		MYTRACE(_T("[%d]------------------\n"), i);
		MYTRACE(_T("m_nFuncLineCRLF	=%d\n"), m_ppcFuncInfoArr[i]->m_nFuncLineCRLF);
		MYTRACE(_T("m_nFuncLineLAYOUT	=%d\n"), m_ppcFuncInfoArr[i]->m_nFuncLineLAYOUT);
		MYTRACE(_T("m_cmemFuncName	=[%ts]\n"), m_ppcFuncInfoArr[i]->m_cmemFuncName.GetStringPtr());
		MYTRACE( _T("m_cmemFileName	=[%ts]\n"),
			(m_ppcFuncInfoArr[i]->m_cmemFileName.GetStringPtr() ? m_ppcFuncInfoArr[i]->m_cmemFileName.GetStringPtr() : _T("NULL")) );
		MYTRACE(_T("m_nInfo			=%d\n"), m_ppcFuncInfoArr[i]->m_nInfo);
	}
	MYTRACE(_T("=============================\n"));
#endif
}

void CFuncInfoArr::SetAppendText(
	int info,
	const std::wstring& s,
	bool overwrite
	)
{
	if (m_AppendTextArr.find(info) == m_AppendTextArr.end()) {
		// �L�[�����݂��Ȃ��ꍇ�A�ǉ�����
		std::pair<int, std::wstring> pair(info, s);
		m_AppendTextArr.insert(pair);
		if (m_nAppendTextLenMax < (int)s.length()) {
			m_nAppendTextLenMax = s.length();
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
			m_AppendTextArr[info] = s;
		}
	}
}

std::wstring CFuncInfoArr::GetAppendText(int info)
{
	if (m_AppendTextArr.find(info) == m_AppendTextArr.end()) {
		// �L�[�����݂��Ȃ��ꍇ�A�󕶎����Ԃ�
		return std::wstring();
	}else {
		// �L�[�����݂���ꍇ�A�l��Ԃ�
		return m_AppendTextArr[info];
	}
}

