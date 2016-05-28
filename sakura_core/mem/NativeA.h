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

#include "Native.h"

class NativeA : public Native {
public:
	NativeA();
	NativeA(const NativeA& rhs);
	NativeA(const char* szData);
	NativeA(const char* pData, size_t nLength);

	// ネイティブ設定
	void SetString(const char* pszData);					// バッファの内容を置き換える
	void SetString(const char* pData, size_t nDataLen);		// バッファの内容を置き換える。nDataLenは文字単位。
	void SetNativeData(const NativeA& pcNative);			// バッファの内容を置き換える
	void AppendString(const char* pszData);					// バッファの最後にデータを追加する
	void AppendString(const char* pszData, size_t nLength);	// バッファの最後にデータを追加する。nLengthは文字単位。
	void AppendNativeData(const NativeA& pcNative);			// バッファの最後にデータを追加する
	void AllocStringBuffer(size_t nDataLen);					// (重要：nDataLenは文字単位) バッファサイズの調整。必要に応じて拡大する。

	// ネイティブ取得
	size_t GetStringLength() const;
	char operator[](size_t nIndex) const;						// 任意位置の文字取得。nIndexは文字単位。
	const char* GetStringPtr() const {
		return reinterpret_cast<const char*>(GetRawPtr());
	}
	char* GetStringPtr() {
		return reinterpret_cast<char*>(GetRawPtr());
	}
	const char* GetStringPtr(size_t* pnLength) const; // [out]pnLengthは文字単位。

	// 演算子
	const NativeA& operator = (char);
	const NativeA& operator += (char);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           変換                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// ネイティブ変換
	void Replace(const char* pszFrom, const char* pszTo);   // 文字列置換
	void Replace_j(const char* pszFrom, const char* pszTo); // 文字列置換（日本語考慮版）
	void ReplaceT( const char* pszFrom, const char* pszTo ){
		Replace_j( pszFrom, pszTo );
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                  型限定インターフェース                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 使用はできるだけ控えるのが望ましい。
	// ひとつはオーバーヘッドを抑える意味で。
	// ひとつは変換によるデータ喪失を抑える意味で。

	// WCHAR
	void SetStringNew(const wchar_t* wszData, size_t nDataLen);
	void SetStringNew(const wchar_t* wszData);
	void AppendStringNew(const wchar_t* pszData);               // バッファの最後にデータを追加する
	void AppendStringNew(const wchar_t* pszData, size_t nDataLen); // バッファの最後にデータを追加する。nDataLenは文字単位。
	void SetStringW(const wchar_t* pszData)					{ return SetStringNew(pszData); }
	void SetStringW(const wchar_t* pData, size_t nLength)		{ return SetStringNew(pData, nLength); }
	void AppendStringW(const wchar_t* pszData)				{ return AppendStringNew(pszData); }
	void AppendStringW(const wchar_t* pData, size_t nLength)	{ return AppendStringNew(pData, nLength); }
	const wchar_t* GetStringW() const;

	// TCHAR
#ifdef _UNICODE
	void SetStringT(const TCHAR* pszData)				{ return SetStringNew(pszData); }
	void SetStringT(const TCHAR* pData, size_t nLength)	{ return SetStringNew(pData, nLength); }
#else
	void SetStringT(const TCHAR* pszData)				{ return SetString(pszData); }
	void SetStringT(const TCHAR* pData, size_t nLength)	{ return SetString(pData, nLength); }
#endif

public:
	// -- -- staticインターフェース -- -- //
	static size_t GetSizeOfChar(const char* pData, size_t nDataLen, size_t nIdx); // 指定した位置の文字が何バイト文字かを返す
	static const char* GetCharNext(const char* pData, size_t nDataLen, const char* pDataCurrent); // ポインタで示した文字の次にある文字の位置を返します
	static const char* GetCharPrev(const char* pData, size_t nDataLen, const char* pDataCurrent); // ポインタで示した文字の直前にある文字の位置を返します
};

