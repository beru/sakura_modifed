#include "StdAfx.h"
#include "CodePage.h"
#include "Unicode.h"
#include "charcode.h"
#include "codechecker.h"
#include "dlg/Dialog.h"
#include "util/tchar_convert.h"
#include <algorithm>

typedef BOOL (WINAPI *pfn_GetCPInfoExT_t)(UINT, DWORD, CPINFOEX*);

const DWORD nToWideCharFlags  = 0;
const DWORD nToMultiByteFlags = WC_SEPCHARS;

// 特定コードでは0を指定しないとエラーになる
DWORD GetMultiByteFlgas(UINT codepage){
	switch (codepage) {
	case 4:
	case 50220:
	case 50221:
	case 50222:
	case 50225:
	case 50227:
	case 50229:
	case 57002:
	case 57003:
	case 57004:
	case 57005:
	case 57006:
	case 57007:
	case 57008:
	case 57009:
	case 57010:
	case 57011:
	case 65000:
	case 65001:
	case 54936:
		return 0;
	}
	return nToMultiByteFlags;
}

// 表記：
// EncodingTypeとCP_ACPは値が衝突している。CODE_CPACP != CP_ACP
// charcodeEx: EncodingType + CODE_CPACP + CODE_CPOEM + コードページ
// codepageEx: CODE_CPACP + CODE_CPOEM + コードページ
// codepage: コードページ + CP_ACP, CP_OEMCP
inline UINT CodePageExToMSCP(int codepageEx)
{
	if (codepageEx == CODE_CPACP) {
		return CP_ACP;
	}else if (codepageEx == CODE_CPOEM) {
		return CP_OEMCP;
	}
	return codepageEx;
}

/*!
	CODEPAGE → Unicode 変換関数
	@param pbError NULL許容
	@return 変換した文字列の長さ(wchar_t単位)
*/
CodeConvertResult CodePage::CPToUni(const char* pSrc, const size_t nSrcLen, wchar_t* pDst, size_t nDstCchLen, size_t& nRetLen, UINT codepage)
{
	if (nSrcLen < 1) {
		nRetLen = 0;
		return CodeConvertResult::Complete;
	}
	nRetLen = MultiByteToWideChar2(codepage, nToWideCharFlags, pSrc, nSrcLen, pDst, nDstCchLen);
	if (nRetLen == 0) {
		// ERROR_INSUFFICIENT_BUFFER
		// ERROR_INVALID_FLAGS
		// ERROR_INVALID_PARAMETER
		// ERROR_NO_UNICODE_TRANSLATION
		return CodeConvertResult::Failure;
	}
	return CodeConvertResult::Complete;
}


/*!	CODEPAGE→Unicodeコード変換
*/
CodeConvertResult CodePage::CPToUnicode(const Memory& src, NativeW* pDst, int codepageEx)
{
	// エラー状態
	bool bError = false;

	// ソース取得
	size_t nSrcLen;
	const char* pSrc = reinterpret_cast<const char*>( src.GetRawPtr(&nSrcLen) );

	UINT codepage = CodePageExToMSCP(codepageEx);
	size_t nDstCch = MultiByteToWideChar2(codepage, nToWideCharFlags, pSrc, nSrcLen, NULL, 0);
	// 変換先バッファサイズとその確保
	assert(nDstCch != 0);
	std::vector<wchar_t> dstBuffer(nDstCch);
	wchar_t* pDstBuffer = &dstBuffer[0];

	// 変換
	size_t nDstLen; // cch
	CodeConvertResult ret = CPToUni(pSrc, nSrcLen, pDstBuffer, nDstCch, nDstLen, codepage);
	pDst->_GetMemory()->SetRawDataHoldBuffer(pDstBuffer, nDstLen*sizeof(wchar_t));

	return ret;
}



CodeConvertResult CodePage::UniToCP(const wchar_t* pSrc, const size_t nSrcLen, char* pDst, size_t nDstByteLen, size_t& nRetLen, UINT codepage)
{
	if (nSrcLen < 1) {
		nRetLen = 0;
		return CodeConvertResult::Complete;
	}

	BOOL bDefaultChar = FALSE;
	DWORD flag = GetMultiByteFlgas(codepage);
	nRetLen = WideCharToMultiByte2(codepage, flag, pSrc, nSrcLen, pDst, nDstByteLen);
	if (nRetLen <= 0) {
#ifdef _DEBUG
		DWORD errorCd = GetLastError();
		switch (errorCd) {
		case ERROR_INSUFFICIENT_BUFFER:
			DEBUG_TRACE(_T("UniToCP::ERROR_INSUFFICIENT_BUFFER\n")); break;
		case ERROR_INVALID_FLAGS:
			DEBUG_TRACE(_T("UniToCP::ERROR_INVALID_FLAGS\n")); break;
		case ERROR_INVALID_PARAMETER:
			DEBUG_TRACE(_T("UniToCP::ERROR_INVALID_PARAMETER\n")); break;
//		case ERROR_NO_UNICODE_TRANSLATION:
//			DEBUG_TRACE(_T("ERROR_NO_UNICODE_TRANSLATION")); break;
		default:
			DEBUG_TRACE(_T("UniToCP %x\n"), errorCd); break;
		}
#endif
		return CodeConvertResult::Failure;
	}
	if (bDefaultChar) {
		return CodeConvertResult::LoseSome;
	}
	return CodeConvertResult::Complete;
}


CodeConvertResult CodePage::UnicodeToCP(const NativeW& src, Memory* pDst, int codepageEx)
{
	const wchar_t* pSrc = src.GetStringPtr();
	size_t nSrcLen = src.GetStringLength();
	
	if (nSrcLen == 0) {
		return CodeConvertResult::Complete;
	}
	UINT codepage = CodePageExToMSCP(codepageEx);

	// 必要なバッファサイズを調べてメモリを確保
	// なんだけど、Windows 2000では 50220,50221,50222(ISO-2022-JP系)を使うと値がおかしいことがあるとか
	DWORD flag = GetMultiByteFlgas(codepage);
	size_t nBuffSize = WideCharToMultiByte2(codepage, flag, pSrc, nSrcLen, NULL, 0);
	if (nBuffSize == 0) {
#ifdef _DEBUG
		DWORD errorCd = GetLastError();
		switch (errorCd) {
		case ERROR_INSUFFICIENT_BUFFER:
			DEBUG_TRACE(_T("UnicodeToCP::ERROR_INSUFFICIENT_BUFFER\n")); break;
		case ERROR_INVALID_FLAGS:
			DEBUG_TRACE(_T("UnicodeToCP::ERROR_INVALID_FLAGS\n")); break;
		case ERROR_INVALID_PARAMETER:
			DEBUG_TRACE(_T("UnicodeToCP::ERROR_INVALID_PARAMETER\n")); break;
//		case ERROR_NO_UNICODE_TRANSLATION:
//			DEBUG_TRACE(_T("ERROR_NO_UNICODE_TRANSLATION")); break;
		default:
			DEBUG_TRACE(_T("UnicodeToCP::UniToCP %x\n"), errorCd); break;
		}
#endif
		return CodeConvertResult::Failure;
	}
	assert(nBuffSize != 0);
	std::vector<char> dstBuffer(nBuffSize);
	char* pDstBuffer = &dstBuffer[0];

	// 変換
	size_t nDstLen;
	CodeConvertResult ret = UniToCP(pSrc, nSrcLen, pDstBuffer, nBuffSize, nDstLen, codepage);
	// pMem を更新
	pDst->SetRawDataHoldBuffer(pDstBuffer, nDstLen);
	return ret;
}

void CodePage::GetEol(Memory* pMemEol, EolType eolType)
{
	NativeW temp;
	Unicode().GetEol(temp._GetMemory(), eolType);
	UnicodeToCode(temp, pMemEol);
	NativeW temp2;
	CodeToUnicode(*pMemEol, &temp2);
	// 双方向変換ができる場合だけ設定
	if (!NativeW::IsEqual(temp, temp2)) {
		pMemEol->Clear();
	}
}

void CodePage::GetBom(Memory* pMemBom)
{
	NativeW temp;
	Unicode().GetBom(temp._GetMemory());
	UnicodeToCode(temp, pMemBom);
	NativeW temp2;
	CodeToUnicode(*pMemBom, &temp2);
	if (!NativeW::IsEqual(temp, temp2)) {
		pMemBom->Clear();
	}
}


// 文字コード表示用	UNICODE → Hex 変換
CodeConvertResult CodePage::UnicodeToHex(const wchar_t* cSrc, size_t iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar)
{
	// コードの特性がわからないので何もしない
	return CodeBase::UnicodeToHex(cSrc, iSLen, pDst, psStatusbar);
}

int CodePage::GetNameNormal(LPTSTR outName, int charcodeEx)
{
	if (IsValidCodeType(charcodeEx)) {
		auto_strcpy(outName, CodeTypeName(static_cast<EncodingType>(charcodeEx)).Normal());
		return 1;
	}
	UINT codepage = CodePageExToMSCP(charcodeEx);
	if (codepage == CP_ACP) {
		auto_strcpy(outName, _T("CP_ACP"));
	}else if (codepage == CP_OEMCP) {
		auto_strcpy(outName, _T("CP_OEM"));
	}else {
		auto_sprintf(outName, _T("CP%d"), codepage);
	}
	return 2;
}

int CodePage::GetNameShort(LPTSTR outName, int charcodeEx)
{
	if (IsValidCodeType(charcodeEx)) {
		auto_strcpy(outName, CodeTypeName(static_cast<EncodingType>(charcodeEx)).Short());
		return 1;
	}
	UINT codepage = CodePageExToMSCP(charcodeEx);
	if (codepage == CP_ACP) {
		auto_strcpy(outName, _T("cp_acp"));
	}else if (codepage == CP_OEMCP) {
		auto_strcpy(outName, _T("cp_oem"));
	}else {
		auto_sprintf(outName, _T("cp%d"), codepage);
	}
	return 2;
}

int CodePage::GetNameLong(LPTSTR outName, int charcodeEx)
{
	if (IsValidCodeType(charcodeEx)) {
		auto_strcpy(outName, CodeTypeName(static_cast<EncodingType>(charcodeEx)).Normal());
		return 1;
	}
	UINT codepage = CodePageExToMSCP(charcodeEx);
	if (codepage == CP_ACP) {
		auto_strcpy(outName, _T("CP_ACP"));
	}else if (codepage == CP_OEMCP) {
		auto_strcpy(outName, _T("CP_OEMCP"));
	}else {
		HMODULE hDLLkernel = ::GetModuleHandleA( "kernel32" );
		
		const char* strFunc_GetCPInfoEx = "GetCPInfoExW";
		pfn_GetCPInfoExT_t pfn_GetCPInfoExT = (pfn_GetCPInfoExT_t)::GetProcAddress(hDLLkernel, strFunc_GetCPInfoEx);
		CPINFOEX cpInfo;
		cpInfo.CodePageName[0] = _T('\0');
		if (pfn_GetCPInfoExT && pfn_GetCPInfoExT(codepage, 0, &cpInfo)) {
			auto_strcpy(outName, cpInfo.CodePageName);
		}else {
			auto_sprintf(outName, _T("CP%d"), codepage);
		}
	}
	return 2;
}

int CodePage::GetNameBracket(LPTSTR outName, int charcodeEx)
{
	if (IsValidCodeType(charcodeEx)) {
		auto_strcpy(outName, CodeTypeName(static_cast<EncodingType>(charcodeEx)).Bracket());
		return 1;
	}
	UINT codepage = CodePageExToMSCP(charcodeEx);
	if (codepage == CP_ACP) {
		auto_strcpy(outName, _T("  [CP_ACP]"));
	}else if (codepage == CP_OEMCP) {
		auto_strcpy(outName, _T("  [CP_OEM]"));
	}else {
		auto_sprintf(outName, _T("  [CP%d]"), charcodeEx);
	}
	return 2;
}

EEncodingTrait CodePage::GetEncodingTrait(int charcodeEx)
{
	switch (charcodeEx) {
	case CODE_SJIS:
	case CODE_JIS:
	case CODE_EUC:
	case CODE_LATIN1:
	case CODE_UTF8:
	case CODE_UTF7:
	case CODE_CESU8:
		return ENCODING_TRAIT_ASCII;
	case CODE_UNICODE:
		return ENCODING_TRAIT_UTF16LE;
	case CODE_UNICODEBE:
		return ENCODING_TRAIT_UTF16BE;
	case CODE_CODEMAX:
	case CODE_AUTODETECT:
	case CODE_ERROR:
		return ENCODING_TRAIT_ERROR;
	}

	UINT codepage = CodePageExToMSCP(charcodeEx);

	switch (codepage) {
	case 1200:
		return ENCODING_TRAIT_UTF16BE;
	case 1201:
		return ENCODING_TRAIT_UTF16BE;
	case 12000:
		return ENCODING_TRAIT_UTF32LE;
	case 12001:
		return ENCODING_TRAIT_UTF32BE;
	}
	CHAR testCrlf[10];
	int nRet = ::WideCharToMultiByte(codepage, 0, L"\r\n", 2, testCrlf, sizeof(testCrlf), NULL, NULL);
	switch (nRet) {
	case 2:
		if (memcmp(testCrlf, "\x0d\x25", 2) == 0) {
			wchar_t nel[1] = {0x0085};
			CHAR testNel[10];
			int nRetNel = ::WideCharToMultiByte(codepage, 0, nel, 1, testNel, sizeof(testNel), NULL, NULL);
			if (nRetNel && memcmp(testNel, "\x15", 1) == 0) {
				return ENCODING_TRAIT_EBCDIC;
			}
			return ENCODING_TRAIT_EBCDIC_CRLF;
		}
		if (memcmp(testCrlf, "\r\n", 2) == 0) { return ENCODING_TRAIT_ASCII; }
		return ENCODING_TRAIT_ERROR;
	case 4:
		if (memcmp(testCrlf, "\r\0\n\0", 4) == 0) { return ENCODING_TRAIT_UTF16LE; }
		if (memcmp(testCrlf, "\0\r\0\n", 4) == 0) { return ENCODING_TRAIT_UTF16BE; }
		return ENCODING_TRAIT_ERROR;
	case 8:
		if (memcmp(testCrlf, "\r\0\0\0\n\0\0\0", 8) == 0) { return ENCODING_TRAIT_UTF32LE; }
		if (memcmp(testCrlf, "\0\0\0\r\0\0\0\n", 8) == 0) { return ENCODING_TRAIT_UTF32BE; }
		return ENCODING_TRAIT_ERROR;
	default:
		return ENCODING_TRAIT_ERROR;
	}
}

volatile CodePage::CodePageList* s_list = nullptr;

struct sortByCodePage{
	bool operator() (const std::pair<int, std::wstring>& left, const std::pair<int, std::wstring>& right) const {
		return left.first < right.first;
	}
};

// static
CodePage::CodePageList& CodePage::GetCodePageList()
{
	static CodePage::CodePageList result;
	// マルチスレッド:s_listにロックが必要
	if (s_list) {
		return result;
	}
	result.clear();
	s_list = &result;
	if (!::EnumSystemCodePages(reinterpret_cast<CODEPAGE_ENUMPROC>(CallBackEnumCodePages), CP_INSTALLED)) {
		return result;
	}
	s_list = nullptr;

	// 名前を取得
	// GetCPInfoEx 98, 2000以上
	HMODULE hDLLkernel = ::GetModuleHandleA("kernel32");
	
	const char* strFunc_GetCPInfoEx = "GetCPInfoExW";
	pfn_GetCPInfoExT_t pfn_GetCPInfoExT = (pfn_GetCPInfoExT_t)::GetProcAddress(hDLLkernel, strFunc_GetCPInfoEx);
	CPINFOEX cpInfo;
	for (auto it=result.begin(); it!=result.end(); ++it) {
		cpInfo.CodePageName[0] = _T('\0');
		if (pfn_GetCPInfoExT && pfn_GetCPInfoExT(it->first, 0, &cpInfo)) {
			it->second = to_wchar(cpInfo.CodePageName);
		}else {
			std::wstring code = it->second;
			it->second += L" CP(";
			it->second += code;
			it->second += L")";
		}
	}
	// 独自実装部分を定義
	result.emplace_back(12000, L"12000 (UTF-32LE)");
	result.emplace_back(12001, L"12001 (UTF-32BE)");

	std::sort(result.begin(),result.end(), sortByCodePage());
	return result;
}

//static
BOOL CALLBACK CodePage::CallBackEnumCodePages( LPCTSTR pCodePageString )
{
	// pCodePageString は文字列に格納された数字
	CodePage::CodePageList* pList = const_cast<CodePage::CodePageList*>(s_list);
	pList->emplace_back(_ttoi( pCodePageString ), to_wchar(pCodePageString));
	return TRUE;
}

int CodePage::AddComboCodePages(HWND hwnd, HWND combo, int nSelCode)
{
	LRESULT nSel = -1;
	LRESULT nIdx = Combo_AddString(combo, _T("CP_ACP"));
	ASSERT_GE(nIdx, 0);
	Combo_SetItemData(combo, nIdx, CODE_CPACP);
	if (nSelCode == CODE_CPACP) {
		Combo_SetCurSel(combo, nIdx);
		nSel = nIdx;
	}
	nIdx = Combo_AddString(combo, _T("CP_OEM"));
	if (nSelCode == CODE_CPOEM) {
		Combo_SetCurSel(combo, nIdx);
		nSel = nIdx;
	}
	Combo_SetItemData(combo, nIdx, CODE_CPOEM);
	CodePage::CodePageList& cpList = CodePage::GetCodePageList();
	for (auto it=cpList.begin(); it!=cpList.end(); ++it) {
		nIdx = Combo_AddString(combo, it->second.c_str());
		Combo_SetItemData(combo, nIdx, it->first);
		if (nSelCode == it->first) {
			Combo_SetCurSel(combo, nIdx);
			nSel = nIdx;
		}
	}
	// 幅を変更
	Dialog::OnCbnDropDown(combo, true);
	return (int)nSel;
}

size_t CodePage::MultiByteToWideChar2(UINT codepage, int flags, const char* pSrc, size_t nSrcLen, wchar_t* pDst, size_t nDstLen)
{
	if (codepage == 12000) {
		return S_UTF32LEToUnicode(pSrc, nSrcLen, pDst, nDstLen);
	}else if (codepage == 12001) {
		return S_UTF32BEToUnicode(pSrc, nSrcLen, pDst, nDstLen);
	}
	return MultiByteToWideChar(codepage, flags, pSrc, (int)nSrcLen, pDst, (int)nDstLen);
}
size_t CodePage::WideCharToMultiByte2(UINT codepage, int flags, const wchar_t* pSrc, size_t nSrcLen, char* pDst, size_t nDstLen)
{
	if (codepage == 12000) {
		return S_UnicodeToUTF32LE(pSrc, nSrcLen, pDst, nDstLen);
	}else if (codepage == 12001) {
		return S_UnicodeToUTF32BE(pSrc, nSrcLen, pDst, nDstLen);
	}
	int ret = ::WideCharToMultiByte(codepage, flags, pSrc, (int)nSrcLen, pDst, (int)nDstLen, NULL, NULL);
	if (ret == 0 && nSrcLen != 0) {
		DWORD errorCd = GetLastError();
		if (errorCd == ERROR_INVALID_FLAGS) {
			// flagsを0にして再挑戦
			ret = ::WideCharToMultiByte(codepage, 0, pSrc, (int)nSrcLen, pDst, (int)nDstLen, NULL, NULL);
		}
	}
	return ret;
}

size_t CodePage::S_UTF32LEToUnicode(const char* pSrc, size_t nSrcLen, wchar_t* pDst, size_t nDstLen)
{
	const unsigned char* pSrcByte = reinterpret_cast<const unsigned char*>(pSrc);
	if (!pDst) {
		size_t nDstUseLen = 0;
		for (size_t i=0; i<nSrcLen; ) {
			if (i + 3 < nSrcLen) {
				if (pSrcByte[i+3] == 0x00 && pSrcByte[i+2] == 0x00) {
					wchar_t c = static_cast<wchar_t>(pSrcByte[i+1] << 8 | pSrcByte[i]);
					if (IsUtf16SurrogHi(c) || IsUtf16SurrogLow(c)) {
						// サロゲート断片。バイトごとに出力する)
						nDstUseLen += 4;
					}else {
						++nDstUseLen;
					}
				}else if (pSrcByte[i+3] == 0x00 && pSrcByte[i+2] <= 0x10) {
					nDstUseLen += 2;
				}else {
					// UCS-4(UTF-16範囲外。バイトごとに出力する)
					nDstUseLen += 4;
				}
				i += 4;
			}else {
				nDstUseLen += nSrcLen - i;
				i += nSrcLen - i;
			}
		}
		return nDstUseLen;
	}
	size_t nDstUseLen = 0;
	size_t nDstUseCharLen;
	unsigned short* pDstShort = reinterpret_cast<unsigned short*>(pDst);
	for (size_t i=0; i<nSrcLen; ) {
		if (i + 3 < nSrcLen) {
			if (pSrcByte[i+3] == 0x00 && pSrcByte[i+2] == 0x00) {
				wchar_t c = static_cast<wchar_t>(pSrcByte[i+1] << 8 | pSrcByte[i]);
				if (IsUtf16SurrogHi(c) || IsUtf16SurrogLow(c)) {
					nDstUseCharLen = 4;
					if (nDstUseLen + nDstUseCharLen <= nDstLen) {
						BinToText(pSrcByte + i, nDstUseCharLen, pDstShort + nDstUseLen);
					}else {
						return 0;
					}
				}else {
					nDstUseCharLen = 1;
					if (nDstUseLen + nDstUseCharLen <= nDstLen) {
						pDst[nDstUseLen] = c;
					}else {
						return 0;
					}
				}
				nDstUseLen += nDstUseCharLen;
				i += 4;
			}else if (pSrcByte[i+3] == 0x00 && pSrcByte[i+2] <= 0x10) {
				nDstUseCharLen = 2;
				if (nDstUseLen + nDstUseCharLen <= nDstLen) {
					UINT c = (pSrcByte[i+2] << 16) | (pSrcByte[i+1] << 8) | pSrcByte[i];
					UINT x = c - 0x10000;
					pDst[nDstUseLen]   = static_cast<wchar_t>(0xd800 | (x >> 10));
					pDst[nDstUseLen+1] = static_cast<wchar_t>(0xdc00 | (x & 0x3ff));
					nDstUseLen += nDstUseCharLen;
				}else {
					return 0;
				}
				i+=4;
			}else {
				// UCS-4(UTF-16範囲外。バイトごとに出力する)
				nDstUseCharLen = 4;
				if (nDstUseLen + nDstUseCharLen <= nDstLen) {
					BinToText(pSrcByte +i, nDstUseCharLen, pDstShort + nDstUseLen);
					nDstUseLen += nDstUseCharLen;
				}else {
					return 0;
				}
			}
		}else {
			nDstUseCharLen = nSrcLen - i;
			if (nDstUseLen + nDstUseCharLen <= nDstLen) {
				BinToText(pSrcByte + i, nDstUseCharLen, pDstShort + nDstUseLen);
				nDstUseLen += nDstUseCharLen;
			}else {
				return 0;
			}
			i += nSrcLen - i;
		}
	}
	return nDstUseLen;
}

size_t CodePage::S_UTF32BEToUnicode(const char* pSrc, size_t nSrcLen, wchar_t* pDst, size_t nDstLen)
{
	const unsigned char* pSrcByte = reinterpret_cast<const unsigned char*>(pSrc);
	if (!pDst) {
		size_t nDstUseLen = 0;
		for (size_t i=0; i<nSrcLen; ) {
			if (i + 3 < nSrcLen) {
				if (pSrcByte[i+1] == 0x00 && pSrcByte[i] == 0x00) {
					wchar_t c = static_cast<wchar_t>(pSrcByte[i+1] << 8 | pSrcByte[i]);
					if (IsUtf16SurrogHi(c) || IsUtf16SurrogLow(c)) {
						// サロゲート断片。バイトごとに出力する)
						nDstUseLen += 4;
					}else {
						++nDstUseLen;
					}
				}else if (pSrcByte[i+1] <= 0x10 && pSrcByte[i] == 0x00) {
					nDstUseLen += 2;
				}else {
					// UCS-4(UTF-16範囲外。バイトごとに出力する)
					nDstUseLen += 4;
				}
				i+=4;
			}else {
				nDstUseLen += nSrcLen - i;
				i += nSrcLen - i;
			}
		}
		return nDstUseLen;
	}
	size_t nDstUseLen = 0;
	size_t nDstUseCharLen;
	size_t i = 0;
	unsigned short* pDstShort = reinterpret_cast<unsigned short*>(pDst);
	for (; i<nSrcLen; ) {
		if (i + 3 < nSrcLen) {
			if (pSrcByte[i] == 0x00 && pSrcByte[i+1] == 0x00) {
				wchar_t c = static_cast<wchar_t>(pSrcByte[i+1] << 8 | pSrcByte[i]);
				if (IsUtf16SurrogHi(c) || IsUtf16SurrogLow(c)) {
					nDstUseCharLen = 4;
					if (nDstUseLen + nDstUseCharLen <= nDstLen) {
						BinToText(pSrcByte +i, nDstUseCharLen, pDstShort + nDstUseLen);
					}else {
						return 0;
					}
				}else {
					nDstUseCharLen = 1;
					if (nDstUseLen + nDstUseCharLen <= nDstLen) {
						pDst[nDstUseLen] = c;
					}else {
						return 0;
					}
				}
				nDstUseLen += nDstUseCharLen;
			}else if (pSrcByte[i] == 0x00 && pSrcByte[i+1] <= 0x10) {
				nDstUseCharLen = 2;
				if (nDstUseLen + nDstUseCharLen <= nDstLen) {
					UINT c = (pSrcByte[i+3] << 16) | (pSrcByte[i+2] << 8) | pSrcByte[i+1];
					UINT x = c - 0x10000;
					pDst[nDstUseLen]   = static_cast<wchar_t>(0xd800 | (x >> 10));
					pDst[nDstUseLen+1] = static_cast<wchar_t>(0xdc00 | (x & 0x3ff));
					nDstUseLen += nDstUseCharLen;
				}else {
					return 0;
				}
			}else {
				// UCS-4(UTF-16範囲外。バイトごとに出力する)
				nDstUseCharLen = 4;
				if (nDstUseLen + nDstUseCharLen <= nDstLen) {
					BinToText(pSrcByte + i, nDstUseCharLen, pDstShort + nDstUseLen);
					nDstUseLen += nDstUseCharLen;
				}else {
					return 0;
				}
			}
			i += 4;
		}else {
			ASSERT_GE(nSrcLen, i);
			nDstUseCharLen = nSrcLen - i;
			if (nDstUseLen + nDstUseCharLen <= nDstLen) {
				BinToText(pSrcByte + i, nDstUseCharLen, pDstShort + nDstUseLen);
				nDstUseLen += nDstUseCharLen;
			}else {
				return 0;
			}
			i += nSrcLen - i;
		}
	}
	return nDstUseLen;
}

static bool BinToUTF32(const unsigned short* pSrc, size_t len, char* pDst, size_t nDstLen)
{
	if (4 <= nDstLen) {
		for (size_t i=0; i<len; ++i) {
			pDst[i] = CodeBase::TextToBin(pSrc[i]);
		}
		for (size_t k=len; k<4; ++k) {
			pDst[k] = 0;
		}
	}else {
		return false;
	}
	return true;
}

size_t CodePage::S_UnicodeToUTF32LE( const wchar_t* pSrc, size_t nSrcLen, char* pDst, size_t nDstLen )
{
	if (!pDst) {
		int nDstUseLen = 0;
		int nBinaryLen = 0;
		for (size_t i=0; i<nSrcLen; ) {
			// 4Byteに満たないバイナリデータは4Byte単位で出力
			if (i + 1 < nSrcLen && IsUtf16SurrogHi( pSrc[i] ) && IsUtf16SurrogLow( pSrc[i+1] )) {
				if (nBinaryLen != 0) {
					nDstUseLen += 4;
					nBinaryLen = 0;
				}
				nDstUseLen += 4;
				i += 2;
			}else if (IsBinaryOnSurrogate( pSrc[i] )) {
				if (nBinaryLen == 4) {
					nDstUseLen += 4;
					nBinaryLen = 0;
				}else {
					++nBinaryLen;
				}
				++i;
			}else {
				if (nBinaryLen != 0) {
					nDstUseLen += 4;
					nBinaryLen = 0;
				}
				nDstUseLen += 4;
				++i;
			}
		}
		if (nBinaryLen != 0) {
			// 末尾のあまりは、そのバイト数のみ出力
			nDstUseLen += nBinaryLen;
		}
		return nDstUseLen;
	}
	size_t nDstUseLen = 0;
	size_t nDstUseCharLen;
	size_t nBinaryPos;
	size_t nBinaryLen = 0;
	const unsigned short* pSrcShort = reinterpret_cast<const unsigned short*>(pSrc);
	unsigned char* pDstByte = reinterpret_cast<unsigned char*>(pDst);
	for (size_t i=0; i<nSrcLen; ) {
		if (i + 1 < nSrcLen && IsUtf16SurrogHi( pSrc[i] ) && IsUtf16SurrogLow( pSrc[i+1] )) {
			if (nBinaryLen != 0) {
				if (!BinToUTF32(pSrcShort + nBinaryPos, nBinaryLen, pDst + nDstUseLen, nDstLen - nDstUseLen)) {
					return 0;
				}
				nBinaryLen = 0;
				nDstUseLen += 4;
			}
			wchar32_t c = DecodeUtf16Surrog( pSrcShort[i], pSrcShort[i+1] );
			nDstUseCharLen = 4;
			if (nDstUseLen + nDstUseCharLen <= nDstLen) {
				pDstByte[nDstUseLen+3] = 0;
				pDstByte[nDstUseLen+2] = static_cast<unsigned char>(c >> 16);
				pDstByte[nDstUseLen+1] = static_cast<unsigned char>(c >> 8);
				pDstByte[nDstUseLen]   = static_cast<unsigned char>(c);
			}else {
				return 0;
			}
			nDstUseLen += nDstUseCharLen;
			i += 2;
		}else if (IsBinaryOnSurrogate( pSrc[i] )) {
			if (nBinaryLen == 4) {
				if (!BinToUTF32(pSrcShort + nBinaryPos, nBinaryLen, pDst + nDstUseLen, nDstLen - nDstUseLen)) {
					return 0;
				}
				nBinaryLen = 0;
				nDstUseLen += 4;
			}else if (nBinaryLen == 0) {
				nBinaryPos = i;
				++nBinaryLen;
			}else {
				++nBinaryLen;
			}
			++i;
		}else {
			if (nBinaryLen != 0) {
				if (!BinToUTF32(pSrcShort + nBinaryPos, nBinaryLen, pDst + nDstUseLen, nDstLen - nDstUseLen)) {
					return 0;
				}
				nBinaryLen = 0;
				nDstUseLen += 4;
			}
			nDstUseCharLen = 4;
			if (nDstUseLen + nDstUseCharLen <= nDstLen) {
				pDstByte[nDstUseLen+3] = 0;
				pDstByte[nDstUseLen+2] = 0;
				pDstByte[nDstUseLen+1] = static_cast<unsigned char>(pSrc[i] >> 8);
				pDstByte[nDstUseLen]   = static_cast<unsigned char>(pSrc[i]);
			}
			nDstUseLen += nDstUseCharLen;
			++i;
		}
	}
	// ファイル末尾の端数は、そのままの長さで出力
	if (nBinaryLen != 0) {
		if (nDstUseLen + nBinaryLen <= nDstLen) {
			for (size_t k=0; k<nBinaryLen; ++k) {
				pDstByte[nDstUseLen + k] = TextToBin(pSrcShort[nBinaryPos]);
			}
		}else {
			return 0;
		}
		nBinaryLen = 0;
		nDstUseLen += nBinaryLen;
	}
	return nDstUseLen;
}

size_t CodePage::S_UnicodeToUTF32BE(const wchar_t* pSrc, size_t nSrcLen, char* pDst, size_t nDstLen)
{
	if (!pDst) {
		size_t nDstUseLen = 0;
		size_t nBinaryLen = 0;
		for (size_t i=0; i<nSrcLen; ) {
			// 4Byteに満たないバイナリデータは4Byte単位で出力
			if (i + 1 < nSrcLen && IsUtf16SurrogHi( pSrc[i] ) && IsUtf16SurrogLow( pSrc[i+1] )) {
				if (nBinaryLen != 0) {
					nDstUseLen += 4;
					nBinaryLen = 0;
				}
				nDstUseLen += 4;
				i += 2;
			}else if (IsBinaryOnSurrogate( pSrc[i] )) {
				if (nBinaryLen == 4) {
					nDstUseLen += 4;
					nBinaryLen = 0;
				}else {
					++nBinaryLen;
				}
				++i;
			}else {
				if (nBinaryLen != 0) {
					nDstUseLen += 4;
					nBinaryLen = 0;
				}
				nDstUseLen += 4;
				++i;
			}
		}
		if (nBinaryLen != 0) {
			// 末尾のあまりは、そのバイト数のみ出力
			nDstUseLen += nBinaryLen;
		}
		return nDstUseLen;
	}
	size_t nDstUseLen = 0;
	size_t nDstUseCharLen;
	size_t nBinaryPos;
	size_t nBinaryLen = 0;
	const unsigned short* pSrcShort = reinterpret_cast<const unsigned short*>(pSrc);
	unsigned char* pDstByte = reinterpret_cast<unsigned char*>(pDst);
	for (size_t i=0; i<nSrcLen; ) {
		if (i + 1 < nSrcLen && IsUtf16SurrogHi( pSrc[i] ) && IsUtf16SurrogLow( pSrc[i+1] )) {
			if (nBinaryLen != 0) {
				if (!BinToUTF32(pSrcShort + nBinaryPos, nBinaryLen, pDst + nDstUseLen, nDstLen - nDstUseLen)) {
					return 0;
				}
				nBinaryLen = 0;
				nDstUseLen += 4;
			}
			wchar32_t c = DecodeUtf16Surrog( pSrcShort[i], pSrcShort[i+1] );
			nDstUseCharLen = 4;
			if (nDstUseLen + nDstUseCharLen <= nDstLen) {
				pDstByte[nDstUseLen] = 0;
				pDstByte[nDstUseLen+1] = static_cast<unsigned char>(c >> 16);
				pDstByte[nDstUseLen+2] = static_cast<unsigned char>(c >> 8);
				pDstByte[nDstUseLen+3] = static_cast<unsigned char>(c);
			}else {
				return 0;
			}
			nDstUseLen += nDstUseCharLen;
			i += 2;
		}else if (IsBinaryOnSurrogate( pSrc[i] )) {
			if (nBinaryLen == 4) {
				if (!BinToUTF32(pSrcShort + nBinaryPos, nBinaryLen, pDst + nDstUseLen, nDstLen - nDstUseLen)) {
					return 0;
				}
				nBinaryLen = 0;
				nDstUseLen += 4;
			}else if (nBinaryLen == 0) {
				nBinaryPos = i;
				++nBinaryLen;
			}else {
				++nBinaryLen;
			}
			++i;
		}else {
			if (nBinaryLen != 0) {
				if (!BinToUTF32(pSrcShort + nBinaryPos, nBinaryLen, pDst + nDstUseLen, nDstLen - nDstUseLen)) {
					return 0;
				}
				nBinaryLen = 0;
				nDstUseLen += 4;
			}
			nDstUseCharLen = 4;
			if (nDstUseLen + nDstUseCharLen <= nDstLen) {
				pDstByte[nDstUseLen] = 0;
				pDstByte[nDstUseLen+1] = 0;
				pDstByte[nDstUseLen+2] = static_cast<unsigned char>(pSrc[i] >> 8);
				pDstByte[nDstUseLen+3]   = static_cast<unsigned char>(pSrc[i]);
			}
			nDstUseLen += nDstUseCharLen;
			++i;
		}
	}
	// ファイル末尾の端数は、そのままの長さで出力
	if (nBinaryLen != 0) {
		if (nDstUseLen + nBinaryLen <= nDstLen) {
			for (size_t k=0; k<nBinaryLen; ++k) {
				pDstByte[nDstUseLen + k] = TextToBin(pSrcShort[nBinaryPos]);
			}
		}else {
			return 0;
		}
		nBinaryLen = 0;
		nDstUseLen += nBinaryLen;
	}
	return nDstUseLen;
}
