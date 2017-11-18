#include "StdAfx.h"
#include "WaitCursor.h"

/*!
	現在のカーソルを保存し、カーソルを砂時計にする
*/
WaitCursor::WaitCursor(HWND hWnd, bool bEnable)
{
	this->bEnable = bEnable;
	if (!bEnable) return;
	SetCapture(hWnd);
	hCursor = ::LoadCursor(NULL, IDC_WAIT);
	hCursorOld = ::SetCursor(hCursor);
	return;
}

/*!
	カーソル形状を元に戻す
*/
WaitCursor::~WaitCursor()
{
	if (bEnable) {
		ReleaseCapture();
		::SetCursor(hCursorOld);
	}
}

