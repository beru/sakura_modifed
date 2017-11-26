/*!	@file
	@brief 共通設定ダイアログボックスの処理
*/
#pragma once

#include "func/FuncLookup.h"
#include "env/CommonSetting.h"

struct DllSharedData;
class ImageListMgr;
class SMacroMgr;
class MenuDrawer;

/*! プロパティシート番号 */
enum PropComSheetOrder {
	ID_PROPCOM_PAGENUM_GENERAL = 0,		// 全般
	ID_PROPCOM_PAGENUM_WIN,				// ウィンドウ
	ID_PROPCOM_PAGENUM_MAINMENU,		// メインメニュー
	ID_PROPCOM_PAGENUM_TOOLBAR,			// ツールバー
	ID_PROPCOM_PAGENUM_TAB,				// タブバー
	ID_PROPCOM_PAGENUM_STATUSBAR,		// ステータスバー
	ID_PROPCOM_PAGENUM_EDIT,			// 編集
	ID_PROPCOM_PAGENUM_FILE,			// ファイル
	ID_PROPCOM_PAGENUM_FILENAME,		// ファイル名表示
	ID_PROPCOM_PAGENUM_BACKUP,			// バックアップ
	ID_PROPCOM_PAGENUM_FORMAT,			// 書式
	ID_PROPCOM_PAGENUM_GREP,			// 検索
	ID_PROPCOM_PAGENUM_KEYBOARD,		// キー割り当て
	ID_PROPCOM_PAGENUM_CUSTMENU,		// カスタムメニュー
	ID_PROPCOM_PAGENUM_KEYWORD,			// 強調キーワード
	ID_PROPCOM_PAGENUM_HELPER,			// 支援
	ID_PROPCOM_PAGENUM_MACRO,			// マクロ
	ID_PROPCOM_PAGENUM_PLUGIN,			// プラグイン
	ID_PROPCOM_PAGENUM_MAX,
};
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief 共通設定ダイアログボックスクラス

	1つのダイアログボックスに複数のプロパティページが入った構造に
	なっており、Dialog procedureとEvent Dispatcherがページごとにある．
*/
class PropCommon {
public:
	/*
	||  Constructors
	*/
	PropCommon();
	~PropCommon();
//@@@ tbMyButtonなどをCShareDataからMenuDrawerへ移動したことによる修正。
	void Create(HWND, ImageListMgr*, MenuDrawer*);	// 初期化

	/*
	||  Attributes & Operations
	*/
	INT_PTR DoPropertySheet(int, bool);	// プロパティシートの作成

	void InitData(void);		// DllSharedDataから一時データ領域に設定を複製する
	void ApplyData(void);		// 一時データ領域からにDllSharedData設定をコピーする
	int GetPageNum() { return nPageNum; }

	//
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

	HWND				hwndParent;	// オーナーウィンドウのハンドル
	HWND				hwndThis;		// このダイアログのハンドル
	PropComSheetOrder	nPageNum;
	DllSharedData*		pShareData;
	int					nKeywordSet1;
	ImageListMgr*	pIcons;	//	Image List
	
	FuncLookup			lookup;

	MenuDrawer*		pMenuDrawer;
	/*
	|| ダイアログデータ
	*/
	CommonSetting	common;

	struct KeywordSetIndex {
		int typeId;
		int index[MAX_KEYWORDSET_PER_TYPE];
	};
	std::vector<KeywordSetIndex> types_nKeywordSetIdx;
	bool	bTrayProc;
	HFONT	hKeywordHelpFont;		// キーワードヘルプ フォント ハンドル
	HFONT	hTabFont;				// タブ フォント ハンドル

protected:
	/*
	||  実装ヘルパ関数
	*/
	void OnHelp(HWND, int);	// ヘルプ
	int	SearchIntArr(int , int* , int);

	// 汎用ダイアログプロシージャ
	static INT_PTR DlgProc(
		INT_PTR (PropCommon::*DispatchPage)(HWND, UINT, WPARAM, LPARAM),
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static INT_PTR DlgProc2( // 独立ウィンドウ用
		INT_PTR (PropCommon::*DispatchPage)(HWND, UINT, WPARAM, LPARAM),
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	typedef	INT_PTR (PropCommon::*pDispatchPage)(HWND, UINT, WPARAM, LPARAM);

	int nLastPos_Macro;			// 前回フォーカスのあった場所
	int nLastPos_FILENAME;	// 前回フォーカスのあった場所 ファイル名タブ用

	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// ダイアログデータの設定
	int  GetData(HWND);	// ダイアログデータの取得
	void Import(HWND);	// インポートする
	void Export(HWND);	// エクスポートする

	HFONT SetCtrlFont(HWND hwndDlg, int idc_static, const LOGFONT& lf);				// コントロールにフォント設定する
	HFONT SetFontLabel(HWND hwndDlg, int idc_static, const LOGFONT& lf, int nps);	// フォントラベルにフォントとフォント名設定する
};


/*!
	@brief 共通設定プロパティページクラス

	1つのプロパティページ毎に定義
	Dialog procedureとEvent Dispatcherがページごとにある．
	変数の定義はPropCommonで行う
*/
//==============================================================
// 全般ページ
class PropGeneral : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// ダイアログデータの設定
	int  GetData(HWND);	// ダイアログデータの取得
};

//==============================================================
// ファイルページ
class PropFile : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// ダイアログデータの設定
	int  GetData(HWND);	// ダイアログデータの取得

private:
	void EnableFilePropInput(HWND hwndDlg);	//	ファイル設定のON/OFF
};

//==============================================================
// キー割り当てページ
class PropKeybind : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// ダイアログデータの設定
	int  GetData(HWND);	// ダイアログデータの取得

	void Import(HWND);	// インポートする
	void Export(HWND);	// エクスポートする

private:
	void ChangeKeyList(HWND); // キーリストをチェックボックスの状態に合わせて更新する
};

//==============================================================
// ツールバーページ
class PropToolbar : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// ダイアログデータの設定
	int  GetData(HWND);	// ダイアログデータの取得

private:
	void DrawToolBarItemList(DRAWITEMSTRUCT*);	// ツールバーボタンリストのアイテム描画
};

//==============================================================
// キーワードページ
class PropKeyword : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static INT_PTR CALLBACK DlgProc_dialog(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// ダイアログデータの設定
	int  GetData(HWND);	// ダイアログデータの取得

private:
	void SetKeywordSet(HWND, size_t);	// 指定キーワードセットの設定
	void ClearKeywordSet(HWND);
	void DispKeywordCount(HWND hwndDlg);

	void Edit_List_Keyword(HWND, HWND);		// リスト中で選択されているキーワードを編集する
	void Delete_List_Keyword(HWND, HWND);	// リスト中で選択されているキーワードを削除する
	void Import_List_Keyword(HWND, HWND);	// リスト中のキーワードをインポートする
	void Export_List_Keyword(HWND, HWND);	// リスト中のキーワードをエクスポートする
	void Clean_List_Keyword(HWND, HWND);	// リスト中のキーワードを整理する
};

//==============================================================
// カスタムメニューページ
class PropCustmenu : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// ダイアログデータの設定
	void SetDataMenuList(HWND, int);
	int  GetData(HWND);	// ダイアログデータの取得
	void Import(HWND);	// カスタムメニュー設定をインポートする
	void Export(HWND);	// カスタムメニュー設定をエクスポートする
};

//==============================================================
// 書式ページ
class PropFormat : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// ダイアログデータの設定
	int  GetData(HWND);	// ダイアログデータの取得

private:
	void ChangeDateExample(HWND hwndDlg);
	void ChangeTimeExample(HWND hwndDlg);

	void EnableFormatPropInput(HWND hwndDlg);	//	書式設定のON/OFF
};

//==============================================================
// 支援ページ
class PropHelper : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// ダイアログデータの設定
	int  GetData(HWND);	// ダイアログデータの取得
};

//==============================================================
// バックアップページ
class PropBackup : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// ダイアログデータの設定
	int  GetData(HWND);	// ダイアログデータの取得

private:
	void EnableBackupInput(HWND hwndDlg);	//	バックアップ設定のON/OFF
	void UpdateBackupFile(HWND hwndDlg);	//	バックアップファイルの詳細設定
};

//==============================================================
// ウィンドウページ
class PropWin : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// ダイアログデータの設定
	int  GetData(HWND);	// ダイアログデータの取得

private:
	void EnableWinPropInput(HWND hwndDlg) ;	//	ウィンドウ設定のON/OFF
};

//==============================================================
// タブ動作ページ
class PropTab : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// ダイアログデータの設定
	int  GetData(HWND);	// ダイアログデータの取得

private:
	void EnableTabPropInput(HWND hwndDlg);
};

//==============================================================
// 編集ページ
class PropEdit : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// ダイアログデータの設定
	int  GetData(HWND);	// ダイアログデータの取得

private:
	void EnableEditPropInput(HWND hwndDlg);
};

//==============================================================
// 検索ページ
class PropGrep : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// ダイアログデータの設定
	int  GetData(HWND);	// ダイアログデータの取得

private:
	void SetRegexpVersion(HWND);
};

//==============================================================
// マクロページ
class PropMacro : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// ダイアログデータの設定
	int  GetData(HWND);	// ダイアログデータの取得

private:
	void InitDialog(HWND hwndDlg);// Macroページの初期化
	void SetMacro2List_Macro(HWND hwndDlg);// Macroデータの設定
	void SelectBaseDir_Macro(HWND hwndDlg);// Macroディレクトリの選択
	void OnFileDropdown_Macro(HWND hwndDlg);// ファイルドロップダウンが開かれるとき
	void CheckListPosition_Macro(HWND hwndDlg);// リストビューのFocus位置確認
	static int CALLBACK DirCallback_Macro(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
};

//==============================================================
// ファイル名表示ページ
class PropFileName : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// ダイアログデータの設定
	int  GetData(HWND);	// ダイアログデータの取得

private:
	static int SetListViewItem_FILENAME(HWND hListView, int, LPTSTR, LPTSTR, bool); // ListViewのアイテムを設定
	static void GetListViewItem_FILENAME(HWND hListView, int, LPTSTR, LPTSTR); // ListViewのアイテムを取得
	static int MoveListViewItem_FILENAME(HWND hListView, int, int); // ListViewのアイテムを移動する
};

//==============================================================
// ステータスバーページ
class PropStatusbar : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// ダイアログデータの設定
	int  GetData(HWND);	// ダイアログデータの取得
};

//==============================================================
// プラグインページ
class PropPlugin : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	std::tstring GetReadMeFile(const std::tstring& sName);	//	Readme ファイルの取得
	bool BrowseReadMe(const std::tstring& sReadMeName);		//	Readme ファイルの表示
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// ダイアログデータの設定
	int  GetData(HWND);	// ダイアログデータの取得

private:
	void SetData_LIST(HWND);
	void InitDialog(HWND hwndDlg);	// Pluginページの初期化
	void EnablePluginPropInput(HWND hwndDlg);
};

//==============================================================
// メインメニューページ
class PropMainMenu : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// ダイアログデータの設定
	int  GetData(HWND);	// ダイアログデータの取得
	void Import(HWND);	// メニュー設定をインポートする
	void Export(HWND);	// メニュー設定をエクスポートする

private:
	bool GetDataTree(HWND, HTREEITEM, int);

	bool Check_MainMenu(HWND, std::wstring&);						// メニューの検査
	bool Check_MainMenu_Sub(HWND, HTREEITEM, int, std::wstring&);	// メニューの検査
};

