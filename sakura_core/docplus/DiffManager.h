#pragma once

#include "view/colors/EColorIndexType.h"
#include "util/design_template.h" // TSingleton

class DocLine;
class DocLineMgr;
class Graphics;

// DIFF情報定数
enum class DiffMark {
	None		= 0,	// 無変更
	Append		= 1,	// 追加
	Change		= 2,	// 変更
	Delete		= 3,	// 削除
	DeleteEx	= 4,	// 削除(EOF以降)
};

// DIFF挙動の管理
class DiffManager : public TSingleton<DiffManager> {
	friend class TSingleton<DiffManager>;
	DiffManager() {}

public:
	void SetDiffUse(bool b) { bIsDiffUse = b; }
	bool IsDiffUse() const { return bIsDiffUse; }		// DIFF使用中

private:
	bool bIsDiffUse;		// DIFF差分表示実施中
};

// 行に付加するDIFF情報
class LineDiffed {
public:
	LineDiffed() : nDiffed(DiffMark::None) { }
	operator DiffMark() const { return nDiffed; }
	LineDiffed& operator = (DiffMark e) { nDiffed = e; return *this; }
private:
	DiffMark nDiffed;
};

// 行のDIFF情報取得
class DiffLineGetter {
public:
	DiffLineGetter(const DocLine* pDocLine) : pDocLine(pDocLine) { }
	DiffMark GetLineDiffMark() const;
	bool GetDiffColor(EColorIndexType* nColor) const;
	bool DrawDiffMark(Graphics& gr, int y, unsigned int nLineHeight, COLORREF color) const;
private:
	const DocLine* pDocLine;
};

// 行のDIFF情報設定
class DiffLineSetter {
public:
	DiffLineSetter(DocLine* pDocLine) : pDocLine(pDocLine) { }
	void SetLineDiffMark(DiffMark mark);
private:
	DocLine* pDocLine;
};

// 行全体のDIFF情報管理
class DiffLineMgr {
public:
	DiffLineMgr(DocLineMgr& docLineMgr) : docLineMgr(docLineMgr) { }
	void ResetAllDiffMark();															// 差分表示の全解除
	bool SearchDiffMark(size_t , SearchDirection, size_t*);						// 差分検索
	void SetDiffMarkRange(DiffMark nMode, size_t nStartLine, size_t nEndLine);	// 差分範囲の登録
private:
	DocLineMgr& docLineMgr;
};

