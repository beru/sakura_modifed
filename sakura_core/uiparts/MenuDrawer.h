#pragma once

#include "Funccode_enum.h"

class MenuDrawer;

class ImageListMgr;
struct DllSharedData;

// ツールバーの拡張
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
	void MyAppendMenu(HMENU hMenu, int nFlag, UINT_PTR nFuncId, const TCHAR*     pszLabel, const TCHAR*     pszKey, bool bAddKeyStr = true, int nForceIconId = -1);	/* メニュー項目を追加 */
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

	TBBUTTON getButton(int nToolBarNo) const;
	void AddToolButton(int iBitmap, int iCommand);	// ツールバーボタンを追加する
	
	// iBitmapに対応する定数
	static const int TOOLBAR_ICON_MACRO_INTERNAL = 384;		// 外部マクロ既定アイコン
	static const int TOOLBAR_ICON_PLUGCOMMAND_DEFAULT = 283;// プラグインコマンド既定アイコン
	// tbMyButtonのindexに対応する定数
	static const int TOOLBAR_BUTTON_F_SEPARATOR = 0;		// セパレータ（ダミー）
	static const int TOOLBAR_BUTTON_F_TOOLBARWRAP = 384;	// ツールバー折返しアイコン（ダミー）

private:
	void DeleteCompDC();
	int FindIndexFromCommandId(int idCommand, bool bOnlyFunc = true) const;  /* ツールバーIndexの取得 */
	int Find(int nFuncID);
	const TCHAR* GetLabel(int nFuncID);
	TCHAR GetAccelCharFromLabel(const TCHAR* pszLabel);
	int ToolbarNoToIndex(int nToolbarNo) const;

private:
	DllSharedData*	pShareData;

	HINSTANCE		hInstance;
	HWND			hWndOwner;

	std::vector<TBBUTTON>	tbMyButton;	// ツールバーのボタン
	size_t nMyButtonNum;
	size_t nMyButtonFixSize;	// 固定部分の最大数
	
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
	ImageListMgr* pIcons;	//	Image List

protected:
	/*
	||  実装ヘルパ関数
	*/
	int GetData(void);	/* ダイアログデータの取得 */

	void SetTBBUTTONVal(TBBUTTON*, int, int, BYTE, BYTE, DWORD_PTR, INT_PTR) const;	/* TBBUTTON構造体にデータをセット */
};

