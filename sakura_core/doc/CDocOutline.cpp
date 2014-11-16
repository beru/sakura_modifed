/*!	@file
	@brief アウトライン解析

	@author genta
	@date	2004.08.08 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, genta
	Copyright (C) 2002, frozen
	Copyright (C) 2003, zenryaku
	Copyright (C) 2005, genta, D.S.Koba, じゅうじ

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <string.h>
#include "doc/CDocOutline.h"
#include "doc/CEditDoc.h"
#include "doc/logic/CDocLine.h"
#include "_main/global.h"
#include "outline/CFuncInfoArr.h"
#include "outline/CFuncInfo.h"
#include "charset/charcode.h"
#include "io/CTextStream.h"
#include "extmodule/CBregexp.h"
#include "util/other_util.h"



/*! ルールファイルの1行を管理する構造体

	@date 2002.04.01 YAZAKI
	@date 2007.11.29 kobake 名前変更: oneRule→SOneRule
*/
struct SOneRule {
	wchar_t szMatch[256];
	int		nLength;
	wchar_t szGroupName[256];
	int		nLv;
	int		nRegexOption;
};


/*! ルールファイルを読み込み、ルール構造体の配列を作成する

	@date 2002.04.01 YAZAKI
	@date 2002.11.03 Moca 引数nMaxCountを追加。バッファ長チェックをするように変更
	@date 2013.06.02 _tfopen_absini,fgetwsをCTextInputStream_AbsIniに変更。UTF-8対応。Regex対応
*/
int CDocOutline::ReadRuleFile(
	const TCHAR*	pszFilename,
	SOneRule*		pcOneRule,
	int				nMaxCount,
	bool&			bRegex,
	std::wstring&	title
	)
{
	// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	CTextInputStream_AbsIni	file = CTextInputStream_AbsIni(pszFilename);
	if (!file.Good()) {
		return 0;
	}
	wchar_t szLine[LINEREADBUFSIZE];
	static const wchar_t* pszDelimit = L" /// ";
	static const int nDelimitLen = wcslen(pszDelimit);
	static const wchar_t* pszKeySeps = L",\0";
	wchar_t	cComment = L';';
	int nCount = 0;
	bRegex = false;
	title = L"";
	int regexOption = CBregexp::optCaseSensitive;
	
	while (file.Good() && nCount < nMaxCount) {
		std::wstring strLine = file.ReadLineW();
		const wchar_t* pszWork = wcsstr(strLine.c_str(), pszDelimit);
		if (pszWork && 0 < strLine.length() && strLine[0] != cComment) {
			int nLen = pszWork - strLine.c_str();
			if (nLen < LINEREADBUFSIZE) {
				wmemcpy(szLine, strLine.c_str(), nLen);
				szLine[nLen] = L'\0';
			}else {
				// この行は長すぎる
				continue;
			}
			pszWork += nDelimitLen;

			// 最初のトークンを取得します。
			wchar_t* pszToken;
			bool bTopDummy = false;
			if (bRegex) {
				// regexのときは,区切りにしない
				pszToken = szLine;
				if (szLine[0] == L'\0') {
					if (0 < nCount) {
						pszToken = NULL;
					}else {
						bTopDummy = true;
					}
				}
			}else {
				pszToken = wcstok(szLine, pszKeySeps);
				if (nCount == 0 && !pszToken) {
					pszToken = szLine;
					bTopDummy = true;
				}
			}
			const WCHAR* p = wcsstr(pszWork, L",Lv=");
			int nLv = 0;
			if (p) {
				nLv = _wtoi(p + 4);
			}
			while (pszToken) {
				wcsncpy(pcOneRule[nCount].szMatch, pszToken, 255);
				wcsncpy(pcOneRule[nCount].szGroupName, pszWork, 255);
				pcOneRule[nCount].szMatch[255] = L'\0';
				pcOneRule[nCount].szGroupName[255] = L'\0';
				pcOneRule[nCount].nLv = nLv;
				pcOneRule[nCount].nLength = wcslen(pcOneRule[nCount].szMatch);
				pcOneRule[nCount].nRegexOption = regexOption;
				nCount++;
				if (bTopDummy || bRegex) {
					pszToken = NULL;
				}else {
					pszToken = wcstok(NULL, pszKeySeps);
				}
			}
		}else {
			if (0 < strLine.length() && strLine[0] == cComment) {
				if (13 <= strLine.length() && strLine.length() <= 14 && 0 == _wcsnicmp(strLine.c_str() + 1, L"CommentChar=", 12)) {
					if (13 == strLine.length()) {
						cComment = L'\0';
					}else {
						cComment = strLine[13];
					}
				}else if (11 == strLine.length() && 0 == wcsicmp(strLine.c_str() + 1, L"Mode=Regex")) {
					bRegex = true;
				}else if (7 <= strLine.length() && 0 == _wcsnicmp(strLine.c_str() + 1, L"Title=", 6)) {
					title = strLine.c_str() + 7;
				}else if (13 < strLine.length() && 0 == _wcsnicmp(strLine.c_str() + 1, L"RegexOption=", 12)) {
					int nCaseFlag = CBregexp::optCaseSensitive;
					regexOption = 0;
					for (int i = 13; i < (int)strLine.length(); i++) {
						if (strLine[i] == L'i') {
							nCaseFlag = 0;
						}else if (strLine[i] == L'g') {
							regexOption |= CBregexp::optGlobal;
						}else if (strLine[i] == L'x') {
							regexOption |= CBregexp::optExtend;
						}else if (strLine[i] == L'a') {
							regexOption |= CBregexp::optASCII;
						}else if (strLine[i] == L'u') {
							regexOption |= CBregexp::optUnicode;
						}else if (strLine[i] == L'd') {
							regexOption |= CBregexp::optDefault;
						}else if (strLine[i] == L'l') {
							regexOption |= CBregexp::optLocale;
						}else if (strLine[i] == L'R') {
							regexOption |= CBregexp::optR;
						}
					}
					regexOption |= nCaseFlag;
				}
			}
		}
	}
	file.Close();
	return nCount;
}

/*! ルールファイルを元に、トピックリストを作成

	@date 2002.04.01 YAZAKI
	@date 2002.11.03 Moca ネストの深さが最大値を超えるとバッファオーバーランするのを修正
		最大値以上は追加せずに無視する
	@date 2007.11.29 kobake SOneRule test[1024] でスタックが溢れていたのを修正
*/
void CDocOutline::MakeFuncList_RuleFile(CFuncInfoArr* pcFuncInfoArr, std::tstring& sTitleOverride)
{
	// ルールファイルの内容をバッファに読み込む
	auto_array_ptr<SOneRule> test(new SOneRule[1024]);	// 1024個許可。 2007.11.29 kobake スタック使いすぎなので、ヒープに確保するように修正。
	bool bRegex;
	std::wstring title;
	int nCount = ReadRuleFile(m_pcDocRef->m_cDocType.GetDocumentAttribute().m_szOutlineRuleFilename, test.get(), 1024, bRegex, title);
	if (nCount < 1) {
		return;
	}
	if (0 < title.size()) {
		sTitleOverride = to_tchar(title.c_str());
	}

	/*	ネストの深さは、32レベルまで、ひとつのヘッダは、最長256文字まで区別
		（256文字まで同じだったら同じものとして扱います）
	*/
	const int	nMaxStack = 32;	// ネストの最深
	int			nDepth = 0;				// いまのアイテムの深さを表す数値。
	wchar_t		pszStack[nMaxStack][256];
	wchar_t		nLvStack[nMaxStack];
	wchar_t		szTitle[256];			// 一時領域
	CBregexp* pRegex = NULL;
	if (bRegex) {
		pRegex = new CBregexp[nCount];
		for (int i = 0; i < nCount; i++) {
			if (0 == test[i].nLength) {
				continue;
			}
			if (!InitRegexp(NULL, pRegex[i], true)) {
				delete [] pRegex;
				return;
			}
			if (!pRegex[i].Compile(test[i].szMatch, test[i].nRegexOption)) {
				ErrorMessage(NULL, LS(STR_DOCOUTLINE_REGEX),
					test[i].szMatch,
					pRegex[i].GetLastMessage()
				);
				delete [] pRegex;
				return;
			}
		}
	}
	// 1つめが空行だった場合は、ルート要素とする
	// 項目名はグループ名
	if (test[0].nLength == 0) {
		const wchar_t* g = test[0].szGroupName;
		wcscpy_s(pszStack[0], g);
		nLvStack[0] = test[0].nLv;
		const wchar_t* p = wcschr(g, L',');
		int len;
		if (p != NULL) {
			len = p - g;
		}else {
			len = wcslen(g);
		}
		CNativeW mem;
		mem.SetString(g, len);
		pcFuncInfoArr->AppendData(CLogicInt(1), CLayoutInt(1), mem.GetStringPtr(), FUNCINFO_NOCLIPTEXT, nDepth);
		nDepth = 1;
	}
	for (CLogicInt nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount) {
		// 行取得
		CLogicInt nLineLen;
		const wchar_t* pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if (!pLine) {
			break;
		}

		// 行頭の空白飛ばし
		int i;
		for (i = 0; i < nLineLen; ++i) {
			if (pLine[i] == L' ' || pLine[i] == L'\t' || pLine[i] == L'　') {
				continue;
			}
			break;
		}
		if (i >= nLineLen) {
			continue;
		}

		// 先頭文字が見出し記号のいずれかであれば、次へ進む
		int j;
		for (j = 0; j < nCount; j++) {
			if (bRegex) {
				if (0 < test[j].nLength && pRegex[j].Match(pLine, nLineLen, 0)) {
					wcscpy_s(szTitle, test[j].szGroupName);
					break;
				}
			}else {
				if (0 < test[j].nLength && 0 == wcsncmp(&pLine[i], test[j].szMatch, test[j].nLength)) {
					wcscpy_s(szTitle, test[j].szGroupName);
					break;
				}
			}
		}
		if (j >= nCount) {
			continue;
		}
		if (0 == wcscmp(szTitle, L"Except")) {
			continue;
		}

		// ルールにマッチした行は、アウトライン結果に表示する。

		// 行文字列から改行を取り除く pLine -> pszText
		wchar_t* pszText = new wchar_t[nLineLen + 1];
		wmemcpy(pszText, &pLine[i], nLineLen);
		pszText[nLineLen] = L'\0';
		for (i = 0; i < pszText[i] != L'\0'; ++i) {
			if (WCODE::IsLineDelimiter(pszText[i])) {
				pszText[i] = L'\0';
				break;
			}
		}

		/*
		  カーソル位置変換
		  物理位置(行頭からのバイト数、折り返し無し行位置)
		  →
		  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
		*/
		CLayoutPoint ptPos;
		m_pcDocRef->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(0, nLineCount),
			&ptPos
		);

		// nDepthを計算
		BOOL bAppend = TRUE;
		int k;
		for (k = 0; k < nDepth; k++) {
			int nResult = wcscmp(pszStack[k], szTitle);
			if (nResult == 0) {
				break;
			}
		}
		if (k < nDepth) {
			// ループ途中でbreak;してきた。＝今までに同じ見出しが存在していた。
			// ので、同じレベルに合わせてAppendData.
			nDepth = k;
		}else if (nMaxStack> k) {
			// いままでに同じ見出しが存在しなかった。
			// Lvが高い場合は、一致するまでさかのぼる
			for (k = nDepth - 1; 0 <= k ; k--) {
				if (nLvStack[k] <= test[j].nLv) {
					k++;
					break;
				}
			}
			if (k < 0) {
				k = 0;
			}
			wcscpy_s(pszStack[k], szTitle);
			nLvStack[k] = test[j].nLv;
			nDepth = k;
		}else {
			// 2002.11.03 Moca 最大値を超えるとバッファオーバーランするから規制する
			// nDepth = nMaxStack;
			bAppend = FALSE;
		}
		
		if (FALSE != bAppend) {
			pcFuncInfoArr->AppendData(nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1) , pszText, 0, nDepth);
			nDepth++;
		}
		delete [] pszText;

	}
	delete [] pRegex;
	return;
}


// From Here 2001.12.03 hor
/*! ブックマークリスト作成（無理矢理！）

	@date 2002.01.19 aroka 空行をマーク対象にするフラグ bMarkUpBlankLineEnable を導入しました。
	@date 2005.10.11 ryoji "ａ@" の右２バイトが全角空白と判定される問題の対処
	@date 2005.11.03 genta 文字列長修正．右端のゴミを除去
*/
void CDocOutline::MakeFuncList_BookMark(CFuncInfoArr* pcFuncInfoArr)
{
	CLogicInt nLineLen;
	BOOL bMarkUpBlankLineEnable = GetDllShareData().m_Common.m_sOutline.m_bMarkUpBlankLineEnable;	// 空行をマーク対象にするフラグ 20020119 aroka
	int nNewLineLen	= m_pcDocRef->m_cDocEditor.m_cNewLineCode.GetLen();
	CLogicInt nLineLast	= m_pcDocRef->m_cDocLineMgr.GetLineCount();
	int nCharChars;

	for (CLogicInt nLineCount = CLogicInt(0); nLineCount <  nLineLast; ++nLineCount) {
		if (!CBookmarkGetter(m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)).IsBookmarked()) continue;
		const wchar_t* pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if (!pLine) {
			break;
		}
		// Jan, 16, 2002 hor
		if (bMarkUpBlankLineEnable) {// 20020119 aroka
			if (nLineLen<=nNewLineLen && nLineCount< nLineLast) {
			  continue;
			}
		}// LTrim
		int leftspace;
		for (leftspace = 0; leftspace < nLineLen; ++leftspace) {
			if (WCODE::IsBlank(pLine[leftspace])) {
				continue;
			}
			break;
		}
		
		if (bMarkUpBlankLineEnable) {// 20020119 aroka
			if ((leftspace >= nLineLen - nNewLineLen && nLineCount < nLineLast)||
				(leftspace >= nLineLen)
			) {
				continue;
			}
		}// RTrim
		// 2005.10.11 ryoji 右から遡るのではなく左から探すように修正（"ａ@" の右２バイトが全角空白と判定される問題の対処）
		int	k;
		int pos_wo_space;
		k = pos_wo_space = leftspace;
		while (k < nLineLen) {
			nCharChars = CNativeW::GetSizeOfChar(pLine, nLineLen, k);
			if (1 == nCharChars) {
				if (!(
						WCODE::IsLineDelimiter(pLine[k]) ||
						pLine[k] == WCODE::SPACE ||
						pLine[k] == WCODE::TAB ||
						WCODE::IsZenkakuSpace(pLine[k]) ||
						pLine[k] == L'\0'
					)
				)
				pos_wo_space = k + nCharChars;
			}
			k += nCharChars;
		}
		// Nov. 3, 2005 genta 文字列長計算式の修正
		wchar_t* pszText;
		{
			int nLen = pos_wo_space - leftspace;
			pszText = new wchar_t[nLen + 1];
			wmemcpy(pszText, &pLine[leftspace], nLen);
			pszText[nLen] = L'\0';
		}
		CLayoutPoint ptXY;
		//int nX,nY
		m_pcDocRef->m_cLayoutMgr.LogicToLayout(	CLogicPoint(CLogicInt(0), nLineCount), &ptXY);
		pcFuncInfoArr->AppendData(nLineCount + CLogicInt(1), ptXY.GetY2()+CLayoutInt(1) , pszText, 0);
		delete [] pszText;
	}
	return;
}
// To Here 2001.12.03 hor

