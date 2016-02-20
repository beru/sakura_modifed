/*!	@file
	@brief 編集ウィンドウ（外枠）管理クラス

	@author Norio Nakatani
	@date 1998/05/13 新規作成
	@date 2002/01/14 YAZAKI PrintPreviewの分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001-2002, YAZAKI
	Copyright (C) 2002, aroka, genta, MIK
	Copyright (C) 2003, MIK, genta, wmlhq
	Copyright (C) 2004, Moca
	Copyright (C) 2005, genta, Moca
	Copyright (C) 2006, ryoji, aroka, fon, yukihane
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, ryoji
	Copyright (C) 2009, nasukoji

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

#include <ShellAPI.h>// HDROP
#include "_main/global.h"
#include "MainToolBar.h"
#include "TabWnd.h"	//@@@ 2003.05.31 MIK
#include "func/FuncKeyWnd.h"
#include "MainStatusBar.h"
#include "view/EditView.h"
#include "window/SplitterWnd.h"
#include "dlg/DlgFind.h"
#include "dlg/DlgReplace.h"
#include "dlg/DlgJump.h"
#include "dlg/DlgGrep.h"
#include "dlg/DlgGrepReplace.h"
#include "dlg/DlgSetCharSet.h"
#include "outline/DlgFuncList.h"
#include "HokanMgr.h"
#include "util/design_template.h"
#include "doc/DocListener.h"
#include "uiparts/MenuDrawer.h"
#include "view/ViewFont.h"

static const int MENUBAR_MESSAGE_MAX_LEN = 30;

//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる変更
class PrintPreview; // 2002/2/10 aroka
class DropTarget;
class Plug;
class EditDoc;
struct DllSharedData;


// メインウィンドウ内コントロールID
#define IDT_EDIT		455  // 20060128 aroka
#define IDT_TOOLBAR		456
#define IDT_CAPTION		457
#define IDT_FIRST_IDLE	458
#define IDT_SYSMENU		1357
#define ID_TOOLBAR		100

struct TabGroupInfo {
	HWND			hwndTop;
	WINDOWPLACEMENT	wpTop;

	TabGroupInfo() : hwndTop(NULL) { }
	bool IsValid() const { return hwndTop != NULL; }
};

//! 編集ウィンドウ（外枠）管理クラス
// 2002.02.17 YAZAKI CShareDataのインスタンスは、Processにひとつあるのみ。
// 2007.10.30 kobake IsFuncEnable,IsFuncCheckedをFunccode.hに移動
// 2007.10.30 kobake OnHelp_MenuItemをCEditAppに移動
class EditWnd :
	public TSingleton<EditWnd>,
	public DocListenerEx
{
	friend class TSingleton<EditWnd>;
	EditWnd();
	~EditWnd();

public:
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           作成                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//	Mar. 7, 2002 genta 文書タイプ用引数追加
	// 2007.06.26 ryoji グループ指定引数追加
	//! 作成
	HWND Create(
		EditDoc*		pEditDoc,
		ImageListMgr*	pIcons,
		int				nGroup
	);
	void _GetTabGroupInfo(TabGroupInfo* pTabGroupInfo, int& nGroup);
	void _GetWindowRectForInit(Rect* rcResult, int nGroup, const TabGroupInfo& tabGroupInfo);	//!< ウィンドウ生成用の矩形を取得
	HWND _CreateMainWindow(int nGroup, const TabGroupInfo& tabGroupInfo);
	void _AdjustInMonitor(const TabGroupInfo& tabGroupInfo);

	void OpenDocumentWhenStart(
		const LoadInfo& loadInfo		//!< [in]
	);

	void SetDocumentTypeWhenCreate(
		EncodingType		nCharCode,					//!< [in] 漢字コード
		bool			bViewMode,					//!< [in] ビューモードで開くかどうか
		TypeConfigNum	nDocumentType = TypeConfigNum(-1)	//!< [in] 文書タイプ．-1のとき強制指定無し．
	);
	void UpdateCaption();
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         イベント                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// ドキュメントイベント
	void OnAfterSave(const SaveInfo& saveInfo);

	// 管理
	void MessageLoop(void);								// メッセージループ
	LRESULT DispatchEvent(HWND, UINT, WPARAM, LPARAM);	// メッセージ処理

	// 各種イベント
	LRESULT OnPaint(HWND, UINT, WPARAM, LPARAM);	// 描画処理
	LRESULT OnSize(WPARAM, LPARAM);	// WM_SIZE 処理
	LRESULT OnSize2(WPARAM, LPARAM, bool);
	LRESULT OnLButtonUp(WPARAM, LPARAM);
	LRESULT OnLButtonDown(WPARAM, LPARAM);
	LRESULT OnMouseMove(WPARAM, LPARAM);
	LRESULT OnMouseWheel(WPARAM, LPARAM);
	BOOL DoMouseWheel(WPARAM wParam, LPARAM lParam);	// マウスホイール処理	// 2007.10.16 ryoji
	LRESULT OnHScroll(WPARAM, LPARAM);
	LRESULT OnVScroll(WPARAM, LPARAM);
	int	OnClose(HWND hWndFrom, bool);	// 終了時の処理
	void OnDropFiles(HDROP);			// ファイルがドロップされた
	BOOL OnPrintPageSetting(void);		// 印刷ページ設定
	LRESULT OnTimer(WPARAM, LPARAM);	// WM_TIMER 処理	// 2007.04.03 ryoji
	void OnEditTimer(void);				// タイマーの処理
	void OnCaptionTimer(void);
	void OnSysMenuTimer(void);
	void OnCommand(WORD, WORD , HWND);
	LRESULT OnNcLButtonDown(WPARAM, LPARAM);
	LRESULT OnNcLButtonUp(WPARAM, LPARAM);
	LRESULT OnLButtonDblClk(WPARAM, LPARAM);
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           通知                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	
	// ファイル名変更通知
	void ChangeFileNameNotify(const TCHAR* pszTabCaption, const TCHAR* pszFilePath, bool bIsGrep);	//@@@ 2003.05.31 MIK, 2006.01.28 ryoji ファイル名、Grepモードパラメータを追加
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         メニュー                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void InitMenu(HMENU, UINT, BOOL);
	void InitMenu_Function(HMENU , EFunctionCode, const wchar_t*, const wchar_t*);
	bool InitMenu_Special(HMENU , EFunctionCode);
	void InitMenubarMessageFont(void);			//	メニューバーへのメッセージ表示機能をEditWndより移管	//	Dec. 4, 2002 genta
	LRESULT WinListMenu(HMENU hMenu, EditNode* pEditNodeArr, int nRowNum, bool bFull);	//!< ウィンドウ一覧メニュー作成処理		2006.03.23 fon
	LRESULT PopupWinList(bool bMousePos);		//!< ウィンドウ一覧ポップアップ表示処理		2006.03.23 fon	// 2007.02.28 ryoji フルパス指定のパラメータを削除
	void RegisterPluginCommand();				// プラグインコマンドをエディタに登録する
	void RegisterPluginCommand(int id);			// プラグインコマンドをエディタに登録する
	void RegisterPluginCommand(Plug* id);		// プラグインコマンドをエディタに登録する

	void SetMenuFuncSel(HMENU hMenu, EFunctionCode nFunc, const WCHAR* sKey, bool flag);				// 表示の動的選択	2010/5/19 Uchi

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           整形                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void LayoutMainMenu(void);		// メインメニュー					// 2010/5/16 Uchi
	void LayoutToolBar(void);		// ツールバーの配置処理				// 2006.12.19 ryoji
	void LayoutFuncKey(void);		// ファンクションキーの配置処理		// 2006.12.19 ryoji
	void LayoutTabBar(void);		// タブバーの配置処理				// 2006.12.19 ryoji
	void LayoutStatusBar(void);		// ステータスバーの配置処理			// 2006.12.19 ryoji
	void LayoutMiniMap();			// ミニマップの配置処理
	void EndLayoutBars(BOOL bAdjust = TRUE);	// バーの配置終了処理		// 2006.12.19 ryoji


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           設定                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void PrintPreviewModeONOFF(void);	// 印刷プレビューモードのオン/オフ
	
	// アイコン
	void SetWindowIcon(HICON, int);	//	Sep. 10, 2002 genta
	void GetDefaultIcon(HICON* hIconBig, HICON* hIconSmall) const;	//	Sep. 10, 2002 genta
	bool GetRelatedIcon(const TCHAR* szFile, HICON* hIconBig, HICON* hIconSmall) const;	//	Sep. 10, 2002 genta
	void SetPageScrollByWheel(BOOL bState) { m_bPageScrollByWheel = bState; }		// ホイール操作によるページスクロール有無を設定する（TRUE=あり, FALSE=なし）	// 2009.01.17 nasukoji
	void SetHScrollByWheel(BOOL bState) { m_bHorizontalScrollByWheel = bState; }	// ホイール操作による横スクロール有無を設定する（TRUE=あり, FALSE=なし）	// 2009.01.17 nasukoji
	void ClearMouseState(void);		// 2009.01.17 nasukoji	マウスの状態をクリアする（ホイールスクロール有無状態をクリア）
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           情報                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	
	//! 自アプリがアクティブかどうか	// 2007.03.08 ryoji
	bool IsActiveApp() const { return m_bIsActiveApp; }
	
	//! ツールチップのテキストを取得。2007.09.08 kobake 追加
	void GetTooltipText(TCHAR* wszBuf, size_t nBufCount, int nID) const;
	
	//! 印刷プレビュー中かどうか
	bool IsInPreviewMode() {
		return m_pPrintPreview != NULL;
	}
	
	BOOL IsPageScrollByWheel() const { return m_bPageScrollByWheel; }		// ホイール操作によるページスクロール有無	// 2009.01.17 nasukoji
	BOOL IsHScrollByWheel() const { return m_bHorizontalScrollByWheel; }	// ホイール操作による横スクロール有無		// 2009.01.17 nasukoji
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           表示                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void PrintMenubarMessage(const TCHAR* msg);
	void SendStatusMessage(const TCHAR* msg);		//	Dec. 4, 2002 genta 実体をEditViewから移動
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      ウィンドウ操作                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void WindowTopMost(int); // 2004.09.21 Moca
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        ビュー管理                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	LRESULT Views_DispatchEvent(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool CreateEditViewBySplit(int);
	void InitAllViews();
	void Views_RedrawAll();
	void Views_Redraw();
	void SetActivePane(int);	// アクティブなペインを設定
	int GetActivePane(void) const { return m_nActivePaneIndex; }	// アクティブなペインを取得		2007.08.26 kobake const追加
	bool SetDrawSwitchOfAllViews(bool bDraw);						// すべてのペインの描画スイッチを設定する	2008.06.08 ryoji
	void RedrawAllViews(EditView* pViewExclude);				// すべてのペインをRedrawする
	void Views_DisableSelectArea(bool bRedraw);
	BOOL DetectWidthOfLineNumberAreaAllPane(bool bRedraw);	// すべてのペインで、行番号表示に必要な幅を再設定する（必要なら再描画する）
	bool WrapWindowWidth(int nPane);	// 右端で折り返す			2008.06.08 ryoji
	BOOL UpdateTextWrap(void);		// 折り返し方法関連の更新	2008.06.10 ryoji
	//	Aug. 14, 2005 genta TAB幅と折り返し位置の更新
	void ChangeLayoutParam(bool bShowProgress, LayoutInt nTabSize, LayoutInt nMaxLineKetas);
	//	Aug. 14, 2005 genta
	LogicPointEx* SavePhysPosOfAllView();
	void RestorePhysPosOfAllView(LogicPointEx* pptPosArray);
	// 互換BMPによる画面バッファ 2007.09.09 Moca
	void Views_DeleteCompatibleBitmap(); //!< EditViewの画面バッファを削除
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       各種アクセサ                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	HWND			GetHwnd() const			{ return m_hWnd; }
	MenuDrawer&	GetMenuDrawer()			{ return m_menuDrawer; }
	EditDoc*		GetDocument()			{ return m_pEditDoc; }
	const EditDoc*	GetDocument() const		{ return m_pEditDoc; }

	// ビュー
	const EditView&	GetActiveView() const		{ return *m_pEditView; }
	EditView&			GetActiveView()				{ return *m_pEditView; }
	const EditView&	GetView(int n) const		{ return *m_pEditViewArr[n]; }
	EditView&			GetView(int n)				{ return *m_pEditViewArr[n]; }
	EditView&          GetMiniMap()       { return *m_pEditViewMiniMap; }
	bool				IsEnablePane(int n) const	{ return 0 <= n && n < m_nEditViewCount; }
	int					GetAllViewCount() const		{ return m_nEditViewCount; }

	EditView*			GetDragSourceView() const	{ return m_pDragSourceView; }
	void				SetDragSourceView(EditView* pDragSourceView)	{ m_pDragSourceView = pDragSourceView; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         実装補助                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// by 鬼
protected:
	enum class IconClickStatus {
		None,
		Down,
		Clicked,
		DoubleClicked,
	};

protected:
	// ドロップダウンメニュー
	int	CreateFileDropDownMenu(HWND);	// 開く(ドロップダウン)	//@@@ 2002.06.15 MIK

	// タイマー
	void Timer_ONOFF(bool); // 更新の開始／停止 20060128 aroka

	// メニュー
	void CheckFreeSubMenu(HWND, HMENU, UINT);		// メニューバーの無効化を検査	2010/6/18 Uchi
	void CheckFreeSubMenuSub(HMENU, int);			// メニューバーの無効化を検査	2010/6/18 Uchi

//public:
	//! 周期内でm_nTimerCountをインクリメント
	void IncrementTimerCount(int nInterval) {
		++m_nTimerCount;
		if (nInterval <= m_nTimerCount) { // 2012.11.29 aroka 呼び出し間隔のバグ修正
			m_nTimerCount = 0;
		}
	}

	void CreateAccelTbl(void); // ウィンドウ毎のアクセラレータテーブル作成(Wine用)
	void DeleteAccelTbl(void); // ウィンドウ毎のアクセラレータテーブル破棄(Wine用)

public:
	// D&Dフラグ管理
	void SetDragPosOrg(Point ptDragPosOrg)	{ m_ptDragPosOrg = ptDragPosOrg; }
	void SetDragMode(bool bDragMode)			{ m_bDragMode = bDragMode; }
	bool GetDragMode() const					{ return m_bDragMode; }
	const Point& GetDragPosOrg() const		{ return m_ptDragPosOrg; }
	
	// IDropTarget実装		2008.06.20 ryoji
	STDMETHODIMP DragEnter(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
	STDMETHODIMP DragOver(DWORD, POINTL, LPDWORD);
	STDMETHODIMP DragLeave(void);
	STDMETHODIMP Drop(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
	
	// フォーカス管理
	int GetCurrentFocus() const { return m_nCurrentFocus; }
	void SetCurrentFocus(int n) { m_nCurrentFocus = n; }
	
	const LOGFONT& GetLogfont(bool bTempSetting = true);
	int GetFontPointSize(bool bTempSetting = true);
	CharWidthCacheMode GetLogfontCacheMode();

	void ClearViewCaretPosInfo();
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        メンバ変数                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
private:
	// 自ウィンドウ
	HWND			m_hWnd;

	// 親ウィンドウ
	HWND			m_hwndParent;

public:
	// 子ウィンドウ
	MainToolBar		m_toolbar;			//!< ツールバー
	TabWnd			m_tabWnd;			//!< タブウィンドウ	//@@@ 2003.05.31 MIK
	FuncKeyWnd		m_funcKeyWnd;		//!< ファンクションバー
	MainStatusBar	m_statusBar;		//!< ステータスバー
	PrintPreview*	m_pPrintPreview;	//!< 印刷プレビュー表示情報。必要になったときのみインスタンスを生成する。

	SplitterWnd		m_splitterWnd;		//!< 分割フレーム
	EditView*		m_pDragSourceView;	//!< ドラッグ元のビュー
	ViewFont*		m_pViewFont;		//!< フォント
	ViewFont*		m_pViewFontMiniMap;		//!< フォント

	// ダイアログ達
	DlgFind			m_dlgFind;			//「検索」ダイアログ
	DlgReplace		m_dlgReplace;		//「置換」ダイアログ
	DlgJump			m_dlgJump;			//「指定行へジャンプ」ダイアログ
	DlgGrep			m_dlgGrep;			// Grepダイアログ
	DlgGrepReplace	m_dlgGrepReplace;	// Grep置換ダイアログ
	DlgFuncList		m_dlgFuncList;		// アウトライン解析結果ダイアログ
	HokanMgr		m_hokanMgr;		// 入力補完
	DlgSetCharSet	m_dlgSetCharSet;	//「文字コードセット設定」ダイアログ

private:
	// 2010.04.10 Moca  public -> private. 起動直後は[0]のみ有効 4つとは限らないので注意
	EditDoc* 		m_pEditDoc;
	EditView*		m_pEditViewArr[4];		//!< ビュー
	EditView*		m_pEditView;			//!< 有効なビュー
	EditView*		m_pEditViewMiniMap;	//!< ミニマップ
	int				m_nActivePaneIndex;		//!< 有効なビューのindex
	int				m_nEditViewCount;		//!< 有効なビューの数
	const int		m_nEditViewMaxCount;	//!< ビューの最大数=4

	// 共有データ
	DllSharedData*	m_pShareData;

	// ヘルパ
	MenuDrawer		m_menuDrawer;

	// メッセージID
	UINT			m_uMSIMEReconvertMsg;
	UINT			m_uATOKReconvertMsg;

	// 状態
	bool			m_bIsActiveApp;			//!< 自アプリがアクティブかどうか	// 2007.03.08 ryoji
	LPTSTR			m_pszLastCaption;
	LPTSTR			m_pszMenubarMessage;	//!< メニューバー右端に表示するメッセージ
public:
	int				m_nTimerCount;			//!< OnTimer用 2003.08.29 wmlhq
	LogicPointEx*	m_posSaveAry;
private:
	int				m_nCurrentFocus;	//!< 現在のフォーカス情報
	int				nWinSizeType;		//!< サイズ変更のタイプ。SIZE_MAXIMIZED, SIZE_MINIMIZED 等。
	BOOL			m_bPageScrollByWheel;		//!< ホイール操作によるページスクロールあり	// 2009.01.17 nasukoji
	BOOL			m_bHorizontalScrollByWheel;	//!< ホイール操作による横スクロールあり		// 2009.01.17 nasukoji
	HACCEL			m_hAccelWine;		//!< ウィンドウ毎のアクセラレータテーブルのハンドル(Wine用)	// 2009.08.15 nasukoji
	HACCEL			m_hAccel;			//!< アクセラレータテーブル(共有 or ウィンドウ毎)

	// フォント・イメージ
	HFONT			m_hFontCaretPosInfo;		//!< キャレットの行桁位置表示用フォント
	int				m_nCaretPosInfoCharWidth;	//!< キャレットの行桁位置表示用フォントの幅
	int				m_nCaretPosInfoCharHeight;	//!< キャレットの行桁位置表示用フォントの高さ

	// D&Dフラグ
	bool			m_bDragMode;
	Point			m_ptDragPosOrg;
	DropTarget*		m_pDropTarget;

	// その他フラグ
	BOOL			m_bUIPI;		// エディタ－トレイ間でのUI特権分離確認用フラグ	// 2007.06.07 ryoji
	IconClickStatus	m_iconClicked;

public:
	SelectCountMode	m_nSelectCountMode; // 選択文字カウント方法

};

