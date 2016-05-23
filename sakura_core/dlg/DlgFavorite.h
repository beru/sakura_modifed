/*!	@file
	@brief 履歴の管理ダイアログボックス

	@author MIK
	@date 2003.4.8
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2010, Moca

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

#include "dlg/Dialog.h"
#include "recent/Recent.h"

// 履歴とお気に入りの管理」ダイアログ
// アクセス方法：[設定] - [履歴の管理]
class DlgFavorite : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgFavorite();
	~DlgFavorite();

	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal(HINSTANCE, HWND, LPARAM);	// モーダルダイアログの表示

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnInitDialog(HWND, WPARAM wParam, LPARAM lParam);
	BOOL	OnBnClicked(int);
	BOOL	OnNotify(WPARAM wParam, LPARAM lParam);
	BOOL	OnActivate(WPARAM wParam, LPARAM lParam);
	LPVOID	GetHelpIdTable(void);
	INT_PTR DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);	// 標準以外のメッセージを捕捉する
	BOOL	OnSize(WPARAM wParam, LPARAM lParam);
	BOOL	OnMove(WPARAM wParam, LPARAM lParam);
	BOOL	OnMinMaxInfo(LPARAM lParam);

	void	SetData(void);	// ダイアログデータの設定
	int		GetData(void);	// ダイアログデータの取得

	void	TabSelectChange(bool);
	bool	RefreshList(void);
	void	SetDataOne(int nIndex, int nLvItemIndex);	// ダイアログデータの設定
	bool	RefreshListOne(int nIndex);
	//void	ChangeSlider(int nIndex);
	void	UpdateUIState();
	
	void    GetFavorite(int nIndex);
	int     DeleteSelected();
	void	AddItem();
	void	EditItem();
	void	RightMenu(POINT&);

private:
	RecentFile			recentFile;
	RecentFolder		recentFolder;
	RecentExceptMRU		recentExceptMRU;
	RecentSearch		recentSearch;
	RecentReplace		recentReplace;
	RecentGrepFile		recentGrepFile;
	RecentGrepFolder	recentGrepFolder;
	RecentCmd			recentCmd;
	RecentCurDir		recentCurDir;

	enum {
		// 管理数
		FAVORITE_INFO_MAX = 10 // 管理数 +1(番兵)
	};

	struct FavoriteInfo {
		Recent*	pRecent;			// オブジェクトへのポインタ
		std::tstring	strCaption;	// キャプション
		const TCHAR*	pszCaption;	// キャプション
		int			nId;				// コントロールのID
		bool		bHaveFavorite;	// お気に入りを持っているか？
		bool		bHaveView;		// 表示数変更機能をもっているか？
		bool		bFilePath;		// ファイル/フォルダか？
		bool		bEditable;		// 編集可能
		bool		bAddExcept;		// 除外へ追加
		int			nViewCount;		// カレントの表示数
		FavoriteInfo():
			pRecent(nullptr)
			,pszCaption(NULL)
			,nId(0)
			,bHaveFavorite(false)
			,bHaveView(false)
			,bFilePath(false)
			,bEditable(false)
			,bAddExcept(false)
			,nViewCount(0)
		{};
	};

	struct ListViewSortInfo {
		HWND	hListView;		// リストビューの HWND
		int		nSortColumn;	// ソート列 -1で未指定
		bool	bSortAscending; // ソートが昇順
	};

	FavoriteInfo        aFavoriteInfo[FAVORITE_INFO_MAX];
	ListViewSortInfo    aListViewInfo[FAVORITE_INFO_MAX];
	POINT				ptDefaultSize;
	RECT				rcListDefault;
	RECT				rcItems[10];

	int		nCurrentTab;
	int		nExceptTab;
	TCHAR	szMsg[1024];

	static void  ListViewSort(ListViewSortInfo&, const Recent* , int, bool);
};

