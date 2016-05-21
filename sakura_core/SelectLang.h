/*!	@file
	@brief 各国語メッセージリソース対応

	@author nasukoji
	@date 2011.04.10	新規作成
*/
/*
	Copyright (C) 2011, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include <windows.h>
#include <vector>

#define MAX_SELLANG_NAME_STR	128		// メッセージリソースの言語名の最大文字列長（サイズは適当）

class SelectLang {
public:
	// メッセージリソース用構造体
	struct SelLangInfo {
		TCHAR szDllName[MAX_PATH];		// メッセージリソースDLLのファイル名
		TCHAR szLangName[MAX_SELLANG_NAME_STR];		// 言語名
		HINSTANCE hInstance;			// 読み込んだリソースのインスタンスハンドル
		WORD wLangId;					// 言語ID
		BOOL bValid;					// メッセージリソースDLLとして有効
	};

protected:
	//static LPTSTR szDefaultLang;					// メッセージリソースDLL未読み込み時のデフォルト言語
	static SelLangInfo* psLangInfo;				// メッセージリソースの情報
public:
	typedef std::vector<SelLangInfo*> PSSelLangInfoList;
	static PSSelLangInfoList psLangInfoList;

public:
	/*
	||  Constructors
	*/
	SelectLang() {}
	~SelectLang();

	/*
	||  Attributes & Operations
	*/
	static HINSTANCE getLangRsrcInstance( void );			// メッセージリソースDLLのインスタンスハンドルを返す
	static LPCTSTR getDefaultLangString( void );			// メッセージリソースDLL未読み込み時のデフォルト言語（"(Japanese)" or "(English(United States))"）
	static WORD getDefaultLangId(void);

	static HINSTANCE InitializeLanguageEnvironment(void);		// 言語環境を初期化する
	static HINSTANCE LoadLangRsrcLibrary( SelLangInfo& lang );	// メッセージ用リソースDLLをロードする
	static void ChangeLang( TCHAR* pszDllName );	// 言語を変更する

protected:
	/*
	||  実装ヘルパ関数
	*/
	static HINSTANCE ChangeLang( UINT nSelIndex );	// 言語を変更する

private:
};


/*!
	@brief 文字列リソース読み込みクラス

	@date 2011.06.01 nasukoji	新規作成
*/

#define LOADSTR_ADD_SIZE		256			// 文字列リソース用バッファの初期または追加サイズ（TCHAR単位）

class ResourceString {
protected:
	// 文字列リソース読み込み用バッファクラス
	class LoadStrBuffer {
	public:
		LoadStrBuffer() {
			pszString   = szString;				// 変数内に準備したバッファを接続
			nBufferSize = _countof(szString);	// 配列個数
			nLength     = 0;
			szString[0] = 0;
		}

		// virtual
		~LoadStrBuffer() {
			// バッファを取得していた場合は解放する。
			if (pszString && pszString != szString) {
				delete[] pszString;
			}
		}

		/*virtual*/ LPCTSTR GetStringPtr() const { return pszString; }	// 読み込んだ文字列のポインタを返す
		/*virtual*/ size_t GetBufferSize() const { return nBufferSize; }	// 読み込みバッファのサイズ（TCHAR単位）を返す
		/*virtual*/ size_t GetStringLength() const { return nLength; }		// 読み込んだ文字数（TCHAR単位）を返す

		/*virtual*/ size_t Load( UINT uid );								// 文字列リソースを読み込む（読み込み実行部）

	protected:
		LPTSTR pszString;						// 文字列読み込みバッファのポインタ
		size_t nBufferSize;						// 取得配列個数（TCHAR単位）
		size_t nLength;							// 取得文字数（TCHAR単位）
		TCHAR szString[LOADSTR_ADD_SIZE];		// 文字列読み込みバッファ（バッファ拡張後は使用されない）

	private:
		LoadStrBuffer( const LoadStrBuffer& );					// コピー禁止とする
		LoadStrBuffer operator = ( const LoadStrBuffer& );		// 代入禁止とする
	};

	static LoadStrBuffer aLoadStrBufferTemp[4];		// 文字列読み込みバッファの配列（ResourceString::LoadStringSt() が使用する）
	static int nDataTempArrayIndex;					// 最後に使用したバッファのインデックス（ResourceString::LoadStringSt() が使用する）
	LoadStrBuffer loadStrBuffer;					// 文字列読み込みバッファ（ResourceString::LoadString() が使用する）

public:
	/*
	||  Constructors
	*/
	ResourceString() {}
	ResourceString( UINT uid ) { Load( uid ); }		// 文字列読み込み付きコンストラクタ
	/*virtual*/ ~ResourceString() {}


	/*
	||  Attributes & Operations
	*/
	/*virtual*/ LPCTSTR GetStringPtr() const { return loadStrBuffer.GetStringPtr(); }	// 読み込んだ文字列のポインタを返す
//	/*virtual*/ int GetBufferSize() const { return loadStrBuffer.GetBufferSize(); }		// 読み込みバッファのサイズ（TCHAR単位）を返す
	/*virtual*/ size_t GetStringLength() const { return loadStrBuffer.GetStringLength(); }	// 読み込んだ文字数（TCHAR単位）を返す

	static LPCTSTR LoadStringSt( UINT uid );			// 静的バッファに文字列リソースを読み込む（各国語メッセージリソース対応）
	/*virtual*/ LPCTSTR Load( UINT uid );			// 文字列リソースを読み込む（各国語メッセージリソース対応）

protected:

private:
	ResourceString( const ResourceString& );					// コピー禁止とする
	ResourceString operator = ( const ResourceString& );		// 代入禁止とする
};

// 文字列ロード簡易化マクロ
#define LS( id ) ( ResourceString::LoadStringSt( id ) )
#define LSW( id ) to_wchar( ResourceString::LoadStringSt( id ) )

