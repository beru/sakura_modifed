#include "StdAfx.h"
#include "docplus/ModifyManager.h"
#include "doc/EditDoc.h"
#include "doc/logic/DocLineMgr.h"
#include "doc/logic/DocLine.h"


void ModifyManager::OnAfterSave(const SaveInfo& saveInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	// 行変更状態をすべてリセット
	ModifyVisitor().ResetAllModifyFlag(&pDoc->m_docLineMgr, pDoc->m_docEditor.m_opeBuf.GetCurrentPointer());
}

bool ModifyVisitor::IsLineModified(const DocLine* pDocLine, int saveSeq) const
{
	return pDocLine->m_mark.m_modified.GetSeq() != saveSeq;
}

int ModifyVisitor::GetLineModifiedSeq(const DocLine* pDocLine) const
{
	return pDocLine->m_mark.m_modified.GetSeq();
}

void ModifyVisitor::SetLineModified(DocLine* pDocLine, int seq)
{
	pDocLine->m_mark.m_modified = seq;
}

// 行変更状態をすべてリセット
/*
  ・変更フラグDocLineオブジェクト作成時にはTRUEである
  ・変更回数はDocLineオブジェクト作成時には1である

  ファイルを読み込んだときは変更フラグを FALSEにする
  ファイルを読み込んだときは変更回数を 0にする

  ファイルを上書きした時は変更フラグを FALSEにする
  ファイルを上書きした時は変更回数は変えない

  変更回数はUndoしたときに-1される
  変更回数が0になった場合は変更フラグをFALSEにする
*/
void ModifyVisitor::ResetAllModifyFlag(DocLineMgr* pDocLineMgr, int seq)
{
	DocLine* pDocLine = pDocLineMgr->GetDocLineTop();
	while (pDocLine) {
		DocLine* pDocLineNext = pDocLine->GetNextLine();
		SetLineModified(pDocLine, seq);
		pDocLine = pDocLineNext;
	}
}

