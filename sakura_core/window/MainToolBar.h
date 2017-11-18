#pragma once

#include "recent/Recent.h"
#include "dlg/Dialog.h"

class EditWnd;
class ImageListMgr;

class MainToolBar {
public:
	MainToolBar(EditWnd& owner);

	void Create(ImageListMgr* pIcons);

	// 作成・破棄
	void CreateToolBar(void);		// ツールバー作成
	void DestroyToolBar(void);		// ツールバー破棄

	// メッセージ
	bool EatMessage(MSG* msg);		// メッセージ処理。なんか処理したなら true を返す。
	void ProcSearchBox(MSG*);		// 検索コンボボックスのメッセージ処理

	// イベント
	void OnToolbarTimer(void);		// タイマーの処理 20060128 aroka
	void UpdateToolbar(void);		// ツールバーの表示を更新する		// 2008.09.23 nasukoji

	// 描画
	LPARAM ToolBarOwnerDraw(LPNMCUSTOMDRAW pnmh);

	// 共有データとの同期
	void AcceptSharedSearchKey();

	// 取得
	HWND GetToolbarHwnd() const	{ return hwndToolBar; }
	HWND GetRebarHwnd() const	{ return hwndReBar; }
	HWND GetSearchHwnd() const	{ return hwndSearchBox; }
	size_t GetSearchKey(std::wstring&); // 検索キーを取得。戻り値は検索キーの文字数。

	// 操作
	void SetFocusSearchBox(void) const;		// ツールバー検索ボックスへフォーカスを移動		2006.06.04 yukihane

private:
	EditWnd&	owner;
    HWND		hwndToolBar;

	// 子ウィンドウ
    HWND		hwndReBar;		// Rebar ウィンドウ	//@@@ 2006.06.17 ryoji
	HWND		hwndSearchBox;	// 検索コンボボックス

	// フォント
	HFONT		hFontSearchBox;	// 検索コンボボックスのフォント

	ComboBoxItemDeleter	comboDel;
	RecentSearch		recentSearch;
	ImageListMgr*		pIcons;
};

