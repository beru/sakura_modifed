/*!	@file
	@brief 1�s���̓_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date	1998/05/31 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class DlgInput1;

#pragma once

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �P�s���̓_�C�A���O�{�b�N�X
*/
class DlgInput1 {
public:
	/*
	||  Constructors
	*/
	DlgInput1();
	~DlgInput1();
	BOOL DoModal(HINSTANCE, HWND, const TCHAR*, const TCHAR*, int, TCHAR*);		// ���[�h���X�_�C�A���O�̕\��
	BOOL DoModal(HINSTANCE, HWND, const TCHAR*, const TCHAR*, int, NOT_TCHAR*);	// ���[�h���X�_�C�A���O�̕\��
	/*
	||  Attributes & Operations
	*/
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);	// �_�C�A���O�̃��b�Z�[�W����

	HINSTANCE	hInstance;	// �A�v���P�[�V�����C���X�^���X�̃n���h��
	HWND		hwndParent;	// �I�[�i�[�E�B���h�E�̃n���h��
	HWND		hWnd;			// ���̃_�C�A���O�̃n���h��

	const TCHAR*	pszTitle;		// �_�C�A���O�^�C�g��
	const TCHAR*	pszMessage;	// ���b�Z�[�W
	int			nMaxTextLen;		// ���̓T�C�Y���
//	char*		pszText;			// �e�L�X�g
	NativeT	memText;			// �e�L�X�g
protected:
	/*
	||  �����w���p�֐�
	*/
};


