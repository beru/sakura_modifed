#include "StdAfx.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "Color_KeywordSet.h"
#include <limits>
#include "mem/NativeW.h"
#include "charset/charcode.h"

/** start�����̌�̋��E�̈ʒu��Ԃ��B
	start���O�̕����͓ǂ܂Ȃ��B��ԑ傫���߂�l�� str.GetLength()�Ɠ������Ȃ�B
*/
static size_t NextWordBreak(const StringRef& str, const size_t start);

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �L�[���[�h�Z�b�g                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

Color_KeywordSet::Color_KeywordSet()
	:
	nKeywordIndex(0),
	nCommentEnd(0)
{
}


// 2005.01.13 MIK �����L�[���[�h���ǉ��ɔ����z��
bool Color_KeywordSet::BeginColor(const StringRef& str, size_t nPos)
{
	if (!str.IsValid()) {
		return false; // �ǂ��ɂ��ł��Ȃ��B
	}

	/*
		Summary:
			���݈ʒu����L�[���[�h�𔲂��o���A���̃L�[���[�h���o�^�P��Ȃ�΁A�F��ς���
	*/

	const ECharKind charKind = WordParse::WhatKindOfChar(str.GetPtr(), str.GetLength(), nPos);
	if (charKind <= CK_SPACE) {
		return false; // ���̕����̓L�[���[�h�Ώە����ł͂Ȃ��B
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
			continue; // �F�ݒ肪��\���Ȃ̂ŃX�L�b�v�B
		}
		const int iKwdSet = pTypeData->nKeywordSetIdx[i];
		if (iKwdSet == -1) {
			continue; // �L�[���[�h�Z�b�g���ݒ肳��Ă��Ȃ��̂ŃX�L�b�v�B
		}
		size_t posWordEnd = nPos; ///< nPos...posWordEnd���L�[���[�h�B
		size_t posWordEndCandidate = posNextWordHead; ///< nPos...posWordEndCandidate�̓L�[���[�h���B
		do {
			const int ret = GetDllShareData().common.specialKeyword.keywordSetMgr.SearchKeyword2(iKwdSet, str.GetPtr() + nPos, posWordEndCandidate - nPos);
			if (0 <= ret) {
				// �o�^���ꂽ�L�[���[�h�������B
				posWordEnd = posWordEndCandidate;
				if (ret == std::numeric_limits<int>::max()) {
					// ��蒷���L�[���[�h�����݂���̂ŉ������ă��g���C�B
					continue;
				}
				break;
			}else if (ret == -1) {
				// �o�^���ꂽ�L�[���[�h�ł͂Ȃ������B
				break;
			}else if (ret == -2) {
				// ����������Ȃ������̂ŉ������ă��g���C�B
				continue;
			}else {
				// �o�^���ꂽ�L�[���[�h�ł͂Ȃ������H
				// CKeywordSetMgr::SearchKeyword2()����z��O�̖߂�l�B
				break;
			}
		}while (posWordEndCandidate < str.GetLength() && ((posWordEndCandidate = NextWordBreak(str, posWordEndCandidate)) != 0));

		// nPos...posWordEnd ���L�[���[�h�B
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

