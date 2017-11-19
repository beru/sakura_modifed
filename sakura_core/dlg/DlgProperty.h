/*!	@file
	@brief ファイルプロパティダイアログ
*/

class DlgProperty;

#pragma once

#include "dlg/Dialog.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class DlgProperty : public Dialog {
public:
	INT_PTR DoModal(HINSTANCE, HWND, LPARAM);	// モーダルダイアログの表示
protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnBnClicked(int);
	void SetData(void);	// ダイアログデータの設定
	LPVOID GetHelpIdTable(void);
};

