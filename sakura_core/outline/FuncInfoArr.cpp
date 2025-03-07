/*!	@file
	@brief アウトライン解析 データ配列
*/

#include "StdAfx.h"
#include <stdlib.h>
#include "outline/FuncInfoArr.h"
#include "outline/FuncInfo.h"


// FuncInfoArrクラス構築
FuncInfoArr::FuncInfoArr()
{
	nFuncInfoArrNum = 0;		// 配列要素数
	ppcFuncInfoArr = nullptr;	// 配列
	nAppendTextLenMax = 0;
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


// 0<=の指定番号のデータを返す
// データがない場合はNULLを返す
FuncInfo* FuncInfoArr::GetAt(size_t nIdx)
{
	if (nIdx >= nFuncInfoArrNum) {
		return nullptr;
	}
	return ppcFuncInfoArr[nIdx];
}

// 配列の最後にデータを追加する
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


/*! 配列の最後にデータを追加する */
void FuncInfoArr::AppendData(
	size_t			nFuncLineCRLF,		// 関数のある行(CRLF単位)
	size_t			nFuncColCRLF,		// 関数のある桁(CRLF単位)
	size_t			nFuncLineLAYOUT,	// 関数のある行(折り返し単位)
	size_t			nFuncColLAYOUT,		// 関数のある桁(折り返し単位)
	const TCHAR*	pszFuncName,		// 関数名
	const TCHAR*	pszFileName,		// ファイル名
	int				nInfo,				// 付加情報
	size_t			nDepth				// 深さ
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
	size_t				nFuncLineCRLF,		// 関数のある行(CRLF単位)
	size_t				nFuncColCRLF,		// 関数のある桁(CRLF単位)
	size_t				nFuncLineLAYOUT,	// 関数のある行(折り返し単位)
	size_t				nFuncColLAYOUT,		// 関数のある桁(折り返し単位)
	const NOT_TCHAR*	pszFuncName,		// 関数名
	const NOT_TCHAR*	pszFileName,		// ファイル名
	int					nInfo,				// 付加情報
	size_t				nDepth				// 深さ
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
	size_t			nFuncLineCRLF,		// 関数のある行(CRLF単位)
	size_t			nFuncLineLAYOUT,	// 関数のある行(折り返し単位)
	const TCHAR*	pszFuncName,		// 関数名
	int				nInfo,				// 付加情報
	size_t			nDepth				// 深さ
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
	size_t				nFuncLineCRLF,		// 関数のある行(CRLF単位)
	size_t				nFuncLineLAYOUT,	// 関数のある行(折り返し単位)
	const NOT_TCHAR*	pszFuncName,		// 関数名
	int					nInfo,				// 付加情報
	size_t				nDepth				// 深さ
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
		// キーが存在しない場合、追加する
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
		// キーが存在する場合、値を書き換える
		if (overwrite) {
			appendTextArr[info] = s;
		}
	}
}

std::wstring FuncInfoArr::GetAppendText(int info)
{
	if (appendTextArr.find(info) == appendTextArr.end()) {
		// キーが存在しない場合、空文字列を返す
		return std::wstring();
	}else {
		// キーが存在する場合、値を返す
		return appendTextArr[info];
	}
}

