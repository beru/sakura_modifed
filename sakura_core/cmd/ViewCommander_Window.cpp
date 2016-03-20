/*!	@file
@brief ViewCommander�N���X�̃R�}���h(�E�B���h�E�n)�֐��Q

	2012/12/15	ViewCommander.cpp,ViewCommander_New.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2001, MIK
	Copyright (C) 2002, YAZAKI, genta, MIK
	Copyright (C) 2003, MIK, genta
	Copyright (C) 2004, Moca, genta, crayonzen, Kazika
	Copyright (C) 2006, genta, ryoji, maru
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2008, syat
	Copyright (C) 2009, syat
	Copyright (C) 2010, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "_main/ControlTray.h"
#include "util/os.h"
#include "env/SakuraEnvironment.h"
#include "env/ShareData.h"


// �㉺�ɕ���	// Sept. 17, 2000 jepro �����́u�c�v���u�㉺�Ɂv�ɕύX
void ViewCommander::Command_SPLIT_V(void)
{
	GetEditWindow().m_splitterWnd.VSplitOnOff();
	return;
}


// ���E�ɕ���	// Sept. 17, 2000 jepro �����́u���v���u���E�Ɂv�ɕύX
void ViewCommander::Command_SPLIT_H(void)
{
	GetEditWindow().m_splitterWnd.HSplitOnOff();
	return;
}


// �c���ɕ���	// Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�
void ViewCommander::Command_SPLIT_VH(void)
{
	GetEditWindow().m_splitterWnd.VHSplitOnOff();
	return;
}


// �E�B���h�E�����
void ViewCommander::Command_WINCLOSE(void)
{
	// ����
	::PostMessage(GetMainWindow(), MYWM_CLOSE, FALSE, 								// 2007.02.13 ryoji WM_CLOSE��MYWM_CLOSE�ɕύX
		(LPARAM)AppNodeManager::getInstance().GetNextTab(GetMainWindow()));	// �^�u�܂Ƃߎ��A���̃^�u�Ɉړ�	2013/4/10 Uchi
	return;
}


// ���ׂẴE�B���h�E�����	// Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
void ViewCommander::Command_FILECLOSEALL(void)
{
	int nGroup = AppNodeManager::getInstance().GetEditNode(GetMainWindow())->GetGroup();
	ControlTray::CloseAllEditor(true, GetMainWindow(), false, nGroup);	// 2006.12.25, 2007.02.13 ryoji �����ǉ�
	return;
}


// ���̃^�u�ȊO�����			// 2008.11.22 syat
// 2009.12.26 syat ���̃E�B���h�E�ȊO�����Ƃ̌��p��
void ViewCommander::Command_TAB_CLOSEOTHER(void)
{
	int nGroup = 0;

	// �E�B���h�E�ꗗ���擾����
	EditNode* pEditNode;
	int nCount = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNode, true);
	if (0 >= nCount) return;

	for (int i=0; i<nCount; ++i) {
		auto& node = pEditNode[i];
		if (node.hWnd == GetMainWindow()) {
			node.hWnd = NULL;		// �������g�͕��Ȃ�
			nGroup = node.nGroup;
		}
	}

	// �I���v�����o��
	AppNodeGroupHandle(nGroup).RequestCloseEditor(pEditNode, nCount, false, true, GetMainWindow());
	delete[] pEditNode;
	return;
}


/*!	@brief �E�B���h�E�ꗗ�|�b�v�A�b�v�\�������i�t�@�C�����̂݁j
	@date  2006.03.23 fon �V�K�쐬
	@date  2006.05.19 genta �R�}���h���s�v����\�������ǉ�
	@date  2007.07.07 genta �R�}���h���s�v���̒l��ύX
*/
void ViewCommander::Command_WINLIST(int nCommandFrom)
{
	// �E�B���h�E�ꗗ���|�b�v�A�b�v�\������
	GetEditWindow().PopupWinList((nCommandFrom & FA_FROMKEYBOARD) != FA_FROMKEYBOARD);
	// 2007.02.27 ryoji �A�N�Z�����[�^�L�[����łȂ���΃}�E�X�ʒu��
}


/*!	@brief �d�˂ĕ\��

	@date 2002.01.08 YAZAKI �u���E�ɕ��ׂĕ\���v����ƁA
		���ōő剻����Ă���G�N�X�v���[�����u���̑傫���v�ɂȂ�o�O�C���B
	@date 2003.06.12 MIK �^�u�E�B���h�E���͓��삵�Ȃ��悤��
	@date 2004.03.19 crayonzen �J�����g�E�B���h�E���Ō�ɔz�u�D
		�E�B���h�E�������ꍇ��2���ڈȍ~�͉E�ɂ��炵�Ĕz�u�D
	@date 2004.03.20 genta Z-Order�̏ォ�珇�ɕ��ׂĂ����悤�ɁD(SetWindowPos�𗘗p)
	@date 2007.06.20 ryoji �^�u���[�h�͉��������O���[�v�P�ʂŕ��ׂ�
*/
void ViewCommander::Command_CASCADE(void)
{
	// ���݊J���Ă���ҏW���̃��X�g���擾����
	EditNode* pEditNodeArr;
	int nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true/*false*/, true);

	if (nRowNum > 0) {
		struct WNDARR {
			HWND	hWnd;
			int		newX;
			int		newY;
		};
		std::vector<WNDARR> wndArr(nRowNum);
		WNDARR*	pWndArr = &wndArr[0];
		int count = 0;	// �����ΏۃE�B���h�E�J�E���g
		// Mar. 20, 2004 genta ���݂̃E�B���h�E�𖖔��Ɏ����Ă����̂Ɏg��
		int		current_win_index = -1;

		// -----------------------------------------
		// �E�B���h�E(�n���h��)���X�g�̍쐬
		// -----------------------------------------

		for (int i=0; i<nRowNum; ++i) {
			auto editNodeHWnd = pEditNodeArr[i].GetHwnd();
			if (::IsIconic(editNodeHWnd)) {	// �ŏ������Ă���E�B���h�E�͖����B
				continue;
			}
			if (!::IsWindowVisible(editNodeHWnd)) {	// �s���E�B���h�E�͖����B
				continue;
			}
			// Mar. 20, 2004 genta
			// ���݂̃E�B���h�E�𖖔��Ɏ����Ă������߂����ł̓X�L�b�v
			if (editNodeHWnd == EditWnd::getInstance().GetHwnd()) {
				current_win_index = i;
				continue;
			}
			pWndArr[count].hWnd = editNodeHWnd;
			++count;
		}

		// Mar. 20, 2004 genta
		// ���݂̃E�B���h�E�𖖔��ɑ}�� inspired by crayonzen
		if (current_win_index >= 0) {
			pWndArr[count].hWnd = pEditNodeArr[current_win_index].GetHwnd();
			++count;
		}

		// �f�X�N�g�b�v�T�C�Y�𓾂�
		RECT rcDesktop;
		// May 01, 2004 genta �}���`���j�^�Ή�
		::GetMonitorWorkRect(m_view.GetHwnd(), &rcDesktop);
		
		int width = (rcDesktop.right - rcDesktop.left) * 4 / 5; // Mar. 9, 2003 genta �������Z�݂̂ɂ���
		int height = (rcDesktop.bottom - rcDesktop.top) * 4 / 5;
		int w_delta = ::GetSystemMetrics(SM_CXSIZEFRAME) + ::GetSystemMetrics(SM_CXSIZE);
		int h_delta = ::GetSystemMetrics(SM_CYSIZEFRAME) + ::GetSystemMetrics(SM_CYSIZE);
		int w_offset = rcDesktop.left; // Mar. 19, 2004 crayonzen ��Βl���ƃG�N�X�v���[���[�̃E�B���h�E�ɏd�Ȃ�̂�
		int h_offset = rcDesktop.top; // �����l���f�X�N�g�b�v���Ɏ��߂�B

		// -----------------------------------------
		// ���W�v�Z
		//
		// Mar. 19, 2004 crayonzen
		//		������f�X�N�g�b�v�̈�ɍ��킹��(�^�X�N�o�[����E���ɂ���ꍇ�̂���)�D
		//		�E�B���h�E���E������͂ݏo���獶��ɖ߂邪�C
		//		2���ڈȍ~�͊J�n�ʒu���E�ɂ��炵�ăA�C�R����������悤�ɂ���D
		//
		// Mar. 20, 2004 genta �����ł͌v�Z�l��ۊǂ��邾���ŃE�B���h�E�̍Ĕz�u�͍s��Ȃ�
		// -----------------------------------------

		int roundtrip = 0; // �Q�x�ڂ̕`��ȍ~�Ŏg�p����J�E���g
		int sw_offset = w_delta; // �E�X���C�h�̕�

		for (int i=0; i<count; ++i) {
			if (w_offset + width > rcDesktop.right || h_offset + height > rcDesktop.bottom) {
				++roundtrip;
				if ((rcDesktop.right - rcDesktop.left) - sw_offset * roundtrip < width) {
					// ����ȏ�E�ɂ��点�Ȃ��Ƃ��͂��傤���Ȃ����獶��ɖ߂�
					roundtrip = 0;
				}
				// �E�B���h�E�̈�̍���ɃZ�b�g
				// craonzen �����l�C��(�Q�x�ڈȍ~�̕`��ŏ����ÂX���C�h)
				w_offset = rcDesktop.left + sw_offset * roundtrip;
				h_offset = rcDesktop.top;
			}
			
			pWndArr[i].newX = w_offset;
			pWndArr[i].newY = h_offset;

			w_offset += w_delta;
			h_offset += h_delta;
		}

		// -----------------------------------------
		// �ő剻/��\������
		// �ő剻���ꂽ�E�B���h�E�����ɖ߂��D���ꂪ�Ȃ��ƁC�ő剻�E�B���h�E��
		// �ő剻��Ԃ̂܂ܕ��ёւ����Ă��܂��C���̌�ő剻���삪�ςɂȂ�D
		//
		// Sep. 04, 2004 genta
		// -----------------------------------------
		for (int i=0; i<count; ++i) {
			::ShowWindow(pWndArr[i].hWnd, SW_RESTORE | SW_SHOWNA);
		}

		// -----------------------------------------
		// �E�B���h�E�z�u
		//
		// Mar. 20, 2004 genta API��f���Ɏg����Z-Order�̏ォ�牺�̏��ŕ��ׂ�D
		// -----------------------------------------

		// �܂��J�����g���őO�ʂ�
		int i = count - 1;
		
		::SetWindowPos(
			pWndArr[i].hWnd, HWND_TOP,
			pWndArr[i].newX, pWndArr[i].newY,
			width, height,
			0
		);

		// �c���1�����ɓ���Ă���
		while (--i >= 0) {
			::SetWindowPos(
				pWndArr[i].hWnd, pWndArr[i + 1].hWnd,
				pWndArr[i].newX, pWndArr[i].newY,
				width, height,
				SWP_NOACTIVATE
			);
		}

		delete [] pEditNodeArr;
	}
	return;
}


// �㉺�ɕ��ׂĕ\��
void ViewCommander::Command_TILE_V(void)
{
	// ���݊J���Ă���ҏW���̃��X�g���擾����
	EditNode* pEditNodeArr;
	int nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true/*false*/, true);

	if (nRowNum > 0) {
		std::vector<HWND> hWnds(nRowNum);
		HWND* phwndArr = &hWnds[0];
		int count = 0;
		// �f�X�N�g�b�v�T�C�Y�𓾂�
		RECT rcDesktop;
		// May 01, 2004 genta �}���`���j�^�Ή�
		::GetMonitorWorkRect(m_view.GetHwnd(), &rcDesktop);
		for (int i=0; i<nRowNum; ++i) {
			auto editNodeHWnd = pEditNodeArr[i].GetHwnd();
			if (::IsIconic(editNodeHWnd)) {	// �ŏ������Ă���E�B���h�E�͖����B
				continue;
			}
			if (!::IsWindowVisible(editNodeHWnd)) {	// �s���E�B���h�E�͖����B
				continue;
			}
			// From Here Jul. 28, 2002 genta
			// ���݂̃E�B���h�E��擪�Ɏ����Ă���
			if (editNodeHWnd == EditWnd::getInstance().GetHwnd()) {
				phwndArr[count] = phwndArr[0];
				phwndArr[0] = editNodeHWnd;
			}else {
				phwndArr[count] = editNodeHWnd;
			}
			// To Here Jul. 28, 2002 genta
			++count;
		}
		int height = (rcDesktop.bottom - rcDesktop.top) / count;
		for (int i=0; i<count; ++i) {
			// Jul. 21, 2002 genta
			::ShowWindow(phwndArr[i], SW_RESTORE);
			::SetWindowPos(
				phwndArr[i], 0,
				rcDesktop.left, rcDesktop.top + height * i, // Mar. 19, 2004 crayonzen ��[����
				rcDesktop.right - rcDesktop.left, height,
				SWP_NOOWNERZORDER | SWP_NOZORDER
			);
		}
		::SetFocus(phwndArr[0]);	// Aug. 17, 2002 MIK

		delete [] pEditNodeArr;
	}
	return;
}


// ���E�ɕ��ׂĕ\��
void ViewCommander::Command_TILE_H(void)
{
	// ���݊J���Ă���ҏW���̃��X�g���擾����
	EditNode* pEditNodeArr;
	int nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true/*false*/, true);

	if (nRowNum > 0) {
		std::vector<HWND> hWnds(nRowNum);
		HWND* phwndArr = &hWnds[0];
		int count = 0;
		// �f�X�N�g�b�v�T�C�Y�𓾂�
		RECT rcDesktop;
		// May 01, 2004 genta �}���`���j�^�Ή�
		::GetMonitorWorkRect(m_view.GetHwnd(), &rcDesktop);
		for (int i=0; i<nRowNum; ++i) {
			auto editNodeHWnd = pEditNodeArr[i].GetHwnd();
			if (::IsIconic(editNodeHWnd)) {	// �ŏ������Ă���E�B���h�E�͖����B
				continue;
			}
			if (!::IsWindowVisible(editNodeHWnd)) {	// �s���E�B���h�E�͖����B
				continue;
			}
			// From Here Jul. 28, 2002 genta
			// ���݂̃E�B���h�E��擪�Ɏ����Ă���
			if (editNodeHWnd == EditWnd::getInstance().GetHwnd()) {
				phwndArr[count] = phwndArr[0];
				phwndArr[0] = editNodeHWnd;
			}else {
				phwndArr[count] = editNodeHWnd;
			}
			// To Here Jul. 28, 2002 genta
			++count;
		}
		int width = (rcDesktop.right - rcDesktop.left) / count;
		for (int i=0; i<count; ++i) {
			// Jul. 21, 2002 genta
			::ShowWindow(phwndArr[i], SW_RESTORE);
			::SetWindowPos(
				phwndArr[i], 0,
				width * i + rcDesktop.left, rcDesktop.top, // Oct. 18, 2003 genta �^�X�N�o�[�����ɂ���ꍇ���l��
				width, rcDesktop.bottom - rcDesktop.top,
				SWP_NOOWNERZORDER | SWP_NOZORDER
			);
		}
		::SetFocus(phwndArr[0]);	// Aug. 17, 2002 MIK
		delete [] pEditNodeArr;
	}
	return;
}


// from ViewCommander_New.cpp
/*! ��Ɏ�O�ɕ\��
	@date 2004.09.21 Moca
*/
void ViewCommander::Command_WINTOPMOST(LPARAM lparam)
{
	GetEditWindow().WindowTopMost(int(lparam));
}


// Start 2004.07.14 Kazika �ǉ�
/*!	@brief �������ĕ\��

	�^�u�E�B���h�E�̌����A�񌋍���؂�ւ���R�}���h�ł��B
	[���ʐݒ�]->[�E�B���h�E]->[�^�u�\�� �܂Ƃ߂Ȃ�]�̐؂�ւ��Ɠ����ł��B
	@author Kazika
	@date 2004.07.14 Kazika �V�K�쐬
	@date 2007.06.20 ryoji GetDllShareData().m_TabWndWndpl�̔p�~�C�O���[�vID���Z�b�g
*/
void ViewCommander::Command_BIND_WINDOW(void)
{
	// �^�u���[�h�ł���Ȃ��
	auto& csTabBar = GetDllShareData().common.tabBar;
	if (csTabBar.bDispTabWnd) {
		// �^�u�E�B���h�E�̐ݒ��ύX
		csTabBar.bDispTabWndMultiWin = !csTabBar.bDispTabWndMultiWin;

		// �܂Ƃ߂�Ƃ��� WS_EX_TOPMOST ��Ԃ𓯊�����	// 2007.05.18 ryoji
		if (!csTabBar.bDispTabWndMultiWin) {
			GetEditWindow().WindowTopMost(
				((DWORD)::GetWindowLongPtr(GetEditWindow().GetHwnd(), GWL_EXSTYLE) & WS_EX_TOPMOST)? 1: 2
			);
		}

		// Start 2004.08.27 Kazika �ύX
		// �^�u�E�B���h�E�̐ݒ��ύX���u���[�h�L���X�g����
		AppNodeManager::getInstance().ResetGroupId();
		AppNodeGroupHandle(0).PostMessageToAllEditors(
			MYWM_TAB_WINDOW_NOTIFY,						// �^�u�E�B���h�E�C�x���g
			(WPARAM)((csTabBar.bDispTabWndMultiWin) ? TabWndNotifyType::Disable : TabWndNotifyType::Enable), // �^�u���[�h�L��/�������C�x���g
			(LPARAM)GetEditWindow().GetHwnd(),	// EditWnd�̃E�B���h�E�n���h��
			m_view.GetHwnd());									// �������g
		// End 2004.08.27 Kazika
	}
}
// End 2004.07.14 Kazika


// �O���[�v�����		// 2007.06.20 ryoji �ǉ�
void ViewCommander::Command_GROUPCLOSE(void)
{
	auto& csTabBar = GetDllShareData().common.tabBar;
	if (
		csTabBar.bDispTabWnd
		&& !csTabBar.bDispTabWndMultiWin
	) {
		int nGroup = AppNodeManager::getInstance().GetEditNode(GetMainWindow())->GetGroup();
		ControlTray::CloseAllEditor(true, GetMainWindow(), true, nGroup);
	}
	return;
}


// ���̃O���[�v			// 2007.06.20 ryoji
void ViewCommander::Command_NEXTGROUP(void)
{
	auto& tabWnd = GetEditWindow().m_tabWnd;
	if (!tabWnd.GetHwnd()) {
		return;
	}
	tabWnd.NextGroup();
}


// �O�̃O���[�v			// 2007.06.20 ryoji
void ViewCommander::Command_PREVGROUP(void)
{
	auto& tabWnd = GetEditWindow().m_tabWnd;
	if (!tabWnd.GetHwnd()) {
		return;
	}
	tabWnd.PrevGroup();
}


// �^�u���E�Ɉړ�		// 2007.06.20 ryoji
void ViewCommander::Command_TAB_MOVERIGHT(void)
{
	auto& tabWnd = GetEditWindow().m_tabWnd;
	if (!tabWnd.GetHwnd()) {
		return;
	}
	tabWnd.MoveRight();
}


// �^�u�����Ɉړ�		// 2007.06.20 ryoji
void ViewCommander::Command_TAB_MOVELEFT(void)
{
	auto& tabWnd = GetEditWindow().m_tabWnd;
	if (!tabWnd.GetHwnd()) {
		return;
	}
	tabWnd.MoveLeft();
}


// �V�K�O���[�v			// 2007.06.20 ryoji
void ViewCommander::Command_TAB_SEPARATE(void)
{
	auto& tabWnd = GetEditWindow().m_tabWnd;
	if (!tabWnd.GetHwnd()) {
		return;
	}
	tabWnd.Separate();
}


// ���̃O���[�v�Ɉړ�	// 2007.06.20 ryoji
void ViewCommander::Command_TAB_JOINTNEXT(void)
{
	auto& tabWnd = GetEditWindow().m_tabWnd;
	if (!tabWnd.GetHwnd()) {
		return;
	}
	tabWnd.JoinNext();
}


// �O�̃O���[�v�Ɉړ�	// 2007.06.20 ryoji
void ViewCommander::Command_TAB_JOINTPREV(void)
{
	auto& tabWnd = GetEditWindow().m_tabWnd;
	if (!tabWnd.GetHwnd()) {
		return;
	}
	tabWnd.JoinPrev();
}


// �������ׂĕ���		// 2008.11.22 syat
void ViewCommander::Command_TAB_CLOSELEFT(void)
{
	if (GetDllShareData().common.tabBar.bDispTabWnd) {
		int nGroup = 0;

		// �E�B���h�E�ꗗ���擾����
		EditNode* pEditNode;
		int nCount = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNode, true);
		bool bSelfFound = false;
		if (0 >= nCount) return;

		for (int i=0; i<nCount; ++i) {
			if (pEditNode[i].hWnd == GetMainWindow()) {
				pEditNode[i].hWnd = NULL;		// �������g�͕��Ȃ�
				nGroup = pEditNode[i].nGroup;
				bSelfFound = true;
			}else if (bSelfFound) {
				pEditNode[i].hWnd = NULL;		// �E�͕��Ȃ�
			}
		}

		// �I���v�����o��
		AppNodeGroupHandle(nGroup).RequestCloseEditor(pEditNode, nCount, false, true, GetMainWindow());
		delete []pEditNode;
	}
	return;
}


// �E�����ׂĕ���		// 2008.11.22 syat
void ViewCommander::Command_TAB_CLOSERIGHT(void)
{
	if (GetDllShareData().common.tabBar.bDispTabWnd) {
		int nGroup = 0;

		// �E�B���h�E�ꗗ���擾����
		EditNode* pEditNode;
		int nCount = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNode, true);
		bool bSelfFound = false;
		if (0 >= nCount) return;

		for (int i=0; i<nCount; ++i) {
			if (pEditNode[i].hWnd == GetMainWindow()) {
				pEditNode[i].hWnd = NULL;		// �������g�͕��Ȃ�
				nGroup = pEditNode[i].nGroup;
				bSelfFound = true;
			}else if (!bSelfFound) {
				pEditNode[i].hWnd = NULL;		// ���͕��Ȃ�
			}
		}

		// �I���v�����o��
		AppNodeGroupHandle(nGroup).RequestCloseEditor(pEditNode, nCount, false, true, GetMainWindow());
		delete []pEditNode;
	}
	return;
}


// �c�����ɍő剻
void ViewCommander::Command_MAXIMIZE_V(void)
{
	RECT rcOrg;
	RECT rcDesktop;
	HWND hwndFrame = GetMainWindow();
	::GetWindowRect(hwndFrame, &rcOrg);
	// May 01, 2004 genta �}���`���j�^�Ή�
	::GetMonitorWorkRect(hwndFrame, &rcDesktop);
	::SetWindowPos(
		hwndFrame, 0,
		rcOrg.left, rcDesktop.top,
		rcOrg.right - rcOrg.left, rcDesktop.bottom - rcDesktop.top,
		SWP_NOOWNERZORDER | SWP_NOZORDER
	);
	return;
}


// 2001.02.10 Start by MIK: �������ɍő剻
// �������ɍő剻
void ViewCommander::Command_MAXIMIZE_H(void)
{
	RECT rcOrg;
	RECT rcDesktop;

	HWND hwndFrame = GetMainWindow();
	::GetWindowRect(hwndFrame, &rcOrg);
	// May 01, 2004 genta �}���`���j�^�Ή�
	::GetMonitorWorkRect(hwndFrame, &rcDesktop);
	::SetWindowPos(
		hwndFrame, 0,
		rcDesktop.left, rcOrg.top,
		rcDesktop.right - rcDesktop.left, rcOrg.bottom - rcOrg.top,
		SWP_NOOWNERZORDER | SWP_NOZORDER
	);
	return;
}
// 2001.02.10 End: �������ɍő剻


// ���ׂčŏ���		// Sept. 17, 2000 jepro �����́u�S�āv���u���ׂāv�ɓ���
void ViewCommander::Command_MINIMIZE_ALL(void)
{
	int j = GetDllShareData().nodes.nEditArrNum;
	if (j == 0) {
		return;
	}
	std::vector<HWND> wnds(j);
	HWND* phWndArr = &wnds[0];
	for (int i=0; i<j; ++i) {
		phWndArr[i] = GetDllShareData().nodes.pEditArr[i].GetHwnd();
	}
	for (int i=0; i<j; ++i) {
		if (IsSakuraMainWindow(phWndArr[i])) {
			if (::IsWindowVisible(phWndArr[i]))
				::ShowWindow(phWndArr[i], SW_MINIMIZE);
		}
	}
}

// �ĕ`��
void ViewCommander::Command_REDRAW(void)
{
	// �t�H�[�J�X�ړ����̍ĕ`��
	m_view.RedrawAll();
	return;
}


// �A�E�g�v�b�g�E�B���h�E�\��
void ViewCommander::Command_WIN_OUTPUT(void)
{
	// 2010.05.11 Moca ShareData::OpenDebugWindow()�ɓ���
	// ���b�Z�[�W�\���E�B���h�E��View����e�ɕύX
	// TraceOut�o�R�ł�CODE_UNICODE,������ł�CODE_SJIS�������̂𖳎w��ɕύX
	ShareData::getInstance().OpenDebugWindow(GetMainWindow(), true);
	return;
}


// from ViewCommander_New.cpp
/*!	@brief �}�N���p�A�E�g�v�b�g�E�B���h�E�ɕ\��
	@date 2006.04.26 maru �V�K�쐬
*/
void ViewCommander::Command_TRACEOUT(const wchar_t* outputstr, int nLen, int nFlgOpt)
{
	if (!outputstr)
		return;

	// 0x01 ExpandParameter�ɂ�镶����W�J�L��
	if (nFlgOpt & 0x01) {
		wchar_t Buffer[2048];
		SakuraEnvironment::ExpandParameter(outputstr, Buffer, 2047);
		ShareData::getInstance().TraceOutString(Buffer);
	}else {
		ShareData::getInstance().TraceOutString(outputstr, nLen);
	}

	// 0x02 ���s�R�[�h�̗L��
	if ((nFlgOpt & 0x02) == 0) {
		ShareData::getInstance().TraceOutString(L"\r\n");
	}

}

