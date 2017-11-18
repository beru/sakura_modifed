#pragma once

// Dialog Box基底クラスヘッダファイル

class Dialog;

struct DllSharedData;
class Recent;

enum class AnchorStyle {
	None			= 0,
	Left			= 1,
	Right			= 2,
	LeftRight		= 3,
	Top				= 4,
	TopLeft			= 5,
	TopRight		= 6,
	TopLeftRight	= 7,
	Bottom			= 8,
	BottomLeft		= 9,
	BottomRight		= 10,
	BottomLeftRight	= 11,
	TopBottom		= 12,
	TopBottomLeft	= 13,
	TopBottomRight	= 14,
	All				= 15
};

struct AnchorListItem
{
	int id;
	AnchorStyle anchor;
};

struct ComboBoxItemDeleter
{
	Recent*	pRecent;
	HWND		hwndCombo;
	WNDPROC		pComboBoxWndProc;
	WNDPROC		pEditWndProc;
	WNDPROC		pListBoxWndProc;
	ComboBoxItemDeleter(): pRecent(NULL), hwndCombo(NULL), pComboBoxWndProc(NULL), pEditWndProc(NULL), pListBoxWndProc(NULL) {}
};

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief ダイアログウィンドウを扱うクラス

	ダイアログボックスを作るときにはここから継承させる．

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、Processにひとつあるのみ。
*/
class Dialog {
public:
	/*
	||  Constructors
	*/
	Dialog(bool bSizable = false, bool bCheckShareData = true);
	virtual ~Dialog();
	/*
	||  Attributes & Operations
	*/
	virtual INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);		// ダイアログのメッセージ処理
	INT_PTR DoModal(HINSTANCE, HWND, int, LPARAM);					// モーダルダイアログの表示
	HWND DoModeless(HINSTANCE, HWND, int, LPARAM, int);				// モードレスダイアログの表示
	HWND DoModeless(HINSTANCE, HWND, LPCDLGTEMPLATE, LPARAM, int);	// モードレスダイアログの表示
	void CloseDialog(int);
	
	virtual BOOL OnInitDialog(HWND, WPARAM wParam, LPARAM lParam);
	virtual void SetDialogPosSize();
	virtual BOOL OnDestroy(void);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam) {return FALSE;}
	BOOL OnSize();
	virtual BOOL OnSize(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnMove(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnDrawItem(WPARAM wParam, LPARAM lParam) {return TRUE;}
	virtual BOOL OnTimer(WPARAM wParam) {return TRUE;}
	virtual BOOL OnKeyDown(WPARAM wParam, LPARAM lParam) {return TRUE;}
	virtual BOOL OnDeviceChange(WPARAM wParam, LPARAM lParam) {return TRUE;}
	virtual int GetData(void) {return 1;}	// ダイアログデータの取得
	virtual void SetData(void) {return;}	// ダイアログデータの設定
	virtual BOOL OnBnClicked(int);
	virtual BOOL OnStnClicked(int) {return FALSE;}
	virtual BOOL OnEnChange(HWND hwndCtl, int wID) {return FALSE;}
	virtual BOOL OnEnKillFocus(HWND hwndCtl, int wID) {return FALSE;}
	virtual BOOL OnLbnSelChange(HWND hwndCtl, int wID) {return FALSE;}
	virtual BOOL OnLbnDblclk(int wID) {return FALSE;}
	virtual BOOL OnCbnSelChange(HWND hwndCtl, int wID) {return FALSE;}
	virtual BOOL OnCbnEditChange(HWND hwndCtl, int wID) {return FALSE;} // @@2005.03.31 MIK タグジャンプDialog
	virtual BOOL OnCbnDropDown(HWND hwndCtl, int wID);
	static BOOL OnCbnDropDown( HWND hwndCtl, bool scrollBar );
//	virtual BOOL OnCbnCloseUp(HWND hwndCtl, int wID) {return FALSE;}
	virtual BOOL OnCbnSelEndOk(HWND hwndCtl, int wID);

	virtual BOOL OnKillFocus(WPARAM wParam, LPARAM lParam) {return FALSE;}
	virtual BOOL OnActivate(WPARAM wParam, LPARAM lParam) {return FALSE;}	//@@@ 2003.04.08 MIK
	virtual int OnVKeyToItem(WPARAM wParam, LPARAM lParam) { return -1; }
	virtual LRESULT OnCharToItem(WPARAM wParam, LPARAM lParam) { return -1; }
	virtual BOOL OnPopupHelp(WPARAM, LPARAM);	//@@@ 2002.01.18 add
	virtual BOOL OnContextMenu(WPARAM, LPARAM);	//@@@ 2002.01.18 add
	virtual LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void ResizeItem(
		HWND hTarget,
		const POINT& ptDlgDefalut,
		const POINT& ptDlgNew,
		const RECT& rcItemDefault,
		AnchorStyle anchor,
		bool bUpdate = true
		);
	void GetItemClientRect(int wID, RECT& rc);
	static void SetComboBoxDeleter(HWND hwndCtl, ComboBoxItemDeleter* data);
public:
	// 設定フォルダ相対ファイル選択(共有データ,ini位置依存)
	static BOOL SelectFile(HWND parent, HWND hwndCtl, const TCHAR* filter, bool resolvePath);
	static bool DirectoryUp(TCHAR*);

public:
	HWND GetHwnd() const { return hWnd; }
	// 特殊インターフェース (使用は好ましくない)
	void _SetHwnd(HWND hwnd) { hWnd = hwnd; }
	bool IsButtonChecked(int id) { return DlgButton_IsChecked(hWnd, id); }
	bool CheckButton(int id, bool bCheck) { return ::CheckDlgButton(hWnd, id, bCheck ? BST_CHECKED : BST_UNCHECKED) != 0; }
	UINT GetItemText(int nIDDlgItem, TCHAR* str, size_t nMaxCount) { ASSERT_GE(INT32_MAX, nMaxCount); return ::GetDlgItemText(hWnd, nIDDlgItem, str, (int)nMaxCount); }
	BOOL SetItemText(int nIDDlgItem, const TCHAR* str) { return ::SetDlgItemText(hWnd, nIDDlgItem, str); }
	UINT GetItemInt(int nIDDlgItem, BOOL *lpTranslated, BOOL bSigned) { return ::GetDlgItemInt(hWnd, nIDDlgItem, lpTranslated, bSigned); }
	bool SetItemInt(int nIDDlgItem, UINT uValue, BOOL bSigned) { return ::SetDlgItemInt(hWnd, nIDDlgItem, uValue, bSigned) != 0; }

	HWND GetItemHwnd(int nID) { return ::GetDlgItem(hWnd, nID); }
	bool EnableItem(int nID, bool bEnable) { return ::EnableWindow(GetItemHwnd(nID), bEnable ? TRUE : FALSE) != 0; }

	bool GetWindowRect(LPRECT lpRect) { return ::GetWindowRect(hWnd, lpRect) != 0; }

public:
	HINSTANCE		hInstance;	// アプリケーションインスタンスのハンドル
	HWND			hwndParent;	// オーナーウィンドウのハンドル
private:
	HWND			hWnd;			// このダイアログのハンドル
public:
	HWND			hwndSizeBox;
	LPARAM			lParam;
	bool			bModal;		// モーダル ダイアログか
	bool			bSizable;		// 可変ダイアログかどうか
	int				nShowCmd;		// 最大化/最小化
	int				nWidth;
	int				nHeight;
	int				xPos;
	int				yPos;
//	void*			pcEditView;
	DllSharedData*	pShareData;
	bool			bInited;
	HINSTANCE		hLangRsrcInstance;		// メッセージリソースDLLのインスタンスハンドル	// 2011.04.10 nasukoji

protected:
	void CreateSizeBox(void);
	BOOL OnCommand(WPARAM, LPARAM);

	// コントロールに画面のフォントを設定	2012/11/27 Uchi
	HFONT SetMainFont(HWND hTarget);
};

