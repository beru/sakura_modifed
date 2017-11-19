/*!	@file
	@brief �w��s�ւ̃W�����v�_�C�A���O�{�b�N�X
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
	INT_PTR DoModal(HINSTANCE, HWND, LPARAM);	// ���[�_���_�C�A���O�̕\��

	UINT	nLineNum;		// �s�ԍ�
	bool	bPLSQL;		// PL/SQL�\�[�X�̗L���s��
	int		nPLSQL_E1;
	int		nPLSQL_E2;
protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL OnNotify(WPARAM, LPARAM);
	BOOL OnCbnSelChange(HWND, int);
	BOOL OnBnClicked(int);
	LPVOID GetHelpIdTable(void);
	void SetData(void);	// �_�C�A���O�f�[�^�̐ݒ�
	int GetData(void);	// �_�C�A���O�f�[�^�̎擾
};

