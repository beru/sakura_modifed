/*!	@file
	@brief DLLプラグインI/F
*/
#pragma once

#include <Windows.h>
#include <OleAuto.h>
#include <list>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PRAGMA_PUSH4
#pragma pack(push, 4)
#endif

/*
	構造体のバージョン
		0x00010000=1.0

	  History:
	  0x00010000	初期バージョン(sakura2, UNICODE)
*/
#define SAKURA_DLL_PLUGIN_VERSION	0x00010000	// 構造体のバージョン

typedef BOOL (WINAPI *HandleFunctionCallback)(LPCWSTR lpszName, LPVOID lpIfObj, LPVOID lpEditView, const DWORD ID, const VARIANT* Arguments, const int ArgSize, VARIANT* Result);
typedef void (WINAPI *HandleCommandCallback)(LPCWSTR lpszName, LPVOID lpIfObj, LPVOID lpEditView, const DWORD ID, LPCWSTR Arguments[], const int* ArgLengths, const int ArgSize);

/*
	マクロ定義情報
*/
typedef struct tagMACRO_FUNC_INFO_EX {
	int					nArgSize;			// 総引数数
	VARTYPE*			lpVarArgEx;		// VARTYPE配列へのポインタ
} MACRO_FUNC_INFO_EX;

typedef struct tagMACRO_FUNC_INFO {
	DWORD				nFuncID;			// 機能ID
	LPCWSTR				lpszFuncName;		// 関数名
	VARTYPE				varArguments[4];	// 引数
	VARTYPE				varResult;		// 戻り値
	MACRO_FUNC_INFO_EX*	lpExData;			// 5個目以降の引数情報
} MACRO_FUNC_INFO;

typedef struct tagSAKURA_DLL_PLUGIN_IF_OBJ {
	wchar_t				szName[64];			// 識別子
	LPVOID				lpIfObj;				// プラグイン情報
	MACRO_FUNC_INFO*	pFunctionInfo;			// マクロ関数情報
	MACRO_FUNC_INFO*	pCommandInfo;			// マクロコマンド情報
} SAKURA_DLL_PLUGIN_IF_OBJ;

/*
	サクラエディタからDLLプラグインに渡される構造体(エディタ→プラグイン)
	この構造体を修正した場合はCBasePluginInitialize::Copy()を修正すること。
*/
typedef struct tagSAKURA_DLL_PLUGIN_OBJ {
	DWORD						dwVersion;			// DLLプラグイン構造体識別バージョン
	DWORD						dwVersionMS;			// sakuraバージョン(MS)
	DWORD						dwVersionLS;			// sakuraバージョン(LS)
	DWORD						dwVersionShare;		// 共有メモリバージョン
	LANGID						wLangId;				// 言語ID
	WORD						wPadding1;			// 予備(パディング)
	HWND						hParentHwnd;			// 親ウィンドウハンドル
	LPVOID						lpDllPluginObj;		// プラグイン情報(CWSHIfObj::List)
	LPVOID						lpEditView;			// EditView情報
	HandleFunctionCallback		fnFunctionHandler;	// 関数ハンドラ
	HandleCommandCallback		fnCommandHandler;		// コマンドハンドラ
	SAKURA_DLL_PLUGIN_IF_OBJ*	IfObjList;			// プラグイン情報
	DWORD						dwIfObjListCount;		// プラグイン情報個数
	DWORD						wPadding2;			// 予備(パディング)
	LPVOID						lpUserData[4];		// ユーザ情報
	DWORD						dwReserve[8];			// 予備
// TODO: 構造体を拡張するときはDLL_PLUGIN_INFO_VERSIONを変更しifdefで拡張する
} SAKURA_DLL_PLUGIN_OBJ;

// DLLプラグインAPI
typedef void (WINAPI *DllPlugHandler)(SAKURA_DLL_PLUGIN_OBJ* obj);

#ifdef PRAGMA_PUSH4
#pragma pack(pop)
#endif

#ifdef __cplusplus
}
#endif

