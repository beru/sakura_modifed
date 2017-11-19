#include "StdAfx.h"
#include <HtmlHelp.h>
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "PropertyManager.h"
#include "EditApp.h"
#include "dlg/DlgAbout.h"
#include "env/HelpManager.h"
#include "util/module.h"
#include "util/shell.h"

// ViewCommanderクラスのコマンド(支援)関数群

/*!	入力補完
	Ctrl+Spaceでここに到着。
	CEditView::bHokan： 現在補完ウィンドウが表示されているかを表すフラグ。
	common.helper.bUseHokan：現在補完ウィンドウが表示されているべきか否かをあらわすフラグ。
*/
void ViewCommander::Command_Hokan(void)
{
	auto& csHelper = GetDllShareData().common.helper;
	if (!csHelper.bUseHokan) {
		csHelper.bUseHokan = true;
	}
	NativeW memData;
	// カーソル直前の単語を取得
	if (0 < view.GetParser().GetLeftWord(&memData, 100)) {
		view.ShowHokanMgr(memData, true);
	}else {
		InfoBeep();
		view.SendStatusMessage(LS(STR_SUPPORT_NOT_COMPLITE)); // ステータスで表示
		csHelper.bUseHokan = false;	// 入力補完終了のお知らせ
	}
	return;
}


/*! キャレット位置の単語を辞書検索ON-OFF */
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
void ViewCommander::Command_Help_Contents(void)
{
	ShowWinHelpContents(view.GetHwnd());	// 目次を表示する
	return;
}


// ヘルプキーワード検索
void ViewCommander::Command_Help_Search(void)
{
	MyWinHelp(view.GetHwnd(), HELP_KEY, (ULONG_PTR)_T(""));
	return;
}


// コマンド一覧
void ViewCommander::Command_Menu_AllFunc(void)
{
	POINT	po;
	RECT	rc;

	po.x = 540;
	po.y = 0;

	auto& editWnd = GetEditWindow();
	::GetClientRect(editWnd.GetHwnd(), &rc);
	po.x = t_min(po.x, rc.right);
	::ClientToScreen(editWnd.GetHwnd(), &po);
	::GetWindowRect(editWnd.splitterWnd.GetHwnd() , &rc);
	po.y = rc.top;

	editWnd.GetMenuDrawer().ResetContents();

	FuncLookup& FuncLookup = GetDocument().funcLookup;
	HMENU hMenu = ::CreatePopupMenu();
	for (size_t i=0; i<FuncLookup.GetCategoryCount(); ++i) {
		HMENU hMenuPopUp = ::CreatePopupMenu();
		for (size_t j=0; j<FuncLookup.GetItemCount(i); ++j) {
			int code = FuncLookup.Pos2FuncCode(i, j, false);	// 未登録マクロ非表示を明示指定
			if (code != 0) {
				wchar_t szLabel[300];
				FuncLookup.Pos2FuncName(i, j, szLabel, 256);
				UINT uFlags = MF_BYPOSITION | MF_STRING | MF_ENABLED;
				editWnd.GetMenuDrawer().MyAppendMenu(hMenuPopUp, uFlags, code, szLabel, L"");
			}
		}
		editWnd.GetMenuDrawer().MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , FuncLookup.Category2Name(i) , _T(""));
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
		::PostMessage(GetMainWindow(), WM_COMMAND, MAKELONG(nId, 0), (LPARAM)NULL);
	}
	return;
}


/* 外部ヘルプ１ */
void ViewCommander::Command_ExtHelp1(void)
{
retry:;
	if (!HelpManager().ExtWinHelpIsSet(&(GetDocument().docType.GetDocumentAttribute()))) {
//	if (wcslen(GetDllShareData().common.szExtHelp1) == 0) {
		ErrorBeep();
//		[Esc]キーと[x]ボタンでも中止できるように変更
		if (::MYMESSAGEBOX(
				NULL,
				MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST,
				GSTR_APPNAME,
				LS(STR_ERR_CEDITVIEW_CMD01)
			) == IDYES
		) {
			// 共通設定 プロパティシート
			if (!EditApp::getInstance().OpenPropertySheet(ID_PROPCOM_PAGENUM_HELPER)) {
				return;
			}
			goto retry;
		}else {
			return;
		}
	}

	NativeW memCurText;
	const TCHAR* helpfile = HelpManager().GetExtWinHelp(&(GetDocument().docType.GetDocumentAttribute()));

	// 現在カーソル位置単語または選択範囲より検索等のキーを取得
	view.GetCurrentTextForSearch(memCurText, false);
	TCHAR path[_MAX_PATH];
	if (_IS_REL_PATH(helpfile)) {
		GetInidirOrExedir(path, helpfile);
	}else {
		auto_strcpy(path, helpfile);
	}
	TCHAR	szExt[_MAX_EXT];
	_tsplitpath(path, NULL, NULL, NULL, szExt);
	if (_tcsicmp(szExt, _T(".chi")) == 0 || _tcsicmp(szExt, _T(".chm")) == 0 || _tcsicmp(szExt, _T(".col")) == 0) {
		std::wstring pathw = to_wchar(path);
		Command_ExtHTMLHelp(pathw.c_str(), memCurText.GetStringPtr());
	}else {
		::WinHelp(view.hwndParent, path, HELP_KEY, (ULONG_PTR)memCurText.GetStringPtr());
	}
	return;
}


/*!
	外部HTMLヘルプ
	
	@param helpfile [in] HTMLヘルプファイル名．NULLのときはタイプ別に設定されたファイル．
	@param kwd [in] 検索キーワード．NULLのときはカーソル位置or選択されたワード
*/
void ViewCommander::Command_ExtHTMLHelp(const wchar_t* _helpfile, const wchar_t* kwd)
{
	std::tstring helpfile;
	if (_helpfile) {
		helpfile = to_tchar(_helpfile);
	}

	HWND hwndHtmlHelp;

	DEBUG_TRACE(_T("helpfile=%ts\n"), helpfile.c_str());

	const TCHAR* filename = NULL;
	if (helpfile.length() == 0) {
		while (!HelpManager().ExtHTMLHelpIsSet(&(GetDocument().docType.GetDocumentAttribute()))) {
			ErrorBeep();
	//		[Esc]キーと[x]ボタンでも中止できるように変更
			if (::MYMESSAGEBOX(
					NULL,
					MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST,
					GSTR_APPNAME,
					LS(STR_ERR_CEDITVIEW_CMD02)
				) != IDYES
			) {
				return;
			}
			// 共通設定 プロパティシート
			if (!EditApp::getInstance().OpenPropertySheet(ID_PROPCOM_PAGENUM_HELPER)) {
				return;
			}
		}
		filename = HelpManager().GetExtHTMLHelp(&(GetDocument().docType.GetDocumentAttribute()));
	}else {
		filename = helpfile.c_str();
	}

	// キーワードの外部指定を可能に
	NativeW	memCurText;
	if (kwd && kwd[0] != _T('\0')) {
		memCurText.SetString(kwd);
	}else {
		// 現在カーソル位置単語または選択範囲より検索等のキーを取得
		view.GetCurrentTextForSearch(memCurText);
	}

	// HtmlHelpビューアはひとつ
	if (HelpManager().HTMLHelpIsSingle(&(GetDocument().docType.GetDocumentAttribute()))) {
		// タスクトレイのプロセスにHtmlHelpを起動させる
		TCHAR* pWork = GetDllShareData().workBuffer.GetWorkBuffer<TCHAR>();
		if (_IS_REL_PATH(filename)) {
			GetInidirOrExedir(pWork, filename);
		}else {
			_tcscpy(pWork, filename);
		}
		size_t nLen = _tcslen(pWork);
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

		if (_IS_REL_PATH(filename)) {
			TCHAR path[_MAX_PATH];
			GetInidirOrExedir(path, filename);
			hwndHtmlHelp = OpenHtmlHelp(
				NULL/*GetDllShareData().handles.hwndTray*/,
				path,
				HH_KEYWORD_LOOKUP,
				(DWORD_PTR)&link
			);
		}else {
			hwndHtmlHelp = OpenHtmlHelp(
				NULL/*GetDllShareData().handles.hwndTray*/,
				filename,
				HH_KEYWORD_LOOKUP,
				(DWORD_PTR)&link
			);
		}
	}

	if (hwndHtmlHelp) {
		::BringWindowToTop(hwndHtmlHelp);
	}

	return;
}


// バージョン情報
void ViewCommander::Command_About(void)
{
	DlgAbout dlgAbout;
	dlgAbout.DoModal(G_AppInstance(), view.GetHwnd());
	return;
}

