/*!	@file
	@brief LayoutとCDocLineのイテレータ

	@author Yazaki
	@date 2002/09/25 新規作成
*/
/*
	Copyright (C) 2002, Yazaki
	Copyright (C) 2003, genta
	Copyright (C) 2005, D.S.Koba

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#pragma once

//	sakura
#include "_main/global.h"
#include "charset/charcode.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
// 2007.10.23 kobake テンプレートである必要も無いので、非テンプレートに変更。

#include "doc/layout/CLayout.h"
#include "doc/logic/CDocLine.h"

// ブロックコメントデリミタを管理する
class CMemoryIterator {
public:
	// CDocLine用コンストラクタ
	CMemoryIterator(const CDocLine* pcT, LayoutInt nTabSpace)
		:
		m_pLine(pcT ? pcT->GetPtr() : NULL),
		m_nLineLen(pcT ? pcT->GetLengthWithEOL() : 0),
		m_nTabSpace(nTabSpace),
		m_nIndent(LayoutInt(0))
	{
		first();
	}

	// Layout用コンストラクタ
	CMemoryIterator(const Layout* pcT, LayoutInt nTabSpace)
		:
		m_pLine(pcT ? pcT->GetPtr() : NULL),
		m_nLineLen(pcT ? pcT->GetLengthWithEOL() : 0),
		m_nTabSpace(nTabSpace),
		m_nIndent(pcT ? pcT->GetIndent() : LayoutInt(0))
	{
		first();
	}

	// 桁位置を行の先頭にセット
	void first() {
		m_nIndex = LogicInt(0);
		m_nColumn = m_nIndent;
		m_nIndex_Delta = LogicInt(0);
		m_nColumn_Delta = LayoutInt(0);
	}

	/*! 行末かどうか
		@return true: 行末, false: 行末ではない
	 */
	bool end() const {
		return (m_nLineLen <= m_nIndex);
	}

	// 次の文字を確認して次の文字との差を求める
	void scanNext() {
		// 2005-09-02 D.S.Koba GetSizeOfChar
		// 2007.09.04 kobake UNICODE化：データ増分と桁増分を別々の値として計算する。

		// データ増分を計算
		m_nIndex_Delta = LogicInt(CNativeW::GetSizeOfChar(m_pLine, m_nLineLen, m_nIndex));
		if (m_nIndex_Delta == 0) {
			m_nIndex_Delta = LogicInt(1);
		}

		// 桁増分を計算
		if (m_pLine[m_nIndex] == WCODE::TAB) {
			m_nColumn_Delta = m_nTabSpace - (m_nColumn % m_nTabSpace);
		}else {
			m_nColumn_Delta = LayoutInt(CNativeW::GetKetaOfChar(m_pLine, m_nLineLen, m_nIndex));
//			if (m_nColumn_Delta == 0) {				// 削除 サロゲートペア対策	2008/7/5 Uchi
//				m_nColumn_Delta = LayoutInt(1);
//			}
		}
	}
	
	/*! 予め計算した差分を桁位置に加える．
		@sa scanNext()
	 */
	void addDelta() {
		m_nColumn += m_nColumn_Delta;
		m_nIndex += m_nIndex_Delta;
	}	// ポインタをずらす
	
	LogicInt	getIndex()			const {	return m_nIndex;	}
	LayoutInt	getColumn()			const {	return m_nColumn;	}
	LogicInt	getIndexDelta()		const {	return m_nIndex_Delta;	}
	LayoutInt	getColumnDelta()	const {	return m_nColumn_Delta;	}

	// 2002.10.07 YAZAKI
	const wchar_t getCurrentChar() {	return m_pLine[m_nIndex];	}
	// Jul. 20, 2003 genta 追加
	// memcpyをするのにポインタがとれないと面倒
	const wchar_t* getCurrentPos() {	return m_pLine + m_nIndex;	}


private:
	// コンストラクタで受け取ったパラメータ (固定)
	const wchar_t*		m_pLine;
	const int			m_nLineLen;  // データ長。文字単位。
	const LayoutInt	m_nTabSpace;
	const LayoutInt	m_nIndent;

	// 状態変数
	LogicInt	m_nIndex;        // データ位置。文字単位。
	LayoutInt	m_nColumn;       // レイアウト位置。桁(半角幅)単位。
	LogicInt	m_nIndex_Delta;  // index増分
	LayoutInt	m_nColumn_Delta; // column増分

};

