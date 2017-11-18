/*!	@file
	@brief DIFF�����\���_�C�A���O�{�b�N�X
*/

class DlgDiff;

#pragma once

#include "dlg/Dialog.h"
/*!
	@brief DIFF�����\���_�C�A���O�{�b�N�X
*/
class DlgDiff : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgDiff();

	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal( HINSTANCE, HWND, LPARAM, const TCHAR* );	// ���[�_���_�C�A���O�̕\��

protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL	OnBnClicked(int);
	BOOL	OnLbnSelChange(HWND hwndCtl, int wID);
	BOOL	OnLbnDblclk(int wID);
	BOOL	OnEnChange(HWND hwndCtl, int wID);
	LPVOID	GetHelpIdTable(void);
	INT_PTR DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);	// �W���ȊO�̃��b�Z�[�W��ߑ�����
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnSize(WPARAM wParam, LPARAM lParam);
	BOOL OnMove(WPARAM wParam, LPARAM lParam);
	BOOL OnMinMaxInfo(LPARAM lParam);

	void	SetData(void);	// �_�C�A���O�f�[�^�̐ݒ�
	int		GetData(void);	// �_�C�A���O�f�[�^�̎擾

private:
	int			nIndexSave;		// �Ō�ɑI������Ă����ԍ�
	POINT		ptDefaultSize;
	RECT		rcItems[22];

public:
	SFilePath	szFile1;			// ���t�@�C��
	SFilePath	szFile2;			// ����t�@�C��
	bool		bIsModifiedDst;		// ����t�@�C���X�V��
	EncodingType	nCodeTypeDst;	// ����t�@�C���̕����R�[�h
	bool		bBomDst;			// ����t�@�C����BOM
	int			nDiffFlgOpt;		// DIFF�I�v�V����
	HWND		hWnd_Dst;			// ����E�B���h�E�n���h��

};


