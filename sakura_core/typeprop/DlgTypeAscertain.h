// �^�C�v�ʐݒ�C���|�[�g�m�F�_�C�A���O

class DlgTypeAscertain;

#pragma once

using std::wstring;
using std::tstring;

#include "dlg/Dialog.h"
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �t�@�C���^�C�v�ꗗ�_�C�A���O
*/
class DlgTypeAscertain : public Dialog {
public:
	// �^
	struct AscertainInfo {
		tstring	sImportFile;	// in �C���|�[�g�t�@�C����
		wstring	sTypeNameTo;	// in �^�C�v���i�C���|�[�g��j
		wstring	sTypeNameFile;	// in �^�C�v���i�t�@�C������j
		int 	nColorType;		// out �������(�J���[�R�s�[�p)
		wstring	sColorFile;		// out �F�ݒ�t�@�C����
		bool	bAddType;		// out �^�C�v��ǉ�����
	};

public:
	// Constructors
	DlgTypeAscertain();
	// ���[�_���_�C�A���O�̕\��
	INT_PTR DoModal(HINSTANCE, HWND, AscertainInfo*);	// ���[�_���_�C�A���O�̕\��

protected:
	// �����w���p�֐�
	BOOL OnBnClicked(int);
	void SetData();	// �_�C�A���O�f�[�^�̐ݒ�
	LPVOID GetHelpIdTable(void);

private:
	AscertainInfo* psi;			// �C���^�[�t�F�C�X
};

