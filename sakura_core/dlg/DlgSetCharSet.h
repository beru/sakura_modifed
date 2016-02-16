/*!	@file
	@brief �����R�[�h�Z�b�g�ݒ�_�C�A���O�{�b�N�X

	@author Uchi
	@date 2010/6/14  �V�K�쐬
*/
/*
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include "dlg/Dialog.h"

// �����R�[�h�Z�b�g�ݒ�_�C�A���O�{�b�N�X
class DlgSetCharSet : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgSetCharSet();
	/*
	||  Attributes & Operations
	*/
	int DoModal(HINSTANCE, HWND, ECodeType*, bool*);	// ���[�_���_�C�A���O�̕\��


	ECodeType*	m_pnCharSet;			// �����R�[�h�Z�b�g
	bool*		m_pbBom;				// BOM
	bool		m_bCP;

	HWND		m_hwndCharSet;
	HWND		m_hwndCheckBOM;

protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL	OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL	OnBnClicked(int);
	BOOL	OnCbnSelChange(HWND, int);
	LPVOID	GetHelpIdTable(void);

	void	SetData(void);	// �_�C�A���O�f�[�^�̐ݒ�
	int 	GetData(void);	// �_�C�A���O�f�[�^�̎擾

	void	SetBOM(void);		// BOM �̐ݒ�
};

