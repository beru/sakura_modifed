/*
	Copyright (C) 2011, Moca

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#include "StdAfx.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "Color_Heredoc.h"
#include "doc/layout/Layout.h"

class LayoutColorHeredocInfo : public LayoutColorInfo {
public:
	std::wstring id;
	bool IsEqual(const LayoutColorInfo* p) const {
		if (!p) {
			return false;
		}
		const LayoutColorHeredocInfo* info = dynamic_cast<const LayoutColorHeredocInfo*>(p);
		if (!info) {
			return false;
		}
		return info->id == this->id;
	}
};

void Color_Heredoc::SetStrategyColorInfo(const LayoutColorInfo* colorInfo)
{
	if (colorInfo) {
		const LayoutColorHeredocInfo* info = dynamic_cast<const LayoutColorHeredocInfo*>(colorInfo);
		if (!info) {
			return;
		}
		pszId = info->id.c_str();
		nSize = (int)info->id.size();
	}
}

LayoutColorInfo* Color_Heredoc::GetStrategyColorInfo() const
{
	LayoutColorHeredocInfo* info = new LayoutColorHeredocInfo();
	info->id.assign(pszId, nSize);
	return info;
}

bool Color_Heredoc::BeginColor(const StringRef& str, size_t nPos)
{
	if (!str.IsValid()) return false;

	// ヒアドキュメント
	// <<<HEREDOC_ID
	// ...
	// HEREDOC_ID
	if (1
		&& pTypeData->nHeredocType == HereDocType::PHP
		&& str.At(nPos) == '<' && nPos + 3 < str.GetLength()
		&& wmemcmp(str.GetPtr() + nPos + 1, L"<<", 2) == 0
	) {
		// <<<[\t]*((['"][_A-Za-z0-9]+['"])|[_A-Za-z0-9]+)[\r\n]+
		const size_t length = str.GetLength();
		size_t nPosIdStart = nPos + 3;
		for (; nPosIdStart<length; ++nPosIdStart) {
			if (str.At(nPosIdStart) != L'\t' && str.At(nPosIdStart) != L' ') {
				break;
			}
		}
		wchar_t quote = L'\0';
		if (!(nPosIdStart < length)) {
			return false;
		}
		if (str.At(nPosIdStart) == L'\'' || str.At(nPosIdStart) == L'"') {
			quote = str.At(nPosIdStart);
			++nPosIdStart;
		}
		size_t i = nPosIdStart;
		for (; i<length; ++i) {
			if (!(WCODE::IsAZ(str.At(i)) || WCODE::Is09(str.At(i)) || str.At(i) == L'_')) {
				break;
			}
		}
		if (nPosIdStart == i) {
			return false;
		}
		const size_t k = i;
		if (quote != L'\0') {
			if (i < length && str.At(i) == quote) {
				++i;
			}else {
				return false;
			}
		}
		if (
			i < length
			&& WCODE::IsLineDelimiter(
				str.At(i),
				GetDllShareData().common.edit.bEnableExtEol
			)
		) {
			id = std::wstring(str.GetPtr() + nPosIdStart, k - nPosIdStart);
			pszId = id.c_str();
			nSize = id.size();
			this->nCommentEnd = length;
			return true;
		}
	}
	return false;
}

bool Color_Heredoc::EndColor(const StringRef& str, size_t nPos)
{
	if (this->nCommentEnd == 0) {
		if (1
			&& pTypeData->nHeredocType == HereDocType::PHP
			&& nPos == 0 && nSize <= str.GetLength()
			&& wmemcmp(str.GetPtr(), pszId, nSize) == 0
		) {
			if (nSize == str.GetLength()) {
				this->nCommentEnd = nSize;
				return false;
			}else {
				size_t i = nSize;
				if (
					i + 1 < str.GetLength()
					&& str.At(i) == L';'
					&& WCODE::IsLineDelimiter(
						str.At(i+1),
						GetDllShareData().common.edit.bEnableExtEol
					)
				) {
					// ID;
					this->nCommentEnd = i;
					return false;
				}else if (
					nSize < str.GetLength()
					&& WCODE::IsLineDelimiter(
						str.At(nSize),
						GetDllShareData().common.edit.bEnableExtEol
					)
				) {
					// ID
					this->nCommentEnd = nSize;
					return false;
				}
			}
			this->nCommentEnd = str.GetLength();
		}else {
			this->nCommentEnd = str.GetLength();
		}
	}else if (nPos == this->nCommentEnd) {
		return true;
	}
	return false;
}

