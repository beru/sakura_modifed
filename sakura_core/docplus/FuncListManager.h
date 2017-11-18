#pragma once

class DocLine;
class DocLineMgr;

// �s�ɕt������Modified���
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

// �s�S�̂�FuncList���A�N�Z�T
class FuncListManager {
public:
	//���
	bool IsLineFuncList(const DocLine* pDocLine, bool bFlag) const;
	bool GetLineFuncList(const DocLine* pDocLine) const;
	void SetLineFuncList(DocLine* pDocLine, bool bFlag);
	bool SearchFuncListMark(const DocLineMgr&, int, SearchDirection, LONG*) const;	// �֐����X�g�}�[�N����

	//�ꊇ����
	void ResetAllFucListMark(DocLineMgr& docLineMgr, bool bFlag);	// �֐����X�g�}�[�N�����ׂă��Z�b�g
};

