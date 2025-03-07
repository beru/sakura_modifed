/*!	@file
	@brief ファイル比較ダイアログボックス
*/

class DlgCompare;

#pragma once

#include "dlg/Dialog.h"
/*!
	@brief ファイル比較ダイアログボックス
*/
class DlgCompare : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgCompare();
	
	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal(HINSTANCE, HWND, LPARAM, const TCHAR*, TCHAR*, HWND*);	// モーダルダイアログの表示
	
	const TCHAR*	pszPath;
	TCHAR*			pszCompareLabel;
	HWND*			phwndCompareWnd;
	bool			bCompareAndTileHorz;	// 左右に並べて表示
	
protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnBnClicked(int);
	LPVOID GetHelpIdTable(void);
	
	INT_PTR DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);	// 標準以外のメッセージを捕捉する
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnSize(WPARAM wParam, LPARAM lParam);
	BOOL OnMove(WPARAM wParam, LPARAM lParam);
	BOOL OnMinMaxInfo(LPARAM lParam);
	
	void SetData(void);	// ダイアログデータの設定
	int GetData(void);	// ダイアログデータの取得
	
private:
	POINT	ptDefaultSize;
	RECT	rcItems[6];
};


