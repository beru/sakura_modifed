#include "StdAfx.h"
#include "docplus/ModifyManager.h"
#include "doc/EditDoc.h"
#include "doc/logic/DocLineMgr.h"
#include "doc/logic/DocLine.h"


void ModifyManager::OnAfterSave(const SaveInfo& saveInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	// �s�ύX��Ԃ����ׂă��Z�b�g
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

// �s�ύX��Ԃ����ׂă��Z�b�g
/*
  �E�ύX�t���ODocLine�I�u�W�F�N�g�쐬���ɂ�TRUE�ł���
  �E�ύX�񐔂�DocLine�I�u�W�F�N�g�쐬���ɂ�1�ł���

  �t�@�C����ǂݍ��񂾂Ƃ��͕ύX�t���O�� FALSE�ɂ���
  �t�@�C����ǂݍ��񂾂Ƃ��͕ύX�񐔂� 0�ɂ���

  �t�@�C�����㏑���������͕ύX�t���O�� FALSE�ɂ���
  �t�@�C�����㏑���������͕ύX�񐔂͕ς��Ȃ�

  �ύX�񐔂�Undo�����Ƃ���-1�����
  �ύX�񐔂�0�ɂȂ����ꍇ�͕ύX�t���O��FALSE�ɂ���
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

