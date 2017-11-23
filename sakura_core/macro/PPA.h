/*!	@file
	@brief PPA Library Handler

	PPA.DLLを利用するためのインターフェース
*/
/*
PPA(Poor-Pascal for Application)はDelphi/C++Builder用のPascalインタプリタコンポーネントです。
*/

#pragma once

#include <ObjIdl.h>  // VARIANT等
#include <stdio.h>
#include "macro/SMacroMgr.h"
#include "extmodule/DllHandler.h"

#define PPADLL_VER 123

/*
PPA(Poor-Pascal for Application)はDelphi/C++Builder用の
Pascalインタプリタコンポーネントです。
アプリケーションにマクロ機能を搭載する事を目的に作成されています。
*/

/*!
	@brief PPA.DLL をサポートするクラス

	DLLの動的ロードを行うため、DllHandlerを継承している。
*/
class PPA : public DllImp {
public:
	PPA();
	virtual ~PPA();

	const char* GetVersion() {		// DLLのバージョン情報を取得。szMsgを壊す
		if (IsAvailable()) {
			auto_sprintf(szMsg, "PPA.DLL Version %d.%d", fnGetVersion() / 100, fnGetVersion() % 100);
			return szMsg;
		}
		return "";
	}

	// PPAメッセージを取得する
	const char* GetLastMessage(void) const { return szMsg; }

	// Jun. 16, 2003 genta 引数追加
	static char* GetDeclarations(const MacroFuncInfo&, char* buf);

protected:
	// Jul. 5, 2001 genta インターフェース変更に伴う引数追加
public:
	virtual LPCTSTR GetDllNameImp(int nIndex);
protected:
	virtual bool InitDllImp();

private:
	// DLL Interfaceの受け皿
	typedef void (WINAPI *PPA_Execute)();
	typedef void (WINAPI *PPA_SetSource) (const char* ss);
	typedef void (WINAPI *PPA_SetDeclare)(const char* ss);
	typedef void (WINAPI *PPA_SetDefProc)(const char* ss);
	typedef void (WINAPI *PPA_SetDefine) (const char* ss);
	typedef void (WINAPI *PPA_AddIntVar) (const char*, int, int);
	typedef void (WINAPI *PPA_AddStrVar) (const char*, const char*, int);
	typedef void (WINAPI *PPA_SetIntFunc)(void* p);
	typedef void (WINAPI *PPA_SetStrFunc)(void* p);
	typedef void (WINAPI *PPA_SetProc)   (void* p);
	typedef void (WINAPI *PPA_SetErrProc)(void* p);
	typedef void (WINAPI *PPA_Abort)     ();
	typedef int  (WINAPI *PPA_GetVersion)();
	typedef void (WINAPI *PPA_DeleteVar) (const char*);
	typedef int  (WINAPI *PPA_GetArgInt) (int);
	typedef char*(WINAPI *PPA_GetArgStr) (int);
	typedef char*(WINAPI *PPA_GetArgBStr)(int);
	typedef void (WINAPI *PPA_SetStrObj) (void* proc);
	typedef void (WINAPI *PPA_SetIntObj) (void* proc);
	typedef void (WINAPI *PPA_AddIntObj) (const char*, int, BOOL, int);
	typedef void (WINAPI *PPA_AddStrObj) (const char*, const char*, BOOL, int);
	typedef int  (WINAPI *PPA_GetIntVar) (const char* ss);
	typedef char*(WINAPI *PPA_GetStrVar) (const char* ss);
	typedef char*(WINAPI *PPA_GetBStrVar)(const char* ss);
	typedef BOOL (WINAPI *PPA_SetIntVar) (const char*, int);
	typedef BOOL (WINAPI *PPA_SetStrVar) (const char*, const char*);

	// 以下は PPA.DLL Version 1.20 で追加された関数 --
	#if PPADLL_VER >= 120
	typedef void   (WINAPI *PPA_AddRealVar)(const char*, double, BOOL);
	typedef void   (WINAPI *PPA_SetRealObj)(void* p);
	typedef void   (WINAPI *PPA_AddRealObj)(const char*, double, BOOL, LONG);
	typedef double (WINAPI *PPA_GetRealVar)(const char*);
	typedef BOOL   (WINAPI *PPA_SetRealVar)(const char*, double);
	typedef void   (WINAPI *PPA_SetRealFunc)(void* p);
	typedef DWORD  (WINAPI *PPA_GetArgReal)(int);
	#endif // PPADLL_VER >= 120

	// 以下は PPA.DLL Version 1.23 で追加された関数 --
	#if PPADLL_VER >= 123
	typedef BYTE (WINAPI *PPA_IsRunning)();
	typedef void (WINAPI *PPA_SetFinishProc)(void* p);	// 2003.06.01 Moca
	#endif // PPADLL_VER >= 123

	PPA_Execute    fnExecute;
	PPA_SetSource  fnSetSource;
	PPA_SetDeclare fnSetDeclare;
	PPA_SetDefProc fnSetDefProc;
	PPA_SetDefine  fnSetDefine;
	PPA_AddIntVar  fnAddIntVar;
	PPA_AddStrVar  fnAddStrVar;
	PPA_SetIntFunc fnSetIntFunc;
	PPA_SetStrFunc fnSetStrFunc;
	PPA_SetProc    fnSetProc;
	PPA_SetErrProc fnSetErrProc;
	PPA_Abort      fnAbort;
	PPA_GetVersion fnGetVersion;
	PPA_DeleteVar  fnDeleteVar;
	PPA_GetArgInt  fnGetArgInt;
	PPA_GetArgStr  fnGetArgStr;
	PPA_GetArgBStr fnGetArgBStr;
	PPA_SetStrObj  fnSetStrObj;
	PPA_SetIntObj  fnSetIntObj;
	PPA_AddIntObj  fnAddIntObj;
	PPA_AddStrObj  fnAddStrObj;
	PPA_GetIntVar  fnGetIntVar;
	PPA_GetStrVar  fnGetStrVar;
	PPA_GetBStrVar fnGetBStrVar;
	PPA_SetIntVar  fnSetIntVar;
	PPA_SetStrVar  fnSetStrVar;

#if PPADLL_VER >= 120
	PPA_AddRealVar  fnAddRealVar;
	PPA_SetRealObj  fnSetRealObj;
	PPA_AddRealObj  fnAddRealObj;
	PPA_GetRealVar  fnGetRealVar;
	PPA_SetRealVar  fnSetRealVar;
	PPA_SetRealFunc fnSetRealFunc;
	PPA_GetArgReal  fnGetArgReal;
#endif

#if PPADLL_VER >= 123
	PPA_IsRunning fnIsRunning;
	PPA_SetFinishProc fnSetFinishProc;	// 2003.06.01 Moca
#endif

public:
	// exported
	// 2007.07.22 genta : flags追加
	bool Execute(class EditView& editView, int flags);
	void SetSource(const char* ss)
		{ fnSetSource(ss); }
	void SetDeclare(const char* ss)
		{ fnSetDeclare(ss); }
	void SetDefProc(const char* ss)
		{ fnSetDefProc(ss); }
	void SetDefine(const char* ss)
		{ fnSetDefine(ss); }
	void AddIntVar(const char* lpszDef, int nVal, int nCnst)
		{ fnAddIntVar(lpszDef, nVal, nCnst); }
	void AddStrVar(const char* lpszDef, const char* lpszVal, int nCnst)
		{ fnAddStrVar(lpszDef, lpszVal, nCnst); }
	void SetIntFunc(void* proc)
		{ fnSetIntFunc(proc); }
	void SetStrFunc(void* proc)
		{ fnSetStrFunc(proc); }
	void SetProc(void* proc)
		{ fnSetProc(proc); }
	void SetErrProc(void* proc)
		{ fnSetErrProc(proc); }
	void Abort()
		{ fnAbort(); }
//	int  GetVersion()
//		{ return fnGetVersion(); }
	void DeleteVar(const char* ss)
		{ fnDeleteVar(ss); }
	int  GetArgInt(int index)
		{ return fnGetArgInt(index); }
	char* GetArgStr(int index)
		{ return fnGetArgStr(index); }
	char* GetArgBStr(int index)
		{ return fnGetArgBStr(index); }
	void SetStrObj(void* proc)
		{ fnSetStrObj(proc); }
	void SetIntObj(void* proc)
		{ fnSetIntObj(proc); }
	void AddIntObj(const char* ss, int def, BOOL read, int index)
		{ fnAddIntObj(ss, def, read, index); }
	void AddStrObj(const char* ss, const char* def, BOOL read, int index)
		{ fnAddStrObj(ss, def, read, index); }
	int  GetIntVar(const char* ss)
		{ return fnGetIntVar(ss); }
	char* GetStrVar(const char* ss)
		{ return fnGetStrVar(ss); }
	char* GetBStrVar(const char* ss)
		{ return fnGetBStrVar(ss); }
	BOOL SetIntVar(const char* ss, int val)
		{ return fnSetIntVar(ss, val); }
	BOOL SetStrVar(const char* ss, const char* val)
		{ return fnSetStrVar(ss, val); }

#if PPADLL_VER >= 120
	void AddRealVar(const char* ss, double val, BOOL cnst)
		{ fnAddRealVar(ss, val, cnst); }
	void SetRealObj(void* proc)
		{ fnSetRealObj(proc); }
	void AddRealObj(const char* ss, double val, BOOL read, LONG index)
		{ fnAddRealObj(ss, val, read, index); }
	double GetRealVar(const char* ss)
		{ return fnGetRealVar(ss); }
	BOOL SetRealVar(const char* ss, double val)
		{ return fnSetRealVar(ss, val); }
	void SetRealFunc(void* proc)
		{ fnSetRealFunc(proc); }
	DWORD GetArgReal(int index)
		{ return fnGetArgReal(index); }
#endif

#if PPADLL_VER >= 123
	BOOL IsRunning()
		{ return (BOOL)fnIsRunning(); }
	void SetFinishProc(void* proc)	// 2003.06.01 Moca
		{ fnSetFinishProc(proc); }
#endif

private:
	// コールバックプロシージャ群
	static void __stdcall stdStrObj(const char*, int, BYTE, int*, char**);	// 2003.06.01 Moca

	static void __stdcall stdProc(const char* FuncName, const int index, const char* Argument[], const int ArgSize, int* Err_CD);
	static void __stdcall stdIntFunc(const char* FuncName, const int index,
		const char* Argument[], const int ArgSize, int* Err_CD, int* ResultValue); // 2002.02.24 Moca
	static void __stdcall stdStrFunc(const char* FuncName, const int index, const char* Argument[], const int ArgSize, int* Err_CD, char** ResultValue);
	static bool CallHandleFunction(const int index, const char* Arg[], int ArgSize, VARIANT* Result); // 2002.02.24 Moca

	static void __stdcall stdError(int, const char*);	// 2003.06.01 Moca
	static void __stdcall stdFinishProc();	// 2003.06.01 Moca

	// メンバ変数
	char szMsg[80];		// PPAからのメッセージを保持する

	// 2007.07.26 genta : PPAのネストを許容するために，別データ構造とする．
	
	struct PpaExecInfo {
		NativeA			memRet;		// コールバックからDLLに渡す文字列を保持
		EditView*		pEditView;	// 2003.06.01 Moca
		DllSharedData*	pShareData;	// 2003.06.01 Moca
		bool			bError;		// エラーが2回表示されるのを防ぐ	2003.06.01 Moca
		NativeA			memDebug;	// デバッグ用変数UserErrorMes 2003.06.01 Moca
		/** オプションフラグ
		
			EditView::HandleCommand()にコマンドと一緒に渡すことで
			コマンドの素性を教える．
		*/
		int commandflags;	// 
	};
	// 2007.07.26 genta : 現在実行中のインスタンス
	static PpaExecInfo* curInstance;
	// PPAの多重起動禁止 2008.10.22 syat
	static bool bIsRunning;	// PPAが同時実行されるのを防ぐ

/*	関数名はCMacroが持つ。
	static struct MacroFuncInfo	S_Table[];
	static int nFuncNum;	// SAKURAエディタ用関数の数
*/
};

