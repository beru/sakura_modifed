/*!	@file
	@brief 外部コマンド実行ダイアログ
*/
#include "dlg/Dialog.h"
#include "recent/RecentCmd.h"

#pragma once

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class DlgExec : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgExec();
	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal(HINSTANCE, HWND, LPARAM);	// モーダルダイアログの表示

	TCHAR		szCommand[1024 + 1];	// コマンドライン
	SFilePath	szCurDir;				// カレントディレクトリ
	bool		bEditable;				// 編集ウィンドウへの入力可能

protected:
	ComboBoxItemDeleter comboDel;
	RecentCmd recentCmd;
	ComboBoxItemDeleter comboDelCur;
	RecentCurDir recentCur;

	// オーバーライド?
	int GetData(void);	// ダイアログデータの取得
	void SetData(void);	// ダイアログデータの設定
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnBnClicked(int);
	LPVOID GetHelpIdTable(void);

};


