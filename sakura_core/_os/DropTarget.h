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
class EditView;// 2002/2/3 aroka �w�b�_�y�ʉ�

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class OleLibrary {
	friend class CYbInterfaceBase;
private:
//	DWORD m_dwCount;	// 2009.01.08 ryoji m_dwCount�폜
	OleLibrary();
public:
	~OleLibrary();
private:
	void Initialize();
	void UnInitialize();
};



class CYbInterfaceBase {
private:
	static OleLibrary m_olelib;
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
	static REFIID m_owniid;
public:
	CYbInterfaceImpl() {AddRef();}
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj)
	{return QueryInterfaceImpl(this, m_owniid, riid, ppvObj);}
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
private: // 2002/2/10 aroka �A�N�Z�X���ύX
	EditWnd*		m_pEditWnd;	// 2008.06.20 ryoji
	HWND			m_hWnd_DropTarget;
	EditView*		m_pEditView;
	//	static REFIID	m_owniid;
public:
	BOOL			Register_DropTarget(HWND);
	BOOL			Revoke_DropTarget(void);
	STDMETHODIMP	DragEnter(LPDATAOBJECT, DWORD, POINTL , LPDWORD);
	STDMETHODIMP	DragOver(DWORD, POINTL, LPDWORD);
	STDMETHODIMP	DragLeave(void);
	STDMETHODIMP	Drop(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
protected:
	/*
	||  �����w���p�֐�
	*/
};


class DropSource : public CYbInterfaceImpl<IDropSource> {
private:
	BOOL m_bLeft;
public:
	DropSource(BOOL bLeft):m_bLeft(bLeft) {}

	STDMETHOD(QueryContinueDrag)(BOOL bEscapePressed, DWORD dwKeyState);
	STDMETHOD(GiveFeedback)(DWORD dropEffect);
};


class DataObject : public CYbInterfaceImpl<IDataObject> {
private:
	friend class EnumFORMATETC;	// 2008.03.26 ryoji

	typedef struct {
		CLIPFORMAT cfFormat;
		// Feb. 26, 2001, fixed by yebisuya sugoroku
		LPBYTE			data;	// �f�[�^
		unsigned int	size;	// �f�[�^�T�C�Y�B�o�C�g�P�ʁB
	} DATA, *PDATA;

	int m_nFormat;
	PDATA m_pData;

public:
	DataObject (LPCWSTR lpszText, int nTextLen, BOOL bColumnSelect ):
		m_nFormat(0),
		m_pData(NULL)
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


// EnumFORMATETC �N���X
// 2008.03.26 ryoji �V�K�쐬
class EnumFORMATETC : public CYbInterfaceImpl<IEnumFORMATETC> {
private:
	LONG m_lRef;
	int m_nIndex;
	DataObject* m_pDataObject;
public:
	EnumFORMATETC(DataObject* pDataObject) : m_lRef(1), m_nIndex(0), m_pDataObject(pDataObject) {}
	STDMETHOD_(ULONG, AddRef)(void)
	{return ::InterlockedIncrement(&m_lRef);}
	STDMETHOD_(ULONG, Release)(void)
	{
		if (::InterlockedDecrement(&m_lRef) == 0) {
			delete this;
			return 0;	// �폜��Ȃ̂� m_lRef �͎g��Ȃ�
		}
		return m_lRef;
	}
	STDMETHOD(Next)(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched);
	STDMETHOD(Skip)(ULONG celt);
	STDMETHOD(Reset)(void);
	STDMETHOD(Clone)(IEnumFORMATETC** ppenum);
};

