/*!	@file
	@brief 検索ダイアログボックス

	@author Norio Nakatani
	@date	1998/12/02 再作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "dlg/CDialog.h"
#include "recent/CRecentSearch.h"
#include "util/window.h"

#pragma once

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class DlgFind : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgFind();
	/*
	||  Attributes & Operations
	*/
//	int DoModal(HINSTANCE, HWND, LPARAM);		// モーダルダイアログの表示
	HWND DoModeless(HINSTANCE, HWND, LPARAM);	// モードレスダイアログの表示

	void ChangeView(LPARAM);

	SearchOption m_searchOption;	// 検索オプション
	bool	m_bNOTIFYNOTFOUND;	// 検索／置換  見つからないときメッセージを表示
	std::wstring	m_strText;	// 検索文字列

	LogicPoint	m_ptEscCaretPos_PHY;	// 検索開始時のカーソル位置退避エリア

	RecentSearch			m_recentSearch;
	ComboBoxItemDeleter	m_comboDel;
	FontAutoDeleter		m_cFontText;

protected:
//@@@ 2002.2.2 YAZAKI CShareDataに移動
//	void AddToSearchKeyArr(const char*);
	// オーバーライド?
	BOOL OnCbnDropDown( HWND hwndCtl, int wID );
	int GetData(void);		// ダイアログデータの取得
	void SetCombosList(void);	// 検索文字列/置換後文字列リストの設定
	void SetData(void);		// ダイアログデータの設定
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnDestroy();
	BOOL OnBnClicked(int);
	BOOL OnActivate(WPARAM wParam, LPARAM lParam);	// 2009.11.29 ryoji

	// virtual BOOL OnKeyDown(WPARAM wParam, LPARAM lParam);
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
};

