#pragma once

/*!	@file
	@brief GREP置換ダイアログボックス
*/

class DlgGrep;

#include "dlg/Dialog.h"
#include "dlg/DlgGrep.h"

// GREP置換ダイアログボックス
class DlgGrepReplace : public DlgGrep
{
public:
	/*
	||  Constructors
	*/
	DlgGrepReplace();
	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal(HINSTANCE, HWND, const TCHAR*, LPARAM);	// モーダルダイアログの表示

	bool		bPaste;
	bool		bBackup;

	std::wstring	strText2;				// 置換後
	int				nReplaceKeySequence;	// 置換後シーケンス

protected:
	FontAutoDeleter		fontText2;

	/*
	||  実装ヘルパ関数
	*/
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnDestroy();
	BOOL OnBnClicked(int);
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void SetData(void);	// ダイアログデータの設定
	int GetData(void);	// ダイアログデータの取得
};

