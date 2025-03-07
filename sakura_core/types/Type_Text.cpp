#include "StdAfx.h"
#include "types/Type.h"
#include "doc/EditDoc.h"
#include "doc/DocOutline.h"
#include "doc/logic/DocLine.h"
#include "env/DllSharedData.h"
#include "outline/FuncInfo.h"
#include "outline/FuncInfoArr.h"
#include "view/colors/EColorIndexType.h"

// テキスト
void CType_Text::InitTypeConfigImp(TypeConfig& type)
{
	// 名前と拡張子
	_tcscpy(type.szTypeName, _T("テキスト"));
	_tcscpy(type.szTypeExts, _T("txt,log,1st,err,ps"));

	// 設定
	type.nMaxLineKetas = 120;					// 折り返し桁数
	type.eDefaultOutline = OutlineType::Text;				// アウトライン解析方法
	type.colorInfoArr[COLORIDX_SSTRING].bDisp = false;	// シングルクォーテーション文字列を色分け表示しない
	type.colorInfoArr[COLORIDX_WSTRING].bDisp = false;	// ダブルクォーテーション文字列を色分け表示しない
	type.bKinsokuHead = false;								// 行頭禁則
	type.bKinsokuTail = false;								// 行末禁則
	type.bKinsokuRet  = false;								// 改行文字をぶら下げる
	type.bKinsokuKuto = false;								// 句読点をぶら下げる
	wcscpy_s(type.szKinsokuHead, L"!%),.:;?]}¢°’”‰′″℃、。々〉》」』】〕゛゜ゝゞ・ヽヾ！％），．：；？］｝｡｣､･ﾞﾟ¢");		/* 行頭禁則 */
	wcscpy_s(type.szKinsokuTail, L"$([{£\\‘“〈《「『【〔＄（［｛｢£￥");		/* 行末禁則 */
	// type.szKinsokuKuto（句読点ぶら下げ文字）はここではなく全タイプにデフォルト設定

	// ※小さな親切として、C:\〜〜 や \\〜〜 などのファイルパスをクリッカブルにする設定を「テキスト」に既定で仕込む
	// ※""で挟まれる設定は挟まれない設定よりも上に無ければならない
	// ※""で挟まれる設定を複製してちょっと修正すれば、<>や[]に挟まれたものにも対応できる（ユーザに任せる）

	// 正規表現キーワード
	size_t keywordPos = 0;
	wchar_t* pKeyword = type.regexKeywordList;
	type.bUseRegexKeyword = true;							// 正規表現キーワードを使うか
	type.regexKeywordArr[0].nColorIndex = COLORIDX_URL;	// 色指定番号
	wcscpyn(&pKeyword[keywordPos],			// 正規表現キーワード
		L"/(?<=\")(\\b[a-zA-Z]:|\\B\\\\\\\\)[^\"\\r\\n]*/k",			//   ""で挟まれた C:\〜, \\〜 にマッチするパターン
		_countof(type.regexKeywordList) - 1);
	keywordPos += auto_strlen(&pKeyword[keywordPos]) + 1;
	type.regexKeywordArr[1].nColorIndex = COLORIDX_URL;	// 色指定番号
	wcscpyn(&pKeyword[keywordPos],			// 正規表現キーワード
		L"/(\\b[a-zA-Z]:\\\\|\\B\\\\\\\\)[\\w\\-_.\\\\\\/$%~]*/k",		//   C:\〜, \\〜 にマッチするパターン
		_countof(type.regexKeywordList) - keywordPos - 1);
	keywordPos += auto_strlen(&pKeyword[keywordPos]) + 1;
	pKeyword[keywordPos] = L'\0';
}


/*!	テキスト・トピックリスト作成 */
void DocOutline::MakeTopicList_txt(FuncInfoArr* pFuncInfoArr)
{
	using namespace WCODE;

	// 見出し記号
	const wchar_t*	pszStarts = GetDllShareData().common.format.szMidashiKigou;
	size_t			nStartsLen = wcslen(pszStarts);

	/*	ネストの深さは、nMaxStackレベルまで、ひとつのヘッダは、最長32文字まで区別
		（32文字まで同じだったら同じものとして扱います）
	*/
	const int nMaxStack = 32;	//	ネストの最深
	int nDepth = 0;				//	いまのアイテムの深さを表す数値。
	wchar_t pszStack[nMaxStack][32];
	wchar_t szTitle[32];			//	一時領域
	size_t nLineCount;
	bool b278a = false;
	for (nLineCount=0; nLineCount<doc.docLineMgr.GetLineCount(); ++nLineCount) {
		// 行取得
		size_t nLineLen;
		const wchar_t* pLine = doc.docLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if (!pLine) {
			break;
		}

		// 行頭の空白飛ばし
		size_t i;
		for (i=0; i<nLineLen; ++i) {
			if (WCODE::IsBlank(pLine[i])) {
				continue;
			}
			break;
		}
		if (i >= nLineLen) {
			continue;
		}

		// 先頭文字が見出し記号のいずれかであれば、次へ進む
		size_t nCharChars = NativeW::GetSizeOfChar(pLine, nLineLen, i);
		size_t nCharChars2;
		size_t j;
		for (j=0; j<nStartsLen; j+=nCharChars2) {
			nCharChars2 = NativeW::GetSizeOfChar(pszStarts, nStartsLen, j);
			if (nCharChars == nCharChars2) {
				if (wmemcmp(&pLine[i], &pszStarts[j], nCharChars) == 0) {
					break;
				}
			}
		}
		if (j >= nStartsLen) {
			continue;
		}

		// 見出し種類の判別 -> szTitle
		if (pLine[i] == L'(') {
			     if (IsInRange(pLine[i + 1], L'0', L'9')) wcscpy_s(szTitle, L"(0)"); // 数字
			else if (IsInRange(pLine[i + 1], L'A', L'Z')) wcscpy_s(szTitle, L"(A)"); // 英大文字
			else if (IsInRange(pLine[i + 1], L'a', L'z')) wcscpy_s(szTitle, L"(a)"); // 英小文字
			else continue; // ※「(」の次が英数字で無い場合、見出しとみなさない
		}
		else if (IsInRange(pLine[i], L'０', L'９')) wcscpy(szTitle, L"０"); // 全角数字
		else if (0
			|| IsInRange(pLine[i], L'�@', L'�S')
			|| pLine[i] == L'\u24ea'
			|| IsInRange(pLine[i], L'\u3251', L'\u325f')
			|| IsInRange(pLine[i], L'\u32b1', L'\u32bf')
		) wcscpy(szTitle, L"�@"); // �@〜�S ○0　○21○35　○36○50
		else if (IsInRange(pLine[i], L'�T', L'\u216f')) wcscpy(szTitle, L"�T"); // �T〜�]　XIXIILCDM
		else if (IsInRange(pLine[i], L'�@', L'\u217f')) wcscpy(szTitle, L"�T"); // �T〜�]　xixiilcdm
		else if (IsInRange(pLine[i], L'\u2474', L'\u2487')) wcscpy(szTitle, L"\u2474"); // (1)-(20)
		else if (IsInRange(pLine[i], L'\u2488', L'\u249b')) wcscpy(szTitle, L"\u2488"); // 1.-20.
		else if (IsInRange(pLine[i], L'\u249c', L'\u24b5')) wcscpy(szTitle, L"\u249c"); // (a)-(z)
		else if (IsInRange(pLine[i], L'\u24b6', L'\u24cf')) wcscpy(szTitle, L"\u24b6"); // ○A-○Z
		else if (IsInRange(pLine[i], L'\u24d0', L'\u24e9')) wcscpy(szTitle, L"\u24d0"); // ○a-○z
		else if (IsInRange(pLine[i], L'\u24eb', L'\u24f4')) { // ●11-●20
			if (b278a) { wcscpy(szTitle, L"\u278a"); }
			else { wcscpy(szTitle, L"\u2776"); }
		}else if (IsInRange(pLine[i], L'\u24f5', L'\u24fe')) wcscpy(szTitle, L"\u24f5"); // ◎1-◎10
		else if (IsInRange(pLine[i], L'\u2776', L'\u277f')) wcscpy(szTitle, L"\u2776"); // ●1-●10
		else if (IsInRange(pLine[i], L'\u2780', L'\u2789')) wcscpy(szTitle, L"\u2780"); // ○1-○10
		else if (IsInRange(pLine[i], L'\u278a', L'\u2793')) { wcscpy(szTitle, L"\u278a"); b278a = true; } // ●1-●10(SANS-SERIF)
		else if (IsInRange(pLine[i], L'\u3220', L'\u3229')) wcscpy(szTitle, L"\ua3220"); // (一)-(十)
		else if (IsInRange(pLine[i], L'\u3280', L'\u3289')) wcscpy(szTitle, L"\u3220"); // ○一-○十
		else if (IsInRange(pLine[i], L'\u32d0', L'\u32fe')) wcscpy(szTitle, L"\u32d0"); // ○ア-○ヲ
		else if (wcschr(L"〇一二三四五六七八九十百零壱弐参伍", pLine[i])) wcscpy(szTitle, L"一"); // 漢数字
		else {
			wcsncpy(szTitle, &pLine[i], nCharChars);	//	先頭文字をszTitleに保持。
			szTitle[nCharChars] = L'\0';
		}

		/*	「見出し記号」に含まれる文字で始まるか、
			(0、(1、...(9、(A、(B、...(Z、(a、(b、...(z
			で始まる行は、アウトライン結果に表示する。
		*/

		// 行文字列から改行を取り除く pLine -> pszText
		std::vector<wchar_t> szText(nLineLen + 1);
		wchar_t* pszText = &szText[0];
		wmemcpy(pszText, &pLine[i], nLineLen);
		pszText[nLineLen] = L'\0';
		bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
		for (i=0; i<nLineLen; ++i) {
			if (WCODE::IsLineDelimiter(pszText[i], bExtEol)) {
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
		Point ptPos = doc.layoutMgr.LogicToLayout(Point(0, (int)nLineCount));
		// nDepthを計算
		int k;
		bool bAppend = true;
		for (k=0; k<nDepth; ++k) {
			int nResult = wcscmp(pszStack[k], szTitle);
			if (nResult == 0) {
				break;
			}
		}
		if (k < nDepth) {
			//	ループ途中でbreak;してきた。＝今までに同じ見出しが存在していた。
			//	ので、同じレベルに合わせてAppendData.
			nDepth = k;
		}else if (nMaxStack > k) {
			//	いままでに同じ見出しが存在しなかった。
			//	ので、pszStackにコピーしてAppendData.
			wcscpy_s(pszStack[nDepth], szTitle);
		}else {
			// 最大値を超えるとバッファオーバーラン
			// nDepth = nMaxStack;
			bAppend = false;
		}
		
		if (bAppend) {
			pFuncInfoArr->AppendData(
				nLineCount + 1,
				ptPos.y + 1,
				pszText,
				0,
				nDepth
			);
			++nDepth;
		}
	}
	return;
}


/*! 階層付きテキスト アウトライン解析 */
void DocOutline::MakeTopicList_wztxt(FuncInfoArr* pFuncInfoArr)
{
	size_t levelPrev = 0;
	bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;

	for (size_t nLineCount=0; nLineCount<doc.docLineMgr.GetLineCount(); ++nLineCount) {
		const wchar_t*	pLine;
		size_t nLineLen;

		pLine = doc.docLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if (!pLine) {
			break;
		}
		if (*pLine == L'.') {
			const wchar_t* pPos;
			int			nLength;
			wchar_t		szTitle[1024];

			//	ピリオドの数＝階層の深さを数える
			for (pPos=pLine+1; *pPos==L'.'; ++pPos)
				;

			Point ptPos = doc.layoutMgr.LogicToLayout(Point(0, (int)nLineCount));
			size_t level = pPos - pLine;

			// 階層が2段位上深くなるときは、無題の要素を追加
			if (levelPrev < level && level != levelPrev + 1) {
				// (無題)を挿入
				//	ただし，TAG一覧には出力されないように
				for (size_t dummyLevel=levelPrev+1; dummyLevel<level; ++dummyLevel) {
					pFuncInfoArr->AppendData(
						nLineCount + 1,
						ptPos.y + 1,
						LSW(STR_NO_TITLE1),
						FUNCINFO_NOCLIPTEXT,
						dummyLevel - 1
					);
				}
			}
			levelPrev = level;

			nLength = auto_sprintf_s(szTitle, L"%d - ", level);
			
			wchar_t* pDest = szTitle + nLength; // 書き込み先
			wchar_t* pDestEnd = szTitle + _countof(szTitle) - 2;
			
			while (pDest < pDestEnd) {
				if (WCODE::IsLineDelimiter(*pPos, bExtEol) || *pPos == L'\0') {
					break;
				}else {
					*pDest++ = *pPos++;
				}
			}
			*pDest = L'\0';
			pFuncInfoArr->AppendData(nLineCount + 1, ptPos.y + 1, szTitle, 0, level - 1);
		}
	}
}

