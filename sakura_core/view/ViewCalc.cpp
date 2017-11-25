#include "StdAfx.h"
#include "ViewCalc.h"
#include "mem/MemoryIterator.h"
#include "view/EditView.h"
#include "doc/EditDoc.h"
#include "doc/layout/Layout.h"

// 外部依存
size_t ViewCalc::GetTabSpace() const
{
	return owner.pEditDoc->layoutMgr.GetTabSpace();
}


/* 指定された桁に対応する行のデータ内の位置を調べる Ver1 */
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


/* 指定された桁に対応する行のデータ内の位置を調べる Ver1 */
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
  指定された桁に対応する行のデータ内の位置を調べる Ver0
  指定された桁より、行が短い場合はpnLineAllColLenに行全体の表示桁数を返す
  それ以外の場合はpnLineAllColLenに０をセットする
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
||	指定された行のデータ内の位置に対応する桁の位置を調べる
*/
size_t ViewCalc::LineIndexToColumn(
	const Layout* pLayout,
	size_t nIndex
	) const
{
	//	以下、iterator版
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
||	指定された行のデータ内の位置に対応する桁の位置を調べる
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


