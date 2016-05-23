#include "StdAfx.h"
#include "ViewCalc.h"
#include "mem/MemoryIterator.h"
#include "view/EditView.h"
#include "doc/EditDoc.h"
#include "doc/layout/Layout.h"

// �O���ˑ�
int ViewCalc::GetTabSpace() const
{
	return owner.pEditDoc->layoutMgr.GetTabSpace();
}


/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1
	
	@@@ 2002.09.28 YAZAKI DocLine��
*/
int ViewCalc::LineColumnToIndex(
	const DocLine* pDocLine,
	int nColumn
	) const
{
	int i2 = 0;
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
int ViewCalc::LineColumnToIndex(
	const Layout* pLayout,
	int nColumn
	) const
{
	int i2 = 0;
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
int ViewCalc::LineColumnToIndex2(
	const Layout* pLayout,
	int nColumn,
	int* pnLineAllColLen
	) const
{
	*pnLineAllColLen = 0;

	int i2 = 0;
	int nPosX2 = 0;
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
int ViewCalc::LineIndexToColumn(
	const Layout* pLayout,
	int nIndex
	) const
{
	//	�ȉ��Aiterator��
	int nPosX2 = 0;
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
int ViewCalc::LineIndexToColumn(
	const DocLine* pDocLine,
	int nIndex
	) const
{
	int nPosX2 = 0;
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


