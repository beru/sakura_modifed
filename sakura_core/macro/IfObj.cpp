/*!	@file
	@brief WSHインタフェースオブジェクト基本クラス
*/
#include "StdAfx.h"
#include "macro/IfObj.h"

// トレースメッセージ有無
#if defined(_DEBUG) && defined(_UNICODE)
#define TEST
#endif

/////////////////////////////////////////////
// スクリプトに渡されるオブジェクトの型情報
class IfObjTypeInfo: public ImplementsIUnknown<ITypeInfo> {
private:
	const IfObj::MethodInfoList& methodsRef;
	const std::wstring& name;
	TYPEATTR typeAttr;
public:
	IfObjTypeInfo(const IfObj::MethodInfoList& methods, const std::wstring& sName);

	virtual HRESULT STDMETHODCALLTYPE GetTypeAttr(
					/* [out] */ TYPEATTR __RPC_FAR *__RPC_FAR *ppTypeAttr)
	{
#ifdef TEST
		DEBUG_TRACE(_T("GetTypeAttr\n"));
#endif
		*ppTypeAttr = &typeAttr;
		return S_OK;
	}
        
	virtual HRESULT STDMETHODCALLTYPE GetTypeComp(
					/* [out] */ ITypeComp __RPC_FAR *__RPC_FAR *ppTComp)
	{
#ifdef TEST
		DEBUG_TRACE(_T("GetTypeComp\n"));
#endif
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetFuncDesc(
				/* [in] */ UINT index,
				/* [out] */ FUNCDESC __RPC_FAR *__RPC_FAR *ppFuncDesc);

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetVarDesc(
	    /* [in] */ UINT index,
	    /* [out] */ VARDESC __RPC_FAR *__RPC_FAR *ppVarDesc)
	{
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetNames(
	    /* [in] */ MEMBERID memid,
	    /* [length_is][size_is][out] */ BSTR __RPC_FAR *rgBstrNames,
	    /* [in] */ UINT cMaxNames,
	    /* [out] */ UINT __RPC_FAR *pcNames);

	virtual HRESULT STDMETHODCALLTYPE GetRefTypeOfImplType(
	    /* [in] */ UINT index,
	    /* [out] */ HREFTYPE __RPC_FAR *pRefType)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE GetImplTypeFlags(
	    /* [in] */ UINT index,
	    /* [out] */ INT __RPC_FAR *pImplTypeFlags)
	{
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetIDsOfNames(
	    /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
	    /* [in] */ UINT cNames,
	    /* [size_is][out] */ MEMBERID __RPC_FAR *pMemId)
	{
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE Invoke(
	    /* [in] */ PVOID pvInstance,
	    /* [in] */ MEMBERID memid,
	    /* [in] */ WORD wFlags,
	    /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
	    /* [out] */ VARIANT __RPC_FAR *pVarResult,
	    /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
	    /* [out] */ UINT __RPC_FAR *puArgErr)
	{
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetDocumentation(
	    /* [in] */ MEMBERID memid,
	    /* [out] */ BSTR __RPC_FAR *pBstrName,
	    /* [out] */ BSTR __RPC_FAR *pBstrDocString,
	    /* [out] */ DWORD __RPC_FAR *pdwHelpContext,
	    /* [out] */ BSTR __RPC_FAR *pBstrHelpFile)
	{
		// とりあえず全部NULLを返す (情報無し)
		// 各パラメータを設定するように
		if (memid == -1) {
			if (pBstrName) {
				*pBstrName = SysAllocString( name.c_str() );
			}
		}else if (0 <= memid && memid < (int)methodsRef.size()) {
			if (pBstrName) {
				*pBstrName = SysAllocString( methodsRef[memid].Name );
			}
		}else {
			return TYPE_E_ELEMENTNOTFOUND;
		}
		if (pBstrDocString) {
			*pBstrDocString = SysAllocString(L"");
		}
		if (pdwHelpContext) {
			*pdwHelpContext = 0;
		}
		if (pBstrHelpFile) {
			*pBstrHelpFile = SysAllocString(L"");
		}
		return S_OK ;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetDllEntry(
	    /* [in] */ MEMBERID memid,
	    /* [in] */ INVOKEKIND invKind,
	    /* [out] */ BSTR __RPC_FAR *pBstrDllName,
	    /* [out] */ BSTR __RPC_FAR *pBstrName,
	    /* [out] */ WORD __RPC_FAR *pwOrdinal)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE GetRefTypeInfo(
	    /* [in] */ HREFTYPE hRefType,
	    /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo)
	{
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE AddressOfMember(
	    /* [in] */ MEMBERID memid,
	    /* [in] */ INVOKEKIND invKind,
	    /* [out] */ PVOID __RPC_FAR *ppv)
	{
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE CreateInstance(
	    /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
	    /* [in] */ REFIID riid,
	    /* [iid_is][out] */ PVOID __RPC_FAR *ppvObj)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE GetMops(
	    /* [in] */ MEMBERID memid,
	    /* [out] */ BSTR __RPC_FAR *pBstrMops)
	{
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetContainingTypeLib(
	    /* [out] */ ITypeLib __RPC_FAR *__RPC_FAR *ppTLib,
	    /* [out] */ UINT __RPC_FAR *pIndex)
	{
		return E_NOTIMPL;
	}

	virtual /* [local] */ void STDMETHODCALLTYPE ReleaseTypeAttr(
					/* [in] */ TYPEATTR __RPC_FAR *pTypeAttr)
	{
	}

	virtual /* [local] */ void STDMETHODCALLTYPE ReleaseFuncDesc(
					/* [in] */ FUNCDESC __RPC_FAR *pFuncDesc)
	{
	}

	virtual /* [local] */ void STDMETHODCALLTYPE ReleaseVarDesc(
				/* [in] */ VARDESC __RPC_FAR *pVarDesc)
	{
	}
};

IfObjTypeInfo::IfObjTypeInfo(const IfObj::MethodInfoList& methods,
							   const std::wstring& sName)
	:
	ImplementsIUnknown<ITypeInfo>(),
	methodsRef(methods),
	name(sName)
{ 
	ZeroMemory(&typeAttr, sizeof(typeAttr));
	typeAttr.cImplTypes = 0; // 親クラスのITypeInfoの数
	typeAttr.cFuncs = (WORD)methodsRef.size();
}

HRESULT STDMETHODCALLTYPE IfObjTypeInfo::GetFuncDesc(
			/* [in] */ UINT index,
			/* [out] */ FUNCDESC __RPC_FAR *__RPC_FAR *ppFuncDesc)
{
#ifdef TEST
	DEBUG_TRACE(_T("GetFuncDesc\n"));
#endif
	*ppFuncDesc = const_cast<FUNCDESC __RPC_FAR *>(&(methodsRef[index].Desc));
	return S_OK;
}

HRESULT STDMETHODCALLTYPE IfObjTypeInfo::GetNames(
    /* [in] */ MEMBERID memid,
    /* [length_is][size_is][out] */ BSTR __RPC_FAR *rgBstrNames,
    /* [in] */ UINT cMaxNames,
    /* [out] */ UINT __RPC_FAR *pcNames)
{
#ifdef TEST
		DEBUG_TRACE(_T("GetNames\n"));
#endif
	*pcNames = 1;
	if (cMaxNames > 0)
		*rgBstrNames = SysAllocString(methodsRef[memid].Name);
	return S_OK;
}


/////////////////////////////////////////////
// インタフェースオブジェクト

// コンストラクタ
IfObj::IfObj(const wchar_t* name, bool isGlobal)
	:
	ImplementsIUnknown<IDispatch>(),
	name(name),
	isGlobal(isGlobal),
	owner(0),
	methods(),
	typeInfo(nullptr)
{ 
};

// デストラクタ
IfObj::~IfObj()
{
	if (typeInfo) {
		typeInfo->Release();
	}
}
	
// IUnknown実装
HRESULT STDMETHODCALLTYPE IfObj::QueryInterface(REFIID iid, void ** ppvObject) 
{
	if (!ppvObject) {
		return E_POINTER;
	}else if (IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, IID_IDispatch)) {
		AddRef();
		*ppvObject = this;
		return S_OK;
	}else {
		return E_NOINTERFACE;
	}
}

// IDispatch実装
HRESULT STDMETHODCALLTYPE IfObj::Invoke(
	DISPID dispidMember,
	REFIID riid,
	LCID lcid,
	WORD wFlags,
	DISPPARAMS FAR* pdispparams,
	VARIANT FAR* pvarResult,
	EXCEPINFO FAR* pexcepinfo,
	UINT FAR* puArgErr
	)
{
	if ((unsigned)dispidMember < methods.size()) {
		return (this->* (methods[dispidMember].Method))(
			methods[dispidMember].ID,
			pdispparams,
			pvarResult,
			owner->GetData()
		);
	}else {
		return E_UNEXPECTED;
	}
}

HRESULT STDMETHODCALLTYPE IfObj::GetTypeInfo(
	/* [in] */ UINT iTInfo,
	/* [in] */ LCID lcid,
	/* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo)
{
	if (!typeInfo) {
		typeInfo = new IfObjTypeInfo(this->methods, this->name);
		typeInfo->AddRef();
	}
	(*ppTInfo) = typeInfo;
	(*ppTInfo)->AddRef();
	return S_OK;
}

HRESULT STDMETHODCALLTYPE IfObj::GetTypeInfoCount(
	/* [out] */ UINT __RPC_FAR *pctinfo)
{
	*pctinfo = 1;
	return S_OK;
}

#ifdef __BORLANDC__
#pragma argsused
#endif
HRESULT STDMETHODCALLTYPE IfObj::GetIDsOfNames(
	REFIID riid,
	OLECHAR FAR* FAR* rgszNames,
	UINT cNames,
	LCID lcid,
	DISPID FAR* rgdispid)
{
	for (unsigned i=0; i<cNames; ++i) {
#ifdef TEST
		// 大量にメッセージが出るので注意。
		//DEBUG_TRACE(_T("GetIDsOfNames: %ls\n"), rgszNames[i]);
#endif
		size_t nSize = methods.size();
		for (size_t j=0; j<nSize; ++j) {
			// Nov. 10, 2003 FILE Win9Xでは、[lstrcmpiW]が無効のため、[_wcsicmp]に修正
			if (_wcsicmp(rgszNames[i], methods[j].Name) == 0) {
				rgdispid[i] = j;
				goto Found;
			}
		}
		return DISP_E_UNKNOWNNAME;
		Found:
		;
	}
	return S_OK;
}

// 型情報にメソッドを追加する
void IfObj::AddMethod(
	const wchar_t*	Name,
	int				ID,
	VARTYPE*		ArgumentTypes,
	int				ArgumentCount,
	VARTYPE			ResultType,
	CIfObjMethod	Method
)
{
	// this->typeInfoが nullptr でなければ AddMethod()は反映されない。
	methods.emplace_back();
	MethodInfo* info = &methods[methods.size() - 1];
	ZeroMemory(info, sizeof(MethodInfo));
	info->Desc.invkind = INVOKE_FUNC;
	info->Desc.cParams = (SHORT)ArgumentCount + 1; // 戻り値の分
	info->Desc.lprgelemdescParam = info->Arguments;
	// Nov. 10, 2003 FILE Win9Xでは、[lstrcpyW]が無効のため、[wcscpy]に修正
	assert(auto_strlen(Name) < _countof(info->Name));
	wcscpy(info->Name, Name);
	info->Method = Method;
	info->ID = ID;
	for (int i=0; i<ArgumentCount; ++i) {
		info->Arguments[i].tdesc.vt = ArgumentTypes[ArgumentCount - i - 1];
		info->Arguments[i].paramdesc.wParamFlags = PARAMFLAG_FIN;
	}
	info->Arguments[ArgumentCount].tdesc.vt = ResultType;
	info->Arguments[ArgumentCount].paramdesc.wParamFlags = PARAMFLAG_FRETVAL;
}

