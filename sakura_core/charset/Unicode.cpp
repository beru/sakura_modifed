// この行は文字化け対策用です．消さないでください

#include "StdAfx.h"
#include "Unicode.h"
#include "codechecker.h"
#include "mem/Memory.h"
#include "Eol.h"

CodeConvertResult Unicode::_UnicodeToUnicode_in( const Memory& src, NativeW* pDstMem, const bool bBigEndian )
{
	// ソース取得
	size_t nSrcLen;
	const unsigned char* pSrc = reinterpret_cast<const unsigned char*>( src.GetRawPtr(&nSrcLen) );
	Memory* pDstMem2 = pDstMem->_GetMemory();

	CodeConvertResult res = CodeConvertResult::Complete;
	bool bCopy = false;
	if (nSrcLen % 2 == 1) {
		// 不足分の最終1バイトとして 0x00 を補う。
		pDstMem2->AllocBuffer( nSrcLen + 1 );
		unsigned char* pDst  = reinterpret_cast<unsigned char*>( pDstMem2->GetRawPtr() );
		if (pDstMem2->GetRawPtr()) {
			if (&src != pDstMem2) {
				pDstMem2->SetRawDataHoldBuffer(pSrc, nSrcLen);
				bCopy = true;
			}
			pDst[nSrcLen] = 0;
			pDstMem2->_SetRawLength(nSrcLen + 1);
			res = CodeConvertResult::LoseSome;
		}else {
			return CodeConvertResult::Failure;
		}
	}

	if (bBigEndian) {
		if (&src != pDstMem2 && !bCopy) {
			// コピーしつつ UnicodeBe -> Unicode
			pDstMem2->SwabHLByte(src);
		}else {
			pDstMem2->SwapHLByte();  // UnicodeBe -> Unicode
		}
	}else if (!bCopy) {
		pDstMem2->SetRawDataHoldBuffer(pSrc, nSrcLen);
	}
	return res;
}


CodeConvertResult Unicode::_UnicodeToUnicode_out( const NativeW& src, Memory* pDstMem, const bool bBigEndian )
{
	if (bBigEndian == true) {
		if (src._GetMemory() == pDstMem) {
			pDstMem->SwapHLByte();   // Unicode -> UnicodeBe
		}else {
			pDstMem->SwabHLByte(*(src._GetMemory()));
		}
	}else {
		if (src._GetMemory() != pDstMem) {
			pDstMem->SetRawDataHoldBuffer(*(src._GetMemory()));
		}else {
			// 何もしない
		}
	}

	return CodeConvertResult::Complete;   // 何もしない
}




void Unicode::GetBom(Memory* pMemBom)
{
	static const BYTE UTF16LE_BOM[] = {0xFF, 0xFE};
	pMemBom->SetRawData(UTF16LE_BOM, sizeof(UTF16LE_BOM));
}


void Unicode::GetEol(Memory* pMemEol, EolType eolType)
{
	static const struct{
		const void* pData;
		int nLen;
	}
	aEolTable[EOL_TYPE_NUM] = {
		{ L"",			0 * sizeof(wchar_t) },	// EolType::None
		{ L"\x0d\x0a",	2 * sizeof(wchar_t) },	// EolType::CRLF
		{ L"\x0a",		1 * sizeof(wchar_t) },	// EolType::LF
		{ L"\x0d",		1 * sizeof(wchar_t) },	// EolType::CR
		{ L"\x85",		1 * sizeof(wchar_t) },	// EolType::NEL
		{ L"\u2028",	1 * sizeof(wchar_t) },	// EolType::LS
		{ L"\u2029",	1 * sizeof(wchar_t) },	// EolType::PS
	};
	pMemEol->SetRawData(aEolTable[(int)eolType].pData, aEolTable[(int)eolType].nLen);
}
