#include "StdAfx.h"
#include "types/Type.h"

// MS-DOSバッチファイル
void CType_Dos::InitTypeConfigImp(TypeConfig& type)
{
	// 名前と拡張子
	_tcscpy(type.szTypeName, _T("MS-DOSバッチファイル"));
	_tcscpy(type.szTypeExts, _T("bat"));

	// 設定
	type.lineComment.CopyTo(0, L"REM ", -1);		// 行コメントデリミタ
	type.eDefaultOutline = OutlineType::Text;		// アウトライン解析方法
	type.nKeywordSetIdx[0] = 7;					// キーワードセット
}

const wchar_t* g_ppszKeywordsBAT[] = {
	L"PATH",
	L"PROMPT",
	L"TEMP",
	L"TMP",
	L"TZ",
	L"CONFIG",
	L"COMSPEC",
	L"DIRCMD",
	L"COPYCMD",
	L"winbootdir",
	L"windir",
	L"DIR",
	L"CALL",
	L"CHCP",
	L"RENAME",
	L"REN",
	L"ERASE",
	L"DEL",
	L"TYPE",
	L"REM",
	L"COPY",
	L"PAUSE",
	L"DATE",
	L"TIME",
	L"VER",
	L"VOL",
	L"CD",
	L"CHDIR",
	L"MD",
	L"MKDIR",
	L"RD",
	L"RMDIR",
	L"BREAK",
	L"VERIFY",
	L"SET",
	L"EXIT",
	L"CTTY",
	L"ECHO",
	L"@ECHO",	// Oct. 31, 2000 JEPRO '@' を強調可能にしたので追加
	L"LOCK",
	L"UNLOCK",
	L"GOTO",
	L"SHIFT",
	L"IF",
	L"FOR",
	L"DO",		// Nov. 2, 2000 JEPRO 追加
	L"IN",		// Nov. 2, 2000 JEPRO 追加
	L"ELSE",	// Nov. 2, 2000 JEPRO 追加 Win2000で使える
	L"CLS",
	L"TRUENAME",
	L"LOADHIGH",
	L"LH",
	L"LFNFOR",
	L"ON",
	L"OFF",
	L"NOT",
	L"ERRORLEVEL",
	L"EXIST",
	L"NUL",
	L"CON",
	L"AUX",
	L"COM1",
	L"COM2",
	L"COM3",
	L"COM4",
	L"PRN",
	L"LPT1",
	L"LPT2",
	L"LPT3",
	L"CLOCK",
	L"CLOCK$",
	L"CONFIG$"
};
size_t g_nKeywordsBAT = _countof(g_ppszKeywordsBAT);

