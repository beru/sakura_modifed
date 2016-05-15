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
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include <Unknwn.h>
#include "util/design_template.h"

class DropTarget;
class CYbInterfaceBase;
class EditWnd;	// 2008.06.20 ryoji
class EditView;// 2002/2/3 aroka ヘッダ軽量化

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class OleLibrary {
	friend class CYbInterfaceBase;
private:
	OleLibrary();
public:
	~OleLibrary();
private:
	void Initialize();
	void UnInitialize();
};



class CYbInterfaceBase {
private:
	static OleLibrary olelib;
protected:
	CYbInterfaceBase();
	~CYbInterfaceBase();
	static HRESULT QueryInterfaceImpl(IUnknown*, REFIID, REFIID, void**);
};


template <class BASEINTERFACE>
class CYbInterfaceImpl :
	public BASEINTERFACE,
	public CYbInterfaceBase
{
private:
	static REFIID owniid;
public:
	CYbInterfaceImpl() {AddRef();}
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj)
	{return QueryInterfaceImpl(this, owniid, riid, ppvObj);}
	STDMETHOD_(ULONG, AddRef)(void)
	{return 1;}
	STDMETHOD_(ULONG, Release)(void)
	{return 0;}
};


class DropTarget : public CYbInterfaceImpl<IDropTarget> {
public:
	/*
	||  Constructors
	*/
	DropTarget(EditWnd*);	// 2008.06.20 ryoji
	DropTarget(EditView*);
	~DropTarget();
	/*
	||  Attributes & Operations
	*/
private: // 2002/2/10 aroka アクセス権変更
	EditWnd*		pEditWnd;	// 2008.06.20 ryoji
	HWND			hWnd_DropTarget;
	EditView*		pEditView;
	//	static REFIID	owniid;
public:
	BOOL			Register_DropTarget(HWND);
	BOOL			Revoke_DropTarget(void);
	STDMETHODIMP	DragEnter(LPDATAOBJECT, DWORD, POINTL , LPDWORD);
	STDMETHODIMP	DragOver(DWORD, POINTL, LPDWORD);
	STDMETHODIMP	DragLeave(void);
	STDMETHODIMP	Drop(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
protected:
	/*
	||  実装ヘルパ関数
	*/
};


class DropSource : public CYbInterfaceImpl<IDropSource> {
private:
	BOOL bLeft;
public:
	DropSource(BOOL bLeft) : bLeft(bLeft) {}

	STDMETHOD(QueryContinueDrag)(BOOL bEscapePressed, DWORD dwKeyState);
	STDMETHOD(GiveFeedback)(DWORD dropEffect);
};


class DataObject : public CYbInterfaceImpl<IDataObject> {
private:
	friend class EnumFORMATETC;	// 2008.03.26 ryoji

	typedef struct {
		CLIPFORMAT cfFormat;
		// Feb. 26, 2001, fixed by yebisuya sugoroku
		LPBYTE			data;	// データ
		unsigned int	size;	// データサイズ。バイト単位。
	} DATA, *PDATA;

	int nFormat;
	PDATA pData;

public:
	DataObject (LPCWSTR lpszText, int nTextLen, BOOL bColumnSelect ):
		nFormat(0),
		pData(nullptr)
	{
		SetText(lpszText, nTextLen, bColumnSelect);
	}
	~DataObject() { SetText(NULL, 0, FALSE); }
	void	SetText(LPCWSTR lpszText, int nTextLen, BOOL bColumnSelect);
	DWORD	DragDrop(BOOL bLeft, DWORD dwEffects);

	STDMETHOD(GetData)(LPFORMATETC, LPSTGMEDIUM);
	STDMETHOD(GetDataHere)(LPFORMATETC, LPSTGMEDIUM);
	STDMETHOD(QueryGetData)(LPFORMATETC);
	STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC, LPFORMATETC);
	STDMETHOD(SetData)(LPFORMATETC, LPSTGMEDIUM, BOOL);
	STDMETHOD(EnumFormatEtc)(DWORD, IEnumFORMATETC**);
	STDMETHOD(DAdvise)(LPFORMATETC, DWORD, LPADVISESINK, LPDWORD);
	STDMETHOD(DUnadvise)(DWORD);
	STDMETHOD(EnumDAdvise)(LPENUMSTATDATA*);

private:
	DISALLOW_COPY_AND_ASSIGN(DataObject);
};


// EnumFORMATETC クラス
// 2008.03.26 ryoji 新規作成
class EnumFORMATETC : public CYbInterfaceImpl<IEnumFORMATETC> {
private:
	LONG lRef;
	int nIndex;
	DataObject* pDataObject;
public:
	EnumFORMATETC(DataObject* pDataObject) : lRef(1), nIndex(0), pDataObject(pDataObject) {}
	STDMETHOD_(ULONG, AddRef)(void)
	{return ::InterlockedIncrement(&lRef);}
	STDMETHOD_(ULONG, Release)(void)
	{
		if (::InterlockedDecrement(&lRef) == 0) {
			delete this;
			return 0;	// 削除後なので lRef は使わない
		}
		return lRef;
	}
	STDMETHOD(Next)(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched);
	STDMETHOD(Skip)(ULONG celt);
	STDMETHOD(Reset)(void);
	STDMETHOD(Clone)(IEnumFORMATETC** ppenum);
};

