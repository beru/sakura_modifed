/*!	@file
	@brief �v���O�C���ݒ�_�C�A���O�{�b�N�X
*/
#pragma once

#include "dlg/Dialog.h"
#include "plugin/PluginManager.h"

class PropPlugin;

/*!	@brief �u�v���O�C���ݒ�v�_�C�A���O

	���ʐݒ�̃v���O�C���ݒ�ŁC�w��v���O�C���̃I�v�V������ݒ肷�邽�߂�
	�g�p�����_�C�A���O�{�b�N�X
*/

// �ҏW�ő咷
#define MAX_LENGTH_VALUE	1024

typedef std::wstring wstring;

// �^ 
static const wstring	OPTION_TYPE_BOOL = wstring(L"bool");
static const wstring	OPTION_TYPE_INT  = wstring(L"int");
static const wstring	OPTION_TYPE_SEL  = wstring(L"sel");
static const wstring	OPTION_TYPE_DIR  = wstring(L"dir");

class DlgPluginOption : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgPluginOption(PropPlugin&);
	~DlgPluginOption();

	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal(HINSTANCE, HWND, int);	// ���[�_���_�C�A���O�̕\��

protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL	OnInitDialog(HWND, WPARAM wParam, LPARAM lParam);
	BOOL	OnBnClicked(int);
	BOOL	OnNotify(WPARAM wParam, LPARAM lParam);
	BOOL	OnCbnSelChange(HWND hwndCtl, int wID);
	BOOL	OnEnChange(HWND hwndCtl, int wID);
	BOOL	OnActivate(WPARAM wParam, LPARAM lParam);
	LPVOID	GetHelpIdTable(void);

	void	SetData(void);	// �_�C�A���O�f�[�^�̐ݒ�
	int		GetData(void);	// �_�C�A���O�f�[�^�̎擾

	void	ChangeListPosition(void);				// �ҏW�̈�����X�g�r���[�ɍ����Đؑւ���
	void	MoveFocusToEdit(void);					// �ҏW�̈�Ƀt�H�[�J�X���ڂ�
	void	SetToEdit(int);
	void	SetFromEdit(int);
	void	SelectEdit(int);						// �ҏW�̈�̐؂�ւ�
	void	SepSelect(wstring, wstring*, wstring*);	// �I��p�����񕪉�
	void	SelectDirectory(int iLine);				// �f�B���N�g����I������

private:
	Plugin*		plugin;
	PropPlugin&	propPlugin;
	int 			id;			// �v���O�C���ԍ��i�G�f�B�^���ӂ�ԍ��j
	int				line;			// ���ݕҏW���̃I�v�V�����s�ԍ�
	std::tstring	sReadMeName;	// ReadMe �t�@�C����
};

