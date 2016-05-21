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
#include "Eol.h"
#include "mem/Memory.h"

#include "docplus/BookmarkManager.h"
#include "docplus/DiffManager.h"
#include "docplus/ModifyManager.h"
#include "docplus/FuncListManager.h"

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
	LogicInt		GetLengthWithoutEOL() const			{ return line.GetStringLength() - eol.GetLen(); } // 戻り値は文字単位。
	const wchar_t*	GetPtr() const						{ return line.GetStringPtr(); }
	size_t			GetLengthWithEOL() const			{ return line.GetStringLength(); }	// CMemoryIterator用
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
	const Eol& GetEol() const { return eol; }
	void SetEol(const Eol& eol, OpeBlk* pOpeBlk);
	void SetEol(); // 現在のバッファから設定

	const NativeW& _GetDocLineDataWithEOL() const { return line; } //###仮
	NativeW& _GetDocLineData() { return line; }

	// データ設定
	void SetDocLineString(const wchar_t* pData, int nLength);
	void SetDocLineString(const NativeW& data);
	void SetDocLineStringMove(NativeW* pData);

	// チェーン属性
	DocLine* GetPrevLine() { return pPrev; }
	const DocLine* GetPrevLine() const { return pPrev; }
	DocLine* GetNextLine() { return pNext; }
	const DocLine* GetNextLine() const { return pNext; }
	void _SetPrevLine(DocLine* pDocLine) { pPrev = pDocLine; }
	void _SetNextLine(DocLine* pDocLine) { pNext = pDocLine; }
	
private: //####
	DocLine*	pPrev;	// 一つ前の要素
	DocLine*	pNext;	// 一つ後の要素
private:
	NativeW	line;	// データ  2007.10.11 kobake ポインタではなく、実体を持つように変更
	Eol		eol;		// 行末コード
public:
	// 拡張情報 $$分離中
	struct MarkType {
		LineModified	modified;	// 変更フラグ
		LineBookmarked	bookmarked;	// ブックマーク
		LineFuncList	funcList;	// 関数リストマーク
		LineDiffed		diffMarked;	// DIFF差分情報
	};
	MarkType mark;

private:
	DISALLOW_COPY_AND_ASSIGN(DocLine);
};

