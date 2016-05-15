#include "StdAfx.h"
#include "VisualProgress.h"
#include "WaitCursor.h"

#include "window/EditWnd.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

VisualProgress::VisualProgress()
	:
	pWaitCursor(nullptr),
	nOldValue(-1)
{
}

VisualProgress::~VisualProgress()
{
	SAFE_DELETE(pWaitCursor);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ���[�h�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void VisualProgress::OnBeforeLoad(LoadInfo* loadInfo)
{
	_Begin();
}

void VisualProgress::OnAfterLoad(const LoadInfo& loadInfo)
{
	_End();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �Z�[�u�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void VisualProgress::OnBeforeSave(const SaveInfo& saveInfo)
{
	_Begin();
}

void VisualProgress::OnFinalSave(SaveResultType eSaveResult)
{
	_End();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �v���O���X��M                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void VisualProgress::OnProgress(int nPer)
{
	_Doing(nPer);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void VisualProgress::_Begin()
{
	// �����v
	if (!pWaitCursor) {
		pWaitCursor = new WaitCursor(EditWnd::getInstance().GetHwnd());
	}

	// �v���O���X�o�[
	HWND hwndProgress = EditWnd::getInstance().m_statusBar.GetProgressHwnd();
	if (hwndProgress) {
		::ShowWindow(hwndProgress, SW_SHOW);
		// �͈͐ݒ�E���Z�b�g
		Progress_SetRange(hwndProgress, 0, 101);
		Progress_SetPos(hwndProgress, 0);
	}
}

void VisualProgress::_Doing(int nPer)
{
	// �v���O���X�o�[�X�V
	HWND hwndProgress = EditWnd::getInstance().m_statusBar.GetProgressHwnd();
	if (hwndProgress) {
		if (nOldValue != nPer) {
			Progress_SetPos(hwndProgress, nPer + 1); // 2013.06.10 Moca Vista/7���Ńv���O���X�o�[���A�j���[�V�����Œx���΍�
			Progress_SetPos(hwndProgress, nPer);
			nOldValue = nPer;
		}
	}
}

void VisualProgress::_End()
{
	// �v���O���X�o�[
	HWND hwndProgress = EditWnd::getInstance().m_statusBar.GetProgressHwnd();
	if (hwndProgress) {
		Progress_SetPos(hwndProgress, 0);
		::ShowWindow(hwndProgress, SW_HIDE);
	}

	// �����v
	SAFE_DELETE(pWaitCursor);
}

