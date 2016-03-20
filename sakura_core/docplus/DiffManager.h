//@@@ 2002.05.25 MIK
// 2008.02.23 kobake �吮��
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

#include "view/colors/EColorIndexType.h"
#include "util/design_template.h" // TSingleton

class DocLine;
class DocLineMgr;
class Graphics;

// DIFF���萔
enum class DiffMark {
	None		= 0,	// ���ύX
	Append		= 1,	// �ǉ�
	Change		= 2,	// �ύX
	Delete		= 3,	// �폜
	DeleteEx	= 4,	// �폜(EOF�ȍ~)
};

// DIFF�����̊Ǘ�
class DiffManager : public TSingleton<DiffManager> {
	friend class TSingleton<DiffManager>;
	DiffManager() {}

public:
	void SetDiffUse(bool b) { m_bIsDiffUse = b; }
	bool IsDiffUse() const { return m_bIsDiffUse; }		// DIFF�g�p��

private:
	bool	m_bIsDiffUse;		// DIFF�����\�����{�� @@@ 2002.05.25 MIK
};

// �s�ɕt������DIFF���
class LineDiffed {
public:
	LineDiffed() : m_nDiffed(DiffMark::None) { }
	operator DiffMark() const { return m_nDiffed; }
	LineDiffed& operator = (DiffMark e) { m_nDiffed = e; return *this; }
private:
	DiffMark m_nDiffed;
};

// �s��DIFF���擾
class DiffLineGetter {
public:
	DiffLineGetter(const DocLine* pDocLine) : m_pDocLine(pDocLine) { }
	DiffMark GetLineDiffMark() const;
	bool GetDiffColor(EColorIndexType* nColor) const;
	bool DrawDiffMark(Graphics& gr, int y, int nLineHeight, COLORREF color) const;
private:
	const DocLine* m_pDocLine;
};

// �s��DIFF���ݒ�
class DiffLineSetter {
public:
	DiffLineSetter(DocLine* pDocLine) : m_pDocLine(pDocLine) { }
	void SetLineDiffMark(DiffMark mark);
private:
	DocLine* m_pDocLine;
};

// �s�S�̂�DIFF���Ǘ�
class DiffLineMgr {
public:
	DiffLineMgr(DocLineMgr& docLineMgr) : m_docLineMgr(docLineMgr) { }
	void ResetAllDiffMark();															// �����\���̑S����
	bool SearchDiffMark(LogicInt , SearchDirection, LogicInt*);						// ��������
	void SetDiffMarkRange(DiffMark nMode, LogicInt nStartLine, LogicInt nEndLine);	// �����͈͂̓o�^
private:
	DocLineMgr& m_docLineMgr;
};

