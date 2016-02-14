/*!	@file
	@brief PPA Library Handler

	PPA.DLLを利用するためのインターフェース

	@author YAZAKI
	@date 2002年1月26日
*/
/*
	Copyright (C) 2001, YAZAKI
	Copyright (C) 2002, YAZAKI, aroka, genta, Moca
	Copyright (C) 2003, Moca, genta

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
#include "CPPA.h"
#include "view/CEditView.h"
#include "func/Funccode.h"
#include "CMacro.h"
#include "macro/CSMacroMgr.h"// 2002/2/10 aroka
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "_os/OleTypes.h"

#define NEVER_USED_PARAM(p) ((void)p)

// 2003.06.01 Moca
#define omGet (0)
#define omSet (1)

// 2007.07.26 genta
PPA::PpaExecInfo* PPA::m_CurInstance = NULL;
bool PPA::m_bIsRunning = false;

PPA::PPA()
{
}

PPA::~PPA()
{
}

// @date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
bool PPA::Execute(EditView* pEditView, int flags)
{
	// PPAの多重起動禁止 2008.10.22 syat
	if (PPA::m_bIsRunning) {
		MYMESSAGEBOX(pEditView->GetHwnd(), MB_OK, LS(STR_ERR_DLGPPA7), LS(STR_ERR_DLGPPA1));
		m_fnAbort();
		PPA::m_bIsRunning = false;
		return false;
	}
	PPA::m_bIsRunning = true;

	PpaExecInfo info;
	info.m_pEditView = pEditView;
	info.m_pShareData = &GetDllShareData();
	info.m_bError = false;			// 2003.06.01 Moca
	info.m_memDebug.SetString("");	// 2003.06.01 Moca
	info.m_commandflags = flags | FA_FROMMACRO;	// 2007.07.22 genta
	
	// 実行前にインスタンスを待避する
	PpaExecInfo* old_instance = m_CurInstance;
	m_CurInstance = &info;
	m_fnExecute();
	
	// マクロ実行完了後はここに戻ってくる
	m_CurInstance = old_instance;

	// PPAの多重起動禁止 2008.10.22 syat
	PPA::m_bIsRunning = false;
	return !info.m_bError;
}

LPCTSTR PPA::GetDllNameImp(int nIndex)
{
	return _T("PPA.DLL");
}

/*!
	DLLの初期化

	関数のアドレスを取得してメンバに保管する．

	@retval 0 成功
	@retval 1 アドレス取得に失敗
*/
bool PPA::InitDllImp()
{
	// PPA.DLLが持っている関数を準備

	// pr. 15, 2002 genta constを付けた
	// アドレスの入れ場所はオブジェクトに依存するので
	// tatic配列にはできない。
	const ImportTable table[] = {
		{ &m_fnExecute,		"Execute" },
		{ &m_fnSetDeclare,	"SetDeclare" },
		{ &m_fnSetSource,	"SetSource" },
		{ &m_fnSetDefProc,	"SetDefProc" },
		{ &m_fnSetDefine,	"SetDefine" },
		{ &m_fnSetIntFunc,	"SetIntFunc" },
		{ &m_fnSetStrFunc,	"SetStrFunc" },
		{ &m_fnSetProc,		"SetProc" },
		{ &m_fnSetErrProc,	"SetErrProc" },
		{ &m_fnAbort,		"ppaAbort" },
		{ &m_fnGetVersion,	"GetVersion" },
		{ &m_fnDeleteVar,	"DeleteVar" },
		{ &m_fnGetArgInt,	"GetArgInt" },
		{ &m_fnGetArgStr,	"GetArgStr" },
		{ &m_fnGetArgBStr,	"GetArgBStr" },
		{ &m_fnGetIntVar,	"GetIntVar" },
		{ &m_fnGetStrVar,	"GetStrVar" },
		{ &m_fnGetBStrVar,	"GetBStrVar" },
		{ &m_fnSetIntVar,	"SetIntVar" },
		{ &m_fnSetStrVar,	"SetStrVar" },
		{ &m_fnAddIntObj,	"AddIntObj" },
		{ &m_fnAddStrObj,	"AddStrObj" },
		{ &m_fnAddIntVar,	"AddIntVar" },
		{ &m_fnAddStrVar,	"AddStrVar" },
		{ &m_fnSetIntObj,	"SetIntObj" },
		{ &m_fnSetStrObj,	"SetStrObj" },

#if PPADLL_VER >= 120
		{ &m_fnAddRealVar,	"AddRealVar" },
		{ &m_fnSetRealObj,	"SetRealObj" },
		{ &m_fnAddRealObj,	"AddRealObj" },
		{ &m_fnGetRealVar,	"GetRealVar" },
		{ &m_fnSetRealVar,	"SetRealVar" },
		{ &m_fnSetRealFunc,	"SetRealFunc" },
		{ &m_fnGetArgReal,	"GetArgReal" },
#endif

#if PPADLL_VER >= 123
		{ &m_fnIsRunning, "IsRunning" },
		{ &m_fnSetFinishProc, "SetFinishProc"}, // 2003.06.23 Moca
#endif

		{ NULL, 0 }
	};

	// pr. 15, 2002 genta
	// DllImpの共通関数化した
	if (! RegisterEntries(table)) {
		return false;
	}

	SetIntFunc((void *)PPA::stdIntFunc);	// 2003.02.24 Moca
	SetStrFunc((void *)PPA::stdStrFunc);
	SetProc((void *)PPA::stdProc);

	// 2003.06.01 Moca エラーメッセージを追加
	SetErrProc((void *)PPA::stdError);
	SetStrObj((void *)PPA::stdStrObj);	// UserErrorMes用
#if PPADLL_VER >= 123
	SetFinishProc((void *)PPA::stdFinishProc);
#endif

	SetDefine("sakura-editor");	// 2003.06.01 Moca SAKURAエディタ用独自関数を準備
	AddStrObj("UserErrorMes", "", FALSE, 2); // 2003.06.01 デバッグ用文字列変数を用意

	// Jun. 16, 2003 genta 一時作業エリア
	char buf[1024];
	// コマンドに置き換えられない関数 ＝ PPA無しでは使えない。。。
	for (int i=0; SMacroMgr::m_macroFuncInfoArr[i].m_pszFuncName; ++i) {
		// 2003.06.08 Moca メモリーリークの修正
		// 2003.06.16 genta バッファを外から与えるように
		// 関数登録用文字列を作成する
		GetDeclarations(SMacroMgr::m_macroFuncInfoArr[i], buf);
		SetDefProc(buf);
	}

	// コマンドに置き換えられる関数 ＝ PPA無しでも使える。
	for (int i=0; SMacroMgr::m_macroFuncInfoCommandArr[i].m_pszFuncName; ++i) {
		// 2003.06.08 Moca メモリーリークの修正
		// 2003.06.16 genta バッファを外から与えるように
		// 関数登録用文字列を作成する
		GetDeclarations(SMacroMgr::m_macroFuncInfoCommandArr[i], buf);
		SetDefProc(buf);
	}
	return true; 
}

/*! PPAに関数を登録するための文字列を作成する

	@param macroFuncInfo [in]	マクロデータ
	@param szBuffer [out]		生成した文字列を入れるバッファへのポインタ

	@note バッファサイズは 9 + 3 + メソッド名の長さ + 13 * 4 + 9 + 5 は最低必要

	@date 2003.06.01 Moca
				スタティックメンバに変更
				macroFuncInfo.m_pszDataを書き換えないように変更

	@date 2003.06.16 genta 無駄なnew/deleteを避けるためバッファを外から与えるように
*/
char* PPA::GetDeclarations( const MacroFuncInfo& macroFuncInfo, char* szBuffer )
{
	char szType[20];	// procedure/function用バッファ
	char szReturn[20];	// 戻り値型用バッファ
	if (macroFuncInfo.m_varResult == VT_EMPTY) {
		strcpy(szType, "procedure");
		szReturn[0] = '\0';
	}else {
		strcpy(szType, "function");
		if (macroFuncInfo.m_varResult == VT_BSTR) {
			strcpy(szReturn, ": string");
		}else if (macroFuncInfo.m_varResult == VT_I4) {
			strcpy(szReturn, ": Integer");
		}else {
			szReturn[0] = '\0';
		}
	}
	
	char szArguments[8][20]; // 引数用バッファ
	int i;
	for (i=0; i<8; ++i) {
		VARTYPE type = VT_EMPTY;
		if (i < 4) {
			type = macroFuncInfo.m_varArguments[i];
		}else {
			if (macroFuncInfo.m_pData && i < macroFuncInfo.m_pData->m_nArgMinSize) {
				type = macroFuncInfo.m_pData->m_pVarArgEx[i - 4];
			}
		}
		if (type == VT_EMPTY) {
			break;
		}
		if (type == VT_BSTR) {
			strcpy(szArguments[i], "s0: string");
			szArguments[i][1] = '0' + (char)i;
		}else if (type == VT_I4) {
			strcpy(szArguments[i], "i0: Integer");
			szArguments[i][1] = '0' + (char)i;
		}else {
			strcpy(szArguments[i], "u0: Unknown");
		}
	}
	if (i > 0) {	// 引数があったとき
		char szArgument[8*20];
		// 2002.12.06 Moca 原因不明だが，strcatがVC6Proでうまく動かなかったため，strcpyにしてみたら動いた
		strcpy(szArgument, szArguments[0]);
		for (int j=1; j<i; ++j) {
			strcat(szArgument, "; ");
			strcat(szArgument, szArguments[j]);
		}
		auto_sprintf( szBuffer, "%hs S_%ls(%hs)%hs; index %d;",
			szType,
			macroFuncInfo.m_pszFuncName,
			szArgument,
			szReturn,
			macroFuncInfo.m_nFuncID
		);
	}else {
		auto_sprintf( szBuffer, "%hs S_%ls%hs; index %d;",
			szType,
			macroFuncInfo.m_pszFuncName,
			szReturn,
			macroFuncInfo.m_nFuncID
		);
	}
	// Jun. 01, 2003 Moca / Jun. 16, 2003 genta
	return szBuffer;
}

/*! ユーザー定義文字列型オブジェクト
	現在は、デバッグ用文字列を設定する為のみ
*/
void __stdcall PPA::stdStrObj(
	const char* ObjName,
	int Index,
	BYTE GS_Mode,
	int* Err_CD,
	char** Value
	)
{
	NEVER_USED_PARAM(ObjName);
	*Err_CD = 0;
	switch (Index) {
	case 2:
		switch (GS_Mode) {
		case omGet:
			*Value = m_CurInstance->m_memDebug.GetStringPtr();
			break;
		case omSet:
			m_CurInstance->m_memDebug.SetString(*Value);
			break;
		}
		break;
	default:
		*Err_CD = -1;
	}
}


/*! ユーザー定義関数のエラーメッセージの作成

	stdProc, stdIntFunc, stdStrFunc がエラーコードを返した場合、PPAから呼び出される。
	異常終了/不正引数時のエラーメッセージを独自に指定する。
	@author Moca
	@param Err_CD IN  0以外各コールバック関数が設定した値
			 1以上 FuncID + 1
			 0     PPAのエラー
			-1以下 その他ユーザ定義エラー
	@param Err_Mes IN エラーメッセージ

	@date 2003.06.01 Moca
*/
void __stdcall PPA::stdError(int Err_CD, const char* Err_Mes)
{
	if (m_CurInstance->m_bError) {
		return;
	}
	m_CurInstance->m_bError = true; // 関数内で関数を呼ぶ場合等、2回表示されるのを防ぐ

	TCHAR szMes[2048]; // 2048あれば足りるかと
	const TCHAR* pszErr;
	pszErr = szMes;
	if (0 < Err_CD) {
		int i, FuncID;
		FuncID = Err_CD - 1;
		char szFuncDec[1024];
		szFuncDec[0] = '\0';
		for (i=0; SMacroMgr::m_macroFuncInfoCommandArr[i].m_nFuncID!=-1; ++i) {
			if (SMacroMgr::m_macroFuncInfoCommandArr[i].m_nFuncID == FuncID) {
				GetDeclarations(SMacroMgr::m_macroFuncInfoCommandArr[i], szFuncDec);
				break;
			}
		}
		if (SMacroMgr::m_macroFuncInfoArr[i].m_nFuncID != -1) {
			for (i=0; SMacroMgr::m_macroFuncInfoArr[i].m_nFuncID!=-1; ++i) {
				if (SMacroMgr::m_macroFuncInfoArr[i].m_nFuncID == FuncID) {
					GetDeclarations(SMacroMgr::m_macroFuncInfoArr[i], szFuncDec);
					break;
				}
			}
		}
		if (szFuncDec[0] != '\0') {
			auto_sprintf( szMes, LS(STR_ERR_DLGPPA2), szFuncDec );
		}else {
			auto_sprintf( szMes, LS(STR_ERR_DLGPPA3), FuncID );
		}
	}else {
		// 2007.07.26 genta : ネスト実行した場合にPPAが不正なポインタを渡す可能性を考慮．
		// 実際には不正なエラーは全てPPA.DLL内部でトラップされるようだが念のため．
		if (IsBadStringPtrA(Err_Mes, 256)) {
			pszErr = LS(STR_ERR_DLGPPA6);
		}else {
			switch (Err_CD) {
			case 0:
				if ('\0' == Err_Mes[0]) {
					pszErr = LS(STR_ERR_DLGPPA4);
				}else {
					pszErr = to_tchar(Err_Mes);
				}
				break;
			default:
				auto_sprintf( szMes, LS(STR_ERR_DLGPPA5), Err_CD, to_tchar(Err_Mes) );
			}
		}
	}
	if (m_CurInstance->m_memDebug.GetStringLength() == 0) {
		MYMESSAGEBOX(m_CurInstance->m_pEditView->GetHwnd(), MB_OK, LS(STR_ERR_DLGPPA7), _T("%ts"), pszErr);
	}else {
		MYMESSAGEBOX(m_CurInstance->m_pEditView->GetHwnd(), MB_OK, LS(STR_ERR_DLGPPA7), _T("%ts\n%hs"), pszErr, m_CurInstance->m_memDebug.GetStringPtr());
	}
}

//----------------------------------------------------------------------
/** プロシージャ実行callback

	@date 2007.07.20 genta Indexと一緒にフラグを渡す
*/
void __stdcall PPA::stdProc(
	const char*	funcName,
	const int	index,
	const char*	args[],
	const int	numArgs,
	int*		err_CD
	)
{
	NEVER_USED_PARAM(funcName);
	EFunctionCode eIndex = (EFunctionCode)index;

	*err_CD = 0;

	// Argumentをwchar_t[]に変換 -> tmpArguments
	WCHAR** tmpArguments2 = new WCHAR*[numArgs];
	int* tmpArgLengths = new int[numArgs];
	for (int i=0; i<numArgs; ++i) {
		if (args[i]) {
			tmpArguments2[i] = mbstowcs_new(args[i]);
			tmpArgLengths[i] = wcslen(tmpArguments2[i]);
		}else {
			tmpArguments2[i] = NULL;
			tmpArgLengths[i] = 0;
		}
	}
	const WCHAR** tmpArguments = (const WCHAR**)tmpArguments2;

	// 処理
	bool bRet = Macro::HandleCommand(
		m_CurInstance->m_pEditView,
		(EFunctionCode)(eIndex | m_CurInstance->m_commandflags),
		tmpArguments,
		tmpArgLengths,
		numArgs
	);
	if (!bRet) {
		*err_CD = eIndex + 1;
	}

	// tmpArgumentsを解放
	for (int i=0; i<numArgs; ++i) {
		if (tmpArguments2[i]) {
			WCHAR* p = const_cast<WCHAR*>(tmpArguments2[i]);
			delete[] p;
		}
	}
	delete[] tmpArguments2;
	delete[] tmpArgLengths;
}

//----------------------------------------------------------------------
/*!
	整数値を返す関数を処理する

	PPAから呼びだされる
	@author Moca
	@date 2003.02.24 Moca
*/
void __stdcall PPA::stdIntFunc(
	const char* FuncName,
	const int Index,
	const char* Argument[],
	const int ArgSize,
	int* Err_CD,
	int* ResultValue
	)
{
	NEVER_USED_PARAM(FuncName);
	VARIANT Ret;
	::VariantInit(&Ret);

	*ResultValue = 0;
	*Err_CD = 0;
	if (false != CallHandleFunction(Index, Argument, ArgSize, &Ret)) {
		switch (Ret.vt) {
		case VT_I4:
			*ResultValue = Ret.lVal;
			break;
		case VT_INT:
			*ResultValue = Ret.intVal;
			break;
		case VT_UINT:
			*ResultValue = Ret.uintVal;
			break;
		default:
			*Err_CD = -2; // 2003.06.01 Moca 値変更
		}
		::VariantClear(&Ret);
		return;
	}
	*Err_CD = Index + 1; // 2003.06.01 Moca
	::VariantClear(&Ret);
	return;
}

//----------------------------------------------------------------------
/*!
	文字列を返す関数を処理する

	PPAから呼びだされる
	@date 2003.02.24 Moca CallHandleFunction対応
*/
void __stdcall PPA::stdStrFunc(
	const char* FuncName,
	const int Index,
	const char* Argument[],
	const int ArgSize,
	int* Err_CD,
	char** ResultValue
	)
{
	NEVER_USED_PARAM(FuncName);

	VARIANT Ret;
	::VariantInit(&Ret);
	*Err_CD = 0;
	if (false != CallHandleFunction(Index, Argument, ArgSize, &Ret)) {
		if (VT_BSTR == Ret.vt) {
			int len;
			char* buf;
			Wrap(&Ret.bstrVal)->Get(&buf, &len);
			m_CurInstance->m_memRet.SetString(buf, len); // Mar. 9, 2003 genta
			delete[] buf;
			*ResultValue = m_CurInstance->m_memRet.GetStringPtr();
			::VariantClear(&Ret);
			return;
		}
	}
	::VariantClear(&Ret);
	*Err_CD = Index + 1;
	*ResultValue = const_cast<char*>("");
	return;
}

/*!
	引数型変換

	文字列で与えられた引数をVARIANT/BSTRに変換してMacro::HandleFunction()を呼びだす
	@author Moca
*/
bool PPA::CallHandleFunction(
	const int Index,
	const char* Arg[],
	int ArgSize,
	VARIANT* Result
	)
{
	int ArgCnt;
	const int maxArgSize = 8;
	VARIANT vtArg[maxArgSize];
	
	const MacroFuncInfo* mfi = SMacroMgr::GetFuncInfoByID(Index);
	for (int i=0; i<maxArgSize && i<ArgSize; ++i) {
		::VariantInit(&vtArg[i]);
	}
	ArgCnt = 0;
	for (int i=0, ArgCnt=0; i<maxArgSize && i<ArgSize; ++i) {
		VARTYPE type = VT_EMPTY;
		if (i < 4) {
			type = mfi->m_varArguments[i];
		}else {
			if (mfi->m_pData && i < mfi->m_pData->m_nArgMinSize) {
				type = mfi->m_pData->m_pVarArgEx[i - 4];
			}
		}
		if (VT_EMPTY == type) {
			break;
		}
		switch (type) {
		case VT_I4:
		{
			vtArg[i].vt = VT_I4;
			vtArg[i].lVal = atoi(Arg[i]);
			break;
		}
		case VT_BSTR:
		{
			SysString S(Arg[i], lstrlenA(Arg[i]));
			Wrap(&vtArg[i])->Receive(S);
			break;
		}
		default:
			for (int i=0; i<maxArgSize && i<ArgSize; ++i) {
				::VariantClear(&vtArg[i]);
			}
			return false;
		}
		++ArgCnt;
	}

	if (Index >= F_FUNCTION_FIRST) {
		bool Ret = Macro::HandleFunction(m_CurInstance->m_pEditView, (EFunctionCode)Index, vtArg, ArgCnt, *Result);
		for (int i=0; i<maxArgSize && i<ArgSize; ++i) {
			::VariantClear(&vtArg[i]);
		}
		return Ret;
	}else {
		for (int i=0; i<maxArgSize && i<ArgSize; ++i) {
			::VariantClear(&vtArg[i]);
		}
		return false;
	}
}

#if PPADLL_VER >= 123

/*!
	PPAマクロの実行終了時に呼ばれる
	
	@date 2003.06.01 Moca
*/
void __stdcall PPA::stdFinishProc()
{
	// 2007.07.26 genta : 終了処理は不要
}

#endif

