/*!	@file
	@brief ファイル比較ダイアログボックス

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
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
	int DoModal( HINSTANCE, HWND, LPARAM, const TCHAR*, TCHAR*, HWND* );	// モーダルダイアログの表示
	
	const TCHAR*	m_pszPath;
	TCHAR*			m_pszCompareLabel;
	HWND*			m_phwndCompareWnd;
	bool			bCompareAndTileHorz;	// 左右に並べて表示
	
protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnBnClicked(int);
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
	
	INT_PTR DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);	// 標準以外のメッセージを捕捉する
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnSize(WPARAM wParam, LPARAM lParam);
	BOOL OnMove(WPARAM wParam, LPARAM lParam);
	BOOL OnMinMaxInfo(LPARAM lParam);
	
	void SetData(void);	// ダイアログデータの設定
	int GetData(void);	// ダイアログデータの取得
	
private:
	POINT			m_ptDefaultSize;
	RECT			m_rcItems[6];
};


