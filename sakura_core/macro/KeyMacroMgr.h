/*!	@file
	@brief キーボードマクロ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, aroka
	Copyright (C) 2002, YAZAKI, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
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
	void Append(EFunctionCode, const LPARAM*, class EditView* pEditView);		// キーマクロのバッファにデータ追加
	void Append(class Macro* macro);		// キーマクロのバッファにデータ追加
	
	// キーボードマクロをまとめて取り扱う
	bool SaveKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath) const;	// Macroの列を、キーボードマクロに保存
	//@@@2002.2.2 YAZAKI PPA.DLLアリ/ナシ共存のためvirtualに。
	// 2007.07.20 genta flags追加
	virtual bool ExecKeyMacro(class EditView* pEditView, int flags) const;	// キーボードマクロの実行
	virtual bool LoadKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath);		// キーボードマクロをファイルから読み込む
	virtual bool LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* pszCode);	// キーボードマクロを文字列から読み込む
	
	// Apr. 29, 2002 genta
	static MacroManagerBase* Creator(const TCHAR* ext);
	static void declare(void);

protected:
	Macro*	m_pTop;	// 先頭と終端を保持
	Macro*	m_pBot;
};


