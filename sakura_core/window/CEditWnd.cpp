/*!	@file
	@brief 編集ウィンドウ（外枠）管理クラス

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, jepro, ao
	Copyright (C) 2001, MIK, Stonee, Misaka, hor, YAZAKI
	Copyright (C) 2002, YAZAKI, genta, hor, aroka, minfu, 鬼, MIK, ai
	Copyright (C) 2003, genta, MIK, Moca, wmlhq, ryoji, KEITA
	Copyright (C) 2004, genta, Moca, yasu, MIK, novice, Kazika
	Copyright (C) 2005, genta, MIK, Moca, aroka, ryoji
	Copyright (C) 2006, genta, ryoji, aroka, fon, yukihane
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, ryoji, nasukoji, Hidetaka Sakai
	Copyright (C) 2010, ryoji, Moca、Uchi
	Copyright (C) 2011, ryoji
	Copyright (C) 2013, Uchi

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include <ShlObj.h>

#include "window/CEditWnd.h"
#include "_main/CControlTray.h"
#include "_main/CCommandLine.h"	/// 2003/1/26 aroka
#include "_main/CAppMode.h"
#include "_os/CDropTarget.h"
#include "_os/COsVersionInfo.h"
#include "dlg/CDlgAbout.h"
#include "dlg/CDlgPrintSetting.h"
#include "env/CShareData.h"
#include "env/CSakuraEnvironment.h"
#include "print/CPrintPreview.h"	/// 2002/2/3 aroka
#include "charset/CharPointer.h"
#include "charset/CCodeFactory.h"
#include "charset/CCodeBase.h"
#include "CEditApp.h"
#include "recent/CMRUFile.h"
#include "recent/CMRUFolder.h"
#include "util/module.h"
#include "util/os.h"		// WM_MOUSEWHEEL,WM_THEMECHANGED
#include "util/window.h"
#include "util/shell.h"
#include "util/string_ex2.h"
#include "plugin/CJackManager.h"
#include "CGrepAgent.h"
#include "CMarkMgr.h"
#include "doc/layout/CLayout.h"
#include "debug/CRunningTimer.h"
#include "sakura_rc.h"


//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたので
//	定義を削除

#ifndef TBSTYLE_ALTDRAG
	#define TBSTYLE_ALTDRAG	0x0400
#endif
#ifndef TBSTYLE_FLAT
	#define TBSTYLE_FLAT	0x0800
#endif
#ifndef TBSTYLE_LIST
	#define TBSTYLE_LIST	0x1000
#endif



#define		YOHAKU_X		4		// ウィンドウ内の枠と紙の隙間最小値
#define		YOHAKU_Y		4		// ウィンドウ内の枠と紙の隙間最小値
//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたので
//	定義を削除


//	状況によりメニューの表示を変えるコマンドリスト(SetMenuFuncSelで使用)
//		2010/5/19	Uchi
//		2012/10/19	syat	各国語対応のため定数化
struct FuncMenuName {
	EFunctionCode	eFunc;
	int				nNameId[2];		// 選択文字列ID
};

static const FuncMenuName	sFuncMenuName[] = {
	{F_RECKEYMACRO,			{F_RECKEYMACRO_REC,				F_RECKEYMACRO_APPE}},
	{F_SAVEKEYMACRO,		{F_SAVEKEYMACRO_REC,			F_SAVEKEYMACRO_APPE}},
	{F_LOADKEYMACRO,		{F_LOADKEYMACRO_REC,			F_LOADKEYMACRO_APPE}},
	{F_EXECKEYMACRO,		{F_EXECKEYMACRO_REC,			F_EXECKEYMACRO_APPE}},
	{F_SPLIT_V,				{F_SPLIT_V_ON,					F_SPLIT_V_OFF}},
	{F_SPLIT_H,				{F_SPLIT_H_ON,					F_SPLIT_H_OFF}},
	{F_SPLIT_VH,			{F_SPLIT_VH_ON,					F_SPLIT_VH_OFF}},
	{F_TAB_CLOSEOTHER,		{F_TAB_CLOSEOTHER_TAB,			F_TAB_CLOSEOTHER_WINDOW}},
	{F_TOPMOST,				{F_TOPMOST_SET,					F_TOPMOST_REL}},
	{F_BIND_WINDOW,			{F_TAB_GROUPIZE,				F_TAB_GROUPDEL}},
	{F_SHOWTOOLBAR,			{F_SHOWTOOLBAR_ON,				F_SHOWTOOLBAR_OFF}},
	{F_SHOWFUNCKEY,			{F_SHOWFUNCKEY_ON,				F_SHOWFUNCKEY_OFF}},
	{F_SHOWTAB,				{F_SHOWTAB_ON,					F_SHOWTAB_OFF}},
	{F_SHOWSTATUSBAR,		{F_SHOWSTATUSBAR_ON,			F_SHOWSTATUSBAR_OFF}},
	{F_SHOWMINIMAP,			{F_SHOWMINIMAP_ON,				F_SHOWMINIMAP_OFF}},
	{F_TOGGLE_KEY_SEARCH,	{F_TOGGLE_KEY_SEARCH_ON,		F_TOGGLE_KEY_SEARCH_OFF}},
};

static void ShowCodeBox(HWND hWnd, EditDoc* pEditDoc)
{
	// カーソル位置の文字列を取得
	const Layout*	pLayout;
	LogicInt		nLineLen;
	const EditView* pView = &pEditDoc->m_pEditWnd->GetActiveView();
	const Caret* pCaret = &pView->GetCaret();
	const LayoutMgr* pLayoutMgr = &pEditDoc->m_layoutMgr;
	const wchar_t* pLine = pLayoutMgr->GetLineStr(pCaret->GetCaretLayoutPos().GetY2(), &nLineLen, &pLayout);

	// -- -- -- -- キャレット位置の文字情報 -> szCaretChar -- -- -- -- //
	//
	if (pLine) {
		// 指定された桁に対応する行のデータ内の位置を調べる
		LogicInt nIdx = pView->LineColumnToIndex(pLayout, pCaret->GetCaretLayoutPos().GetX2());
		if (nIdx < nLineLen) {
			if (nIdx < nLineLen - (pLayout->GetLayoutEol().GetLen() ? 1 : 0)) {
				// 一時的に表示方法の設定を変更する
				CommonSetting_StatusBar sStatusbar;
				sStatusbar.m_bDispUniInSjis		= false;
				sStatusbar.m_bDispUniInJis		= false;
				sStatusbar.m_bDispUniInEuc		= false;
				sStatusbar.m_bDispUtf8Codepoint	= false;
				sStatusbar.m_bDispSPCodepoint	= false;

				TCHAR szMsg[128];
				TCHAR szCode[CODE_CODEMAX][32];
				wchar_t szChar[3];
				int nCharChars = NativeW::GetSizeOfChar(pLine, nLineLen, nIdx);
				memcpy(szChar, &pLine[nIdx], nCharChars * sizeof(wchar_t));
				szChar[nCharChars] = L'\0';
				for (int i=0; i<CODE_CODEMAX; ++i) {
					if (i == CODE_SJIS || i == CODE_JIS || i == CODE_EUC || i == CODE_LATIN1 || i == CODE_UNICODE || i == CODE_UTF8 || i == CODE_CESU8) {
						//auto_sprintf(szCaretChar, _T("%04x"),);
						// 任意の文字コードからUnicodeへ変換する		2008/6/9 Uchi
						CodeBase* pCode = CodeFactory::CreateCodeBase((ECodeType)i, false);
						CodeConvertResult ret = pCode->UnicodeToHex(&pLine[nIdx], nLineLen - nIdx, szCode[i], &sStatusbar);
						delete pCode;
						if (ret != CodeConvertResult::Complete) {
							// うまくコードが取れなかった
							auto_strcpy(szCode[i], _T("-"));
						}
					}
				}
				// コードポイント部（サロゲートペアも）
				TCHAR szCodeCP[32];
				sStatusbar.m_bDispSPCodepoint = true;
				CodeBase* pCode = CodeFactory::CreateCodeBase(CODE_UNICODE, false);
				CodeConvertResult ret = pCode->UnicodeToHex(&pLine[nIdx], nLineLen - nIdx, szCodeCP, &sStatusbar);
				delete pCode;
				if (ret != CodeConvertResult::Complete) {
					// うまくコードが取れなかった
					auto_strcpy(szCodeCP, _T("-"));
				}

				// メッセージボックス表示
				auto_sprintf_s(szMsg, LS(STR_ERR_DLGEDITWND13),
					szChar, szCodeCP, szCode[CODE_SJIS], szCode[CODE_JIS], szCode[CODE_EUC], szCode[CODE_LATIN1], szCode[CODE_UNICODE], szCode[CODE_UTF8], szCode[CODE_CESU8]);
				::MessageBox(hWnd, szMsg, GSTR_APPNAME, MB_OK);
			}
		}
	}
}

//	// メッセージループ
//	DWORD MessageLoop_Thread(DWORD pCEditWndObject);

LRESULT CALLBACK CEditWndProc(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
	EditWnd* pWnd = (EditWnd*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (pWnd) {
		return pWnd->DispatchEvent(hwnd, uMsg, wParam, lParam);
	}
	return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//	@date 2002.2.17 YAZAKI ShareDataのインスタンスは、Processにひとつあるのみ。
EditWnd::EditWnd()
	:
	m_hWnd(NULL)
	, m_toolbar(this)			// warning C4355: 'this' : ベース メンバー初期化子リストで使用されました。
	, m_statusBar(this)		// warning C4355: 'this' : ベース メンバー初期化子リストで使用されました。
	, m_pPrintPreview(NULL) //@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる変更
	, m_pDragSourceView(NULL)
	, m_nActivePaneIndex(0)
	, m_nEditViewCount(1)
	, m_nEditViewMaxCount(_countof(m_pEditViewArr))	// 今のところ最大値は固定
	, m_uMSIMEReconvertMsg(::RegisterWindowMessage(RWM_RECONVERT)) // 20020331 aroka 再変換対応 for 95/NT
	, m_uATOKReconvertMsg(::RegisterWindowMessage(MSGNAME_ATOK_RECONVERT))
	, m_bIsActiveApp(false)
	, m_pszLastCaption(NULL)
	, m_pszMenubarMessage(new TCHAR[MENUBAR_MESSAGE_MAX_LEN])
	, m_posSaveAry(NULL)
	, m_nCurrentFocus(0)
	, m_hAccelWine(NULL)
	, m_hAccel(NULL)
	, m_bDragMode(false)
	, m_iconClicked(IconClickStatus::None) // by 鬼(2)
	, m_nSelectCountMode(SelectCountMode::Toggle)	// 文字カウント方法の初期値はSELECT_COUNT_TOGGLE→共通設定に従う
{
	g_pcEditWnd = this;
}

EditWnd::~EditWnd()
{
	g_pcEditWnd = NULL;

	delete m_pPrintPreview;
	m_pPrintPreview = NULL;

	for (int i=0; i<m_nEditViewMaxCount; ++i) {
		delete m_pEditViewArr[i];
		m_pEditViewArr[i] = NULL;
	}
	m_pEditView = NULL;

	delete m_pEditViewMiniMap;
	m_pEditViewMiniMap = NULL;

	delete m_pViewFont;
	m_pViewFont = NULL;

	delete m_pViewFontMiniMap;
	m_pViewFontMiniMap = NULL;

	delete[] m_pszMenubarMessage;
	delete[] m_pszLastCaption;

	//	Dec. 4, 2002 genta
	// キャレットの行桁位置表示用フォント
	::DeleteObject(m_hFontCaretPosInfo);

	delete m_pDropTarget;	// 2008.06.20 ryoji
	m_pDropTarget = NULL;

	// ウィンドウ毎に作成したアクセラレータテーブルを破棄する(Wine用)
	DeleteAccelTbl();

	m_hWnd = NULL;
}


// ドキュメントリスナ：セーブ後
// 2008.02.02 kobake
void EditWnd::OnAfterSave(const SaveInfo& saveInfo)
{
	// ビュー再描画
	this->Views_RedrawAll();

	// キャプションの更新を行う
	UpdateCaption();

	// キャレットの行桁位置を表示する
	GetActiveView().GetCaret().ShowCaretPosInfo();
}

void EditWnd::UpdateCaption()
{
	if (!GetActiveView().GetDrawSwitch()) {
		return;
	}

	// キャプション文字列の生成 -> pszCap
	wchar_t	pszCap[1024];
	const CommonSetting_Window& setting = GetDllShareData().m_common.m_window;
	const wchar_t* pszFormat = NULL;
	if (!this->IsActiveApp())	pszFormat = to_wchar(setting.m_szWindowCaptionInactive);
	else						pszFormat = to_wchar(setting.m_szWindowCaptionActive);
	SakuraEnvironment::ExpandParameter(
		pszFormat,
		pszCap,
		_countof(pszCap)
	);

	// キャプション更新
	::SetWindowText(this->GetHwnd(), to_tchar(pszCap));

	//@@@ From Here 2003.06.13 MIK
	// タブウィンドウのファイル名を通知
	SakuraEnvironment::ExpandParameter(GetDllShareData().m_common.m_tabBar.m_szTabWndCaption, pszCap, _countof(pszCap));
	this->ChangeFileNameNotify(to_tchar(pszCap), GetListeningDoc()->m_docFile.GetFilePath(), EditApp::getInstance()->m_pGrepAgent->m_bGrepMode);	// 2006.01.28 ryoji ファイル名、Grepモードパラメータを追加
	//@@@ To Here 2003.06.13 MIK
}



// ウィンドウ生成用の矩形を取得
void EditWnd::_GetWindowRectForInit(Rect* rcResult, int nGroup, const TabGroupInfo& tabGroupInfo)
{
	// ウィンドウサイズ継承
	int	nWinCX, nWinCY;
	//	2004.05.13 Moca m_common.m_eSaveWindowSizeをBOOLからenumに変えたため
	auto& csWindow = m_pShareData->m_common.m_window;
	if (csWindow.m_eSaveWindowSize != WinSizeMode::Default) {
		nWinCX = csWindow.m_nWinSizeCX;
		nWinCY = csWindow.m_nWinSizeCY;
	}else {
		nWinCX = CW_USEDEFAULT;
		nWinCY = 0;
	}

	// ウィンドウサイズ指定
	EditInfo fi;
	CommandLine::getInstance()->GetEditInfo(&fi);
	if (fi.m_nWindowSizeX >= 0) {
		nWinCX = fi.m_nWindowSizeX;
	}
	if (fi.m_nWindowSizeY >= 0) {
		nWinCY = fi.m_nWindowSizeY;
	}

	// ウィンドウ位置指定
	int nWinOX = CW_USEDEFAULT;
	int nWinOY = 0;
	// ウィンドウ位置固定
	//	2004.05.13 Moca 保存したウィンドウ位置を使う場合は共有メモリからセット
	if (csWindow.m_eSaveWindowPos != WinSizeMode::Default) {
		nWinOX =  csWindow.m_nWinPosX;
		nWinOY =  csWindow.m_nWinPosY;
	}

	//	2004.05.13 Moca マルチディスプレイでは負の値も有効なので，
	//	未設定の判定方法を変更．(負の値→CW_USEDEFAULT)
	if (fi.m_nWindowOriginX != CW_USEDEFAULT) {
		nWinOX = fi.m_nWindowOriginX;
	}
	if (fi.m_nWindowOriginY != CW_USEDEFAULT) {
		nWinOY = fi.m_nWindowOriginY;
	}

	// 必要なら、タブグループにフィットするよう、変更
	if (tabGroupInfo.IsValid()) {
		RECT rcWork, rcMon;
		GetMonitorWorkRect(tabGroupInfo.hwndTop, &rcWork, &rcMon);

		const WINDOWPLACEMENT& wpTop = tabGroupInfo.wpTop;
		nWinCX = wpTop.rcNormalPosition.right  - wpTop.rcNormalPosition.left;
		nWinCY = wpTop.rcNormalPosition.bottom - wpTop.rcNormalPosition.top;
		nWinOX = wpTop.rcNormalPosition.left   + (rcWork.left - rcMon.left);
		nWinOY = wpTop.rcNormalPosition.top    + (rcWork.top - rcMon.top);
	}

	// 結果
	rcResult->SetXYWH(nWinOX, nWinOY, nWinCX, nWinCY);
}

HWND EditWnd::_CreateMainWindow(int nGroup, const TabGroupInfo& tabGroupInfo)
{
	// -- -- -- -- ウィンドウクラス登録 -- -- -- -- //
	WNDCLASSEX	wc;
	//	Apr. 27, 2000 genta
	//	サイズ変更時のちらつきを抑えるためCS_HREDRAW | CS_VREDRAW を外した
	wc.style			= CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc		= CEditWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 32;
	wc.hInstance		= G_AppInstance();
	//	Dec, 2, 2002 genta アイコン読み込み方法変更
	wc.hIcon			= GetAppIcon(G_AppInstance(), ICON_DEFAULT_APP, FN_APP_ICON, false);

	wc.hCursor			= NULL/*LoadCursor(NULL, IDC_ARROW)*/;
	wc.hbrBackground	= (HBRUSH)NULL/*(COLOR_3DSHADOW + 1)*/;
	wc.lpszMenuName		= NULL;	// MAKEINTRESOURCE(IDR_MENU1);	2010/5/16 Uchi
	wc.lpszClassName	= GSTR_EDITWINDOWNAME;

	//	Dec. 6, 2002 genta
	//	small icon指定のため RegisterClassExに変更
	wc.cbSize			= sizeof(wc);
	wc.hIconSm			= GetAppIcon(G_AppInstance(), ICON_DEFAULT_APP, FN_APP_ICON, true);
	ATOM atom = RegisterClassEx(&wc);
	if (atom == 0) {
		//	2004.05.13 Moca return NULLを有効にした
		return NULL;
	}

	// 矩形取得
	Rect rc;
	_GetWindowRectForInit(&rc, nGroup, tabGroupInfo);

	// 作成
	HWND hwndResult = ::CreateWindowEx(
		0,				 	// extended window style
		GSTR_EDITWINDOWNAME,		// pointer to registered class name
		GSTR_EDITWINDOWNAME,		// pointer to window name
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,	// window style
		rc.left,			// horizontal position of window
		rc.top,				// vertical position of window
		rc.Width(),			// window width
		rc.Height(),		// window height
		NULL,				// handle to parent or owner window
		NULL,				// handle to menu or child-window identifier
		G_AppInstance(),		// handle to application instance
		NULL				// pointer to window-creation data
	);
	return hwndResult;
}

void EditWnd::_GetTabGroupInfo(TabGroupInfo* pTabGroupInfo, int& nGroup)
{
	HWND hwndTop = NULL;
	WINDOWPLACEMENT	wpTop = {0};

	// From Here @@@ 2003.05.31 MIK
	// タブウィンドウの場合は現状値を指定
	if (m_pShareData->m_common.m_tabBar.m_bDispTabWnd && !m_pShareData->m_common.m_tabBar.m_bDispTabWndMultiWin) {
		if (nGroup < 0)	// 不正なグループID
			nGroup = 0;	// グループ指定無し（最近アクティブのグループに入れる）
		EditNode* pEditNode = AppNodeGroupHandle(nGroup).GetEditNodeAt(0);	// グループの先頭ウィンドウ情報を取得	// 2007.06.20 ryoji
		hwndTop = pEditNode? pEditNode->GetHwnd(): NULL;

		if (hwndTop) {
			//	Sep. 11, 2003 MIK 新規TABウィンドウの位置が上にずれないように
			// 2007.06.20 ryoji 非プライマリモニタまたはタスクバーを動かした後でもずれないように

			wpTop.length = sizeof(wpTop);
			if (::GetWindowPlacement(hwndTop, &wpTop)) {	// 現在の先頭ウィンドウから位置を取得
				if (wpTop.showCmd == SW_SHOWMINIMIZED)
					wpTop.showCmd = pEditNode->m_showCmdRestore;
			}else {
				hwndTop = NULL;
			}
		}
	}
	// To Here @@@ 2003.05.31 MIK

	// 結果
	pTabGroupInfo->hwndTop = hwndTop;
	pTabGroupInfo->wpTop = wpTop;
}

void EditWnd::_AdjustInMonitor(const TabGroupInfo& tabGroupInfo)
{
	RECT	rcOrg;
	RECT	rcDesktop;
//	int		nWork;

	//	May 01, 2004 genta マルチモニタ対応
	::GetMonitorWorkRect(GetHwnd(), &rcDesktop);
	::GetWindowRect(GetHwnd(), &rcOrg);

	// 2005.11.23 Moca マルチモニタ等で問題があったため計算方法変更
	// ウィンドウ位置調整
	if (rcOrg.bottom > rcDesktop.bottom) {
		rcOrg.top -= rcOrg.bottom - rcDesktop.bottom;
		rcOrg.bottom = rcDesktop.bottom;	//@@@ 2002.01.08
	}
	if (rcOrg.right > rcDesktop.right) {
		rcOrg.left -= rcOrg.right - rcDesktop.right;
		rcOrg.right = rcDesktop.right;	//@@@ 2002.01.08
	}
	
	if (rcOrg.top < rcDesktop.top) {
		rcOrg.bottom += rcDesktop.top - rcOrg.top;
		rcOrg.top = rcDesktop.top;
	}
	if (rcOrg.left < rcDesktop.left) {
		rcOrg.right += rcDesktop.left - rcOrg.left;
		rcOrg.left = rcDesktop.left;
	}

	// ウィンドウサイズ調整
	if (rcOrg.bottom > rcDesktop.bottom) {
		//rcOrg.bottom = rcDesktop.bottom - 1;	//@@@ 2002.01.08
		rcOrg.bottom = rcDesktop.bottom;	//@@@ 2002.01.08
	}
	if (rcOrg.right > rcDesktop.right) {
		//rcOrg.right = rcDesktop.right - 1;	//@@@ 2002.01.08
		rcOrg.right = rcDesktop.right;	//@@@ 2002.01.08
	}

	// From Here @@@ 2003.06.13 MIK
	if (m_pShareData->m_common.m_tabBar.m_bDispTabWnd
		&& !m_pShareData->m_common.m_tabBar.m_bDispTabWndMultiWin
		&& tabGroupInfo.hwndTop
	) {
		// 現在の先頭ウィンドウから WS_EX_TOPMOST 状態を引き継ぐ	// 2007.05.18 ryoji
		DWORD dwExStyle = (DWORD)::GetWindowLongPtr(tabGroupInfo.hwndTop, GWL_EXSTYLE);
		::SetWindowPos(GetHwnd(), (dwExStyle & WS_EX_TOPMOST)? HWND_TOPMOST: HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

		// タブウィンドウ時は現状を維持
		// ウィンドウサイズ継承
		// Vista 以降の初回表示アニメーション効果を抑止する
		if (!IsWinVista_or_later()) {
			if (tabGroupInfo.wpTop.showCmd == SW_SHOWMAXIMIZED) {
				::ShowWindow(GetHwnd(), SW_SHOWMAXIMIZED);
			}else {
				::ShowWindow(GetHwnd(), SW_SHOW);
			}
		}else {
			// 初回表示のアニメーション効果を抑止する

			// 先頭ウィンドウの背後で画面描画してから手前に出す（ツールバーやビューのちらつきを抑える）
			// ここでは、あとで正式に適用されるはずのドキュメントタイプを仮設定して一時描画しておく（ビューの配色切替によるちらつきを抑える）
			// さらに、タイプを戻して画面を無効化だけしておく（何らかの原因で途中停止した場合にはもとのタイプ色で再描画されるように ← 例えばファイルサイズが大きすぎる警告を出すときなど）
			// ※ 正攻法とはいえないかもしれないがあちこち手を入れることなく簡潔に済ませられるのでこうしておく
			TypeConfigNum typeOld, typeNew(-1);
			typeOld = GetDocument()->m_docType.GetDocumentType();	// 現在のタイプ
			{
				EditInfo ei, mruei;
				CommandLine::getInstance()->GetEditInfo(&ei);
				if (ei.m_szDocType[0] != '\0') {
					typeNew = DocTypeManager().GetDocumentTypeOfExt(ei.m_szDocType);
				}else {
					if (MRUFile().GetEditInfo(ei.m_szPath, &mruei) && 0 < mruei.m_nTypeId) {
						typeNew = DocTypeManager().GetDocumentTypeOfId(mruei.m_nTypeId);
					}
					if (!typeNew.IsValidType()) {
						if (ei.m_szPath[0]) {
							typeNew = DocTypeManager().GetDocumentTypeOfPath(ei.m_szPath);
						}else {
							typeNew = typeOld;
						}
					}
				}
			}
			GetDocument()->m_docType.SetDocumentType(typeNew, true, true);	// 仮設定

			// 可能な限り画面描画の様子が見えないよう一時的に先頭ウィンドウの後ろに配置
			::SetWindowPos(GetHwnd(), tabGroupInfo.hwndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

			// アニメーション効果は一時的に OFF にする
			ANIMATIONINFO ai = {sizeof(ANIMATIONINFO)};
			::SystemParametersInfo(SPI_GETANIMATION, sizeof(ANIMATIONINFO), &ai, 0);
			int iMinAnimateOld = ai.iMinAnimate;
			ai.iMinAnimate = 0;
			::SystemParametersInfo(SPI_SETANIMATION, sizeof(ANIMATIONINFO), &ai, 0);

			// 可視化する（最大化のときは次の ::ShowWindow() で手前に出てしまうので、アニメーション除去効果はあるがクライアント領域のちらつきは抑えきれない）
			int nCmdShow = (tabGroupInfo.wpTop.showCmd == SW_SHOWMAXIMIZED)? SW_SHOWMAXIMIZED: SW_SHOWNOACTIVATE;
			::ShowWindow(GetHwnd(), nCmdShow);
			::UpdateWindow(GetHwnd());	// 画面更新
			::BringWindowToTop(GetHwnd());
			::ShowWindow(tabGroupInfo.hwndTop , SW_HIDE);	// 以前の先頭ウィンドウはここで消しておかないと消えるアニメーションが見える場合がある

			// アニメーション効果を戻す
			ai.iMinAnimate = iMinAnimateOld;
			::SystemParametersInfo(SPI_SETANIMATION, sizeof(ANIMATIONINFO), &ai, 0);

			// アイドリング開始時にその時点のタイプ別設定色で再描画されるようにしておく
			GetDocument()->m_docType.SetDocumentType(typeOld, true, true);	// タイプ戻し
			::InvalidateRect(GetHwnd(), NULL, TRUE);	// 画面無効化
		}
	}else {
		::SetWindowPos(
			GetHwnd(), 0,
			rcOrg.left, rcOrg.top,
			rcOrg.right - rcOrg.left, rcOrg.bottom - rcOrg.top,
			SWP_NOOWNERZORDER | SWP_NOZORDER
		);

		// ウィンドウサイズ継承
		auto& csWindow = m_pShareData->m_common.m_window;
		if (csWindow.m_eSaveWindowSize != WinSizeMode::Default &&
			csWindow.m_nWinSizeType == SIZE_MAXIMIZED
		) {
			::ShowWindow(GetHwnd(), SW_SHOWMAXIMIZED);
		}else
		// 2004.05.14 Moca ウィンドウサイズを直接指定する場合は、最小化表示を受け入れる
		if (csWindow.m_eSaveWindowSize == WinSizeMode::Set &&
			csWindow.m_nWinSizeType == SIZE_MINIMIZED
		) {
			::ShowWindow(GetHwnd(), SW_SHOWMINIMIZED);
		}else {
			::ShowWindow(GetHwnd(), SW_SHOW);
		}
	}
	// To Here @@@ 2003.06.13 MIK
}

/*!
	作成

	@date 2002.03.07 genta nDocumentType追加
	@date 2007.06.26 ryoji nGroup追加
	@date 2008.04.19 ryoji 初回アイドリング検出用ゼロ秒タイマーのセット処理を追加
*/
HWND EditWnd::Create(
	EditDoc*		pEditDoc,
	ImageListMgr*	pIcons,	// [in] Image List
	int				nGroup		// [in] グループID
	)
{
	MY_RUNNINGTIMER(cRunningTimer, "EditWnd::Create");

	// 共有データ構造体のアドレスを返す
	m_pShareData = &GetDllShareData();

	m_pEditDoc = pEditDoc;

	for (int i=0; i<_countof(m_pEditViewArr); ++i) {
		m_pEditViewArr[i] = NULL;
	}
	// [0] - [3] まで作成・初期化していたものを[0]だけ作る。ほかは分割されるまで何もしない
	m_pEditViewArr[0] = new EditView(this);
	m_pEditView = m_pEditViewArr[0];

	m_pViewFont = new ViewFont(&GetLogfont());

	m_pEditViewMiniMap = new EditView(this);

	m_pViewFontMiniMap = new ViewFont(&GetLogfont(), true);

	auto_memset(m_pszMenubarMessage, _T(' '), MENUBAR_MESSAGE_MAX_LEN);	// null終端は不要

	//	Dec. 4, 2002 genta
	InitMenubarMessageFont();

	m_pDropTarget = new DropTarget(this);	// 右ボタンドロップ用	// 2008.06.20 ryoji

	// 2009.01.17 nasukoji	ホイールスクロール有無状態をクリア
	ClearMouseState();

	// ウィンドウ毎にアクセラレータテーブルを作成する(Wine用)
	CreateAccelTbl();

	// ウィンドウ数制限
	if (m_pShareData->m_nodes.m_nEditArrNum >= MAX_EDITWINDOWS) {	// 最大値修正	//@@@ 2003.05.31 MIK
		OkMessage(NULL, LS(STR_MAXWINDOW), MAX_EDITWINDOWS);
		return NULL;
	}

	// タブグループ情報取得
	TabGroupInfo tabGroupInfo;
	_GetTabGroupInfo(&tabGroupInfo, nGroup);


	// -- -- -- -- ウィンドウ作成 -- -- -- -- //
	HWND hWnd = _CreateMainWindow(nGroup, tabGroupInfo);
	if (!hWnd) {
		return NULL;
	}
	m_hWnd = hWnd;

	// 初回アイドリング検出用のゼロ秒タイマーをセットする	// 2008.04.19 ryoji
	// ゼロ秒タイマーが発動（初回アイドリング検出）したら MYWM_FIRST_IDLE を起動元プロセスにポストする。
	// ※起動元での起動先アイドリング検出については ControlTray::OpenNewEditor を参照
	::SetTimer(GetHwnd(), IDT_FIRST_IDLE, 0, NULL);

	// 編集ウィンドウリストへの登録
	// 2011.01.12 ryoji この処理は以前はウィンドウ可視化よりも後の位置にあった
	// Vista/7 での初回表示アニメーション抑止（rev1868）とのからみで、ウィンドウが可視化される時点でタブバーに全タブが揃っていないと見苦しいのでここに移動。
	// AddEditWndList() で自ウィンドウにポストされる MYWM_TAB_WINDOW_NOTIFY(TWNT_ADD) はタブバー作成後の初回アイドリング時に処理されるので特に問題は無いはず。
	if (!AppNodeGroupHandle(nGroup).AddEditWndList(GetHwnd())) {	// 2007.06.26 ryoji nGroup引数追加
		OkMessage(GetHwnd(), LS(STR_MAXWINDOW), MAX_EDITWINDOWS);
		::DestroyWindow(GetHwnd());
		m_hWnd = hWnd = NULL;
		return hWnd;
	}

	// コモンコントロール初期化
	MyInitCommonControls();

	// イメージ、ヘルパなどの作成
	m_menuDrawer.Create(G_AppInstance(), GetHwnd(), pIcons);
	m_toolbar.Create(pIcons);

	// プラグインコマンドを登録する
	RegisterPluginCommand();

	SelectCharWidthCache(CharWidthFontMode::MiniMap, CharWidthCacheMode::Local); // Init
	InitCharWidthCache(m_pViewFontMiniMap->GetLogfont(), CharWidthFontMode::MiniMap);
	SelectCharWidthCache(CharWidthFontMode::Edit, GetLogfontCacheMode());
	InitCharWidthCache(GetLogfont());


	// -- -- -- -- 子ウィンドウ作成 -- -- -- -- //

	// 分割フレーム作成
	m_splitterWnd.Create(G_AppInstance(), GetHwnd(), this);

	// ビュー
	GetView(0).Create(m_splitterWnd.GetHwnd(), GetDocument(), 0, TRUE, false);
	GetView(0).OnSetFocus();

	// 子ウィンドウの設定
	HWND hWndArr[2];
	hWndArr[0] = GetView(0).GetHwnd();
	hWndArr[1] = NULL;
	m_splitterWnd.SetChildWndArr(hWndArr);

	MY_TRACETIME(cRunningTimer, "View created");


	// -- -- -- -- 各種バー作成 -- -- -- -- //

	// メインメニュー
	LayoutMainMenu();

	// ツールバー
	LayoutToolBar();

	// ステータスバー
	LayoutStatusBar();

	// ファンクションキー バー
	LayoutFuncKey();

	// タブウィンドウ
	LayoutTabBar();

	// ミニマップ
	LayoutMiniMap();

	// バーの配置終了
	EndLayoutBars(FALSE);

	// -- -- -- -- その他調整など -- -- -- -- //

	// 画面表示直前にDispatchEventを有効化する
	::SetWindowLongPtr(GetHwnd(), GWLP_USERDATA, (LONG_PTR)this);

	// デスクトップからはみ出さないようにする
	_AdjustInMonitor(tabGroupInfo);

	// ドロップされたファイルを受け入れる
	::DragAcceptFiles(GetHwnd(), TRUE);
	m_pDropTarget->Register_DropTarget(m_hWnd);	// 右ボタンドロップ用	// 2008.06.20 ryoji

	// アクティブ情報
	m_bIsActiveApp = (::GetActiveWindow() == GetHwnd());	// 2007.03.08 ryoji

	// エディタ－トレイ間でのUI特権分離の確認（Vista UIPI機能） 2007.06.07 ryoji
	if (IsWinVista_or_later()) {
		m_bUIPI = FALSE;
		::SendMessage(m_pShareData->m_handles.m_hwndTray, MYWM_UIPI_CHECK,  (WPARAM)0, (LPARAM)GetHwnd());
		if (!m_bUIPI) {	// 返事が返らない
			TopErrorMessage(GetHwnd(),
				LS(STR_ERR_DLGEDITWND02)
			);
			::DestroyWindow(GetHwnd());
			m_hWnd = hWnd = NULL;
			return hWnd;
		}
	}

	ShareData::getInstance()->SetTraceOutSource(GetHwnd());	// TraceOut()起動元ウィンドウの設定	// 2006.06.26 ryoji

	// Aug. 29, 2003 wmlhq
	m_nTimerCount = 0;
	// タイマーを起動	タイマーのIDと間隔を変更 20060128 aroka
	if (::SetTimer(GetHwnd(), IDT_EDIT, 500, NULL) == 0) {
		WarningMessage(GetHwnd(), LS(STR_ERR_DLGEDITWND03));
	}
	// ツールバーのタイマーを分離した 20060128 aroka
	Timer_ONOFF(true);

	// デフォルトのIMEモード設定
	GetDocument()->m_docEditor.SetImeMode(GetDocument()->m_docType.GetDocumentAttribute().m_nImeState);

	return GetHwnd();
}



// 起動時のファイルオープン処理
void EditWnd::OpenDocumentWhenStart(
	const LoadInfo& argLoadInfo		// [in]
	)
{
	if (argLoadInfo.filePath.Length()) {
		::ShowWindow(GetHwnd(), SW_SHOW);
		// Oct. 03, 2004 genta コード確認は設定に依存
		LoadInfo loadInfo = argLoadInfo;
		bool bReadResult = GetDocument()->m_docFileOperation.FileLoadWithoutAutoMacro(&loadInfo);	// 自動実行マクロは後で別の場所で実行される
		if (!bReadResult) {
			// ファイルが既に開かれている
			if (loadInfo.bOpened) {
				::PostMessage(GetHwnd(), WM_CLOSE, 0, 0);
				// 2004.07.12 Moca return NULLだと、メッセージループを通らずにそのまま破棄されてしまい、タブの終了処理が抜ける
				//	この後は正常ルートでメッセージループに入った後WM_CLOSEを受信して直ちにCLOSE & DESTROYとなる．
				//	その中で編集ウィンドウの削除が行われる．
			}
		}
	}
}

void EditWnd::SetDocumentTypeWhenCreate(
	ECodeType		nCharCode,		// [in] 漢字コード
	bool			bViewMode,		// [in] ビューモードで開くかどうか
	TypeConfigNum	nDocumentType	// [in] 文書タイプ．-1のとき強制指定無し．
	)
{
	//	Mar. 7, 2002 genta 文書タイプの強制指定
	//	Jun. 4 ,2004 genta ファイル名指定が無くてもタイプ強制指定を有効にする
	if (nDocumentType.IsValidType()) {
		GetDocument()->m_docType.SetDocumentType(nDocumentType, true);
		//	2002/05/07 YAZAKI タイプ別設定一覧の一時適用のコードを流用
		GetDocument()->m_docType.LockDocumentType();
	}

	// 文字コードの指定	2008/6/14 Uchi
	if (IsValidCodeType(nCharCode) || nDocumentType.IsValidType()) {
		const TypeConfig& types = GetDocument()->m_docType.GetDocumentAttribute();
		ECodeType eDefaultCharCode = types.m_encoding.m_eDefaultCodetype;
		if (!IsValidCodeType(nCharCode)) {
			nCharCode = eDefaultCharCode;	// 直接コード指定がなければタイプ指定のデフォルト文字コードを使用
		}
		if (nCharCode == eDefaultCharCode) {	// デフォルト文字コードと同じ文字コードが選択されたとき
			GetDocument()->SetDocumentEncoding(nCharCode, types.m_encoding.m_bDefaultBom);
			GetDocument()->m_docEditor.m_newLineCode = static_cast<EolType>(types.m_encoding.m_eDefaultEoltype);
		}else {
			GetDocument()->SetDocumentEncoding(nCharCode, CodeTypeName(nCharCode).IsBomDefOn());
			GetDocument()->m_docEditor.m_newLineCode = EolType::CRLF;
		}
	}

	//	Jun. 4 ,2004 genta ファイル名指定が無くてもビューモード強制指定を有効にする
	AppMode::getInstance()->SetViewMode(bViewMode);

	if (nDocumentType.IsValidType()) {
		// 設定変更を反映させる
		GetDocument()->OnChangeSetting();	// <--- 内部に BlockingHook() 呼び出しがあるので溜まった描画がここで実行される
	}
}


/*! メインメニューの配置処理
	@date 2010/05/16 Uchi
	@date 2012/10/18 syat 各国語対応
*/
void EditWnd::LayoutMainMenu()
{
	TCHAR		szLabel[300];
	TCHAR		szKey[10];
	CommonSetting_MainMenu*	pMenu = &m_pShareData->m_common.m_mainMenu;
	MainMenu*	mainMenu;
	HWND		hWnd = GetHwnd();
	HMENU		hMenu;
	int 		nCount;
	LPCTSTR		pszName;

	hMenu = ::CreateMenu();
	auto& csKeyBind = m_pShareData->m_common.m_keyBind;
	for (int i=0; i<MAX_MAINMENU_TOP && pMenu->m_nMenuTopIdx[i] >= 0; ++i) {
		nCount = (i >= MAX_MAINMENU_TOP || pMenu->m_nMenuTopIdx[i + 1] < 0 ? pMenu->m_nMainMenuNum : pMenu->m_nMenuTopIdx[i+1])
				- pMenu->m_nMenuTopIdx[i];		// メニュー項目数
		mainMenu = &pMenu->m_mainMenuTbl[pMenu->m_nMenuTopIdx[i]];
		switch (mainMenu->m_nType) {
		case MainMenuType::Node:
			// ラベル未設定かつFunctionコードがありならストリングテーブルから取得 2012/10/18 syat 各国語対応
			pszName = (mainMenu->m_sName[0] == L'\0' && mainMenu->nFunc != F_NODE)
								? LS(mainMenu->nFunc) : to_tchar(mainMenu->m_sName);
			::AppendMenu(hMenu, MF_POPUP | MF_STRING | (nCount <= 1 ? MF_GRAYED : 0), (UINT_PTR)CreatePopupMenu(), 
				KeyBind::MakeMenuLabel(pszName, to_tchar(mainMenu->m_sKey)));
			break;
		case MainMenuType::Leaf:
			// メニューラベルの作成
			// 2014.05.04 Moca プラグイン/マクロ等を置けるようにFunccode2Nameを使うように
			{
				WCHAR szLabelW[256];
				GetDocument()->m_funcLookup.Funccode2Name(mainMenu->nFunc, szLabelW, 256);
				auto_strncpy(szLabel, to_tchar(szLabelW), _countof(szLabel) - 1);
				szLabel[_countof(szLabel) - 1] = _T('\0');
			}
			auto_strcpy(szKey, to_tchar(mainMenu->m_sKey));
			if (!KeyBind::GetMenuLabel(
				G_AppInstance(),
				csKeyBind.m_nKeyNameArrNum,
				csKeyBind.m_pKeyNameArr,
				mainMenu->nFunc,
				szLabel,
				to_tchar(mainMenu->m_sKey),
				FALSE,
				_countof(szLabel)
				)
			) {
				auto_strcpy(szLabel, _T("?"));
			}
			::AppendMenu(hMenu, MF_STRING, mainMenu->nFunc, szLabel);
			break;
		case MainMenuType::Separator:
			::AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
			break;
		case MainMenuType::Special:
			nCount = 0;
			switch (mainMenu->nFunc) {
			case F_WINDOW_LIST:				// ウィンドウリスト
				EditNode*	pEditNodeArr;
				nCount = AppNodeManager::getInstance()->GetOpenedWindowArr(&pEditNodeArr, TRUE);
				delete [] pEditNodeArr;
				break;
			case F_FILE_USED_RECENTLY:		// 最近使ったファイル
				{
					RecentFile	cRecentFile;
					nCount = cRecentFile.GetViewCount();
				}
				break;
			case F_FOLDER_USED_RECENTLY:	// 最近使ったフォルダ
				{
					RecentFolder	cRecentFolder;
					nCount = cRecentFolder.GetViewCount();
				}
				break;
			case F_CUSTMENU_LIST:			// カスタムメニューリスト
				//	右クリックメニュー
				if (m_pShareData->m_common.m_customMenu.m_nCustMenuItemNumArr[0] > 0) {
					++nCount;
				}
				//	カスタムメニュー
				for (int j=1; j<MAX_CUSTOM_MENU; ++j) {
					if (m_pShareData->m_common.m_customMenu.m_nCustMenuItemNumArr[j] > 0) {
						++nCount;
					}
				}
				break;
			case F_USERMACRO_LIST:			// 登録済みマクロリスト
				for (int j=0; j<MAX_CUSTMACRO; ++j) {
					MacroRec *mp = &m_pShareData->m_common.m_macro.m_macroTable[j];
					if (mp->IsEnabled()) {
						++nCount;
					}
				}
				break;
			case F_PLUGIN_LIST:				// プラグインコマンドリスト
				// プラグインコマンドを提供するプラグインを列挙する
				{
					const JackManager* pJackManager = JackManager::getInstance();

					Plug::Array plugs = pJackManager->GetPlugs(PP_COMMAND);
					for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
						++nCount;
					}
				}
				break;
			}
			::AppendMenu(hMenu, MF_POPUP | MF_STRING | (nCount <= 0 ? MF_GRAYED : 0), (UINT_PTR)CreatePopupMenu(), 
				KeyBind::MakeMenuLabel(LS(mainMenu->nFunc), to_tchar(mainMenu->m_sKey)));
			break;
		}
	}
	HMENU hMenuOld = ::GetMenu(hWnd);
	SetMenu(hWnd, hMenu);
	if (hMenuOld) {
		DestroyMenu(hMenuOld);
	}

	DrawMenuBar(hWnd);
}

/*! ツールバーの配置処理
	@date 2006.12.19 ryoji 新規作成
*/
void EditWnd::LayoutToolBar(void)
{
	if (m_pShareData->m_common.m_window.m_bDispTOOLBAR) {	// ツールバーを表示する
		m_toolbar.CreateToolBar();
	}else {
		m_toolbar.DestroyToolBar();
	}
}

/*! ステータスバーの配置処理
	@date 2006.12.19 ryoji 新規作成
*/
void EditWnd::LayoutStatusBar(void)
{
	if (m_pShareData->m_common.m_window.m_bDispSTATUSBAR) {	// ステータスバーを表示する
		// ステータスバー作成
		m_statusBar.CreateStatusBar();
	}else {
		// ステータスバー破棄
		m_statusBar.DestroyStatusBar();
	}
}

/*! ファンクションキーの配置処理
	@date 2006.12.19 ryoji 新規作成
*/
void EditWnd::LayoutFuncKey(void)
{
	if (m_pShareData->m_common.m_window.m_bDispFUNCKEYWND) {	// ファンクションキーを表示する
		if (!m_funcKeyWnd.GetHwnd()) {
			bool bSizeBox;
			if (m_pShareData->m_common.m_window.m_nFUNCKEYWND_Place == 0) {	// ファンクションキー表示位置／0:上 1:下
				bSizeBox = false;
			}else {
				bSizeBox = true;
				// ステータスバーがあるときはサイズボックスを表示しない
				if (m_statusBar.GetStatusHwnd()) {
					bSizeBox = false;
				}
			}
			m_funcKeyWnd.Open(G_AppInstance(), GetHwnd(), GetDocument(), bSizeBox);
		}
	}else {
		m_funcKeyWnd.Close();
	}
}

/*! タブバーの配置処理
	@date 2006.12.19 ryoji 新規作成
*/
void EditWnd::LayoutTabBar(void)
{
	if (m_pShareData->m_common.m_tabBar.m_bDispTabWnd) {	// タブバーを表示する
		if (!m_tabWnd.GetHwnd()) {
			m_tabWnd.Open(G_AppInstance(), GetHwnd());
		}else {
			m_tabWnd.UpdateStyle();
		}
	}else {
		m_tabWnd.Close();
		m_tabWnd.SizeBox_ONOFF(false);
	}
}

/*! ミニマップの配置処理
	@date 2014.07.14 新規作成
*/
void EditWnd::LayoutMiniMap( void )
{
	if (m_pShareData->m_common.m_window.m_bDispMiniMap) {	// タブバーを表示する
		if (!GetMiniMap().GetHwnd()) {
			GetMiniMap().Create(GetHwnd(), GetDocument(), -1, TRUE, true);
		}
	}else {
		if (GetMiniMap().GetHwnd()) {
			GetMiniMap().Close();
		}
	}
}

/*! バーの配置終了処理
	@date 2006.12.19 ryoji 新規作成
	@date 2007.03.04 ryoji 印刷プレビュー時はバーを隠す
	@date 2011.01.21 ryoji アウトライン画面にゴミが描画されるのを抑止する
*/
void EditWnd::EndLayoutBars(BOOL bAdjust/* = TRUE*/)
{
	int nCmdShow = m_pPrintPreview? SW_HIDE: SW_SHOW;
	HWND hwndToolBar = m_toolbar.GetRebarHwnd() ? m_toolbar.GetRebarHwnd(): m_toolbar.GetToolbarHwnd();
	if (hwndToolBar)
		::ShowWindow(hwndToolBar, nCmdShow);
	if (m_statusBar.GetStatusHwnd())
		::ShowWindow(m_statusBar.GetStatusHwnd(), nCmdShow);
	if (m_funcKeyWnd.GetHwnd())
		::ShowWindow(m_funcKeyWnd.GetHwnd(), nCmdShow);
	if (m_tabWnd.GetHwnd())
		::ShowWindow(m_tabWnd.GetHwnd(), nCmdShow);
	if (m_dlgFuncList.GetHwnd() && m_dlgFuncList.IsDocking()) {
		::ShowWindow(m_dlgFuncList.GetHwnd(), nCmdShow);
		// アウトラインを最背後にしておく（ゴミ描画の抑止策）
		// この対策以前は、アウトラインを下ドッキングしている状態で、
		// メニューから[ファンクションキーを表示]/[ステータスバーを表示]を実行して非表示のバーをアウトライン直下に表示したり、
		// その後、ウィンドウの下部境界を上下ドラッグしてサイズ変更するとゴミが現れることがあった。
		::SetWindowPos(m_dlgFuncList.GetHwnd(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	}

	if (bAdjust) {
		RECT rc;
		m_splitterWnd.DoSplit(-1, -1);
		::GetClientRect(GetHwnd(), &rc);
		::SendMessage(GetHwnd(), WM_SIZE, m_nWinSizeType, MAKELONG(rc.right - rc.left, rc.bottom - rc.top));
		::RedrawWindow(GetHwnd(), NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);	// ステータスバーに必要？

		GetActiveView().SetIMECompFormPos();
	}
}

static inline BOOL MyIsDialogMessage(HWND hwnd, MSG* msg)
{
	if (!hwnd) {
		return FALSE;
	}
	return ::IsDialogMessage(hwnd, msg);
}

// 複数プロセス版
// メッセージループ
// 2004.02.17 Moca GetMessageのエラーチェック
void EditWnd::MessageLoop(void)
{
	MSG	msg;
	while (GetHwnd()) {
		// メッセージ取得
		int ret = GetMessage(&msg, NULL, 0, 0);
		if (ret == 0) break; // WM_QUIT
		if (ret == -1) break; // GetMessage失敗

		// ダイアログメッセージ
		     if (MyIsDialogMessage(m_pPrintPreview->GetPrintPreviewBarHANDLE_Safe(),	&msg)) {}	// 印刷プレビュー 操作バー
		else if (MyIsDialogMessage(m_dlgFind.GetHwnd(),								&msg)) {}	//「検索」ダイアログ
		else if (MyIsDialogMessage(m_dlgFuncList.GetHwnd(),							&msg)) {}	//「アウトライン」ダイアログ
		else if (MyIsDialogMessage(m_dlgReplace.GetHwnd(),								&msg)) {}	//「置換」ダイアログ
		else if (MyIsDialogMessage(m_dlgGrep.GetHwnd(),								&msg)) {}	//「Grep」ダイアログ
		else if (MyIsDialogMessage(m_hokanMgr.GetHwnd(),								&msg)) {}	//「入力補完」
		else if (m_toolbar.EatMessage(&msg)) { }													// ツールバー
		// アクセラレータ
		else {
			if (m_hAccel && TranslateAccelerator(msg.hwnd, m_hAccel, &msg)) {}
			// 通常メッセージ
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
}


LRESULT EditWnd::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
	int					nRet;
	LPNMHDR				pnmh;
	int					nPane;
	EditInfo*			pfi;
	LPHELPINFO			lphi;
	
	UINT				idCtl;	// コントロールのID
	MEASUREITEMSTRUCT*	lpmis;
	LPDRAWITEMSTRUCT	lpdis;	// 項目描画情報
	int					nItemWidth;
	int					nItemHeight;
	UINT				uItem;
	LRESULT				lRes;
	TypeConfigNum		typeNew;

	switch (uMsg) {
	case WM_PAINTICON:
		return 0;
	case WM_ICONERASEBKGND:
		return 0;
	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam, lParam);
	case WM_MOUSEMOVE:
		return OnMouseMove(wParam, lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(wParam, lParam);
	case WM_MOUSEWHEEL:
		return OnMouseWheel(wParam, lParam);
	case WM_HSCROLL:
		return OnHScroll(wParam, lParam);
	case WM_VSCROLL:
		return OnVScroll(wParam, lParam);


	case WM_MENUCHAR:
		// メニューアクセスキー押下時の処理(WM_MENUCHAR処理)
		return m_menuDrawer.OnMenuChar(hwnd, uMsg, wParam, lParam);

	// 2007.09.09 Moca 互換BMPによる画面バッファ
	case WM_SHOWWINDOW:
		if (!wParam) {
			Views_DeleteCompatibleBitmap();
		}
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);

	case WM_MENUSELECT:
		if (!m_statusBar.GetStatusHwnd()) {
			return 1;
		}
		uItem = (UINT) LOWORD(wParam);		// menu item or submenu index
		{
			// メニュー機能のテキストをセット
			NativeT memWork;

			// 機能に対応するキー名の取得(複数)
			NativeT** ppcAssignedKeyList;
			auto& csKeyBind = m_pShareData->m_common.m_keyBind;
			int nAssignedKeyNum = KeyBind::GetKeyStrList(
				G_AppInstance(),
				csKeyBind.m_nKeyNameArrNum,
				(KEYDATA*)csKeyBind.m_pKeyNameArr,
				&ppcAssignedKeyList,
				uItem
			);
			if (0 < nAssignedKeyNum) {
				for (int j=0; j<nAssignedKeyNum; ++j) {
					if (j > 0) {
						memWork.AppendString(_T(" , "));
					}
					memWork.AppendNativeData(*ppcAssignedKeyList[j]);
					delete ppcAssignedKeyList[j];
				}
				delete[] ppcAssignedKeyList;
			}
			const TCHAR* pszItemStr = memWork.GetStringPtr();
			m_statusBar.SetStatusText(0, SBT_NOBORDERS, pszItemStr);
		}
		return 0;

	case WM_DRAWITEM:
		idCtl = (UINT) wParam;				// コントロールのID
		lpdis = (DRAWITEMSTRUCT*) lParam;	// 項目描画情報
		if (IDW_STATUSBAR == idCtl) {
			if (lpdis->itemID == 5) { // 2003.08.26 Moca idがずれて作画されなかった
				int	nColor;
				if (m_pShareData->m_flags.m_bRecordingKeyMacro	// キーボードマクロの記録中
				 && m_pShareData->m_flags.m_hwndRecordingKeyMacro == GetHwnd()	// キーボードマクロを記録中のウィンドウ
				) {
					nColor = COLOR_BTNTEXT;
				}else {
					nColor = COLOR_3DSHADOW;
				}
				::SetTextColor(lpdis->hDC, ::GetSysColor(nColor));
				::SetBkMode(lpdis->hDC, TRANSPARENT);
				
				// 2003.08.26 Moca 上下中央位置に作画
				TEXTMETRIC tm;
				::GetTextMetrics(lpdis->hDC, &tm);
				int y = (lpdis->rcItem.bottom - lpdis->rcItem.top - tm.tmHeight + 1) / 2 + lpdis->rcItem.top;
				::TextOut(lpdis->hDC, lpdis->rcItem.left, y, _T("REC"), _tcslen(_T("REC")));
				if (COLOR_BTNTEXT == nColor) {
					::TextOut(lpdis->hDC, lpdis->rcItem.left + 1, y, _T("REC"), _tcslen(_T("REC")));
				}
			}
			return 0;
		}else {
			switch (lpdis->CtlType) {
			case ODT_MENU:	// オーナー描画メニュー
				// メニューアイテム描画
				m_menuDrawer.DrawItem(lpdis);
				return TRUE;
			}
		}
		return FALSE;
	case WM_MEASUREITEM:
		idCtl = (UINT) wParam;					// control identifier
		lpmis = (MEASUREITEMSTRUCT*) lParam;	// item-size information
		switch (lpmis->CtlType) {
		case ODT_MENU:	// オーナー描画メニュー
//			MenuDrawer* pMenuDrawer;
//			pMenuDrawer = (MenuDrawer*)lpmis->itemData;


//			MYTRACE(_T("WM_MEASUREITEM  lpmis->itemID=%d\n"), lpmis->itemID);
			// メニューアイテムの描画サイズを計算
			nItemWidth = m_menuDrawer.MeasureItem(lpmis->itemID, &nItemHeight);
			if (0 < nItemWidth) {
				lpmis->itemWidth = nItemWidth;
				lpmis->itemHeight = nItemHeight;
			}
			return TRUE;
		}
		return FALSE;

	case WM_PAINT:
		return OnPaint(hwnd, uMsg, wParam, lParam);

	case WM_PASTE:
		return GetActiveView().GetCommander().HandleCommand(F_PASTE, true, 0, 0, 0, 0);

	case WM_COPY:
		return GetActiveView().GetCommander().HandleCommand(F_COPY, true, 0, 0, 0, 0);

	case WM_HELP:
		lphi = (LPHELPINFO) lParam;
		switch (lphi->iContextType) {
		case HELPINFO_MENUITEM:
			MyWinHelp(hwnd, HELP_CONTEXT, FuncID_To_HelpContextID((EFunctionCode)lphi->iCtrlId));
			break;
		}
		return TRUE;

	case WM_ACTIVATEAPP:
		m_bIsActiveApp = (wParam != 0);	// 自アプリがアクティブかどうか

		// アクティブ化なら編集ウィンドウリストの先頭に移動する		// 2007.04.08 ryoji WM_SETFOCUS から移動
		if (m_bIsActiveApp) {
			AppNodeGroupHandle(0).AddEditWndList(GetHwnd());	// リスト移動処理

			// 2009.01.17 nasukoji	ホイールスクロール有無状態をクリア
			ClearMouseState();
		}

		// タイマーON/OFF		// 2007.03.08 ryoji WM_ACTIVATEから移動
		UpdateCaption();
		m_funcKeyWnd.Timer_ONOFF(m_bIsActiveApp); // 20060126 aroka
		this->Timer_ONOFF(m_bIsActiveApp); // 20060128 aroka

		return 0L;

	case WM_ENABLE:
		// 右ドロップファイルの受け入れ設定／解除	// 2009.01.09 ryoji
		// Note: DragAcceptFilesを適用した左ドロップについては Enable/Disable で自動的に受け入れ設定／解除が切り替わる
		if ((BOOL)wParam) {
			m_pDropTarget->Register_DropTarget(m_hWnd);
		}else {
			m_pDropTarget->Revoke_DropTarget();
		}
		return 0L;

	case WM_WINDOWPOSCHANGED:
		// ポップアップウィンドウの表示切替指示をポストする	// 2007.10.22 ryoji
		// ・WM_SHOWWINDOWはすべての表示切替で呼ばれるわけではないのでWM_WINDOWPOSCHANGEDで処理
		//   （タブグループ解除などの設定変更時はWM_SHOWWINDOWは呼ばれない）
		// ・即時切替だとタブ切替に干渉して元のタブに戻ってしまうことがあるので後で切り替える
		WINDOWPOS* pwp;
		pwp = (WINDOWPOS*)lParam;
		if (pwp->flags & SWP_SHOWWINDOW)
			::PostMessage(hwnd, MYWM_SHOWOWNEDPOPUPS, TRUE, 0);
		else if (pwp->flags & SWP_HIDEWINDOW)
			::PostMessage(hwnd, MYWM_SHOWOWNEDPOPUPS, FALSE, 0);

		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);

	case MYWM_SHOWOWNEDPOPUPS:
		::ShowOwnedPopups(m_hWnd, (BOOL)wParam);	// 2007.10.22 ryoji
		return 0L;

	case WM_SIZE:
//		MYTRACE(_T("WM_SIZE\n"));
		/* WM_SIZE 処理 */
		if (SIZE_MINIMIZED == wParam) {
			this->UpdateCaption();
		}
		return OnSize(wParam, lParam);

	// From here 2003.05.31 MIK
	case WM_MOVE:
		// From Here 2004.05.13 Moca ウィンドウ位置継承
		//	最後の位置を復元するため，移動されるたびに共有メモリに位置を保存する．
		if (WinSizeMode::Save == m_pShareData->m_common.m_window.m_eSaveWindowPos) {
			if (!::IsZoomed(GetHwnd()) && !::IsIconic(GetHwnd())) {
				// 2005.11.23 Moca ワークエリア座標だとずれるのでスクリーン座標に変更
				// Aero Snapで縦方向最大化で終了して次回起動するときは元のサイズにする必要があるので、
				// GetWindowRect()ではなくGetWindowPlacement()で得たワークエリア座標をスクリーン座標に変換して記憶する	// 2009.09.02 ryoji
				WINDOWPLACEMENT wp;
				wp.length = sizeof(wp);
				::GetWindowPlacement(GetHwnd(), &wp);	// ワークエリア座標
				RECT rcWin = wp.rcNormalPosition;
				RECT rcWork, rcMon;
				GetMonitorWorkRect(GetHwnd(), &rcWork, &rcMon);
				::OffsetRect(&rcWin, rcWork.left - rcMon.left, rcWork.top - rcMon.top);	// スクリーン座標に変換
				m_pShareData->m_common.m_window.m_nWinPosX = rcWin.left;
				m_pShareData->m_common.m_window.m_nWinPosY = rcWin.top;
			}
		}
		// To Here 2004.05.13 Moca ウィンドウ位置継承
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	// To here 2003.05.31 MIK
	case WM_SYSCOMMAND:
		// タブまとめ表示では閉じる動作はオプション指定に従う	// 2006.02.13 ryoji
		//	Feb. 11, 2007 genta 動作を選べるように(MDI風と従来動作)
		// 2007.02.22 ryoji Alt+F4 のデフォルト機能でモード毎の動作が得られるようになった
		if (wParam == SC_CLOSE) {
			// 印刷プレビューモードでウィンドウを閉じる操作のときはプレビューを閉じる	// 2007.03.04 ryoji
			if (m_pPrintPreview) {
				PrintPreviewModeONOFF();	// 印刷プレビューモードのオン/オフ
				return 0L;
			}
			OnCommand(0, (WORD)KeyBind::GetDefFuncCode(VK_F4, _ALT), NULL);
			return 0L;
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
#if 0
	case WM_IME_COMPOSITION:
		if (lParam & GCS_RESULTSTR) {
			// メッセージの配送
			return Views_DispatchEvent(hwnd, uMsg, wParam, lParam);
		}else {
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
#endif
	//case WM_KILLFOCUS:
	case WM_CHAR:
	case WM_IME_CHAR:
	case WM_KEYUP:
	case WM_SYSKEYUP:	// 2004.04.28 Moca ALT+キーのキーリピート処理のため追加
	case WM_ENTERMENULOOP:
#if 0
	case MYWM_IME_REQUEST:   // 再変換対応 by minfu 2002.03.27	20020331 aroka
#endif
		if (GetActiveView().m_nAutoScrollMode) {
			GetActiveView().AutoScrollExit();
		}
		// メッセージの配送
		return Views_DispatchEvent(hwnd, uMsg, wParam, lParam);

	case WM_EXITMENULOOP:
//		MYTRACE(_T("WM_EXITMENULOOP\n"));
		if (m_statusBar.GetStatusHwnd()) {
			m_statusBar.SetStatusText(0, SBT_NOBORDERS, _T(""));
		}
		m_menuDrawer.EndDrawMenu();
		// メッセージの配送
		return Views_DispatchEvent(hwnd, uMsg, wParam, lParam);

	case WM_SETFOCUS:
//		MYTRACE(_T("WM_SETFOCUS\n"));

		// Aug. 29, 2003 wmlhq & ryojiファイルのタイムスタンプのチェック処理 OnTimer に移行
		m_nTimerCount = 9;

		// ビューにフォーカスを移動する	// 2007.10.16 ryoji
		if (!m_pPrintPreview && m_pEditView) {
			::SetFocus(GetActiveView().GetHwnd());
		}
		lRes = 0;

//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる変更
		// 印刷プレビューモードのときは、キー操作は全部PrintPreviewBarへ転送
		if (m_pPrintPreview) {
			m_pPrintPreview->SetFocusToPrintPreviewBar();
		}
		return lRes;

	case WM_NOTIFY:
		pnmh = (LPNMHDR) lParam;
		//	From Here Feb. 15, 2004 genta 
		//	ステータスバーのダブルクリックでモード切替ができるようにする
		if (m_statusBar.GetStatusHwnd() && pnmh->hwndFrom == m_statusBar.GetStatusHwnd()) {
			if (pnmh->code == NM_DBLCLK) {
				LPNMMOUSE mp = (LPNMMOUSE) lParam;
				if (mp->dwItemSpec == 6) {	//	上書き/挿入
					GetDocument()->HandleCommand(F_CHGMOD_INS);
				}else if (mp->dwItemSpec == 5) {	//	マクロの記録開始・終了
					GetDocument()->HandleCommand(F_RECKEYMACRO);
				}else if (mp->dwItemSpec == 1) {	//	桁位置→行番号ジャンプ
					GetDocument()->HandleCommand(F_JUMP_DIALOG);
				}else if (mp->dwItemSpec == 3) {	//	文字コード→各種コード
					ShowCodeBox(GetHwnd(), GetDocument());
				}else if (mp->dwItemSpec == 4) {	//	文字コードセット→文字コードセット指定
					GetDocument()->HandleCommand(F_CHG_CHARSET);
				}
			}else if (pnmh->code == NM_RCLICK) {
				LPNMMOUSE mp = (LPNMMOUSE) lParam;
				if (mp->dwItemSpec == 2) {	//	入力改行モード
					enum eEolExts {
						F_CHGMOD_EOL_NEL = F_CHGMOD_EOL_CR + 1,
						F_CHGMOD_EOL_PS,
						F_CHGMOD_EOL_LS,
					};
					m_menuDrawer.ResetContents();
					HMENU hMenuPopUp = ::CreatePopupMenu();
					m_menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CRLF, 
						LS(F_CHGMOD_EOL_CRLF), _T("C")); // 入力改行コード指定(CRLF)
					m_menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_LF,
						LS(F_CHGMOD_EOL_LF), _T("L")); // 入力改行コード指定(LF)
					m_menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CR,
						LS(F_CHGMOD_EOL_CR), _T("R")); // 入力改行コード指定(CR)
					// 拡張EOLが有効の時だけ表示
					if (GetDllShareData().m_common.m_edit.m_bEnableExtEol) {
						m_menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_NEL,
							LS(STR_EDITWND_MENU_NEL), _T(""), TRUE, -2); // 入力改行コード指定(NEL)
						m_menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_LS,
							LS(STR_EDITWND_MENU_LS), _T(""), TRUE, -2); // 入力改行コード指定(LS)
						m_menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_PS,
							LS(STR_EDITWND_MENU_PS), _T(""), TRUE, -2); // 入力改行コード指定(PS)
					}
					
					//	mp->ptはステータスバー内部の座標なので，スクリーン座標への変換が必要
					POINT po = mp->pt;
					::ClientToScreen(m_statusBar.GetStatusHwnd(), &po);
					EFunctionCode nId = (EFunctionCode)::TrackPopupMenu(
						hMenuPopUp,
						TPM_CENTERALIGN
						| TPM_BOTTOMALIGN
						| TPM_RETURNCMD
						| TPM_LEFTBUTTON
						,
						po.x,
						po.y,
						0,
						GetHwnd(),
						NULL
					);
					::DestroyMenu(hMenuPopUp);
					EolType nEOLCode = EolType::None;
					switch (nId) {
					case F_CHGMOD_EOL_CRLF: nEOLCode = EolType::CRLF; break;
					case F_CHGMOD_EOL_CR: nEOLCode = EolType::CR; break;
					case F_CHGMOD_EOL_LF: nEOLCode = EolType::LF; break;
					case F_CHGMOD_EOL_NEL: nEOLCode = EolType::NEL; break;
					case F_CHGMOD_EOL_PS: nEOLCode = EolType::PS; break;
					case F_CHGMOD_EOL_LS: nEOLCode = EolType::LS; break;
					default:
						nEOLCode = EolType::Unknown;
					}
					if (nEOLCode != EolType::Unknown) {
						GetActiveView().GetCommander().HandleCommand(F_CHGMOD_EOL, true, (int)nEOLCode, 0, 0, 0);
					}
				}
			}
			return 0L;
		}
		//	To Here Feb. 15, 2004 genta 

		switch (pnmh->code) {
		// 2007.09.08 kobake TTN_NEEDTEXTの処理をA版とW版に分けて明示的に処理するようにしました。
		//                   ※テキストが80文字を超えそうならTOOLTIPTEXT::lpszTextを利用してください。
		// 2008.11.03 syat   矩形範囲選択開始のツールチップで80文字超えていたのでlpszTextに変更。
		case TTN_NEEDTEXT:
			{
				static TCHAR szText[256] = {0};
				// ツールチップテキスト取得、設定
				LPTOOLTIPTEXT lptip = (LPTOOLTIPTEXT)pnmh;
				GetTooltipText(szText, _countof(szText), lptip->hdr.idFrom);
				lptip->lpszText = szText;
			}
			break;

		case TBN_DROPDOWN:
			{
				int	nId;
				nId = CreateFileDropDownMenu(pnmh->hwndFrom);
				if (nId != 0) OnCommand((WORD)0 /*メニュー*/, (WORD)nId, (HWND)0);
			}
			return FALSE;
		//	From Here Jul. 21, 2003 genta
		case NM_CUSTOMDRAW:
			if (pnmh->hwndFrom == m_toolbar.GetToolbarHwnd()) {
				//	ツールバーのOwner Draw
				return m_toolbar.ToolBarOwnerDraw((LPNMCUSTOMDRAW)pnmh);
			}
			break;
		//	To Here Jul. 21, 2003 genta
		}
		return 0L;
	case WM_COMMAND:
		OnCommand(HIWORD(wParam), LOWORD(wParam), (HWND) lParam);
		return 0L;
	case WM_INITMENUPOPUP:
		InitMenu((HMENU)wParam, (UINT)LOWORD(lParam), (BOOL)HIWORD(lParam));
		return 0L;
	case WM_DROPFILES:
		// ファイルがドロップされた
		OnDropFiles((HDROP) wParam);
		return 0L;
	case WM_QUERYENDSESSION:	// OSの終了
		if (OnClose(NULL, false)) {
			::DestroyWindow(hwnd);
			return TRUE;
		}else {
			return FALSE;
		}
	case WM_CLOSE:
		if (OnClose(NULL, false)) {
			::DestroyWindow(hwnd);
		}
		return 0L;
	case WM_DESTROY:
		if (m_pShareData->m_flags.m_bRecordingKeyMacro) {					// キーボードマクロの記録中
			if (m_pShareData->m_flags.m_hwndRecordingKeyMacro == GetHwnd()) {	// キーボードマクロを記録中のウィンドウ
				m_pShareData->m_flags.m_bRecordingKeyMacro = FALSE;			// キーボードマクロの記録中
				m_pShareData->m_flags.m_hwndRecordingKeyMacro = NULL;		// キーボードマクロを記録中のウィンドウ
			}
		}

		// タイマーを削除
		::KillTimer(GetHwnd(), IDT_TOOLBAR);

		// ドロップされたファイルを受け入れるのを解除
		::DragAcceptFiles(hwnd, FALSE);
		m_pDropTarget->Revoke_DropTarget();	// 右ボタンドロップ用	// 2008.06.20 ryoji

		// 編集ウィンドウリストからの削除
		AppNodeGroupHandle(GetHwnd()).DeleteEditWndList(GetHwnd());

		if (m_pShareData->m_handles.m_hwndDebug == GetHwnd()) {
			m_pShareData->m_handles.m_hwndDebug = NULL;
		}
		m_hWnd = NULL;


		// 編集ウィンドウオブジェクトからのオブジェクト削除要求
		::PostMessage(m_pShareData->m_handles.m_hwndTray, MYWM_DELETE_ME, 0, 0);

		// Windows にスレッドの終了を要求します
		::PostQuitMessage(0);

		return 0L;

	case WM_THEMECHANGED:
		// 2006.06.17 ryoji
		// ビジュアルスタイル／クラシックスタイルが切り替わったらツールバーを再作成する
		// （ビジュアルスタイル: Rebar 有り、クラシックスタイル: Rebar 無し）
		if (m_toolbar.GetToolbarHwnd()) {
			if (IsVisualStyle() == (!m_toolbar.GetRebarHwnd())) {
				m_toolbar.DestroyToolBar();
				LayoutToolBar();
				EndLayoutBars();
			}
		}
		return 0L;

	case MYWM_UIPI_CHECK:
		// エディタ－トレイ間でのUI特権分離の確認メッセージ	2007.06.07 ryoji
		m_bUIPI = TRUE;	// トレイからの返事を受け取った
		return 0L;

	case MYWM_CLOSE:
		// エディタへの終了要求
		if (nRet = OnClose(
				(HWND)lParam,
				PM_CLOSE_GREPNOCONFIRM == (PM_CLOSE_GREPNOCONFIRM & wParam)
			)
		) { // Jan. 23, 2002 genta 警告抑制
			// プラグイン：DocumentCloseイベント実行
			Plug::Array plugs;
			WSHIfObj::List params;
			JackManager::getInstance()->GetUsablePlug(PP_DOCUMENT_CLOSE, 0, &plugs);
			for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
				(*it)->Invoke(&GetActiveView(), params);
			}

			// プラグイン：EditorEndイベント実行
			plugs.clear();
			JackManager::getInstance()->GetUsablePlug(PP_EDITOR_END, 0, &plugs);
			for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
				(*it)->Invoke(&GetActiveView(), params);
			}

			// タブまとめ表示では閉じる動作はオプション指定に従う	// 2006.02.13 ryoji
			if ((wParam & PM_CLOSE_EXIT) != PM_CLOSE_EXIT) {	// 全終了要求でない場合
				// タブまとめ表示で(無題)を残す指定の場合、残ウィンドウが１個なら新規エディタを起動して終了する
				if (m_pShareData->m_common.m_tabBar.m_bDispTabWnd &&
					!m_pShareData->m_common.m_tabBar.m_bDispTabWndMultiWin &&
					m_pShareData->m_common.m_tabBar.m_bTab_RetainEmptyWin
				) {
					// 自グループ内の残ウィンドウ数を調べる	// 2007.06.20 ryoji
					int nGroup = AppNodeManager::getInstance()->GetEditNode(GetHwnd())->GetGroup();
					if (AppNodeGroupHandle(nGroup).GetEditorWindowsNum() == 1) {
						EditNode* pEditNode = AppNodeManager::getInstance()->GetEditNode(GetHwnd());
						if (pEditNode)
							pEditNode->m_bClosing = TRUE;	// 自分はタブ表示してもらわなくていい
						LoadInfo loadInfo;
						loadInfo.filePath = _T("");
						loadInfo.eCharCode = CODE_NONE;
						loadInfo.bViewMode = false;
						ControlTray::OpenNewEditor(
							G_AppInstance(),
							GetHwnd(),
							loadInfo,
							NULL,
							true
						);
					}
				}
			}
			::DestroyWindow(hwnd);
		}
		return nRet;
	case MYWM_ALLOWACTIVATE:
		::AllowSetForegroundWindow(wParam);
		return 0L;

		
	case MYWM_GETFILEINFO:
		// トレイからエディタへの編集ファイル名要求通知
		pfi = (EditInfo*)&m_pShareData->m_workBuffer.m_EditInfo_MYWM_GETFILEINFO;

		// 編集ファイル情報を格納
		GetDocument()->GetEditInfo(pfi);
		return 0L;
	case MYWM_CHANGESETTING:
		// 設定変更の通知
		switch ((e_PM_CHANGESETTING_SELECT)lParam) {
		case PM_CHANGESETTING_ALL:
			// 言語を選択する
			SelectLang::ChangeLang(GetDllShareData().m_common.m_window.m_szLanguageDll);
			ShareData::getInstance()->RefreshString();

			// メインメニュー	2010/5/16 Uchi
			LayoutMainMenu();

			// Oct 10, 2000 ao
			// 設定変更時、ツールバーを再作成するようにする（バーの内容変更も反映）
			m_toolbar.DestroyToolBar();
			LayoutToolBar();
			// Oct 10, 2000 ao ここまで

			// 2008.10.05 nasukoji	非アクティブなウィンドウのツールバーを更新する
			// アクティブなウィンドウはタイマにより更新されるが、それ以外のウィンドウは
			// タイマを停止させており設定変更すると全部有効となってしまうため、ここで
			// ツールバーを更新する
			if (!m_bIsActiveApp)
				m_toolbar.UpdateToolbar();

			// ファンクションキーを再作成する（バーの内容、位置、グループボタン数の変更も反映）	// 2006.12.19 ryoji
			m_funcKeyWnd.Close();
			LayoutFuncKey();

			// タブバーの表示／非表示切り替え	// 2006.12.19 ryoji
			LayoutTabBar();

			// ステータスバーの表示／非表示切り替え	// 2006.12.19 ryoji
			LayoutStatusBar();

			// 水平スクロールバーの表示／非表示切り替え	// 2006.12.19 ryoji
			{
				bool b1 = (m_pShareData->m_common.m_window.m_bScrollBarHorz == FALSE);
				for (int i=0; i<GetAllViewCount(); ++i) {
					bool b2 = (GetView(i).m_hwndHScrollBar == NULL);
					if (b1 != b2) {		// 水平スクロールバーを使う
						GetView(i).DestroyScrollBar();
						GetView(i).CreateScrollBar();
					}
				}
			}

			LayoutMiniMap();

			// バー変更で画面が乱れないように	// 2006.12.19 ryoji
			EndLayoutBars();

			// アクセラレータテーブルを再作成する(Wine用)
			// ウィンドウ毎に作成したアクセラレータテーブルを破棄する(Wine用)
			DeleteAccelTbl();
			// ウィンドウ毎にアクセラレータテーブルを作成する(Wine用)
			CreateAccelTbl();
			
			if (m_pShareData->m_common.m_tabBar.m_bDispTabWnd) {
				// タブ表示のままグループ化する／しないが変更されていたらタブを更新する必要がある
				m_tabWnd.Refresh(false);
			}
			if (m_pShareData->m_common.m_tabBar.m_bDispTabWnd && !m_pShareData->m_common.m_tabBar.m_bDispTabWndMultiWin) {
				if (AppNodeManager::getInstance()->GetEditNode(GetHwnd())->IsTopInGroup()) {
					if (!::IsWindowVisible(GetHwnd())) {
						// ::ShowWindow(GetHwnd(), SW_SHOWNA) だと非表示から表示に切り替わるときに Z-order がおかしくなることがあるので ::SetWindowPos を使う
						::SetWindowPos(GetHwnd(), NULL, 0, 0, 0, 0,
										SWP_SHOWWINDOW | SWP_NOACTIVATE
										| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);

						// このウィンドウの WS_EX_TOPMOST 状態を全ウィンドウに反映する	// 2007.05.18 ryoji
						WindowTopMost(((DWORD)::GetWindowLongPtr(GetHwnd(), GWL_EXSTYLE) & WS_EX_TOPMOST)? 1: 2);
					}
				}else {
					if (::IsWindowVisible(GetHwnd())) {
						::ShowWindow(GetHwnd(), SW_HIDE);
					}
				}
			}else {
				if (!::IsWindowVisible(GetHwnd())) {
					// ::ShowWindow(GetHwnd(), SW_SHOWNA) だと非表示から表示に切り替わるときに Z-order がおかしくなることがあるので ::SetWindowPos を使う
					::SetWindowPos(GetHwnd(), NULL, 0, 0, 0, 0,
									SWP_SHOWWINDOW | SWP_NOACTIVATE
									| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
				}
			}

			//	Aug, 21, 2000 genta
			GetDocument()->m_autoSaveAgent.ReloadAutoSaveParam();

			GetDocument()->OnChangeSetting();	// ビューに設定変更を反映させる
			GetDocument()->m_docType.SetDocumentIcon();	// Sep. 10, 2002 genta 文書アイコンの再設定

			break;
		case PM_CHANGESETTING_FONT:
			GetDocument()->OnChangeSetting(true);	// フォントで文字幅が変わるので、レイアウト再構築
			break;
		case PM_CHANGESETTING_FONTSIZE:
			if ((wParam == -1 && GetLogfontCacheMode() == CharWidthCacheMode::Share)
				|| GetDocument()->m_docType.GetDocumentType().GetIndex() == wParam
			) {
				GetDocument()->OnChangeSetting( false );	// ビューに設定変更を反映させる(レイアウト情報の再作成しない)
			}
			break;
		case PM_CHANGESETTING_TYPE:
			typeNew = DocTypeManager().GetDocumentTypeOfPath(GetDocument()->m_docFile.GetFilePath());
			if (GetDocument()->m_docType.GetDocumentType().GetIndex() == wParam
				|| typeNew.GetIndex() == wParam
			) {
				GetDocument()->OnChangeSetting();

				// アウトライン解析画面処理
				bool bAnalyzed = FALSE;
#if 0
				if (/* 必要なら変更条件をここに記述する（将来用） */) {
					// アウトライン解析画面の位置を現在の設定に合わせる
					bAnalyzed = m_dlgFuncList.ChangeLayout(OUTLINE_LAYOUT_BACKGROUND);	// 外部からの変更通知と同等の扱い
				}
#endif
				if (m_dlgFuncList.GetHwnd() && !bAnalyzed) {	// アウトラインを開いていれば再解析
					// SHOW_NORMAL: 解析方法が変化していれば再解析される。そうでなければ描画更新（変更されたカラーの適用）のみ。
					EFunctionCode nFuncCode = m_dlgFuncList.GetFuncCodeRedraw(m_dlgFuncList.m_nOutlineType);
					GetActiveView().GetCommander().HandleCommand(nFuncCode, true, (LPARAM)ShowDialogType::Normal, 0, 0, 0);
				}
				if (MyGetAncestor(::GetForegroundWindow(), GA_ROOTOWNER2) == GetHwnd())
					::SetFocus(GetActiveView().GetHwnd());	// フォーカスを戻す
			}
			break;
		case PM_CHANGESETTING_TYPE2:
			typeNew = DocTypeManager().GetDocumentTypeOfPath(GetDocument()->m_docFile.GetFilePath());
			if (GetDocument()->m_docType.GetDocumentType().GetIndex() == wParam
				|| typeNew.GetIndex() == wParam
			) {
				// indexのみ更新
				GetDocument()->m_docType.SetDocumentTypeIdx();
				// タイプが変更になった場合は適用する
				if (GetDocument()->m_docType.GetDocumentType().GetIndex() != wParam) {
					::SendMessage(m_hWnd, MYWM_CHANGESETTING, wParam, PM_CHANGESETTING_TYPE);
				}
			}
			break;
		case PM_PRINTSETTING:
			{
				if (m_pPrintPreview) {
					m_pPrintPreview->OnChangeSetting();
				}
			}
			break;
		default:
			break;
		}
		return 0L;
	case MYWM_SAVEEDITSTATE:
		{
			if (m_pPrintPreview) {
				// 一時的に設定を戻す
				SelectCharWidthCache(CharWidthFontMode::Edit, CharWidthCacheMode::Neutral);
			}
			// フォント変更前の座標の保存
			m_posSaveAry = SavePhysPosOfAllView();
			if (m_pPrintPreview) {
				// 設定を戻す
				SelectCharWidthCache(CharWidthFontMode::Print, CharWidthCacheMode::Local);
			}
		}
		return 0L; 
	case MYWM_SETACTIVEPANE:
		if ((int)wParam == -1) {
			if (lParam == 0) {
				nPane = m_splitterWnd.GetFirstPane();
			}else {
				nPane = m_splitterWnd.GetLastPane();
			}
			this->SetActivePane(nPane);
		}
		return 0L;
		
	case MYWM_SETCARETPOS:	// カーソル位置変更通知
		{
			//	2006.07.09 genta LPARAMに新たな意味を追加
			//	bit 0 (MASK 1): (bit 1==0のとき) 0/選択クリア, 1/選択開始・変更
			//	bit 1 (MASK 2): 0: bit 0の設定に従う．1:現在の選択ロックs状態を継続
			//	既存の実装では どちらも0なので強制解除と解釈される．
			//	呼び出し時はe_PM_SETCARETPOS_SELECTSTATEの値を使うこと．
			bool bSelect = ((lParam & 1) != 0);
			if (lParam & 2) {
				// 現在の状態をKEEP
				bSelect = GetActiveView().GetSelectionInfo().m_bSelectingLock;
			}
			
			//	2006.07.09 genta 強制解除しない
			/*
			カーソル位置変換
			 物理位置(行頭からのバイト数、折り返し無し行位置)
			→
			 レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/
			LogicPoint* ppoCaret = &(m_pShareData->m_workBuffer.m_LogicPoint);
			LayoutPoint ptCaretPos;
			GetDocument()->m_layoutMgr.LogicToLayout(
				*ppoCaret,
				&ptCaretPos
			);
			// 改行の真ん中にカーソルが来ないように	// 2007.08.22 ryoji
			// Note. もとが改行単位の桁位置なのでレイアウト折り返しの桁位置を超えることはない。
			//       選択指定(bSelect==TRUE)の場合にはどうするのが妥当かよくわからないが、
			//       2007.08.22現在ではアウトライン解析ダイアログから桁位置0で呼び出される
			//       パターンしかないので実用上特に問題は無い。
			if (!bSelect) {
				const DocLine *pTmpDocLine = GetDocument()->m_docLineMgr.GetLine(ppoCaret->GetY2());
				if (pTmpDocLine) {
					if (pTmpDocLine->GetLengthWithoutEOL() < ppoCaret->x) ptCaretPos.x--;
				}
			}
			//	2006.07.09 genta 選択範囲を考慮して移動
			//	MoveCursorの位置調整機能があるので，最終行以降への
			//	移動指示の調整もMoveCursorにまかせる
			GetActiveView().MoveCursorSelecting(ptCaretPos, bSelect, _CARETMARGINRATE / 3);
		}
		return 0L;


	case MYWM_GETCARETPOS:	// カーソル位置取得要求
		/*
		カーソル位置変換
		 レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
		→
		物理位置(行頭からのバイト数、折り返し無し行位置)
		*/
		{
			LogicPoint* ppoCaret = &(m_pShareData->m_workBuffer.m_LogicPoint);
			GetDocument()->m_layoutMgr.LayoutToLogic(
				GetActiveView().GetCaret().GetCaretLayoutPos(),
				ppoCaret
			);
		}
		return 0L;

	case MYWM_GETLINEDATA:	// 行(改行単位)データの要求
	{
		// 共有データ：自分Write→相手Read
		// return 0以上：行データあり。wParamオフセットを除いた行データ長。0はEOFかOffsetがちょうどバッファ長だった
		//       -1以下：エラー
		LogicInt	nLineNum = LogicInt(wParam);
		LogicInt	nLineOffset = LogicInt(lParam);
		if (nLineNum < 0 || GetDocument()->m_docLineMgr.GetLineCount() < nLineNum) {
			return -2; // 行番号不正。LineCount == nLineNum はEOF行として下で処理
		}
		LogicInt nLineLen = LogicInt(0);
		const wchar_t* pLine = GetDocument()->m_docLineMgr.GetLine(nLineNum)->GetDocLineStrWithEOL( &nLineLen );
		if (nLineOffset < 0 || nLineLen < nLineOffset) {
			return -3; // オフセット位置不正
		}
		if (nLineNum == GetDocument()->m_docLineMgr.GetLineCount()) {
			return 0; // EOF正常終了
		}
 		if (!pLine) {
			return -4; // 不明なエラー
		}
		if (nLineLen == nLineOffset) {
 			return 0;
 		}
		pLine = GetDocument()->m_docLineMgr.GetLine(LogicInt(wParam))->GetDocLineStrWithEOL( &nLineLen );
		pLine += nLineOffset;
		nLineLen -= nLineOffset;
		size_t nEnd = t_min<size_t>(nLineLen, m_pShareData->m_workBuffer.GetWorkBufferCount<EDIT_CHAR>());
		auto_memcpy( m_pShareData->m_workBuffer.GetWorkBuffer<EDIT_CHAR>(), pLine, nEnd );
		return nLineLen;
	}

	// 2010.05.11 Moca MYWM_ADDSTRINGLEN_Wを追加 NULセーフ
	case MYWM_ADDSTRINGLEN_W:
		{
			EDIT_CHAR* pWork = m_pShareData->m_workBuffer.GetWorkBuffer<EDIT_CHAR>();
			size_t addSize = t_min((size_t)wParam, m_pShareData->m_workBuffer.GetWorkBufferCount<EDIT_CHAR>());
			GetActiveView().GetCommander().HandleCommand(F_ADDTAIL_W, true, (LPARAM)pWork, (LPARAM)addSize, 0, 0);
			GetActiveView().GetCommander().HandleCommand(F_GOFILEEND, true, 0, 0, 0, 0);
		}
		return 0L;

	// タブウィンドウ	//@@@ 2003.05.31 MIK
	case MYWM_TAB_WINDOW_NOTIFY:
		m_tabWnd.TabWindowNotify(wParam, lParam);
		{
			RECT rc;
			::GetClientRect(GetHwnd(), &rc);
			OnSize2(m_nWinSizeType, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ), false);
			GetActiveView().SetIMECompFormPos();
		}
		return 0L;

	// アウトライン	// 2010.06.06 ryoji
	case MYWM_OUTLINE_NOTIFY:
		m_dlgFuncList.OnOutlineNotify(wParam, lParam);
		return 0L;

	// バーの表示・非表示	//@@@ 2003.06.10 MIK
	case MYWM_BAR_CHANGE_NOTIFY:
		if (GetHwnd() != (HWND)lParam) {
			switch ((BarChangeNotifyType)wParam) {
			case BarChangeNotifyType::Toolbar:
				LayoutToolBar();	// 2006.12.19 ryoji
				break;
			case BarChangeNotifyType::FuncKey:
				LayoutFuncKey();	// 2006.12.19 ryoji
				break;
			case BarChangeNotifyType::Tab:
				LayoutTabBar();		// 2006.12.19 ryoji
				if (m_pShareData->m_common.m_tabBar.m_bDispTabWnd
					&& !m_pShareData->m_common.m_tabBar.m_bDispTabWndMultiWin
				) {
					::ShowWindow(GetHwnd(), SW_HIDE);
				}else {
					// ::ShowWindow(hwnd, SW_SHOWNA) だと非表示から表示に切り替わるときに Z-order がおかしくなることがあるので ::SetWindowPos を使う
					::SetWindowPos(hwnd, NULL, 0,0,0,0,
									SWP_SHOWWINDOW | SWP_NOACTIVATE
									| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
				}
				break;
			case BarChangeNotifyType::StatusBar:
				LayoutStatusBar();		// 2006.12.19 ryoji
				break;
			case BarChangeNotifyType::MiniMap:
				LayoutMiniMap();
				break;
			}
			EndLayoutBars();	// 2006.12.19 ryoji
		}
		return 0L;

	// by 鬼 (2) MYWM_CHECKSYSMENUDBLCLKは不要に, WM_LBUTTONDBLCLK追加
	case WM_NCLBUTTONDOWN:
		return OnNcLButtonDown(wParam, lParam);

	case WM_NCLBUTTONUP:
		return OnNcLButtonUp(wParam, lParam);

	case WM_LBUTTONDBLCLK:
		return OnLButtonDblClk(wParam, lParam);

#if 0
	case WM_IME_NOTIFY:	// Nov. 26, 2006 genta
		if (wParam == IMN_SETCONVERSIONMODE || wParam == IMN_SETOPENSTATUS) {
			GetActiveView().GetCaret().ShowEditCaret();
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
#endif

	case WM_NCPAINT:
		DefWindowProc(hwnd, uMsg, wParam, lParam);
		if (!m_statusBar.GetStatusHwnd()) {
			PrintMenubarMessage(NULL);
		}
		return 0;

	case WM_NCACTIVATE:
		// 編集ウィンドウ切替中（タブまとめ時）はタイトルバーのアクティブ／非アクティブ状態をできるだけ変更しないように（１）	// 2007.04.03 ryoji
		// 前面にいるのが編集ウィンドウならアクティブ状態を保持する
		if (m_pShareData->m_flags.m_bEditWndChanging && IsSakuraMainWindow(::GetForegroundWindow())) {
			wParam = TRUE;	// アクティブ
		}
		lRes = DefWindowProc(hwnd, uMsg, wParam, lParam);
		if (!m_statusBar.GetStatusHwnd()) {
			PrintMenubarMessage(NULL);
		}
		return lRes;

	case WM_SETTEXT:
		// 編集ウィンドウ切替中（タブまとめ時）はタイトルバーのアクティブ／非アクティブ状態をできるだけ変更しないように（２）	// 2007.04.03 ryoji
		// タイマーを使用してタイトルの変更を遅延する
		if (m_pShareData->m_flags.m_bEditWndChanging) {
			delete[] m_pszLastCaption;
			m_pszLastCaption = new TCHAR[::_tcslen((LPCTSTR)lParam) + 1];
			::_tcscpy(m_pszLastCaption, (LPCTSTR)lParam);	// 変更後のタイトルを記憶しておく
			::SetTimer(GetHwnd(), IDT_CAPTION, 50, NULL);
			return 0L;
		}
		::KillTimer(GetHwnd(), IDT_CAPTION);	// タイマーが残っていたら削除する（遅延タイトルを破棄）
		return DefWindowProc(hwnd, uMsg, wParam, lParam);

	case WM_TIMER:
		if (!OnTimer(wParam, lParam))
			return 0L;
		return DefWindowProc(hwnd, uMsg, wParam, lParam);

	default:
#if 0
// << 20020331 aroka 再変換対応 for 95/NT
		if (uMsg == m_uMSIMEReconvertMsg || uMsg == m_uATOKReconvertMsg) {
			return Views_DispatchEvent(hwnd, uMsg, wParam, lParam);
		}
// >> by aroka
#endif
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

/*! 終了時の処理

	@param hWndFrom [in] 終了要求の Wimdow Handle	// 2013/4/9 Uchi

	@retval TRUE: 終了して良い / FALSE: 終了しない
*/
int	EditWnd::OnClose(HWND hWndActive, bool bGrepNoConfirm)
{
	// ファイルを閉じるときのMRU登録 & 保存確認 & 保存実行
	int nRet = GetDocument()->OnFileClose(bGrepNoConfirm);
	if (!nRet) return nRet;
	// パラメータでハンドルを貰う様にしたので検索を削除	2013/4/9 Uchi
	if (hWndActive) {
		// アクティブ化制御ウィンドウをアクティブ化する
		if (IsSakuraMainWindow(hWndActive)) {
			ActivateFrameWindow(hWndActive);	// エディタ
		}else {
			::SetForegroundWindow(hWndActive);	// タスクトレイ
		}
	}

#if 0
	// 2005.09.01 ryoji タブまとめ表示の場合は次のウィンドウを前面に（終了時のウィンドウちらつきを抑制）
	if (m_pShareData->m_common.m_tabBar.m_bDispTabWnd
		&& !m_pShareData->m_common.m_tabBar.m_bDispTabWndMultiWin
	) {
		int i, j;
		EditNode* p = NULL;
		int nCount = AppNodeManager::getInstance()->GetOpenedWindowArr(&p, FALSE);
		if (nCount > 1) {
			for (i=0; i<nCount; ++i) {
				if (p[i].GetHwnd() == GetHwnd())
					break;
			}
			if (i < nCount) {
				for (j=i+1; j<nCount; ++j) {
					if (p[j].m_nGroup == p[i].m_nGroup)
						break;
				}
				if (j >= nCount) {
					for (j=0; j<i; ++j) {
						if (p[j].m_nGroup == p[i].m_nGroup)
							break;
					}
				}
				if (j != i) {
					HWND hwnd = p[j].GetHwnd();
					{
						// 2006.01.28 ryoji
						// タブまとめ表示でこの画面が非表示から表示に変わってすぐ閉じる場合(タブの中クリック時等)、
						// 以前のウィンドウが消えるよりも先に一気にここまで処理が進んでしまうと
						// あとで画面がちらつくので、以前のウィンドウが消えるのをちょっとだけ待つ
						int iWait = 0;
						while (::IsWindowVisible(hwnd) && iWait++ < 20)
							::Sleep(1);
					}
					if (!::IsWindowVisible(hwnd)) {
						ActivateFrameWindow(hwnd);
					}
				}
			}
		}
		if (p) delete []p;
	}
#endif	// 0

	return nRet;
}


/*! WM_COMMAND処理
	@date 2000.11.15 JEPRO // ショートカットキーがうまく働かないので殺してあった下の2行(F_HELP_CONTENTS,F_HELP_SEARCH)を修正・復活
	@date 2013.05.09 novice 重複するメッセージ処理削除
*/
void EditWnd::OnCommand(WORD wNotifyCode, WORD wID , HWND hwndCtl)
{
	// 検索ボックスからの WM_COMMAND はすべてコンボボックス通知
	// ##### 検索ボックス処理はツールバー側の WindowProc に集約するほうがスマートかも
	if (m_toolbar.GetSearchHwnd() && hwndCtl == m_toolbar.GetSearchHwnd()) {
		switch (wNotifyCode) {
		case CBN_SETFOCUS:
			m_nCurrentFocus = F_SEARCH_BOX;
			break;
		case CBN_KILLFOCUS:
			m_nCurrentFocus = 0;
			// フォーカスがはずれたときに検索キーにしてしまう。
			// 検索キーワードを取得
			std::wstring	strText;
			if (m_toolbar.GetSearchKey(strText)) {	// キー文字列がある
				// 検索キーを登録
				if (strText.length() < _MAX_PATH) {
					SearchKeywordManager().AddToSearchKeyArr(strText.c_str());
				}
				GetActiveView().m_strCurSearchKey = strText;
				GetActiveView().m_bCurSearchUpdate = true;
				GetActiveView().ChangeCurRegexp();
			}
			break;
		}
		return;	// CBN_SELCHANGE(1) がアクセラレータと誤認されないようにここで抜ける（rev1886 の問題の抜本対策）
	}

	switch (wNotifyCode) {
	// メニューからのメッセージ
	case 0:
	case CMD_FROM_MOUSE: // 2006.05.19 genta マウスから呼びだされた場合
		// ウィンドウ切り替え
		if (wID - IDM_SELWINDOW >= 0 && wID - IDM_SELWINDOW < m_pShareData->m_nodes.m_nEditArrNum) {
			ActivateFrameWindow(m_pShareData->m_nodes.m_pEditArr[wID - IDM_SELWINDOW].GetHwnd());
		}
		// 最近使ったファイル
		else if (wID - IDM_SELMRU >= 0 && wID - IDM_SELMRU < 999) {
			// 指定ファイルが開かれているか調べる
			const MRUFile mru;
			EditInfo checkEditInfo;
			mru.GetEditInfo(wID - IDM_SELMRU, &checkEditInfo);
			LoadInfo loadInfo(checkEditInfo.m_szPath, checkEditInfo.m_nCharCode, false);
			GetDocument()->m_docFileOperation.FileLoad(&loadInfo);	//	Oct.  9, 2004 genta 共通関数化
		}
		// 最近使ったフォルダ
		else if (wID - IDM_SELOPENFOLDER >= 0 && wID - IDM_SELOPENFOLDER < 999) {
			// フォルダ取得
			const MRUFolder mruFolder;
			LPCTSTR pszFolderPath = mruFolder.GetPath(wID - IDM_SELOPENFOLDER);

			// Stonee, 2001/12/21 UNCであれば接続を試みる
			NetConnect(pszFolderPath);

			//「ファイルを開く」ダイアログ
			LoadInfo loadInfo(_T(""), CODE_AUTODETECT, false);
			DocFileOperation& docOp = GetDocument()->m_docFileOperation;
			std::vector<std::tstring> files;
			if (docOp.OpenFileDialog(GetHwnd(), pszFolderPath, &loadInfo, files)) {
				loadInfo.filePath = files[0].c_str();
				// 開く
				docOp.FileLoad(&loadInfo);

				// 新たな編集ウィンドウを起動
				size_t nSize = files.size();
				for (size_t f=1; f<nSize; ++f) {
					loadInfo.filePath = files[f].c_str();
					ControlTray::OpenNewEditor(
						G_AppInstance(),
						GetHwnd(),
						loadInfo,
						NULL,
						true
					);
				}
			}
		}else {
			// その他コマンド
			// ビューにフォーカスを移動しておく
			if (wID != F_SEARCH_BOX && m_nCurrentFocus == F_SEARCH_BOX) {
				::SetFocus(GetActiveView().GetHwnd());
			}

			// コマンドコードによる処理振り分け
			//	May 19, 2006 genta 上位ビットを渡す
			//	Jul. 7, 2007 genta 上位ビットを定数に
			GetDocument()->HandleCommand((EFunctionCode)(wID | 0));
		}
		break;
	// アクセラレータからのメッセージ
	case 1:
		{
			// ビューにフォーカスを移動しておく
			if (wID != F_SEARCH_BOX && m_nCurrentFocus == F_SEARCH_BOX)
				::SetFocus(GetActiveView().GetHwnd());
			auto& csKeyBind = m_pShareData->m_common.m_keyBind;
			EFunctionCode nFuncCode = KeyBind::GetFuncCode(
				wID,
				csKeyBind.m_nKeyNameArrNum,
				csKeyBind.m_pKeyNameArr
			);
			GetDocument()->HandleCommand((EFunctionCode)(nFuncCode | FA_FROMKEYBOARD));
		}
		break;
	}
	return;
}



//	キーワード：メニューバー順序
//	Sept.14, 2000 Jepro note: メニューバーの項目のキャプションや順番設定などは以下で行っているらしい
//	Sept.16, 2000 Jepro note: アイコンとの関連付けはCShareData_new2.cppファイルで行っている
//	2010/5/16	Uchi	動的に作成する様に変更	
void EditWnd::InitMenu(HMENU hMenu, UINT uPos, BOOL fSystemMenu)
{
	int		numMenuItems;
	int		nPos;
	HMENU	hMenuPopUp;

	if (hMenu == ::GetSubMenu(::GetMenu(GetHwnd()), uPos)
		&& !fSystemMenu
	) {
		// 情報取得
		const CommonSetting_MainMenu*	pMenu = &m_pShareData->m_common.m_mainMenu;
		const MainMenu*	pMainMenu;
		int		nIdxStr;
		int		nIdxEnd;
		int		nLv;
		std::vector<HMENU>	hSubMenu;
		std::wstring tmpMenuName;
		const wchar_t* pMenuName;

		nIdxStr = pMenu->m_nMenuTopIdx[uPos];
		nIdxEnd = (uPos < MAX_MAINMENU_TOP) ? pMenu->m_nMenuTopIdx[uPos + 1] : -1;
		if (nIdxEnd < 0) {
			nIdxEnd = pMenu->m_nMainMenuNum;
		}

		// メニュー 初期化
		m_menuDrawer.ResetContents();
		numMenuItems = ::GetMenuItemCount(hMenu);
		for (int i=numMenuItems-1; i>=0; --i) {
			::DeleteMenu(hMenu, i, MF_BYPOSITION);
		}

		// メニュー作成
		hSubMenu.push_back(hMenu);
		nLv = 1;
		if (pMenu->m_mainMenuTbl[nIdxStr].m_nType == MainMenuType::Special) {
			nLv = 0;
			--nIdxStr;
		}
		for (int i=nIdxStr+1; i<nIdxEnd; ++i) {
			pMainMenu = &pMenu->m_mainMenuTbl[i];
			if (pMainMenu->m_nLevel != nLv) {
				nLv = pMainMenu->m_nLevel;
				if (hSubMenu.size() < (size_t)nLv) {
					// 保護
					break;
				}
				hMenu = hSubMenu[nLv-1];
			}
			switch (pMainMenu->m_nType) {
			case MainMenuType::Node:
				hMenuPopUp = ::CreatePopupMenu();
				if (pMainMenu->nFunc != 0 && pMainMenu->m_sName[0] == L'\0') {
					// ストリングテーブルから読み込み
					tmpMenuName = LSW(pMainMenu->nFunc);
					if (MAX_MAIN_MENU_NAME_LEN < tmpMenuName.length()) {
						tmpMenuName = tmpMenuName.substr(0, MAX_MAIN_MENU_NAME_LEN);
					}
					pMenuName = tmpMenuName.c_str();
				}else {
					pMenuName = pMainMenu->m_sName;
				}
				m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , 
					pMenuName, pMainMenu->m_sKey);
				if (hSubMenu.size() > (size_t)nLv) {
					hSubMenu[nLv] = hMenuPopUp;
				}else {
					hSubMenu.push_back(hMenuPopUp);
				}
				break;
			case MainMenuType::Leaf:
				InitMenu_Function(hMenu, pMainMenu->nFunc, pMainMenu->m_sName, pMainMenu->m_sKey);
				break;
			case MainMenuType::Separator:
				m_menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
				break;
			case MainMenuType::Special:
				bool	bInList;		// リストが1個以上ある
				bInList = InitMenu_Special(hMenu, pMainMenu->nFunc);
				// リストが無い場合の処理
				if (!bInList) {
					// 分割線に囲まれ、かつリストなし ならば 次の分割線をスキップ
					if ((i == nIdxStr + 1
						  || (pMenu->m_mainMenuTbl[i - 1].m_nType == MainMenuType::Separator 
							&& pMenu->m_mainMenuTbl[i - 1].m_nLevel == pMainMenu->m_nLevel))
						&& i + 1 < nIdxEnd
						&& pMenu->m_mainMenuTbl[i + 1].m_nType == MainMenuType::Separator 
						&& pMenu->m_mainMenuTbl[i + 1].m_nLevel == pMainMenu->m_nLevel) {
						++i;		// スキップ
					}
				}
				break;
			}
		}
		if (nLv > 0) {
			// レベルが戻っていない
			hMenu = hSubMenu[0];
		}
		// 子の無い設定SubMenuのDesable
		CheckFreeSubMenu(GetHwnd(), hMenu, uPos);
	}

//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる変更
//	if (m_pPrintPreview)	return;	//	印刷プレビューモードなら排除。（おそらく排除しなくてもいいと思うんだけど、念のため）

	// 機能が利用可能かどうか、チェック状態かどうかを一括チェック
	numMenuItems = ::GetMenuItemCount(hMenu);
	for (nPos=0; nPos<numMenuItems; ++nPos) {
		EFunctionCode	id = (EFunctionCode)::GetMenuItemID(hMenu, nPos);
		// 機能が利用可能か調べる
		//	Jan.  8, 2006 genta 機能が有効な場合には明示的に再設定しないようにする．
		if (!IsFuncEnable(GetDocument(), m_pShareData, id)) {
			UINT fuFlags = MF_BYCOMMAND | MF_GRAYED;
			::EnableMenuItem(hMenu, id, fuFlags);
		}

		// 機能がチェック状態か調べる
		if (IsFuncChecked(GetDocument(), m_pShareData, id)) {
			UINT fuFlags = MF_BYCOMMAND | MF_CHECKED;
			::CheckMenuItem(hMenu, id, fuFlags);
		}
		/* else {
			fuFlags = MF_BYCOMMAND | MF_UNCHECKED;
		}
		*/
	}

	return;
}



/*!	通常コマンド(Special以外)のメニューへの追加
*/
void EditWnd::InitMenu_Function(HMENU hMenu, EFunctionCode eFunc, const wchar_t* pszName, const wchar_t* pszKey)
{
	const wchar_t* psName = NULL;
	// メニューラベルの作成
	// カスタムメニュー
	if (eFunc == F_MENU_RBUTTON
	  || eFunc >= F_CUSTMENU_1 && eFunc <= F_CUSTMENU_24
	) {
		int j;
		// 右クリックメニュー
		if (eFunc == F_MENU_RBUTTON) {
			j = CUSTMENU_INDEX_FOR_RBUTTONUP;
		}else {
			j = eFunc - F_CUSTMENU_BASE;
		}

		int nFlag = MF_BYPOSITION | MF_STRING | MF_GRAYED;
		if (m_pShareData->m_common.m_customMenu.m_nCustMenuItemNumArr[j] > 0) {
			nFlag = MF_BYPOSITION | MF_STRING;
		}
		WCHAR buf[MAX_CUSTOM_MENU_NAME_LEN + 1];
		m_menuDrawer.MyAppendMenu(hMenu, nFlag,
			eFunc, GetDocument()->m_funcLookup.Custmenu2Name(j, buf, _countof(buf)), pszKey);
	}
	// マクロ
	else if (eFunc >= F_USERMACRO_0 && eFunc < F_USERMACRO_0 + MAX_CUSTMACRO) {
		MacroRec *mp = &m_pShareData->m_common.m_macro.m_macroTable[eFunc - F_USERMACRO_0];
		if (mp->IsEnabled()) {
			psName = to_wchar(mp->m_szName[0] ? mp->m_szName : mp->m_szFile);
			m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING,
				eFunc, psName, pszKey);
		}else {
			psName = L"-- undefined macro --";
			m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED,
				eFunc, psName, pszKey);
		}
	}
	// プラグインコマンド
	else if (eFunc >= F_PLUGCOMMAND_FIRST && eFunc < F_PLUGCOMMAND_LAST) {
		WCHAR szLabel[256];
		if (0 < JackManager::getInstance()->GetCommandName( eFunc, szLabel, _countof(szLabel) )) {
			m_menuDrawer.MyAppendMenu(
				hMenu, MF_BYPOSITION | MF_STRING,
				eFunc, szLabel, pszKey,
				TRUE, eFunc
			);
		}else {
			// not found
			psName = L"-- undefined plugin command --";
			m_menuDrawer.MyAppendMenu(
				hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED,
				eFunc, psName, pszKey
			);
		}
	}else {
		switch (eFunc) {
		case F_RECKEYMACRO:
		case F_SAVEKEYMACRO:
		case F_LOADKEYMACRO:
		case F_EXECKEYMACRO:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!m_pShareData->m_flags.m_bRecordingKeyMacro);
			break;
		case F_SPLIT_V:	
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				m_splitterWnd.GetAllSplitRows() == 1);
			break;
		case F_SPLIT_H:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				m_splitterWnd.GetAllSplitCols() == 1);
			break;
		case F_SPLIT_VH:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				m_splitterWnd.GetAllSplitRows() == 1 || m_splitterWnd.GetAllSplitCols() == 1);
			break;
		case F_TAB_CLOSEOTHER:
			SetMenuFuncSel(hMenu, eFunc, pszKey, m_pShareData->m_common.m_tabBar.m_bDispTabWnd);
			break;
		case F_TOPMOST:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				((DWORD)::GetWindowLongPtr(GetHwnd(), GWL_EXSTYLE) & WS_EX_TOPMOST) == 0);
			break;
		case F_BIND_WINDOW:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				(!m_pShareData->m_common.m_tabBar.m_bDispTabWnd 
				|| m_pShareData->m_common.m_tabBar.m_bDispTabWndMultiWin));
			break;
		case F_SHOWTOOLBAR:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!m_pShareData->m_common.m_window.m_bMenuIcon | !m_toolbar.GetToolbarHwnd());
			break;
		case F_SHOWFUNCKEY:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!m_pShareData->m_common.m_window.m_bMenuIcon | !m_funcKeyWnd.GetHwnd());
			break;
		case F_SHOWTAB:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!m_pShareData->m_common.m_window.m_bMenuIcon | !m_tabWnd.GetHwnd());
			break;
		case F_SHOWSTATUSBAR:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!m_pShareData->m_common.m_window.m_bMenuIcon | !m_statusBar.GetStatusHwnd());
			break;
		case F_SHOWMINIMAP:
			SetMenuFuncSel( hMenu, eFunc, pszKey, 
				!m_pShareData->m_common.m_window.m_bMenuIcon | !GetMiniMap().GetHwnd() );
			break;
		case F_TOGGLE_KEY_SEARCH:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!m_pShareData->m_common.m_window.m_bMenuIcon | !IsFuncChecked(GetDocument(), m_pShareData, F_TOGGLE_KEY_SEARCH));
			break;
		case F_WRAPWINDOWWIDTH:
			{
				LayoutInt ketas;
				WCHAR*	pszLabel;
				EditView::TOGGLE_WRAP_ACTION mode = GetActiveView().GetWrapMode(&ketas);
				if (mode == EditView::TGWRAP_NONE) {
					m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_WRAPWINDOWWIDTH , L"", pszKey);
				}else {
					WCHAR szBuf[60];
					pszLabel = szBuf;
					if (mode == EditView::TGWRAP_FULL) {
						auto_sprintf_s(
							szBuf,
							LSW(STR_WRAP_WIDTH_FULL),	// L"折り返し桁数: %d 桁（最大）",
							MAXLINEKETAS
						);
					}else if (mode == EditView::TGWRAP_WINDOW) {
						auto_sprintf_s(
							szBuf,
							LSW(STR_WRAP_WIDTH_WINDOW),	//L"折り返し桁数: %d 桁（右端）",
							GetActiveView().ViewColNumToWrapColNum(
								GetActiveView().GetTextArea().m_nViewColNum
							)
						);
					}else {
						auto_sprintf_s(
							szBuf,
							LSW(STR_WRAP_WIDTH_FIXED),	//L"折り返し桁数: %d 桁（指定）",
							GetDocument()->m_docType.GetDocumentAttribute().m_nMaxLineKetas
						);
					}
					m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_WRAPWINDOWWIDTH , pszLabel, pszKey);
				}
			}
			break;
		default:
			m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, eFunc, 
				pszName, pszKey);
			break;
		}
	}
}


/*!	Specialコマンドのメニューへの追加
*/
bool EditWnd::InitMenu_Special(HMENU hMenu, EFunctionCode eFunc)
{
	bool bInList = false;
	switch (eFunc) {
	case F_WINDOW_LIST:				// ウィンドウリスト
		{
			EditNode* pEditNodeArr;
			int nRowNum = AppNodeManager::getInstance()->GetOpenedWindowArr(&pEditNodeArr, TRUE);
			WinListMenu(hMenu, pEditNodeArr, nRowNum, false);
			bInList = (nRowNum > 0);
			delete [] pEditNodeArr;
		}
		break;
	case F_FILE_USED_RECENTLY:		// 最近使ったファイル
		// MRUリストのファイルのリストをメニューにする
		{
			//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
			const MRUFile mru;
			mru.CreateMenu(hMenu, &m_menuDrawer);	//	ファイルメニュー
			bInList = (mru.MenuLength() > 0);
		}
		break;
	case F_FOLDER_USED_RECENTLY:	// 最近使ったフォルダ
		// 最近使ったフォルダのメニューを作成
		{
			//@@@ 2001.12.26 YAZAKI OPENFOLDERリストは、MRUFolderにすべて依頼する
			const MRUFolder mruFolder;
			mruFolder.CreateMenu(hMenu, &m_menuDrawer);
			bInList = (mruFolder.MenuLength() > 0);
		}
		break;
	case F_CUSTMENU_LIST:			// カスタムメニューリスト
		WCHAR buf[MAX_CUSTOM_MENU_NAME_LEN + 1];
		//	右クリックメニュー
		if (m_pShareData->m_common.m_customMenu.m_nCustMenuItemNumArr[0] > 0) {
			 m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING,
				 F_MENU_RBUTTON, GetDocument()->m_funcLookup.Custmenu2Name(0, buf, _countof(buf)), L"");
			bInList = true;
		}
		// カスタムメニュー
		for (int j=1; j<MAX_CUSTOM_MENU; ++j) {
			if (m_pShareData->m_common.m_customMenu.m_nCustMenuItemNumArr[j] > 0) {
				 m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING,
			 		F_CUSTMENU_BASE + j, GetDocument()->m_funcLookup.Custmenu2Name(j, buf, _countof(buf)), L"" );
				bInList = true;
			}
		}
		break;
	case F_USERMACRO_LIST:			// 登録済みマクロリスト
		for (int j=0; j<MAX_CUSTMACRO; ++j) {
			MacroRec *mp = &m_pShareData->m_common.m_macro.m_macroTable[j];
			if (mp->IsEnabled()) {
				if (mp->m_szName[0]) {
					m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_USERMACRO_0 + j, mp->m_szName, _T(""));
				}else {
					m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_USERMACRO_0 + j, mp->m_szFile, _T(""));
				}
				bInList = true;
			}
		}
		break;
	case F_PLUGIN_LIST:				// プラグインコマンドリスト
		// プラグインコマンドを提供するプラグインを列挙する
		{
			const JackManager* pJackManager = JackManager::getInstance();
			const Plugin* prevPlugin = NULL;
			HMENU hMenuPlugin = 0;

			Plug::Array plugs = pJackManager->GetPlugs(PP_COMMAND);
			for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
				const Plugin* curPlugin = &(*it)->m_plugin;
				if (curPlugin != prevPlugin) {
					// プラグインが変わったらプラグインポップアップメニューを登録
					hMenuPlugin = ::CreatePopupMenu();
					m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPlugin, curPlugin->m_sName.c_str(), L"");
					prevPlugin = curPlugin;
				}

				// コマンドを登録
				m_menuDrawer.MyAppendMenu(hMenuPlugin, MF_BYPOSITION | MF_STRING,
					(*it)->GetFunctionCode(), to_tchar((*it)->m_sLabel.c_str()), _T(""),
					TRUE, (*it)->GetFunctionCode());
			}
			bInList = (prevPlugin != NULL);
		}
		break;
	}
	return bInList;
}


// メニューバーの無効化を検査	2010/6/18 Uchi
void EditWnd::CheckFreeSubMenu(HWND hWnd, HMENU hMenu, UINT uPos)
{
	int cMenuItems = ::GetMenuItemCount(hMenu);
	if (cMenuItems == 0) {
		// 下が無いので無効化
		::EnableMenuItem(::GetMenu(hWnd), uPos, MF_BYPOSITION | MF_GRAYED);
	}else {
		// 下位レベルを検索
		CheckFreeSubMenuSub(hMenu, 1);
	}
}

// メニューバーの無効化を検査	2010/6/18 Uchi
void EditWnd::CheckFreeSubMenuSub(HMENU hMenu, int nLv)
{
	int numMenuItems = ::GetMenuItemCount(hMenu);
	for (int nPos=0; nPos<numMenuItems; ++nPos) {
		HMENU hSubMenu = ::GetSubMenu(hMenu, nPos);
		if (hSubMenu) {
			if (::GetMenuItemCount(hSubMenu) == 0) {
				// 下が無いので無効化
				::EnableMenuItem(hMenu, nPos, MF_BYPOSITION | MF_GRAYED);
			}else {
				// 下位レベルを検索
				CheckFreeSubMenuSub(hSubMenu, nLv + 1);
			}
		}
	}
}


//	フラグにより表示文字列の選択をする。
//		2010/5/19	Uchi
void EditWnd::SetMenuFuncSel(HMENU hMenu, EFunctionCode nFunc, const WCHAR* sKey, bool flag)
{
	const WCHAR* sName = L"";
	for (int i=0; i<_countof(sFuncMenuName); ++i) {
		if (sFuncMenuName[i].eFunc == nFunc) {
			sName = flag ? LSW(sFuncMenuName[i].nNameId[0]) : LSW(sFuncMenuName[i].nNameId[1]);
		}
	}
	assert(auto_strlen(sName));

	m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, nFunc, sName, sKey);
}




STDMETHODIMP EditWnd::DragEnter( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect)
{
	if (!pDataObject || !pdwEffect) {
		return E_INVALIDARG;
	}

	// 右ボタンファイルドロップの場合だけ処理する
	if (!((MK_RBUTTON & dwKeyState) && IsDataAvailable(pDataObject, CF_HDROP))) {
		*pdwEffect = DROPEFFECT_NONE;
		return E_INVALIDARG;
	}

	// 印刷プレビューでは受け付けない
	if (m_pPrintPreview) {
		*pdwEffect = DROPEFFECT_NONE;
		return E_INVALIDARG;
	}

	*pdwEffect &= DROPEFFECT_LINK;
	return S_OK;
}

STDMETHODIMP EditWnd::DragOver(DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect)
{
	if (!pdwEffect)
		return E_INVALIDARG;
	
	*pdwEffect &= DROPEFFECT_LINK;
	return S_OK;
}

STDMETHODIMP EditWnd::DragLeave(void)
{
	return S_OK;
}

STDMETHODIMP EditWnd::Drop(LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect)
{
	if (!pDataObject || !pdwEffect)
		return E_INVALIDARG;

	// ファイルドロップをアクティブビューで処理する
	*pdwEffect &= DROPEFFECT_LINK;
	return GetActiveView().PostMyDropFiles(pDataObject);
}

// ファイルがドロップされた
void EditWnd::OnDropFiles(HDROP hDrop)
{
	POINT pt;
	::DragQueryPoint(hDrop, &pt);
	int cFiles = (int)::DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	// ファイルをドロップしたときは閉じて開く
	auto& csFile = m_pShareData->m_common.m_file;
	if (csFile.m_bDropFileAndClose) {
		cFiles = 1;
	}
	// 一度にドロップ可能なファイル数
	if (cFiles > csFile.m_nDropFileNumMax) {
		cFiles = csFile.m_nDropFileNumMax;
	}

	// アクティブにする	// 2009.08.20 ryoji 処理開始前に無条件でアクティブ化
	ActivateFrameWindow(GetHwnd());

	for (int i=0; i<cFiles; ++i) {
		// ファイルパス取得、解決。
		TCHAR		szFile[_MAX_PATH + 1];
		::DragQueryFile(hDrop, i, szFile, _countof(szFile));
		SakuraEnvironment::ResolvePath(szFile);

		// 指定ファイルが開かれているか調べる
		HWND hWndOwner;
		if (ShareData::getInstance()->IsPathOpened(szFile, &hWndOwner)) {
			::SendMessage(hWndOwner, MYWM_GETFILEINFO, 0, 0);
			EditInfo* pfi = (EditInfo*)&m_pShareData->m_workBuffer.m_EditInfo_MYWM_GETFILEINFO;
			// アクティブにする
			ActivateFrameWindow(hWndOwner);
			// MRUリストへの登録
			MRUFile mru;
			mru.Add(pfi);
		}else {
			// 変更フラグがオフで、ファイルを読み込んでいない場合
			//	2005.06.24 Moca
			if (GetDocument()->IsAcceptLoad()) {
				// ファイル読み込み
				LoadInfo loadInfo(szFile, CODE_AUTODETECT, false);
				GetDocument()->m_docFileOperation.FileLoad(&loadInfo);
			}else {
				// ファイルをドロップしたときは閉じて開く
				if (csFile.m_bDropFileAndClose) {
					// ファイル読み込み
					LoadInfo loadInfo(szFile, CODE_AUTODETECT, false);
					GetDocument()->m_docFileOperation.FileCloseOpen(loadInfo);
				}else {
					// 編集ウィンドウの上限チェック
					if (m_pShareData->m_nodes.m_nEditArrNum >= MAX_EDITWINDOWS) {	// 最大値修正	//@@@ 2003.05.31 MIK
						OkMessage(NULL, LS(STR_MAXWINDOW), MAX_EDITWINDOWS);
						return;
					}
					// 新たな編集ウィンドウを起動
					LoadInfo loadInfo;
					loadInfo.filePath = szFile;
					loadInfo.eCharCode = CODE_NONE;
					loadInfo.bViewMode = false;
					ControlTray::OpenNewEditor(
						G_AppInstance(),
						GetHwnd(),
						loadInfo
					);
				}
			}
		}
	}
	::DragFinish(hDrop);
	return;
}

/*! WM_TIMER 処理 
	@date 2007.04.03 ryoji 新規
	@date 2008.04.19 ryoji IDT_FIRST_IDLE での MYWM_FIRST_IDLE ポスト処理を追加
	@date 2013.06.09 novice コントロールプロセスへの MYWM_FIRST_IDLE ポスト処理を追加
*/
LRESULT EditWnd::OnTimer(WPARAM wParam, LPARAM lParam)
{
	// タイマー ID で処理を振り分ける
	switch (wParam) {
	case IDT_EDIT:
		OnEditTimer();
		break;
	case IDT_TOOLBAR:
		m_toolbar.OnToolbarTimer();
		break;
	case IDT_CAPTION:
		OnCaptionTimer();
		break;
	case IDT_SYSMENU:
		OnSysMenuTimer();
		break;
	case IDT_FIRST_IDLE:
		m_dlgFuncList.m_bEditWndReady = true;	// エディタ画面の準備完了
		AppNodeGroupHandle(0).PostMessageToAllEditors(MYWM_FIRST_IDLE, ::GetCurrentProcessId(), 0, NULL);	// プロセスの初回アイドリング通知	// 2008.04.19 ryoji
		::PostMessage(m_pShareData->m_handles.m_hwndTray, MYWM_FIRST_IDLE, (WPARAM)::GetCurrentProcessId(), (LPARAM)0);
		::KillTimer(m_hWnd, wParam);
		break;
	default:
		return 1L;
	}

	return 0L;
}


/*! キャプション更新用タイマーの処理
	@date 2007.04.03 ryoji 新規
*/
void EditWnd::OnCaptionTimer(void)
{
	// 編集画面の切替（タブまとめ時）が終わっていたらタイマーを終了してタイトルバーを更新する
	// まだ切替中ならタイマー継続
	if (!m_pShareData->m_flags.m_bEditWndChanging) {
		::KillTimer(GetHwnd(), IDT_CAPTION);
		::SetWindowText(GetHwnd(), m_pszLastCaption);
	}
}

/*! システムメニュー表示用タイマーの処理
	@date 2007.04.03 ryoji パラメータ無しにした
	                       以前はコールバック関数でやっていたKillTimer()をここで行うようにした
*/
void EditWnd::OnSysMenuTimer(void) // by 鬼(2)
{
	::KillTimer(GetHwnd(), IDT_SYSMENU);	// 2007.04.03 ryoji

	if (m_iconClicked == IconClickStatus::Clicked) {
		ReleaseCapture();

		// システムメニュー表示
		// 2006.04.21 ryoji マルチモニタ対応の修正
		// 2007.05.13 ryoji 0x0313メッセージをポストする方式に変更（TrackPopupMenuだとメニュー項目の有効／無効状態が不正になる問題対策）
		RECT R;
		GetWindowRect(GetHwnd(), &R);
		POINT pt;
		pt.x = R.left + GetSystemMetrics(SM_CXFRAME);
		pt.y = R.top + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
		GetMonitorWorkRect(pt, &R);
		::PostMessage(
			GetHwnd(),
			0x0313, // 右クリックでシステムメニューを表示する際に送信するモノらしい
			0,
			MAKELPARAM((pt.x > R.left)? pt.x: R.left, (pt.y < R.bottom)? pt.y: R.bottom)
		);
	}
	m_iconClicked = IconClickStatus::None;
}


//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる変更

// 印刷プレビューモードのオン/オフ
void EditWnd::PrintPreviewModeONOFF(void)
{
	// 2006.06.17 ryoji Rebar があればそれをツールバー扱いする
	HWND hwndToolBar = m_toolbar.GetRebarHwnd() ? m_toolbar.GetRebarHwnd(): m_toolbar.GetToolbarHwnd();

	// 印刷プレビューモードか
//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる変更
	if (m_pPrintPreview) {
//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる変更
		// 印刷プレビューモードを解除します。
		delete m_pPrintPreview;	//	削除。
		m_pPrintPreview = NULL;	//	NULLか否かで、プリントプレビューモードか判断するため。

		// 通常モードに戻す
		::ShowWindow(this->m_splitterWnd.GetHwnd(), SW_SHOW);
		::ShowWindow(hwndToolBar, SW_SHOW);	// 2006.06.17 ryoji
		::ShowWindow(m_statusBar.GetStatusHwnd(), SW_SHOW);
		::ShowWindow(m_funcKeyWnd.GetHwnd(), SW_SHOW);
		::ShowWindow(m_tabWnd.GetHwnd(), SW_SHOW);	//@@@ 2003.06.25 MIK
		::ShowWindow(m_dlgFuncList.GetHwnd(), SW_SHOW);	// 2010.06.25 ryoji

		// その他のモードレスダイアログも戻す	// 2010.06.25 ryoji
		::ShowWindow(m_dlgFind.GetHwnd(), SW_SHOW);
		::ShowWindow(m_dlgReplace.GetHwnd(), SW_SHOW);
		::ShowWindow(m_dlgGrep.GetHwnd(), SW_SHOW);

		::SetFocus(GetHwnd());

		// メニューを動的に作成するように変更
		//hMenu = ::LoadMenu(G_AppInstance(), MAKEINTRESOURCE(IDR_MENU1));
		//::SetMenu(GetHwnd(), hMenu);
		//::DrawMenuBar(GetHwnd());
		LayoutMainMenu();				// 2010/5/16 Uchi

//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる変更
		::InvalidateRect(GetHwnd(), NULL, TRUE);
	}else {
//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる変更
		// 通常モードを隠す
		HMENU hMenu = ::GetMenu(GetHwnd());
		//	Jun. 18, 2001 genta Print Previewではメニューを削除
		::SetMenu(GetHwnd(), NULL);
		::DestroyMenu(hMenu);
		::DrawMenuBar(GetHwnd());

		::ShowWindow(this->m_splitterWnd.GetHwnd(), SW_HIDE);
		::ShowWindow(hwndToolBar, SW_HIDE);	// 2006.06.17 ryoji
		::ShowWindow(m_statusBar.GetStatusHwnd(), SW_HIDE);
		::ShowWindow(m_funcKeyWnd.GetHwnd(), SW_HIDE);
		::ShowWindow(m_tabWnd.GetHwnd(), SW_HIDE);	//@@@ 2003.06.25 MIK
		::ShowWindow(m_dlgFuncList.GetHwnd(), SW_HIDE);	// 2010.06.25 ryoji

		// その他のモードレスダイアログも隠す	// 2010.06.25 ryoji
		::ShowWindow(m_dlgFind.GetHwnd(), SW_HIDE);
		::ShowWindow(m_dlgReplace.GetHwnd(), SW_HIDE);
		::ShowWindow(m_dlgGrep.GetHwnd(), SW_HIDE);

//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる変更
		m_pPrintPreview = new PrintPreview(this);
		// 現在の印刷設定
		m_pPrintPreview->SetPrintSetting(
			&m_pShareData->m_printSettingArr[
				GetDocument()->m_docType.GetDocumentAttribute().m_nCurrentPrintSetting]
		);

		// プリンタの情報を取得。

		// 現在のデフォルトプリンタの情報を取得
		BOOL bRes = m_pPrintPreview->GetDefaultPrinterInfo();
		if (!bRes) {
			TopInfoMessage(GetHwnd(), LS(STR_ERR_DLGEDITWND14));
			return;
		}

		// 印刷設定の反映
//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる変更
		m_pPrintPreview->OnChangePrintSetting();
		::InvalidateRect(GetHwnd(), NULL, TRUE);
		::UpdateWindow(GetHwnd() /* m_pPrintPreview->GetPrintPreviewBarHANDLE() */);

	}
	return;

}


// WM_SIZE 処理
LRESULT EditWnd::OnSize(WPARAM wParam, LPARAM lParam)
{
	return OnSize2(wParam, lParam, true);
}

LRESULT EditWnd::OnSize2( WPARAM wParam, LPARAM lParam, bool bUpdateStatus )
{
	RECT rc;
//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる
//	変数削除

	int cx = LOWORD(lParam);
	int cy = HIWORD(lParam);
	auto& csWindow = m_pShareData->m_common.m_window;
	
	// ウィンドウサイズ継承
	if (wParam != SIZE_MINIMIZED) {						// 最小化は継承しない
		//	2004.05.13 Moca m_eSaveWindowSizeの解釈追加のため
		if (csWindow.m_eSaveWindowSize == WinSizeMode::Save) {		// ウィンドウサイズ継承をするか
			if (wParam == SIZE_MAXIMIZED) {					// 最大化はサイズを記録しない
				if (csWindow.m_nWinSizeType != (int)wParam) {
					csWindow.m_nWinSizeType = wParam;
				}
			}else {
				// Aero Snapの縦方向最大化状態で終了して次回起動するときは元のサイズにする必要があるので、
				// GetWindowRect()ではなくGetWindowPlacement()で得たワークエリア座標をスクリーン座標に変換して記憶する	// 2009.09.02 ryoji
				WINDOWPLACEMENT wp;
				wp.length = sizeof(wp);
				::GetWindowPlacement(GetHwnd(), &wp);	// ワークエリア座標
				RECT rcWin = wp.rcNormalPosition;
				RECT rcWork, rcMon;
				GetMonitorWorkRect(GetHwnd(), &rcWork, &rcMon);
				::OffsetRect(&rcWin, rcWork.left - rcMon.left, rcWork.top - rcMon.top);	// スクリーン座標に変換
				// ウィンドウサイズに関するデータが変更されたか
				if (csWindow.m_nWinSizeType != (int)wParam ||
					csWindow.m_nWinSizeCX != rcWin.right - rcWin.left ||
					csWindow.m_nWinSizeCY != rcWin.bottom - rcWin.top
				) {
					csWindow.m_nWinSizeType = wParam;
					csWindow.m_nWinSizeCX = rcWin.right - rcWin.left;
					csWindow.m_nWinSizeCY = rcWin.bottom - rcWin.top;
				}
			}
		}

		// 元に戻すときのサイズ種別を記憶	// 2007.06.20 ryoji
		EditNode* p = AppNodeManager::getInstance()->GetEditNode(GetHwnd());
		if (p) {
			p->m_showCmdRestore = ::IsZoomed(p->GetHwnd())? SW_SHOWMAXIMIZED: SW_SHOWNORMAL;
		}
	}

	m_nWinSizeType = wParam;	// サイズ変更のタイプ

	// 2006.06.17 ryoji Rebar があればそれをツールバー扱いする
	HWND hwndToolBar = m_toolbar.GetRebarHwnd() ? m_toolbar.GetRebarHwnd(): m_toolbar.GetToolbarHwnd();
	int nToolBarHeight = 0;
	if (hwndToolBar) {
		::SendMessage(hwndToolBar, WM_SIZE, wParam, lParam);
		::GetWindowRect(hwndToolBar, &rc);
		nToolBarHeight = rc.bottom - rc.top;
	}
	int nFuncKeyWndHeight = 0;
	if (m_funcKeyWnd.GetHwnd()) {
		::SendMessage(m_funcKeyWnd.GetHwnd(), WM_SIZE, wParam, lParam);
		::GetWindowRect(m_funcKeyWnd.GetHwnd(), &rc);
		nFuncKeyWndHeight = rc.bottom - rc.top;
	}
	//@@@ From Here 2003.05.31 MIK
	//@@@ To Here 2003.05.31 MIK
	bool bMiniMapSizeBox = true;
	if (wParam == SIZE_MAXIMIZED) {
		bMiniMapSizeBox = false;
	}
	int nStatusBarHeight = 0;
	if (m_statusBar.GetStatusHwnd()) {
		::SendMessage(m_statusBar.GetStatusHwnd(), WM_SIZE, wParam, lParam);
		::GetClientRect(m_statusBar.GetStatusHwnd(), &rc);
		//	May 12, 2000 genta
		//	2カラム目に改行コードの表示を挿入
		//	From Here
		int			nStArr[8];
		// 2003.08.26 Moca CR0LF0廃止に従い、適当に調整
		// 2004-02-28 yasu 文字列を出力時の書式に合わせる
		// 幅を変えた場合にはEditView::ShowCaretPosInfo()での表示方法を見直す必要あり．
		// ※pszLabel[3]: ステータスバー文字コード表示領域は大きめにとっておく
		const TCHAR*	pszLabel[7] = { _T(""), _T("99999 行 9999 列"), _T("CRLF"), _T("AAAAAAAAAAAA"), _T("Unicode BOM付"), _T("REC"), _T("上書") };	// Oct. 30, 2000 JEPRO 千万行も要らん	文字コード枠を広げる 2008/6/21	Uchi
		int			nStArrNum = 7;
		//	To Here
		int			nAllWidth = rc.right - rc.left;
		int			nSbxWidth = ::GetSystemMetrics(SM_CXVSCROLL) + ::GetSystemMetrics(SM_CXEDGE); // サイズボックスの幅
		int			nBdrWidth = ::GetSystemMetrics(SM_CXSIZEFRAME) + ::GetSystemMetrics(SM_CXEDGE) * 2; // 境界の幅
		SIZE		sz;
		HDC			hdc;
		int			i;
		// 2004-02-28 yasu
		// 正確な幅を計算するために、表示フォントを取得してhdcに選択させる。
		hdc = ::GetDC(m_statusBar.GetStatusHwnd());
		HFONT hFont = (HFONT)::SendMessage(m_statusBar.GetStatusHwnd(), WM_GETFONT, 0, 0);
		if (hFont) {
			hFont = (HFONT)::SelectObject(hdc, hFont);
		}
		nStArr[nStArrNum - 1] = nAllWidth;
		if (wParam != SIZE_MAXIMIZED) {
			nStArr[nStArrNum - 1] -= nSbxWidth;
		}
		for (i=nStArrNum-1; i>0; --i) {
			::GetTextExtentPoint32(hdc, pszLabel[i], _tcslen(pszLabel[i]), &sz);
			nStArr[i - 1] = nStArr[i] - (sz.cx + nBdrWidth);
		}

		//	Nov. 8, 2003 genta
		//	初期状態ではすべての部分が「枠あり」だが，メッセージエリアは枠を描画しないようにしている
		//	ため，初期化時の枠が変な風に残ってしまう．初期状態で枠を描画させなくするため，
		//	最初に「枠無し」状態を設定した後でバーの分割を行う．
		if (bUpdateStatus) {
			m_statusBar.SetStatusText(0, SBT_NOBORDERS, _T(""));
		}

		StatusBar_SetParts(m_statusBar.GetStatusHwnd(), nStArrNum, nStArr);
		if (hFont) {
			::SelectObject(hdc, hFont);
		}
		::ReleaseDC(m_statusBar.GetStatusHwnd(), hdc);

		::UpdateWindow(m_statusBar.GetStatusHwnd());	// 2006.06.17 ryoji 即時描画でちらつきを減らす
		::GetWindowRect(m_statusBar.GetStatusHwnd(), &rc);
		nStatusBarHeight = rc.bottom - rc.top;
		bMiniMapSizeBox = false;
	}
	RECT rcClient;
	::GetClientRect(GetHwnd(), &rcClient);

	//@@@ From 2003.05.31 MIK
	// タブウィンドウ追加に伴い，ファンクションキー表示位置も調整

	// タブウィンドウ
	int nTabHeightBottom = 0;
	int nTabWndHeight = 0;		//タブウィンドウ	//@@@ 2003.05.31 MIK
	if (m_tabWnd.GetHwnd()) {
		// タブ多段はSizeBox/ウィンドウ幅で高さが変わる可能性がある
		TabPosition tabPosition = m_pShareData->m_common.m_tabBar.m_eTabPosition;
		bool bHidden = false;
		if (tabPosition == TabPosition::Top) {
			// 上から下に移動するとゴミが表示されるので一度非表示にする
			if (m_tabWnd.m_eTabPosition != TabPosition::None && m_tabWnd.m_eTabPosition != TabPosition::Top) {
				bHidden = true;
				::ShowWindow( m_tabWnd.GetHwnd(), SW_HIDE );
			}
			m_tabWnd.SizeBox_ONOFF( false );
			::GetWindowRect( m_tabWnd.GetHwnd(), &rc );
			nTabWndHeight = rc.bottom - rc.top;
			if (csWindow.m_nFUNCKEYWND_Place == 0) {
				::MoveWindow(m_tabWnd.GetHwnd(), 0, nToolBarHeight + nFuncKeyWndHeight, cx, nTabWndHeight, TRUE);
			}else {
				::MoveWindow(m_tabWnd.GetHwnd(), 0, nToolBarHeight, cx, nTabWndHeight, TRUE);
			}
			m_tabWnd.OnSize();
			::GetWindowRect( m_tabWnd.GetHwnd(), &rc );
			if (nTabWndHeight != rc.bottom - rc.top) {
				nTabWndHeight = rc.bottom - rc.top;
				if (csWindow.m_nFUNCKEYWND_Place == 0) {
					::MoveWindow( m_tabWnd.GetHwnd(), 0, nToolBarHeight + nFuncKeyWndHeight, cx, nTabWndHeight, TRUE );
				}else {
					::MoveWindow( m_tabWnd.GetHwnd(), 0, nToolBarHeight, cx, nTabWndHeight, TRUE );
				}
			}
		}else if (tabPosition == TabPosition::Bottom) {
			// 上から下に移動するとゴミが表示されるので一度非表示にする
			if (m_tabWnd.m_eTabPosition != TabPosition::None && m_tabWnd.m_eTabPosition != TabPosition::Bottom) {
				bHidden = true;
				ShowWindow( m_tabWnd.GetHwnd(), SW_HIDE );
			}
			bool bSizeBox = true;
			if (m_statusBar.GetStatusHwnd()) {
				bSizeBox = false;
			}
			if (m_funcKeyWnd.GetHwnd()) {
				if (csWindow.m_nFUNCKEYWND_Place == 1 ){
					bSizeBox = false;
				}
			}
			if (wParam == SIZE_MAXIMIZED) {
				bSizeBox = false;
			}
			m_tabWnd.SizeBox_ONOFF( bSizeBox );
			::GetWindowRect( m_tabWnd.GetHwnd(), &rc );
			nTabWndHeight = rc.bottom - rc.top;
			::MoveWindow( m_tabWnd.GetHwnd(), 0,
				cy - nFuncKeyWndHeight - nStatusBarHeight - nTabWndHeight, cx, nTabWndHeight, TRUE );
			m_tabWnd.OnSize();
			::GetWindowRect( m_tabWnd.GetHwnd(), &rc );
			if (nTabWndHeight != rc.bottom - rc.top) {
				nTabWndHeight = rc.bottom - rc.top;
				::MoveWindow( m_tabWnd.GetHwnd(), 0,
					cy - nFuncKeyWndHeight - nStatusBarHeight - nTabWndHeight, cx, nTabWndHeight, TRUE );
			}
			nTabHeightBottom = rc.bottom - rc.top;
			nTabWndHeight = 0;
			bMiniMapSizeBox = false;
		}
		if (bHidden) {
			::ShowWindow( m_tabWnd.GetHwnd(), SW_SHOW );
		}
		m_tabWnd.m_eTabPosition = tabPosition;
	}

	//	2005.04.23 genta ファンクションキー非表示の時は移動しない
	if (m_funcKeyWnd.GetHwnd()) {
		if (csWindow.m_nFUNCKEYWND_Place == 0) {
			// ファンクションキー表示位置／0:上 1:下
			::MoveWindow(
				m_funcKeyWnd.GetHwnd(),
				0,
				nToolBarHeight,
				cx,
				nFuncKeyWndHeight, TRUE);
		}else if (csWindow.m_nFUNCKEYWND_Place == 1) {
			// ファンクションキー表示位置／0:上 1:下
			::MoveWindow(
				m_funcKeyWnd.GetHwnd(),
				0,
				cy - nFuncKeyWndHeight - nStatusBarHeight,
				cx,
				nFuncKeyWndHeight, TRUE
			);

			bool bSizeBox = true;
			if (m_statusBar.GetStatusHwnd()) {
				bSizeBox = false;
			}
			if (wParam == SIZE_MAXIMIZED) {
				bSizeBox = false;
			}
			m_funcKeyWnd.SizeBox_ONOFF(bSizeBox);
			bMiniMapSizeBox = false;
		}
		::UpdateWindow(m_funcKeyWnd.GetHwnd());	// 2006.06.17 ryoji 即時描画でちらつきを減らす
	}

	int nFuncListWidth = 0;
	int nFuncListHeight = 0;
	if (m_dlgFuncList.GetHwnd() && m_dlgFuncList.IsDocking()) {
		::SendMessage(m_dlgFuncList.GetHwnd(), WM_SIZE, wParam, lParam);
		::GetWindowRect(m_dlgFuncList.GetHwnd(), &rc);
		nFuncListWidth = rc.right - rc.left;
		nFuncListHeight = rc.bottom - rc.top;
	}

	DockSideType eDockSideFL = m_dlgFuncList.GetDockSide();
	int nTop = nToolBarHeight + nTabWndHeight;
	if (csWindow.m_nFUNCKEYWND_Place == 0)
		nTop += nFuncKeyWndHeight;
	int nHeight = cy - nToolBarHeight - nFuncKeyWndHeight - nTabWndHeight - nTabHeightBottom - nStatusBarHeight;
	if (m_dlgFuncList.GetHwnd() && m_dlgFuncList.IsDocking()) {
		::MoveWindow(
			m_dlgFuncList.GetHwnd(),
			(eDockSideFL == DockSideType::Right)? cx - nFuncListWidth: 0,
			(eDockSideFL == DockSideType::Bottom)? nTop + nHeight - nFuncListHeight: nTop,
			(eDockSideFL == DockSideType::Left || eDockSideFL == DockSideType::Right)? nFuncListWidth: cx,
			(eDockSideFL == DockSideType::Top || eDockSideFL == DockSideType::Bottom)? nFuncListHeight: nHeight,
			TRUE
		);
		if (eDockSideFL == DockSideType::Right || eDockSideFL == DockSideType::Bottom) {
			bMiniMapSizeBox = false;
		}
	}

	// ミニマップ
	int nMiniMapWidth = 0;
	if (GetMiniMap().GetHwnd()) {
		nMiniMapWidth = GetDllShareData().m_common.m_window.m_nMiniMapWidth;
		::MoveWindow( m_pEditViewMiniMap->GetHwnd(), 
			(eDockSideFL == DockSideType::Right)? cx - nFuncListWidth - nMiniMapWidth: cx - nMiniMapWidth,
			(eDockSideFL == DockSideType::Top)? nTop + nFuncListHeight: nTop,
			nMiniMapWidth,
			(eDockSideFL == DockSideType::Top || eDockSideFL == DockSideType::Bottom)? nHeight - nFuncListHeight: nHeight,
			TRUE
		);
		GetMiniMap().SplitBoxOnOff(false, false, bMiniMapSizeBox);
	}

	::MoveWindow(
		m_splitterWnd.GetHwnd(),
		(eDockSideFL == DockSideType::Left)? nFuncListWidth: 0,
		(eDockSideFL == DockSideType::Top)? nTop + nFuncListHeight: nTop,	//@@@ 2003.05.31 MIK
		((eDockSideFL == DockSideType::Left || eDockSideFL == DockSideType::Right)? cx - nFuncListWidth: cx) - nMiniMapWidth,
		(eDockSideFL == DockSideType::Top || eDockSideFL == DockSideType::Bottom)? nHeight - nFuncListHeight: nHeight,	//@@@ 2003.05.31 MIK
		TRUE
	);
	//@@@ To 2003.05.31 MIK

	// 印刷プレビューモードか
//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる変更
	if (!m_pPrintPreview) {
		return 0L;
	}
	return m_pPrintPreview->OnSize(wParam, lParam);
}


// WM_PAINT 描画処理
LRESULT EditWnd::OnPaint(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる変更
	// 印刷プレビューモードか
	if (!m_pPrintPreview) {
		PAINTSTRUCT ps;
		::BeginPaint(hwnd, &ps);
		::EndPaint(hwnd, &ps);
		return 0L;
	}
//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる変更
	return m_pPrintPreview->OnPaint(hwnd, uMsg, wParam, lParam);
}

// 印刷プレビュー 垂直スクロールバーメッセージ処理 WM_VSCROLL
LRESULT EditWnd::OnVScroll(WPARAM wParam, LPARAM lParam)
{
	// 印刷プレビューモードか
	if (!m_pPrintPreview) {
		return 0;
	}
//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる変更
	return m_pPrintPreview->OnVScroll(wParam, lParam);
}

// 印刷プレビュー 水平スクロールバーメッセージ処理
LRESULT EditWnd::OnHScroll(WPARAM wParam, LPARAM lParam)
{
//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる変更
	// 印刷プレビューモードか
	if (!m_pPrintPreview) {
		return 0;
	}
	return m_pPrintPreview->OnHScroll(wParam, lParam);
}

LRESULT EditWnd::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	// by 鬼(2) キャプチャーして押されたら非クライアントでもこっちに来る
	if (m_iconClicked != IconClickStatus::None)
		return 0;

	m_ptDragPosOrg.x = LOWORD(lParam);	// horizontal position of cursor
	m_ptDragPosOrg.y = HIWORD(lParam);	// vertical position of cursor
	m_bDragMode = true;
	SetCapture(GetHwnd());

	return 0;
}

LRESULT EditWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	// by 鬼 2002/04/18
	if (m_iconClicked != IconClickStatus::None) {
		if (m_iconClicked == IconClickStatus::Down) {
			m_iconClicked = IconClickStatus::Clicked;
			// by 鬼(2) タイマー(IDは適当です)
			SetTimer(GetHwnd(), IDT_SYSMENU, GetDoubleClickTime(), NULL);
		}
		return 0;
	}

	m_bDragMode = false;
//	MYTRACE(_T("m_bDragMode = FALSE (OnLButtonUp)\n"));
	ReleaseCapture();
	::InvalidateRect(GetHwnd(), NULL, TRUE);
	return 0;
}


/*!	WM_MOUSEMOVE処理
	@date 2008.05.05 novice メモリリーク修正
*/
LRESULT EditWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	// by 鬼
	if (m_iconClicked != IconClickStatus::None) {
		// by 鬼(2) 一回押された時だけ
		if (m_iconClicked == IconClickStatus::Down) {
			POINT P;
			GetCursorPos(&P); // スクリーン座標
			if (SendMessage(GetHwnd(), WM_NCHITTEST, 0, P.x | (P.y << 16)) != HTSYSMENU) {
				ReleaseCapture();
				m_iconClicked = IconClickStatus::None;

				if (GetDocument()->m_docFile.GetFilePathClass().IsValidPath()) {
					// 2010.08.22 Moca C:\temp.txt などのtopのファイルがD&Dできないバグの修正
					NativeW cmemTitle;
					NativeW cmemDir;
					cmemTitle = to_wchar(GetDocument()->m_docFile.GetFileName());
					cmemDir   = to_wchar(GetDocument()->m_docFile.GetFilePathClass().GetDirPath().c_str());

					IDataObject *DataObject;
					IMalloc *Malloc;
					IShellFolder *Desktop, *Folder;
					LPITEMIDLIST PathID, ItemID;
					SHGetMalloc(&Malloc);
					SHGetDesktopFolder(&Desktop);
					DWORD Eaten, Attribs;
					if (SUCCEEDED(Desktop->ParseDisplayName(0, NULL, cmemDir.GetStringPtr(), &Eaten, &PathID, &Attribs))) {
						Desktop->BindToObject(PathID, NULL, IID_IShellFolder, (void**)&Folder);
						Malloc->Free(PathID);
						if (SUCCEEDED(Folder->ParseDisplayName(0, NULL, cmemTitle.GetStringPtr(), &Eaten, &ItemID, &Attribs))) {
							LPCITEMIDLIST List[1];
							List[0] = ItemID;
							Folder->GetUIObjectOf(0, 1, List, IID_IDataObject, NULL, (void**)&DataObject);
							Malloc->Free(ItemID);
#define DDASTEXT
#ifdef  DDASTEXT
							// テキストでも持たせる…便利
							{
								FORMATETC F;
								F.cfFormat = CF_UNICODETEXT;
								F.ptd      = NULL;
								F.dwAspect = DVASPECT_CONTENT;
								F.lindex   = -1;
								F.tymed    = TYMED_HGLOBAL;

								STGMEDIUM M;
								const wchar_t* pFilePath = to_wchar(GetDocument()->m_docFile.GetFilePath());
								int Len = wcslen(pFilePath);
								M.tymed          = TYMED_HGLOBAL;
								M.pUnkForRelease = NULL;
								M.hGlobal        = GlobalAlloc(GMEM_MOVEABLE, (Len + 1) * sizeof(wchar_t));
								void* p = GlobalLock(M.hGlobal);
								CopyMemory(p, pFilePath, (Len + 1) * sizeof(wchar_t));
								GlobalUnlock(M.hGlobal);

								DataObject->SetData(&F, &M, TRUE);
							}
#endif
							// 移動は禁止
							DWORD R;
							DropSource drop(TRUE);
							DoDragDrop(DataObject, &drop, DROPEFFECT_COPY | DROPEFFECT_LINK, &R);
							DataObject->Release();
						}
						Folder->Release();
					}
					Desktop->Release();
					Malloc->Release();
				}
			}
		}
		return 0;
	}

//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる変更
	if (!m_pPrintPreview) {
		return 0;
	}else {
		return m_pPrintPreview->OnMouseMove(wParam, lParam);
	}
}


LRESULT EditWnd::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
	if (m_pPrintPreview) {
		return m_pPrintPreview->OnMouseWheel(wParam, lParam);
	}
	return Views_DispatchEvent(GetHwnd(), WM_MOUSEWHEEL, wParam, lParam);
}

/** マウスホイール処理

	@date 2007.10.16 ryoji OnMouseWheel()から処理抜き出し
*/
BOOL EditWnd::DoMouseWheel(WPARAM wParam, LPARAM lParam)
{
//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる変更
	// 印刷プレビューモードか
	if (!m_pPrintPreview) {
		// 2006.03.26 ryoji by assitance with John タブ上ならウィンドウ切り替え
		if (m_pShareData->m_common.m_tabBar.m_bChgWndByWheel && m_tabWnd.m_hwndTab) {
			POINT pt;
			pt.x = (short)LOWORD(lParam);
			pt.y = (short)HIWORD(lParam);
			int nDelta = (short)HIWORD(wParam);
			HWND hwnd = ::WindowFromPoint(pt);
			if ((hwnd == m_tabWnd.m_hwndTab || hwnd == m_tabWnd.GetHwnd())) {
				// 現在開いている編集窓のリストを得る
				EditNode* pEditNodeArr;
				int nRowNum = AppNodeManager::getInstance()->GetOpenedWindowArr(&pEditNodeArr, TRUE);
				if (nRowNum > 0) {
					// 自分のウィンドウを調べる
					int i, j;
					int nGroup = 0;
					for (i=0; i<nRowNum; ++i) {
						if (GetHwnd() == pEditNodeArr[i].GetHwnd()) {
							nGroup = pEditNodeArr[i].m_nGroup;
							break;
						}
					}
					if (i < nRowNum) {
						if (nDelta < 0) {
							// 次のウィンドウ
							for (j=i+1; j<nRowNum; ++j) {
								if (nGroup == pEditNodeArr[j].m_nGroup)
									break;
							}
							if (j >= nRowNum) {
								for (j=0; j<i; ++j) {
									if (nGroup == pEditNodeArr[j].m_nGroup)
										break;
								}
							}
						}else {
							// 前のウィンドウ
							for (j=i-1; j>=0; --j) {
								if (nGroup == pEditNodeArr[j].m_nGroup)
									break;
							}
							if (j < 0) {
								for (j=nRowNum-1; j>i; --j) {
									if (nGroup == pEditNodeArr[j].m_nGroup)
										break;
								}
							}
						}

						// 次の（or 前の）ウィンドウをアクティブにする
						if (i != j)
							ActivateFrameWindow(pEditNodeArr[j].GetHwnd());
					}
					delete []pEditNodeArr;
				}
				return TRUE;	// 処理した
			}
		}
		return FALSE;	// 処理しなかった
	}
	return FALSE;	// 処理しなかった
}

/* 印刷ページ設定
	印刷プレビュー時にも、そうでないときでも呼ばれる可能性がある。
*/
BOOL EditWnd::OnPrintPageSetting(void)
{
	// 印刷設定（CANCEL押したときに破棄するための領域）
	DlgPrintSetting	dlgPrintSetting;
	BOOL				bRes;
	int					nCurrentPrintSetting;
	int					nLineNumberColumns;

	nCurrentPrintSetting = GetDocument()->m_docType.GetDocumentAttribute().m_nCurrentPrintSetting;
	if (m_pPrintPreview) {
		nLineNumberColumns = GetActiveView().GetTextArea().DetectWidthOfLineNumberArea_calculate(m_pPrintPreview->m_pLayoutMgr_Print); // 印刷プレビュー時は文書の桁数 2013.5.10 aroka
	}else {
		nLineNumberColumns = 3; // ファイルメニューからの設定時は最小値 2013.5.10 aroka
	}

	bRes = dlgPrintSetting.DoModal(
		G_AppInstance(),
//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる変更
		GetHwnd(),
		&nCurrentPrintSetting, // 現在選択している印刷設定
		m_pShareData->m_printSettingArr, // 現在の設定はダイアログ側で保持する 2013.5.1 aroka
		nLineNumberColumns // 行番号表示用に桁数を渡す 2013.5.10 aroka
	);

	if (bRes) {
		bool bChangePrintSettingNo = false;
		// 現在選択されているページ設定の番号が変更されたか
		if (GetDocument()->m_docType.GetDocumentAttribute().m_nCurrentPrintSetting != nCurrentPrintSetting) {
			// 変更フラグ(タイプ別設定)
			TypeConfig* type = new TypeConfig();
			DocTypeManager().GetTypeConfig(GetDocument()->m_docType.GetDocumentType(), *type);
			type->m_nCurrentPrintSetting = nCurrentPrintSetting;
			DocTypeManager().SetTypeConfig(GetDocument()->m_docType.GetDocumentType(), *type);
			delete type;
			GetDocument()->m_docType.GetDocumentAttributeWrite().m_nCurrentPrintSetting = nCurrentPrintSetting; // 今の設定にも反映
			AppNodeGroupHandle(0).SendMessageToAllEditors(
				MYWM_CHANGESETTING,
				(WPARAM)GetDocument()->m_docType.GetDocumentType().GetIndex(),
				(LPARAM)PM_CHANGESETTING_TYPE,
				EditWnd::getInstance()->GetHwnd()
			);
			bChangePrintSettingNo = true;
		}

//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる変更
		//	印刷プレビュー時のみ。
		if (m_pPrintPreview) {
			// 現在の印刷設定
			// 2013.08.27 印刷設定番号が変更された時に対応できていなかった
			if (bChangePrintSettingNo) {
				m_pPrintPreview->SetPrintSetting(&m_pShareData->m_printSettingArr[GetDocument()->m_docType.GetDocumentAttribute().m_nCurrentPrintSetting]);
			}

			// 印刷プレビュー スクロールバー初期化
			//m_pPrintPreview->InitPreviewScrollBar();

			// 印刷設定の反映
			// m_pPrintPreview->OnChangePrintSetting();

			//::InvalidateRect(GetHwnd(), NULL, TRUE);
		}
		AppNodeGroupHandle(0).SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0,
			(LPARAM)PM_PRINTSETTING,
			EditWnd::getInstance()->GetHwnd()
		);
	}
//@@@ 2002.01.14 YAZAKI 印刷プレビューをPrintPreviewに独立させたことによる変更
	::UpdateWindow(GetHwnd() /* m_pPrintPreview->GetPrintPreviewBarHANDLE() */);
	return bRes;
}

///////////////////////////// by 鬼

LRESULT EditWnd::OnNcLButtonDown(WPARAM wp, LPARAM lp)
{
	LRESULT Result;
	if (wp == HTSYSMENU) {
		SetCapture(GetHwnd());
		m_iconClicked = IconClickStatus::Down;
		Result = 0;
	}else
		Result = DefWindowProc(GetHwnd(), WM_NCLBUTTONDOWN, wp, lp);

	return Result;
}

LRESULT EditWnd::OnNcLButtonUp(WPARAM wp, LPARAM lp)
{
	LRESULT Result;
	if (m_iconClicked != IconClickStatus::None) {
		// 念のため
		ReleaseCapture();
		m_iconClicked = IconClickStatus::None;
		Result = 0;
	}else if (wp == HTSYSMENU) {
		Result = 0;
	}else {
		//	2004.05.23 Moca メッセージミス修正
		//	フレームのダブルクリック時後にウィンドウサイズ
		//	変更モードなっていた
		Result = DefWindowProc(GetHwnd(), WM_NCLBUTTONUP, wp, lp);
	}

	return Result;
}

LRESULT EditWnd::OnLButtonDblClk(WPARAM wp, LPARAM lp) // by 鬼(2)
{
	LRESULT Result;
	if (m_iconClicked != IconClickStatus::None) {
		ReleaseCapture();
		m_iconClicked = IconClickStatus::DoubleClicked;

		SendMessage(GetHwnd(), WM_SYSCOMMAND, SC_CLOSE, 0);

		Result = 0;
	}else {
		// 2004.05.23 Moca メッセージミス修正
		Result = DefWindowProc(GetHwnd(), WM_LBUTTONDBLCLK, wp, lp);
	}

	return Result;
}

// ドロップダウンメニュー(開く)	@@@ 2002.06.15 MIK
int	EditWnd::CreateFileDropDownMenu(HWND hwnd)
{
	int			nId;
	HMENU		hMenu;
	HMENU		hMenuPopUp;
	POINT		po;
	RECT		rc;
	int			nIndex;

	// メニュー表示位置を決める	// 2007.03.25 ryoji
	// ※ TBN_DROPDOWN 時の NMTOOLBAR::iItem や NMTOOLBAR::rcButton にはドロップダウンメニュー(開く)ボタンが
	//    複数あるときはどれを押した時も１個目のボタン情報が入るようなのでマウス位置からボタン位置を求める
	::GetCursorPos(&po);
	::ScreenToClient(hwnd, &po);
	nIndex = Toolbar_Hittest(hwnd, &po);
	if (nIndex < 0) {
		return 0;
	}
	Toolbar_GetItemRect(hwnd, nIndex, &rc);
	po.x = rc.left;
	po.y = rc.bottom;
	::ClientToScreen(hwnd, &po);
	GetMonitorWorkRect(po, &rc);
	if (po.x < rc.left)
		po.x = rc.left;
	if (po.y < rc.top)
		po.y = rc.top;

	m_menuDrawer.ResetContents();

	// MRUリストのファイルのリストをメニューにする
	const MRUFile mru;
	hMenu = mru.CreateMenu(&m_menuDrawer);
	if (mru.MenuLength() > 0) {
		m_menuDrawer.MyAppendMenuSep(
			hMenu,
			MF_BYPOSITION | MF_SEPARATOR,
			0,
			NULL,
			FALSE
		);
	}

	// 最近使ったフォルダのメニューを作成
	const MRUFolder mruFolder;
	hMenuPopUp = mruFolder.CreateMenu(&m_menuDrawer);
	if (mruFolder.MenuLength() > 0) {
		// アクティブ
		m_menuDrawer.MyAppendMenu(
			hMenu,
			MF_BYPOSITION | MF_STRING | MF_POPUP,
			(UINT_PTR)hMenuPopUp,
			LS(F_FOLDER_USED_RECENTLY),
			_T("")
		);
	}else {
		// 非アクティブ
		m_menuDrawer.MyAppendMenu(
			hMenu,
			MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED,
			(UINT_PTR)hMenuPopUp,
			LS(F_FOLDER_USED_RECENTLY),
			_T("")
		);
	}

	m_menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE);

	m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW, _T(""), _T("N"), FALSE);
	m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW_NEWWINDOW, _T(""), _T("M"), FALSE);
	m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN, _T(""), _T("O"), FALSE);

	nId = ::TrackPopupMenu(
		hMenu,
		TPM_TOPALIGN
		| TPM_LEFTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		,
		po.x,
		po.y,
		0,
		GetHwnd(),	// 2009.02.03 ryoji アクセスキー有効化のため hwnd -> GetHwnd() に変更
		NULL
	);

	::DestroyMenu(hMenu);

	return nId;
}


/*!
	@brief ウィンドウのアイコン設定

	指定されたアイコンをウィンドウに設定する．
	以前のアイコンは破棄する．

	@param hIcon [in] 設定するアイコン
	@param flag [in] アイコン種別．ICON_BIGまたはICON_SMALL.
	@author genta
	@date 2002.09.10
*/
void EditWnd::SetWindowIcon(HICON hIcon, int flag)
{
	HICON hOld = (HICON)::SendMessage(GetHwnd(), WM_SETICON, flag, (LPARAM)hIcon);
	if (hOld) {
		::DestroyIcon(hOld);
	}
}

/*!
	標準アイコンの取得

	@param hIconBig   [out] 大きいアイコンのハンドル
	@param hIconSmall [out] 小さいアイコンのハンドル

	@author genta
	@date 2002.09.10
	@date 2002.12.02 genta 新設した共通関数を使うように
*/
void EditWnd::GetDefaultIcon(HICON* hIconBig, HICON* hIconSmall) const
{
	*hIconBig   = GetAppIcon(G_AppInstance(), ICON_DEFAULT_APP, FN_APP_ICON, false);
	*hIconSmall = GetAppIcon(G_AppInstance(), ICON_DEFAULT_APP, FN_APP_ICON, true);
}

/*!
	アイコンの取得
	
	指定されたファイル名に対応するアイコン(大・小)を取得して返す．
	
	@param szFile     [in] ファイル名
	@param hIconBig   [out] 大きいアイコンのハンドル
	@param hIconSmall [out] 小さいアイコンのハンドル
	
	@retval true 関連づけられたアイコンが見つかった
	@retval false 関連づけられたアイコンが見つからなかった
	
	@author genta
	@date 2002.09.10
*/
bool EditWnd::GetRelatedIcon(const TCHAR* szFile, HICON* hIconBig, HICON* hIconSmall) const
{
	if (szFile && szFile[0] != _T('\0')) {
		TCHAR szExt[_MAX_EXT];
		TCHAR FileType[1024];

		// (.で始まる)拡張子の取得
		_tsplitpath(szFile, NULL, NULL, NULL, szExt);
		
		if (ReadRegistry(HKEY_CLASSES_ROOT, szExt, NULL, FileType, _countof(FileType) - 13)) {
			_tcscat(FileType, _T("\\DefaultIcon"));
			if (ReadRegistry(HKEY_CLASSES_ROOT, FileType, NULL, NULL, 0)) {
				// 関連づけられたアイコンを取得する
				SHFILEINFO shfi;
				SHGetFileInfo(szFile, 0, &shfi, sizeof(shfi), SHGFI_ICON | SHGFI_LARGEICON);
				*hIconBig = shfi.hIcon;
				SHGetFileInfo(szFile, 0, &shfi, sizeof(shfi), SHGFI_ICON | SHGFI_SMALLICON);
				*hIconSmall = shfi.hIcon;
				return true;
			}
		}
	}

	// 標準のアイコンを返す
	GetDefaultIcon(hIconBig, hIconSmall);
	return false;
}

/*
	@brief メニューバー表示用フォントの初期化
	
	メニューバー表示用フォントの初期化を行う．
	
	@date 2002.12.04 EditViewのコンストラクタから移動
*/
void EditWnd::InitMenubarMessageFont(void)
{
	TEXTMETRIC	tm;
	HDC			hdc;
	HFONT		hFontOld;

	// LOGFONTの初期化
	LOGFONT lf = {0};
	lf.lfHeight			= DpiPointsToPixels(-9);	// 2009.10.01 ryoji 高DPI対応（ポイント数から算出）
	lf.lfWidth			= 0;
	lf.lfEscapement		= 0;
	lf.lfOrientation	= 0;
	lf.lfWeight			= 400;
	lf.lfItalic			= 0x0;
	lf.lfUnderline		= 0x0;
	lf.lfStrikeOut		= 0x0;
	lf.lfCharSet		= 0x80;
	lf.lfOutPrecision	= 0x3;
	lf.lfClipPrecision	= 0x2;
	lf.lfQuality		= 0x1;
	lf.lfPitchAndFamily	= 0x31;
	_tcscpy(lf.lfFaceName, _T("ＭＳ ゴシック"));
	m_hFontCaretPosInfo = ::CreateFontIndirect(&lf);

	hdc = ::GetDC(::GetDesktopWindow());
	hFontOld = (HFONT)::SelectObject(hdc, m_hFontCaretPosInfo);
	::GetTextMetrics(hdc, &tm);
	m_nCaretPosInfoCharWidth = tm.tmAveCharWidth;
	m_nCaretPosInfoCharHeight = tm.tmHeight;
	::SelectObject(hdc, hFontOld);
	::ReleaseDC(::GetDesktopWindow(), hdc);
}

/*
	@brief メニューバーにメッセージを表示する
	
	事前にメニューバー表示用フォントが初期化されていなくてはならない．
	指定できる文字数は最大30バイト．それ以上の場合はうち切って表示する．
	
	@author genta
	@date 2002.12.04
*/
void EditWnd::PrintMenubarMessage(const TCHAR* msg)
{
	if (!::GetMenu(GetHwnd()))	// 2007.03.08 ryoji 追加
		return;

	POINT	po, poFrame;
	RECT	rc, rcFrame;
	HFONT	hFontOld;
	int		nStrLen;

	// msg == NULL のときは以前の m_pszMenubarMessage で再描画
	if (msg) {
		int len = _tcslen(msg);
		_tcsncpy(m_pszMenubarMessage, msg, MENUBAR_MESSAGE_MAX_LEN);
		if (len < MENUBAR_MESSAGE_MAX_LEN) {
			auto_memset(m_pszMenubarMessage + len, _T(' '), MENUBAR_MESSAGE_MAX_LEN - len);	//  null終端は不要
		}
	}

	HDC hdc = ::GetWindowDC(GetHwnd());
	poFrame.x = 0;
	poFrame.y = 0;
	::ClientToScreen(GetHwnd(), &poFrame);
	::GetWindowRect(GetHwnd(), &rcFrame);
	po.x = rcFrame.right - rcFrame.left;
	po.y = poFrame.y - rcFrame.top;
	hFontOld = (HFONT)::SelectObject(hdc, m_hFontCaretPosInfo);
	nStrLen = MENUBAR_MESSAGE_MAX_LEN;
	rc.left = po.x - nStrLen * m_nCaretPosInfoCharWidth - (::GetSystemMetrics(SM_CXSIZEFRAME) + 2);
	rc.right = rc.left + nStrLen * m_nCaretPosInfoCharWidth + 2;
	rc.top = po.y - m_nCaretPosInfoCharHeight - 2;
	rc.bottom = rc.top + m_nCaretPosInfoCharHeight;
	::SetTextColor(hdc, ::GetSysColor(COLOR_MENUTEXT));
	//	Sep. 6, 2003 genta Windows XP(Luna)の場合にはCOLOR_MENUBARを使わなくてはならない
	COLORREF bkColor =
		::GetSysColor(IsWinXP_or_later() ? COLOR_MENUBAR : COLOR_MENU);
	::SetBkColor(hdc, bkColor);
	/*
	int m_pnCaretPosInfoDx[64];	// 文字列描画用文字幅配列
	for (i=0; i<_countof(m_pnCaretPosInfoDx); ++i) {
		m_pnCaretPosInfoDx[i] = (m_nCaretPosInfoCharWidth);
	}
	*/
	::ExtTextOut(hdc, rc.left, rc.top, ETO_CLIPPED | ETO_OPAQUE,&rc,m_pszMenubarMessage,nStrLen,NULL/*m_pnCaretPosInfoDx*/); // 2007.10.17 kobake めんどいので今のところは文字間隔配列を使わない。
	::SelectObject(hdc, hFontOld);
	::ReleaseDC(GetHwnd(), hdc);
}

/*!
	@brief メッセージの表示
	
	指定されたメッセージをステータスバーに表示する．
	ステータスバーが非表示の場合はメニューバーの右端に表示する．
	
	@param msg [in] 表示するメッセージ
	@date 2002.01.26 hor 新規作成
	@date 2002.12.04 genta EditViewより移動
*/
void EditWnd::SendStatusMessage(const TCHAR* msg)
{
	if (!m_statusBar.GetStatusHwnd()) {
		// メニューバーへ
		PrintMenubarMessage(msg);
	}else {
		// ステータスバーへ
		m_statusBar.SetStatusText(0, SBT_NOBORDERS, msg);
	}
}

/*! ファイル名変更通知

	@author MIK
	@date 2003.05.31 新規作成
	@date 2006.01.28 ryoji ファイル名、Grepモードパラメータを追加
*/
void EditWnd::ChangeFileNameNotify(const TCHAR* pszTabCaption, const TCHAR* _pszFilePath, bool bIsGrep)
{
	const TCHAR* pszFilePath = _pszFilePath;
	
	EditNode* p;
	int nIndex;
	
	if (!pszTabCaption) pszTabCaption = _T("");	// ガード
	if (!pszFilePath) pszFilePath = _FT("");	// ガード 2006.01.28 ryoji
	
	RecentEditNode	recentEditNode;
	nIndex = recentEditNode.FindItemByHwnd(GetHwnd());
	if (nIndex != -1) {
		p = recentEditNode.GetItem(nIndex);
		if (p) {
			int	size = _countof(p->m_szTabCaption) - 1;
			_tcsncpy(p->m_szTabCaption, pszTabCaption, size);
			p->m_szTabCaption[size] = _T('\0');

			// 2006.01.28 ryoji ファイル名、Grepモード追加
			size = _countof2(p->m_szFilePath) - 1;
			_tcsncpy(p->m_szFilePath, pszFilePath, size);
			p->m_szFilePath[size] = _T('\0');

			p->m_bIsGrep = bIsGrep;
		}
	}
	recentEditNode.Terminate();

	// ファイル名変更通知をブロードキャストする。
	int nGroup = AppNodeManager::getInstance()->GetEditNode(GetHwnd())->GetGroup();
	AppNodeGroupHandle(nGroup).PostMessageToAllEditors(
		MYWM_TAB_WINDOW_NOTIFY,
		(WPARAM)TabWndNotifyType::Rename,
		(LPARAM)GetHwnd(),
		GetHwnd()
	);

	return;
}

/*! 常に手前に表示
	@param top  0:トグル動作 1:最前面 2:最前面解除 その他:なにもしない
	@date 2004.09.21 Moca
*/
void EditWnd::WindowTopMost(int top)
{
	if (top == 0) {
		DWORD dwExstyle = (DWORD)::GetWindowLongPtr(GetHwnd(), GWL_EXSTYLE);
		if (dwExstyle & WS_EX_TOPMOST) {
			top = 2; // 最前面である -> 解除
		}else {
			top = 1;
		}
	}

	HWND hwndInsertAfter;
	switch (top) {
	case 1:
		hwndInsertAfter = HWND_TOPMOST;
		break;
	case 2:
		hwndInsertAfter = HWND_NOTOPMOST;
		break;
	default:
		return;
	}

	::SetWindowPos(GetHwnd(), hwndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	// タブまとめ時は WS_EX_TOPMOST 状態を全ウィンドウで同期する	// 2007.05.18 ryoji
	auto& csTabBar = m_pShareData->m_common.m_tabBar;
	if (m_pShareData->m_common.m_tabBar.m_bDispTabWnd && !m_pShareData->m_common.m_tabBar.m_bDispTabWndMultiWin) {
		hwndInsertAfter = GetHwnd();
		for (int i=0; i<m_pShareData->m_nodes.m_nEditArrNum; ++i) {
			HWND hwnd = m_pShareData->m_nodes.m_pEditArr[i].GetHwnd();
			if (hwnd != GetHwnd() && IsSakuraMainWindow(hwnd)) {
				if (!AppNodeManager::IsSameGroup(GetHwnd(), hwnd))
					continue;
				::SetWindowPos(hwnd, hwndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				hwndInsertAfter = hwnd;
			}
		}
	}
}


// タイマーの更新を開始／停止する。 20060128 aroka
// ツールバー表示はタイマーにより更新しているが、
// アプリのフォーカスが外れたときにウィンドウからON/OFFを
//	呼び出してもらうことにより、余計な負荷を停止したい。
void EditWnd::Timer_ONOFF(bool bStart)
{
	if (GetHwnd()) {
		if (bStart) {
			// タイマーを起動
			if (::SetTimer(GetHwnd(), IDT_TOOLBAR, 300, NULL) == 0) {
				WarningMessage(GetHwnd(), LS(STR_ERR_DLGEDITWND03));
			}
		}else {
			// タイマーを削除
			::KillTimer(GetHwnd(), IDT_TOOLBAR);
		}
	}
	return;
}

/*!	@brief ウィンドウ一覧をポップアップ表示

	@param[in] bMousePos true: マウス位置にポップアップ表示する

	@date 2006.03.23 fon OnListBtnClickをベースに新規作成
	@date 2006.05.10 ryoji ポップアップ位置変更、その他微修正
	@date 2007.02.28 ryoji フルパス指定のパラメータを削除
	@date 2009.06.02 ryoji m_menuDrawerの初期化漏れ修正
*/
LRESULT EditWnd::PopupWinList(bool bMousePos)
{
	POINT pt;

	// ポップアップ位置をアクティブビューの上辺に設定
	RECT rc;
	
	if (bMousePos) {
		::GetCursorPos(&pt);	// マウスカーソル位置に変更
	}else {
		::GetWindowRect(GetActiveView().GetHwnd(), &rc);
		pt.x = rc.right - 150;
		if (pt.x < rc.left)
			pt.x = rc.left;
		pt.y = rc.top;
	}

	// ウィンドウ一覧メニューをポップアップ表示する
	if (m_tabWnd.GetHwnd()) {
		m_tabWnd.TabListMenu(pt);
	}else {
		m_menuDrawer.ResetContents();	// 2009.06.02 ryoji 追加
		EditNode* pEditNodeArr;
		HMENU hMenu = ::CreatePopupMenu();	// 2006.03.23 fon
		int nRowNum = AppNodeManager::getInstance()->GetOpenedWindowArr(&pEditNodeArr, TRUE);
		WinListMenu(hMenu, pEditNodeArr, nRowNum, true);
		// メニューを表示する
		RECT rcWork;
		GetMonitorWorkRect(pt, &rcWork);	// モニタのワークエリア
		int nId = ::TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
									(pt.x > rcWork.left)? pt.x: rcWork.left,
									(pt.y < rcWork.bottom)? pt.y: rcWork.bottom,
									0, GetHwnd(), NULL);
		delete[] pEditNodeArr;
		::DestroyMenu(hMenu);
		::SendMessage(GetHwnd(), WM_COMMAND, (WPARAM)nId, (LPARAM)NULL);
	}

	return 0L;
}

/*! @brief 現在開いている編集窓のリストをメニューにする 
	@date  2006.03.23 fon EditWnd::InitMenuから移動。////が元からあるコメント。//>は追加コメントアウト。
	@date 2009.06.02 ryoji アイテム数が多いときはアクセスキーを 1-9,A-Z の範囲で再使用する（従来は36個未満を仮定）
*/
LRESULT EditWnd::WinListMenu(HMENU hMenu, EditNode* pEditNodeArr, int nRowNum, bool bFull)
{
	if (nRowNum > 0) {
		TCHAR szMenu[_MAX_PATH * 2 + 3];
		FileNameManager::getInstance()->TransformFileName_MakeCache();

		NONCLIENTMETRICS met;
		met.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, met.cbSize, &met, 0);
		DCFont dcFont(met.lfMenuFont, GetHwnd());
		for (int i=0; i<nRowNum; ++i) {
			// トレイからエディタへの編集ファイル名要求通知
			::SendMessage(pEditNodeArr[i].GetHwnd(), MYWM_GETFILEINFO, 0, 0);
////	From Here Oct. 4, 2000 JEPRO commented out & modified	開いているファイル数がわかるように履歴とは違って1から数える
			const EditInfo*	pfi = (EditInfo*)&m_pShareData->m_workBuffer.m_EditInfo_MYWM_GETFILEINFO;
			FileNameManager::getInstance()->GetMenuFullLabel_WinList(szMenu, _countof(szMenu), pfi, pEditNodeArr[i].m_nId, i, dcFont.GetHDC());
			m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, IDM_SELWINDOW + pEditNodeArr[i].m_nIndex, szMenu, _T(""));
			if (GetHwnd() == pEditNodeArr[i].GetHwnd()) {
				::CheckMenuItem(hMenu, IDM_SELWINDOW + pEditNodeArr[i].m_nIndex, MF_BYCOMMAND | MF_CHECKED);
			}
		}
	}
	return 0L;
}

// 2007.09.08 kobake 追加
// ツールチップのテキストを取得
void EditWnd::GetTooltipText(TCHAR* wszBuf, size_t nBufCount, int nID) const
{
	// 機能文字列の取得 -> tmp -> wszBuf
	WCHAR tmp[256];
	size_t nLen;
	GetDocument()->m_funcLookup.Funccode2Name(nID, tmp, _countof(tmp));
	nLen = _wcstotcs(wszBuf, tmp, nBufCount);

	// 機能に対応するキー名の取得(複数)
	auto& csKeyBind = m_pShareData->m_common.m_keyBind;
	NativeT** ppcAssignedKeyList;
	int nAssignedKeyNum = KeyBind::GetKeyStrList(
		G_AppInstance(),
		csKeyBind.m_nKeyNameArrNum,
		csKeyBind.m_pKeyNameArr,
		&ppcAssignedKeyList,
		nID
	);

	// wszBufへ結合
	if (0 < nAssignedKeyNum) {
		for (int j=0; j<nAssignedKeyNum; ++j) {
			const TCHAR* pszKey = ppcAssignedKeyList[j]->GetStringPtr();
			int nKeyLen = _tcslen(pszKey);
			if (nLen + 9 + nKeyLen < nBufCount) {
				_tcscat_s(wszBuf, nBufCount, _T("\n        "));
				_tcscat_s(wszBuf, nBufCount, pszKey);
				nLen += 9 + nKeyLen;
			}
			delete ppcAssignedKeyList[j];
		}
		delete[] ppcAssignedKeyList;
	}
}


/*! タイマーの処理
	@date 2002.01.03 YAZAKI m_tbMyButtonなどをShareDataからMenuDrawerへ移動したことによる修正。
	@date 2003.08.29 wmlhq, ryoji nTimerCountの導入
	@date 2006.01.28 aroka ツールバー更新を OnToolbarTimerに移動した
	@date 2007.04.03 ryoji パラメータ無しにした
*/
void EditWnd::OnEditTimer(void)
{
	//static	int	nLoopCount = 0; // wmlhq m_nTimerCountに移行
	// タイマーの呼び出し間隔を 500msに変更。300*10→500*6にする。 20060128 aroka
	IncrementTimerCount(6);

	// 2006.01.28 aroka ツールバー更新関連は OnToolbarTimerに移動した。
	
	//	Aug. 29, 2003 wmlhq, ryoji
	if (m_nTimerCount == 0 && !GetCapture()) { 
		// ファイルのタイムスタンプのチェック処理
		GetDocument()->m_autoReloadAgent.CheckFileTimeStamp();

#if 0	// 2011.02.11 ryoji 書込禁止の監視を廃止（復活させるなら「更新の監視」付随ではなく別オプションにしてほしい）
		// ファイル書込可能のチェック処理
		if (GetDocument()->m_autoReloadAgent._ToDoChecking()) {
			bool bOld = GetDocument()->m_docLocker.IsDocWritable();
			GetDocument()->m_docLocker.CheckWritable(false);
			if (bOld != GetDocument()->m_docLocker.IsDocWritable()) {
				this->UpdateCaption();
			}
		}
#endif
	}

	GetDocument()->m_autoSaveAgent.CheckAutoSave();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ビュー管理                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	EditViewの画面バッファを削除
	@date 2007.09.09 Moca 新規作成
*/
void EditWnd::Views_DeleteCompatibleBitmap()
{
	// EditView群へ転送する
	for (int i=0; i<GetAllViewCount(); ++i) {
		if (GetView(i).GetHwnd()) {
			GetView(i).DeleteCompatibleBitmap();
		}
	}
}

LRESULT EditWnd::Views_DispatchEvent(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_ENTERMENULOOP:
	case WM_EXITMENULOOP:
		for (int i=0; i<GetAllViewCount(); ++i) {
			GetView(i).DispatchEvent(hwnd, msg, wParam, lParam);
		}
		return 0L;
	default:
		return GetActiveView().DispatchEvent(hwnd, msg, wParam, lParam);
	}
}

/*
	分割指示。2つ目以降のビューを作る
	@param nViewCount  既存のビューも含めたビューの合計要求数
*/
bool EditWnd::CreateEditViewBySplit(int nViewCount)
{
	if (m_nEditViewMaxCount < nViewCount) {
		return false;
	}
	if (GetAllViewCount() < nViewCount) {
		for (int i=GetAllViewCount(); i<nViewCount; ++i) {
			assert(!m_pEditViewArr[i]);
			m_pEditViewArr[i] = new EditView(this);
			m_pEditViewArr[i]->Create(m_splitterWnd.GetHwnd(), GetDocument(), i, FALSE, false);
		}
		m_nEditViewCount = nViewCount;

		std::vector<HWND> hWndArr;
		hWndArr.reserve(nViewCount + 1);
		for (int i=0; i<nViewCount; ++i) {
			hWndArr.push_back(GetView(i).GetHwnd());
		}
		hWndArr.push_back(NULL);

		m_splitterWnd.SetChildWndArr(&hWndArr[0]);
	}
	return true;
}

/*
	ビューの再初期化
	@date 2010.04.10 EditDoc::InitAllViewから移動
*/
void EditWnd::InitAllViews()
{
	// 先頭へカーソルを移動
	for (int i=0; i<GetAllViewCount(); ++i) {
		//	Apr. 1, 2001 genta
		// 移動履歴の消去
		GetView(i).m_pHistory->Flush();

		// 現在の選択範囲を非選択状態に戻す
		GetView(i).GetSelectionInfo().DisableSelectArea(false);

		GetView(i).OnChangeSetting();
		GetView(i).GetCaret().MoveCursor(LayoutPoint(0, 0), true);
		GetView(i).GetCaret().m_nCaretPosX_Prev = LayoutInt(0);
	}
	GetMiniMap().OnChangeSetting();
}


void EditWnd::Views_RedrawAll()
{
	// アクティブ以外を再描画してから…
	for (int v=0; v<GetAllViewCount(); ++v) {
		if (m_nActivePaneIndex != v) {
			GetView(v).RedrawAll();
		}
	}
	GetMiniMap().RedrawAll();
	// アクティブを再描画
	GetActiveView().RedrawAll();
}

void EditWnd::Views_Redraw()
{
	// アクティブ以外を再描画してから…
	for (int v=0; v<GetAllViewCount(); ++v) {
		if (m_nActivePaneIndex != v)
			GetView(v).Redraw();
	}
	GetMiniMap().Redraw();
	// アクティブを再描画
	GetActiveView().Redraw();
}


// アクティブなペインを設定
void  EditWnd::SetActivePane(int nIndex)
{
	assert_warning(nIndex < GetAllViewCount());
	DEBUG_TRACE(_T("EditWnd::SetActivePane %d\n"), nIndex);

	// アクティブなビューを切り替える
	int nOldIndex = m_nActivePaneIndex;
	m_nActivePaneIndex = nIndex;
	m_pEditView = m_pEditViewArr[m_nActivePaneIndex];

	// フォーカスを移動する	// 2007.10.16 ryoji
	GetView(nOldIndex).GetCaret().m_cUnderLine.CaretUnderLineOFF(true);	//	2002/05/11 YAZAKI
	if (::GetActiveWindow() == GetHwnd()
		&& ::GetFocus() != GetActiveView().GetHwnd()
	) {
		// ::SetFocus()でフォーカスを切り替える
		::SetFocus(GetActiveView().GetHwnd());
	}else {
		// 2010.04.08 ryoji
		// 起動と同時にエディットボックスにフォーカスのあるダイアログを表示すると当該エディットボックスに
		// キャレットが表示されない問題(*1)を修正するのため、内部的な切り替えをするのはアクティブペインが
		// 切り替わるときだけにした。← EditView::OnKillFocus()は自スレッドのキャレットを破棄するので
		// (*1) -GREPDLGオプションによるGREPダイアログ表示や開ファイル後自動実行マクロでのInputBox表示
		if (m_nActivePaneIndex != nOldIndex) {
			// アクティブでないときに::SetFocus()するとアクティブになってしまう
			// （不可視なら可視になる）ので内部的に切り替えるだけにする
			GetView(nOldIndex).OnKillFocus();
			GetActiveView().OnSetFocus();
		}
	}

	GetActiveView().RedrawAll();	// フォーカス移動時の再描画

	m_splitterWnd.SetActivePane(nIndex);

	if (m_dlgFind.GetHwnd()) {		//「検索」ダイアログ
		// モードレス時：検索対象となるビューの変更
		m_dlgFind.ChangeView((LPARAM)&GetActiveView());
	}
	if (m_dlgReplace.GetHwnd()) {	//「置換」ダイアログ
		// モードレス時：検索対象となるビューの変更
		m_dlgReplace.ChangeView((LPARAM)&GetActiveView());
	}
	if (m_hokanMgr.GetHwnd()) {	//「入力補完」ダイアログ
		m_hokanMgr.Hide();
		// モードレス時：検索対象となるビューの変更
		m_hokanMgr.ChangeView((LPARAM)&GetActiveView());
	}
	if (m_dlgFuncList.GetHwnd()) {	//「アウトライン」ダイアログ	20060201 aroka
		// モードレス時：現在位置表示の対象となるビューの変更
		m_dlgFuncList.ChangeView((LPARAM)&GetActiveView());
	}

	return;
}

/** すべてのペインの描画スイッチを設定する

	@param bDraw [in] 描画スイッチの設定値

	@date 2008.06.08 ryoji 新規作成
*/
bool EditWnd::SetDrawSwitchOfAllViews(bool bDraw)
{
	bool bDrawSwitchOld = GetActiveView().GetDrawSwitch();
	for (int i=0; i<GetAllViewCount(); ++i) {
		GetView(i).SetDrawSwitch(bDraw);
	}
	GetMiniMap().SetDrawSwitch( bDraw );
	return bDrawSwitchOld;
}


/** すべてのペインをRedrawする

	スクロールバーの状態更新はパラメータでフラグ制御 or 別関数にしたほうがいい？
	@date 2007.07.22 ryoji スクロールバーの状態更新を追加

	@param pViewExclude [in] Redrawから除外するビュー
	@date 2008.06.08 ryoji pViewExclude パラメータ追加
*/
void EditWnd::RedrawAllViews(EditView* pViewExclude)
{
	for (int i=0; i<GetAllViewCount(); ++i) {
		EditView* pView = &GetView(i);
		if (pView == pViewExclude)
			continue;
		if (i == m_nActivePaneIndex) {
			pView->RedrawAll();
		}else {
			pView->Redraw();
			pView->AdjustScrollBars();
		}
	}
	GetMiniMap().Redraw();
	GetMiniMap().AdjustScrollBars();
}


void EditWnd::Views_DisableSelectArea(bool bRedraw)
{
	for (int i=0; i<GetAllViewCount(); ++i) {
		if (GetView(i).GetSelectionInfo().IsTextSelected()) {	// テキストが選択されているか
			// 現在の選択範囲を非選択状態に戻す
			GetView(i).GetSelectionInfo().DisableSelectArea(true);
		}
	}
}


// すべてのペインで、行番号表示に必要な幅を再設定する（必要なら再描画する）
BOOL EditWnd::DetectWidthOfLineNumberAreaAllPane(bool bRedraw)
{
	if (GetAllViewCount() == 1) {
		return GetActiveView().GetTextArea().DetectWidthOfLineNumberArea(bRedraw);
	}
	// 以下2,4分割限定

	if (GetActiveView().GetTextArea().DetectWidthOfLineNumberArea(bRedraw)) {
		// ActivePaneで計算したら、再設定・再描画が必要と判明した
		if (m_splitterWnd.GetAllSplitCols() == 2) {
			GetView(m_nActivePaneIndex^1).GetTextArea().DetectWidthOfLineNumberArea(bRedraw);
		}else {
			// 表示されていないので再描画しない
			GetView(m_nActivePaneIndex^1).GetTextArea().DetectWidthOfLineNumberArea(false);
		}
		if (m_splitterWnd.GetAllSplitRows() == 2) {
			GetView(m_nActivePaneIndex^2).GetTextArea().DetectWidthOfLineNumberArea(bRedraw);
			if (m_splitterWnd.GetAllSplitCols() == 2) {
				GetView((m_nActivePaneIndex^1)^2).GetTextArea().DetectWidthOfLineNumberArea(bRedraw);
			}
		}else {
			GetView(m_nActivePaneIndex^2).GetTextArea().DetectWidthOfLineNumberArea(false);
			GetView((m_nActivePaneIndex^1)^2).GetTextArea().DetectWidthOfLineNumberArea(false);
		}
		return TRUE;
	}
	return FALSE;
}


/** 右端で折り返す
	@param nViewColNum	[in] 右端で折り返すペインの番号
	@retval 折り返しを変更したかどうか
	@date 2008.06.08 ryoji 新規作成
*/
bool EditWnd::WrapWindowWidth(int nPane)
{
	// 右端で折り返す
	LayoutInt nWidth = GetView(nPane).ViewColNumToWrapColNum(GetView(nPane).GetTextArea().m_nViewColNum);
	if (GetDocument()->m_layoutMgr.GetMaxLineKetas() != nWidth) {
		ChangeLayoutParam(false, GetDocument()->m_layoutMgr.GetTabSpace(), nWidth);
		ClearViewCaretPosInfo();
		return true;
	}
	return false;
}

/** 折り返し方法関連の更新
	@retval 画面更新したかどうか
	@date 2008.06.10 ryoji 新規作成
*/
BOOL EditWnd::UpdateTextWrap(void)
{
	// この関数はコマンド実行ごとに処理の最終段階で利用する
	// （アンドゥ登録＆全ビュー更新のタイミング）
	if (GetDocument()->m_nTextWrapMethodCur == TextWrappingMethod::WindowWidth) {
		bool bWrap = WrapWindowWidth(0);	// 右端で折り返す
		if (bWrap) {
			// WrapWindowWidth() で追加した更新リージョンで画面更新する
			for (int i=0; i<GetAllViewCount(); ++i) {
				::UpdateWindow(GetView(i).GetHwnd());
			}
			if (GetMiniMap().GetHwnd()) {
				::UpdateWindow( GetMiniMap().GetHwnd() );
			}
		}
		return bWrap;	// 画面更新＝折り返し変更
	}
	return FALSE;	// 画面更新しなかった
}

/*!	レイアウトパラメータの変更

	具体的にはタブ幅と折り返し位置を変更する．
	現在のドキュメントのレイアウトのみを変更し，共通設定は変更しない．

	@date 2005.08.14 genta 新規作成
	@date 2008.06.18 ryoji レイアウト変更途中はカーソル移動の画面スクロールを見せない（画面のちらつき抑止）
*/
void EditWnd::ChangeLayoutParam(bool bShowProgress, LayoutInt nTabSize, LayoutInt nMaxLineKetas)
{
	HWND hwndProgress = NULL;
	if (bShowProgress && this) {
		hwndProgress = this->m_statusBar.GetProgressHwnd();
		// Status Barが表示されていないときはm_hwndProgressBar == NULL
	}

	if (hwndProgress) {
		::ShowWindow(hwndProgress, SW_SHOW);
	}

	// 座標の保存
	LogicPointEx* posSave = SavePhysPosOfAllView();

	// レイアウトの更新
	GetDocument()->m_layoutMgr.ChangeLayoutParam(nTabSize, nMaxLineKetas);
	ClearViewCaretPosInfo();

	// 座標の復元
	// レイアウト変更途中はカーソル移動の画面スクロールを見せない	// 2008.06.18 ryoji
	const bool bDrawSwitchOld = SetDrawSwitchOfAllViews(false);
	RestorePhysPosOfAllView(posSave);
	SetDrawSwitchOfAllViews(bDrawSwitchOld);

	for (int i=0; i<GetAllViewCount(); ++i) {
		if (GetView(i).GetHwnd()) {
			InvalidateRect(GetView(i).GetHwnd(), NULL, TRUE);
			GetView(i).AdjustScrollBars();	// 2008.06.18 ryoji
		}
	}
	if (GetMiniMap().GetHwnd()) {
		InvalidateRect(GetMiniMap().GetHwnd(), NULL, TRUE);
		GetMiniMap().AdjustScrollBars();
	}
	GetActiveView().GetCaret().ShowCaretPosInfo();	// 2009.07.25 ryoji

	if (hwndProgress) {
		::ShowWindow(hwndProgress, SW_HIDE);
	}
}


/*!
	レイアウトの変更に先立って，全てのViewの座標を物理座標に変換して保存する．

	@return データを保存した配列へのポインタ

	@note 取得した値はレイアウト変更後にEditWnd::RestorePhysPosOfAllViewへ渡す．
	渡し忘れるとメモリリークとなる．

	@date 2005.08.11 genta  新規作成
	@date 2007.09.06 kobake 戻り値をLogicPoint*に変更
	@date 2011.12.28 LogicPointをLogicPointExに変更。改行より右側でも復帰できるように
*/
LogicPointEx* EditWnd::SavePhysPosOfAllView()
{
	const int NUM_OF_VIEW = GetAllViewCount();
	const int NUM_OF_POS = 6;
	
	LogicPointEx* pptPosArray = new LogicPointEx[NUM_OF_VIEW * NUM_OF_POS];
	
	for (int i=0; i<NUM_OF_VIEW; ++i) {
		LayoutPoint tmp = LayoutPoint(LayoutInt(0), GetView(i).m_pTextArea->GetViewTopLine());
		const Layout* layoutLine = GetDocument()->m_layoutMgr.SearchLineByLayoutY(tmp.GetY2());
		if (layoutLine) {
			LogicInt nLineCenter = layoutLine->GetLogicOffset() + layoutLine->GetLengthWithoutEOL() / 2;
			pptPosArray[i * NUM_OF_POS + 0].x = nLineCenter;
			pptPosArray[i * NUM_OF_POS + 0].y = layoutLine->GetLogicLineNo();
		}else {
			pptPosArray[i * NUM_OF_POS + 0].x = LogicInt(0);
			pptPosArray[i * NUM_OF_POS + 0].y = LogicInt(0);
		}
		pptPosArray[i * NUM_OF_POS + 0].ext = LayoutInt(0);
		if (GetView(i).GetSelectionInfo().m_selectBgn.GetFrom().y >= 0) {
			GetDocument()->m_layoutMgr.LayoutToLogicEx(
				GetView(i).GetSelectionInfo().m_selectBgn.GetFrom(),
				&pptPosArray[i * NUM_OF_POS + 1]
			);
		}
		if (GetView(i).GetSelectionInfo().m_selectBgn.GetTo().y >= 0) {
			GetDocument()->m_layoutMgr.LayoutToLogicEx(
				GetView(i).GetSelectionInfo().m_selectBgn.GetTo(),
				&pptPosArray[i * NUM_OF_POS + 2]
			);
		}
		if (GetView(i).GetSelectionInfo().m_select.GetFrom().y >= 0) {
			GetDocument()->m_layoutMgr.LayoutToLogicEx(
				GetView(i).GetSelectionInfo().m_select.GetFrom(),
				&pptPosArray[i * NUM_OF_POS + 3]
			);
		}
		if (GetView(i).GetSelectionInfo().m_select.GetTo().y >= 0) {
			GetDocument()->m_layoutMgr.LayoutToLogicEx(
				GetView(i).GetSelectionInfo().m_select.GetTo(),
				&pptPosArray[i * NUM_OF_POS + 4]
			);
		}
		GetDocument()->m_layoutMgr.LayoutToLogicEx(
			GetView(i).GetCaret().GetCaretLayoutPos(),
			&pptPosArray[i * NUM_OF_POS + 5]
		);
	}
	return pptPosArray;
}


/*!	座標の復元

	EditWnd::SavePhysPosOfAllViewで保存したデータを元に座標値を再計算する．

	@date 2005.08.11 genta  新規作成
	@date 2007.09.06 kobake 引数をLogicPoint*に変更
	@date 2011.12.28 LogicPointをLogicPointExに変更。改行より右側でも復帰できるように
*/
void EditWnd::RestorePhysPosOfAllView(LogicPointEx* pptPosArray)
{
	const int NUM_OF_VIEW = GetAllViewCount();
	const int NUM_OF_POS = 6;

	for (int i=0; i<NUM_OF_VIEW; ++i) {
		LayoutPoint tmp;
		GetDocument()->m_layoutMgr.LogicToLayoutEx(
			pptPosArray[i * NUM_OF_POS + 0],
			&tmp
		);
		GetView(i).m_pTextArea->SetViewTopLine(tmp.GetY2());

		if (GetView(i).GetSelectionInfo().m_selectBgn.GetFrom().y >= 0) {
			GetDocument()->m_layoutMgr.LogicToLayoutEx(
				pptPosArray[i * NUM_OF_POS + 1],
				GetView(i).GetSelectionInfo().m_selectBgn.GetFromPointer()
			);
		}
		if (GetView(i).GetSelectionInfo().m_selectBgn.GetTo().y >= 0) {
			GetDocument()->m_layoutMgr.LogicToLayoutEx(
				pptPosArray[i * NUM_OF_POS + 2],
				GetView(i).GetSelectionInfo().m_selectBgn.GetToPointer()
			);
		}
		if (GetView(i).GetSelectionInfo().m_select.GetFrom().y >= 0) {
			GetDocument()->m_layoutMgr.LogicToLayoutEx(
				pptPosArray[i * NUM_OF_POS + 3],
				GetView(i).GetSelectionInfo().m_select.GetFromPointer()
			);
		}
		if (GetView(i).GetSelectionInfo().m_select.GetTo().y >= 0) {
			GetDocument()->m_layoutMgr.LogicToLayoutEx(
				pptPosArray[i * NUM_OF_POS + 4],
				GetView(i).GetSelectionInfo().m_select.GetToPointer()
			);
		}
		LayoutPoint ptPosXY;
		GetDocument()->m_layoutMgr.LogicToLayoutEx(
			pptPosArray[i * NUM_OF_POS + 5],
			&ptPosXY
		);
		GetView(i).GetCaret().MoveCursor(ptPosXY, false); // 2013.06.05 bScrollをtrue=>falase
		GetView(i).GetCaret().m_nCaretPosX_Prev = GetView(i).GetCaret().GetCaretLayoutPos().GetX2();

		LayoutInt nLeft = LayoutInt(0);
		if (GetView(i).GetTextArea().m_nViewColNum < GetView(i).GetRightEdgeForScrollBar()) {
			nLeft = GetView(i).GetRightEdgeForScrollBar() - GetView(i).GetTextArea().m_nViewColNum;
		}
		if (nLeft < GetView(i).GetTextArea().GetViewLeftCol()) {
			GetView(i).GetTextArea().SetViewLeftCol(nLeft);
		}

		GetView(i).GetCaret().ShowEditCaret();
	}
	GetActiveView().GetCaret().ShowCaretPosInfo();
	delete[] pptPosArray;
}

/*!
	@brief マウスの状態をクリアする（ホイールスクロール有無状態をクリア）

	@note ホイール操作によるページスクロール・横スクロール対応のために追加。
		  ページスクロール・横スクロールありフラグをOFFする。

	@date 2009.01.17 nasukoji	新規作成
*/
void EditWnd::ClearMouseState(void)
{
	SetPageScrollByWheel(FALSE);		// ホイール操作によるページスクロール有無
	SetHScrollByWheel(FALSE);			// ホイール操作による横スクロール有無
}

/*! ウィンドウ毎にアクセラレータテーブルを作成する(Wine用)
	@date 2009.08.15 Hidetaka Sakai, nasukoji
	@date 2013.10.19 novice 共有メモリの代わりにWine実行判定処理を呼び出す

	@note Wineでは別プロセスで作成したアクセラレータテーブルを使用することができない。
	      IsWine()によりプロセス毎にアクセラレータテーブルが作成されるようになる
	      ため、ショートカットキーやカーソルキーが正常に処理されるようになる。
*/
void EditWnd::CreateAccelTbl(void)
{
	if (IsWine()) {
		auto& csKeyBind = m_pShareData->m_common.m_keyBind;
		m_hAccelWine = KeyBind::CreateAccerelator(
			csKeyBind.m_nKeyNameArrNum,
			csKeyBind.m_pKeyNameArr
		);

		if (!m_hAccelWine) {
			ErrorMessage(
				NULL,
				LS(STR_ERR_DLGEDITWND01)
			);
		}
	}

	m_hAccel = m_hAccelWine ? m_hAccelWine : m_pShareData->m_handles.m_hAccel;
}

/*! ウィンドウ毎に作成したアクセラレータテーブルを破棄する
	@datet 2009.08.15 Hidetaka Sakai, nasukoji
*/
void EditWnd::DeleteAccelTbl(void)
{
	m_hAccel = NULL;

	if (m_hAccelWine) {
		::DestroyAcceleratorTable(m_hAccelWine);
		m_hAccelWine = NULL;
	}
}

// プラグインコマンドをエディタに登録する
void EditWnd::RegisterPluginCommand(int idCommand)
{
	Plug* plug = JackManager::getInstance()->GetCommandById(idCommand);
	RegisterPluginCommand(plug);
}

// プラグインコマンドをエディタに登録する（一括）
void EditWnd::RegisterPluginCommand()
{
	const Plug::Array& plugs = JackManager::getInstance()->GetPlugs(PP_COMMAND);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		RegisterPluginCommand(*it);
	}
}

// プラグインコマンドをエディタに登録する
void EditWnd::RegisterPluginCommand(Plug* plug)
{
	int iBitmap = MenuDrawer::TOOLBAR_ICON_PLUGCOMMAND_DEFAULT - 1;
	if (!plug->m_sIcon.empty()) {
		iBitmap = m_menuDrawer.m_pIcons->Add(to_tchar(plug->m_plugin.GetFilePath(to_tchar(plug->m_sIcon.c_str())).c_str()));
	}

	m_menuDrawer.AddToolButton(iBitmap, plug->GetFunctionCode());
}


const LOGFONT& EditWnd::GetLogfont(bool bTempSetting)
{
	if (bTempSetting && GetDocument()->m_blfCurTemp) {
		return GetDocument()->m_lfCur;
	}
	bool bUseTypeFont = GetDocument()->m_docType.GetDocumentAttribute().m_bUseTypeFont;
	if (bUseTypeFont) {
		return GetDocument()->m_docType.GetDocumentAttribute().m_lf;
	}
	return m_pShareData->m_common.m_view.m_lf;
}

int EditWnd::GetFontPointSize(bool bTempSetting)
{
	if (bTempSetting && GetDocument()->m_blfCurTemp) {
		return GetDocument()->m_nPointSizeCur;
	}
	bool bUseTypeFont = GetDocument()->m_docType.GetDocumentAttribute().m_bUseTypeFont;
	if (bUseTypeFont) {
		return GetDocument()->m_docType.GetDocumentAttribute().m_nPointSize;
	}
	return m_pShareData->m_common.m_view.m_nPointSize;
}
CharWidthCacheMode EditWnd::GetLogfontCacheMode()
{
	if (GetDocument()->m_blfCurTemp) {
		return CharWidthCacheMode::Local;
	}
	bool bUseTypeFont = GetDocument()->m_docType.GetDocumentAttribute().m_bUseTypeFont;
	if (bUseTypeFont) {
		return CharWidthCacheMode::Local;
	}
	return CharWidthCacheMode::Share;
}


void EditWnd::ClearViewCaretPosInfo()
{
	for (int v=0; v<GetAllViewCount(); ++v) {
		GetView(v).GetCaret().ClearCaretPosInfoCache();
	}
}
