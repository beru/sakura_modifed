/*!	@file
	@brief 1行入力ダイアログボックス
*/

class DlgInput1;

#pragma once

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief １行入力ダイアログボックス
*/
class DlgInput1 {
public:
	/*
	||  Constructors
	*/
	DlgInput1();
	~DlgInput1();
	BOOL DoModal(HINSTANCE, HWND, const TCHAR*, const TCHAR*, size_t, TCHAR*);		// モードレスダイアログの表示
	BOOL DoModal(HINSTANCE, HWND, const TCHAR*, const TCHAR*, size_t, NOT_TCHAR*);	// モードレスダイアログの表示
	/*
	||  Attributes & Operations
	*/
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);	// ダイアログのメッセージ処理

	HINSTANCE	hInstance;	// アプリケーションインスタンスのハンドル
	HWND		hwndParent;	// オーナーウィンドウのハンドル
	HWND		hWnd;			// このダイアログのハンドル

	const TCHAR*	pszTitle;		// ダイアログタイトル
	const TCHAR*	pszMessage;		// メッセージ
	size_t			nMaxTextLen;	// 入力サイズ上限
//	char*		pszText;			// テキスト
	NativeT	memText;			// テキスト
protected:
	/*
	||  実装ヘルパ関数
	*/
};


