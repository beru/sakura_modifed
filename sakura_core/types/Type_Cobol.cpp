#include "StdAfx.h"
#include "types/Type.h"
#include "doc/EditDoc.h"
#include "doc/DocOutline.h"
#include "doc/logic/DocLine.h"
#include "outline/FuncInfoArr.h"
#include "view/Colors/EColorIndexType.h"

// COBOL
void CType_Cobol::InitTypeConfigImp(TypeConfig& type)
{
	// 名前と拡張子
	_tcscpy(type.szTypeName, _T("COBOL"));
	_tcscpy(type.szTypeExts, _T("cbl,cpy,pco,cob"));

	// 設定
	type.lineComment.CopyTo(0, L"*", 6);
	type.lineComment.CopyTo(1, L"D", 6);
	type.stringType = StringLiteralType::PLSQL;		// 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""]['']
	wcscpy_s(type.szIndentChars, L"*");				// その他のインデント対象文字
	type.nKeywordSetIdx[0] = 3;						// キーワードセット	
	type.eDefaultOutline = OutlineType::Cobol;		// アウトライン解析方法
	// 指定桁縦線
	type.colorInfoArr[COLORIDX_VERTLINE].bDisp = true;
	type.nVertLineIdx[0] = 7;
	type.nVertLineIdx[1] = 8;
	type.nVertLineIdx[2] = 12;
	type.nVertLineIdx[3] = 73;
}

// COBOL アウトライン解析
void DocOutline::MakeTopicList_cobol(FuncInfoArr* pFuncInfoArr)
{
	const wchar_t*	pLine;
	size_t			nLineLen;
	size_t			i;
	int				k;
	wchar_t			szDivision[1024];
	wchar_t			szLabel[1024];
	const wchar_t*	pszKeyword;
	bool			bDivision;
	bool			bExtEol = GetDllShareData().common.edit.bEnableExtEol;

	szDivision[0] = L'\0';
	szLabel[0] =  L'\0';


	size_t nLineCount;
	for (nLineCount=0; nLineCount<doc.docLineMgr.GetLineCount(); ++nLineCount) {
		pLine = doc.docLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
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
			size_t nKeywordLen = wcslen(pszKeyword);
			bDivision = false;
			size_t nLen = wcslen(szLabel) - nKeywordLen;
			for (i=0; i<=nLen ; ++i) {
				if (auto_memicmp(&szLabel[i], pszKeyword, nKeywordLen) == 0) {
					szLabel[i + nKeywordLen] = L'\0';
					wcscpy_s(szDivision, szLabel);
					bDivision = true;
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

			wchar_t	szWork[1024];
			Point ptPos = doc.layoutMgr.LogicToLayout(Point(0, (int)nLineCount));
			auto_sprintf_s(szWork, L"%ls::%ls", szDivision, szLabel);
			pFuncInfoArr->AppendData((int)nLineCount + 1, ptPos.y + 1 , szWork, 0);
		}
	}
	return;
}

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
size_t g_nKeywordsCOBOL = _countof(g_ppszKeywordsCOBOL);

