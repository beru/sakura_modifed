// �����F�^�w�i�F����_�C�A���O

#pragma once

#include "dlg/Dialog.h"

struct TypeConfig;

/*!	@brief �����F�^�w�i�F����_�C�A���O

	�^�C�v�ʐݒ�̃J���[�ݒ�ŁC�����F�^�w�i�F����̑ΏېF���w�肷�邽�߂ɕ⏕�I��
	�g�p�����_�C�A���O�{�b�N�X
*/
class DlgSameColor : public Dialog {
public:
	DlgSameColor();
	~DlgSameColor();
	INT_PTR DoModal(HINSTANCE, HWND, WORD, TypeConfig*, COLORREF);		// ���[�_���_�C�A���O�̕\��

protected:

	virtual LPVOID GetHelpIdTable(void);
	virtual INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);		// �_�C�A���O�̃��b�Z�[�W����
	virtual BOOL OnInitDialog(HWND, WPARAM, LPARAM);				// WM_INITDIALOG ����
	virtual BOOL OnBnClicked(int);									// BN_CLICKED ����
	virtual BOOL OnDrawItem(WPARAM wParam, LPARAM lParam);			// WM_DRAWITEM ����
	BOOL OnSelChangeListColors(HWND hwndCtl);						// �F�I�����X�g�� LBN_SELCHANGE ����

	static LRESULT CALLBACK ColorStatic_SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);	// �T�u�N���X�����ꂽ�w��F�X�^�e�B�b�N�̃E�B���h�E�v���V�[�W��
	static LRESULT CALLBACK ColorList_SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);		// �T�u�N���X�����ꂽ�F�I�����X�g�̃E�B���h�E�v���V�[�W��

	WNDPROC wpColorStaticProc;	// �T�u�N���X���ȑO�̎w��F�X�^�e�B�b�N�̃E�B���h�E�v���V�[�W��
	WNDPROC wpColorListProc;	// �T�u�N���X���ȑO�̐F�I�����X�g�̃E�B���h�E�v���V�[�W��

	WORD wID;				// �^�C�v�ʐݒ�_�C�A���O�i�e�_�C�A���O�j�ŉ����ꂽ�{�^��ID
	TypeConfig* pTypes;		// �^�C�v�ʐݒ�f�[�^
	COLORREF cr;			// �w��F
};

