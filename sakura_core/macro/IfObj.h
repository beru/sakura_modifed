/*!	@file
	@brief WSHインタフェースオブジェクト基本クラス
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
	int refCount;
	ImplementsIUnknown(const ImplementsIUnknown&);
	ImplementsIUnknown& operator = (const ImplementsIUnknown&);
public:
	#ifdef __BORLANDC__
	#pragma argsused
	#endif
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject) {
		return E_NOINTERFACE; 
	}
	virtual ULONG STDMETHODCALLTYPE AddRef() { ++ refCount; return refCount; }
	virtual ULONG STDMETHODCALLTYPE Release() { -- refCount; int r = refCount; if (refCount == 0) delete this; return r; }
public:
	ImplementsIUnknown(): refCount(0) {}
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
	typedef std::vector<MethodInfo> MethodInfoList;

	// コンストラクタ・デストラクタ
	IfObj(const wchar_t* name, bool isGlobal);
	virtual ~IfObj();

	// フィールド・アクセサ
	const std::wstring::value_type* Name() const { return this->name.c_str(); } // インタフェースオブジェクト名
	bool IsGlobal() const { return this->isGlobal; } // オブジェクト名の省略可否
	IWSHClient* Owner() const { return this->owner; } // オーナーIWSHClient
	std::wstring name;
	bool isGlobal;
	IWSHClient* owner;

	// 操作
	void AddMethod(const wchar_t* Name, int ID, VARTYPE *ArgumentTypes,
		int ArgumentCount, VARTYPE ResultType, CIfObjMethod Method);
	void ReserveMethods(int Count) {
		methods.reserve(Count);
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
	MethodInfoList methods;			// メソッド情報リスト
	ITypeInfo* typeInfo;
};

