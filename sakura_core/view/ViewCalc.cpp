#include "StdAfx.h"
#include "ViewCalc.h"
#include "mem/MemoryIterator.h"
#include "view/EditView.h"
#include "doc/EditDoc.h"
#include "doc/layout/Layout.h"

// 外部依存
LayoutInt ViewCalc::GetTabSpace() const
{
	return m_pOwner->m_pEditDoc->m_layoutMgr.GetTabSpace();
}


/* 指定された桁に対応する行のデータ内の位置を調べる Ver1
	
	@@@ 2002.09.28 YAZAKI DocLine版
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


/* 指定された桁に対応する行のデータ内の位置を調べる Ver1
	
	@@@ 2002.09.28 YAZAKI Layoutが必要になりました。
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
  指定された桁に対応する行のデータ内の位置を調べる Ver0
  指定された桁より、行が短い場合はpnLineAllColLenに行全体の表示桁数を返す
  それ以外の場合はpnLineAllColLenに０をセットする
  @@@ 2002.09.28 YAZAKI Layoutが必要になりました。
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
||	指定された行のデータ内の位置に対応する桁の位置を調べる
||
||	@@@ 2002.09.28 YAZAKI Layoutが必要になりました。
*/
LayoutInt ViewCalc::LineIndexToColumn(const Layout* pLayout, LogicInt nIndex) const
{
	//	以下、iterator版
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
||	指定された行のデータ内の位置に対応する桁の位置を調べる
||
||	@@@ 2002.09.28 YAZAKI DocLine版
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


