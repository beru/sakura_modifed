#include "StdAfx.h"
#include "CVisualProgress.h"
#include "CWaitCursor.h"

#include "window/CEditWnd.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

VisualProgress::VisualProgress()
	:
	m_pcWaitCursor(NULL),
	nOldValue(-1)
{
}

VisualProgress::~VisualProgress()
{
	SAFE_DELETE(m_pcWaitCursor);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ロード前後                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void VisualProgress::OnBeforeLoad(LoadInfo* sLoadInfo)
{
	_Begin();
}

void VisualProgress::OnAfterLoad(const LoadInfo& sLoadInfo)
{
	_End();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        セーブ前後                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void VisualProgress::OnBeforeSave(const SaveInfo& sSaveInfo)
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
	if (!m_pcWaitCursor) {
		m_pcWaitCursor = new WaitCursor(EditWnd::getInstance()->GetHwnd());
	}

	// プログレスバー
	HWND hwndProgress = EditWnd::getInstance()->m_cStatusBar.GetProgressHwnd();
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
	HWND hwndProgress = EditWnd::getInstance()->m_cStatusBar.GetProgressHwnd();
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
	HWND hwndProgress = EditWnd::getInstance()->m_cStatusBar.GetProgressHwnd();
	if (hwndProgress) {
		Progress_SetPos(hwndProgress, 0);
		::ShowWindow(hwndProgress, SW_HIDE);
	}

	// 砂時計
	SAFE_DELETE(m_pcWaitCursor);
}

