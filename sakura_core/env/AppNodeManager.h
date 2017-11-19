#pragma once

#include "util/design_template.h"
#include "config/maxdata.h"

class AppNodeGroupHandle;

// 編集ウィンドウノード
struct EditNode {
	ptrdiff_t		nIndex;
	int				nGroup;						// グループID
	HWND			hWnd;
	int				nId;						// 無題Id
	WIN_CHAR		szTabCaption[_MAX_PATH];	// タブウィンドウ用：キャプション名
	SFilePath		szFilePath;					// タブウィンドウ用：ファイル名
	bool			bIsGrep;					// Grepのウィンドウか
	UINT			showCmdRestore;				// 元のサイズに戻すときのサイズ種別
	bool			bClosing;					// 終了中か（「最後のファイルを閉じても(無題)を残す」用）

	HWND GetHwnd() const { return GetSafeHwnd(); }
	HWND GetSafeHwnd() const { if (this) return hWnd; else return NULL; }
	int GetId() const { return GetSafeId(); }
	int GetSafeId() const { if (this) return nId; else return 0; }
	AppNodeGroupHandle GetGroup() const;
	bool IsTopInGroup() const;
};

// 拡張構造体
struct EditNodeEx {
	EditNode*	p;			// 編集ウィンドウ配列要素へのポインタ
	int			nGroupMru;	// グループ単位のMRU番号
};


// 共有メモリ内構造体
struct Share_Nodes {
	size_t		nEditArrNum;
	EditNode	pEditArr[MAX_EDITWINDOWS];
	LONG		nSequences;			// ウィンドウ連番
	LONG		nNonameSequences;	// 無題連番
	LONG		nGroupSequences;	// タブグループ連番
};


// ノードアクセサ
class AppNodeHandle {
public:
	AppNodeHandle(HWND hwnd);
	EditNode* operator->() { return pNodeRef; }
private:
	EditNode* pNodeRef;
};

// グループアクセサ
class AppNodeGroupHandle {
public:
	AppNodeGroupHandle(int nGroupId) : nGroup(nGroupId) { }
	AppNodeGroupHandle(HWND hwnd) { nGroup = AppNodeHandle(hwnd)->GetGroup(); }

	EditNode* GetTopEditNode() { return GetEditNodeAt(0); }	//
	EditNode* GetEditNodeAt(size_t nIndex);					// 指定位置の編集ウィンドウ情報を取得する
	bool AddEditWndList(HWND);								// 編集ウィンドウの登録
	void DeleteEditWndList(HWND);							// 編集ウィンドウリストからの削除
	bool RequestCloseEditor(EditNode* pWndArr, size_t nArrCnt, bool bExit, bool bCheckConfirm, HWND hWndFrom);
															// いくつかのウィンドウへ終了要求を出す

	int GetEditorWindowsNum(bool bExcludeClosing = true);				// 現在の編集ウィンドウの数を調べる

	// 全ウィンドウ一括操作
	bool PostMessageToAllEditors(UINT uMsg, WPARAM wParam, LPARAM lParam, HWND hWndLast);	// 全編集ウィンドウへメッセージをポストする
	bool SendMessageToAllEditors(UINT uMsg, WPARAM wParam, LPARAM lParam, HWND hWndLast);	// 全編集ウィンドウへメッセージを送る

public:
	bool operator == (const AppNodeGroupHandle& rhs) const { return nGroup == rhs.nGroup; }
	bool IsValidGroup() const { return nGroup >= 0; }
	operator int() const { return nGroup; }

private:
	int nGroup;
};


class AppNodeManager : public TSingleton<AppNodeManager> {
	friend class TSingleton<AppNodeManager>;
	AppNodeManager() {}

public:
	// グループ
	void ResetGroupId();									// グループをIDリセットする

	// ウィンドウハンドル → ノード　変換
	EditNode* GetEditNode(HWND hWnd);							// 編集ウィンドウ情報を取得する
	int GetNoNameNumber(HWND);

	// タブ
	bool ReorderTab(HWND hSrcTab, HWND hDstTab);				// タブ移動に伴うウィンドウの並び替え
	HWND SeparateGroup(HWND hwndSrc, HWND hwndDst, bool bSrcIsTop, int notifygroups[]);	// タブ分離に伴うウィンドウ処理

	// 総合情報
	size_t GetOpenedWindowArr(EditNode** , bool, bool bGSort = false);				// 現在開いている編集ウィンドウの配列を返す

protected:
	size_t _GetOpenedWindowArrCore(EditNode** , bool, bool bGSort = false);			// 現在開いている編集ウィンドウの配列を返す（コア処理部）

public:
	static bool IsSameGroup(HWND hWnd1, HWND hWnd2);					// 同一グループかどうかを調べる
	int GetFreeGroupId(void);											// 空いているグループ番号を取得する
	HWND GetNextTab(HWND hWndCur);										// Close した時の次のWindowを取得する(タブまとめ表示の場合)
};


inline AppNodeGroupHandle EditNode::GetGroup() const { if (this) return nGroup; else return 0; }

inline bool EditNode::IsTopInGroup() const { return this && (AppNodeGroupHandle(nGroup).GetEditNodeAt(0) == this); }

inline AppNodeHandle::AppNodeHandle(HWND hwnd)
{
	pNodeRef = AppNodeManager::getInstance().GetEditNode(hwnd);
}

