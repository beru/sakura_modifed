#include "StdAfx.h"
#include "tchar_receive.h"

using namespace std;

// TcharReceiver実装
#ifdef _UNICODE
	// UNICODEビルドで、WCHARを受け取る
	template <> TCHAR* TcharReceiver<WCHAR>::GetBufferPointer() { return pReceiver; }
	template <> void   TcharReceiver<WCHAR>::Apply() {} // 何もしない

	// UNICODEビルドで、ACHARを受け取る
	template <> TCHAR* TcharReceiver<ACHAR>::GetBufferPointer() { return (pBuff = new TCHAR[nReceiverCount]); }
	template <> void   TcharReceiver<ACHAR>::Apply() { _tcstombs(pReceiver, pBuff, nReceiverCount); delete[] pBuff; }

#else
	// ANSIビルドで、WCHARを受け取る
	template <> TCHAR* TcharReceiver<WCHAR>::GetBufferPointer() { return (pBuff = new TCHAR[nReceiverCount]); }
	template <> void   TcharReceiver<WCHAR>::Apply() { _tcstowcs(pReceiver, pBuff, nReceiverCount); delete[] pBuff; }

	// ANSIビルドで、ACHARを受け取る
	template <> TCHAR* TcharReceiver<ACHAR>::GetBufferPointer() { return pReceiver; }
	template <> void   TcharReceiver<ACHAR>::Apply() {} // 何もしない

#endif

// インスタンス化
template class TcharReceiver<WCHAR>;
template class TcharReceiver<ACHAR>;


