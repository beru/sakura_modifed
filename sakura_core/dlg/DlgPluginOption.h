/*!	@file
	@brief プラグイン設定ダイアログボックス
*/
#pragma once

#include "dlg/Dialog.h"
#include "plugin/PluginManager.h"

class PropPlugin;

/*!	@brief 「プラグイン設定」ダイアログ

	共通設定のプラグイン設定で，指定プラグインのオプションを設定するために
	使用されるダイアログボックス
*/

// 編集最大長
#define MAX_LENGTH_VALUE	1024

typedef std::wstring wstring;

// 型 
static const wstring	OPTION_TYPE_BOOL = wstring(L"bool");
static const wstring	OPTION_TYPE_INT  = wstring(L"int");
static const wstring	OPTION_TYPE_SEL  = wstring(L"sel");
static const wstring	OPTION_TYPE_DIR  = wstring(L"dir");

class DlgPluginOption : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgPluginOption(PropPlugin&);
	~DlgPluginOption();

	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal(HINSTANCE, HWND, int);	// モーダルダイアログの表示

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnInitDialog(HWND, WPARAM wParam, LPARAM lParam);
	BOOL	OnBnClicked(int);
	BOOL	OnNotify(WPARAM wParam, LPARAM lParam);
	BOOL	OnCbnSelChange(HWND hwndCtl, int wID);
	BOOL	OnEnChange(HWND hwndCtl, int wID);
	BOOL	OnActivate(WPARAM wParam, LPARAM lParam);
	LPVOID	GetHelpIdTable(void);

	void	SetData(void);	// ダイアログデータの設定
	int		GetData(void);	// ダイアログデータの取得

	void	ChangeListPosition(void);				// 編集領域をリストビューに合せて切替える
	void	MoveFocusToEdit(void);					// 編集領域にフォーカスを移す
	void	SetToEdit(int);
	void	SetFromEdit(int);
	void	SelectEdit(int);						// 編集領域の切り替え
	void	SepSelect(wstring, wstring*, wstring*);	// 選択用文字列分解
	void	SelectDirectory(int iLine);				// ディレクトリを選択する

private:
	Plugin*		plugin;
	PropPlugin&	propPlugin;
	int 			id;			// プラグイン番号（エディタがふる番号）
	int				line;			// 現在編集中のオプション行番号
	std::tstring	sReadMeName;	// ReadMe ファイル名
};

