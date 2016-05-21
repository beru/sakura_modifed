/*!	@file
	@brief メモリバッファクラス

	@author Norio Nakatani
	@date 1998/03/06 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, jepro, Stonee
	Copyright (C) 2002, Moca, genta, aroka
	Copyright (C) 2005, D.S.Koba

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

// ファイル文字コードセット判別時の先読み最大サイズ
#define CheckKanjiCode_MAXREADLENGTH 16384

#include "_main/global.h"

// メモリバッファクラス
class Memory {
	// コンストラクタ・デストラクタ
public:
	Memory();
	Memory(const Memory& rhs);
	Memory(const void* pData, size_t nDataLenBytes);
	virtual ~Memory();
protected:
	void _init_members();

	// インターフェース
public:
	void AllocBuffer(size_t);											// バッファサイズの調整。必要に応じて拡大する。
	void SetRawData(const void* pData, size_t nDataLen);				// バッファの内容を置き換える
	void SetRawData(const Memory&);										// バッファの内容を置き換える
	void SetRawDataHoldBuffer( const void* pData, size_t nDataLen );	// バッファの内容を置き換える(バッファを保持)
	void SetRawDataHoldBuffer( const Memory& );						// バッファの内容を置き換える(バッファを保持)
	void AppendRawData(const void* pData, size_t nDataLen);			// バッファの最後にデータを追加する
	void AppendRawData(const Memory*);								// バッファの最後にデータを追加する
	void Clean() { _Empty(); }
	void Clear() { _Empty(); }

	inline const void* GetRawPtr(size_t* pnLength) const;			// データへのポインタと長さ返す
	inline void* GetRawPtr(size_t* pnLength);						// データへのポインタと長さ返す
	inline const void* GetRawPtr() const { return pRawData; } // データへのポインタを返す
	inline void* GetRawPtr() { return pRawData; }				// データへのポインタを返す
	size_t GetRawLength() const { return nRawLen; }				// データ長を返す。バイト単位。

	// 演算子
	const Memory& operator = (const Memory&);

	// 比較
	static int IsEqual(Memory&, Memory&);	// 等しい内容か

	// 変換関数
	static void SwapHLByte(char*, const size_t); // 下記関数のstatic関数版
	void SwapHLByte();					// Byteを交換する
	bool SwabHLByte( const Memory& );	// Byteを交換する(コピー版)


protected:
	/*
	||  実装ヘルパ関数
	*/
	void _Empty(void); // 解放する。pRawDataはNULLになる。
	void _AddData(const void*, size_t);
public:
	void _AppendSz(const char* str);
	void _SetRawLength(size_t nLength);
	void swap(Memory& left) {
		std::swap(nDataBufSize, left.nDataBufSize);
		std::swap(pRawData, left.pRawData);
		std::swap(nRawLen, left.nRawLen);
	}
	int capacity() const { return nDataBufSize ? nDataBufSize - 2: 0; }

#ifdef _DEBUG
protected:
	typedef char* PCHAR;
	PCHAR& _DebugGetPointerRef() { return pRawData; } // デバッグ用。バッファポインタの参照を返す。
#endif

private: // 2002/2/10 aroka アクセス権変更
	/*
	|| メンバ変数
	*/
	char*	pRawData;		// バッファ
	size_t	nRawLen;		// データサイズ(nDataBufSize以内)。バイト単位。
	size_t	nDataBufSize;	//バッファサイズ。バイト単位。
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     inline関数の実装                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
inline
const void* Memory::GetRawPtr(size_t* pnLength) const // データへのポインタと長さ返す
{
	if (pnLength) *pnLength = GetRawLength();
	return pRawData;
}

inline
void* Memory::GetRawPtr(size_t* pnLength) // データへのポインタと長さ返す
{
	if (pnLength) *pnLength = GetRawLength();
	return pRawData;
}

