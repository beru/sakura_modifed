/*!	@file
	@brief �t�@�C���v���p�e�B�_�C�A���O
*/

class DlgProperty;

#pragma once

#include "dlg/Dialog.h"
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class DlgProperty : public Dialog {
public:
	INT_PTR DoModal(HINSTANCE, HWND, LPARAM);	// ���[�_���_�C�A���O�̕\��
protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL OnBnClicked(int);
	void SetData(void);	// �_�C�A���O�f�[�^�̐ݒ�
	LPVOID GetHelpIdTable(void);
};

