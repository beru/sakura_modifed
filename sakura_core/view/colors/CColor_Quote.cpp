#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_Quote.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     クォーテーション                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class LayoutColorQuoteInfo : public LayoutColorInfo {
public:
	std::wstring m_tag;
	int m_nColorTypeIndex;
	bool IsEqual(const LayoutColorInfo* p) const {
		if (!p) {
			return false;
		}
		const LayoutColorQuoteInfo* info = dynamic_cast<const LayoutColorQuoteInfo*>(p);
		if (!info) {
			return false;
		}
		return info->m_tag == this->m_tag;
	}
};

void Color_Quote::Update(void)
{
	const EditDoc* pEditDoc = EditDoc::GetInstance(0);
	m_pTypeData = &pEditDoc->m_docType.GetDocumentAttribute();
	m_nStringType = m_pTypeData->m_nStringType;
	StringLiteralType nEspaceTypeList[] = {
		StringLiteralType::CPP,
		StringLiteralType::PLSQL,
		StringLiteralType::HTML,
		StringLiteralType::CPP,
		StringLiteralType::CPP,
	};
	m_nEscapeType = nEspaceTypeList[(int)m_nStringType];
	bool* pbEscapeEndList[] = {
		&m_bEscapeEnd,
		NULL,
		NULL,
		NULL,
		&m_bEscapeEnd,
	};
	m_pbEscapeEnd = pbEscapeEndList[(int)m_nStringType];
}

void Color_Quote::SetStrategyColorInfo(const LayoutColorInfo* colorInfo)
{
	if (colorInfo) {
		const LayoutColorQuoteInfo* info = dynamic_cast<const LayoutColorQuoteInfo*>(colorInfo);
		if (!info) {
			return;
		}
		m_tag = info->m_tag;
		m_nColorTypeIndex = info->m_nColorTypeIndex;
	}else {
		m_nColorTypeIndex = 0;
	}
}

LayoutColorInfo* Color_Quote::GetStrategyColorInfo() const
{
	if (0 < m_nColorTypeIndex) {
		LayoutColorQuoteInfo* info = new LayoutColorQuoteInfo();
		info->m_tag = m_tag;
		info->m_nColorTypeIndex = m_nColorTypeIndex;
		return info;
	}
	return NULL;
}

// nPos "の位置
//staic
bool Color_Quote::IsCppRawString(const StringRef& cStr, int nPos)
{
	if (
		0 < nPos
		&& cStr.At(nPos-1) == 'R'
		&& cStr.At(nPos) == '"'
		&& nPos + 1 < cStr.GetLength()
	) {
		// \b(u8|u|U|L|)R"[^(]*\(
		// \b = ^|[\s!"#$%&'()=@{};:<>?,.*/\-\+\[\]\]
		wchar_t c1 = L' ';
		if (2 <= nPos) {
			c1 = cStr.At(nPos-2);
		}
		wchar_t c2 = L' ';
		if (3 <= nPos) {
			c2 = cStr.At(nPos-3);
		}
		const wchar_t* pszSep = L" \t!\"#$%&'()=@{};:<>?,.*/-+[]";
		if ((c1 == 'u' || c1 == 'U' || c1 == 'L')) {
			if (wcschr(pszSep, c2)) {
				return true;
			}
		}else if (c1 == '8' && c2 == 'u') {
			wchar_t c3 = L'\0';
			if (4 <= nPos) {
				c3 = cStr.At(nPos-4);
			}
			if (wcschr(pszSep, c3)) {
				return true;
			}
		}else if (wcschr(pszSep, c1)) {
			return true;
		}
	}
	return false;
}

bool Color_Quote::BeginColor(const StringRef& cStr, int nPos)
{
	if (!cStr.IsValid()) return false;

	if (cStr.At(nPos) == m_cQuote) {
		m_nCOMMENTEND = -1;
		StringLiteralType nStringType = m_pTypeData->m_nStringType;
		bool bPreString = true;
		// クォーテーション文字列の終端があるか
		switch (nStringType) {
		case StringLiteralType::CPP:
			if (
				0 < nPos
				&& cStr.At(nPos - 1) == 'R'
				&& cStr.At(nPos) == '"'
				&& nPos + 1 < cStr.GetLength()
			) {
				for (int i=nPos+1; i<cStr.GetLength(); ++i) {
					if (cStr.At(i) == '(') {
						if (nPos + 1 < i) {
							m_tag = L')';
							m_tag.append(cStr.GetPtr() + nPos + 1, i - (nPos + 1));
							m_tag += L'"';
						}else {
							m_tag.assign(L")\"", 2);
						}
						m_nCOMMENTEND = Match_QuoteStr(m_tag.c_str(), m_tag.size(), i + 1, cStr, false);
						m_nColorTypeIndex = 1;
						return true;
					}
				}
			}
			break;
		case StringLiteralType::HTML:
			{
				int i;
				for (i=nPos-1; 0<=i; --i) {
					if (cStr.At(i) != L' ' && cStr.At(i) != L'\t') {
						break;
					}
				}
				if (!(0 <= i && cStr.At(i) == L'=')) {
					bPreString = false;
				}
			}
			break;
		case StringLiteralType::CSharp:
			if (0 < nPos && cStr.At(nPos - 1) == L'@' && m_cQuote == L'"') {
				m_nCOMMENTEND = Match_Quote(m_cQuote, nPos + 1, cStr, StringLiteralType::PLSQL);
				m_nColorTypeIndex = 2;
				return true;
			}
			break;
		case StringLiteralType::Python:
			if (
				nPos + 2 < cStr.GetLength()
			 	&& cStr.At(nPos + 1) == m_cQuote
			 	&& cStr.At(nPos + 2) == m_cQuote
			) {
				m_nCOMMENTEND = Match_QuoteStr(m_szQuote, 3, nPos + 3, cStr, true);
				m_nColorTypeIndex = 3;
				return true;
			}
			break;
		}
		m_bEscapeEnd = false;
		if (bPreString) {
			m_nCOMMENTEND = Match_Quote(m_cQuote, nPos + 1, cStr, m_nEscapeType, m_pbEscapeEnd);
			m_nColorTypeIndex = 0;
		}

		// 「文字列は行内のみ」(C++ Raw String、Pythonのlong string、@""は特別)
		if (
			m_pTypeData->m_bStringLineOnly
			&& !m_bEscapeEnd
			&& m_nCOMMENTEND == cStr.GetLength()
		) {
			// 終了文字列がない場合は行末までを色分け
			if (m_pTypeData->m_bStringEndLine) {
				// 改行コードを除く
				if (
					0 < cStr.GetLength()
					&& WCODE::IsLineDelimiter(
						cStr.At(cStr.GetLength() - 1),
						GetDllShareData().m_common.m_edit.m_bEnableExtEol
					)
				) {
					if (1 &&
						1 < cStr.GetLength()
						&& cStr.At(cStr.GetLength() - 2) == WCODE::CR
						&& cStr.At(cStr.GetLength() - 1) == WCODE::LF
					) {
						m_nCOMMENTEND = cStr.GetLength() - 2;
					}else {
						m_nCOMMENTEND = cStr.GetLength() - 1;
					}
				}
				return true;
			}
			// 終了文字列がない場合は色分けしない
			m_nCOMMENTEND = -1;
			return false;
		}
		if (0 < m_nCOMMENTEND) {
			return true;
		}
	}
	return false;
}

bool Color_Quote::EndColor(const StringRef& cStr, int nPos)
{
	if (m_nCOMMENTEND == -1) {
		// ここにくるのは行頭のはず
		assert_warning(nPos == 0);
		// クォーテーション文字列の終端があるか
		switch (m_nColorTypeIndex) {
		case 0:
			m_nCOMMENTEND = Match_Quote(m_cQuote, nPos, cStr, m_nEscapeType);
			break;
		case 1:
			m_nCOMMENTEND = Match_QuoteStr(m_tag.c_str(), m_tag.size(), nPos, cStr, false);
			break;
		case 2:
			m_nCOMMENTEND = Match_Quote(m_cQuote, nPos, cStr, StringLiteralType::PLSQL);
			break;
		case 3:
			m_nCOMMENTEND = Match_QuoteStr(m_szQuote, 3, nPos, cStr, true);
			break;
		}
		// -1でEndColorが呼び出されるのは行を超えてきたからなので行内チェックは不要
	}
	else if (nPos == m_nCOMMENTEND) {
		return true;
	}
	return false;
}

int Color_Quote::Match_Quote(
	wchar_t wcQuote,
	int nPos,
	const StringRef& lineStr,
	StringLiteralType escapeType,
	bool* pbEscapeEnd
	)
{
	int nCharChars;
	for (int i=nPos; i<lineStr.GetLength(); ++i) {
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = (Int)t_max(LogicInt(1), NativeW::GetSizeOfChar(lineStr.GetPtr(), lineStr.GetLength(), i));
		if (escapeType == StringLiteralType::CPP) {
			// エスケープ \"
			if (nCharChars == 1 && lineStr.At(i) == L'\\') {
				++i;
				if (
					i < lineStr.GetLength()
					&& WCODE::IsLineDelimiter(
						lineStr.At(i),
						GetDllShareData().m_common.m_edit.m_bEnableExtEol
					)
				) {
					if (pbEscapeEnd) {
						*pbEscapeEnd = true;
					}
				}
			}else if (nCharChars == 1 && lineStr.At(i) == wcQuote) {
				return i + 1;
			}
		}else if (escapeType == StringLiteralType::PLSQL) {
			// エスケープ ""
			if (nCharChars == 1 && lineStr.At(i) == wcQuote) {
				if (i + 1 < lineStr.GetLength() && lineStr.At(i + 1) == wcQuote) {
					++i;
				}else {
					return i + 1;
				}
			}
		}else {
			// エスケープなし
			if (nCharChars == 1 && lineStr.At(i) == wcQuote) {
				return i + 1;
			}
		}
		if (nCharChars == 2) {
			++i;
		}
	}
	return lineStr.GetLength() + 1; // 終端なしはLength + 1
}

int Color_Quote::Match_QuoteStr(const wchar_t* pszQuote, int nQuoteLen, int nPos, const StringRef& lineStr, bool bEscape)
{
	int nCharChars;
	const int nCompLen = lineStr.GetLength() - nQuoteLen + 1;
	const WCHAR quote1 = pszQuote[0];
	const WCHAR* pLine = lineStr.GetPtr();
	for (int i=nPos; i<nCompLen; i+=nCharChars) {
		if (quote1 == pLine[i] && wmemcmp(pszQuote + 1, pLine + i + 1, nQuoteLen - 1) == 0) {
			return i + nQuoteLen;
		}
		nCharChars = (Int)t_max(LogicInt(1), NativeW::GetSizeOfChar(pLine, lineStr.GetLength(), i));
		if (bEscape && pLine[i] == L'\\') {
			i += (Int)t_max(LogicInt(1), NativeW::GetSizeOfChar(pLine, lineStr.GetLength(), i + nCharChars));
		}
	}
	return lineStr.GetLength();
}

