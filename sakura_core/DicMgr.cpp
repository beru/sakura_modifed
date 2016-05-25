/*!	@file
	@brief DicMgrクラス

	@author Norio Nakatani
	@date	1998/11/05 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka, Moca
	Copyright (C) 2003, Moca
	Copyright (C) 2006, fon
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <stdio.h>
#include "DicMgr.h"
#include "mem/Memory.h" // 2002/2/10 aroka ヘッダ整理
#include "debug/RunningTimer.h"
#include "io/TextStream.h"
using namespace std;

DicMgr::DicMgr()
{
	return;
}


DicMgr::~DicMgr()
{
	return;
}


/*!
	キーワードの検索
	最初に見つかったキーワードの意味を返す

	@date 2006.04.10 fon 検索ヒット行を返す引数pLineを追加
*/
BOOL DicMgr::Search(
	const wchar_t*	pszKey,				// 検索キーワード
	const size_t	nCmpLen,			// 検索キーワードの長さ
	NativeW**		ppMemKey,			// 見つかったキーワード．呼び出し元の責任で解放する．
	NativeW**		ppMemMean,			// 見つかったキーワードに対応する辞書内容．呼び出し元の責任で解放する．
	const TCHAR*	pszKeywordHelpFile,	// キーワードヘルプファイルのパス名
	int*			pLine				// 見つかったキーワードのキーワードヘルプファイル内での行番号
	)
{
#ifdef _DEBUG
	RunningTimer runningTimer("DicMgr::Search");
#endif
	const wchar_t*	pszDelimit = L" /// ";
	const wchar_t*	pszKeySeps = L",\0";

	// 辞書ファイル
	if (pszKeywordHelpFile[0] == _T('\0')) {
		return FALSE;
	}
	// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	TextInputStream_AbsIni in(pszKeywordHelpFile);
	if (!in) {
		return FALSE;
	}

	wchar_t	szLine[LINEREADBUFSIZE];
	for (int line=1; in; ++line) {	// 2006.04.10 fon
		// 1行読み込み
		{
			wstring tmp = in.ReadLineW(); //fgetws(szLine, _countof(szLine), pFile) != NULL;
			wcsncpy_s(szLine, _countof(szLine), tmp.c_str(), _TRUNCATE);
			// auto_strlcpy(szLine, tmp.c_str(), _countof(szLine));
		}

		wchar_t* pszWork = wcsstr(szLine, pszDelimit);
		if (pszWork && szLine[0] != L';') {
			*pszWork = L'\0';
			pszWork += wcslen(pszDelimit);

			// 最初のトークンを取得します。
			wchar_t* pszToken = wcstok(szLine, pszKeySeps);
			while (pszToken) {
				int nRes = _wcsnicmp(pszKey, pszToken, nCmpLen);	// 2006.04.10 fon
				if (nRes == 0) {
					int nLen = (int)wcslen(pszWork);
					for (int i=0; i<nLen; ++i) {
						if (WCODE::IsLineDelimiterBasic(pszWork[i])) {
							pszWork[i] = L'\0';
							break;
						}
					}
					// キーワードのセット
					*ppMemKey = new NativeW;	// 2006.04.10 fon
					(*ppMemKey)->SetString(pszToken);
					// 意味のセット
					*ppMemMean = new NativeW;
					(*ppMemMean)->SetString(pszWork);

					*pLine = line;	// 2006.04.10 fon
					return TRUE;
				}
				pszToken = wcstok(NULL, pszKeySeps);
			}
		}
	}
	return FALSE;
}


/*
||  入力補完キーワードの検索
||
||  ・指定された候補の最大数を超えると処理を中断する
||  ・見つかった数を返す
||
*/
int DicMgr::HokanSearch(
	const wchar_t*	pszKey,
	bool			bHokanLoHiCase,	// 英大文字小文字を同一視する
	vector_ex<std::wstring>&		vKouho,	// [out] 候補リスト
	int				nMaxKouho,		// Max候補数(0==無制限)
	const TCHAR*	pszKeywordFile
	)
{
	if (pszKeywordFile[0] == _T('\0')) {
		return 0;
	}

	TextInputStream_AbsIni in(pszKeywordFile);
	if (!in) {
		return 0;
	}
	size_t nKeyLen = wcslen(pszKey);
	while (in) {
		wstring szLine = in.ReadLineW();
		if (nKeyLen > (int)szLine.length()) {
			continue;
		}

		// コメント無視
		if (szLine[0] == L';') {
			continue;
		}

		// 空行無視
		if (szLine.length() == 0) {
			continue;
		}

		int nRet;
		if (bHokanLoHiCase) {	// 英大文字小文字を同一視する
			nRet = auto_memicmp(pszKey, szLine.c_str(), nKeyLen);
		}else {
			nRet = auto_memcmp(pszKey, szLine.c_str(), nKeyLen);
		}
		if (nRet == 0) {
			vKouho.push_back(szLine);
			if (nMaxKouho != 0 && nMaxKouho <= (int)vKouho.size()) {
				break;
			}
		}
	}
	in.Close();
	return (int)vKouho.size();
}

