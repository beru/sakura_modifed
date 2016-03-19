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
#pragma once

#include "util/design_template.h"
#include "config/maxdata.h"

class AppNodeGroupHandle;

// 編集ウィンドウノード
struct EditNode {
	int				nIndex;
	int				nGroup;						// グループID							//@@@ 2007.06.20 ryoji
	HWND			hWnd;
	int				nId;						// 無題Id
	WIN_CHAR		szTabCaption[_MAX_PATH];	// タブウィンドウ用：キャプション名		//@@@ 2003.05.31 MIK
	SFilePath		szFilePath;					// タブウィンドウ用：ファイル名			//@@@ 2006.01.28 ryoji
	bool			bIsGrep;					// Grepのウィンドウか					//@@@ 2006.01.28 ryoji
	UINT			showCmdRestore;				// 元のサイズに戻すときのサイズ種別		//@@@ 2007.06.20 ryoji
	BOOL			bClosing;					// 終了中か（「最後のファイルを閉じても(無題)を残す」用）	//@@@ 2007.06.20 ryoji

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
	int			nEditArrNum;		// short->intに修正	//@@@ 2003.05.31 MIK
	EditNode	pEditArr[MAX_EDITWINDOWS];	// 最大値修正	@@@ 2003.05.31 MIK
	LONG		nSequences;			// ウィンドウ連番
	LONG		nNonameSequences;	// 無題連番
	LONG		nGroupSequences;	// タブグループ連番	// 2007.06.20 ryoji
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
	EditNode* GetEditNodeAt(int nIndex);					// 指定位置の編集ウィンドウ情報を取得する
	bool AddEditWndList(HWND);								// 編集ウィンドウの登録	// 2007.06.26 ryoji nGroup引数追加
	void DeleteEditWndList(HWND);							// 編集ウィンドウリストからの削除
	bool RequestCloseEditor(EditNode* pWndArr, int nArrCnt, bool bExit, bool bCheckConfirm, HWND hWndFrom);
															// いくつかのウィンドウへ終了要求を出す	// 2007.02.13 ryoji 「編集の全終了」を示す引数(bExit)を追加	// 2007.06.20 ryoji nGroup引数追加

	int GetEditorWindowsNum(bool bExcludeClosing = true);				// 現在の編集ウィンドウの数を調べる		// 2007.06.20 ryoji nGroup引数追加	// 2008.04.19 ryoji bExcludeClosing引数追加

	// 全ウィンドウ一括操作
	bool PostMessageToAllEditors(UINT uMsg, WPARAM wParam, LPARAM lParam, HWND hWndLast);	// 全編集ウィンドウへメッセージをポストする	// 2007.06.20 ryoji nGroup引数追加
	bool SendMessageToAllEditors(UINT uMsg, WPARAM wParam, LPARAM lParam, HWND hWndLast);	// 全編集ウィンドウへメッセージを送る		// 2007.06.20 ryoji nGroup引数追加

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
	bool ReorderTab(HWND hSrcTab, HWND hDstTab);				// タブ移動に伴うウィンドウの並び替え 2007.07.07 genta
	HWND SeparateGroup(HWND hwndSrc, HWND hwndDst, bool bSrcIsTop, int notifygroups[]);	// タブ分離に伴うウィンドウ処理 2007.07.07 genta

	// 総合情報
	int GetOpenedWindowArr(EditNode** , bool, bool bGSort = false);				// 現在開いている編集ウィンドウの配列を返す

protected:
	int _GetOpenedWindowArrCore(EditNode** , bool, bool bGSort = false);			// 現在開いている編集ウィンドウの配列を返す（コア処理部）

public:
	static bool IsSameGroup(HWND hWnd1, HWND hWnd2);					// 同一グループかどうかを調べる
	int GetFreeGroupId(void);											// 空いているグループ番号を取得する
	HWND GetNextTab(HWND hWndCur);										// Close した時の次のWindowを取得する(タブまとめ表示の場合)	2013/4/10 Uchi
};


inline AppNodeGroupHandle EditNode::GetGroup() const { if (this) return nGroup; else return 0; }

inline bool EditNode::IsTopInGroup() const { return this && (AppNodeGroupHandle(nGroup).GetEditNodeAt(0) == this); }

inline AppNodeHandle::AppNodeHandle(HWND hwnd)
{
	pNodeRef = AppNodeManager::getInstance().GetEditNode(hwnd);
}

