#include "StdAfx.h"
#include "docplus/FuncListManager.h"
#include "doc/logic/DocLineMgr.h"
#include "doc/logic/DocLine.h"


bool FuncListManager::IsLineFuncList(const DocLine* pDocLine, bool bFlag) const
{
	return pDocLine->mark.funcList.GetFuncListMark() != bFlag;
}
bool FuncListManager::GetLineFuncList(const DocLine* pDocLine) const
{
	return pDocLine->mark.funcList.GetFuncListMark();
}
void FuncListManager::SetLineFuncList(DocLine* pDocLine, bool bFlag)
{
	pDocLine->mark.funcList = bFlag;
}

/*! 差分検索
	@author	MIK
	@date	2002.05.25
*/
bool FuncListManager::SearchFuncListMark(
	const DocLineMgr&	docLineMgr,
	int					nLineNum,		// 検索開始行
	SearchDirection		bPrevOrNext,
	LONG*				pnLineNum 		// マッチ行
	) const
{
	int nLinePos = nLineNum;

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

