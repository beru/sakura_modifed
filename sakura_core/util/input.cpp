#include "StdAfx.h"
#include "input.h"

/*!
	Shift,Ctrl,Alt�L�[��Ԃ̎擾

	@retval nIdx Shift,Ctrl,Alt�L�[���
*/
int GetCtrlKeyState()
{
	int nIdx = 0;

	// Shift�L�[��������Ă���Ȃ�
	if (GetKeyState_Shift()) {
		nIdx |= _SHIFT;
	}
	// Ctrl�L�[��������Ă���Ȃ�
	if (GetKeyState_Control()) {
		nIdx |= _CTRL;
	}
	// Alt�L�[��������Ă���Ȃ�
	if (GetKeyState_Alt()) {
		nIdx |= _ALT;
	}

	return nIdx;
}

