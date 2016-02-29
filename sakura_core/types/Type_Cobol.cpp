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

// COBOL
void CType_Cobol::InitTypeConfigImp(TypeConfig* pType)
{
	// 名前と拡張子
	_tcscpy(pType->szTypeName, _T("COBOL"));
	_tcscpy(pType->szTypeExts, _T("cbl,cpy,pco,cob"));	// Jun. 04, 2001 JEPRO KENCH氏の助言に従い追加

	// 設定
	pType->lineComment.CopyTo(0, L"*", 6);				// Jun. 02, 2001 JEPRO 修正
	pType->lineComment.CopyTo(1, L"D", 6);				// Jun. 04, 2001 JEPRO 追加
	pType->stringType = StringLiteralType::PLSQL;		// 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""]['']
	wcscpy_s(pType->szIndentChars, L"*");				// その他のインデント対象文字
	pType->nKeywordSetIdx[0] = 3;						// キーワードセット		// Jul. 10, 2001 JEPRO
	pType->eDefaultOutline = OutlineType::Cobol;		// アウトライン解析方法
	// 指定桁縦線	// 2005.11.08 Moca
	pType->colorInfoArr[COLORIDX_VERTLINE].bDisp = true;
	pType->nVertLineIdx[0] = LayoutInt(7);
	pType->nVertLineIdx[1] = LayoutInt(8);
	pType->nVertLineIdx[2] = LayoutInt(12);
	pType->nVertLineIdx[3] = LayoutInt(73);
}


//! COBOL アウトライン解析
void DocOutline::MakeTopicList_cobol(FuncInfoArr* pFuncInfoArr)
{
	const wchar_t*	pLine;
	LogicInt		nLineLen;
	int				i;
	int				k;
	wchar_t			szDivision[1024];
	wchar_t			szLabel[1024];
	const wchar_t*	pszKeyword;
	int				nKeywordLen;
	BOOL			bDivision;
	bool			bExtEol = GetDllShareData().common.edit.bEnableExtEol;

	szDivision[0] = L'\0';
	szLabel[0] =  L'\0';


	LogicInt	nLineCount;
	for (nLineCount=LogicInt(0); nLineCount<m_pDocRef->m_docLineMgr.GetLineCount(); ++nLineCount) {
		pLine = m_pDocRef->m_docLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if (!pLine) {
			break;
		}
		// コメント行か
		if (7 <= nLineLen && pLine[6] == L'*') {
			continue;
		}
		// ラベル行か
		if (8 <= nLineLen && pLine[7] != L' ') {
			k = 0;
			for (i=7; i<nLineLen; ) {
				if (pLine[i] == '.'
				 || WCODE::IsLineDelimiter(pLine[i], bExtEol)
				) {
					break;
				}
				szLabel[k] = pLine[i];
				++k;
				++i;
				if (pLine[i - 1] == L' ') {
					for (; i<nLineLen; ++i) {
						if (pLine[i] != L' ') {
							break;
						}
					}
				}
			}
			szLabel[k] = L'\0';
//			MYTRACE(_T("szLabel=[%ls]\n"), szLabel);

			pszKeyword = L"division";
			nKeywordLen = wcslen(pszKeyword);
			bDivision = FALSE;
			int nLen = (int)wcslen(szLabel) - nKeywordLen;
			for (i=0; i<=nLen ; ++i) {
				if (auto_memicmp(&szLabel[i], pszKeyword, nKeywordLen) == 0) {
					szLabel[i + nKeywordLen] = L'\0';
					wcscpy_s(szDivision, szLabel);
					bDivision = TRUE;
					break;
				}
			}
			if (bDivision) {
				continue;
			}
			/*
			  カーソル位置変換
			  物理位置(行頭からのバイト数、折り返し無し行位置)
			  →
			  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/

			LayoutPoint ptPos;
			wchar_t	szWork[1024];
			m_pDocRef->m_layoutMgr.LogicToLayout(
				LogicPoint(0, nLineCount),
				&ptPos
			);
			auto_sprintf_s(szWork, L"%ls::%ls", szDivision, szLabel);
			pFuncInfoArr->AppendData(nLineCount + LogicInt(1), ptPos.GetY2() + LayoutInt(1) , szWork, 0);
		}
	}
	return;
}

// Jul. 10, 2001 JEPRO 追加
const wchar_t* g_ppszKeywordsCOBOL[] = {
	L"ACCEPT",
	L"ADD",
	L"ADVANCING",
	L"AFTER",
	L"ALL",
	L"AND",
	L"ARGUMENT",
	L"ASSIGN",
	L"AUTHOR",
	L"BEFORE",
	L"BLOCK",
	L"BY",
	L"CALL",
	L"CHARACTERS",
	L"CLOSE",
	L"COMP",
	L"COMPILED",
	L"COMPUTE",
	L"COMPUTER",
	L"CONFIGURATION",
	L"CONSOLE",
	L"CONTAINS",
	L"CONTINUE",
	L"CONTROL",
	L"COPY",
	L"DATA",
	L"DELETE",
	L"DISPLAY",
	L"DIVIDE",
	L"DIVISION",
	L"ELSE",
	L"END",
	L"ENVIRONMENT",
	L"EVALUATE",
	L"EXAMINE",
	L"EXIT",
	L"EXTERNAL",
	L"FD",
	L"FILE",
	L"FILLER",
	L"FROM",
	L"GIVING",
	L"GO",
	L"GOBACK",
	L"HIGH-VALUE",
	L"IDENTIFICATION"
	L"IF",
	L"INITIALIZE",
	L"INPUT",
	L"INTO",
	L"IS",
	L"LABEL",
	L"LINKAGE",
	L"LOW-VALUE",
	L"MODE",
	L"MOVE",
	L"NOT",
	L"OBJECT",
	L"OCCURS",
	L"OF",
	L"ON",
	L"OPEN",
	L"OR",
	L"OTHER",
	L"OUTPUT",
	L"PERFORM",
	L"PIC",
	L"PROCEDURE",
	L"PROGRAM",
	L"READ",
	L"RECORD",
	L"RECORDING",
	L"REDEFINES",
	L"REMAINDER",
	L"REMARKS",
	L"REPLACING",
	L"REWRITE",
	L"ROLLBACK",
	L"SECTION",
	L"SELECT",
	L"SOURCE",
	L"SPACE",
	L"STANDARD",
	L"STOP",
	L"STORAGE",
	L"SYSOUT",
	L"TEST",
	L"THEN",
	L"TO",
	L"TODAY",
	L"TRANSFORM",
	L"UNTIL",
	L"UPON",
	L"USING",
	L"VALUE",
	L"VARYING",
	L"WHEN",
	L"WITH",
	L"WORKING",
	L"WRITE",
	L"WRITTEN",
	L"ZERO"
};
int g_nKeywordsCOBOL = _countof(g_ppszKeywordsCOBOL);

