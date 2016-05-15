/*!	@file
	@brief 砂時計カーソル

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

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

