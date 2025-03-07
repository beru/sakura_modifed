/*!	@file
	@brief LayoutとDocLineのイテレータ
*/
#pragma once

//	sakura
#include "_main/global.h"
#include "charset/charcode.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/

#include "doc/layout/Layout.h"
#include "doc/logic/DocLine.h"

// ブロックコメントデリミタを管理する
class MemoryIterator {
public:
	// DocLine用コンストラクタ
	MemoryIterator(const DocLine* pcT, size_t nTabSpace)
		:
		pLine(pcT ? pcT->GetPtr() : NULL),
		nLineLen(pcT ? pcT->GetLengthWithEOL() : 0),
		nTabSpace(nTabSpace),
		nIndent(0)
	{
		first();
	}

	// Layout用コンストラクタ
	MemoryIterator(const Layout* pcT, size_t nTabSpace)
		:
		pLine(pcT ? pcT->GetPtr() : NULL),
		nLineLen(pcT ? pcT->GetLengthWithEOL() : 0),
		nTabSpace(nTabSpace),
		nIndent(pcT ? pcT->GetIndent() : 0)
	{
		first();
	}

	// 桁位置を行の先頭にセット
	void first() {
		nIndex = 0;
		nColumn = nIndent;
		nIndex_Delta = 0;
		nColumn_Delta = 0;
	}

	/*! 行末かどうか
		@return true: 行末, false: 行末ではない
	 */
	bool end() const {
		return (nLineLen <= nIndex);
	}

	// 次の文字を確認して次の文字との差を求める
	void scanNext() {

		// データ増分を計算
		nIndex_Delta = NativeW::GetSizeOfChar(pLine, nLineLen, nIndex);
		if (nIndex_Delta == 0) {
			nIndex_Delta = 1;
		}

		// 桁増分を計算
		if (pLine[nIndex] == WCODE::TAB) {
			nColumn_Delta = nTabSpace - (nColumn % nTabSpace);
		}else {
			nColumn_Delta = NativeW::GetKetaOfChar(pLine, nLineLen, nIndex);
		}
	}
	
	/*! 予め計算した差分を桁位置に加える．
		@sa scanNext()
	 */
	void addDelta() {
		nColumn += nColumn_Delta;
		nIndex += nIndex_Delta;
	}	// ポインタをずらす
	
	size_t	getIndex()			const {	return nIndex;	}
	size_t	getColumn()			const {	return nColumn;	}
	size_t	getIndexDelta()		const {	return nIndex_Delta;	}
	size_t	getColumnDelta()	const {	return nColumn_Delta;	}

	const wchar_t getCurrentChar() {	return pLine[nIndex];	}
	const wchar_t* getCurrentPos() {	return pLine + nIndex;	}


private:
	// コンストラクタで受け取ったパラメータ (固定)
	const wchar_t*	pLine;
	const size_t	nLineLen;  // データ長。文字単位。
	const size_t	nTabSpace;
	const size_t	nIndent;

	// 状態変数
	size_t	nIndex;        // データ位置。文字単位。
	size_t	nColumn;       // レイアウト位置。桁(半角幅)単位。
	size_t	nIndex_Delta;  // index増分
	size_t	nColumn_Delta; // column増分

};

