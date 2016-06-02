/*!	@file
	@brief OLE Type wrapper

	@author 鬼
	@date 2003.0221
*/
/*
	Copyright (C) 2003, 鬼, Moca
	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.

*/
#pragma once

#include <Windows.h>
#include <OleAuto.h>

/*! BSTRのWrapper class

	データ構造はBSTRと互換性あり
*/
struct SysString {
	BSTR data;

	SysString()                         { data = NULL; }
	SysString(const SysString& source)  { data = ::SysAllocStringLen(source.data, SysStringLen(source.data)); }
	SysString(BSTR& source)             { data = ::SysAllocStringLen(source, SysStringLen(source)); }
	SysString(const wchar_t* s, size_t l)  { data = ::SysAllocStringLen(s, (UINT)l); }
	SysString(const char* s, size_t l) { 
		auto buf = std::make_unique<wchar_t[]>(l + 1);
		int l2 = ::MultiByteToWideChar(CP_ACP, 0, s, (int)l, &buf[0], (int)l);
		data = ::SysAllocStringLen(&buf[0], l2); 
	}
	~SysString()                        { ::SysFreeString(data); }
	SysString& operator = (const SysString& source) {
		data = ::SysAllocStringLen(source.data, SysStringLen(source.data));
		return *this;
	}
	UINT Length()						{ return ::SysStringLen(data); }
	void Get(char** s, int* l) {
		UINT len = ::SysStringLen(data);
		*s = new char[len * 2 + 1];
		*l = ::WideCharToMultiByte(CP_ACP, 0, data, len, *s, len * 2, NULL, NULL);
		(*s)[*l] = 0;
	}
	void GetW(wchar_t** s, int* l) {
		UINT len = ::SysStringLen(data);
		*s = new wchar_t[len + 1];
		*l = len;
		wmemcpy(*s, data, len);
		(*s)[len] = L'\0';
	}
	void Get(std::string* str) {
		char* s;
		int len;
		Get(&s, &len);
		str->assign(s, len);
		delete[] s;
	}
	void GetW(std::wstring* str) {
		UINT len = ::SysStringLen(data);
		str->assign(data, len);
	}
	void GetT(TCHAR** s, int* l) { GetW(s, l); }
	void GetT(std::wstring* str) { GetW(str); }
};

/*! VARIANTのWrapper class

	データ構造はVARIANTと互換性あり
*/
struct Variant {
	VARIANT data;
	Variant()                       { ::VariantInit(&data); }
	Variant(Variant& Source)        { ::VariantCopyInd(&data, &Source.data); }
	Variant(VARIANT& Source)        { ::VariantCopyInd(&data, &Source); }
	~Variant()                      { ::VariantClear(&data); }
	Variant& operator = (Variant& Source) { ::VariantCopyInd(&data, &Source.data); return *this; }
	/*! SysStringをVariantにセットする
	
		セット後、SysStringの方は中身がNULLになる。
	*/
	void Receive(SysString& Source) {
		::VariantClear(&data); 
		data.vt = VT_BSTR; 
		data.bstrVal = Source.data; 
		Source.data = NULL; 
	}

	// 2003.06.25 Moca
	// intを戻り値として返す場合に対応
	int Receive(int i) {
		::VariantClear(&data); 
		data.vt = VT_I4;
		return data.lVal = i;
	}
};
/*
#if sizeof(SysString) != 4
#error "error"
#endif

#if sizeof(Variant) != 16
#error "error"
#endif
*/
inline
Variant* Wrap(VARIANT *Value)
{
	return reinterpret_cast<Variant*>(Value);
}

inline
SysString* Wrap(BSTR *Value)
{
	return reinterpret_cast<SysString*>(Value);
}

