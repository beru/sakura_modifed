// �t�@�C���^�C�v�ꗗ�_�C�A���O

class DlgTypeList;

#pragma once

#include "dlg/Dialog.h"
using std::wstring;

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �t�@�C���^�C�v�ꗗ�_�C�A���O
*/
class DlgTypeList : public Dialog {
public:
	// �^
	struct Result {
		TypeConfigNum	documentType;	// �������
		bool			bTempChange;	// ��PROP_TEMPCHANGE_FLAG
	};

public:
	// �C���^�[�t�F�[�X
	INT_PTR DoModal(HINSTANCE, HWND, Result*);	// ���[�_���_�C�A���O�̕\��

protected:
	// �����w���p�֐�
	BOOL OnLbnDblclk(int);
	BOOL OnBnClicked(int);
	BOOL OnActivate(WPARAM wParam, LPARAM lParam);
	INT_PTR DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
	void SetData();		// �_�C�A���O�f�[�^�̐ݒ�
	void SetData(int);	// �_�C�A���O�f�[�^�̐ݒ�
	LPVOID GetHelpIdTable(void);
	bool Import(void);
	bool Export(void);
	bool InitializeType(void);
	bool CopyType();
	bool UpType();
	bool DownType();
	bool AddType();
	bool DelType();
	bool AlertFileAssociation();

private:
	TypeConfigNum nSettingType;
	// �֘A�t�����
	bool bRegistryChecked[MAX_TYPES];	// ���W�X�g���m�F ���^��
	bool bExtRMenu[MAX_TYPES];			// �E�N���b�N�o�^ ���^��
	bool bExtDblClick[MAX_TYPES];		// �_�u���N���b�N ���^��
	bool bAlertFileAssociation;			// �֘A�t���x���̕\���t���O
	bool bEnableTempChange;				// �ꎞ�K�p�̗L����
};

