#pragma once

#include "util/design_template.h" // TSingleton
#include "doc/DocListener.h" // DocListenerEx

class DocLine;
class DocLineMgr;

// Modified�Ǘ�
class ModifyManager :
	public TSingleton<ModifyManager>,
	public DocListenerEx
{
	friend class TSingleton<ModifyManager>;
	ModifyManager() {}

public:
	void OnAfterSave(const SaveInfo& saveInfo);

};

// �s�ɕt������Modified���
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

// �s�S�̂�Modified���A�N�Z�T
class ModifyVisitor {
public:
	// ���
	bool IsLineModified(const DocLine* pDocLine, int nSaveSeq) const;
	int GetLineModifiedSeq(const DocLine* pDocLine) const;
	void SetLineModified(DocLine* pDocLine, int nModifiedSeq);

	// �ꊇ����
	void ResetAllModifyFlag(DocLineMgr& docLineMgr, int nSeq);	// �s�ύX��Ԃ����ׂă��Z�b�g
};

