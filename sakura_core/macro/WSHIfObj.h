/*!	@file
	@brief WSHインタフェースオブジェクト基本クラス
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

