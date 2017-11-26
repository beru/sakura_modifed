/*!	@file
	@brief GREP�_�C�A���O�{�b�N�X
*/

class DlgGrep;

#pragma once

#include "dlg/Dialog.h"
#include "recent/Recent.h"
#include "util/window.h"

// GREP�_�C�A���O�{�b�N�X
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
	INT_PTR DoModal(HINSTANCE, HWND, const TCHAR*);		// ���[�_���_�C�A���O�̕\��
//	HWND DoModeless(HINSTANCE, HWND, const char*);	// ���[�h���X�_�C�A���O�̕\��


	bool		bSubFolder;		// �T�u�t�H���_�������������
	bool		bFromThisText;	// ���̕ҏW���̃e�L�X�g���猟������

	SearchOption	searchOption;	// �����I�v�V����

	EncodingType	nGrepCharSet;			// �����R�[�h�Z�b�g
	int			nGrepOutputStyle;			// Grep: �o�͌`��
	int			nGrepOutputLineType;		// ���ʏo�́F�s���o��/�Y������/�ۃ}�b�`�s
	bool		bGrepOutputFileOnly;		// �t�@�C�����ŏ��̂݌���
	bool		bGrepOutputBaseFolder;		// �x�[�X�t�H���_�\��
	bool		bGrepSeparateFolder;		// �t�H���_���ɕ\��


	std::wstring	strText;				// ����������
	bool			bSetText;				// �����������ݒ肵����
	SFilePath	szFile;					// �����t�@�C��
	SFilePath	szFolder;					// �����t�H���_
	SFilePath	szCurrentFilePath;
protected:
	ComboBoxItemDeleter	comboDelText;
	RecentSearch		recentSearch;
	ComboBoxItemDeleter	comboDelFile;
	RecentGrepFile		recentGrepFile;
	ComboBoxItemDeleter	comboDelFolder;
	RecentGrepFolder	recentGrepFolder;
	FontAutoDeleter		fontText;

	/*
	||  �����w���p�֐�
	*/
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnDestroy();
	BOOL OnBnClicked(int);
	LPVOID GetHelpIdTable(void);

	void SetData(void);	// �_�C�A���O�f�[�^�̐ݒ�
	int GetData(void);	// �_�C�A���O�f�[�^�̎擾
	void SetDataFromThisText(bool);	// ���ݕҏW���t�@�C�����猟���`�F�b�N�ł̐ݒ�
};

