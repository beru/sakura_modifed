#pragma once

#include "Wnd.h"
#include "util/design_template.h"

class Graphics;
struct EditNode;
struct DllSharedData;

// タブバーウィンドウ
class TabWnd : public Wnd {
public:
	/*
	||  Constructors
	*/
	TabWnd();
	virtual ~TabWnd();

	/*
	|| メンバ関数
	*/
	HWND Open(HINSTANCE, HWND);		// ウィンドウ オープン
	void Close(void);				// ウィンドウ クローズ
	void TabWindowNotify(WPARAM wParam, LPARAM lParam);
	void Refresh(bool bEnsureVisible = true, bool bRebuild = false);
	void NextGroup(void);			// 次のグループ
	void PrevGroup(void);			// 前のグループ
	void MoveRight(void);			// タブを右に移動
	void MoveLeft(void);			// タブを左に移動
	void Separate(void);			// 新規グループ
	void JoinNext(void);			// 次のグループに移動
	void JoinPrev(void);			// 前のグループに移動

	LRESULT TabWndDispatchEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT TabListMenu(POINT pt, bool bSel = true, bool bFull = false, bool bOtherGroup = true);	// タブ一覧メニュー作成処理	// 2006.03.23 fon

	void SizeBox_ONOFF(bool bSizeBox);
	HWND GetHwndSizeBox() {
		return hwndSizeBox;
	}
	void OnSize() {
		OnSize( GetHwnd(), WM_SIZE, 0, 0 );
	}
	void UpdateStyle();
protected:
	/*
	|| 実装ヘルパ系
	*/
	int FindTabIndexByHWND(HWND hWnd);
	void AdjustWindowPlacement(void);							// 編集ウィンドウの位置合わせ
	int SetCarmWindowPlacement(HWND hwnd, const WINDOWPLACEMENT* pWndpl);	// アクティブ化の少ない SetWindowPlacement() を実行する
	void ShowHideWindow(HWND hwnd, BOOL bDisp);
	void HideOtherWindows(HWND hwndExclude);					// 他の編集ウィンドウを隠す
	void ForceActiveWindow(HWND hwnd);
	void TabWnd_ActivateFrameWindow(HWND hwnd, bool bForce = true);
	HWND GetNextGroupWnd(void);	// 次のグループの先頭ウィンドウを探す
	HWND GetPrevGroupWnd(void);	// 前のグループの先頭ウィンドウを探す
	void GetTabName(EditNode* pEditNode, bool bFull, bool bDupamp, LPTSTR pszName, size_t nLen);	// タブ名取得処理

	// 仮想関数
	virtual void AfterCreateWindow(void) {}	// ウィンドウ作成後の処理

	// 仮想関数 メッセージ処理
	virtual LRESULT OnSize(HWND, UINT, WPARAM, LPARAM);				// WM_SIZE処理
	virtual LRESULT OnDestroy(HWND, UINT, WPARAM, LPARAM);			// WM_DSESTROY処理
	virtual LRESULT OnNotify(HWND, UINT, WPARAM, LPARAM);			// WM_NOTIFY処理
	virtual LRESULT OnPaint(HWND, UINT, WPARAM, LPARAM);			// WM_PAINT処理
	virtual LRESULT OnCaptureChanged(HWND, UINT, WPARAM, LPARAM);	// WM_CAPTURECHANGED 処理
	virtual LRESULT OnLButtonDown(HWND, UINT, WPARAM, LPARAM);		// WM_LBUTTONDOWN処理
	virtual LRESULT OnLButtonUp(HWND, UINT, WPARAM, LPARAM);		// WM_LBUTTONUP処理
	virtual LRESULT OnRButtonDown(HWND, UINT, WPARAM, LPARAM);		// WM_RBUTTONDOWN処理
	virtual LRESULT OnLButtonDblClk(HWND, UINT, WPARAM, LPARAM);	// WM_LBUTTONDBLCLK処理
	virtual LRESULT OnMouseMove(HWND, UINT, WPARAM, LPARAM);		// WM_MOUSEMOVE処理
	virtual LRESULT OnTimer(HWND, UINT, WPARAM, LPARAM);			// WM_TIMER処理
	virtual LRESULT OnMeasureItem(HWND, UINT, WPARAM, LPARAM);		// WM_MEASUREITEM処理
	virtual LRESULT OnDrawItem(HWND, UINT, WPARAM, LPARAM);			// WM_DRAWITEM処理

	// ドラッグアンドドロップでタブの順序変更を可能に
	// サブクラス化した Tab でのメッセージ処理
	LRESULT OnTabLButtonDown(WPARAM wParam, LPARAM lParam);			// タブ部 WM_LBUTTONDOWN 処理
	LRESULT OnTabLButtonUp(WPARAM wParam, LPARAM lParam);			// タブ部 WM_LBUTTONUP 処理
	LRESULT OnTabMouseMove(WPARAM wParam, LPARAM lParam);			// タブ部 WM_MOUSEMOVE 処理
	LRESULT OnTabTimer(WPARAM wParam, LPARAM lParam);				// タブ部 WM_TIMER処理
	LRESULT OnTabCaptureChanged(WPARAM wParam, LPARAM lParam);		// タブ部 WM_CAPTURECHANGED 処理
	LRESULT OnTabRButtonDown(WPARAM wParam, LPARAM lParam);			// タブ部 WM_RBUTTONDOWN 処理
	LRESULT OnTabRButtonUp(WPARAM wParam, LPARAM lParam);			// タブ部 WM_RBUTTONUP 処理
	LRESULT OnTabMButtonDown(WPARAM wParam, LPARAM lParam);			// タブ部 WM_MBUTTONDOWN 処理
	LRESULT OnTabMButtonUp(WPARAM wParam, LPARAM lParam);			// タブ部 WM_MBUTTONUP 処理
	LRESULT OnTabNotify(WPARAM wParam, LPARAM lParam);				// タブ部 WM_NOTIFY 処理

	// 実装補助インターフェース
	void BreakDrag(void) { if (::GetCapture() == hwndTab) ::ReleaseCapture(); eDragState = DRAG_NONE; nTabCloseCapture = -1; }	// ドラッグ状態解除処理
	bool ReorderTab(int nSrcTab, int nDstTab);			// タブ順序変更処理
	void BroadcastRefreshToGroup(void);
	bool SeparateGroup(HWND hwndSrc, HWND hwndDst, POINT ptDrag, POINT ptDrop);	// タブ分離処理
	LRESULT ExecTabCommand(int nId, POINTS pts);		// タブ部 コマンド実行処理
	void LayoutTab(void);								// タブのレイアウト調整処理

	HIMAGELIST InitImageList(void);						// イメージリストの初期化処理
	int GetImageIndex(EditNode* pNode);					// イメージリストのインデックス取得処理
	HIMAGELIST ImageList_Duplicate(HIMAGELIST himl);	// イメージリストの複製処理

	// タブ一覧を追加
	void DrawBtnBkgnd(HDC hdc, const LPRECT lprcBtn, BOOL bBtnHilighted);	// ボタン背景描画処理
	void DrawListBtn(Graphics& gr, const LPRECT lprcClient);				// 一覧ボタン描画処理
	void DrawCloseFigure(Graphics& gr, const RECT &btnRect);				// 閉じるマーク描画処理
	void DrawCloseBtn(Graphics& gr, const LPRECT lprcClient);				// 閉じるボタン描画処理
	void DrawTabCloseBtn(Graphics& gr, const LPRECT lprcClient, bool selected, bool bHover);	// タブを閉じるボタン描画処理
	void GetListBtnRect(const LPRECT lprcClient, LPRECT lprc);	// 一覧ボタンの矩形取得処理
	void GetCloseBtnRect(const LPRECT lprcClient, LPRECT lprc);	// 閉じるボタンの矩形取得処理
	void GetTabCloseBtnRect(const LPRECT lprcClient, LPRECT lprc, bool selected);	// タブを閉じるボタンの矩形取得処理

	HFONT CreateMenuFont(void)
	{
		// メニュー用フォント作成
		NONCLIENTMETRICS	ncm;
		// 以前のプラットフォームに WINVER >= 0x0600 で定義される構造体のフルサイズを渡すと失敗する
		ncm.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, (PVOID)&ncm, 0);
		return ::CreateFontIndirect(&ncm.lfMenuFont);
	}

protected:
	enum DragState {
		DRAG_NONE,
		DRAG_CHECK,
		DRAG_DRAG,
	};
	enum CaptureSrc {
		CAPT_NONE,
		CAPT_CLOSE,
	};

	typedef HIMAGELIST (WINAPI *FN_ImageList_Duplicate)(HIMAGELIST himl);

	/*
	|| メンバ変数
	*/
public:
	DllSharedData*	pShareData;			// 共有データ
	HFONT			hFont;				// 表示用フォント
	HWND			hwndTab;			// タブコントロール
	HWND			hwndToolTip;		// ツールチップ（ボタン用）
	TCHAR			szTextTip[1024];	// ツールチップのテキスト（タブ用）
	TabPosition		eTabPosition;		// タブ表示位置

private:
	DragState	eDragState;				// ドラッグ状態
	int			nSrcTab;				// 移動元タブ
	POINT		ptSrcCursor;			// ドラッグ開始カーソル位置
	HCURSOR		hDefaultCursor;			// ドラッグ開始時のカーソル

	// タブへのアイコン表示を可能に
	FN_ImageList_Duplicate	realImageList_Duplicate;

	HIMAGELIST	hIml;					// イメージリスト
	HICON		hIconApp;				// アプリケーションアイコン
	HICON		hIconGrep;				// Grepアイコン
	int			iIconApp;				// アプリケーションアイコンのインデックス
	int			iIconGrep;				// Grepアイコンのインデックス

	bool		bVisualStyle;			// ビジュアルスタイルかどうか
	bool		bHovering;
	bool		bListBtnHilighted;
	bool		bCloseBtnHilighted;		// 閉じるボタンハイライト状態
	CaptureSrc	eCaptureSrc;			// キャプチャー元
	bool		bTabSwapped;			// ドラッグ中にタブの入れ替えがあったかどうか
	LONG*		nTabBorderArray;		// ドラッグ前のタブ境界位置配列
	LOGFONT		lf;						// 表示フォントの特性情報
	bool		bMultiLine;				// 複数行

	// タブ内の閉じるボタン用変数
	int			nTabHover;				// マウスカーソル下のタブ（無いときは-1）
	bool		bTabCloseHover;			// マウスカーソル下にタブ内の閉じるボタンがあるか
	int			nTabCloseCapture;		// 閉じるボタンがマウス押下されているタブ（無いときは-1）

	HWND		hwndSizeBox;
	bool		bSizeBox;

private:
	DISALLOW_COPY_AND_ASSIGN(TabWnd);
};

