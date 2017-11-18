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

/*! ��������
	@author	MIK
	@date	2002.05.25
*/
bool FuncListManager::SearchFuncListMark(
	const DocLineMgr&	docLineMgr,
	int					nLineNum,		// �����J�n�s
	SearchDirection		bPrevOrNext,
	LONG*				pnLineNum 		// �}�b�`�s
	) const
{
	int nLinePos = nLineNum;

	if (bPrevOrNext == SearchDirection::Backward) {
		// �������(��)
		--nLinePos;
		const DocLine* pDocLine = docLineMgr.GetLine( nLinePos );
		while (pDocLine) {
			if (GetLineFuncList(pDocLine)) {
				*pnLineNum = nLinePos;				// �}�b�`�s
				return true;
			}
			--nLinePos;
			pDocLine = pDocLine->GetPrevLine();
		}
	}else {
		// �O������(��)
		++nLinePos;
		const DocLine* pDocLine = docLineMgr.GetLine( nLinePos );
		while (pDocLine) {
			if (GetLineFuncList(pDocLine)) {
				*pnLineNum = nLinePos;				// �}�b�`�s
				return true;
			}
			++nLinePos;
			pDocLine = pDocLine->GetNextLine();
		}
	}
	return false;
}

// �֐����X�g�}�[�N�����ׂă��Z�b�g
void FuncListManager::ResetAllFucListMark(DocLineMgr& docLineMgr, bool bFlag)
{
	DocLine* pDocLine = docLineMgr.GetDocLineTop();
	while (pDocLine) {
		DocLine* pDocLineNext = pDocLine->GetNextLine();
		SetLineFuncList(pDocLine, bFlag);
		pDocLine = pDocLineNext;
	}
}

