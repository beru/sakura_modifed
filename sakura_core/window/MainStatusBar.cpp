#include "StdAfx.h"
#include "MainStatusBar.h"
#include "window/EditWnd.h"
#include "EditApp.h"

MainStatusBar::MainStatusBar(EditWnd& owner)
	:
	owner(owner),
	hwndStatusBar(NULL),
	hwndProgressBar(NULL)
{
}


//	�L�[���[�h�F�X�e�[�^�X�o�[����
// �X�e�[�^�X�o�[�쐬
void MainStatusBar::CreateStatusBar()
{
	if (hwndStatusBar) return;

	// �X�e�[�^�X�o�[
	hwndStatusBar = ::CreateStatusWindow(
		WS_CHILD/* | WS_VISIBLE*/ | WS_EX_RIGHT | SBARS_SIZEGRIP,	// 2007.03.08 ryoji WS_VISIBLE ����
		_T(""),
		owner.GetHwnd(),
		IDW_STATUSBAR
	);

	// �v���O���X�o�[
	hwndProgressBar = ::CreateWindowEx(
		WS_EX_TOOLWINDOW,
		PROGRESS_CLASS,
		NULL,
		WS_CHILD /*|  WS_VISIBLE*/,
		3,
		5,
		150,
		13,
		hwndStatusBar,
		NULL,
		EditApp::getInstance().GetAppInstance(),
		0
	);

	if (owner.funcKeyWnd.GetHwnd()) {
		owner.funcKeyWnd.SizeBox_ONOFF(false);
	}

	// �X�v���b�^�[�́A�T�C�Y�{�b�N�X�̈ʒu��ύX
	owner.splitterWnd.DoSplit(-1, -1);
}


// �X�e�[�^�X�o�[�j��
void MainStatusBar::DestroyStatusBar()
{
	if (hwndProgressBar) {
		::DestroyWindow(hwndProgressBar);
		hwndProgressBar = NULL;
	}
	::DestroyWindow(hwndStatusBar);
	hwndStatusBar = NULL;

	if (owner.funcKeyWnd.GetHwnd()) {
		bool bSizeBox;
		if (GetDllShareData().common.window.nFuncKeyWnd_Place == 0) {	// �t�@���N�V�����L�[�\���ʒu�^0:�� 1:��
			// �T�C�Y�{�b�N�X�̕\���^��\���؂�ւ�
			bSizeBox = false;
		}else {
			bSizeBox = true;
			// �X�e�[�^�X�p�[��\�����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ�
			if (hwndStatusBar) {
				bSizeBox = false;
			}
		}
		owner.funcKeyWnd.SizeBox_ONOFF(bSizeBox);
	}
	// �X�v���b�^�[�́A�T�C�Y�{�b�N�X�̈ʒu��ύX
	owner.splitterWnd.DoSplit(-1, -1);
}


/*!
	@brief ���b�Z�[�W�̕\��
	
	�w�肳�ꂽ���b�Z�[�W���X�e�[�^�X�o�[�ɕ\������D
	���j���[�o�[�E�[�ɓ���Ȃ����̂�C���ʒu�\�����B�������Ȃ����̂Ɏg��
	
	�Ăяo���O��SendStatusMessage2IsEffective()�ŏ����̗L����
	�m�F���邱�ƂŖ��ʂȏ������Ȃ����Ƃ��o����D

	@param msg [in] �\�����郁�b�Z�[�W
	
	@sa SendStatusMessage2IsEffective
*/
void MainStatusBar::SendStatusMessage2(const TCHAR* msg)
{
	if (hwndStatusBar) {
		// �X�e�[�^�X�o�[��
		StatusBar_SetText(hwndStatusBar, 0 | SBT_NOBORDERS, msg);
	}
}


void MainStatusBar::SetStatusText(int nIndex, int nOption, const TCHAR* pszText)
{
	StatusBar_SetText(hwndStatusBar, nIndex | nOption, pszText);
}

