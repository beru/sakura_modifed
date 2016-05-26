#include "StdAfx.h"
#include <string>
#include <mbstring.h>
#include "mem/NativeA.h"
#include "Eol.h"
#include "charset/ShiftJis.h"
#include "charset/charcode.h"
#include "util/string_ex2.h"

NativeA::NativeA(const char* szData)
	:
	Native()
{
	SetString(szData);
}

NativeA::NativeA()
	:
	Native()
{
}

NativeA::NativeA(const NativeA& rhs)
	:
	Native()
{
	SetString(rhs.GetStringPtr(), rhs.GetStringLength());
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              �l�C�e�B�u�ݒ�C���^�[�t�F�[�X                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �o�b�t�@�̓��e��u��������
void NativeA::SetString(const char* pszData)
{
	SetString(pszData, strlen(pszData));
}

// �o�b�t�@�̓��e��u��������BnLen�͕����P�ʁB
void NativeA::SetString(const char* pData, int nDataLen)
{
	int nDataLenBytes = nDataLen * sizeof(char);
	Native::SetRawData(pData, nDataLenBytes);
}

// �o�b�t�@�̓��e��u��������
void NativeA::SetNativeData(const NativeA& pcNative)
{
	Native::SetRawData(pcNative);
}

// �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����
void NativeA::AppendString(const char* pszData)
{
	AppendString(pszData, strlen(pszData));
}

// �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����BnLength�͕����P�ʁB
void NativeA::AppendString(const char* pszData, size_t nLength)
{
	Native::AppendRawData(pszData, nLength * sizeof(char));
}

const NativeA& NativeA::operator = (char cChar)
{
	char pszChar[2];
	pszChar[0] = cChar;
	pszChar[1] = '\0';
	SetRawData(pszChar, 1);
	return *this;
}

// �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����
void NativeA::AppendNativeData(const NativeA& pcNative)
{
	AppendString(pcNative.GetStringPtr(), pcNative.GetStringLength());
}

// (�d�v�FnDataLen�͕����P��) �o�b�t�@�T�C�Y�̒����B�K�v�ɉ����Ċg�傷��B
void NativeA::AllocStringBuffer(size_t nDataLen)
{
	Native::AllocBuffer(nDataLen * sizeof(char));
}

const NativeA& NativeA::operator += (char ch)
{
	char szChar[2] = {ch, '\0'};
	AppendString(szChar);
	return *this;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �݊�                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void NativeA::SetStringNew(
	const wchar_t* wszData,
	size_t nDataLen
	)
{
	std::wstring buf(wszData, nDataLen); // �؂�o��
	char* tmp = wcstombs_new(buf.c_str());
	SetString(tmp);
	delete[] tmp;
}

void NativeA::SetStringNew(const wchar_t* wszData)
{
	SetStringNew(wszData, wcslen(wszData));
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              �l�C�e�B�u�擾�C���^�[�t�F�[�X                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

size_t NativeA::GetStringLength() const
{
	return Native::GetRawLength() / sizeof(char);
}

const char* NativeA::GetStringPtr(size_t* pnLength) const
{
	if (pnLength) {
		*pnLength = GetStringLength();
	}
	return GetStringPtr();
}

// �C�ӈʒu�̕����擾�BnIndex�͕����P�ʁB
char NativeA::operator[](size_t nIndex) const
{
	if (nIndex < GetStringLength()) {
		return GetStringPtr()[nIndex];
	}else {
		return 0;
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              �l�C�e�B�u�ϊ��C���^�[�t�F�[�X                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ������u��
void NativeA::Replace(
	const char* pszFrom,
	const char* pszTo
	)
{
	NativeA	memWork;
	size_t nFromLen = strlen(pszFrom);
	size_t nToLen = strlen(pszTo);
	size_t nBgnOld = 0;
	size_t nBgn = 0;
	ASSERT_GE(GetStringLength(), nFromLen);
	while (nBgn <= GetStringLength() - nFromLen) {
		if (auto_memcmp(&GetStringPtr()[nBgn], pszFrom, nFromLen) == 0) {
			if (0 < nBgn - nBgnOld) {
				memWork.AppendString(&GetStringPtr()[nBgnOld], nBgn - nBgnOld);
			}
			memWork.AppendString(pszTo, nToLen);
			nBgn += nFromLen;
			nBgnOld = nBgn;
		}else {
			++nBgn;
		}
	}
	if (0 < GetStringLength() - nBgnOld) {
		memWork.AppendString(&GetStringPtr()[nBgnOld], GetStringLength() - nBgnOld);
	}
	SetNativeData(memWork);
	return;
}

// ������u���i���{��l���Łj
void NativeA::Replace_j(
	const char* pszFrom,
	const char* pszTo
	)
{
	NativeA	memWork;
	size_t nFromLen = strlen(pszFrom);
	size_t nToLen = strlen(pszTo);
	size_t nBgnOld = 0;
	size_t nBgn = 0;
	ASSERT_GE(GetStringLength(), nFromLen);
	while (nBgn <= GetStringLength() - nFromLen) {
		if (memcmp(&GetStringPtr()[nBgn], pszFrom, nFromLen) == 0) {
			if (0 < nBgn - nBgnOld) {
				memWork.AppendString(&GetStringPtr()[nBgnOld], nBgn - nBgnOld);
			}
			memWork.AppendString(pszTo, nToLen);
			nBgn += nFromLen;
			nBgnOld = nBgn;
		}else {
			if (_IS_SJIS_1((unsigned char)GetStringPtr()[nBgn])) {
				++nBgn;
			}
			++nBgn;
		}
	}
	if (0  < GetStringLength() - nBgnOld) {
		memWork.AppendString(&GetStringPtr()[nBgnOld], GetStringLength() - nBgnOld);
	}
	SetNativeData(memWork);
	return;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   ��ʃC���^�[�t�F�[�X                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  static�C���^�[�t�F�[�X                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �w�肵���ʒu�̕��������o�C�g��������Ԃ�
size_t NativeA::GetSizeOfChar(
	const char* pData,
	size_t nDataLen,
	size_t nIdx
	)
{
	return ShiftJis::GetSizeOfChar(pData, nDataLen, nIdx);
}

// �|�C���^�Ŏ����������̎��ɂ��镶���̈ʒu��Ԃ��܂�
// ���ɂ��镶�����o�b�t�@�̍Ō�̈ʒu���z����ꍇ��&pData[nDataLen]��Ԃ��܂�
const char* NativeA::GetCharNext(
	const char* pData,
	size_t nDataLen,
	const char* pDataCurrent
	)
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer("Memory::MemCharNext");
//#endif

	const char*	pNext;
	if (pDataCurrent[0] == '\0') {
		pNext = pDataCurrent + 1;
	}else {
//		pNext = ::CharNext(pDataCurrent);
		if (
			// SJIS�S�p�R�[�h��1�o�C�g�ڂ�	// Sept. 1, 2000 jepro '�V�t�g'��'S'�ɕύX
			_IS_SJIS_1((unsigned char)pDataCurrent[0])
			&&
			// SJIS�S�p�R�[�h��2�o�C�g�ڂ�	// Sept. 1, 2000 jepro '�V�t�g'��'S'�ɕύX
			_IS_SJIS_2((unsigned char)pDataCurrent[1])
		) {
			pNext = pDataCurrent + 2;
		}else {
			pNext = pDataCurrent + 1;
		}
	}

	if (pNext >= &pData[nDataLen]) {
		pNext = &pData[nDataLen];
	}
	return pNext;
}

// �|�C���^�Ŏ����������̒��O�ɂ��镶���̈ʒu��Ԃ��܂�
// ���O�ɂ��镶�����o�b�t�@�̐擪�̈ʒu���z����ꍇ��pData��Ԃ��܂�
const char* NativeA::GetCharPrev(const char* pData, size_t nDataLen, const char* pDataCurrent)
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer("Memory::MemCharPrev");
//#endif

	const char*	pPrev = ::CharPrevA(pData, pDataCurrent);

//===1999.08.12  ���̂������ƁA�_���������B===============-
//
//	if ((pDataCurrent - 1)[0] == '\0') {
//		pPrev = pDataCurrent - 1;
//	}else {
//		if (pDataCurrent - pData >= 2 &&
//			// SJIS�S�p�R�[�h��1�o�C�g�ڂ�		// Sept. 1, 2000 jepro '�V�t�g'��'S'�ɕύX
//			(
//			((unsigned char)0x81 <= (unsigned char)pDataCurrent[-2] && (unsigned char)pDataCurrent[-2] <= (unsigned char)0x9F) ||
//			((unsigned char)0xE0 <= (unsigned char)pDataCurrent[-2] && (unsigned char)pDataCurrent[-2] <= (unsigned char)0xFC)
//			) &&
//			// SJIS�S�p�R�[�h��2�o�C�g�ڂ�		// Sept. 1, 2000 jepro '�V�t�g'��'S'�ɕύX
//			(
//			((unsigned char)0x40 <= (unsigned char)pDataCurrent[-1] && (unsigned char)pDataCurrent[-1] <= (unsigned char)0x7E) ||
//			((unsigned char)0x80 <= (unsigned char)pDataCurrent[-1] && (unsigned char)pDataCurrent[-1] <= (unsigned char)0xFC)
//			)
//		) {
//			pPrev = pDataCurrent - 2;
//		}else {
//			pPrev = pDataCurrent - 1;
//		}
//	}
//	if (pPrev < pData) {
//		pPrev = pData;
//	}
	return pPrev;
}


void NativeA::AppendStringNew(const wchar_t* pszData)
{
	AppendStringNew(pszData, wcslen(pszData));
}

void NativeA::AppendStringNew(const wchar_t* pData, size_t nDataLen)
{
	char* buf = wcstombs_new(pData, nDataLen);
	AppendString(buf);
	delete[] buf;
}

const wchar_t* NativeA::GetStringW() const
{
	return to_wchar(GetStringPtr(), GetStringLength());
}

