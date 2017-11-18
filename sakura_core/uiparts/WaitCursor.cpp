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

