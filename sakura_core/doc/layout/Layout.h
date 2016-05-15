/*!	@file
	@brief テキストのレイアウト情報

	@author Norio Nakatani
	@date 1998/3/11 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI, aroka
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include "util/design_template.h"
#include "Eol.h"// 2002/2/10 aroka
#include "doc/logic/DocLine.h"// 2002/4/21 YAZAKI
#include "mem/Memory.h"// 2002/4/21 YAZAKI
#include "LayoutExInfo.h"
#include "view/colors/EColorIndexType.h"

class Layout;
class LayoutMgr;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class Layout {
protected:
	friend class LayoutMgr; //####仮
public:
	/*
	||  Constructors
	*/
	// 2007.08.23 kobake コンストラクタでメンバ変数を初期化するようにした
	Layout(
		const DocLine*	pDocLine,		// 実データへの参照
		LogicPoint		ptLogicPos,		// 実データ参照位置
		LogicInt		nLength,		// 実データ内データ長
		EColorIndexType	nTypePrev,
		LayoutInt		nTypeIndent,
		LayoutColorInfo*	pColorInfo
		)
	{
		this->pPrev			= nullptr;
		this->pNext			= nullptr;
		this->pDocLine		= pDocLine;
		this->ptLogicPos	= ptLogicPos;	// 実データ参照位置
		this->nLength		= nLength;		// 実データ内データ長
		this->nTypePrev		= nTypePrev;	// タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列
		nIndent		= nTypeIndent;	// このレイアウト行のインデント数 @@@ 2002.09.23 YAZAKI
		exInfo.SetColorInfo(pColorInfo);
	}
	~Layout();
	void DUMP(void);
	
	// ptLogicPos.xで補正したあとの文字列を得る
	const wchar_t* GetPtr() const { return pDocLine->GetPtr() + ptLogicPos.x; }
	LogicInt GetLengthWithEOL() const { return nLength;	}	//	ただしEOLは常に1文字とカウント？？
	LogicInt GetLengthWithoutEOL() const { return nLength - (eol.GetLen() ? 1 : 0);	}
	//LogicInt GetLength() const { return nLength; }	// CMemoryIterator用（EOL含む）
	LayoutInt GetIndent() const { return nIndent; }	// このレイアウト行のインデントサイズを取得。単位は半角文字。	CMemoryIterator用

	// 取得インターフェース
	LogicInt GetLogicLineNo() const { if (this) return ptLogicPos.GetY2(); else return LogicInt(-1); } //$$$高速化
	LogicInt GetLogicOffset() const { return ptLogicPos.GetX2(); }
	LogicPoint GetLogicPos() const { return ptLogicPos; }
	EColorIndexType GetColorTypePrev() const { return nTypePrev; } //#########汚っ
	LayoutInt GetLayoutWidth() const { return nLayoutWidth; }		// 2009.08.28 nasukoji	このレイアウト行の改行を含むレイアウト長を返す

	// 変更インターフェース
	void OffsetLogicLineNo(LogicInt n) { ptLogicPos.y += n; }
	void SetColorTypePrev(EColorIndexType n) {
		nTypePrev = n;
	}
	void SetLayoutWidth(LayoutInt nWidth) { nLayoutWidth = nWidth; }

	// レイアウト幅を計算。改行は含まない。2007.10.11 kobake
	LayoutInt CalcLayoutWidth(const LayoutMgr& layoutMgr) const;

	// オフセット値をレイアウト単位に変換して取得。2007.10.17 kobake
	LayoutInt CalcLayoutOffset(const LayoutMgr& layoutMgr,
								LogicInt nStartPos = LogicInt(0),
								LayoutInt nStartOffset = LayoutInt(0)) const;

	// 文字列参照を取得
	StringRef GetStringRef() const { return StringRef(GetPtr(), GetLengthWithEOL()); }

	// チェーン属性
	Layout* GetPrevLayout() { return pPrev; }
	const Layout* GetPrevLayout() const { return pPrev; }
	Layout* GetNextLayout() { return pNext; }
	const Layout* GetNextLayout() const { return pNext; }
	void _SetPrevLayout(Layout* pLayout) { pPrev = pLayout; }
	void _SetNextLayout(Layout* pLayout) { pNext = pLayout; }

	// 実データ参照
	const DocLine* GetDocLineRef() const { if (this) return pDocLine; else return NULL; } //$$note:高速化

	// その他属性参照
	const Eol& GetLayoutEol() const { return eol; }
	const LayoutColorInfo* GetColorInfo() const { return exInfo.GetColorInfo(); }
	LayoutExInfo* GetLayoutExInfo() {
		return &exInfo;
	}
	
private:
	Layout*			pPrev;
	Layout*			pNext;

	// データ参照範囲
	const DocLine*		pDocLine;			// 実データへの参照
	LogicPoint			ptLogicPos;		// 対応するロジック参照位置
	LogicInt			nLength;			// このレイアウト行の長さ。文字単位。
	
	// その他属性
	EColorIndexType		nTypePrev;		// タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列
	LayoutInt			nIndent;			// このレイアウト行のインデント数 @@@ 2002.09.23 YAZAKI
	Eol					eol;
	LayoutInt			nLayoutWidth;		// このレイアウト行の改行を含むレイアウト長（「折り返さない」選択時のみ）	// 2009.08.28 nasukoji
	LayoutExInfo		exInfo;			// 色分け詳細情報

private:
	DISALLOW_COPY_AND_ASSIGN(Layout);
};

