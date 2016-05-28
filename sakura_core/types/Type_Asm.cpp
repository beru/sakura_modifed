/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "types/Type.h"
#include "doc/EditDoc.h"
#include "doc/DocOutline.h"
#include "doc/logic/DocLine.h"
#include "outline/FuncInfoArr.h"
#include "view/Colors/EColorIndexType.h"

// アセンブラ
//	2004.05.01 MIK/genta
// Mar. 10, 2001 JEPRO	半角数値を色分け表示
void CType_Asm::InitTypeConfigImp(TypeConfig& type)
{
	// 名前と拡張子
	_tcscpy(type.szTypeName, _T("アセンブラ"));
	_tcscpy(type.szTypeExts, _T("asm"));

	// 設定
	type.lineComment.CopyTo(0, L";", -1);				// 行コメントデリミタ
	type.eDefaultOutline = OutlineType::Asm;			// アウトライン解析方法
	type.colorInfoArr[COLORIDX_DIGIT].bDisp = true;
}


/*! アセンブラ アウトライン解析

	@author MIK
	@date 2004.04.12 作り直し
*/
void DocOutline::MakeTopicList_asm(FuncInfoArr* pFuncInfoArr)
{
	int nTotalLine;

	nTotalLine = doc.docLineMgr.GetLineCount();

	for (int nLineCount=0; nLineCount<nTotalLine; ++nLineCount) {
		const WCHAR* pLine;
		size_t nLineLen;
		WCHAR* pTmpLine;
		size_t length;
		size_t offset;
#define MAX_ASM_TOKEN 2
		WCHAR* token[MAX_ASM_TOKEN];
		WCHAR* p;

		// 1行取得する。
		pLine = doc.docLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if (!pLine) break;

		// 作業用にコピーを作成する。バイナリがあったらその後ろは知らない。
		pTmpLine = wcsdup(pLine);
		if (!pTmpLine) break;
		if (wcslen(pTmpLine) >= (unsigned int)nLineLen) {	// バイナリを含んでいたら短くなるので...
			pTmpLine[nLineLen] = L'\0';	// 指定長で切り詰め
		}

		// 行コメント削除
		p = wcsstr(pTmpLine, L";");
		if (p) *p = L'\0';

		length = wcslen(pTmpLine);
		offset = 0;

		// トークンに分割
		for (size_t j=0; j<MAX_ASM_TOKEN; ++j) token[j] = NULL;
		for (size_t j=0; j<MAX_ASM_TOKEN; ++j) {
			token[j] = my_strtok<WCHAR>(pTmpLine, length, &offset, L" \t\r\n");
			if (!token[j]) break;
			// トークンに含まれるべき文字でないか？
			if (wcsstr(token[j], L"\"")
			 || wcsstr(token[j], L"\\")
			 || wcsstr(token[j], L"'")
			) {
				token[j] = NULL;
				break;
			}
		}

		if (token[0]) {	// トークンが1個以上ある
			int nFuncId = -1;
			WCHAR* entry_token = NULL;

			length = wcslen(token[0]);
			if (length >= 2
				&& token[0][length - 1] == L':'
			) {	// ラベル
				token[0][length - 1] = L'\0';
				nFuncId = 51;
				entry_token = token[0];
			}else if (token[1]) {	// トークンが2個以上ある
				if (wcsicmp(token[1], L"proc") == 0) {	// 関数
					nFuncId = 50;
					entry_token = token[0];
				}else if (wcsicmp(token[1], L"endp") == 0) {	// 関数終了
					nFuncId = 52;
					entry_token = token[0];
				//}else
				//if (my_stricmp(token[1], _T("macro")) == 0) {	// マクロ
				//	nFuncId = -1;
				//	entry_token = token[0];
				//}else
				//if (my_stricmp(token[1], _T("struc")) == 0) {	// 構造体
				//	nFuncId = -1;
				//	entry_token = token[0];
				}
			}

			if (nFuncId >= 0) {
				/*
				  カーソル位置変換
				  物理位置(行頭からのバイト数、折り返し無し行位置)
				  →
				  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
				*/
				Point ptPos = doc.layoutMgr.LogicToLayout(Point(0, nLineCount));
				pFuncInfoArr->AppendData(nLineCount + 1, ptPos.y + 1, entry_token, nFuncId);
			}
		}

		free(pTmpLine);
	}

	return;
}

