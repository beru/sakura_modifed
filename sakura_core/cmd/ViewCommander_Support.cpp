/*!	@file
@brief ViewCommanderクラスのコマンド(支援)関数群

	2012/12/16	ViewCommander_Hokan.cppにcpp,ViewCommander.cppから支援関連を分離しViewCommander_Support.cppに名称変更
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, asa-o
	Copyright (C) 2002, genta, aroka
	Copyright (C) 2003, Moca
	Copyright (C) 2005, genta
	Copyright (C) 2006, ryoji, fon
	Copyright (C) 2007, kobake, ryoji
	Copyright (C) 2011, Moca
	Copyright (C) 2012, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <HtmlHelp.h>
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "PropertyManager.h"
#include "EditApp.h"
#include "dlg/DlgAbout.h"	// Dec. 24, 2000 JEPRO 追加
#include "env/HelpManager.h"
#include "util/module.h"
#include "util/shell.h"

/*!	入力補完
	Ctrl+Spaceでここに到着。
	CEditView::m_bHokan： 現在補完ウィンドウが表示されているかを表すフラグ。
	common.helper.bUseHokan：現在補完ウィンドウが表示されているべきか否かをあらわすフラグ。

    @date 2001/06/19 asa-o 英大文字小文字を同一視する
                     候補が1つのときはそれに確定する
	@date 2001/06/14 asa-o 参照データ変更
	                 開くプロパティシートをタイプ別に変更y
	@date 2000/09/15 JEPRO [Esc]キーと[x]ボタンでも中止できるように変更
	@date 2005/01/10 genta CEditView_Commandから移動
*/
void ViewCommander::Command_HOKAN(void)
{
	auto& csHelper = GetDllShareData().common.helper;
	if (!csHelper.bUseHokan) {
		csHelper.bUseHokan = TRUE;
	}
#if 0
// 2011.06.24 Moca Plugin導入に従い未設定の確認をやめる
retry:;
	// 補完候補一覧ファイルが設定されていないときは、設定するように促す。
	// 2003.06.22 Moca ファイル内から検索する場合には補完ファイルの設定は必須ではない
	if (!m_pCommanderView->m_pTypeData->bUseHokanByFile &&
		!m_pCommanderView->m_pTypeData->bUseHokanByKeyword &&
		m_pCommanderView->m_pTypeData->szHokanFile[0] == _T('\0')
	) {
		ConfirmBeep();
		if (::ConfirmMessage(
				GetMainWindow(),
				LS(STR_ERR_DLGEDITVWHOKAN1)
			) == IDYES
		) {
			// タイプ別設定 プロパティシート
			if (!CEditApp::getInstance()->m_pPropertyManager->OpenPropertySheetTypes(2, GetDocument()->m_docType.GetDocumentType())) {
				return;
			}
			goto retry;
		}
	}
#endif
	NativeW memData;
	// カーソル直前の単語を取得
	if (0 < m_pCommanderView->GetParser().GetLeftWord(&memData, 100)) {
		m_pCommanderView->ShowHokanMgr(memData, true);
	}else {
		InfoBeep(); // 2010.04.03 Error→Info
		m_pCommanderView->SendStatusMessage(LS(STR_SUPPORT_NOT_COMPLITE)); // 2010.05.29 ステータスで表示
		csHelper.bUseHokan = FALSE;	// 入力補完終了のお知らせ
	}
	return;
}


/*! キャレット位置の単語を辞書検索ON-OFF

	@date 2006.03.24 fon 新規作成
*/
void ViewCommander::Command_ToggleKeySearch(int option)
{	// 共通設定ダイアログの設定をキー割り当てでも切り替えられるように
	auto& bUseCaretKeyword = GetDllShareData().common.search.bUseCaretKeyword;
	if (option == 0) {
		bUseCaretKeyword = !bUseCaretKeyword;
	}else if (option == 1) {
		bUseCaretKeyword = true;
	}else if (option == 2) {
		bUseCaretKeyword = false;
	}
}


// ヘルプ目次
void ViewCommander::Command_HELP_CONTENTS(void)
{
	ShowWinHelpContents(m_pCommanderView->GetHwnd());	// 目次を表示する
	return;
}


// ヘルプキーワード検索
void ViewCommander::Command_HELP_SEARCH(void)
{
	MyWinHelp(m_pCommanderView->GetHwnd(), HELP_KEY, (ULONG_PTR)_T(""));	// 2006.10.10 ryoji MyWinHelpに変更に変更
	return;
}


// コマンド一覧
void ViewCommander::Command_MENU_ALLFUNC(void)
{
	POINT	po;
	RECT	rc;

// From Here Sept. 15, 2000 JEPRO
// サブメニュー、特に「その他」のコマンドに対してステータスバーに表示されるキーアサイン情報が
// メニューで隠れないように右にずらした
// (本当はこの「コマンド一覧」メニューをダイアログに変更しバーをつまんで自由に移動できるようにしたい)
//	po.x = 0;
	po.x = 540;
//	To Here Sept. 15, 2000 (Oct. 7, 2000 300→500; Nov. 3, 2000 500→540)
	po.y = 0;

	EditWnd* pEditWnd = GetEditWindow();	// Sep. 10, 2002 genta
	::GetClientRect(pEditWnd->GetHwnd(), &rc);
	po.x = t_min(po.x, rc.right);
	::ClientToScreen(pEditWnd->GetHwnd(), &po);
	::GetWindowRect(pEditWnd->m_splitterWnd.GetHwnd() , &rc);
	po.y = rc.top;

	pEditWnd->GetMenuDrawer().ResetContents();

	// Oct. 3, 2001 genta
	FuncLookup& FuncLookup = GetDocument()->m_funcLookup;
	HMENU hMenu = ::CreatePopupMenu();
// Oct. 14, 2000 JEPRO 「--未定義--」を表示させないように変更したことで1番(カーソル移動系)が前にシフトされた(この変更によって i=1→i=0 と変更)
	// Oct. 3, 2001 genta
	for (int i=0; i<FuncLookup.GetCategoryCount(); ++i) {
		HMENU hMenuPopUp = ::CreatePopupMenu();
		for (int j=0; j<FuncLookup.GetItemCount(i); ++j) {
			// Oct. 3, 2001 genta
			int code = FuncLookup.Pos2FuncCode(i, j, false);	// 2007.11.02 ryoji 未登録マクロ非表示を明示指定
			if (code != 0) {
				WCHAR	szLabel[300];
				FuncLookup.Pos2FuncName(i, j, szLabel, 256);
				UINT uFlags = MF_BYPOSITION | MF_STRING | MF_ENABLED;
				// Oct. 3, 2001 genta
				pEditWnd->GetMenuDrawer().MyAppendMenu(hMenuPopUp, uFlags, code, szLabel, L"");
			}
		}
		// Oct. 3, 2001 genta
		pEditWnd->GetMenuDrawer().MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , FuncLookup.Category2Name(i) , _T(""));
//		pEditWnd->GetMenuDrawer().MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , nsFuncCode::ppszFuncKind[i]);
	}
	int nId = ::TrackPopupMenu(
		hMenu,
		TPM_TOPALIGN
		| TPM_LEFTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		,
		po.x,
		po.y,
		0,
		GetMainWindow()/*GetHwnd()*/,
		NULL
	);
	::DestroyMenu(hMenu);
	if (nId != 0) {
		// コマンドコードによる処理振り分け
//		HandleCommand(nFuncID, true, 0, 0, 0, 0);
		::PostMessage(GetMainWindow(), WM_COMMAND, MAKELONG(nId, 0), (LPARAM)NULL);
	}
	return;
}


/* 外部ヘルプ１
	@date 2012.09.26 Moca HTMLHELP対応
*/
void ViewCommander::Command_EXTHELP1(void)
{
retry:;
	if (!HelpManager().ExtWinHelpIsSet(&(GetDocument()->m_docType.GetDocumentAttribute()))) {
//	if (wcslen(GetDllShareData().common.m_szExtHelp1) == 0) {
		ErrorBeep();
// From Here Sept. 15, 2000 JEPRO
//		[Esc]キーと[x]ボタンでも中止できるように変更
		if (::MYMESSAGEBOX(
				NULL,
				MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST,
				GSTR_APPNAME,
// To Here Sept. 15, 2000
				LS(STR_ERR_CEDITVIEW_CMD01)
			) == IDYES
		) {
			// 共通設定 プロパティシート
			if (!EditApp::getInstance()->OpenPropertySheet(ID_PROPCOM_PAGENUM_HELPER)) {
				return;
			}
			goto retry;
		}else {
		// Jun. 15, 2000 genta
			return;
		}
	}

	NativeW memCurText;
	const TCHAR* helpfile = HelpManager().GetExtWinHelp(&(GetDocument()->m_docType.GetDocumentAttribute()));

	// 現在カーソル位置単語または選択範囲より検索等のキーを取得
	m_pCommanderView->GetCurrentTextForSearch(memCurText, false);
	TCHAR path[_MAX_PATH];
	if (_IS_REL_PATH(helpfile)) {
		// 2003.06.23 Moca 相対パスは実行ファイルからのパス
		// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
		GetInidirOrExedir(path, helpfile);
	}else {
		auto_strcpy(path, helpfile);
	}
	// 2012.09.26 Moca HTMLHELP対応
	TCHAR	szExt[_MAX_EXT];
	_tsplitpath(path, NULL, NULL, NULL, szExt);
	if (_tcsicmp(szExt, _T(".chi")) == 0 || _tcsicmp(szExt, _T(".chm")) == 0 || _tcsicmp(szExt, _T(".col")) == 0) {
		std::wstring pathw = to_wchar(path);
		Command_EXTHTMLHELP(pathw.c_str(), memCurText.GetStringPtr());
	}else {
		::WinHelp(m_pCommanderView->m_hwndParent, path, HELP_KEY, (ULONG_PTR)memCurText.GetStringPtr());
	}
	return;
}


/*!
	外部HTMLヘルプ
	
	@param helpfile [in] HTMLヘルプファイル名．NULLのときはタイプ別に設定されたファイル．
	@param kwd [in] 検索キーワード．NULLのときはカーソル位置or選択されたワード
	@date 2002.07.05 genta 任意のファイル・キーワードの指定ができるよう引数追加
*/
void ViewCommander::Command_EXTHTMLHELP(const WCHAR* _helpfile, const WCHAR* kwd)
{
	std::tstring helpfile;
	if (_helpfile) {
		helpfile = to_tchar(_helpfile);
	}

	HWND hwndHtmlHelp;

	DEBUG_TRACE(_T("helpfile=%ts\n"), helpfile.c_str());

	// From Here Jul. 5, 2002 genta
	const TCHAR* filename = NULL;
	if (helpfile.length() == 0) {
		while (!HelpManager().ExtHTMLHelpIsSet(&(GetDocument()->m_docType.GetDocumentAttribute()))) {
			ErrorBeep();
	// From Here Sept. 15, 2000 JEPRO
	//		[Esc]キーと[x]ボタンでも中止できるように変更
			if (IDYES != ::MYMESSAGEBOX(
					NULL,
					MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST,
					GSTR_APPNAME,
	// To Here Sept. 15, 2000
					LS(STR_ERR_CEDITVIEW_CMD02)
				)
			) {
				return;
			}
			// 共通設定 プロパティシート
			if (!EditApp::getInstance()->OpenPropertySheet(ID_PROPCOM_PAGENUM_HELPER)) {
				return;
			}
		}
		filename = HelpManager().GetExtHTMLHelp(&(GetDocument()->m_docType.GetDocumentAttribute()));
	}else {
		filename = helpfile.c_str();
	}
	// To Here Jul. 5, 2002 genta

	// Jul. 5, 2002 genta
	// キーワードの外部指定を可能に
	NativeW	memCurText;
	if (kwd && kwd[0] != _T('\0')) {
		memCurText.SetString(kwd);
	}else {
		// 現在カーソル位置単語または選択範囲より検索等のキーを取得
		m_pCommanderView->GetCurrentTextForSearch(memCurText);
	}

	// HtmlHelpビューアはひとつ
	if (HelpManager().HTMLHelpIsSingle(&(GetDocument()->m_docType.GetDocumentAttribute()))) {
		// タスクトレイのプロセスにHtmlHelpを起動させる
		// 2003.06.23 Moca 相対パスは実行ファイルからのパス
		// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
		TCHAR* pWork = GetDllShareData().workBuffer.GetWorkBuffer<TCHAR>();
		if (_IS_REL_PATH(filename)) {
			GetInidirOrExedir(pWork, filename);
		}else {
			_tcscpy(pWork, filename); // Jul. 5, 2002 genta
		}
		int nLen = _tcslen(pWork);
		_tcscpy(&pWork[nLen + 1], memCurText.GetStringT());
		hwndHtmlHelp = (HWND)::SendMessage(
			GetDllShareData().handles.hwndTray,
			MYWM_HTMLHELP,
			(WPARAM)GetMainWindow(),
			0
		);
	}else {
		// 自分でHtmlHelpを起動させる
		HH_AKLINK	link;
		link.cbStruct = sizeof(link) ;
		link.fReserved = FALSE ;
		link.pszKeywords = memCurText.GetStringT();
		link.pszUrl = NULL;
		link.pszMsgText = NULL;
		link.pszMsgTitle = NULL;
		link.pszWindow = NULL;
		link.fIndexOnFail = TRUE;

		// 2003.06.23 Moca 相対パスは実行ファイルからのパス
		// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
		if (_IS_REL_PATH(filename)) {
			TCHAR path[_MAX_PATH];
			GetInidirOrExedir(path, filename);
			// Jul. 6, 2001 genta HtmlHelpの呼び出し方法変更
			hwndHtmlHelp = OpenHtmlHelp(
				NULL/*GetDllShareData().handles.hwndTray*/,
				path, // Jul. 5, 2002 genta
				HH_KEYWORD_LOOKUP,
				(DWORD_PTR)&link
			);
		}else {
			// Jul. 6, 2001 genta HtmlHelpの呼び出し方法変更
			hwndHtmlHelp = OpenHtmlHelp(
				NULL/*GetDllShareData().handles.hwndTray*/,
				filename, // Jul. 5, 2002 genta
				HH_KEYWORD_LOOKUP,
				(DWORD_PTR)&link
			);
		}
	}

	// Jul. 6, 2001 genta hwndHtmlHelpのチェックを追加
	if (hwndHtmlHelp) {
		::BringWindowToTop(hwndHtmlHelp);
	}

	return;
}


// バージョン情報
void ViewCommander::Command_ABOUT(void)
{
	DlgAbout dlgAbout;
	dlgAbout.DoModal(G_AppInstance(), m_pCommanderView->GetHwnd());
	return;
}

