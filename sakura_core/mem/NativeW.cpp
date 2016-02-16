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
	m_pDebugData((PWCHAR&)_DebugGetPointerRef())
#endif
{
}

NativeW::NativeW(const NativeW& rhs)
#if _DEBUG
	:
	m_pDebugData((PWCHAR&)_DebugGetPointerRef())
#endif
{
	SetNativeData(rhs);
}

// nDataLen�͕����P�ʁB
NativeW::NativeW(const wchar_t* pData, int nDataLen)
#if _DEBUG
	:
	m_pDebugData((PWCHAR&)_DebugGetPointerRef())
#endif
{
	SetString(pData, nDataLen);
}

NativeW::NativeW(const wchar_t* pData)
#if _DEBUG
	:
	m_pDebugData((PWCHAR&)_DebugGetPointerRef())
#endif
{
	SetString(pData, wcslen(pData));
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              �l�C�e�B�u�ݒ�C���^�[�t�F�[�X                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// �o�b�t�@�̓��e��u��������
void NativeW::SetString(const wchar_t* pData, int nDataLen)
{
	Native::SetRawData(pData, nDataLen * sizeof(wchar_t));
}

// �o�b�t�@�̓��e��u��������
void NativeW::SetString(const wchar_t* pszData)
{
	Native::SetRawData(pszData,wcslen(pszData) * sizeof(wchar_t));
}

void NativeW::SetStringHoldBuffer( const wchar_t* pData, int nDataLen )
{
	Native::SetRawDataHoldBuffer(pData, nDataLen * sizeof(wchar_t));
}

// �o�b�t�@�̓��e��u��������
void NativeW::SetNativeData(const NativeW& pcNative)
{
	Native::SetRawData(pcNative);
}

// (�d�v�FnDataLen�͕����P��) �o�b�t�@�T�C�Y�̒����B�K�v�ɉ����Ċg�傷��B
void NativeW::AllocStringBuffer(int nDataLen)
{
	Native::AllocBuffer(nDataLen * sizeof(wchar_t));
}

// �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����
void NativeW::AppendString(const wchar_t* pszData)
{
	Native::AppendRawData(pszData,wcslen(pszData) * sizeof(wchar_t));
}

// �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����BnLength�͕����P�ʁB
void NativeW::AppendString(const wchar_t* pszData, int nLength)
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
void NativeW::SetStringOld(const char* pData, int nDataLen)
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

void NativeW::AppendStringOld(const char* pData, int nDataLen)
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
wchar_t NativeW::operator[](int nIndex) const
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

	int nLen1;
	int nLen2;
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
	int nFromLen = wcslen(pszFrom);
	int nToLen = wcslen(pszTo);
	Replace( pszFrom, nFromLen, pszTo, nToLen );
}

void NativeW::Replace(
	const wchar_t* pszFrom,
	int nFromLen,
	const wchar_t* pszTo,
	int nToLen
	)
{
	NativeW memWork;
	int nBgnOld = 0;
	int nBgn = 0;
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
LogicInt NativeW::GetSizeOfChar(
	const wchar_t* pData,
	int nDataLen,
	int nIdx
	)
{
	if (nIdx >= nDataLen) {
		return LogicInt(0);
	}

	// �T���Q�[�g�`�F�b�N					2008/7/5 Uchi
	if (IsUTF16High(pData[nIdx])) {
		if (nIdx + 1 < nDataLen && IsUTF16Low(pData[nIdx + 1])) {
			// �T���Q�[�g�y�A 2��
			return LogicInt(2);
		}
	}

	return LogicInt(1);
}

// �w�肵���ʒu�̕��������p��������Ԃ�
LayoutInt NativeW::GetKetaOfChar(
	const wchar_t* pData,
	int nDataLen,
	int nIdx
	)
{
	// ������͈͊O�Ȃ� 0
	if (nIdx >= nDataLen) {
		return LayoutInt(0);
	}

	// �T���Q�[�g�`�F�b�N BMP �ȊO�͑S�p����		2008/7/5 Uchi
	if (IsUTF16High(pData[nIdx])) {
		return LayoutInt(2);	// ��
	}
	if (IsUTF16Low(pData[nIdx])) {
		if (nIdx > 0 && IsUTF16High(pData[nIdx - 1])) {
			// �T���Q�[�g�y�A�i���ʁj
			return LayoutInt(0);
		}
		// �P�Ɓi�u���[�N���y�A�j
		// return LayoutInt(2);
		if (IsBinaryOnSurrogate(pData[nIdx])) {
			return LayoutInt(1);
		}else {
			return LayoutInt(2);
		}
	}

	// ���p�����Ȃ� 1
	if (WCODE::IsHankaku(pData[nIdx])) {
		return LayoutInt(1);
	// �S�p�����Ȃ� 2
	}else {
		return LayoutInt(2);
	}
}

// �|�C���^�Ŏ����������̎��ɂ��镶���̈ʒu��Ԃ��܂�
// ���ɂ��镶�����o�b�t�@�̍Ō�̈ʒu���z����ꍇ��&pData[nDataLen]��Ԃ��܂�
const wchar_t* NativeW::GetCharNext(
	const wchar_t* pData,
	int nDataLen,
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
	int nDataLen,
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

