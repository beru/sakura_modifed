#pragma once

#include "parse/WordParse.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         判定関数                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           定数                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// SJISのコードページ(CP_ACP では無くこれを使えばおそらく英語版Winでも動くはず。)
#define CP_SJIS		932


// 定数の素 (直接使用は控えてください)
#define TAB_ 				'\t'
#define SPACE_				' '
#define CR_					'\015'
#define LF_					'\012'
#define ESC_				'\x1b'
#define CRLF_				"\015\012"

// ANSI定数
namespace ACODE {
	// 文字
	static const char TAB   = TAB_;
	static const char SPACE = SPACE_;
	static const char CR	= CR_;
	static const char LF	= LF_;
	static const char ESC	= ESC_;

	// 文字列
	static const char CRLF[] = CRLF_;

	// 特殊 (BREGEXP)
	static const wchar_t BREGEXP_DELIMITER = (wchar_t)0xFF;
}

// UNICODE定数
namespace WCODE {
	// 文字
	static const wchar_t TAB   = LCHAR(TAB_);
	static const wchar_t SPACE = LCHAR(SPACE_);
	static const wchar_t CR    = LCHAR(CR_);
	static const wchar_t LF    = LCHAR(LF_);
	static const wchar_t ESC   = LCHAR(ESC_);

	// 文字列
	static const wchar_t CRLF[] = LTEXT(CRLF_);

	// 特殊 (BREGEXP)
	//$$ UNICODE版の仮デリミタ。bregonigの仕様がよくわかんないので、とりあえずこんな値にしてます。
	static const wchar_t BREGEXP_DELIMITER = (wchar_t)0xFFFF;

}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         判定関数                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// キーワードキャラクタ
extern const unsigned char g_keyword_char[128];

inline bool IS_KEYWORD_CHAR(wchar_t wc)
{
	return (1
		&& 0 <= wc
		&& wc < _countof(g_keyword_char)
		&& (g_keyword_char[wc] == CK_CSYM||g_keyword_char[wc] == CK_UDEF)
	);
}


// UNICODE判定関数群
namespace WCODE {
	inline bool IsAZ(wchar_t wc) {
		return (wc >= L'A' && wc <= L'Z') || (wc >= L'a' && wc <= L'z');
	}
	inline bool Is09(wchar_t wc) {
		return (wc >= L'0' && wc <= L'9');
	}
	inline bool IsInRange(wchar_t c, wchar_t front, wchar_t back) {
		return c >= front && c <= back;
	}

	bool CalcHankakuByFont(wchar_t c);

	// 半角文字(縦長長方形)かどうか判定
	inline bool IsHankaku(wchar_t wc)
	{
		// ※ほぼ未検証。

		// 参考：http://www.swanq.co.jp/blog/archives/000783.html
		if (
			   wc <= 0x007E // ACODEとか
//			|| wc == 0x00A5 // 円マーク
//			|| wc == 0x203E // にょろ
			|| (wc >= 0xFF61 && wc <= 0xFF9f)	// 半角カタカナ
		) return true;

		// 0x7F ～ 0xA0 も半角とみなす
		// http://ja.wikipedia.org/wiki/Unicode%E4%B8%80%E8%A6%A7_0000-0FFF を見て、なんとなく
		if (wc >= 0x007F && wc <= 0x00A0) return true;	// Control Code ISO/IEC 6429

		// 漢字はすべて同一幅とみなす
		if (wc >= 0x4E00 && wc <= 0x9FBB		// Unified Ideographs, CJK
		  || wc>= 0x3400 && wc <= 0x4DB5		// Unified Ideographs Extension A, CJK
		) {
			wc = 0x4E00; // '一'(0x4E00)の幅で代用
		}
		else
		// ハングルはすべて同一幅とみなす
		if (wc >= 0xAC00 && wc <= 0xD7A3) {		// Hangul Syllables
			wc = 0xAC00; // (0xAC00)の幅で代用
		}
		else
		// 外字はすべて同一幅とみなす
		if (wc >= 0xE000 && wc <= 0xE8FF) { // Private Use Area
			wc = 0xE000; // (0xE000)の幅で代用
		}

		//$$ 仮。もう動的に計算しちゃえ。(初回のみ)
		return CalcHankakuByFont(wc);
	}

	// 制御文字であるかどうか
	inline bool IsControlCode(wchar_t wc)
	{
		//// 改行は制御文字とみなさない
		//if (IsLineDelimiter(wc)) return false;

		//// タブは制御文字とみなさない
		//if (wc == TAB) return false;

		//return iswcntrl(wc) != 0;
		return (wc < 128 && g_keyword_char[wc] == CK_CTRL);
	}

	// 全角文字(正方形)かどうか判定
	inline bool IsZenkaku(wchar_t wc) {
		return !IsHankaku(wc);
	}

	// 全角スペースかどうか判定
	inline bool IsZenkakuSpace(wchar_t wc) {
		return wc == 0x3000; // L'　'
	}

	// 改行文字であるかどうか
	inline bool IsLineDelimiter(wchar_t wc, bool ext) {
		return wc == CR || wc == LF || wc == 0x85 || wc == 0x2028 || wc == 0x2029;
	}
	inline bool IsLineDelimiterBasic(wchar_t wc) {
		return wc==CR || wc==LF;
	}
	inline bool IsLineDelimiterExt(wchar_t wc) {
		return wc==CR || wc==LF || wc==0x85 || wc==0x2028 || wc==0x2029;
	}

	// 単語の区切り文字であるかどうか
	inline bool IsWordDelimiter(wchar_t wc) {
		return wc == SPACE || wc == TAB || IsZenkakuSpace(wc);
	}

	// インデント構成要素であるかどうか。bAcceptZenSpace: 全角スペースを含めるかどうか
	inline bool IsIndentChar(wchar_t wc, bool bAcceptZenSpace) {
		if (wc == TAB || wc == SPACE) return true;
		if (bAcceptZenSpace && IsZenkakuSpace(wc)) return true;
		return false;
	}

	// 空白かどうか
	inline bool IsBlank(wchar_t wc) {
		return wc == TAB || wc == SPACE || IsZenkakuSpace(wc);
	}

	// ファイル名に使える文字であるかどうか
	inline bool IsValidFilenameChar(const wchar_t* pData, size_t nIndex) {
		static const wchar_t* table = L"<>?\"|*";

		wchar_t wc = pData[nIndex];
		return (wcschr(table, wc) == NULL); // table内の文字が含まれていたら、ダメ。
	}

	// タブ表示に使える文字かどうか
	inline bool IsTabAvailableCode(wchar_t wc) {
		//$$要検証
		if (wc == L'\0') return false;
		if (wc == L'\r') return false;
		if (wc == L'\n') return false;
		if (wc == L'\t') return false;
		return true;
	}

	// 半角カナかどうか
	inline bool IsHankakuKatakana(wchar_t c) {
		// 参考: http://ash.jp/code/unitbl1.htm
		return c >= 0xFF61 && c <= 0xFF9F;
	}

	// 全角記号かどうか
	inline bool IsZenkakuKigou(wchar_t c) {
		//$ 他にも全角記号はあると思うけど、とりあえずANSI版時代の判定を踏襲。パフォーマンス悪し。
		// 「ゝゞ（ひらがな）」「ヽヾ（カタカナ）」「゛゜（全角濁点）」「仝々〇（漢字）」「ー（長音）」を除外
		// ANSI版の修正にあわせて「〆」を記号→漢字にする
		static const wchar_t* table = L"　、。，．・：；？！´｀¨＾￣＿〃―‐／＼～∥｜…‥‘’“”（）〔〕［］｛｝〈〉《》「」『』【】＋－±×÷＝≠＜＞≦≧∞∴♂♀°′″℃￥＄￠￡％＃＆＊＠§☆★○●◎◇◆□■△▲▽▼※〒→←↑↓〓∈∋⊆⊇⊂⊃∪∩∧∨￢⇒⇔∀∃∠⊥⌒∂∇≡≒≪≫√∽∝∵∫∬Å‰♯♭♪†‡¶◯";
		return wcschr(table, c) != NULL;
	}

	// ひらがなかどうか
	inline bool IsHiragana(wchar_t c) {
		// 「ゝゞ」を追加
		return (c >= 0x3041 && c <= 0x3096) || (c >= 0x309D && c <= 0x309E);
	}

	// カタカナかどうか
	inline bool IsZenkakuKatakana(wchar_t c) {
		// 「ヽヾ」を追加
		return (c >= 0x30A1 && c <= 0x30FA) || (c >= 0x30FD && c <= 0x30FE);
	}

	// ギリシャ文字かどうか
	inline bool IsGreek(wchar_t c) {
		return c >= 0x0391 && c <= 0x03C9;
	}

	// キリル文字かどうか
	inline bool IsCyrillic(wchar_t c) {
		return c >= 0x0410 && c <= 0x044F;
	}

	// BOX DRAWING 文字 かどうか
	inline bool IsBoxDrawing(wchar_t c) {
		return c >= 0x2500 && c <= 0x257F;
	}

	// 句読点か
	//bool IsKutoten(wchar_t wc);

}


// ANSI判定関数群
namespace ACODE
{
	inline bool IsAZ(char c) {
		return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
	}

	// 制御文字であるかどうか
	inline bool IsControlCode(char c) {
		unsigned char n = (unsigned char)c;
		if (c == TAB) return false;
		if (c == CR) return false;
		if (c == LF) return false;
		if (n <= 0x1F) return true;
		if (n >= 0x7F && n <= 0x9F) return true;
		if (n >= 0xE0) return true;
		return false;
	}

	// タブ表示に使える文字かどうか
	inline bool IsTabAvailableCode(char c) {
		if (c == '\0') return false;
		if (c <= 0x1f) return false;
		if (c >= 0x7f) return false;
		return true;
	}

	// ファイル名に使える文字であるかどうか
	inline bool IsValidFilenameChar(const char* pData, size_t nIndex) {
		static const char* table = "<>?\"|*";
		char c = pData[nIndex];
		// table内の文字が含まれていて
		return (strchr(table, c) == NULL);
	}
}

// TCHAR判定関数群
namespace TCODE {
	using namespace WCODE;
}

// 文字幅の動的計算用キャッシュ関連
struct CharWidthCache {
	// 文字半角全角キャッシュ
	TCHAR		lfFaceName[LF_FACESIZE];
	BYTE		bCharWidthCache[0x10000/4];		// 16KB 文字半角全角キャッシュ
	int			nCharWidthCacheTest;				// cache溢れ検出
};

enum class CharWidthFontMode {
	Edit,
	Print,
	MiniMap,
	Max,
};

enum class CharWidthCacheMode {
	Neutral,
	Share,
	Local,
};

// キャッシュの初期化関数群
void SelectCharWidthCache(CharWidthFontMode fMode, CharWidthCacheMode cMode);  // モードを変更したいとき
void InitCharWidthCache(const LOGFONT& lf, CharWidthFontMode fMode = CharWidthFontMode::Edit); // フォントを変更したとき

