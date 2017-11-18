/*!	@file
	@brief GREPダイアログボックス
*/
#include "StdAfx.h"
#include <ShellAPI.h>
#include "dlg/DlgGrep.h"
#include "GrepAgent.h"
#include "GrepEnumKeys.h"
#include "func/Funccode.h"		// Stonee, 2001/03/12
#include "charset/CodePage.h"
#include "util/module.h"
#include "util/shell.h"
#include "util/os.h"
#include "util/window.h"
#include "env/DllSharedData.h"
#include "env/SakuraEnvironment.h"
#include "sakura_rc.h"
#include "sakura.hh"

//GREP CDlgGrep.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12000
	IDC_BUTTON_FOLDER,				HIDC_GREP_BUTTON_FOLDER,			// フォルダ
	IDC_BUTTON_CURRENTFOLDER,		HIDC_GREP_BUTTON_CURRENTFOLDER,		// 現フォルダ
	IDOK,							HIDOK_GREP,							// 検索
	IDCANCEL,						HIDCANCEL_GREP,						// キャンセル
	IDC_BUTTON_HELP,				HIDC_GREP_BUTTON_HELP,				// ヘルプ
	IDC_CHK_WORD,					HIDC_GREP_CHK_WORD,					// 単語単位
	IDC_CHK_SUBFOLDER,				HIDC_GREP_CHK_SUBFOLDER,			// サブフォルダも検索
	IDC_CHK_FROMTHISTEXT,			HIDC_GREP_CHK_FROMTHISTEXT,			// このファイルから
	IDC_CHK_LOHICASE,				HIDC_GREP_CHK_LOHICASE,				// 大文字小文字
	IDC_CHK_REGULAREXP,				HIDC_GREP_CHK_REGULAREXP,			// 正規表現
	IDC_COMBO_CHARSET,				HIDC_GREP_COMBO_CHARSET,			// 文字コードセット
	IDC_CHECK_CP,					HIDC_GREP_CHECK_CP,					// コードページ
	IDC_COMBO_TEXT,					HIDC_GREP_COMBO_TEXT,				// 条件
	IDC_COMBO_FILE,					HIDC_GREP_COMBO_FILE,				// ファイル
	IDC_COMBO_FOLDER,				HIDC_GREP_COMBO_FOLDER,				// フォルダ
	IDC_BUTTON_FOLDER_UP,			HIDC_GREP_BUTTON_FOLDER_UP,			// 上
	IDC_RADIO_OUTPUTLINE,			HIDC_GREP_RADIO_OUTPUTLINE,			// 結果出力：行単位
	IDC_RADIO_OUTPUTMARKED,			HIDC_GREP_RADIO_OUTPUTMARKED,		// 結果出力：該当部分
	IDC_RADIO_OUTPUTSTYLE1,			HIDC_GREP_RADIO_OUTPUTSTYLE1,		// 結果出力形式：ノーマル
	IDC_RADIO_OUTPUTSTYLE2,			HIDC_GREP_RADIO_OUTPUTSTYLE2,		// 結果出力形式：ファイル毎
	IDC_RADIO_OUTPUTSTYLE3,			HIDC_RADIO_OUTPUTSTYLE3,			// 結果出力形式：結果のみ
	IDC_STATIC_JRE32VER,			HIDC_GREP_STATIC_JRE32VER,			// 正規表現バージョン
	IDC_CHK_DEFAULTFOLDER,			HIDC_GREP_CHK_DEFAULTFOLDER,		// フォルダの初期値をカレントフォルダにする
	IDC_CHECK_FILE_ONLY,			HIDC_CHECK_FILE_ONLY,				// ファイル毎最初のみ検索
	IDC_CHECK_BASE_PATH,			HIDC_CHECK_BASE_PATH,				// ベースフォルダ表示
	IDC_CHECK_SEP_FOLDER,			HIDC_CHECK_SEP_FOLDER,				// フォルダ毎に表示
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

static void SetGrepFolder(HWND hwndCtrl, LPCTSTR folder);

DlgGrep::DlgGrep()
{
	bSubFolder = false;				// サブフォルダからも検索する
	bFromThisText = false;			// この編集中のテキストから検索する
	searchOption.Reset();			// 検索オプション
	nGrepCharSet = CODE_SJIS;			// 文字コードセット
	nGrepOutputLineType = 1;			// 行を出力/該当部分/否マッチ行 を出力
	nGrepOutputStyle = 1;				// Grep: 出力形式
	bGrepOutputFileOnly = false;
	bGrepOutputBaseFolder = false;
	bGrepSeparateFolder = false;

	bSetText = false;
	szFile[0] = 0;
	szFolder[0] = 0;
	return;
}

/*!
	コンボボックスのドロップダウンメッセージを捕捉する

	@date 2013.03.24 novice 新規作成
*/
BOOL DlgGrep::OnCbnDropDown(HWND hwndCtl, int wID)
{
	auto& searchKeywords = pShareData->searchKeywords;
	switch (wID) {
	case IDC_COMBO_TEXT:
		if (::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			size_t nSize = searchKeywords.searchKeys.size();
			for (size_t i=0; i<nSize; ++i) {
				Combo_AddString( hwndCtl, searchKeywords.searchKeys[i] );
			}
		}
		break;
	case IDC_COMBO_FILE:
		if (::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			size_t nSize = searchKeywords.grepFiles.size();
			for (size_t i=0; i<nSize; ++i) {
				Combo_AddString( hwndCtl, searchKeywords.grepFiles[i] );
			}
		}
		break;
	case IDC_COMBO_FOLDER:
		if (::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			size_t nSize = searchKeywords.grepFolders.size();
			for (size_t i=0; i<nSize; ++i) {
				Combo_AddString( hwndCtl, searchKeywords.grepFolders[i] );
			}
		}
		break;
	}
	return Dialog::OnCbnDropDown( hwndCtl, wID );
}

// モーダルダイアログの表示
INT_PTR DlgGrep::DoModal(
	HINSTANCE hInstance,
	HWND hwndParent,
	const TCHAR* pszCurrentFilePath
	)
{
	auto& csSearch = pShareData->common.search;
	bSubFolder = csSearch.bGrepSubFolder;			// Grep: サブフォルダも検索
	searchOption = csSearch.searchOption;			// 検索オプション
	nGrepCharSet = csSearch.nGrepCharSet;			// 文字コードセット
	nGrepOutputLineType = csSearch.nGrepOutputLineType;	// 行を出力/該当部分/否マッチ行 を出力
	nGrepOutputStyle = csSearch.nGrepOutputStyle;	// Grep: 出力形式
	bGrepOutputFileOnly = csSearch.bGrepOutputFileOnly;
	bGrepOutputBaseFolder = csSearch.bGrepOutputBaseFolder;
	bGrepSeparateFolder = csSearch.bGrepSeparateFolder;

	// 2013.05.21 コンストラクタからDoModalに移動
	// strText は呼び出し元で設定済み
	auto& searchKeywords = pShareData->searchKeywords;
	if (szFile[0] == _T('\0') && searchKeywords.grepFiles.size()) {
		_tcscpy(szFile, searchKeywords.grepFiles[0]);		// 検索ファイル
	}
	if (szFolder[0] == _T('\0') && searchKeywords.grepFolders.size()) {
		_tcscpy(szFolder, searchKeywords.grepFolders[0]);	// 検索フォルダ
	}

	if (pszCurrentFilePath) {	// 2010.01.10 ryoji
		_tcscpy(szCurrentFilePath, pszCurrentFilePath);
	}

	return Dialog::DoModal(hInstance, hwndParent, IDD_GREP, (LPARAM)NULL);
}

// 2007.02.09 bosagami
LRESULT CALLBACK OnFolderProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
WNDPROC g_pOnFolderProc;

BOOL DlgGrep::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	_SetHwnd(hwndDlg);

	// ユーザーがコンボボックスのエディット コントロールに入力できるテキストの長さを制限する
	// Combo_LimitText(GetItemHwnd(IDC_COMBO_TEXT), _MAX_PATH - 1);
	Combo_LimitText(GetItemHwnd(IDC_COMBO_FILE), _MAX_PATH - 1);
	Combo_LimitText(GetItemHwnd(IDC_COMBO_FOLDER), _MAX_PATH - 1);
	
	// コンボボックスのユーザー インターフェイスを拡張インターフェースにする
	Combo_SetExtendedUI(GetItemHwnd(IDC_COMBO_TEXT), TRUE);
	Combo_SetExtendedUI(GetItemHwnd(IDC_COMBO_FILE), TRUE);
	Combo_SetExtendedUI(GetItemHwnd(IDC_COMBO_FOLDER), TRUE);

	// ダイアログのアイコン
// 2002.02.08 Grepアイコンも大きいアイコンと小さいアイコンを別々にする。
	// Dec, 2, 2002 genta アイコン読み込み方法変更
	HICON hIconBig   = GetAppIcon(hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, false);
	HICON hIconSmall = GetAppIcon(hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, true);
	::SendMessage(GetHwnd(), WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
	::SendMessage(GetHwnd(), WM_SETICON, ICON_BIG, (LPARAM)hIconBig);

	// 文字コードセット選択コンボボックス初期化
	CodeTypesForCombobox codeTypes;
	for (size_t i=0; i<codeTypes.GetCount(); ++i) {
		LONG_PTR idx = Combo_AddString(GetItemHwnd(IDC_COMBO_CHARSET), codeTypes.GetName(i));
		Combo_SetItemData(GetItemHwnd(IDC_COMBO_CHARSET), idx, codeTypes.GetCode(i));
	}
	// 2007.02.09 bosagami
	HWND hFolder = GetItemHwnd(IDC_COMBO_FOLDER);
	DragAcceptFiles(hFolder, true);
	g_pOnFolderProc = (WNDPROC)GetWindowLongPtr(hFolder, GWLP_WNDPROC);
	SetWindowLongPtr(hFolder, GWLP_WNDPROC, (LONG_PTR)OnFolderProc);

	comboDelText = ComboBoxItemDeleter();
	comboDelText.pRecent = &recentSearch;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_TEXT), &comboDelText);
	comboDelFile = ComboBoxItemDeleter();
	comboDelFile.pRecent = &recentGrepFile;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_FILE), &comboDelFile);
	comboDelFolder = ComboBoxItemDeleter();
	comboDelFolder.pRecent = &recentGrepFolder;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_FOLDER), &comboDelFolder);

	// フォント設定	2012/11/27 Uchi
	HFONT hFontOld = (HFONT)::SendMessage(GetItemHwnd(IDC_COMBO_TEXT), WM_GETFONT, 0, 0);
	HFONT hFont = SetMainFont(GetItemHwnd(IDC_COMBO_TEXT));
	fontText.SetFont(hFontOld, hFont, GetItemHwnd(IDC_COMBO_TEXT));

	// 基底クラスメンバ
//	CreateSizeBox();
	return Dialog::OnInitDialog(hwndDlg, wParam, lParam);
}

/*! @brief フォルダ指定EditBoxのコールバック関数

	@date 2007.02.09 bosagami 新規作成
	@date 2007.09.02 genta ディレクトリチェックを強化
*/
LRESULT CALLBACK OnFolderProc(
	HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam
	)
{
	if (msg == WM_DROPFILES) do {
		// From Here 2007.09.02 genta 
		SFilePath path;
		if (DragQueryFile((HDROP)wparam, 0, NULL, 0) > _countof2(path) - 1) {
			// skip if the length of the path exceeds buffer capacity
			break;
		}
		DragQueryFile((HDROP)wparam, 0, path, _countof2(path) - 1);

		// ファイルパスの解決
		SakuraEnvironment::ResolvePath(path);
		
		// ファイルがドロップされた場合はフォルダを切り出す
		// フォルダの場合は最後が失われるのでsplitしてはいけない．
		if (IsFileExists(path, true)) {	// 第2引数がtrueだとディレクトリは対象外
			SFilePath szWork;
			SplitPath_FolderAndFile(path, szWork, NULL);
			_tcscpy(path, szWork);
		}

		SetGrepFolder(hwnd, path);
	}while (0);	// 1回しか通らない. breakでここまで飛ぶ

	return  CallWindowProc(g_pOnFolderProc, hwnd, msg, wparam, lparam);
}

BOOL DlgGrep::OnDestroy()
{
	fontText.ReleaseOnDestroy();
	return Dialog::OnDestroy();
}

BOOL DlgGrep::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_HELP:
		//「Grep」のヘルプ
		// Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_GREP_DIALOG));	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
	case IDC_CHK_FROMTHISTEXT:	// この編集中のテキストから検索する
		// 2010.05.30 関数化
		SetDataFromThisText(IsButtonChecked(IDC_CHK_FROMTHISTEXT));
		return TRUE;
	case IDC_BUTTON_CURRENTFOLDER:	// 現在編集中のファイルのフォルダ
		// ファイルを開いているか
		if (szCurrentFilePath[0] != _T('\0')) {
			TCHAR	szWorkFolder[MAX_PATH];
			TCHAR	szWorkFile[MAX_PATH];
			SplitPath_FolderAndFile(szCurrentFilePath, szWorkFolder, szWorkFile);
			SetGrepFolder(GetItemHwnd(IDC_COMBO_FOLDER), szWorkFolder);
		}else {
			// 現在のプロセスのカレントディレクトリを取得します
			TCHAR	szWorkFolder[MAX_PATH];
			::GetCurrentDirectory(_countof(szWorkFolder) - 1, szWorkFolder);
			SetGrepFolder(GetItemHwnd(IDC_COMBO_FOLDER), szWorkFolder);
		}
		return TRUE;
	case IDC_BUTTON_FOLDER_UP:
		{
			HWND hwnd = GetItemHwnd(IDC_COMBO_FOLDER);
			TCHAR szFolder[_MAX_PATH];
			::GetWindowText(hwnd, szFolder, _countof(szFolder));
			std::vector<std::tstring> vPaths;
			GrepAgent::CreateFolders(szFolder, vPaths);
			if (0 < vPaths.size()) {
				// 最後のパスが操作対象
				auto_strncpy(szFolder, vPaths.rbegin()->c_str(), _MAX_PATH);
				szFolder[_MAX_PATH-1] = _T('\0');
				if (DirectoryUp(szFolder)) {
					*(vPaths.rbegin()) = szFolder;
					szFolder[0] = _T('\0');
					for (size_t i=0; i<vPaths.size(); ++i) {
						TCHAR szFolderItem[_MAX_PATH];
						auto_strncpy(szFolderItem, vPaths[i].c_str(), _MAX_PATH);
						szFolderItem[_MAX_PATH-1] = _T('\0');
						if (auto_strchr(szFolderItem, _T(';'))) {
							szFolderItem[0] = _T('"');
							auto_strncpy(szFolderItem + 1, vPaths[i].c_str(), _MAX_PATH - 1);
							szFolderItem[_MAX_PATH-1] = _T('\0');
							auto_strcat(szFolderItem, _T("\""));
							szFolderItem[_MAX_PATH-1] = _T('\0');
						}
						if (i) {
							auto_strcat(szFolder, _T(";"));
							szFolder[_MAX_PATH-1] = _T('\0');
						}
						auto_strcat_s(szFolder, _MAX_PATH, szFolderItem);
					}
					::SetWindowText(hwnd, szFolder);
				}
			}
		}
		return TRUE;

//	case IDC_CHK_LOHICASE:	// 英大文字と英小文字を区別する
//		MYTRACE(_T("IDC_CHK_LOHICASE\n"));
//		return TRUE;
	case IDC_CHK_REGULAREXP:	// 正規表現
//		MYTRACE(_T("IDC_CHK_REGULAREXP ::IsDlgButtonChecked(GetHwnd(), IDC_CHK_REGULAREXP) = %d\n"), ::IsDlgButtonChecked(GetHwnd(), IDC_CHK_REGULAREXP));
		if (IsButtonChecked(IDC_CHK_REGULAREXP)) {
			// From Here Jun. 26, 2001 genta
			// 正規表現ライブラリの差し替えに伴う処理の見直し
			if (!CheckRegexpVersion(GetHwnd(), IDC_STATIC_JRE32VER, true)) {
				CheckButton(IDC_CHK_REGULAREXP, false);
			}else {
				// To Here Jun. 26, 2001 genta
				// 英大文字と英小文字を区別する
				// 正規表現のときも選択できるように。
//				CheckButton(IDC_CHK_LOHICASE, true);
//				EnableItem(IDC_CHK_LOHICASE), false);

				// 2001/06/23 N.Nakatani
				// 単語単位で検索
				EnableItem(IDC_CHK_WORD, false);
			}
		}else {
			// 英大文字と英小文字を区別する
			// 正規表現のときも選択できるように。
//			EnableItem(IDC_CHK_LOHICASE), true);
//			CheckButton(IDC_CHK_LOHICASE, false);


// 2001/06/23 N.Nakatani
// 単語単位のgrepが実装されたらコメントを外すと思います
// 2002/03/07実装してみた。
			// 単語単位で検索
			EnableItem(IDC_CHK_WORD, true);
		}
		return TRUE;

	case IDC_BUTTON_FOLDER:
		// フォルダ参照ボタン
		{
			TCHAR	szFolder[MAX_PATH];
			// 検索フォルダ
			GetItemText(IDC_COMBO_FOLDER, szFolder, _MAX_PATH - 1);
			if (szFolder[0] == _T('\0')) {
				::GetCurrentDirectory(_countof(szFolder), szFolder);
			}
			if (SelectDir(GetHwnd(), LS(STR_DLGGREP1), szFolder, szFolder)) {
				SetGrepFolder(GetItemHwnd(IDC_COMBO_FOLDER), szFolder);
			}
		}

		return TRUE;
	case IDC_CHECK_CP:
		{
			if (IsButtonChecked(IDC_CHECK_CP)) {
				EnableItem(IDC_CHECK_CP, false);
				HWND combo = GetItemHwnd(IDC_COMBO_CHARSET );
				CodePage::AddComboCodePages(GetHwnd(), combo, -1);
			}
		}
		return TRUE;
	case IDC_CHK_DEFAULTFOLDER:
		// フォルダの初期値をカレントフォルダにする
		{
			pShareData->common.search.bGrepDefaultFolder = IsButtonChecked(IDC_CHK_DEFAULTFOLDER);
		}
		return TRUE;
	case IDC_RADIO_OUTPUTSTYLE3:
		{
			EnableItem(IDC_CHECK_BASE_PATH, false);
			EnableItem(IDC_CHECK_SEP_FOLDER, false);
		}
		break;
	case IDC_RADIO_OUTPUTSTYLE1:
	case IDC_RADIO_OUTPUTSTYLE2:
		{
			EnableItem(IDC_CHECK_BASE_PATH, true);
			EnableItem(IDC_CHECK_SEP_FOLDER, true);
		}
		break;
	case IDOK:
		// ダイアログデータの取得
		if (GetData()) {
//			::EndDialog(hwndDlg, TRUE);
			CloseDialog(TRUE);
		}
		return TRUE;
	case IDCANCEL:
//		::EndDialog(hwndDlg, FALSE);
		CloseDialog(FALSE);
		return TRUE;
	}

	// 基底クラスメンバ
	return Dialog::OnBnClicked(wID);
}


// ダイアログデータの設定
void DlgGrep::SetData(void)
{
	// 検索文字列
	SetItemText(IDC_COMBO_TEXT, strText.c_str());

	// 検索ファイル
	SetItemText(IDC_COMBO_FILE, szFile);

	// 検索フォルダ
	SetItemText(IDC_COMBO_FOLDER, szFolder);

	if (1
		&& (szFolder[0] == _T('\0') || pShareData->common.search.bGrepDefaultFolder)
		&& szCurrentFilePath[0] != _T('\0')
	) {
		TCHAR szWorkFolder[MAX_PATH];
		TCHAR szWorkFile[MAX_PATH];
		SplitPath_FolderAndFile(szCurrentFilePath, szWorkFolder, szWorkFile);
		SetGrepFolder(GetItemHwnd(IDC_COMBO_FOLDER), szWorkFolder);
	}

	// サブフォルダからも検索する
	CheckButton(IDC_CHK_SUBFOLDER, bSubFolder);

	// この編集中のテキストから検索する
	CheckButton(IDC_CHK_FROMTHISTEXT, bFromThisText);
	// 2010.05.30 関数化
	SetDataFromThisText(bFromThisText);

	// 英大文字と英小文字を区別する
	CheckButton(IDC_CHK_LOHICASE, searchOption.bLoHiCase);

	// 2001/06/23 N.Nakatani 現時点ではGrepでは単語単位の検索はサポートできていません
	// 2002/03/07 テストサポート
	// 一致する単語のみ検索する
	CheckButton(IDC_CHK_WORD, searchOption.bWordOnly);
//	EnableItem(IDC_CHK_WORD) , false);	// チェックボックスを使用不可にすも

	// 文字コード自動判別
//	CheckButton(IDC_CHK_KANJICODEAUTODETECT, bKanjiCode_AutoDetect);

	// 2002/09/22 Moca Add
	// 文字コードセット
	{
		int	nCurIdx = -1;
		EncodingType nCharSet;
		HWND hWndCombo = GetItemHwnd(IDC_COMBO_CHARSET);
		nCurIdx = Combo_GetCurSel(hWndCombo);
		CodeTypesForCombobox codeTypes;
		for (size_t nIdx=0; nIdx<codeTypes.GetCount(); ++nIdx) {
			nCharSet = (EncodingType)Combo_GetItemData(hWndCombo, nIdx);
			if (nCharSet == nGrepCharSet) {
				nCurIdx = (int)nIdx;
			}
		}
		if (nCurIdx != -1) {
			Combo_SetCurSel(hWndCombo, nCurIdx);
		}else {
			CheckButton(IDC_CHECK_CP, true);
			EnableItem(IDC_CHECK_CP, false);
			nCurIdx = CodePage::AddComboCodePages(GetHwnd(), hWndCombo, nGrepCharSet);
			if (nCurIdx == -1) {
				Combo_SetCurSel( hWndCombo, 0 );
			}
		}
	}

	// 行を出力するか該当部分だけ出力するか
	if (nGrepOutputLineType == 1) {
		CheckButton(IDC_RADIO_OUTPUTLINE, true);
	}else if (nGrepOutputLineType == 2) {
		CheckButton(IDC_RADIO_NOHIT, true);
	}else {
		CheckButton(IDC_RADIO_OUTPUTMARKED, true);
	}

	EnableItem(IDC_CHECK_BASE_PATH, true);
	EnableItem(IDC_CHECK_SEP_FOLDER, true);
	// Grep: 出力形式
	if (nGrepOutputStyle == 1) {
		CheckButton(IDC_RADIO_OUTPUTSTYLE1, true);
	}else if (nGrepOutputStyle == 2) {
		CheckButton(IDC_RADIO_OUTPUTSTYLE2, true);
	}else if (nGrepOutputStyle == 3) {
		CheckButton(IDC_RADIO_OUTPUTSTYLE3, true);
		EnableItem(IDC_CHECK_BASE_PATH, false);
		EnableItem(IDC_CHECK_SEP_FOLDER, false);
	}else {
		CheckButton(IDC_RADIO_OUTPUTSTYLE1, true);
	}

	// From Here Jun. 29, 2001 genta
	// 正規表現ライブラリの差し替えに伴う処理の見直し
	// 処理フロー及び判定条件の見直し。必ず正規表現のチェックと
	// 無関係にCheckRegexpVersionを通過するようにした。
	if (1
		&& CheckRegexpVersion(GetHwnd(), IDC_STATIC_JRE32VER, false)
		&& searchOption.bRegularExp
	) {
		// 英大文字と英小文字を区別する
		CheckButton(IDC_CHK_REGULAREXP, true);
		// 正規表現のときも選択できるように。
//		CheckButton(IDC_CHK_LOHICASE, true);
//		EnableItem(IDC_CHK_LOHICASE), false);

		// 2001/06/23 N.Nakatani
		// 単語単位で探す
		EnableItem(IDC_CHK_WORD, false);
	}else {
		CheckButton(IDC_CHK_REGULAREXP, false);
	}
	// To Here Jun. 29, 2001 genta

	EnableItem(IDC_CHK_FROMTHISTEXT, szCurrentFilePath[0] != _T('\0'));

	CheckButton(IDC_CHECK_FILE_ONLY, bGrepOutputFileOnly);
	CheckButton(IDC_CHECK_BASE_PATH, bGrepOutputBaseFolder);
	CheckButton(IDC_CHECK_SEP_FOLDER, bGrepSeparateFolder);

	// フォルダの初期値をカレントフォルダにする
	CheckButton(IDC_CHK_DEFAULTFOLDER, pShareData->common.search.bGrepDefaultFolder);

	return;
}


/*!
	現在編集中ファイルから検索チェックでの設定
*/
void DlgGrep::SetDataFromThisText(bool bChecked)
{
	bool bEnableControls = true;
	if (szCurrentFilePath[0] != 0 && bChecked) {
		TCHAR szWorkFolder[MAX_PATH];
		TCHAR szWorkFile[MAX_PATH];
		// 2003.08.01 Moca ファイル名はスペースなどは区切り記号になるので、""で囲い、エスケープする
		szWorkFile[0] = _T('"');
		SplitPath_FolderAndFile(szCurrentFilePath, szWorkFolder, szWorkFile + 1);
		_tcscat(szWorkFile, _T("\"")); // 2003.08.01 Moca
		SetItemText(IDC_COMBO_FILE, szWorkFile);
		SetGrepFolder(GetItemHwnd(IDC_COMBO_FOLDER), szWorkFolder);

		CheckButton(IDC_CHK_SUBFOLDER, false);
		bEnableControls = false;
	}
	EnableItem(IDC_COMBO_FILE,    bEnableControls);
	EnableItem(IDC_COMBO_FOLDER,  bEnableControls);
	EnableItem(IDC_BUTTON_FOLDER, bEnableControls);
	EnableItem(IDC_CHK_SUBFOLDER, bEnableControls);
	return;
}

// ダイアログデータの取得
// TRUE==正常  FALSE==入力エラー
int DlgGrep::GetData(void)
{
	// サブフォルダからも検索する
	bSubFolder = IsButtonChecked(IDC_CHK_SUBFOLDER);

	auto& csSearch = pShareData->common.search;
	csSearch.bGrepSubFolder = bSubFolder;		// Grep：サブフォルダも検索

	// この編集中のテキストから検索する
	bFromThisText = IsButtonChecked(IDC_CHK_FROMTHISTEXT);
	// 英大文字と英小文字を区別する
	searchOption.bLoHiCase = IsButtonChecked(IDC_CHK_LOHICASE);

	// 2001/06/23 N.Nakatani
	// 単語単位で検索
	searchOption.bWordOnly = IsButtonChecked(IDC_CHK_WORD);

	// 正規表現
	searchOption.bRegularExp = IsButtonChecked(IDC_CHK_REGULAREXP);

	// 文字コード自動判別
//	bKanjiCode_AutoDetect = IsButtonChecked(IDC_CHK_KANJICODEAUTODETECT);

	// 文字コードセット
	{
		int		nIdx;
		HWND	hWndCombo = GetItemHwnd(IDC_COMBO_CHARSET);
		nIdx = Combo_GetCurSel(hWndCombo);
		nGrepCharSet = (EncodingType)Combo_GetItemData(hWndCombo, nIdx);
	}

	// 行を出力/該当部分/否マッチ行 を出力
	if (IsButtonChecked(IDC_RADIO_OUTPUTLINE )) {
		nGrepOutputLineType = 1;
	}else if (IsButtonChecked(IDC_RADIO_NOHIT )) {
		nGrepOutputLineType = 2;
	}else {
		nGrepOutputLineType = 0;
	}
	
	// Grep: 出力形式
	if (IsButtonChecked(IDC_RADIO_OUTPUTSTYLE1)) {
		nGrepOutputStyle = 1;				// Grep: 出力形式
	}
	if (IsButtonChecked(IDC_RADIO_OUTPUTSTYLE2)) {
		nGrepOutputStyle = 2;				// Grep: 出力形式
	}
	if (IsButtonChecked(IDC_RADIO_OUTPUTSTYLE3)) {
		nGrepOutputStyle = 3;
	}

	bGrepOutputFileOnly = IsButtonChecked(IDC_CHECK_FILE_ONLY);
	bGrepOutputBaseFolder = IsButtonChecked(IDC_CHECK_BASE_PATH);
	bGrepSeparateFolder = IsButtonChecked(IDC_CHECK_SEP_FOLDER);

	// 検索文字列
	int nBufferSize = ::GetWindowTextLength(GetItemHwnd(IDC_COMBO_TEXT)) + 1;
	std::vector<TCHAR> vText(nBufferSize);
	GetItemText(IDC_COMBO_TEXT, &vText[0], nBufferSize);
	strText = to_wchar(&vText[0]);
	bSetText = true;
	
	// 検索ファイル
	GetItemText(IDC_COMBO_FILE, szFile, _countof2(szFile));
	// 検索フォルダ
	GetItemText(IDC_COMBO_FOLDER, szFolder, _countof2(szFolder));

	csSearch.nGrepCharSet = nGrepCharSet;				// 文字コード自動判別
	csSearch.nGrepOutputLineType = nGrepOutputLineType;	// 行を出力/該当部分/否マッチ行 を出力
	csSearch.nGrepOutputStyle = nGrepOutputStyle;		// Grep: 出力形式
	csSearch.bGrepOutputFileOnly = bGrepOutputFileOnly;
	csSearch.bGrepOutputBaseFolder = bGrepOutputBaseFolder;
	csSearch.bGrepSeparateFolder = bGrepSeparateFolder;

// やめました
//	if (wcslen(szText) == 0) {
//		WarningMessage(	GetHwnd(), _T("検索のキーワードを指定してください。"));
//		return FALSE;
//	}
	if (auto_strlen(szFile) != 0) {
		GrepEnumKeys enumKeys;
		int nErrorNo = enumKeys.SetFileKeys(szFile);
		if (nErrorNo == 1) {
			WarningMessage(	GetHwnd(), LS(STR_DLGGREP2));
			return FALSE;
		}else if (nErrorNo == 2) {
			WarningMessage(	GetHwnd(), LS(STR_DLGGREP3));
			return FALSE;
		}
	}
	// この編集中のテキストから検索する
	if (szFile[0] == _T('\0')) {
		// Jun. 16, 2003 Moca
		// 検索パターンが指定されていない場合のメッセージ表示をやめ、
		//「*.*」が指定されたものと見なす．
		_tcscpy(szFile, _T("*.*"));
	}
	if (szFolder[0] == _T('\0')) {
		WarningMessage(	GetHwnd(), LS(STR_DLGGREP4));
		return FALSE;
	}

	{
		// カレントディレクトリを保存。このブロックから抜けるときに自動でカレントディレクトリは復元される。
		CurrentDirectoryBackupPoint cCurDirBackup;

		// 2011.11.24 Moca 複数フォルダ指定
		std::vector<std::tstring> paths;
		GrepAgent::CreateFolders(szFolder, paths);
		size_t nFolderLen = 0;
		TCHAR szFolder[_MAX_PATH];
		szFolder[0] = _T('\0');
		for (size_t i=0; i<paths.size(); ++i) {
			// 相対パス→絶対パス
			if (!::SetCurrentDirectory(paths[i].c_str())) {
				WarningMessage(	GetHwnd(), LS(STR_DLGGREP5));
				return FALSE;
			}
			TCHAR szFolderItem[_MAX_PATH];
			::GetCurrentDirectory(_MAX_PATH, szFolderItem);
			// ;がフォルダ名に含まれていたら""で囲う
			if (auto_strchr(szFolderItem, _T(';'))) {
				szFolderItem[0] = _T('"');
				::GetCurrentDirectory(_MAX_PATH, szFolderItem + 1);
				auto_strcat(szFolderItem, _T("\""));
			}
			size_t nFolderItemLen = auto_strlen(szFolderItem);
			if (_MAX_PATH < nFolderLen + nFolderItemLen + 1) {
				WarningMessage(	GetHwnd(), LS(STR_DLGGREP6));
				return FALSE;
			}
			if (i) {
				auto_strcat(szFolder, _T(";"));
			}
			auto_strcat(szFolder, szFolderItem);
			nFolderLen = auto_strlen(szFolder);
		}
		auto_strcpy(szFolder, szFolder);
	}

//@@@ 2002.2.2 YAZAKI CShareData.AddToSearchKeys()追加に伴う変更
	// 検索文字列
	if (0 < strText.size()) {
		// From Here Jun. 26, 2001 genta
		// 正規表現ライブラリの差し替えに伴う処理の見直し
		int nFlag = 0;
		nFlag |= searchOption.bLoHiCase ? 0x01 : 0x00;
		if (searchOption.bRegularExp  && !CheckRegexpSyntax(strText.c_str(), GetHwnd(), true, nFlag)) {
			return FALSE;
		}
		// To Here Jun. 26, 2001 genta 正規表現ライブラリ差し替え
		if (strText.size() < _MAX_PATH) {
			SearchKeywordManager().AddToSearchKeys(strText.c_str());
			csSearch.searchOption = searchOption;		// 検索オプション
		}
	}else {
		// 2014.07.01 空キーも登録する
		SearchKeywordManager().AddToSearchKeys( L"" );
	}

	// この編集中のテキストから検索する場合、履歴に残さない	Uchi 2008/5/23
	if (!bFromThisText) {
		// 検索ファイル
		SearchKeywordManager().AddToGrepFiles(szFile);

		// 検索フォルダ
		SearchKeywordManager().AddToGrepFolders(szFolder);
	}

	return TRUE;
}

//@@@ 2002.01.18 add start
LPVOID DlgGrep::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


static void SetGrepFolder(HWND hwndCtrl, LPCTSTR folder)
{
	if (auto_strchr(folder, _T(';'))) {
		TCHAR szQuoteFolder[MAX_PATH];
		szQuoteFolder[0] = _T('"');
		auto_strcpy(szQuoteFolder + 1, folder);
		auto_strcat(szQuoteFolder, _T("\""));
		::SetWindowText(hwndCtrl, szQuoteFolder);
	}else {
		::SetWindowText(hwndCtrl, folder);
	}
}

