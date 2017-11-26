#pragma once

#include <ShellAPI.h>// HDROP
#include "_main/global.h"
#include "MainToolBar.h"
#include "TabWnd.h"
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

static const size_t MENUBAR_MESSAGE_MAX_LEN = 30;

class PrintPreview;
class DropTarget;
class Plug;
class EditDoc;
struct DllSharedData;


// メインウィンドウ内コントロールID
#define IDT_EDIT		455
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

// 編集ウィンドウ（外枠）管理クラス
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
	// 作成
	HWND Create(
		EditDoc*		pEditDoc,
		ImageListMgr*	pIcons,
		int				nGroup
	);
	void _GetTabGroupInfo(TabGroupInfo* pTabGroupInfo, int& nGroup);
	void _GetWindowRectForInit(Rect* rcResult, int nGroup, const TabGroupInfo& tabGroupInfo);	// ウィンドウ生成用の矩形を取得
	HWND _CreateMainWindow(int nGroup, const TabGroupInfo& tabGroupInfo);
	void _AdjustInMonitor(const TabGroupInfo& tabGroupInfo);

	void OpenDocumentWhenStart(
		const LoadInfo& loadInfo		// [in]
	);

	void SetDocumentTypeWhenCreate(
		EncodingType	nCharCode,						// [in] 漢字コード
		bool			bViewMode,							// [in] ビューモードで開くかどうか
		TypeConfigNum	nDocumentType = TypeConfigNum(-1)	// [in] 文書タイプ．-1のとき強制指定無し．
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
	bool DoMouseWheel(WPARAM wParam, LPARAM lParam);	// マウスホイール処理
	LRESULT OnHScroll(WPARAM, LPARAM);
	LRESULT OnVScroll(WPARAM, LPARAM);
	int	OnClose(HWND hWndFrom, bool);	// 終了時の処理
	void OnDropFiles(HDROP);			// ファイルがドロップされた
	bool OnPrintPageSetting(void);		// 印刷ページ設定
	LRESULT OnTimer(WPARAM, LPARAM);	// WM_TIMER 処理
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
	void ChangeFileNameNotify(const TCHAR* pszTabCaption, const TCHAR* pszFilePath, bool bIsGrep);
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         メニュー                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void InitMenu(HMENU, UINT, BOOL);
	void InitMenu_Function(HMENU , EFunctionCode, const wchar_t*, const wchar_t*);
	bool InitMenu_Special(HMENU , EFunctionCode);
	void InitMenubarMessageFont(void);			//	メニューバーへのメッセージ表示機能をEditWndより移管
	LRESULT WinListMenu(HMENU hMenu, EditNode* pEditNodeArr, size_t nRowNum, bool bFull);	// ウィンドウ一覧メニュー作成処理
	LRESULT PopupWinList(bool bMousePos);		// ウィンドウ一覧ポップアップ表示処理
	void RegisterPluginCommand();				// プラグインコマンドをエディタに登録する
	void RegisterPluginCommand(int id);			// プラグインコマンドをエディタに登録する
	void RegisterPluginCommand(Plug* id);		// プラグインコマンドをエディタに登録する

	void SetMenuFuncSel(HMENU hMenu, EFunctionCode nFunc, const wchar_t* sKey, bool flag);				// 表示の動的選択

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           整形                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void LayoutMainMenu(void);		// メインメニュー
	void LayoutToolBar(void);		// ツールバーの配置処理
	void LayoutFuncKey(void);		// ファンクションキーの配置処理
	void LayoutTabBar(void);		// タブバーの配置処理
	void LayoutStatusBar(void);		// ステータスバーの配置処理
	void LayoutMiniMap();			// ミニマップの配置処理
	void EndLayoutBars(bool bAdjust = true);	// バーの配置終了処理


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           設定                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void PrintPreviewModeONOFF(void);	// 印刷Previewモードのオン/オフ
	
	// アイコン
	void SetWindowIcon(HICON, int);	// 
	void GetDefaultIcon(HICON* hIconBig, HICON* hIconSmall) const;	// 
	bool GetRelatedIcon(const TCHAR* szFile, HICON* hIconBig, HICON* hIconSmall) const;	// 
	void SetPageScrollByWheel(bool bState) { bPageScrollByWheel = bState; }		// ホイール操作によるページスクロール有無を設定する（TRUE=あり, FALSE=なし）
	void SetHScrollByWheel(bool bState) { bHorizontalScrollByWheel = bState; }	// ホイール操作による横スクロール有無を設定する（TRUE=あり, FALSE=なし）
	void ClearMouseState(void);		// マウスの状態をクリアする（ホイールスクロール有無状態をクリア）
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           情報                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	
	// 自アプリがアクティブかどうか
	bool IsActiveApp() const { return bIsActiveApp; }
	
	// ツールチップのテキストを取得
	void GetTooltipText(TCHAR* wszBuf, size_t nBufCount, int nID) const;
	
	// 印刷Preview中かどうか
	bool IsInPreviewMode() {
		return pPrintPreview != nullptr;
	}
	
	bool IsPageScrollByWheel() const { return bPageScrollByWheel; }		// ホイール操作によるページスクロール有無	
	bool IsHScrollByWheel() const { return bHorizontalScrollByWheel; }	// ホイール操作による横スクロール有無
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           表示                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void PrintMenubarMessage(const TCHAR* msg);
	void SendStatusMessage(const TCHAR* msg);
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      ウィンドウ操作                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void WindowTopMost(int);
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        ビュー管理                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	LRESULT Views_DispatchEvent(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool CreateEditViewBySplit(int);
	void InitAllViews();
	void Views_RedrawAll();
	void Views_Redraw();
	void SetActivePane(int);	// アクティブなペインを設定
	int GetActivePane(void) const { return nActivePaneIndex; }	// アクティブなペインを取得
	bool SetDrawSwitchOfAllViews(bool bDraw);						// すべてのペインの描画スイッチを設定する
	void RedrawAllViews(EditView* pViewExclude);					// すべてのペインをRedrawする
	void Views_DisableSelectArea(bool bRedraw);
	bool DetectWidthOfLineNumberAreaAllPane(bool bRedraw);	// すべてのペインで、行番号表示に必要な幅を再設定する（必要なら再描画する）
	bool WrapWindowWidth(int nPane);	// 右端で折り返す
	bool UpdateTextWrap(void);		// 折り返し方法関連の更新
	void ChangeLayoutParam(bool bShowProgress, size_t nTabSize, size_t nMaxLineKetas);
	PointEx* SavePhysPosOfAllView();
	void RestorePhysPosOfAllView(PointEx* pptPosArray);
	void Views_DeleteCompatibleBitmap(); // EditViewの画面バッファを削除
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       各種アクセサ                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	HWND			GetHwnd() const			{ return hWnd; }
	MenuDrawer&		GetMenuDrawer()			{ return menuDrawer; }
	EditDoc&		GetDocument()			{ return *pEditDoc; }
	const EditDoc&	GetDocument() const		{ return *pEditDoc; }

	// ビュー
	const EditView&		GetActiveView() const		{ return *pEditView; }
	EditView&			GetActiveView()				{ return *pEditView; }
	const EditView&		GetView(int n) const		{ return *pEditViewArr[n]; }
	EditView&			GetView(int n)				{ return *pEditViewArr[n]; }
	EditView&			GetMiniMap()				{ return *pEditViewMiniMap; }
	bool				IsEnablePane(int n) const	{ return 0 <= n && n < nEditViewCount; }
	int					GetAllViewCount() const		{ return nEditViewCount; }

	EditView*			GetDragSourceView() const	{ return pDragSourceView; }
	void				SetDragSourceView(EditView* pDragSourceView)	{ this->pDragSourceView = pDragSourceView; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         実装補助                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	enum class IconClickStatus {
		None,
		Down,
		Clicked,
		DoubleClicked,
	};

protected:
	// ドロップダウンメニュー
	int	CreateFileDropDownMenu(HWND);	// 開く(ドロップダウン)

	// タイマー
	void Timer_ONOFF(bool); // 更新の開始／停止

	// メニュー
	void CheckFreeSubMenu(HWND, HMENU, UINT);		// メニューバーの無効化を検査
	void CheckFreeSubMenuSub(HMENU, int);			// メニューバーの無効化を検査

//public:
	// 周期内でnTimerCountをインクリメント
	void IncrementTimerCount(int nInterval) {
		++nTimerCount;
		if (nInterval <= nTimerCount) {
			nTimerCount = 0;
		}
	}

	void CreateAccelTbl(void); // ウィンドウ毎のアクセラレータテーブル作成(Wine用)
	void DeleteAccelTbl(void); // ウィンドウ毎のアクセラレータテーブル破棄(Wine用)

public:
	// D&Dフラグ管理
	void SetDragPosOrg(Point ptDragPosOrg)	{ ptDragPosOrg = ptDragPosOrg; }
	void SetDragMode(bool bDragMode)		{ this->bDragMode = bDragMode; }
	bool GetDragMode() const				{ return bDragMode; }
	const Point& GetDragPosOrg() const		{ return ptDragPosOrg; }
	
	// IDropTarget実装
	STDMETHODIMP DragEnter(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
	STDMETHODIMP DragOver(DWORD, POINTL, LPDWORD);
	STDMETHODIMP DragLeave(void);
	STDMETHODIMP Drop(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
	
	// フォーカス管理
	int GetCurrentFocus() const { return nCurrentFocus; }
	void SetCurrentFocus(int n) { nCurrentFocus = n; }
	
	const LOGFONT& GetLogfont(bool bTempSetting = true);
	int GetFontPointSize(bool bTempSetting = true);
	CharWidthCacheMode GetLogfontCacheMode();

	void ClearViewCaretPosInfo();
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        メンバ変数                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
private:
	// 自ウィンドウ
	HWND			hWnd;

	// 親ウィンドウ
	HWND			hwndParent;

public:
	// 子ウィンドウ
	MainToolBar		toolbar;		// ツールバー
	TabWnd			tabWnd;			// タブウィンドウ
	FuncKeyWnd		funcKeyWnd;		// ファンクションバー
	MainStatusBar	statusBar;		// ステータスバー
	PrintPreview*	pPrintPreview;	// 印刷Preview表示情報。必要になったときのみインスタンスを生成する。

	SplitterWnd		splitterWnd;		// 分割フレーム
	EditView*		pDragSourceView;	// ドラッグ元のビュー
	ViewFont*		pViewFont;			// フォント
	ViewFont*		pViewFontMiniMap;	// フォント

	// ダイアログ達
	DlgFind			dlgFind;		//「検索」ダイアログ
	DlgReplace		dlgReplace;		//「置換」ダイアログ
	DlgJump			dlgJump;		//「指定行へジャンプ」ダイアログ
	DlgGrep			dlgGrep;		// Grepダイアログ
	DlgGrepReplace	dlgGrepReplace;	// Grep置換ダイアログ
	DlgFuncList		dlgFuncList;	// アウトライン解析結果ダイアログ
	HokanMgr		hokanMgr;		// 入力補完
	DlgSetCharSet	dlgSetCharSet;	//「文字コードセット設定」ダイアログ

private:
	EditDoc* 		pEditDoc;
	// 起動直後は[0]のみ有効 4つとは限らないので注意
	EditView*		pEditViewArr[4];		// ビュー
	EditView*		pEditView;			// 有効なビュー
	EditView*		pEditViewMiniMap;		// ミニマップ
	int				nActivePaneIndex;		// 有効なビューのindex
	int				nEditViewCount;		// 有効なビューの数
	const int		nEditViewMaxCount;	// ビューの最大数=4

	// 共有データ
	DllSharedData*	pShareData;

	// ヘルパ
	MenuDrawer		menuDrawer;

	// メッセージID
	UINT			uMSIMEReconvertMsg;
	UINT			uATOKReconvertMsg;

	// 状態
	bool			bIsActiveApp;			// 自アプリがアクティブかどうか
	LPTSTR			pszLastCaption;
	LPTSTR			pszMenubarMessage;		// メニューバー右端に表示するメッセージ
public:
	int				nTimerCount;			// OnTimer用
	PointEx*		posSaveAry;
private:
	int				nCurrentFocus;				// 現在のフォーカス情報
	int				nWinSizeType;				// サイズ変更のタイプ。SIZE_MAXIMIZED, SIZE_MINIMIZED 等。
	bool			bPageScrollByWheel;			// ホイール操作によるページスクロールあり
	bool			bHorizontalScrollByWheel;	// ホイール操作による横スクロールあり
	HACCEL			hAccelWine;					// ウィンドウ毎のアクセラレータテーブルのハンドル(Wine用)
	HACCEL			hAccel;						// アクセラレータテーブル(共有 or ウィンドウ毎)

	// フォント・イメージ
	HFONT			hFontCaretPosInfo;			// キャレットの行桁位置表示用フォント
	int				nCaretPosInfoCharWidth;		// キャレットの行桁位置表示用フォントの幅
	int				nCaretPosInfoCharHeight;	// キャレットの行桁位置表示用フォントの高さ

	// D&Dフラグ
	bool			bDragMode;
	Point			ptDragPosOrg;
	DropTarget*		pDropTarget;

	// その他フラグ
	BOOL			bUIPI;		// エディタ－トレイ間でのUI特権分離確認用フラグ
	IconClickStatus	iconClicked;

public:
	SelectCountMode	nSelectCountMode; // 選択文字カウント方法

};

