/*!	@file
	@brief MRUリストと呼ばれるリストを管理する

	@author YAZAKI
	@date 2001/12/23  新規作成
*/
/*
	Copyright (C) 1998-2001, YAZAKI
	Copyright (C) 2000, jepro
	Copyright (C) 2002, YAZAKI, aroka
	Copyright (C) 2003, MIK
	Copyright (C) 2004, genta

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

