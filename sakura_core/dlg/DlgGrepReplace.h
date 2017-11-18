#pragma once

/*!	@file
	@brief GREP�u���_�C�A���O�{�b�N�X
*/

class DlgGrep;

#include "dlg/Dialog.h"
#include "dlg/DlgGrep.h"

// GREP�u���_�C�A���O�{�b�N�X
class DlgGrepReplace : public DlgGrep
{
public:
	/*
	||  Constructors
	*/
	DlgGrepReplace();
	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal(HINSTANCE, HWND, const TCHAR*, LPARAM);	// ���[�_���_�C�A���O�̕\��

	bool		bPaste;
	bool		bBackup;

	std::wstring	strText2;				// �u����
	int				nReplaceKeySequence;	// �u����V�[�P���X

protected:
	FontAutoDeleter		fontText2;

	/*
	||  �����w���p�֐�
	*/
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnDestroy();
	BOOL OnBnClicked(int);
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void SetData(void);	// �_�C�A���O�f�[�^�̐ݒ�
	int GetData(void);	// �_�C�A���O�f�[�^�̎擾
};

