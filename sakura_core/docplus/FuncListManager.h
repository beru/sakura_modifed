#pragma once

class DocLine;
class DocLineMgr;

// 行に付加するModified情報
class LineFuncList {
public:
	LineFuncList() : bFuncList(false) { }
	bool GetFuncListMark() const { return bFuncList; }
	LineFuncList& operator = (bool bSet)
	{
		bFuncList = bSet;
		return *this;
	}
private:
	bool bFuncList;
};

// 行全体のFuncList情報アクセサ
class FuncListManager {
public:
	//状態
	bool IsLineFuncList(const DocLine* pDocLine, bool bFlag) const;
	bool GetLineFuncList(const DocLine* pDocLine) const;
	void SetLineFuncList(DocLine* pDocLine, bool bFlag);
	bool SearchFuncListMark(const DocLineMgr&, int, SearchDirection, LONG*) const;	// 関数リストマーク検索

	//一括操作
	void ResetAllFucListMark(DocLineMgr& docLineMgr, bool bFlag);	// 関数リストマークをすべてリセット
};

