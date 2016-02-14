#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_KeywordSet.h"
#include <limits>
#include "mem/CNativeW.h"
#include "charset/charcode.h"

/** startより後ろの語の境界の位置を返す。
	startより前の文字は読まない。一番大きい戻り値は str.GetLength()と等しくなる。
*/
static int NextWordBreak(const StringRef& str, const int start);

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     キーワードセット                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

Color_KeywordSet::Color_KeywordSet()
	:
	m_nKeywordIndex(0),
	m_nCOMMENTEND(0)
{
}


// 2005.01.13 MIK 強調キーワード数追加に伴う配列化
bool Color_KeywordSet::BeginColor(const StringRef& str, int nPos)
{
	if (!str.IsValid()) {
		return false; // どうにもできない。
	}

	/*
		Summary:
			現在位置からキーワードを抜き出し、そのキーワードが登録単語ならば、色を変える
	*/

	const ECharKind charKind = WordParse::WhatKindOfChar(str.GetPtr(), str.GetLength() , nPos);
	if (charKind <= CK_SPACE) {
		return false; // この文字はキーワード対象文字ではない。
	}
	if (0 < nPos) {
		const ECharKind charKindPrev = WordParse::WhatKindOfChar(str.GetPtr(), str.GetLength() , nPos-1);
		const ECharKind charKindTwo = WordParse::WhatKindOfTwoChars4KW(charKindPrev, charKind);
		if (charKindTwo != CK_NULL) {
			return false;
		}
	}

	const int posNextWordHead = NextWordBreak(str, nPos);
	for (int i=0; i<MAX_KEYWORDSET_PER_TYPE; ++i) {
		if (!m_pTypeData->m_colorInfoArr[COLORIDX_KEYWORD1 + i].m_bDisp) {
			continue; // 色設定が非表示なのでスキップ。
		}
		const int iKwdSet = m_pTypeData->m_nKeyWordSetIdx[i];
		if (iKwdSet == -1) {
			continue; // キーワードセットが設定されていないのでスキップ。
		}
		int posWordEnd = nPos; ///< nPos...posWordEndがキーワード。
		int posWordEndCandidate = posNextWordHead; ///< nPos...posWordEndCandidateはキーワード候補。
		do {
			const int ret = GetDllShareData().m_common.m_specialKeyword.m_keyWordSetMgr.SearchKeyWord2(iKwdSet, str.GetPtr() + nPos, posWordEndCandidate - nPos);
			if (0 <= ret) {
				// 登録されたキーワードだった。
				posWordEnd = posWordEndCandidate;
				if (ret == std::numeric_limits<int>::max()) {
					// より長いキーワードも存在するので延長してリトライ。
					continue;
				}
				break;
			}else if (ret == -1) {
				// 登録されたキーワードではなかった。
				break;
			}else if (ret == -2) {
				// 長さが足りなかったので延長してリトライ。
				continue;
			}else {
				// 登録されたキーワードではなかった？
				// CKeyWordSetMgr::SearchKeyWord2()から想定外の戻り値。
				break;
			}
		}while (posWordEndCandidate < str.GetLength() && ((posWordEndCandidate = NextWordBreak(str, posWordEndCandidate)) != 0));

		// nPos...posWordEnd がキーワード。
		if (nPos < posWordEnd) {
			this->m_nCOMMENTEND = posWordEnd;
			this->m_nKeywordIndex = i;
			return true;
		}
	}
	return false;
}

bool Color_KeywordSet::EndColor(const StringRef& str, int nPos)
{
	return nPos == this->m_nCOMMENTEND;
}


static inline int NextWordBreak(const StringRef& str, const int start)
{
	LogicInt nColumnNew;
	if (WordParse::SearchNextWordPosition4KW(str.GetPtr(), LogicInt(str.GetLength()), LogicInt(start), &nColumnNew, true)) {
		return nColumnNew;
	}
	return start;
}

