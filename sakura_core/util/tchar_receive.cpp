#include "StdAfx.h"
#include "tchar_receive.h"

using namespace std;

// TcharReceiver����
#ifdef _UNICODE
	// UNICODE�r���h�ŁAwchar_t���󂯎��
	template <> TCHAR* TcharReceiver<wchar_t>::GetBufferPointer() { return pReceiver; }
	template <> void   TcharReceiver<wchar_t>::Apply() {} // �������Ȃ�

	// UNICODE�r���h�ŁAACHAR���󂯎��
	template <> TCHAR* TcharReceiver<ACHAR>::GetBufferPointer() { return (pBuff = new TCHAR[nReceiverCount]); }
	template <> void   TcharReceiver<ACHAR>::Apply() { _tcstombs(pReceiver, pBuff, nReceiverCount); delete[] pBuff; }

#else
	// ANSI�r���h�ŁAwchar_t���󂯎��
	template <> TCHAR* TcharReceiver<wchar_t>::GetBufferPointer() { return (pBuff = new TCHAR[nReceiverCount]); }
	template <> void   TcharReceiver<wchar_t>::Apply() { _tcstowcs(pReceiver, pBuff, nReceiverCount); delete[] pBuff; }

	// ANSI�r���h�ŁAACHAR���󂯎��
	template <> TCHAR* TcharReceiver<ACHAR>::GetBufferPointer() { return pReceiver; }
	template <> void   TcharReceiver<ACHAR>::Apply() {} // �������Ȃ�

#endif

// �C���X�^���X��
template class TcharReceiver<wchar_t>;
template class TcharReceiver<ACHAR>;


