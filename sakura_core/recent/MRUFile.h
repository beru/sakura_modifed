#pragma once

#include <Windows.h> /// BOOL,HMENU // 2002/2/10 aroka
#include <vector>
#include "recent/RecentFile.h"

struct EditInfo; // 2004.04.11 genta パラメータ内のstructを削除するため．doxygen対策
class MenuDrawer;

//	@date 2002.2.17 YAZAKI ShareDataのインスタンスは、Processにひとつあるのみ。
class MruFile {
public:
	//	コンストラクタ
	MruFile();
	~MruFile();

	//	メニューを取得する
	HMENU CreateMenu(MenuDrawer& menuDrawer) const;	//	うーん。menuDrawerが必要なくなるといいなぁ。
	HMENU CreateMenu(HMENU hMenu, MenuDrawer& menuDrawer) const;	//	2010/5/21 Uchi
	BOOL DestroyMenu(HMENU hMenu) const;
	
	//	ファイル名の一覧を教えて
	std::vector<LPCTSTR> GetPathList() const;

	//	アクセス関数
	size_t Length(void) const;	//	アイテムの数。
	size_t MenuLength(void) const { return t_min(Length(), recentFile.GetViewCount()); }	//	メニューに表示されるアイテムの数
	void ClearAll(void);	//	アイテムを削除～。
	bool GetEditInfo(size_t num, EditInfo* pfi) const;				//	番号で指定したEditInfo（情報をまるごと）
	bool GetEditInfo(const TCHAR* pszPath, EditInfo* pfi) const;	//	ファイル名で指定したEditInfo（情報をまるごと）
	void Add(EditInfo* pEditInfo);		//	*pEditInfoを追加する。

protected:
	// 共有メモリアクセス用。
	struct DllSharedData* pShareData;		//	共有メモリを参照するよ。
	
private:
	RecentFile	recentFile;	// 履歴	//@@@ 2003.04.08 MIK
};

