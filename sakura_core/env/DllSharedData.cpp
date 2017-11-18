#include "StdAfx.h"
#include "DllSharedData.h"
#include "_main/Mutex.h"
#include "dlg/DlgCancel.h"
#include "uiparts/WaitCursor.h"
#include "util/os.h"
#include "util/window.h"
#include "sakura_rc.h"

// GetDllShareData用グローバル変数
DllSharedData* g_theDLLSHAREDATA = nullptr;

static Mutex g_keywordMutex(FALSE, GSTR_MUTEX_SAKURA_KEYWORD);

ShareDataLockCounter::ShareDataLockCounter() {
	LockGuard<Mutex> guard(g_keywordMutex);
	assert_warning(0 <= GetDllShareData().nLockCount);
	GetDllShareData().nLockCount++;
}

ShareDataLockCounter::~ShareDataLockCounter() {
	LockGuard<Mutex> guard(g_keywordMutex);
	GetDllShareData().nLockCount--;
	assert_warning(0 <= GetDllShareData().nLockCount);
}

int ShareDataLockCounter::GetLockCounter() {
	LockGuard<Mutex> guard(g_keywordMutex);
	assert_warning(0 <= GetDllShareData().nLockCount);
	return GetDllShareData().nLockCount;
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
			// マーキーにする(CommCtrl 6.0以上)
			HWND hwndProgress = GetItemHwnd(IDC_PROGRESS);
			// スタイル変更 + メッセージでないと機能しない
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

// countが0だったらLockして返す
static
int GetCountIf0Lock(ShareDataLockCounter** ppLock)
{
	LockGuard<Mutex> guard(g_keywordMutex);
	int count = GetDllShareData().nLockCount;
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
		LockCancel* pDlg = nullptr;
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

