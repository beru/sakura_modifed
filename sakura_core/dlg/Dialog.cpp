/*!	@file
	@brief Dialog Boxの基底クラス

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, jepro, Stonee
	Copyright (C) 2002, aroka, YAZAKI
	Copyright (C) 2003, MIK, KEITA
	Copyright (C) 2005, MIK
	Copyright (C) 2006, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2011, nasukoji
	Copyright (C) 2012, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "dlg/Dialog.h"
#include "EditApp.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "DlgOpenFile.h"
#include "recent/Recent.h"
#include "util/os.h"
#include "util/shell.h"
#include "util/module.h"

// ダイアログプロシージャ
INT_PTR CALLBACK MyDialogProc(
	HWND hwndDlg,	// handle to dialog box
	UINT uMsg,		// message
	WPARAM wParam,	// first message parameter
	LPARAM lParam 	// second message parameter
	)
{
	Dialog* pDialog;
	switch (uMsg) {
	case WM_INITDIALOG:
		pDialog = (Dialog*) lParam;
		if (pDialog) {
			return pDialog->DispatchEvent(hwndDlg, uMsg, wParam, lParam);
		}else {
			return FALSE;
		}
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pDialog = (Dialog*)::GetWindowLongPtr(hwndDlg, DWLP_USER);
		if (pDialog) {
			return pDialog->DispatchEvent(hwndDlg, uMsg, wParam, lParam);
		}else {
			return FALSE;
		}
	}
}


/*!	コンストラクタ

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
Dialog::Dialog(bool bSizable, bool bCheckShareData)
{
//	MYTRACE(_T("Dialog::Dialog()\n"));
	// 共有データ構造体のアドレスを返す
	pShareData = &GetDllShareData(bCheckShareData);

	hInstance = NULL;		// アプリケーションインスタンスのハンドル
	hwndParent = NULL;	// オーナーウィンドウのハンドル
	hWnd  = NULL;			// このダイアログのハンドル
	hwndSizeBox = NULL;
	bSizable = bSizable;
	lParam = (LPARAM)NULL;
	nShowCmd = SW_SHOW;
	xPos = -1;
	yPos = -1;
	nWidth = -1;
	nHeight = -1;

	return;
}

Dialog::~Dialog()
{
//	MYTRACE(_T("Dialog::~Dialog()\n"));
	CloseDialog(0);
	return;
}

// モーダルダイアログの表示
/*!
	@param hInstance [in] アプリケーションインスタンスのハンドル
	@param hwndParent [in] オーナーウィンドウのハンドル

	@date 2011.04.10 nasukoji	各国語メッセージリソース対応
*/
INT_PTR Dialog::DoModal(
	HINSTANCE hInstance,
	HWND hwndParent,
	int nDlgTemplete,
	LPARAM lParam
	)
{
	bInited = false;
	bModal = true;
	this->hInstance = hInstance;	// アプリケーションインスタンスのハンドル
	this->hwndParent = hwndParent;	// オーナーウィンドウのハンドル
	this->lParam = lParam;
	hLangRsrcInstance = SelectLang::getLangRsrcInstance();		// メッセージリソースDLLのインスタンスハンドル
	return ::DialogBoxParam(
		this->hLangRsrcInstance,
		MAKEINTRESOURCE(nDlgTemplete),
		hwndParent,
		MyDialogProc,
		(LPARAM)this
	);
}

// モードレスダイアログの表示
/*!
	@param hInstance [in] アプリケーションインスタンスのハンドル
	@param hwndParent [in] オーナーウィンドウのハンドル

	@date 2011.04.10 nasukoji	各国語メッセージリソース対応
*/
HWND Dialog::DoModeless(
	HINSTANCE hInstance,
	HWND hwndParent,
	int nDlgTemplete,
	LPARAM lParam,
	int nCmdShow
	)
{
	bInited = false;
	bModal = false;
	this->hInstance = hInstance;	// アプリケーションインスタンスのハンドル
	this->hwndParent = hwndParent;	// オーナーウィンドウのハンドル
	this->lParam = lParam;
	hLangRsrcInstance = SelectLang::getLangRsrcInstance();		// メッセージリソースDLLのインスタンスハンドル
	hWnd = ::CreateDialogParam(
		hLangRsrcInstance,
		MAKEINTRESOURCE(nDlgTemplete),
		hwndParent,
		MyDialogProc,
		(LPARAM)this
	);
	if (hWnd) {
		::ShowWindow(hWnd, nCmdShow);
	}
	return hWnd;
}

HWND Dialog::DoModeless(
	HINSTANCE hInstance,
	HWND hwndParent,
	LPCDLGTEMPLATE lpTemplate,
	LPARAM lParam,
	int nCmdShow
	)
{
	bInited = false;
	bModal = false;
	this->hInstance = hInstance;	// アプリケーションインスタンスのハンドル
	this->hwndParent = hwndParent;	// オーナーウィンドウのハンドル
	this->lParam = lParam;
	hWnd = ::CreateDialogIndirectParam(
		hInstance,
		lpTemplate,
		hwndParent,
		MyDialogProc,
		(LPARAM)this
	);
	if (hWnd) {
		::ShowWindow(hWnd, nCmdShow);
	}
	return hWnd;
}

void Dialog::CloseDialog(int nModalRetVal)
{
	if (hWnd) {
		if (bModal) {
			::EndDialog(hWnd, nModalRetVal);
		}else {
			::DestroyWindow(hWnd);
		}
		hWnd = NULL;
	}
	return;
}


BOOL Dialog::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	hWnd = hwndDlg;
	// Modified by KEITA for WIN64 2003.9.6
	::SetWindowLongPtr(hWnd, DWLP_USER, lParam);

	// ダイアログデータの設定
	SetData();

	SetDialogPosSize();

	bInited = true;
	return TRUE;
}

void Dialog::SetDialogPosSize()
{
#if 0
	// ダイアログのサイズ、位置の再現
	if (xPos != -1 && yPos != -1) {
		::SetWindowPos(hWnd, NULL, xPos, yPos, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
		DEBUG_TRACE(_T("Dialog::OnInitDialog() xPos=%d yPos=%d\n"), xPos, yPos);
	}
	if (nWidth != -1 && nHeight != -1) {
		::SetWindowPos(hWnd, NULL, 0, 0, nWidth, nHeight, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
	}
#endif

	if (xPos != -1 && yPos != -1) {
		// ウィンドウ位置・サイズを再現
		// 2014.11.28 フォント変更対応
		if (nWidth == -1) {
			RECT rc;
			::GetWindowRect(hWnd, &rc);
			nWidth = rc.right - rc.left;
			nHeight = rc.bottom - rc.top;
		}

		if (!(::GetWindowLongPtr(hWnd, GWL_STYLE) & WS_CHILD)) {
			// 2006.06.09 ryoji
			// モニタのワーク領域よりも左右上下に１ドット小さい領域内に全体が収まるように位置調整する
			//
			// note: ダイアログをワーク領域境界にぴったり合わせようとすると、
			//       強制的に親の中央に移動させられてしまうときがある
			//      （マルチモニタ環境で親が非プライマリモニタにある場合だけ？）
			//       状況に合わせて処理を変えるのは厄介なので、一律、１ドットの空きを入れる

			RECT rc;
			RECT rcWork;
			rc.left = xPos;
			rc.top = yPos;
			rc.right = xPos + nWidth;
			rc.bottom = yPos + nHeight;
			GetMonitorWorkRect(&rc, &rcWork);
			rcWork.top += 1;
			rcWork.bottom -= 1;
			rcWork.left += 1;
			rcWork.right -= 1;
			if (rc.bottom > rcWork.bottom) {
				rc.top -= (rc.bottom - rcWork.bottom);
				rc.bottom = rcWork.bottom;
			}
			if (rc.right > rcWork.right) {
				rc.left -= (rc.right - rcWork.right);
				rc.right = rcWork.right;
			}
			if (rc.top < rcWork.top) {
				rc.bottom += (rcWork.top - rc.top);
				rc.top = rcWork.top;
			}
			if (rc.left < rcWork.left) {
				rc.right += (rcWork.left - rc.left);
				rc.left = rcWork.left;
			}
			xPos = rc.left;
			yPos = rc.top;
			nWidth = rc.right - rc.left;
			nHeight = rc.bottom - rc.top;
		}

		WINDOWPLACEMENT windowPlacement;
		windowPlacement.length = sizeof(windowPlacement);
		windowPlacement.showCmd = nShowCmd;	// 最大化・最小化
		windowPlacement.rcNormalPosition.left = xPos;
		windowPlacement.rcNormalPosition.top = yPos;
		windowPlacement.rcNormalPosition.right = nWidth + xPos;
		windowPlacement.rcNormalPosition.bottom = nHeight + yPos;
		::SetWindowPlacement(hWnd, &windowPlacement);
	}
}

BOOL Dialog::OnDestroy(void)
{
	// ウィンドウ位置・サイズを記憶
	WINDOWPLACEMENT windowPlacement;
	windowPlacement.length = sizeof(windowPlacement);
	if (::GetWindowPlacement(hWnd, &windowPlacement)) {
		nShowCmd = windowPlacement.showCmd;	// 最大化・最小化
		xPos = windowPlacement.rcNormalPosition.left;
		yPos = windowPlacement.rcNormalPosition.top;
		nWidth = windowPlacement.rcNormalPosition.right - windowPlacement.rcNormalPosition.left;
		nHeight = windowPlacement.rcNormalPosition.bottom - windowPlacement.rcNormalPosition.top;
		// 2014.11.28 フォント変更によるサイズ変更対応
		if (!bSizable) {
			nWidth = -1;
			nHeight = -1;
		}
	}
	// 破棄
	if (hwndSizeBox) {
		::DestroyWindow(hwndSizeBox);
		hwndSizeBox = NULL;
	}
	hWnd = NULL;
	return TRUE;
}


BOOL Dialog::OnBnClicked(int wID)
{
	switch (wID) {
	case IDCANCEL:	// Fall through.
	case IDOK:
		CloseDialog(wID);
		return TRUE;
	}
	return FALSE;
}

BOOL Dialog::OnSize()
{
	return Dialog::OnSize(0, 0);
}

BOOL Dialog::OnSize(WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	::GetWindowRect(hWnd, &rc);

	// ダイアログのサイズの記憶
	xPos = rc.left;
	yPos = rc.top;
	nWidth = rc.right - rc.left;
	nHeight = rc.bottom - rc.top;

	// サイズボックスの移動
	if (hwndSizeBox) {
		::GetClientRect(hWnd, &rc);
//		::SetWindowPos(hwndSizeBox, NULL,
// Sept. 17, 2000 JEPRO_16thdot アイコンの16dot目が表示されるように次行を変更する必要ある？
// Jan. 12, 2001 JEPRO (directed by stonee) 15を16に変更するとアウトライン解析のダイアログの右下にある
// グリップサイズに`遊び'ができてしまい(移動する！)、ダイアログを大きくできないという障害が発生するので
// 変更しないことにした(要するに原作版に戻しただけ)
//			rc.right - rc.left - 15, rc.bottom - rc.top - 15,
//			13, 13,
//			SWP_NOOWNERZORDER | SWP_NOZORDER
//		);

// Jan. 12, 2001 Stonee (suggested by genta)
//		"13"という固定値ではなくシステムから取得したスクロールバーサイズを使うように修正
		::SetWindowPos(hwndSizeBox, NULL,
			rc.right - rc.left - GetSystemMetrics(SM_CXVSCROLL), //<-- stonee
			rc.bottom - rc.top - GetSystemMetrics(SM_CYHSCROLL), //<-- stonee
			GetSystemMetrics(SM_CXVSCROLL), //<-- stonee
			GetSystemMetrics(SM_CYHSCROLL), //<-- stonee
			SWP_NOOWNERZORDER | SWP_NOZORDER
		);

		// SizeBox問題テスト
		if (wParam == SIZE_MAXIMIZED) {
			::ShowWindow(hwndSizeBox, SW_HIDE);
		}else {
			::ShowWindow(hwndSizeBox, SW_SHOW);
		}
		::InvalidateRect(hwndSizeBox, NULL, TRUE);
	}
	return FALSE;

}

BOOL Dialog::OnMove(WPARAM wParam, LPARAM lParam)
{
	// ダイアログの位置の記憶
	if (!bInited) {
		return TRUE;
	}
	RECT rc;
	::GetWindowRect(hWnd, &rc);

	// ダイアログのサイズの記憶
	xPos = rc.left;
	yPos = rc.top;
	nWidth = rc.right - rc.left;
	nHeight = rc.bottom - rc.top;
	DEBUG_TRACE(_T("Dialog::OnMove() xPos=%d yPos=%d\n"), xPos, yPos);
	return TRUE;

}

void Dialog::CreateSizeBox(void)
{
	// サイズボックス
	hwndSizeBox = ::CreateWindowEx(
		WS_EX_CONTROLPARENT,								// no extended styles
		_T("SCROLLBAR"),									// scroll bar control class
		NULL,												// text for window title bar
		WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, // scroll bar styles
		0,													// horizontal position
		0,													// vertical position
		0,													// width of the scroll bar
		0,													// default height
		hWnd/*hdlg*/, 									// handle of main window
		(HMENU) NULL,										// no menu for a scroll bar
		SelectLang::getLangRsrcInstance(),					// instance owning this window
		(LPVOID) NULL										// pointer not needed
	);
	::ShowWindow(hwndSizeBox, SW_SHOW);

}


// ダイアログのメッセージ処理
INT_PTR Dialog::DispatchEvent(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
//	DEBUG_TRACE(_T("Dialog::DispatchEvent() uMsg == %xh\n"), uMsg);
	switch (uMsg) {
	case WM_INITDIALOG:	return OnInitDialog(hwndDlg, wParam, lParam);
	case WM_DESTROY:	return OnDestroy();
	case WM_COMMAND:	return OnCommand(wParam, lParam);
	case WM_NOTIFY:		return OnNotify(wParam, lParam);
	case WM_SIZE:
		hWnd = hwndDlg;
		return OnSize(wParam, lParam);
	case WM_MOVE:
		hWnd = hwndDlg;
		return OnMove(wParam, lParam);
	case WM_DRAWITEM:	return OnDrawItem(wParam, lParam);
	case WM_TIMER:		return OnTimer(wParam);
	case WM_KEYDOWN:	return OnKeyDown(wParam, lParam);
	case WM_KILLFOCUS:	return OnKillFocus(wParam, lParam);
	case WM_ACTIVATE:	return OnActivate(wParam, lParam);	//@@@ 2003.04.08 MIK
	case WM_VKEYTOITEM:	return OnVKeyToItem(wParam, lParam);
	case WM_CHARTOITEM:	return OnCharToItem(wParam, lParam);
	case WM_HELP:		return OnPopupHelp(wParam, lParam);	//@@@ 2002.01.18 add
	case WM_CONTEXTMENU:return OnContextMenu(wParam, lParam);	//@@@ 2002.01.18 add
	}
	return FALSE;
}

BOOL Dialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	WORD	wNotifyCode;
	WORD	wID;
	HWND	hwndCtl;
	wNotifyCode = HIWORD(wParam);	// 通知コード
	wID			= LOWORD(wParam);	// 項目ID､ コントロールID､ またはアクセラレータID
	hwndCtl		= (HWND) lParam;	// コントロールのハンドル
	TCHAR	szClass[32];

	// IDOK と IDCANCEL はボタンからでなくても同じ扱い
	// MSDN [Windows Management] "Dialog Box Programming Considerations"
	if (wID == IDOK || wID == IDCANCEL) {
		return OnBnClicked(wID);
	}
	
	// 通知元がコントロールだった場合の処理
	if (hwndCtl) {
		::GetClassName(hwndCtl, szClass, _countof(szClass));
		if (::lstrcmpi(szClass, _T("Button")) == 0) {
			switch (wNotifyCode) {
			// ボタン／チェックボックスがクリックされた
			case BN_CLICKED:	return OnBnClicked(wID);
			}
		}else if (::lstrcmpi(szClass, _T("Static")) == 0) {
			switch (wNotifyCode) {
			case STN_CLICKED:	return OnStnClicked(wID);
			}
		}else if (::lstrcmpi(szClass, _T("Edit")) == 0) {
			switch (wNotifyCode) {
			case EN_CHANGE:		return OnEnChange(hwndCtl, wID);
			case EN_KILLFOCUS:	return OnEnKillFocus(hwndCtl, wID);
			}
		}else if (::lstrcmpi(szClass, _T("ListBox")) == 0) {
			switch (wNotifyCode) {
			case LBN_SELCHANGE:	return OnLbnSelChange(hwndCtl, wID);
			case LBN_DBLCLK:	return OnLbnDblclk(wID);
			}
		}else if (::lstrcmpi(szClass, _T("ComboBox")) == 0) {
			switch (wNotifyCode) {
			// コンボボックス用メッセージ
			case CBN_SELCHANGE:	return OnCbnSelChange(hwndCtl, wID);
			// @@2005.03.31 MIK タグジャンプDialogで使うので追加
			case CBN_EDITCHANGE:	return OnCbnEditChange(hwndCtl, wID);
			case CBN_DROPDOWN:	return OnCbnDropDown(hwndCtl, wID);
		//	case CBN_CLOSEUP:	return OnCbnCloseUp(hwndCtl, wID);
			case CBN_SELENDOK:	return OnCbnSelEndOk(hwndCtl, wID);
			}
		}
	}

	return FALSE;
}

//@@@ 2002.01.18 add start
BOOL Dialog::OnPopupHelp(WPARAM wPara, LPARAM lParam)
{
	HELPINFO* p = (HELPINFO*) lParam;
	MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)GetHelpIdTable());	// 2006.10.10 ryoji MyWinHelpに変更に変更
	return TRUE;
}

BOOL Dialog::OnContextMenu(WPARAM wPara, LPARAM lParam)
{
	MyWinHelp(hWnd, HELP_CONTEXTMENU, (ULONG_PTR)GetHelpIdTable());	// 2006.10.10 ryoji MyWinHelpに変更に変更
	return TRUE;
}

const DWORD p_helpids[] = {
	0, 0
};

LPVOID Dialog::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

BOOL Dialog::OnCbnSelEndOk(HWND hwndCtl, int wID)
{
	// コンボボックスのリストを表示したまま文字列を編集し、Enterキーを
	// 押すと文字列が消える現象の対策。
	// Enterキーを押してこの関数に入ったら、リストを非表示にしてしまう。

	// リストを非表示にすると前方一致する文字列を選んでしまうので、
	// 事前に文字列を退避し、リスト非表示後に復元する。

	// 文字列を退避
	int nLength = ::GetWindowTextLength(hwndCtl);
	std::vector<TCHAR> buf(nLength + 1);
	LPTSTR sBuf = &buf[0];
	::GetWindowText(hwndCtl, sBuf, nLength + 1);
	sBuf[nLength] = _T('\0');

	// リストを非表示にする
	Combo_ShowDropdown(hwndCtl, FALSE);

	// 文字列を復元・全選択
	::SetWindowText(hwndCtl, sBuf);
	Combo_SetEditSel(hwndCtl, 0, -1);

	return TRUE;
}

BOOL Dialog::OnCbnDropDown(HWND hwndCtl, int wID)
{
	return OnCbnDropDown( hwndCtl, false );
}

/** コンボボックスのドロップダウン時処理

	コンボボックスがドロップダウンされる時に
	ドロップダウンリストの幅をアイテム文字列の最大表示幅に合わせる

	@param hwndCtl [in]		コンボボックスのウィンドウハンドル
	@param wID [in]			コンボボックスのID

	@author ryoji
	@date 2009.03.29 新規作成
*/
BOOL Dialog::OnCbnDropDown(HWND hwndCtl, bool scrollBar)
{
	SIZE sizeText;
	const int nMargin = 8;
	int nScrollWidth = scrollBar ? ::GetSystemMetrics( SM_CXVSCROLL ) + 2 : 2;

	HDC hDC = ::GetDC(hwndCtl);
	if (!hDC)
		return FALSE;
	HFONT hFont = (HFONT)::SendMessage(hwndCtl, WM_GETFONT, 0, (LPARAM)NULL);
	hFont = (HFONT)::SelectObject(hDC, hFont);
	int nItem = Combo_GetCount(hwndCtl);
	RECT rc;
	::GetWindowRect(hwndCtl, &rc);
	LONG nWidth = rc.right - rc.left - nMargin + nScrollWidth;
	for (int iItem=0; iItem<nItem; ++iItem) {
		int nTextLen = Combo_GetLBTextLen(hwndCtl, iItem);
		if (0 < nTextLen) {
			std::vector<TCHAR> szText(nTextLen + 1);
			TCHAR* pszText = &szText[0];
			Combo_GetLBText(hwndCtl, iItem, pszText);
			if (::GetTextExtentPoint32(hDC, pszText, nTextLen, &sizeText)) {
				nWidth = std::max(nWidth, sizeText.cx + nScrollWidth);
			}
		}
	}
	Combo_SetDroppedWidth(hwndCtl, nWidth + nMargin);
	::SelectObject(hDC, hFont);
	::ReleaseDC(hwndCtl, hDC);
	return TRUE;
}

/*! ファイル選択
	@note 実行ファイルのパスor設定ファイルのパスが含まれる場合は相対パスに変換
*/
BOOL Dialog::SelectFile(
	HWND parent,
	HWND hwndCtl,
	const TCHAR* filter,
	bool resolvePath
	)
{
	DlgOpenFile dlgOpenFile;
	TCHAR szFilePath[_MAX_PATH + 1];
	TCHAR szPath[_MAX_PATH + 1];
	::GetWindowText(hwndCtl, szFilePath, _countof(szFilePath));
	// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	if (resolvePath && _IS_REL_PATH(szFilePath)) {
		GetInidirOrExedir(szPath, szFilePath);
	}else {
		auto_strcpy(szPath, szFilePath);
	}
	// ファイルオープンダイアログの初期化
	dlgOpenFile.Create(
		::GetModuleHandle(NULL),
		parent,
		filter,
		szPath
	);
	if (dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
		const TCHAR* fileName;
		if (resolvePath) {
			fileName = GetRelPath(szPath);
		}else {
			fileName = szPath;
		}
		::SetWindowText(hwndCtl, fileName);
		return TRUE;
	}
	return FALSE;
}


// static
bool Dialog::DirectoryUp(TCHAR* szDir)
{
	size_t nLen = auto_strlen(szDir);
	if (3 < nLen) {
		// X:\ や\\. より長い
		CutLastYenFromDirectoryPath(szDir);
		const TCHAR* p = GetFileTitlePointer(szDir);
		if (0 < p - szDir) {
			if (3 < p - szDir) {
				szDir[p - szDir - 1] = '\0'; // \を削るので-1
			}else {
				// 「C:\」の\を残す
				szDir[p - szDir] = '\0';
			}
		}
		return true;
	}
	return false;
}

// コントロールに画面のフォントを設定	2012/11/27 Uchi
HFONT Dialog::SetMainFont(HWND hTarget)
{
	if (!hTarget) {
		return NULL;
	}

	// 設定するフォントの高さを取得
	HFONT hFont = (HFONT)::SendMessage(hTarget, WM_GETFONT, 0, 0);
	LOGFONT	lf;
	GetObject(hFont, sizeof(lf), &lf);
	LONG nfHeight = lf.lfHeight;

	// LOGFONTの作成
	lf = pShareData->common.view.lf;
	lf.lfHeight			= nfHeight;
	lf.lfWidth			= 0;
	lf.lfEscapement		= 0;
	lf.lfOrientation	= 0;
	lf.lfWeight			= FW_NORMAL;
	lf.lfItalic			= FALSE;
	lf.lfUnderline		= FALSE;
	lf.lfStrikeOut		= FALSE;
	//lf.lfCharSet		= lf.lfCharSet;
	lf.lfOutPrecision	= OUT_TT_ONLY_PRECIS;		// Raster Font を使わないように
	//lf.lfClipPrecision	= lf.lfClipPrecision;
	//lf.lfQuality		= lf.lfQuality;
	//lf.lfPitchAndFamily	= lf.lfPitchAndFamily;
	//_tcsncpy(lf.lfFaceName, lf.lfFaceName, _countof(lf.lfFaceName));	// 画面のフォントに設定	2012/11/27 Uchi

	// フォントを作成
	hFont = ::CreateFontIndirect(&lf);
	if (hFont) {
		// フォントの設定
		::SendMessage(hTarget, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
	}
	return hFont;
}

void Dialog::ResizeItem(
	HWND hTarget,
	const POINT& ptDlgDefault,
	const POINT& ptDlgNew,
	const RECT& rcItemDefault,
	AnchorStyle anchor,
	bool bUpdate
	)
{
	POINT pt;
	int height, width;
	pt.x = rcItemDefault.left;
	pt.y = rcItemDefault.top;
	width = rcItemDefault.right - rcItemDefault.left;
	height = rcItemDefault.bottom - rcItemDefault.top;
	if (((int)anchor & ((int)AnchorStyle::Left | (int)AnchorStyle::Right)) == (int)AnchorStyle::Left) {
		// なし
	}else if (((int)anchor & ((int)AnchorStyle::Left | (int)AnchorStyle::Right)) == (int)AnchorStyle::Right) {
		/*
			[<- rcItemDefault.left ->[ ]     ]
			[<- rcItemDefault.right  [->]     ]
			[<-    ptDlgDefault.x             ->]
			[<-    ptDlgNew.x             [ ]    ->]
			[<-    pt.x                 ->[ ]     ]
		*/
		pt.x = rcItemDefault.left + (ptDlgNew.x - ptDlgDefault.x);
	}else if (((int)anchor & ((int)AnchorStyle::Left | (int)AnchorStyle::Right)) == ((int)AnchorStyle::Left | (int)AnchorStyle::Right)) {
		/*
			[<-    ptDlgNew.x        [ ]         ->]
			[                       [<-width->]    ]
		*/
		width = ptDlgNew.x - rcItemDefault.left - (ptDlgDefault.x - rcItemDefault.right);
	}
	
	if (((int)anchor & ((int)AnchorStyle::Top | (int)AnchorStyle::Bottom)) == (int)AnchorStyle::Top) {
		// なし
	}else if (((int)anchor & ((int)AnchorStyle::Top | (int)AnchorStyle::Bottom)) == (int)AnchorStyle::Bottom) {
		pt.y = rcItemDefault.top + (ptDlgNew.y - ptDlgDefault.y);
	}else if (((int)anchor & ((int)AnchorStyle::Top | (int)AnchorStyle::Bottom)) == ((int)AnchorStyle::Top | (int)AnchorStyle::Bottom)) {
		height = ptDlgNew.y - rcItemDefault.top - (ptDlgDefault.y - rcItemDefault.bottom);
	}
//	::MoveWindow(hTarget, pt.x, pt.y, width, height, FALSE);
	::SetWindowPos(hTarget, NULL, pt.x, pt.y, width, height,
				SWP_NOOWNERZORDER | SWP_NOZORDER);
	if (bUpdate) {
		::InvalidateRect(hTarget, NULL, TRUE);
	}
}

void Dialog::GetItemClientRect(int wID, RECT& rc)
{
	POINT po;
	::GetWindowRect(GetItemHwnd(wID), &rc);
	po.x = rc.left;
	po.y = rc.top;
	::ScreenToClient(GetHwnd(), &po);
	rc.left = po.x;
	rc.top  = po.y;
	po.x = rc.right;
	po.y = rc.bottom;
	::ScreenToClient(GetHwnd(), &po);
	rc.right  = po.x;
	rc.bottom = po.y;
}

static const TCHAR* TSTR_SUBCOMBOBOXDATA = _T("SubComboBoxData");

static
void DeleteItem(HWND hwnd, Recent* pRecent)
{
	int nIndex = Combo_GetCurSel(hwnd);
	if (0 <= nIndex) {
		std::vector<TCHAR> szText;
		szText.resize(Combo_GetLBTextLen(hwnd, nIndex) + 1);
		Combo_GetLBText(hwnd, nIndex, &szText[0]);
		Combo_DeleteString(hwnd, nIndex);
		int nRecentIndex = pRecent->FindItemByText(&szText[0]);
		if (0 <= nRecentIndex) {
			pRecent->DeleteItem(nRecentIndex);
		}
	}
}

LRESULT CALLBACK SubEditProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	ComboBoxItemDeleter* data = (ComboBoxItemDeleter*)::GetProp(hwnd, TSTR_SUBCOMBOBOXDATA);
	switch (uMsg) {
	case WM_KEYDOWN:
	{
		if (wParam == VK_DELETE) {
			HWND hwndCombo = data->hwndCombo;
			BOOL bShow = Combo_GetDroppedState(hwndCombo);
			if (bShow) {
				DeleteItem(hwndCombo, data->pRecent);
				return 0;
			}
		}
		break;
	}
	case WM_DESTROY:
	{
		::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)data->pEditWndProc);
		::RemoveProp(hwnd, TSTR_SUBCOMBOBOXDATA);
		data->pEditWndProc = NULL;
		break;
	}
	default:
		break;
	}
	return CallWindowProc(data->pEditWndProc, hwnd, uMsg, wParam, lParam);
}


LRESULT CALLBACK SubListBoxProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	ComboBoxItemDeleter* data = (ComboBoxItemDeleter*)::GetProp(hwnd, TSTR_SUBCOMBOBOXDATA);
	switch (uMsg) {
	case WM_KEYDOWN:
	{
		if (wParam == VK_DELETE) {
			HWND hwndCombo = data->hwndCombo;
			BOOL bShow = Combo_GetDroppedState(hwndCombo);
			if (bShow) {
				DeleteItem(hwndCombo, data->pRecent);
				return 0;
			}
		}
		break;
	}
	case WM_DESTROY:
	{
		::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)data->pListBoxWndProc);
		::RemoveProp(hwnd, TSTR_SUBCOMBOBOXDATA);
		data->pListBoxWndProc = NULL;
		break;
	}
	default:
		break;
	}
	return CallWindowProc(data->pListBoxWndProc, hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK SubComboBoxProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	ComboBoxItemDeleter* data = (ComboBoxItemDeleter*)::GetProp(hwnd, TSTR_SUBCOMBOBOXDATA);
	switch (uMsg) {
	case WM_CTLCOLOREDIT:
	{
		if (!data->pEditWndProc) {
			HWND hwndCtl = (HWND)lParam;
			data->pEditWndProc = (WNDPROC)::GetWindowLongPtr(hwndCtl, GWLP_WNDPROC);
			::SetProp(hwndCtl, TSTR_SUBCOMBOBOXDATA, data);
			::SetWindowLongPtr(hwndCtl, GWLP_WNDPROC, (LONG_PTR)SubEditProc);
		}
		break;
	}
	case WM_CTLCOLORLISTBOX:
	{
		if (!data->pListBoxWndProc) {
			HWND hwndCtl = (HWND)lParam;
			data->pListBoxWndProc = (WNDPROC)::GetWindowLongPtr(hwndCtl, GWLP_WNDPROC);
			::SetProp(hwndCtl, TSTR_SUBCOMBOBOXDATA, data);
			::SetWindowLongPtr(hwndCtl, GWLP_WNDPROC, (LONG_PTR)SubListBoxProc);
		}
		break;
	}
	case WM_DESTROY:
	{
		::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)data->pComboBoxWndProc);
		::RemoveProp(hwnd, TSTR_SUBCOMBOBOXDATA);
		data->pComboBoxWndProc = NULL;
		break;
	}

	default:
		break;
	}
	return CallWindowProc(data->pComboBoxWndProc, hwnd, uMsg, wParam, lParam);
}

void Dialog::SetComboBoxDeleter(HWND hwndCtl, ComboBoxItemDeleter* data)
{
	if (!data->pRecent) {
		return;
	}
	data->hwndCombo = hwndCtl;
	data->pComboBoxWndProc = (WNDPROC)::GetWindowLongPtr(hwndCtl, GWLP_WNDPROC);
	::SetProp(hwndCtl, TSTR_SUBCOMBOBOXDATA, data);
	::SetWindowLongPtr(hwndCtl, GWLP_WNDPROC, (LONG_PTR)SubComboBoxProc);
}

