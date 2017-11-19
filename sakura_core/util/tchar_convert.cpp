#include "StdAfx.h"
#include "util/tchar_convert.h"
#include "mem/RecycledBuffer.h"

static RecycledBuffer        g_bufSmall;
static RecycledBufferDynamic g_bufBig;


const wchar_t* to_wchar(const char* src)
{
	if (!src) return NULL;

	return to_wchar(src, strlen(src));
}

const wchar_t* to_wchar(const char* pSrc, size_t nSrcLength)
{
	if (!pSrc) return NULL;

	// 必要なサイズを計算
	int nDstLen = MultiByteToWideChar(
		CP_SJIS,
		0,
		pSrc,
		(int)nSrcLength,
		NULL,
		0
	);
	size_t nDstCnt = (size_t)nDstLen + 1;

	// バッファ取得
	wchar_t* pDst;
	if (nDstCnt < g_bufSmall.GetMaxCount<wchar_t>()) {
		pDst = g_bufSmall.GetBuffer<wchar_t>(&nDstCnt);
	}else {
		pDst = g_bufBig.GetBuffer<wchar_t>(nDstCnt);
	}

	// 変換
	nDstLen = MultiByteToWideChar(
		CP_SJIS,
		0,
		pSrc,
		(int)nSrcLength,
		pDst,
		nDstLen
	);
	pDst[nDstLen] = L'\0';

	return pDst;
}


const char* to_achar(const wchar_t* src)
{
	if (!src) return NULL;

	return to_achar(src, wcslen(src));
}

const char* to_achar(const wchar_t* pSrc, size_t nSrcLength)
{
	if (!pSrc) return NULL;

	// 必要なサイズを計算
	int nDstLen = WideCharToMultiByte(
		CP_SJIS,
		0,
		pSrc,
		(int)nSrcLength,
		NULL,
		0,
		NULL,
		NULL
	);
	size_t nDstCnt = (size_t)nDstLen + 1;

	// バッファ取得
	char* pDst;
	if (nDstCnt < g_bufSmall.GetMaxCount<char>()) {
		pDst = g_bufSmall.GetBuffer<char>(&nDstCnt);
	}else {
		pDst = g_bufBig.GetBuffer<char>(nDstCnt);
	}

	// 変換
	nDstLen = WideCharToMultiByte(
		CP_SJIS,
		0,
		pSrc,
		(int)nSrcLength,
		pDst,
		nDstLen,
		NULL,
		NULL
	);
	pDst[nDstLen] = '\0';

	return pDst;
}

const wchar_t* easy_format(const wchar_t* format, ...)
{
	wchar_t* buf = g_bufBig.GetBuffer<wchar_t>(1024);
	va_list v;
	va_start(v, format);
	tchar_vsprintf(buf, format, v);
	va_end(v);
	return buf;
}


