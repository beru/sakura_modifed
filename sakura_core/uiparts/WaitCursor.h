#pragma once

#include <Windows.h>

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
// �����v�J�[�\���N���X
/*!
	�I�u�W�F�N�g�̑������Ă���ԃJ�[�\���`��������v�ɂ���D
	�I�u�W�F�N�g���j�������ƃJ�[�\���`��͌��ɖ߂�
*/
class WaitCursor {
public:
	/*
	||  Constructors
	*/
	WaitCursor(HWND, bool bEnable = true);
	~WaitCursor();

	bool IsEnable() { return bEnable; }
private:
	HCURSOR	hCursor;
	HCURSOR	hCursorOld;
	bool	bEnable;
};

