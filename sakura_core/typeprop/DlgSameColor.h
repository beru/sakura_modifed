// 文字色／背景色統一ダイアログ

#pragma once

#include "dlg/Dialog.h"

struct TypeConfig;

/*!	@brief 文字色／背景色統一ダイアログ

	タイプ別設定のカラー設定で，文字色／背景色統一の対象色を指定するために補助的に
	使用されるダイアログボックス
*/
class DlgSameColor : public Dialog {
public:
	DlgSameColor();
	~DlgSameColor();
	INT_PTR DoModal(HINSTANCE, HWND, WORD, TypeConfig*, COLORREF);		// モーダルダイアログの表示

protected:

	virtual LPVOID GetHelpIdTable(void);
	virtual INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);		// ダイアログのメッセージ処理
	virtual BOOL OnInitDialog(HWND, WPARAM, LPARAM);				// WM_INITDIALOG 処理
	virtual BOOL OnBnClicked(int);									// BN_CLICKED 処理
	virtual BOOL OnDrawItem(WPARAM wParam, LPARAM lParam);			// WM_DRAWITEM 処理
	BOOL OnSelChangeListColors(HWND hwndCtl);						// 色選択リストの LBN_SELCHANGE 処理

	static LRESULT CALLBACK ColorStatic_SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);	// サブクラス化された指定色スタティックのウィンドウプロシージャ
	static LRESULT CALLBACK ColorList_SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);		// サブクラス化された色選択リストのウィンドウプロシージャ

	WNDPROC wpColorStaticProc;	// サブクラス化以前の指定色スタティックのウィンドウプロシージャ
	WNDPROC wpColorListProc;	// サブクラス化以前の色選択リストのウィンドウプロシージャ

	WORD wID;				// タイプ別設定ダイアログ（親ダイアログ）で押されたボタンID
	TypeConfig* pTypes;		// タイプ別設定データ
	COLORREF cr;			// 指定色
};

