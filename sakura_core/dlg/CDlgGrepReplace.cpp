/*!	@file
	@brief GREP置換ダイアログボックス

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, Stonee, genta
	Copyright (C) 2002, MIK, genta, Moca, YAZAKI
	Copyright (C) 2003, Moca
	Copyright (C) 2006, ryoji
	Copyright (C) 2010, ryoji
	Copyright (C) 2014, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "dlg/CDlgGrepReplace.h"
#include "View/CEditView.h"
#include "func/Funccode.h"		// Stonee, 2001/03/12
#include "util/module.h"
#include "util/shell.h"
#include "util/os.h"
#include "util/window.h"
#include "env/DLLSHAREDATA.h"
#include "env/CSakuraEnvironment.h"
#include "sakura_rc.h"
#include "sakura.hh"

const DWORD p_helpids[] = {
	IDC_BUTTON_CURRENTFOLDER,		HIDC_GREP_REP_BUTTON_CURRENTFOLDER,		// 現フォルダ
	IDC_BUTTON_FOLDER,				HIDC_GREP_REP_BUTTON_FOLDER,			// フォルダ
	IDOK,							HIDOK_GREP_REP,							// 置換開始
	IDCANCEL,						HIDCANCEL_GREP_REP,						// キャンセル
	IDC_BUTTON_HELP,				HIDC_GREP_REP_BUTTON_HELP,				// ヘルプ
	IDC_CHK_PASTE,					HIDC_GREP_REP_CHK_PASTE,				// クリップボードから貼り付け
	IDC_CHK_WORD,					HIDC_GREP_REP_CHK_WORD,					// 単語単位
	IDC_CHK_SUBFOLDER,				HIDC_GREP_REP_CHK_SUBFOLDER,			// サブフォルダも検索
//	IDC_CHK_FROMTHISTEXT,			HIDC_GREP_REP_CHK_FROMTHISTEXT,			// このファイルから
	IDC_CHK_LOHICASE,				HIDC_GREP_REP_CHK_LOHICASE,				// 大文字小文字
	IDC_CHK_REGULAREXP,				HIDC_GREP_REP_CHK_REGULAREXP,			// 正規表現
	IDC_CHK_BACKUP,					HIDC_GREP_REP_CHK_BACKUP,				// バックアップ作成
	IDC_COMBO_CHARSET,				HIDC_GREP_REP_COMBO_CHARSET,			// 文字コードセット
	IDC_CHECK_CP,					HIDC_GREP_REP_CHECK_CP,					// CP
	IDC_COMBO_TEXT,					HIDC_GREP_REP_COMBO_TEXT,				// 置換前
	IDC_COMBO_TEXT2,				HIDC_GREP_REP_COMBO_TEXT2,				// 置換後
	IDC_COMBO_FILE,					HIDC_GREP_REP_COMBO_FILE,				// ファイル
	IDC_COMBO_FOLDER,				HIDC_GREP_REP_COMBO_FOLDER,				// フォルダ
	IDC_BUTTON_FOLDER_UP,			HIDC_GREP_REP_BUTTON_FOLDER_UP,			// 上
	IDC_RADIO_OUTPUTLINE,			HIDC_GREP_REP_RADIO_OUTPUTLINE,			// 結果出力：行単位
	IDC_RADIO_OUTPUTMARKED,			HIDC_GREP_REP_RADIO_OUTPUTMARKED,		// 結果出力：該当部分
	IDC_RADIO_OUTPUTSTYLE1,			HIDC_GREP_REP_RADIO_OUTPUTSTYLE1,		// 結果出力形式：ノーマル
	IDC_RADIO_OUTPUTSTYLE2,			HIDC_GREP_REP_RADIO_OUTPUTSTYLE2,		// 結果出力形式：ファイル毎
	IDC_RADIO_OUTPUTSTYLE3,			HIDC_GREP_REP_RADIO_OUTPUTSTYLE3,		// 結果出力形式：結果のみ
	IDC_STATIC_JRE32VER,			HIDC_GREP_REP_STATIC_JRE32VER,			// 正規表現バージョン
	IDC_CHK_DEFAULTFOLDER,			HIDC_GREP_REP_CHK_DEFAULTFOLDER,		// フォルダの初期値をカレントフォルダにする
	IDC_CHECK_FILE_ONLY,			HIDC_GREP_REP_CHECK_FILE_ONLY,			// ファイル毎最初のみ検索
	IDC_CHECK_BASE_PATH,			HIDC_GREP_REP_CHECK_BASE_PATH,			// ベースフォルダ表示
	IDC_CHECK_SEP_FOLDER,			HIDC_GREP_REP_CHECK_SEP_FOLDER,			// フォルダ毎に表示
	0, 0
};

DlgGrepReplace::DlgGrepReplace()
{
	if (0 < m_pShareData->m_searchKeywords.m_aReplaceKeys.size()) {
		m_strText2 = m_pShareData->m_searchKeywords.m_aReplaceKeys[0];
	}
	return;
}



// モーダルダイアログの表示
int DlgGrepReplace::DoModal(
	HINSTANCE hInstance,
	HWND hwndParent,
	const TCHAR* pszCurrentFilePath,
	LPARAM lParam
	)
{
	auto& csSearch = m_pShareData->m_common.m_search;
	m_bSubFolder = csSearch.m_bGrepSubFolder;				// Grep: サブフォルダも検索
	m_searchOption = csSearch.m_searchOption;				// 検索オプション
	m_nGrepCharSet = csSearch.m_nGrepCharSet;				// 文字コードセット
	m_nGrepOutputLineType = csSearch.m_nGrepOutputLineType;	// 行を出力するか該当部分だけ出力するか
	m_nGrepOutputStyle = csSearch.m_nGrepOutputStyle;		// Grep: 出力形式
	m_bPaste = false;
	m_bBackup = csSearch.m_bGrepBackup;

	auto& searchKeywords = m_pShareData->m_searchKeywords;
	if (m_szFile[0] == _T('\0') && searchKeywords.m_aGrepFiles.size()) {
		_tcscpy(m_szFile, searchKeywords.m_aGrepFiles[0]);		// 検索ファイル
	}
	if (m_szFolder[0] == _T('\0') && searchKeywords.m_aGrepFolders.size()) {
		_tcscpy(m_szFolder, searchKeywords.m_aGrepFolders[0]);	// 検索フォルダ
	}
	if (pszCurrentFilePath) {	// 2010.01.10 ryoji
		_tcscpy(m_szCurrentFilePath, pszCurrentFilePath);
	}

	return (int)Dialog::DoModal( hInstance, hwndParent, IDD_GREP_REPLACE, lParam );
}

BOOL DlgGrepReplace::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	_SetHwnd( hwndDlg );

	// コンボボックスのユーザー インターフェイスを拡張インターフェースにする
	Combo_SetExtendedUI(GetItemHwnd(IDC_COMBO_TEXT2), TRUE);

	HFONT hFontOld = (HFONT)::SendMessage(GetItemHwnd( IDC_COMBO_TEXT2 ), WM_GETFONT, 0, 0);
	HFONT hFont = SetMainFont(GetItemHwnd( IDC_COMBO_TEXT2 ));
	m_cFontText2.SetFont(hFontOld, hFont, GetItemHwnd( IDC_COMBO_TEXT2 ));

	return DlgGrep::OnInitDialog( hwndDlg, wParam, lParam );
}


BOOL DlgGrepReplace::OnDestroy()
{
	m_cFontText2.ReleaseOnDestroy();
	return DlgGrep::OnDestroy();
}


BOOL DlgGrepReplace::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_HELP:
		MyWinHelp( GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_GREP_REPLACE_DLG) );
		return TRUE;
	case IDC_CHK_PASTE:
	case IDOK:
		{
			bool bStop = false;
			EditView* pEditView = (EditView*)m_lParam;
			if (IsButtonChecked(IDC_CHK_PASTE)
				&& !pEditView->m_pEditDoc->m_docEditor.IsEnablePaste()
			) {
				OkMessage(GetHwnd(), LS(STR_DLGREPLC_CLIPBOARD));
				CheckButton(IDC_CHK_PASTE, false);
				bStop = true;
			}
			EnableItem(IDC_COMBO_TEXT2, !IsButtonChecked(IDC_CHK_PASTE));
			if (wID == IDOK && bStop) {
				return TRUE;
			}
		}
	}
	// 基底クラスメンバ
	return DlgGrep::OnBnClicked( wID );
}



// ダイアログデータの設定
void DlgGrepReplace::SetData(void)
{
	// 置換後
	SetItemText(IDC_COMBO_TEXT2, m_strText2.c_str() );
	HWND hwndCombo = GetItemHwnd(IDC_COMBO_TEXT2);
	auto& replaceKeys = m_pShareData->m_searchKeywords.m_aReplaceKeys;
	for (int i=0; i<replaceKeys.size(); ++i) {
		Combo_AddString(hwndCombo, replaceKeys[i]);
	}
	CheckDlgButtonBool(GetHwnd(), IDC_CHK_BACKUP, m_bBackup);
	DlgGrep::SetData();
}


/*! ダイアログデータの取得
	TRUE==正常  FALSE==入力エラー
*/
int DlgGrepReplace::GetData(void)
{
	m_bPaste = IsButtonChecked(IDC_CHK_PASTE);

	// 置換後
	int nBufferSize = ::GetWindowTextLength( GetItemHwnd(IDC_COMBO_TEXT2) ) + 1;
	std::vector<TCHAR> vText(nBufferSize);
	GetItemText(IDC_COMBO_TEXT2, &vText[0], nBufferSize);
	m_strText2 = to_wchar(&vText[0]);

	if (::GetWindowTextLength( GetItemHwnd(IDC_COMBO_TEXT) ) == 0){
		WarningMessage(	GetHwnd(), LS(STR_DLGREPLC_REPSTR) );
		return FALSE;
	}

	m_bBackup = IsButtonChecked(IDC_CHK_BACKUP);
	m_pShareData->m_common.m_search.m_bGrepBackup = m_bBackup;

	if (!DlgGrep::GetData()) {
		return FALSE;
	}

	if (m_strText2.size() < _MAX_PATH) {
		SearchKeywordManager().AddToReplaceKeyArr( m_strText2.c_str() );
	}
	m_nReplaceKeySequence = GetDllShareData().m_common.m_search.m_nReplaceKeySequence;

	return TRUE;
}

LPVOID DlgGrepReplace::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}


