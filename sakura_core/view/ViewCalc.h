#pragma once

/*
	X値の単位変換関数群。
*/

class Layout;
class DocLine;
class EditView;

class ViewCalc {
protected:
	// 外部依存
	size_t GetTabSpace() const;

public:
	ViewCalc(const EditView& owner) : owner(owner) { }
	virtual ~ViewCalc() {}

	// 単位変換: レイアウト→ロジック
	size_t LineColumnToIndex (const Layout*  pLayout,  size_t nColumn) const;		// 指定された桁に対応する行のデータ内の位置を調べる Ver1
	size_t LineColumnToIndex (const DocLine* pDocLine, size_t nColumn) const;		// 指定された桁に対応する行のデータ内の位置を調べる Ver1
	size_t LineColumnToIndex2(const Layout*  pLayout,  size_t nColumn, size_t* pnLineAllColLen) const;	// 指定された桁に対応する行のデータ内の位置を調べる Ver0

	// 単位変換: ロジック→レイアウト
	size_t LineIndexToColumn (const Layout*  pLayout,  size_t nIndex) const;		// 指定された行のデータ内の位置に対応する桁の位置を調べる
	size_t LineIndexToColumn (const DocLine* pLayout,  size_t nIndex) const;		// 指定された行のデータ内の位置に対応する桁の位置を調べる

private:
	const EditView& owner;
};

