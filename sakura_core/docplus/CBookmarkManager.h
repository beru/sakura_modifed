/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#pragma once

#include "_main/global.h" // SearchDirection, SearchOption

class DocLine;
class DocLineMgr;
class Bregexp;

#include "CSearchAgent.h"

// �s�ɕt������u�b�N�}�[�N���
class LineBookmarked {
public:
	LineBookmarked() : m_bBookmarked(false) { }
	operator bool() const { return m_bBookmarked; }
	LineBookmarked& operator = (bool b) { m_bBookmarked = b; return *this; }
private:
	bool m_bBookmarked;
};

// �s�̃u�b�N�}�[�N���̎擾
class BookmarkGetter {
public:
	BookmarkGetter(const DocLine* pDocLine) : m_pDocLine(pDocLine) { }
	bool IsBookmarked() const;
private:
	const DocLine* m_pDocLine;
};

// �s�̃u�b�N�}�[�N���̎擾�E�ݒ�
class BookmarkSetter : public BookmarkGetter {
public:
	BookmarkSetter(DocLine* pDocLine) : BookmarkGetter(pDocLine), m_pDocLine(pDocLine) { }
	void SetBookmark(bool bFlag);
private:
	DocLine* m_pDocLine;
};

// �s�S�̂̃u�b�N�}�[�N���̊Ǘ�
class BookmarkManager {
public:
	BookmarkManager(DocLineMgr* pDocLineMgr) : m_pDocLineMgr(pDocLineMgr) { }

	void ResetAllBookMark();															// �u�b�N�}�[�N�̑S����
	bool SearchBookMark(LogicInt nLineNum, SearchDirection, LogicInt* pnLineNum);	// �u�b�N�}�[�N����
	void SetBookMarks(wchar_t*);														// �����s�ԍ��̃��X�g����܂Ƃ߂čs�}�[�N
	LPCWSTR GetBookMarks();																// �s�}�[�N����Ă镨���s�ԍ��̃��X�g�����
	void MarkSearchWord(const SearchStringPattern&);			// ���������ɊY������s�Ƀu�b�N�}�[�N���Z�b�g����

private:
	DocLineMgr* m_pDocLineMgr;
};

