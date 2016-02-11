/*!	@file
	キーボードマクロ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2001, hor
	Copyright (C) 2002, YAZAKI, aroka, genta, Moca, hor
	Copyright (C) 2003, 鬼, ryoji, Moca
	Copyright (C) 2004, genta, zenryaku
	Copyright (C) 2005, MIK, genta, maru, zenryaku, FILE
	Copyright (C) 2006, かろと, ryoji
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, nasukoji, ryoji
	Copyright (C) 2009, ryoji, nasukoji
	Copyright (C) 2011, syat

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
#include "func/Funccode.h"
#include "CMacro.h"
#include "_main/CControlTray.h"
#include "cmd/CViewCommander_inline.h"
#include "view/CEditView.h"		// 2002/2/10 aroka
#include "macro/CSMacroMgr.h"	// 2002/2/10 aroka
#include "doc/CEditDoc.h"		// 2002/5/13 YAZAKI ヘッダ整理
#include "_os/OleTypes.h"		// 2003-02-21 鬼
#include "io/CTextStream.h"
#include "window/CEditWnd.h"
#include "env/CSakuraEnvironment.h"
#include "dlg/CDlgInput1.h"
#include "dlg/CDlgOpenFile.h"
#include "util/format.h"
#include "util/shell.h"
#include "util/ole_convert.h"
#include "util/os.h"
#include "uiparts/CWaitCursor.h"

Macro::Macro(EFunctionCode nFuncID)
{
	m_nFuncID = nFuncID;
	m_pNext = NULL;
	m_pParamTop = m_pParamBot = NULL;
}

Macro::~Macro(void)
{
	ClearMacroParam();
}

void Macro::ClearMacroParam()
{
	MacroParam* p = m_pParamTop;
	MacroParam* del_p;
	while (p) {
		del_p = p;
		p = p->m_pNext;
		delete del_p;
	}
	m_pParamTop = NULL;
	m_pParamBot = NULL;
	return;
}

/*	引数の型振り分け
	機能IDによって、期待する型は異なります。
	そこで、引数の型を機能IDによって振り分けて、AddParamしましょう。
	たとえば、F_INSTEXT_Wの1つめ、2つめの引数は文字列、3つめの引数はintだったりするのも、ここでうまく振り分けられることを期待しています。

	lParamは、HandleCommandのparamに値を渡しているコマンドの場合にのみ使います。
*/
void Macro::AddLParam(
	const LPARAM* lParams,
	const EditView* pcEditView
	)
{
	int nOption = 0;
	LPARAM lParam = lParams[0];
	switch (m_nFuncID) {
	case F_GOLOGICALLINETOP_BOX:
	case F_GOLINETOP_BOX:
	case F_GOLINEEND_BOX:
	case F_HalfPageUp_BOX:
	case F_HalfPageDown_BOX:
	case F_1PageUp_BOX:
	case F_1PageDown_BOX:
		nOption = 1;
	case F_UP_BOX:
	case F_DOWN_BOX:
	case F_LEFT_BOX:
	case F_RIGHT_BOX:
	case F_UP2_BOX:
	case F_DOWN2_BOX:
	case F_WORDLEFT_BOX:
	case F_WORDRIGHT_BOX:
	case F_GOFILETOP_BOX:
	case F_GOFILEEND_BOX:
		{
			if (nOption == 1) {
				switch (m_nFuncID) {
				case F_HalfPageUp_BOX:
				case F_HalfPageDown_BOX:
					if (lParam == 0) {
						AddIntParam( (Int)pcEditView->GetTextArea().m_nViewRowNum / 2 );
					}else {
						AddIntParam( lParam );
					}
					break;
				case F_1PageUp_BOX:
				case F_1PageDown_BOX:
					if (lParam == 0) {
						AddIntParam( (Int)pcEditView->GetTextArea().m_nViewRowNum - 1 );
					}else {
						AddIntParam( lParam );
					}
					break;
				default:
					AddIntParam( lParam );
					break;
				}
			}
			int nParamOption;
			if (nOption == 1) {
				nParamOption = lParams[1];
			}else {
				nParamOption = lParam;
			}
			if (nParamOption == 0) {
				if (GetDllShareData().m_common.m_edit.m_bBoxSelectLock) {
					nParamOption = 0x01;
				}else {
					nParamOption = 0x02;
				}
			}
			AddIntParam( nParamOption );
		}
		break;
	case F_INSTEXT_W:
	case F_FILEOPEN:
	case F_EXECEXTMACRO:
		{
			AddStringParam((const wchar_t*)lParam);	// lParamを追加。
		}
		break;

	case F_EXECMD:
		{
			AddStringParam((const wchar_t*)lParam);	// lParamを追加。
			AddIntParam((int)lParams[1]);
			if (lParams[2] != 0) {
				AddStringParam((const wchar_t*)lParams[2]);
			}
		}
		break;

	case F_JUMP:	// 指定行へジャンプ（ただしPL/SQLコンパイルエラー行へのジャンプは未対応）
		{
			AddIntParam(pcEditView->m_pEditWnd->m_dlgJump.m_nLineNum);
			LPARAM lFlag = 0x00;
			lFlag |= GetDllShareData().m_bLineNumIsCRLF_ForJump		? 0x01 : 0x00;
			lFlag |= pcEditView->m_pEditWnd->m_dlgJump.m_bPLSQL	? 0x02 : 0x00;
			AddIntParam(lFlag);
		}
		break;

	case F_BOOKMARK_PATTERN:	//2002.02.08 hor
	case F_SEARCH_NEXT:
	case F_SEARCH_PREV:
		{
			AddStringParam(pcEditView->m_strCurSearchKey.c_str());	// lParamを追加。

			LPARAM lFlag = 0x00;
			lFlag |= pcEditView->m_curSearchOption.bWordOnly		? 0x01 : 0x00;
			lFlag |= pcEditView->m_curSearchOption.bLoHiCase		? 0x02 : 0x00;
			lFlag |= pcEditView->m_curSearchOption.bRegularExp		? 0x04 : 0x00;
			lFlag |= GetDllShareData().m_common.m_search.m_bNOTIFYNOTFOUND				? 0x08 : 0x00;
			lFlag |= GetDllShareData().m_common.m_search.m_bAutoCloseDlgFind			? 0x10 : 0x00;
			lFlag |= GetDllShareData().m_common.m_search.m_bSearchAll					? 0x20 : 0x00;
			AddIntParam(lFlag);
		}
		break;
	case F_REPLACE:
	case F_REPLACE_ALL:
		{
			AddStringParam(pcEditView->m_strCurSearchKey.c_str());	// lParamを追加。
			AddStringParam(pcEditView->m_pEditWnd->m_dlgReplace.m_strText2.c_str());	// lParamを追加。

			LPARAM lFlag = 0x00;
			lFlag |= pcEditView->m_curSearchOption.bWordOnly		? 0x01 : 0x00;
			lFlag |= pcEditView->m_curSearchOption.bLoHiCase		? 0x02 : 0x00;
			lFlag |= pcEditView->m_curSearchOption.bRegularExp	? 0x04 : 0x00;
			lFlag |= GetDllShareData().m_common.m_search.m_bNOTIFYNOTFOUND				? 0x08 : 0x00;
			lFlag |= GetDllShareData().m_common.m_search.m_bAutoCloseDlgFind			? 0x10 : 0x00;
			lFlag |= GetDllShareData().m_common.m_search.m_bSearchAll					? 0x20 : 0x00;
			lFlag |= pcEditView->m_pEditWnd->m_dlgReplace.m_bPaste					? 0x40 : 0x00;	// CShareDataに入れなくていいの？
			lFlag |= GetDllShareData().m_common.m_search.m_bSelectedArea				? 0x80 : 0x00;	// 置換する時は選べない
			lFlag |= pcEditView->m_pEditWnd->m_dlgReplace.m_nReplaceTarget << 8;	// 8bitシフト（0x100で掛け算）
			lFlag |= GetDllShareData().m_common.m_search.m_bConsecutiveAll				? 0x0400: 0x00;	// 2007.01.16 ryoji
			AddIntParam(lFlag);
		}
		break;
	case F_GREP_REPLACE:
	case F_GREP:
		{
			DlgGrep* pcDlgGrep;
			DlgGrepReplace* pcDlgGrepRep;
			if (F_GREP == m_nFuncID) {
				pcDlgGrep = &pcEditView->m_pEditWnd->m_dlgGrep;
				pcDlgGrepRep = NULL;
				AddStringParam( pcDlgGrep->m_strText.c_str() );
			}else {
				pcDlgGrep = pcDlgGrepRep = &pcEditView->m_pEditWnd->m_dlgGrepReplace;
				AddStringParam( pcDlgGrep->m_strText.c_str() );
				AddStringParam( pcEditView->m_pEditWnd->m_dlgGrepReplace.m_strText2.c_str() );
			}
			AddStringParam(GetDllShareData().m_searchKeywords.m_aGrepFiles[0]);	// lParamを追加。
			AddStringParam(GetDllShareData().m_searchKeywords.m_aGrepFolders[0]);	// lParamを追加。

			LPARAM lFlag = 0x00;
			lFlag |= GetDllShareData().m_common.m_search.m_bGrepSubFolder				? 0x01 : 0x00;
			// この編集中のテキストから検索する(0x02.未実装)
			lFlag |= pcDlgGrep->m_searchOption.bLoHiCase		? 0x04 : 0x00;
			lFlag |= pcDlgGrep->m_searchOption.bRegularExp	? 0x08 : 0x00;
			lFlag |= (GetDllShareData().m_common.m_search.m_nGrepCharSet == CODE_AUTODETECT) ? 0x10 : 0x00;	// 2002/09/21 Moca 下位互換性のための処理
			lFlag |= GetDllShareData().m_common.m_search.m_nGrepOutputLineType == 1	? 0x20 : 0x00;
			lFlag |= GetDllShareData().m_common.m_search.m_nGrepOutputLineType == 2	? 0x400000 : 0x00;	// 2014.09.23 否ヒット行
			lFlag |= (GetDllShareData().m_common.m_search.m_nGrepOutputStyle == 2)		? 0x40 : 0x00;	// CShareDataに入れなくていいの？
			lFlag |= (GetDllShareData().m_common.m_search.m_nGrepOutputStyle == 3)		? 0x80 : 0x00;
			ECodeType code = GetDllShareData().m_common.m_search.m_nGrepCharSet;
			if (IsValidCodeType(code) || CODE_AUTODETECT == code) {
				lFlag |= code << 8;
			}
			lFlag |= pcDlgGrep->m_searchOption.bWordOnly								? 0x10000 : 0x00;
			lFlag |= GetDllShareData().m_common.m_search.m_bGrepOutputFileOnly			? 0x20000 : 0x00;
			lFlag |= GetDllShareData().m_common.m_search.m_bGrepOutputBaseFolder		? 0x40000 : 0x00;
			lFlag |= GetDllShareData().m_common.m_search.m_bGrepSeparateFolder			? 0x80000 : 0x00;
			if (F_GREP_REPLACE == m_nFuncID) {
				lFlag |= pcDlgGrepRep->m_bPaste											? 0x100000 : 0x00;
				lFlag |= GetDllShareData().m_common.m_search.m_bGrepBackup				? 0x200000 : 0x00;
			}
			AddIntParam(lFlag);
			AddIntParam(code);
		}
		break;
	// 数値パラメータを追加
	case F_WCHAR:
	case F_CTRL_CODE:
		AddIntParam(lParam); // ※文字コードが渡される
		break;
	case F_CHGMOD_EOL:
		{
			// EOLタイプ値をマクロ引数値に変換する	// 2009.08.18 ryoji
			int nFlag;
			switch ((int)lParam) {
			case EolType::CRLF:	nFlag = 1; break;
//			case EOL_LFCR:	nFlag = 2; break;
			case EolType::LF:	nFlag = 3; break;
			case EolType::CR:	nFlag = 4; break;
			case EolType::NEL:	nFlag = 5; break;
			case EolType::LS:	nFlag = 6; break;
			case EolType::PS:	nFlag = 7; break;
			default:		nFlag = 0; break;
			}
			AddIntParam(nFlag);
		}
		break;
	case F_SETFONTSIZE:
		{
			AddIntParam(lParam);
			AddIntParam(lParams[1]);
			AddIntParam(lParams[2]);
		}
		break;
	// 2014.01.15 PageUp/Down系追加
	case F_HalfPageUp:
	case F_HalfPageUp_Sel:
	case F_HalfPageDown:
	case F_HalfPageDown_Sel:
		if (lParam == 0) {
			AddIntParam( (Int)pcEditView->GetTextArea().m_nViewRowNum / 2 );
		}else {
			AddIntParam( lParam );
		}
		break;
	case F_1PageUp:
	case F_1PageUp_Sel:
	case F_1PageDown:
	case F_1PageDown_Sel:
		if (lParam == 0) {
			AddIntParam( (Int)pcEditView->GetTextArea().m_nViewRowNum - 1 );
		}else {
			AddIntParam( lParam );
		}
		break;

	// 標準もパラメータを追加
	default:
		AddIntParam(lParam);
		break;
	}
}

void MacroParam::SetStringParam( const WCHAR* szParam, int nLength )
{
	Clear();
	int nLen;
	if (nLength == -1) {
		nLen = auto_strlen( szParam );
	}else {
		nLen = nLength;
	}
	m_pData = new WCHAR[nLen + 1];
	auto_memcpy( m_pData, szParam, nLen );
	m_pData[nLen] = LTEXT('\0');
	m_nDataLen = nLen;
	m_type = MacroParamType::Str;
}

void MacroParam::SetIntParam(const int nParam)
{
	Clear();
	m_pData = new WCHAR[16];	//	数値格納（最大16桁）用
	_itow(nParam, m_pData, 10);
	m_nDataLen = auto_strlen(m_pData);
	m_type = MacroParamType::Int;
}

// 引数に文字列を追加。
void Macro::AddStringParam(const WCHAR* szParam, int nLength)
{
	MacroParam* param = new MacroParam();

	param->SetStringParam( szParam, nLength );

	// リストの整合性を保つ
	if (m_pParamTop) {
		m_pParamBot->m_pNext = param; 
		m_pParamBot = param;
	}else {
		m_pParamTop = param;
		m_pParamBot = m_pParamTop;
	}
}

// 引数に数値を追加
void Macro::AddIntParam(const int nParam)
{
	MacroParam* param = new MacroParam();

	param->SetIntParam( nParam );

	// リストの整合性を保つ
	if (m_pParamTop) {
		m_pParamBot->m_pNext = param; 
		m_pParamBot = param;
	}else {
		m_pParamTop = param;
		m_pParamBot = m_pParamTop;
	}
}

/**	コマンドを実行する（pcEditView->GetCommander().HandleCommandを発行する）
	m_nFuncIDによって、引数の型を正確に渡してあげましょう。
	
	@note
	paramArrは何かのポインタ（アドレス）をLONGであらわした値になります。
	引数がchar*のときは、paramArr[i]をそのままHandleCommandに渡してかまいません。
	引数がintのときは、*((int*)paramArr[i])として渡しましょう。
	
	たとえば、F_INSTEXT_Wの1つめ、2つめの引数は文字列、3つめの引数はint、4つめの引数が無し。だったりする場合は、次のようにしましょう。
	pcEditView->GetCommander().HandleCommand(m_nFuncID, true, paramArr[0], paramArr[1], *((int*)paramArr[2]), 0);
	
	@date 2007.07.20 genta : flags追加．FA_FROMMACROはflagsに含めて渡すものとする．
		(1コマンド発行毎に毎回演算する必要はないので)
*/
bool Macro::Exec(EditView* pcEditView, int flags) const
{
	const int maxArg = 8;
	const WCHAR* paramArr[maxArg] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	int paramLenArr[maxArg] = {0, 0, 0, 0, 0, 0, 0, 0};

	MacroParam* p = m_pParamTop;
	int i = 0;
	for (i=0; i<maxArg; ++i) {
		if (!p) break;	// pが無ければbreak;
		paramArr[i] = p->m_pData;
		paramLenArr[i] = wcslen(paramArr[i]);
		p = p->m_pNext;
	}
	return Macro::HandleCommand(pcEditView, (EFunctionCode)(m_nFuncID | flags), paramArr, paramLenArr, i);
}

WCHAR* Macro::GetParamAt(MacroParam* p, int index)
{
	MacroParam* x = p;
	int i = 0;
	while (i < index) {
		if (!x) {
			return NULL;
		}
		x = x->m_pNext;
		++i;
	}
	if (!x) {
		return NULL;
	}
	return x->m_pData;
}

int Macro::GetParamCount() const
{
	MacroParam* p = m_pParamTop;
	int n = 0;
	while (p) {
		++n;
		p = p->m_pNext;
	}
	return n;
}

static inline int wtoi_def(
	const WCHAR* arg,
	int def_val
	)
{
	return (!arg ? def_val: _wtoi(arg));
}

static inline const WCHAR* wtow_def(
	const WCHAR* arg,
	const WCHAR* def_val
	)
{
	return (!arg ? def_val: arg);
}

/*	Macroを再現するための情報をhFileに書き出します。

	InsText("なんとか");
	のように。
	AddLParam以外にCKeyMacroMgr::LoadKeyMacroによってもMacroが作成される点に注意
*/
void Macro::Save(HINSTANCE hInstance, TextOutputStream& out) const
{
	WCHAR			szFuncName[1024];
	WCHAR			szFuncNameJapanese[500];
	int				nTextLen;
	const WCHAR*	pText;
	NativeW		cmemWork;
	int nFuncID = m_nFuncID;

	// 2002.2.2 YAZAKI SMacroMgrに頼む
	if (SMacroMgr::GetFuncInfoByID( hInstance, nFuncID, szFuncName, szFuncNameJapanese)){
		// 2014.01.24 Moca マクロ書き出しをm_typeを追加して統合
		out.WriteF( L"S_%ls(", szFuncName );
		MacroParam* pParam = m_pParamTop;
		while (pParam) {
			if (pParam != m_pParamTop) {
				out.WriteString( L", " );
			}
			switch (pParam->m_type) {
			case MacroParamType::Int:
				out.WriteString( pParam->m_pData );
				break;
			case MacroParamType::Str:
				pText = pParam->m_pData;
				nTextLen = pParam->m_nDataLen;
				cmemWork.SetString( pText, nTextLen );
				cmemWork.Replace( L"\\", L"\\\\" );
				cmemWork.Replace( L"\'", L"\\\'" );
				cmemWork.Replace( L"\r", L"\\\r" );
				cmemWork.Replace( L"\n", L"\\\n" );
				cmemWork.Replace( L"\t", L"\\\t" );
				cmemWork.Replace( L"\0", 1, L"\\u0000", 6 );
				const wchar_t u0085[] = {0x85, 0};
				cmemWork.Replace( u0085, L"\\u0085" );
				cmemWork.Replace( L"\u2028", L"\\u2028" );
				cmemWork.Replace( L"\u2029", L"\\u2029" );
				for (int c=0; c<0x20; ++c) {
					int nLen = cmemWork.GetStringLength();
					const wchar_t* p = cmemWork.GetStringPtr();
					for (int i=0; i<nLen; ++i) {
						if (p[i] == c) {
							wchar_t from[2];
							wchar_t to[7];
							from[0] = c;
							from[1] = L'\0';
							auto_sprintf( to, L"\\u%4x", c );
							cmemWork.Replace( from, to );
							break;
						}
					}
				}
				const wchar_t u007f[] = {0x7f, 0};
				cmemWork.Replace( u007f, L"\\u007f" );
				out.WriteString( L"'" );
				out.WriteString( cmemWork.GetStringPtr(), cmemWork.GetStringLength() );
				out.WriteString( L"'" );
				break;
			}
			pParam = pParam->m_pNext;
		}
		out.WriteF( L");\t// %ls\r\n", szFuncNameJapanese );
		return;
	}
	out.WriteF( LSW(STR_ERR_DLGMACRO01) );
}

/**	マクロ引数変換

	MacroコマンドをpcEditView->GetCommander().HandleCommandに引き渡す．
	引数がないマクロを除き，マクロとHandleCommandでの対応をここで定義する必要がある．

	@param pcEditView	[in]	操作対象EditView
	@param Index	[in] 下位16bit: 機能ID, 上位ワードはそのままMacro::HandleCommand()に渡す．
	@param Argument [in] 引数
	@param ArgSize	[in] 引数の数
	
	@date 2007.07.08 genta Indexのコマンド番号を下位ワードに制限
*/
bool Macro::HandleCommand(
	EditView*			pcEditView,
	const EFunctionCode	Index,
	const WCHAR*		Argument[],
	const int			ArgLengths[],
	const int			ArgSize
	)
{
	std::tstring EXEC_ERROR_TITLE_string = LS(STR_ERR_DLGMACRO02);
	const TCHAR* EXEC_ERROR_TITLE = EXEC_ERROR_TITLE_string.c_str();
	int nOptions = 0;

	switch (LOWORD(Index)) {
	case F_WCHAR:		// 文字入力。数値は文字コード
	case F_IME_CHAR:	// 日本語入力
	case F_CTRL_CODE:
		// Jun. 16, 2002 genta
		if (!Argument[0]) {
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO03)
			);
			return false;
		}
	case F_PASTE:	// 2011.06.26 Moca
	case F_PASTEBOX:	// 2011.06.26 Moca
	case F_TEXTWRAPMETHOD:	// テキストの折り返し方法の指定。数値は、0x0（折り返さない）、0x1（指定桁で折り返す）、0x2（右端で折り返す）	// 2008.05.30 nasukoji
	case F_GOLINETOP:	// 行頭に移動。数値は、0x0（デフォルト）、0x1（空白を無視して先頭に移動）、0x2（未定義）、0x4（選択して移動）、0x8（改行単位で先頭に移動）
	case F_GOLINETOP_SEL:
	case F_GOLINEEND:	// 行末に移動
	case F_GOLINEEND_SEL:
	case F_SELECT_COUNT_MODE:	// 文字カウントの方法を指定。数値は、0x0（変更せず取得のみ）、0x1（文字数）、0x2（バイト数）、0x3（文字数⇔バイト数トグル）	// 2009.07.06 syat
	case F_OUTLINE:	// アウトライン解析のアクションを指定。数値は、0x0（画面表示）、0x1（画面表示＆再解析）、0x2（画面表示トグル）
	case F_CHANGETYPE:
	case F_TOGGLE_KEY_SEARCH:
		// 一つ目の引数が数値。
	case F_WHEELUP:
	case F_WHEELDOWN:
	case F_WHEELLEFT:
	case F_WHEELRIGHT:
	case F_WHEELPAGEUP:
	case F_WHEELPAGEDOWN:
	case F_WHEELPAGELEFT:
	case F_WHEELPAGERIGHT:
	case F_HalfPageUp:
	case F_HalfPageDown:
	case F_HalfPageUp_Sel:
	case F_HalfPageDown_Sel:
	case F_1PageUp:
	case F_1PageDown:
	case F_1PageUp_Sel:
	case F_1PageDown_Sel:
		pcEditView->GetCommander().HandleCommand(Index, true, (Argument[0] ? _wtoi(Argument[0]) : 0), 0, 0, 0);
		break;
	case F_UP_BOX:
	case F_DOWN_BOX:
	case F_LEFT_BOX:
	case F_RIGHT_BOX:
	case F_UP2_BOX:
	case F_DOWN2_BOX:
	case F_WORDLEFT_BOX:
	case F_WORDRIGHT_BOX:
	case F_GOFILETOP_BOX:
	case F_GOFILEEND_BOX:
		nOptions = 1;
	case F_GOLOGICALLINETOP_BOX:
	case F_GOLINETOP_BOX:
	case F_GOLINEEND_BOX:
	case F_HalfPageUp_BOX:
	case F_HalfPageDown_BOX:
	case F_1PageUp_BOX:
	case F_1PageDown_BOX:
		{
			// 0: 共通設定
			// 1: true(マクロのデフォルト値)
			// 2: false
			// マクロのデフォルト値はtrue(1)だが、EditView側のデフォルトは共通設定(0)
			int nBoxLock = wtoi_def(Argument[nOptions == 1 ? 0 : 1], 1);
			if (nOptions == 1) {
				pcEditView->GetCommander().HandleCommand( Index, true, nBoxLock, 0, 0, 0 );
			}else {
				pcEditView->GetCommander().HandleCommand( Index, true, wtoi_def(Argument[0], 0), nBoxLock, 0, 0 );
			}
		}
		break;
	case F_CHGMOD_EOL:	// 入力改行コード指定。EolTypeの数値を指定。2003.06.23 Moca
		// Jun. 16, 2002 genta
		if (!Argument[0]) {
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO03_1)
			);
			return false;
		}
		{
			// マクロ引数値をEOLタイプ値に変換する	// 2009.08.18 ryoji
			EolType nEol;
			switch (Argument[0] ? _wtoi(Argument[0]) : 0) {
			case 1:		nEol = EolType::CRLF; break;
//			case 2:		nEol = EOL_LFCR; break;
			case 3:		nEol = EolType::LF; break;
			case 4:		nEol = EolType::CR; break;
			case 5:		nEol = EolType::NEL; break;
			case 6:		nEol = EolType::LS; break;
			case 7:		nEol = EolType::PS; break;
			default:	nEol = EolType::None; break;
			}
			if (nEol != EolType::None) {
				pcEditView->GetCommander().HandleCommand(Index, true, (int)nEol, 0, 0, 0);
			}
		}
		break;
	case F_SET_QUOTESTRING:	// Jan. 29, 2005 genta 追加 テキスト引数1つを取るマクロはここに統合していこう．
		{
			if (!Argument[0]) {
				::MYMESSAGEBOX(
					NULL,
					MB_OK | MB_ICONSTOP | MB_TOPMOST,
					EXEC_ERROR_TITLE,
					LS(STR_ERR_DLGMACRO04)
				);
				return false;
			} {
				pcEditView->GetCommander().HandleCommand(Index, true, (LPARAM)Argument[0], 0, 0, 0);	// 標準
			}
		}
		break;
	case F_INSTEXT_W:		// テキスト挿入
	case F_ADDTAIL_W:		// この操作はキーボード操作では存在しないので保存することができない？
	case F_INSBOXTEXT:
		// 一つ目の引数が文字列。
		if (!Argument[0]) {
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO04)
			);
			return false;
		}
		{
			int len = ArgLengths[0];
			pcEditView->GetCommander().HandleCommand(Index, true, (LPARAM)Argument[0], len, 0, 0);	// 標準
		}
		break;
	// 一つ目、二つ目とも引数は数値
	case F_CHG_CHARSET:
		{
			int		nCharSet = (!Argument[0] || Argument[0][0] == '\0') ? CODE_NONE : _wtoi(Argument[0]);
			BOOL	bBOM = (!Argument[1]) ? FALSE : (_wtoi(Argument[1]) != 0);
			pcEditView->GetCommander().HandleCommand(Index, true, (LPARAM)nCharSet, (LPARAM)bBOM, 0, 0);
		}
		break;
	case F_JUMP:		// 指定行へジャンプ（ただしPL/SQLコンパイルエラー行へのジャンプは未対応）
		// Argument[0]へジャンプ。オプションはArgument[1]に。
		//		******** 以下「行番号の単位」 ********
		//		0x00	折り返し単位の行番号
		//		0x01	改行単位の行番号
		//		**************************************
		//		0x02	PL/SQLコンパイルエラー行を処理する
		//		未定義	テキストの□行目をブロックの1行目とする
		//		未定義	検出されたPL/SQLパッケージのブロックから選択
		if (!Argument[0]) {
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO05)
			);
			return false;
		}
		{
			pcEditView->m_pEditWnd->m_dlgJump.m_nLineNum = _wtoi(Argument[0]);	// ジャンプ先
			LPARAM lFlag = Argument[1] ? _wtoi(Argument[1]) : 1; // デフォルト1
			GetDllShareData().m_bLineNumIsCRLF_ForJump = ((lFlag & 0x01) != 0);
			pcEditView->m_pEditWnd->m_dlgJump.m_bPLSQL = lFlag & 0x02 ? 1 : 0;
			pcEditView->GetCommander().HandleCommand(Index, true, 0, 0, 0, 0);	// 標準
		}
		break;
	// 一つ目の引数は文字列、二つ目の引数は数値
	case F_BOOKMARK_PATTERN:	// 2002.02.08 hor
		if (!Argument[0]) {
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO06)
			);
			return false;
		}
		// NO BREAK
	case F_SEARCH_NEXT:
	case F_SEARCH_PREV:
		//	Argument[0] を検索。(省略時、元の検索文字列・オプションを使う)
		//	Argument[1]:オプション (省略時、0のみなす)
		//		0x01	単語単位で探す
		//		0x02	英大文字と小文字を区別する
		//		0x04	正規表現
		//		0x08	見つからないときにメッセージを表示
		//		0x10	検索ダイアログを自動的に閉じる
		//		0x20	先頭（末尾）から再検索する
		//		0x800	(マクロ専用)検索キーを履歴に登録しない
		//		0x1000	(マクロ専用)検索オプションを元に戻す
		{
			LPARAM lFlag = Argument[1] ? _wtoi(Argument[1]) : 0;
			SearchOption searchOption;
			searchOption.bWordOnly			= ((lFlag & 0x01) != 0);
			searchOption.bLoHiCase			= ((lFlag & 0x02) != 0);
			searchOption.bRegularExp		= ((lFlag & 0x04) != 0);
			bool bAddHistory = ((lFlag & 0x800) == 0);
			bool bBackupFlag = ((lFlag & 0x1000) != 0);
			CommonSetting_Search backupFlags;
			SearchOption backupLocalFlags;
			std::wstring backupStr;
			bool backupKeyMark;
			int nBackupSearchKeySequence;
			if (bBackupFlag) {
				backupFlags = GetDllShareData().m_common.m_search;
				backupLocalFlags = pcEditView->m_curSearchOption;
				backupStr = pcEditView->m_strCurSearchKey;
				backupKeyMark = pcEditView->m_bCurSrchKeyMark;
				nBackupSearchKeySequence = pcEditView->m_nCurSearchKeySequence;
				bAddHistory = false;
			}
			const WCHAR* pszSearchKey = wtow_def(Argument[0], L"");
			int nLen = wcslen( pszSearchKey );
			if (0 < nLen) {
				// 正規表現
				if (lFlag & 0x04
					&& !CheckRegexpSyntax(Argument[0], NULL, true)
				) {
					break;
				}

				// 検索文字列
				if (nLen < _MAX_PATH && bAddHistory) {
					SearchKeywordManager().AddToSearchKeyArr(Argument[0]);
					GetDllShareData().m_common.m_search.m_searchOption = searchOption;
				}
				pcEditView->m_strCurSearchKey = Argument[0];
				pcEditView->m_curSearchOption = searchOption;
				pcEditView->m_bCurSearchUpdate = true;
				pcEditView->m_nCurSearchKeySequence = GetDllShareData().m_common.m_search.m_nSearchKeySequence;
			}
			// 設定値バックアップ
			// マクロパラメータ→設定値変換
			GetDllShareData().m_common.m_search.m_bNOTIFYNOTFOUND	= lFlag & 0x08 ? 1 : 0;
			GetDllShareData().m_common.m_search.m_bAutoCloseDlgFind	= lFlag & 0x10 ? 1 : 0;
			GetDllShareData().m_common.m_search.m_bSearchAll			= lFlag & 0x20 ? 1 : 0;

			// コマンド発行
			pcEditView->GetCommander().HandleCommand(Index, true, 0, 0, 0, 0);
			if (bBackupFlag) {
				GetDllShareData().m_common.m_search = backupFlags;
				pcEditView->m_curSearchOption = backupLocalFlags;
				pcEditView->m_strCurSearchKey = backupStr;
				pcEditView->m_bCurSearchUpdate = true;
				pcEditView->m_nCurSearchKeySequence = GetDllShareData().m_common.m_search.m_nSearchKeySequence;
				pcEditView->ChangeCurRegexp( backupKeyMark );
				pcEditView->m_bCurSrchKeyMark = backupKeyMark;
				if (!backupKeyMark) {
					pcEditView->Redraw();
				}
				pcEditView->m_nCurSearchKeySequence = nBackupSearchKeySequence;
			}
		}
		break;
	case F_DIFF:
		// Argument[0]とDiff差分表示。オプションはArgument[1]に。
		// Argument[1]:
		//		次の数値の和。
		//		0x0001 -i ignore-case         大文字小文字同一視
		//		0x0002 -w ignore-all-space    空白無視
		//		0x0004 -b ignore-space-change 空白変更無視
		//		0x0008 -B ignore-blank-lines  空行無視
		//		0x0010 -t expand-tabs         TAB-SPACE変換
		//		0x0020    (編集中のファイルが旧ファイル)
		//		0x0040    (DIFF差分がないときにメッセージ表示)
		// NO BREAK

	case F_EXECMD:
		// Argument[0]を実行。オプションはArgument[1]に。
		// Argument[1]:
		//		次の数値の和。
		//		0x01	標準出力を得る
		//		0x02	標準出力をキャレット位置に	// 2007.01.02 maru 引数の拡張
		//		0x04	編集中ファイルを標準入力へ	// 2007.01.02 maru 引数の拡張
		// Argument[2]:カレントディレクトリ
		if (!Argument[0]) {
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO07)
			);
			return false;
		}
		{
			int nOpt = wtoi_def(Argument[1], 0);
			const wchar_t* pDir = wtow_def(Argument[2], NULL);
			pcEditView->GetCommander().HandleCommand(Index, true, (LPARAM)Argument[0], nOpt, (LPARAM)pDir, 0);
		}
		break;

	case F_TRACEOUT:		// 2006.05.01 マクロ用アウトプットウィンドウに出力
		// Argument[0]を出力。オプションはArgument[1]に。
		// Argument[1]:
		//		次の数値の和。
		//		0x01	ExpandParameterによる文字列展開を行う
		//		0x02	テキスト末尾に改行コードを付加しない
		if (!Argument[0]) {
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO07)
			);
			return false;
		}
		{
			pcEditView->GetCommander().HandleCommand(Index, true, (LPARAM)Argument[0], ArgLengths[0], (LPARAM)(Argument[1] ? _wtoi(Argument[1]) : 0), 0);
		}
		break;

	// はじめの引数は文字列。２つ目と３つ目は数値
	case F_PUTFILE:		// 2006.12.10 作業中ファイルの一時出力
		// Argument[0]に出力。Argument[1]に文字コード。オプションはArgument[2]に。
		// Argument[2]:
		//		次の値の和
		//		0x01	選択範囲を出力（非選択状態なら空ファイルを生成）
		// no break

	case F_INSFILE:		// 2006.12.10 キャレット位置にファイル挿入
		// Argument[0]に出力。Argument[1]に文字コード。オプションはArgument[2]に。
		// Argument[2]:
		//		現在は特になし
		if (!Argument[0]) {
			::MYMESSAGEBOX(NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO08));
			return false;
		}
		{
			pcEditView->GetCommander().HandleCommand(
				Index,
				true,
				(LPARAM)Argument[0], 
				(LPARAM)(Argument[1] ? _wtoi(Argument[1]) : 0),
				(LPARAM)(Argument[2] ? _wtoi(Argument[2]) : 0),
				0
			);
		}
		break;

	// はじめの2つの引数は文字列。3つ目は数値
	case F_REPLACE:
	case F_REPLACE_ALL:
		// Argument[0]を、Argument[1]に置換。オプションはArgument[2]に（入れる予定）
		// Argument[2]:
		//		次の数値の和。
		//		0x001	単語単位で探す
		//		0x002	英大文字と小文字を区別する
		//		0x004	正規表現
		//		0x008	見つからないときにメッセージを表示
		//		0x010	検索ダイアログを自動的に閉じる
		//		0x020	先頭（末尾）から再検索する
		//		0x040	クリップボードから貼り付ける
		//		******** 以下「置換範囲」 ********
		//		0x000	ファイル全体
		//		0x080	選択範囲
		//		**********************************
		//		******** 以下「置換対象」 ********
		//		0x000	見つかった文字列と置換
		//		0x100	見つかった文字列の前に挿入
		//		0x200	見つかった文字列の後に追加
		//		**********************************
		//		0x400	「すべて置換」は置換の繰返し（ON:連続置換, OFF:一括置換）
		//		0x800	(マクロ専用)検索キーを履歴に登録しない
		//		0x1000	(マクロ専用)検索オプションを元に戻す
		if (Argument[0] == NULL || Argument[0][0] == L'\0') {
			::MYMESSAGEBOX(NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO09));
			return false;
		}
		if (!Argument[1]) {
			::MYMESSAGEBOX(NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO10));
			return false;
		}
		{
			DlgReplace& cDlgReplace = pcEditView->m_pEditWnd->m_dlgReplace;
			LPARAM lFlag = Argument[2] ? _wtoi(Argument[2]) : 0;
			SearchOption searchOption;
			searchOption.bWordOnly			= ((lFlag & 0x01) != 0);
			searchOption.bLoHiCase			= ((lFlag & 0x02) != 0);
			searchOption.bRegularExp		= ((lFlag & 0x04) != 0);
			bool bAddHistory = ((lFlag & 0x800) == 0);
			bool bBackupFlag = ((lFlag & 0x1000) != 0);
			CommonSetting_Search backupFlags;
			SearchOption backupLocalFlags;
			std::wstring backupStr;
			std::wstring backupStrRep;
			int nBackupSearchKeySequence;
			bool backupKeyMark;
			if (bBackupFlag) {
				backupFlags = GetDllShareData().m_common.m_search;
				backupLocalFlags = pcEditView->m_curSearchOption;
				backupStr = pcEditView->m_strCurSearchKey;
				backupStrRep = cDlgReplace.m_strText2;
				backupKeyMark = pcEditView->m_bCurSrchKeyMark;
				nBackupSearchKeySequence = pcEditView->m_nCurSearchKeySequence;
				bAddHistory = false;
			}
			// 正規表現
			if (lFlag & 0x04
				&& !CheckRegexpSyntax(Argument[0], NULL, true)
			) {
				break;
			}

			// 検索文字列
			if (wcslen(Argument[0]) < _MAX_PATH && bAddHistory) {
				SearchKeywordManager().AddToSearchKeyArr(Argument[0]);
				GetDllShareData().m_common.m_search.m_searchOption = searchOption;
			}
			pcEditView->m_strCurSearchKey = Argument[0];
			pcEditView->m_curSearchOption = searchOption;
			pcEditView->m_bCurSearchUpdate = true;
			pcEditView->m_nCurSearchKeySequence = GetDllShareData().m_common.m_search.m_nSearchKeySequence;

			// 置換後文字列
			if (wcslen(Argument[1]) < _MAX_PATH && bAddHistory) {
				SearchKeywordManager().AddToReplaceKeyArr(Argument[1]);
			}
			cDlgReplace.m_strText2 = Argument[1];

			GetDllShareData().m_common.m_search.m_bNOTIFYNOTFOUND	= lFlag & 0x08 ? 1 : 0;
			GetDllShareData().m_common.m_search.m_bAutoCloseDlgFind	= lFlag & 0x10 ? 1 : 0;
			GetDllShareData().m_common.m_search.m_bSearchAll			= lFlag & 0x20 ? 1 : 0;
			cDlgReplace.m_bPaste			= lFlag & 0x40 ? 1 : 0;	// CShareDataに入れなくていいの？
			cDlgReplace.m_bConsecutiveAll = lFlag & 0x0400 ? 1 : 0;	// 2007.01.16 ryoji
			if (LOWORD(Index) == F_REPLACE) {	// 2007.07.08 genta コマンドは下位ワード
				// 置換する時は選べない
				cDlgReplace.m_bSelectedArea = 0;
			}else if (LOWORD(Index) == F_REPLACE_ALL) {	// 2007.07.08 genta コマンドは下位ワード
				// 全置換の時は選べる？
				cDlgReplace.m_bSelectedArea	= lFlag & 0x80 ? 1 : 0;
			}
			cDlgReplace.m_nReplaceTarget	= (lFlag >> 8) & 0x03;	// 8bitシフト（0x100で割り算）	// 2007.01.16 ryoji 下位 2bitだけ取り出す
			if (bAddHistory) {
				GetDllShareData().m_common.m_search.m_bConsecutiveAll = cDlgReplace.m_bConsecutiveAll;
				GetDllShareData().m_common.m_search.m_bSelectedArea = cDlgReplace.m_bSelectedArea;
			}
			// コマンド発行
			pcEditView->GetCommander().HandleCommand(Index, true, 0, 0, 0, 0);
			if (bBackupFlag) {
				GetDllShareData().m_common.m_search = backupFlags;
				pcEditView->m_curSearchOption = backupLocalFlags;
				pcEditView->m_strCurSearchKey = backupStr;
				pcEditView->m_bCurSearchUpdate = true;
				cDlgReplace.m_strText2 = backupStrRep;
				pcEditView->m_nCurSearchKeySequence = GetDllShareData().m_common.m_search.m_nSearchKeySequence;
				pcEditView->ChangeCurRegexp( backupKeyMark );
				pcEditView->m_bCurSrchKeyMark = backupKeyMark;
				if (!backupKeyMark) {
					pcEditView->Redraw();
				}
				pcEditView->m_nCurSearchKeySequence = nBackupSearchKeySequence;
			}
		}
		break;
	case F_GREP_REPLACE:
	case F_GREP:
		// Argument[0]	検索文字列
		// Argument[1]	検索対象にするファイル名
		// Argument[2]	検索対象にするフォルダ名
		// Argument[3]:
		//		次の数値の和。
		//		0x01	サブフォルダからも検索する
		//		0x02	この編集中のテキストから検索する（未実装）
		//		0x04	英大文字と英小文字を区別する
		//		0x08	正規表現
		//		0x10	文字コード自動判別
		//		******** 以下「結果出力」 ********
		//		0x00	該当行
		//		0x20	該当部分
		//		0x400000	否ヒット行	// 2014.09.23
		//		0x400020	(未使用)	// 2014.09.23
		//		**********************************
		//		******** 以下「出力形式」 ********
		//		0x00	ノーマル
		//		0x40	ファイル毎
		//		0x80	結果のみ // 2011.11.24
		//		0xC0	(未使用) // 2011.11.24
		//		**********************************
		//		0x0100 〜 0xff00	文字コードセット番号 * 0x100
		//		0x010000	単語単位で探す
		//		0x020000	ファイル毎最初のみ検索
		//		0x040000	ベースフォルダ表示
		//		0x080000	フォルダ毎に表示
		{
			if (Argument[0] == NULL) {
				::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
					LS(STR_ERR_DLGMACRO11));
				return false;
			}
			int ArgIndex = 0;
			bool bGrepReplace = LOWORD(Index) == F_GREP_REPLACE;
			if (bGrepReplace) {
				if (ArgLengths[0] == 0) {
					::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
						LS(STR_ERR_DLGMACRO11));
					return false;
				}
				ArgIndex = 1;
				if (Argument[1] == NULL) {
					::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
						LS(STR_ERR_DLGMACRO17));
					return false;
				}
			}
			if (Argument[ArgIndex+1] == NULL) {
				::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
					LS(STR_ERR_DLGMACRO12));
				return false;
			}
			if (Argument[ArgIndex+2] == NULL) {
				::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
					LS(STR_ERR_DLGMACRO13));
				return false;
			}
			//	常に外部ウィンドウに。
			// ======= Grepの実行 =============
			// Grep結果ウィンドウの表示
			NativeW cmWork1;	cmWork1.SetString( Argument[0] );	cmWork1.Replace( L"\"", L"\"\"" );	//	検索文字列
			NativeW cmWork4;
			if (bGrepReplace) {
				cmWork4.SetString( Argument[1] );	cmWork4.Replace( L"\"", L"\"\"" );	//	置換後
			}
			CNativeT cmWork2;	cmWork2.SetStringW( Argument[ArgIndex+1] );	cmWork2.Replace( _T("\""), _T("\"\"") );	//	ファイル名
			CNativeT cmWork3;	cmWork3.SetStringW( Argument[ArgIndex+2] );	cmWork3.Replace( _T("\""), _T("\"\"") );	//	フォルダ名

			LPARAM lFlag = wtoi_def(Argument[ArgIndex+3], 5);

			// 2002/09/21 Moca 文字コードセット
			ECodeType	nCharSet;
			{
				nCharSet = CODE_SJIS;
				if (lFlag & 0x10) {	// 文字コード自動判別(下位互換用)
					nCharSet = CODE_AUTODETECT;
				}
				int nCode = (lFlag >> 8) & 0xff; // 下から 7-15 ビット目(0開始)を使う
				if (IsValidCodeTypeExceptSJIS(nCode) || CODE_AUTODETECT == nCode) {
					nCharSet = (ECodeType)nCode;
				}
				// 2013.06.11 5番目の引き数を文字コードにする
				if (ArgIndex + 5 <= ArgSize) {
					nCharSet = (ECodeType)_wtoi(Argument[ArgIndex + 4]);
				}
			}

			// -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
			CNativeT cCmdLine;
			TCHAR	szTemp[20];
			TCHAR	pOpt[64];
			cCmdLine.AppendString(_T("-GREPMODE -GKEY=\""));
			cCmdLine.AppendStringW(cmWork1.GetStringPtr());
			if (bGrepReplace) {
				cCmdLine.AppendString(_T("\" -GREPR=\""));
				cCmdLine.AppendStringW(cmWork4.GetStringPtr());
			}
			cCmdLine.AppendString(_T("\" -GFILE=\""));
			cCmdLine.AppendString(cmWork2.GetStringPtr());
			cCmdLine.AppendString(_T("\" -GFOLDER=\""));
			cCmdLine.AppendString(cmWork3.GetStringPtr());
			cCmdLine.AppendString(_T("\" -GCODE="));
			auto_sprintf( szTemp, _T("%d"), nCharSet );
			cCmdLine.AppendString(szTemp);

			// GOPTオプション
			pOpt[0] = '\0';
			if (lFlag & 0x01) _tcscat( pOpt, _T("S") );	// サブフォルダからも検索する
			if (lFlag & 0x04) _tcscat( pOpt, _T("L") );	// 英大文字と英小文字を区別する
			if (lFlag & 0x08) _tcscat( pOpt, _T("R") );	// 正規表現
			if ((lFlag & 0x400020) == 0x20) _tcscat( pOpt, _T("P") );			// 行を出力する
			else if ((lFlag & 0x400020) == 0x400000) _tcscat( pOpt, _T("N") );	// 否ヒット行を出力する
			if ((lFlag & 0xC0) == 0x40) _tcscat( pOpt, _T("2") );				// Grep: 出力形式
			else if ((lFlag & 0xC0) == 0x80) _tcscat( pOpt, _T("3") );
			else _tcscat( pOpt, _T("1") );
			if (lFlag & 0x10000) _tcscat( pOpt, _T("W") );
			if (lFlag & 0x20000) _tcscat( pOpt, _T("F") );
			if (lFlag & 0x40000) _tcscat( pOpt, _T("B") );
			if (lFlag & 0x80000) _tcscat( pOpt, _T("D") );
			if (bGrepReplace) {
				if (lFlag & 0x100000) _tcscat( pOpt, _T("C") );
				if (lFlag & 0x200000) _tcscat( pOpt, _T("O") );
			}
			if (pOpt[0] != _T('\0')) {
				auto_sprintf( szTemp, _T(" -GOPT=%ts"), pOpt );
				cCmdLine.AppendString(szTemp);
			}

			// 新規編集ウィンドウの追加 ver 0
			LoadInfo sLoadInfo;
			sLoadInfo.filePath = _T("");
			sLoadInfo.eCharCode = CODE_NONE;
			sLoadInfo.bViewMode = false;
			ControlTray::OpenNewEditor(
				G_AppInstance(),
				pcEditView->GetHwnd(),
				sLoadInfo,
				cCmdLine.GetStringPtr()
			);
			// ======= Grepの実行 =============
			// Grep結果ウィンドウの表示
		}
		break;
	case F_FILEOPEN2:
		// Argument[0]を開く。
		if (!Argument[0]) {
			::MYMESSAGEBOX(NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO14));
			return false;
		}
		{
			int  nCharCode = wtoi_def(Argument[1], CODE_AUTODETECT);
			BOOL nViewMode = wtoi_def(Argument[2], FALSE);
			const WCHAR* pDefFileName = wtow_def(Argument[3], L"");
			pcEditView->GetCommander().HandleCommand(Index, true, (LPARAM)Argument[0], (LPARAM)nCharCode, (LPARAM)nViewMode, (LPARAM)pDefFileName);
		}
		break;
	case F_FILESAVEAS_DIALOG:
	case F_FILESAVEAS:
		// Argument[0]を別名で保存。
		if (LOWORD(Index) == F_FILESAVEAS && (!Argument[0] ||  L'\0' == Argument[0][0])) {
			// F_FILESAVEAS_DIALOGの場合は空文字列を許容
			::MYMESSAGEBOX(NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO15));
			return false;
		}
		{
			// 文字コードセット
			// Sep. 11, 2004 genta 文字コード設定の範囲チェック
			ECodeType nCharCode = CODE_NONE;	// デフォルト値
			if (Argument[1]) {
				nCharCode = (ECodeType)_wtoi(Argument[1]);
			}
			if (LOWORD(Index) == F_FILESAVEAS && IsValidCodeOrCPType(nCharCode) && nCharCode != pcEditView->m_pEditDoc->GetDocumentEncoding()) {
				// From Here Jul. 26, 2003 ryoji BOM状態を初期化
				pcEditView->m_pEditDoc->SetDocumentEncoding(nCharCode, CodeTypeName(pcEditView->m_pEditDoc->GetDocumentEncoding()).IsBomDefOn());
				// To Here Jul. 26, 2003 ryoji BOM状態を初期化
			}

			// 改行コード
			int nSaveLineCode = 0;	// デフォルト値	// Sep. 11, 2004 genta 初期値を「変更しない」に
			if (Argument[2]) {
				nSaveLineCode = _wtoi(Argument[2]);
			}
			EolType eEol;
			switch (nSaveLineCode) {
			case 0:		eEol = EolType::None;	break;
			case 1:		eEol = EolType::CRLF;	break;
			case 2:		eEol = EolType::LF;		break;
			case 3:		eEol = EolType::CR;		break;
			default:	eEol = EolType::None;	break;
			}
			
			pcEditView->GetCommander().HandleCommand(Index, true, (LPARAM)Argument[0], (LPARAM)nCharCode, (LPARAM)eEol, 0);
		}
		break;
	// 2つの引数が文字列
	// Jul. 5, 2002 genta
	case F_EXTHTMLHELP:
	case F_EXECEXTMACRO:				// 2009.06.14 syat
		pcEditView->GetCommander().HandleCommand(Index, true, (LPARAM)Argument[0], (LPARAM)Argument[1], 0, 0);
		break;
	// From Here Dec. 4, 2002 genta
	case F_FILE_REOPEN				: // 開き直す
	case F_FILE_REOPEN_SJIS			: // SJISで開き直す
	case F_FILE_REOPEN_JIS			: // JISで開き直す
	case F_FILE_REOPEN_EUC			: // EUCで開き直す
	case F_FILE_REOPEN_UNICODE		: // Unicodeで開き直す
	case F_FILE_REOPEN_UNICODEBE	: // UnicodeBEで開き直す
	case F_FILE_REOPEN_UTF8			: // UTF-8で開き直す
	case F_FILE_REOPEN_UTF7			: // UTF-7で開き直す
		{
			int noconfirm = 0;
			if (Argument[0]) {
				noconfirm = (_wtoi(Argument[0]) != 0);
			}
			pcEditView->GetCommander().HandleCommand(Index, true, noconfirm, 0, 0, 0);
		}
		break;
	// To Here Dec. 4, 2002 genta
	case F_TOPMOST:
		{
			int lparam1;
			if (Argument[0]) {
				lparam1 = _wtoi(Argument[0]);
				pcEditView->GetCommander().HandleCommand(Index, true, lparam1, 0, 0, 0);
			}
		}
		break;	// Jan. 29, 2005 genta 抜けていた
	case F_TAGJUMP_KEYWORD:	// @@ 2005.03.31 MIK
		{
			// 引数はNULLでもOK
			pcEditView->GetCommander().HandleCommand(Index, true, (LPARAM)Argument[0], 0, 0, 0);
		}
		break;
	case F_NEXTWINDOW:
	case F_PREVWINDOW:
		pcEditView->GetDocument()->HandleCommand(Index);	// 2009.04.11 ryoji F_NEXTWINDOW/F_PREVWINDOWが動作しなかったのを修正
		break;
	case F_MESSAGEBOX:	// メッセージボックスの表示
	case F_ERRORMSG:	// メッセージボックス（エラー）の表示
	case F_WARNMSG:		// メッセージボックス（警告）の表示
	case F_INFOMSG:		// メッセージボックス（情報）の表示
	case F_OKCANCELBOX:	// メッセージボックス（確認：OK／キャンセル）の表示
	case F_YESNOBOX:	// メッセージボックス（確認：はい／いいえ）の表示
		{
			VARIANT vArg[2];			// HandleFunctionに渡す引数
			VARIANT vResult;			// HandleFunctionから返る値
			if (!Argument[0]) {
				break;
			}
			SysString S(Argument[0], wcslen(Argument[0]));
			Wrap(&vArg[0])->Receive(S);
			int nArgSize = 1;
			// 2つ目の引数が数値。
			if (F_MESSAGEBOX == LOWORD(Index)) {
				vArg[1].vt = VT_I4;
				vArg[1].intVal = (Argument[1] ? _wtoi(Argument[1]) : 0);
				nArgSize = 2;
			}
			return HandleFunction(pcEditView, Index, vArg, nArgSize, vResult);
		}
	case F_MOVECURSORLAYOUT:
	case F_MOVECURSOR:
		{
			if (Argument[0] && Argument[1] && Argument[2]) {
				int lparam1 = _wtoi(Argument[0]) - 1;
				int lparam2 = _wtoi(Argument[1]) - 1;
				int lparam3 = _wtoi(Argument[2]);
				pcEditView->GetCommander().HandleCommand(Index, true, lparam1, lparam2, lparam3, 0);
			}else {
				::MYMESSAGEBOX(NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO16));
				return false;
			}
		}
		break;
	case F_CHGTABWIDTH:		//  タブサイズを取得、設定する（キーマクロでは取得は無意味）
	case F_CHGWRAPCOLUMN:	//  折り返し桁を取得、設定する（キーマクロでは取得は無意味）
	case F_MACROSLEEP:
	case F_SETDRAWSWITCH:	//  再描画スイッチを取得、設定する
		{
			VARIANT vArg[1];			// HandleFunctionに渡す引数
			VARIANT vResult;			// HandleFunctionから返る値
			// 一つ目の引数が数値。
			vArg[0].vt = VT_I4;
			vArg[0].intVal = (Argument[0] ? _wtoi(Argument[0]) : 0);
			return HandleFunction(pcEditView, Index, vArg, 1, vResult);
		}
	case F_SETFONTSIZE:
		{
			int val0 = Argument[0] ? _wtoi(Argument[0]) : 0;
			int val1 = Argument[1] ? _wtoi(Argument[1]) : 0;
			int val2 = Argument[2] ? _wtoi(Argument[2]) : 0;
			pcEditView->GetCommander().HandleCommand(Index, true, (LPARAM)val0, (LPARAM)val1, (LPARAM)val2, 0);
		}
		break;
	case F_STATUSMSG:
		{
			if (!Argument[0]) {
				::MYMESSAGEBOX(NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
					LS(STR_ERR_DLGMACRO07));
				return false;
			}
			std::tstring val0 = to_tchar(Argument[0]);
			int val1 = Argument[1] ? _wtoi(Argument[1]) : 0;
			if ((val1 & 0x03) == 0) {
				pcEditView->SendStatusMessage(val0.c_str());
			}else if ((val1 & 0x03) == 1) {
				if (pcEditView->m_pEditWnd->m_statusBar.GetStatusHwnd()) {
					pcEditView->SendStatusMessage(val0.c_str());
				}else {
					InfoMessage(pcEditView->GetHwnd(), _T("%ts"), val0.c_str());
				}
			}else if ((val1 & 0x03) == 2) {
				pcEditView->m_pEditWnd->m_statusBar.SendStatusMessage2(val0.c_str());
			}
		}
		break;
	case F_MSGBEEP:
		{
			int val0 = Argument[0] ? _wtoi(Argument[0]) : 0;
			switch (val0) {
			case -1: break;
			case 0: val0 = MB_OK; break;
			case 1: val0 = MB_ICONERROR; break;
			case 2: val0 = MB_ICONQUESTION; break;
			case 3: val0 = MB_ICONWARNING; break;
			case 4: val0 = MB_ICONINFORMATION; break;
			default: val0 = MB_OK; break;
			}
			::MessageBeep(val0);
		}
		break;
	case F_COMMITUNDOBUFFER:
		{
			OpeBlk* opeBlk = pcEditView->m_commander.GetOpeBlk();
			if (opeBlk) {
				int nCount = opeBlk->GetRefCount();
				opeBlk->SetRefCount(1); // 強制的にリセットするため1を指定
				pcEditView->SetUndoBuffer();
				if (!pcEditView->m_commander.GetOpeBlk() && 0 < nCount) {
					pcEditView->m_commander.SetOpeBlk(new OpeBlk());
					pcEditView->m_commander.GetOpeBlk()->SetRefCount(nCount);
				}
			}
		}
		break;
	case F_ADDREFUNDOBUFFER:
		{
			OpeBlk* opeBlk = pcEditView->m_commander.GetOpeBlk();
			if (!opeBlk) {
				pcEditView->m_commander.SetOpeBlk(new OpeBlk());
			}
			pcEditView->m_commander.GetOpeBlk()->AddRef();
		}
		break;
	case F_SETUNDOBUFFER:
		{
			pcEditView->SetUndoBuffer();
		}
		break;
	case F_APPENDUNDOBUFFERCURSOR:
		{
			OpeBlk* opeBlk = pcEditView->m_commander.GetOpeBlk();
			if (!opeBlk) {
				pcEditView->m_commander.SetOpeBlk(new OpeBlk());
			}
			opeBlk = pcEditView->m_commander.GetOpeBlk();
			opeBlk->AddRef();
			opeBlk->AppendOpe(
				new MoveCaretOpe(
					pcEditView->GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
				)
			);
			pcEditView->SetUndoBuffer();
		}
		break;
	case F_CLIPBOARDEMPTY:
		{
			Clipboard cClipboard(pcEditView->GetHwnd());
			cClipboard.Empty();
		}
		break;
	case F_SETVIEWTOP:
		{
			if (ArgSize <= 0) {
				return false;
			}
			if (ArgLengths[0] <= 0) {
				return false;
			}
			if (!WCODE::Is09( Argument[0][0] )) {
				return false;
			}
			LayoutYInt nLineNum = LayoutYInt(_wtoi(Argument[0]) - 1);
			if (nLineNum < 0) {
				nLineNum = LayoutYInt(0);
			}
			pcEditView->SyncScrollV( pcEditView->ScrollAtV( nLineNum ));
		}
		break;
	case F_SETVIEWLEFT:
		{
			if (ArgSize <= 0) {
				return false;
			}
			if (ArgLengths[0] <= 0) {
				return false;
			}
			if (!WCODE::Is09( Argument[0][0] )) {
				return false;
			}
			LayoutXInt nColumn = LayoutXInt(_wtoi(Argument[0]) - 1);
			if (nColumn < 0) {
				nColumn = LayoutXInt(0);
			}
			pcEditView->SyncScrollH( pcEditView->ScrollAtH( nColumn ) );
		}
		break;
	default:
		// 引数なし。
		pcEditView->GetCommander().HandleCommand(Index, true, 0, 0, 0, 0);	// 標準
		break;
	}
	return true;
}


inline bool VariantToBStr(Variant& varCopy, const VARIANT& arg)
{
	return VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(arg)), 0, VT_BSTR) == S_OK;
}

inline bool VariantToI4(Variant& varCopy, const VARIANT& arg)
{
	return VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(arg)), 0, VT_I4) == S_OK;
}

/**	値を返す関数を処理する

	@param View      [in] 対象となるView
	@param ID        [in] 下位16bit: 関数番号
	@param Arguments [in] 引数の配列
	@param ArgSize   [in] 引数の数(Argument)
	@param Result  [out] 結果の値を返す場所。戻り値がfalseのときは不定。
	
	@return true: 成功, false: 失敗

	@author 鬼
	@date 2003.02.21 鬼
	@date 2003.06.01 Moca 関数追加
	@date 2005.08.05 maru,zenryaku 関数追加
	@date 2005.11.29 FILE VariantChangeType対応
*/
bool Macro::HandleFunction(
	EditView* view,
	EFunctionCode id,
	const VARIANT* args,
	int numArgs,
	VARIANT& result
	)
{
	Variant varCopy;	// VT_BYREFだと困るのでコピー用

	// 2003-02-21 鬼
	switch (LOWORD(id)) {
	case F_GETFILENAME:
		{
			const TCHAR* FileName = view->m_pEditDoc->m_docFile.GetFilePath();
			SysString S(FileName, _tcslen(FileName));
			Wrap(&result)->Receive(S);
		}
		return true;
	case F_GETSAVEFILENAME:
		// 2006.09.04 ryoji 保存時のファイルのパス
		{
			const TCHAR* FileName = view->m_pEditDoc->m_docFile.GetSaveFilePath();
			SysString S(FileName, lstrlen(FileName));
			Wrap(&result)->Receive(S);
		}
		return true;
	case F_GETSELECTED:
		{
			if (view->GetSelectionInfo().IsTextSelected()) {
				NativeW cMem;
				if (!view->GetSelectedDataSimple(cMem)) return false;
				SysString S(cMem.GetStringPtr(), cMem.GetStringLength());
				Wrap(&result)->Receive(S);
			}else {
				result.vt = VT_BSTR;
				result.bstrVal = SysAllocString(L"");
			}
		}
		return true;
	case F_EXPANDPARAMETER:
		// 2003.02.24 Moca
		{
			if (numArgs != 1) {
				return false;
			}
			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) {
				return false;	// VT_BSTRとして解釈
			}
			//void ExpandParameter(const char* pszSource, char* pszBuffer, int nBufferLen);
			// pszSourceを展開して、pszBufferにコピー
			wchar_t* source;
			int sourceLength;
			Wrap(&varCopy.data.bstrVal)->GetW(&source, &sourceLength);
			wchar_t buffer[2048];
			SakuraEnvironment::ExpandParameter(source, buffer, 2047);
			delete[] source;
			SysString s(buffer, wcslen(buffer));
			Wrap(&result)->Receive(s);
		}
		return true;
	case F_GETLINESTR:
		// 2003.06.01 Moca マクロ追加
		{
			if (numArgs != 1) return false;
			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_I4) != S_OK) return false;	// VT_I4として解釈
			if (-1 < varCopy.data.lVal) {
				const wchar_t* Buffer;
				LogicInt nLength;
				LogicInt nLine;
				if (varCopy.data.lVal == 0) {
					nLine = view->GetCaret().GetCaretLogicPos().GetY2();
				}else {
					nLine = LogicInt(varCopy.data.lVal - 1);
				}
				Buffer = view->m_pEditDoc->m_docLineMgr.GetLine(nLine)->GetDocLineStrWithEOL(&nLength);
				if (Buffer) {
					SysString S(Buffer, nLength);
					Wrap(&result)->Receive(S);
				}else {
					result.vt = VT_BSTR;
					result.bstrVal = SysAllocString(L"");
				}
			}else {
				return false;
			}
		}
		return true;
	case F_GETLINECOUNT:
		// 2003.06.01 Moca マクロ追加
		{
			if (numArgs != 1) return false;
			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_I4) != S_OK) return false;	// VT_I4として解釈
			if (varCopy.data.lVal == 0) {
				int nLineCount;
				nLineCount = view->m_pEditDoc->m_docLineMgr.GetLineCount();
				Wrap(&result)->Receive(nLineCount);
			}else {
				return false;
			}
		}
		return true;
	case F_CHGTABWIDTH:
		// 2004.03.16 zenryaku マクロ追加
		{
			if (numArgs != 1) return false;
			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_I4) != S_OK) return false;	// VT_I4として解釈
			int nTab = (Int)view->m_pEditDoc->m_layoutMgr.GetTabSpace();
			Wrap(&result)->Receive(nTab);
			// 2013.04.30 Moca 条件追加。不要な場合はChangeLayoutParamを呼ばない
			if (0 < varCopy.data.iVal && nTab != varCopy.data.iVal) {
				view->GetDocument()->m_bTabSpaceCurTemp = true;
				view->m_pEditWnd->ChangeLayoutParam(
					false, 
					LayoutInt(varCopy.data.iVal),
					view->m_pEditDoc->m_layoutMgr.GetMaxLineKetas()
				);

				// 2009.08.28 nasukoji	「折り返さない」選択時にTAB幅が変更されたらテキスト最大幅の再算出が必要
				if (view->m_pEditDoc->m_nTextWrapMethodCur == (int)TextWrappingMethod::NoWrapping) {
					// 最大幅の再算出時に各行のレイアウト長の計算も行う
					view->m_pEditDoc->m_layoutMgr.CalculateTextWidth();
				}
				view->m_pEditWnd->RedrawAllViews(NULL);		// TAB幅が変わったので再描画が必要
			}
		}
		return true;
	case F_ISTEXTSELECTED:
		// 2005.07.30 maru マクロ追加
		{
			if (view->GetSelectionInfo().IsTextSelected()) {
				if (view->GetSelectionInfo().IsBoxSelecting()) {
					Wrap(&result)->Receive(2);	// 矩形選択中
				}else {
					Wrap(&result)->Receive(1);	// 選択中
				}
			}else {
				Wrap(&result)->Receive(0);		// 非選択中
			}
		}
		return true;
	case F_GETSELLINEFROM:
		// 2005.07.30 maru マクロ追加
		{
			Wrap(&result)->Receive((Int)view->GetSelectionInfo().m_sSelect.GetFrom().y + 1);
		}
		return true;
	case F_GETSELCOLUMNFROM:
		// 2005.07.30 maru マクロ追加
		{
			Wrap(&result)->Receive((Int)view->GetSelectionInfo().m_sSelect.GetFrom().x + 1);
		}
		return true;
	case F_GETSELLINETO:
		// 2005.07.30 maru マクロ追加
		{
			Wrap(&result)->Receive((Int)view->GetSelectionInfo().m_sSelect.GetTo().y + 1);
		}
		return true;
	case F_GETSELCOLUMNTO:
		// 2005.07.30 maru マクロ追加
		{
			Wrap(&result)->Receive((Int)view->GetSelectionInfo().m_sSelect.GetTo().x + 1);
		}
		return true;
	case F_ISINSMODE:
		// 2005.07.30 maru マクロ追加
		{
			Wrap(&result)->Receive(view->IsInsMode() /* Oct. 2, 2005 genta */);
		}
		return true;
	case F_GETCHARCODE:
		// 2005.07.31 maru マクロ追加
		{
			Wrap(&result)->Receive(view->m_pEditDoc->GetDocumentEncoding());
		}
		return true;
	case F_GETLINECODE:
		// 2005.08.04 maru マクロ追加
		{
			int n = 0;
			switch (view->m_pEditDoc->m_docEditor.GetNewLineCode()) {
			case EolType::CRLF:
				n = 0;
				break;
			case EolType::CR:
				n = 1;
				break;
			case EolType::LF:
				n = 2;
				break;
			case EolType::NEL:
				n = 3;
				break;
			case EolType::LS:
				n = 4;
				break;
			case EolType::PS:
				n = 5;
				break;
			}
			Wrap(&result)->Receive(n);
		}
		return true;
	case F_ISPOSSIBLEUNDO:
		// 2005.08.04 maru マクロ追加
		{
			Wrap(&result)->Receive(view->m_pEditDoc->m_docEditor.IsEnableUndo());
		}
		return true;
	case F_ISPOSSIBLEREDO:
		// 2005.08.04 maru マクロ追加
		{
			Wrap(&result)->Receive(view->m_pEditDoc->m_docEditor.IsEnableRedo());
		}
		return true;
	case F_CHGWRAPCOLUMN:
		// 2008.06.19 ryoji マクロ追加
		{
			if (numArgs != 1) {
				return false;
			}
			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_I4) != S_OK) {
				return false;	// VT_I4として解釈
			}
			Wrap(&result)->Receive((Int)view->m_pEditDoc->m_layoutMgr.GetMaxLineKetas());
			if (varCopy.data.iVal < MINLINEKETAS || varCopy.data.iVal > MAXLINEKETAS) {
				return true;
			}
			view->m_pEditDoc->m_nTextWrapMethodCur = (int)TextWrappingMethod::SettingWidth;
			view->m_pEditDoc->m_bTextWrapMethodCurTemp = !(view->m_pEditDoc->m_nTextWrapMethodCur == view->m_pEditDoc->m_docType.GetDocumentAttribute().m_nTextWrapMethod);
			view->m_pEditWnd->ChangeLayoutParam(
				false, 
				view->m_pEditDoc->m_layoutMgr.GetTabSpace(),
				LayoutInt(varCopy.data.iVal)
			);
		}
		return true;
	case F_ISCURTYPEEXT:
		// 2006.09.04 ryoji 指定した拡張子が現在のタイプ別設定に含まれているかどうかを調べる
		{
			if (numArgs != 1) {
				return false;
			}

			TCHAR* source;
			int sourceLength;

			int nType1 = view->m_pEditDoc->m_docType.GetDocumentType().GetIndex();	// 現在のタイプ

			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) {
				return false;	// VT_BSTRとして解釈
			}
			Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
			int nType2 = DocTypeManager().GetDocumentTypeOfExt(source).GetIndex();	// 指定拡張子のタイプ
			delete[] source;

			Wrap(&result)->Receive((nType1 == nType2)? 1: 0);	// タイプ別設定の一致／不一致
		}
		return true;
	case F_ISSAMETYPEEXT:
		// 2006.09.04 ryoji ２つの拡張子が同じタイプ別設定に含まれているかどうかを調べる
		{
			if (numArgs != 2) {
				return false;
			}

			TCHAR* source;
			int sourceLength;

			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) {
				return false;	// VT_BSTRとして解釈
			}
			Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
			int nType1 = DocTypeManager().GetDocumentTypeOfExt(source).GetIndex();	// 拡張子１のタイプ
			delete[] source;

			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_BSTR) != S_OK) {
				return false;	// VT_BSTRとして解釈
			}
			Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
			int nType2 = DocTypeManager().GetDocumentTypeOfExt(source).GetIndex();	// 拡張子２のタイプ
			delete[] source;

			Wrap(&result)->Receive((nType1 == nType2)? 1: 0);	// タイプ別設定の一致／不一致
		}
		return true;
	case F_INPUTBOX:
		// 2011.03.18 syat テキスト入力ダイアログの表示
		{
			if (numArgs < 1) {
				return false;
			}
			TCHAR* source;
			int sourceLength;

			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) {
				return false;	// VT_BSTRとして解釈
			}
			Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
			std::tstring sMessage = source;	// 表示メッセージ
			delete[] source;

			std::tstring sDefaultValue = _T("");
			if (numArgs >= 2) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_BSTR) != S_OK) {
					return false;	// VT_BSTRとして解釈
				}
				Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
				sDefaultValue = source;	// デフォルト値
				delete[] source;
			}

			int nMaxLen = _MAX_PATH;
			if (numArgs >= 3) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[2])), 0, VT_I4) != S_OK) {
					return false;	// VT_I4として解釈
				}
				nMaxLen = varCopy.data.intVal;	// 最大入力長
				if (nMaxLen <= 0) {
					nMaxLen = _MAX_PATH;
				}
			}

			std::vector<TCHAR> buffer(nMaxLen + 1);
			TCHAR* pBuffer = &buffer[0];
			_tcscpy( pBuffer, sDefaultValue.c_str() );
			DlgInput1 cDlgInput1;
			if (cDlgInput1.DoModal(G_AppInstance(), view->GetHwnd(), _T("sakura macro"), sMessage.c_str(), nMaxLen, pBuffer)) {
				SysString S(pBuffer, _tcslen(pBuffer));
				Wrap(&result)->Receive(S);
			}else {
				result.vt = VT_BSTR;
				result.bstrVal = SysAllocString(L"");
			}
		}
		return true;
	case F_MESSAGEBOX:	// メッセージボックスの表示
	case F_ERRORMSG:	// メッセージボックス（エラー）の表示
	case F_WARNMSG:		// メッセージボックス（警告）の表示
	case F_INFOMSG:		// メッセージボックス（情報）の表示
	case F_OKCANCELBOX:	// メッセージボックス（確認：OK／キャンセル）の表示
	case F_YESNOBOX:	// メッセージボックス（確認：はい／いいえ）の表示
		// 2011.03.18 syat メッセージボックスの表示
		{
			if (numArgs < 1) {
				return false;
			}
			TCHAR* source;
			int sourceLength;

			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) {
				return false;	// VT_BSTRとして解釈
			}
			Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
			std::tstring sMessage = source;	// 表示文字列
			delete[] source;

			UINT uType = 0;		// メッセージボックス種別
			switch (LOWORD(id)) {
			case F_MESSAGEBOX:
				if (numArgs >= 2) {
					if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_I4) != S_OK) {
						return false;	// VT_I4として解釈
					}
					uType = varCopy.data.uintVal;
				}else {
					uType = MB_OK;
				}
				break;
			case F_ERRORMSG:
				uType |= MB_OK | MB_ICONSTOP;
				break;
			case F_WARNMSG:
				uType |= MB_OK | MB_ICONEXCLAMATION;
				break;
			case F_INFOMSG:
				uType |= MB_OK | MB_ICONINFORMATION;
				break;
			case F_OKCANCELBOX:
				uType |= MB_OKCANCEL | MB_ICONQUESTION;
				break;
			case F_YESNOBOX:
				uType |= MB_YESNO | MB_ICONQUESTION;
				break;
			}
			int ret = ::MessageBox(view->GetHwnd(), sMessage.c_str(), _T("sakura macro"), uType);
			Wrap(&result)->Receive(ret);
		}
		return true;
	case F_COMPAREVERSION:
		// 2011.03.18 syat バージョン番号の比較
		{
			if (numArgs != 2) {
				return false;
			}
			TCHAR* source;
			int sourceLength;

			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) {
				return false;	// VT_BSTRとして解釈
			}
			Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
			std::tstring sVerA = source;	// バージョンA
			delete[] source;

			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_BSTR) != S_OK) {
				return false;	// VT_BSTRとして解釈
			}
			Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
			std::tstring sVerB = source;	// バージョンB
			delete[] source;

			Wrap(&result)->Receive(CompareVersion(sVerA.c_str(), sVerB.c_str()));
		}
		return true;
	case F_MACROSLEEP:
		// 2011.03.18 syat 指定した時間（ミリ秒）停止する
		{
			if (numArgs != 1) {
				return false;
			}

			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_UI4) != S_OK) {
				return false;	// VT_UI4として解釈
			}
			WaitCursor cWaitCursor(view->GetHwnd());	// カーソルを砂時計にする
			::Sleep(varCopy.data.uintVal);
			Wrap(&result)->Receive(0);	// 戻り値は今のところ0固定
		}
		return true;
	case F_FILEOPENDIALOG:
	case F_FILESAVEDIALOG:
		// 2011.03.18 syat ファイルダイアログの表示
		{
			TCHAR* source;
			int sourceLength;
			std::tstring sDefault;
			std::tstring sFilter;

			if (numArgs >= 1) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) {
					return false;	// VT_BSTRとして解釈
				}
				Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
				sDefault = source;	// 既定のファイル名
				delete[] source;
			}

			if (numArgs >= 2) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_BSTR) != S_OK) return false;	// VT_BSTRとして解釈
				Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
				sFilter = source;	// フィルタ文字列
				delete[] source;
			}

			DlgOpenFile cDlgOpenFile;
			cDlgOpenFile.Create(
				G_AppInstance(), view->GetHwnd(),
				sFilter.c_str(),
				sDefault.c_str()
			);
			bool bRet;
			TCHAR szPath[_MAX_PATH];
			_tcscpy( szPath, sDefault.c_str() );
			if (LOWORD(id) == F_FILEOPENDIALOG) {
				bRet = cDlgOpenFile.DoModal_GetOpenFileName(szPath);
			}else {
				bRet = cDlgOpenFile.DoModal_GetSaveFileName(szPath);
			}
			if (bRet) {
				SysString S(szPath, _tcslen(szPath));
				Wrap(&result)->Receive(S);
			}else {
				result.vt = VT_BSTR;
				result.bstrVal = SysAllocString(L"");
			}
		}
		return true;
	case F_FOLDERDIALOG:
		// 2011.03.18 syat フォルダダイアログの表示
		{
			TCHAR* source;
			int sourceLength;
			std::tstring sMessage;
			std::tstring sDefault;

			if (numArgs >= 1) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) {
					return false;	// VT_BSTRとして解釈
				}
				Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
				sMessage = source;	// 表示メッセージ
				delete[] source;
			}

			if (numArgs >= 2) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_BSTR) != S_OK) {
					return false;	// VT_BSTRとして解釈
				}
				Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
				sDefault = source;	// 既定のファイル名
				delete[] source;
			}

			TCHAR szPath[_MAX_PATH];
			int nRet = SelectDir(view->GetHwnd(), sMessage.c_str(), sDefault.c_str(), szPath);
			if (nRet == IDOK) {
				SysString S(szPath, _tcslen(szPath));
				Wrap(&result)->Receive(S);
			}else {
				result.vt = VT_BSTR;
				result.bstrVal = SysAllocString(L"");
			}
		}
		return true;
	case F_GETCLIPBOARD:
		// 2011.03.18 syat クリップボードの文字列を取得
		{
			int nOpt = 0;

			if (numArgs >= 1) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_I4) != S_OK) {
					return false;	// VT_I4として解釈
				}
				nOpt = varCopy.data.intVal;	// オプション
			}

			NativeW memBuff;
			bool bColumnSelect = false;
			bool bLineSelect = false;
			bool bRet = view->MyGetClipboardData(memBuff, &bColumnSelect, &bLineSelect);
			if (bRet) {
				SysString S(memBuff.GetStringPtr(), memBuff.GetStringLength());
				Wrap(&result)->Receive(S);
			}else {
				result.vt = VT_BSTR;
				result.bstrVal = SysAllocString(L"");
			}
		}
		return true;
	case F_SETCLIPBOARD:
		// 2011.03.18 syat クリップボードに文字列を設定
		{
			std::tstring sValue;
			int nOpt = 0;

			if (numArgs >= 1) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_I4) != S_OK) {
					return false;	// VT_I4として解釈
				}
				nOpt = varCopy.data.intVal;	// オプション
			}

			if (numArgs >= 2) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_BSTR) != S_OK) {
					return false;	// VT_BSTRとして解釈
				}
				Wrap(&varCopy.data.bstrVal)->GetT(&sValue);
			}

			// 2013.06.12 オプション設定
			bool bColumnSelect = ((nOpt & 0x01) == 0x01);
			bool bLineSelect = ((nOpt & 0x02) == 0x02);
			bool bRet = view->MySetClipboardData(sValue.c_str(), sValue.size(), bColumnSelect, bLineSelect);
			Wrap(&result)->Receive(bRet);
		}
		return true;

	case F_LAYOUTTOLOGICLINENUM:
		// レイアウト→ロジック行
		{
			if (numArgs < 1) {
				return false;
			}
			if (!VariantToI4(varCopy, args[0])) {
				return false;
			}
			LayoutInt nLineNum = LayoutInt(varCopy.data.lVal - 1);
			int ret = 0;
			if (view->m_pEditDoc->m_layoutMgr.GetLineCount() == nLineNum) {
				ret = (Int)view->m_pEditDoc->m_docLineMgr.GetLineCount() + 1;
			}else {
				const Layout* pcLayout = view->m_pEditDoc->m_layoutMgr.SearchLineByLayoutY(nLineNum);
				if (pcLayout) {
					ret = pcLayout->GetLogicLineNo() + 1;
				}else {
					return false;
				}
			}
			Wrap(&result)->Receive(ret);
		}
		return true;

	case F_LINECOLUMNTOINDEX:
		// レイアウト→ロジック桁
		{
			if (numArgs < 2) {
				return false;
			}
			if (!VariantToI4(varCopy, args[0])) {
				return false;
			}
			LayoutInt nLineNum = LayoutInt(varCopy.data.lVal - 1);
			if (!VariantToI4(varCopy, args[1])) {
				return false;
			}
			LayoutInt nLineCol = LayoutInt(varCopy.data.lVal - 1);
			if (nLineNum < 0) {
				return false;
			}

			LayoutPoint nLayoutPos(nLineCol, nLineNum);
			LogicPoint nLogicPos(LogicInt(0), LogicInt(0));
			view->m_pEditDoc->m_layoutMgr.LayoutToLogic(nLayoutPos, &nLogicPos);
			int ret = nLogicPos.GetX() + 1;
			Wrap(&result)->Receive(ret);
		}
		return true;

	case F_LOGICTOLAYOUTLINENUM:
	case F_LINEINDEXTOCOLUMN:
		// ロジック→レイアウト行/桁
		{
			if (numArgs < 2) {
				return false;
			}
			if (!VariantToI4(varCopy, args[0])) {
				return false;
			}
			LogicInt nLineNum = LogicInt(varCopy.data.lVal - 1);
			if (!VariantToI4(varCopy, args[1])) {
				return false;
			}
			LogicInt nLineIdx = LogicInt(varCopy.data.lVal - 1);
			if (nLineNum < 0) {
				return false;
			}

			LogicPoint nLogicPos(nLineIdx, nLineNum);
			LayoutPoint nLayoutPos(LayoutInt(0), LayoutInt(0));
			view->m_pEditDoc->m_layoutMgr.LogicToLayout(nLogicPos, &nLayoutPos);
			int ret = ((LOWORD(id) == F_LOGICTOLAYOUTLINENUM) ? (Int)nLayoutPos.GetY2() : (Int)nLayoutPos.GetX2()) + 1;
			Wrap(&result)->Receive(ret);
		}
		return true;

	case F_GETCOOKIE:
		{
			Variant varCopy2;
			if (numArgs >= 2) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) {
					return false;
				}
				if (VariantChangeType(&varCopy2.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_BSTR) != S_OK) {
					return false;
				}
				SysString ret = view->GetDocument()->m_cookie.GetCookie(varCopy.data.bstrVal, varCopy2.data.bstrVal);
				Wrap(&result)->Receive(ret);
				return true;
			}
			return false;
		}
	case F_GETCOOKIEDEFAULT:
		{
			Variant varCopy2, varCopy3;
			if (numArgs >= 3) {
				if (0
					|| VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK
					|| VariantChangeType(&varCopy2.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_BSTR) != S_OK
					|| VariantChangeType(&varCopy3.data, const_cast<VARIANTARG*>(&(args[2])), 0, VT_BSTR) != S_OK
				) {
					return false;
				}
				SysString ret = view->GetDocument()->m_cookie.GetCookieDefault(
					varCopy.data.bstrVal,
					varCopy2.data.bstrVal,
					varCopy3.data.bstrVal,
					SysStringLen(varCopy3.data.bstrVal)
				);
				Wrap(&result)->Receive(ret);
				return true;
			}
			return false;
		}
	case F_SETCOOKIE:
		{
			Variant varCopy2, varCopy3;
			if (numArgs >= 3) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) return false;
				if (VariantChangeType(&varCopy2.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_BSTR) != S_OK) return false;
				if (VariantChangeType(&varCopy3.data, const_cast<VARIANTARG*>(&(args[2])), 0, VT_BSTR) != S_OK) return false;
				int ret = view->GetDocument()->m_cookie.SetCookie(varCopy.data.bstrVal, varCopy2.data.bstrVal,
					varCopy3.data.bstrVal, SysStringLen(varCopy3.data.bstrVal));
				Wrap(&result)->Receive(ret);
				return true;
			}
			return false;
		}
	case F_DELETECOOKIE:
		{
			Variant varCopy2;
			if (numArgs >= 2) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) return false;
				if (VariantChangeType(&varCopy2.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_BSTR) != S_OK) return false;
				int ret = view->GetDocument()->m_cookie.DeleteCookie(varCopy.data.bstrVal, varCopy2.data.bstrVal);
				Wrap(&result)->Receive(ret);
				return true;
			}
			return false;
		}
	case F_GETCOOKIENAMES:
		{
			if (numArgs >= 1) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) return false;
				SysString ret = view->GetDocument()->m_cookie.GetCookieNames(varCopy.data.bstrVal);
				Wrap(&result)->Receive(ret);
				return true;
			}
			return false;
		}
	case F_SETDRAWSWITCH:
		{
			if (1 <= numArgs) {
				if (!VariantToI4(varCopy, args[0])) return false;
				int ret = (view->m_pEditWnd->SetDrawSwitchOfAllViews(varCopy.data.iVal != 0) ? 1: 0);
				Wrap(&result)->Receive(ret);
				return true;
			}
			return false;
		}
	case F_GETDRAWSWITCH:
		{
			int ret = (view->GetDrawSwitch() ? 1: 0);
			Wrap(&result)->Receive(ret);
			return true;
		}
	case F_ISSHOWNSTATUS:
		{
			int ret = (view->m_pEditWnd->m_statusBar.GetStatusHwnd() ? 1: 0);
			Wrap(&result)->Receive(ret);
			return true;
		}
	case F_GETSTRWIDTH:
		{
			Variant varCopy2;
			if (1 <= numArgs) {
				if (!VariantToBStr(varCopy, args[0])) { return false; }
				if (2 <= numArgs) {
					if (!VariantToI4(varCopy2, args[1])) { return false; }
				}else {
					varCopy2.data.lVal = 1;
				}
				const wchar_t* pLine = varCopy.data.bstrVal;
				int nLen = ::SysStringLen(varCopy.data.bstrVal);
				if (2 <= nLen) {
					if (pLine[nLen-2] == WCODE::CR && pLine[nLen-1] == WCODE::LF) {
						--nLen;
					}
				}
				const int nTabWidth = (Int)view->GetDocument()->m_layoutMgr.GetTabSpaceKetas();
				int nPosX = varCopy2.data.lVal - 1;
				for (int i=0; i<nLen;) {
					if (pLine[i] == WCODE::TAB) {
						nPosX += nTabWidth - (nPosX % nTabWidth);
					}else {
						nPosX += (Int)NativeW::GetKetaOfChar(pLine, nLen, i);
					}
					i += t_max(1, (int)(Int)NativeW::GetSizeOfChar(pLine, nLen, i));
				}
				nPosX -=  varCopy2.data.lVal - 1;
				Wrap(&result)->Receive(nPosX);
				return true;
			}
			return false;
		}
	case F_GETSTRLAYOUTLENGTH:
		{
			Variant varCopy2;
			if (1 <= numArgs) {
				if (!VariantToBStr(varCopy, args[0])) { return false; }
				if (2 <= numArgs) {
					if (!VariantToI4(varCopy2, args[1])) { return false; }
				}else {
					varCopy2.data.lVal = 1;
				}
				DocLine tmpDocLine;
				tmpDocLine.SetDocLineString(varCopy.data.bstrVal, ::SysStringLen(varCopy.data.bstrVal));
				const int tmpLenWithEol1 = tmpDocLine.GetLengthWithoutEOL() + (0 < tmpDocLine.GetEol().GetLen() ? 1: 0);
				const LayoutXInt offset(varCopy2.data.lVal - 1);
				const Layout tmpLayout(
					&tmpDocLine,
					LogicPoint(0, 0),
					LogicXInt(tmpLenWithEol1),
					COLORIDX_TEXT,
					offset,
					NULL
				);
				LayoutXInt width = view->LineIndexToColumn(&tmpLayout, tmpDocLine.GetLengthWithEOL()) - offset;
				Wrap(&result)->Receive((Int)width);
				return true;
			}
			return false;
		}
	case F_GETDEFAULTCHARLENGTH:
		{
			LayoutXInt width = view->GetTextMetrics().GetLayoutXDefault();
			Wrap(&result)->Receive((Int)width);
			return true;
		}
	case F_ISINCLUDECLIPBOARDFORMAT:
		{
			if (1 <= numArgs) {
				if (!VariantToBStr(varCopy, args[0])) return false;
				Clipboard cClipboard(view->GetHwnd());
				bool bret = cClipboard.IsIncludeClipboradFormat(varCopy.data.bstrVal);
				Wrap(&result)->Receive(bret ? 1 : 0);
				return true;
			}
			return false;
		}
	case F_GETCLIPBOARDBYFORMAT:
		{
			Variant varCopy2, varCopy3;
			if (2 <= numArgs) {
				if (!VariantToBStr(varCopy, args[0])) return false;
				if (!VariantToI4(varCopy2, args[1])) return false;
				if (3 <= numArgs) {
					if (!VariantToI4(varCopy3, args[2])) return false;
				}else {
					varCopy3.data.lVal = -1;
				}
				Clipboard cClipboard(view->GetHwnd());
				NativeW mem;
				Eol cEol = view->m_pEditDoc->m_docEditor.GetNewLineCode();
				cClipboard.GetClipboradByFormat(mem, varCopy.data.bstrVal, varCopy2.data.lVal, varCopy3.data.lVal, cEol);
				SysString ret = SysString(mem.GetStringPtr(), mem.GetStringLength());
				Wrap(&result)->Receive(ret);
				return true;
			}
			return false;
		}
	case F_SETCLIPBOARDBYFORMAT:
		{
			Variant varCopy2, varCopy3, varCopy4;
			if (3 <= numArgs) {
				if (!VariantToBStr(varCopy, args[0])) return false;
				if (!VariantToBStr(varCopy2, args[1])) return false;
				if (!VariantToI4(varCopy3, args[2])) return false;
				if (3 <= numArgs) {
					if (!VariantToI4(varCopy4, args[3])) return false;
				}else {
					varCopy4.data.lVal = -1;
				}
				Clipboard cClipboard(view->GetHwnd());
				StringRef cstr(varCopy.data.bstrVal, ::SysStringLen(varCopy.data.bstrVal));
				bool bret = cClipboard.SetClipboradByFormat(cstr, varCopy2.data.bstrVal, varCopy3.data.lVal, varCopy4.data.lVal);
				Wrap(&result)->Receive(bret ? 1 : 0);
				return true;
			}
			return false;
		}
	case F_GETLINEATTRIBUTE:
		{
			int nLineNum;
			int nAttType;
			if (numArgs < 2) {
				return false;
			}
			if (!variant_to_int(args[0], nLineNum)) return false;
			if (!variant_to_int(args[1], nAttType)) return false;
			LogicInt nLine;
			if (nLineNum == 0) {
				nLine = view->GetCaret().GetCaretLogicPos().GetY2();
			}else if (nLineNum < 0) {
				return false;
			}else {
				nLine = LogicInt(nLineNum - 1); // nLineNumは1開始
			}
			const DocLine* pDocLine = view->GetDocument()->m_docLineMgr.GetLine(nLine);
			if (!pDocLine) {
				return false;
			}
			int nRet;
			switch (nAttType) {
			case 0:
				nRet = (ModifyVisitor().IsLineModified(pDocLine, view->GetDocument()->m_docEditor.m_opeBuf.GetNoModifiedSeq()) ? 1: 0);
				break;
			case 1:
				nRet = pDocLine->m_mark.m_modified.GetSeq();
				break;
			case 2:
				nRet = (pDocLine->m_mark.m_bookmarked ? 1: 0);
				break;
			case 3:
				nRet = (int)(DiffMark)(pDocLine->m_mark.m_diffMarked);
				break;
			case 4:
				nRet = (pDocLine->m_mark.m_funcList.GetFuncListMark() ? 1: 0 );
				break;
			default:
				return false;
			}
			Wrap( &result )->Receive( nRet );
			return true;
		}
	case F_ISTEXTSELECTINGLOCK:
		{
			if (view->GetSelectionInfo().m_bSelectingLock) {
				if (view->GetSelectionInfo().IsBoxSelecting()) {
					Wrap( &result )->Receive( 2 );	//選択ロック+矩形選択中
				}else {
					Wrap( &result )->Receive( 1 );	//選択ロック中
				}
			}else {
				Wrap( &result )->Receive( 0 );		//非ロック中
			}
		}
		return true;
	case F_GETVIEWLINES:
		{
			int nLines = (Int)view->GetTextArea().m_nViewRowNum;
			Wrap( &result )->Receive( nLines );
			return true;
		}
	case F_GETVIEWCOLUMNS:
		{
			int nColumns = (Int)view->GetTextArea().m_nViewColNum;
			Wrap( &result )->Receive( nColumns );
			return true;
		}
	case F_CREATEMENU:
		{
			Variant varCopy2;
			if (2 <= numArgs) {
				if (!VariantToI4(varCopy, args[0])) {
					return false;
				}
				if (!VariantToBStr(varCopy2, args[1])) {
					return false;
				}
				std::vector<wchar_t> vStrMenu;
				int nLen = (int)auto_strlen(varCopy2.data.bstrVal);
				vStrMenu.assign( nLen + 1, L'\0' );
				auto_strcpy(&vStrMenu[0], varCopy2.data.bstrVal);
				HMENU hMenu = ::CreatePopupMenu();
				std::vector<HMENU> vHmenu;
				vHmenu.push_back( hMenu );
				HMENU hMenuCurrent = hMenu;
				int nPos = 0;
				wchar_t* p;
				int i = 1;
				while (p = my_strtok( &vStrMenu[0], nLen, &nPos, L"," )) {
					wchar_t* r = p;
					int nFlags = MF_STRING;
					int nFlagBreak = 0;
					bool bSpecial = false;
					bool bRadio = false;
					bool bSubMenu = false;
					int nBreakNum = 0;
					if (p[0] == L'[') {
						++r;
						while (*r != L']' && *r != L'\0') {
							switch (*r) {
							case L'S':
								if (!bSubMenu) {
									HMENU hMenuSub = ::CreatePopupMenu();
									vHmenu.push_back( hMenuSub );
									bSubMenu = true;
								}
								break;
							case L'E':
								++nBreakNum;
								break;
							case L'C':
								nFlags |= MF_CHECKED;
								break;
							case L'D':
								nFlags |= MF_GRAYED;
								break;
							case L'R':
								bRadio = true;
								break;
							case L'B':
								nFlagBreak |= MF_MENUBARBREAK;
								break;
							default:
								break;
							}
							++r;
						}
						if (*r == L']') {
							++r;
						}
					}
					if (p[0] == L'-' && p[1] == L'\0') {
						nFlags |= MF_SEPARATOR;
						++r;
						bSpecial = true;
					}

					if (bSubMenu) {
						nFlags |= nFlagBreak;
						::InsertMenu( hMenuCurrent, -1, nFlags | MF_BYPOSITION | MF_POPUP, (UINT_PTR)vHmenu.back(), to_tchar(r) );
						hMenuCurrent = vHmenu.back();
					}else if (bSpecial) {
						nFlags |= nFlagBreak;
						::InsertMenu( hMenuCurrent, -1, nFlags | MF_BYPOSITION, 0, NULL );
					}else {
						nFlags |= nFlagBreak;
						::InsertMenu( hMenuCurrent, -1, nFlags | MF_BYPOSITION, i, to_tchar(r) );
						if (bRadio) {
							::CheckMenuRadioItem( hMenuCurrent, i, i, i, MF_BYCOMMAND );
						}
						++i;
					}
					for (int n=0; n<nBreakNum; ++n) {
						if (1 < vHmenu.size()) {
							vHmenu.resize( vHmenu.size() - 1 );
						}
						hMenuCurrent = vHmenu.back();
					}
				}
				POINT pt;
				int nViewPointType = varCopy.data.lVal;
				if (nViewPointType == 1) {
					// キャレット位置
					pt = view->GetCaret().CalcCaretDrawPos( view->GetCaret().GetCaretLayoutPos() );
					if (view->GetTextArea().GetAreaLeft() <= pt.x
						&& view->GetTextArea().GetAreaTop() <= pt.y
						&& pt.x < view->GetTextArea().GetAreaRight()
						&& pt.y < view->GetTextArea().GetAreaBottom()
					) {
						::ClientToScreen( view->GetHwnd(), &pt );
					}else {
						::GetCursorPos( &pt );
					}
				}else {
					// マウス位置
					::GetCursorPos( &pt );
				}
				RECT rcWork;
				GetMonitorWorkRect( pt, &rcWork );
				int nId = ::TrackPopupMenu(
					hMenu,
					TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
					(pt.x > rcWork.left) ? pt.x: rcWork.left,
					(pt.y < rcWork.bottom) ? pt.y: rcWork.bottom,
					0,
					view->GetHwnd(),
					NULL
				);
				::DestroyMenu( hMenu );
				Wrap( &result )->Receive( nId );
				return true;
			}
			return false;
		}
	default:
		return false;
	}
}

