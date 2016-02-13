// ‚±‚Ìs‚Í•¶Žš‰»‚¯‘Îô—p‚Å‚·DÁ‚³‚È‚¢‚Å‚­‚¾‚³‚¢

#include "StdAfx.h"
#include "CUnicodeBe.h"
#include "CEol.h"

#include "codechecker.h"


void UnicodeBe::GetBom(Memory* pMemBom)
{
	static const BYTE UTF16BE_BOM[] = {0xFE, 0xFF};
	pMemBom->SetRawData(UTF16BE_BOM, sizeof(UTF16BE_BOM));
}

void UnicodeBe::GetEol(Memory* pMemEol, EolType eolType)
{
	static const struct{
		const void* pData;
		int nLen;
	}
	aEolTable[EOL_TYPE_NUM] = {
		{ "",					0 * sizeof(wchar_t) },	// EolType::NONE
		{ "\x00\x0d\x00\x0a",	2 * sizeof(wchar_t) },	// EolType::CRLF
		{ "\x00\x0a",			1 * sizeof(wchar_t) },	// EolType::LF
		{ "\x00\x0d",			1 * sizeof(wchar_t) },	// EolType::CR
		{ "\x00\x85",			1 * sizeof(wchar_t) },	// EolType::NEL
		{ "\x20\x28",			1 * sizeof(wchar_t) },	// EolType::LS
		{ "\x20\x29",			1 * sizeof(wchar_t) },	// EolType::PS
	};
	pMemEol->SetRawData(aEolTable[(int)eolType].pData, aEolTable[(int)eolType].nLen);
}
