#include "StdAfx.h"
#include <string>
#include <mbstring.h>
#include "mem/NativeA.h"
#include "Eol.h"
#include "charset/ShiftJis.h"
#include "charset/charcode.h"
#include "util/string_ex2.h"

NativeA::NativeA(const char* szData)
	:
	Native()
{
	SetString(szData);
}

NativeA::NativeA()
	:
	Native()
{
}

NativeA::NativeA(const NativeA& rhs)
	:
	Native()
{
	SetString(rhs.GetStringPtr(), rhs.GetStringLength());
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ設定インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// バッファの内容を置き換える
void NativeA::SetString(const char* pszData)
{
	SetString(pszData, strlen(pszData));
}

// バッファの内容を置き換える。nLenは文字単位。
void NativeA::SetString(const char* pData, int nDataLen)
{
	int nDataLenBytes = nDataLen * sizeof(char);
	Native::SetRawData(pData, nDataLenBytes);
}

// バッファの内容を置き換える
void NativeA::SetNativeData(const NativeA& pcNative)
{
	Native::SetRawData(pcNative);
}

// バッファの最後にデータを追加する
void NativeA::AppendString(const char* pszData)
{
	AppendString(pszData, strlen(pszData));
}

// バッファの最後にデータを追加する。nLengthは文字単位。
void NativeA::AppendString(const char* pszData, size_t nLength)
{
	Native::AppendRawData(pszData, nLength * sizeof(char));
}

const NativeA& NativeA::operator = (char cChar)
{
	char pszChar[2];
	pszChar[0] = cChar;
	pszChar[1] = '\0';
	SetRawData(pszChar, 1);
	return *this;
}

// バッファの最後にデータを追加する
void NativeA::AppendNativeData(const NativeA& pcNative)
{
	AppendString(pcNative.GetStringPtr(), pcNative.GetStringLength());
}

// (重要：nDataLenは文字単位) バッファサイズの調整。必要に応じて拡大する。
void NativeA::AllocStringBuffer(size_t nDataLen)
{
	Native::AllocBuffer(nDataLen * sizeof(char));
}

const NativeA& NativeA::operator += (char ch)
{
	char szChar[2] = {ch, '\0'};
	AppendString(szChar);
	return *this;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           互換                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void NativeA::SetStringNew(
	const wchar_t* wszData,
	size_t nDataLen
	)
{
	std::wstring buf(wszData, nDataLen); // 切り出し
	char* tmp = wcstombs_new(buf.c_str());
	SetString(tmp);
	delete[] tmp;
}

void NativeA::SetStringNew(const wchar_t* wszData)
{
	SetStringNew(wszData, wcslen(wszData));
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ取得インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

size_t NativeA::GetStringLength() const
{
	return Native::GetRawLength() / sizeof(char);
}

const char* NativeA::GetStringPtr(size_t* pnLength) const
{
	if (pnLength) {
		*pnLength = GetStringLength();
	}
	return GetStringPtr();
}

// 任意位置の文字取得。nIndexは文字単位。
char NativeA::operator[](size_t nIndex) const
{
	if (nIndex < GetStringLength()) {
		return GetStringPtr()[nIndex];
	}else {
		return 0;
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ変換インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 文字列置換
void NativeA::Replace(
	const char* pszFrom,
	const char* pszTo
	)
{
	NativeA	memWork;
	size_t nFromLen = strlen(pszFrom);
	size_t nToLen = strlen(pszTo);
	size_t nBgnOld = 0;
	size_t nBgn = 0;
	ASSERT_GE(GetStringLength(), nFromLen);
	while (nBgn <= GetStringLength() - nFromLen) {
		if (auto_memcmp(&GetStringPtr()[nBgn], pszFrom, nFromLen) == 0) {
			if (0 < nBgn - nBgnOld) {
				memWork.AppendString(&GetStringPtr()[nBgnOld], nBgn - nBgnOld);
			}
			memWork.AppendString(pszTo, nToLen);
			nBgn += nFromLen;
			nBgnOld = nBgn;
		}else {
			++nBgn;
		}
	}
	if (0 < GetStringLength() - nBgnOld) {
		memWork.AppendString(&GetStringPtr()[nBgnOld], GetStringLength() - nBgnOld);
	}
	SetNativeData(memWork);
	return;
}

// 文字列置換（日本語考慮版）
void NativeA::Replace_j(
	const char* pszFrom,
	const char* pszTo
	)
{
	NativeA	memWork;
	size_t nFromLen = strlen(pszFrom);
	size_t nToLen = strlen(pszTo);
	size_t nBgnOld = 0;
	size_t nBgn = 0;
	ASSERT_GE(GetStringLength(), nFromLen);
	while (nBgn <= GetStringLength() - nFromLen) {
		if (memcmp(&GetStringPtr()[nBgn], pszFrom, nFromLen) == 0) {
			if (0 < nBgn - nBgnOld) {
				memWork.AppendString(&GetStringPtr()[nBgnOld], nBgn - nBgnOld);
			}
			memWork.AppendString(pszTo, nToLen);
			nBgn += nFromLen;
			nBgnOld = nBgn;
		}else {
			if (_IS_SJIS_1((unsigned char)GetStringPtr()[nBgn])) {
				++nBgn;
			}
			++nBgn;
		}
	}
	if (0  < GetStringLength() - nBgnOld) {
		memWork.AppendString(&GetStringPtr()[nBgnOld], GetStringLength() - nBgnOld);
	}
	SetNativeData(memWork);
	return;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   一般インターフェース                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  staticインターフェース                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 指定した位置の文字が何バイト文字かを返す
size_t NativeA::GetSizeOfChar(
	const char* pData,
	size_t nDataLen,
	size_t nIdx
	)
{
	return ShiftJis::GetSizeOfChar(pData, nDataLen, nIdx);
}

// ポインタで示した文字の次にある文字の位置を返します
// 次にある文字がバッファの最後の位置を越える場合は&pData[nDataLen]を返します
const char* NativeA::GetCharNext(
	const char* pData,
	size_t nDataLen,
	const char* pDataCurrent
	)
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer("Memory::MemCharNext");
//#endif

	const char*	pNext;
	if (pDataCurrent[0] == '\0') {
		pNext = pDataCurrent + 1;
	}else {
//		pNext = ::CharNext(pDataCurrent);
		if (
			// SJIS全角コードの1バイト目か	// Sept. 1, 2000 jepro 'シフト'を'S'に変更
			_IS_SJIS_1((unsigned char)pDataCurrent[0])
			&&
			// SJIS全角コードの2バイト目か	// Sept. 1, 2000 jepro 'シフト'を'S'に変更
			_IS_SJIS_2((unsigned char)pDataCurrent[1])
		) {
			pNext = pDataCurrent + 2;
		}else {
			pNext = pDataCurrent + 1;
		}
	}

	if (pNext >= &pData[nDataLen]) {
		pNext = &pData[nDataLen];
	}
	return pNext;
}

// ポインタで示した文字の直前にある文字の位置を返します
// 直前にある文字がバッファの先頭の位置を越える場合はpDataを返します
const char* NativeA::GetCharPrev(const char* pData, size_t nDataLen, const char* pDataCurrent)
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer("Memory::MemCharPrev");
//#endif

	const char*	pPrev = ::CharPrevA(pData, pDataCurrent);

//===1999.08.12  このやり方だと、ダメだった。===============-
//
//	if ((pDataCurrent - 1)[0] == '\0') {
//		pPrev = pDataCurrent - 1;
//	}else {
//		if (pDataCurrent - pData >= 2 &&
//			// SJIS全角コードの1バイト目か		// Sept. 1, 2000 jepro 'シフト'を'S'に変更
//			(
//			((unsigned char)0x81 <= (unsigned char)pDataCurrent[-2] && (unsigned char)pDataCurrent[-2] <= (unsigned char)0x9F) ||
//			((unsigned char)0xE0 <= (unsigned char)pDataCurrent[-2] && (unsigned char)pDataCurrent[-2] <= (unsigned char)0xFC)
//			) &&
//			// SJIS全角コードの2バイト目か		// Sept. 1, 2000 jepro 'シフト'を'S'に変更
//			(
//			((unsigned char)0x40 <= (unsigned char)pDataCurrent[-1] && (unsigned char)pDataCurrent[-1] <= (unsigned char)0x7E) ||
//			((unsigned char)0x80 <= (unsigned char)pDataCurrent[-1] && (unsigned char)pDataCurrent[-1] <= (unsigned char)0xFC)
//			)
//		) {
//			pPrev = pDataCurrent - 2;
//		}else {
//			pPrev = pDataCurrent - 1;
//		}
//	}
//	if (pPrev < pData) {
//		pPrev = pData;
//	}
	return pPrev;
}


void NativeA::AppendStringNew(const wchar_t* pszData)
{
	AppendStringNew(pszData, wcslen(pszData));
}

void NativeA::AppendStringNew(const wchar_t* pData, size_t nDataLen)
{
	char* buf = wcstombs_new(pData, nDataLen);
	AppendString(buf);
	delete[] buf;
}

const wchar_t* NativeA::GetStringW() const
{
	return to_wchar(GetStringPtr(), GetStringLength());
}

