#include "StdAfx.h"
#include "types/Type.h"
#include "doc/EditDoc.h"
#include "doc/DocOutline.h"
#include "doc/logic/DocLine.h"
#include "outline/FuncInfoArr.h"

// PL/SQL
void CType_Sql::InitTypeConfigImp(TypeConfig& type)
{
	// 名前と拡張子
	_tcscpy(type.szTypeName, _T("PL/SQL"));
	_tcscpy(type.szTypeExts, _T("sql,plsql"));

	// 設定
	type.lineComment.CopyTo(0, L"--", -1);					// 行コメントデリミタ
	type.blockComments[0].SetBlockCommentRule(L"/*", L"*/");	// ブロックコメントデリミタ
	type.stringType = StringLiteralType::PLSQL;				// 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""]['']
	wcscpy_s(type.szIndentChars, L"|★");						// その他のインデント対象文字
	type.nKeywordSetIdx[0] = 2;								// キーワードセット
	type.eDefaultOutline = OutlineType::PLSQL;				// アウトライン解析方法
}


// PL/SQL関数リスト作成
void DocOutline::MakeFuncList_PLSQL(FuncInfoArr* pFuncInfoArr)
{
	const wchar_t*	pLine;
	size_t		nLineLen;
	wchar_t		szWordPrev[100];
	wchar_t		szWord[100];
	size_t		nWordIdx = 0;
	size_t 		nMaxWordLeng = 70;
	int			nMode;
	wchar_t		szFuncName[100];
	size_t		nFuncLine = 0;
	int			nFuncId = 0;
	int			nFuncNum;
	int			nFuncOrProc = 0;
	int			nParseCnt = 0;
	bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;

	szWordPrev[0] = L'\0';
	szWord[nWordIdx] = L'\0';
	nMode = 0;
	nFuncNum = 0;
	for (size_t nLineCount=0; nLineCount<doc.docLineMgr.GetLineCount(); ++nLineCount) {
		pLine = doc.docLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		for (size_t i=0; i<nLineLen; ++i) {
			// 1バイト文字だけを処理する
			size_t nCharChars = NativeW::GetSizeOfChar(pLine, nLineLen, i);
			if (nCharChars == 0) {
				nCharChars = 1;
			}
//			if (1 < nCharChars) {
//				i += (nCharChars - 1);
//				continue;
//			}
			// シングルクォーテーション文字列読み込み中
			if (nMode == 20) {
				if (L'\'' == pLine[i]) {
					if (i + 1 < nLineLen && L'\'' == pLine[i + 1]) {
						++i;
					}else {
						nMode = 0;
						continue;
					}
				}else {
				}
			}else
			// コメント読み込み中
			if (nMode == 8) {
				if (i + 1 < nLineLen && L'*' == pLine[i] &&  L'/' == pLine[i + 1]) {
					++i;
					nMode = 0;
					continue;
				}else {
				}
			}else
			// 単語読み込み中
			if (nMode == 1) {
				if ((nCharChars == 1 && (
					pLine[i] == L'_' ||
					pLine[i] == L'~' ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z')||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z')||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9')||
					(L'\u00a1' <= pLine[i] && !iswcntrl(pLine[i]) && !iswspace(pLine[i])) // 日本語対応
					))
				 || nCharChars == 2
				) {
//					++nWordIdx;
					if (nWordIdx >= nMaxWordLeng) {
						nMode = 999;
						i += (nCharChars - 1);
						continue;
					}else {
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						wmemcpy(&szWord[nWordIdx], &pLine[i], nCharChars);
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += nCharChars;
					}
				}else {
					if (nParseCnt == 0 && wcsicmp(szWord, L"FUNCTION") == 0) {
						nFuncOrProc = 1;
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}else if (nParseCnt == 0 && wcsicmp(szWord, L"PROCEDURE") == 0) {
						nFuncOrProc = 2;
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}else if (nParseCnt == 0 && wcsicmp(szWord, L"PACKAGE") == 0) {
						nFuncOrProc = 3;
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}else if (nParseCnt == 1 && nFuncOrProc == 3 && wcsicmp(szWord, L"BODY") == 0) {
						nFuncOrProc = 4;
						nParseCnt = 1;
					}else if (nParseCnt == 1) {
						if (nFuncOrProc == 1 ||
							nFuncOrProc == 2 ||
							nFuncOrProc == 3 ||
							nFuncOrProc == 4
						) {
							++nParseCnt;
							wcscpy_s(szFuncName, szWord);
//						}else
//						if (nFuncOrProc == 3) {

						}
					}else if (nParseCnt == 2) {
						if (wcsicmp(szWord, L"IS") == 0) {
							if (nFuncOrProc == 1) {
								nFuncId = 11;	// ファンクション本体
							}else if (nFuncOrProc == 2) {
								nFuncId = 21;	// プロシージャ本体
							}else if (nFuncOrProc == 3) {
								nFuncId = 31;	// パッケージ仕様部
							}else if (nFuncOrProc == 4) {
								nFuncId = 41;	// パッケージ本体
							}
							++nFuncNum;
							/*
							  カーソル位置変換
							  物理位置(行頭からのバイト数、折り返し無し行位置)
							  →
							  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
							*/
							Point ptPos = doc.layoutMgr.LogicToLayout(Point(0, (int)nFuncLine - 1));
							pFuncInfoArr->AppendData(nFuncLine, ptPos.y + 1, szFuncName, nFuncId);
							nParseCnt = 0;
						}
						if (wcsicmp(szWord, L"AS") == 0) {
							if (nFuncOrProc == 3) {
								nFuncId = 31;	// パッケージ仕様部
								++nFuncNum;
								/*
								  カーソル位置変換
								  物理位置(行頭からのバイト数、折り返し無し行位置)
								  →
								  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
								*/
								Point ptPos = doc.layoutMgr.LogicToLayout(Point(0, (int)nFuncLine - 1));
								pFuncInfoArr->AppendData(nFuncLine, ptPos.y + 1 , szFuncName, nFuncId);
								nParseCnt = 0;
							}else if (nFuncOrProc == 4) {
								nFuncId = 41;	// パッケージ本体
								++nFuncNum;
								/*
								  カーソル位置変換
								  物理位置(行頭からのバイト数、折り返し無し行位置)
								  →
								  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
								*/
								Point ptPos = doc.layoutMgr.LogicToLayout(Point(0, (int)nFuncLine - 1));
								pFuncInfoArr->AppendData(nFuncLine, ptPos.y + 1 , szFuncName, nFuncId);
								nParseCnt = 0;
							}
						}
					}
					wcscpy_s(szWordPrev, szWord);
					nWordIdx = 0;
					szWord[0] = L'\0';
					nMode = 0;
					--i;
					continue;
				}
			// 記号列読み込み中
			}else if (nMode == 2) {
				if (L'_' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z')||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z')||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9')||
					(L'\u00a1' <= pLine[i] && !iswcntrl(pLine[i]) && !iswspace(pLine[i]))|| // 日本語対応
					L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::IsLineDelimiter(pLine[i], bExtEol) ||
					L'{' == pLine[i] ||
					L'}' == pLine[i] ||
					L'(' == pLine[i] ||
					L')' == pLine[i] ||
					L';' == pLine[i] ||
					L'\'' == pLine[i] ||
					L'/' == pLine[i] ||
					L'-' == pLine[i] ||
					nCharChars == 2
				) {
					wcscpy_s(szWordPrev, szWord);
					nWordIdx = 0;
					szWord[0] = L'\0';
					nMode = 0;
					--i;
					continue;
				}else {
//					++nWordIdx;
					if (nWordIdx >= nMaxWordLeng) {
						nMode = 999;
						continue;
					}else {
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						wmemcpy(&szWord[nWordIdx], &pLine[i], nCharChars);
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);
					}
				}
			// 長過ぎる単語無視中
			}else if (nMode == 999) {
				// 空白やタブ記号等を飛ばす
				if (pLine[i] == L'\t' ||
					pLine[i] == L' ' ||
					WCODE::IsLineDelimiter(pLine[i], bExtEol)
				) {
					nMode = 0;
					continue;
				}
			// ノーマルモード
			}else if (nMode == 0) {
				// 空白やタブ記号等を飛ばす
				if (pLine[i] == L'\t' ||
					pLine[i] == L' ' ||
					WCODE::IsLineDelimiter(pLine[i], bExtEol)
				) {
					continue;
				}else if (i < nLineLen - 1 && pLine[i] == L'-' && pLine[i + 1] == L'-') {
					break;
				}else if (i < nLineLen - 1 && pLine[i] == L'/' && pLine[i + 1] == L'*') {
					++i;
					nMode = 8;
					continue;
				}else if (pLine[i] == L'\'') {
					nMode = 20;
					continue;
				}else if (pLine[i] == L';') {
					if (nParseCnt == 2) {
						if (nFuncOrProc == 1) {
							nFuncId = 10;	// ファンクション宣言
						}else {
							nFuncId = 20;	// プロシージャ宣言
						}
						++nFuncNum;
						/*
						  カーソル位置変換
						  物理位置(行頭からのバイト数、折り返し無し行位置)
						  →
						  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
						*/
						Point ptPos = doc.layoutMgr.LogicToLayout(Point(0, (int)nFuncLine - 1));
						pFuncInfoArr->AppendData(nFuncLine, ptPos.y + 1 , szFuncName, nFuncId);
						nParseCnt = 0;
					}
					nMode = 0;
					continue;
				}else {
					if (
						(
							nCharChars == 1
							&& (
								pLine[i] == L'_' ||
								pLine[i] == L'~' ||
								(L'a' <= pLine[i] && pLine[i] <= L'z')||
								(L'A' <= pLine[i] && pLine[i] <= L'Z')||
								(L'0' <= pLine[i] && pLine[i] <= L'9')||
								(L'\u00a1' <= pLine[i] && !iswcntrl(pLine[i]) && !iswspace(pLine[i])) // 日本語対応
							)
						)
						|| nCharChars == 2
					) {
						wcscpy_s(szWordPrev, szWord);
						nWordIdx = 0;

//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						wmemcpy(&szWord[nWordIdx], &pLine[i], nCharChars);
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);

						nMode = 1;
					}else {
						wcscpy_s(szWordPrev, szWord);
						nWordIdx = 0;
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';

						wmemcpy(&szWord[nWordIdx], &pLine[i], nCharChars);
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);

						nMode = 2;
					}
				}
			}
			i += (nCharChars - 1);
		}
	}
	return;
}

const wchar_t* g_ppszKeywordsPLSQL[] = {
	L"AND",
	L"AS",
	L"BEGIN",
	L"BINARY_INTEGER",
	L"BODY",
	L"BOOLEAN",
	L"BY",
	L"CHAR",
	L"CHR",
	L"COMMIT",
	L"COUNT",
	L"CREATE",
	L"CURSOR",
	L"DATE",
	L"DECLARE",
	L"DEFAULT",
	L"DELETE",
	L"ELSE",
	L"ELSIF",
	L"END",
	L"ERRORS",
	L"EXCEPTION",
	L"FALSE",
	L"FOR",
	L"FROM",
	L"FUNCTION",
	L"GOTO",
	L"HTP",
	L"IDENT_ARR",
	L"IF",
	L"IN",
	L"INDEX",
	L"INTEGER",
	L"IS",
	L"LOOP",
	L"NOT",
	L"NO_DATA_FOUND",
	L"NULL",
	L"NUMBER",
	L"OF",
	L"OR",
	L"ORDER",
	L"OUT",
	L"OWA_UTIL",
	L"PACKAGE",
	L"PRAGMA",
	L"PRN",
	L"PROCEDURE",
	L"REPLACE",
	L"RESTRICT_REFERENCES",
	L"RETURN",
	L"ROWTYPE",
	L"SELECT",
	L"SHOW",
	L"SUBSTR",
	L"TABLE",
	L"THEN",
	L"TRUE",
	L"TYPE",
	L"UPDATE",
	L"VARCHAR",
	L"VARCHAR2",
	L"WHEN",
	L"WHERE",
	L"WHILE",
	L"WNDS",
	L"WNPS",
	L"RAISE",
	L"INSERT",
	L"INTO",
	L"VALUES",
	L"SET",
	L"SYSDATE",
	L"RTRIM",
	L"LTRIM",
	L"TO_CHAR",
	L"DUP_VAL_ON_INDEX",
	L"ROLLBACK",
	L"OTHERS",
	L"SQLCODE"
};
size_t g_nKeywordsPLSQL = _countof(g_ppszKeywordsPLSQL);

