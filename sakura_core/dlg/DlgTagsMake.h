/*!	@file
	@brief タグファイル作成ダイアログボックス
*/

class DlgTagsMake;

#pragma once

#include "dlg/Dialog.h"
/*!
	@brief タグファイル作成ダイアログボックス
*/
class DlgTagsMake : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgTagsMake();

	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal(HINSTANCE, HWND, LPARAM, const TCHAR*);	// モーダルダイアログの表示

	TCHAR	szPath[_MAX_PATH + 1];	// フォルダ
	TCHAR	szTagsCmdLine[_MAX_PATH];	// コマンドラインオプション(個別)
	int		nTagsOpt;					// CTAGSオプション(チェック)

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnBnClicked(int);
	LPVOID	GetHelpIdTable(void);

	void	SetData(void);	// ダイアログデータの設定
	int		GetData(void);	// ダイアログデータの取得

private:
	void SelectFolder();

};

