#include "StdAfx.h"
#include "tchar_receive.h"

using namespace std;

// TcharReceiver����
// UNICODE�r���h�ŁAwchar_t���󂯎��
template <> TCHAR* TcharReceiver<wchar_t>::GetBufferPointer() { return pReceiver; }
template <> void   TcharReceiver<wchar_t>::Apply() {} // �������Ȃ�

// UNICODE�r���h�ŁAACHAR���󂯎��
template <> TCHAR* TcharReceiver<ACHAR>::GetBufferPointer() { return (pBuff = new TCHAR[nReceiverCount]); }
template <> void   TcharReceiver<ACHAR>::Apply() { _tcstombs(pReceiver, pBuff, nReceiverCount); delete[] pBuff; }

// �C���X�^���X��
template class TcharReceiver<wchar_t>;
template class TcharReceiver<ACHAR>;


