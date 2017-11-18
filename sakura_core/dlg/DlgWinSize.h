/*!	@file
	@brief �E�B���h�E�̈ʒu�Ƒ傫���_�C�A���O
*/

#pragma once

#include "dlg/Dialog.h"
#include "env/CommonSetting.h"

/*!	@brief �ʒu�Ƒ傫���̐ݒ�_�C�A���O

	���ʐݒ�̃E�B���h�E�ݒ�ŁC�E�B���h�E�ʒu���w�肷�邽�߂ɕ⏕�I��
	�g�p�����_�C�A���O�{�b�N�X
*/
class DlgWinSize : public Dialog {
public:
	DlgWinSize();
	~DlgWinSize();
	INT_PTR DoModal(HINSTANCE, HWND, WinSizeMode&, WinSizeMode&, int&, RECT&);	// ���[�_���_�C�A���O�̕\��

protected:

	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnBnClicked(int);
	int  GetData(void);
	void SetData(void);
	LPVOID GetHelpIdTable(void);

	void RenewItemState(void);

private:
	WinSizeMode	eSaveWinSize;	// �E�B���h�E�T�C�Y�̕ۑ�: 0/�f�t�H���g�C1/�p���C2/�w��
	WinSizeMode	eSaveWinPos;	// �E�B���h�E�ʒu�̕ۑ�: 0/�f�t�H���g�C1/�p���C2/�w��
	int			nWinSizeType;	// �E�B���h�E�\�����@: 0/�W���C1/�ő剻�C2/�ŏ���
	RECT		rc;
};

