#pragma once

#include "_main/global.h" // SearchDirection, SearchOption

class DocLine;
class DocLineMgr;
class Bregexp;

#include "SearchAgent.h"

// �s�ɕt������u�b�N�}�[�N���
class LineBookmarked {
public:
	LineBookmarked() : bBookmarked(false) { }
	operator bool() const { return bBookmarked; }
	LineBookmarked& operator = (bool b) { bBookmarked = b; return *this; }
private:
	bool bBookmarked;
};

// �s�̃u�b�N�}�[�N���̎擾
class BookmarkGetter {
public:
	BookmarkGetter(const DocLine* pDocLine) : pDocLine(pDocLine) { }
	bool IsBookmarked() const;
private:
	const DocLine* pDocLine;
};

// �s�̃u�b�N�}�[�N���̎擾�E�ݒ�
class BookmarkSetter : public BookmarkGetter {
public:
	BookmarkSetter(DocLine* pDocLine) : BookmarkGetter(pDocLine), pDocLine(pDocLine) { }
	void SetBookmark(bool bFlag);
private:
	DocLine* pDocLine;
};

// �s�S�̂̃u�b�N�}�[�N���̊Ǘ�
class BookmarkManager {
public:
	BookmarkManager(DocLineMgr& docLineMgr) : docLineMgr(docLineMgr) { }

	void ResetAllBookMark();												// �u�b�N�}�[�N�̑S����
	bool SearchBookMark(int nLineNum, SearchDirection, int* pnLineNum);		// �u�b�N�}�[�N����
	void SetBookMarks(wchar_t*);											// �����s�ԍ��̃��X�g����܂Ƃ߂čs�}�[�N
	LPCWSTR GetBookMarks();													// �s�}�[�N����Ă镨���s�ԍ��̃��X�g�����
	void MarkSearchWord(const SearchStringPattern&);			// ���������ɊY������s�Ƀu�b�N�}�[�N���Z�b�g����

private:
	DocLineMgr& docLineMgr;
};

