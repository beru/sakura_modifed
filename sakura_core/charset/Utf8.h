#pragma once

#include "CodeBase.h"
#include "charset/codeutil.h"

struct CommonSetting_StatusBar;

class Utf8 : public CodeBase {
public:

	// CodeBaseインターフェース
	CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst) {	// 特定コード → UNICODE    変換
		return UTF8ToUnicode(src, pDst);
	}
	CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst) {	// UNICODE    → 特定コード 変換
		return UnicodeToUTF8(src, pDst);
	}
	void GetBom(Memory* pMemBom);										// BOMデータ取得
	void GetEol(Memory* pMemEol, EolType eolType);
	CodeConvertResult _UnicodeToHex(const wchar_t* src, size_t iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar, const bool CESU8Mode);			// UNICODE → Hex 変換
	CodeConvertResult UnicodeToHex(const wchar_t* ps, size_t nsl, TCHAR* pd, const CommonSetting_StatusBar* psStatusbar){ return _UnicodeToHex(ps, nsl, pd, psStatusbar, false); }

public:
	// UTF-8 / CESU-8 <-> Unicodeコード変換
	static CodeConvertResult _UTF8ToUnicode( const Memory& src, NativeW* pDstMem, bool bCESU8Mode );
	static CodeConvertResult _UnicodeToUTF8( const NativeW& src, Memory* pDstMem, bool bCESU8Mode );
	inline static CodeConvertResult UTF8ToUnicode( const Memory& src, NativeW* pDst ){ return _UTF8ToUnicode(src, pDst, false); }	// UTF-8 -> Unicodeコード変換
	inline static CodeConvertResult CESU8ToUnicode( const Memory& src, NativeW* pDst ){ return _UTF8ToUnicode(src, pDst, true); }	// CESU-8 -> Unicodeコード変換
	inline static CodeConvertResult UnicodeToUTF8( const NativeW& src, Memory* pDst ){ return  _UnicodeToUTF8(src, pDst, false); }	// Unicode → UTF-8コード変換
	inline static CodeConvertResult UnicodeToCESU8( const NativeW& src, Memory* pDst ){ return _UnicodeToUTF8(src, pDst, true); }	// Unicode → CESU-8コード変換

protected:
	// 変換の実装
	inline static size_t _Utf8ToUni_char(const unsigned char*, const size_t, unsigned short*, bool bCESU8Mode);
	static size_t Utf8ToUni(const char*, const size_t, wchar_t*, bool bCESU8Mode);
	inline static size_t _UniToUtf8_char(const unsigned short*, const size_t, unsigned char*, const bool bCSU8Mode);
	static size_t UniToUtf8(const wchar_t*, const size_t, char*, bool* pbError, bool bCSU8Mode);
};

/*!
	UTF-8 の一文字変換

	UTF-8 と CESU-8 とを場合分けして、それぞれ変換する

	高速化のため、インライン化

*/
inline size_t Utf8::_Utf8ToUni_char( const unsigned char* pSrc, const size_t nSrcLen, unsigned short* pDst, bool bCESUMode )
{
	size_t nret;

	if (nSrcLen < 1) {
		return 0;
	}

	if (!bCESUMode) {
		// UTF-8 の処理
		if (nSrcLen < 4) {
			pDst[0] = static_cast<unsigned short>(DecodeUtf8(pSrc, nSrcLen) & 0x0000ffff);
			nret = 1;
		}else if (nSrcLen == 4) {
			// UTF-8 サロゲート領域の処理
			wchar32_t wc32 = DecodeUtf8(pSrc, 4);
			EncodeUtf16Surrog(wc32, pDst);
			nret = 2;
		}else {
			// 保護コード
			pDst[0] = L'?';
			nret = 1;
		}
	}else {
		// CESU-8 の処理
		if (nSrcLen < 4) {
			pDst[0] = static_cast<unsigned short>(DecodeUtf8(pSrc, nSrcLen) & 0x0000ffff);
			nret = 1;
		}else if (nSrcLen == 6) {
			// CESU-8 サロゲート領域の処理
			pDst[0] = static_cast<unsigned short>(DecodeUtf8(&pSrc[0], 3) & 0x0000ffff);
			pDst[1] = static_cast<unsigned short>(DecodeUtf8(&pSrc[3], 3) & 0x0000ffff);
			nret = 2;
		}else {
			// 保護コード
			pDst[0] = L'?';
			nret = 1;
		}
	}

	return nret;
}



/*!
	Unicode -> UTF-8 の一文字変換

	nSrcLen は 1 または 2

	高速化のため、インライン化
*/
inline size_t Utf8::_UniToUtf8_char( const unsigned short* pSrc, const size_t nSrcLen, unsigned char* pDst, bool bCESU8Mode )
{
	size_t nret;

	if (nSrcLen < 1) {
		return 0;
	}

	if (!bCESU8Mode) {
		// UTF-8 の処理
		wchar32_t wc32;
		if (nSrcLen == 2) {
			wc32 = DecodeUtf16Surrog(pSrc[0], pSrc[1]);
		}else if (nSrcLen == 1) {	// nSrcLen == 1
			wc32 = pSrc[0];
		}else {
			wc32 = L'?';
		}
		nret = EncodeUtf8(wc32, &pDst[0]);
	}else {
		// CESU-8 の処理
		int nclen = 0;
		nclen += EncodeUtf8(pSrc[0], &pDst[0]);
		if (nSrcLen == 2) {
			nclen += EncodeUtf8(pSrc[1], &pDst[nclen]);
		}else {
			;
		}
		nret = nclen;
	}

	return nret;
}

