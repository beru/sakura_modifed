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
#include "CEol.h"// 2002/2/10 aroka
#include "doc/logic/CDocLine.h"// 2002/4/21 YAZAKI
#include "mem/CMemory.h"// 2002/4/21 YAZAKI
#include "CLayoutExInfo.h"
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
		const CDocLine*	pcDocLine,		//!< 実データへの参照
		LogicPoint		ptLogicPos,		//!< 実データ参照位置
		LogicInt		nLength,		//!< 実データ内データ長
		EColorIndexType	nTypePrev,
		LayoutInt		nTypeIndent,
		LayoutColorInfo*	pColorInfo
		)
	{
		m_pPrev			= NULL;
		m_pNext			= NULL;
		m_pCDocLine		= pcDocLine;
		m_ptLogicPos	= ptLogicPos;	// 実データ参照位置
		m_nLength		= nLength;		// 実データ内データ長
		m_nTypePrev		= nTypePrev;	// タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列
		m_nIndent		= nTypeIndent;	// このレイアウト行のインデント数 @@@ 2002.09.23 YAZAKI
		m_cExInfo.SetColorInfo(pColorInfo);
	}
	~Layout();
	void DUMP(void);
	
	// m_ptLogicPos.xで補正したあとの文字列を得る
	const wchar_t* GetPtr() const { return m_pCDocLine->GetPtr() + m_ptLogicPos.x; }
	LogicInt GetLengthWithEOL() const { return m_nLength;	}	//	ただしEOLは常に1文字とカウント？？
	LogicInt GetLengthWithoutEOL() const { return m_nLength - (m_cEol.GetLen() ? 1 : 0);	}
	//LogicInt GetLength() const { return m_nLength; }	// CMemoryIterator用（EOL含む）
	LayoutInt GetIndent() const { return m_nIndent; }	//!< このレイアウト行のインデントサイズを取得。単位は半角文字。	CMemoryIterator用

	// 取得インターフェース
	LogicInt GetLogicLineNo() const { if (this) return m_ptLogicPos.GetY2(); else return LogicInt(-1); } //$$$高速化
	LogicInt GetLogicOffset() const { return m_ptLogicPos.GetX2(); }
	LogicPoint GetLogicPos() const { return m_ptLogicPos; }
	EColorIndexType GetColorTypePrev() const { return m_nTypePrev; } //#########汚っ
	LayoutInt GetLayoutWidth() const { return m_nLayoutWidth; }		// 2009.08.28 nasukoji	このレイアウト行の改行を含むレイアウト長を返す

	// 変更インターフェース
	void OffsetLogicLineNo(LogicInt n) { m_ptLogicPos.y += n; }
	void SetColorTypePrev(EColorIndexType n) {
		m_nTypePrev = n;
	}
	void SetLayoutWidth(LayoutInt nWidth) { m_nLayoutWidth = nWidth; }

	//! レイアウト幅を計算。改行は含まない。2007.10.11 kobake
	LayoutInt CalcLayoutWidth(const LayoutMgr& cLayoutMgr) const;

	//! オフセット値をレイアウト単位に変換して取得。2007.10.17 kobake
	LayoutInt CalcLayoutOffset(const LayoutMgr& cLayoutMgr,
								LogicInt nStartPos = LogicInt(0),
								LayoutInt nStartOffset = LayoutInt(0)) const;

	//! 文字列参照を取得
	CStringRef GetStringRef() const { return CStringRef(GetPtr(), GetLengthWithEOL()); }

	// チェーン属性
	Layout* GetPrevLayout() { return m_pPrev; }
	const Layout* GetPrevLayout() const { return m_pPrev; }
	Layout* GetNextLayout() { return m_pNext; }
	const Layout* GetNextLayout() const { return m_pNext; }
	void _SetPrevLayout(Layout* pcLayout) { m_pPrev = pcLayout; }
	void _SetNextLayout(Layout* pcLayout) { m_pNext = pcLayout; }

	// 実データ参照
	const CDocLine* GetDocLineRef() const { if (this) return m_pCDocLine; else return NULL; } //$$note:高速化

	// その他属性参照
	const CEol& GetLayoutEol() const { return m_cEol; }
	const LayoutColorInfo* GetColorInfo() const { return m_cExInfo.GetColorInfo(); }
	LayoutExInfo* GetLayoutExInfo() {
		return &m_cExInfo;
	}
	
private:
	Layout*			m_pPrev;
	Layout*			m_pNext;

	// データ参照範囲
	const CDocLine*		m_pCDocLine;		//!< 実データへの参照
	LogicPoint			m_ptLogicPos;		//!< 対応するロジック参照位置
	LogicInt			m_nLength;			//!< このレイアウト行の長さ。文字単位。
	
	// その他属性
	EColorIndexType		m_nTypePrev;		//!< タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列
	LayoutInt			m_nIndent;			//!< このレイアウト行のインデント数 @@@ 2002.09.23 YAZAKI
	CEol				m_cEol;
	LayoutInt			m_nLayoutWidth;		//!< このレイアウト行の改行を含むレイアウト長（「折り返さない」選択時のみ）	// 2009.08.28 nasukoji
	LayoutExInfo		m_cExInfo;			//!< 色分け詳細情報

private:
	DISALLOW_COPY_AND_ASSIGN(Layout);
};

