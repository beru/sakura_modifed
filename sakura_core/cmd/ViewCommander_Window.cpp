#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "_main/ControlTray.h"
#include "util/os.h"
#include "env/SakuraEnvironment.h"
#include "env/ShareData.h"

// ViewCommander�N���X�̃R�}���h(�E�B���h�E�n)�֐��Q

// �㉺�ɕ���
void ViewCommander::Command_Split_V(void)
{
	GetEditWindow().splitterWnd.VSplitOnOff();
	return;
}


// ���E�ɕ���
void ViewCommander::Command_Split_H(void)
{
	GetEditWindow().splitterWnd.HSplitOnOff();
	return;
}


// �c���ɕ���
void ViewCommander::Command_Split_VH(void)
{
	GetEditWindow().splitterWnd.VHSplitOnOff();
	return;
}


// �E�B���h�E�����
void ViewCommander::Command_WinClose(void)
{
	// ����
	::PostMessage(GetMainWindow(), MYWM_CLOSE, FALSE,
		(LPARAM)AppNodeManager::getInstance().GetNextTab(GetMainWindow()));	// �^�u�܂Ƃߎ��A���̃^�u�Ɉړ�
	return;
}


// ���ׂẴE�B���h�E�����
void ViewCommander::Command_FileCloseAll(void)
{
	int nGroup = AppNodeManager::getInstance().GetEditNode(GetMainWindow())->GetGroup();
	ControlTray::CloseAllEditor(true, GetMainWindow(), false, nGroup);
	return;
}


// ���̃^�u�ȊO�����
void ViewCommander::Command_Tab_CloseOther(void)
{
	int nGroup = 0;

	// �E�B���h�E�ꗗ���擾����
	EditNode* pEditNode;
	size_t nCount = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNode, true);
	if (nCount == 0) {
		return;
	}

	for (size_t i=0; i<nCount; ++i) {
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


/*!	@brief �E�B���h�E�ꗗ�|�b�v�A�b�v�\�������i�t�@�C�����̂݁j*/
void ViewCommander::Command_WinList(int nCommandFrom)
{
	// �E�B���h�E�ꗗ���|�b�v�A�b�v�\������
	GetEditWindow().PopupWinList((nCommandFrom & FA_FROMKEYBOARD) != FA_FROMKEYBOARD);
	// �A�N�Z�����[�^�L�[����łȂ���΃}�E�X�ʒu��
}


/*!	@brief �d�˂ĕ\�� */
void ViewCommander::Command_Cascade(void)
{
	// ���݊J���Ă���ҏW���̃��X�g���擾����
	EditNode* pEditNodeArr;
	size_t nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true/*false*/, true);
	if (nRowNum == 0) {
		return;
	}

	struct WNDARR {
		HWND	hWnd;
		int		newX;
		int		newY;
	};
	std::vector<WNDARR> wndArr(nRowNum);
	WNDARR*	pWndArr = &wndArr[0];
	size_t count = 0;	// �����ΏۃE�B���h�E�J�E���g
	// ���݂̃E�B���h�E�𖖔��Ɏ����Ă����̂Ɏg��
	int	current_win_index = -1;

	// -----------------------------------------
	// �E�B���h�E(�n���h��)���X�g�̍쐬
	// -----------------------------------------
	for (size_t i=0; i<nRowNum; ++i) {
		auto editNodeHWnd = pEditNodeArr[i].GetHwnd();
		if (::IsIconic(editNodeHWnd)) {	// �ŏ������Ă���E�B���h�E�͖����B
			continue;
		}
		if (!::IsWindowVisible(editNodeHWnd)) {	// �s���E�B���h�E�͖����B
			continue;
		}
		// ���݂̃E�B���h�E�𖖔��Ɏ����Ă������߂����ł̓X�L�b�v
		if (editNodeHWnd == EditWnd::getInstance().GetHwnd()) {
			current_win_index = (int)i;
			continue;
		}
		pWndArr[count].hWnd = editNodeHWnd;
		++count;
	}

	// ���݂̃E�B���h�E�𖖔��ɑ}��
	if (current_win_index >= 0) {
		pWndArr[count].hWnd = pEditNodeArr[current_win_index].GetHwnd();
		++count;
	}

	// �f�X�N�g�b�v�T�C�Y�𓾂�
	RECT rcDesktop;
	// �}���`���j�^�Ή�
	::GetMonitorWorkRect(view.GetHwnd(), &rcDesktop);
	
	int width = (rcDesktop.right - rcDesktop.left) * 4 / 5;
	int height = (rcDesktop.bottom - rcDesktop.top) * 4 / 5;
	int w_delta = ::GetSystemMetrics(SM_CXSIZEFRAME) + ::GetSystemMetrics(SM_CXSIZE);
	int h_delta = ::GetSystemMetrics(SM_CYSIZEFRAME) + ::GetSystemMetrics(SM_CYSIZE);
	int w_offset = rcDesktop.left; // ��Βl���ƃG�N�X�v���[���[�̃E�B���h�E�ɏd�Ȃ�̂�
	int h_offset = rcDesktop.top; // �����l���f�X�N�g�b�v���Ɏ��߂�B

	// -----------------------------------------
	// ���W�v�Z
	//		������f�X�N�g�b�v�̈�ɍ��킹��(�^�X�N�o�[����E���ɂ���ꍇ�̂���)�D
	//		�E�B���h�E���E������͂ݏo���獶��ɖ߂邪�C
	//		2���ڈȍ~�͊J�n�ʒu���E�ɂ��炵�ăA�C�R����������悤�ɂ���D
	//
	// �����ł͌v�Z�l��ۊǂ��邾���ŃE�B���h�E�̍Ĕz�u�͍s��Ȃ�
	// -----------------------------------------

	int roundtrip = 0; // �Q�x�ڂ̕`��ȍ~�Ŏg�p����J�E���g
	int sw_offset = w_delta; // �E�X���C�h�̕�

	for (size_t i=0; i<count; ++i) {
		if (w_offset + width > rcDesktop.right || h_offset + height > rcDesktop.bottom) {
			++roundtrip;
			if ((rcDesktop.right - rcDesktop.left) - sw_offset * roundtrip < width) {
				// ����ȏ�E�ɂ��点�Ȃ��Ƃ��͂��傤���Ȃ����獶��ɖ߂�
				roundtrip = 0;
			}
			// �E�B���h�E�̈�̍���ɃZ�b�g
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
	// -----------------------------------------
	for (size_t i=0; i<count; ++i) {
		::ShowWindow(pWndArr[i].hWnd, SW_RESTORE | SW_SHOWNA);
	}

	// -----------------------------------------
	// �E�B���h�E�z�u
	//
	// API��f���Ɏg����Z-Order�̏ォ�牺�̏��ŕ��ׂ�D
	// -----------------------------------------

	// �܂��J�����g���őO�ʂ�
	size_t i = count - 1;
	
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

	delete[] pEditNodeArr;
}


// �㉺�ɕ��ׂĕ\��
void ViewCommander::Command_Tile_V(void)
{
	// ���݊J���Ă���ҏW���̃��X�g���擾����
	EditNode* pEditNodeArr;
	size_t nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true/*false*/, true);

	if (nRowNum == 0) {
		return;
	}
	std::vector<HWND> hWnds(nRowNum);
	HWND* phwndArr = &hWnds[0];
	size_t count = 0;
	// �f�X�N�g�b�v�T�C�Y�𓾂�
	RECT rcDesktop;
	// �}���`���j�^�Ή�
	::GetMonitorWorkRect(view.GetHwnd(), &rcDesktop);
	for (size_t i=0; i<nRowNum; ++i) {
		auto editNodeHWnd = pEditNodeArr[i].GetHwnd();
		if (::IsIconic(editNodeHWnd)) {	// �ŏ������Ă���E�B���h�E�͖����B
			continue;
		}
		if (!::IsWindowVisible(editNodeHWnd)) {	// �s���E�B���h�E�͖����B
			continue;
		}
		// ���݂̃E�B���h�E��擪�Ɏ����Ă���
		if (editNodeHWnd == EditWnd::getInstance().GetHwnd()) {
			phwndArr[count] = phwndArr[0];
			phwndArr[0] = editNodeHWnd;
		}else {
			phwndArr[count] = editNodeHWnd;
		}
		++count;
	}
	size_t height = (rcDesktop.bottom - rcDesktop.top) / count;
	for (size_t i=0; i<count; ++i) {
		::ShowWindow(phwndArr[i], SW_RESTORE);
		::SetWindowPos(
			phwndArr[i], 0,
			rcDesktop.left, rcDesktop.top + (int)(height * i), // ��[����
			rcDesktop.right - rcDesktop.left, (int)height,
			SWP_NOOWNERZORDER | SWP_NOZORDER
		);
	}
	::SetFocus(phwndArr[0]);

	delete[] pEditNodeArr;
}


// ���E�ɕ��ׂĕ\��
void ViewCommander::Command_Tile_H(void)
{
	// ���݊J���Ă���ҏW���̃��X�g���擾����
	EditNode* pEditNodeArr;
	size_t nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true/*false*/, true);
	if (nRowNum == 0) {
		return;
	}
	std::vector<HWND> hWnds(nRowNum);
	HWND* phwndArr = &hWnds[0];
	size_t count = 0;
	// �f�X�N�g�b�v�T�C�Y�𓾂�
	RECT rcDesktop;
	// �}���`���j�^�Ή�
	::GetMonitorWorkRect(view.GetHwnd(), &rcDesktop);
	for (size_t i=0; i<nRowNum; ++i) {
		auto editNodeHWnd = pEditNodeArr[i].GetHwnd();
		if (::IsIconic(editNodeHWnd)) {	// �ŏ������Ă���E�B���h�E�͖����B
			continue;
		}
		if (!::IsWindowVisible(editNodeHWnd)) {	// �s���E�B���h�E�͖����B
			continue;
		}
		// ���݂̃E�B���h�E��擪�Ɏ����Ă���
		if (editNodeHWnd == EditWnd::getInstance().GetHwnd()) {
			phwndArr[count] = phwndArr[0];
			phwndArr[0] = editNodeHWnd;
		}else {
			phwndArr[count] = editNodeHWnd;
		}
		++count;
	}
	size_t width = (rcDesktop.right - rcDesktop.left) / count;
	for (size_t i=0; i<count; ++i) {
		::ShowWindow(phwndArr[i], SW_RESTORE);
		::SetWindowPos(
			phwndArr[i], 0,
			(int)(width * i) + rcDesktop.left, rcDesktop.top, // �^�X�N�o�[�����ɂ���ꍇ���l��
			(int)width, rcDesktop.bottom - rcDesktop.top,
			SWP_NOOWNERZORDER | SWP_NOZORDER
		);
	}
	::SetFocus(phwndArr[0]);
	delete[] pEditNodeArr;
}


/*! ��Ɏ�O�ɕ\�� */
void ViewCommander::Command_WinTopMost(LPARAM lparam)
{
	GetEditWindow().WindowTopMost(int(lparam));
}

/*!	@brief �������ĕ\��

	�^�u�E�B���h�E�̌����A�񌋍���؂�ւ���R�}���h�ł��B
	[���ʐݒ�]->[�E�B���h�E]->[�^�u�\�� �܂Ƃ߂Ȃ�]�̐؂�ւ��Ɠ����ł��B
*/
void ViewCommander::Command_Bind_Window(void)
{
	// �^�u���[�h�ł���Ȃ��
	auto& csTabBar = GetDllShareData().common.tabBar;
	if (!csTabBar.bDispTabWnd) {
		return;
	}
	// �^�u�E�B���h�E�̐ݒ��ύX
	csTabBar.bDispTabWndMultiWin = !csTabBar.bDispTabWndMultiWin;

	// �܂Ƃ߂�Ƃ��� WS_EX_TOPMOST ��Ԃ𓯊�����
	if (!csTabBar.bDispTabWndMultiWin) {
		GetEditWindow().WindowTopMost(
			((DWORD)::GetWindowLongPtr(GetEditWindow().GetHwnd(), GWL_EXSTYLE) & WS_EX_TOPMOST)? 1: 2
		);
	}

	// �^�u�E�B���h�E�̐ݒ��ύX���u���[�h�L���X�g����
	AppNodeManager::getInstance().ResetGroupId();
	AppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_TAB_WINDOW_NOTIFY,						// �^�u�E�B���h�E�C�x���g
		(WPARAM)((csTabBar.bDispTabWndMultiWin) ? TabWndNotifyType::Disable : TabWndNotifyType::Enable), // �^�u���[�h�L��/�������C�x���g
		(LPARAM)GetEditWindow().GetHwnd(),	// EditWnd�̃E�B���h�E�n���h��
		view.GetHwnd());									// �������g
}

// �O���[�v�����
void ViewCommander::Command_GroupClose(void)
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

// ���̃O���[�v
void ViewCommander::Command_NextGroup(void)
{
	auto& tabWnd = GetEditWindow().tabWnd;
	if (!tabWnd.GetHwnd()) {
		return;
	}
	tabWnd.NextGroup();
}

// �O�̃O���[�v
void ViewCommander::Command_PrevGroup(void)
{
	auto& tabWnd = GetEditWindow().tabWnd;
	if (!tabWnd.GetHwnd()) {
		return;
	}
	tabWnd.PrevGroup();
}

// �^�u���E�Ɉړ�
void ViewCommander::Command_Tab_MoveRight(void)
{
	auto& tabWnd = GetEditWindow().tabWnd;
	if (!tabWnd.GetHwnd()) {
		return;
	}
	tabWnd.MoveRight();
}

// �^�u�����Ɉړ�
void ViewCommander::Command_Tab_MoveLeft(void)
{
	auto& tabWnd = GetEditWindow().tabWnd;
	if (!tabWnd.GetHwnd()) {
		return;
	}
	tabWnd.MoveLeft();
}

// �V�K�O���[�v
void ViewCommander::Command_Tab_Separate(void)
{
	auto& tabWnd = GetEditWindow().tabWnd;
	if (!tabWnd.GetHwnd()) {
		return;
	}
	tabWnd.Separate();
}

// ���̃O���[�v�Ɉړ�
void ViewCommander::Command_Tab_JointNext(void)
{
	auto& tabWnd = GetEditWindow().tabWnd;
	if (!tabWnd.GetHwnd()) {
		return;
	}
	tabWnd.JoinNext();
}

// �O�̃O���[�v�Ɉړ�
void ViewCommander::Command_Tab_JointPrev(void)
{
	auto& tabWnd = GetEditWindow().tabWnd;
	if (!tabWnd.GetHwnd()) {
		return;
	}
	tabWnd.JoinPrev();
}


// �������ׂĕ���
void ViewCommander::Command_Tab_CloseLeft(void)
{
	if (!GetDllShareData().common.tabBar.bDispTabWnd) {
		return;
	}
	int nGroup = 0;

	// �E�B���h�E�ꗗ���擾����
	EditNode* pEditNode;
	size_t nCount = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNode, true);
	bool bSelfFound = false;
	if (0 >= nCount) return;

	for (size_t i=0; i<nCount; ++i) {
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
	delete[] pEditNode;
}


// �E�����ׂĕ���
void ViewCommander::Command_Tab_CloseRight(void)
{
	if (!GetDllShareData().common.tabBar.bDispTabWnd) {
		return;
	}
	int nGroup = 0;

	// �E�B���h�E�ꗗ���擾����
	EditNode* pEditNode;
	size_t nCount = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNode, true);
	bool bSelfFound = false;
	if (0 >= nCount) return;

	for (size_t i=0; i<nCount; ++i) {
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
	delete[] pEditNode;
}


// �c�����ɍő剻
void ViewCommander::Command_Maximize_V(void)
{
	RECT rcOrg;
	RECT rcDesktop;
	HWND hwndFrame = GetMainWindow();
	::GetWindowRect(hwndFrame, &rcOrg);
	// �}���`���j�^�Ή�
	::GetMonitorWorkRect(hwndFrame, &rcDesktop);
	::SetWindowPos(
		hwndFrame, 0,
		rcOrg.left, rcDesktop.top,
		rcOrg.right - rcOrg.left, rcDesktop.bottom - rcDesktop.top,
		SWP_NOOWNERZORDER | SWP_NOZORDER
	);
	return;
}


// �������ɍő剻
void ViewCommander::Command_Maximize_H(void)
{
	RECT rcOrg;
	RECT rcDesktop;

	HWND hwndFrame = GetMainWindow();
	::GetWindowRect(hwndFrame, &rcOrg);
	// �}���`���j�^�Ή�
	::GetMonitorWorkRect(hwndFrame, &rcDesktop);
	::SetWindowPos(
		hwndFrame, 0,
		rcDesktop.left, rcOrg.top,
		rcDesktop.right - rcDesktop.left, rcOrg.bottom - rcOrg.top,
		SWP_NOOWNERZORDER | SWP_NOZORDER
	);
	return;
}

// ���ׂčŏ���
void ViewCommander::Command_Minimize_All(void)
{
	size_t j = GetDllShareData().nodes.nEditArrNum;
	if (j == 0) {
		return;
	}
	std::vector<HWND> wnds(j);
	HWND* phWndArr = &wnds[0];
	for (size_t i=0; i<j; ++i) {
		phWndArr[i] = GetDllShareData().nodes.pEditArr[i].GetHwnd();
	}
	for (size_t i=0; i<j; ++i) {
		if (IsSakuraMainWindow(phWndArr[i])) {
			if (::IsWindowVisible(phWndArr[i]))
				::ShowWindow(phWndArr[i], SW_MINIMIZE);
		}
	}
}

// �ĕ`��
void ViewCommander::Command_Redraw(void)
{
	// �t�H�[�J�X�ړ����̍ĕ`��
	view.RedrawAll();
	return;
}


// �A�E�g�v�b�g�E�B���h�E�\��
void ViewCommander::Command_Win_Output(void)
{
	// ���b�Z�[�W�\���E�B���h�E��View����e�ɕύX
	// TraceOut�o�R�ł�CODE_UNICODE,������ł�CODE_SJIS�������̂𖳎w��ɕύX
	ShareData::getInstance().OpenDebugWindow(GetMainWindow(), true);
	return;
}


/*!	@brief �}�N���p�A�E�g�v�b�g�E�B���h�E�ɕ\�� */
void ViewCommander::Command_TraceOut(const wchar_t* outputstr, int nLen, int nFlgOpt)
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

