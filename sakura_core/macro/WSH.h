/*!	@file
	@brief WSH Handler
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

