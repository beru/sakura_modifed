/*!	@file
	@brief �^�u�E�B���h�E

	@author MIK
	@date 2003.5.30
	@date 2004.01.27 break�R��Ή��BTCHAR���B�^�u�\���������(?)�̑Ή��B
*/
/*
	Copyright (C) 2003, MIK, KEITA
	Copyright (C) 2004, Moca, MIK, genta, Kazika
	Copyright (C) 2005, ryoji
	Copyright (C) 2006, ryoji, fon
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2012, Moca, syat, novice, uchi
	Copyright (C) 2013, Moca, Uchi, aroka, novice, syat, ryoji

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include <limits.h>
#include "TabWnd.h"
#include "window/EditWnd.h"
#include "_main/global.h"
#include "_os/OSVersionInfo.h"
#include "charset/charcode.h"
#include "extmodule/UxTheme.h"
#include "env/ShareData.h"
#include "env/SakuraEnvironment.h"
#include "uiparts/Graphics.h"
#include "recent/RecentEditNode.h"
#include "util/os.h" // WM_THEMECHANGED
#include "util/window.h"
#include "util/module.h"
#include "util/string_ex2.h"
#include "sakura_rc.h"


//#if (WINVER >= 0x0500)
#ifndef	SPI_GETFOREGROUNDLOCKTIMEOUT
#define SPI_GETFOREGROUNDLOCKTIMEOUT        0x2000
#endif
#ifndef	SPI_SETFOREGROUNDLOCKTIMEOUT
#define SPI_SETFOREGROUNDLOCKTIMEOUT        0x2001
#endif
//#endif

// 2006.01.30 ryoji �^�u�̃T�C�Y�^�ʒu�Ɋւ����`
// 2009.10.01 ryoji ��DPI�Ή��X�P�[�����O
#define TAB_MARGIN_TOP		DpiScaleY(3)
#define TAB_MARGIN_LEFT		DpiScaleX(1)
#define TAB_MARGIN_RIGHT	DpiScaleX(47)

//#define TAB_FONT_HEIGHT		DpiPointsToPixels(9)
#define TAB_FONT_HEIGHT		abs(GetDllShareData().common.tabBar.lf.lfHeight)
#define TAB_ITEM_HEIGHT		(TAB_FONT_HEIGHT + DpiScaleY(7))
#define TAB_WINDOW_HEIGHT	(TAB_ITEM_HEIGHT + TAB_MARGIN_TOP + 2)

#define MAX_TABITEM_WIDTH	DpiScaleX(200)
#define MIN_TABITEM_WIDTH	DpiScaleX(60)
#define MIN_TABITEM_WIDTH_MULTI	DpiScaleX(GetDllShareData().common.tabBar.nTabMinWidthOnMulti)

#define CX_SMICON			DpiScaleX(16)
#define CY_SMICON			DpiScaleY(16)

static const RECT rcBtnBase = { 0, 0, 16, 16 };

// 2006.02.01 ryoji �^�u�ꗗ���j���[�p�f�[�^
typedef struct {
	HWND	hwnd;
	int		iItem;
	int		iImage;
	TCHAR	szText[_MAX_PATH];
} TABMENU_DATA;

/*!	�^�u�ꗗ���j���[�p�f�[�^�� qsort() �R�[���o�b�N����
	@date 2006.02.01 ryoji �V�K�쐬
*/
static
int compTABMENU_DATA(const void* arg1, const void* arg2)
{
	int ret;

	// �����͕�����\�[�g�itcscmp�j�ł͂Ȃ��P��\�[�g�ilstrcmp�j���g�p����
	// ������\�[�g: "XYZ" �� "ABC" �� "abc" �Ƃ̊ԂɊ����ē���
	// �P��\�[�g: "ABC" �� "abc" �Ƃ͗אڂ� "XYZ" �͂����̌��ɓ���i���ۂ̎����Ɠ��l�ȏ����j
	ret = ::lstrcmp(((TABMENU_DATA*)arg1)->szText, ((TABMENU_DATA*)arg2)->szText);
	if (ret == 0)
		ret = ((TABMENU_DATA*)arg1)->iItem - ((TABMENU_DATA*)arg2)->iItem;
	return ret;
}


WNDPROC	g_pOldWndProc = NULL;

// �{���� TabWnd �E�B���h�E�v���V�[�W���Ăяo��
inline
LRESULT CALLBACK DefTabWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (g_pOldWndProc)
		return ::CallWindowProc(g_pOldWndProc, hwnd, uMsg, wParam, lParam);
	else
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// TabWnd�E�B���h�E���b�Z�[�W�̃R�[���o�b�N�֐�
LRESULT CALLBACK TabWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Modified by KEITA for WIN64 2003.9.6
	TabWnd* pTabWnd = (TabWnd*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (pTabWnd) {
		//return
		if (0L == pTabWnd->TabWndDispatchEvent(hwnd, uMsg, wParam, lParam)) {
			return 0L;
		}
	}

	return DefTabWndProc(hwnd, uMsg, wParam, lParam);
}

// ���b�Z�[�W�z��
LRESULT TabWnd::TabWndDispatchEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// 2005.09.01 ryoji �^�u���̃��b�Z�[�W�������ʂɊ֐������A�^�u�����ύX�̏�����ǉ�
	switch (uMsg) {
	case WM_LBUTTONDOWN:
		return OnTabLButtonDown(wParam, lParam);

	case WM_LBUTTONUP:
		return OnTabLButtonUp(wParam, lParam);

	case WM_MOUSEMOVE:
		return OnTabMouseMove(wParam, lParam);

	case WM_TIMER:
		return OnTabTimer(wParam, lParam);

	case WM_CAPTURECHANGED:
		return OnTabCaptureChanged(wParam, lParam);

	case WM_RBUTTONDOWN:
		return OnTabRButtonDown(wParam, lParam);

	case WM_RBUTTONUP:
		return OnTabRButtonUp(wParam, lParam);

	case WM_MBUTTONDOWN:
		return OnTabMButtonDown(wParam, lParam);

	case WM_MBUTTONUP:
		return OnTabMButtonUp(wParam, lParam);

	case WM_NOTIFY:
		return OnTabNotify(wParam, lParam);

	case WM_HSCROLL:
		::InvalidateRect(GetHwnd(), NULL, TRUE);	// �A�N�e�B�u�^�u�̈ʒu���ς��̂Ńg�b�v�o���h���X�V����	// 2006.03.27 ryoji
		break;

	case WM_THEMECHANGED:
		bVisualStyle = ::IsVisualStyle();
		break;

	// default:
	}
	return 1L;	// �f�t�H���g�̃f�B�X�p�b�`�ɂ܂킷
}

// �^�u�� WM_LBUTTONDOWN ����
LRESULT TabWnd::OnTabLButtonDown(WPARAM wParam, LPARAM lParam)
{
	// �{�^���������ꂽ�ʒu���m�F����
	TCHITTESTINFO hitinfo;
	hitinfo.pt.x = LOWORD((DWORD)lParam);
	hitinfo.pt.y = HIWORD((DWORD)lParam);
	int nSrcTab = TabCtrl_HitTest(hwndTab, (LPARAM)&hitinfo);
	if (0 > nSrcTab)
		return 1L;

	// �^�u�̕���{�^����������
	if (pShareData->common.tabBar.dispTabClose != DispTabCloseType::No) {
		// ����{�^���̃`�F�b�N
		RECT rcItem;
		RECT rcClose;
		TabCtrl_GetItemRect(hwndTab, nSrcTab, &rcItem);
		GetTabCloseBtnRect(&rcItem, &rcClose, nSrcTab == TabCtrl_GetCurSel(hwndTab));
		if (::PtInRect(&rcClose, hitinfo.pt)) {
			// ����{�^����Ȃ�L���v�`���[�J�n
			nTabCloseCapture = nSrcTab;
			::SetCapture(hwndTab);
			return 0L;
		}
	}

	// �}�E�X�h���b�O�J�n����
	eDragState = DRAG_CHECK;	// �h���b�O�̃`�F�b�N���J�n

	// �h���b�O���^�u���L������
	this->nSrcTab = nSrcTab;
	::GetCursorPos(&ptSrcCursor);

	::SetCapture(hwndTab);

	return 0L;
}

// �^�u�� WM_LBUTTONUP ����
LRESULT TabWnd::OnTabLButtonUp(WPARAM wParam, LPARAM lParam)
{
	TCHITTESTINFO hitinfo;
	hitinfo.pt.x = LOWORD((DWORD)lParam);
	hitinfo.pt.y = HIWORD((DWORD)lParam);
	int nDstTab = TabCtrl_HitTest(hwndTab, (LPARAM)&hitinfo);
	int nSelfTab = FindTabIndexByHWND(GetParentHwnd());

	// �^�u�̕���{�^����������
	if (nTabCloseCapture >= 0) {	// �^�u���̕���{�^���������������Ă���?
		// ���̕���{�^���Ɠ���̕���{�^����Ȃ�^�u�����
		RECT rcItem;
		RECT rcClose;
		TabCtrl_GetItemRect(hwndTab, nTabCloseCapture, &rcItem);
		GetTabCloseBtnRect(&rcItem, &rcClose, nTabCloseCapture == TabCtrl_GetCurSel(hwndTab));
		if (::PtInRect(&rcClose, hitinfo.pt)) {
			ExecTabCommand(F_WINCLOSE, MAKEPOINTS(lParam));
		}
		// �L���v�`���[����
		BreakDrag();
		return 0L;
	}

	// �}�E�X�h���b�v����
	switch (eDragState) {
	case DRAG_CHECK:
		if (nSrcTab == nDstTab && nSrcTab != nSelfTab) {
			// �w��̃E�B���h�E���A�N�e�B�u��
			TCITEM	tcitem;
			tcitem.mask   = TCIF_PARAM;
			tcitem.lParam = 0;
			TabCtrl_GetItem(hwndTab, nDstTab, &tcitem);

			ShowHideWindow((HWND)tcitem.lParam, TRUE);
		}
		break;

	case DRAG_DRAG:
		if (0 > nDstTab) {	// �^�u�̊O�Ńh���b�v
			// �^�u�̕�������
			if (pShareData->common.tabBar.bDispTabWnd && !pShareData->common.tabBar.bDispTabWndMultiWin) {
				HWND hwndAncestor;
				POINT ptCursor;

				::GetCursorPos(&ptCursor);
				hwndAncestor = MyGetAncestor(::WindowFromPoint(ptCursor), GA_ROOT);
				if (hwndAncestor != GetParentHwnd()) {	// ����ʂ̊O�Ńh���b�v
					// �^�u�ړ�
					TCITEM	tcitem;
					tcitem.mask   = TCIF_PARAM;
					tcitem.lParam = 0;
					TabCtrl_GetItem(hwndTab, nSrcTab, &tcitem);
					HWND hwndSrc = (HWND)tcitem.lParam;
					HWND hwndDst = IsSakuraMainWindow(hwndAncestor)? hwndAncestor: NULL;

					SeparateGroup(hwndSrc, hwndDst, ptSrcCursor, ptCursor);
				}
			}
		}
		if (bTabSwapped) {
			// �^�u�͈ړ��ς݁B�ق��̃E�B���h�E�̂ݍX�V
			BroadcastRefreshToGroup();
		}
		if (nTabBorderArray) {
			delete[] nTabBorderArray;
			nTabBorderArray = nullptr;
		}
		Tooltip_Activate(TabCtrl_GetToolTips(hwndTab), TRUE);	// �c�[���`�b�v�L����
		break;

	default:
		break;
	}

	BreakDrag();	// 2006.01.28 ryoji �h���b�O��Ԃ���������(�֐���)

	return 0L;
}

// �^�u�� WM_MOUSEMOVE ����
LRESULT TabWnd::OnTabMouseMove(WPARAM wParam, LPARAM lParam)
{
	TCHITTESTINFO hitinfo;
	int nTabCount;
	hitinfo.pt.x = LOWORD((DWORD)lParam);
	hitinfo.pt.y = HIWORD((DWORD)lParam);
	int nDstTab = TabCtrl_HitTest(hwndTab, (LPARAM)&hitinfo);

	// �e�^�u�̕���{�^���`��p����
	DispTabCloseType dispTabClose = pShareData->common.tabBar.dispTabClose;
	if (dispTabClose != DispTabCloseType::No && ::GetCapture() != hwndTab) {
		int nTabHoverPrev = nTabHover;
		int nTabHoverCur = nDstTab;
		RECT rcPrev;
		RECT rcCur;
		RECT rcCurClose;
		TabCtrl_GetItemRect(hwndTab, nTabHoverPrev, &rcPrev);
		TabCtrl_GetItemRect(hwndTab, nTabHoverCur, &rcCur);
		GetTabCloseBtnRect(&rcCur, &rcCurClose, nTabHoverCur == TabCtrl_GetCurSel(hwndTab));

		nTabHover = nTabHoverCur;
		if (nTabHoverCur >= 0) {	// �J�[�\�����ǂꂩ�^�u���ɂ���
			if (nTabHoverPrev < 0) {	// �^�u�O���������
				::SetTimer(hwndTab, 1, 200, NULL);	// �^�C�}�[�N��
			}

			// ����{�^���̎����\��
			if (dispTabClose == DispTabCloseType::Auto) {
				if (nTabHoverCur != nTabHoverPrev) {	// �^�u�O�܂��͕ʂ̃^�u���������
					if (nTabHoverPrev >= 0) {	// �ʂ̃^�u���������
						// �O��̃^�u���ĕ`�悷��
						::InvalidateRect(hwndTab, &rcPrev, TRUE);
					}
					// ���̃^�u���ĕ`�悷��
					::InvalidateRect(hwndTab, &rcCur, TRUE);
				}
			}

			// ����{�^����̃z�o�[��Ԃ�ς���
			if (::PtInRect(&rcCurClose, hitinfo.pt)) {	// ����{�^����
				if (!bTabCloseHover || nTabHoverCur != nTabHoverPrev) {	// �ȑO�̓}�E�X���̕���{�^�����n�C���C�g���Ă��Ȃ�����
					bTabCloseHover = true;
					if (nTabHoverCur != nTabHoverPrev) {
						// �O��̃^�u���ĕ`�悷��
						::InvalidateRect(hwndTab, &rcPrev, TRUE);
					}
					// ���̃^�u���ĕ`�悷��
					::InvalidateRect(hwndTab, &rcCur, TRUE);
				}
			}else {
				if (bTabCloseHover) {	// ����{�^������o��
					// �O��A����{�^�����n�C���C�g���Ă����^�u���ĕ`�悷��
					bTabCloseHover = false;
					::InvalidateRect(hwndTab, &rcPrev, TRUE);
				}
			}
		}else {	// �J�[�\�����^�u�O�ɏo��
			::KillTimer(hwndTab, 1);	// �^�C�}�[�폜
			if (dispTabClose == DispTabCloseType::Auto || bTabCloseHover) {
				if (nTabHoverPrev >= 0) {
					// �O��̃^�u���ĕ`�悷��
					::InvalidateRect(hwndTab, &rcPrev, TRUE);
				}
			}
			bTabCloseHover = false;
		}
	}

	// �}�E�X�h���b�O���̏���
	switch (eDragState) {
	case DRAG_CHECK:
		// ���̃^�u���痣�ꂽ��h���b�O�J�n
		if (nSrcTab == nDstTab) {
			break;
		}
		eDragState = DRAG_DRAG;
		hDefaultCursor = ::GetCursor();
		bTabSwapped = false;

		// ���݂̃^�u���E�ʒu���L������
		nTabCount = TabCtrl_GetItemCount(hwndTab);
		if (nTabBorderArray) {
			delete[] nTabBorderArray;
		}
		nTabBorderArray = new LONG[nTabCount];
		int i;
		for (i=0; i<nTabCount-1; ++i) {
			RECT rc;
			TabCtrl_GetItemRect(hwndTab, i, &rc);
			nTabBorderArray[i] = rc.right;
		}
		nTabBorderArray[i] = 0;		// �Ō�̗v�f�͔ԕ�
		Tooltip_Activate(TabCtrl_GetToolTips(hwndTab), FALSE);	// �c�[���`�b�v������
		// �����ɗ�����h���b�O�J�n�Ȃ̂� break ���Ȃ��ł��̂܂� DRAG_DRAG �����ɓ���

	case DRAG_DRAG:
		// �h���b�O���̃}�E�X�J�[�\����\������
		HINSTANCE hInstance;
		LPCTSTR lpCursorName;
		lpCursorName = IDC_NO;	// �֎~�J�[�\��
		if (0 <= nDstTab) {	// �^�u�̏�ɃJ�[�\��������
			lpCursorName = NULL;	// �J�n���J�[�\���w��

			// �h���b�O�J�n���̃^�u�ʒu�ňړ���^�u���Čv�Z
			for (nDstTab=0; nTabBorderArray[nDstTab]!=0; ++nDstTab) {
				if (hitinfo.pt.x < nTabBorderArray[nDstTab]) {
					break;
				}
			}

			// �h���b�O���ɑ����ړ�
			if (nSrcTab != nDstTab) {
				// �������F�ړ���^�u�̍��[�������W�Ȃ�ړ����Ȃ�
				// �� �^�u�����������č��X�N���[���\�ɂȂ��Ă���Ƃ��ɂ́A�^�u�o�[���[�̂ق��̋͂��Ȍ��ԂɂP��O�̃^�u�������ɑ��݂���B
				RECT rc;
				TabCtrl_GetItemRect(hwndTab, nDstTab, &rc);
				if (rc.left > 0) {
					// TAB�܂Ƃ߂� => ���������X�V���Č��Refresh�ʒm
					// TAB�܂Ƃ߂Ȃ��ꍇ�́ARefresh�ʒm�����������������}�E�X�L���v�`�����I������̂ŁA�܂Ƃ߂�Ɠ��������ɂ���
					ReorderTab(nSrcTab, nDstTab);
					Refresh(false);
					if (nTabHover == nSrcTab) {
						nTabHover = nDstTab;	// �����\���̕���{�^�����ꏏ�Ɉړ�����
					}
					nSrcTab = nDstTab;
					bTabSwapped = TRUE;
					::InvalidateRect(GetHwnd(), NULL, TRUE);

					// ����� WM_MOUSEMOVE ���ړ���̃^�u��Ŕ����������̂悤�ɋU�����ă}�E�X�I�[�o�[�n�C���C�g���ړ�����
					TabCtrl_GetItemRect(hwndTab, nDstTab, &rc);
					DefTabWndProc(hwndTab, WM_MOUSEMOVE, wParam, MAKELPARAM((rc.left + rc.right) / 2, HIWORD(lParam)));
				}
			}
		}else {
			if (pShareData->common.tabBar.bDispTabWnd && !pShareData->common.tabBar.bDispTabWndMultiWin) {
				HWND hwndAncestor;
				POINT ptCursor;

				::GetCursorPos(&ptCursor);
				hwndAncestor = MyGetAncestor(::WindowFromPoint(ptCursor), GA_ROOT);
				if (hwndAncestor != GetParentHwnd()) {	// ����ʂ̊O�ɃJ�[�\��������
					if (IsSakuraMainWindow(hwndAncestor)) {
						lpCursorName = MAKEINTRESOURCE(IDC_CURSOR_TAB_JOIN);	// �����J�[�\��
					}else {
						lpCursorName = MAKEINTRESOURCE(IDC_CURSOR_TAB_SEPARATE);	// �����J�[�\��
					}
				}
			}
		}
		if (lpCursorName) {
			hInstance = (lpCursorName == IDC_NO)? NULL: ::GetModuleHandle(NULL);
			::SetCursor(::LoadCursor(hInstance, lpCursorName));
		}else {
			::SetCursor(hDefaultCursor);
		}
		break;

	default:
		return 1L;
	}

	return 0L;
}

// �^�u�� WM_TIMER ����
LRESULT TabWnd::OnTabTimer(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 1) {
		// �J�[�\�����^�u�O�ɂ���ꍇ�ɂ� WM_MOUSEMOVE �𑗂�
		TCHITTESTINFO	hitinfo;
		::GetCursorPos(&hitinfo.pt);
		::ScreenToClient(hwndTab, &hitinfo.pt);
		int nDstTab = TabCtrl_HitTest(hwndTab, (LPARAM)&hitinfo);
		if (nDstTab < 0) {
			::SendMessage(hwndTab, WM_MOUSEMOVE, 0, MAKELONG(hitinfo.pt.x, hitinfo.pt.y));
		}
	}

	return 0L;
}

// �^�u�� WM_CAPTURECHANGED ����
LRESULT TabWnd::OnTabCaptureChanged(WPARAM wParam, LPARAM lParam)
{
	if (eDragState != DRAG_NONE) {
		eDragState = DRAG_NONE;
	}
	return 0L;
}

// �^�u�� WM_RBUTTONDOWN ����
LRESULT TabWnd::OnTabRButtonDown(WPARAM wParam, LPARAM lParam)
{
	BreakDrag();	// 2006.01.28 ryoji �h���b�O��Ԃ���������(�֐���)
	return 0L;	// 2006.01.28 ryoji OnTabMButtonDown �ɂ��킹�� 0 ��Ԃ��悤�ɕύX
}

// �^�u�� WM_RBUTTONUP ����
LRESULT TabWnd::OnTabRButtonUp(WPARAM wParam, LPARAM lParam)
{
	// 2006.01.28 ryoji �^�u�̃J�X�^�����j���[�\���R�}���h�����s����(�֐���)
	return ExecTabCommand(F_CUSTMENU_BASE + CUSTMENU_INDEX_FOR_TABWND, MAKEPOINTS(lParam));
}

/*! �^�u�� WM_MBUTTONDOWN ����
	@date 2006.01.28 ryoji �V�K�쐬
*/
LRESULT TabWnd::OnTabMButtonDown(WPARAM wParam, LPARAM lParam)
{
	BreakDrag();	// 2006.01.28 ryoji �h���b�O��Ԃ���������(�֐���)

	return 0L;	// �t�H�[�J�X���^�u�Ɉڂ�Ȃ��悤�A�����ł� 0 ��Ԃ�
}

/*! �^�u�� WM_MBUTTONUP ����
	@date 2006.01.28 ryoji �V�K�쐬
*/
LRESULT TabWnd::OnTabMButtonUp(WPARAM wParam, LPARAM lParam)
{
	// �E�B���h�E�����R�}���h�����s����
	return ExecTabCommand(F_WINCLOSE, MAKEPOINTS(lParam));
}

/*! �^�u�� WM_NOTIFY ����

	@date 2005.09.01 ryoji �֐���
	@date 2006.10.31 ryoji �c�[���`�b�v�̃t���p�X�����ȈՕ\������
	@date 2007.12.06 ryoji �c�[���`�b�v������OnNotify()�Ɉړ��i�^�u��TCS_TOOLTIPS�X�^�C�����j
*/

LRESULT TabWnd::OnTabNotify(WPARAM wParam, LPARAM lParam)
{
	// �c�[���`�b�v�����폜	// 2007.12.06 ryoji
	return 1L;
}

/*! @brief �^�u�����ύX����

	@param[in] nSrcTab �ړ�����^�u�̃C���f�b�N�X
	@param[in] nDstTab �ړ���^�u�̃C���f�b�N�X

	@date 2005.09.01 ryoji �V�K�쐬
	@date 2007.07.07 genta �E�B���h�E���X�g���암��CShareData::ReorderTab()��
	@date 2010.07.11 Moca �u���[�h�L���X�g�����𕪗�
*/
bool TabWnd::ReorderTab(int nSrcTab, int nDstTab)
{
	TCITEM	tcitem;
	HWND	hwndSrc;	// �ړ����E�B���h�E
	HWND	hwndDst;	// �ړ���E�B���h�E

	if (0 > nSrcTab || 0 > nDstTab || nSrcTab == nDstTab) {
		return false;
	}

	// �ړ����^�u�A�ړ���^�u�̃E�B���h�E���擾����
	tcitem.mask = TCIF_PARAM;
	tcitem.lParam = 0;
	TabCtrl_GetItem(hwndTab, nSrcTab, &tcitem);
	hwndSrc = (HWND)tcitem.lParam;

	tcitem.mask = TCIF_PARAM;
	tcitem.lParam = 0;
	TabCtrl_GetItem(hwndTab, nDstTab, &tcitem);
	hwndDst = (HWND)tcitem.lParam;

	//	2007.07.07 genta CShareData::ReorderTab�Ƃ��ēƗ�
	if (! AppNodeManager::getInstance().ReorderTab(hwndSrc, hwndDst)) {
		return false;
	}
	return true;
}

void TabWnd::BroadcastRefreshToGroup()
{
	// �ĕ\�����b�Z�[�W���u���[�h�L���X�g����B
	int nGroup = AppNodeManager::getInstance().GetEditNode(GetParentHwnd())->GetGroup();
	AppNodeGroupHandle(nGroup).PostMessageToAllEditors(
		MYWM_TAB_WINDOW_NOTIFY,
		(WPARAM)TabWndNotifyType::Refresh,
		(LPARAM)FALSE,
		GetParentHwnd()
	);
}

/** �^�u��������

	�A�N�e�B�u�ȃE�B���h�E�i�擪�E�B���h�E�j���A�N�e�B�u���ێ�����悤�ɕ�������
	���Ȃ�ׂ���A�N�e�B�u�ɂȂ�Ȃ��悤�ɕ������A��A�N�e�B�u�ɂȂ��Ă��܂����狭���I�ɖ߂�

	@date 2007.06.20 ryoji �V�K�쐬
	@date 2007.11.30 ryoji �����I�ȃA�N�e�B�u���߂���ǉ����čő剻���̃^�u�����̕��������
*/
bool TabWnd::SeparateGroup(HWND hwndSrc, HWND hwndDst, POINT ptDrag, POINT ptDrop)
{
	HWND hWnd = GetParentHwnd();
	EditNode* pTopEditNode = AppNodeGroupHandle(0).GetEditNodeAt(0);
	if (!pTopEditNode)
		return false;
	if (hWnd != pTopEditNode->hWnd || hWnd != ::GetForegroundWindow())
		return false;
	auto& appNodeMgr = AppNodeManager::getInstance();
	if (hWnd != appNodeMgr.GetEditNode(hwndSrc)->GetGroup().GetTopEditNode()->GetHwnd())
		return false;
	if (hwndDst && hwndDst != appNodeMgr.GetEditNode(hwndDst)->GetGroup().GetTopEditNode()->GetHwnd())
		return false;
	if (hwndSrc == hwndDst)
		return true;

	EditNode* pSrcEditNode = appNodeMgr.GetEditNode(hwndSrc);
	EditNode* pDstEditNode = appNodeMgr.GetEditNode(hwndDst);
	int showCmdRestore = pDstEditNode? pDstEditNode->showCmdRestore: SW_SHOWNA;

	// �O���[�v�ύX����E�B���h�E���擪�E�B���h�E�Ȃ玟�̃E�B���h�E�����ɂ���i��O�ɂ͏o���Ȃ��j
	// �����łȂ���ΐV�K�O���[�v�ɂȂ�ꍇ�ɕʃE�B���h�E���͎�O�ɕ\�������悤�s���̂܂ܐ擪�E�B���h�E�̂������ɂ����Ă��Ă���
	HWND hwndTop = appNodeMgr.GetEditNode(hwndSrc)->GetGroup().GetTopEditNode()->GetHwnd();
	bool bSrcIsTop = (hwndSrc == hwndTop);
	if (bSrcIsTop) {
		EditNode* pNextEditNode = AppNodeGroupHandle(pSrcEditNode->nGroup).GetEditNodeAt(1);
		if (pNextEditNode) {
			DWORD_PTR dwResult;
			::SendMessageTimeout(
				pNextEditNode->hWnd,
				MYWM_TAB_WINDOW_NOTIFY,
				(WPARAM)TabWndNotifyType::Adjust,
				(LPARAM)NULL,
				SMTO_ABORTIFHUNG | SMTO_BLOCK,
				10000,
				&dwResult
			);
		}
	}else if (!pDstEditNode) {
		::SetWindowPos(hwndSrc, hwndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	//	2007.07.07 genta �����I�ȃO���[�v�ړ��̑����CShareData�ֈړ�
	int notifygroups[2];
	hwndDst = appNodeMgr.SeparateGroup(hwndSrc, hwndDst, bSrcIsTop, notifygroups);

	WINDOWPLACEMENT wp;
	RECT rcDstWork;
	GetMonitorWorkRect(ptDrop, &rcDstWork);
	wp.length = sizeof(wp);
	if (!hwndDst) {	// �V�K�O���[�v�̃E�B���h�E����
		// �E�B���h�E���ړ���ɕ\������
		::GetWindowPlacement(hwndTop, &wp);
		if (wp.showCmd != SW_SHOWMAXIMIZED) {
			// �ړ����̐擪�E�B���h�E�̃T�C�Y�ŉ�ʓ��𑊑Έړ�����
			wp.rcNormalPosition.left += (ptDrop.x - ptDrag.x);
			wp.rcNormalPosition.right += (ptDrop.x - ptDrag.x);
			wp.rcNormalPosition.top += (ptDrop.y - ptDrag.y);
			wp.rcNormalPosition.bottom += (ptDrop.y - ptDrag.y);

			// ��[�����j�^��ʂ���o�Ă��܂�Ȃ��悤�Ɉʒu����
			if (wp.rcNormalPosition.top < rcDstWork.top) {
				wp.rcNormalPosition.bottom += (rcDstWork.top - wp.rcNormalPosition.top);
				wp.rcNormalPosition.top = rcDstWork.top;
			}
		}else {
			// �ړ��惂�j�^�ɍő�\������
			// �i���ɖ߂��T�C�Y�̓��j�^���قȂ�ꍇ�����j�^�����Έʒu���ێ�����悤�Ɉړ����Ă����j
			RECT rcSrcWork;
			GetMonitorWorkRect(ptDrag, &rcSrcWork);
			wp.rcNormalPosition.left += (rcDstWork.left - rcSrcWork.left);
			wp.rcNormalPosition.right += (rcDstWork.left - rcSrcWork.left);
			wp.rcNormalPosition.top += (rcDstWork.top - rcSrcWork.top);
			wp.rcNormalPosition.bottom += (rcDstWork.top - rcSrcWork.top);

			// ���ɖ߂��T�C�Y�����j�^��ʂ���o�Ă��܂�Ȃ��悤�Ɉʒu����
			if (wp.rcNormalPosition.right > rcDstWork.right) {
				wp.rcNormalPosition.left -= (wp.rcNormalPosition.right - rcDstWork.right);
				wp.rcNormalPosition.right -= (wp.rcNormalPosition.right - rcDstWork.right);
			}
			if (wp.rcNormalPosition.bottom > rcDstWork.bottom) {
				wp.rcNormalPosition.top -= (wp.rcNormalPosition.bottom - rcDstWork.bottom);
				wp.rcNormalPosition.bottom -= (wp.rcNormalPosition.bottom - rcDstWork.bottom);
			}
			if (wp.rcNormalPosition.left < rcDstWork.left) {
				wp.rcNormalPosition.right += (rcDstWork.left - wp.rcNormalPosition.left);
				wp.rcNormalPosition.left += (rcDstWork.left - wp.rcNormalPosition.left);
			}
			if (wp.rcNormalPosition.top < rcDstWork.top) {
				wp.rcNormalPosition.bottom += (rcDstWork.top - wp.rcNormalPosition.top);
				wp.rcNormalPosition.top += (rcDstWork.top - wp.rcNormalPosition.top);
			}
		}

		SetCarmWindowPlacement(hwndSrc, &wp);
	}else {
		// �����O���[�v�̃E�B���h�E����
		// �ړ���� WS_EX_TOPMOST ��Ԃ������p��
		if (bSrcIsTop) {
			// �擪�E�B���h�E�̊����O���[�v�ւ̈ړ�
			// �ړ���� WS_EX_TOPMOST ��Ԃ������p��
			HWND hWndInsertAfter = (::GetWindowLongPtr(hwndDst, GWL_EXSTYLE) & WS_EX_TOPMOST)? HWND_TOPMOST: HWND_NOTOPMOST;
			::SetWindowPos(hwndSrc, hWndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

			// �E�B���h�E���ړ���ɕ\������
			::GetWindowPlacement(hwndDst, &wp);
			if (wp.showCmd == SW_SHOWMINIMIZED)
				wp.showCmd = showCmdRestore;
			SetCarmWindowPlacement(hwndSrc, &wp);
			::ShowWindow(hwndDst, SW_HIDE);	// �ړ���̈ȑO�̐擪�E�B���h�E������
		}
	}

	// �ĕ\�����b�Z�[�W���u���[�h�L���X�g����B
	//	2007.07.07 genta 2�񃋁[�v��
	for (size_t group=0; group<_countof(notifygroups); ++group) {
		AppNodeGroupHandle(notifygroups[group]).PostMessageToAllEditors(
			MYWM_TAB_WINDOW_NOTIFY,
			(WPARAM)TabWndNotifyType::Refresh,
			(LPARAM)bSrcIsTop,
			NULL
		);
	}

	// �����I�Ȑ擪�E�B���h�E���ω����Ă����猳�̐擪�E�B���h�E��擪�ɖ߂��B
	// Note. ��A�N�e�B�u�E�B���h�E�ɑ΂��Ď��̑�����s�����ꍇ�A��ʂ���O�ɂ͏o�Ȃ��ꍇ�ł�
	// �A�N�e�B�u���͔������ē����Ǘ��̃E�B���h�E�������ω����Ă��܂��Ă���B
	// �i�����Ǘ��̃E�B���h�E�����͊e����ɓ������ĕω�����j
	//   �ESW_SHOWMAXIMIZED����
	//   �E���ɖ߂��T�C�Y���ő剻�̃E�B���h�E�ɑ΂���SW_SHOWNOACTIVATE����
	// Windows�̃A�N�e�B�u�E�B���h�E�̓X���b�h�P�ʂɊǗ������̂ŕ����̃E�B���h�E���A�N�e�B�u�ɂȂ��Ă���ꍇ������B
	pTopEditNode = AppNodeGroupHandle(0).GetEditNodeAt(0);	// �S�̂̐擪�E�B���h�E�����擾
	HWND hwndLastTop = pTopEditNode? pTopEditNode->hWnd: NULL;
	if (hwndLastTop != hwndTop) {
		HWND hwndFore = ::GetForegroundWindow();
		if (hwndFore == hwndTop) {
			// ���̐擪�E�B���h�E����O�Ȃ̂ɍŌ�ɃA�N�e�B�u�����ꂽ�̂��ʃE�B���h�E�ɂȂ��Ă���
			// �����I�Ȑ擪�E�B���h�E����O�ɏo���ď�Ԑ����i���̐擪�E�B���h�E�͈�U��A�N�e�B�u�ɂ���j
			SetForegroundWindow(hwndLastTop);
			hwndFore = ::GetForegroundWindow();
		}

		// ���̐擪�E�B���h�E�i�����j����O�ɏo��
		DWORD dwTidFore = ::GetWindowThreadProcessId(hwndFore, NULL);
		DWORD dwTidTop = ::GetWindowThreadProcessId(hwndTop, NULL);
		::AttachThreadInput(dwTidTop, dwTidFore, TRUE);
		SetForegroundWindow(hwndTop);
		::AttachThreadInput(dwTidTop, dwTidFore, FALSE);

		// WinXP Visual Style�ł͏�̑���� hwndLastTop ����A�N�e�B�u�����Ă�
		// �^�C�g���o�[�F�����A�N�e�B�u�̂܂܂Ƃ������Ƃ����邩������Ȃ�
	}

	return true;
}

/*! �^�u�� �R�}���h���s����
	@date 2006.01.28 ryoji �V�K�쐬
*/
LRESULT TabWnd::ExecTabCommand(int nId, POINTS pts)
{
	// �}�E�X�ʒu(pt)�̃^�u���擾����
	TCHITTESTINFO hitinfo;
	hitinfo.pt.x = pts.x;
	hitinfo.pt.y = pts.y;
	int nTab = TabCtrl_HitTest(hwndTab, (LPARAM)&hitinfo);
	if (nTab < 0)
		return 1L;

	// �ΏۃE�B���h�E���擾����
	TCITEM	tcitem;
	tcitem.mask   = TCIF_PARAM;
	tcitem.lParam = 0;
	if (!TabCtrl_GetItem(hwndTab, nTab, &tcitem)) {
		return 1L;
	}
	HWND hWnd = (HWND)tcitem.lParam;

	// �ΏۃE�B���h�E���A�N�e�B�u�ɂ���B
	ShowHideWindow(hWnd, TRUE);

	// �R�}���h��ΏۃE�B���h�E�ɑ���B
	::PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(nId, 0), (LPARAM)NULL);

	return 0L;
}

TabWnd::TabWnd()
	:
	Wnd(_T("::TabWnd")),
	eTabPosition(TabPosition::None),
	eDragState(DRAG_NONE),
	bVisualStyle(false),		// 2007.04.01 ryoji
	bHovering(false),			// 2006.02.01 ryoji
	bListBtnHilighted(false),	// 2006.02.01 ryoji
	bCloseBtnHilighted(false),	// 2006.10.21 ryoji
	eCaptureSrc(CAPT_NONE),		// 2006.11.30 ryoji
	nTabBorderArray(NULL),		// 2012.04.22 syat
	nTabHover(-1),
	bTabCloseHover(false),
	nTabCloseCapture(-1),
	hwndSizeBox(NULL),
	bSizeBox(false)
{
	// ���L�f�[�^�\���̂̃A�h���X��Ԃ�
	pShareData = &GetDllShareData();

	hwndTab    = NULL;
	hFont      = NULL;
	g_pOldWndProc = nullptr;
	hwndToolTip = NULL;
	hIml = NULL;

	// 2006.02.17 ryoji ImageList_Duplicate() �̃A�h���X���擾����
	// �iIE4.0 �����̊��ł�����\�Ȃ悤�ɓ��I���[�h�j
    HINSTANCE hinst = ::GetModuleHandle(TEXT("comctl32"));
    *(FARPROC*)&realImageList_Duplicate = ::GetProcAddress(hinst, "ImageList_Duplicate");

	return;
}

TabWnd::~TabWnd()
{
	return;
}

// �E�B���h�E �I�[�v��
HWND TabWnd::Open(HINSTANCE hInstance, HWND hwndParent)
{
	LPCTSTR pszClassName = _T("TabWnd");
	
	// ������
	hwndTab = NULL;
	hFont = NULL;
	g_pOldWndProc = nullptr;
	hwndToolTip = NULL;
	bVisualStyle = ::IsVisualStyle();	// 2007.04.01 ryoji
	eDragState = DRAG_NONE;	//	2005.09.29 ryoji
	bHovering = false;			// 2006.02.01 ryoji
	bListBtnHilighted = false;	// 2006.02.01 ryoji
	bCloseBtnHilighted = false;	// 2006.10.21 ryoji
	eCaptureSrc = CAPT_NONE;	// 2006.11.30 ryoji
	eTabPosition = TabPosition::None;

	// �E�B���h�E�N���X�쐬
	RegisterWC(
		hInstance,
		NULL,								// Handle to the class icon.
		NULL,								// Handle to a small icon
		::LoadCursor(NULL, IDC_ARROW),		// Handle to the class cursor.
		// 2006.01.30 ryoji �w�i�� WM_PAINT �ŕ`�悷��ق���������Ȃ��i�Ǝv���j
		//(HBRUSH)(COLOR_3DFACE + 1),			// Handle to the class background brush.
		NULL,								// Handle to the class background brush.
		NULL,								// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
		pszClassName						// Pointer to a null-terminated string or is an atom.
	);

	RECT rcParent;
	::GetWindowRect(hwndParent, &rcParent);

	// ���N���X�����o�Ăяo��
	Wnd::Create(
		hwndParent,
		0,									// extended window style
		pszClassName,						// Pointer to a null-terminated string or is an atom.
		pszClassName,						// pointer to window name
		WS_CHILD/* | WS_VISIBLE*/,			// window style	// 2007.03.08 ryoji WS_VISIBLE ����
		// 2006.01.30 ryoji �����z�u������
		// ���^�u��\�� -> �\���ؑւŕҏW�E�B���h�E�ɃS�~���\������邱�Ƃ�����̂ŏ������̓[����
		CW_USEDEFAULT,						// horizontal position of window
		0,									// vertical position of window
		rcParent.right - rcParent.left,		// window width
		TAB_WINDOW_HEIGHT,					// window height
		NULL								// handle to menu, or child-window identifier
	);

	// �^�u�E�B���h�E���쐬����B
	hwndTab = ::CreateWindow(
		WC_TABCONTROL,
		_T(""),
		//	2004.05.22 MIK ������TAB�΍��WS_CLIPSIBLINGS�ǉ�
		// 2007.12.06 ryoji TCS_TOOLTIPS�ǉ��i�^�u�p�̃c�[���`�b�v�̓^�u�ɍ�点��j
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_TOOLTIPS,
		// 2006.01.30 ryoji �����z�u������
		TAB_MARGIN_LEFT,
		TAB_MARGIN_TOP,
		rcParent.right - rcParent.left - (TAB_MARGIN_LEFT + TAB_MARGIN_RIGHT),
		TAB_WINDOW_HEIGHT,
		GetHwnd(),
		(HMENU)NULL,
		GetAppInstance(),
		(LPVOID)NULL
		);
	if (hwndTab) {
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr(hwndTab, GWLP_USERDATA, (LONG_PTR) this);
		g_pOldWndProc = (WNDPROC)::SetWindowLongPtr(hwndTab, GWLP_WNDPROC, (LONG_PTR) TabWndProc);

		// �X�^�C����ύX����B
		UINT lngStyle;
		lngStyle = (UINT)::GetWindowLongPtr(hwndTab, GWL_STYLE);
		//	Feb. 14, 2004 MIK �}���`���C�����̕ύX�����߂�
		lngStyle &= ~(TCS_BUTTONS | TCS_MULTILINE);
		if (pShareData->common.tabBar.bTabMultiLine) {
			lngStyle |= TCS_MULTILINE;
		}else {
			lngStyle |= TCS_SINGLELINE;
		}
		bMultiLine = pShareData->common.tabBar.bTabMultiLine;
		lngStyle |= TCS_TABS | TCS_FOCUSNEVER | TCS_FIXEDWIDTH | TCS_FORCELABELLEFT;	// 2006.01.28 ryoji
		//lngStyle &= ~(TCS_BUTTONS | TCS_SINGLELINE);	//2004.01.31
		//lngStyle |= TCS_TABS | TCS_MULTILINE;
		::SetWindowLongPtr(hwndTab, GWL_STYLE, lngStyle);
		TabCtrl_SetItemSize(hwndTab, MAX_TABITEM_WIDTH, TAB_ITEM_HEIGHT);	// 2006.01.28 ryoji

		// �^�u�̃c�[���`�b�v�X�^�C����ύX����	// 2007.12.06 ryoji
		HWND hwndToolTips;
		hwndToolTips = TabCtrl_GetToolTips(hwndTab);
		lngStyle = (UINT)::GetWindowLongPtr(hwndToolTips, GWL_STYLE);
		lngStyle |= TTS_ALWAYSTIP | TTS_NOPREFIX;	// �]���ʂ�TTS_ALWAYSTIP�ɂ��Ă���
		::SetWindowLongPtr(hwndToolTips, GWL_STYLE, lngStyle);

		// �\���p�t�H���g
		// LOGFONT�̏�����
		lf = pShareData->common.tabBar.lf;
		hFont = ::CreateFontIndirect(&lf);
		// �t�H���g�ύX
		::SendMessage(hwndTab, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

		// �c�[���`�b�v���쐬����B�i�^�u�ł͂Ȃ��u����v�Ȃǂ̃{�^���p�j
		// 2005.08.11 ryoji �u�d�˂ĕ\���v��Z-order�����������Ȃ�̂�TOPMOST�w�������
		hwndToolTip = ::CreateWindowEx(
			0,
			TOOLTIPS_CLASS,
			NULL,
			WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			GetHwnd(), //hwndTab,
			NULL,
			GetAppInstance(),
			NULL
			);

		// �c�[���`�b�v���}���`���C���\�ɂ���iSHRT_MAX: Win95��INT_MAX���ƕ\������Ȃ��j	// 2007.03.03 ryoji
		Tooltip_SetMaxTipWidth(hwndToolTip, SHRT_MAX);

		// �^�u�o�[�Ƀc�[���`�b�v��ǉ�����
		TOOLINFO ti;
		ti.cbSize      = CCSIZEOF_STRUCT(TOOLINFO, lpszText);
		ti.uFlags      = TTF_SUBCLASS | TTF_IDISHWND;	// TTF_IDISHWND: uId �� HWND �� rect �͖����iHWND �S�́j
		ti.hwnd        = GetHwnd();
		ti.hinst       = GetAppInstance();
		ti.uId         = (UINT_PTR)GetHwnd();
		ti.lpszText    = NULL;
		ti.rect.left   = 0;
		ti.rect.top    = 0;
		ti.rect.right  = 0;
		ti.rect.bottom = 0;
		Tooltip_AddTool(hwndToolTip, &ti);

		// 2006.02.22 ryoji �C���[�W���X�g������������
		InitImageList();

		Refresh();	// �^�u��\������\���ɐ؂�ւ�����Ƃ��Ɋe�E�B���h�E�̏����^�u�o�^����K�v������

		LayoutTab();
	}

	return GetHwnd();
}

void TabWnd::UpdateStyle()
{
	if (bMultiLine != pShareData->common.tabBar.bTabMultiLine) {
		bMultiLine = pShareData->common.tabBar.bTabMultiLine;
		UINT lngStyle = (UINT)::GetWindowLongPtr(hwndTab, GWL_STYLE);
		if (pShareData->common.tabBar.bTabMultiLine) {
			lngStyle |= TCS_MULTILINE;
		}else {
			lngStyle &= ~TCS_MULTILINE;
		}
		::SetWindowLongPtr(hwndTab, GWL_STYLE, lngStyle);
	}
}

// �E�B���h�E �N���[�Y
void TabWnd::Close(void)
{
	if (GetHwnd()) {
		if (g_pOldWndProc) {
			// Modified by KEITA for WIN64 2003.9.6
			::SetWindowLongPtr(hwndTab, GWLP_WNDPROC, (LONG_PTR)g_pOldWndProc);
			g_pOldWndProc = NULL;
		}
		
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr(hwndTab, GWLP_USERDATA, (LONG_PTR)NULL);

		if (hwndToolTip) {
			::DestroyWindow(hwndToolTip);
			hwndToolTip = NULL;
		}

		DestroyWindow();
	}
}

// WM_SIZE����
LRESULT TabWnd::OnSize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!GetHwnd() || !hwndTab) {
		return 0L;
	}
	RECT rcWnd;
	GetWindowRect(&rcWnd);

	int nSizeBoxWidth = 0;
	if (hwndSizeBox) {
		nSizeBoxWidth = ::GetSystemMetrics(SM_CXVSCROLL);
		int nSizeBoxHeight = ::GetSystemMetrics(SM_CYHSCROLL);
		::MoveWindow(
			hwndSizeBox,
			rcWnd.right - rcWnd.left - nSizeBoxWidth,
			rcWnd.bottom - rcWnd.top - nSizeBoxHeight,
			nSizeBoxWidth,
			nSizeBoxHeight,
			TRUE
		);
	}
	
	LayoutTab();	// 2006.01.28 ryoji �^�u�̃��C�A�E�g��������

	::InvalidateRect(GetHwnd(), NULL, FALSE);	//	2006.02.01 ryoji

	return 0L;
}

// WM_DSESTROY����
LRESULT TabWnd::OnDestroy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// �^�u�R���g���[�����폜
	if (hwndTab) {
		::DestroyWindow(hwndTab);
		hwndTab = NULL;
	}

	// �\���p�t�H���g
	if (hFont) {
		::DeleteObject(hFont);
		hFont = NULL;
	}

	// 2006.01.28 ryoji �C���[�W���X�g���폜
	if (hIml) {
		ImageList_Destroy(hIml);
		hIml = NULL;
	}

	::KillTimer(hwnd, 1);	//	2006.02.01 ryoji

	_SetHwnd(NULL);

	return 0L;
}
 
/*! WM_LBUTTONDBLCLK����
	@date 2006.03.26 ryoji �V�K�쐬
*/
LRESULT TabWnd::OnLButtonDblClk(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// �V�K�쐬�R�}���h�����s����
	::SendMessage(GetParentHwnd(), WM_COMMAND, MAKEWPARAM(F_FILENEW, 0), (LPARAM)NULL);
	return 0L;
}

/*!	WM_CAPTURECHANGED����
	@date 2006.11.30 ryoji �V�K�쐬
*/
LRESULT TabWnd::OnCaptureChanged(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (eCaptureSrc != CAPT_NONE) {
		eCaptureSrc = CAPT_NONE;
	}
	return 0L;
}

/*!	WM_LBUTTONDOWN����
	@date 2006.02.01 ryoji �V�K�쐬
	@date 2006.11.30 ryoji �^�u�ꗗ�{�^���N���b�N�֐���p�~���ď�����荞��
	                       ����{�^����Ȃ�L���v�`���[�J�n
*/
LRESULT TabWnd::OnLButtonDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	RECT rc;
	RECT rcBtn;

	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	::GetClientRect(GetHwnd(), &rc);

	// �^�u�ꗗ�{�^����Ȃ�^�u�ꗗ���j���[�i�^�u���j��\������
	GetListBtnRect(&rc, &rcBtn);
	if (::PtInRect(&rcBtn, pt)) {
		pt.x = rcBtn.left;
		pt.y = rcBtn.bottom;
		::ClientToScreen(GetHwnd(), &pt);
		TabListMenu(pt, false, false, false);	// �^�u�ꗗ���j���[�i�^�u���j
	}else {
		// ����{�^����Ȃ�L���v�`���[�J�n
		GetCloseBtnRect(&rc, &rcBtn);
		if (::PtInRect(&rcBtn, pt)) {
			eCaptureSrc = CAPT_CLOSE;	// �L���v�`���[���͕���{�^��
			::SetCapture(GetHwnd());
		}
	}

	return 0L;
}

/*!	WM_LBUTTONUP����
	@date 2006.11.30 ryoji �V�K�쐬
*/
LRESULT TabWnd::OnLButtonUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	RECT rc;
	RECT rcBtn;

	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	::GetClientRect(GetHwnd(), &rc);

	if (::GetCapture() == GetHwnd()) { // ���E�B���h�E���}�E�X�L���v�`���[���Ă���?
		if (eCaptureSrc == CAPT_CLOSE) {	// �L���v�`���[���͕���{�^��?
			// ����{�^����Ȃ�^�u�����
			GetCloseBtnRect(&rc, &rcBtn);
			if (::PtInRect(&rcBtn, pt)) {
				int nId;
				if (pShareData->common.tabBar.bDispTabWnd && !pShareData->common.tabBar.bDispTabWndMultiWin) {
					if (!pShareData->common.tabBar.bTab_CloseOneWin) {
						nId = F_WINCLOSE;	// ����i�^�C�g���o�[�̕���{�^���͕ҏW�̑S�I���j
					}else {
						nId = F_GROUPCLOSE;	// �O���[�v�����
					}
				}else {
					nId = F_EXITALLEDITORS;	// �ҏW�̑S�I���i�^�C�g���o�[�̕���{�^���͂P��������j
				}
				::PostMessage(GetParentHwnd(), WM_COMMAND, MAKEWPARAM(nId, 0), (LPARAM)NULL);
			}
		}

		// �L���v�`���[����
		eCaptureSrc = CAPT_NONE;
		::ReleaseCapture();
	}

	return 0L;
}

/*!	WM_RBUTTONDOWN����
	@date 2006.02.01 ryoji �V�K�쐬
	@date 2006.11.30 ryoji �^�u�ꗗ�{�^���N���b�N�֐���p�~���ď�����荞��
*/
LRESULT TabWnd::OnRButtonDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	RECT rc;
	RECT rcBtn;

	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	::GetClientRect(GetHwnd(), &rc);

	// �^�u�ꗗ�{�^����Ȃ�^�u�ꗗ���j���[�i�t���p�X�j��\������	// 2006.11.30 ryoji
	GetListBtnRect(&rc, &rcBtn);
	if (::PtInRect(&rcBtn, pt)) {
		pt.x = rcBtn.left;
		pt.y = rcBtn.bottom;
		::ClientToScreen(GetHwnd(), &pt);
		TabListMenu(pt, false, true, false);	// �^�u�ꗗ���j���[�i�t���p�X�j
	}

	return 0L;
}

/*!	WM_MEASUREITEM����
	@date 2006.02.01 ryoji �V�K�쐬
*/
LRESULT TabWnd::OnMeasureItem(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MEASUREITEMSTRUCT* lpmis = (MEASUREITEMSTRUCT*)lParam;
	if (lpmis->CtlType == ODT_MENU) {
		TABMENU_DATA* pData = (TABMENU_DATA*)lpmis->itemData;

		HDC hdc = ::GetDC(hwnd);
		HFONT hFont = CreateMenuFont();
		HFONT hFontOld = (HFONT)::SelectObject(hdc, hFont);

		SIZE size;
		::GetTextExtentPoint32(hdc, pData->szText, ::_tcslen(pData->szText), &size);

		int cxIcon = CX_SMICON;
		int cyIcon = CY_SMICON;
		if (hIml) {
			ImageList_GetIconSize(hIml, &cxIcon, &cyIcon);
		}

		lpmis->itemHeight = ::GetSystemMetrics(SM_CYMENU);
		lpmis->itemWidth = (cxIcon + DpiScaleX(8)) + size.cx;

		::SelectObject(hdc, hFontOld);
		::DeleteObject(hFont);
		::ReleaseDC(hwnd, hdc);
	}

	return 0L;
}

/*!	WM_DRAWITEM����
	@date 2006.02.01 ryoji �V�K�쐬
	@date 2012.04.14 syat �^�u�̃I�[�i�[�h���[�ǉ�
*/
LRESULT TabWnd::OnDrawItem(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DRAWITEMSTRUCT* lpdis = (DRAWITEMSTRUCT*)lParam;
	if (lpdis->CtlType == ODT_MENU) {
		// �^�u�ꗗ���j���[��`�悷��
		TABMENU_DATA* pData = (TABMENU_DATA*)lpdis->itemData;

		// �`��Ώ�
		HDC hdc = lpdis->hDC;
		Graphics gr(hdc);
		RECT rcItem = lpdis->rcItem;

		// ��Ԃɏ]���ăe�L�X�g�Ɣw�i�F�����߂�
		COLORREF clrText;
		INT_PTR nSysClrBk;
		if (lpdis->itemState & ODS_SELECTED) {
			clrText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
			nSysClrBk = COLOR_HIGHLIGHT;
		}else {
			clrText = ::GetSysColor(COLOR_MENUTEXT);
			nSysClrBk = COLOR_MENU;
		}

		// �w�i�`��
		::FillRect(gr, &rcItem, (HBRUSH)(nSysClrBk + 1));

		// �A�C�R���`��
		int cxIcon = CX_SMICON;
		int cyIcon = CY_SMICON;
		if (hIml) {
			ImageList_GetIconSize(hIml, &cxIcon, &cyIcon);
			if (0 <= pData->iImage) {
				int top = rcItem.top + (rcItem.bottom - rcItem.top - cyIcon) / 2;
				ImageList_Draw(hIml, pData->iImage, lpdis->hDC, rcItem.left + DpiScaleX(2), top, ILD_TRANSPARENT);
			}
		}

		// �e�L�X�g�`��
		gr.PushTextForeColor(clrText);
		gr.SetTextBackTransparent(true);
		HFONT hFont = CreateMenuFont();
		gr.PushMyFont(hFont);
		RECT rcText = rcItem;
		rcText.left += (cxIcon + DpiScaleX(8));

		::DrawText(gr, pData->szText, -1, &rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER);

		gr.PopTextForeColor();
		gr.PopMyFont();
		::DeleteObject(hFont);

		// �`�F�b�N��ԂȂ�O�g�`��
		if (lpdis->itemState & ODS_CHECKED) {
			gr.SetPen(::GetSysColor(COLOR_HIGHLIGHT));
			gr.SetBrushColor(-1); //NULL_BRUSH
			::Rectangle(gr, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
		}
	}else if (lpdis->CtlType == ODT_TAB) {
		// �^�u��`�悷��
		int nTabIndex = lpdis->itemID;
		HWND hwndItem = lpdis->hwndItem;
		TCITEM item;
		TCHAR szBuf[256];
		int nSelIndex = TabCtrl_GetCurSel(hwndTab);
		bool bSelected = (nSelIndex == nTabIndex);
		int nTabCount = TabCtrl_GetItemCount(hwndTab);

		item.mask = TCIF_TEXT | TCIF_PARAM | TCIF_IMAGE;
		item.pszText = szBuf;
		item.cchTextMax = _countof(szBuf);
		TabCtrl_GetItem(hwndItem, nTabIndex, &item);

		// �`��Ώ�
		HDC hdc = lpdis->hDC;
		Graphics gr(hdc);
		RECT rcItem = lpdis->rcItem;
		RECT rcFullItem(rcItem);

		// ��Ԃɏ]���ăe�L�X�g�Ɣw�i�F�����߂�

		// �w�i�`��
		if (!IsVisualStyle()) {
			::FillRect(gr, &rcItem, (HBRUSH)(COLOR_BTNFACE + 1));
		}else {
			UxTheme& uxTheme = UxTheme::getInstance();
			int iPartId = TABP_TABITEM;
			int iStateId = TIS_NORMAL;
			HTHEME hTheme = uxTheme.OpenThemeData(hwndTab, L"TAB");
			if (hTheme) {
				if (!bSelected) {
					::InflateRect(&rcFullItem, DpiScaleX(2), DpiScaleY(2));
					if (nTabIndex == nSelIndex - 1) {
						rcFullItem.right -= DpiScaleX(1);
					}else if (nTabIndex == nSelIndex + 1) {
						rcFullItem.left += DpiScaleX(1);
					}
				}
				bool bHotTracked = ::GetTextColor(hdc) == GetSysColor(COLOR_HOTLIGHT);

				RECT rcBk(rcFullItem);
				if (bSelected) {
					iStateId = TIS_SELECTED;
					if (nTabIndex == 0) {
						if (nTabIndex == nTabCount - 1) {
							iPartId = TABP_TOPTABITEMBOTHEDGE;
						}else {
							iPartId = TABP_TOPTABITEMLEFTEDGE;
						}
					}else if (nTabIndex == nTabCount - 1) {
						iPartId = TABP_TOPTABITEMRIGHTEDGE;
					}else {
						iPartId = TABP_TOPTABITEM;
					}
				}else {
					rcFullItem.top += DpiScaleY(2);
					rcBk.top += DpiScaleY(2);
					iStateId = bHotTracked ? TIS_HOT : TIS_NORMAL;
					if (nTabIndex == 0) {
						if (nTabIndex == nTabCount - 1) {
							iPartId = TABP_TABITEMBOTHEDGE;
						}else {
							iPartId = TABP_TABITEMLEFTEDGE;
						}
					}else if (nTabIndex == nTabCount - 1) {
						iPartId = TABP_TABITEMRIGHTEDGE;
					}else {
						iPartId = TABP_TABITEM;
					}
				}

				if (uxTheme.IsThemeBackgroundPartiallyTransparent(hTheme, iPartId, iStateId)) {
					uxTheme.DrawThemeParentBackground(hwndTab, hdc, &rcFullItem);
				}
				uxTheme.DrawThemeBackground(hTheme, hdc, iPartId, iStateId, &rcBk, NULL);
			}
		}

		rcItem.left += DpiScaleX(4) + (bSelected ? DpiScaleX(4) : 0);

		// �A�C�R���`��
		int cxIcon = CX_SMICON;
		int cyIcon = CY_SMICON;
		if (hIml) {
			ImageList_GetIconSize(hIml, &cxIcon, &cyIcon);
			if (0 <= item.iImage) {
				int top = rcItem.top + (rcItem.bottom - rcItem.top - cyIcon) / 2 - 1;
				ImageList_Draw(hIml, item.iImage, lpdis->hDC, rcItem.left,
					top + (bSelected ? 0 : DpiScaleY(3)), ILD_TRANSPARENT);
				rcItem.left += cxIcon + DpiScaleX(6);
			}
		}

		// �e�L�X�g�`��
		COLORREF clrText;
		clrText = ::GetSysColor(COLOR_MENUTEXT);
		gr.PushTextForeColor(clrText);
		gr.SetTextBackTransparent(true);
		RECT rcText = rcItem;
		rcText.top += (bSelected ? 0 : DpiScaleY(5)) - DpiScaleY(1);

		// �e�L�X�g��`�͍ő�ł��^�u�����{�^���̍��[�܂łɐ؂�l�߂�
		// �^�u�����{�^���̋�`�͑��̉ӏ��Ɠ��l TabCtrl_GetItemRect �̋�`����擾�ilpdis->rcItem �̋�`���Ǝ኱�����j
		DispTabCloseType dispTabClose = pShareData->common.tabBar.dispTabClose;
		bool bDrawTabCloseBtn = (dispTabClose == DispTabCloseType::Always || (dispTabClose == DispTabCloseType::Auto && nTabIndex == nTabHover));
		RECT rcGetItemRect;
		TabCtrl_GetItemRect(hwndTab, nTabIndex, &rcGetItemRect);
		if (bDrawTabCloseBtn) {
			RECT rcClose;
			GetTabCloseBtnRect(&rcGetItemRect, &rcClose, nTabIndex == TabCtrl_GetCurSel(hwndTab));
			rcText.right = rcClose.left;
		}

		::DrawText(gr, szBuf, -1, &rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER);

		gr.PopTextForeColor();

		// �^�u�����{�^����`��
		if (bDrawTabCloseBtn) {
			DrawTabCloseBtn(gr, &rcGetItemRect, bSelected, (nTabIndex == nTabHover) && bTabCloseHover);
		}

		// Vista�ȍ~�ł̓I�[�i�[�h���[�^�u�Ɏ�����3D�g���`�悳��Ă��܂����߁A
		// �`��͈͂𖳌�������
		if (IsVisualStyle()) {
			ExcludeClipRect(hdc, rcFullItem.left, rcFullItem.top, rcFullItem.right, rcFullItem.bottom);
		}
	}

	return 0L;
}

/*!	WM_MOUSEMOVE����
	@date 2006.02.01 ryoji �V�K�쐬
	@date 2007.03.05 ryoji �{�^���̏o����Ńc�[���`�b�v���X�V����
*/
LRESULT TabWnd::OnMouseMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// �J�[�\�����E�B���h�E���ɓ�������^�C�}�[�N��
	// �E�B���h�E�O�ɏo����^�C�}�[�폜
	POINT pt;
	RECT rc;
	bool bHovering;

	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	::GetClientRect(hwnd, &rc);
	bHovering = ::PtInRect(&rc, pt) != FALSE;
	if (this->bHovering != bHovering) {
		this->bHovering = bHovering;
		if (bHovering) {
			::SetTimer(hwnd, 1, 200, NULL);
		}else {
			::KillTimer(hwnd, 1);
		}
	}

	// �J�[�\�����{�^������o���肷��Ƃ��ɍĕ`��
	RECT rcBtn;
	LPTSTR pszTip = (LPTSTR)-1L;
	TCHAR szText[80];	// 2007.12.06 ryoji �����o�ϐ����g���K�v�͖����̂Ń��[�J���ϐ��ɂ���

	GetListBtnRect(&rc, &rcBtn);
	bHovering = ::PtInRect(&rcBtn, pt) != FALSE;
	if (bHovering != bListBtnHilighted) {
		bListBtnHilighted = bHovering;
		::InvalidateRect(hwnd, &rcBtn, FALSE);

		// �c�[���`�b�v�p�̕�����쐬	// 2007.03.05 ryoji
		pszTip = NULL;	// �{�^���̊O�ɏo��Ƃ��͏���
		if (bListBtnHilighted) {	// �{�^���ɓ����Ă���?
			pszTip = szText;
			_tcscpy_s(szText, LS(STR_TABWND_LR_INFO));
		}
	}

	GetCloseBtnRect(&rc, &rcBtn);
	bHovering = ::PtInRect(&rcBtn, pt) != FALSE;
	if (bHovering != bCloseBtnHilighted) {
		bCloseBtnHilighted = bHovering;
		::InvalidateRect(hwnd, &rcBtn, FALSE);

		// �c�[���`�b�v�p�̕�����쐬	// 2007.03.05 ryoji
		pszTip = NULL;	// �{�^���̊O�ɏo��Ƃ��͏���
		if (bCloseBtnHilighted) {	// �{�^���ɓ����Ă���?
			pszTip = szText;
			auto& csTabBar = pShareData->common.tabBar;
			if (csTabBar.bDispTabWnd && !csTabBar.bDispTabWndMultiWin) {
				if (!csTabBar.bTab_CloseOneWin) {
					_tcscpy_s(szText, LS(STR_TABWND_CLOSETAB));
				}else {
					::LoadString(GetAppInstance(), F_GROUPCLOSE, szText, _countof(szText));
					szText[_countof(szText) - 1] = _T('\0');
				}
			}else {
				::LoadString(GetAppInstance(), F_EXITALLEDITORS, szText, _countof(szText));
				szText[_countof(szText) - 1] = _T('\0');
			}
		}
	}
	
	// �c�[���`�b�v�X�V	// 2007.03.05 ryoji
	if (pszTip != (LPTSTR)-1L) {	// �{�^���ւ̏o���肪������?
		TOOLINFO ti = {0};
		ti.cbSize       = CCSIZEOF_STRUCT(TOOLINFO, lpszText);
		ti.hwnd         = GetHwnd();
		ti.hinst        = GetAppInstance();
		ti.uId          = (UINT_PTR)GetHwnd();
		ti.lpszText     = pszTip;
		Tooltip_UpdateTipText(hwndToolTip, &ti);
	}

	return 0L;
}

/*!	WM_TIMER����
	@date 2006.02.01 ryoji �V�K�쐬
*/
LRESULT TabWnd::OnTimer(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == 1) {
		// �J�[�\�����E�B���h�E�O�ɂ���ꍇ�ɂ� WM_MOUSEMOVE �𑗂�
		POINT pt;
		RECT rc;

		::GetCursorPos(&pt);
		::ScreenToClient(hwnd, &pt);
		::GetClientRect(hwnd, &rc);
		if (!::PtInRect(&rc, pt)) {
			::SendMessage(hwnd, WM_MOUSEMOVE, 0, MAKELONG(pt.x, pt.y));
		}
	}

	return 0L;
}

/*!	WM_PAINT����

	@date 2005.09.01 ryoji �^�u�̏�ɋ��E����ǉ�
	@date 2006.01.30 ryoji �w�i�`�揈����ǉ��i�w�i�u���V�� NULL �ɕύX�j
	@date 2006.02.01 ryoji �ꗗ�{�^���̕`�揈����ǉ�
	@date 2006.10.21 ryoji ����{�^���̕`�揈����ǉ�
	@date 2007.03.27 ryoji Windows�N���V�b�N�X�^�C���̏ꍇ�̓A�N�e�B�u�^�u�̏㕔�Ƀg�b�v�o���h��`�悷��
*/
LRESULT TabWnd::OnPaint(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	RECT rc;

	// �`��Ώ�
	HDC hdc = ::BeginPaint(hwnd, &ps);
	Graphics gr(hdc);

	// �w�i��`�悷��
	::GetClientRect(hwnd, &rc);
	::FillRect(gr, &rc, (HBRUSH)(COLOR_3DFACE + 1));

	// �{�^����`�悷��
	DrawListBtn(gr, &rc);
	DrawCloseBtn(gr, &rc);	// 2006.10.21 ryoji �ǉ�

	// �㑤�ɋ��E����`�悷��
	::DrawEdge(gr, &rc, EDGE_ETCHED, BF_TOP);

	// Windows�N���V�b�N�X�^�C���̏ꍇ�̓A�N�e�B�u�^�u�̏㕔�Ƀg�b�v�o���h��`�悷��	// 2006.03.27 ryoji
	if (!bVisualStyle) {
		int nCurSel = TabCtrl_GetCurSel(hwndTab);
		if (nCurSel >= 0) {
			POINT pt;
			RECT rcCurSel;

			TabCtrl_GetItemRect(hwndTab, nCurSel, &rcCurSel);
			pt.x = rcCurSel.left;
			pt.y = 0;
			::ClientToScreen(hwndTab, &pt);
			::ScreenToClient(GetHwnd(), &pt);
			rcCurSel.right = pt.x + (rcCurSel.right - rcCurSel.left) - 1;
			rcCurSel.left = pt.x + 1;
			rcCurSel.top = rc.top + TAB_MARGIN_TOP - 2;
			rcCurSel.bottom = rc.top + TAB_MARGIN_TOP;

			if (rcCurSel.left < rc.left + TAB_MARGIN_LEFT)
				rcCurSel.left = rc.left + TAB_MARGIN_LEFT;	// ���[���E�l

			HWND hwndUpDown = ::FindWindowEx(hwndTab, NULL, UPDOWN_CLASS, 0);	// �^�u���� Up-Down �R���g���[��
			if (hwndUpDown && ::IsWindowVisible(hwndUpDown)) {
				POINT ptREnd;
				RECT rcUpDown;

				::GetWindowRect(hwndUpDown, &rcUpDown);
				ptREnd.x = rcUpDown.left;
				ptREnd.y = 0;
				::ScreenToClient(GetHwnd(), &ptREnd);
				if (rcCurSel.right > ptREnd.x)
					rcCurSel.right = ptREnd.x;	// �E�[���E�l
			}

			if (rcCurSel.left < rcCurSel.right) {
				HBRUSH hBr = ::CreateSolidBrush(RGB(255, 128, 0));
				::FillRect(gr, &rcCurSel, hBr);
				::DeleteObject(hBr);
			}
		}
	}

	// �T�C�Y�{�b�N�X��`�悷��
	auto& csWindow = pShareData->common.window;
	if (!csWindow.bDispStatusBar 
		&& !csWindow.bDispFuncKeyWnd
		&& pShareData->common.tabBar.eTabPosition == TabPosition::Bottom
	) {
		SizeBox_ONOFF(true);
	}

	::EndPaint(hwnd, &ps);

	return 0L;
}

/*! WM_NOTIFY����

	@date 2005.09.01 ryoji �E�B���h�E�؂�ւ��� OnTabLButtonUp() �Ɉړ�
	@date 2007.12.06 ryoji �^�u�̃c�[���`�b�v������OnTabNotify()����ړ��i�^�u��TCS_TOOLTIPS�X�^�C�����j
*/
LRESULT TabWnd::OnNotify(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// 2005.09.01 ryoji �E�B���h�E�؂�ւ��� OnTabLButtonUp() �Ɉړ�
	NMHDR* pnmh = (NMHDR*)lParam;
	if (pnmh->hwndFrom == TabCtrl_GetToolTips(hwndTab)) {
		switch (pnmh->code) {
		//case TTN_NEEDTEXT:
		case TTN_GETDISPINFO:
			// �c�[���`�b�v�\������ݒ肷��
			TCITEM	tcitem;
			tcitem.mask   = TCIF_PARAM;
			tcitem.lParam = (LPARAM)NULL;
			if (TabCtrl_GetItem(hwndTab, pnmh->idFrom, &tcitem)) {
				EditNode* pEditNode;
				pEditNode = AppNodeManager::getInstance().GetEditNode((HWND)tcitem.lParam);
				GetTabName(pEditNode, true, false, szTextTip, _countof(szTextTip));
				((NMTTDISPINFO*)pnmh)->lpszText = szTextTip;	// NMTTDISPINFO::szText[80]�ł͒Z��
				((NMTTDISPINFO*)pnmh)->hinst = NULL;
			}
			return 0L;
		}
	}
	return 0L;
}

void TabWnd::TabWindowNotify(WPARAM wParam, LPARAM lParam)
{
	if (!hwndTab) {
		return;
	}

	bool	bFlag = false;	// �O�񉽂��^�u���Ȃ��������H
	int		nCount;
	int		nIndex;
	HWND	hwndUpDown;
	DWORD	nScrollPos;

	BreakDrag();	// 2006.01.28 ryoji �h���b�O��Ԃ���������(�֐���)

	nCount = TabCtrl_GetItemCount(hwndTab);
	if (nCount <= 0) {
		bFlag = true;
		// �ŏ��̂Ƃ��͂��łɑ��݂���E�B���h�E�̏����o�^����K�v������B
		// �N�����ATabWnd::Open()����Refresh()�ł͂܂��O���[�v����O�̂��ߊ��ɕʃE�B���h�E�������Ă��^�u�͋�
		if (wParam == (WPARAM)TabWndNotifyType::Add)
			Refresh();	// ������TWNT_ADD�����Ŏ����ȊO�̃E�B���h�E���B��
	}

	switch ((TabWndNotifyType)wParam) {
	case TabWndNotifyType::Add:	// �E�B���h�E�o�^
		nIndex = FindTabIndexByHWND((HWND)lParam);
		if (nIndex == -1) {
			TCITEM	tcitem;
			TCHAR	szName[1024];
			_tcscpy_s(szName, LS(STR_NO_TITLE1));
			tcitem.mask    = TCIF_TEXT | TCIF_PARAM;
			tcitem.pszText = szName;
			tcitem.lParam  = (LPARAM)lParam;
			// 2006.01.28 ryoji �^�u�ɃA�C�R���C���[�W��ǉ�����
			tcitem.mask |= TCIF_IMAGE;
			tcitem.iImage = GetImageIndex(NULL);
			TabCtrl_InsertItem(hwndTab, nCount, &tcitem);
			nIndex = nCount;
		}

		if (AppNodeManager::getInstance().GetEditNode(GetParentHwnd())->IsTopInGroup()) {
			// �����Ȃ�A�N�e�B�u��
			if (!::IsWindowVisible(GetParentHwnd())) {
				ShowHideWindow(GetParentHwnd(), TRUE);
				// �����ɗ����Ƃ������Ƃ͂��łɃA�N�e�B�u
				// �R�}���h���s���̃A�E�g�v�b�g�Ŗ�肪����̂ŃA�N�e�B�u�ɂ���
			}
			TabCtrl_SetCurSel(hwndTab, nIndex);
			// �����ȊO���B��
			HideOtherWindows(GetParentHwnd());
		}
		break;

	case TabWndNotifyType::Delete:	// �E�B���h�E�폜
		nIndex = FindTabIndexByHWND((HWND)lParam);
		if (nIndex != -1) {
			if (AppNodeManager::getInstance().GetEditNode(GetParentHwnd())->IsTopInGroup()) {
				if (!::IsWindowVisible(GetParentHwnd())) {
					ShowHideWindow(GetParentHwnd(), TRUE);
					ForceActiveWindow(GetParentHwnd());
				}
			}
			TabCtrl_DeleteItem(hwndTab, nIndex);

			// 2005.09.01 ryoji �X�N���[���ʒu����
			// �i�E�[�̂ق��̃^�u�A�C�e�����폜�����Ƃ��A�X�N���[���\�Ȃ̂ɉE�ɗ]�����ł��邱�Ƃւ̑΍�j
			hwndUpDown = ::FindWindowEx(hwndTab, NULL, UPDOWN_CLASS, 0);	// �^�u���� Up-Down �R���g���[��
			if (hwndUpDown && ::IsWindowVisible(hwndUpDown)) {	// 2007.09.24 ryoji hwndUpDown���̏����ǉ�
				nScrollPos = LOWORD(UpDown_GetPos(hwndUpDown));

				// ���݈ʒu nScrollPos �Ɖ�ʕ\���Ƃ���v������
				::SendMessage(hwndTab, WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, LOWORD(nScrollPos)), (LPARAM)NULL);	// �ݒ�ʒu�Ƀ^�u���X�N���[��
			}
		}
		break;

	case TabWndNotifyType::Reorder:	// �E�B���h�E�����ύX
		nIndex = FindTabIndexByHWND((HWND)lParam);
		if (nIndex != -1) {
			if (AppNodeManager::getInstance().GetEditNode(GetParentHwnd())->IsTopInGroup()) {
				// �����Ȃ�A�N�e�B�u��
				if (!::IsWindowVisible(GetParentHwnd())) {
					ShowHideWindow(GetParentHwnd(), TRUE);
				}
				// �����ɗ����Ƃ������Ƃ͂��łɃA�N�e�B�u

				// ���^�u�A�C�e���������I�ɉ��ʒu�ɂ��邽�߂ɁA
				// ���^�u�A�C�e���I��O�Ɉꎞ�I�ɉ�ʍ��[�̃^�u�A�C�e����I������
				hwndUpDown = ::FindWindowEx(hwndTab, NULL, UPDOWN_CLASS, 0);	// �^�u���� Up-Down �R���g���[��
				nScrollPos = (hwndUpDown && ::IsWindowVisible(hwndUpDown))? LOWORD(UpDown_GetPos(hwndUpDown)): 0;	// 2007.09.24 ryoji hwndUpDown���̏����ǉ�
				TabCtrl_SetCurSel(hwndTab, nScrollPos);
				TabCtrl_SetCurSel(hwndTab, nIndex);

				// �����ȊO���B��
				// �i�A���ؑ֎��� TWNT_ORDER ����ʔ����E�������āH��ʂ����ׂď����Ă��܂����肷��̂�h���j
				HideOtherWindows(GetParentHwnd());
			}
		}else {
			// �w��̃E�B���h�E���Ȃ��̂ōĕ\��
			if (!AppNodeManager::IsSameGroup(GetParentHwnd(), (HWND)lParam))
				Refresh();
		}
		break;

	case TabWndNotifyType::Rename:	// �t�@�C�����ύX
		nIndex = FindTabIndexByHWND((HWND)lParam);
		if (nIndex != -1) {
			TCITEM tcitem;
			RecentEditNode	cRecentEditNode;
			TCHAR szName[1024];
			//	Jun. 19, 2004 genta
			EditNode* p = AppNodeManager::getInstance().GetEditNode((HWND)lParam);
			GetTabName(p, false, true, szName, _countof(szName));

			tcitem.mask = TCIF_TEXT | TCIF_IMAGE;
			TCHAR szNameOld[1024];
			tcitem.pszText = szNameOld;
			tcitem.cchTextMax = _countof(szNameOld);
			TabCtrl_GetItem( hwndTab, nIndex, &tcitem );
			if (auto_strcmp( szNameOld, szName ) != 0
				|| tcitem.iImage != GetImageIndex( p )
			) {
				tcitem.mask    = TCIF_TEXT | TCIF_PARAM;
				tcitem.pszText = szName;
				tcitem.lParam  = lParam;
	
				// 2006.01.28 ryoji �^�u�̃A�C�R���C���[�W��ύX����
				tcitem.mask |= TCIF_IMAGE;
				tcitem.iImage = GetImageIndex(p);
				TabCtrl_SetItem(hwndTab, nIndex, &tcitem);
			}
		}else {
			// �w��̃E�B���h�E���Ȃ��̂ōĕ\��
			if (!AppNodeManager::IsSameGroup(GetParentHwnd(), (HWND)lParam))
				Refresh();
		}
		break;

	case TabWndNotifyType::Refresh:	// �ĕ\��
		Refresh(lParam != 0);
		break;

	// Start 2004.07.14 Kazika �ǉ�
	// �^�u���[�h�L���ɂȂ����ꍇ�A�܂Ƃ߂��鑤�̃E�B���h�E�͉B���
	case TabWndNotifyType::Enable:
		Refresh();
		if (AppNodeManager::getInstance().GetEditNode(GetParentHwnd())->IsTopInGroup()) {
			if (!::IsWindowVisible(GetParentHwnd())) {
				// �\����ԂƂ���(�t�H�A�O���E���h�ɂ͂��Ȃ�)
				TabWnd_ActivateFrameWindow(GetParentHwnd(), false);
			}
			// �����ȊO���B��
			HideOtherWindows(GetParentHwnd());
		}
		break;
	// End 2004.07.14 Kazika

	// Start 2004.08.27 Kazika �ǉ�
	// �^�u���[�h�����ɂȂ����ꍇ�A�B��Ă����E�B���h�E�͕\����ԂƂȂ�
	case TabWndNotifyType::Disable:
		Refresh();
		if (!::IsWindowVisible(GetParentHwnd())) {
			// �\����ԂƂ���(�t�H�A�O���E���h�ɂ͂��Ȃ�)
			TabWnd_ActivateFrameWindow(GetParentHwnd(), false);
		}
		break;
	// End 2004.08.27 Kazika

	case TabWndNotifyType::Adjust:	// �E�B���h�E�ʒu���킹	// 2007.04.03 ryoji
		AdjustWindowPlacement();
		return;

	default:
		break;
	}

	// �^�u�̕\���E��\����؂肩����B
	nCount = TabCtrl_GetItemCount(hwndTab);
	if (nCount <= 0) {
		::ShowWindow(hwndTab, SW_HIDE);
	}else {
		if (bFlag) {
			::ShowWindow(hwndTab, SW_SHOW);
		}
	}

//	LayoutTab();	// 2006.01.28 ryoji �^�u�̃��C�A�E�g��������

	// �X�V
//	::InvalidateRect( hwndTab, NULL, FALSE );
//	::InvalidateRect( GetHwnd(), NULL, FALSE );		// 2006.10.21 ryoji �^�u���{�^���ĕ`��̂��߂ɒǉ�

	return;
}

/*! �w��̃E�B���h�E�n���h���������^�u�ʒu��T�� */
int TabWnd::FindTabIndexByHWND(HWND hWnd)
{
	if (!hwndTab) {
		return -1;
	}
	TCITEM tcitem;
	int nCount = TabCtrl_GetItemCount(hwndTab);
	for (int i=0; i<nCount; ++i) {
		tcitem.mask   = TCIF_PARAM;
		tcitem.lParam = (LPARAM)0;
		TabCtrl_GetItem(hwndTab, i, &tcitem);
		
		if ((HWND)tcitem.lParam == hWnd) return i;
	}

	return -1;
}

/*! �^�u���X�g���ĕ\������

	@date 2004.06.19 genta &���܂܂�Ă���t�@�C�������������\������Ȃ�
	@date 2006.02.06 ryoji �I���^�u���w�肷��HWND��������т��̏����͕s�v�Ȃ̂ō폜�i���E�B���h�E���펞�I���j
	@date 2006.06.24 ryoji �X�N���[�����Ȃ��ōX�V������@��ύX
*/
void TabWnd::Refresh(bool bEnsureVisible/* = true*/, bool bRebuild/* = false*/)
{
	TCITEM		tcitem;
	EditNode*	pEditNode;
	int			nGroup = 0;
	int			nTab;
	int			nSel = 0;
	int			nCurTab;
	int			nCurSel;
	int			j;

	if (!hwndTab) {
		return;
	}

	pEditNode = nullptr;
	size_t nCount = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNode, true);

	// ���E�B���h�E�̃O���[�v�ԍ��𒲂ׂ�
	size_t i;
	for (i=0; i<nCount; ++i) {
		auto& node = pEditNode[i];
		if (node.hWnd == GetParentHwnd()) {
			nGroup = node.nGroup;
			break;
		}
	}

	if (i >= nCount) {
		// ������Ȃ������̂őS�^�u���폜
		TabCtrl_DeleteAllItems(hwndTab);
	}else {
		::SendMessage(hwndTab, WM_SETREDRAW, (WPARAM)FALSE, (LPARAM)0);	// 2005.09.01 ryoji �ĕ`��֎~

		if (bRebuild) {
			TabCtrl_DeleteAllItems(hwndTab);	// �쐬���Ȃ���
		}

		// �쐬����^�u���ƑI����Ԃɂ���^�u�ʒu�i���E�B���h�E�̈ʒu�j�𒲂ׂ�
		for (i=0, j=0; i<nCount; ++i) {
			auto& node = pEditNode[i];
			if (node.nGroup != nGroup) {
				continue;
			}
			if (node.bClosing) {	// ���̂��Ƃ����ɕ���E�B���h�E�Ȃ̂Ń^�u�\�����Ȃ�
				continue;
			}
			if (node.hWnd == GetParentHwnd()) {
				nSel = j;	// �I����Ԃɂ���^�u�ʒu
			}
			++j;
		}
		nTab = j;	// �쐬����^�u��

		// �^�u��������΂P�쐬���đI����Ԃɂ���i���E�B���h�E�̃^�u�p�j
		TCHAR szName[2048];
		szName[0] = 0;
		tcitem.mask    = TCIF_TEXT | TCIF_PARAM;
		tcitem.pszText = szName;
		tcitem.lParam  = (LPARAM)GetParentHwnd();
		if (TabCtrl_GetItemCount(hwndTab) == 0) {
			TabCtrl_InsertItem(hwndTab, 0, &tcitem);
			TabCtrl_SetCurSel(hwndTab, 0);
		}

		// �I���^�u�����O�̉ߕs���𒲐�����
		// �i�I���^�u�̒��O�ʒu�ւ̒ǉ��^�폜���J��Ԃ����ƂŃX�N���[��������ጸ�j
		nCurSel = TabCtrl_GetCurSel(hwndTab);	// ���݂̑I���^�u�ʒu
		if (nCurSel > nSel) {
			for (i=0; i<nCurSel-nSel; ++i) {
				TabCtrl_DeleteItem(hwndTab, nCurSel - 1 - i);	// �]�����폜
			}
		}else {
			for (i=0; i<nSel-nCurSel; ++i) {
				TabCtrl_InsertItem(hwndTab, nCurSel + i, &tcitem);	// �s����ǉ�
			}
		}

		// �I���^�u������̉ߕs���𒲐�����
		nCurTab = TabCtrl_GetItemCount(hwndTab);	// ���݂̃^�u��
		if (nCurTab > nTab) {
			for (i=0; i<nCurTab-nTab; ++i) {
				TabCtrl_DeleteItem(hwndTab, nSel + 1);	// �]�����폜
			}
		}else {
			for (i=0; i<nTab-nCurTab; ++i) {
				TabCtrl_InsertItem(hwndTab, nSel + 1, &tcitem);	// �s����ǉ�
			}
		}

		// �쐬�����^�u�Ɋe�E�B���h�E����ݒ肷��
		for (i=0, j=0; i<nCount; ++i) {
			auto& node = pEditNode[i];
			if (node.nGroup != nGroup) {
				continue;
			}
			if (node.bClosing) {	// ���̂��Ƃ����ɕ���E�B���h�E�Ȃ̂Ń^�u�\�����Ȃ�
				continue;
			}

			GetTabName(&node, false, true, szName, _countof(szName));

			tcitem.mask    = TCIF_TEXT | TCIF_PARAM;
			tcitem.pszText = szName;
			tcitem.lParam  = (LPARAM)node.hWnd;

			// 2006.01.28 ryoji �^�u�ɃA�C�R����ǉ�����
			tcitem.mask |= TCIF_IMAGE;
			tcitem.iImage = GetImageIndex(&node);

			TabCtrl_SetItem(hwndTab, j, &tcitem);
			++j;
		}

		::SendMessage(hwndTab, WM_SETREDRAW, (WPARAM)TRUE, (LPARAM)0);	// 2005.09.01 ryoji �ĕ`�拖��

		// �I���^�u�����ʒu�ɂ���
		if (bEnsureVisible) {
			// TabCtrl_SetCurSel() ���g���Ɠ����^�u�̂Ƃ��ɑI���^�u�����[�̂ق��Ɋ���Ă��܂�
//			TabCtrl_SetCurSel(hwndTab, 0);
//			TabCtrl_SetCurSel(hwndTab, nSel);
			::PostMessage(hwndTab, TCM_SETCURSEL, 0, 0);
			::PostMessage(hwndTab, TCM_SETCURSEL, nSel, 0);
		}
	}

	if (pEditNode) {
		delete[] pEditNode;
	}
	
	return;
}


/*!	�ҏW�E�B���h�E�̈ʒu���킹

	@author ryoji
	@date 2007.04.03 �V�K�쐬
*/
void TabWnd::AdjustWindowPlacement(void)
{
	// �^�u�܂Ƃߕ\���̏ꍇ�͕ҏW�E�B���h�E�̕\���ʒu�𕜌�����
	auto& csTabBar = pShareData->common.tabBar;
	if (csTabBar.bDispTabWnd && !csTabBar.bDispTabWndMultiWin) {
		HWND hwnd = GetParentHwnd();	// ���g�̕ҏW�E�B���h�E
		WINDOWPLACEMENT wp;
		if (!::IsWindowVisible(hwnd)) {	// ��������Ƃ����������p��
			// �Ȃ�ׂ���ʂ���O�ɏo�����ɉ�������
			// Note. ��A�N�e�B�u�X���b�h������s����̂ł���΃A�N�e�B�u���w��ł���O�ɂ͏o�Ȃ�
			// Note. SW_xxxxx �̒��ɂ́u�A�N�e�B�u�������̍ő剻�v�w��͑��݂��Ȃ�
			// Note. �s���̏�Ԃ��炢���Ȃ��O�ɏo�Ă��܂��Ǝ��̂悤�Ȍ��ۂ��N����
			//  �E��ʕ`�悳���ہA�N���C�A���g�̈�S�̂��ꎞ�I�ɐ^�����ɂȂ�iVista Aero�j
			//  �E�ő剻�ؑցiSW_SHOWMAXIMIZED�j�̍ہA�ȑO�ɒʏ�\����������ʂ̃X�e�[�^�X�o�[��t�@���N�V�����L�[���ꎞ�I�ɒʏ�T�C�Y�ŕ\�������

			// �E�B���h�E��w��ɔz�u����
			// Note. WS_EX_TOPMOST �ɂ��Ă� hwndInsertAfter �E�B���h�E�̏�Ԃ������p�����
			EditNode* pEditNode;
			pEditNode = AppNodeManager::getInstance().GetEditNode(hwnd)->GetGroup().GetTopEditNode();
			if (!pEditNode) {
				::ShowWindow(hwnd, SW_SHOWNA);
				return;
			}
			HWND hwndInsertAfter = pEditNode->hWnd;
			wp.length = sizeof(wp);
			::GetWindowPlacement(hwndInsertAfter, &wp);
			if (wp.showCmd == SW_SHOWMINIMIZED)
				wp.showCmd = pEditNode->showCmdRestore;
			::SetWindowPos(hwnd, hwndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			SetCarmWindowPlacement(hwnd, &wp);	// �ʒu�𕜌�����
			::UpdateWindow(hwnd);	// �����`��
		}
	}
}

/*!	�A�N�e�B�u���̏��Ȃ� SetWindowPlacement() �����s����

	@author ryoji
	@date 2007.11.30 �V�K�쐬
*/
int TabWnd::SetCarmWindowPlacement(HWND hwnd, const WINDOWPLACEMENT* pWndpl)
{
	WINDOWPLACEMENT wp = *pWndpl;
	if (wp.showCmd == SW_SHOWMAXIMIZED && ::IsZoomed(hwnd)) {
		WINDOWPLACEMENT wpCur;
		wpCur.length = sizeof(WINDOWPLACEMENT);
		::GetWindowPlacement(hwnd, &wpCur);
		if (!::EqualRect(&wp.rcNormalPosition, &wpCur.rcNormalPosition)) {
			// �E�B���h�E�̒ʏ�T�C�Y���ړI�̃T�C�Y�ƈ���Ă���Ƃ��͈�U�ʏ�T�C�Y�ŕ\�����Ă���ő剻����
			// Note. �}���`���j�^�ňȑO�ɕʃ��j�^�ōő剻����Ă�����ʂ͈�U�ʏ�T�C�Y�ɖ߂��Ă����Ȃ��ƌ��̕ʃ��j�^���ɕ\������Ă��܂�
			// �i�{���͂����͕\�����j�^���ς��Ƃ���������OK�����ǁAGetMonitorWorkRect()��SetWindowPlacement�ł͍��W�n��������ƈႤ�̂Łj
			wp.showCmd = SW_SHOWNOACTIVATE;
			::SetWindowPlacement(hwnd, &wp);
			wp.showCmd = SW_SHOWMAXIMIZED;
		}else {
			wp.showCmd = SW_SHOWNA;	// ���̂܂܍ő�\��
		}
	}else if (wp.showCmd != SW_SHOWMAXIMIZED) {
		wp.showCmd = SW_SHOWNOACTIVATE;
	}
	::SetWindowPlacement(hwnd, &wp);
	return wp.showCmd;
}

void TabWnd::ShowHideWindow(HWND hwnd, BOOL bDisp)
{
	if (!hwnd) {
		return;
	}
	auto& csTabBar = pShareData->common.tabBar;
	if (bDisp) {
		if (csTabBar.bDispTabWnd && !csTabBar.bDispTabWndMultiWin) {
			if (pShareData->flags.bEditWndChanging) {
				return;	// �ؑւ̍Œ�(busy)�͗v���𖳎�����
			}
			pShareData->flags.bEditWndChanging = true;	// �ҏW�E�B���h�E�ؑ֒�ON	2007.04.03 ryoji

			// �ΏۃE�B���h�E�̃X���b�h�Ɉʒu���킹���˗�����	// 2007.04.03 ryoji
			DWORD_PTR dwResult;
			::SendMessageTimeout(
				hwnd,
				MYWM_TAB_WINDOW_NOTIFY,
				(WPARAM)TabWndNotifyType::Adjust,
				(LPARAM)NULL,
				SMTO_ABORTIFHUNG | SMTO_BLOCK, 10000, &dwResult);
		}
		TabWnd_ActivateFrameWindow(hwnd);
		pShareData->flags.bEditWndChanging = false;	// �ҏW�E�B���h�E�ؑ֒�OFF	2007.04.03 ryoji
	}else {
		if (csTabBar.bDispTabWnd && !csTabBar.bDispTabWndMultiWin) {
			::ShowWindow(hwnd, SW_HIDE);
		}
	}
	return;
}

/*!	���̕ҏW�E�B���h�E���B��

	@param hwndExclude [in] ��\�������珜�O����E�B���h�E

	@author ryoji
	@date 2007.05.17 �V�K�쐬
*/
void TabWnd::HideOtherWindows(HWND hwndExclude)
{
	auto& csTabBar = pShareData->common.tabBar;
	if (csTabBar.bDispTabWnd && !csTabBar.bDispTabWndMultiWin) {
		HWND hwnd;
		for (size_t i=0; i<pShareData->nodes.nEditArrNum; ++i) {
			hwnd = pShareData->nodes.pEditArr[i].hWnd;
			if (IsSakuraMainWindow(hwnd)) {
				if (!AppNodeManager::IsSameGroup(hwndExclude, hwnd))
					continue;
				if (hwnd != hwndExclude && ::IsWindowVisible(hwnd)) {
					::ShowWindow(hwnd, SW_HIDE);
				}
			}
		}
	}
}

// �E�B���h�E�������I�ɑO�ʂɎ����Ă���
void TabWnd::ForceActiveWindow(HWND hwnd)
{
	int nId2 = ::GetWindowThreadProcessId(::GetForegroundWindow(), NULL);
	int nId1 = ::GetWindowThreadProcessId(hwnd, NULL);

	::AttachThreadInput(nId1, nId2, TRUE);

	DWORD dwTime;
	::SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &dwTime, 0);
	::SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID)0, 0);

	// �E�B���h�E���t�H�A�O���E���h�ɂ���
	::SetForegroundWindow(hwnd);
	::BringWindowToTop(hwnd);

	::SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID)(INT_PTR)dwTime, 0);

	::AttachThreadInput(nId1, nId2, FALSE);
}

/*!	�A�N�e�B�u�ɂ���

	@param hwnd [in] �ΏۃE�B���h�E�̃E�B���h�E�n���h��
	@param bForeground [in] true: active and forground / false: active

	@date 2004.08.27 Kazika ����bForeground�ǉ��BbForeground��false�̏ꍇ�̓E�B���h�E���t�H�A�O���E���h�ɂ��Ȃ��B
	@date 2005.11.05 ryoji Grep�_�C�A���O���t�H�[�J�X������Ȃ��悤�ɂ��邽�߁C
		�ΏۃE�B���h�E�̃v���Z�X�����Ƀt�H�A�O���E���h�Ȃ牽�����Ȃ��悤�ɂ���D
	@date 2007.11.07 ryoji �Ώۂ�disable�̂Ƃ��͍ŋ߂̃|�b�v�A�b�v���t�H�A�O���E���h������D
		�i���[�_���_�C�A���O�⃁�b�Z�[�W�{�b�N�X��\�����Ă���悤�ȂƂ��j
*/
void TabWnd::TabWnd_ActivateFrameWindow(HWND hwnd, bool bForeground)
{
	if (bForeground) {
		// 2005.11.05 ryoji �ΏۃE�B���h�E�̃v���Z�X�����Ƀt�H�A�O���E���h�Ȃ�ؑւ��ς݂Ȃ̂ŉ������Ȃ��ł���
		DWORD dwPid1, dwPid2;
		::GetWindowThreadProcessId(hwnd, &dwPid1);
		::GetWindowThreadProcessId(::GetForegroundWindow(), &dwPid2);
		if (dwPid1 == dwPid2) {
			return;
		}

		// �Ώۂ�disable�̂Ƃ��͍ŋ߂̃|�b�v�A�b�v���t�H�A�O���E���h������
		HWND hwndActivate = ::IsWindowEnabled(hwnd)? hwnd: ::GetLastActivePopup(hwnd);
		if (::IsIconic(hwnd)) {
			::ShowWindow(hwnd, SW_RESTORE);	// Nov. 7. 2003 MIK �A�C�R�����͌��̃T�C�Y�ɖ߂�
		}else if (::IsZoomed(hwnd)) {
			::ShowWindow(hwnd, SW_MAXIMIZE);
		}else {
			::ShowWindow(hwnd, SW_SHOW);
		}
		::SetForegroundWindow(hwndActivate);
		::BringWindowToTop(hwndActivate);
	}else {
		// 2005.09.01 ryoji ::ShowWindow(hwnd, SW_SHOWNA) ���Ɣ�\������\���ɐ؂�ւ��Ƃ��� Z-order �����������Ȃ邱�Ƃ�����̂� ::SetWindowPos �ɕύX
		::SetWindowPos(hwnd, NULL, 0,0,0,0,
						SWP_SHOWWINDOW | SWP_NOACTIVATE
						| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
	}

	return;
}

/*! �^�u�̃��C�A�E�g��������
	@date 2006.01.28 ryoji �V�K�쐬
*/
void TabWnd::LayoutTab(void)
{
	auto& csTabBar = pShareData->common.tabBar;
	// �t�H���g��؂�ւ��� 2011.12.01 Moca
	bool bChgFont = (memcmp(&lf, &csTabBar.lf, sizeof(lf)) != 0);
	int nSizeBoxWidth = 0;
	if (hwndSizeBox) {
		nSizeBoxWidth = ::GetSystemMetrics( SM_CXVSCROLL );
	}
	if (bChgFont) {
		HFONT hFontOld = hFont;
		lf = csTabBar.lf;
		hFont = ::CreateFontIndirect(&lf);
		::SendMessage(hwndTab, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
		::DeleteObject(hFontOld);
		// �E�B���h�E�̍������C��
	}

	// �A�C�R���̕\����؂�ւ���
	bool bDispTabIcon = csTabBar.bDispTabIcon;
	HIMAGELIST hImg = TabCtrl_GetImageList(hwndTab);
	if (!hImg && bDispTabIcon) {
		if (InitImageList())
			Refresh(true, true);
	}else if (hImg && !bDispTabIcon) {
		InitImageList();
	}

	// ���݂̃E�B���h�E�X�^�C�����擾����
	UINT lStyle = (UINT)::GetWindowLongPtr(hwndTab, GWL_STYLE);
	UINT lStyleOld = lStyle;

	// �^�u�̃A�C�e�����̓�����؂�ւ���
	bool bSameTabWidth = csTabBar.bSameTabWidth;
	if (bSameTabWidth && !(lStyle & TCS_FIXEDWIDTH)) {
		lStyle |= (TCS_FIXEDWIDTH | TCS_FORCELABELLEFT);
	}else if (!bSameTabWidth && (lStyle & TCS_FIXEDWIDTH)) {
		lStyle &= ~(TCS_FIXEDWIDTH | TCS_FORCELABELLEFT);
	}

	// �I�[�i�[�h���[��Ԃ����ʐݒ�ɒǐ�������
	DispTabCloseType dispTabClose = csTabBar.dispTabClose;
	bool bOwnerDraw = (dispTabClose != DispTabCloseType::No);
	if (bOwnerDraw && !(lStyle & TCS_OWNERDRAWFIXED)) {
		lStyle |= TCS_OWNERDRAWFIXED;
	}else if (!bOwnerDraw && (lStyle & TCS_OWNERDRAWFIXED)) {
		lStyle &= ~TCS_OWNERDRAWFIXED;
	}

	// �^�u�̃A�C�e���T�C�Y�𒲐�����i�����̂Ƃ��̃T�C�Y��t�H���g�ؑ֎��̍��������j
	// �� ��ʂ̂������̊����\�ɂ��قǉe���͖��������Ȃ̂ŏ������i�炸���� TabCtrl_SetItemSize() �����s����
	RECT rcTab;
	int nCount;
	int cx;
	::GetClientRect(hwndTab, &rcTab);
	nCount = TabCtrl_GetItemCount(hwndTab);
	if (0 < nCount) {
		cx = (rcTab.right - rcTab.left - 8) / nCount;
		int min = MIN_TABITEM_WIDTH;
		if (csTabBar.bTabMultiLine) {
			min = MIN_TABITEM_WIDTH_MULTI;
		}
		if (MAX_TABITEM_WIDTH < cx)
			cx = MAX_TABITEM_WIDTH;
		else if (MIN_TABITEM_WIDTH > cx)
			cx = MIN_TABITEM_WIDTH;
		TabCtrl_SetItemSize(hwndTab, cx, TAB_ITEM_HEIGHT);
	}

	// �^�u�]���ݒ�i�u����{�^���v��u�A�C�R���v�̐ݒ�ؑ֎��̗]���ؑցj
	// �� ��ʂ̂������̊����\�ɂ��قǉe���͖��������Ȃ̂ŏ������i�炸���� TabCtrl_SetPadding() �����s����
	cx = 6;
	if (dispTabClose == DispTabCloseType::Always) {
		// ����{�^���̕������p�f�B���O��ǉ����ĉ������L����
		int nWidth = rcBtnBase.right - rcBtnBase.left;
		cx += bDispTabIcon? (nWidth + 2)/3: (nWidth + 1)/2;	// ������ۂ�����: �{�^������ 1/3�i�A�C�R���L�j or 1/2�i�A�C�R�����j
	}
	TabCtrl_SetPadding(hwndTab, DpiScaleX(cx), DpiScaleY(3));

	// �V�����E�B���h�E�X�^�C����K�p����
	// �� TabCtrl_SetPadding() �̌�ł��Ȃ��Ɛݒ�ύX�̒���ɃA�C�R����e�L�X�g�̕`��ʒu�������ꍇ������
	// �i��jVer 2.1.0 �ł͉��L�̐ݒ�ύX������ƒ���ɕ`��ʒu������Ă���
	//           �ύX�O�F[����]OFF�A[����{�^��]ON
	//           �ύX��F[����]ON�A[����{�^��]OFF
	if (lStyle != lStyleOld) {
		::SetWindowLongPtr(hwndTab, GWL_STYLE, lStyle);
		if (bOwnerDraw) {
			// �I�[�i�[�h���[�ɐ؂�ւ���Ƃ��͍ēx�E�B���h�E�X�^�C����ݒ肷��B
			// �� �ݒ肪�P�x�������ƁA��A�N�e�B�u�^�u��Ń}�E�X�I�[�o�[���Ă��^�u���n�C���C�g�\���ɂȂ炸�A
			//    �����̃^�C�~���O�� ::SetWindowLongPtr() ���Ď��s�����ƈȌ�̓n�C���C�g�\�������B
			//    �iVista/7/8 �œ��l�̏Ǐ���m�F�j
			::SetWindowLongPtr(hwndTab, GWL_STYLE, lStyle);
		}
	}
	RECT rcWnd;
	GetWindowRect(&rcWnd);

	int nHeight = TAB_WINDOW_HEIGHT;
	::GetWindowRect(hwndTab, &rcTab);
	if (csTabBar.bTabMultiLine
		&& TabCtrl_GetItemCount(hwndTab)
	) {
		// ���m�ɍĔz�u�i���i�^�u�ł͒i�����ς�邱�Ƃ�����̂ŕK�{�j
		RECT rcDisp = rcTab;
		rcDisp.left = TAB_MARGIN_LEFT;
		rcDisp.right = rcTab.left + (rcWnd.right - rcWnd.left) - (TAB_MARGIN_LEFT + TAB_MARGIN_RIGHT + nSizeBoxWidth);
		TabCtrl_AdjustRect(hwndTab, FALSE, &rcDisp);
		nHeight = (rcDisp.top - rcTab.top - 2) + TAB_MARGIN_TOP;
	}
	::SetWindowPos(
		GetHwnd(),
		NULL,
		0,
		0,
		rcWnd.right - rcWnd.left,
		nHeight,
		SWP_NOMOVE | SWP_NOZORDER
	);
	int nWidth = (rcWnd.right - rcWnd.left) - (TAB_MARGIN_LEFT + TAB_MARGIN_RIGHT + nSizeBoxWidth);
	if ((nWidth != rcTab.right - rcTab.left) || (nHeight != rcTab.bottom - rcTab.top)) {
		::MoveWindow(
			hwndTab,
			TAB_MARGIN_LEFT,
			TAB_MARGIN_TOP,
			nWidth,
			nHeight,
			TRUE
		);
	}
}

/*! �C���[�W���X�g�̏���������
	@date 2006.02.22 ryoji �V�K�쐬
*/
HIMAGELIST TabWnd::InitImageList(void)
{
	SHFILEINFO sfi;
	HIMAGELIST hImlSys;
	HIMAGELIST hImlNew;

	hImlNew = NULL;
	if (pShareData->common.tabBar.bDispTabIcon) {
		// �V�X�e���C���[�W���X�g���擾����
		// ���F������ɍ����ւ��ė��p����A�C�R���ɂ͎��O�ɃA�N�Z�X���Ă����Ȃ��ƃC���[�W������Ȃ�
		//     �����ł́u�t�H���_������A�C�R���v�A�u�t�H���_���J�����A�C�R���v�������ւ��p�Ƃ��ė��p
		//     WinNT4.0 �ł� SHGetFileInfo() �̑������ɓ������w�肷��Ɠ����C���f�b�N�X��Ԃ��Ă��邱�Ƃ�����H

		hImlSys = (HIMAGELIST)::SHGetFileInfo(
			_T(".0"),
			FILE_ATTRIBUTE_DIRECTORY,
			&sfi,
			sizeof(sfi),
			SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES
		);
		if (!hImlSys) {
			goto l_end;
		}
		iIconApp = sfi.iIcon;

		hImlSys = (HIMAGELIST)::SHGetFileInfo(
			_T(".1"),
			FILE_ATTRIBUTE_DIRECTORY,
			&sfi,
			sizeof(sfi),
			SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES | SHGFI_OPENICON
		);
		if (!hImlSys) {
			goto l_end;
		}
		iIconGrep = sfi.iIcon;

		// �V�X�e���C���[�W���X�g�𕡐�����
		hImlNew = ImageList_Duplicate(hImlSys);
		if (!hImlNew) {
			goto l_end;
		}
		ImageList_SetBkColor(hImlNew, CLR_NONE);

		// �C���[�W���X�g�ɃA�v���P�[�V�����A�C�R���� Grep�A�C�R����o�^����
		// �i���p���Ȃ��A�C�R���ƍ����ւ���j
		hIconApp = GetAppIcon(GetAppInstance(), ICON_DEFAULT_APP, FN_APP_ICON, true);
		ImageList_ReplaceIcon(hImlNew, iIconApp, hIconApp);
		hIconGrep = GetAppIcon(GetAppInstance(), ICON_DEFAULT_GREP, FN_GREP_ICON, true);
		ImageList_ReplaceIcon(hImlNew, iIconGrep, hIconGrep);
	}

l_end:
	// �^�u�ɐV�����A�C�R���C���[�W��ݒ肷��
	TabCtrl_SetImageList(hwndTab, hImlNew);

	// �V�����C���[�W���X�g���L������
	if (hIml) {
		ImageList_Destroy(hIml);
	}
	hIml = hImlNew;

	return hIml;	// �V�����C���[�W���X�g��Ԃ�
}

/*! �C���[�W���X�g�̃C���f�b�N�X�擾����
	@date 2006.01.28 ryoji �V�K�쐬
*/
int TabWnd::GetImageIndex(EditNode* pNode)
{
	SHFILEINFO sfi;
	HIMAGELIST hImlSys;
	HIMAGELIST hImlNew;

	if (!hIml) {
		return -1;	// �C���[�W���X�g���g���Ă��Ȃ�
	}
	if (pNode) {
		if (pNode->szFilePath[0]) {
			// �g���q�����o��
			TCHAR szExt[_MAX_EXT];
			_tsplitpath(pNode->szFilePath, NULL, NULL, NULL, szExt);

			// �g���q�Ɋ֘A�t����ꂽ�A�C�R���C���[�W�̃C���f�b�N�X���擾����
			hImlSys = (HIMAGELIST)::SHGetFileInfo(szExt, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
			if (!hImlSys)
				return -1;
			if (ImageList_GetImageCount(hIml) > sfi.iIcon)
				return sfi.iIcon;	// �C���f�b�N�X��Ԃ�

			// �V�X�e���C���[�W���X�g�𕡐�����
			hImlNew = ImageList_Duplicate(hImlSys);
			if (!hImlNew)
				return -1;
			ImageList_SetBkColor(hImlNew, CLR_NONE);

			// �C���[�W���X�g�ɃA�v���P�[�V�����A�C�R���� Grep�A�C�R����o�^����
			// �i���p���Ȃ��A�C�R���ƍ����ւ���j
			ImageList_ReplaceIcon(hImlNew, iIconApp, hIconApp);
			ImageList_ReplaceIcon(hImlNew, iIconGrep, hIconGrep);

			// �^�u�ɃA�C�R���C���[�W��ݒ肷��
			if (pShareData->common.tabBar.bDispTabIcon)
				TabCtrl_SetImageList(hwndTab, hImlNew);

			// �V�����C���[�W���X�g���L������
			ImageList_Destroy(hIml);
			hIml = hImlNew;

			return sfi.iIcon;	// �C���f�b�N�X��Ԃ�
		}else if (pNode->bIsGrep)
			return iIconGrep;	// grep�A�C�R���̃C���f�b�N�X��Ԃ�
	}

	return iIconApp;	// �A�v���P�[�V�����A�C�R���̃C���f�b�N�X��Ԃ�
}

/*! �C���[�W���X�g�̕�������
	@date 2006.02.17 ryoji �V�K�쐬
*/
HIMAGELIST TabWnd::ImageList_Duplicate(HIMAGELIST himl)
{
	if (!himl)
		return NULL;

	// �{���� ImageList_Duplicate() ������΂�����Ăяo��
	HIMAGELIST hImlNew;
	if (realImageList_Duplicate) {
		hImlNew = realImageList_Duplicate(himl);
		if (hImlNew) {
			return hImlNew;
		}
		realImageList_Duplicate = NULL;	// 2006.06.20 ryoji ���s���͑�֏����ɐ؂�ւ�
	}

	// �{���� ImageList_Duplicate() �̑�֏���
	// �V�����C���[�W���X�g���쐬���ăA�C�R���P�ʂŃR�s�[����
	//�i���̏ꍇ�A���F�A�C�R�����Y��ɂ͕\������Ȃ���������Ȃ��j
	int cxIcon = CX_SMICON;
	int cyIcon = CY_SMICON;
	ImageList_GetIconSize(himl, &cxIcon, &cyIcon);
	hImlNew = ImageList_Create(cxIcon, cyIcon, ILC_COLOR32 | ILC_MASK, 4, 4);
	if (hImlNew) {
		ImageList_SetBkColor(hImlNew, CLR_NONE);
		int nCount = ImageList_GetImageCount(himl);
		for (int i=0; i<nCount; ++i) {
			HICON hIcon = ImageList_GetIcon(himl, i, ILD_TRANSPARENT);
			if (!hIcon) {
				ImageList_Destroy(hImlNew);
				return NULL;
			}
			int iIcon = ImageList_AddIcon(hImlNew, hIcon);
			::DestroyIcon(hIcon);
			if (0 > iIcon) {
				ImageList_Destroy(hImlNew);
				return NULL;
			}
		}
	}
	return hImlNew;
}

/*! �{�^���w�i�`�揈��
	@date 2006.10.21 ryoji �V�K�쐬
*/
void TabWnd::DrawBtnBkgnd(HDC hdc, const LPRECT lprcBtn, BOOL bBtnHilighted)
{
	if (bBtnHilighted) {
		Graphics gr(hdc);
		gr.SetPen(::GetSysColor(COLOR_HIGHLIGHT));
		gr.SetBrushColor(::GetSysColor(COLOR_MENU));
		::Rectangle(gr, lprcBtn->left, lprcBtn->top, lprcBtn->right, lprcBtn->bottom);
	}
}

/*! �ꗗ�{�^���`�揈��
	@date 2006.02.01 ryoji �V�K�쐬
	@date 2006.10.21 ryoji �w�i�`����֐��Ăяo���ɕύX
	@date 2009.10.01 ryoji �`��C���[�W����`�����ɂ����Ă���
*/
void TabWnd::DrawListBtn(Graphics& gr, const LPRECT lprcClient)
{
	static const POINT ptBase[4] = { {4, 8}, {7, 11}, {8, 11}, {11, 8} };	// �`��C���[�W�`��
	POINT pt[4];

	RECT rcBtn;
	GetListBtnRect(lprcClient, &rcBtn);
	DrawBtnBkgnd(gr, &rcBtn, bListBtnHilighted);	// 2006.10.21 ryoji

	// �`��C���[�W����`�����ɂ����Ă���	// 2009.10.01 ryoji
	rcBtn.left = rcBtn.left + ((rcBtn.right - rcBtn.left) - (rcBtnBase.right - rcBtnBase.left)) / 2;
	rcBtn.top = rcBtn.top + ((rcBtn.bottom - rcBtn.top) - (rcBtnBase.bottom - rcBtnBase.top)) / 2;
	rcBtn.right = rcBtn.left + (rcBtnBase.right - rcBtnBase.left);
	rcBtn.bottom = rcBtn.top + (rcBtnBase.bottom - rcBtnBase.left);

	int nIndex = bListBtnHilighted? COLOR_MENUTEXT: COLOR_BTNTEXT;
	gr.SetPen(::GetSysColor(nIndex));
	gr.SetBrushColor(::GetSysColor(nIndex)); //$$ GetSysColorBrush��p���������̂ق��������͗ǂ�
	for (size_t i=0; i<_countof(ptBase); ++i) {
		pt[i].x = ptBase[i].x + rcBtn.left;
		pt[i].y = ptBase[i].y + rcBtn.top;
	}
	::Polygon(gr, pt, _countof(pt));
}

// ����}�[�N�`�揈��
void TabWnd::DrawCloseFigure(Graphics& gr, const RECT& rcBtn)
{
	// [x]�`��C���[�W�`��i����6�{�j
	static const POINT ptBase1[6][2] = {
		{{4, 5}, {12, 13}},
		{{4, 4}, {13, 13}},
		{{5, 4}, {13, 12}},
		{{11, 4}, {3, 12}},
		{{12, 4}, {3, 13}},
		{{12, 5}, {4, 13}}
	};
	POINT pt[2];
	// [x]��`��i����6�{�j
	for (size_t i=0; i<_countof(ptBase1); ++i) {
		pt[0].x = ptBase1[i][0].x + rcBtn.left;
		pt[0].y = ptBase1[i][0].y + rcBtn.top;
		pt[1].x = ptBase1[i][1].x + rcBtn.left;
		pt[1].y = ptBase1[i][1].y + rcBtn.top;
		::MoveToEx(gr, pt[0].x, pt[0].y, NULL);
		::LineTo(gr, pt[1].x, pt[1].y);
	}
}

/*! ����{�^���`�揈��
	@date 2006.10.21 ryoji �V�K�쐬
	@date 2009.10.01 ryoji �`��C���[�W����`�����ɂ����Ă���
	@date 2012.05.12 syat �}�[�N�`�敔�����֐��ɐ؂�o��
*/
void TabWnd::DrawCloseBtn(Graphics& gr, const LPRECT lprcClient)
{
	// [xx]�`��C���[�W�`��i��`10�j
	static const POINT ptBase2[10][2] = {
		{{3, 4}, {5, 6}},
		{{6, 4}, {8, 6}},
		{{4, 6}, {7, 10}},
		{{3, 10}, {5, 12}},
		{{6, 10}, {8, 12}},
		{{9, 4}, {11, 6}},
		{{12, 4}, {14, 6}},
		{{10, 6}, {13, 10}},
		{{9, 10}, {11, 12}},
		{{12, 10}, {14, 12}}
	};

	POINT pt[2];

	RECT rcBtn;
	GetCloseBtnRect(lprcClient, &rcBtn);

	// �{�^���̍����ɃZ�p���[�^��`�悷��	// 2007.02.27 ryoji
	gr.SetPen(::GetSysColor(COLOR_3DSHADOW));
	::MoveToEx(gr, rcBtn.left - DpiScaleX(4), rcBtn.top + 1, NULL);
	::LineTo(gr, rcBtn.left - DpiScaleX(4), rcBtn.bottom - 1);

	DrawBtnBkgnd(gr, &rcBtn, bCloseBtnHilighted);

	// �`��C���[�W����`�����ɂ����Ă���	// 2009.10.01 ryoji
	rcBtn.left = rcBtn.left + ((rcBtn.right - rcBtn.left) - (rcBtnBase.right - rcBtnBase.left)) / 2;
	rcBtn.top = rcBtn.top + ((rcBtn.bottom - rcBtn.top) - (rcBtnBase.bottom - rcBtnBase.top)) / 2;
	rcBtn.right = rcBtn.left + (rcBtnBase.right - rcBtnBase.left);
	rcBtn.bottom = rcBtn.top + (rcBtnBase.bottom - rcBtnBase.left);

	int nIndex = bCloseBtnHilighted? COLOR_MENUTEXT: COLOR_BTNTEXT;
	gr.SetPen(::GetSysColor(nIndex));
	gr.SetBrushColor(::GetSysColor(nIndex));
	auto& csTabBar = pShareData->common.tabBar;
	if (csTabBar.bDispTabWnd &&
		!csTabBar.bDispTabWndMultiWin &&
		!csTabBar.bTab_CloseOneWin			// 2007.02.13 ryoji �����ǉ��i�E�B���h�E�̕���{�^���͑S������j
	) {
		DrawCloseFigure(gr, rcBtn);
	}else {
		// [xx]��`��i��`10�j
		for (size_t i=0; i<_countof(ptBase2); ++i) {
			pt[0].x = ptBase2[i][0].x + rcBtn.left;
			pt[0].y = ptBase2[i][0].y + rcBtn.top;
			pt[1].x = ptBase2[i][1].x + rcBtn.left;
			pt[1].y = ptBase2[i][1].y + rcBtn.top;
			::Rectangle(gr, pt[0].x, pt[0].y, pt[1].x, pt[1].y);
		}
	}
}

/*! �^�u�����{�^���`�揈��
	@date 2012.04.08 syat �V�K�쐬
*/
void TabWnd::DrawTabCloseBtn(Graphics& gr, const LPRECT lprcClient, bool selected, bool bHover)
{
	RECT rcBtn;
	GetTabCloseBtnRect(lprcClient, &rcBtn, selected);

	DrawBtnBkgnd(gr, &rcBtn, bHover);

	// �`��C���[�W����`�����ɂ����Ă���	// 2009.10.01 ryoji
	rcBtn.left = rcBtn.left + ((rcBtn.right - rcBtn.left) - (rcBtnBase.right - rcBtnBase.left)) / 2;
	rcBtn.top = rcBtn.top + ((rcBtn.bottom - rcBtn.top) - (rcBtnBase.bottom - rcBtnBase.top)) / 2 - 1;
	rcBtn.right = rcBtn.left + (rcBtnBase.right - rcBtnBase.left);
	rcBtn.bottom = rcBtn.top + (rcBtnBase.bottom - rcBtnBase.left);

	int nIndex = COLOR_BTNTEXT;
	gr.SetPen(::GetSysColor(nIndex));
	gr.SetBrushColor(::GetSysColor(nIndex));
	DrawCloseFigure(gr, rcBtn);
}

/*! �ꗗ�{�^���̋�`�擾����
	@date 2006.02.01 ryoji �V�K�쐬
*/
void TabWnd::GetListBtnRect(const LPRECT lprcClient, LPRECT lprc)
{
	*lprc = rcBtnBase;
	DpiScaleRect(lprc);	// 2009.10.01 ryoji ��DPI�Ή��X�P�[�����O
	int nSizeBoxWidth = 0;
	if (hwndSizeBox) {
		nSizeBoxWidth = ::GetSystemMetrics( SM_CXVSCROLL );
	}
	::OffsetRect(lprc, lprcClient->right - TAB_MARGIN_RIGHT - nSizeBoxWidth + DpiScaleX(4), lprcClient->top + TAB_MARGIN_TOP + DpiScaleY(2) );
}

/*! ����{�^���̋�`�擾����
	@date 2006.10.21 ryoji �V�K�쐬
*/
void TabWnd::GetCloseBtnRect(const LPRECT lprcClient, LPRECT lprc)
{
	*lprc = rcBtnBase;
	DpiScaleRect(lprc);	// 2009.10.01 ryoji ��DPI�Ή��X�P�[�����O
	int nSizeBoxWidth = 0;
	if (hwndSizeBox) {
		nSizeBoxWidth = ::GetSystemMetrics( SM_CXVSCROLL );
	}
	::OffsetRect(lprc,
		lprcClient->right - TAB_MARGIN_RIGHT - nSizeBoxWidth + DpiScaleX(4) + (DpiScaleX(rcBtnBase.right) - DpiScaleX(rcBtnBase.left)) + DpiScaleX(7),
		lprcClient->top + TAB_MARGIN_TOP + DpiScaleY(2));
}

/*! �^�u�����{�^���̋�`�擾����
	@date 2012.04.08 syat �V�K�쐬
*/
void TabWnd::GetTabCloseBtnRect(const LPRECT lprcTab, LPRECT lprc, bool selected)
{
	*lprc = rcBtnBase;
	DpiScaleRect(lprc);	// 2009.10.01 ryoji ��DPI�Ή��X�P�[�����O
	::OffsetRect(lprc,
		(lprcTab->right + (selected ? 0: -2)) - ((DpiScaleX(rcBtnBase.right) - DpiScaleX(rcBtnBase.left)) + DpiScaleX(2)),
		(lprcTab->top + (selected ? -2: 0)) + DpiScaleY(2)
	);
}


/** �^�u���擾����

	@param[in] EditNode �ҏW�E�B���h�E���
	@param[in] bFull �p�X���ŕ\������
	@param[in] bDupamp &��&&�ɒu��������
	@param[out] pszName �^�u���i�[��
	@param[in] nLen �i�[��ő啶�����i�I�[��null�����܂ށj

	@date 2007.06.28 ryoji �V�K�쐬
*/
void TabWnd::GetTabName(EditNode* pEditNode, bool bFull, bool bDupamp, LPTSTR pszName, size_t nLen)
{
	std::vector<TCHAR> szText(nLen);
	LPTSTR pszText = &szText[0];

	if (!pEditNode) {
		::lstrcpyn(pszText, LS(STR_NO_TITLE1), nLen);
	}else if (!bFull || pEditNode->szFilePath[0] == '\0') {
		if (pEditNode->szTabCaption[0]) {
			::lstrcpyn(pszText, pEditNode->szTabCaption, nLen);
		}else {
			::lstrcpyn(pszText, LS(STR_NO_TITLE1), nLen);
		}
	}else {
		// �t���p�X�����ȈՖ��ɕϊ�����
		HDC hdc = ::GetDC(hwndTab);
		HFONT hFontOld = (HFONT)SelectObject(hdc, hFont);
		FileNameManager::getInstance().GetTransformFileNameFast(
			pEditNode->szFilePath,
			pszText,
			nLen,
			hdc,
			false
		);
		SelectObject(hdc, hFontOld);
		::ReleaseDC(hwndTab, hdc);
	}

	if (bDupamp) {
		// &��&&�ɒu��������
		std::vector<TCHAR> szText_amp(nLen * 2);
		LPTSTR pszText_amp = &szText_amp[0];
		dupamp(pszText, pszText_amp);
		::lstrcpyn(pszName, pszText_amp, nLen);
	}else {
		::lstrcpyn(pszName, pszText, nLen);
	}

}


/**	�^�u�ꗗ�\������

	@param pt [in] �\���ʒu
	@param bSel [in] �\���ؑփ��j���[��ǉ�����
	@param bFull [in] �p�X���ŕ\������ibSel��TRUE�̏ꍇ�͖����j
	@param bOtherGroup [in] ���O���[�v�̃E�B���h�E���\������

	@date 2006.02.01 ryoji �V�K�쐬
	@date 2006.03.23 fon OnListBtnClick����ړ�(�s����//>���ύX��)
	@date 2006.10.31 ryoji ���j���[�̃t���p�X�����ȈՕ\������
	@date 2007.02.28 ryoji �^�u���ꗗ�^�p�X���ꗗ�̕\�������j���[���g�Ő؂�ւ���
	@date 2007.06.28 ryoji �O���[�v���Ή��i���O���[�v�̃E�B���h�E��\������^���Ȃ��j
*/
LRESULT TabWnd::TabListMenu(POINT pt, bool bSel/* = true*/, bool bFull/* = false*/, bool bOtherGroup/* = true*/)
{
	bool bRepeat;
	auto& csTabBar = pShareData->common.tabBar;
	if (bSel) {
		bFull = csTabBar.bTab_ListFull;
	}
	do {
		EditNode* pEditNode;

		// �^�u���j���[�p�̏����擾����
		size_t nCount = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNode, true);
		if (nCount == 0)
			return 0L;

		int nGroup = 0;
		// ���E�B���h�E�̃O���[�v�ԍ��𒲂ׂ�
		int i;
		for (i=0; i<nCount; ++i) {
			auto& node = pEditNode[i];
			if (node.hWnd == GetParentHwnd()) {
				nGroup = node.nGroup;
				break;
			}
		}
		if (nCount <= i) {
			return 0L;
		}

		TABMENU_DATA* pData = new TABMENU_DATA[nCount];	// �^�u���j���[�p�̏��

		// ���O���[�v�̃E�B���h�E�ꗗ�����쐬����
		int nSelfTab = 0;
		if (i < nCount) {
			for (int i=0; i<nCount; ++i) {
				auto& node = pEditNode[i];
				if (node.nGroup != nGroup)
					continue;
				if (node.bClosing)	// ���̂��Ƃ����ɕ���E�B���h�E�Ȃ̂Ń^�u�\�����Ȃ�
					continue;
				GetTabName(&node, bFull, true, pData[nSelfTab].szText, _countof(pData[0].szText));
				pData[nSelfTab].hwnd = node.hWnd;
				pData[nSelfTab].iItem = i;
				pData[nSelfTab].iImage = GetImageIndex(&node);
				++nSelfTab;
			}
			// �\�������Ń\�[�g����
			if (nSelfTab > 0 && csTabBar.bSortTabList)	// 2006.03.23 fon �ύX
				qsort(pData, nSelfTab, sizeof(pData[0]), compTABMENU_DATA);
		}

		// ���O���[�v�̃E�B���h�E�ꗗ�����쐬����
		int nTab = nSelfTab;
		for (int i=0; i<nCount; ++i) {
			auto& node = pEditNode[i];
			if (node.nGroup == nGroup)
				continue;
			if (node.bClosing)	// ���̂��Ƃ����ɕ���E�B���h�E�Ȃ̂Ń^�u�\�����Ȃ�
				continue;
			GetTabName(&node, bFull, true, pData[nTab].szText, _countof(pData[0].szText));
			pData[nTab].hwnd = node.hWnd;
			pData[nTab].iItem = i;
			pData[nTab].iImage = GetImageIndex(&node);
			++nTab;
		}
		// �\�������Ń\�[�g����
		if (nTab > nSelfTab && csTabBar.bSortTabList) {
			qsort(pData + nSelfTab, nTab - nSelfTab, sizeof(pData[0]), compTABMENU_DATA);
		}
		delete[] pEditNode;

		// ���j���[���쐬����
		// 2007.02.28 ryoji �\���ؑւ����j���[�ɒǉ�
		int iMenuSel = -1;
		UINT uFlags = MF_BYPOSITION | (hIml? MF_OWNERDRAW: MF_STRING);
		HMENU hMenu = ::CreatePopupMenu();
		for (int i=0; i<nSelfTab; ++i) {
			::InsertMenu(hMenu, i, uFlags, IDM_SELWINDOW + i, hIml? (LPCTSTR)&pData[i]: pData[i].szText);
			if (pData[i].hwnd == GetParentHwnd())
				iMenuSel = i;
		}

		// ���E�B���h�E�ɑΉ����郁�j���[���`�F�b�N��Ԃɂ���
		if (iMenuSel >= 0) {
			::CheckMenuRadioItem(hMenu, 0, nSelfTab - 1, iMenuSel, MF_BYPOSITION);
		}

		// ���O���[�v�̃E�B���h�E�ꗗ��ǉ�����
		if (nTab > nSelfTab) {
			if (bOtherGroup) {
				for (int i=nSelfTab; i<nTab; ++i) {
					::InsertMenu(hMenu, i, uFlags, IDM_SELWINDOW + i, hIml? (LPCTSTR)&pData[i]: pData[i].szText);
				}
			}else {
				::InsertMenu(hMenu, nSelfTab, MF_BYPOSITION, 101, LS(STR_TABWND_SHOWALL));
			}
			::InsertMenu(hMenu, nSelfTab, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);	// �Z�p���[�^
		}

		// �\���ؑփ��j���[��ǉ�����
		if (bSel) {
			::InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, 100, bFull? LS(STR_TABWND_SHOWTABNAME): LS(STR_TABWND_SHOWPATHNAME));
			::InsertMenu(hMenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);	// �Z�p���[�^
		}

		// ���j���[��\������
		// 2006.04.21 ryoji �}���`���j�^�Ή��̏C��
		RECT rcWork;
		GetMonitorWorkRect(pt, &rcWork);	// ���j�^�̃��[�N�G���A
		int nId = ::TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
									(pt.x > rcWork.left)? pt.x: rcWork.left,
									(pt.y < rcWork.bottom)? pt.y: rcWork.bottom,
									0, GetHwnd(), NULL);
		::DestroyMenu(hMenu);

		// ���j���[�I�����ꂽ�^�u�̃E�B���h�E���A�N�e�B�u�ɂ���
		bRepeat = false;
		if (nId == 100) {	// �\���ؑ�
			bFull = !bFull;
			bRepeat = true;
		}else if (nId == 101) {
			bOtherGroup = !bOtherGroup;
			bRepeat = true;
		}else if (IDM_SELWINDOW <= nId && nId < IDM_SELWINDOW + nTab) {
			ActivateFrameWindow(pData[nId - IDM_SELWINDOW].hwnd);
		}

		delete[] pData;

	} while (bRepeat);

	if (bSel) {
		csTabBar.bTab_ListFull = bFull;
	}

	return 0L;
}


/** ���̃O���[�v�̐擪�E�B���h�E��T��
	@date 2007.06.20 ryoji �V�K�쐬
*/
HWND TabWnd::GetNextGroupWnd(void)
{
	HWND hwndRet = NULL;
	auto& csTabBar = pShareData->common.tabBar;
	if (csTabBar.bDispTabWnd && !csTabBar.bDispTabWndMultiWin) {
		EditNode* pWndArr;
		size_t n = AppNodeManager::getInstance().GetOpenedWindowArr(&pWndArr, false, true);	// �O���[�v�ԍ����\�[�g
		if (n == 0)
			return NULL;
		int i;
		for (i=0; i<n; ++i) {
			if (pWndArr[i].hWnd == GetParentHwnd())
				break;
		}
		if (i < n) {
			int j;
			for (j=i+1; j<n; ++j) {
				if (pWndArr[j].nGroup != pWndArr[i].nGroup) {
					hwndRet = AppNodeManager::getInstance().GetEditNode(pWndArr[j].hWnd)->GetGroup().GetTopEditNode()->GetHwnd();
					break;
				}
			}
			if (j >= n) {
				for (j=0; j<i; ++j) {
					if (pWndArr[j].nGroup != pWndArr[i].nGroup) {
						hwndRet = AppNodeManager::getInstance().GetEditNode(pWndArr[j].hWnd)->GetGroup().GetTopEditNode()->GetHwnd();
						break;
					}
				}
			}
		}
		delete[] pWndArr;
	}

	return hwndRet;
}

/** �O�̃O���[�v�̐擪�E�B���h�E��T��
	@date 2007.06.20 ryoji �V�K�쐬
*/
HWND TabWnd::GetPrevGroupWnd(void)
{
	HWND hwndRet = NULL;
	auto& csTabBar = pShareData->common.tabBar;
	if (csTabBar.bDispTabWnd && !csTabBar.bDispTabWndMultiWin) {
		EditNode* pWndArr;
		auto& appNodeMgr = AppNodeManager::getInstance();
		size_t n = appNodeMgr.GetOpenedWindowArr(&pWndArr, false, true);	// �O���[�v�ԍ����\�[�g
		if (n == 0)
			return NULL;
		size_t i;
		for (i=0; i<n; ++i) {
			if (pWndArr[i].hWnd == GetParentHwnd())
				break;
		}
		if (i < n) {
			size_t j;
			for (j=i-1; j>=0; --j) {
				if (pWndArr[j].nGroup != pWndArr[i].nGroup) {
					hwndRet = appNodeMgr.GetEditNode(pWndArr[j].hWnd)->GetGroup().GetTopEditNode()->GetHwnd();
					break;
				}
			}
			if (j < 0) {
				for (j=n-1; j>i; --j) {
					if (pWndArr[j].nGroup != pWndArr[i].nGroup) {
						hwndRet = appNodeMgr.GetEditNode(pWndArr[j].hWnd)->GetGroup().GetTopEditNode()->GetHwnd();
						break;
					}
				}
			}
		}
		delete[] pWndArr;
	}

	return hwndRet;
}

/** ���̃O���[�v���A�N�e�B�u�ɂ���
	@date 2007.06.20 ryoji �V�K�쐬
*/
void TabWnd::NextGroup(void)
{
	HWND hWnd = GetNextGroupWnd();
	if (hWnd) {
		ActivateFrameWindow(hWnd);
	}
}

/** �O�̃O���[�v���A�N�e�B�u�ɂ���
	@date 2007.06.20 ryoji �V�K�쐬
*/
void TabWnd::PrevGroup(void)
{
	HWND hWnd = GetPrevGroupWnd();
	if (hWnd) {
		ActivateFrameWindow(hWnd);
	}
}

/** �^�u���E�Ɉړ�����
	@date 2007.06.20 ryoji �V�K�쐬
*/
void TabWnd::MoveRight(void)
{
	if (pShareData->common.tabBar.bDispTabWnd) {
		int nIndex = FindTabIndexByHWND(GetParentHwnd());
		if (nIndex != -1) {
			int nCount = TabCtrl_GetItemCount(hwndTab);
			if (nCount - 1 > nIndex) {
				if (ReorderTab(nIndex, nIndex + 1)) {
					BroadcastRefreshToGroup();
				}
			}
		}
	}
}

/** �^�u�����Ɉړ�����
	@date 2007.06.20 ryoji �V�K�쐬
*/
void TabWnd::MoveLeft(void)
{
	if (pShareData->common.tabBar.bDispTabWnd) {
		int nIndex = FindTabIndexByHWND(GetParentHwnd());
		if (nIndex != -1) {
			if (0 < nIndex) {
				if (ReorderTab(nIndex, nIndex - 1)) {
					BroadcastRefreshToGroup();
				}
			}
		}
	}
}

/** �V�K�O���[�v���쐬����i���݂̃O���[�v���番���j
	@date 2007.06.20 ryoji �V�K�쐬
	@date 2007.11.30 ryoji �ő剻���̕����Ή�
*/
void TabWnd::Separate(void)
{
	auto& csTabBar = pShareData->common.tabBar;
	if (csTabBar.bDispTabWnd && !csTabBar.bDispTabWndMultiWin) {
		RECT rc;
		POINT ptSrc;
		POINT ptDst;
		RECT rcWork;
		int cy;

		::GetWindowRect(GetParentHwnd(), &rc);
		if (::IsZoomed(GetParentHwnd())) {
			ptSrc.x = ptDst.x = (rc.left + rc.right) / 2;
			ptSrc.y = ptDst.y = (rc.top + rc.bottom) / 2;
		}else {
			ptSrc.x = rc.left;
			ptSrc.y = rc.top;
			cy = ::GetSystemMetrics(SM_CYCAPTION);
			rc.left += cy;
			rc.right += cy;
			rc.top += cy;
			rc.bottom += cy;
			GetMonitorWorkRect(GetParentHwnd(), &rcWork);
			if (rc.bottom > rcWork.bottom) {
				rc.top -= (rc.bottom - rcWork.bottom);
				rc.bottom = rcWork.bottom;
			}
			if (rc.right > rcWork.right) {
				rc.left -= (rc.right - rcWork.right);
				rc.right = rcWork.right;
			}
			if (rc.top < rcWork.top) {
				rc.bottom += (rcWork.top - rc.top);
				rc.top = rcWork.top;
			}
			if (rc.left < rcWork.left) {
				rc.right += (rcWork.left - rc.left);
				rc.left = rcWork.left;
			}
			ptDst.x = rc.left;
			ptDst.y = rc.top;
		}

		SeparateGroup(GetParentHwnd(), NULL, ptSrc, ptDst);
	}
}

/** ���̃O���[�v�Ɉړ�����i���݂̃O���[�v���番���A�����j
	@date 2007.06.20 ryoji �V�K�쐬
*/
void TabWnd::JoinNext(void)
{
	HWND hWnd = GetNextGroupWnd();
	if (hWnd) {
		POINT ptSrc;
		POINT ptDst;
		ptSrc.x = ptSrc.y = ptDst.x = ptDst.y = 0;
		SeparateGroup(GetParentHwnd(), hWnd, ptSrc, ptDst);
	}
}

/** �O�̃O���[�v�Ɉړ�����i���݂̃O���[�v���番���A�����j
	@date 2007.06.20 ryoji �V�K�쐬
*/
void TabWnd::JoinPrev(void)
{
	HWND hWnd = GetPrevGroupWnd();
	if (hWnd) {
		POINT ptSrc;
		POINT ptDst;
		ptSrc.x = ptSrc.y = ptDst.x = ptDst.y = 0;
		SeparateGroup(GetParentHwnd(), hWnd, ptSrc, ptDst);
	}
}


// �T�C�Y�{�b�N�X�̕\���^��\���؂�ւ�
void TabWnd::SizeBox_ONOFF(bool bSizeBox)
{
	RECT rc;
	GetWindowRect(&rc);
	if (this->bSizeBox == bSizeBox) {
		return;
	}
	if (this->bSizeBox) {
		::DestroyWindow(hwndSizeBox);
		hwndSizeBox = NULL;
		this->bSizeBox = false;
		OnSize();
	}else {
		hwndSizeBox = ::CreateWindowEx(
			0L, 						// no extended styles
			_T("SCROLLBAR"),			// scroll bar control class
			NULL,						// text for window title bar
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, // scroll bar styles
			0,							// horizontal position
			0,							// vertical position
			200,						// width of the scroll bar
			CW_USEDEFAULT,				// default height
			GetHwnd(), 					// handle of main window
			(HMENU) NULL,				// no menu for a scroll bar
			GetAppInstance(),			// instance owning this window
			(LPVOID) NULL				// pointer not needed
		);
		::ShowWindow( hwndSizeBox, SW_SHOW );
		this->bSizeBox = true;
		OnSize();
	}
	return;
}
