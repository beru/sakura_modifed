#pragma once

#include "doc/DocListener.h"

class EditWnd;

class MainStatusBar : public DocListenerEx {
public:
	// �쐬�E�j��
	MainStatusBar(EditWnd& owner);
	void CreateStatusBar();		// �X�e�[�^�X�o�[�쐬
	void DestroyStatusBar();	// �X�e�[�^�X�o�[�j��
	void SendStatusMessage2(const TCHAR* msg);	//	Jul. 9, 2005 genta ���j���[�o�[�E�[�ɂ͏o�������Ȃ����߂̃��b�Z�[�W���o��
	/*!	SendStatusMessage2()�������ڂ����邩��\�߃`�F�b�N
		@note ����SendStatusMessage2()�ŃX�e�[�^�X�o�[�\���ȊO�̏�����ǉ�
		����ꍇ�ɂ͂�����ύX���Ȃ��ƐV�����ꏊ�ւ̏o�͂��s���Ȃ��D
		
		@sa SendStatusMessage2
	*/
	bool SendStatusMessage2IsEffective() const {
		return hwndStatusBar != NULL;
	}

	// �擾
	HWND GetStatusHwnd() const { return hwndStatusBar; }
	HWND GetProgressHwnd() const { return hwndProgressBar; }

	// �ݒ�
	void SetStatusText(int nIndex, int nOption, const TCHAR* pszText);
private:
	EditWnd&	owner;
	HWND		hwndStatusBar;
	HWND		hwndProgressBar;
};

