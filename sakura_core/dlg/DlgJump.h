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
	BOOL OnNotify(WPARAM, LPARAM);	// Oct. 6, 2000 JEPRO added for Spin control
	BOOL OnCbnSelChange(HWND, int);
	BOOL OnBnClicked(int);
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
	void SetData(void);	// ダイアログデータの設定
	int GetData(void);	// ダイアログデータの取得
};

