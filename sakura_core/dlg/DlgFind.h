/*!	@file
	@brief 検索ダイアログボックス
*/
#include "dlg/Dialog.h"
#include "recent/RecentSearch.h"
#include "util/window.h"

#pragma once

class DlgFind : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgFind();
	/*
	||  Attributes & Operations
	*/
//	INT_PTR DoModal(HINSTANCE, HWND, LPARAM);		// モーダルダイアログの表示
	HWND DoModeless(HINSTANCE, HWND, LPARAM);	// モードレスダイアログの表示

	void ChangeView(LPARAM);

	SearchOption searchOption;	// 検索オプション
	bool	bNotifyNotFound;	// 検索／置換  見つからないときメッセージを表示
	std::wstring	strText;	// 検索文字列

	Point	ptEscCaretPos_PHY;	// 検索開始時のカーソル位置退避エリア

	RecentSearch		recentSearch;
	ComboBoxItemDeleter	comboDel;
	FontAutoDeleter		fontText;

protected:
	// オーバーライド?
	BOOL OnCbnDropDown(HWND hwndCtl, int wID);
	int GetData(void);		// ダイアログデータの取得
	void SetCombosList(void);	// 検索文字列/置換後文字列リストの設定
	void SetData(void);		// ダイアログデータの設定
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnDestroy();
	BOOL OnBnClicked(int);
	BOOL OnActivate(WPARAM wParam, LPARAM lParam);

	// virtual BOOL OnKeyDown(WPARAM wParam, LPARAM lParam);
	LPVOID GetHelpIdTable(void);
};

