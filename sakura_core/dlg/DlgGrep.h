/*!	@file
	@brief GREPダイアログボックス

	@author Norio Nakatani
	@date 1998.09/07  新規作成
	@date 1999.12/05 再作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class DlgGrep;

#pragma once

#include "dlg/Dialog.h"
#include "recent/Recent.h"
#include "util/window.h"

//! GREPダイアログボックス
class DlgGrep : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgGrep();
	/*
	||  Attributes & Operations
	*/
	BOOL OnCbnDropDown( HWND hwndCtl, int wID );
	int DoModal(HINSTANCE, HWND, const TCHAR*);		// モーダルダイアログの表示
//	HWND DoModeless(HINSTANCE, HWND, const char*);	// モードレスダイアログの表示


	bool		m_bSubFolder;		// サブフォルダからも検索する
	bool		m_bFromThisText;	// この編集中のテキストから検索する

	SearchOption	searchOption;	// 検索オプション

	EncodingType	nGrepCharSet;			// 文字コードセット
	int			nGrepOutputStyle;			// Grep: 出力形式
	int			nGrepOutputLineType;		// 結果出力：行を出力/該当部分/否マッチ行
	bool		bGrepOutputFileOnly;		// ファイル毎最初のみ検索
	bool		bGrepOutputBaseFolder;		// ベースフォルダ表示
	bool		bGrepSeparateFolder;		// フォルダ毎に表示


	std::wstring	m_strText;				// 検索文字列
	bool			m_bSetText;				// 検索文字列を設定したか
	SFilePath	m_szFile;					// 検索ファイル
	SFilePath	m_szFolder;					// 検索フォルダ
	SFilePath	m_szCurrentFilePath;
protected:
	ComboBoxItemDeleter	m_comboDelText;
	RecentSearch		m_recentSearch;
	ComboBoxItemDeleter	m_comboDelFile;
	RecentGrepFile		m_recentGrepFile;
	ComboBoxItemDeleter	m_comboDelFolder;
	RecentGrepFolder	m_recentGrepFolder;
	FontAutoDeleter		m_fontText;

	/*
	||  実装ヘルパ関数
	*/
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnDestroy();
	BOOL OnBnClicked(int);
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void SetData(void);	// ダイアログデータの設定
	int GetData(void);	// ダイアログデータの取得
	void SetDataFromThisText(bool);	// 現在編集中ファイルから検索チェックでの設定
};

