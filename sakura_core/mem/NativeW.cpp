#include "StdAfx.h"
#include "mem/NativeW.h"
#include "Eol.h"
#include "charset/ShiftJis.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
NativeW::NativeW()
#if _DEBUG
	:
	pDebugData((PWCHAR&)_DebugGetPointerRef())
#endif
{
}

NativeW::NativeW(const NativeW& rhs)
#if _DEBUG
	:
	pDebugData((PWCHAR&)_DebugGetPointerRef())
#endif
{
	SetNativeData(rhs);
}

// nDataLen�͕����P�ʁB
NativeW::NativeW(const wchar_t* pData, size_t nDataLen)
#if _DEBUG
	:
	pDebugData((PWCHAR&)_DebugGetPointerRef())
#endif
{
	SetString(pData, nDataLen);
}

NativeW::NativeW(const wchar_t* pData)
#if _DEBUG
	:
	pDebugData((PWCHAR&)_DebugGetPointerRef())
#endif
{
	SetString(pData, wcslen(pData));
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              �l�C�e�B�u�ݒ�C���^�[�t�F�[�X                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// �o�b�t�@�̓��e��u��������
void NativeW::SetString(const wchar_t* pData, size_t nDataLen)
{
	Native::SetRawData(pData, nDataLen * sizeof(wchar_t));
}

// �o�b�t�@�̓��e��u��������
void NativeW::SetString(const wchar_t* pszData)
{
	Native::SetRawData(pszData,wcslen(pszData) * sizeof(wchar_t));
}

void NativeW::SetStringHoldBuffer( const wchar_t* pData, size_t nDataLen )
{
	Native::SetRawDataHoldBuffer(pData, nDataLen * sizeof(wchar_t));
}

// �o�b�t�@�̓��e��u��������
void NativeW::SetNativeData(const NativeW& pcNative)
{
	Native::SetRawData(pcNative);
}

// (�d�v�FnDataLen�͕����P��) �o�b�t�@�T�C�Y�̒����B�K�v�ɉ����Ċg�傷��B
void NativeW::AllocStringBuffer(size_t nDataLen)
{
	Native::AllocBuffer(nDataLen * sizeof(wchar_t));
}

// �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����
void NativeW::AppendString(const wchar_t* pszData)
{
	Native::AppendRawData(pszData,wcslen(pszData) * sizeof(wchar_t));
}

// �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����BnLength�͕����P�ʁB
void NativeW::AppendString(const wchar_t* pszData, size_t nLength)
{
	Native::AppendRawData(pszData, nLength * sizeof(wchar_t));
}

// �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����
void NativeW::AppendNativeData(const NativeW& memData)
{
	Native::AppendRawData(memData.GetStringPtr(), memData.GetRawLength());
}

// -- -- char����̈ڍs�p -- -- //

// �o�b�t�@�̓��e��u��������BnDataLen�͕����P�ʁB
void NativeW::SetStringOld(const char* pData, size_t nDataLen)
{
	int nLen;
	wchar_t* szTmp = mbstowcs_new(pData, nDataLen, &nLen);
	SetString(szTmp, nLen);
	delete[] szTmp;
}

// �o�b�t�@�̓��e��u��������
void NativeW::SetStringOld(const char* pszData)
{
	SetStringOld(pszData, strlen(pszData));
}

void NativeW::AppendStringOld(const char* pData, size_t nDataLen)
{
	int nLen;
	wchar_t* szTmp=mbstowcs_new(pData, nDataLen, &nLen);
	AppendString(szTmp, nLen);
	delete[] szTmp;
}

// �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����BpszData��SJIS�B
void NativeW::AppendStringOld(const char* pszData)
{
	AppendStringOld(pszData, strlen(pszData));
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              �l�C�e�B�u�擾�C���^�[�t�F�[�X                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// GetAt()�Ɠ��@�\
wchar_t NativeW::operator[](size_t nIndex) const
{
	if (nIndex < GetStringLength()) {
		return GetStringPtr()[nIndex];
	}else {
		return 0;
	}
}


// ���������e��
bool NativeW::IsEqual(
	const NativeW& mem1,
	const NativeW& mem2
	)
{
	if (&mem1 == &mem2) {
		return true;
	}

	size_t nLen1;
	size_t nLen2;
	const wchar_t* psz1 = mem1.GetStringPtr(&nLen1);
	const wchar_t* psz2 = mem2.GetStringPtr(&nLen2);
	
	if (nLen1 == nLen2) {
		if (wmemcmp(psz1, psz2, nLen1) == 0) {
			return true;
		}
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              �l�C�e�B�u�ϊ��C���^�[�t�F�[�X                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ������u��
void NativeW::Replace(
	const wchar_t* pszFrom,
	const wchar_t* pszTo
	)
{
	size_t nFromLen = wcslen(pszFrom);
	size_t nToLen = wcslen(pszTo);
	Replace( pszFrom, nFromLen, pszTo, nToLen );
}

void NativeW::Replace(
	const wchar_t* pszFrom,
	size_t nFromLen,
	const wchar_t* pszTo,
	size_t nToLen
	)
{
	NativeW memWork;
	size_t nBgnOld = 0;
	size_t nBgn = 0;
	while (nBgn <= GetStringLength() - nFromLen) {
		if (wmemcmp(&GetStringPtr()[nBgn], pszFrom, nFromLen) == 0) {
			if (nBgnOld == 0 && nFromLen <= nToLen) {
				memWork.AllocStringBuffer(GetStringLength());
			}
			if (0  < nBgn - nBgnOld) {
				memWork.AppendString(&GetStringPtr()[nBgnOld], nBgn - nBgnOld);
			}
			memWork.AppendString(pszTo, nToLen);
			nBgn = nBgn + nFromLen;
			nBgnOld = nBgn;
		}else {
			++nBgn;
		}
	}
	if (nBgnOld != 0) {
		if (0 < GetStringLength() - nBgnOld) {
			memWork.AppendString(&GetStringPtr()[nBgnOld], GetStringLength() - nBgnOld);
		}
		SetNativeData(memWork);
	}else {
		if (!this->GetStringPtr()) {
			this->SetString(L"");
		}
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  static�C���^�[�t�F�[�X                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �w�肵���ʒu�̕�����wchar_t��������Ԃ�
size_t NativeW::GetSizeOfChar(
	const wchar_t* pData,
	size_t nDataLen,
	size_t nIdx
	)
{
	if (nIdx >= nDataLen) {
		return 0;
	}

	// �T���Q�[�g�`�F�b�N					2008/7/5 Uchi
	if (IsUTF16High(pData[nIdx])) {
		if (nIdx + 1 < nDataLen && IsUTF16Low(pData[nIdx + 1])) {
			// �T���Q�[�g�y�A 2��
			return 2;
		}
	}

	return 1;
}

// �w�肵���ʒu�̕��������p��������Ԃ�
size_t NativeW::GetKetaOfChar(
	const wchar_t* pData,
	size_t nDataLen,
	size_t nIdx
	)
{
	// ������͈͊O�Ȃ� 0
	if (nIdx >= nDataLen) {
		return 0;
	}

	// �T���Q�[�g�`�F�b�N BMP �ȊO�͑S�p����		2008/7/5 Uchi
	if (IsUTF16High(pData[nIdx])) {
		return 2;	// ��
	}
	if (IsUTF16Low(pData[nIdx])) {
		if (nIdx > 0 && IsUTF16High(pData[nIdx - 1])) {
			// �T���Q�[�g�y�A�i���ʁj
			return 0;
		}
		// �P�Ɓi�u���[�N���y�A�j
		// return 2;
		if (IsBinaryOnSurrogate(pData[nIdx])) {
			return 1;
		}else {
			return 2;
		}
	}

	// ���p�����Ȃ� 1
	if (WCODE::IsHankaku(pData[nIdx])) {
		return 1;
	// �S�p�����Ȃ� 2
	}else {
		return 2;
	}
}

// �|�C���^�Ŏ����������̎��ɂ��镶���̈ʒu��Ԃ��܂�
// ���ɂ��镶�����o�b�t�@�̍Ō�̈ʒu���z����ꍇ��&pData[nDataLen]��Ԃ��܂�
const wchar_t* NativeW::GetCharNext(
	const wchar_t* pData,
	size_t nDataLen,
	const wchar_t* pDataCurrent
	)
{
	const wchar_t* pNext = pDataCurrent + 1;

	if (pNext >= &pData[nDataLen]) {
		return &pData[nDataLen];
	}

	// �T���Q�[�g�y�A�Ή�	2008/7/6 Uchi
	if (IsUTF16High(*pDataCurrent)) {
		if (IsUTF16Low(*pNext)) {
			pNext += 1;
		}
	}

	return pNext;
}

// �|�C���^�Ŏ����������̒��O�ɂ��镶���̈ʒu��Ԃ��܂�
// ���O�ɂ��镶�����o�b�t�@�̐擪�̈ʒu���z����ꍇ��pData��Ԃ��܂�
const wchar_t* NativeW::GetCharPrev(
	const wchar_t* pData,
	size_t nDataLen,
	const wchar_t* pDataCurrent
	)
{
	const wchar_t* pPrev = pDataCurrent - 1;
	if (pPrev <= pData) {
		return pData;
	}

	// �T���Q�[�g�y�A�Ή�	2008/7/6 Uchi
	if (IsUTF16Low(*pPrev)) {
		if (IsUTF16High(*(pPrev-1))) {
			pPrev -= 1;
		}
	}

	return pPrev;
//	return ::CharPrevW_AnyBuild(pData, pDataCurrent);
}


// ShiftJIS�ɕϊ����ĕԂ�
const char* NativeW::GetStringPtrOld() const
{
	return to_achar(GetStringPtr(), GetStringLength());
}

