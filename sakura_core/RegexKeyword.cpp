/*!	@file
	@brief RegexKeyword Library

	正規表現キーワードを扱う。
	BREGEXP.DLLを利用する。

	@author MIK
	@date Nov. 17, 2001
*/
/*
	Copyright (C) 2001, MIK
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

//@@@ 2001.11.17 add start MIK

#include "StdAfx.h"
#include "RegexKeyword.h"
#include "extmodule/Bregexp.h"
#include "types/Type.h"
#include "view/colors/EColorIndexType.h"

#if 0
#include <stdio.h>
#define	MYDBGMSG(s) \
{\
	FILE* fp = fopen("debug.log", "a");\
	fprintf(fp, "%08x: %ls  BMatch(%d)=%d, Use=%d, Idx=%d\n", &m_pTypes, s, &BMatch, BMatch, m_bUseRegexKeyword, m_nTypeIndex);\
	fclose(fp);\
}
#else
#define	MYDBGMSG(a)
#endif

/*
 * パラメータ宣言
 */
#define RK_EMPTY          0      // 初期状態
#define RK_CLOSE          1      // BREGEXPクローズ
#define RK_OPEN           2      // BREGEXPオープン
#define RK_ACTIVE         3      // コンパイル済み
#define RK_ERROR          9      // コンパイルエラー

#define RK_MATCH          4      // マッチする
#define RK_NOMATCH        5      // この行ではマッチしない

#define RK_SIZE           100    // 最大登録可能数

//#define RK_HEAD_CHAR      '^'    // 行先頭の正規表現
#define RK_HEAD_STR1      L"/^"   // BREGEXP
#define RK_HEAD_STR1_LEN  2
#define RK_HEAD_STR2      L"m#^"  // BREGEXP
#define RK_HEAD_STR2_LEN  3
#define RK_HEAD_STR3      L"m/^"  // BREGEXP
#define RK_HEAD_STR3_LEN  3
//#define RK_HEAD_STR4      "#^"   // BREGEXP
//#define RK_HEAD_STR4_LEN  2

#define RK_KAKOMI_1_START "/"
#define RK_KAKOMI_1_END   "/k"
#define RK_KAKOMI_2_START "m#"
#define RK_KAKOMI_2_END   "#k"
#define RK_KAKOMI_3_START "m/"
#define RK_KAKOMI_3_END   "/k"
//#define RK_KAKOMI_4_START "#"
//#define RK_KAKOMI_4_END   "#k"


// コンストラクタ
/*!	@brief コンストラクタ

	BREGEXP.DLL 初期化、正規表現キーワード初期化を行う。

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2007.08.12 genta 正規表現DLL指定のため引数追加
*/
RegexKeyword::RegexKeyword(LPCTSTR regexp_dll )
{
	InitDll(regexp_dll);	// 2007.08.12 genta 引数追加
	MYDBGMSG("RegexKeyword")

	m_pTypes    = NULL;
	m_nTypeIndex = -1;
	m_nTypeId = -1;

	RegexKeyInit();
}

// デストラクタ
/*!	@brief デストラクタ

	コンパイル済みデータの破棄を行う。
*/
RegexKeyword::~RegexKeyword()
{
	MYDBGMSG("~RegexKeyword")
	// コンパイル済みのバッファを解放する。
	for (int i=0; i<MAX_REGEX_KEYWORD; ++i) {
		if (m_info[i].pBregexp && IsAvailable()) {
			BRegfree(m_info[i].pBregexp);
		}
		m_info[i].pBregexp = NULL;
	}
	
	RegexKeyInit();

	m_nTypeIndex = -1;
	m_pTypes     = NULL;
}

// 正規表現キーワード初期化処理
/*!	@brief 正規表現キーワード初期化

	 正規表現キーワードに関する変数類を初期化する。

	@retval TRUE 成功
*/
bool RegexKeyword::RegexKeyInit(void)
{
	MYDBGMSG("RegexKeyInit")
	m_nTypeIndex = -1;
	m_nTypeId = -1;
	m_nCompiledMagicNumber = 1;
	m_bUseRegexKeyword = false;
	m_nRegexKeyCount = 0;
	for (int i=0; i<MAX_REGEX_KEYWORD; ++i) {
		m_info[i].pBregexp = NULL;
#ifdef USE_PARENT
#else
		m_info[i].sRegexKey.m_nColorIndex = COLORIDX_REGEX1;
#endif
	}
#ifdef USE_PARENT
#else
	wmemset(m_keywordList, _countof(m_keywordList), L'\0');
#endif

	return true;
}

// 現在タイプ設定処理
/*!	@brief 現在タイプ設定

	現在のタイプ設定を設定する。

	@param pTypesPtr [in] タイプ設定構造体へのポインタ

	@retval TRUE 成功
	@retval FALSE 失敗

	@note タイプ設定が変わったら再ロードしコンパイルする。
*/
bool RegexKeyword::RegexKeySetTypes(const TypeConfig *pTypesPtr)
{
	MYDBGMSG("RegexKeySetTypes")
	if (!pTypesPtr)  {
		m_pTypes = NULL;
		m_bUseRegexKeyword = false;
		return false;
	}

	if (!pTypesPtr->bUseRegexKeyword) {
		// OFFになったのにまだONならOFFにする。
		if (m_bUseRegexKeyword) {
			m_pTypes = NULL;
			m_bUseRegexKeyword = false;
		}
		return false;
	}

	if (m_nTypeId == pTypesPtr->id
		&& m_nCompiledMagicNumber == pTypesPtr->nRegexKeyMagicNumber
		&& m_pTypes  // 2014.07.02 条件追加
	) {
		return true;
	}

	m_pTypes = pTypesPtr;

	RegexKeyCompile();
	
	return true;
}

// 正規表現キーワードコンパイル処理
/*!	@brief 正規表現キーワードコンパイル

	正規表現キーワードをコンパイルする。

	@retval TRUE 成功
	@retval FALSE 失敗

	@note すでにコンパイル済みの場合はそれを破棄する。
	キーワードはコンパイルデータとして内部変数にコピーする。
	先頭指定、色指定側の使用・未使用をチェックする。
*/
bool RegexKeyword::RegexKeyCompile(void)
{
	static const wchar_t dummy[2] = L"\0";
	const struct RegexKeywordInfo	*rp;

	MYDBGMSG("RegexKeyCompile")
	// コンパイル済みのバッファを解放する。
	for (int i=0; i<MAX_REGEX_KEYWORD; ++i) {
		if (m_info[i].pBregexp && IsAvailable()) {
			BRegfree(m_info[i].pBregexp);
		}
		m_info[i].pBregexp = NULL;
	}

	// コンパイルパターンを内部変数に移す。
	m_nRegexKeyCount = 0;
	const wchar_t* pKeyword = &m_pTypes->regexKeywordList[0];
#ifdef USE_PARENT
#else
	wmemcpy(m_keywordList,  m_pTypes->m_RegexKeywordList, _countof(m_RegexKeywordList));
#endif
	for (int i=0; i<MAX_REGEX_KEYWORD; ++i) {
		if (pKeyword[0] == L'\0') {
			break;
		}
#ifdef USE_PARENT
#else
		m_info[i].sRegexKey.m_nColorIndex = m_pTypes->m_RegexKeywordArr[i].m_nColorIndex;
#endif
		++m_nRegexKeyCount;
		for (; *pKeyword!='\0'; ++pKeyword) {}
		++pKeyword;
	}

	m_nTypeIndex = m_pTypes->nIdx;
	m_nTypeId = m_pTypes->id;
	m_nCompiledMagicNumber = 1;	// Not Compiled.
	m_bUseRegexKeyword = m_pTypes->bUseRegexKeyword;
	if (!m_bUseRegexKeyword) {
		return false;
	}

	if (!IsAvailable()) {
		m_bUseRegexKeyword = false;
		return false;
	}

#ifdef USE_PARENT
	pKeyword = &m_pTypes->regexKeywordList[0];
#else
	pKeyword = &m_keywordList[0];
#endif
	// パターンをコンパイルする。
	for (int i=0; i<m_nRegexKeyCount; ++i) {
#ifdef USE_PARENT
		rp = &m_pTypes->regexKeywordArr[i];
#else
		rp = &m_info[i].sRegexKey;
#endif

		if (RegexKeyCheckSyntax(pKeyword)) {
			m_szMsg[0] = '\0';
			BMatch(pKeyword, dummy, dummy + 1, &m_info[i].pBregexp, m_szMsg);

			if (m_szMsg[0] == '\0') {	// エラーがないかチェックする
				// 先頭以外は検索しなくてよい
				if (wcsncmp(RK_HEAD_STR1, pKeyword, RK_HEAD_STR1_LEN) == 0
				 || wcsncmp(RK_HEAD_STR2, pKeyword, RK_HEAD_STR2_LEN) == 0
				 || wcsncmp(RK_HEAD_STR3, pKeyword, RK_HEAD_STR3_LEN) == 0
				) {
					m_info[i].nHead = 1;
				}else {
					m_info[i].nHead = 0;
				}

				if (COLORIDX_REGEX1  <= rp->m_nColorIndex
				 && COLORIDX_REGEX10 >= rp->m_nColorIndex
				) {
					// 色指定でチェックが入ってなければ検索しなくてもよい
					if (m_pTypes->colorInfoArr[rp->m_nColorIndex].bDisp) {
						m_info[i].nFlag = RK_EMPTY;
					}else {
						// 正規表現では色指定のチェックを見る。
						m_info[i].nFlag = RK_NOMATCH;
					}
				}else {
					// 正規表現以外では、色指定チェックは見ない。
					// 例えば、半角数値は正規表現を使い、基本機能を使わないという指定もあり得るため
					m_info[i].nFlag = RK_EMPTY;
				}
			}else {
				// コンパイルエラーなので検索対象からはずす
				m_info[i].nFlag = RK_NOMATCH;
			}
		}else {
			// 書式エラーなので検索対象からはずす
			m_info[i].nFlag = RK_NOMATCH;
		}
		for (; *pKeyword!='\0'; ++pKeyword) {}
		++pKeyword;
	}

	m_nCompiledMagicNumber = m_pTypes->nRegexKeyMagicNumber;	// Compiled.

	return true;
}

// 行検索開始処理
/*!	@brief 行検索開始

	行検索を開始する。

	@retval TRUE 成功
	@retval FALSE 失敗または検索しない指定あり

	@note それぞれの行検索の最初に実行する。
	タイプ設定等が変更されている場合はリロードする。
*/
bool RegexKeyword::RegexKeyLineStart(void)
{
	MYDBGMSG("RegexKeyLineStart")

	// 動作に必要なチェックをする。
	if (!m_bUseRegexKeyword || !IsAvailable() || !m_pTypes) {
		return false;
	}

#if 0	// RegexKeySetTypesで設定されているはずなので廃止
	// 情報不一致ならマスタから取得してコンパイルする。
	if (0
		|| m_nCompiledMagicNumber != m_pTypes->m_nRegexKeyMagicNumber
	 	|| m_nTypeIndex           != m_pTypes->m_nIdx
	) {
		RegexKeyCompile();
	}
#endif

	// 検索開始のためにオフセット情報等をクリアする。
	for (int i=0; i<m_nRegexKeyCount; ++i) {
		m_info[i].nOffset = -1;
		// m_info[i].nMatch  = RK_EMPTY;
		m_info[i].nMatch  = m_info[i].nFlag;
		m_info[i].nStatus = RK_EMPTY;
	}

	return true;
}

// 正規表現検索処理
/*!	@brief 正規表現検索

	正規表現キーワードを検索する。

	@retval TRUE 一致
	@retval FALSE 不一致

	@note RegexKeyLineStart関数によって初期化されていること。
*/
bool RegexKeyword::RegexIsKeyword(
	const StringRef&	str,		// [in] 検索対象文字列
//	const wchar_t*		pLine,		// [in] １行のデータ
	int					nPos,		// [in] 検索開始オフセット
//	int					nLineLen,	// [in] １行の長さ
	int*				nMatchLen,	// [out] マッチした長さ
	int*				nMatchColor	// [out] マッチした色番号
	)
{
	MYDBGMSG("RegexIsKeyword")

	// 動作に必要なチェックをする。
	if (0
		|| !m_bUseRegexKeyword
		|| !IsAvailable()
#ifdef USE_PARENT
		|| !m_pTypes
#endif
		// || (!pLine)
	) {
		return false;
	}

	for (int i=0; i<m_nRegexKeyCount; ++i) {
		if (m_info[i].nMatch != RK_NOMATCH) {  // この行にキーワードがないと分かっていない
			if (m_info[i].nOffset == nPos) {  // 以前検索した結果に一致する
				*nMatchLen   = m_info[i].nLength;
#ifdef USE_PARENT
				*nMatchColor = m_pTypes->regexKeywordArr[i].m_nColorIndex;
#else
				*nMatchColor = m_info[i].sRegexKey.m_nColorIndex;
#endif
				return true;  // マッチした
			}

			// 以前の結果はもう古いので再検索する
			if (m_info[i].nOffset < nPos) {
#ifdef USE_PARENT
				int matched = ExistBMatchEx()
					? BMatchEx(NULL, str.GetPtr(), str.GetPtr() + nPos, str.GetPtr() + str.GetLength(), &m_info[i].pBregexp, m_szMsg)
					: BMatch(NULL,                  str.GetPtr() + nPos, str.GetPtr() + str.GetLength(), &m_info[i].pBregexp, m_szMsg);
#else
				int matched = ExistBMatchEx()
					? BMatchEx(NULL, str.GetPtr(), str.GetPtr() + nPos, str.GetPtr() + str.GetLength(), &m_info[i].pBregexp, m_szMsg);
					: BMatch(NULL,                  str.GetPtr() + nPos, str.GetPtr() + str.GetLength(), &m_info[i].pBregexp, m_szMsg);
#endif
				if (matched) {
					m_info[i].nOffset = m_info[i].pBregexp->startp[0] - str.GetPtr();
					m_info[i].nLength = m_info[i].pBregexp->endp[0] - m_info[i].pBregexp->startp[0];
					m_info[i].nMatch  = RK_MATCH;
				
					// 指定の開始位置でマッチした
					if (m_info[i].nOffset == nPos) {
						if (m_info[i].nHead != 1 || nPos == 0) {
							*nMatchLen   = m_info[i].nLength;
#ifdef USE_PARENT
							*nMatchColor = m_pTypes->regexKeywordArr[i].m_nColorIndex;
#else
							*nMatchColor = m_info[i].sRegexKey.m_nColorIndex;
#endif
							return true;  // マッチした
						}
					}

					// 行先頭を要求する正規表現では次回から無視する
					if (m_info[i].nHead == 1) {
						m_info[i].nMatch = RK_NOMATCH;
					}
				}else {
					// この行にこのキーワードはない
					m_info[i].nMatch = RK_NOMATCH;
				}
			}
		}
	}  // for

	return false;
}

bool RegexKeyword::RegexKeyCheckSyntax(const wchar_t* s)
{
	static const wchar_t* kakomi[7 * 2] = {
		L"/",  L"/k",
		L"m/", L"/k",
		L"m#", L"#k",
		L"/",  L"/ki",
		L"m/", L"/ki",
		L"m#", L"#ki",
		NULL, NULL,
	};

	int	length = wcslen(s);
	for (int i=0; kakomi[i]; i+=2) {
		// 文字長を確かめる
		if (length > (int)wcslen(kakomi[i]) + (int)wcslen(kakomi[i + 1])) {
			// 始まりを確かめる
			if (wcsncmp(kakomi[i], s, wcslen(kakomi[i])) == 0) {
				// 終わりを確かめる
				const wchar_t* p = &s[length - wcslen(kakomi[i+1])];
				if (wcscmp(p, kakomi[i + 1]) == 0) {
					// 正常
					return true;
				}
			}
		}
	}
	return false;
}

//@@@ 2001.11.17 add end MIK

// static
DWORD RegexKeyword::GetNewMagicNumber()
{
	return ::GetTickCount();
}

