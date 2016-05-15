/*!	@file
	@brief �����v�J�[�\��

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
	���݂̃J�[�\����ۑ����A�J�[�\���������v�ɂ���
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
	�J�[�\���`������ɖ߂�
*/
WaitCursor::~WaitCursor()
{
	if (bEnable) {
		ReleaseCapture();
		::SetCursor(hCursorOld);
	}
}

