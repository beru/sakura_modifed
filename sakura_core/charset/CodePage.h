/*!	@file
	@brief コードページ
	
	@author Sakura-Editor collaborators
*/
/*
	Copyright (C) 2010-2012 Moca

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
#include <vector>
#include <utility>
#include <string>
#include "ShiftJis.h"

enum EEncodingTrait {
	ENCODING_TRAIT_ERROR, // error
	ENCODING_TRAIT_ASCII,// ASCII comportible 1byte
	ENCODING_TRAIT_UTF16LE,// UTF-16LE
	ENCODING_TRAIT_UTF16BE,// UTF-16BE
	ENCODING_TRAIT_UTF32LE,// UTF-32LE 0123
	ENCODING_TRAIT_UTF32BE,// UTF-32BE 3210
	ENCODING_TRAIT_EBCDIC_CRLF,// EBCDIC/CR,LF
	ENCODING_TRAIT_EBCDIC,// EBCDIC/CR,LF,NEL
};


/*
	システムコードページによる文字コード変換
*/
class CodePage : public CodeBase{
public:
	CodePage(int codepageEx) : nCodePageEx(codepageEx) { }
	
	//CodeBaseインターフェース
	CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst){ return CPToUnicode(src, pDst, nCodePageEx); }	// 特定コード → UNICODE    変換
	CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst){ return UnicodeToCP(src, pDst, nCodePageEx); }	// UNICODE    → 特定コード 変換
	void GetEol(Memory* pMemEol, EolType eolType);	// 改行データ取得
	void GetBom(Memory* pMemBom);	// BOMデータ取得
	CodeConvertResult UnicodeToHex(const wchar_t* cSrc, size_t iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar);			// UNICODE → Hex 変換

public:
	//実装
	static CodeConvertResult CPToUnicode(const Memory& src, NativeW* pDst, int codepageEx);		// CodePage  → Unicodeコード変換 
	static CodeConvertResult UnicodeToCP(const NativeW& src, Memory* pDst, int codepageEx);		// Unicode   → CodePageコード変換

	typedef std::vector<std::pair<int, std::wstring>> CodePageList;
	
	//GUI用補助関数
	static CodePage::CodePageList& GetCodePageList();
	static int GetNameNormal(LPTSTR outName, int charcodeEx);
	static int GetNameShort(LPTSTR outName, int charcodeEx);
	static int GetNameLong(LPTSTR outName, int charcodeEx);
	static int GetNameBracket(LPTSTR outName, int charcodeEx);
	static int AddComboCodePages(HWND hwnd, HWND combo, int nSelCode);
	
	//CP補助情報
	static EEncodingTrait GetEncodingTrait(int charcodeEx);
	
protected:
	// 実装
	static CodeConvertResult CPToUni(const char*, const size_t, wchar_t*, size_t, size_t&, UINT);
	static CodeConvertResult UniToCP(const wchar_t*, const size_t, char*, size_t, size_t&, UINT);
	
	int nCodePageEx;
	
	static BOOL CALLBACK CallBackEnumCodePages( LPCTSTR );

	static size_t MultiByteToWideChar2(UINT, int, const char*, size_t, wchar_t*, size_t);
	static size_t WideCharToMultiByte2(UINT, int, const wchar_t*, size_t, char*, size_t);
	static size_t S_UTF32LEToUnicode(const char*, size_t, wchar_t*, size_t);
	static size_t S_UTF32BEToUnicode(const char*, size_t, wchar_t*, size_t);
	static size_t S_UnicodeToUTF32LE(const wchar_t*, size_t, char*, size_t);
	static size_t S_UnicodeToUTF32BE(const wchar_t*, size_t, char*, size_t);
};

