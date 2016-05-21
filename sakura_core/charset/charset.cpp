/*! @file
	@brief 文字コードセットの管理

	@author kobake
	@date 2008
	@date 2010/6/21	Uchi	全面的に作り変え(マージは2013/1/4)
							interfaceはあまり変えない様にした
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2009, rastiv
	Copyright (C) 2010, Uchi
	Copyright (C) 2012, novice
	Copyright (C) 2013, Moca, Uchi

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
#include "StdAfx.h"
#include "charset.h"
#include "CodePage.h"
#include <vector>
#include <map>

struct CodeSet {
	EncodingType	eCodeSet;
	const WCHAR*	sNormal;
	const WCHAR*	sShort;
	const WCHAR*	sLong;			// for Combo
	bool			bUseBom;		// BOMが使えるか
	bool			bIsBomDefOn;	// BOMのデフォルトがOnか
	bool			bCanDefault;	// デフォルト文字コードになれるか
};

// 文字コードセット(初期データ)
static CodeSet ASCodeSet[] = {
	{ CODE_AUTODETECT,	L"Auto",	L"Auto",	L"自動選択",	false,	false,	false },	// 文字コード自動判別	// mapには入れない
	{ CODE_SJIS,		L"SJIS",	L"SJIS",	L"SJIS",		false,	false,	true  },	// SJIS				(MS-CP932(Windows-31J), シフトJIS(Shift_JIS))
	{ CODE_JIS,			L"JIS",		L"JIS",		L"JIS",			false,	false,	false },	// JIS				(MS-CP5022x(ISO-2022-JP-MS))
	{ CODE_EUC,			L"EUC",		L"EUC",		L"EUC-JP",		false,	false,	true  },	// EUC				(MS-CP51932)	// eucJP-ms(eucJP-open)ではない
	{ CODE_LATIN1,		L"Latin1",	L"Latin1",	L"Latin1",		false,	false,	true  },	// Latin1				(欧文, Windows-932, Windows Codepage 1252 West European)
	{ CODE_UNICODE,		L"Unicode",	L"Uni",		L"Unicode",		true,	true,	true  },	// Unicode			(UTF-16 LittleEndian)	// UCS-2
	{ CODE_UNICODEBE,	L"UniBE",	L"UniBE",	L"UnicodeBE",	true,	true,	true  },	// Unicode BigEndian	(UTF-16 BigEndian)		// UCS-2
	{ CODE_UTF8,		L"UTF-8",	L"UTF-8",	L"UTF-8",		true,	false,	true  },	// UTF-8
	{ CODE_CESU8,		L"CESU-8",	L"CESU-8",	L"CESU-8",		true,	false,	true  },	// CESU-8				(UCS-2からUTF-8化)
	{ CODE_UTF7,		L"UTF-7",	L"UTF-7",	L"UTF-7",		true,	false,	false },	// UTF-7
};

// 文字コードセット
typedef	std::map<int, CodeSet>	MSCodeSet;
static MSCodeSet				msCodeSet;
// 表示順
static std::vector<EncodingType>	vDispIdx;


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           初期化                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void InitCodeSet()
{
	if (msCodeSet.empty()) {
		for (int i=0; i<_countof(ASCodeSet); ++i) {
			vDispIdx.push_back(ASCodeSet[i].eCodeSet);
			if (i > 0) {
				msCodeSet[ASCodeSet[i].eCodeSet] = ASCodeSet[i];
			}
		}
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           判定                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
bool IsValidCodeType(int code)
{
	// 初期化
	InitCodeSet();
	return (msCodeSet.find(code) != msCodeSet.end());
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           名前                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

LPCTSTR CodeTypeName::Normal() const
{
	if (msCodeSet.find(eCodeType) == msCodeSet.end()) {
		return NULL;
	}
	return to_tchar(msCodeSet[eCodeType].sNormal);
}

LPCTSTR CodeTypeName::Short() const
{
	if (msCodeSet.find(eCodeType) == msCodeSet.end()) {
		return NULL;
	}
	return to_tchar(msCodeSet[eCodeType].sShort);
}

LPCTSTR CodeTypeName::Bracket() const
{
	if (msCodeSet.find(eCodeType) == msCodeSet.end()) {
		return NULL;
	}

//	static	std::wstring	sWork = L"  [" + msCodeSet[eCodeType].sShort + L"]";
	static	std::wstring	sWork;
	sWork = std::wstring(L"  [") + msCodeSet[eCodeType].sShort + L"]";	// 変数の定義と値の設定を一緒にやるとバグる様なので分離	// 2013/4/20 Uchi

	return to_tchar(sWork.c_str());
}


bool CodeTypeName::UseBom()
{
	if (msCodeSet.find(eCodeType) == msCodeSet.end()) {
		if (IsValidCodeOrCPType(eCodeType)) {
			CodePage encoding(eCodeType);
			Memory mem;
			encoding.GetBom(&mem);
			return 0 < mem.GetRawLength();
		}
		return false;
	}

	return msCodeSet[eCodeType].bUseBom;
}

bool CodeTypeName::IsBomDefOn()
{
	if (msCodeSet.find(eCodeType) == msCodeSet.end()) {
		return false;
	}

	return msCodeSet[eCodeType].bIsBomDefOn;
}

bool CodeTypeName::CanDefault()
{
	if (msCodeSet.find(eCodeType) == msCodeSet.end()) {
		if (IsValidCodeOrCPType(eCodeType)) {
			return true;
		}
		return false;
	}

	return msCodeSet[eCodeType].bCanDefault;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      コンボボックス                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

size_t CodeTypesForCombobox::GetCount() const
{
	return vDispIdx.size();
}

EncodingType CodeTypesForCombobox::GetCode(int nIndex) const
{
	return vDispIdx[nIndex];
}

LPCTSTR CodeTypesForCombobox::GetName(int nIndex) const
{
	if (nIndex == 0) {
		return LS(STR_ERR_GLOBAL01);
	}
	return to_tchar(msCodeSet[vDispIdx[nIndex]].sLong);
}
