#include "StdAfx.h"
#include "VisualProgress.h"
#include "WaitCursor.h"

#include "window/EditWnd.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
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
//                        ロード前後                           //
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
//                        セーブ前後                           //
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
//                      プログレス受信                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void VisualProgress::OnProgress(int nPer)
{
	_Doing(nPer);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void VisualProgress::_Begin()
{
	// 砂時計
	if (!pWaitCursor) {
		pWaitCursor = new WaitCursor(EditWnd::getInstance().GetHwnd());
	}

	// プログレスバー
	HWND hwndProgress = EditWnd::getInstance().m_statusBar.GetProgressHwnd();
	if (hwndProgress) {
		::ShowWindow(hwndProgress, SW_SHOW);
		// 範囲設定・リセット
		Progress_SetRange(hwndProgress, 0, 101);
		Progress_SetPos(hwndProgress, 0);
	}
}

void VisualProgress::_Doing(int nPer)
{
	// プログレスバー更新
	HWND hwndProgress = EditWnd::getInstance().m_statusBar.GetProgressHwnd();
	if (hwndProgress) {
		if (nOldValue != nPer) {
			Progress_SetPos(hwndProgress, nPer + 1); // 2013.06.10 Moca Vista/7等でプログレスバーがアニメーションで遅れる対策
			Progress_SetPos(hwndProgress, nPer);
			nOldValue = nPer;
		}
	}
}

void VisualProgress::_End()
{
	// プログレスバー
	HWND hwndProgress = EditWnd::getInstance().m_statusBar.GetProgressHwnd();
	if (hwndProgress) {
		Progress_SetPos(hwndProgress, 0);
		::ShowWindow(hwndProgress, SW_HIDE);
	}

	// 砂時計
	SAFE_DELETE(pWaitCursor);
}

