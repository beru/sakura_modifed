#include "StdAfx.h"
#include "tchar_receive.h"

using namespace std;

// TcharReceiver����
#ifdef _UNICODE
	// UNICODE�r���h�ŁAWCHAR���󂯎��
	template <> TCHAR* TcharReceiver<WCHAR>::GetBufferPointer() { return pReceiver; }
	template <> void   TcharReceiver<WCHAR>::Apply() {} // �������Ȃ�

	// UNICODE�r���h�ŁAACHAR���󂯎��
	template <> TCHAR* TcharReceiver<ACHAR>::GetBufferPointer() { return (pBuff = new TCHAR[nReceiverCount]); }
	template <> void   TcharReceiver<ACHAR>::Apply() { _tcstombs(pReceiver, pBuff, nReceiverCount); delete[] pBuff; }

#else
	// ANSI�r���h�ŁAWCHAR���󂯎��
	template <> TCHAR* TcharReceiver<WCHAR>::GetBufferPointer() { return (pBuff = new TCHAR[nReceiverCount]); }
	template <> void   TcharReceiver<WCHAR>::Apply() { _tcstowcs(pReceiver, pBuff, nReceiverCount); delete[] pBuff; }

	// ANSI�r���h�ŁAACHAR���󂯎��
	template <> TCHAR* TcharReceiver<ACHAR>::GetBufferPointer() { return pReceiver; }
	template <> void   TcharReceiver<ACHAR>::Apply() {} // �������Ȃ�

#endif

// �C���X�^���X��
template class TcharReceiver<WCHAR>;
template class TcharReceiver<ACHAR>;


