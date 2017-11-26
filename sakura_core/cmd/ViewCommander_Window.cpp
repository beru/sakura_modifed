#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "_main/ControlTray.h"
#include "util/os.h"
#include "env/SakuraEnvironment.h"
#include "env/ShareData.h"

// ViewCommanderクラスのコマンド(ウィンドウ系)関数群

// 上下に分割
void ViewCommander::Command_Split_V(void)
{
	GetEditWindow().splitterWnd.VSplitOnOff();
	return;
}


// 左右に分割
void ViewCommander::Command_Split_H(void)
{
	GetEditWindow().splitterWnd.HSplitOnOff();
	return;
}


// 縦横に分割
void ViewCommander::Command_Split_VH(void)
{
	GetEditWindow().splitterWnd.VHSplitOnOff();
	return;
}


// ウィンドウを閉じる
void ViewCommander::Command_WinClose(void)
{
	// 閉じる
	::PostMessage(GetMainWindow(), MYWM_CLOSE, FALSE,
		(LPARAM)AppNodeManager::getInstance().GetNextTab(GetMainWindow()));	// タブまとめ時、次のタブに移動
	return;
}


// すべてのウィンドウを閉じる
void ViewCommander::Command_FileCloseAll(void)
{
	int nGroup = AppNodeManager::getInstance().GetEditNode(GetMainWindow())->GetGroup();
	ControlTray::CloseAllEditor(true, GetMainWindow(), false, nGroup);
	return;
}


// このタブ以外を閉じる
void ViewCommander::Command_Tab_CloseOther(void)
{
	int nGroup = 0;

	// ウィンドウ一覧を取得する
	EditNode* pEditNode;
	size_t nCount = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNode, true);
	if (nCount == 0) {
		return;
	}

	for (size_t i=0; i<nCount; ++i) {
		auto& node = pEditNode[i];
		if (node.hWnd == GetMainWindow()) {
			node.hWnd = NULL;		// 自分自身は閉じない
			nGroup = node.nGroup;
		}
	}

	// 終了要求を出す
	AppNodeGroupHandle(nGroup).RequestCloseEditor(pEditNode, nCount, false, true, GetMainWindow());
	delete[] pEditNode;
	return;
}


/*!	@brief ウィンドウ一覧ポップアップ表示処理（ファイル名のみ）*/
void ViewCommander::Command_WinList(int nCommandFrom)
{
	// ウィンドウ一覧をポップアップ表示する
	GetEditWindow().PopupWinList((nCommandFrom & FA_FROMKEYBOARD) != FA_FROMKEYBOARD);
	// アクセラレータキーからでなければマウス位置に
}


/*!	@brief 重ねて表示 */
void ViewCommander::Command_Cascade(void)
{
	// 現在開いている編集窓のリストを取得する
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
	size_t count = 0;	// 処理対象ウィンドウカウント
	// 現在のウィンドウを末尾に持っていくのに使う
	int	current_win_index = -1;

	// -----------------------------------------
	// ウィンドウ(ハンドル)リストの作成
	// -----------------------------------------
	for (size_t i=0; i<nRowNum; ++i) {
		auto editNodeHWnd = pEditNodeArr[i].GetHwnd();
		if (::IsIconic(editNodeHWnd)) {	// 最小化しているウィンドウは無視。
			continue;
		}
		if (!::IsWindowVisible(editNodeHWnd)) {	// 不可視ウィンドウは無視。
			continue;
		}
		// 現在のウィンドウを末尾に持っていくためここではスキップ
		if (editNodeHWnd == EditWnd::getInstance().GetHwnd()) {
			current_win_index = (int)i;
			continue;
		}
		pWndArr[count].hWnd = editNodeHWnd;
		++count;
	}

	// 現在のウィンドウを末尾に挿入
	if (current_win_index >= 0) {
		pWndArr[count].hWnd = pEditNodeArr[current_win_index].GetHwnd();
		++count;
	}

	// デスクトップサイズを得る
	RECT rcDesktop;
	// マルチモニタ対応
	::GetMonitorWorkRect(view.GetHwnd(), &rcDesktop);
	
	int width = (rcDesktop.right - rcDesktop.left) * 4 / 5;
	int height = (rcDesktop.bottom - rcDesktop.top) * 4 / 5;
	int w_delta = ::GetSystemMetrics(SM_CXSIZEFRAME) + ::GetSystemMetrics(SM_CXSIZE);
	int h_delta = ::GetSystemMetrics(SM_CYSIZEFRAME) + ::GetSystemMetrics(SM_CYSIZE);
	int w_offset = rcDesktop.left; // 絶対値だとエクスプローラーのウィンドウに重なるので
	int h_offset = rcDesktop.top; // 初期値をデスクトップ内に収める。

	// -----------------------------------------
	// 座標計算
	//		左上をデスクトップ領域に合わせる(タスクバーが上・左にある場合のため)．
	//		ウィンドウが右下からはみ出たら左上に戻るが，
	//		2周目以降は開始位置を右にずらしてアイコンが見えるようにする．
	//
	// ここでは計算値を保管するだけでウィンドウの再配置は行わない
	// -----------------------------------------

	int roundtrip = 0; // ２度目の描画以降で使用するカウント
	int sw_offset = w_delta; // 右スライドの幅

	for (size_t i=0; i<count; ++i) {
		if (w_offset + width > rcDesktop.right || h_offset + height > rcDesktop.bottom) {
			++roundtrip;
			if ((rcDesktop.right - rcDesktop.left) - sw_offset * roundtrip < width) {
				// これ以上右にずらせないときはしょうがないから左上に戻る
				roundtrip = 0;
			}
			// ウィンドウ領域の左上にセット
			w_offset = rcDesktop.left + sw_offset * roundtrip;
			h_offset = rcDesktop.top;
		}
		
		pWndArr[i].newX = w_offset;
		pWndArr[i].newY = h_offset;

		w_offset += w_delta;
		h_offset += h_delta;
	}

	// -----------------------------------------
	// 最大化/非表示解除
	// 最大化されたウィンドウを元に戻す．これがないと，最大化ウィンドウが
	// 最大化状態のまま並び替えられてしまい，その後最大化動作が変になる．
	// -----------------------------------------
	for (size_t i=0; i<count; ++i) {
		::ShowWindow(pWndArr[i].hWnd, SW_RESTORE | SW_SHOWNA);
	}

	// -----------------------------------------
	// ウィンドウ配置
	//
	// APIを素直に使ってZ-Orderの上から下の順で並べる．
	// -----------------------------------------

	// まずカレントを最前面に
	size_t i = count - 1;
	
	::SetWindowPos(
		pWndArr[i].hWnd, HWND_TOP,
		pWndArr[i].newX, pWndArr[i].newY,
		width, height,
		0
	);

	// 残りを1つずつ下に入れていく
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


// 上下に並べて表示
void ViewCommander::Command_Tile_V(void)
{
	// 現在開いている編集窓のリストを取得する
	EditNode* pEditNodeArr;
	size_t nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true/*false*/, true);

	if (nRowNum == 0) {
		return;
	}
	std::vector<HWND> hWnds(nRowNum);
	HWND* phwndArr = &hWnds[0];
	size_t count = 0;
	// デスクトップサイズを得る
	RECT rcDesktop;
	// マルチモニタ対応
	::GetMonitorWorkRect(view.GetHwnd(), &rcDesktop);
	for (size_t i=0; i<nRowNum; ++i) {
		auto editNodeHWnd = pEditNodeArr[i].GetHwnd();
		if (::IsIconic(editNodeHWnd)) {	// 最小化しているウィンドウは無視。
			continue;
		}
		if (!::IsWindowVisible(editNodeHWnd)) {	// 不可視ウィンドウは無視。
			continue;
		}
		// 現在のウィンドウを先頭に持ってくる
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
			rcDesktop.left, rcDesktop.top + (int)(height * i), // 上端調整
			rcDesktop.right - rcDesktop.left, (int)height,
			SWP_NOOWNERZORDER | SWP_NOZORDER
		);
	}
	::SetFocus(phwndArr[0]);

	delete[] pEditNodeArr;
}


// 左右に並べて表示
void ViewCommander::Command_Tile_H(void)
{
	// 現在開いている編集窓のリストを取得する
	EditNode* pEditNodeArr;
	size_t nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true/*false*/, true);
	if (nRowNum == 0) {
		return;
	}
	std::vector<HWND> hWnds(nRowNum);
	HWND* phwndArr = &hWnds[0];
	size_t count = 0;
	// デスクトップサイズを得る
	RECT rcDesktop;
	// マルチモニタ対応
	::GetMonitorWorkRect(view.GetHwnd(), &rcDesktop);
	for (size_t i=0; i<nRowNum; ++i) {
		auto editNodeHWnd = pEditNodeArr[i].GetHwnd();
		if (::IsIconic(editNodeHWnd)) {	// 最小化しているウィンドウは無視。
			continue;
		}
		if (!::IsWindowVisible(editNodeHWnd)) {	// 不可視ウィンドウは無視。
			continue;
		}
		// 現在のウィンドウを先頭に持ってくる
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
			(int)(width * i) + rcDesktop.left, rcDesktop.top, // タスクバーが左にある場合を考慮
			(int)width, rcDesktop.bottom - rcDesktop.top,
			SWP_NOOWNERZORDER | SWP_NOZORDER
		);
	}
	::SetFocus(phwndArr[0]);
	delete[] pEditNodeArr;
}


/*! 常に手前に表示 */
void ViewCommander::Command_WinTopMost(LPARAM lparam)
{
	GetEditWindow().WindowTopMost(int(lparam));
}

/*!	@brief 結合して表示

	タブウィンドウの結合、非結合を切り替えるコマンドです。
	[共通設定]->[ウィンドウ]->[タブ表示 まとめない]の切り替えと同じです。
*/
void ViewCommander::Command_Bind_Window(void)
{
	// タブモードであるならば
	auto& csTabBar = GetDllShareData().common.tabBar;
	if (!csTabBar.bDispTabWnd) {
		return;
	}
	// タブウィンドウの設定を変更
	csTabBar.bDispTabWndMultiWin = !csTabBar.bDispTabWndMultiWin;

	// まとめるときは WS_EX_TOPMOST 状態を同期する
	if (!csTabBar.bDispTabWndMultiWin) {
		GetEditWindow().WindowTopMost(
			((DWORD)::GetWindowLongPtr(GetEditWindow().GetHwnd(), GWL_EXSTYLE) & WS_EX_TOPMOST)? 1: 2
		);
	}

	// タブウィンドウの設定を変更をブロードキャストする
	AppNodeManager::getInstance().ResetGroupId();
	AppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_TAB_WINDOW_NOTIFY,						// タブウィンドウイベント
		(WPARAM)((csTabBar.bDispTabWndMultiWin) ? TabWndNotifyType::Disable : TabWndNotifyType::Enable), // タブモード有効/無効化イベント
		(LPARAM)GetEditWindow().GetHwnd(),	// EditWndのウィンドウハンドル
		view.GetHwnd());									// 自分自身
}

// グループを閉じる
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

// 次のグループ
void ViewCommander::Command_NextGroup(void)
{
	auto& tabWnd = GetEditWindow().tabWnd;
	if (!tabWnd.GetHwnd()) {
		return;
	}
	tabWnd.NextGroup();
}

// 前のグループ
void ViewCommander::Command_PrevGroup(void)
{
	auto& tabWnd = GetEditWindow().tabWnd;
	if (!tabWnd.GetHwnd()) {
		return;
	}
	tabWnd.PrevGroup();
}

// タブを右に移動
void ViewCommander::Command_Tab_MoveRight(void)
{
	auto& tabWnd = GetEditWindow().tabWnd;
	if (!tabWnd.GetHwnd()) {
		return;
	}
	tabWnd.MoveRight();
}

// タブを左に移動
void ViewCommander::Command_Tab_MoveLeft(void)
{
	auto& tabWnd = GetEditWindow().tabWnd;
	if (!tabWnd.GetHwnd()) {
		return;
	}
	tabWnd.MoveLeft();
}

// 新規グループ
void ViewCommander::Command_Tab_Separate(void)
{
	auto& tabWnd = GetEditWindow().tabWnd;
	if (!tabWnd.GetHwnd()) {
		return;
	}
	tabWnd.Separate();
}

// 次のグループに移動
void ViewCommander::Command_Tab_JointNext(void)
{
	auto& tabWnd = GetEditWindow().tabWnd;
	if (!tabWnd.GetHwnd()) {
		return;
	}
	tabWnd.JoinNext();
}

// 前のグループに移動
void ViewCommander::Command_Tab_JointPrev(void)
{
	auto& tabWnd = GetEditWindow().tabWnd;
	if (!tabWnd.GetHwnd()) {
		return;
	}
	tabWnd.JoinPrev();
}


// 左をすべて閉じる
void ViewCommander::Command_Tab_CloseLeft(void)
{
	if (!GetDllShareData().common.tabBar.bDispTabWnd) {
		return;
	}
	int nGroup = 0;

	// ウィンドウ一覧を取得する
	EditNode* pEditNode;
	size_t nCount = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNode, true);
	bool bSelfFound = false;
	if (0 >= nCount) return;

	for (size_t i=0; i<nCount; ++i) {
		if (pEditNode[i].hWnd == GetMainWindow()) {
			pEditNode[i].hWnd = NULL;		// 自分自身は閉じない
			nGroup = pEditNode[i].nGroup;
			bSelfFound = true;
		}else if (bSelfFound) {
			pEditNode[i].hWnd = NULL;		// 右は閉じない
		}
	}

	// 終了要求を出す
	AppNodeGroupHandle(nGroup).RequestCloseEditor(pEditNode, nCount, false, true, GetMainWindow());
	delete[] pEditNode;
}


// 右をすべて閉じる
void ViewCommander::Command_Tab_CloseRight(void)
{
	if (!GetDllShareData().common.tabBar.bDispTabWnd) {
		return;
	}
	int nGroup = 0;

	// ウィンドウ一覧を取得する
	EditNode* pEditNode;
	size_t nCount = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNode, true);
	bool bSelfFound = false;
	if (0 >= nCount) return;

	for (size_t i=0; i<nCount; ++i) {
		if (pEditNode[i].hWnd == GetMainWindow()) {
			pEditNode[i].hWnd = NULL;		// 自分自身は閉じない
			nGroup = pEditNode[i].nGroup;
			bSelfFound = true;
		}else if (!bSelfFound) {
			pEditNode[i].hWnd = NULL;		// 左は閉じない
		}
	}

	// 終了要求を出す
	AppNodeGroupHandle(nGroup).RequestCloseEditor(pEditNode, nCount, false, true, GetMainWindow());
	delete[] pEditNode;
}


// 縦方向に最大化
void ViewCommander::Command_Maximize_V(void)
{
	RECT rcOrg;
	RECT rcDesktop;
	HWND hwndFrame = GetMainWindow();
	::GetWindowRect(hwndFrame, &rcOrg);
	// マルチモニタ対応
	::GetMonitorWorkRect(hwndFrame, &rcDesktop);
	::SetWindowPos(
		hwndFrame, 0,
		rcOrg.left, rcDesktop.top,
		rcOrg.right - rcOrg.left, rcDesktop.bottom - rcDesktop.top,
		SWP_NOOWNERZORDER | SWP_NOZORDER
	);
	return;
}


// 横方向に最大化
void ViewCommander::Command_Maximize_H(void)
{
	RECT rcOrg;
	RECT rcDesktop;

	HWND hwndFrame = GetMainWindow();
	::GetWindowRect(hwndFrame, &rcOrg);
	// マルチモニタ対応
	::GetMonitorWorkRect(hwndFrame, &rcDesktop);
	::SetWindowPos(
		hwndFrame, 0,
		rcDesktop.left, rcOrg.top,
		rcDesktop.right - rcDesktop.left, rcOrg.bottom - rcOrg.top,
		SWP_NOOWNERZORDER | SWP_NOZORDER
	);
	return;
}

// すべて最小化
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

// 再描画
void ViewCommander::Command_Redraw(void)
{
	// フォーカス移動時の再描画
	view.RedrawAll();
	return;
}


// アウトプットウィンドウ表示
void ViewCommander::Command_Win_Output(void)
{
	// メッセージ表示ウィンドウをViewから親に変更
	// TraceOut経由ではCODE_UNICODE,こちらではCODE_SJISだったのを無指定に変更
	ShareData::getInstance().OpenDebugWindow(GetMainWindow(), true);
	return;
}


/*!	@brief マクロ用アウトプットウィンドウに表示 */
void ViewCommander::Command_TraceOut(const wchar_t* outputstr, int nLen, int nFlgOpt)
{
	if (!outputstr)
		return;

	// 0x01 ExpandParameterによる文字列展開有無
	if (nFlgOpt & 0x01) {
		wchar_t Buffer[2048];
		SakuraEnvironment::ExpandParameter(outputstr, Buffer, 2047);
		ShareData::getInstance().TraceOutString(Buffer);
	}else {
		ShareData::getInstance().TraceOutString(outputstr, nLen);
	}

	// 0x02 改行コードの有無
	if ((nFlgOpt & 0x02) == 0) {
		ShareData::getInstance().TraceOutString(L"\r\n");
	}

}

