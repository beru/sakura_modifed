#pragma once

#include "CodeBase.h"

class Jis : public CodeBase {
public:
	Jis(bool base64decode = true) : base64decode(base64decode) { }
public:
	// CodeBaseインターフェース
	CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst){ return JISToUnicode(src, pDst, base64decode); }	// 特定コード → UNICODE    変換
	CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst){ return UnicodeToJIS(src, pDst); }	// UNICODE    → 特定コード 変換
// GetEolはCodeBaseに移動	2010/6/13 Uchi
	CodeConvertResult UnicodeToHex(const wchar_t* cSrc, size_t iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar);			// UNICODE → Hex 変換

public:
	// 実装
	static CodeConvertResult JISToUnicode(const Memory& src, NativeW* pDstMem, bool base64decode = true);	// E-Mail(JIS→Unicode)コード変換
	static CodeConvertResult UnicodeToJIS(const NativeW& src, Memory* pDstMem);		// Unicode   → JISコード変換

protected:
	static size_t _JisToUni_block(const unsigned char*, const size_t, unsigned short*, const EMyJisEscseq, bool* pbError);
	static size_t JisToUni(const char*, const size_t, wchar_t*, bool* pbError);
	static size_t _SjisToJis_char(const unsigned char*, unsigned char*, const ECharSet, bool* pbError);
	static size_t UniToJis(const wchar_t*, const size_t, char*, bool* pbError);

private:
	// 変換方針
	bool base64decode;

public:
	// 各種判定定数
	// JIS コードのエスケープシーケンス文字列データ
	static const char JISESCDATA_ASCII7[];
	static const char JISESCDATA_JISX0201Latin[];
	static const char JISESCDATA_JISX0201Latin_OLD[];
	static const char JISESCDATA_JISX0201Katakana[];
	static const char JISESCDATA_JISX0208_1978[];
	static const char JISESCDATA_JISX0208_1983[];
	static const char JISESCDATA_JISX0208_1990[];
//	static const int TABLE_JISESCLEN[];
//	static const char* TABLE_JISESCDATA[];
};


#if 0 // codechecker.h に定義されている
// JIS コードのエスケープシーケンスたち
/*
	符号化文字集合       16進表現            文字列表現[*1]
	------------------------------------------------------------
	JIS C 6226-1978      1b 24 40            ESC $ @
	JIS X 0208-1983      1b 24 42            ESC $ B
	JIS X 0208-1990      1b 26 40 1b 24 42   ESC & @ ESC $ B
	JIS X 0212-1990      1b 24 28 44         ESC $ (D
	JIS X 0213:2000 1面  1b 24 28 4f         ESC $ (O
	JIS X 0213:2004 1面  1b 24 28 51         ESC $ (Q
	JIS X 0213:2000 2面  1b 24 28 50         ESC $ (P
	JIS X 0201 ラテン    1b 28 4a            ESC (J
	JIS X 0201 ラテン    1b 28 48            ESC (H         (歴史的[*2])
	JIS X 0201 片仮名    1b 28 49            ESC (I
	ISO/IEC 646 IRV      1b 28 42            ESC (B
	
	
	  [*1] 各バイトを便宜的にISO/IEC 646 IRVの文字で表したもの。
	       ただしESCはバイト値1bを表す。
	
	  [*2] JIS X 0201の指示としては使用すべきでないが、古いデータでは
	       使われている可能性がある。
	
	出展：http://www.asahi-net.or.jp/~wq6k-yn/code/
	参考：http://homepage2.nifty.com/zaco/code/
*/
enum EJisESCSeqType {
	JISESC_UNKNOWN,
	JISESC_ASCII,
	JISESC_JISX0201Latin,
	JISESC_JISX0201Latin_OLD,
	JISESC_JISX0201Katakana,
	JISESC_JISX0208_1978,
	JISESC_JISX0208_1983,
	JISESC_JISX0208_1990,
};

#endif

