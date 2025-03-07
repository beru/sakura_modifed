#include "StdAfx.h"
#include "env/AppNodeManager.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "env/SakuraEnvironment.h"
#include "recent/RecentEditNode.h"
#include "util/window.h"
#include "_main/Mutex.h"


// GetOpenedWindowArr用静的変数／構造体
static bool s_bSort;	// ソート指定
static bool s_bGSort;	// グループ指定

/*! @brief CShareData::pEditArr保護用Mutex

	複数のエディタが非同期に一斉動作しているときでも、CShareData::pEditArrを
	安全に操作できるよう操作中はMutexをLock()する。

	@par（非同期一斉動作の例）
		多数のウィンドウを表示していてグループ化を有効にしたタスクバーで「グループを閉じる」操作をしたとき

	@par（保護する箇所の例）
		CShareData::AddEditWndList(): エントリの追加／並び替え
		CShareData::DeleteEditWndList(): エントリの削除
		CShareData::GetOpenedWindowArr(): 配列のコピー作成

	下手にどこにでも入れるとデッドロックする危険があるので入れるときは慎重に。
	（Lock()期間中にSendMessage()などで他ウィンドウの操作をすると危険性大）
	CShareData::pEditArrを直接参照したり変更するような箇所には潜在的な危険があるが、
	対話型で順次操作している範囲であればまず問題は起きない。
*/
static Mutex g_editArrMutex(FALSE, GSTR_MUTEX_SAKURA_EDITARR);

// GetOpenedWindowArr用ソート関数
static bool __cdecl cmpGetOpenedWindowArr(const EditNodeEx& e1, const EditNodeEx& e2)
{
	// 異なるグループのときはグループ比較する
	int nGroup1;
	int nGroup2;

	if (s_bGSort) {
		// オリジナルのグループ番号のほうを見る
		nGroup1 = e1.p->nGroup;
		nGroup2 = e2.p->nGroup;
	}else {
		// グループのMRU番号のほうを見る
		nGroup1 = e1.nGroupMru;
		nGroup2 = e2.nGroupMru;
	}
	if (nGroup1 != nGroup2) {
		return nGroup1 < nGroup2;	// グループ比較
	}

	// グループ比較が行われなかったときはウィンドウ比較する
	if (s_bSort) {
		return e1.p->nIndex < e2.p->nIndex;	// ウィンドウ番号比較
	}
	return e1.p < e2.p;	// ウィンドウMRU比較（ソートしない）
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         グループ                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/** 指定位置の編集ウィンドウ情報を取得する */
EditNode* AppNodeGroupHandle::GetEditNodeAt(size_t nIndex)
{
	auto& nodes = GetDllShareData().nodes;
	size_t iIndex = 0;
	for (size_t i=0; i<nodes.nEditArrNum; ++i) {
		if (nGroup == 0 || nGroup == nodes.pEditArr[i].nGroup) {
			if (IsSakuraMainWindow(nodes.pEditArr[i].hWnd)) {
				if (iIndex == nIndex) {
					return &nodes.pEditArr[i];
				}
				++iIndex;
			}
		}
	}
	return NULL;
}


/** 編集ウィンドウリストへの登録

	@param hWnd   [in] 登録する編集ウィンドウのハンドル
*/
bool AppNodeGroupHandle::AddEditWndList(HWND hWnd)
{
	auto& nodes = GetDllShareData().nodes;

	TabWndNotifyType subCommand = TabWndNotifyType::Add;
	EditNode editNode = {0};
	editNode.hWnd = hWnd;

	{
		LockGuard<Mutex> guard(g_editArrMutex);

		RecentEditNode	recentEditNode;

		// 登録済みか？
		int nIndex = recentEditNode.FindItemByHwnd(hWnd);
		if (nIndex != -1) {
			// もうこれ以上登録できないか？
			if (recentEditNode.GetItemCount() >= recentEditNode.GetArrayCount()) {
				recentEditNode.Terminate();
				return false;
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
			nodes.nSequences++;
			::SetWindowLongPtr(hWnd, sizeof(LONG_PTR) , (LONG_PTR)nodes.nSequences);

			// 連番を更新する。
			editNode.nIndex = nodes.nSequences;
			editNode.nId = -1;

			// タブグループ連番
			if (nGroup > 0) {
				editNode.nGroup = nGroup;	// 指定のグループ
				if (nodes.nGroupSequences < nGroup) {
					// 指定グループが現在のGroup Sequencesを超えていた場合の補正
					nodes.nGroupSequences = nGroup;
				}
			}else {
				EditNode* p = recentEditNode.GetItem(0);
				if (!p) {
					editNode.nGroup = ++nodes.nGroupSequences;	// 新規グループ
				}else {
					editNode.nGroup = p->nGroup;	// 最近アクティブのグループ
				}
			}

			editNode.showCmdRestore = ::IsZoomed(hWnd)? SW_SHOWMAXIMIZED: SW_SHOWNORMAL;
			editNode.bClosing = false;
		}

		// 追加または先頭に移動する。
		recentEditNode.AppendItem(&editNode);
		recentEditNode.Terminate();
	}	// Lock領域終わり

	// ウィンドウ登録メッセージをブロードキャストする。
	AppNodeGroupHandle(hWnd).PostMessageToAllEditors(MYWM_TAB_WINDOW_NOTIFY, (WPARAM)subCommand, (LPARAM)hWnd, hWnd);

	return true;
}


/** 編集ウィンドウリストからの削除 */
void AppNodeGroupHandle::DeleteEditWndList(HWND hWnd)
{
	// ウィンドウをリストから削除する
	{
		LockGuard<Mutex> guard(g_editArrMutex);

		RecentEditNode	recentEditNode;
		recentEditNode.DeleteItemByHwnd(hWnd);
		recentEditNode.Terminate();
	}

	// ウィンドウ削除メッセージをブロードキャストする。
	AppNodeGroupHandle(nGroup).PostMessageToAllEditors(MYWM_TAB_WINDOW_NOTIFY, (WPARAM)TabWndNotifyType::Delete, (LPARAM)hWnd, hWnd);
}

/** いくつかのウィンドウへ終了要求を出す

	@param pWndArr [in] EditNodeの配列。hWndがNULLの要素は処理しない
	@param nArrCnt [in] pWndArrの長さ
	@param bExit [in] true: 編集の全終了 / false: すべて閉じる
	@param bCheckConfirm [in] false:複数ウィンドウを閉じるときの警告を出さない / true:警告を出す（設定による）
	@param hWndFrom [in] 終了要求元のウィンドウ（警告メッセージの親となる）
*/
bool AppNodeGroupHandle::RequestCloseEditor(EditNode* pWndArr, size_t nArrCnt, bool bExit, bool bCheckConfirm, HWND hWndFrom)
{
	// クローズ対象ウィンドウを調べる
	int iGroup = -1;
	HWND hWndLast = NULL;
	int nCloseCount = 0;
	for (size_t i=0; i<nArrCnt; ++i) {
		auto& wnd = pWndArr[i];
		if (nGroup == 0 || nGroup == wnd.nGroup) {
			if (IsSakuraMainWindow(wnd.hWnd)) {
				++nCloseCount;
				if (iGroup == -1) {
					iGroup = wnd.nGroup;	// 最初に閉じるグループ
					hWndLast = wnd.hWnd;
				}else if (iGroup == wnd.nGroup) {
					hWndLast = wnd.hWnd;	// 最初に閉じるグループの最後のウィンドウ
				}
			}
		}
	}

	if (bCheckConfirm && GetDllShareData().common.general.bCloseAllConfirm) {	// [すべて閉じる]で他に編集用のウィンドウがあれば確認する
		if (1 < nCloseCount) {
			if (::MYMESSAGEBOX(
				hWndFrom,
				MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION,
				GSTR_APPNAME,
				LS(STR_ERR_CSHAREDATA19)
				) != IDYES
			) {
				return false;
			}
		}
	}

	// アクティブ化制御ウィンドウを決める
	// ・メッセージを表示していない間はこの制御ウィンドウをアクティブに保つようにする
	// ・閉じられるエディタが保存確認のメッセージを表示する場合は、この制御ウィンドウにアクティブ化要求（MYWM_ALLOWACTIVATE）を出してアクティブにしてもらう
	// ・タブグループ表示かどうかなどの条件に応じて、ちらつきを最小限にするのに都合の良いウィンドウをここで選択しておく
	HWND hWndActive;
	bool bTabGroup = (GetDllShareData().common.tabBar.bDispTabWnd && !GetDllShareData().common.tabBar.bDispTabWndMultiWin);
	if (bTabGroup) {
		hWndActive = hWndLast;	// 最後に閉じるウィンドウが担当
	}else {
		hWndActive = GetDllShareData().handles.hwndTray;	// タスクトレイが担当
	}

	// アクティブ化制御ウィンドウをアクティブにしておく
	if (IsSakuraMainWindow(hWndActive)) {
		ActivateFrameWindow(hWndActive);	// エディタウィンドウ
	}else {
		::SetForegroundWindow(hWndActive);	// タスクトレイ
	}

	// エディタへの終了要求
	for (size_t i=0; i<nArrCnt; ++i) {
		auto& wnd = pWndArr[i];
		if (nGroup == 0 || nGroup == wnd.nGroup) {
			if (IsSakuraMainWindow(wnd.hWnd)) {
				// タブグループ表示で次に閉じるのがアクティブ化制御ウィンドウの場合、
				// アクティブ化制御ウィンドウを次のグループの最後のウィンドウに切替える
				if (bTabGroup && wnd.hWnd == hWndActive) {
					iGroup = -1;
					hWndActive = IsSakuraMainWindow(hWndFrom) ? hWndFrom: NULL;	// 一番最後用
					for (size_t j=i+1; j<nArrCnt; ++j) {
						auto& wnd2 = pWndArr[j];
						if (nGroup == 0 || nGroup == wnd2.nGroup) {
							if (IsSakuraMainWindow(wnd2.hWnd)) {
								if (iGroup == -1) {
									iGroup = wnd2.nGroup;	// 次に閉じるグループ
									hWndActive = wnd2.hWnd;
								}else if (iGroup == wnd2.nGroup) {
									hWndActive = wnd2.hWnd;	// 次に閉じるグループの最後のウィンドウ
								}else {
									break;
								}
							}
						}
					}
				}
				DWORD dwPid;
				::GetWindowThreadProcessId(wnd.hWnd, &dwPid);
				::SendMessage(hWndActive, MYWM_ALLOWACTIVATE, dwPid, 0);	// アクティブ化の許可を依頼する
				if (!::SendMessage(wnd.hWnd, MYWM_CLOSE, bExit ? PM_CLOSE_EXIT : 0, (LPARAM)hWndActive)) {
					return false;
				}
			}
		}
	}

	return true;
}


/** 現在の編集ウィンドウの数を調べる

	@param bExcludeClosing [in] 終了中の編集ウィンドウはカウントしない
*/
int AppNodeGroupHandle::GetEditorWindowsNum(bool bExcludeClosing/* = true */)
{
	auto& nodes = GetDllShareData().nodes;
	int cnt = 0;
	auto& appNodeMgr = AppNodeManager::getInstance();
	for (size_t i=0; i<nodes.nEditArrNum; ++i) {
		auto& node = nodes.pEditArr[i];
		if (IsSakuraMainWindow(node.hWnd)) {
			if (1
				&& nGroup != 0
				&& nGroup != appNodeMgr.GetEditNode(node.hWnd)->GetGroup()
			) {
				continue;
			}
			if (1
				&& bExcludeClosing
				&& node.bClosing
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
	size_t n = AppNodeManager::getInstance().GetOpenedWindowArr(&pWndArr, false);
	if (n == 0) {
		return true;
	}

	// hWndLast以外へのメッセージ
	for (size_t i=0; i<n; ++i) {
		auto& node = pWndArr[i];
		if (!hWndLast || node.hWnd != hWndLast) {
			if (nGroup == 0 || nGroup == node.nGroup) {
				if (IsSakuraMainWindow(node.hWnd)) {
					// メッセージをポスト
					func(node.hWnd, uMsg, wParam, lParam);
				}
			}
		}
	}

	// hWndLastへのメッセージ
	for (size_t i=0; i<n; ++i) {
		auto& node = pWndArr[i];
		if (hWndLast == node.hWnd) {
			if (nGroup == 0 || nGroup == node.nGroup) {
				if (IsSakuraMainWindow(node.hWnd)) {
					// メッセージをポスト
					func(node.hWnd, uMsg, wParam, lParam);
				}
			}
		}
	}

	delete[] pWndArr;
	return true;
}

/** 全編集ウィンドウへメッセージをポストする */
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
		nGroup,
		::PostMessage
	);
}

/** 全編集ウィンドウへメッセージを送る */
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
		nGroup,
		::SendMessage
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        マネージャ                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/** グループをIDリセットする */
void AppNodeManager::ResetGroupId()
{
	auto& nodes = GetDllShareData().nodes;
	int nGroup = ++nodes.nGroupSequences;
	for (size_t i=0; i<nodes.nEditArrNum; ++i) {
		auto& node = nodes.pEditArr[i];
		if (IsSakuraMainWindow(node.hWnd)) {
			node.nGroup = nGroup;
		}
	}
}

/** 編集ウィンドウ情報を取得する

	@warning この関数はpEditArr内の要素へのポインタを返す．
	pEditArrが変更された後ではアクセスしないよう注意が必要．

	@note NULLを返す場合があるので戻り値のチェックが必要です
*/
EditNode* AppNodeManager::GetEditNode(HWND hWnd)
{
	auto& nodes = GetDllShareData().nodes;
	for (size_t i=0; i<nodes.nEditArrNum; ++i) {
		auto& node = nodes.pEditArr[i];
		if (hWnd == node.hWnd) {
			if (IsSakuraMainWindow(node.hWnd)) {
				return &node;
			}
		}
	}
	return nullptr;
}


// 無題番号取得
int AppNodeManager::GetNoNameNumber(HWND hWnd)
{
	auto& nodes = GetDllShareData().nodes;
	EditNode* editNode = GetEditNode(hWnd);
	if (editNode) {
		if (editNode->nId == -1) {
			nodes.nNonameSequences++;
			editNode->nId = nodes.nNonameSequences;
		}
		return editNode->nId;
	}
	return -1;
}


/** 現在開いている編集ウィンドウの配列を返す

	@param[out] ppEditNode 配列を受け取るポインタ
		戻り値が0の場合はNULLが返されるが，それを期待しないこと．
		また，不要になったらdelete[]しなくてはならない．
	@param[in] bSort true: ソートあり / false: ソート無し
	@param[in] bGSort true: グループソートあり / false: グループソート無し

	もとの編集ウィンドウリストはソートしなければウィンドウのMRU順に並んでいる
	-------------------------------------------------
	bSort	bGSort	処理結果
	-------------------------------------------------
	false	false	グループMRU順−ウィンドウMRU順
	true	false	グループMRU順−ウィンドウ番号順
	false	true	グループ番号順−ウィンドウMRU順
	true	true	グループ番号順−ウィンドウ番号順
	-------------------------------------------------

	@return 配列の要素数を返す
	@note 要素数>0 の場合は呼び出し側で配列をdelete[]してください
*/
size_t AppNodeManager::GetOpenedWindowArr(EditNode** ppEditNode, bool bSort, bool bGSort/* = false */)
{
	LockGuard<Mutex> guard(g_editArrMutex);
	size_t nRet = _GetOpenedWindowArrCore(ppEditNode, bSort, bGSort);
	return nRet;
}

// GetOpenedWindowArr関数コア処理部
size_t AppNodeManager::_GetOpenedWindowArrCore(EditNode** ppEditNode, bool bSort, bool bGSort/* = false */)
{
	auto& nodes = GetDllShareData().nodes;

	// 編集ウィンドウ数を取得する。
	*ppEditNode = nullptr;
	if (nodes.nEditArrNum <= 0) {
		return 0;
	}

	// 編集ウィンドウリスト格納領域を作成する。
	*ppEditNode = new EditNode[nodes.nEditArrNum];
	if (!(*ppEditNode)) {
		return 0;
	}

	// 拡張リストを作成する
	// ソート処理用の拡張リスト
	std::vector<EditNodeEx> nodesEx(nodes.nEditArrNum);
	EditNodeEx*	pNode = &nodesEx[0];
	// 拡張リストの各要素に編集ウィンドウリストの各要素へのポインタを格納する
	size_t nRowNum = 0;	// 編集ウィンドウ数
	for (size_t i=0; i<nodes.nEditArrNum; ++i) {
		auto& node = nodes.pEditArr[i];
		if (IsSakuraMainWindow(node.hWnd)) {
			pNode[nRowNum].p = &node;	// ポインタ格納
			pNode[nRowNum].nGroupMru = -1;	// グループ単位のMRU番号初期化
			++nRowNum;
		}
	}
	if (nRowNum == 0) {
		delete[] (*ppEditNode);
		*ppEditNode = nullptr;
		return 0;
	}

	// 拡張リスト上でグループ単位のMRU番号をつける
	if (!bGSort) {
		int iGroupMru = 0;	// グループ単位のMRU番号
		int nGroup = -1;
		for (size_t i=0; i<nRowNum; ++i) {
			if (pNode[i].nGroupMru == -1
				&& nGroup != pNode[i].p->nGroup
			) {
				nGroup = pNode[i].p->nGroup;
				++iGroupMru;
				pNode[i].nGroupMru = iGroupMru;	// MRU番号付与
				// 同一グループのウィンドウに同じMRU番号をつける
				for (size_t j=i+1; j<nRowNum; ++j) {
					if (pNode[j].p->nGroup == nGroup)
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
	std::sort(nodesEx.begin(), nodesEx.begin()+nRowNum, cmpGetOpenedWindowArr);

	// 拡張リストのソート結果をもとに編集ウィンドウリスト格納領域に結果を格納する
	for (size_t i=0; i<nRowNum; ++i) {
		(*ppEditNode)[i] = *pNode[i].p;

		// インデックスを付ける。
		// このインデックスは pEditArr の配列番号です。
		(*ppEditNode)[i].nIndex = pNode[i].p - nodes.pEditArr;	// ポインタ減算＝配列番号
	}

	return nRowNum;
}

/** ウィンドウの並び替え

	@param[in] hwndSrc 移動するウィンドウ
	@param[in] hwndDst 移動先ウィンドウ
*/
bool AppNodeManager::ReorderTab(HWND hwndSrc, HWND hwndDst)
{
	auto& nodes = GetDllShareData().nodes;
	EditNode* p = nullptr;
	int nSrcTab = -1;
	int nDstTab = -1;
	LockGuard<Mutex> guard(g_editArrMutex);
	size_t nCount = _GetOpenedWindowArrCore(&p, true);	// ロックは自分でやっているので直接コア部呼び出し
	for (size_t i=0; i<nCount; ++i) {
		if (hwndSrc == p[i].hWnd) {
			nSrcTab = (int)i;
		}
		if (hwndDst == p[i].hWnd) {
			nDstTab = (int)i;
		}
	}

	if (0 > nSrcTab || 0 > nDstTab || nSrcTab == nDstTab) {
		if (p) {
			delete[] p;
		}
		return false;
	}

	// タブの順序を入れ替えるためにウィンドウのインデックスを入れ替える
	ptrdiff_t nArr0, nArr1;
	ptrdiff_t nIndex;

	nArr0 = p[nDstTab].nIndex;
	nIndex = nodes.pEditArr[nArr0].nIndex;
	if (nSrcTab < nDstTab) {
		// タブ左方向ローテート
		for (int i=nDstTab-1; i>=nSrcTab; --i) {
			nArr1 = p[i].nIndex;
			nodes.pEditArr[nArr0].nIndex = nodes.pEditArr[nArr1].nIndex;
			nArr0 = nArr1;
		}
	}else {
		// タブ右方向ローテート
		for (int i=nDstTab+1; i<=nSrcTab; ++i) {
			nArr1 = p[i].nIndex;
			nodes.pEditArr[nArr0].nIndex = nodes.pEditArr[nArr1].nIndex;
			nArr0 = nArr1;
		}
	}
	nodes.pEditArr[nArr0].nIndex = nIndex;

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
*/
HWND AppNodeManager::SeparateGroup(HWND hwndSrc, HWND hwndDst, bool bSrcIsTop, int notifygroups[])
{
    auto& nodes = GetDllShareData().nodes;

	LockGuard<Mutex> guard(g_editArrMutex);

	EditNode* pSrcEditNode = GetEditNode(hwndSrc);
	EditNode* pDstEditNode = GetEditNode(hwndDst);
	int nSrcGroup = pSrcEditNode->nGroup;
	int nDstGroup;
	if (!pDstEditNode) {
		hwndDst = NULL;
		nDstGroup = ++nodes.nGroupSequences;	// 新規グループ
	}else {
		nDstGroup = pDstEditNode->nGroup;	// 既存グループ
	}

	pSrcEditNode->nGroup = nDstGroup;
	pSrcEditNode->nIndex = ++nodes.nSequences;	// タブ並びの最後（起動順の最後）にもっていく

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
*/
bool AppNodeManager::IsSameGroup(HWND hWnd1, HWND hWnd2)
{
	if (hWnd1 == hWnd2) {
		return true;
	}
	auto& nodeMgr = AppNodeManager::getInstance();
	AppNodeGroupHandle group1 = nodeMgr.GetEditNode(hWnd1)->GetGroup();
	AppNodeGroupHandle group2 = nodeMgr.GetEditNode(hWnd2)->GetGroup();
	if (group1.IsValidGroup() && group1 == group2) {
		return true;
	}

	return false;
}

// 空いているグループ番号を取得する
int AppNodeManager::GetFreeGroupId(void)
{
	return ++GetDllShareData().nodes.nGroupSequences;	// 新規グループ
}

// Close した時の次のWindowを取得する
//  (タブまとめ表示の場合)
//
// @param hWndCur [in] Close対象のウィンドウハンドル
// @retval クローズ後移動するウィンドウ
//			NULLはタブまとめ表示で無いかグループに他にウィンドウが無い場合
//
HWND AppNodeManager::GetNextTab(HWND hWndCur)
{
	HWND hWnd = NULL;
	auto& tabBar = GetDllShareData().common.tabBar;
	if (1
		&& tabBar.bDispTabWnd
		&& !tabBar.bDispTabWndMultiWin
	) {
		int nGroup = 0;
		bool bFound = false;
		EditNode* p = nullptr;
		size_t nCount = AppNodeManager::getInstance().GetOpenedWindowArr(&p, false, false);
		if (nCount > 1) {
			// search Group No.
			for (size_t i=0; i<nCount; ++i) {
				if (p[i].GetHwnd() == hWndCur) {
					nGroup = p[i].nGroup;
					break;
				}
			}
			// Search Next Window
			for (size_t i=0; i<nCount; ++i) {
				if (p[i].nGroup == nGroup) {
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
			delete[] p;
		}
	}

	return hWnd;
}

