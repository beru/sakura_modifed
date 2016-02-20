#pragma once

/*!	@file
	@brief GREP置換ダイアログボックス

	@author Norio Nakatani
	@date 2011.12.15 CDlgFrep.hから作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, Moca
	Copyright (C) 2014, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class DlgGrep;

#include "dlg/Dialog.h"
#include "dlg/DlgGrep.h"

//! GREP置換ダイアログボックス
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
	int DoModal( HINSTANCE, HWND, const TCHAR*, LPARAM );	// モーダルダイアログの表示

	bool		m_bPaste;
	bool		m_bBackup;

	std::wstring	m_strText2;				//!< 置換後
	int				nReplaceKeySequence;	//!< 置換後シーケンス

protected:
	FontAutoDeleter		m_fontText2;

	/*
	||  実装ヘルパ関数
	*/
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnDestroy();
	BOOL OnBnClicked( int );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void SetData( void );	// ダイアログデータの設定
	int GetData( void );	// ダイアログデータの取得
};

