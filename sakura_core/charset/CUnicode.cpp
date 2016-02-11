// この行は文字化け対策用です．消さないでください

#include "StdAfx.h"
#include "CUnicode.h"
#include "codechecker.h"
#include "mem/CMemory.h"
#include "CEol.h"

CodeConvertResult Unicode::_UnicodeToUnicode_in( const Memory& cSrc, NativeW* pDstMem, const bool bBigEndian )
{
	// ソース取得
	int nSrcLen;
	const unsigned char* pSrc = reinterpret_cast<const unsigned char*>( cSrc.GetRawPtr(&nSrcLen) );
	Memory* pDstMem2 = pDstMem->_GetMemory();

	CodeConvertResult res = CodeConvertResult::Complete;
	bool bCopy = false;
	if (nSrcLen % 2 == 1) {
		// 不足分の最終1バイトとして 0x00 を補う。
		pDstMem2->AllocBuffer( nSrcLen + 1 );
		unsigned char* pDst  = reinterpret_cast<unsigned char*>( pDstMem2->GetRawPtr() );
		if (pDstMem2->GetRawPtr()) {
			if (&cSrc != pDstMem2) {
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
		if (&cSrc != pDstMem2 && !bCopy) {
			// コピーしつつ UnicodeBe -> Unicode
			pDstMem2->SwabHLByte(cSrc);
		}else {
			pDstMem2->SwapHLByte();  // UnicodeBe -> Unicode
		}
	}else if (!bCopy) {
		pDstMem2->SetRawDataHoldBuffer(pSrc, nSrcLen);
	}
	return res;
}


CodeConvertResult Unicode::_UnicodeToUnicode_out( const NativeW& cSrc, Memory* pDstMem, const bool bBigEndian )
{
	if (bBigEndian == true) {
		if (cSrc._GetMemory() == pDstMem) {
			pDstMem->SwapHLByte();   // Unicode -> UnicodeBe
		}else {
			pDstMem->SwabHLByte(*(cSrc._GetMemory()));
		}
	}else {
		if (cSrc._GetMemory() != pDstMem) {
			pDstMem->SetRawDataHoldBuffer(*(cSrc._GetMemory()));
		}else {
			// 何もしない
		}
	}

	return CodeConvertResult::Complete;   // 何もしない
}




void Unicode::GetBom(Memory* pcmemBom)
{
	static const BYTE UTF16LE_BOM[] = {0xFF, 0xFE};
	pcmemBom->SetRawData(UTF16LE_BOM, sizeof(UTF16LE_BOM));
}


void Unicode::GetEol(Memory* pcmemEol, EolType eEolType)
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
	pcmemEol->SetRawData(aEolTable[(int)eEolType].pData, aEolTable[(int)eEolType].nLen);
}
