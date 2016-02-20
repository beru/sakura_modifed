#include "StdAfx.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "Color_Quote.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �N�H�[�e�[�V����                        //
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
	m_nStringType = m_pTypeData->stringType;
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

// nPos "�̈ʒu
//staic
bool Color_Quote::IsCppRawString(const StringRef& str, int nPos)
{
	if (
		0 < nPos
		&& str.At(nPos-1) == 'R'
		&& str.At(nPos) == '"'
		&& nPos + 1 < str.GetLength()
	) {
		// \b(u8|u|U|L|)R"[^(]*\(
		// \b = ^|[\s!"#$%&'()=@{};:<>?,.*/\-\+\[\]\]
		wchar_t c1 = L' ';
		if (2 <= nPos) {
			c1 = str.At(nPos-2);
		}
		wchar_t c2 = L' ';
		if (3 <= nPos) {
			c2 = str.At(nPos-3);
		}
		const wchar_t* pszSep = L" \t!\"#$%&'()=@{};:<>?,.*/-+[]";
		if ((c1 == 'u' || c1 == 'U' || c1 == 'L')) {
			if (wcschr(pszSep, c2)) {
				return true;
			}
		}else if (c1 == '8' && c2 == 'u') {
			wchar_t c3 = L'\0';
			if (4 <= nPos) {
				c3 = str.At(nPos-4);
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

bool Color_Quote::BeginColor(const StringRef& str, int nPos)
{
	if (!str.IsValid()) return false;

	if (str.At(nPos) == m_cQuote) {
		m_nCOMMENTEND = -1;
		StringLiteralType nStringType = m_pTypeData->stringType;
		bool bPreString = true;
		// �N�H�[�e�[�V����������̏I�[�����邩
		switch (nStringType) {
		case StringLiteralType::CPP:
			if (
				0 < nPos
				&& str.At(nPos - 1) == 'R'
				&& str.At(nPos) == '"'
				&& nPos + 1 < str.GetLength()
			) {
				for (int i=nPos+1; i<str.GetLength(); ++i) {
					if (str.At(i) == '(') {
						if (nPos + 1 < i) {
							m_tag = L')';
							m_tag.append(str.GetPtr() + nPos + 1, i - (nPos + 1));
							m_tag += L'"';
						}else {
							m_tag.assign(L")\"", 2);
						}
						m_nCOMMENTEND = Match_QuoteStr(m_tag.c_str(), m_tag.size(), i + 1, str, false);
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
					if (str.At(i) != L' ' && str.At(i) != L'\t') {
						break;
					}
				}
				if (!(0 <= i && str.At(i) == L'=')) {
					bPreString = false;
				}
			}
			break;
		case StringLiteralType::CSharp:
			if (0 < nPos && str.At(nPos - 1) == L'@' && m_cQuote == L'"') {
				m_nCOMMENTEND = Match_Quote(m_cQuote, nPos + 1, str, StringLiteralType::PLSQL);
				m_nColorTypeIndex = 2;
				return true;
			}
			break;
		case StringLiteralType::Python:
			if (
				nPos + 2 < str.GetLength()
			 	&& str.At(nPos + 1) == m_cQuote
			 	&& str.At(nPos + 2) == m_cQuote
			) {
				m_nCOMMENTEND = Match_QuoteStr(m_szQuote, 3, nPos + 3, str, true);
				m_nColorTypeIndex = 3;
				return true;
			}
			break;
		}
		m_bEscapeEnd = false;
		if (bPreString) {
			m_nCOMMENTEND = Match_Quote(m_cQuote, nPos + 1, str, m_nEscapeType, m_pbEscapeEnd);
			m_nColorTypeIndex = 0;
		}

		// �u������͍s���̂݁v(C++ Raw String�APython��long string�A@""�͓���)
		if (
			m_pTypeData->bStringLineOnly
			&& !m_bEscapeEnd
			&& m_nCOMMENTEND == str.GetLength()
		) {
			// �I�������񂪂Ȃ��ꍇ�͍s���܂ł�F����
			if (m_pTypeData->bStringEndLine) {
				// ���s�R�[�h������
				if (
					0 < str.GetLength()
					&& WCODE::IsLineDelimiter(
						str.At(str.GetLength() - 1),
						GetDllShareData().m_common.edit.bEnableExtEol
					)
				) {
					if (1 &&
						1 < str.GetLength()
						&& str.At(str.GetLength() - 2) == WCODE::CR
						&& str.At(str.GetLength() - 1) == WCODE::LF
					) {
						m_nCOMMENTEND = str.GetLength() - 2;
					}else {
						m_nCOMMENTEND = str.GetLength() - 1;
					}
				}
				return true;
			}
			// �I�������񂪂Ȃ��ꍇ�͐F�������Ȃ�
			m_nCOMMENTEND = -1;
			return false;
		}
		if (0 < m_nCOMMENTEND) {
			return true;
		}
	}
	return false;
}

bool Color_Quote::EndColor(const StringRef& str, int nPos)
{
	if (m_nCOMMENTEND == -1) {
		// �����ɂ���͍̂s���̂͂�
		assert_warning(nPos == 0);
		// �N�H�[�e�[�V����������̏I�[�����邩
		switch (m_nColorTypeIndex) {
		case 0:
			m_nCOMMENTEND = Match_Quote(m_cQuote, nPos, str, m_nEscapeType);
			break;
		case 1:
			m_nCOMMENTEND = Match_QuoteStr(m_tag.c_str(), m_tag.size(), nPos, str, false);
			break;
		case 2:
			m_nCOMMENTEND = Match_Quote(m_cQuote, nPos, str, StringLiteralType::PLSQL);
			break;
		case 3:
			m_nCOMMENTEND = Match_QuoteStr(m_szQuote, 3, nPos, str, true);
			break;
		}
		// -1��EndColor���Ăяo�����͍̂s�𒴂��Ă�������Ȃ̂ōs���`�F�b�N�͕s�v
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
			// �G�X�P�[�v \"
			if (nCharChars == 1 && lineStr.At(i) == L'\\') {
				++i;
				if (
					i < lineStr.GetLength()
					&& WCODE::IsLineDelimiter(
						lineStr.At(i),
						GetDllShareData().m_common.edit.bEnableExtEol
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
			// �G�X�P�[�v ""
			if (nCharChars == 1 && lineStr.At(i) == wcQuote) {
				if (i + 1 < lineStr.GetLength() && lineStr.At(i + 1) == wcQuote) {
					++i;
				}else {
					return i + 1;
				}
			}
		}else {
			// �G�X�P�[�v�Ȃ�
			if (nCharChars == 1 && lineStr.At(i) == wcQuote) {
				return i + 1;
			}
		}
		if (nCharChars == 2) {
			++i;
		}
	}
	return lineStr.GetLength() + 1; // �I�[�Ȃ���Length + 1
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

