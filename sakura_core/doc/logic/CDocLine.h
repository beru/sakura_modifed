/*!	@file
	@brief 文書データ1行

	@author Norio Nakatani

	@date 2001/12/03 hor しおり(bookmark)機能追加に伴うメンバー追加
	@date 2001/12/18 hor bookmark, 修正フラグのアクセス関数化
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#pragma once

#include "util/design_template.h"
#include "CEol.h"
#include "mem/CMemory.h"

#include "docplus/CBookmarkManager.h"
#include "docplus/CDiffManager.h"
#include "docplus/CModifyManager.h"
#include "docplus/CFuncListManager.h"

class DocLine;
class OpeBlk;

// 文書データ1行
class DocLine {
protected:
	friend class DocLineMgr; //######仮
public:
	// コンストラクタ・デストラクタ
	DocLine();
	~DocLine();

	// 判定
	bool			IsEmptyLine() const;		// このDocLineが空行（スペース、タブ、改行記号のみの行）かどうか。

	// データ取得
	LogicInt		GetLengthWithoutEOL() const			{ return m_line.GetStringLength() - m_eol.GetLen(); } // 戻り値は文字単位。
	const wchar_t*	GetPtr() const						{ return m_line.GetStringPtr(); }
	LogicInt		GetLengthWithEOL() const			{ return m_line.GetStringLength(); }	// CMemoryIterator用
#ifdef USE_STRICT_INT
	const wchar_t*	GetDocLineStrWithEOL(int* pnLen) const {	//###仮の名前、仮の対処
		LogicInt n;
		const wchar_t* p = GetDocLineStrWithEOL(&n);
		*pnLen = n;
		return p;
	}
#endif
	const wchar_t*	GetDocLineStrWithEOL(LogicInt* pnLen) const {	//###仮の名前、仮の対処
		if (this) {
			*pnLen = GetLengthWithEOL();
			return GetPtr();
		}else {
			*pnLen = 0;
			return NULL;
		}
	}
	StringRef GetStringRefWithEOL() const {	//###仮の名前、仮の対処
		if (this) {
			return StringRef(GetPtr(), GetLengthWithEOL());
		}else {
			return StringRef(NULL, 0);
		}
	}
	const Eol& GetEol() const { return m_eol; }
	void SetEol(const Eol& cEol, OpeBlk* pcOpeBlk);
	void SetEol(); // 現在のバッファから設定

	const NativeW& _GetDocLineDataWithEOL() const { return m_line; } //###仮
	NativeW& _GetDocLineData() { return m_line; }

	// データ設定
	void SetDocLineString(const wchar_t* pData, int nLength);
	void SetDocLineString(const NativeW& cData);
	void SetDocLineStringMove(NativeW* pcData);

	// チェーン属性
	DocLine* GetPrevLine() { return m_pPrev; }
	const DocLine* GetPrevLine() const { return m_pPrev; }
	DocLine* GetNextLine() { return m_pNext; }
	const DocLine* GetNextLine() const { return m_pNext; }
	void _SetPrevLine(DocLine* pDocLine) { m_pPrev = pDocLine; }
	void _SetNextLine(DocLine* pDocLine) { m_pNext = pDocLine; }
	
private: //####
	DocLine*	m_pPrev;	// 一つ前の要素
	DocLine*	m_pNext;	// 一つ後の要素
private:
	NativeW	m_line;	// データ  2007.10.11 kobake ポインタではなく、実体を持つように変更
	Eol		m_eol;		// 行末コード
public:
	// 拡張情報 $$分離中
	struct MarkType {
		LineModified	m_modified;		// 変更フラグ
		LineBookmarked	m_bookmarked;	// ブックマーク
		LineFuncList	m_funcList;		// 関数リストマーク
		LineDiffed		m_diffMarked;	// DIFF差分情報
	};
	MarkType m_mark;

private:
	DISALLOW_COPY_AND_ASSIGN(DocLine);
};

