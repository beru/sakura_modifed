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
#include "mem/NativeT.h"
#include "basis/SakuraBasis.h"
#include "debug/Debug2.h" // assert


// 文字列への参照を取得するインターフェース
class IStringRef {
public:
	virtual const wchar_t*	GetPtr()	const = 0;
	virtual size_t			GetLength()	const = 0;
};


// 文字列への参照を保持するクラス
class StringRef : public IStringRef {
public:
	StringRef() : pData(NULL), nDataLen(0) { }
	StringRef(const wchar_t* pData, size_t nDataLen) : pData(pData), nDataLen(nDataLen) { }
	const wchar_t*	GetPtr()		const { return pData;    }
	size_t			GetLength()		const { return nDataLen; }

	//########補助
	bool			IsValid()		const { return pData != NULL; }
	wchar_t			At(size_t nIndex)	const { assert(nIndex < nDataLen); return pData[nIndex]; }
private:
	const wchar_t*	pData;
	size_t			nDataLen;
};


// UNICODE文字列管理クラス
class NativeW : public Native {
public:
	// コンストラクタ・デストラクタ
	NativeW();
	NativeW(const NativeW&);
	NativeW(const wchar_t* pData, size_t nDataLen); // nDataLenは文字単位。
	NativeW(const wchar_t* pData);

	// 管理
	void AllocStringBuffer(size_t nDataLen);                    // (重要：nDataLenは文字単位) バッファサイズの調整。必要に応じて拡大する。

	// wchar_t
	void SetString(const wchar_t* pData, size_t nDataLen);      // バッファの内容を置き換える。nDataLenは文字単位。
	void SetString(const wchar_t* pszData);                  // バッファの内容を置き換える
	void SetStringHoldBuffer( const wchar_t* pData, size_t nDataLen );
	void AppendString(const wchar_t* pszData);               // バッファの最後にデータを追加する
	void AppendString(const wchar_t* pszData, size_t nLength);  // バッファの最後にデータを追加する。nLengthは文字単位。成功すればtrue。メモリ確保に失敗したらfalseを返す。

	template <size_t nLength>
	__forceinline void AppendStringLiteral(const wchar_t(&pszData)[nLength])
	{
		AppendString(pszData, nLength-1);
	}

	// NativeW
	void SetNativeData(const NativeW& pcNative);            // バッファの内容を置き換える
	void AppendNativeData(const NativeW&);                  // バッファの最後にデータを追加する

	// 演算子
	const NativeW& operator += (wchar_t wch)			{ AppendString(&wch, 1);   return *this; }
	const NativeW& operator = (wchar_t wch)				{ SetString(&wch, 1);      return *this; }
	const NativeW& operator += (const NativeW& rhs)		{ AppendNativeData(rhs); return *this; }
	const NativeW& operator = (const NativeW& rhs)		{ SetNativeData(rhs);    return *this; }
	NativeW operator + (const NativeW& rhs) const		{ NativeW tmp = *this; return tmp += rhs; }

	// ネイティブ取得インターフェース
	wchar_t operator [] (size_t nIndex) const;					// 任意位置の文字取得。nIndexは文字単位。
	size_t GetStringLength() const {						// 文字列長を返す。文字単位。
		return Native::GetRawLength() / sizeof(wchar_t);
	}
	const wchar_t* GetStringPtr() const {
		return reinterpret_cast<const wchar_t*>(GetRawPtr());
	}
	wchar_t* GetStringPtr() {
		return reinterpret_cast<wchar_t*>(GetRawPtr());
	}
	const wchar_t* GetStringPtr(size_t* pnLength) const {		// [out]pnLengthは文字単位。
		*pnLength = GetStringLength();
		return reinterpret_cast<const wchar_t*>(GetRawPtr());
	}

	// 特殊
	void _SetStringLength(size_t nLength) {
		_GetMemory()->_SetRawLength(nLength * sizeof(wchar_t));
	}
	// 末尾を1文字削る
	void Chop() {
		size_t n = GetStringLength();
		if (n > 0) {
			n -= 1;
			_SetStringLength(n);
		}
	}
	void swap(NativeW& left) {
		_GetMemory()->swap(*left._GetMemory());
	}
	
	size_t capacity() {
		return _GetMemory()->capacity() / sizeof(wchar_t);
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           判定                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	
	// 同一の文字列ならtrue
	static bool IsEqual(const NativeW& mem1, const NativeW& mem2);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           変換                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void Replace(const wchar_t* pszFrom, const wchar_t* pszTo);   // 文字列置換
	void ReplaceT( const wchar_t* pszFrom, const wchar_t* pszTo ){
		Replace( pszFrom, pszTo );
	}
	void Replace( const wchar_t* pszFrom, size_t nFromLen, const wchar_t* pszTo, size_t nToLen );   // 文字列置換


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                  型限定インターフェース                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 使用はできるだけ控えるのが望ましい。
	// ひとつはオーバーヘッドを抑える意味で。
	// ひとつは変換によるデータ喪失を抑える意味で。

	// char
	void SetStringOld(const char* pData, size_t nDataLen);    // バッファの内容を置き換える。pDataはSJIS。nDataLenは文字単位。
	void SetStringOld(const char* pszData);                // バッファの内容を置き換える。pszDataはSJIS。
	void AppendStringOld(const char* pData, size_t nDataLen); // バッファの最後にデータを追加する。pszDataはSJIS。
	void AppendStringOld(const char* pszData);             // バッファの最後にデータを追加する。pszDataはSJIS。
	const char* GetStringPtrOld() const; // ShiftJISに変換して返す

	// wchar_t
	void SetStringW(const wchar_t* pszData)					{ return SetString(pszData); }
	void SetStringW(const wchar_t* pData, size_t nLength)	{ return SetString(pData, nLength); }
	void AppendStringW(const wchar_t* pszData)				{ return AppendString(pszData); }
	void AppendStringW(const wchar_t* pData, size_t nLength){ return AppendString(pData, nLength); }
	const wchar_t* GetStringW() const						{ return GetStringPtr(); }

	// TCHAR
	void SetStringT(const TCHAR* pData, size_t nDataLen)	{ return SetString(pData, nDataLen); }
	void SetStringT(const TCHAR* pszData)					{ return SetString(pszData); }
	void AppendStringT(const TCHAR* pszData)				{ return AppendString(pszData); }
	void AppendStringT(const TCHAR* pData, size_t nLength)	{ return AppendString(pData, nLength); }
	void AppendNativeDataT(const NativeT& rhs)				{ return AppendNativeData(rhs); }
	const TCHAR* GetStringT() const							{ return GetStringPtr(); }

#if _DEBUG
private:
	typedef wchar_t* PWCHAR;
	PWCHAR& pDebugData; // デバッグ用。CMemoryの内容をwchar_t*型でウォッチするためのモノ
#endif

public:
	// -- -- staticインターフェース -- -- //
	static size_t GetSizeOfChar(const wchar_t* pData, size_t nDataLen, size_t nIdx);	// 指定した位置の文字がwchar_t何個分かを返す
	static size_t GetKetaOfChar(const wchar_t* pData, size_t nDataLen, size_t nIdx);	// 指定した位置の文字が半角何個分かを返す
	static const wchar_t* GetCharNext(const wchar_t* pData, size_t nDataLen, const wchar_t* pDataCurrent); // ポインタで示した文字の次にある文字の位置を返します
	static const wchar_t* GetCharPrev(const wchar_t* pData, size_t nDataLen, const wchar_t* pDataCurrent); // ポインタで示した文字の直前にある文字の位置を返します

	static size_t GetKetaOfChar(const StringRef& str, size_t nIdx) { // 指定した位置の文字が半角何個分かを返す
		return GetKetaOfChar(str.GetPtr(), str.GetLength(), nIdx);
	}
};

namespace std {
	template <>
	inline void swap(NativeW& n1, NativeW& n2) {
		n1.swap(n2);
	}
}

