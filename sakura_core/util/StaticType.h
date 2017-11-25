#pragma once

#include "util/string_ex.h"

// �q�[�v��p���Ȃ�vector
template <class ELEMENT_TYPE, size_t MAX_SIZE, class SET_TYPE = const ELEMENT_TYPE&>
class StaticVector {
public:
	// �^
	typedef ELEMENT_TYPE ElementType;

public:
	// ����
	size_t size() const { return nCount; }
	size_t max_size() const { return MAX_SIZE; }

	// �v�f�A�N�Z�X
	ElementType&       operator[](size_t nIndex)		{ assert(nIndex<MAX_SIZE); assert_warning(nIndex<nCount); return aElements[nIndex]; }
	const ElementType& operator[](size_t nIndex) const	{ assert(nIndex<MAX_SIZE); assert_warning(nIndex<nCount); return aElements[nIndex]; }

	// ����
	void clear() { nCount = 0; }
	void push_back(SET_TYPE e) {
		assert(nCount<MAX_SIZE);
		++nCount;
		aElements[nCount-1] = e;
	}
	void resize(size_t nNewSize) {
		ASSERT_GE(nNewSize, 0);
		ASSERT_GE(MAX_SIZE, nNewSize);
		nCount = nNewSize;
	}
	
	// �v�f����0�ł��v�f�ւ̃|�C���^���擾
	ElementType* dataPtr() { return aElements;}

	// ����
	size_t& _GetSizeRef() { return nCount; }
	void SetSizeLimit() {
		if (MAX_SIZE < nCount) {
			nCount = MAX_SIZE;
		}else if (nCount < 0) {
			nCount = 0;
		}
	}
	
private:
	size_t nCount;
	ElementType aElements[MAX_SIZE];
};

// �q�[�v��p���Ȃ�������N���X
template <class CHAR_TYPE, size_t N_BUFFER_COUNT>
class StaticString {
private:
	typedef StaticString<CHAR_TYPE, N_BUFFER_COUNT> Me;
public:
	static const size_t BUFFER_COUNT = N_BUFFER_COUNT;
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	StaticString() { szData[0] = 0; }
	StaticString(const CHAR_TYPE* rhs) { if (!rhs) szData[0] = 0; else auto_strcpy(szData, rhs); }

	// �N���X����
	size_t GetBufferCount() const { return N_BUFFER_COUNT; }

	// �f�[�^�A�N�Z�X
	CHAR_TYPE*       GetBufferPointer()			{ return szData; }
	const CHAR_TYPE* GetBufferPointer() const	{ return szData; }
	const CHAR_TYPE* c_str()            const	{ return szData; } // std::string��

	// �ȈՃf�[�^�A�N�Z�X
	operator       CHAR_TYPE*()			{ return szData; }
	operator const CHAR_TYPE*() const	{ return szData; }

	CHAR_TYPE At(size_t nIndex) const	{ return szData[nIndex]; }

	// �ȈՃR�s�[
	void Assign(const CHAR_TYPE* src)	{ if (!src) szData[0] = 0; else auto_strcpy_s(szData, _countof(szData), src); }
	Me& operator = (const CHAR_TYPE* src)	{ Assign(src); return *this; }

	// �e�탁�\�b�h
	size_t Length() const	{ return auto_strlen(szData); }

private:
	CHAR_TYPE szData[N_BUFFER_COUNT];
};

#define _countof2(s) s.BUFFER_COUNT

