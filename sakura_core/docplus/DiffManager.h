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
	void SetDiffUse(bool b) { bIsDiffUse = b; }
	bool IsDiffUse() const { return bIsDiffUse; }		// DIFF�g�p��

private:
	bool bIsDiffUse;		// DIFF�����\�����{��
};

// �s�ɕt������DIFF���
class LineDiffed {
public:
	LineDiffed() : nDiffed(DiffMark::None) { }
	operator DiffMark() const { return nDiffed; }
	LineDiffed& operator = (DiffMark e) { nDiffed = e; return *this; }
private:
	DiffMark nDiffed;
};

// �s��DIFF���擾
class DiffLineGetter {
public:
	DiffLineGetter(const DocLine* pDocLine) : pDocLine(pDocLine) { }
	DiffMark GetLineDiffMark() const;
	bool GetDiffColor(EColorIndexType* nColor) const;
	bool DrawDiffMark(Graphics& gr, int y, unsigned int nLineHeight, COLORREF color) const;
private:
	const DocLine* pDocLine;
};

// �s��DIFF���ݒ�
class DiffLineSetter {
public:
	DiffLineSetter(DocLine* pDocLine) : pDocLine(pDocLine) { }
	void SetLineDiffMark(DiffMark mark);
private:
	DocLine* pDocLine;
};

// �s�S�̂�DIFF���Ǘ�
class DiffLineMgr {
public:
	DiffLineMgr(DocLineMgr& docLineMgr) : docLineMgr(docLineMgr) { }
	void ResetAllDiffMark();															// �����\���̑S����
	bool SearchDiffMark(size_t , SearchDirection, size_t*);						// ��������
	void SetDiffMarkRange(DiffMark nMode, size_t nStartLine, size_t nEndLine);	// �����͈͂̓o�^
private:
	DocLineMgr& docLineMgr;
};

