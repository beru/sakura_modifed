//@@@ 2002.05.25 MIK
// 2008.02.23 kobake 大整理
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

// DIFF情報定数
enum EDiffMark{
	MARK_DIFF_NONE		= 0,	// 無変更
	MARK_DIFF_APPEND	= 1,	// 追加
	MARK_DIFF_CHANGE	= 2,	// 変更
	MARK_DIFF_DELETE	= 3,	// 削除
	MARK_DIFF_DEL_EX	= 4,	// 削除(EOF以降)
};

// DIFF挙動の管理
class DiffManager : public TSingleton<DiffManager> {
	friend class TSingleton<DiffManager>;
	DiffManager() {}

public:
	void SetDiffUse(bool b) { m_bIsDiffUse = b; }
	bool IsDiffUse() const { return m_bIsDiffUse; }		// DIFF使用中

private:
	bool	m_bIsDiffUse;		// DIFF差分表示実施中 @@@ 2002.05.25 MIK
};

// 行に付加するDIFF情報
class LineDiffed {
public:
	LineDiffed() : m_nDiffed(MARK_DIFF_NONE) { }
	operator EDiffMark() const { return m_nDiffed; }
	LineDiffed& operator = (EDiffMark e) { m_nDiffed = e; return *this; }
private:
	EDiffMark m_nDiffed;
};

// 行のDIFF情報取得
class DiffLineGetter {
public:
	DiffLineGetter(const DocLine* pcDocLine) : m_pcDocLine(pcDocLine) { }
	EDiffMark GetLineDiffMark() const;
	bool GetDiffColor(EColorIndexType* nColor) const;
	bool DrawDiffMark(Graphics& gr, int y, int nLineHeight, COLORREF color) const;
private:
	const DocLine* m_pcDocLine;
};

// 行のDIFF情報設定
class DiffLineSetter {
public:
	DiffLineSetter(DocLine* pcDocLine) : m_pcDocLine(pcDocLine) { }
	void SetLineDiffMark(EDiffMark mark);
private:
	DocLine* m_pcDocLine;
};

// 行全体のDIFF情報管理
class DiffLineMgr {
public:
	DiffLineMgr(DocLineMgr* pcDocLineMgr) : m_pcDocLineMgr(pcDocLineMgr) { }
	void ResetAllDiffMark();															// 差分表示の全解除
	bool SearchDiffMark(LogicInt , eSearchDirection, LogicInt*);						// 差分検索
	void SetDiffMarkRange(EDiffMark nMode, LogicInt nStartLine, LogicInt nEndLine);	// 差分範囲の登録
private:
	DocLineMgr* m_pcDocLineMgr;
};

