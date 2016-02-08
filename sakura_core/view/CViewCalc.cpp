#include "StdAfx.h"
#include "CViewCalc.h"
#include "mem/CMemoryIterator.h"
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "doc/layout/CLayout.h"

// �O���ˑ�
LayoutInt CViewCalc::GetTabSpace() const
{
	return m_pOwner->m_pcEditDoc->m_cLayoutMgr.GetTabSpace();
}


/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1
	
	@@@ 2002.09.28 YAZAKI CDocLine��
*/
LogicInt CViewCalc::LineColumnToIndex(const CDocLine* pcDocLine, LayoutInt nColumn) const
{
	LogicInt i2 = LogicInt(0);
	CMemoryIterator it(pcDocLine, GetTabSpace());
	while (!it.end()) {
		it.scanNext();
		if (it.getColumn() + it.getColumnDelta() > nColumn) {
			break;
		}
		it.addDelta();
	}
	i2 += it.getIndex();
	return i2;
}


/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1
	
	@@@ 2002.09.28 YAZAKI Layout���K�v�ɂȂ�܂����B
*/
LogicInt CViewCalc::LineColumnToIndex(const Layout* pcLayout, LayoutInt nColumn) const
{
	LogicInt i2 = LogicInt(0);
	CMemoryIterator it(pcLayout, GetTabSpace());
	while (!it.end()) {
		it.scanNext();
		if (it.getColumn() + it.getColumnDelta() > nColumn) {
			break;
		}
		it.addDelta();
	}
	i2 += it.getIndex();
	return i2;
}



/*
  �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver0
  �w�肳�ꂽ�����A�s���Z���ꍇ��pnLineAllColLen�ɍs�S�̂̕\��������Ԃ�
  ����ȊO�̏ꍇ��pnLineAllColLen�ɂO���Z�b�g����
  @@@ 2002.09.28 YAZAKI Layout���K�v�ɂȂ�܂����B
*/
LogicInt CViewCalc::LineColumnToIndex2(const Layout* pcLayout, LayoutInt nColumn, LayoutInt* pnLineAllColLen) const
{
	*pnLineAllColLen = LayoutInt(0);

	LogicInt i2 = LogicInt(0);
	LayoutInt nPosX2 = LayoutInt(0);
	CMemoryIterator it(pcLayout, GetTabSpace());
	while (!it.end()) {
		it.scanNext();
		if (it.getColumn() + it.getColumnDelta() > nColumn) {
			break;
		}
		it.addDelta();
	}
	i2 += it.getIndex();
	if (i2 >= pcLayout->GetLengthWithEOL()) {
		nPosX2 += it.getColumn();
		*pnLineAllColLen = nPosX2;
	}
	return i2;
}


/*
||	�w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
||
||	@@@ 2002.09.28 YAZAKI Layout���K�v�ɂȂ�܂����B
*/
LayoutInt CViewCalc::LineIndexToColumn(const Layout* pcLayout, LogicInt nIndex) const
{
	//	�ȉ��Aiterator��
	LayoutInt nPosX2 = LayoutInt(0);
	CMemoryIterator it(pcLayout, GetTabSpace());
	while (!it.end()) {
		it.scanNext();
		if (it.getIndex() + it.getIndexDelta() > nIndex) {
			break;
		}
		it.addDelta();
	}
	nPosX2 += it.getColumn();
	return nPosX2;
}


/*
||	�w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
||
||	@@@ 2002.09.28 YAZAKI CDocLine��
*/
LayoutInt CViewCalc::LineIndexToColumn(const CDocLine* pcDocLine, LogicInt nIndex) const
{
	LayoutInt nPosX2 = LayoutInt(0);
	CMemoryIterator it(pcDocLine, GetTabSpace());
	while (!it.end()) {
		it.scanNext();
		if (it.getIndex() + it.getIndexDelta() > nIndex) {
			break;
		}
		it.addDelta();
	}
	nPosX2 += it.getColumn();
	return nPosX2;
}


