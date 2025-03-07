/*!	@file
	@brief BREGEXP Library Handler

	Perl5互換正規表現を扱うDLLであるBREGEXP.DLLを利用するためのインターフェース
*/
#pragma once

#include "BregexpDll2.h"

/*!
	@brief Perl互換正規表現 BREGEXP.DLL をサポートするクラス

	DLLの動的ロードを行うため、DllHandlerを継承している。

	CJreに近い動作をさせるため、バッファをクラス内に1つ保持し、
	データの設定と検索の２つのステップに分割するようにしている。
	Jreエミュレーション関数を使うときは入れ子にならないように注意すること。

	本来はこのような部分は別クラスとして分離すべきだが、その場合このクラスが
	破棄される前に全てのクラスを破棄する必要がある。
	その安全性を保証するのが難しいため、現時点では両者を１つのクラスに入れた。

	@note このクラスはThread Safeではない。
*/
class Bregexp : public BregexpDll2 {
public:
	Bregexp();
	virtual ~Bregexp();

	enum Option {
		optNothing = 0,					// オプションなし
		optCaseSensitive = 1,			// 大文字小文字区別オプション(/iをつけない)
		optGlobal = 2,					// 全域オプション(/g)
		optExtend = 4,					// 拡張正規表現(/x)
		optASCII = 8,					// ASCII(/a)
		optUnicode = 0x10,				// Unicode(/u)
		optDefault = 0x20,				// Default(/d)
		optLocale = 0x40,				// Locale(/l)
		optR = 0x80,					// CRLF(/R)
	};
	// 検索パターン定義
	enum Pattern {
		PAT_UNKNOWN = 0,		// 不明（初期値)
		PAT_NORMAL = 1,			// 通常
		PAT_TOP = 2,			// 行頭"^"
		PAT_BOTTOM = 4,			// 行末"$"
		PAT_TAB = 8,			// 行頭行末"^$"
		PAT_LOOKAHEAD = 16		// 先読み"(?[=]"
	};

	// DLLのバージョン情報を取得
	const TCHAR* GetVersionT() { return IsAvailable() ? to_tchar(BRegexpVersion()) : _T(""); }

	// CJreエミュレーション関数
	// 検索パターンのコンパイル
	bool Compile(const wchar_t* szPattern, int nOption = 0) {
		return Compile(szPattern, NULL, nOption);
	}
	bool Compile(const wchar_t* szPattern0, const wchar_t* szPattern1, int nOption = 0, bool bKakomi = false);	// Replace用
	bool Match(const wchar_t* szTarget, size_t nLen, size_t nStart = 0);					// 検索を実行する
	int Replace(const wchar_t* szTarget, size_t nLen, size_t nStart = 0);					// 置換を実行する

	//-----------------------------------------
	/*! @{
		@name 結果を得るメソッドを追加し、BREGEXPを外部から隠す
	*/
	/*!
	    検索に一致した文字列の先頭位置を返す(文字列先頭なら0)
		@retval 検索に一致した文字列の先頭位置
	*/
	ptrdiff_t GetIndex(void) {
		return pRegExp->startp[0] - szTarget;
	}
	/*!
	    検索に一致した文字列の次の位置を返す
		@retval 検索に一致した文字列の次の位置
	*/
	ptrdiff_t GetLastIndex(void) {
		return pRegExp->endp[0] - szTarget;
	}
	/*!
		検索に一致した文字列の長さを返す
		@retval 検索に一致した文字列の長さ
	*/
	size_t GetMatchLen(void) {
		return pRegExp->endp[0] - pRegExp->startp[0];
	}
	/*!
		置換された文字列の長さを返す
		@retval 置換された文字列の長さ
	*/
	size_t GetStringLen(void) {
		// 置換後文字列が０幅なら outp、outendpもNULLになる
		// NULLポインタの引き算は問題なく０になる。
		// outendpは '\0'なので、文字列長は +1不要

		// Jun. 03, 2005 Karoto
		// 置換後文字列が0幅の場合にoutpがNULLでもoutendpがNULLでない場合があるので，
		// outpのNULLチェックが必要

		if (!pRegExp->outp) {
			return 0;
		} else {
			return pRegExp->outendp - pRegExp->outp;
		}
	}
	/*!
		置換された文字列を返す
		@retval 置換された文字列へのポインタ
	*/
	const wchar_t* GetString(void) {
		return pRegExp->outp;
	}
	/*! @} */
	//-----------------------------------------

	/*! BREGEXPメッセージを取得する
		@retval メッセージへのポインタ
	*/
	const TCHAR* GetLastMessage() const;// { return szMsg; }

	/*!	先読みパターンが存在するかを返す
		この関数は、コンパイル後であることが前提なので、コンパイル前はfalse
		@retval true 先読みがある
		@retval false 先読みがない 又は コンパイル前
	*/
	bool IsLookAhead(void) {
		return (ePatType & PAT_LOOKAHEAD) ? true : false;
	}
	/*!	検索パターンに先読みが含まれるか？（コンパイル前でも判別可能）
		@param[in] pattern 検索パターン
		@retval true 先読みがある
		@retval false 先読みがない
	*/
	bool IsLookAhead(const wchar_t* pattern) {
		CheckPattern(pattern);
		return IsLookAhead();
	}

protected:


	// コンパイルバッファを解放する
	/*!
		pcRegをBRegfree()に渡して解放する．解放後はNULLにセットする．
		元々NULLなら何もしない
	*/
	void ReleaseCompileBuffer(void) {
		if (pRegExp) {
			BRegfree( pRegExp );
			pRegExp = nullptr;
		}
		ePatType = PAT_UNKNOWN;
	}

private:
	// 内部関数

	// 検索パターン作成
	size_t CheckPattern( const wchar_t* szPattern );
	wchar_t* MakePatternSub( const wchar_t* szPattern, const wchar_t* szPattern2, const wchar_t* szAdd2, int nOption );
	wchar_t* MakePattern( const wchar_t* szPattern, const wchar_t* szPattern2, int nOption );
	wchar_t* MakePatternAlternate( const wchar_t* const szSearch, const wchar_t* const szReplace, int nOption );

	// メンバ変数
	BREGEXP_W*			pRegExp;			// コンパイル構造体
	int					ePatType;			// 検索文字列パターン種別
	const wchar_t*		szTarget;			// 対象文字列へのポインタ
	wchar_t				szMsg[80];		// BREGEXP_Wからのメッセージを保持する

	// 静的メンバ変数
	static const wchar_t	tmpBuf[2];	// ダミー文字列
};

// 正規表現ライブラリのバージョン取得
bool CheckRegexpVersion( HWND hWnd, int nCmpId, bool bShowMsg = false );
bool CheckRegexpSyntax( const wchar_t* szPattern, HWND hWnd, bool bShowMessage, int nOption = -1, bool bKakomi = false );
bool InitRegexp( HWND hWnd, Bregexp& rRegexp, bool bShowMessage );

