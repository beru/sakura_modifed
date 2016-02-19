/*!	@file
	@brief アプリケーションノードマネージャ

	@author kobake
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2009, syat
	Copyright (C) 2011, syat
	Copyright (C) 2012, syat, Uchi
	Copyright (C) 2013, Moca, Uchi

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
#include "env/AppNodeManager.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "env/SakuraEnvironment.h"
#include "recent/RecentEditNode.h"
#include "util/window.h"
#include "_main/Mutex.h"


// GetOpenedWindowArr用静的変数／構造体
static BOOL s_bSort;	// ソート指定
static BOOL s_bGSort;	// グループ指定

/*! @brief CShareData::m_pEditArr保護用Mutex

	複数のエディタが非同期に一斉動作しているときでも、CShareData::m_pEditArrを
	安全に操作できるよう操作中はMutexをLock()する。

	@par（非同期一斉動作の例）
		多数のウィンドウを表示していてグループ化を有効にしたタスクバーで「グループを閉じる」操作をしたとき

	@par（保護する箇所の例）
		CShareData::AddEditWndList(): エントリの追加／並び替え
		CShareData::DeleteEditWndList(): エントリの削除
		CShareData::GetOpenedWindowArr(): 配列のコピー作成

	下手にどこにでも入れるとデッドロックする危険があるので入れるときは慎重に。
	（Lock()期間中にSendMessage()などで他ウィンドウの操作をすると危険性大）
	CShareData::m_pEditArrを直接参照したり変更するような箇所には潜在的な危険があるが、
	対話型で順次操作している範囲であればまず問題は起きない。

	@date 2007.07.05 ryoji 新規導入
	@date 2007.07.07 genta CShareDataのメンバへ移動
*/
static Mutex g_editArrMutex(FALSE, GSTR_MUTEX_SAKURA_EDITARR);

// GetOpenedWindowArr用ソート関数
static int __cdecl cmpGetOpenedWindowArr(const void *e1, const void *e2)
{
	// 異なるグループのときはグループ比較する
	int nGroup1;
	int nGroup2;

	if (s_bGSort) {
		// オリジナルのグループ番号のほうを見る
		nGroup1 = ((EditNodeEx*)e1)->p->m_nGroup;
		nGroup2 = ((EditNodeEx*)e2)->p->m_nGroup;
	}else {
		// グループのMRU番号のほうを見る
		nGroup1 = ((EditNodeEx*)e1)->nGroupMru;
		nGroup2 = ((EditNodeEx*)e2)->nGroupMru;
	}
	if (nGroup1 != nGroup2) {
		return nGroup1 - nGroup2;	// グループ比較
	}

	// グループ比較が行われなかったときはウィンドウ比較する
	if (s_bSort) {
		return (((EditNodeEx*)e1)->p->m_nIndex - ((EditNodeEx*)e2)->p->m_nIndex);	// ウィンドウ番号比較
	}
	return (((EditNodeEx*)e1)->p - ((EditNodeEx*)e2)->p);	// ウィンドウMRU比較（ソートしない）
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         グループ                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/** 指定位置の編集ウィンドウ情報を取得する

	@date 2007.06.20 ryoji
*/
EditNode* AppNodeGroupHandle::GetEditNodeAt(int nIndex)
{
	DllSharedData* pShare = &GetDllShareData();

	int iIndex = 0;
	for (int i = 0; i < pShare->m_nodes.m_nEditArrNum; ++i) {
		if (m_nGroup == 0 || m_nGroup == pShare->m_nodes.m_pEditArr[i].m_nGroup) {
			if (IsSakuraMainWindow(pShare->m_nodes.m_pEditArr[i].m_hWnd)) {
				if (iIndex == nIndex)
					return &pShare->m_nodes.m_pEditArr[i];
				++iIndex;
			}
		}
	}

	return NULL;
}


/** 編集ウィンドウリストへの登録

	@param hWnd   [in] 登録する編集ウィンドウのハンドル

	@date 2003.06.28 MIK CRecent利用で書き換え
	@date 2007.06.20 ryoji 新規ウィンドウにはグループIDを付与する
*/
BOOL AppNodeGroupHandle::AddEditWndList(HWND hWnd)
{
	DllSharedData* pShare = &GetDllShareData();

	TabWndNotifyType subCommand = TabWndNotifyType::Add;
	EditNode editNode = {0};
	editNode.m_hWnd = hWnd;

	{	// 2007.07.07 genta Lock領域
		LockGuard<Mutex> guard(g_editArrMutex);

		RecentEditNode	recentEditNode;

		// 登録済みか？
		int nIndex = recentEditNode.FindItemByHwnd(hWnd);
		if (nIndex != -1) {
			// もうこれ以上登録できないか？
			if (recentEditNode.GetItemCount() >= recentEditNode.GetArrayCount()) {
				recentEditNode.Terminate();
				return FALSE;
			}
			subCommand = TabWndNotifyType::Reorder;

			// 以前の情報をコピーする。
			EditNode* p = recentEditNode.GetItem(nIndex);
			if (p) {
				editNode = *p;
			}
		}

		// ウィンドウ連番
		if (::GetWindowLongPtr(hWnd, sizeof(LONG_PTR)) == 0) {
			pShare->m_nodes.m_nSequences++;
			::SetWindowLongPtr(hWnd, sizeof(LONG_PTR) , (LONG_PTR)pShare->m_nodes.m_nSequences);

			// 連番を更新する。
			editNode.m_nIndex = pShare->m_nodes.m_nSequences;
			editNode.m_nId = -1;

			// タブグループ連番
			if (m_nGroup > 0) {
				editNode.m_nGroup = m_nGroup;	// 指定のグループ
				if (pShare->m_nodes.m_nGroupSequences < m_nGroup) {
					// 指定グループが現在のGroup Sequencesを超えていた場合の補正
					pShare->m_nodes.m_nGroupSequences = m_nGroup;
				}
			}else {
				EditNode* p = recentEditNode.GetItem(0);
				if (!p) {
					editNode.m_nGroup = ++pShare->m_nodes.m_nGroupSequences;	// 新規グループ
				}else {
					editNode.m_nGroup = p->m_nGroup;	// 最近アクティブのグループ
				}
			}

			editNode.m_showCmdRestore = ::IsZoomed(hWnd)? SW_SHOWMAXIMIZED: SW_SHOWNORMAL;
			editNode.m_bClosing = FALSE;
		}

		// 追加または先頭に移動する。
		recentEditNode.AppendItem(&editNode);
		recentEditNode.Terminate();
	}	// 2007.07.07 genta Lock領域終わり

	// ウィンドウ登録メッセージをブロードキャストする。
	AppNodeGroupHandle(hWnd).PostMessageToAllEditors(MYWM_TAB_WINDOW_NOTIFY, (WPARAM)subCommand, (LPARAM)hWnd, hWnd);

	return TRUE;
}


/** 編集ウィンドウリストからの削除

	@date 2003.06.28 MIK CRecent利用で書き換え
	@date 2007.07.05 ryoji mutexで保護
*/
void AppNodeGroupHandle::DeleteEditWndList(HWND hWnd)
{
	// ウィンドウをリストから削除する。
	{	// 2007.07.07 genta Lock領域
		LockGuard<Mutex> guard(g_editArrMutex);

		RecentEditNode	recentEditNode;
		recentEditNode.DeleteItemByHwnd(hWnd);
		recentEditNode.Terminate();
	}

	// ウィンドウ削除メッセージをブロードキャストする。
	AppNodeGroupHandle(m_nGroup).PostMessageToAllEditors(MYWM_TAB_WINDOW_NOTIFY, (WPARAM)TabWndNotifyType::Delete, (LPARAM)hWnd, hWnd);
}

/** いくつかのウィンドウへ終了要求を出す

	@param pWndArr [in] EditNodeの配列。m_hWndがNULLの要素は処理しない
	@param nArrCnt [in] pWndArrの長さ
	@param bExit [in] TRUE: 編集の全終了 / FALSE: すべて閉じる
	@param bCheckConfirm [in] FALSE:複数ウィンドウを閉じるときの警告を出さない / TRUE:警告を出す（設定による）
	@param hWndFrom [in] 終了要求元のウィンドウ（警告メッセージの親となる）

	@date 2007.02.13 ryoji 「編集の全終了」を示す引数(bExit)を追加
	@date 2007.06.22 ryoji nGroup引数を追加
	@date 2008.11.22 syat 全て→いくつかに変更。複数ウィンドウを閉じる時の警告メッセージを追加
	@date 2013.03.09 Uchi 終了要求に要求元のウィンドウを渡す
*/
BOOL AppNodeGroupHandle::RequestCloseEditor(EditNode* pWndArr, int nArrCnt, bool bExit, bool bCheckConfirm, HWND hWndFrom)
{
	// クローズ対象ウィンドウを調べる
	int iGroup = -1;
	HWND hWndLast = NULL;
	int nCloseCount = 0;
	for (int i=0; i<nArrCnt; ++i) {
		auto& wnd = pWndArr[i];
		if (m_nGroup == 0 || m_nGroup == wnd.m_nGroup) {
			if (IsSakuraMainWindow(wnd.m_hWnd)) {
				++nCloseCount;
				if (iGroup == -1) {
					iGroup = wnd.m_nGroup;	// 最初に閉じるグループ
					hWndLast = wnd.m_hWnd;
				}else if (iGroup == wnd.m_nGroup) {
					hWndLast = wnd.m_hWnd;	// 最初に閉じるグループの最後のウィンドウ
				}
			}
		}
	}

	if (bCheckConfirm && GetDllShareData().m_common.m_general.m_bCloseAllConfirm) {	// [すべて閉じる]で他に編集用のウィンドウがあれば確認する
		if (1 < nCloseCount) {
			if (::MYMESSAGEBOX(
				hWndFrom,
				MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION,
				GSTR_APPNAME,
				LS(STR_ERR_CSHAREDATA19)
				) != IDYES
			) {
				return FALSE;
			}
		}
	}

	// アクティブ化制御ウィンドウを決める
	// ・メッセージを表示していない間はこの制御ウィンドウをアクティブに保つようにする
	// ・閉じられるエディタが保存確認のメッセージを表示する場合は、この制御ウィンドウにアクティブ化要求（MYWM_ALLOWACTIVATE）を出してアクティブにしてもらう
	// ・タブグループ表示かどうかなどの条件に応じて、ちらつきを最小限にするのに都合の良いウィンドウをここで選択しておく
	HWND hWndActive;
	bool bTabGroup = (GetDllShareData().m_common.m_tabBar.m_bDispTabWnd && !GetDllShareData().m_common.m_tabBar.m_bDispTabWndMultiWin);
	if (bTabGroup) {
		hWndActive = hWndLast;	// 最後に閉じるウィンドウが担当
	}else {
		hWndActive = GetDllShareData().m_handles.m_hwndTray;	// タスクトレイが担当
	}

	// アクティブ化制御ウィンドウをアクティブにしておく
	if (IsSakuraMainWindow(hWndActive)) {
		ActivateFrameWindow(hWndActive);	// エディタウィンドウ
	}else {
		::SetForegroundWindow(hWndActive);	// タスクトレイ
	}

	// エディタへの終了要求
	for (int i=0; i<nArrCnt; ++i) {
		auto& wnd = pWndArr[i];
		if (m_nGroup == 0 || m_nGroup == wnd.m_nGroup) {
			if (IsSakuraMainWindow(wnd.m_hWnd)) {
				// タブグループ表示で次に閉じるのがアクティブ化制御ウィンドウの場合、
				// アクティブ化制御ウィンドウを次のグループの最後のウィンドウに切替える
				if (bTabGroup && wnd.m_hWnd == hWndActive) {
					iGroup = -1;
					hWndActive = IsSakuraMainWindow(hWndFrom) ? hWndFrom: NULL;	// 一番最後用
					for (int j=i+1; j<nArrCnt; ++j) {
						auto& wnd2 = pWndArr[j];
						if (m_nGroup == 0 || m_nGroup == wnd2.m_nGroup) {
							if (IsSakuraMainWindow(wnd2.m_hWnd)) {
								if (iGroup == -1) {
									iGroup = wnd2.m_nGroup;	// 次に閉じるグループ
									hWndActive = wnd2.m_hWnd;
								}else if (iGroup == wnd2.m_nGroup) {
									hWndActive = wnd2.m_hWnd;	// 次に閉じるグループの最後のウィンドウ
								}else {
									break;
								}
							}
						}
					}
				}
				DWORD dwPid;
				::GetWindowThreadProcessId(wnd.m_hWnd, &dwPid);
				::SendMessage(hWndActive, MYWM_ALLOWACTIVATE, dwPid, 0);	// アクティブ化の許可を依頼する
				if (!::SendMessage(wnd.m_hWnd, MYWM_CLOSE, bExit ? PM_CLOSE_EXIT : 0, (LPARAM)hWndActive)) {	// 2007.02.13 ryoji bExitを引き継ぐ
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}


/** 現在の編集ウィンドウの数を調べる

	@param bExcludeClosing [in] 終了中の編集ウィンドウはカウントしない

	@date 2007.06.22 ryoji nGroup引数を追加
	@date 2008.04.19 ryoji bExcludeClosing引数を追加
*/
int AppNodeGroupHandle::GetEditorWindowsNum(bool bExcludeClosing/* = true */)
{
	DllSharedData* pShare = &GetDllShareData();
	int cnt = 0;
	auto appNodeMgr = AppNodeManager::getInstance();
	for (int i=0; i<pShare->m_nodes.m_nEditArrNum; ++i) {
		auto& node = pShare->m_nodes.m_pEditArr[i];
		if (IsSakuraMainWindow(node.m_hWnd)) {
			if (1
				&& m_nGroup != 0
				&& m_nGroup != appNodeMgr->GetEditNode(node.m_hWnd)->GetGroup()
			) {
				continue;
			}
			if (1
				&& bExcludeClosing
				&& node.m_bClosing
			) {
				continue;
			}
			++cnt;
		}
	}
	return cnt;
}

template <typename Func>
bool relayMessageToAllEditors(
	UINT		uMsg,		// 送るメッセージ
	WPARAM		wParam,		// 第1メッセージ パラメータ
	LPARAM		lParam,		// 第2メッセージ パラメータ
	HWND		hWndLast,	// 最後に送りたいウィンドウ
	int			nGroup,		// 指定グループ
	Func		func
	)
{
	EditNode* pWndArr;
	int n = AppNodeManager::getInstance()->GetOpenedWindowArr(&pWndArr, FALSE);
	if (n == 0) {
		return true;
	}

	// hWndLast以外へのメッセージ
	for (int i=0; i<n; ++i) {
		// Jan. 24, 2005 genta hWndLast == NULLのときにメッセージが送られるように
		auto& node = pWndArr[i];
		if (!hWndLast || node.m_hWnd != hWndLast) {
			if (nGroup == 0 || nGroup == node.m_nGroup) {
				if (IsSakuraMainWindow(node.m_hWnd)) {
					// メッセージをポスト
					func(node.m_hWnd, uMsg, wParam, lParam);
				}
			}
		}
	}

	// hWndLastへのメッセージ
	for (int i=0; i<n; ++i) {
		auto& node = pWndArr[i];
		if (hWndLast == node.m_hWnd) {
			if (nGroup == 0 || nGroup == node.m_nGroup) {
				if (IsSakuraMainWindow(node.m_hWnd)) {
					// メッセージをポスト
					func(node.m_hWnd, uMsg, wParam, lParam);
				}
			}
		}
	}

	delete[] pWndArr;
	return true;
}

/** 全編集ウィンドウへメッセージをポストする

	@date 2005.01.24 genta hWndLast == NULLのとき全くメッセージが送られなかった
	@date 2007.06.22 ryoji nGroup引数を追加、グループ単位で順番に送る
*/
bool AppNodeGroupHandle::PostMessageToAllEditors(
	UINT		uMsg,		// ポストするメッセージ
	WPARAM		wParam,		// 第1メッセージ パラメータ
	LPARAM		lParam,		// 第2メッセージ パラメータ
	HWND		hWndLast	// 最後に送りたいウィンドウ
	)
{
	return relayMessageToAllEditors(
		uMsg,
		wParam,
		lParam,
		hWndLast,
		m_nGroup,
		::PostMessage
	);
}

/** 全編集ウィンドウへメッセージを送る

	@date 2005.01.24 genta m_hWndLast == NULLのとき全くメッセージが送られなかった
	@date 2007.06.22 ryoji nGroup引数を追加、グループ単位で順番に送る
*/
bool AppNodeGroupHandle::SendMessageToAllEditors(
	UINT	uMsg,		// ポストするメッセージ
	WPARAM	wParam,		// 第1メッセージ パラメータ
	LPARAM	lParam,		// 第2メッセージ パラメータ
	HWND	hWndLast	// 最後に送りたいウィンドウ
	)
{
	return relayMessageToAllEditors(
		uMsg,
		wParam,
		lParam,
		hWndLast,
		m_nGroup,
		::SendMessage
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        マネージャ                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/** グループをIDリセットする

	@date 2007.06.20 ryoji
*/
void AppNodeManager::ResetGroupId()
{
	DllSharedData* pShare = &GetDllShareData();
	auto& nodes = pShare->m_nodes;
	int nGroup = ++nodes.m_nGroupSequences;
	for (int i=0; i<nodes.m_nEditArrNum; ++i) {
		auto& node = nodes.m_pEditArr[i];
		if (IsSakuraMainWindow(node.m_hWnd)) {
			node.m_nGroup = nGroup;
		}
	}
}

/** 編集ウィンドウ情報を取得する

	@date 2007.06.20 ryoji

	@warning この関数はm_pEditArr内の要素へのポインタを返す．
	m_pEditArrが変更された後ではアクセスしないよう注意が必要．

	@note NULLを返す場合があるので戻り値のチェックが必要です
*/
EditNode* AppNodeManager::GetEditNode(HWND hWnd)
{
	DllSharedData* pShare = &GetDllShareData();
	auto& nodes = pShare->m_nodes;
	for (int i=0; i<nodes.m_nEditArrNum; ++i) {
		auto& node = nodes.m_pEditArr[i];
		if (hWnd == node.m_hWnd) {
			if (IsSakuraMainWindow(node.m_hWnd)) {
				return &node;
			}
		}
	}
	return nullptr;
}


// 無題番号取得
int AppNodeManager::GetNoNameNumber(HWND hWnd)
{
	DllSharedData* pShare = &GetDllShareData();
	EditNode* editNode = GetEditNode(hWnd);
	if (editNode) {
		if (editNode->m_nId == -1) {
			pShare->m_nodes.m_nNonameSequences++;
			editNode->m_nId = pShare->m_nodes.m_nNonameSequences;
		}
		return editNode->m_nId;
	}
	return -1;
}


/** 現在開いている編集ウィンドウの配列を返す

	@param[out] ppEditNode 配列を受け取るポインタ
		戻り値が0の場合はNULLが返されるが，それを期待しないこと．
		また，不要になったらdelete []しなくてはならない．
	@param[in] bSort TRUE: ソートあり / FALSE: ソート無し
	@param[in]bGSort TRUE: グループソートあり / FALSE: グループソート無し

	もとの編集ウィンドウリストはソートしなければウィンドウのMRU順に並んでいる
	-------------------------------------------------
	bSort	bGSort	処理結果
	-------------------------------------------------
	FALSE	FALSE	グループMRU順－ウィンドウMRU順
	TRUE	FALSE	グループMRU順－ウィンドウ番号順
	FALSE	TRUE	グループ番号順－ウィンドウMRU順
	TRUE	TRUE	グループ番号順－ウィンドウ番号順
	-------------------------------------------------

	@return 配列の要素数を返す
	@note 要素数>0 の場合は呼び出し側で配列をdelete []してください

	@date 2003.06.28 MIK CRecent利用で書き換え
	@date 2007.06.20 ryoji bGroup引数追加、ソート処理を自前のものからqsortに変更
*/
int AppNodeManager::GetOpenedWindowArr(EditNode** ppEditNode, BOOL bSort, BOOL bGSort/* = FALSE */)
{
	LockGuard<Mutex> guard(g_editArrMutex);
	int nRet = _GetOpenedWindowArrCore(ppEditNode, bSort, bGSort);
	return nRet;
}

// GetOpenedWindowArr関数コア処理部
int AppNodeManager::_GetOpenedWindowArrCore(EditNode** ppEditNode, BOOL bSort, BOOL bGSort/* = FALSE */)
{
	DllSharedData* pShare = &GetDllShareData();
	auto& nodes = pShare->m_nodes;

	// 編集ウィンドウ数を取得する。
	*ppEditNode = nullptr;
	if (nodes.m_nEditArrNum <= 0) {
		return 0;
	}

	// 編集ウィンドウリスト格納領域を作成する。
	*ppEditNode = new EditNode[nodes.m_nEditArrNum];
	if (!(*ppEditNode)) {
		return 0;
	}

	// 拡張リストを作成する
	// ソート処理用の拡張リスト
	std::vector<EditNodeEx> nodesEx(nodes.m_nEditArrNum);
	EditNodeEx*	pNode = &nodesEx[0];

	// 拡張リストの各要素に編集ウィンドウリストの各要素へのポインタを格納する
	int nRowNum = 0;	// 編集ウィンドウ数
	for (int i=0; i<nodes.m_nEditArrNum; ++i) {
		auto& node = nodes.m_pEditArr[i];
		if (IsSakuraMainWindow(node.m_hWnd)) {
			pNode[nRowNum].p = &node;	// ポインタ格納
			pNode[nRowNum].nGroupMru = -1;	// グループ単位のMRU番号初期化
			++nRowNum;
		}
	}
	if (nRowNum <= 0) {
		delete [](*ppEditNode);
		*ppEditNode = nullptr;
		return 0;
	}

	// 拡張リスト上でグループ単位のMRU番号をつける
	if (!bGSort) {
		int iGroupMru = 0;	// グループ単位のMRU番号
		int nGroup = -1;
		for (int i=0; i<nRowNum; ++i) {
			if (pNode[i].nGroupMru == -1
				&& nGroup != pNode[i].p->m_nGroup
			) {
				nGroup = pNode[i].p->m_nGroup;
				++iGroupMru;
				pNode[i].nGroupMru = iGroupMru;	// MRU番号付与
				// 同一グループのウィンドウに同じMRU番号をつける
				for (int j=i+1; j<nRowNum; ++j) {
					if (pNode[j].p->m_nGroup == nGroup)
						pNode[j].nGroupMru = iGroupMru;
				}
			}
		}
	}

	// 拡張リストをソートする
	// Note. グループが１個だけの場合は従来（bGSort 引数無し）と同じ結果が得られる
	//       （グループ化する設定でなければグループは１個）
	s_bSort = bSort;
	s_bGSort = bGSort;
	qsort(pNode, nRowNum, sizeof(EditNodeEx), cmpGetOpenedWindowArr);

	// 拡張リストのソート結果をもとに編集ウィンドウリスト格納領域に結果を格納する
	for (int i=0; i<nRowNum; ++i) {
		(*ppEditNode)[i] = *pNode[i].p;

		// インデックスを付ける。
		// このインデックスは m_pEditArr の配列番号です。
		(*ppEditNode)[i].m_nIndex = pNode[i].p - nodes.m_pEditArr;	// ポインタ減算＝配列番号
	}

	return nRowNum;
}

/** ウィンドウの並び替え

	@param[in] hwndSrc 移動するウィンドウ
	@param[in] hwndDst 移動先ウィンドウ

	@author ryoji
	@date 2007.07.07 genta ウィンドウ配列操作部をCTabWndより移動
*/
bool AppNodeManager::ReorderTab(HWND hwndSrc, HWND hwndDst)
{
	DllSharedData* pShare = &GetDllShareData();
	EditNode* p = NULL;
	int nSrcTab = -1;
	int nDstTab = -1;
	LockGuard<Mutex> guard(g_editArrMutex);
	int nCount = _GetOpenedWindowArrCore(&p, TRUE);	// ロックは自分でやっているので直接コア部呼び出し
	for (int i=0; i<nCount; ++i) {
		if (hwndSrc == p[i].m_hWnd) {
			nSrcTab = i;
		}
		if (hwndDst == p[i].m_hWnd) {
			nDstTab = i;
		}
	}

	if (0 > nSrcTab || 0 > nDstTab || nSrcTab == nDstTab) {
		if (p) {
			delete []p;
		}
		return false;
	}

	// タブの順序を入れ替えるためにウィンドウのインデックスを入れ替える
	int nArr0, nArr1;
	int	nIndex;

	nArr0 = p[nDstTab].m_nIndex;
	auto& nodes = pShare->m_nodes;
	nIndex = nodes.m_pEditArr[nArr0].m_nIndex;
	if (nSrcTab < nDstTab) {
		// タブ左方向ローテート
		for (int i=nDstTab-1; i>=nSrcTab; --i) {
			nArr1 = p[i].m_nIndex;
			nodes.m_pEditArr[nArr0].m_nIndex = nodes.m_pEditArr[nArr1].m_nIndex;
			nArr0 = nArr1;
		}
	}else {
		// タブ右方向ローテート
		for (int i=nDstTab+1; i<=nSrcTab; ++i) {
			nArr1 = p[i].m_nIndex;
			nodes.m_pEditArr[nArr0].m_nIndex = nodes.m_pEditArr[nArr1].m_nIndex;
			nArr0 = nArr1;
		}
	}
	nodes.m_pEditArr[nArr0].m_nIndex = nIndex;

	if (p) {
		delete[] p;
	}
	return true;
}

/** タブ移動に伴うウィンドウ処理

	@param[in] hwndSrc 移動するウィンドウ
	@param[in] hwndDst 移動先ウィンドウ．新規独立時はNULL．
	@param[in] bSrcIsTop 移動するウィンドウが可視ウィンドウならtrue
	@param[in] notifygroups タブの更新が必要なグループのグループID．int[2]を呼び出し元で用意する．

	@return 更新されたhwndDst (移動先が既に閉じられた場合などにNULLに変更されることがある)

	@author ryoji
	@date 2007.07.07 genta CTabWnd::SeparateGroup()より独立
*/
HWND AppNodeManager::SeparateGroup(HWND hwndSrc, HWND hwndDst, bool bSrcIsTop, int notifygroups[])
{
	DllSharedData* pShare = &GetDllShareData();

	LockGuard<Mutex> guard(g_editArrMutex);

	EditNode* pSrcEditNode = GetEditNode(hwndSrc);
	EditNode* pDstEditNode = GetEditNode(hwndDst);
	int nSrcGroup = pSrcEditNode->m_nGroup;
	int nDstGroup;
	if (!pDstEditNode) {
		hwndDst = NULL;
		nDstGroup = ++pShare->m_nodes.m_nGroupSequences;	// 新規グループ
	}else {
		nDstGroup = pDstEditNode->m_nGroup;	// 既存グループ
	}

	pSrcEditNode->m_nGroup = nDstGroup;
	pSrcEditNode->m_nIndex = ++pShare->m_nodes.m_nSequences;	// タブ並びの最後（起動順の最後）にもっていく

	// 非表示のタブを既存グループに移動するときは非表示のままにするので
	// 内部情報も先頭にはならないよう、必要なら先頭ウィンドウと位置を交換する。
	if (!bSrcIsTop && pDstEditNode) {
		if (pSrcEditNode < pDstEditNode) {
			EditNode en = *pDstEditNode;
			*pDstEditNode = *pSrcEditNode;
			*pSrcEditNode = en;
		}
	}
	
	notifygroups[0] = nSrcGroup;
	notifygroups[1] = nDstGroup;
	
	return hwndDst;
}


/** 同一グループかどうかを調べる

	@param[in] hWnd1 比較するウィンドウ1
	@param[in] hWnd2 比較するウィンドウ2
	
	@return 2つのウィンドウが同一グループに属していればtrue

	@date 2007.06.20 ryoji
*/
bool AppNodeManager::IsSameGroup(HWND hWnd1, HWND hWnd2)
{
	if (hWnd1 == hWnd2) {
		return true;
	}
	auto* pNodeMgr = AppNodeManager::getInstance();
	AppNodeGroupHandle group1 = pNodeMgr->GetEditNode(hWnd1)->GetGroup();
	AppNodeGroupHandle group2 = pNodeMgr->GetEditNode(hWnd2)->GetGroup();
	if (group1.IsValidGroup() && group1 == group2) {
		return true;
	}

	return false;
}

// 空いているグループ番号を取得する
int AppNodeManager::GetFreeGroupId(void)
{
	DllSharedData* pShare = &GetDllShareData();
	return ++pShare->m_nodes.m_nGroupSequences;	// 新規グループ
}

// Close した時の次のWindowを取得する
//  (タブまとめ表示の場合)
//
// @param hWndCur [in] Close対象のウィンドウハンドル
// @retval クローズ後移動するウィンドウ
//			NULLはタブまとめ表示で無いかグループに他にウィンドウが無い場合
//
// @date 2013.04.10 Uchi
// @date 2013.10.25 Moca 次のウィンドウは「1つ前のアクティブなタブ」にする
//
HWND AppNodeManager::GetNextTab(HWND hWndCur)
{
	HWND hWnd = NULL;
	auto& tabBar = GetDllShareData().m_common.m_tabBar;
	if (1
		&& tabBar.m_bDispTabWnd
		&& !tabBar.m_bDispTabWndMultiWin
	) {
		int			nGroup = 0;
		bool		bFound = false;
		EditNode*	p = nullptr;
		int			nCount = AppNodeManager::getInstance()->GetOpenedWindowArr(&p, FALSE, FALSE);
		if (nCount > 1) {
			// search Group No.
			for (int i=0; i<nCount; ++i) {
				if (p[i].GetHwnd() == hWndCur) {
					nGroup = p[i].m_nGroup;
					break;
				}
			}
			// Search Next Window
			for (int i=0; i<nCount; ++i) {
				if (p[i].m_nGroup == nGroup) {
					if (p[i].GetHwnd() == hWndCur) {
						bFound= true;
					}else {
						if (!bFound && hWnd == NULL || bFound) {
							hWnd = p[i].GetHwnd();
						}
						if (bFound) {
							break;
						}
					}
				}
			}
		}
		if (p) {
			delete []p;
		}
	}

	return hWnd;
}

