/*!	@file
	@brief �R���g���[���R�[�h���̓_�C�A���O�{�b�N�X
*/

class DlgCtrlCode;

#pragma once

#include "dlg/Dialog.h"
/*!
	@brief �R���g���[���R�[�h���̓_�C�A���O�{�b�N�X
*/
class DlgCtrlCode : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgCtrlCode();

	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal(HINSTANCE, HWND, LPARAM);	// ���[�_���_�C�A���O�̕\��

	wchar_t GetCharCode() const { return nCode; } // �I�����ꂽ�����R�[�h���擾

private:
	/*
	||  �����w���p�֐�
	*/
	BOOL	OnInitDialog(HWND, WPARAM wParam, LPARAM lParam);
	BOOL	OnBnClicked(int);
	BOOL	OnNotify(WPARAM wParam, LPARAM lParam);
	LPVOID	GetHelpIdTable(void);

	void	SetData(void);	// �_�C�A���O�f�[�^�̐ݒ�
	int		GetData(void);	// �_�C�A���O�f�[�^�̎擾

private:
	/*
	|| �����o�ϐ�
	*/
	wchar_t		nCode;	// �R�[�h
};


