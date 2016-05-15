/*!	@file
	@brief �������E�B���h�E�N���X

	@author Norio Nakatani
	@date 1998/07/07 �V�K�쐬
	@date 2002/2/3 aroka ���g�p�R�[�h����
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka, YAZAKI
	Copyright (C) 2003, MIK
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "window/SplitterWnd.h"
#include "window/SplitBoxWnd.h"
#include "window/EditWnd.h"
#include "view/EditView.h"
#include "env/DllSharedData.h"


//	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
SplitterWnd::SplitterWnd()
	:
	Wnd(_T("::SplitterWnd")),
	pEditWnd(NULL),
	nAllSplitRows(1),					// �����s��
	nAllSplitCols(1),					// ��������
	nVSplitPos(0),					// ���������ʒu
	nHSplitPos(0),					// ���������ʒu
	nChildWndCount(0),
	bDragging(0),						// �����o�[���h���b�O����
	nDragPosX(0),						// �h���b�O�ʒu�w
	nDragPosY(0),						// �h���b�O�ʒu�x
	nActivePane(0)					// �A�N�e�B�u�ȃy�C�� 0-3
{
	// ���L�f�[�^�\���̂̃A�h���X��Ԃ�
	pShareData = &GetDllShareData();

	hcurOld = NULL;					// ���Ƃ̃}�E�X�J�[�\��

	for (int v=0; v<MAXCOUNTOFVIEW; ++v) {
		childWndArr[v] = NULL;		// �q�E�B���h�E�z��
	}
	return;
}


SplitterWnd::~SplitterWnd()
{
}


// ������
HWND SplitterWnd::Create(HINSTANCE hInstance, HWND hwndParent, EditWnd* pEditWnd)
{
	LPCTSTR pszClassName = _T("SplitterWndClass");
	
	// ������
	this->pEditWnd = pEditWnd;

	// �E�B���h�E�N���X�쐬
	ATOM atWork;
	atWork = RegisterWC(
		hInstance,
		NULL,// Handle to the class icon.
		NULL,	// Handle to a small icon
		NULL,// Handle to the class cursor.
		(HBRUSH)NULL,// Handle to the class background brush.
		NULL/*MAKEINTRESOURCE(MYDOCUMENT)*/,// Pointer to a null-terminated 
				// character string that specifies the resource name of the class menu,
				// as the name appears in the resource file.
		pszClassName// Pointer to a null-terminated string or is an atom.
	);
	if (atWork == 0) {
		ErrorMessage(NULL, LS(STR_ERR_CSPLITTER01));
	}

	// ���N���X�����o�Ăяo��
	return Wnd::Create(
		hwndParent,
		0, // extended window style
		pszClassName,	// Pointer to a null-terminated string or is an atom.
		pszClassName,	// pointer to window name
		WS_CHILD | WS_VISIBLE, // window style
		CW_USEDEFAULT, // horizontal position of window
		0, // vertical position of window
		CW_USEDEFAULT, // window width
		0, // window height
		NULL // handle to menu, or child-window identifier
	);
}


/* �q�E�B���h�E�̐ݒ�
	@param hwndEditViewArr [in] HWND�z�� NULL�I�[
*/
void SplitterWnd::SetChildWndArr(HWND* hwndEditViewArr)
{
	int v = 0;
	for (; v<MAXCOUNTOFVIEW && hwndEditViewArr[v]; ++v) {
		childWndArr[v] = hwndEditViewArr[v];				// �q�E�B���h�E�z��
	}
	nChildWndCount = v;
	// �c���NULL�Ŗ��߂�
	for (; v<MAXCOUNTOFVIEW; ++v) {
		childWndArr[v] = NULL;
	}

	// 2002/05/11 YAZAKI �s�v�ȏ����Ǝv����
	// �E�B���h�E�̕���
//	DoSplit(nHSplitPos, nVSplitPos);
//	DoSplit(0, 0);
	return;
}


// �����t���[���`��
void SplitterWnd::DrawFrame(HDC hdc, RECT* prc)
{
	SplitBoxWnd::Draw3dRect(hdc, prc->left, prc->top, prc->right, prc->bottom,
		::GetSysColor(COLOR_3DSHADOW),
		::GetSysColor(COLOR_3DHILIGHT)
	);
	SplitBoxWnd::Draw3dRect(hdc, prc->left + 1, prc->top + 1, prc->right - 2, prc->bottom - 2,
		RGB(0, 0, 0),
		::GetSysColor(COLOR_3DFACE)
	);
	return;
}


// �����g���b�J�[�̕\��
void SplitterWnd::DrawSplitter(int xPos, int yPos, int bEraseOld)
{
	RECT		rc;
	RECT		rc2;
	int			nTrackerWidth = 6;

	HDC hdc = ::GetDC(GetHwnd());
	HBRUSH hBrush = ::CreateSolidBrush(RGB(255, 255, 255));
	HBRUSH hBrushOld = (HBRUSH)::SelectObject(hdc, hBrush);
	::SetROP2(hdc, R2_XORPEN);
	::SetBkMode(hdc, TRANSPARENT);
	::GetClientRect(GetHwnd(), &rc);

	if (bEraseOld) {
		if (bDragging & 1) {	// �����o�[���h���b�O����
			rc2.left = -1;
			rc2.top = nDragPosY;
			rc2.right = rc.right;
			rc2.bottom = rc2.top + nTrackerWidth;
			::Rectangle(hdc, rc2.left, rc2.top, rc2.right, rc2.bottom);
		}
		if (bDragging & 2) {	// �����o�[���h���b�O����
			rc2.left = nDragPosX;
			rc2.top = 0;
			rc2.right = rc2.left + nTrackerWidth;
			rc2.bottom = rc.bottom;
			::Rectangle(hdc, rc2.left, rc2.top, rc2.right, rc2.bottom);
		}
	}

	nDragPosX = xPos;
	nDragPosY = yPos;
	if (bDragging & 1) {	// �����o�[���h���b�O����
		rc2.left = -1;
		rc2.top = nDragPosY;
		rc2.right = rc.right;
		rc2.bottom = rc2.top + nTrackerWidth;
		::Rectangle(hdc, rc2.left, rc2.top, rc2.right, rc2.bottom);
	}
	if (bDragging & 2) {	// �����o�[���h���b�O����
		rc2.left = nDragPosX;
		rc2.top = 0;
		rc2.right = rc2.left + nTrackerWidth;
		rc2.bottom = rc.bottom;
		::Rectangle(hdc, rc2.left, rc2.top, rc2.right, rc2.bottom);
	}

	::SelectObject(hdc, hBrushOld);
	::DeleteObject(hBrush);
	::ReleaseDC(GetHwnd(), hdc);
	return;
}


// �����o�[�ւ̃q�b�g�e�X�g
int SplitterWnd::HitTestSplitter(int xPos, int yPos)
{
	int		nFrameWidth = 3;
	int		nMargin = 2;

	if (nAllSplitRows == 1 && nAllSplitCols == 1) {
		return 0;
	}else
	if (nAllSplitRows == 2 && nAllSplitCols == 1) {
		if (nVSplitPos - nMargin < yPos && yPos < nVSplitPos + nFrameWidth + nMargin) {
			return 1;
		}else {
			return 0;
		}
	}else
	if (nAllSplitRows == 1 && nAllSplitCols == 2) {
		if (nHSplitPos - nMargin < xPos && xPos < nHSplitPos + nFrameWidth + nMargin) {
			return 2;
		}else {
			return 0;
		}
	}else {
		if (nVSplitPos - nMargin < yPos && yPos < nVSplitPos + nFrameWidth + nMargin &&
			nHSplitPos - nMargin < xPos && xPos < nHSplitPos + nFrameWidth + nMargin
		) {
			return 3;
		}else
		if (nVSplitPos - nMargin < yPos && yPos < nVSplitPos + nFrameWidth + nMargin) {
			return 1;
		}else
		if (nHSplitPos - nMargin < xPos && xPos < nHSplitPos + nFrameWidth + nMargin) {
			return 2;
		}else {
			return 0;
		}
	}
}

/*! �E�B���h�E�̕���
	@param nHorizontal �����N���C�A���g���W 1�ȏ�ŕ��� 0:�������Ȃ�  -1: �O�̐ݒ��ێ�
	@param nVertical   �����N���C�A���g���W 1�ȏ�ŕ��� 0:�������Ȃ�  -1: �O�̐ݒ��ێ�
*/
void SplitterWnd::DoSplit(int nHorizontal, int nVertical)
{
	int			nActivePane;
	int			nLimit = 32;
	RECT		rc;
	int			nAllSplitRowsOld = nAllSplitRows;	// �����s��
	int			nAllSplitColsOld = nAllSplitCols;	// ��������
	EditView*	pViewArr[MAXCOUNTOFVIEW];
	bool		bSizeBox;
	
	bool bVUp = false;
	bool bHUp = false;

	if (nHorizontal == -1 && nVertical == -1) {
		nVertical = nVSplitPos;		// ���������ʒu
		nHorizontal = nHSplitPos;		// ���������ʒu
	}

	if (nVertical != 0 || nHorizontal != 0) {
		// �����w���B�܂����쐬�Ȃ�2�ڈȍ~�̃r���[���쐬���܂�
		// ���̂Ƃ���͕������Ɋ֌W�Ȃ�4�܂ň�x�ɍ��܂��B
		pEditWnd->CreateEditViewBySplit(2*2);
	}
	/*
	|| �t�@���N�V�����L�[�����ɕ\�����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ�
	|| �X�e�[�^�X�p�[��\�����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ�
	*/
	if (!pEditWnd
		|| (
			pEditWnd->funcKeyWnd.GetHwnd()
	 		&& pShareData->common.window.nFuncKeyWnd_Place == 1	// �t�@���N�V�����L�[�\���ʒu�^0:�� 1:��
	 	)
	) {
		bSizeBox = false;
	}else if (pEditWnd->tabWnd.GetHwnd()
		&& pShareData->common.tabBar.eTabPosition == TabPosition::Bottom
	) {
		bSizeBox = false;
	}else {
		bSizeBox = true;
		// �X�e�[�^�X�p�[��\�����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ�
		if (pEditWnd->statusBar.GetStatusHwnd()) {
			bSizeBox = false;
		}
	}
	if (pEditWnd->dlgFuncList.GetHwnd()) {
		DockSideType eDockSideFL = pEditWnd->dlgFuncList.GetDockSide();
		if (eDockSideFL == DockSideType::Right || eDockSideFL == DockSideType::Bottom) {
			bSizeBox = false;
		}
	}
	if (pEditWnd->GetMiniMap().GetHwnd()) {
		bSizeBox = false;
	}
	// ���C���E�B���h�E���ő剻����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ�
	WINDOWPLACEMENT	wp;
	wp.length = sizeof(wp);
	::GetWindowPlacement(GetParentHwnd(), &wp);
	if (wp.showCmd == SW_SHOWMAXIMIZED) {
		bSizeBox = false;
	}

	int v;
	for (v=0; v<nChildWndCount; ++v) {
		pViewArr[v] = (EditView*)::GetWindowLongPtr(childWndArr[v], 0);
	}
	::GetClientRect(GetHwnd(), &rc);
	if (nHorizontal < nLimit) {
		if (nHorizontal > 0) {
			bHUp = true;
		}
		nHorizontal = 0;
	}
	if (nHorizontal > rc.right - nLimit * 2) {
		nHorizontal = 0;
	}
	if (nVertical < nLimit) {
		if (nVertical > 0) {
			bVUp = true;
		}
		nVertical = 0;
	}
	if (nVertical > rc.bottom - nLimit * 2) {
		nVertical = 0;
	}
	nVSplitPos = nVertical;		// ���������ʒu
	nHSplitPos = nHorizontal;		// ���������ʒu

	if (nVertical == 0 && nHorizontal == 0) {
		nAllSplitRows = 1;	// �����s��
		nAllSplitCols = 1;	// ��������
		if (childWndArr[0]) ::ShowWindow(childWndArr[0], SW_SHOW);
		if (childWndArr[1]) ::ShowWindow(childWndArr[1], SW_HIDE);
		if (childWndArr[2]) ::ShowWindow(childWndArr[2], SW_HIDE);
		if (childWndArr[3]) ::ShowWindow(childWndArr[3], SW_HIDE);

		if (pViewArr[0]) pViewArr[0]->SplitBoxOnOff(true, true, bSizeBox);	// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e
//		if (pViewArr[1]) pViewArr[1]->SplitBoxOnOff(false, false, false);	// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e
//		if (pViewArr[2]) pViewArr[2]->SplitBoxOnOff(false, false, false);	// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e
//		if (pViewArr[3]) pViewArr[3]->SplitBoxOnOff(false, false, false);	// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e

		OnSize(0, 0, 0, 0);

		if (nAllSplitRowsOld == 1 && nAllSplitColsOld == 1) {
		}else
		if (nAllSplitRowsOld > 1 && nAllSplitColsOld == 1) {
			if (bVUp) {
				// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
				if (pViewArr[2] && pViewArr[0]) {
					pViewArr[2]->CopyViewStatus(pViewArr[0]);
				}
			}else {
				// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
				if (this->nActivePane != 0 &&
					pViewArr[this->nActivePane] && pViewArr[0]
				) {
					pViewArr[this->nActivePane]->CopyViewStatus(pViewArr[0]);
				}
			}
		}else
		if (nAllSplitRowsOld == 1 && nAllSplitColsOld > 1) {
			if (bHUp) {
				// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
				if (pViewArr[1] && pViewArr[0]) {
					pViewArr[1]->CopyViewStatus(pViewArr[0]);
				}
			}else {
				// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
				if (this->nActivePane != 0 &&
					pViewArr[this->nActivePane] && pViewArr[0]
				) {
					pViewArr[this->nActivePane]->CopyViewStatus(pViewArr[0]);
				}
			}
		}else {
			if (!bVUp && !bHUp) {
				// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
				if (this->nActivePane != 0 &&
					pViewArr[this->nActivePane] && pViewArr[0]
				) {
					pViewArr[this->nActivePane]->CopyViewStatus(pViewArr[0]);
				}
			}else
			if (bVUp && !bHUp) {
				// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
				if (pViewArr[2] && pViewArr[0]) {
					pViewArr[2]->CopyViewStatus(pViewArr[0]);
				}
			}else
			if (!bVUp && bHUp) {
				// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
				if (pViewArr[1] && pViewArr[0]) {
					pViewArr[1]->CopyViewStatus(pViewArr[0]);
				}
			}else {
				// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
				if (pViewArr[3] && pViewArr[0]) {
					pViewArr[3]->CopyViewStatus(pViewArr[0]);
				}
			}
		}
		nActivePane = 0;
	}else
	if (nVertical > 0 &&  nHorizontal == 0) {
		nAllSplitRows = 2;	// �����s��
		nAllSplitCols = 1;	// ��������

		if (childWndArr[0]) ::ShowWindow(childWndArr[0], SW_SHOW);
		if (childWndArr[1]) ::ShowWindow(childWndArr[1], SW_HIDE);
		if (childWndArr[2]) ::ShowWindow(childWndArr[2], SW_SHOW);
		if (childWndArr[3]) ::ShowWindow(childWndArr[3], SW_HIDE);
		if (pViewArr[0]) pViewArr[0]->SplitBoxOnOff(false, false, false);	// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e
//		if (pViewArr[1]) pViewArr[1]->SplitBoxOnOff(false, false, false);	// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e
		if (pViewArr[2]) pViewArr[2]->SplitBoxOnOff(false, true, bSizeBox);	// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e
//		if (pViewArr[3]) pViewArr[3]->SplitBoxOnOff(false, false, false);	// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e

		OnSize(0, 0, 0, 0);

		if (nAllSplitRowsOld == 1 && nAllSplitColsOld == 1) {
			// �㉺�ɕ��������Ƃ�
			// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
			if (pViewArr[0] && pViewArr[2]) {
				pViewArr[0]->CopyViewStatus(pViewArr[2]);
			}
			// YAZAKI
			pViewArr[2]->GetTextArea().SetViewTopLine(pViewArr[0]->GetTextArea().GetViewTopLine() + pViewArr[0]->GetTextArea().nViewRowNum);
		}else if (nAllSplitRowsOld > 1 && nAllSplitColsOld == 1) {
		}else if (nAllSplitRowsOld == 1 && nAllSplitColsOld > 1) {
		}else {
			if (bHUp) {
				// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
				if (pViewArr[1] && pViewArr[0]) {
					pViewArr[1]->CopyViewStatus(pViewArr[0]);
				}
				// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
				if (pViewArr[3] && pViewArr[2]) {
					pViewArr[3]->CopyViewStatus(pViewArr[2]);
				}
			}else {
				// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
				if (this->nActivePane != 0 &&
					this->nActivePane != 2 &&
					pViewArr[0] &&
					pViewArr[1] &&
					pViewArr[2] &&
					pViewArr[3]
				) {
					pViewArr[1]->CopyViewStatus(pViewArr[0]);
					pViewArr[3]->CopyViewStatus(pViewArr[2]);
				}
			}
		}
		if (this->nActivePane == 0 || this->nActivePane == 1) {
			// 2007.10.01 ryoji
			// ������������̐ؑ֎��̂ݏ]���R�[�h�����s���ăA�N�e�B�u�y�C�������߂�B
			// ����ȊO�̏ꍇ�̓y�C��0���A�N�e�B�u�ɂ���B
			// �]���́A�㉺�ɕ������Ă����āA
			// �E�㉺�����o�[�𓮂���
			// �E�X�e�[�^�X�o�[�ȂǊe��o�[�̕\���^��\����؂�ւ���
			// �E�ݒ��ʂ�OK�ŕ���
			// �E���E���������č��E��������������
			// �Ƃ�������������邾���ŉ��̃y�C�����A�N�e�B�u������邱�Ƃ��������B
			// �i�V���v����0�Œ�ɂ��Ă��܂��Ă��ǂ��C�͂��邯��ǁD�D�D�j
			nActivePane = 0;
			if (nAllSplitRowsOld == 1 && nAllSplitColsOld == 1) {
				if (pViewArr[2]->GetTextArea().GetViewTopLine() < pViewArr[2]->GetCaret().GetCaretLayoutPos().y) {
					nActivePane = 2;
				}else {
					nActivePane = 0;
				}
			}
		}else {
			nActivePane = 2;
		}
	}else if (nVertical == 0 &&  nHorizontal > 0) {
		nAllSplitRows = 1;	// �����s��
		nAllSplitCols = 2;	// ��������

		if (childWndArr[0]) ::ShowWindow(childWndArr[0], SW_SHOW);
		if (childWndArr[1]) ::ShowWindow(childWndArr[1], SW_SHOW);
		if (childWndArr[2]) ::ShowWindow(childWndArr[2], SW_HIDE);
		if (childWndArr[3]) ::ShowWindow(childWndArr[3], SW_HIDE);
		if (pViewArr[0]) pViewArr[0]->SplitBoxOnOff(false, false, false);	// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e
		if (pViewArr[1]) pViewArr[1]->SplitBoxOnOff(true, false, bSizeBox);	// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e
//		if (pViewArr[2]) pViewArr[2]->SplitBoxOnOff(false, false);			// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e
//		if (pViewArr[3]) pViewArr[3]->SplitBoxOnOff(false, false);			// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e

		OnSize(0, 0, 0, 0);

		if (nAllSplitRowsOld == 1 && nAllSplitColsOld == 1) {
			// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
			if (pViewArr[0] && pViewArr[1]) {
				pViewArr[0]->CopyViewStatus(pViewArr[1]);
			}
		}else
		if (nAllSplitRowsOld > 1 && nAllSplitColsOld == 1) {
		}else
		if (nAllSplitRowsOld == 1 && nAllSplitColsOld > 1) {
		}else {
			if (bVUp) {
				// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
				if (pViewArr[2] && pViewArr[0]) {
					pViewArr[2]->CopyViewStatus(pViewArr[0]);
				}
				// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
				if (pViewArr[3] && pViewArr[1]) {
					pViewArr[3]->CopyViewStatus(pViewArr[1]);
				}
			}else {
				// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
				if (this->nActivePane != 0 &&
					this->nActivePane != 1 &&
					pViewArr[0] &&
					pViewArr[1] &&
					pViewArr[2] &&
					pViewArr[3]
				) {
					pViewArr[2]->CopyViewStatus(pViewArr[0]);
					pViewArr[3]->CopyViewStatus(pViewArr[1]);
				}
			}
		}
		if (this->nActivePane == 0 || this->nActivePane == 2) {
			nActivePane = 0;
		}else {
			nActivePane = 1;
		}
	}else {
		nAllSplitRows = 2;	// �����s��
		nAllSplitCols = 2;	// ��������
		if (childWndArr[0]) { ::ShowWindow(childWndArr[0], SW_SHOW); }
		if (childWndArr[1]) { ::ShowWindow(childWndArr[1], SW_SHOW); }
		if (childWndArr[2]) { ::ShowWindow(childWndArr[2], SW_SHOW); }
		if (childWndArr[3]) { ::ShowWindow(childWndArr[3], SW_SHOW); }
		if (pViewArr[0]) { pViewArr[0]->SplitBoxOnOff(false, false, false);}	// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e
		if (pViewArr[1]) { pViewArr[1]->SplitBoxOnOff(false, false, false);}	// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e
		if (pViewArr[2]) { pViewArr[2]->SplitBoxOnOff(false, false, false);}	// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e
		if (pViewArr[3]) { pViewArr[3]->SplitBoxOnOff(false, false, bSizeBox);}	// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e

		OnSize(0, 0, 0, 0);

		if (nAllSplitRowsOld == 1 && nAllSplitColsOld == 1) {
			// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
			if (pViewArr[0] && pViewArr[1]) {
				pViewArr[0]->CopyViewStatus(pViewArr[1]);
			}
			// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
			if (pViewArr[0] && pViewArr[2]) {
				pViewArr[0]->CopyViewStatus(pViewArr[2]);
			}
			// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
			if (pViewArr[0] && pViewArr[3]) {
				pViewArr[0]->CopyViewStatus(pViewArr[3]);
			}
		}else
		if (nAllSplitRowsOld > 1 && nAllSplitColsOld == 1) {
			// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
			if (pViewArr[0] && pViewArr[1]) {
				pViewArr[0]->CopyViewStatus(pViewArr[1]);
			}
			// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
			if (pViewArr[2] && pViewArr[3]) {
				pViewArr[2]->CopyViewStatus(pViewArr[3]);
			}
		}else
		if (nAllSplitRowsOld == 1 && nAllSplitColsOld > 1) {
			// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
			if (pViewArr[0] && pViewArr[2]) {
				pViewArr[0]->CopyViewStatus(pViewArr[2]);
			}
			// �y�C���̕\����Ԃ𑼂̃r���[�ɃR�s�[
			if (pViewArr[1] && pViewArr[3]) {
				pViewArr[1]->CopyViewStatus(pViewArr[3]);
			}
		}else {
		}
		nActivePane = this->nActivePane;
	}
	OnSize(0, 0, 0, 0);

	// �A�N�e�B�u�ɂȂ������Ƃ��y�C���ɒʒm
	if (childWndArr[nActivePane]) {
		::PostMessage(childWndArr[nActivePane], MYWM_SETACTIVEPANE, 0, 0);
	}

	return;
}

// �A�N�e�B�u�y�C���̐ݒ�
void SplitterWnd::SetActivePane(int nIndex)
{
	assert(nIndex < MAXCOUNTOFVIEW);
	nActivePane = nIndex;
	return;
}


// �c�����n�m�^�n�e�e
void SplitterWnd::VSplitOnOff(void)
{
	RECT rc;
	::GetClientRect(GetHwnd(), &rc);

	if (nAllSplitRows == 1 && nAllSplitCols == 1) {
		DoSplit(0, rc.bottom / 2);
	}else
	if (nAllSplitRows == 1 && nAllSplitCols > 1) {
		DoSplit(nHSplitPos, rc.bottom / 2);
	}else
	if (nAllSplitRows > 1 && nAllSplitCols == 1) {
		DoSplit(0, 0);
	}else {
		DoSplit(nHSplitPos, 0);
	}
	return;
}


// �������n�m�^�n�e�e
void SplitterWnd::HSplitOnOff(void)
{
	RECT rc;
	::GetClientRect(GetHwnd(), &rc);

	if (nAllSplitRows == 1 && nAllSplitCols == 1) {
		DoSplit(rc.right / 2, 0);
	}else
	if (nAllSplitRows == 1 && nAllSplitCols > 1) {
		DoSplit(0, 0);
	}else
	if (nAllSplitRows > 1 && nAllSplitCols == 1) {
		DoSplit(rc.right / 2 , nVSplitPos);
	}else {
		DoSplit(0, nVSplitPos);
	}
	return;
}


// �c�������n�m�^�n�e�e
void SplitterWnd::VHSplitOnOff(void)
{
	int		nX;
	int		nY;
	RECT	rc;
	::GetClientRect(GetHwnd(), &rc);

	if (nAllSplitRows > 1 && nAllSplitCols > 1) {
		nX = 0;
		nY = 0;
	}else {
		if (nAllSplitRows == 1) {
			nY = rc.bottom / 2;
		}else {
			nY = nVSplitPos;
		}
		if (nAllSplitCols == 1) {
			nX = rc.right / 2;
		}else {
			nX = nHSplitPos;
		}
	}
	DoSplit(nX, nY);

	return;
}


// �O�̃y�C����Ԃ�
int SplitterWnd::GetPrevPane(void)
{
	int nPane;
	nPane = -1;
	if (nAllSplitRows == 1 &&	nAllSplitCols == 1) {
		nPane = -1;
	}else
	if (nAllSplitRows == 2 &&	nAllSplitCols == 1) {
		switch (nActivePane) {
		case 0:
			nPane = -1;
			break;
		case 2:
			nPane = 0;
			break;
		}
	}else
	if (nAllSplitRows == 1 &&	nAllSplitCols == 2) {
		switch (nActivePane) {
		case 0:
			nPane = -1;
			break;
		case 1:
			nPane = 0;
			break;
		}
	}else {
		switch (nActivePane) {
		case 0:
			nPane = -1;
			break;
		case 1:
			nPane = 0;
			break;
		case 2:
			nPane = 1;
			break;
		case 3:
			nPane = 2;
			break;
		}
	}
	return nPane;
}


// ���̃y�C����Ԃ�
int SplitterWnd::GetNextPane(void)
{
	int nPane;
	nPane = -1;
	if (nAllSplitRows == 1 &&	nAllSplitCols == 1) {
		nPane = -1;
	}else
	if (nAllSplitRows == 2 &&	nAllSplitCols == 1) {
		switch (nActivePane) {
		case 0:
			nPane = 2;
			break;
		case 2:
			nPane = -1;
			break;
		}
	}else
	if (nAllSplitRows == 1 &&	nAllSplitCols == 2) {
		switch (nActivePane) {
		case 0:
			nPane = 1;
			break;
		case 1:
			nPane = -1;
			break;
		}
	}else {
		switch (nActivePane) {
		case 0:
			nPane = 1;
			break;
		case 1:
			nPane = 2;
			break;
		case 2:
			nPane = 3;
			break;
		case 3:
			nPane = -1;
			break;
		}
	}
	return nPane;
}


// �ŏ��̃y�C����Ԃ�
int SplitterWnd::GetFirstPane(void)
{
	return 0;
}


// �Ō�̃y�C����Ԃ�
int SplitterWnd::GetLastPane(void)
{
	int nPane;
	if (nAllSplitRows == 1 &&	nAllSplitCols == 1) {
		nPane = 0;
	}else
	if (nAllSplitRows == 1 &&	nAllSplitCols == 2) {
		nPane = 1;
	}else
	if (nAllSplitRows == 2 &&	nAllSplitCols == 1) {
		nPane = 2;
	}else {
		nPane = 3;
	}
	return nPane;
}


// �`�揈��
LRESULT SplitterWnd::OnPaint(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	RECT		rc;
	RECT		rcFrame;
	int			nFrameWidth = 3;
	HBRUSH		hBrush;
	hdc = ::BeginPaint(hwnd, &ps);
	::GetClientRect(GetHwnd(), &rc);
	hBrush = ::CreateSolidBrush(::GetSysColor(COLOR_3DFACE));
	if (nAllSplitRows > 1) {
		::SetRect(&rcFrame, rc.left, nVSplitPos, rc.right, nVSplitPos + nFrameWidth);
		::FillRect(hdc, &rcFrame, hBrush);
	}
	if (nAllSplitCols > 1) {
		::SetRect(&rcFrame, nHSplitPos, rc.top, nHSplitPos + nFrameWidth, rc.bottom);
		::FillRect(hdc, &rcFrame, hBrush);
	}
	::DeleteObject(hBrush);
	::EndPaint(hwnd, &ps);
	return 0L;
}


// �E�B���h�E�T�C�Y�̕ύX����
LRESULT SplitterWnd::OnSize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	EditView*	pViewArr[MAXCOUNTOFVIEW];
	int			nFrameWidth = 3;
	bool		bSizeBox;
	for (int i=0; i<nChildWndCount; ++i) {
		pViewArr[i] = (EditView*)::GetWindowLongPtr(childWndArr[i], 0);
	}

	/*
	|| �t�@���N�V�����L�[�����ɕ\�����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ�
	|| �X�e�[�^�X�p�[��\�����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ�
	*/
	if (!pEditWnd
	 	|| (
	 		pEditWnd->funcKeyWnd.GetHwnd()
	  		&& pShareData->common.window.nFuncKeyWnd_Place == 1	// �t�@���N�V�����L�[�\���ʒu�^0:�� 1:��
	 	)
	) {
		bSizeBox = false;
	}else if (pEditWnd->tabWnd.GetHwnd()
		&& pShareData->common.tabBar.eTabPosition == TabPosition::Bottom
	) {
		bSizeBox = false;
	}else {
		bSizeBox = true;
		// �X�e�[�^�X�p�[��\�����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ�
		if (pEditWnd->statusBar.GetStatusHwnd()) {
			bSizeBox = false;
		}
	}
	if (pEditWnd->dlgFuncList.GetHwnd()) {
		DockSideType eDockSideFL = pEditWnd->dlgFuncList.GetDockSide();
		if (eDockSideFL == DockSideType::Right || eDockSideFL == DockSideType::Bottom) {
			bSizeBox = false;
		}
	}
	if (pEditWnd->GetMiniMap().GetHwnd()) {
		bSizeBox = false;
	}

	// ���C���E�B���h�E���ő剻����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ�
	WINDOWPLACEMENT	wp;
	wp.length = sizeof(wp);
	::GetWindowPlacement(GetParentHwnd(), &wp);
	if (wp.showCmd == SW_SHOWMAXIMIZED) {
		bSizeBox = false;
	}

	RECT rcClient;
	::GetClientRect(GetHwnd(), &rcClient);

	if (nAllSplitRows == 1 && nAllSplitCols == 1) {
		if (childWndArr[0]) {
			::MoveWindow(childWndArr[0], 0, 0, rcClient.right,  rcClient.bottom, TRUE);		// �q�E�B���h�E�z��

			pViewArr[0]->SplitBoxOnOff(true, true, bSizeBox);	// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e
		}
	}else
	if (nAllSplitRows == 2 && nAllSplitCols == 1) {
		if (childWndArr[0]) {
			::MoveWindow(childWndArr[0], 0, 0, rcClient.right,  nVSplitPos, TRUE);		// �q�E�B���h�E�z��
			pViewArr[0]->SplitBoxOnOff(false, false, false);		// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e
		}
		if (childWndArr[2]) {
			::MoveWindow(childWndArr[2], 0, nVSplitPos + nFrameWidth, rcClient.right, rcClient.bottom - (nVSplitPos + nFrameWidth), TRUE);			// �q�E�B���h�E�z��
			pViewArr[2]->SplitBoxOnOff(false, true, bSizeBox);	// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e
		}
	}else
	if (nAllSplitRows == 1 && nAllSplitCols == 2) {
		if (childWndArr[0]) {
			::MoveWindow(childWndArr[0], 0, 0, nHSplitPos, rcClient.bottom, TRUE);		// �q�E�B���h�E�z��
			pViewArr[0]->SplitBoxOnOff(false, false, false);		// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e
		}
		if (childWndArr[1]) {
			::MoveWindow(childWndArr[1], nHSplitPos + nFrameWidth, 0, rcClient.right - (nHSplitPos + nFrameWidth),  rcClient.bottom, TRUE);			// �q�E�B���h�E�z��
			pViewArr[1]->SplitBoxOnOff(true, false, bSizeBox);	// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e
		}
	}else {
		if (childWndArr[0]) {
			::MoveWindow(childWndArr[0], 0, 0, nHSplitPos,  nVSplitPos, TRUE);			// �q�E�B���h�E�z��
			pViewArr[0]->SplitBoxOnOff(false, false, false);		// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e
		}
		if (childWndArr[1]) {
			::MoveWindow(childWndArr[1], nHSplitPos + nFrameWidth, 0, rcClient.right - (nHSplitPos + nFrameWidth),  nVSplitPos, TRUE);				// �q�E�B���h�E�z��
			pViewArr[1]->SplitBoxOnOff(false, false, false);		// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e
		}
		if (childWndArr[2]) {
			::MoveWindow(childWndArr[2], 0, nVSplitPos + nFrameWidth , nHSplitPos,  rcClient.bottom - (nVSplitPos + nFrameWidth), TRUE);			// �q�E�B���h�E�z��
			pViewArr[2]->SplitBoxOnOff(false, false, false);		// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e
		}
		if (childWndArr[3]) {
			::MoveWindow(childWndArr[3], nHSplitPos + nFrameWidth, nVSplitPos + nFrameWidth, rcClient.right - (nHSplitPos + nFrameWidth),  rcClient.bottom - (nVSplitPos + nFrameWidth), TRUE);			// �q�E�B���h�E�z��
			pViewArr[3]->SplitBoxOnOff(false, false, bSizeBox);	// �c�E���̕����{�b�N�X�̂n�m�^�n�e�e
		}
	}
	// �f�X�N�g�b�v��������̂ł���!
	//::InvalidateRect(GetHwnd(), NULL, TRUE);	// �ĕ`�悵�ĂˁB	//@@@ 2003.06.11 MIK
	return 0L;
}


// �}�E�X�ړ����̏���
LRESULT SplitterWnd::OnMouseMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int xPos = (int)(short)LOWORD(lParam);
	int yPos = (int)(short)HIWORD(lParam);
	int nHit = HitTestSplitter(xPos, yPos);
	switch (nHit) {
	case 1:
		::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
		break;
	case 2:
		::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
		break;
	case 3:
		::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
		break;
	}
	if (bDragging != 0) {		// �����o�[���h���b�O����
		RECT rc;
		::GetClientRect(GetHwnd(), &rc);
		if (xPos < 1) {
			xPos = 1;
		}
		if (xPos > rc.right - 6) {
			xPos = rc.right - 6;
		}
		if (yPos < 1) {
			yPos = 1;
		}
		if (yPos > rc.bottom - 6) {
			yPos = rc.bottom - 6;
		}
		// �����g���b�J�[�̕\��
		DrawSplitter(xPos, yPos, TRUE);
//		MYTRACE(_T("xPos=%d yPos=%d \n"), xPos, yPos);
	}
	return 0L;
}


// �}�E�X���{�^���������̏���
LRESULT SplitterWnd::OnLButtonDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int xPos = (int)(short)LOWORD(lParam);
	int yPos = (int)(short)HIWORD(lParam);
	::SetFocus(GetParentHwnd());
	// �����o�[�ւ̃q�b�g�e�X�g
	int nHit = HitTestSplitter(xPos, yPos);
	if (nHit != 0) {
		bDragging = nHit;	// �����o�[���h���b�O����
		::SetCapture(GetHwnd());
	}
	// �����g���b�J�[�̕\��
	DrawSplitter(xPos, yPos, FALSE);

	return 0L;
}



// �}�E�X���{�^��������̏���
LRESULT SplitterWnd::OnLButtonUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (bDragging) {
		// �����g���b�J�[�̕\��
		DrawSplitter(nDragPosX, nDragPosY, FALSE);
		int bDraggingOld = bDragging;
		bDragging = 0;
		::ReleaseCapture();
		if (hcurOld) {
			::SetCursor(hcurOld);
		}
		int nX;
		int nY;
		// �E�B���h�E�̕���
		if (nAllSplitRows == 1) {
			nY = 0;
		}else {
			nY = nDragPosY;
		}
		if (nAllSplitCols == 1) {
			nX = 0;
		}else {
			nX = nDragPosX;
		}
		if (bDraggingOld == 1) {
			DoSplit(nHSplitPos, nY);
		}else
		if (bDraggingOld == 2) {
			DoSplit(nX, nVSplitPos);
		}else
		if (bDraggingOld == 3) {
			DoSplit(nX, nY);
		}
	}
	return 0L;
}



// �}�E�X���{�^���_�u���N���b�N���̏���
LRESULT SplitterWnd::OnLButtonDblClk(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int nX;
	int nY;
	int	xPos = (int)(short)LOWORD(lParam);
	int	yPos = (int)(short)HIWORD(lParam);
	int	nHit = HitTestSplitter(xPos, yPos);
	if (nHit == 1) {
		if (nAllSplitCols == 1) {
			nX = 0;
		}else {
			nX = nHSplitPos;
		}
		DoSplit(nX , 0);
	}else
	if (nHit == 2) {
		if (nAllSplitRows == 1) {
			nY = 0;
		}else {
			nY = nVSplitPos;
		}
		DoSplit(0 , nY);
	}else
	if (nHit == 3) {
		DoSplit(0 , 0);
	}
	OnMouseMove(GetHwnd(), 0, 0, MAKELONG(xPos, yPos));
	return 0L;
}


// �A�v���P�[�V������`�̃��b�Z�[�W(WM_APP <= msg <= 0xBFFF)
LRESULT SplitterWnd::DispatchEvent_WM_APP(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int nPosX;
	int nPosY;
	switch (uMsg) {
	case MYWM_DOSPLIT:
		nPosX = (int)wParam;
		nPosY = (int)lParam;
//		MYTRACE(_T("MYWM_DOSPLIT nPosX=%d nPosY=%d\n"), nPosX, nPosY);

		// �E�B���h�E�̕���
		if (nHSplitPos != 0) {
			nPosX = nHSplitPos;
		}
		if (nVSplitPos != 0) {
			nPosY = nVSplitPos;
		}
		DoSplit(nPosX , nPosY);
		break;
	case MYWM_SETACTIVEPANE:
		SetActivePane((int)wParam);
		break;
	}
	return 0L;
}

