#pragma once

#include <mbstring.h>
#include "charset/CodeBase.h"
#include "charset/codeutil.h"

class Euc : public CodeBase {
public:
	// CodeBaseインターフェース
	CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst){ return EUCToUnicode(src, pDst); }	// 特定コード → UNICODE    変換
	CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst){ return UnicodeToEUC(src, pDst); }	// UNICODE    → 特定コード 変換
	CodeConvertResult UnicodeToHex(const wchar_t* pSrc, size_t iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar);	// UNICODE → Hex 変換

public:
	// 実装
	static CodeConvertResult EUCToUnicode(const Memory& src, NativeW* pDstMem);		// EUC       → Unicodeコード変換
	static CodeConvertResult UnicodeToEUC(const NativeW& src, Memory* pDstMem);		// Unicode   → EUCコード変換

public:
	// 実装
	inline static size_t _EucjpToUni_char(const unsigned char*, unsigned short*, const ECharSet, bool* pbError, bool* pbHex);
protected:
	static size_t EucjpToUni(const char*, const size_t, wchar_t*, bool* pbError);
	inline static size_t _UniToEucjp_char(const unsigned short*, unsigned char*, const ECharSet, bool* pbError);
	static size_t UniToEucjp(const wchar_t*, const size_t, char*, bool* pbError);
};


/*!
	EUCJP(CP51932) の全角一文字または半角片仮名一文字の変換

	eCharset は CHARSET_JIS_HANKATA か CHARSET_JIS_ZENKAKU .

	高速化のため、インライン化
*/
inline size_t Euc::_EucjpToUni_char(const unsigned char* pSrc, unsigned short* pDst, const ECharSet eCharset, bool* pbError, bool* pbHex = nullptr)
{
	size_t nret;
	unsigned char czenkaku[2];
	unsigned int ctemp;
	bool berror = false;
	bool hex = false;

	switch (eCharset) {
	case CHARSET_JIS_HANKATA:
		// 半角カタカナを処理。エラーは起こらない
		nret = MyMultiByteToWideChar_JP(&pSrc[1], 1, pDst);
		// 保護コード
		if (nret < 1) {
			nret = 1;
		}
		break;
	case CHARSET_JIS_ZENKAKU:
		// EUCJP(CP51932) → JIS
		czenkaku[0] = (pSrc[0] & 0x7f);
		czenkaku[1] = (pSrc[1] & 0x7f);
		// JIS → SJIS
		ctemp = _mbcjistojms((static_cast<unsigned int>(czenkaku[0]) << 8) | czenkaku[1]);
		if (ctemp != 0) {
			// NEC選定IBM拡張コードポイントををIBM拡張コードポイントにに変換
			unsigned int ctemp_ = SjisFilter_nec2ibm(ctemp);
			ctemp = ctemp_;
			// SJIS → Unicode
			czenkaku[0] = static_cast<unsigned char>((ctemp & 0x0000ff00) >> 8);
			czenkaku[1] = static_cast<unsigned char>(ctemp & 0x000000ff);
			nret = MyMultiByteToWideChar_JP(&czenkaku[0], 2, pDst);
			if (nret < 1) {
				nret = BinToText(pSrc, 2, pDst);
				hex = true;
			}
		}else {
			// JIS -> SJIS の変換エラー
			// エラー処理関数を使う
			nret = BinToText(pSrc, 2, pDst);
			hex = true;
		}
		break;
	default:
		// 保護コード
		berror = true;
		hex = true;
		pDst[0] = '?';
		nret = 1;
	}

	if (pbError) {
		*pbError = berror;
	}
	if (pbHex) {
		*pbHex = hex;
	}

	return nret;
}

/*
	Unicode -> EUCJP 一文字変換

	高速化のため、インライン化
*/
inline size_t Euc::_UniToEucjp_char(const unsigned short* pSrc, unsigned char* pDst, const ECharSet eCharset, bool* pbError)
{
	int nret = 0;
	unsigned char cbuf[4];
	unsigned int ctemp;
	bool berror = false;

	if (eCharset == CHARSET_UNI_SURROG) {
		// サロゲートは SJIS に変換できない。
		berror = true;
		pDst[0] = '?';
		nret = 1;
	}else if (eCharset == CHARSET_UNI_NORMAL) {
		size_t nclen = MyWideCharToMultiByte_JP(pSrc, 1, cbuf);
		if (nclen < 1) {
			// Uni -> SJIS で変換エラー
			berror = true;
			pDst[0] = '?';
			nret = 1;
		}else if (nclen == 1 && IsAscii7(cbuf[0])) {
			// 7bit ASCII の処理
			pDst[0] = cbuf[0];
			nret = 1;
		}else if (nclen == 1 && IsSjisHankata(cbuf[0])) {
			// 半角カタカナ文字の処理：
			pDst[0] = 0x8e;
			pDst[1] = cbuf[0];
			nret = 2;
		}else if (nclen == 2/* && IsSjisZen(reinterpret_cast<char*>(cbuf)) */) {
			// 全角文字の処理：
			// SJIS -> JIS
			unsigned int ctemp_ = SjisFilter_ibm2nec((static_cast<unsigned int>(cbuf[0]) << 8) | cbuf[1]);
				// < IBM拡張文字をNEC選定IBM拡張文字に変換
			ctemp = _mbcjmstojis(ctemp_);
			if (ctemp == 0) {
				berror = true;
				pDst[0] = '?';
				nret = 1;
			}else {
				// JIS -> EUCJP
				pDst[0] = static_cast<unsigned char>((ctemp & 0x0000ff00) >> 8) | 0x80;
				pDst[1] = static_cast<unsigned char>(ctemp & 0x000000ff) | 0x80;
				nret = 2;
			}
		}else {
			// 保護コード
			berror = true;
			pDst[0] = '?';
			nret = 1;
		}
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

