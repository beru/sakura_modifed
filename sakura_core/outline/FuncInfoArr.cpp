/*!	@file
	@brief アウトライン解析 データ配列

	@author Norio Nakatani
	@date	1998/06/23 作成
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


// FuncInfoArrクラス構築
FuncInfoArr::FuncInfoArr()
{
	m_nFuncInfoArrNum = 0;		// 配列要素数
	m_ppcFuncInfoArr = nullptr;	// 配列
	m_nAppendTextLenMax = 0;
	return;
}


// FuncInfoArrクラス消滅
FuncInfoArr::~FuncInfoArr()
{
	Empty();
	return;
}

void FuncInfoArr::Empty(void)
{
	if (m_nFuncInfoArrNum > 0 && m_ppcFuncInfoArr) {
		for (int i=0; i<m_nFuncInfoArrNum; ++i) {
			delete m_ppcFuncInfoArr[i];
			m_ppcFuncInfoArr[i] = nullptr;
		}
		m_nFuncInfoArrNum = 0;
		free(m_ppcFuncInfoArr);
		m_ppcFuncInfoArr = nullptr;
	}
	m_AppendTextArr.clear();
	m_nAppendTextLenMax = 0;
	return;
}


// 0<=の指定番号のデータを返す
// データがない場合はNULLを返す
FuncInfo* FuncInfoArr::GetAt(int nIdx)
{
	if (nIdx >= m_nFuncInfoArrNum) {
		return NULL;
	}
	return m_ppcFuncInfoArr[nIdx];
}

// 配列の最後にデータを追加する
void FuncInfoArr::AppendData(FuncInfo* pFuncInfo)
{
	if (m_nFuncInfoArrNum == 0) {
		m_ppcFuncInfoArr = (FuncInfo**)malloc(sizeof(FuncInfo*) * (m_nFuncInfoArrNum + 1));
	}else {
		m_ppcFuncInfoArr = (FuncInfo**)realloc(m_ppcFuncInfoArr, sizeof(FuncInfo*) * (m_nFuncInfoArrNum + 1));
	}
	m_ppcFuncInfoArr[m_nFuncInfoArrNum] = pFuncInfo;
	++m_nFuncInfoArrNum;
	return;
}


/*! 配列の最後にデータを追加する

	@date 2002.04.01 YAZAKI 深さ導入
*/
void FuncInfoArr::AppendData(
	LogicInt		nFuncLineCRLF,		// 関数のある行(CRLF単位)
	LogicInt		nFuncColCRLF,		// 関数のある桁(CRLF単位)
	LayoutInt		nFuncLineLAYOUT,	// 関数のある行(折り返し単位)
	LayoutInt		nFuncColLAYOUT,		// 関数のある桁(折り返し単位)
	const TCHAR*	pszFuncName,		// 関数名
	const TCHAR*	pszFileName,		// ファイル名
	int				nInfo,				// 付加情報
	int				nDepth				// 深さ
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
	LogicInt			nFuncLineCRLF,		// 関数のある行(CRLF単位)
	LogicInt			nFuncColCRLF,		// 関数のある桁(CRLF単位)
	LayoutInt			nFuncLineLAYOUT,	// 関数のある行(折り返し単位)
	LayoutInt			nFuncColLAYOUT,		// 関数のある桁(折り返し単位)
	const NOT_TCHAR*	pszFuncName,		// 関数名
	const NOT_TCHAR*	pszFileName,		// ファイル名
	int					nInfo,				// 付加情報
	int					nDepth				// 深さ
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
	LogicInt		nFuncLineCRLF,		// 関数のある行(CRLF単位)
	LayoutInt		nFuncLineLAYOUT,	// 関数のある行(折り返し単位)
	const TCHAR*	pszFuncName,		// 関数名
	int				nInfo,				// 付加情報
	int				nDepth				// 深さ
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
	LogicInt			nFuncLineCRLF,		// 関数のある行(CRLF単位)
	LayoutInt			nFuncLineLAYOUT,	// 関数のある行(折り返し単位)
	const NOT_TCHAR*	pszFuncName,		// 関数名
	int					nInfo,				// 付加情報
	int					nDepth				// 深さ
	)
{
	AppendData(nFuncLineCRLF, nFuncLineLAYOUT, to_tchar(pszFuncName), nInfo, nDepth);
}


void FuncInfoArr::DUMP(void)
{
#ifdef _DEBUG
	MYTRACE(_T("=============================\n"));
	for (int i=0; i<m_nFuncInfoArrNum; ++i) {
		MYTRACE(_T("[%d]------------------\n"), i);
		MYTRACE(_T("m_nFuncLineCRLF	=%d\n"), m_ppcFuncInfoArr[i]->m_nFuncLineCRLF);
		MYTRACE(_T("m_nFuncLineLAYOUT	=%d\n"), m_ppcFuncInfoArr[i]->m_nFuncLineLAYOUT);
		MYTRACE(_T("m_memFuncName	=[%ts]\n"), m_ppcFuncInfoArr[i]->m_memFuncName.GetStringPtr());
		MYTRACE( _T("m_memFileName	=[%ts]\n"),
			(m_ppcFuncInfoArr[i]->m_memFileName.GetStringPtr() ? m_ppcFuncInfoArr[i]->m_memFileName.GetStringPtr() : _T("NULL")) );
		MYTRACE(_T("m_nInfo			=%d\n"), m_ppcFuncInfoArr[i]->m_nInfo);
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
	if (m_AppendTextArr.find(info) == m_AppendTextArr.end()) {
		// キーが存在しない場合、追加する
		m_AppendTextArr.emplace(info, s);
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
		// キーが存在する場合、値を書き換える
		if (overwrite) {
			m_AppendTextArr[info] = s;
		}
	}
}

std::wstring FuncInfoArr::GetAppendText(int info)
{
	if (m_AppendTextArr.find(info) == m_AppendTextArr.end()) {
		// キーが存在しない場合、空文字列を返す
		return std::wstring();
	}else {
		// キーが存在する場合、値を返す
		return m_AppendTextArr[info];
	}
}

