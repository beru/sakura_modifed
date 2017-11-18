/*!	@file
	@brief ファイル比較ダイアログボックス
*/
#include "StdAfx.h"
#include "dlg/DlgCompare.h"
#include "window/EditWnd.h"
#include "func/Funccode.h"
#include "util/shell.h"
#include "util/fileUtil.h"
#include "util/string_ex2.h"
#include "sakura_rc.h"
#include "sakura.hh"

// ファイル内容比較 DlgCompare.cpp
const DWORD p_helpids[] = {	//12300
//	IDC_STATIC,						-1,
	IDOK,							HIDOK_CMP,					// OK
	IDCANCEL,						HIDCANCEL_CMP,				// キャンセル
	IDC_BUTTON_HELP,				HIDC_CMP_BUTTON_HELP,		// ヘルプ
	IDC_CHECK_TILE_H,				HIDC_CMP_CHECK_TILE_H,		// 左右に表示
	IDC_LIST_FILES,					HIDC_CMP_LIST_FILES,		// ファイル一覧
	IDC_STATIC_COMPARESRC,			HIDC_CMP_STATIC_COMPARESRC,	// ソースファイル
	0, 0
};

static const AnchorListItem anchorList[] = {
	{IDOK,					AnchorStyle::Bottom},
	{IDCANCEL,				AnchorStyle::Bottom},
	{IDC_BUTTON_HELP,		AnchorStyle::Bottom},
	{IDC_CHECK_TILE_H,		AnchorStyle::Left},
	{IDC_LIST_FILES,        AnchorStyle::All},
	{IDC_STATIC_COMPARESRC, AnchorStyle::LeftRight},
};

DlgCompare::DlgCompare()
	:
	Dialog(true),
	pszPath(NULL)
{
	// サイズ変更時に位置を制御するコントロール数
	assert(_countof(anchorList) == _countof(rcItems));

	bCompareAndTileHorz = true;	// 左右に並べて表示

	ptDefaultSize.x = -1;
	ptDefaultSize.y = -1;
	return;
}


// モーダルダイアログの表示
INT_PTR DlgCompare::DoModal(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	LPARAM			lParam,
	const TCHAR*	pszPath,
	TCHAR*			pszCompareLabel,
	HWND*			phwndCompareWnd
	)
{
	this->pszPath = pszPath;
	this->pszCompareLabel = pszCompareLabel;
	this->phwndCompareWnd = phwndCompareWnd;
	return Dialog::DoModal(hInstance, hwndParent, IDD_COMPARE, lParam);
}

BOOL DlgCompare::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_HELP:
		//「内容比較」のヘルプ
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_COMPARE));
		return TRUE;
// チェックボックスをボタン化してDlgCompare.cppに直接書き込んでみたが失敗
// ダイアログのボタンは下に不可視化しておいてあります。
// 以下の追加コードは全部消して結構ですから誰か作ってください。水平スクロールも入れてくれるとなおうれしいです。
//	case IDC_BUTTON1:	/* 上下に表示 */
//		/* ダイアログデータの取得 */
//		return TRUE;
//	case IDOK:			/* 左右に表示 */
//		/* ダイアログデータの取得 */
//		HWND	hwndCompareWnd;
//		HWND*	phwndArr;
//		int		i;
//		phwndArr = new HWND[2];
//		phwndArr[0] = ::GetParent(hwndParent);
//		phwndArr[1] = hwndCompareWnd;
//		for (i=0; i<2; ++i) {
//			if (::IsZoomed(phwndArr[i])) {
//				::ShowWindow(phwndArr[i], SW_RESTORE);
//			}
//		}
//		::TileWindows(NULL, MDITILE_VERTICAL, NULL, 2, phwndArr);
//		delete[] phwndArr;
//		CloseDialog(0);
//		return TRUE;
	case IDOK:			// 左右に表示
		// ダイアログデータの取得
		::EndDialog(GetHwnd(), GetData());
		return TRUE;
	case IDCANCEL:
		::EndDialog(GetHwnd(), FALSE);
		return TRUE;
	}
	// 基底クラスメンバ
	return Dialog::OnBnClicked(wID);
}


// ダイアログデータの設定
void DlgCompare::SetData(void)
{
	EditNode*	pEditNodeArr;
	TCHAR		szMenu[512];
	int			selIndex = 0;

	HWND hwndList = GetItemHwnd(IDC_LIST_FILES);

	// 現在開いている編集窓のリストをメニューにする
	size_t nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true);
	if (nRowNum > 0) {
		// 水平スクロール幅は実際に表示する文字列の幅を計測して決める
		TextWidthCalc calc(hwndList);
		size_t score = 0;
		TCHAR szFile1[_MAX_PATH];
		SplitPath_FolderAndFile(pszPath, NULL, szFile1);
		for (size_t i=0; i<nRowNum; ++i) {
			// トレイからエディタへの編集ファイル名要求通知
			::SendMessage(pEditNodeArr[i].GetHwnd(), MYWM_GETFILEINFO, 0, 0);
			EditInfo* pfi = (EditInfo*)&pShareData->workBuffer.editInfo_MYWM_GETFILEINFO;

			if (pEditNodeArr[i].GetHwnd() == EditWnd::getInstance().GetHwnd()) {
				// 自分の名前もここから設定する
				FileNameManager::getInstance().GetMenuFullLabel_WinListNoEscape(szMenu, _countof(szMenu), pfi, pEditNodeArr[i].nId, -1, calc.GetDC());
				SetItemText(IDC_STATIC_COMPARESRC, szMenu);
				continue;
			}
			// 番号は ウィンドウリストと同じになるようにする
			FileNameManager::getInstance().GetMenuFullLabel_WinListNoEscape(szMenu, _countof(szMenu), pfi, pEditNodeArr[i].nId, i, calc.GetDC());

			LRESULT nItem = ::List_AddString(hwndList, szMenu);
			List_SetItemData(hwndList, nItem, pEditNodeArr[i].GetHwnd());

			// 横幅を計算する
			calc.SetTextWidthIfMax(szMenu);

			// ファイル名一致のスコアを計算する
			TCHAR szFile2[_MAX_PATH];
			SplitPath_FolderAndFile(pfi->szPath, NULL, szFile2);
			size_t scoreTemp = FileMatchScoreSepExt(szFile1, szFile2);
			if (score < scoreTemp) {
				// スコアのいいものを選択
				score = scoreTemp;
				selIndex = nItem;
			}
		}
		delete[] pEditNodeArr;
		// リストビューの横幅を設定。これをやらないと水平スクロールバーが使えない
		List_SetHorizontalExtent( hwndList, calc.GetCx() );
	}
	List_SetCurSel(hwndList, selIndex);

	// 左右に並べて表示
	// TAB 1ウィンドウ表示のときは並べて比較できなくする
	if (pShareData->common.tabBar.bDispTabWnd
		&& !pShareData->common.tabBar.bDispTabWndMultiWin
	) {
		bCompareAndTileHorz = false;
		EnableItem(IDC_CHECK_TILE_H, false);
	}
	CheckButton(IDC_CHECK_TILE_H, bCompareAndTileHorz);
	return;
}


// ダイアログデータの取得
// TRUE==正常  FALSE==入力エラー
int DlgCompare::GetData(void)
{
	HWND hwndList = GetItemHwnd(IDC_LIST_FILES);
	int nItem = List_GetCurSel(hwndList);
	if (nItem == LB_ERR) {
		return FALSE;
	}else {
		*phwndCompareWnd = (HWND)List_GetItemData(hwndList, nItem);
		// トレイからエディタへの編集ファイル名要求通知
		::SendMessage(*phwndCompareWnd, MYWM_GETFILEINFO, 0, 0);
		EditInfo* pfi = (EditInfo*)&pShareData->workBuffer.editInfo_MYWM_GETFILEINFO;

		int nId = AppNodeManager::getInstance().GetEditNode(*phwndCompareWnd)->GetId();
		TextWidthCalc calc(hwndList);
		FileNameManager::getInstance().GetMenuFullLabel_WinListNoEscape(pszCompareLabel, _MAX_PATH/*長さ不明*/, pfi, nId, -1, calc.GetDC());

		// 左右に並べて表示
		bCompareAndTileHorz = IsButtonChecked(IDC_CHECK_TILE_H);

		return TRUE;
	}
}

LPVOID DlgCompare::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

INT_PTR DlgCompare::DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR result = Dialog::DispatchEvent(hWnd, wMsg, wParam, lParam);
	if (wMsg == WM_GETMINMAXINFO) {
		return OnMinMaxInfo(lParam);
	}
	return result;
}

BOOL DlgCompare::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	_SetHwnd(hwndDlg);

	CreateSizeBox();
	Dialog::OnSize();
	
	RECT rc;
	::GetWindowRect(hwndDlg, &rc);
	ptDefaultSize.x = rc.right - rc.left;
	ptDefaultSize.y = rc.bottom - rc.top;

	for (size_t i=0; i<_countof(anchorList); ++i) {
		GetItemClientRect(anchorList[i].id, rcItems[i]);
	}

	RECT rcDialog = GetDllShareData().common.others.rcCompareDialog;
	if (rcDialog.left != 0
		|| rcDialog.bottom != 0
	) {
		xPos = rcDialog.left;
		yPos = rcDialog.top;
		nWidth = rcDialog.right - rcDialog.left;
		nHeight = rcDialog.bottom - rcDialog.top;
	}

	return Dialog::OnInitDialog(hwndDlg, wParam, lParam);
}

BOOL DlgCompare::OnSize(WPARAM wParam, LPARAM lParam)
{
	// 基底クラスメンバ
	Dialog::OnSize(wParam, lParam);

	GetWindowRect(&GetDllShareData().common.others.rcCompareDialog);

	RECT rc;
	GetWindowRect(&rc);
	POINT ptNew;
	ptNew.x = rc.right - rc.left;
	ptNew.y = rc.bottom - rc.top;
	for (size_t i=0; i<_countof(anchorList); ++i) {
		ResizeItem(GetItemHwnd(anchorList[i].id), ptDefaultSize, ptNew, rcItems[i], anchorList[i].anchor);
	}
	::InvalidateRect(GetHwnd(), NULL, TRUE);
	return TRUE;
}

BOOL DlgCompare::OnMove(WPARAM wParam, LPARAM lParam)
{
	GetWindowRect(&GetDllShareData().common.others.rcCompareDialog);
	return Dialog::OnMove(wParam, lParam);
}

BOOL DlgCompare::OnMinMaxInfo(LPARAM lParam)
{
	LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
	if (ptDefaultSize.x < 0) {
		return 0;
	}
	lpmmi->ptMinTrackSize.x = ptDefaultSize.x;
	lpmmi->ptMinTrackSize.y = ptDefaultSize.y;
	lpmmi->ptMaxTrackSize.x = ptDefaultSize.x*2;
	lpmmi->ptMaxTrackSize.y = ptDefaultSize.y*3;
	return 0;
}


