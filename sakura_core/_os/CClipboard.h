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

class Eol;

// サクラエディタ用クリップボードクラス。後々はこの中で全てのクリップボードAPIを呼ばせたい。
class Clipboard {
public:
	// コンストラクタ・デストラクタ
	Clipboard(HWND hwnd); // コンストラクタ内でクリップボードが開かれる
	virtual ~Clipboard(); // デストラクタ内でCloseが呼ばれる

	// インターフェース
	void Empty(); // クリップボードを空にする
	void Close(); // クリップボードを閉じる
	bool SetText(const wchar_t* pData, int nDataLen, bool bColumnSelect, bool bLineSelect, UINT uFormat = (UINT)-1);   // テキストを設定する
	bool SetHtmlText(const NativeW& memBUf);
	bool GetText(NativeW* pMemBuf, bool* pbColumnSelect, bool* pbLineSelect, const Eol& eol, UINT uGetFormat = (UINT)-1); // テキストを取得する
	bool IsIncludeClipboradFormat(const wchar_t* pFormatName);
	bool SetClipboradByFormat(const StringRef& str, const wchar_t* pFormatName, int nMode, int nEndMode);
	bool GetClipboradByFormat(NativeW& mem, const wchar_t* pFormatName, int nMode, int nEndMode, const Eol& eol);
	
	// 演算子
	operator bool() const { return m_bOpenResult != FALSE; } // クリップボードを開けたならtrue
	
private:
	HWND m_hwnd;
	BOOL m_bOpenResult;
	
	// -- -- staticインターフェース -- -- //
public:
	static bool HasValidData();    // クリップボード内に、サクラエディタで扱えるデータがあればtrue
	static CLIPFORMAT GetSakuraFormat(); // サクラエディタ独自のクリップボードデータ形式
	static int GetDataType();      // クリップボードデータ形式(CF_UNICODETEXT等)の取得
};

