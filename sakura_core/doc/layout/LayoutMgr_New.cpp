/*!	@file
	@brief テキストのレイアウト情報管理

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK
	Copyright (C) 2002, MIK, aroka, genta, YAZAKI, novice
	Copyright (C) 2003, genta
	Copyright (C) 2004, Moca, genta
	Copyright (C) 2005, D.S.Koba, Moca
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "doc/EditDoc.h" /// 2003/07/20 genta
#include "doc/layout/LayoutMgr.h"
#include "doc/layout/Layout.h"/// 2002/2/10 aroka
#include "charset/charcode.h"
#include "mem/MemoryIterator.h"
#include "util/window.h"


/*!
	行頭禁則文字に該当するかを調べる．

	@param[in] pLine 調べる文字へのポインタ
	@param[in] length 当該箇所の文字サイズ
	@retval true 禁則文字に該当
	@retval false 禁則文字に該当しない
*/
bool LayoutMgr::IsKinsokuHead(wchar_t wc)
{
	return pszKinsokuHead_1.exist(wc);
}

/*!
	行末禁則文字に該当するかを調べる．

	@param[in] pLine 調べる文字へのポインタ
	@param[in] length 当該箇所の文字サイズ
	@retval true 禁則文字に該当
	@retval false 禁則文字に該当しない
*/
bool LayoutMgr::IsKinsokuTail(wchar_t wc)
{
	return pszKinsokuTail_1.exist(wc);
}


/*!
	禁則対象句読点に該当するかを調べる．

	@param [in] pLine  調べる文字へのポインタ
	@param [in] length 当該箇所の文字サイズ
	@retval true 禁則文字に該当
	@retval false 禁則文字に該当しない
*/
bool LayoutMgr::IsKinsokuKuto(wchar_t wc)
{
	return pszKinsokuKuto_1.exist(wc);
}

/*!
	@date 2005-08-20 D.S.Koba _DoLayout()とDoLayout_Range()から分離
*/
bool LayoutMgr::IsKinsokuPosHead(
	size_t nRest,		// [in] 行の残り文字数
	size_t nCharKetas,	// [in] 現在位置の文字サイズ
	size_t nCharKetas2	// [in] 現在位置の次の文字サイズ
	)
{
	switch (nRest) {
	//    321012  ↓マジックナンバー
	// 3 "る）" : 22 "）"の2バイト目で折り返しのとき
	// 2  "Z）" : 12 "）"の2バイト目で折り返しのとき
	// 2  "る）": 22 "）"で折り返しのとき
	// 2  "る)" : 21 ")"で折り返しのとき
	// 1   "Z）": 12 "）"で折り返しのとき
	// 1   "Z)" : 11 ")"で折り返しのとき
	// ↑何文字前か？
	// ※ただし、"るZ"部分が禁則なら処理しない。
	case 3:	// 3文字前
		if (nCharKetas == 2 && nCharKetas2 == 2) {
			return true;
		}
		break;
	case 2:	// 2文字前
		if (nCharKetas == 2) {
			return true;
		}else if (nCharKetas == 1 && nCharKetas2 == 2) {
			return true;
		}
		break;
	case 1:	// 1文字前
		if (nCharKetas == 1) {
			return true;
		}
		break;
	}
	return false;
}

/*!
	@date 2005-08-20 D.S.Koba _DoLayout()とDoLayout_Range()から分離
*/
bool LayoutMgr::IsKinsokuPosTail(
	size_t nRest,		// [in] 行の残り文字数
	size_t nCharKetas,	// [in] 現在位置の文字サイズ
	size_t nCharKetas2	// [in] 現在位置の次の文字サイズ
	)
{
	switch (nRest) {
	case 3:	// 3文字前
		if (nCharKetas == 2 && nCharKetas2 == 2) {
			// "（あ": "あ"の2バイト目で折り返しのとき
			return true;
		}
		break;
	case 2:	// 2文字前
		if (nCharKetas == 2) {
			// "（あ": "あ"で折り返しのとき
			return true;
		}else if (nCharKetas == 1 && nCharKetas2 == 2) {
			// "(あ": "あ"の2バイト目で折り返しのとき
			return true;
		}
		break;
	case 1:	// 1文字前
		if (nCharKetas == 1) {
			// "(あ": "あ"で折り返しのとき
			return true;
		}
		break;
	}
	return false;
}


/*!
	@brief 行の長さを計算する (2行目以降の字下げ無し)
	
	字下げを行わないので，常に0を返す．
	引数は使わない．
	
	@return 1行の表示文字数 (常に0)
	
	@author genta
	@date 2002.10.01
*/
size_t LayoutMgr::getIndentOffset_Normal(Layout*)
{
	return 0;
}

/*!
	@brief インデント幅を計算する (Tx2x)
	
	前の行の最後のTABの位置をインデント位置として返す．
	ただし，残り幅が6文字未満の場合はインデントを行わない．
	
	@author Yazaki
	@return インデントすべき文字数
	
	@date 2002.10.01 
	@date 2002.10.07 YAZAKI 名称変更, 処理見直し
*/
size_t LayoutMgr::getIndentOffset_Tx2x(Layout* pLayoutPrev)
{
	// 前の行が無いときは、インデント不要。
	if (!pLayoutPrev) {
		return 0;
	}
	size_t nIpos = pLayoutPrev->GetIndent();

	// 前の行が折り返し行ならばそれに合わせる
	if (pLayoutPrev->GetLogicOffset() > 0) {
		return nIpos;
	}
	
	MemoryIterator it(pLayoutPrev, GetTabSpace());
	while (!it.end()) {
		it.scanNext();
		if (it.getIndexDelta() == 1 && it.getCurrentChar() == WCODE::TAB) {
			nIpos = it.getColumn() + it.getColumnDelta();
		}
		it.addDelta();
	}
	// 2010.07.06 Moca TAB=8などの場合に折り返すと無限ループする不具合の修正. 6固定を nTabSpace + 2に変更
	if (GetMaxLineKetas() - nIpos < GetTabSpace() + 2) {
		nIpos = t_max((size_t)0, GetMaxLineKetas() - (GetTabSpace() + 2)); // 2013.05.12 Chg:0だったのを最大幅に変更
	}
	return nIpos;	// インデント
}

/*!
	@brief インデント幅を計算する (スペース字下げ版)
	
	論理行行頭のホワイトスペースの終わりインデント位置として返す．
	ただし，残り幅が6文字未満の場合はインデントを行わない．
	
	@author genta
	@return インデントすべき文字数
	
	@date 2002.10.01 
*/
size_t LayoutMgr::getIndentOffset_LeftSpace(Layout* pLayoutPrev)
{
	// 前の行が無いときは、インデント不要。
	if (!pLayoutPrev) {
		return 0;
	}
	// インデントの計算
	size_t nIpos = pLayoutPrev->GetIndent();
	
	// Oct. 5, 2002 genta
	// 折り返しの3行目以降は1つ前の行のインデントに合わせる．
	if (pLayoutPrev->GetLogicOffset() > 0) {
		return nIpos;
	}
	
	// 2002.10.07 YAZAKI インデントの計算
	MemoryIterator it(pLayoutPrev, GetTabSpace());

	// Jul. 20, 2003 genta 自動インデントに準じた動作にする
	bool bZenSpace = pTypeConfig->bAutoIndent_ZENSPACE;
	const wchar_t* szSpecialIndentChar = pTypeConfig->szIndentChars;
	while (!it.end()) {
		it.scanNext();
		if (it.getIndexDelta() == 1 && WCODE::IsIndentChar(it.getCurrentChar(), bZenSpace)) {
			// インデントのカウントを継続する
		// Jul. 20, 2003 genta インデント対象文字
		}else if (szSpecialIndentChar[0] != L'\0') {
			wchar_t buf[3]; // 文字の長さは1 or 2
			wmemcpy(buf, it.getCurrentPos(), it.getIndexDelta());
			buf[it.getIndexDelta()] = L'\0';
			if (wcsstr(szSpecialIndentChar, buf)) {
				// インデントのカウントを継続する
			}else {
				nIpos = it.getColumn();	// 終了
				break;
			}
		}else {
			nIpos = it.getColumn();	// 終了
			break;
		}
		it.addDelta();
	}
	if (it.end()) {
		nIpos = it.getColumn();	// 終了
	}
	// 2010.07.06 Moca TAB=8などの場合に折り返すと無限ループする不具合の修正. 6固定を nTabSpace + 2に変更
	if (GetMaxLineKetas() - nIpos < GetTabSpace() + 2) {
		nIpos = t_max((size_t)0, GetMaxLineKetas() - (GetTabSpace() + 2)); // 2013.05.12 Chg:0だったのを最大幅に変更
	}
	return nIpos;	// インデント
}

/*!
	@brief  テキスト最大幅を算出する

	指定されたラインを走査してテキストの最大幅を作成する。
	全て削除された時は未算出状態に戻す。

	@param bCalLineLen	[in] 各レイアウト行の長さの算出も行う
	@param nStart		[in] 算出開始レイアウト行
	@param nEnd			[in] 算出終了レイアウト行

	@retval TRUE 最大幅が変化した
	@retval FALSE 最大幅が変化しなかった

	@note nStart, nEndが両方とも-1の時、全ラインを走査する
		  範囲が指定されている場合は最大幅の拡大のみチェックする

	@date 2009.08.28 nasukoji	新規作成
*/
BOOL LayoutMgr::CalculateTextWidth(bool bCalLineLen, int nStart, int nEnd)
{
	bool bRet = false;
	bool bOnlyExpansion = true;		// 最大幅の拡大のみをチェックする
	size_t nMaxLen = 0;
	int nMaxLineNum = 0;

	size_t nLines = GetLineCount();	// テキストのレイアウト行数

	// 開始・終了位置がどちらも指定されていない
	if (nStart < 0 && nEnd < 0) {
		bOnlyExpansion = false;		// 最大幅の拡大・縮小をチェックする
	}
	if (nStart < 0) {				// 算出開始行の指定なし
		nStart = 0;
	}else if (nStart > (int)nLines) {	// 範囲オーバー
		nStart = (int)nLines;
	}
	if (nEnd < 0 || nEnd >= (int)nLines) {	// 算出終了行の指定なし または 文書行数以上
		nEnd = (int)nLines;
	}else {
		++nEnd;						// 算出終了行の次行
	}
	Layout* pLayout;
	// 算出開始レイアウト行を探す
	// 2013.05.13 SearchLineByLayoutYを使う
	if (nStart == 0) {
		pLayout = pLayoutTop;
	}else {
		pLayout = SearchLineByLayoutY(nStart);
	}
#if 0
	if (nStart * 2 < nLines) {
		// 前方からサーチ
		int nCount = 0;
		pLayout = pLayoutTop;
		while (pLayout) {
			if (nStart == nCount) {
				break;
			}
			pLayout = pLayout->GetNextLayout();
			++nCount;
		}
	}else {
		// 後方からサーチ
		int nCount = nLines - 1;
		pLayout = pLayoutBot;
		while (pLayout) {
			if (nStart == nCount) {
				break;
			}
			pLayout = pLayout->GetPrevLayout();
			nCount--;
		}
	}
#endif

	// レイアウト行の最大幅を取り出す
	for (int i=nStart; i<nEnd; ++i) {
		if (!pLayout) {
			break;
		}
		// レイアウト行の長さを算出する
		if (bCalLineLen) {
			int nWidth = pLayout->CalcLayoutWidth(*this) + pLayout->GetLayoutEol().GetLen() > 0 ? 1 : 0;
			pLayout->SetLayoutWidth(nWidth);
		}

		// 最大幅を更新
		if (nMaxLen < pLayout->GetLayoutWidth()) {
			nMaxLen = pLayout->GetLayoutWidth();
			nMaxLineNum = i;		// 最大幅のレイアウト行

			// アプリケーションの最大幅となったら算出は停止
			if (nMaxLen >= MAXLINEKETAS && !bCalLineLen) {
				break;
			}
		}

		// 次のレイアウト行のデータ
		pLayout = pLayout->GetNextLayout();
	}

	// テキストの幅の変化をチェック
	if (nMaxLen) {
		// 最大幅が拡大した または 最大幅の拡大のみチェックでない
		if (nTextWidth < nMaxLen || !bOnlyExpansion) {
			nTextWidthMaxLine = nMaxLineNum;
			if (nTextWidth != nMaxLen) {	// 最大幅変化あり
				nTextWidth = nMaxLen;
				bRet = true;
			}
		}
	}else if (nTextWidth && !nLines) {
		// 全削除されたら幅の記憶をクリア
		nTextWidthMaxLine = 0;
		nTextWidth = 0;
		bRet = true;
	}
	
	return bRet;
}

/*!
	@brief  各行のレイアウト行長の記憶をクリアする
	
	@note 折り返し方法が「折り返さない」以外の時は、パフォーマンスの低下を
		  防止する目的で各行のレイアウト行長(nLayoutWidth)を計算していない。
		  後で設計する人が誤って使用してしまうかもしれないので「折り返さない」
		  以外の時はクリアしておく。
		  パフォーマンスの低下が気にならない程なら、全ての折り返し方法で計算
		  するようにしても良いと思う。

	@date 2009.08.28 nasukoji	新規作成
*/
void LayoutMgr::ClearLayoutLineWidth(void)
{
	Layout* pLayout = pLayoutTop;
	while (pLayout) {
		pLayout->nLayoutWidth = 0;			// レイアウト行長をクリア
		pLayout = pLayout->GetNextLayout();		// 次のレイアウト行のデータ
	}
}

