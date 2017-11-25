#include "StdAfx.h"
#include "ViewCalc.h"
#include "mem/MemoryIterator.h"
#include "view/EditView.h"
#include "doc/EditDoc.h"
#include "doc/layout/Layout.h"

// �O���ˑ�
size_t ViewCalc::GetTabSpace() const
{
	return owner.pEditDoc->layoutMgr.GetTabSpace();
}


/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1 */
size_t ViewCalc::LineColumnToIndex(
	const DocLine* pDocLine,
	size_t nColumn
	) const
{
	size_t i2 = 0;
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


/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1 */
size_t ViewCalc::LineColumnToIndex(
	const Layout* pLayout,
	size_t nColumn
	) const
{
	size_t i2 = 0;
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
*/
size_t ViewCalc::LineColumnToIndex2(
	const Layout* pLayout,
	size_t nColumn,
	size_t* pnLineAllColLen
	) const
{
	*pnLineAllColLen = 0;

	size_t i2 = 0;
	size_t nPosX2 = 0;
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
*/
size_t ViewCalc::LineIndexToColumn(
	const Layout* pLayout,
	size_t nIndex
	) const
{
	//	�ȉ��Aiterator��
	size_t nPosX2 = 0;
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
*/
size_t ViewCalc::LineIndexToColumn(
	const DocLine* pDocLine,
	size_t nIndex
	) const
{
	size_t nPosX2 = 0;
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


