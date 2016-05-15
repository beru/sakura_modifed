/*!	@file
	@brief �����v�J�[�\��

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

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
private: // 2002/2/10 aroka
	HCURSOR	hCursor;
	HCURSOR	hCursorOld;
	bool	bEnable;
};

