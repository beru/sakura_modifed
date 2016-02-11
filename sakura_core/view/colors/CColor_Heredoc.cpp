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
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_Heredoc.h"
#include "doc/layout/CLayout.h"

class LayoutColorHeredocInfo : public LayoutColorInfo {
public:
	std::wstring m_id;
	bool IsEqual(const LayoutColorInfo* p) const {
		if (!p) {
			return false;
		}
		const LayoutColorHeredocInfo* info = dynamic_cast<const LayoutColorHeredocInfo*>(p);
		if (!info) {
			return false;
		}
		return info->m_id == this->m_id;
	}
};

void Color_Heredoc::SetStrategyColorInfo(const LayoutColorInfo* colorInfo)
{
	if (colorInfo) {
		const LayoutColorHeredocInfo* info = dynamic_cast<const LayoutColorHeredocInfo*>(colorInfo);
		if (!info) {
			return;
		}
		m_pszId = info->m_id.c_str();
		m_nSize = (int)info->m_id.size();
	}
}

LayoutColorInfo* Color_Heredoc::GetStrategyColorInfo() const
{
	LayoutColorHeredocInfo* info = new LayoutColorHeredocInfo();
	info->m_id.assign(m_pszId, m_nSize);
	return info;
}

bool Color_Heredoc::BeginColor(const StringRef& cStr, int nPos)
{
	if (!cStr.IsValid()) return false;

	// ヒアドキュメント
	// <<<HEREDOC_ID
	// ...
	// HEREDOC_ID
	if (1
		&& m_pTypeData->m_nHeredocType == HereDocType::PHP
		&& cStr.At(nPos) == '<' && nPos + 3 < cStr.GetLength()
		&& wmemcmp(cStr.GetPtr() + nPos + 1, L"<<", 2) == 0
	) {
		// <<<[\t]*((['"][_A-Za-z0-9]+['"])|[_A-Za-z0-9]+)[\r\n]+
		const int length = cStr.GetLength();
		int nPosIdStart = nPos + 3;
		for (; nPosIdStart<length; ++nPosIdStart) {
			if (cStr.At(nPosIdStart) != L'\t' && cStr.At(nPosIdStart) != L' ') {
				break;
			}
		}
		wchar_t quote = L'\0';
		if (!(nPosIdStart < length)) {
			return false;
		}
		if (cStr.At(nPosIdStart) == L'\'' || cStr.At(nPosIdStart) == L'"') {
			quote = cStr.At(nPosIdStart);
			++nPosIdStart;
		}
		int i = nPosIdStart;
		for (; i<length; ++i) {
			if (!(WCODE::IsAZ(cStr.At(i)) || WCODE::Is09(cStr.At(i)) || cStr.At(i) == L'_')) {
				break;
			}
		}
		if (nPosIdStart == i) {
			return false;
		}
		const int k = i;
		if (quote != L'\0') {
			if (i < length && cStr.At(i) == quote) {
				++i;
			}else {
				return false;
			}
		}
		if (
			i < length
			&& WCODE::IsLineDelimiter(
				cStr.At(i),
				GetDllShareData().m_common.m_edit.m_bEnableExtEol
			)
		) {
			m_id = std::wstring(cStr.GetPtr() + nPosIdStart, k - nPosIdStart);
			m_pszId = m_id.c_str();
			m_nSize = m_id.size();
			this->m_nCOMMENTEND = length;
			return true;
		}
	}
	return false;
}

bool Color_Heredoc::EndColor(const StringRef& cStr, int nPos)
{
	if (this->m_nCOMMENTEND == 0) {
		if (1
			&& m_pTypeData->m_nHeredocType == HereDocType::PHP
			&& nPos == 0 && m_nSize <= cStr.GetLength()
			&& wmemcmp(cStr.GetPtr(), m_pszId, m_nSize) == 0
		) {
			if (m_nSize == cStr.GetLength()) {
				this->m_nCOMMENTEND = m_nSize;
				return false;
			}else {
				int i = m_nSize;
				if (
					i + 1 < cStr.GetLength()
					&& cStr.At(i) == L';'
					&& WCODE::IsLineDelimiter(
						cStr.At(i+1),
						GetDllShareData().m_common.m_edit.m_bEnableExtEol
					)
				) {
					// ID;
					this->m_nCOMMENTEND = i;
					return false;
				}else if (
					m_nSize < cStr.GetLength()
					&& WCODE::IsLineDelimiter(
						cStr.At(m_nSize),
						GetDllShareData().m_common.m_edit.m_bEnableExtEol
					)
				) {
					// ID
					this->m_nCOMMENTEND = m_nSize;
					return false;
				}
			}
			this->m_nCOMMENTEND = cStr.GetLength();
		}else {
			this->m_nCOMMENTEND = cStr.GetLength();
		}
	}else if (nPos == this->m_nCOMMENTEND) {
		return true;
	}
	return false;
}

