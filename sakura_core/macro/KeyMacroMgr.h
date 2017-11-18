/*!	@file
	@brief キーボードマクロ
*/

#pragma once

#include <Windows.h>
#include "MacroManagerBase.h"
#include "Funccode_enum.h"

class Macro;

//#define MAX_STRLEN			70
//#define MAX_KEYMACRONUM		10000
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
// キーボードマクロ
/*!
	キーボードマクロクラス
*/
class KeyMacroMgr : public MacroManagerBase {
public:
	/*
	||  Constructors
	*/
	KeyMacroMgr();
	~KeyMacroMgr();

	/*
	||  Attributes & Operations
	*/
	void ClearAll(void);				// キーマクロのバッファをクリアする
	void Append(EFunctionCode, const LPARAM*, class EditView& editView);		// キーマクロのバッファにデータ追加
	void Append(class Macro* macro);		// キーマクロのバッファにデータ追加
	
	// キーボードマクロをまとめて取り扱う
	bool SaveKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath) const;	// Macroの列を、キーボードマクロに保存
	// PPA.DLLアリ/ナシ共存のためvirtualに。
	virtual bool ExecKeyMacro(class EditView& editView, int flags) const;	// キーボードマクロの実行
	virtual bool LoadKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath);		// キーボードマクロをファイルから読み込む
	virtual bool LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* pszCode);	// キーボードマクロを文字列から読み込む
	
	// Apr. 29, 2002 genta
	static MacroManagerBase* Creator(class EditView& view, const TCHAR* ext);
	static void Declare(void);

protected:
	Macro*	pTop;	// 先頭と終端を保持
	Macro*	pBot;
};


