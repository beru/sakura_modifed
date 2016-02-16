#include "StdAfx.h"
#include "ViewCalc.h"
#include "mem/MemoryIterator.h"
#include "view/EditView.h"
#include "doc/EditDoc.h"
#include "doc/layout/Layout.h"

// �O���ˑ�
LayoutInt ViewCalc::GetTabSpace() const
{
	return m_pOwner->m_pEditDoc->m_layoutMgr.GetTabSpace();
}


/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1
	
	@@@ 2002.09.28 YAZAKI DocLine��
*/
LogicInt ViewCalc::LineColumnToIndex(const DocLine* pDocLine, LayoutInt nColumn) const
{
	LogicInt i2 = LogicInt(0);
	MemoryIterator it(pDocLine, GetTabSpace());
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
LogicInt ViewCalc::LineColumnToIndex(const Layout* pLayout, LayoutInt nColumn) const
{
	LogicInt i2 = LogicInt(0);
	MemoryIterator it(pLayout, GetTabSpace());
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
LogicInt ViewCalc::LineColumnToIndex2(const Layout* pLayout, LayoutInt nColumn, LayoutInt* pnLineAllColLen) const
{
	*pnLineAllColLen = LayoutInt(0);

	LogicInt i2 = LogicInt(0);
	LayoutInt nPosX2 = LayoutInt(0);
	MemoryIterator it(pLayout, GetTabSpace());
	while (!it.end()) {
		it.scanNext();
		if (it.getColumn() + it.getColumnDelta() > nColumn) {
			break;
		}
		it.addDelta();
	}
	i2 += it.getIndex();
	if (i2 >= pLayout->GetLengthWithEOL()) {
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
LayoutInt ViewCalc::LineIndexToColumn(const Layout* pLayout, LogicInt nIndex) const
{
	//	�ȉ��Aiterator��
	LayoutInt nPosX2 = LayoutInt(0);
	MemoryIterator it(pLayout, GetTabSpace());
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
||	@@@ 2002.09.28 YAZAKI DocLine��
*/
LayoutInt ViewCalc::LineIndexToColumn(const DocLine* pDocLine, LogicInt nIndex) const
{
	LayoutInt nPosX2 = LayoutInt(0);
	MemoryIterator it(pDocLine, GetTabSpace());
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


