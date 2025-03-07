#pragma once

#include "_main/global.h"
#include "extmodule/Bregexp.h"
#include "config/maxdata.h" // MAX_REGEX_KEYWORD

struct TypeConfig;

#define USE_PARENT	// 親を使ってキーワード格納領域を削減する。


struct RegexKeywordInfo {
	int	nColorIndex;		// 色指定番号
};

// 正規表現キーワード検索情報構造体
struct RegexInfo {
	BREGEXP_W* pBregexp;	// BREGEXP_W構造体
#ifdef USE_PARENT
#else
	struct RegexKeywordInfo	sRegexKey;	// コンパイルパターンを保持
#endif
	int    nStatus;		// 状態(EMPTY,CLOSE,OPEN,ACTIVE,ERROR)
	int    nMatch;		// このキーワードのマッチ状態(EMPTY,MATCH,NOMATCH)
	int    nOffset;		// マッチした位置
	int    nLength;		// マッチした長さ
	int    nHead;		// 先頭のみチェックするか？
	int    nFlag;		// 色指定のチェックが入っているか？ YES=RK_EMPTY, NO=RK_NOMATCH
};


// 正規表現キーワードクラス
/*!
	正規表現キーワードを扱う。
*/
class RegexKeyword : public Bregexp {
public:
	RegexKeyword(LPCTSTR);
	~RegexKeyword();

	// 行検索開始
	bool RegexKeyLineStart(void);
	// 行検索
	bool RegexIsKeyword(const StringRef& str, int nPos, size_t* nMatchLen, int* nMatchColor);
	// タイプ設定
	bool RegexKeySetTypes(const TypeConfig* pTypesPtr);

	// 書式(囲み)チェック
	static bool RegexKeyCheckSyntax(const wchar_t* s);
	
	static DWORD GetNewMagicNumber();

protected:
	// コンパイル
	bool RegexKeyCompile(void);
	// 変数初期化
	bool RegexKeyInit(void);

public:
	int				nTypeIndex;				// 現在のタイプ設定番号
	bool			bUseRegexKeyword;		// 正規表現キーワードを使用する・しない

private:
	const TypeConfig*	pTypes;					// タイプ設定へのポインタ(呼び出し側が持っているもの)
	int				nTypeId;					// タイプ設定ID
	DWORD			nCompiledMagicNumber;		// コンパイル済みか？
	int				nRegexKeyCount;				// 現在のキーワード数
	RegexInfo		info[MAX_REGEX_KEYWORD];	// キーワード一覧(BREGEXPコンパイル対象)
#ifdef USE_PARENT
#else
	wchar_t			keywordList[MAX_REGEX_KEYWORDLISTLEN];
#endif
	wchar_t			szMsg[256];				// BREGEXP_Wからのメッセージを保持する
};

