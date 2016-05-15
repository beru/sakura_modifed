/*!	@file
	@brief �w��s�ւ̃W�����v�_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date 1998/05/31 �쐬
	@date 1999/12/05 �č쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class DlgJump;

#pragma once

#include "dlg/Dialog.h"
// �w��s�ւ̃W�����v�_�C�A���O�{�b�N�X
class DlgJump : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgJump();
	/*
	||  Attributes & Operations
	*/
	int DoModal(HINSTANCE, HWND, LPARAM);	// ���[�_���_�C�A���O�̕\��

	int		nLineNum;		// �s�ԍ�
	bool	bPLSQL;		// PL/SQL�\�[�X�̗L���s��
	int		nPLSQL_E1;
	int		nPLSQL_E2;
protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL OnNotify(WPARAM, LPARAM);	// Oct. 6, 2000 JEPRO added for Spin control
	BOOL OnCbnSelChange(HWND, int);
	BOOL OnBnClicked(int);
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
	void SetData(void);	// �_�C�A���O�f�[�^�̐ݒ�
	int GetData(void);	// �_�C�A���O�f�[�^�̎擾
};

