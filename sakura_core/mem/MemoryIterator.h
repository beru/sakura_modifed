/*!	@file
	@brief LayoutとDocLineのイテレータ

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

#include "doc/layout/Layout.h"
#include "doc/logic/DocLine.h"

// ブロックコメントデリミタを管理する
class MemoryIterator {
public:
	// DocLine用コンストラクタ
	MemoryIterator(const DocLine* pcT, LayoutInt nTabSpace)
		:
		pLine(pcT ? pcT->GetPtr() : NULL),
		nLineLen(pcT ? pcT->GetLengthWithEOL() : 0),
		nTabSpace(nTabSpace),
		nIndent(LayoutInt(0))
	{
		first();
	}

	// Layout用コンストラクタ
	MemoryIterator(const Layout* pcT, LayoutInt nTabSpace)
		:
		pLine(pcT ? pcT->GetPtr() : NULL),
		nLineLen(pcT ? pcT->GetLengthWithEOL() : 0),
		nTabSpace(nTabSpace),
		nIndent(pcT ? pcT->GetIndent() : LayoutInt(0))
	{
		first();
	}

	// 桁位置を行の先頭にセット
	void first() {
		nIndex = LogicInt(0);
		nColumn = nIndent;
		nIndex_Delta = LogicInt(0);
		nColumn_Delta = LayoutInt(0);
	}

	/*! 行末かどうか
		@return true: 行末, false: 行末ではない
	 */
	bool end() const {
		return (nLineLen <= nIndex);
	}

	// 次の文字を確認して次の文字との差を求める
	void scanNext() {
		// 2005-09-02 D.S.Koba GetSizeOfChar
		// 2007.09.04 kobake UNICODE化：データ増分と桁増分を別々の値として計算する。

		// データ増分を計算
		nIndex_Delta = LogicInt(NativeW::GetSizeOfChar(pLine, nLineLen, nIndex));
		if (nIndex_Delta == 0) {
			nIndex_Delta = LogicInt(1);
		}

		// 桁増分を計算
		if (pLine[nIndex] == WCODE::TAB) {
			nColumn_Delta = nTabSpace - (nColumn % nTabSpace);
		}else {
			nColumn_Delta = LayoutInt(NativeW::GetKetaOfChar(pLine, nLineLen, nIndex));
//			if (nColumn_Delta == 0) {				// 削除 サロゲートペア対策	2008/7/5 Uchi
//				nColumn_Delta = LayoutInt(1);
//			}
		}
	}
	
	/*! 予め計算した差分を桁位置に加える．
		@sa scanNext()
	 */
	void addDelta() {
		nColumn += nColumn_Delta;
		nIndex += nIndex_Delta;
	}	// ポインタをずらす
	
	LogicInt	getIndex()			const {	return nIndex;	}
	LayoutInt	getColumn()			const {	return nColumn;	}
	LogicInt	getIndexDelta()		const {	return nIndex_Delta;	}
	LayoutInt	getColumnDelta()	const {	return nColumn_Delta;	}

	// 2002.10.07 YAZAKI
	const wchar_t getCurrentChar() {	return pLine[nIndex];	}
	// Jul. 20, 2003 genta 追加
	// memcpyをするのにポインタがとれないと面倒
	const wchar_t* getCurrentPos() {	return pLine + nIndex;	}


private:
	// コンストラクタで受け取ったパラメータ (固定)
	const wchar_t*		pLine;
	const int			nLineLen;  // データ長。文字単位。
	const LayoutInt	nTabSpace;
	const LayoutInt	nIndent;

	// 状態変数
	LogicInt	nIndex;        // データ位置。文字単位。
	LayoutInt	nColumn;       // レイアウト位置。桁(半角幅)単位。
	LogicInt	nIndex_Delta;  // index増分
	LayoutInt	nColumn_Delta; // column増分

};

