#include "StdAfx.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "Color_Quote.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     クォーテーション                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class LayoutColorQuoteInfo : public LayoutColorInfo {
public:
	std::wstring tag;
	int nColorTypeIndex;
	bool IsEqual(const LayoutColorInfo* p) const {
		if (!p) {
			return false;
		}
		const LayoutColorQuoteInfo* info = dynamic_cast<const LayoutColorQuoteInfo*>(p);
		if (!info) {
			return false;
		}
		return info->tag == this->tag;
	}
};

void Color_Quote::Update(void)
{
	const EditDoc* pEditDoc = EditDoc::GetInstance(0);
	pTypeData = &pEditDoc->docType.GetDocumentAttribute();
	nStringType = pTypeData->stringType;
	StringLiteralType nEspaceTypeList[] = {
		StringLiteralType::CPP,
		StringLiteralType::PLSQL,
		StringLiteralType::HTML,
		StringLiteralType::CPP,
		StringLiteralType::CPP,
	};
	nEscapeType = nEspaceTypeList[(int)nStringType];
	bool* pbEscapeEndList[] = {
		&bEscapeEnd,
		NULL,
		NULL,
		NULL,
		&bEscapeEnd,
	};
	pbEscapeEnd = pbEscapeEndList[(int)nStringType];
}

void Color_Quote::SetStrategyColorInfo(const LayoutColorInfo* colorInfo)
{
	if (colorInfo) {
		const LayoutColorQuoteInfo* info = dynamic_cast<const LayoutColorQuoteInfo*>(colorInfo);
		if (!info) {
			return;
		}
		tag = info->tag;
		nColorTypeIndex = info->nColorTypeIndex;
	}else {
		nColorTypeIndex = 0;
	}
}

LayoutColorInfo* Color_Quote::GetStrategyColorInfo() const
{
	if (0 < nColorTypeIndex) {
		LayoutColorQuoteInfo* info = new LayoutColorQuoteInfo();
		info->tag = tag;
		info->nColorTypeIndex = nColorTypeIndex;
		return info;
	}
	return NULL;
}

bool Color_Quote::BeginColor(const StringRef& str, size_t nPos)
{
	if (!str.IsValid()) return false;

	if (str.At(nPos) == cQuote) {
		nCommentEnd = -1;
		StringLiteralType nStringType = pTypeData->stringType;
		bool bPreString = true;
		// クォーテーション文字列の終端があるか
		switch (nStringType) {
		case StringLiteralType::CPP:
			if (
				0 < nPos
				&& str.At(nPos - 1) == 'R'
				&& str.At(nPos) == '"'
				&& nPos + 1 < str.GetLength()
			) {
				for (size_t i=nPos+1; i<str.GetLength(); ++i) {
					if (str.At(i) == '(') {
						if (nPos + 1 < i) {
							tag = L')';
							tag.append(str.GetPtr() + nPos + 1, i - (nPos + 1));
							tag += L'"';
						}else {
							tag.assign(L")\"", 2);
						}
						nCommentEnd = Match_QuoteStr(tag.c_str(), tag.size(), i + 1, str, false);
						nColorTypeIndex = 1;
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
			if (0 < nPos && str.At(nPos - 1) == L'@' && cQuote == L'"') {
				nCommentEnd = Match_Quote(cQuote, nPos + 1, str, StringLiteralType::PLSQL);
				nColorTypeIndex = 2;
				return true;
			}
			break;
		case StringLiteralType::Python:
			if (
				nPos + 2 < str.GetLength()
			 	&& str.At(nPos + 1) == cQuote
			 	&& str.At(nPos + 2) == cQuote
			) {
				nCommentEnd = Match_QuoteStr(szQuote, 3, nPos + 3, str, true);
				nColorTypeIndex = 3;
				return true;
			}
			break;
		}
		bEscapeEnd = false;
		if (bPreString) {
			nCommentEnd = Match_Quote(cQuote, nPos + 1, str, nEscapeType, pbEscapeEnd);
			nColorTypeIndex = 0;
		}

		// 「文字列は行内のみ」(C++ Raw String、Pythonのlong string、@""は特別)
		if (
			pTypeData->bStringLineOnly
			&& !bEscapeEnd
			&& nCommentEnd == str.GetLength()
		) {
			// 終了文字列がない場合は行末までを色分け
			if (pTypeData->bStringEndLine) {
				// 改行コードを除く
				if (
					0 < str.GetLength()
					&& WCODE::IsLineDelimiter(
						str.At(str.GetLength() - 1),
						GetDllShareData().common.edit.bEnableExtEol
					)
				) {
					if (1 &&
						1 < str.GetLength()
						&& str.At(str.GetLength() - 2) == WCODE::CR
						&& str.At(str.GetLength() - 1) == WCODE::LF
					) {
						nCommentEnd = (int)str.GetLength() - 2;
					}else {
						nCommentEnd = (int)str.GetLength() - 1;
					}
				}
				return true;
			}
			// 終了文字列がない場合は色分けしない
			nCommentEnd = -1;
			return false;
		}
		if (0 < nCommentEnd) {
			return true;
		}
	}
	return false;
}

bool Color_Quote::EndColor(const StringRef& str, size_t nPos)
{
	if (nCommentEnd == -1) {
		// ここにくるのは行頭のはず
		assert_warning(nPos == 0);
		// クォーテーション文字列の終端があるか
		switch (nColorTypeIndex) {
		case 0:
			nCommentEnd = Match_Quote(cQuote, nPos, str, nEscapeType);
			break;
		case 1:
			nCommentEnd = Match_QuoteStr(tag.c_str(), tag.size(), nPos, str, false);
			break;
		case 2:
			nCommentEnd = Match_Quote(cQuote, nPos, str, StringLiteralType::PLSQL);
			break;
		case 3:
			nCommentEnd = Match_QuoteStr(szQuote, 3, nPos, str, true);
			break;
		}
		// -1でEndColorが呼び出されるのは行を超えてきたからなので行内チェックは不要
	}
	else if (nPos == nCommentEnd) {
		return true;
	}
	return false;
}

size_t Color_Quote::Match_Quote(
	wchar_t wcQuote,
	int nPos,
	const StringRef& lineStr,
	StringLiteralType escapeType,
	bool* pbEscapeEnd
	)
{
	size_t nCharChars;
	for (size_t i=nPos; i<lineStr.GetLength(); ++i) {
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = t_max((size_t)1, NativeW::GetSizeOfChar(lineStr.GetPtr(), lineStr.GetLength(), i));
		if (escapeType == StringLiteralType::CPP) {
			// エスケープ \"
			if (nCharChars == 1 && lineStr.At(i) == L'\\') {
				++i;
				if (
					i < lineStr.GetLength()
					&& WCODE::IsLineDelimiter(
						lineStr.At(i),
						GetDllShareData().common.edit.bEnableExtEol
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

size_t Color_Quote::Match_QuoteStr(const wchar_t* pszQuote, size_t nQuoteLen, size_t nPos, const StringRef& lineStr, bool bEscape)
{
	size_t nCharChars;
	const size_t nCompLen = lineStr.GetLength() - nQuoteLen + 1;
	const wchar_t quote1 = pszQuote[0];
	const wchar_t* pLine = lineStr.GetPtr();
	for (size_t i=nPos; i<nCompLen; i+=nCharChars) {
		if (quote1 == pLine[i] && wmemcmp(pszQuote + 1, pLine + i + 1, nQuoteLen - 1) == 0) {
			return i + nQuoteLen;
		}
		nCharChars = t_max((size_t)1, NativeW::GetSizeOfChar(pLine, lineStr.GetLength(), i));
		if (bEscape && pLine[i] == L'\\') {
			i += t_max((size_t)1, NativeW::GetSizeOfChar(pLine, lineStr.GetLength(), i + nCharChars));
		}
	}
	return lineStr.GetLength();
}

