#include "StdAfx.h"
#include "mem/NativeW.h"
#include "Eol.h"
#include "charset/ShiftJis.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
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

// nDataLenは文字単位。
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
//              ネイティブ設定インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// バッファの内容を置き換える
void NativeW::SetString(const wchar_t* pData, size_t nDataLen)
{
	Native::SetRawData(pData, nDataLen * sizeof(wchar_t));
}

// バッファの内容を置き換える
void NativeW::SetString(const wchar_t* pszData)
{
	Native::SetRawData(pszData,wcslen(pszData) * sizeof(wchar_t));
}

void NativeW::SetStringHoldBuffer( const wchar_t* pData, size_t nDataLen )
{
	Native::SetRawDataHoldBuffer(pData, nDataLen * sizeof(wchar_t));
}

// バッファの内容を置き換える
void NativeW::SetNativeData(const NativeW& pcNative)
{
	Native::SetRawData(pcNative);
}

// (重要：nDataLenは文字単位) バッファサイズの調整。必要に応じて拡大する。
void NativeW::AllocStringBuffer(size_t nDataLen)
{
	Native::AllocBuffer(nDataLen * sizeof(wchar_t));
}

// バッファの最後にデータを追加する
void NativeW::AppendString(const wchar_t* pszData)
{
	Native::AppendRawData(pszData,wcslen(pszData) * sizeof(wchar_t));
}

// バッファの最後にデータを追加する。nLengthは文字単位。
void NativeW::AppendString(const wchar_t* pszData, size_t nLength)
{
	Native::AppendRawData(pszData, nLength * sizeof(wchar_t));
}

// バッファの最後にデータを追加する
void NativeW::AppendNativeData(const NativeW& memData)
{
	Native::AppendRawData(memData.GetStringPtr(), memData.GetRawLength());
}

// -- -- charからの移行用 -- -- //

// バッファの内容を置き換える。nDataLenは文字単位。
void NativeW::SetStringOld(const char* pData, size_t nDataLen)
{
	int nLen;
	wchar_t* szTmp = mbstowcs_new(pData, nDataLen, &nLen);
	SetString(szTmp, nLen);
	delete[] szTmp;
}

// バッファの内容を置き換える
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

// バッファの最後にデータを追加する。pszDataはSJIS。
void NativeW::AppendStringOld(const char* pszData)
{
	AppendStringOld(pszData, strlen(pszData));
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ取得インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// GetAt()と同機能
wchar_t NativeW::operator[](size_t nIndex) const
{
	if (nIndex < GetStringLength()) {
		return GetStringPtr()[nIndex];
	}else {
		return 0;
	}
}


// 等しい内容か
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
//              ネイティブ変換インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 文字列置換
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
//                  staticインターフェース                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 指定した位置の文字がwchar_t何個分かを返す
size_t NativeW::GetSizeOfChar(
	const wchar_t* pData,
	size_t nDataLen,
	size_t nIdx
	)
{
	if (nIdx >= nDataLen) {
		return 0;
	}

	// サロゲートチェック
	if (IsUTF16High(pData[nIdx])) {
		if (nIdx + 1 < nDataLen && IsUTF16Low(pData[nIdx + 1])) {
			// サロゲートペア 2個分
			return 2;
		}
	}

	return 1;
}

// 指定した位置の文字が半角何個分かを返す
size_t NativeW::GetKetaOfChar(
	const wchar_t* pData,
	size_t nDataLen,
	size_t nIdx
	)
{
	// 文字列範囲外なら 0
	if (nIdx >= nDataLen) {
		return 0;
	}

	// サロゲートチェック BMP 以外は全角扱い
	if (IsUTF16High(pData[nIdx])) {
		return 2;	// 仮
	}
	if (IsUTF16Low(pData[nIdx])) {
		if (nIdx > 0 && IsUTF16High(pData[nIdx - 1])) {
			// サロゲートペア（下位）
			return 0;
		}
		// 単独（ブロークンペア）
		// return 2;
		if (IsBinaryOnSurrogate(pData[nIdx])) {
			return 1;
		}else {
			return 2;
		}
	}

	// 半角文字なら 1
	if (WCODE::IsHankaku(pData[nIdx])) {
		return 1;
	// 全角文字なら 2
	}else {
		return 2;
	}
}

// ポインタで示した文字の次にある文字の位置を返します
// 次にある文字がバッファの最後の位置を越える場合は&pData[nDataLen]を返します
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

	// サロゲートペア対応
	if (IsUTF16High(*pDataCurrent)) {
		if (IsUTF16Low(*pNext)) {
			pNext += 1;
		}
	}

	return pNext;
}

// ポインタで示した文字の直前にある文字の位置を返します
// 直前にある文字がバッファの先頭の位置を越える場合はpDataを返します
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

	// サロゲートペア対応
	if (IsUTF16Low(*pPrev)) {
		if (IsUTF16High(*(pPrev-1))) {
			pPrev -= 1;
		}
	}

	return pPrev;
//	return ::CharPrevW_AnyBuild(pData, pDataCurrent);
}


// ShiftJISに変換して返す
const char* NativeW::GetStringPtrOld() const
{
	return to_achar(GetStringPtr(), GetStringLength());
}

