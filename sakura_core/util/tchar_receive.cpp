#include "StdAfx.h"
#include "tchar_receive.h"

using namespace std;

// TcharReceiver実装
#ifdef _UNICODE
	// UNICODEビルドで、wchar_tを受け取る
	template <> TCHAR* TcharReceiver<wchar_t>::GetBufferPointer() { return pReceiver; }
	template <> void   TcharReceiver<wchar_t>::Apply() {} // 何もしない

	// UNICODEビルドで、ACHARを受け取る
	template <> TCHAR* TcharReceiver<ACHAR>::GetBufferPointer() { return (pBuff = new TCHAR[nReceiverCount]); }
	template <> void   TcharReceiver<ACHAR>::Apply() { _tcstombs(pReceiver, pBuff, nReceiverCount); delete[] pBuff; }

#else
	// ANSIビルドで、wchar_tを受け取る
	template <> TCHAR* TcharReceiver<wchar_t>::GetBufferPointer() { return (pBuff = new TCHAR[nReceiverCount]); }
	template <> void   TcharReceiver<wchar_t>::Apply() { _tcstowcs(pReceiver, pBuff, nReceiverCount); delete[] pBuff; }

	// ANSIビルドで、ACHARを受け取る
	template <> TCHAR* TcharReceiver<ACHAR>::GetBufferPointer() { return pReceiver; }
	template <> void   TcharReceiver<ACHAR>::Apply() {} // 何もしない

#endif

// インスタンス化
template class TcharReceiver<wchar_t>;
template class TcharReceiver<ACHAR>;


