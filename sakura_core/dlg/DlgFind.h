/*!	@file
	@brief �����_�C�A���O�{�b�N�X
*/
#include "dlg/Dialog.h"
#include "recent/RecentSearch.h"
#include "util/window.h"

#pragma once

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class DlgFind : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgFind();
	/*
	||  Attributes & Operations
	*/
//	INT_PTR DoModal(HINSTANCE, HWND, LPARAM);		// ���[�_���_�C�A���O�̕\��
	HWND DoModeless(HINSTANCE, HWND, LPARAM);	// ���[�h���X�_�C�A���O�̕\��

	void ChangeView(LPARAM);

	SearchOption searchOption;	// �����I�v�V����
	bool	bNotifyNotFound;	// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
	std::wstring	strText;	// ����������

	Point	ptEscCaretPos_PHY;	// �����J�n���̃J�[�\���ʒu�ޔ��G���A

	RecentSearch		recentSearch;
	ComboBoxItemDeleter	comboDel;
	FontAutoDeleter		fontText;

protected:
//@@@ 2002.2.2 YAZAKI CShareData�Ɉړ�
//	void AddToSearchKeys(const char*);
	// �I�[�o�[���C�h?
	BOOL OnCbnDropDown(HWND hwndCtl, int wID);
	int GetData(void);		// �_�C�A���O�f�[�^�̎擾
	void SetCombosList(void);	// ����������/�u���㕶���񃊃X�g�̐ݒ�
	void SetData(void);		// �_�C�A���O�f�[�^�̐ݒ�
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnDestroy();
	BOOL OnBnClicked(int);
	BOOL OnActivate(WPARAM wParam, LPARAM lParam);	// 2009.11.29 ryoji

	// virtual BOOL OnKeyDown(WPARAM wParam, LPARAM lParam);
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
};

