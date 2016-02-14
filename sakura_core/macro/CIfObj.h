/*!	@file
	@brief WSHインタフェースオブジェクト基本クラス

	@date 2009.10.29 syat CWSH.hから切り出し

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

#include <string>
#include <vector>
#include "_os/OleTypes.h"
class EditView;

// COM一般

template <class Base>
class ImplementsIUnknown : public Base {
private:
	int m_refCount;
	ImplementsIUnknown(const ImplementsIUnknown&);
	ImplementsIUnknown& operator = (const ImplementsIUnknown&);
public:
	#ifdef __BORLANDC__
	#pragma argsused
	#endif
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject) {
		return E_NOINTERFACE; 
	}
	virtual ULONG STDMETHODCALLTYPE AddRef() { ++ m_refCount; return m_refCount; }
	virtual ULONG STDMETHODCALLTYPE Release() { -- m_refCount; int R = m_refCount; if (m_refCount == 0) delete this; return R; }
public:
	ImplementsIUnknown(): m_refCount(0) {}
	virtual ~ImplementsIUnknown() {}
};

// WSH一般

class IfObj;
typedef HRESULT (IfObj::*CIfObjMethod)(int ID, DISPPARAMS* Arguments, VARIANT* Result, void* Data);

// IfObjが必要とするWSHClientのインタフェース
class IWSHClient {
public:
	virtual void* GetData() const = 0;
};

// スクリプトに渡されるオブジェクト

class IfObj : public ImplementsIUnknown<IDispatch> {
public:
	// 型定義
	struct MethodInfo {
		FUNCDESC		Desc;
		wchar_t			Name[64];
		CIfObjMethod	Method;
		ELEMDESC		Arguments[9];
		int				ID;
	};
	typedef std::vector<MethodInfo> CMethodInfoList;

	// コンストラクタ・デストラクタ
	IfObj(const wchar_t* name, bool isGlobal);
	virtual ~IfObj();

	// フィールド・アクセサ
	const std::wstring::value_type* Name() const { return this->m_sName.c_str(); } // インタフェースオブジェクト名
	bool IsGlobal() const { return this->m_isGlobal; } // オブジェクト名の省略可否
	IWSHClient* Owner() const { return this->m_Owner; } // オーナーIWSHClient
	std::wstring m_sName;
	bool m_isGlobal;
	IWSHClient *m_Owner;

	// 操作
	void AddMethod(const wchar_t* Name, int ID, VARTYPE *ArgumentTypes,
		int ArgumentCount, VARTYPE ResultType, CIfObjMethod Method);
	void ReserveMethods(int Count) {
		m_methods.reserve(Count);
	}

	// 実装
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject);
	virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(
					REFIID riid,
					OLECHAR FAR* FAR* rgszNames,
					UINT cNames,
					LCID lcid,
					DISPID FAR* rgdispid);
	virtual HRESULT STDMETHODCALLTYPE Invoke(
					DISPID dispidMember,
					REFIID riid,
					LCID lcid,
					WORD wFlags,
					DISPPARAMS FAR* pdispparams,
					VARIANT FAR* pvarResult,
					EXCEPINFO FAR* pexcepinfo,
					UINT FAR* puArgErr);
	virtual HRESULT STDMETHODCALLTYPE GetTypeInfo(
					/* [in] */ UINT iTInfo,
					/* [in] */ LCID lcid,
					/* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
	virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount(
					/* [out] */ UINT __RPC_FAR *pctinfo);

private:
	// メンバ変数
	CMethodInfoList m_methods;			// メソッド情報リスト
	ITypeInfo* m_typeInfo;
};

