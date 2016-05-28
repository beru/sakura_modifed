/*
	Copyright (C) 2008, kobake

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
#pragma once

#include "CodeBase.h"
#include "charset/codeutil.h"

struct CommonSetting_StatusBar;

class ShiftJis : public CodeBase {

public:
	// CodeBaseインターフェース
	CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst){ return SJISToUnicode(src, pDst); }	// 特定コード → UNICODE    変換
	CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst){ return UnicodeToSJIS(src, pDst); }	// UNICODE    → 特定コード 変換
// GetEolはCodeBaseに移動	2010/6/13 Uchi
	CodeConvertResult UnicodeToHex(const wchar_t* cSrc, size_t iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar);	// UNICODE → Hex 変換

public:
	// 実装
	static CodeConvertResult SJISToUnicode(const Memory& src, NativeW* pDstMem);		// SJIS      → Unicodeコード変換
	static CodeConvertResult UnicodeToSJIS(const NativeW& src, Memory* pDstMem);		// Unicode   → SJISコード変換
// S_GetEolはCodeBaseに移動	2010/6/13 Uchi
	// 2005-09-02 D.S.Koba
	// 2007.08.14 kobake MemoryからShiftJisへ移動
	static size_t GetSizeOfChar(const char* pData, size_t nDataLen, size_t nIdx); // 指定した位置の文字が何バイト文字かを返す

protected:
	// 実装
	// 2008.11.10 変換ロジックを書き直す
	inline static size_t _SjisToUni_char(const unsigned char*, unsigned short*, const ECharSet, bool* pbError);
	static size_t SjisToUni(const char*, const size_t, wchar_t*, bool* pbError);
	inline static size_t _UniToSjis_char(const unsigned short*, unsigned char*, const ECharSet, bool* pbError);
	static size_t UniToSjis(const wchar_t*, const size_t, char*, bool* pbError);
};


/*!
	SJIS の全角一文字または半角一文字のUnicodeへの変換

	eCharset は CHARSET_JIS_ZENKAKU または CHARSET_JIS_HANKATA 。

	高速化のため、インライン化
*/
inline size_t ShiftJis::_SjisToUni_char(const unsigned char* pSrc, unsigned short* pDst, const ECharSet eCharset, bool* pbError)
{
	size_t nret;
	bool berror = false;

	switch (eCharset) {
	case CHARSET_JIS_HANKATA:
		// 半角カタカナを処理
		// エラーは起こらない。
		nret = MyMultiByteToWideChar_JP(pSrc, 1, pDst);
		// 保護コード
		if (nret < 1) {
			nret = 1;
		}
		break;
	case CHARSET_JIS_ZENKAKU:
		// 全角文字を処理
		nret = MyMultiByteToWideChar_JP(pSrc, 2, pDst);
		if (nret < 1) {	// SJIS -> Unicode 変換に失敗
			nret = BinToText(pSrc, 2, pDst);
		}
		break;
	default:
		// 致命的エラー回避コード
		berror = true;
		pDst[0] = L'?';
		nret = 1;
	}

	if (pbError) {
		*pbError = berror;
	}

	return nret;
}




/*!
	UNICODE -> SJIS 一文字変換

	eCharset は CHARSET_UNI_NORMAL または CHARSET_UNI_SURROG。

	高速化のため、インライン化
*/
inline size_t ShiftJis::_UniToSjis_char(const unsigned short* pSrc, unsigned char* pDst, const ECharSet eCharset, bool* pbError)
{
	size_t nret;
	bool berror = false;

	if (eCharset == CHARSET_UNI_NORMAL) {
		nret = MyWideCharToMultiByte_JP(pSrc, 1, pDst);
		if (nret < 1) {
			// Uni -> SJIS 変換に失敗
			berror = true;
			pDst[0] = '?';
			nret = 1;
		}
	}else if (eCharset == CHARSET_UNI_SURROG) {
		// サロゲートペアは SJIS に変換できない。
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

