/*!	@file
	@brief �^�O�t�@�C���쐬�_�C�A���O�{�b�N�X
*/

class DlgTagsMake;

#pragma once

#include "dlg/Dialog.h"
/*!
	@brief �^�O�t�@�C���쐬�_�C�A���O�{�b�N�X
*/
class DlgTagsMake : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgTagsMake();

	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal(HINSTANCE, HWND, LPARAM, const TCHAR*);	// ���[�_���_�C�A���O�̕\��

	TCHAR	szPath[_MAX_PATH + 1];	// �t�H���_
	TCHAR	szTagsCmdLine[_MAX_PATH];	// �R�}���h���C���I�v�V����(��)
	int		nTagsOpt;					// CTAGS�I�v�V����(�`�F�b�N)

protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL	OnBnClicked(int);
	LPVOID	GetHelpIdTable(void);

	void	SetData(void);	// �_�C�A���O�f�[�^�̐ݒ�
	int		GetData(void);	// �_�C�A���O�f�[�^�̎擾

private:
	void SelectFolder();

};

