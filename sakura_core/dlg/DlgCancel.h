#pragma once

#include "dlg/Dialog.h"

class DlgCancel;

// �L�����Z���{�^���_�C�A���O



/*!
	@brief �L�����Z���{�^���_�C�A���O
*/
class DlgCancel : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgCancel();
//	void Create(HINSTANCE, HWND);	// ������

	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal(HINSTANCE, HWND, int);		// ���[�h���X�_�C�A���O�̕\��
	HWND DoModeless(HINSTANCE, HWND, int);	// ���[�h���X�_�C�A���O�̕\��

//	HWND Open(LPCTSTR);
//	void Close(void);	// ���[�h���X�_�C�A���O�̍폜
	bool IsCanceled(void) { return bCANCEL; } // IDCANCEL�{�^���������ꂽ���H
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);	// �_�C�A���O�̃��b�Z�[�W���� BOOL->INT_PTR 2008/7/18 Uchi
	void DeleteAsync(void);	// �����j����x�����s����	// 2008.05.28 ryoji

//	HINSTANCE	hInstance;	// �A�v���P�[�V�����C���X�^���X�̃n���h��
//	HWND		hwndParent;	// �I�[�i�[�E�B���h�E�̃n���h��
//	HWND		hWnd;			// ���̃_�C�A���O�̃n���h��
	bool		bCANCEL;		// IDCANCEL�{�^���������ꂽ
	bool		bAutoCleanup;	// �����㏈���^	// 2008.05.28 ryoji

protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnBnClicked(int);
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
};

