/*!	@file
	@brief �u���_�C�A���O
*/

#pragma once

#include "dlg/Dialog.h"
#include "recent/Recent.h"
#include "util/window.h"

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �u���_�C�A���O�{�b�N�X
*/
class DlgReplace : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgReplace();
	/*
	||  Attributes & Operations
	*/
	HWND DoModeless(HINSTANCE, HWND, LPARAM, bool);	// ���[�_���_�C�A���O�̕\��
	void ChangeView(LPARAM);	// ���[�h���X���F�u���E�����ΏۂƂȂ�r���[�̕ύX

	SearchOption	searchOption;		// �����I�v�V����
	bool			bConsecutiveAll;	//�u���ׂĒu���v�͒u���̌J�Ԃ�  2007.01.16 ryoji
	std::wstring	strText;			// ����������
	std::wstring	strText2;			// �u���㕶����
	int				nReplaceKeySequence;// �u����V�[�P���X
	bool			bSelectedArea;		// �I��͈͓��u��
	bool			bNotifyNotFound;	// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
	bool			bSelected;			// �e�L�X�g�I�𒆂�
	int				nReplaceTarget;		// �u���Ώ�		2001.12.03 hor
	bool			bPaste;				// �\��t���H	2001.12.03 hor
	int				nReplaceCnt;		// ���ׂĒu���̎��s����		// 2002.02.08 hor
	bool			bCanceled;			// ���ׂĒu���Œ��f������	// 2002.02.08 hor

	Point			ptEscCaretPos_PHY;	// ����/�u���J�n���̃J�[�\���ʒu�ޔ��G���A

protected:
	RecentSearch		recentSearch;
	ComboBoxItemDeleter	comboDelText;
	RecentReplace		recentReplace;
	ComboBoxItemDeleter	comboDelText2;
	FontAutoDeleter		fontText;
	FontAutoDeleter		fontText2;

	/*
	||  �����w���p�֐�
	*/
	BOOL OnCbnDropDown(HWND hwndCtl, int wID);
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnDestroy();
	BOOL OnBnClicked(int);
	BOOL OnActivate(WPARAM wParam, LPARAM lParam);	// 2009.11.29 ryoji
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void SetData(void);		// �_�C�A���O�f�[�^�̐ݒ�
	void SetCombosList(void);	// ����������/�u���㕶���񃊃X�g�̐ݒ�
	int GetData(void);		// �_�C�A���O�f�[�^�̎擾
};

