/*!	@file
	@brief コントロールコード入力ダイアログボックス
*/

class DlgCtrlCode;

#pragma once

#include "dlg/Dialog.h"
/*!
	@brief コントロールコード入力ダイアログボックス
*/
class DlgCtrlCode : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgCtrlCode();

	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal(HINSTANCE, HWND, LPARAM);	// モーダルダイアログの表示

	wchar_t GetCharCode() const { return nCode; } // 選択された文字コードを取得

private:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnInitDialog(HWND, WPARAM wParam, LPARAM lParam);
	BOOL	OnBnClicked(int);
	BOOL	OnNotify(WPARAM wParam, LPARAM lParam);
	LPVOID	GetHelpIdTable(void);

	void	SetData(void);	// ダイアログデータの設定
	int		GetData(void);	// ダイアログデータの取得

private:
	/*
	|| メンバ変数
	*/
	wchar_t		nCode;	// コード
};


