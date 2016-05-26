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
		Point			ptLogicPos,		// 実データ参照位置
		size_t			nLength,		// 実データ内データ長
		EColorIndexType	nTypePrev,
		int				nTypeIndent,
		LayoutColorInfo*	pColorInfo
		)
		:
		pPrev(nullptr),
		pNext(nullptr),
		pDocLine(pDocLine),
		ptLogicPos(ptLogicPos),
		nLength(nLength),
		nTypePrev(nTypePrev),
		nIndent(nTypeIndent)
	{
		exInfo.SetColorInfo(pColorInfo);
	}
	~Layout();
	void DUMP(void);
	
	// ptLogicPos.xで補正したあとの文字列を得る
	const wchar_t* GetPtr() const { return pDocLine->GetPtr() + ptLogicPos.x; }
	size_t GetLengthWithEOL() const { return nLength;	}	//	ただしEOLは常に1文字とカウント？？
	size_t GetLengthWithoutEOL() const { return nLength - (eol.GetLen() ? 1 : 0);	}
	//int GetLength() const { return nLength; }	// CMemoryIterator用（EOL含む）
	size_t GetIndent() const { return nIndent; }	// このレイアウト行のインデントサイズを取得。単位は半角文字。	CMemoryIterator用

	// 取得インターフェース
	int GetLogicLineNo() const { assert(this); return ptLogicPos.y; } //$$$高速化
	int GetLogicOffset() const { return ptLogicPos.x; }
	Point GetLogicPos() const { return ptLogicPos; }
	EColorIndexType GetColorTypePrev() const { return nTypePrev; } //#########汚っ
	size_t GetLayoutWidth() const { return nLayoutWidth; }		// 2009.08.28 nasukoji	このレイアウト行の改行を含むレイアウト長を返す

	// 変更インターフェース
	void OffsetLogicLineNo(int n) { ptLogicPos.y += n; }
	void SetColorTypePrev(EColorIndexType n) {
		nTypePrev = n;
	}
	void SetLayoutWidth(size_t nWidth) { nLayoutWidth = nWidth; }

	// レイアウト幅を計算。改行は含まない。2007.10.11 kobake
	size_t CalcLayoutWidth(const LayoutMgr& layoutMgr) const;

	// オフセット値をレイアウト単位に変換して取得。2007.10.17 kobake
	int CalcLayoutOffset(const LayoutMgr& layoutMgr,
							int nStartPos = 0,
							int nStartOffset = 0) const;

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
	const DocLine* GetDocLineRef() const { if (this) return pDocLine; else return nullptr; } //$$note:高速化

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
	const DocLine*		pDocLine;		// 実データへの参照
	Point				ptLogicPos;		// 対応するロジック参照位置
	size_t				nLength;		// このレイアウト行の長さ。文字単位。
	
	// その他属性
	EColorIndexType		nTypePrev;		// タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列
	size_t				nIndent;		// このレイアウト行のインデント数 @@@ 2002.09.23 YAZAKI
	Eol					eol;
	size_t				nLayoutWidth;	// このレイアウト行の改行を含むレイアウト長（「折り返さない」選択時のみ）	// 2009.08.28 nasukoji
	LayoutExInfo		exInfo;			// 色分け詳細情報

private:
	DISALLOW_COPY_AND_ASSIGN(Layout);
};

