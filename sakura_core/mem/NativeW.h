/*
	Copyright (C) 2008, kobake

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

#include "Native.h"
#include "mem/NativeT.h"
#include "basis/SakuraBasis.h"
#include "debug/Debug2.h" // assert


// ������ւ̎Q�Ƃ��擾����C���^�[�t�F�[�X
class IStringRef {
public:
	virtual const wchar_t*	GetPtr()	const = 0;
	virtual int				GetLength()	const = 0;
};


// ������ւ̎Q�Ƃ�ێ�����N���X
class StringRef : public IStringRef {
public:
	StringRef() : m_pData(NULL), m_nDataLen(0) { }
	StringRef(const wchar_t* pData, int nDataLen) : m_pData(pData), m_nDataLen(nDataLen) { }
	const wchar_t*	GetPtr()		const { return m_pData;    }
	int				GetLength()		const { return m_nDataLen; }

	//########�⏕
	bool			IsValid()		const { return m_pData != NULL; }
	wchar_t			At(int nIndex)	const { assert(nIndex >= 0 && nIndex < m_nDataLen); return m_pData[nIndex]; }
private:
	const wchar_t*	m_pData;
	int				m_nDataLen;
};


// UNICODE������Ǘ��N���X
class NativeW : public Native {
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	NativeW();
	NativeW(const NativeW&);
	NativeW(const wchar_t* pData, size_t nDataLen); // nDataLen�͕����P�ʁB
	NativeW(const wchar_t* pData);

	// �Ǘ�
	void AllocStringBuffer(size_t nDataLen);                    // (�d�v�FnDataLen�͕����P��) �o�b�t�@�T�C�Y�̒����B�K�v�ɉ����Ċg�傷��B

	// WCHAR
	void SetString(const wchar_t* pData, size_t nDataLen);      // �o�b�t�@�̓��e��u��������BnDataLen�͕����P�ʁB
	void SetString(const wchar_t* pszData);                  // �o�b�t�@�̓��e��u��������
	void SetStringHoldBuffer( const wchar_t* pData, size_t nDataLen );
	void AppendString(const wchar_t* pszData);               // �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����
	void AppendString(const wchar_t* pszData, size_t nLength);  // �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����BnLength�͕����P�ʁB���������true�B�������m�ۂɎ��s������false��Ԃ��B

	template <size_t nLength>
	__forceinline void AppendStringLiteral(const wchar_t(&pszData)[nLength])
	{
		AppendString(pszData, nLength);
	}

	// NativeW
	void SetNativeData(const NativeW& pcNative);            // �o�b�t�@�̓��e��u��������
	void AppendNativeData(const NativeW&);                  // �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����

	// ���Z�q
	const NativeW& operator += (wchar_t wch)			{ AppendString(&wch, 1);   return *this; }
	const NativeW& operator = (wchar_t wch)				{ SetString(&wch, 1);      return *this; }
	const NativeW& operator += (const NativeW& rhs)		{ AppendNativeData(rhs); return *this; }
	const NativeW& operator = (const NativeW& rhs)		{ SetNativeData(rhs);    return *this; }
	NativeW operator + (const NativeW& rhs) const		{ NativeW tmp = *this; return tmp += rhs; }

	// �l�C�e�B�u�擾�C���^�[�t�F�[�X
	wchar_t operator [] (int nIndex) const;					// �C�ӈʒu�̕����擾�BnIndex�͕����P�ʁB
	LogicInt GetStringLength() const {						// �����񒷂�Ԃ��B�����P�ʁB
		return LogicInt(Native::GetRawLength() / sizeof(wchar_t));
	}
	const wchar_t* GetStringPtr() const {
		return reinterpret_cast<const wchar_t*>(GetRawPtr());
	}
	wchar_t* GetStringPtr() {
		return reinterpret_cast<wchar_t*>(GetRawPtr());
	}
	const wchar_t* GetStringPtr(int* pnLength) const {		// [out]pnLength�͕����P�ʁB
		*pnLength = GetStringLength();
		return reinterpret_cast<const wchar_t*>(GetRawPtr());
	}
#ifdef USE_STRICT_INT
	const wchar_t* GetStringPtr(LogicInt* pnLength) const { // [out]pnLength�͕����P�ʁB
		int n;
		const wchar_t* p = GetStringPtr(&n);
		*pnLength = LogicInt(n);
		return p;
	}
#endif

	// ����
	void _SetStringLength(size_t nLength) {
		_GetMemory()->_SetRawLength(nLength * sizeof(wchar_t));
	}
	// ������1�������
	void Chop() {
		int n = GetStringLength();
		n -= 1;
		if (n >= 0) {
			_SetStringLength(n);
		}
	}
	void swap(NativeW& left) {
		_GetMemory()->swap(*left._GetMemory());
	}
	
	int capacity() {
		return _GetMemory()->capacity() / sizeof(wchar_t);
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ����                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	
	// ����̕�����Ȃ�true
	static bool IsEqual(const NativeW& mem1, const NativeW& mem2);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ϊ�                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void Replace(const wchar_t* pszFrom, const wchar_t* pszTo);   // ������u��
	void ReplaceT( const wchar_t* pszFrom, const wchar_t* pszTo ){
		Replace( pszFrom, pszTo );
	}
	void Replace( const wchar_t* pszFrom, size_t nFromLen, const wchar_t* pszTo, size_t nToLen );   // ������u��


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                  �^����C���^�[�t�F�[�X                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �g�p�͂ł��邾���T����̂��]�܂����B
	// �ЂƂ̓I�[�o�[�w�b�h��}����Ӗ��ŁB
	// �ЂƂ͕ϊ��ɂ��f�[�^�r����}����Ӗ��ŁB

	// ACHAR
	void SetStringOld(const char* pData, size_t nDataLen);    // �o�b�t�@�̓��e��u��������BpData��SJIS�BnDataLen�͕����P�ʁB
	void SetStringOld(const char* pszData);                // �o�b�t�@�̓��e��u��������BpszData��SJIS�B
	void AppendStringOld(const char* pData, size_t nDataLen); // �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����BpszData��SJIS�B
	void AppendStringOld(const char* pszData);             // �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����BpszData��SJIS�B
	const char* GetStringPtrOld() const; // ShiftJIS�ɕϊ����ĕԂ�

	// WCHAR
	void SetStringW(const wchar_t* pszData)					{ return SetString(pszData); }
	void SetStringW(const wchar_t* pData, size_t nLength)	{ return SetString(pData, nLength); }
	void AppendStringW(const wchar_t* pszData)				{ return AppendString(pszData); }
	void AppendStringW(const wchar_t* pData, size_t nLength){ return AppendString(pData, nLength); }
	const wchar_t* GetStringW() const						{ return GetStringPtr(); }

	// TCHAR
#ifdef _UNICODE
	void SetStringT(const TCHAR* pData, size_t nDataLen)	{ return SetString(pData, nDataLen); }
	void SetStringT(const TCHAR* pszData)					{ return SetString(pszData); }
	void AppendStringT(const TCHAR* pszData)				{ return AppendString(pszData); }
	void AppendStringT(const TCHAR* pData, size_t nLength)	{ return AppendString(pData, nLength); }
	void AppendNativeDataT(const NativeT& rhs)				{ return AppendNativeData(rhs); }
	const TCHAR* GetStringT() const							{ return GetStringPtr(); }
#else
	void SetStringT(const TCHAR* pData, size_t nDataLen)	{ return SetStringOld(pData, nDataLen); }
	void SetStringT(const TCHAR* pszData)					{ return SetStringOld(pszData); }
	void AppendStringT(const TCHAR* pszData)				{ return AppendStringOld(pszData); }
	void AppendStringT(const TCHAR* pData, size_t nLength)	{ return AppendStringOld(pData, nLength); }
	void AppendNativeDataT(const NativeT& rhs)				{ return AppendStringOld(rhs.GetStringPtr(), rhs.GetStringLength()); }
	const TCHAR* GetStringT() const							{ return GetStringPtrOld(); }
#endif

#if _DEBUG
private:
	typedef wchar_t* PWCHAR;
	PWCHAR& m_pDebugData; // �f�o�b�O�p�BCMemory�̓��e��wchar_t*�^�ŃE�H�b�`���邽�߂̃��m
#endif

public:
	// -- -- static�C���^�[�t�F�[�X -- -- //
	static LogicInt GetSizeOfChar(const wchar_t* pData, size_t nDataLen, size_t nIdx);	// �w�肵���ʒu�̕�����wchar_t��������Ԃ�
	static LayoutInt GetKetaOfChar(const wchar_t* pData, size_t nDataLen, size_t nIdx);	// �w�肵���ʒu�̕��������p��������Ԃ�
	static const wchar_t* GetCharNext(const wchar_t* pData, size_t nDataLen, const wchar_t* pDataCurrent); // �|�C���^�Ŏ����������̎��ɂ��镶���̈ʒu��Ԃ��܂�
	static const wchar_t* GetCharPrev(const wchar_t* pData, size_t nDataLen, const wchar_t* pDataCurrent); // �|�C���^�Ŏ����������̒��O�ɂ��镶���̈ʒu��Ԃ��܂�

	static LayoutInt GetKetaOfChar(const StringRef& str, size_t nIdx) { // �w�肵���ʒu�̕��������p��������Ԃ�
		return GetKetaOfChar(str.GetPtr(), str.GetLength(), nIdx);
	}
};

namespace std {
	template <>
	inline void swap(NativeW& n1, NativeW& n2) {
		n1.swap(n2);
	}
}

