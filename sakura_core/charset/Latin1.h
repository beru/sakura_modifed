#pragma once

#include "CodeBase.h"

// Latin1 (Latin1, 欧文, Windows-1252, Windows Codepage 1252 West European) 対応クラス
class Latin1 : public CodeBase {

public:
	// CodeBaseインターフェース
	CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst){ return Latin1ToUnicode(src, pDst); }	// 特定コード → UNICODE    変換
	CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst){ return UnicodeToLatin1(src, pDst); }	// UNICODE    → 特定コード 変換
	CodeConvertResult UnicodeToHex(const wchar_t* pSrc, size_t iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar);	// UNICODE → Hex 変換

public:
	// 実装
	static CodeConvertResult Latin1ToUnicode(const Memory& src, NativeW* pDstMem);		// Latin1   → Unicodeコード変換
	static CodeConvertResult UnicodeToLatin1(const NativeW& src, Memory* pDstMem);		// Unicode  → Latin1コード変換
	static size_t GetSizeOfChar(const char* pData, size_t nDataLen, size_t nIdx); // 指定した位置の文字が何バイト文字かを返す

protected:
	// 実装
	static size_t Latin1ToUni(const char*, const size_t, wchar_t*, bool* pbError);
	inline static int _UniToLatin1_char(const unsigned short*, unsigned char*, const ECharSet, bool* pbError);
	static size_t UniToLatin1(const wchar_t*, const size_t, char*, bool* pbError);
};

/*!
	UNICODE -> Latin1 一文字変換

	eCharset は CHARSET_UNI_NORMAL または CHARSET_UNI_SURROG。

	高速化のため、インライン化
*/
inline int Latin1::_UniToLatin1_char(const unsigned short* pSrc, unsigned char* pDst, const ECharSet eCharset, bool* pbError)
{
	int nret;
	bool berror = false;
	BOOL blost;

	if (eCharset == CHARSET_UNI_NORMAL) {
		if ((pSrc[0] >= 0 && pSrc[0] <= 0x7f) || (pSrc[0] >= 0xa0 && pSrc[0] <= 0xff)) {
			// ISO 58859-1の範囲
			pDst[0] = (unsigned char)pSrc[0];
			nret = 1;
		} else {
			// ISO 8859-1以外
			nret = ::WideCharToMultiByte(1252, 0, reinterpret_cast<const wchar_t*>(pSrc), 1, reinterpret_cast<char*>(pDst), 4, NULL, &blost);
			if (blost != FALSE) {
				// Uni -> Latin1 変換に失敗
				berror = true;
				pDst[0] = '?';
				nret = 1;
			}
		}
	}else if (eCharset == CHARSET_UNI_SURROG) {
		// サロゲートペアは Latin1 に変換できない。
		berror = true;
		pDst[0] = '?';
		nret = 1;
	}else {
		// 保護コード
		berror = true;
		pDst[0] = '?';
		nret = 1;
	}

	if (pbError) {
		*pbError = berror;
	}

	return nret;
}

