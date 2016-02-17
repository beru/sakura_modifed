/*
	Copyright (C) 2008, kobake

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

// 定数
enum class CodeConvertResult {
	Complete, // データを失うことなく変換が完了した。
	LoseSome, // 変換が完了したが、一部のデータが失われた。
	Failure,  // 何らかの原因により失敗した。
};

class Memory;
class NativeW;
struct CommonSetting_StatusBar;
enum class EolType;

/*!
	文字コード基底クラス。
	
	ここで言う「特定コード」とは、
	CodeBaseを継承した子クラスが定める、一意の文字コードのことです。
*/
class CodeBase {
public:
	virtual ~CodeBase() {}
//	virtual bool IsCode(const Memory* pMem) {return false;}  // 特定コードであればtrue

	// 文字コード変換
	virtual CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst) = 0;	// 特定コード → UNICODE    変換
	virtual CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst) = 0;	// UNICODE    → 特定コード 変換
	// UNICODE    → 特定コード 変換
	virtual CodeConvertResult UnicodeToCode(const StringRef& src, Memory* pDst) {
		NativeW mem(src.GetPtr(), src.GetLength());
		return UnicodeToCode(mem, pDst);
	}

	// ファイル形式
	virtual void GetBom(Memory* pMemBom);											// BOMデータ取得
	void GetEol(Memory* pMemEol, EolType eEolType) { S_GetEol(pMemEol, eEolType); }	// 改行データ取得 virtualから実体へ	2010/6/13 Uchi

	// 文字コード表示用		2008/6/9 Uchi
	virtual CodeConvertResult UnicodeToHex(const wchar_t* pSrc, const int iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar);			// UNICODE → Hex 変換

	// 変換エラー処理（１バイト <-> U+D800 から U+D8FF）
	static int BinToText(const unsigned char*, const int, unsigned short*);
	static int TextToBin(const unsigned short);

	// MIME Header デコーダ
	static bool MIMEHeaderDecode(const char*, const int, Memory*, const EncodingType);

	// CShiftJisより移動 2010/6/13 Uchi
	static void S_GetEol(Memory* pMemEol, EolType eEolType);	// 改行データ取得
	
protected:

};

/*!
	バイナリ１バイトを U+DC00 から U+DCFF までに対応付ける
*/
inline int CodeBase::BinToText(const unsigned char* pSrc, const int nLen, unsigned short* pDst)
{
	int i;

	for (i=0; i<nLen; ++i) {
		pDst[i] = static_cast<unsigned short>(pSrc[i]) + 0xdc00;
	}

	return i;
}


/*!
	U+DC00 から U+DCFF からバイナリ1バイトを復元
*/
inline int CodeBase::TextToBin(const unsigned short cSrc)
{
	return static_cast<int>((cSrc - 0xdc00) & 0x00ff);
}

