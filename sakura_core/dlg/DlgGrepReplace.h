#pragma once

/*!	@file
	@brief GREP�u���_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date 2011.12.15 CDlgFrep.h����쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, Moca
	Copyright (C) 2014, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class DlgGrep;

#include "dlg/Dialog.h"
#include "dlg/DlgGrep.h"

//! GREP�u���_�C�A���O�{�b�N�X
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
	int DoModal( HINSTANCE, HWND, const TCHAR*, LPARAM );	// ���[�_���_�C�A���O�̕\��

	bool		m_bPaste;
	bool		m_bBackup;

	std::wstring	m_strText2;				// �u����
	int				nReplaceKeySequence;	// �u����V�[�P���X

protected:
	FontAutoDeleter		m_fontText2;

	/*
	||  �����w���p�֐�
	*/
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnDestroy();
	BOOL OnBnClicked( int );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void SetData( void );	// �_�C�A���O�f�[�^�̐ݒ�
	int GetData( void );	// �_�C�A���O�f�[�^�̎擾
};

