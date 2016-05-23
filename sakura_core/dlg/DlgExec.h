/*!	@file
	@brief 外部コマンド実行ダイアログ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
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
	bool		bEditable;				// 編集ウィンドウへの入力可能	// 2009.02.21 ryoji

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
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

};


