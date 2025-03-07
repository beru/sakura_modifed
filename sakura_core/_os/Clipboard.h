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
	bool SetText(const wchar_t* pData, size_t nDataLen, bool bColumnSelect, bool bLineSelect, UINT uFormat = (UINT)-1);   // テキストを設定する
	bool SetHtmlText(const NativeW& memBUf);
	bool GetText(NativeW* pMemBuf, bool* pbColumnSelect, bool* pbLineSelect, const Eol& eol, UINT uGetFormat = (UINT)-1); // テキストを取得する
	bool IsIncludeClipboradFormat(const wchar_t* pFormatName);
	bool SetClipboradByFormat(const StringRef& str, const wchar_t* pFormatName, int nMode, int nEndMode);
	bool GetClipboradByFormat(NativeW& mem, const wchar_t* pFormatName, int nMode, int nEndMode, const Eol& eol);
	
	// 演算子
	operator bool() const { return bOpenResult != FALSE; } // クリップボードを開けたならtrue
	
private:
	HWND hwnd;
	BOOL bOpenResult;
	
	// -- -- staticインターフェース -- -- //
public:
	static bool HasValidData();    // クリップボード内に、サクラエディタで扱えるデータがあればtrue
	static CLIPFORMAT GetSakuraFormat(); // サクラエディタ独自のクリップボードデータ形式
	static int GetDataType();      // クリップボードデータ形式(CF_UNICODETEXT等)の取得
};

