#pragma once

#include "Funccode_enum.h"

class MenuDrawer;

class ImageListMgr;// 2002/2/10 aroka
struct DllSharedData;

//#define MAX_MENUPOS	10
//	Jul. 2, 2005 genta : マクロをたくさん登録すると上限を超えてしまうので
//	初期値の300から増やす
// #define MAX_MENUITEMS	400 // 2011.11.23 Moca 上限撤廃

// ツールバーの拡張	//@@@ 2002.06.15 MIK
#define TBSTYLE_COMBOBOX	((BYTE)0x40)	// ツールバーにコンボボックス
#ifndef TBSTYLE_DROPDOWN	// IE3以上
	#define TBSTYLE_DROPDOWN	0x0008
#endif
#ifndef TB_SETEXTENDEDSTYLE	// IE4以上
	#define TB_SETEXTENDEDSTYLE     (WM_USER + 84)  // For TBSTYLE_EX_*
#endif
#ifndef TBSTYLE_EX_DRAWDDARROWS	// IE4以上
	#define TBSTYLE_EX_DRAWDDARROWS 0x00000001
#endif

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief メニュー表示＆管理

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 20050809 aroka クラス外部からアクセスされないメンバはprivateにした。
*/
class MenuDrawer {
public:
	/*
	||  Constructors
	*/
	MenuDrawer();
	~MenuDrawer();
	void Create(HINSTANCE, HWND, ImageListMgr*);

	/*
	||  Attributes & Operations
	*/
	void ResetContents(void);
	//void MyAppendMenu(HMENU , int , int , const char*, BOOL = TRUE);	/* メニュー項目を追加 */
	void MyAppendMenu(HMENU hMenu, int nFlag, UINT_PTR nFuncId, const TCHAR*     pszLabel, const TCHAR*     pszKey, bool bAddKeyStr = true, int nForceIconId = -1);	/* メニュー項目を追加 */	// お気に入り	//@@@ 2003.04.08 MIK	// add pszKey	2010/5/17 Uchi
	void MyAppendMenu(HMENU hMenu, int nFlag, UINT_PTR nFuncId, const NOT_TCHAR* pszLabel, const NOT_TCHAR* pszKey, bool bAddKeyStr = true, int nForceIconId = -1) {
		MyAppendMenu(hMenu, nFlag, nFuncId, to_tchar(pszLabel), to_tchar(pszKey), bAddKeyStr, nForceIconId);
	}
	void MyAppendMenuSep(HMENU hMenu, int nFlag, int nFuncId, const TCHAR* pszLabel, bool bAddKeyStr = true, int nForceIconId = -1) {
		MyAppendMenu(hMenu, nFlag, nFuncId, pszLabel, _T(""), bAddKeyStr, nForceIconId);
	}
	int MeasureItem(int, int*);	/* メニューアイテムの描画サイズを計算 */
	void DrawItem(DRAWITEMSTRUCT*);	/* メニューアイテム描画 */
	void EndDrawMenu();
	LRESULT OnMenuChar(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	int FindToolbarNoFromCommandId(int idCommand, bool bOnlyFunc = true)const; // ツールバーNoの取得
	int GetIconIdByFuncId(int nIndex) const;

	TBBUTTON getButton(int nToolBarNo) const; // 20050809 aroka
	void AddToolButton(int iBitmap, int iCommand);	// ツールバーボタンを追加する 2009.11.14 syat
	
	// iBitmapに対応する定数
	static const int TOOLBAR_ICON_MACRO_INTERNAL = 384;		// 外部マクロ既定アイコン
	static const int TOOLBAR_ICON_PLUGCOMMAND_DEFAULT = 283;// プラグインコマンド既定アイコン
	// tbMyButtonのindexに対応する定数
	static const int TOOLBAR_BUTTON_F_SEPARATOR = 0;		// セパレータ（ダミー）
	static const int TOOLBAR_BUTTON_F_TOOLBARWRAP = 384;	// ツールバー折返しアイコン（ダミー）

private:
	void DeleteCompDC();
	int FindIndexFromCommandId(int idCommand, bool bOnlyFunc = true) const;  /* ツールバーIndexの取得 */// 20050809 aroka
	int Find(int nFuncID);
	const TCHAR* GetLabel(int nFuncID);
	TCHAR GetAccelCharFromLabel(const TCHAR* pszLabel);
	int ToolbarNoToIndex(int nToolbarNo) const;

private:
	DllSharedData*	pShareData;

	HINSTANCE		hInstance;
	HWND			hWndOwner;

//@@@ 2002.01.03 YAZAKI tbMyButtonなどをCShareDataからMenuDrawerへ移動したことによる修正。
// 2009.11.14 syat プラグインコマンド動的追加のためvector化
	std::vector<TBBUTTON>	tbMyButton;	// ツールバーのボタン
	size_t nMyButtonNum;
	size_t nMyButtonFixSize;	// 固定部分の最大数
	
	// 2011.11.18 MenuItemのvector化
	struct MyMenuItemInfo {
		int				nBitmapIdx;
		int				nFuncId;
		NativeT			memLabel;
	};
	std::vector<MyMenuItemInfo> menuItems;
	int				nMenuHeight;
	int				nMenuFontHeight;
	HFONT			hFontMenu;
	HBITMAP			hCompBitmap;
	HBITMAP			hCompBitmapOld;
	HDC				hCompDC;
	int				nCompBitmapHeight;
	int				nCompBitmapWidth;

public:
	// 2010.01.30 syat アイコンイメージリストをprivate->public
	//	Oct. 16, 2000 genta
	ImageListMgr* pIcons;	//	Image List

protected:
	/*
	||  実装ヘルパ関数
	*/
	int GetData(void);	/* ダイアログデータの取得 */

//@@@ 2002.01.03 YAZAKI tbMyButtonなどをCShareDataからMenuDrawerへ移動したことによる修正。
	void SetTBBUTTONVal(TBBUTTON*, int, int, BYTE, BYTE, DWORD_PTR, INT_PTR) const;	/* TBBUTTON構造体にデータをセット */
};

