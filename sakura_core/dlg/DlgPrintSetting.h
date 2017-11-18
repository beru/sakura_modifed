/*!	@file
	@brief ����ݒ�_�C�A���O
*/
#pragma once

#include "dlg/Dialog.h"
#include "config/maxdata.h"	// MAX_PrintSettingARR
#include "print/Print.h"	// PrintSetting

/*!	����ݒ�_�C�A���O

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
class DlgPrintSetting : public Dialog {
public:
	/*
	||  Constructors
	*/

	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal(HINSTANCE, HWND, int*, PrintSetting*, int);	// ���[�_���_�C�A���O�̕\��

private:
	int				nCurrentPrintSetting;
	PrintSetting	printSettingArr[MAX_PrintSettingARR];
	int				nLineNumberColumns;					// �s�ԍ��\������ꍇ�̌���
	bool			bPrintableLinesAndColumnInvalid;
	HFONT			hFontDlg;								// �_�C�A���O�̃t�H���g�n���h��
	int				nFontHeight;							// �_�C�A���O�̃t�H���g�̃T�C�Y

protected:
	/*
	||  �����w���p�֐�
	*/
	void SetData(void);	// �_�C�A���O�f�[�^�̐ݒ�
	int GetData(void);	// �_�C�A���O�f�[�^�̎擾
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnDestroy(void);
	BOOL OnNotify(WPARAM,  LPARAM);
	BOOL OnCbnSelChange(HWND, int);
	BOOL OnBnClicked(int);
	BOOL OnStnClicked(int);
	BOOL OnEnChange(HWND hwndCtl, int wID);
	BOOL OnEnKillFocus(HWND hwndCtl, int wID);
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void OnChangeSettingType(BOOL);	// �ݒ�̃^�C�v���ς����
	void OnSpin(int , BOOL);			// �X�s���R���g���[���̏���
	int DataCheckAndCorrect(int , int);	// ���͒l(���l)�̃G���[�`�F�b�N�����Đ������l��Ԃ�
	bool CalcPrintableLineAndColumn();		// �s���ƌ������v�Z
	void UpdatePrintableLineAndColumn();	// �s���ƌ����̌v�Z�v��
	void SetFontName(int idTxt, int idUse, LOGFONT& lf, int nPointSize);	// �t�H���g��/�g�p�{�^���̐ݒ�
};


