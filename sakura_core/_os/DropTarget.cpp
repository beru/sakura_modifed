/*!	@file
	@brief Drag & Drop

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani, Yebisuya Sugoroku
	Copyright (C) 2002, aroka
	Copyright (C) 2008, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "DropTarget.h"
#include "window/EditWnd.h"	// 2008.06.20 ryoji
#include "view/EditView.h"// 2002/2/3 aroka
#include "_main/global.h"
#include "Clipboard.h"

OleLibrary CYbInterfaceBase::olelib;

CYbInterfaceBase::CYbInterfaceBase()
{
	olelib.Initialize();
	return;
}

CYbInterfaceBase::~CYbInterfaceBase()
{
	olelib.UnInitialize();
	return;
}

HRESULT CYbInterfaceBase::QueryInterfaceImpl(
	IUnknown*	pThis,
	REFIID		owniid,
	REFIID		riid,
	void**		ppvObj
	)
{
	if (riid == IID_IUnknown || riid == owniid) {
		pThis->AddRef();
		*ppvObj = pThis;
		return S_OK;
	}
	*ppvObj = NULL;
	return E_NOINTERFACE;
}

/////////////////////////////////////////

OleLibrary::OleLibrary()//:dwCount(0)	// 2009.01.08 ryoji dwCount削除
{
	return;
}

OleLibrary::~OleLibrary()
{
// 2009.01.08 ryoji OleUninitialize削除（WinMainにOleInitialize/OleUninitialize追加）
//	if (dwCount > 0)
//		::OleUninitialize();
	return;
}


void OleLibrary::Initialize()
{
// 2009.01.08 ryoji OleInitialize削除（WinMainにOleInitialize/OleUninitialize追加）
//	if (dwCount++ == 0)
//		::OleInitialize(NULL);
	return;
}

void OleLibrary::UnInitialize()
{
// 2009.01.08 ryoji OleUninitialize削除（WinMainにOleInitialize/OleUninitialize追加）
//	if (dwCount > 0 && --dwCount == 0)
//		::OleUninitialize();
	return;
}


#define DECLARE_YB_INTERFACEIMPL(BASEINTERFACE) \
template <> REFIID CYbInterfaceImpl<BASEINTERFACE>::owniid = IID_##BASEINTERFACE;

DECLARE_YB_INTERFACEIMPL(IDataObject)
DECLARE_YB_INTERFACEIMPL(IDropSource)
DECLARE_YB_INTERFACEIMPL(IDropTarget)
DECLARE_YB_INTERFACEIMPL(IEnumFORMATETC)


DropTarget::DropTarget(EditWnd* pEditWnd)
{
	this->pEditWnd = pEditWnd;	// 2008.06.20 ryoji
	this->pEditView = nullptr;
	hWnd_DropTarget = NULL;
	return;
}

DropTarget::DropTarget(EditView* pEditView)
{
	this->pEditWnd = nullptr;	// 2008.06.20 ryoji
	this->pEditView = pEditView;
	hWnd_DropTarget = NULL;
	return;
}


DropTarget::~DropTarget()
{
	Revoke_DropTarget();
	return;
}


BOOL DropTarget::Register_DropTarget(HWND hWnd)
{
	if (FAILED(::RegisterDragDrop(hWnd, this))) {
		TopWarningMessage(hWnd, LS(STR_ERR_DLGDRPTGT1));
		return FALSE;
	}
	hWnd_DropTarget = hWnd;
	return TRUE;
}


BOOL DropTarget::Revoke_DropTarget(void)
{
	BOOL bResult = TRUE;
	if (hWnd_DropTarget) {
		bResult = SUCCEEDED(::RevokeDragDrop(hWnd_DropTarget));
		hWnd_DropTarget = NULL;
	}
	return bResult;
}

STDMETHODIMP DropTarget::DragEnter(
	LPDATAOBJECT pDataObject,
	DWORD dwKeyState,
	POINTL pt,
	LPDWORD pdwEffect
	)
{
	DEBUG_TRACE(_T("DropTarget::DragEnter()\n"));
	if (pEditWnd) {	// 2008.06.20 ryoji
		return pEditWnd->DragEnter(pDataObject, dwKeyState, pt, pdwEffect);
	}
	return pEditView->DragEnter(pDataObject, dwKeyState, pt, pdwEffect);
}

STDMETHODIMP DropTarget::DragOver(DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect)
{
	if (pEditWnd) {	// 2008.06.20 ryoji
		return pEditWnd->DragOver(dwKeyState, pt, pdwEffect);
	}
	return pEditView->DragOver(dwKeyState, pt, pdwEffect);
}

STDMETHODIMP DropTarget::DragLeave(void)
{
	if (pEditWnd) {	// 2008.06.20 ryoji
		return pEditWnd->DragLeave();
	}
	return pEditView->DragLeave();
}


STDMETHODIMP DropTarget::Drop(
	LPDATAOBJECT pDataObject,
	DWORD dwKeyState,
	POINTL pt,
	LPDWORD pdwEffect
	)
{
	if (pEditWnd) {	// 2008.06.20 ryoji
		return pEditWnd->Drop(pDataObject, dwKeyState, pt, pdwEffect);
	}
	return pEditView->Drop(pDataObject, dwKeyState, pt, pdwEffect);
}


STDMETHODIMP DropSource::QueryContinueDrag(BOOL bEscapePressed, DWORD dwKeyState)
{
	if (bEscapePressed || (dwKeyState & (bLeft ? MK_RBUTTON : MK_LBUTTON))) {
		return DRAGDROP_S_CANCEL;
	}
	if (!(dwKeyState & (bLeft ? MK_LBUTTON : MK_RBUTTON))) {
		return DRAGDROP_S_DROP;
	}
	return S_OK;
}

STDMETHODIMP DropSource::GiveFeedback(DWORD dropEffect)
{
	return DRAGDROP_S_USEDEFAULTCURSORS;
}


/** 転送対象の文字列を設定する
	@param lpszText [in] 文字列
	@param nTextLen [in] pszTextの長さ
	@param bColumnSelect [in] 矩形選択か

	@date 2008.03.26 ryoji 複数フォーマット対応
*/
void DataObject::SetText(LPCWSTR lpszText, int nTextLen, BOOL bColumnSelect)
{
	// Feb. 26, 2001, fixed by yebisuya sugoroku
	if (pData) {
		for (int i=0; i<nFormat; ++i) {
			delete [](pData[i].data);
		}
		delete []pData;
		pData = nullptr;
		nFormat = 0;
	}
	if (lpszText) {
		nFormat = bColumnSelect? 4: 3;	// 矩形を含めるか
		pData = new DATA[nFormat];

		int i = 0;
		pData[0].cfFormat = CF_UNICODETEXT;
		pData[0].size = (nTextLen + 1) * sizeof(wchar_t);
		pData[0].data = new BYTE[pData[0].size];
		memcpy_raw(pData[0].data, lpszText, nTextLen * sizeof(wchar_t));
		*((wchar_t*)pData[0].data + nTextLen) = L'\0';

		++i;
		pData[i].cfFormat = CF_TEXT;
		pData[i].size = ::WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)pData[0].data, pData[0].size/sizeof(wchar_t), NULL, 0, NULL, NULL);
		pData[i].data = new BYTE[pData[i].size];
		::WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)pData[0].data, pData[0].size/sizeof(wchar_t), (LPSTR)pData[i].data, pData[i].size, NULL, NULL);

		++i;
		pData[i].cfFormat = Clipboard::GetSakuraFormat();
		pData[i].size = sizeof(int) + nTextLen * sizeof(wchar_t);
		pData[i].data = new BYTE[pData[i].size];
		*(int*)pData[i].data = nTextLen;
		memcpy_raw(pData[i].data + sizeof(int), lpszText, nTextLen * sizeof(wchar_t));

		++i;
		if (bColumnSelect) {
			pData[i].cfFormat = (CLIPFORMAT)::RegisterClipboardFormat(_T("MSDEVColumnSelect"));
			pData[i].size = 1;
			pData[i].data = new BYTE[1];
			pData[i].data[0] = '\0';
		}
	}
}

DWORD DataObject::DragDrop(BOOL bLeft, DWORD dwEffects)
{
	DWORD dwEffect;
	DropSource drop(bLeft);
	if (SUCCEEDED(::DoDragDrop(this, &drop, dwEffects, &dwEffect))) {
		return dwEffect;
	}
	return DROPEFFECT_NONE;
}

/** IDataObject::GetData
	@date 2008.03.26 ryoji 複数フォーマット対応
*/
STDMETHODIMP DataObject::GetData(LPFORMATETC lpfe, LPSTGMEDIUM lpsm)
{
	// Feb. 26, 2001, fixed by yebisuya sugoroku
	if (!lpfe || !lpsm)
		return E_INVALIDARG;
	if (!pData)
		return OLE_E_NOTRUNNING;
	if (lpfe->lindex != -1)
		return DV_E_LINDEX;
	if ((lpfe->tymed & TYMED_HGLOBAL) == 0)
		return DV_E_TYMED;
	if (lpfe->dwAspect != DVASPECT_CONTENT)
		return DV_E_DVASPECT;
	if (!(lpfe->tymed & TYMED_HGLOBAL)
		|| lpfe->lindex != -1
		|| lpfe->dwAspect != DVASPECT_CONTENT)
		return DV_E_FORMATETC;

	int i;
	for (i=0; i<nFormat; ++i) {
		if (lpfe->cfFormat == pData[i].cfFormat)
			break;
	}
	if (i == nFormat)
		return DV_E_FORMATETC;

	lpsm->tymed = TYMED_HGLOBAL;
	lpsm->hGlobal = ::GlobalAlloc(GHND | GMEM_DDESHARE, pData[i].size);
	memcpy_raw(::GlobalLock(lpsm->hGlobal), pData[i].data, pData[i].size);
	::GlobalUnlock(lpsm->hGlobal);
	lpsm->pUnkForRelease = nullptr;

	return S_OK;
}

/** IDataObject::GetDataHere
	@date 2008.03.26 ryoji 複数フォーマット対応
*/
STDMETHODIMP DataObject::GetDataHere(LPFORMATETC lpfe, LPSTGMEDIUM lpsm)
{
	// Feb. 26, 2001, fixed by yebisuya sugoroku
	if (!lpfe || !lpsm || !lpsm->hGlobal)
		return E_INVALIDARG;
	if (!pData)
		return OLE_E_NOTRUNNING;

	if (lpfe->lindex != -1)
		return DV_E_LINDEX;
	if (lpfe->tymed != TYMED_HGLOBAL
		|| lpsm->tymed != TYMED_HGLOBAL)
		return DV_E_TYMED;
	if (lpfe->dwAspect != DVASPECT_CONTENT)
		return DV_E_DVASPECT;

	int i;
	for (i=0; i<nFormat; ++i) {
		if (lpfe->cfFormat == pData[i].cfFormat)
			break;
	}
	if (i == nFormat)
		return DV_E_FORMATETC;
	if (pData[i].size > ::GlobalSize(lpsm->hGlobal))
		return STG_E_MEDIUMFULL;

	memcpy_raw(::GlobalLock(lpsm->hGlobal), pData[i].data, pData[i].size);
	::GlobalUnlock(lpsm->hGlobal);

	return S_OK;
}

/** IDataObject::QueryGetData
	@date 2008.03.26 ryoji 複数フォーマット対応
*/
STDMETHODIMP DataObject::QueryGetData(LPFORMATETC lpfe)
{
	if (!lpfe)
		return E_INVALIDARG;
	// Feb. 26, 2001, fixed by yebisuya sugoroku
	if (!pData)
		return OLE_E_NOTRUNNING;

	if (lpfe->ptd
		|| lpfe->dwAspect != DVASPECT_CONTENT
		|| lpfe->lindex != -1
		|| !(lpfe->tymed & TYMED_HGLOBAL))
		return DATA_E_FORMATETC;

	int i;
	for (i=0; i<nFormat; ++i) {
		if (lpfe->cfFormat == pData[i].cfFormat)
			break;
	}
	if (i == nFormat)
		return DATA_E_FORMATETC;
	return S_OK;
}

STDMETHODIMP DataObject::GetCanonicalFormatEtc(LPFORMATETC, LPFORMATETC)
{
	return DATA_S_SAMEFORMATETC;
}

STDMETHODIMP DataObject::SetData(LPFORMATETC, LPSTGMEDIUM, BOOL)
{
	return E_NOTIMPL;
}

/** IDataObject::EnumFormatEtc
	@date 2008.03.26 ryoji IEnumFORMATETCをサポート
*/
STDMETHODIMP DataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc)
{
	if (dwDirection != DATADIR_GET)
		return S_FALSE;
	*ppenumFormatetc = new EnumFORMATETC(this);
	return *ppenumFormatetc? S_OK: S_FALSE;
}

STDMETHODIMP DataObject::DAdvise(LPFORMATETC, DWORD, LPADVISESINK, LPDWORD)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP DataObject::DUnadvise(DWORD)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP DataObject::EnumDAdvise(LPENUMSTATDATA*)
{
	return OLE_E_ADVISENOTSUPPORTED;
}


/** IEnumFORMATETC::Next
	@date 2008.03.26 ryoji 新規作成
*/
STDMETHODIMP EnumFORMATETC::Next(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched)
{
	if (celt <= 0 || !rgelt || nIndex >= pDataObject->nFormat)
		return S_FALSE;
	if (celt != 1 && !pceltFetched)
		return S_FALSE;

	ULONG i = celt;
	while (nIndex < pDataObject->nFormat && i > 0) {
		(*rgelt).cfFormat = pDataObject->pData[nIndex].cfFormat;
		(*rgelt).ptd = nullptr;
		(*rgelt).dwAspect = DVASPECT_CONTENT;
		(*rgelt).lindex = -1;
		(*rgelt).tymed = TYMED_HGLOBAL;
		++rgelt;
		++nIndex;
		--i;
	}
	if (pceltFetched)
		*pceltFetched = celt - i;

	return (i == 0) ? S_OK : S_FALSE;
}

/** IEnumFORMATETC::Skip
	@date 2008.03.26 ryoji 新規作成
*/
STDMETHODIMP EnumFORMATETC::Skip(ULONG celt)
{
	while (nIndex < pDataObject->nFormat && celt > 0) {
		++nIndex;
		--celt;
	}

	return (celt == 0) ? S_OK : S_FALSE;
}

/** IEnumFORMATETC::Reset
	@date 2008.03.26 ryoji 新規作成
*/
STDMETHODIMP EnumFORMATETC::Reset(void)
{
	nIndex = 0;
	return S_OK;
}

/** IEnumFORMATETC::Clone
	@date 2008.03.26 ryoji 新規作成
*/
STDMETHODIMP EnumFORMATETC::Clone(IEnumFORMATETC** ppenum)
{
	return E_NOTIMPL;
}

