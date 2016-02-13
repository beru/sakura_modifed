/*!	@file
	@brief �O���R�}���h���s�_�C�A���O

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "dlg/CDialog.h"
#include "recent/CRecentCmd.h"

#pragma once

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class DlgExec : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgExec();
	/*
	||  Attributes & Operations
	*/
	int DoModal(HINSTANCE, HWND, LPARAM);	// ���[�_���_�C�A���O�̕\��

	TCHAR	m_szCommand[1024 + 1];	// �R�}���h���C��
	SFilePath	m_szCurDir;			// �J�����g�f�B���N�g��
	bool	m_bEditable;			// �ҏW�E�B���h�E�ւ̓��͉\	// 2009.02.21 ryoji


protected:
	ComboBoxItemDeleter m_comboDel;
	RecentCmd m_recentCmd;
	ComboBoxItemDeleter m_comboDelCur;
	RecentCurDir m_recentCur;

	// �I�[�o�[���C�h?
	int GetData(void);	// �_�C�A���O�f�[�^�̎擾
	void SetData(void);	// �_�C�A���O�f�[�^�̐ݒ�
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnBnClicked(int);
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add


};


