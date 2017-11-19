#include "StdAfx.h"
#include "input.h"

/*!
	Shift,Ctrl,Altキー状態の取得

	@retval nIdx Shift,Ctrl,Altキー状態
*/
int GetCtrlKeyState()
{
	int nIdx = 0;

	// Shiftキーが押されているなら
	if (GetKeyState_Shift()) {
		nIdx |= _SHIFT;
	}
	// Ctrlキーが押されているなら
	if (GetKeyState_Control()) {
		nIdx |= _CTRL;
	}
	// Altキーが押されているなら
	if (GetKeyState_Alt()) {
		nIdx |= _ALT;
	}

	return nIdx;
}

