#include "StdAfx.h"
#include "DocReader.h"
#include "logic/DocLine.h"
#include "logic/DocLineMgr.h"

/* 全行データを返す
	改行コードは、CRLFに統一される。
	@retval 全行データ。freeで開放しなければならない。
	@note   Debug版のテストにのみ使用している。
*/
wchar_t* DocReader::GetAllData(size_t* pnDataLen)
{
	const DocLine* pDocLine = pDocLineMgr->GetDocLineTop();
	size_t nDataLen = 0;
	while (pDocLine) {
		// Oct. 7, 2002 YAZAKI
		nDataLen += pDocLine->GetLengthWithoutEOL() + 2;	// \r\nを追加して返すため + 2する。
		pDocLine = pDocLine->GetNextLine();
	}
	
	wchar_t* pData = (wchar_t*)malloc((nDataLen + 1) * sizeof(wchar_t));
	if (!pData) {
		TopErrorMessage(NULL, LS(STR_ERR_DLGDOCLM6), nDataLen + 1);
		return NULL;
	}
	pDocLine = pDocLineMgr->GetDocLineTop();
	
	nDataLen = 0;
	while (pDocLine) {
		// Oct. 7, 2002 YAZAKI
		size_t nLineLen = pDocLine->GetLengthWithoutEOL();
		if (0 < nLineLen) {
			wmemcpy(&pData[nDataLen], pDocLine->GetPtr(), nLineLen);
			nDataLen += nLineLen;
		}
		pData[nDataLen++] = L'\r';
		pData[nDataLen++] = L'\n';
		pDocLine = pDocLine->GetNextLine();
	}
	pData[nDataLen] = L'\0';
	*pnDataLen = nDataLen;
	return pData;
}

const wchar_t* DocReader::GetLineStr(size_t nLine, size_t* pnLineLen)
{
	const DocLine* pDocLine = pDocLineMgr->GetLine(nLine);
	if (!pDocLine) {
		*pnLineLen = 0;
		return NULL;
	}
	// 2002/2/10 aroka CMemory のメンバ変数に直接アクセスしない(inline化されているので速度的な問題はない)
	return pDocLine->GetDocLineStrWithEOL(pnLineLen);
}

/*!
	指定された行番号の文字列と改行コードを除く長さを取得
	
	@author Moca
	@date 2003.06.22
*/
const wchar_t* DocReader::GetLineStrWithoutEOL(size_t nLine, size_t* pnLineLen)
{
	const DocLine* pDocLine = pDocLineMgr->GetLine(nLine);
	if (!pDocLine) {
		*pnLineLen = 0;
		return NULL;
	}
	*pnLineLen = pDocLine->GetLengthWithoutEOL();
	return pDocLine->GetPtr();
}


/*! 順アクセスモード：先頭行を得る

	@param pnLineLen [out] 行の長さが返る。
	@return 1行目の先頭へのポインタ。
	データが1行もないときは、長さ0、ポインタNULLが返る。

*/
const wchar_t* DocReader::GetFirstLinrStr(size_t* pnLineLen)
{
	const wchar_t* pszLine;
	if (pDocLineMgr->GetLineCount() == 0) {
		pszLine = NULL;
		*pnLineLen = 0;
	}else {
		pszLine = pDocLineMgr->GetDocLineTop()->GetDocLineStrWithEOL(pnLineLen);
		pDocLineMgr->pDocLineCurrent = const_cast<DocLine*>(pDocLineMgr->GetDocLineTop()->GetNextLine());
	}
	return pszLine;
}


/*!
	順アクセスモード：次の行を得る

	@param pnLineLen [out] 行の長さが返る。
	@return 次行の先頭へのポインタ。
	GetFirstLinrStr()が呼び出されていないとNULLが返る

*/
const wchar_t* DocReader::GetNextLinrStr(size_t* pnLineLen)
{
	const wchar_t* pszLine;
	if (!pDocLineMgr->pDocLineCurrent) {
		pszLine = NULL;
		*pnLineLen = 0;
	}else {
		pszLine = pDocLineMgr->pDocLineCurrent->GetDocLineStrWithEOL(pnLineLen);
		pDocLineMgr->pDocLineCurrent = pDocLineMgr->pDocLineCurrent->GetNextLine();
	}
	return pszLine;
}

