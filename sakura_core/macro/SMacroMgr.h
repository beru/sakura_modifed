/*!	@file
	@brief キーボードマクロ(直接実行用)
*/
#pragma once

#include <Windows.h>
#include <WTypes.h> // VARTYPE

#include "MacroManagerBase.h"
#include "env/DllSharedData.h"
#include "config/maxdata.h"
#include "util/design_template.h"

class EditView;


const int STAND_KEYMACRO	= -1;	// 標準マクロ(キーマクロ)
const int TEMP_KEYMACRO		= -2;	// 一時マクロ(名前を指定してマクロ実行)
const int INVALID_MACRO_IDX	= -3;	// 無効なマクロのインデックス番号

struct MacroFuncInfoEx {
	int			nArgMinSize;
	int			nArgMaxSize;
	VARTYPE*	pVarArgEx;
};

// マクロ関数情報構造体
// 関数名はSMacroMgrが持つ
struct MacroFuncInfo {
	int				nFuncID;
	const wchar_t*	pszFuncName;
	VARTYPE			varArguments[4];	// 引数の型の配列
	VARTYPE			varResult;		// 戻り値の型 VT_EMPTYならprocedureということで
	MacroFuncInfoEx*	pData;
};
// マクロ関数情報構造体配列
typedef MacroFuncInfo* MacroFuncInfoArray;

class SMacroMgr {
	// データの型宣言
	MacroManagerBase* savedKeyMacros[MAX_CUSTMACRO];	// キーマクロをカスタムメニューの数だけ管理
	// キーマクロに標準マクロ以外のマクロを読み込めるように
	MacroManagerBase* pKeyMacro;	// 標準の（保存ができる）キーマクロも管理

	// 一時マクロ（名前を指定してマクロ実行）を管理
	MacroManagerBase* pTempMacro;

public:

	/*
	||  Constructors
	*/
	SMacroMgr();
	~SMacroMgr();

	/*
	||  Attributes & Operations
	*/
	void Clear(int idx);
	void ClearAll(void);	// キーマクロのバッファをクリアする

	// キーボードマクロの実行
	bool Exec(int idx, HINSTANCE hInstance, EditView& editView, int flags);
	
	//	実行可能か？CShareDataに問い合わせ
	bool IsEnabled(int idx) const {
		return (0 <= idx && idx < MAX_CUSTMACRO) ?
		pShareData->common.macro.macroTable[idx].IsEnabled() : false;
	}
	
	//	表示する名前の取得
	const TCHAR* GetTitle(int idx) const {
		return (0 <= idx && idx < MAX_CUSTMACRO) ?
		pShareData->common.macro.macroTable[idx].GetTitle() : NULL;
	}
	
	//	表示名の取得
	const TCHAR* GetName(int idx) const {
		return (0 <= idx && idx < MAX_CUSTMACRO) ?
		pShareData->common.macro.macroTable[idx].szName : NULL;
	}
	
	/*!	@brief ファイル名の取得
		@param idx [in] マクロ番号
	*/
	const TCHAR* GetFile(int idx) const {
		return (0 <= idx && idx < MAX_CUSTMACRO) ?
		pShareData->common.macro.macroTable[idx].szFile : 
		((idx == STAND_KEYMACRO || idx == TEMP_KEYMACRO) && sMacroPath != _T("")) ?
		sMacroPath.c_str() : NULL;
	}

	// キーボードマクロの読み込み
	bool Load(EditView& view, int idx, HINSTANCE hInstance, const TCHAR* pszPath, const TCHAR* pszType);
	bool Save(int idx, HINSTANCE hInstance, const TCHAR* pszPath);
	void UnloadAll(void);

	// キーマクロのバッファにデータ追加
	int Append(int idx, EFunctionCode nFuncID, const LPARAM* lParams, EditView& editView);

	/*
	||  Attributes & Operations
	*/
	static wchar_t* GetFuncInfoByID(HINSTANCE , int , wchar_t* , wchar_t*);	// 機能ID→関数名，機能名日本語
	static EFunctionCode GetFuncInfoByName(HINSTANCE , const wchar_t* , wchar_t*);	// 関数名→機能ID，機能名日本語
	static bool CanFuncIsKeyMacro(int);	// キーマクロに記録可能な機能かどうかを調べる
	
	// Jun. 16, 2002 genta
	static const MacroFuncInfo* GetFuncInfoByID(int);
	
	bool IsSaveOk(void);

	// Sep. 15, 2005 FILE	実行中マクロのインデックス番号操作 (INVALID_MACRO_IDX:無効)
	int GetCurrentIdx(void) const {
		return currentIdx;
	}
	int SetCurrentIdx(int idx) {
		int oldIdx = currentIdx;
		currentIdx = idx;
		return oldIdx;
	}

	MacroManagerBase* SetTempMacro(MacroManagerBase* newMacro);

private:
	DllSharedData*	pShareData;
	MacroManagerBase** Idx2Ptr(int idx);

	/*!	実行中マクロのインデックス番号 (INVALID_MACRO_IDX:無効) */
	int currentIdx;
	std::tstring sMacroPath;	// Loadしたマクロ名

public:
	static MacroFuncInfo	macroFuncInfoCommandArr[];	// コマンド情報(戻り値なし)
	static MacroFuncInfo	macroFuncInfoArr[];		// 関数情報(戻り値あり)

private:
	DISALLOW_COPY_AND_ASSIGN(SMacroMgr);
};

