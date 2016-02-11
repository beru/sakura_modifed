#include "StdAfx.h"
#include "docplus/CModifyManager.h"
#include "doc/CEditDoc.h"
#include "doc/logic/CDocLineMgr.h"
#include "doc/logic/CDocLine.h"


void ModifyManager::OnAfterSave(const SaveInfo& sSaveInfo)
{
	EditDoc* pcDoc = GetListeningDoc();

	// �s�ύX��Ԃ����ׂă��Z�b�g
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
void ModifyVisitor::ResetAllModifyFlag(DocLineMgr* pcDocLineMgr, int seq)
{
	DocLine* pDocLine = pcDocLineMgr->GetDocLineTop();
	while (pDocLine) {
		DocLine* pDocLineNext = pDocLine->GetNextLine();
		SetLineModified(pDocLine, seq);
		pDocLine = pDocLineNext;
	}
}

