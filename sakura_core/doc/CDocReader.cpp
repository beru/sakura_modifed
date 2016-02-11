#include "StdAfx.h"
#include "CDocReader.h"
#include "logic/CDocLine.h"
#include "logic/CDocLineMgr.h"

/* 全行データを返す
	改行コードは、CFLF統一される。
	@retval 全行データ。freeで開放しなければならない。
	@note   Debug版のテストにのみ使用している。
*/
wchar_t* DocReader::GetAllData(int* pnDataLen)
{
	const DocLine* pDocLine = m_pcDocLineMgr->GetDocLineTop();
	int nDataLen = 0;
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
	pDocLine = m_pcDocLineMgr->GetDocLineTop();
	
	nDataLen = 0;
	while (pDocLine) {
		// Oct. 7, 2002 YAZAKI
		int nLineLen = pDocLine->GetLengthWithoutEOL();
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

const wchar_t* DocReader::GetLineStr(LogicInt nLine, LogicInt* pnLineLen)
{
	const DocLine* pDocLine = m_pcDocLineMgr->GetLine(nLine);
	if (!pDocLine) {
		*pnLineLen = LogicInt(0);
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
const wchar_t* DocReader::GetLineStrWithoutEOL(LogicInt nLine, int* pnLineLen)
{
	const DocLine* pDocLine = m_pcDocLineMgr->GetLine(nLine);
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
const wchar_t* DocReader::GetFirstLinrStr(int* pnLineLen)
{
	const wchar_t* pszLine;
	if (m_pcDocLineMgr->GetLineCount() == LogicInt(0)) {
		pszLine = NULL;
		*pnLineLen = 0;
	}else {
		pszLine = m_pcDocLineMgr->GetDocLineTop()->GetDocLineStrWithEOL(pnLineLen);
		m_pcDocLineMgr->m_pDocLineCurrent = const_cast<DocLine*>(m_pcDocLineMgr->GetDocLineTop()->GetNextLine());
	}
	return pszLine;
}


/*!
	順アクセスモード：次の行を得る

	@param pnLineLen [out] 行の長さが返る。
	@return 次行の先頭へのポインタ。
	GetFirstLinrStr()が呼び出されていないとNULLが返る

*/
const wchar_t* DocReader::GetNextLinrStr(int* pnLineLen)
{
	const wchar_t* pszLine;
	if (!m_pcDocLineMgr->m_pDocLineCurrent) {
		pszLine = NULL;
		*pnLineLen = 0;
	}else {
		pszLine = m_pcDocLineMgr->m_pDocLineCurrent->GetDocLineStrWithEOL(pnLineLen);
		m_pcDocLineMgr->m_pDocLineCurrent = m_pcDocLineMgr->m_pDocLineCurrent->GetNextLine();
	}
	return pszLine;
}

