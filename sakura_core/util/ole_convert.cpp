/*!	@file
	@brief OLE�^�iVARIANT, BSTR�Ȃǁj�̕ϊ��֐�

*/
#include "StdAfx.h"
#include "ole_convert.h"

// VARIANT�ϐ���BSTR�Ƃ݂Ȃ��Awstring�ɕϊ�����
// CMacro::HandleFunction���Q�l�Ƃ����B
bool variant_to_wstr(const VARIANT& v, std::wstring& wstr)
{
	Variant varCopy;	// VT_BYREF���ƍ���̂ŃR�s�[�p
	if (VariantChangeType(&varCopy.data, &v, 0, VT_BSTR) != S_OK) return false;	// VT_BSTR�Ƃ��ĉ���

	wchar_t* source;
	int sourceLength;
	Wrap(&varCopy.data.bstrVal)->GetW(&source, &sourceLength);

	wstr.assign(source, sourceLength);
	delete[] source;

	return true;
}

// VARIANT�ϐ��𐮐��Ƃ݂Ȃ��Aint�ɕϊ�����
// CMacro::HandleFunction���Q�l�Ƃ����B
bool variant_to_int(const VARIANT& v, int& n)
{
	Variant varCopy;	// VT_BYREF���ƍ���̂ŃR�s�[�p
	if (VariantChangeType(&varCopy.data, &v, 0, VT_I4) != S_OK) return false;	// VT_I4�Ƃ��ĉ���

	n = varCopy.data.lVal;
	return true;
}

