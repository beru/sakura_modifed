/*!	@file
	@brief �A�E�g���C����� �f�[�^�z��
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
		for (size_t i=0; i<nFuncInfoArrNum; ++i) {
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
FuncInfo* FuncInfoArr::GetAt(size_t nIdx)
{
	if (nIdx >= nFuncInfoArrNum) {
		return nullptr;
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
	size_t			nFuncLineCRLF,		// �֐��̂���s(CRLF�P��)
	size_t			nFuncColCRLF,		// �֐��̂��錅(CRLF�P��)
	size_t			nFuncLineLAYOUT,	// �֐��̂���s(�܂�Ԃ��P��)
	size_t			nFuncColLAYOUT,		// �֐��̂��錅(�܂�Ԃ��P��)
	const TCHAR*	pszFuncName,		// �֐���
	const TCHAR*	pszFileName,		// �t�@�C����
	int				nInfo,				// �t�����
	size_t			nDepth				// �[��
	)
{
	FuncInfo* pFuncInfo = new FuncInfo(nFuncLineCRLF,
									  nFuncColCRLF,
									  nFuncLineLAYOUT,
									  nFuncColLAYOUT,
									  pszFuncName,
									  pszFileName,
									  nInfo);
	pFuncInfo->nDepth = nDepth;
	AppendData(pFuncInfo);
	return;
}

void FuncInfoArr::AppendData(
	size_t				nFuncLineCRLF,		// �֐��̂���s(CRLF�P��)
	size_t				nFuncColCRLF,		// �֐��̂��錅(CRLF�P��)
	size_t				nFuncLineLAYOUT,	// �֐��̂���s(�܂�Ԃ��P��)
	size_t				nFuncColLAYOUT,		// �֐��̂��錅(�܂�Ԃ��P��)
	const NOT_TCHAR*	pszFuncName,		// �֐���
	const NOT_TCHAR*	pszFileName,		// �t�@�C����
	int					nInfo,				// �t�����
	size_t				nDepth				// �[��
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
	size_t			nFuncLineCRLF,		// �֐��̂���s(CRLF�P��)
	size_t			nFuncLineLAYOUT,	// �֐��̂���s(�܂�Ԃ��P��)
	const TCHAR*	pszFuncName,		// �֐���
	int				nInfo,				// �t�����
	size_t			nDepth				// �[��
	)
{
	AppendData(nFuncLineCRLF,
			  1,
			  nFuncLineLAYOUT,
			  1,
			  pszFuncName,
			  NULL,
			  nInfo,
			  nDepth);
	return;
}

void FuncInfoArr::AppendData(
	size_t				nFuncLineCRLF,		// �֐��̂���s(CRLF�P��)
	size_t				nFuncLineLAYOUT,	// �֐��̂���s(�܂�Ԃ��P��)
	const NOT_TCHAR*	pszFuncName,		// �֐���
	int					nInfo,				// �t�����
	size_t				nDepth				// �[��
	)
{
	AppendData(nFuncLineCRLF, nFuncLineLAYOUT, to_tchar(pszFuncName), nInfo, nDepth);
}


void FuncInfoArr::DUMP(void)
{
#ifdef _DEBUG
	MYTRACE(_T("=============================\n"));
	for (size_t i=0; i<nFuncInfoArrNum; ++i) {
		MYTRACE(_T("[%d]------------------\n"), i);
		MYTRACE(_T("nFuncLineCRLF	=%d\n"), ppcFuncInfoArr[i]->nFuncLineCRLF);
		MYTRACE(_T("nFuncLineLAYOUT	=%d\n"), ppcFuncInfoArr[i]->nFuncLineLAYOUT);
		MYTRACE(_T("memFuncName	=[%ts]\n"), ppcFuncInfoArr[i]->memFuncName.GetStringPtr());
		MYTRACE( _T("memFileName	=[%ts]\n"),
			(ppcFuncInfoArr[i]->memFileName.GetStringPtr() ? ppcFuncInfoArr[i]->memFileName.GetStringPtr() : _T("NULL")) );
		MYTRACE(_T("nInfo			=%d\n"), ppcFuncInfoArr[i]->nInfo);
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
		if (nAppendTextLenMax < (int)t.length()) {
			nAppendTextLenMax = t.length();
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

