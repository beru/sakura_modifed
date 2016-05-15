/*!	@file
	@brief WSH Handler

	@author 鬼
	@date 2002年4月28日,5月3日,5月5日,5月6日,5月13日,5月16日
	@date 2002.08.25 genta リンクエラー回避のためWSHManager.hにエディタの
		マクロインターフェース部を分離．
	@date 2009.10.29 syat インタフェースオブジェクト部分をWSHIfObj.hに分離
*/
/*
	Copyright (C) 2002, 鬼, genta
	Copyright (C) 2009, syat

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.

*/

#pragma once

#include <ActivScp.h>
// ↑Microsoft Platform SDK より
#include "macro/IfObj.h"

/* 2009.10.29 syat インタフェースオブジェクト部分をWSHIfObj.hに分離
template <class Base>
class ImplementsIUnknown: public Base

class InterfaceObject: public ImplementsIUnknown<IDispatch>
 */
typedef void (*ScriptErrorHandler)(BSTR Description, BSTR Source, void *Data);

class WSHClient : IWSHClient {
public:
	// 型定義
	typedef std::vector<IfObj*> List;      // 所有しているインタフェースオブジェクトのリスト
	typedef List::const_iterator ListIter;	// そのイテレータ

	// コンストラクタ・デストラクタ
	WSHClient(const wchar_t* AEngine, ScriptErrorHandler AErrorHandler, void* AData);
	~WSHClient();

	// フィールド・アクセサ
	ScriptErrorHandler onError;
	void* data;
	bool isValid; ///< trueの場合スクリプトエンジンが使用可能。falseになる場合は ScriptErrorHandlerにエラー内容が通知されている。
	virtual /*override*/ void* GetData() const { return this->data; }
	const List& GetInterfaceObjects() {	return this->ifObjArr; }

	// 操作
	void AddInterfaceObject(IfObj* obj);
	bool Execute(const wchar_t* AScript);
	void Error(BSTR Description, BSTR Source); ///< ScriptErrorHandlerを呼び出す。
	void Error(const wchar_t* Description);          ///< ScriptErrorHandlerを呼び出す。

private:
	IActiveScript* engine;
	List ifObjArr;
};

