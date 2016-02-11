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

class DataProfile;
class MenuDrawer;
struct CommonSetting_CustomMenu;	// defined CommonSetting.h
struct CommonSetting_MainMenu;		// defined CommonSetting.h
struct CommonSetting_KeyBind;		// defined CommonSetting.h
struct ColorInfo; // defined doc/CDocTypeSetting.h
struct FileTree;

// 2008.XX.XX kobake CShareDataから分離
// 2008.05.24 Uchi   ShareData_IO_CustMenu, ShareData_IO_KeyBind  move Export、Importに使用
// 2010.08.21 Moca アクセス権、関数名の整理
class ShareData_IO {
public:
	// セーブ・ロード
	static bool LoadShareData();	// 共有データのロード
	static void SaveShareData();	// 共有データの保存

protected:
	static bool ShareData_IO_2(bool);	// 共有データの保存

	// Feb. 12, 2006 D.S.Koba
	static void ShareData_IO_Mru(DataProfile&);
	static void ShareData_IO_Keys(DataProfile&);
	static void ShareData_IO_Grep(DataProfile&);
	static void ShareData_IO_Folders(DataProfile&);
	static void ShareData_IO_Cmd(DataProfile&);
	static void ShareData_IO_Nickname(DataProfile&);
	static void ShareData_IO_Common(DataProfile&);
	static void ShareData_IO_Toolbar(DataProfile&, MenuDrawer*);
	static void ShareData_IO_CustMenu(DataProfile&);
	static void ShareData_IO_Font(DataProfile&);
	static void ShareData_IO_KeyBind(DataProfile&);
	static void ShareData_IO_Print(DataProfile&);
	static void ShareData_IO_Types(DataProfile&);
	static void ShareData_IO_KeyWords(DataProfile&);
	static void ShareData_IO_Macro(DataProfile&);
	static void ShareData_IO_Statusbar(DataProfile&);				// 2008/6/21 Uchi
	static void ShareData_IO_Plugin(DataProfile&, MenuDrawer*);	// 2009/11/30 syat
	static void ShareData_IO_MainMenu(DataProfile&);				// 2010/5/15 Uchi
	static void ShareData_IO_Other(DataProfile&);

public:
	static void ShareData_IO_FileTree( DataProfile&, FileTree&, const WCHAR* );
	static void ShareData_IO_FileTreeItem( DataProfile&, FileTreeItem&, const WCHAR*, int i );
	static void ShareData_IO_Type_One(DataProfile&, TypeConfig& , const WCHAR*);	// 2010/04/12 Uchi 分離

public:
	static void IO_CustMenu(DataProfile&, CommonSetting_CustomMenu&, bool);
	static void IO_KeyBind(DataProfile&, CommonSetting_KeyBind&, bool);	// 2012.11.22 aroka
	static void IO_MainMenu( DataProfile& c, CommonSetting_MainMenu& s, bool b ){		// 2010/5/15 Uchi
		IO_MainMenu(c, NULL, s, b);
	}
	static void IO_MainMenu( DataProfile& cProfile, std::vector<std::wstring>* pData,
		CommonSetting_MainMenu& mainmenu, bool bOutCmdName);
	static void IO_ColorSet(DataProfile*, const WCHAR*, ColorInfo*);		// 色設定 I/O // Feb. 12, 2006 D.S.Koba
};

