/*!	@file
	@brief 文字コードセット設定ダイアログボックス
*/

#pragma once

#include "dlg/Dialog.h"

// 文字コードセット設定ダイアログボックス
class DlgSetCharSet : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgSetCharSet();
	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal(HINSTANCE, HWND, EncodingType*, bool*);	// モーダルダイアログの表示


	EncodingType*	pnCharSet;			// 文字コードセット
	bool*		pbBom;				// BOM
	bool		bCP;

	HWND		hwndCharSet;
	HWND		hwndCheckBOM;

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL	OnBnClicked(int);
	BOOL	OnCbnSelChange(HWND, int);
	LPVOID	GetHelpIdTable(void);

	void	SetData(void);	// ダイアログデータの設定
	int 	GetData(void);	// ダイアログデータの取得

	void	SetBOM(void);		// BOM の設定
};

