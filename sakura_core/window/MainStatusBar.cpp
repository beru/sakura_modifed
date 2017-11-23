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


//	キーワード：ステータスバー順序
// ステータスバー作成
void MainStatusBar::CreateStatusBar()
{
	if (hwndStatusBar) return;

	// ステータスバー
	hwndStatusBar = ::CreateStatusWindow(
		WS_CHILD/* | WS_VISIBLE*/ | WS_EX_RIGHT | SBARS_SIZEGRIP,	// 2007.03.08 ryoji WS_VISIBLE 除去
		_T(""),
		owner.GetHwnd(),
		IDW_STATUSBAR
	);

	// プログレスバー
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

	// スプリッターの、サイズボックスの位置を変更
	owner.splitterWnd.DoSplit(-1, -1);
}


// ステータスバー破棄
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
		if (GetDllShareData().common.window.nFuncKeyWnd_Place == 0) {	// ファンクションキー表示位置／0:上 1:下
			// サイズボックスの表示／非表示切り替え
			bSizeBox = false;
		}else {
			bSizeBox = true;
			// ステータスパーを表示している場合はサイズボックスを表示しない
			if (hwndStatusBar) {
				bSizeBox = false;
			}
		}
		owner.funcKeyWnd.SizeBox_ONOFF(bSizeBox);
	}
	// スプリッターの、サイズボックスの位置を変更
	owner.splitterWnd.DoSplit(-1, -1);
}


/*!
	@brief メッセージの表示
	
	指定されたメッセージをステータスバーに表示する．
	メニューバー右端に入らないものや，桁位置表示を隠したくないものに使う
	
	呼び出し前にSendStatusMessage2IsEffective()で処理の有無を
	確認することで無駄な処理を省くことが出来る．

	@param msg [in] 表示するメッセージ
	
	@sa SendStatusMessage2IsEffective
*/
void MainStatusBar::SendStatusMessage2(const TCHAR* msg)
{
	if (hwndStatusBar) {
		// ステータスバーへ
		StatusBar_SetText(hwndStatusBar, 0 | SBT_NOBORDERS, msg);
	}
}


void MainStatusBar::SetStatusText(int nIndex, int nOption, const TCHAR* pszText)
{
	StatusBar_SetText(hwndStatusBar, nIndex | nOption, pszText);
}

