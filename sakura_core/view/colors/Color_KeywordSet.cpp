#include "StdAfx.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "Color_KeywordSet.h"
#include <limits>
#include "mem/NativeW.h"
#include "charset/charcode.h"

/** startより後ろの語の境界の位置を返す。
	startより前の文字は読まない。一番大きい戻り値は str.GetLength()と等しくなる。
*/
static size_t NextWordBreak(const StringRef& str, const size_t start);

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     キーワードセット                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

Color_KeywordSet::Color_KeywordSet()
	:
	nKeywordIndex(0),
	nCommentEnd(0)
{
}


bool Color_KeywordSet::BeginColor(const StringRef& str, size_t nPos)
{
	if (!str.IsValid()) {
		return false; // どうにもできない。
	}

	/*
		Summary:
			現在位置からキーワードを抜き出し、そのキーワードが登録単語ならば、色を変える
	*/

	const ECharKind charKind = WordParse::WhatKindOfChar(str.GetPtr(), str.GetLength(), nPos);
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

	const size_t posNextWordHead = NextWordBreak(str, nPos);
	for (int i=0; i<MAX_KEYWORDSET_PER_TYPE; ++i) {
		if (!pTypeData->colorInfoArr[COLORIDX_KEYWORD1 + i].bDisp) {
			continue; // 色設定が非表示なのでスキップ。
		}
		const int iKwdSet = pTypeData->nKeywordSetIdx[i];
		if (iKwdSet == -1) {
			continue; // キーワードセットが設定されていないのでスキップ。
		}
		size_t posWordEnd = nPos; ///< nPos...posWordEndがキーワード。
		size_t posWordEndCandidate = posNextWordHead; ///< nPos...posWordEndCandidateはキーワード候補。
		do {
			const int ret = GetDllShareData().common.specialKeyword.keywordSetMgr.SearchKeyword2(iKwdSet, str.GetPtr() + nPos, posWordEndCandidate - nPos);
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
				// CKeywordSetMgr::SearchKeyword2()から想定外の戻り値。
				break;
			}
		}while (posWordEndCandidate < str.GetLength() && ((posWordEndCandidate = NextWordBreak(str, posWordEndCandidate)) != 0));

		// nPos...posWordEnd がキーワード。
		if (nPos < posWordEnd) {
			this->nCommentEnd = posWordEnd;
			this->nKeywordIndex = i;
			return true;
		}
	}
	return false;
}

bool Color_KeywordSet::EndColor(const StringRef& str, size_t nPos)
{
	return nPos == this->nCommentEnd;
}


static inline size_t NextWordBreak(const StringRef& str, const size_t start)
{
	size_t nColumnNew;
	if (WordParse::SearchNextWordPosition4KW(str.GetPtr(), str.GetLength(), start, &nColumnNew, true)) {
		return nColumnNew;
	}
	return start;
}

