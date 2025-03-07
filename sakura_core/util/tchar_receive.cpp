#include "StdAfx.h"
#include "tchar_receive.h"

using namespace std;

// TcharReceiver実装
// UNICODEビルドで、wchar_tを受け取る
template <> TCHAR* TcharReceiver<wchar_t>::GetBufferPointer() { return pReceiver; }
template <> void   TcharReceiver<wchar_t>::Apply() {} // 何もしない

// UNICODEビルドで、charを受け取る
template <> TCHAR* TcharReceiver<char>::GetBufferPointer() { return (pBuff = new TCHAR[nReceiverCount]); }
template <> void   TcharReceiver<char>::Apply() { _tcstombs(pReceiver, pBuff, nReceiverCount); delete[] pBuff; }

// インスタンス化
template class TcharReceiver<wchar_t>;
template class TcharReceiver<char>;


