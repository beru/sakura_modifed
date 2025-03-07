#pragma once

#include "basis/SakuraBasis.h"
class NativeW;

// 文字種類識別子
enum ECharKind {
	CK_NULL,			// NULL
	CK_TAB,				// タブ 0x9<=c<=0x9
	CK_CR,				// CR = 0x0d 
	CK_LF,				// LF = 0x0a 
	CK_CTRL,			// 上記以外の c<0x20

	CK_SPACE,			// 半角のスペース 0x20<=c<=0x20
	CK_CSYM,			// 識別子に使用可能な文字 (英数字、アンダースコア)
	CK_KATA,			// 半角のカタカナ 0xA1<=c<=0xFD
	CK_LATIN,			// ラテン１補助、ラテン拡張のうちアルファベット風のもの 0x00C0<=c<0x0180
	CK_UDEF,			// ユーザ定義キーワード文字（#$@\）
	CK_ETC,				// 半角のその他

	CK_ZEN_SPACE,		// 全角スペース
	CK_ZEN_NOBASU,		// 伸ばす記号 0x815B<=c<=0x815B 'ー'
	CK_ZEN_DAKU,		// 全角濁点 0x309B<=c<=0x309C 「゛゜」
	CK_ZEN_CSYM,		// 全角版、識別子に使用可能な文字 (英数字、アンダースコア)

	CK_ZEN_KIGO,		// 全角の記号
	CK_HIRA,			// ひらがな
	CK_ZEN_KATA,		// 全角カタカナ
	CK_GREEK,			// ギリシャ文字
	CK_ZEN_ROS,			// ロシア文字:
	CK_ZEN_SKIGO,		// 全角の特殊記号
	CK_ZEN_ETC,			// 全角のその他（漢字など）
};

class WordParse {
public:
	/*!
		@brief 現在位置の単語の範囲を調べる staticメンバ
		@retval true	成功 現在位置のデータは「単語」と認識する。
		@retval false	失敗 現在位置のデータは「単語」とは言いきれない気がする。
	*/
	static bool WhereCurrentWord_2(
		const wchar_t*	pLine,			// [in]  調べるメモリ全体の先頭アドレス
		size_t			nLineLen,		// [in]  調べるメモリ全体の有効長
		size_t			nIdx,			// [out] 調査開始地点:pLineからの相対的な位置
		size_t*			pnIdxFrom,		// [out] 単語が見つかった場合は、単語の先頭インデックスを返す。
		size_t*			pnIdxTo,		// [out] 単語が見つかった場合は、単語の終端の次のバイトの先頭インデックスを返す。
		NativeW*		pcmcmWord,		// [out] 単語が見つかった場合は、現在単語を切り出して指定されたCMemoryオブジェクトに格納する。情報が不要な場合はNULLを指定する。
		NativeW*		pcmcmWordLeft	// [out] 単語が見つかった場合は、現在単語の左に位置する単語を切り出して指定されたCMemoryオブジェクトに格納する。情報が不要な場合はNULLを指定する。
	);

	// 現在位置の文字の種類を調べる
	static ECharKind WhatKindOfChar(
		const wchar_t*	pData,
		size_t			pDataLen,
		size_t			nIdx
	);

	// 二つの文字を結合したものの種類を調べる
	static ECharKind WhatKindOfTwoChars(
		ECharKind		kindPre,
		ECharKind		kindCur
	);

	// 二つの文字を結合したものの種類を調べる for 強調キーワード
	static ECharKind WhatKindOfTwoChars4KW(
		ECharKind		kindPre,
		ECharKind		kindCur
	);

	//	pLine（長さ：nLineLen）の文字列から次の単語を探す。探し始める位置はnIdxで指定。
	static bool SearchNextWordPosition(
		const wchar_t*	pLine,
		size_t			nLineLen,
		size_t			nIdx,			//	桁数
		size_t*			pnColumnNew,	//	見つかった位置
		bool			bStopsBothEnds	//	単語の両端で止まる
	);

	//	pLine（長さ：nLineLen）の文字列から次の単語を探す。探し始める位置はnIdxで指定。 for 強調キーワード
	static bool SearchNextWordPosition4KW(
		const wchar_t*	pLine,
		size_t			nLineLen,
		size_t			nIdx,			//	桁数
		size_t*			pnColumnNew,	//	見つかった位置
		bool			bStopsBothEnds	//	単語の両端で止まる
	);


	template <class CHAR_TYPE>
	static size_t GetWord(const CHAR_TYPE*, const size_t, const CHAR_TYPE* pszSplitCharList,
		CHAR_TYPE** ppWordStart, size_t* pnWordLen);

protected:

	static bool _match_charlist(const char c, const char* pszList);
	static bool _match_charlist(const wchar_t c, const wchar_t* pszList);
};

bool IsURL(const wchar_t*, size_t, size_t*);			// 指定アドレスがURLの先頭ならばTRUEとその長さを返す
bool IsMailAddress(const wchar_t*, size_t, size_t*);	// 現在位置がメールアドレスならば、NULL以外と、その長さを返す


// char 版
inline bool WordParse::_match_charlist(const char c, const char* pszList)
{
	for (size_t i=0; pszList[i]!='\0'; ++i) {
		if (pszList[i] == c) {
			return true;
		}
	}
	return false;
}
// wchar_t 版
inline bool WordParse::_match_charlist(const wchar_t c, const wchar_t* pszList)
{
	for (size_t i=0; pszList[i]!=L'\0'; ++i) {
		if (pszList[i] == c) {
			return true;
		}
	}
	return false;
}

/*!
	@param [in] pS					文字列バッファ
	@param [in] nLen				文字列バッファの長さ
	@param [in] pszSplitCharList	区切り文字たち
	@param [out] ppWordStart		単語の開始位置
	@param [out] pnWordLen			単語の長さ

	@return 読んだデータの長さ。
*/
template <class CHAR_TYPE>
size_t WordParse::GetWord(
	const CHAR_TYPE* pS,
	const size_t nLen,
	const CHAR_TYPE* pszSplitCharList,
	CHAR_TYPE** ppWordStart,
	size_t* pnWordLen)
{
	const CHAR_TYPE* pr = pS;
	CHAR_TYPE* pwordstart;
	size_t nwordlen;

	if (nLen < 1) {
		pwordstart = const_cast<CHAR_TYPE *>(pS);
		nwordlen = 0;
		goto end_func;
	}

	// 区切り文字をスキップ
	for (; pr<pS+nLen; ++pr) {
		// 区切り文字でない文字の間ループ
		if (!_match_charlist(*pr, pszSplitCharList)) {
			break;
		}
	}
	pwordstart = const_cast<CHAR_TYPE*>(pr);   // 単語の先頭位置を記録

	// 単語をスキップ
	for (; pr<pS+nLen; ++pr) {
		// 区切り文字がくるまでループ
		if (_match_charlist(*pr, pszSplitCharList)) {
			break;
		}
	}
	nwordlen = pr - pwordstart;  // 単語の長さを記録

end_func:
	if (ppWordStart) {
		*ppWordStart = pwordstart;
	}
	if (pnWordLen) {
		*pnWordLen = nwordlen;
	}
	return pr - pS;
}

