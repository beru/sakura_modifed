#include "StdAfx.h"
#include "docplus/CModifyManager.h"
#include "doc/CEditDoc.h"
#include "doc/logic/CDocLineMgr.h"
#include "doc/logic/CDocLine.h"


void ModifyManager::OnAfterSave(const SaveInfo& sSaveInfo)
{
	EditDoc* pcDoc = GetListeningDoc();

	// 行変更状態をすべてリセット
	ModifyVisitor().ResetAllModifyFlag(&pcDoc->m_docLineMgr, pcDoc->m_cDocEditor.m_cOpeBuf.GetCurrentPointer());
}

bool ModifyVisitor::IsLineModified(const DocLine* pcDocLine, int saveSeq) const
{
	return pcDocLine->m_sMark.m_cModified.GetSeq() != saveSeq;
}

int ModifyVisitor::GetLineModifiedSeq(const DocLine* pcDocLine) const
{
	return pcDocLine->m_sMark.m_cModified.GetSeq();
}

void ModifyVisitor::SetLineModified(DocLine* pcDocLine, int seq)
{
	pcDocLine->m_sMark.m_cModified = seq;
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
void ModifyVisitor::ResetAllModifyFlag(DocLineMgr* pcDocLineMgr, int seq)
{
	DocLine* pDocLine = pcDocLineMgr->GetDocLineTop();
	while (pDocLine) {
		DocLine* pDocLineNext = pDocLine->GetNextLine();
		SetLineModified(pDocLine, seq);
		pDocLine = pDocLineNext;
	}
}

