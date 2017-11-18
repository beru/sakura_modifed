#pragma once

#include "util/design_template.h" // TSingleton
#include "doc/DocListener.h" // DocListenerEx

class DocLine;
class DocLineMgr;

// Modified管理
class ModifyManager :
	public TSingleton<ModifyManager>,
	public DocListenerEx
{
	friend class TSingleton<ModifyManager>;
	ModifyManager() {}

public:
	void OnAfterSave(const SaveInfo& saveInfo);

};

// 行に付加するModified情報
class LineModified {
public:
	LineModified() : nModifiedSeq(0) { }
	int GetSeq() const { return nModifiedSeq; }
	LineModified& operator = (int seq) {
		nModifiedSeq = seq;
		return *this;
	}
private:
	int nModifiedSeq;
};

// 行全体のModified情報アクセサ
class ModifyVisitor {
public:
	// 状態
	bool IsLineModified(const DocLine* pDocLine, int nSaveSeq) const;
	int GetLineModifiedSeq(const DocLine* pDocLine) const;
	void SetLineModified(DocLine* pDocLine, int nModifiedSeq);

	// 一括操作
	void ResetAllModifyFlag(DocLineMgr& docLineMgr, int nSeq);	// 行変更状態をすべてリセット
};

