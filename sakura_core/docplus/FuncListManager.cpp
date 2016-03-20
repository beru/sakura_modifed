/*
	Copyright (C) 2008, kobake
	Copyright (C) 2014, Moca

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
#include "docplus/FuncListManager.h"
#include "doc/logic/DocLineMgr.h"
#include "doc/logic/DocLine.h"


bool FuncListManager::IsLineFuncList(const DocLine* pDocLine, bool bFlag) const
{
	return pDocLine->m_mark.m_funcList.GetFuncListMark() != bFlag;
}
bool FuncListManager::GetLineFuncList(const DocLine* pDocLine) const
{
	return pDocLine->m_mark.m_funcList.GetFuncListMark();
}
void FuncListManager::SetLineFuncList(DocLine* pDocLine, bool bFlag)
{
	pDocLine->m_mark.m_funcList = bFlag;
}

/*! 差分検索
	@author	MIK
	@date	2002.05.25
*/
bool FuncListManager::SearchFuncListMark(
	const DocLineMgr&	docLineMgr,
	LogicInt			nLineNum,		// 検索開始行
	SearchDirection		bPrevOrNext,
	LogicInt*			pnLineNum 		// マッチ行
	) const
{
	LogicInt nLinePos = nLineNum;

	if (bPrevOrNext == SearchDirection::Backward) {
		// 後方検索(↑)
		--nLinePos;
		const DocLine* pDocLine = docLineMgr.GetLine( nLinePos );
		while (pDocLine) {
			if (GetLineFuncList(pDocLine)) {
				*pnLineNum = nLinePos;				// マッチ行
				return true;
			}
			--nLinePos;
			pDocLine = pDocLine->GetPrevLine();
		}
	}else {
		// 前方検索(↓)
		++nLinePos;
		const DocLine* pDocLine = docLineMgr.GetLine( nLinePos );
		while (pDocLine) {
			if (GetLineFuncList(pDocLine)) {
				*pnLineNum = nLinePos;				// マッチ行
				return true;
			}
			++nLinePos;
			pDocLine = pDocLine->GetNextLine();
		}
	}
	return false;
}

// 関数リストマークをすべてリセット
void FuncListManager::ResetAllFucListMark(DocLineMgr& docLineMgr, bool bFlag)
{
	DocLine* pDocLine = docLineMgr.GetDocLineTop();
	while (pDocLine) {
		DocLine* pDocLineNext = pDocLine->GetNextLine();
		SetLineFuncList(pDocLine, bFlag);
		pDocLine = pDocLineNext;
	}
}

