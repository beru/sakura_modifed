/*!	@file
	@brief PPA.DLLマクロ

	@author YAZAKI
	@date 2002年1月26日
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
#include "KeyMacroMgr.h"
#include "PPA.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
// PPAマクロ
class PPAMacroMgr: public MacroManagerBase {
public:
	/*
	||  Constructors
	*/
	PPAMacroMgr();
	~PPAMacroMgr();

	/*
	||	PPA.DLLに委譲する部分
	*/
	virtual bool ExecKeyMacro(class EditView& editView, int flags) const;	// PPAマクロの実行
	virtual bool LoadKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath);		// キーボードマクロをファイルから読み込み、CMacroの列に変換
	virtual bool LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* pszCode);	// キーボードマクロを文字列から読み込み、CMacroの列に変換

	static class PPA cPPA;

	// Apr. 29, 2002 genta
	static MacroManagerBase* Creator(EditView& view, const TCHAR* ext);
	static void Declare(void);

protected:
	NativeW buffer;
};

