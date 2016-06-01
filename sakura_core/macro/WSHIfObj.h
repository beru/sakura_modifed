/*!	@file
	@brief WSHインタフェースオブジェクト基本クラス

	@date 2009.10.29 syat WSH.hから切り出し

*/
/*
	Copyright (C) 2002, 鬼, genta
	Copyright (C) 2009, syat

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
#pragma once

#include <list>
#include <ActivScp.h>
#include "_os/OleTypes.h"
#include "macro/IfObj.h"
#include "macro/WSH.h" // CWSHClient::List, ListIter
#include "macro/SMacroMgr.h" // MacroFuncInfo, MacroFuncInfoArray
class EditView;

/* WSHIfObj - プラグインやマクロに公開するオブジェクト
 * 使用上の注意:
 *   1. 生成はnewで。
 *      参照カウンタを持つので、自動変数で生成するとスコープ抜けて解放されるときにヒープエラーが出ます。
 *   2. 生成したらAddRef()、不要になったらRelease()を呼ぶこと。
 *   3. 新しいIfObjを作る時はWSHIfObjを継承し、以下の4つをオーバーライドすること。
 *      GetMacroCommandInfo, GetMacroFuncInfo, HandleCommand, HandleFunction
 */
class WSHIfObj : public IfObj {
public:
	// 型定義
	typedef std::list<WSHIfObj*> List;
	typedef List::const_iterator ListIter;

	// コンストラクタ
	WSHIfObj(const wchar_t* name, bool isGlobal)
		:
		IfObj(name, isGlobal)
	{
	}

	virtual void ReadyMethods(EditView& view, int flags);

protected:
	// 操作
	//	2007.07.20 genta : flags追加
	//  2009.09.05 syat CWSHManagerから移動
	void ReadyCommands(MacroFuncInfo* info, int flags);
	HRESULT MacroCommand(int index, DISPPARAMS* arguments, VARIANT* result, void* data);

	// 非実装提供
	virtual bool HandleFunction(EditView& view, EFunctionCode index, const VARIANT* arguments, const int argSize, VARIANT& result) = 0;		// 関数を処理する
	virtual bool HandleCommand(EditView& view, EFunctionCode index, const wchar_t* arguments[], const int argLengths[], const int argSize) = 0;	// コマンドを処理する
	virtual MacroFuncInfoArray GetMacroCommandInfo() const = 0;	// コマンド情報を取得する
	virtual MacroFuncInfoArray GetMacroFuncInfo() const = 0;	// 関数情報を取得する

	EditView* pView;
};

