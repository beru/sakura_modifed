#pragma once

class Eol;

// Aug. 16, 2007 kobake
wchar_t* wcsncpy_ex(wchar_t* dst, size_t dst_count, const wchar_t* src, size_t src_count);
wchar_t* wcs_pushW(wchar_t* dst, size_t dst_count, const wchar_t* src, size_t src_count);
wchar_t* wcs_pushW(wchar_t* dst, size_t dst_count, const wchar_t* src);
wchar_t* wcs_pushA(wchar_t* dst, size_t dst_count, const char* src, size_t src_count);
wchar_t* wcs_pushA(wchar_t* dst, size_t dst_count, const char* src);
#define wcs_pushT wcs_pushW

int AddLastChar(TCHAR*, size_t, TCHAR); // 2003.06.24 Moca 最後の文字が指定された文字でないときは付加する
size_t LimitStringLength(const wchar_t*, size_t, size_t, NativeW&); // データを指定「文字数」以内に切り詰める

const char* GetNextLimitedLengthText(const char*, size_t, size_t, size_t*, size_t*); // 指定長以下のテキストに切り分ける
const char* GetNextLine(const char*, size_t, size_t*, size_t*, Eol*); // CR0LF0,CRLF,LF,CRで区切られる「行」を返す。改行コードは行長に加えない
const wchar_t* GetNextLineW(const wchar_t*, size_t, size_t*, size_t*, Eol*, bool); // GetNextLineのwchar_t版
//wchar_t* GetNextLineWB(const wchar_t*, int, int*, int*, Eol*); // GetNextLineのwchar_t版(ビックエンディアン用)  // 未使用
void GetLineColumn(const wchar_t*, int*, int*);

size_t cescape(const TCHAR* org, TCHAR* buf, TCHAR cesc, TCHAR cwith);

/*!	&の二重化
	メニューに含まれる&を&&に置き換える
	@author genta
	@date 2002/01/30 cescapeに拡張し，
	@date 2004/06/19 genta Generic mapping
*/
inline void dupamp(const TCHAR* org, TCHAR* out)
{ cescape(org, out, _T('&'), _T('&')); }


/*
	scanf的安全スキャン

	使用例:
		int a[3];
		scan_ints("1,23,4,5", "%d,%d,%d", a);
		// 結果: a[0]=1, a[1]=23, a[2]=4 となる。
*/
int scan_ints(
	const wchar_t*	pszData,	// [in]  データ文字列
	const wchar_t*	pszFormat,	// [in]  データフォーマット
	int*			anBuf		// [out] 取得した数値 (要素数は最大32まで)
);

