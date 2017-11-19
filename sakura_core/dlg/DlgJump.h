/*!	@file
	@brief 指定行へのジャンプダイアログボックス
*/

class DlgJump;

#pragma once

#include "dlg/Dialog.h"
// 指定行へのジャンプダイアログボックス
class DlgJump : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgJump();
	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal(HINSTANCE, HWND, LPARAM);	// モーダルダイアログの表示

	UINT	nLineNum;		// 行番号
	bool	bPLSQL;		// PL/SQLソースの有効行か
	int		nPLSQL_E1;
	int		nPLSQL_E2;
protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnNotify(WPARAM, LPARAM);
	BOOL OnCbnSelChange(HWND, int);
	BOOL OnBnClicked(int);
	LPVOID GetHelpIdTable(void);
	void SetData(void);	// ダイアログデータの設定
	int GetData(void);	// ダイアログデータの取得
};

