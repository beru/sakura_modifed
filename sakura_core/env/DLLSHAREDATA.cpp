/*
	Copyright (C) 2008, kobake

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
#include "DLLSHAREDATA.h"
#include "_main/Mutex.h"
#include "dlg/DlgCancel.h"
#include "uiparts/WaitCursor.h"
#include "util/os.h"
#include "util/window.h"
#include "sakura_rc.h"

// GetDllShareData�p�O���[�o���ϐ�
DLLSHAREDATA* g_theDLLSHAREDATA = NULL;

static Mutex g_cKeywordMutex(FALSE, GSTR_MUTEX_SAKURA_KEYWORD);

ShareDataLockCounter::ShareDataLockCounter() {
	LockGuard<Mutex> guard(g_cKeywordMutex);
	assert_warning(0 <= GetDllShareData().m_nLockCount);
	GetDllShareData().m_nLockCount++;
}

ShareDataLockCounter::~ShareDataLockCounter() {
	LockGuard<Mutex> guard(g_cKeywordMutex);
	GetDllShareData().m_nLockCount--;
	assert_warning(0 <= GetDllShareData().m_nLockCount);
}

int ShareDataLockCounter::GetLockCounter() {
	LockGuard<Mutex> guard(g_cKeywordMutex);
	assert_warning(0 <= GetDllShareData().m_nLockCount);
	return GetDllShareData().m_nLockCount;
}

class LockCancel: public DlgCancel {
public:
	virtual BOOL OnInitDialog(HWND hwnd, WPARAM wParam, LPARAM lParam) {
		BOOL ret = DlgCancel::OnInitDialog(hwnd, wParam, lParam);
		HWND hwndCancel = GetHwnd();
		HWND hwndMsg = ::GetDlgItem(hwndCancel, IDC_STATIC_MSG);
		HWND hwndCancelButton = ::GetDlgItem(hwndCancel, IDCANCEL);
		HWND hwndKensuu = ::GetDlgItem(hwndCancel, IDC_STATIC_KENSUU);
		LPCTSTR msg = LS(STR_PRINT_WAITING);
		TextWidthCalc calc(hwndMsg);
		calc.SetTextWidthIfMax(msg);
		RECT rc;
		GetItemClientRect(IDC_STATIC_MSG, rc);
		rc.right = rc.left + calc.GetCx() + 2;
		::MoveWindow(hwndMsg, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, FALSE);
		::SetWindowText(hwndMsg, msg);
		::ShowWindow(hwndCancelButton, SW_HIDE);
		::ShowWindow(hwndKensuu, SW_HIDE);
		if (GetComctl32Version() >= PACKVERSION(6, 0)) {
			// �}�[�L�[�ɂ���(CommCtrl 6.0�ȏ�)
			HWND hwndProgress = GetItemHwnd(IDC_PROGRESS);
			// �X�^�C���ύX + ���b�Z�[�W�łȂ��Ƌ@�\���Ȃ�
			LONG_PTR style = ::GetWindowLongPtr(hwndProgress, GWL_STYLE);
			::SetWindowLongPtr(hwndProgress, GWL_STYLE, style | PBS_MARQUEE);
			Progress_SetMarquee(hwndProgress, TRUE, 100);
		}else {
			HWND hwndProgress = ::GetDlgItem(hwndCancel, IDC_PROGRESS);
			::ShowWindow(hwndProgress, SW_HIDE);
		}
		return ret;
	}
};

// count��0��������Lock���ĕԂ�
static
int GetCountIf0Lock(ShareDataLockCounter** ppLock)
{
	LockGuard<Mutex> guard(g_cKeywordMutex);
	int count = GetDllShareData().m_nLockCount;
	if (count <= 0) {
		if (ppLock) {
			*ppLock = new ShareDataLockCounter();
		}
	}
	return count;
}

void ShareDataLockCounter::WaitLock(HWND hwndParent, ShareDataLockCounter** ppLock) {
	if (0 < GetCountIf0Lock(ppLock)) {
		DWORD dwTime = ::GetTickCount();
		WaitCursor waitCursor(hwndParent);
		LockCancel* pDlg = NULL;
		HWND hwndCancel = NULL;
		::EnableWindow(hwndParent, FALSE);
		while (0 < GetCountIf0Lock(ppLock)) {
			DWORD dwResult = MsgWaitForMultipleObjects(0, NULL, FALSE, 100, QS_ALLEVENTS);
			if (dwResult == 0xFFFFFFFF) {
				break;
			}
			if (!BlockingHook(hwndCancel)) {
				break;
			}
			if (!pDlg) {
				DWORD dwTimeNow = ::GetTickCount();
				if (2000 < dwTimeNow - dwTime) {
					pDlg = new LockCancel();
					hwndCancel = pDlg->DoModeless(::GetModuleHandle(NULL), hwndParent, IDD_OPERATIONRUNNING);
				}
			}
		}
		if (pDlg) {
			pDlg->CloseDialog(0);
			delete pDlg;
		}
		::EnableWindow(hwndParent, TRUE);
	}
}

