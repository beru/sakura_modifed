#pragma once

// MRUリストと呼ばれるリストを管理する。フォルダ版。

#include <Windows.h> /// BOOL,HMENU // 2002/2/10 aroka
#include "recent/RecentFolder.h"

class MenuDrawer;

class MruFolder {
public:
	// コンストラクタ
	MruFolder();
	~MruFolder();

	// メニューを取得する
	HMENU CreateMenu(MenuDrawer& menuDrawer) const;	// うーん。pMenuDrawerが必要なくなるといいなぁ。
	HMENU CreateMenu(HMENU hMenu, MenuDrawer& menuDrawer) const;
	BOOL DestroyMenu(HMENU hMenu) const;
	
	// フォルダ名の一覧を教えて
	std::vector<LPCTSTR> GetPathList() const;

	// アクセス関数
	size_t Length() const;	// アイテムの数。
	size_t MenuLength(void) const { return t_min(Length(), recentFolder.GetViewCount()); }	// メニューに表示されるアイテムの数
	void ClearAll();					// アイテムを削除～。
	void Add(const TCHAR* pszFolder);	// pszFolderを追加する。
	const TCHAR* GetPath(int num) const;

protected:
	// 共有メモリアクセス用。
	struct DllSharedData* pShareData;			// 共有メモリを参照するよ。

private:
	RecentFolder recentFolder;	// 履歴	//@@@ 2003.04.08 MIK
};

