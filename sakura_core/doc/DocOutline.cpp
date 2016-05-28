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
#include <memory>

#include "doc/DocOutline.h"
#include "doc/EditDoc.h"
#include "doc/logic/DocLine.h"
#include "_main/global.h"
#include "outline/FuncInfoArr.h"
#include "outline/FuncInfo.h"
#include "charset/charcode.h"
#include "io/TextStream.h"
#include "extmodule/Bregexp.h"



/*! ルールファイルの1行を管理する構造体

	@date 2002.04.01 YAZAKI
	@date 2007.11.29 kobake 名前変更: oneRule→OneRule
*/
struct OneRule {
	wchar_t szMatch[256];
	int		nLength;
	wchar_t szText[256]; // RegexReplace時の置換後文字列
	wchar_t szGroupName[256];
	int		nLv;
	int		nRegexOption;
	int		nRegexMode; // 0 ==「Mode=Regex」, 1 == 「Mode=RegexReplace」
};


/*! ルールファイルを読み込み、ルール構造体の配列を作成する

	@date 2002.04.01 YAZAKI
	@date 2002.11.03 Moca 引数nMaxCountを追加。バッファ長チェックをするように変更
	@date 2013.06.02 _tfopen_absini,fgetwsをTextInputStream_AbsIniに変更。UTF-8対応。Regex対応
	@date 2014.06.20 RegexReplace 正規表現置換モード追加
*/
int DocOutline::ReadRuleFile(
	const TCHAR*	pszFilename,
	OneRule*		pOneRule,
	int				nMaxCount,
	bool&			bRegex,
	std::wstring&	title
	)
{
	// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	TextInputStream_AbsIni	file = TextInputStream_AbsIni(pszFilename);
	if (!file.Good()) {
		return 0;
	}
	wchar_t szLine[LINEREADBUFSIZE];
	wchar_t szText[256];
	static const wchar_t* pszDelimit = L" /// ";
	static const size_t nDelimitLen = wcslen(pszDelimit);
	static const wchar_t* pszKeySeps = L",\0";
	wchar_t	cComment = L';';
	int nCount = 0;
	bRegex = false;
	bool bRegexReplace = false;
	title = L"";
	int regexOption = Bregexp::optCaseSensitive;
	
	// 通常モード
	// key1,key2 /// GroupName,Lv=1
	// 正規表現モード
	// RegexMode /// GroupName,Lv=1
	// 正規表現置換モード
	// RegexReplace /// TitleReplace /// GroupName
	while (file.Good() && nCount < nMaxCount) {
		std::wstring strLine = file.ReadLineW();
		const wchar_t* pszWork = wcsstr(strLine.c_str(), pszDelimit);
		if (pszWork && 0 < strLine.length() && strLine[0] != cComment) {
			ptrdiff_t nLen = pszWork - strLine.c_str();
			if (nLen < LINEREADBUFSIZE) {
				// szLine == 「key1,key2」
				wmemcpy(szLine, strLine.c_str(), nLen);
				szLine[nLen] = L'\0';
			}else {
				// この行は長すぎる
				continue;
			}
			pszWork += nDelimitLen;

			// 最初のトークンを取得します。
			const wchar_t* pszTextReplace = L"";
			wchar_t* pszToken;
			bool bTopDummy = false;
			bool bRegexRep2 = false;
			if (bRegex) {
				// regexのときは,区切りにしない
				pszToken = szLine;
				if (szLine[0] == L'\0') {
					if (0 < nCount) {
						// 空のKey は無視
						pszToken = NULL;
					}else {
						// 最初の要素が空のKeyだったらダミー要素
						bTopDummy = true;
					}
				}
				if (bRegexReplace && pszToken) {
					const wchar_t* pszGroupDel = wcsstr(pszWork, pszDelimit);
					if (pszGroupDel && 0 < pszWork[0] != L'\0') {
						// pszWork = 「titleRep /// group」
						// pszGroupDel = 「 /// group」
						ptrdiff_t nTitleLen = pszGroupDel - pszWork; // Len == 0 OK
						if (nTitleLen < _countof(szText)) {
							wcsncpy_s(szText, _countof(szText), pszWork, nTitleLen);
						}else {
							wcsncpy_s(szText, _countof(szText), pszWork, _TRUNCATE);
						}
						pszTextReplace = szText;
						bRegexRep2 = true;
						pszWork = pszGroupDel + nDelimitLen; // group
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
				wcsncpy(pOneRule[nCount].szMatch, pszToken, 255);
				wcsncpy_s( pOneRule[nCount].szText, _countof(pOneRule[0].szText), pszTextReplace, _TRUNCATE );
				wcsncpy(pOneRule[nCount].szGroupName, pszWork, 255);
				pOneRule[nCount].szMatch[255] = L'\0';
				pOneRule[nCount].szGroupName[255] = L'\0';
				pOneRule[nCount].nLv = nLv;
				pOneRule[nCount].nLength = wcslen(pOneRule[nCount].szMatch);
				pOneRule[nCount].nRegexOption = regexOption;
				pOneRule[nCount].nRegexMode = bRegexRep2 ? 1 : 0; // 文字列が正しい時だけReplaceMode
				++nCount;
				if (bTopDummy || bRegex) {
					pszToken = NULL;
				}else {
					pszToken = wcstok(NULL, pszKeySeps);
				}
			}
		}else {
			if (strLine.length() > 0 && strLine[0] == cComment) {
				if (strLine.length() >= 13 && strLine.length() <= 14 && _wcsnicmp(strLine.c_str() + 1, L"CommentChar=", 12) == 0) {
					if (strLine.length() == 13) {
						cComment = L'\0';
					}else {
						cComment = strLine[13];
					}
				}else if (strLine.length() == 11 && wcsicmp(strLine.c_str() + 1, L"Mode=Regex") == 0) {
					bRegex = true;
					bRegexReplace = false;
				}else if (strLine.length() == 18 && wcsicmp(strLine.c_str() + 1, L"Mode=RegexReplace") == 0) {
					bRegex = true;
					bRegexReplace = true;
				}else if (strLine.length() >= 7 && _wcsnicmp(strLine.c_str() + 1, L"Title=", 6) == 0) {
					title = strLine.c_str() + 7;
				}else if (strLine.length() > 13 && _wcsnicmp(strLine.c_str() + 1, L"RegexOption=", 12) == 0) {
					int nCaseFlag = Bregexp::optCaseSensitive;
					regexOption = 0;
					for (size_t i=13; i<strLine.length(); ++i) {
						if (strLine[i] == L'i') {
							nCaseFlag = 0;
						}else if (strLine[i] == L'g') {
							regexOption |= Bregexp::optGlobal;
						}else if (strLine[i] == L'x') {
							regexOption |= Bregexp::optExtend;
						}else if (strLine[i] == L'a') {
							regexOption |= Bregexp::optASCII;
						}else if (strLine[i] == L'u') {
							regexOption |= Bregexp::optUnicode;
						}else if (strLine[i] == L'd') {
							regexOption |= Bregexp::optDefault;
						}else if (strLine[i] == L'l') {
							regexOption |= Bregexp::optLocale;
						}else if (strLine[i] == L'R') {
							regexOption |= Bregexp::optR;
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
	@date 2007.11.29 kobake OneRule test[1024] でスタックが溢れていたのを修正
*/
void DocOutline::MakeFuncList_RuleFile(
	FuncInfoArr* pFuncInfoArr,
	std::tstring& sTitleOverride
	)
{
	// ルールファイルの内容をバッファに読み込む
	auto test = std::make_unique<OneRule[]>(1024);	// 1024個許可。 2007.11.29 kobake スタック使いすぎなので、ヒープに確保するように修正。
	bool bRegex;
	std::wstring title;
	int nCount = ReadRuleFile(doc.docType.GetDocumentAttribute().szOutlineRuleFilename, test.get(), 1024, bRegex, title);
	if (nCount < 1) {
		return;
	}
	if (0 < title.size()) {
		sTitleOverride = to_tchar(title.c_str());
	}

	/*	ネストの深さは、32レベルまで、ひとつのヘッダは、最長256文字まで区別
		（256文字まで同じだったら同じものとして扱います）
	*/
	const int nMaxStack = 32;	// ネストの最深
	size_t nDepth = 0;				// いまのアイテムの深さを表す数値。
	wchar_t pszStack[nMaxStack][256];
	wchar_t nLvStack[nMaxStack];
	wchar_t szTitle[256];			// 一時領域
	Bregexp* pRegex = nullptr;
	if (bRegex) {
		pRegex = new Bregexp[nCount];
		for (int i=0; i<nCount; ++i) {
			if (test[i].nLength == 0) {
				continue;
			}
			if (!InitRegexp(NULL, pRegex[i], true)) {
				delete[] pRegex;
				return;
			}
			if (test[i].nRegexMode == 1) {
				if (!pRegex[i].Compile(test[i].szMatch, test[i].szText, test[i].nRegexOption)) {
					std::wstring str = test[i].szMatch;
					str += L"\n";
					str += test[i].szText;
					ErrorMessage( NULL, LS(STR_DOCOUTLINE_REGEX),
						str.c_str(),
						pRegex[i].GetLastMessage()
					);
					delete[] pRegex;
					return;
				}
			}else if (!pRegex[i].Compile(test[i].szMatch, test[i].nRegexOption)) {
				ErrorMessage(NULL, LS(STR_DOCOUTLINE_REGEX),
					test[i].szMatch,
					pRegex[i].GetLastMessage()
				);
				delete[] pRegex;
				return;
			}
		}
	}
	// 1つめが空行だった場合は、ルート要素とする
	// 項目名はグループ名
	if (test[0].nLength == 0) {
		const wchar_t* g = test[0].szGroupName;
		wcscpy(pszStack[0], g);
		nLvStack[0] = test[0].nLv;
		const wchar_t* p = wcschr(g, L',');
		size_t len;
		if (p) {
			len = p - g;
		}else {
			len = wcslen(g);
		}
		NativeW mem;
		mem.SetString(g, len);
		pFuncInfoArr->AppendData(1, 1, mem.GetStringPtr(), FUNCINFO_NOCLIPTEXT, nDepth);
		nDepth = 1;
	}
	for (size_t nLineCount=0; nLineCount<doc.docLineMgr.GetLineCount(); ++nLineCount) {
		// 行取得
		size_t nLineLen;
		const wchar_t* pLine = doc.docLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if (!pLine) {
			break;
		}

		// 行頭の空白飛ばし
		size_t i = 0;
		if (!bRegex) {
			for (i=0; i<nLineLen; ++i) {
				if (pLine[i] == L' ' || pLine[i] == L'\t' || pLine[i] == L'　') {
					continue;
				}
				break;
			}
			if (i >= nLineLen) {
				continue;
			}
		}

		// 先頭文字が見出し記号のいずれかであれば、次へ進む
		wchar_t* pszText = NULL;
		int j;
		for (j=0; j<nCount; ++j) {
			if (bRegex) {
				if (test[j].nRegexMode == 0) {
					if (0 < test[j].nLength && pRegex[j].Match(pLine, nLineLen, 0)) {
						wcscpy(szTitle, test[j].szGroupName);
						break;
					}
				}else {
					if (0 < test[j].nLength && 0 < pRegex[j].Replace( pLine, nLineLen, 0 )) {
						// pLine = "ABC123DEF"
						// testのszMatch = "\d+"
						// testのszText = "$&456"
						// GetString() = "ABC123456DEF"
						// pszText = "123456"
						int nIndex = pRegex[j].GetIndex();
						size_t nMatchLen = pRegex[j].GetMatchLen();
						size_t nTextLen = pRegex[j].GetStringLen() - nLineLen + nMatchLen;
						pszText = new wchar_t[nTextLen + 1];
						wmemcpy( pszText, pRegex[j].GetString() + nIndex, nTextLen );
						pszText[nTextLen] = L'\0';
						wcscpy( szTitle, test[j].szGroupName );
						break;
					}
				}
			}else {
				if (0 < test[j].nLength && wcsncmp(&pLine[i], test[j].szMatch, test[j].nLength) == 0) {
					wcscpy(szTitle, test[j].szGroupName);
					break;
				}
			}
		}
		if (j >= nCount) {
			continue;
		}
		if (wcscmp(szTitle, L"Except") == 0) {
			continue;
		}

		// ルールにマッチした行は、アウトライン結果に表示する。

		// 行文字列から改行を取り除く pLine -> pszText
		// 正規表現置換のときは設定済み
		if (!pszText) {
			pszText = new wchar_t[nLineLen + 1];
			wmemcpy(pszText, &pLine[i], nLineLen);
			pszText[nLineLen] = L'\0';
			bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
			for (i=0; pszText[i]!=L'\0'; ++i) {
				if (WCODE::IsLineDelimiter(pszText[i], bExtEol)) {
					pszText[i] = L'\0';
					break;
				}
			}
		}

		/*
		  カーソル位置変換
		  物理位置(行頭からのバイト数、折り返し無し行位置)
		  →
		  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
		*/
		Point ptPos = doc.layoutMgr.LogicToLayout(Point(0, nLineCount));

		// nDepthを計算
		bool bAppend = true;
		size_t k;
		for (k=0; k<nDepth; ++k) {
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
			for (k=nDepth-1; 0<=k; --k) {
				if (nLvStack[k] <= test[j].nLv) {
					++k;
					break;
				}
			}
			if (k < 0) {
				k = 0;
			}
			wcscpy(pszStack[k], szTitle);
			nLvStack[k] = test[j].nLv;
			nDepth = k;
		}else {
			// 2002.11.03 Moca 最大値を超えるとバッファオーバーランするから規制する
			// nDepth = nMaxStack;
			bAppend = false;
		}
		
		if (bAppend) {
			pFuncInfoArr->AppendData(nLineCount + 1, ptPos.y + 1 , pszText, 0, nDepth);
			++nDepth;
		}
		delete[] pszText;

	}
	delete[] pRegex;
	return;
}


// From Here 2001.12.03 hor
/*! ブックマークリスト作成（無理矢理！）

	@date 2002.01.19 aroka 空行をマーク対象にするフラグ bMarkUpBlankLineEnable を導入しました。
	@date 2005.10.11 ryoji "ａ@" の右２バイトが全角空白と判定される問題の対処
	@date 2005.11.03 genta 文字列長修正．右端のゴミを除去
*/
void DocOutline::MakeFuncList_BookMark(FuncInfoArr* pFuncInfoArr)
{
	size_t nLineLen;
	bool bMarkUpBlankLineEnable = GetDllShareData().common.outline.bMarkUpBlankLineEnable;	// 空行をマーク対象にするフラグ 20020119 aroka
	size_t nNewLineLen	= doc.docEditor.newLineCode.GetLen();
	size_t nLineLast = doc.docLineMgr.GetLineCount();

	for (size_t nLineCount=0; nLineCount<nLineLast; ++nLineCount) {
		if (!BookmarkGetter(doc.docLineMgr.GetLine(nLineCount)).IsBookmarked()) {
			continue;
		}
		const wchar_t* pLine = doc.docLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if (!pLine) {
			break;
		}
		// Jan, 16, 2002 hor
		if (bMarkUpBlankLineEnable) {// 20020119 aroka
			if (nLineLen<=nNewLineLen && nLineCount< nLineLast) {
			  continue;
			}
		}// LTrim
		size_t leftspace;
		for (leftspace=0; leftspace<nLineLen; ++leftspace) {
			if (WCODE::IsBlank(pLine[leftspace])) {
				continue;
			}
			break;
		}
		
		if (bMarkUpBlankLineEnable) {// 20020119 aroka
			ASSERT_GE(nLineLen, nNewLineLen);
			if ((leftspace >= nLineLen - nNewLineLen && nLineCount < nLineLast)||
				(leftspace >= nLineLen)
			) {
				continue;
			}
		}// RTrim
		// 2005.10.11 ryoji 右から遡るのではなく左から探すように修正（"ａ@" の右２バイトが全角空白と判定される問題の対処）
		size_t k;
		size_t pos_wo_space;
		k = pos_wo_space = leftspace;
		bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
		while (k < nLineLen) {
			size_t nCharChars = NativeW::GetSizeOfChar(pLine, nLineLen, k);
			if (nCharChars == 1) {
				if (!(
						WCODE::IsLineDelimiter(pLine[k], bExtEol) ||
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
		ASSERT_GE(pos_wo_space, leftspace);
		size_t nLen = pos_wo_space - leftspace;
		std::vector<wchar_t> szText(nLen + 1);
		wchar_t* pszText = &szText[0];
		wmemcpy(pszText, &pLine[leftspace], nLen);
		pszText[nLen] = L'\0';
		Point ptXY = doc.layoutMgr.LogicToLayout(Point(0, nLineCount));
		pFuncInfoArr->AppendData(nLineCount + 1, ptXY.y+1 , pszText, 0);
	}
	return;
}
// To Here 2001.12.03 hor

