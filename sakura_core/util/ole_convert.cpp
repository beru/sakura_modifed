/*!	@file
	@brief OLE型（VARIANT, BSTRなど）の変換関数

*/
#include "StdAfx.h"
#include "ole_convert.h"

// VARIANT変数をBSTRとみなし、wstringに変換する
// CMacro::HandleFunctionを参考とした。
bool variant_to_wstr(const VARIANT& v, std::wstring& wstr)
{
	Variant varCopy;	// VT_BYREFだと困るのでコピー用
	if (VariantChangeType(&varCopy.data, &v, 0, VT_BSTR) != S_OK) return false;	// VT_BSTRとして解釈

	wchar_t* source;
	int sourceLength;
	Wrap(&varCopy.data.bstrVal)->GetW(&source, &sourceLength);

	wstr.assign(source, sourceLength);
	delete[] source;

	return true;
}

// VARIANT変数を整数とみなし、intに変換する
// CMacro::HandleFunctionを参考とした。
bool variant_to_int(const VARIANT& v, int& n)
{
	Variant varCopy;	// VT_BYREFだと困るのでコピー用
	if (VariantChangeType(&varCopy.data, &v, 0, VT_I4) != S_OK) return false;	// VT_I4として解釈

	n = varCopy.data.lVal;
	return true;
}

