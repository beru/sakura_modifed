/*!	@file
	@brief GREP�_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date 1998.09/07  �V�K�쐬
	@date 1999.12/05 �č쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class DlgGrep;

#pragma once

#include "dlg/Dialog.h"
#include "recent/Recent.h"
#include "util/window.h"

//! GREP�_�C�A���O�{�b�N�X
class DlgGrep : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgGrep();
	/*
	||  Attributes & Operations
	*/
	BOOL OnCbnDropDown( HWND hwndCtl, int wID );
	int DoModal(HINSTANCE, HWND, const TCHAR*);		// ���[�_���_�C�A���O�̕\��
//	HWND DoModeless(HINSTANCE, HWND, const char*);	// ���[�h���X�_�C�A���O�̕\��


	bool		m_bSubFolder;		// �T�u�t�H���_�������������
	bool		m_bFromThisText;	// ���̕ҏW���̃e�L�X�g���猟������

	SearchOption	searchOption;	// �����I�v�V����

	EncodingType	nGrepCharSet;			// �����R�[�h�Z�b�g
	int			nGrepOutputStyle;			// Grep: �o�͌`��
	int			nGrepOutputLineType;		// ���ʏo�́F�s���o��/�Y������/�ۃ}�b�`�s
	bool		bGrepOutputFileOnly;		// �t�@�C�����ŏ��̂݌���
	bool		bGrepOutputBaseFolder;		// �x�[�X�t�H���_�\��
	bool		bGrepSeparateFolder;		// �t�H���_���ɕ\��


	std::wstring	m_strText;				// ����������
	bool			m_bSetText;				// �����������ݒ肵����
	SFilePath	m_szFile;					// �����t�@�C��
	SFilePath	m_szFolder;					// �����t�H���_
	SFilePath	m_szCurrentFilePath;
protected:
	ComboBoxItemDeleter	m_comboDelText;
	RecentSearch		m_recentSearch;
	ComboBoxItemDeleter	m_comboDelFile;
	RecentGrepFile		m_recentGrepFile;
	ComboBoxItemDeleter	m_comboDelFolder;
	RecentGrepFolder	m_recentGrepFolder;
	FontAutoDeleter		m_fontText;

	/*
	||  �����w���p�֐�
	*/
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnDestroy();
	BOOL OnBnClicked(int);
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void SetData(void);	// �_�C�A���O�f�[�^�̐ݒ�
	int GetData(void);	// �_�C�A���O�f�[�^�̎擾
	void SetDataFromThisText(bool);	// ���ݕҏW���t�@�C�����猟���`�F�b�N�ł̐ݒ�
};

