// 編集ウィンドウ（外枠）管理クラス

#include "StdAfx.h"
#include <ShlObj.h>

#include "window/EditWnd.h"
#include "_main/ControlTray.h"
#include "_main/CommandLine.h"
#include "_main/AppMode.h"
#include "_os/DropTarget.h"
#include "_os/OsVersionInfo.h"
#include "dlg/DlgAbout.h"
#include "dlg/DlgPrintSetting.h"
#include "env/ShareData.h"
#include "env/SakuraEnvironment.h"
#include "print/PrintPreview.h"
#include "charset/CharPointer.h"
#include "charset/CodeFactory.h"
#include "charset/CodeBase.h"
#include "EditApp.h"
#include "recent/MRUFile.h"
#include "recent/MRUFolder.h"
#include "util/module.h"
#include "util/os.h"		// WM_MOUSEWHEEL,WM_THEMECHANGED
#include "util/window.h"
#include "util/shell.h"
#include "util/string_ex2.h"
#include "plugin/JackManager.h"
#include "GrepAgent.h"
#include "MarkMgr.h"
#include "doc/layout/Layout.h"
#include "debug/RunningTimer.h"
#include "sakura_rc.h"


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

//	状況によりメニューの表示を変えるコマンドリスト(SetMenuFuncSelで使用)
struct FuncMenuName {
	EFunctionCode	eFunc;
	int				nNameId[2];		// 選択文字列ID
};

static const FuncMenuName	gFuncMenuName[] = {
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
	size_t nLineLen;
	const EditView* pView = &pEditDoc->pEditWnd->GetActiveView();
	const Caret* pCaret = &pView->GetCaret();
	const LayoutMgr* pLayoutMgr = &pEditDoc->layoutMgr;
	const wchar_t* pLine = pLayoutMgr->GetLineStr(pCaret->GetCaretLayoutPos().y, &nLineLen, &pLayout);

	// -- -- -- -- キャレット位置の文字情報 -> szCaretChar -- -- -- -- //
	//
	if (pLine) {
		// 指定された桁に対応する行のデータ内の位置を調べる
		size_t nIdx = pView->LineColumnToIndex(pLayout, pCaret->GetCaretLayoutPos().x);
		if (nIdx < nLineLen) {
			if (nIdx < nLineLen - (pLayout->GetLayoutEol().GetLen() ? 1 : 0)) {
				// 一時的に表示方法の設定を変更する
				CommonSetting_StatusBar sStatusbar;
				sStatusbar.bDispUniInSjis		= false;
				sStatusbar.bDispUniInJis		= false;
				sStatusbar.bDispUniInEuc		= false;
				sStatusbar.bDispUtf8Codepoint	= false;
				sStatusbar.bDispSPCodepoint	= false;

				TCHAR szMsg[128];
				TCHAR szCode[CODE_CODEMAX][32];
				wchar_t szChar[3];
				size_t nCharChars = NativeW::GetSizeOfChar(pLine, nLineLen, nIdx);
				memcpy(szChar, &pLine[nIdx], nCharChars * sizeof(wchar_t));
				szChar[nCharChars] = L'\0';
				for (size_t i=0; i<CODE_CODEMAX; ++i) {
					if (i == CODE_SJIS || i == CODE_JIS || i == CODE_EUC || i == CODE_LATIN1 || i == CODE_UNICODE || i == CODE_UTF8 || i == CODE_CESU8) {
						// 任意の文字コードからUnicodeへ変換する
						CodeBase* pCode = CodeFactory::CreateCodeBase((EncodingType)i, false);
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
				sStatusbar.bDispSPCodepoint = true;
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

EditWnd::EditWnd()
	:
	hWnd(NULL)
	, toolbar(*this)
	, statusBar(*this)
	, pPrintPreview(nullptr)
	, pDragSourceView(nullptr)
	, nActivePaneIndex(0)
	, nEditViewCount(1)
	, nEditViewMaxCount(_countof(pEditViewArr))	// 今のところ最大値は固定
	, uMSIMEReconvertMsg(::RegisterWindowMessage(RWM_RECONVERT))
	, uATOKReconvertMsg(::RegisterWindowMessage(MSGNAME_ATOK_RECONVERT))
	, bIsActiveApp(false)
	, pszLastCaption(NULL)
	, pszMenubarMessage(new TCHAR[MENUBAR_MESSAGE_MAX_LEN])
	, posSaveAry(nullptr)
	, nCurrentFocus(0)
	, hAccelWine(NULL)
	, hAccel(NULL)
	, bDragMode(false)
	, iconClicked(IconClickStatus::None)
	, nSelectCountMode(SelectCountMode::Toggle)	// 文字カウント方法の初期値はSELECT_COUNT_TOGGLE→共通設定に従う
{
	g_pcEditWnd = this;
}

EditWnd::~EditWnd()
{
	g_pcEditWnd = nullptr;

	delete pPrintPreview;
	pPrintPreview = nullptr;

	for (int i=0; i<nEditViewMaxCount; ++i) {
		delete pEditViewArr[i];
		pEditViewArr[i] = nullptr;
	}
	pEditView = nullptr;

	delete pEditViewMiniMap;
	pEditViewMiniMap = nullptr;

	delete pViewFont;
	pViewFont = nullptr;

	delete pViewFontMiniMap;
	pViewFontMiniMap = nullptr;

	delete[] pszMenubarMessage;
	delete[] pszLastCaption;

	// キャレットの行桁位置表示用フォント
	::DeleteObject(hFontCaretPosInfo);

	delete pDropTarget;
	pDropTarget = nullptr;

	// ウィンドウ毎に作成したアクセラレータテーブルを破棄する(Wine用)
	DeleteAccelTbl();

	hWnd = NULL;
}


// ドキュメントリスナ：セーブ後
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
	const CommonSetting_Window& setting = GetDllShareData().common.window;
	const wchar_t* pszFormat = NULL;
	if (!this->IsActiveApp())	pszFormat = to_wchar(setting.szWindowCaptionInactive);
	else						pszFormat = to_wchar(setting.szWindowCaptionActive);
	SakuraEnvironment::ExpandParameter(
		pszFormat,
		pszCap,
		_countof(pszCap)
	);

	// キャプション更新
	::SetWindowText(this->GetHwnd(), to_tchar(pszCap));

	// タブウィンドウのファイル名を通知
	SakuraEnvironment::ExpandParameter(GetDllShareData().common.tabBar.szTabWndCaption, pszCap, _countof(pszCap));
	this->ChangeFileNameNotify(
		to_tchar(pszCap),
		GetListeningDoc()->docFile.GetFilePath(),
		EditApp::getInstance().pGrepAgent->bGrepMode
	);
}



// ウィンドウ生成用の矩形を取得
void EditWnd::_GetWindowRectForInit(Rect* rcResult, int nGroup, const TabGroupInfo& tabGroupInfo)
{
	// ウィンドウサイズ継承
	int	nWinCX, nWinCY;
	auto& csWindow = pShareData->common.window;
	if (csWindow.eSaveWindowSize != WinSizeMode::Default) {
		nWinCX = csWindow.nWinSizeCX;
		nWinCY = csWindow.nWinSizeCY;
	}else {
		nWinCX = CW_USEDEFAULT;
		nWinCY = 0;
	}

	// ウィンドウサイズ指定
	EditInfo fi = CommandLine::getInstance().GetEditInfo();
	if (fi.nWindowSizeX >= 0) {
		nWinCX = fi.nWindowSizeX;
	}
	if (fi.nWindowSizeY >= 0) {
		nWinCY = fi.nWindowSizeY;
	}

	// ウィンドウ位置指定
	int nWinOX = CW_USEDEFAULT;
	int nWinOY = 0;
	// ウィンドウ位置固定
	if (csWindow.eSaveWindowPos != WinSizeMode::Default) {
		nWinOX =  csWindow.nWinPosX;
		nWinOY =  csWindow.nWinPosY;
	}

	if (fi.nWindowOriginX != CW_USEDEFAULT) {
		nWinOX = fi.nWindowOriginX;
	}
	if (fi.nWindowOriginY != CW_USEDEFAULT) {
		nWinOY = fi.nWindowOriginY;
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
	wc.style			= CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc		= CEditWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 32;
	wc.hInstance		= G_AppInstance();
	wc.hIcon			= GetAppIcon(G_AppInstance(), ICON_DEFAULT_APP, FN_APP_ICON, false);

	wc.hCursor			= NULL/*LoadCursor(NULL, IDC_ARROW)*/;
	wc.hbrBackground	= (HBRUSH)NULL/*(COLOR_3DSHADOW + 1)*/;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= GSTR_EDITWINDOWNAME;

	wc.cbSize			= sizeof(wc);
	wc.hIconSm			= GetAppIcon(G_AppInstance(), ICON_DEFAULT_APP, FN_APP_ICON, true);
	ATOM atom = RegisterClassEx(&wc);
	if (atom == 0) {
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

	// タブウィンドウの場合は現状値を指定
	if (pShareData->common.tabBar.bDispTabWnd && !pShareData->common.tabBar.bDispTabWndMultiWin) {
		if (nGroup < 0)	// 不正なグループID
			nGroup = 0;	// グループ指定無し（最近アクティブのグループに入れる）
		EditNode* pEditNode = AppNodeGroupHandle(nGroup).GetEditNodeAt(0);	// グループの先頭ウィンドウ情報を取得
		hwndTop = pEditNode? pEditNode->GetHwnd(): NULL;

		if (hwndTop) {
			wpTop.length = sizeof(wpTop);
			if (::GetWindowPlacement(hwndTop, &wpTop)) {	// 現在の先頭ウィンドウから位置を取得
				if (wpTop.showCmd == SW_SHOWMINIMIZED)
					wpTop.showCmd = pEditNode->showCmdRestore;
			}else {
				hwndTop = NULL;
			}
		}
	}

	// 結果
	pTabGroupInfo->hwndTop = hwndTop;
	pTabGroupInfo->wpTop = wpTop;
}

void EditWnd::_AdjustInMonitor(const TabGroupInfo& tabGroupInfo)
{
	RECT	rcOrg;
	RECT	rcDesktop;
//	int		nWork;

	::GetMonitorWorkRect(GetHwnd(), &rcDesktop);
	::GetWindowRect(GetHwnd(), &rcOrg);

	// ウィンドウ位置調整
	if (rcOrg.bottom > rcDesktop.bottom) {
		rcOrg.top -= rcOrg.bottom - rcDesktop.bottom;
		rcOrg.bottom = rcDesktop.bottom;
	}
	if (rcOrg.right > rcDesktop.right) {
		rcOrg.left -= rcOrg.right - rcDesktop.right;
		rcOrg.right = rcDesktop.right;
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
		rcOrg.bottom = rcDesktop.bottom;
	}
	if (rcOrg.right > rcDesktop.right) {
		rcOrg.right = rcDesktop.right;
	}

	if (pShareData->common.tabBar.bDispTabWnd
		&& !pShareData->common.tabBar.bDispTabWndMultiWin
		&& tabGroupInfo.hwndTop
	) {
		// 現在の先頭ウィンドウから WS_EX_TOPMOST 状態を引き継ぐ
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
			typeOld = GetDocument().docType.GetDocumentType();	// 現在のタイプ
			{
				EditInfo ei = CommandLine::getInstance().GetEditInfo();
				if (ei.szDocType[0] != '\0') {
					typeNew = DocTypeManager().GetDocumentTypeOfExt(ei.szDocType);
				}else {
					EditInfo mruei;
					if (MruFile().GetEditInfo(ei.szPath, &mruei) && 0 < mruei.nTypeId) {
						typeNew = DocTypeManager().GetDocumentTypeOfId(mruei.nTypeId);
					}
					if (!typeNew.IsValidType()) {
						if (ei.szPath[0]) {
							typeNew = DocTypeManager().GetDocumentTypeOfPath(ei.szPath);
						}else {
							typeNew = typeOld;
						}
					}
				}
			}
			GetDocument().docType.SetDocumentType(typeNew, true, true);	// 仮設定

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
			GetDocument().docType.SetDocumentType(typeOld, true, true);	// タイプ戻し
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
		auto& csWindow = pShareData->common.window;
		if (csWindow.eSaveWindowSize != WinSizeMode::Default &&
			csWindow.nWinSizeType == SIZE_MAXIMIZED
		) {
			::ShowWindow(GetHwnd(), SW_SHOWMAXIMIZED);
		}else
		// ウィンドウサイズを直接指定する場合は、最小化表示を受け入れる
		if (csWindow.eSaveWindowSize == WinSizeMode::Set &&
			csWindow.nWinSizeType == SIZE_MINIMIZED
		) {
			::ShowWindow(GetHwnd(), SW_SHOWMINIMIZED);
		}else {
			::ShowWindow(GetHwnd(), SW_SHOW);
		}
	}
}

/*!
	作成
*/
HWND EditWnd::Create(
	EditDoc*		pEditDoc,
	ImageListMgr*	pIcons,	// [in] Image List
	int				nGroup		// [in] グループID
	)
{
	MY_RUNNINGTIMER(runningTimer, "EditWnd::Create");

	// 共有データ構造体のアドレスを返す
	pShareData = &GetDllShareData();

	this->pEditDoc = pEditDoc;

	for (size_t i=0; i<_countof(pEditViewArr); ++i) {
		pEditViewArr[i] = nullptr;
	}
	// [0] - [3] まで作成・初期化していたものを[0]だけ作る。ほかは分割されるまで何もしない
	pEditViewArr[0] = new EditView(*this);
	pEditView = pEditViewArr[0];

	pViewFont = new ViewFont(&GetLogfont());

	pEditViewMiniMap = new EditView(*this);

	pViewFontMiniMap = new ViewFont(&GetLogfont(), true);

	auto_memset(pszMenubarMessage, _T(' '), MENUBAR_MESSAGE_MAX_LEN);	// null終端は不要

	InitMenubarMessageFont();

	pDropTarget = new DropTarget(this);	// 右ボタンドロップ用

	// ホイールスクロール有無状態をクリア
	ClearMouseState();

	// ウィンドウ毎にアクセラレータテーブルを作成する(Wine用)
	CreateAccelTbl();

	// ウィンドウ数制限
	if (pShareData->nodes.nEditArrNum >= MAX_EDITWINDOWS) {	// 最大値修正
		OkMessage(NULL, LS(STR_MAXWINDOW), MAX_EDITWINDOWS);
		return NULL;
	}

	// タブグループ情報取得
	TabGroupInfo tabGroupInfo;
	_GetTabGroupInfo(&tabGroupInfo, nGroup);


	// -- -- -- -- ウィンドウ作成 -- -- -- -- //
	hWnd = _CreateMainWindow(nGroup, tabGroupInfo);
	if (!hWnd) {
		return NULL;
	}

	// 初回アイドリング検出用のゼロ秒タイマーをセットする
	// ゼロ秒タイマーが発動（初回アイドリング検出）したら MYWM_FIRST_IDLE を起動元プロセスにポストする。
	// ※起動元での起動先アイドリング検出については ControlTray::OpenNewEditor を参照
	::SetTimer(GetHwnd(), IDT_FIRST_IDLE, 0, NULL);

	// 編集ウィンドウリストへの登録
	if (!AppNodeGroupHandle(nGroup).AddEditWndList(GetHwnd())) {
		OkMessage(GetHwnd(), LS(STR_MAXWINDOW), MAX_EDITWINDOWS);
		::DestroyWindow(GetHwnd());
		hWnd = NULL;
		return hWnd;
	}

	// コモンコントロール初期化
	MyInitCommonControls();

	// イメージ、ヘルパなどの作成
	menuDrawer.Create(G_AppInstance(), GetHwnd(), pIcons);
	toolbar.Create(pIcons);

	// プラグインコマンドを登録する
	RegisterPluginCommand();

	SelectCharWidthCache(CharWidthFontMode::MiniMap, CharWidthCacheMode::Local); // Init
	InitCharWidthCache(pViewFontMiniMap->GetLogfont(), CharWidthFontMode::MiniMap);
	SelectCharWidthCache(CharWidthFontMode::Edit, GetLogfontCacheMode());
	InitCharWidthCache(GetLogfont());

	// -- -- -- -- 子ウィンドウ作成 -- -- -- -- //

	// 分割フレーム作成
	splitterWnd.Create(G_AppInstance(), GetHwnd(), this);

	// ビュー
	GetView(0).Create(splitterWnd.GetHwnd(), GetDocument(), 0, TRUE, false);
	GetView(0).OnSetFocus();

	// 子ウィンドウの設定
	HWND hWndArr[2];
	hWndArr[0] = GetView(0).GetHwnd();
	hWndArr[1] = NULL;
	splitterWnd.SetChildWndArr(hWndArr);

	MY_TRACETIME(runningTimer, "View created");

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
	pDropTarget->Register_DropTarget(hWnd);	// 右ボタンドロップ用

	// アクティブ情報
	bIsActiveApp = (::GetActiveWindow() == GetHwnd());

	// エディタ－トレイ間でのUI特権分離の確認（Vista UIPI機能）
	if (IsWinVista_or_later()) {
		bUIPI = FALSE;
		::SendMessage(pShareData->handles.hwndTray, MYWM_UIPI_CHECK,  (WPARAM)0, (LPARAM)GetHwnd());
		if (!bUIPI) {	// 返事が返らない
			TopErrorMessage(GetHwnd(),
				LS(STR_ERR_DLGEDITWND02)
			);
			::DestroyWindow(GetHwnd());
			hWnd = NULL;
			return hWnd;
		}
	}

	ShareData::getInstance().SetTraceOutSource(GetHwnd());	// TraceOut()起動元ウィンドウの設定

	nTimerCount = 0;
	if (::SetTimer(GetHwnd(), IDT_EDIT, 500, NULL) == 0) {
		WarningMessage(GetHwnd(), LS(STR_ERR_DLGEDITWND03));
	}
	Timer_ONOFF(true);

	// デフォルトのIMEモード設定
	GetDocument().docEditor.SetImeMode(GetDocument().docType.GetDocumentAttribute().nImeState);

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
		bool bReadResult = GetDocument().docFileOperation.FileLoadWithoutAutoMacro(&loadInfo);	// 自動実行マクロは後で別の場所で実行される
		if (!bReadResult) {
			// ファイルが既に開かれている
			if (loadInfo.bOpened) {
				::PostMessage(GetHwnd(), WM_CLOSE, 0, 0);
				// return NULLだと、メッセージループを通らずにそのまま破棄されてしまい、タブの終了処理が抜ける
				//	この後は正常ルートでメッセージループに入った後WM_CLOSEを受信して直ちにCLOSE & DESTROYとなる．
				//	その中で編集ウィンドウの削除が行われる．
			}
		}
	}
}

void EditWnd::SetDocumentTypeWhenCreate(
	EncodingType	nCharCode,		// [in] 漢字コード
	bool			bViewMode,		// [in] ビューモードで開くかどうか
	TypeConfigNum	nDocumentType	// [in] 文書タイプ．-1のとき強制指定無し．
	)
{
	auto& docType = GetDocument().docType;
	if (nDocumentType.IsValidType()) {
		docType.SetDocumentType(nDocumentType, true);
		// タイプ別設定一覧の一時適用のコードを流用
		docType.LockDocumentType();
	}

	// 文字コードの指定
	if (IsValidCodeType(nCharCode) || nDocumentType.IsValidType()) {
		const TypeConfig& types = docType.GetDocumentAttribute();
		EncodingType eDefaultCharCode = types.encoding.eDefaultCodetype;
		if (!IsValidCodeType(nCharCode)) {
			nCharCode = eDefaultCharCode;	// 直接コード指定がなければタイプ指定のデフォルト文字コードを使用
		}
		if (nCharCode == eDefaultCharCode) {	// デフォルト文字コードと同じ文字コードが選択されたとき
			GetDocument().SetDocumentEncoding(nCharCode, types.encoding.bDefaultBom);
			GetDocument().docEditor.newLineCode = static_cast<EolType>(types.encoding.eDefaultEoltype);
		}else {
			GetDocument().SetDocumentEncoding(nCharCode, CodeTypeName(nCharCode).IsBomDefOn());
			GetDocument().docEditor.newLineCode = EolType::CRLF;
		}
	}

	AppMode::getInstance().SetViewMode(bViewMode);

	if (nDocumentType.IsValidType()) {
		// 設定変更を反映させる
		GetDocument().OnChangeSetting();	// <--- 内部に BlockingHook() 呼び出しがあるので溜まった描画がここで実行される
	}
}


/*! メインメニューの配置処理 */
void EditWnd::LayoutMainMenu()
{
	TCHAR		szLabel[300];
	TCHAR		szKey[10];
	CommonSetting_MainMenu*	pMenu = &pShareData->common.mainMenu;
	MainMenu*	mainMenu;
	HWND		hWnd = GetHwnd();
	HMENU		hMenu;
	int 		nCount;
	LPCTSTR		pszName;

	hMenu = ::CreateMenu();
	auto& csKeyBind = pShareData->common.keyBind;
	for (int i=0; i<MAX_MAINMENU_TOP && pMenu->nMenuTopIdx[i] >= 0; ++i) {
		nCount = (i >= MAX_MAINMENU_TOP || pMenu->nMenuTopIdx[i + 1] < 0 ? pMenu->nMainMenuNum : pMenu->nMenuTopIdx[i+1])
				- pMenu->nMenuTopIdx[i];		// メニュー項目数
		mainMenu = &pMenu->mainMenuTbl[pMenu->nMenuTopIdx[i]];
		switch (mainMenu->type) {
		case MainMenuType::Node:
			// ラベル未設定かつFunctionコードがありならストリングテーブルから取得
			pszName = (mainMenu->sName[0] == L'\0' && mainMenu->nFunc != F_NODE)
								? LS(mainMenu->nFunc) : to_tchar(mainMenu->sName);
			::AppendMenu(hMenu, MF_POPUP | MF_STRING | (nCount <= 1 ? MF_GRAYED : 0), (UINT_PTR)CreatePopupMenu(), 
				KeyBind::MakeMenuLabel(pszName, to_tchar(mainMenu->sKey)));
			break;
		case MainMenuType::Leaf:
			// メニューラベルの作成
			{
				wchar_t szLabelW[256];
				GetDocument().funcLookup.Funccode2Name(mainMenu->nFunc, szLabelW, 256);
				auto_strncpy(szLabel, to_tchar(szLabelW), _countof(szLabel) - 1);
				szLabel[_countof(szLabel) - 1] = _T('\0');
			}
			auto_strcpy(szKey, to_tchar(mainMenu->sKey));
			if (!KeyBind::GetMenuLabel(
				G_AppInstance(),
				csKeyBind.nKeyNameArrNum,
				csKeyBind.pKeyNameArr,
				mainMenu->nFunc,
				szLabel,
				to_tchar(mainMenu->sKey),
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
				nCount = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true);
				delete[] pEditNodeArr;
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
				if (pShareData->common.customMenu.nCustMenuItemNumArr[0] > 0) {
					++nCount;
				}
				//	カスタムメニュー
				for (int j=1; j<MAX_CUSTOM_MENU; ++j) {
					if (pShareData->common.customMenu.nCustMenuItemNumArr[j] > 0) {
						++nCount;
					}
				}
				break;
			case F_USERMACRO_LIST:			// 登録済みマクロリスト
				for (int j=0; j<MAX_CUSTMACRO; ++j) {
					MacroRec *mp = &pShareData->common.macro.macroTable[j];
					if (mp->IsEnabled()) {
						++nCount;
					}
				}
				break;
			case F_PLUGIN_LIST:				// プラグインコマンドリスト
				// プラグインコマンドを提供するプラグインを列挙する
				{
					auto& jackManager = JackManager::getInstance();

					Plug::Array plugs = jackManager.GetPlugs(PP_COMMAND);
					for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
						++nCount;
					}
				}
				break;
			}
			::AppendMenu(hMenu, MF_POPUP | MF_STRING | (nCount <= 0 ? MF_GRAYED : 0), (UINT_PTR)CreatePopupMenu(), 
				KeyBind::MakeMenuLabel(LS(mainMenu->nFunc), to_tchar(mainMenu->sKey)));
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

/*! ツールバーの配置処理 */
void EditWnd::LayoutToolBar(void)
{
	if (pShareData->common.window.bDispToolBar) {	// ツールバーを表示する
		toolbar.CreateToolBar();
	}else {
		toolbar.DestroyToolBar();
	}
}

/*! ステータスバーの配置処理 */
void EditWnd::LayoutStatusBar(void)
{
	if (pShareData->common.window.bDispStatusBar) {	// ステータスバーを表示する
		// ステータスバー作成
		statusBar.CreateStatusBar();
	}else {
		// ステータスバー破棄
		statusBar.DestroyStatusBar();
	}
}

/*! ファンクションキーの配置処理 */
void EditWnd::LayoutFuncKey(void)
{
	if (pShareData->common.window.bDispFuncKeyWnd) {	// ファンクションキーを表示する
		if (!funcKeyWnd.GetHwnd()) {
			bool bSizeBox;
			if (pShareData->common.window.nFuncKeyWnd_Place == 0) {	// ファンクションキー表示位置／0:上 1:下
				bSizeBox = false;
			}else {
				bSizeBox = true;
				// ステータスバーがあるときはサイズボックスを表示しない
				if (statusBar.GetStatusHwnd()) {
					bSizeBox = false;
				}
			}
			funcKeyWnd.Open(G_AppInstance(), GetHwnd(), &GetDocument(), bSizeBox);
		}
	}else {
		funcKeyWnd.Close();
	}
}

/*! タブバーの配置処理 */
void EditWnd::LayoutTabBar(void)
{
	if (pShareData->common.tabBar.bDispTabWnd) {	// タブバーを表示する
		if (!tabWnd.GetHwnd()) {
			tabWnd.Open(G_AppInstance(), GetHwnd());
		}else {
			tabWnd.UpdateStyle();
		}
	}else {
		tabWnd.Close();
		tabWnd.SizeBox_ONOFF(false);
	}
}

/*! ミニマップの配置処理 */
void EditWnd::LayoutMiniMap( void )
{
	if (pShareData->common.window.bDispMiniMap) {	// タブバーを表示する
		if (!GetMiniMap().GetHwnd()) {
			GetMiniMap().Create(GetHwnd(), GetDocument(), -1, TRUE, true);
		}
	}else {
		if (GetMiniMap().GetHwnd()) {
			GetMiniMap().Close();
		}
	}
}

/*! バーの配置終了処理 */
void EditWnd::EndLayoutBars(bool bAdjust/* = true*/)
{
	int nCmdShow = pPrintPreview? SW_HIDE: SW_SHOW;
	HWND hwndToolBar = toolbar.GetRebarHwnd() ? toolbar.GetRebarHwnd(): toolbar.GetToolbarHwnd();
	if (hwndToolBar)
		::ShowWindow(hwndToolBar, nCmdShow);
	if (statusBar.GetStatusHwnd())
		::ShowWindow(statusBar.GetStatusHwnd(), nCmdShow);
	if (funcKeyWnd.GetHwnd())
		::ShowWindow(funcKeyWnd.GetHwnd(), nCmdShow);
	if (tabWnd.GetHwnd())
		::ShowWindow(tabWnd.GetHwnd(), nCmdShow);
	if (dlgFuncList.GetHwnd() && dlgFuncList.IsDocking()) {
		::ShowWindow(dlgFuncList.GetHwnd(), nCmdShow);
		// アウトラインを最背後にしておく（ゴミ描画の抑止策）
		// この対策以前は、アウトラインを下ドッキングしている状態で、
		// メニューから[ファンクションキーを表示]/[ステータスバーを表示]を実行して非表示のバーをアウトライン直下に表示したり、
		// その後、ウィンドウの下部境界を上下ドラッグしてサイズ変更するとゴミが現れることがあった。
		::SetWindowPos(dlgFuncList.GetHwnd(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	}

	if (bAdjust) {
		RECT rc;
		splitterWnd.DoSplit(-1, -1);
		::GetClientRect(GetHwnd(), &rc);
		::SendMessage(GetHwnd(), WM_SIZE, nWinSizeType, MAKELONG(rc.right - rc.left, rc.bottom - rc.top));
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
void EditWnd::MessageLoop(void)
{
	MSG	msg;
	while (GetHwnd()) {
		// メッセージ取得
		int ret = GetMessage(&msg, NULL, 0, 0);
		if (ret == 0) break; // WM_QUIT
		if (ret == -1) break; // GetMessage失敗

		// ダイアログメッセージ
		     if (MyIsDialogMessage(pPrintPreview->GetPrintPreviewBarHANDLE_Safe(),	&msg)) {}	// 印刷Preview 操作バー
		else if (MyIsDialogMessage(dlgFind.GetHwnd(),									&msg)) {}	//「検索」ダイアログ
		else if (MyIsDialogMessage(dlgFuncList.GetHwnd(),								&msg)) {}	//「アウトライン」ダイアログ
		else if (MyIsDialogMessage(dlgReplace.GetHwnd(),								&msg)) {}	//「置換」ダイアログ
		else if (MyIsDialogMessage(dlgGrep.GetHwnd(),									&msg)) {}	//「Grep」ダイアログ
		else if (MyIsDialogMessage(hokanMgr.GetHwnd(),								&msg)) {}	//「入力補完」
		else if (toolbar.EatMessage(&msg)) { }													// ツールバー
		// アクセラレータ
		else {
			if (hAccel && TranslateAccelerator(msg.hwnd, hAccel, &msg)) {}
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
		return menuDrawer.OnMenuChar(hwnd, uMsg, wParam, lParam);

	case WM_SHOWWINDOW:
		if (!wParam) {
			Views_DeleteCompatibleBitmap();
		}
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);

	case WM_MENUSELECT:
		if (!statusBar.GetStatusHwnd()) {
			return 1;
		}
		uItem = (UINT) LOWORD(wParam);		// menu item or submenu index
		{
			// メニュー機能のテキストをセット
			NativeT memWork;

			// 機能に対応するキー名の取得(複数)
			NativeT** ppcAssignedKeyList;
			auto& csKeyBind = pShareData->common.keyBind;
			int nAssignedKeyNum = KeyBind::GetKeyStrList(
				G_AppInstance(),
				csKeyBind.nKeyNameArrNum,
				(KeyData*)csKeyBind.pKeyNameArr,
				&ppcAssignedKeyList,
				uItem
			);
			if (0 < nAssignedKeyNum) {
				for (int j=0; j<nAssignedKeyNum; ++j) {
					if (j > 0) {
						memWork.AppendStringLiteral(_T(" , "));
					}
					memWork.AppendNativeData(*ppcAssignedKeyList[j]);
					delete ppcAssignedKeyList[j];
				}
				delete[] ppcAssignedKeyList;
			}
			const TCHAR* pszItemStr = memWork.GetStringPtr();
			statusBar.SetStatusText(0, SBT_NOBORDERS, pszItemStr);
		}
		return 0;

	case WM_DRAWITEM:
		idCtl = (UINT) wParam;				// コントロールのID
		lpdis = (DRAWITEMSTRUCT*) lParam;	// 項目描画情報
		if (idCtl == IDW_STATUSBAR) {
			if (lpdis->itemID == 5) {
				int	nColor;
				if (pShareData->flags.bRecordingKeyMacro	// キーボードマクロの記録中
				 && pShareData->flags.hwndRecordingKeyMacro == GetHwnd()	// キーボードマクロを記録中のウィンドウ
				) {
					nColor = COLOR_BTNTEXT;
				}else {
					nColor = COLOR_3DSHADOW;
				}
				::SetTextColor(lpdis->hDC, ::GetSysColor(nColor));
				::SetBkMode(lpdis->hDC, TRANSPARENT);
				
				// 上下中央位置に作画
				TEXTMETRIC tm;
				::GetTextMetrics(lpdis->hDC, &tm);
				int y = (lpdis->rcItem.bottom - lpdis->rcItem.top - tm.tmHeight + 1) / 2 + lpdis->rcItem.top;
				::TextOut(lpdis->hDC, lpdis->rcItem.left, y, _T("REC"), _tcslen(_T("REC")));
				if (nColor == COLOR_BTNTEXT) {
					::TextOut(lpdis->hDC, lpdis->rcItem.left + 1, y, _T("REC"), _tcslen(_T("REC")));
				}
			}
			return 0;
		}else {
			switch (lpdis->CtlType) {
			case ODT_MENU:	// オーナー描画メニュー
				// メニューアイテム描画
				menuDrawer.DrawItem(lpdis);
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
			nItemWidth = menuDrawer.MeasureItem(lpmis->itemID, &nItemHeight);
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
		bIsActiveApp = (wParam != 0);	// 自アプリがアクティブかどうか

		// アクティブ化なら編集ウィンドウリストの先頭に移動する
		if (bIsActiveApp) {
			AppNodeGroupHandle(0).AddEditWndList(GetHwnd());	// リスト移動処理

			// ホイールスクロール有無状態をクリア
			ClearMouseState();
		}

		// タイマーON/OFF
		UpdateCaption();
		funcKeyWnd.Timer_ONOFF(bIsActiveApp);
		this->Timer_ONOFF(bIsActiveApp);

		return 0L;

	case WM_ENABLE:
		// 右ドロップファイルの受け入れ設定／解除
		// Note: DragAcceptFilesを適用した左ドロップについては Enable/Disable で自動的に受け入れ設定／解除が切り替わる
		if ((BOOL)wParam) {
			pDropTarget->Register_DropTarget(hWnd);
		}else {
			pDropTarget->Revoke_DropTarget();
		}
		return 0L;

	case WM_WINDOWPOSCHANGED:
		// ポップアップウィンドウの表示切替指示をポストする
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
		::ShowOwnedPopups(hWnd, (BOOL)wParam);
		return 0L;

	case WM_SIZE:
//		MYTRACE(_T("WM_SIZE\n"));
		/* WM_SIZE 処理 */
		if (wParam == SIZE_MINIMIZED) {
			this->UpdateCaption();
		}
		return OnSize(wParam, lParam);

	case WM_MOVE:
		// ウィンドウ位置継承
		//	最後の位置を復元するため，移動されるたびに共有メモリに位置を保存する．
		if (WinSizeMode::Save == pShareData->common.window.eSaveWindowPos) {
			if (!::IsZoomed(GetHwnd()) && !::IsIconic(GetHwnd())) {
				// Aero Snapで縦方向最大化で終了して次回起動するときは元のサイズにする必要があるので、
				// GetWindowRect()ではなくGetWindowPlacement()で得たワークエリア座標をスクリーン座標に変換して記憶する
				WINDOWPLACEMENT wp;
				wp.length = sizeof(wp);
				::GetWindowPlacement(GetHwnd(), &wp);	// ワークエリア座標
				RECT rcWin = wp.rcNormalPosition;
				RECT rcWork, rcMon;
				GetMonitorWorkRect(GetHwnd(), &rcWork, &rcMon);
				::OffsetRect(&rcWin, rcWork.left - rcMon.left, rcWork.top - rcMon.top);	// スクリーン座標に変換
				pShareData->common.window.nWinPosX = rcWin.left;
				pShareData->common.window.nWinPosY = rcWin.top;
			}
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	case WM_SYSCOMMAND:
		// タブまとめ表示では閉じる動作はオプション指定に従う
		//	Feb. 11, 2007 genta 動作を選べるように(MDI風と従来動作)
		if (wParam == SC_CLOSE) {
			// 印刷Previewモードでウィンドウを閉じる操作のときはPreviewを閉じる
			if (pPrintPreview) {
				PrintPreviewModeONOFF();	// 印刷Previewモードのオン/オフ
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
	case WM_SYSKEYUP:	// ALT+キーのキーリピート処理のため
	case WM_ENTERMENULOOP:
		if (GetActiveView().nAutoScrollMode) {
			GetActiveView().AutoScrollExit();
		}
		// メッセージの配送
		return Views_DispatchEvent(hwnd, uMsg, wParam, lParam);

	case WM_EXITMENULOOP:
//		MYTRACE(_T("WM_EXITMENULOOP\n"));
		if (statusBar.GetStatusHwnd()) {
			statusBar.SetStatusText(0, SBT_NOBORDERS, _T(""));
		}
		menuDrawer.EndDrawMenu();
		// メッセージの配送
		return Views_DispatchEvent(hwnd, uMsg, wParam, lParam);

	case WM_SETFOCUS:
//		MYTRACE(_T("WM_SETFOCUS\n"));

		nTimerCount = 9;

		// ビューにフォーカスを移動する
		if (!pPrintPreview && pEditView) {
			::SetFocus(GetActiveView().GetHwnd());
		}
		lRes = 0;

		// 印刷Previewモードのときは、キー操作は全部PrintPreviewBarへ転送
		if (pPrintPreview) {
			pPrintPreview->SetFocusToPrintPreviewBar();
		}
		return lRes;

	case WM_NOTIFY:
		pnmh = (LPNMHDR) lParam;
		//	ステータスバーのダブルクリックでモード切替ができるようにする
		if (statusBar.GetStatusHwnd() && pnmh->hwndFrom == statusBar.GetStatusHwnd()) {
			if (pnmh->code == NM_DBLCLK) {
				LPNMMOUSE mp = (LPNMMOUSE) lParam;
				if (mp->dwItemSpec == 6) {	//	上書き/挿入
					GetDocument().HandleCommand(F_CHGMOD_INS);
				}else if (mp->dwItemSpec == 5) {	//	マクロの記録開始・終了
					GetDocument().HandleCommand(F_RECKEYMACRO);
				}else if (mp->dwItemSpec == 1) {	//	桁位置→行番号ジャンプ
					GetDocument().HandleCommand(F_JUMP_DIALOG);
				}else if (mp->dwItemSpec == 3) {	//	文字コード→各種コード
					ShowCodeBox(GetHwnd(), &GetDocument());
				}else if (mp->dwItemSpec == 4) {	//	文字コードセット→文字コードセット指定
					GetDocument().HandleCommand(F_CHG_CHARSET);
				}
			}else if (pnmh->code == NM_RCLICK) {
				LPNMMOUSE mp = (LPNMMOUSE) lParam;
				if (mp->dwItemSpec == 2) {	//	入力改行モード
					enum eEolExts {
						F_CHGMOD_EOL_NEL = F_CHGMOD_EOL_CR + 1,
						F_CHGMOD_EOL_PS,
						F_CHGMOD_EOL_LS,
					};
					menuDrawer.ResetContents();
					HMENU hMenuPopUp = ::CreatePopupMenu();
					menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CRLF, 
						LS(F_CHGMOD_EOL_CRLF), _T("C")); // 入力改行コード指定(CRLF)
					menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_LF,
						LS(F_CHGMOD_EOL_LF), _T("L")); // 入力改行コード指定(LF)
					menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CR,
						LS(F_CHGMOD_EOL_CR), _T("rec")); // 入力改行コード指定(CR)
					// 拡張EOLが有効の時だけ表示
					if (GetDllShareData().common.edit.bEnableExtEol) {
						menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_NEL,
							LS(STR_EDITWND_MENU_NEL), _T(""), TRUE, -2); // 入力改行コード指定(NEL)
						menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_LS,
							LS(STR_EDITWND_MENU_LS), _T(""), TRUE, -2); // 入力改行コード指定(LS)
						menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_PS,
							LS(STR_EDITWND_MENU_PS), _T(""), TRUE, -2); // 入力改行コード指定(PS)
					}
					
					//	mp->ptはステータスバー内部の座標なので，スクリーン座標への変換が必要
					POINT po = mp->pt;
					::ClientToScreen(statusBar.GetStatusHwnd(), &po);
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

		switch (pnmh->code) {
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
		case NM_CUSTOMDRAW:
			if (pnmh->hwndFrom == toolbar.GetToolbarHwnd()) {
				//	ツールバーのOwner Draw
				return toolbar.ToolBarOwnerDraw((LPNMCUSTOMDRAW)pnmh);
			}
			break;
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
		if (pShareData->flags.bRecordingKeyMacro) {					// キーボードマクロの記録中
			if (pShareData->flags.hwndRecordingKeyMacro == GetHwnd()) {	// キーボードマクロを記録中のウィンドウ
				pShareData->flags.bRecordingKeyMacro = false;			// キーボードマクロの記録中
				pShareData->flags.hwndRecordingKeyMacro = NULL;		// キーボードマクロを記録中のウィンドウ
			}
		}

		// タイマーを削除
		::KillTimer(GetHwnd(), IDT_TOOLBAR);

		// ドロップされたファイルを受け入れるのを解除
		::DragAcceptFiles(hwnd, FALSE);
		pDropTarget->Revoke_DropTarget();	// 右ボタンドロップ用

		// 編集ウィンドウリストからの削除
		AppNodeGroupHandle(GetHwnd()).DeleteEditWndList(GetHwnd());

		if (pShareData->handles.hwndDebug == GetHwnd()) {
			pShareData->handles.hwndDebug = NULL;
		}
		hWnd = NULL;


		// 編集ウィンドウオブジェクトからのオブジェクト削除要求
		::PostMessage(pShareData->handles.hwndTray, MYWM_DELETE_ME, 0, 0);

		// Windows にスレッドの終了を要求します
		::PostQuitMessage(0);

		return 0L;

	case WM_THEMECHANGED:
		// ビジュアルスタイル／クラシックスタイルが切り替わったらツールバーを再作成する
		// （ビジュアルスタイル: Rebar 有り、クラシックスタイル: Rebar 無し）
		if (toolbar.GetToolbarHwnd()) {
			if (IsVisualStyle() == (!toolbar.GetRebarHwnd())) {
				toolbar.DestroyToolBar();
				LayoutToolBar();
				EndLayoutBars();
			}
		}
		return 0L;

	case MYWM_UIPI_CHECK:
		// エディタ－トレイ間でのUI特権分離の確認メッセージ
		bUIPI = TRUE;	// トレイからの返事を受け取った
		return 0L;

	case MYWM_CLOSE:
		// エディタへの終了要求
		if (nRet = OnClose(
				(HWND)lParam,
				(wParam & PM_CLOSE_GREPNOCONFIRM) == PM_CLOSE_GREPNOCONFIRM
			)
		) {
			// プラグイン：DocumentCloseイベント実行
			Plug::Array plugs;
			WSHIfObj::List params;
			JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_CLOSE, 0, &plugs);
			for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
				(*it)->Invoke(GetActiveView(), params);
			}

			// プラグイン：EditorEndイベント実行
			plugs.clear();
			JackManager::getInstance().GetUsablePlug(PP_EDITOR_END, 0, &plugs);
			for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
				(*it)->Invoke(GetActiveView(), params);
			}

			// タブまとめ表示では閉じる動作はオプション指定に従う
			if ((wParam & PM_CLOSE_EXIT) != PM_CLOSE_EXIT) {	// 全終了要求でない場合
				// タブまとめ表示で(無題)を残す指定の場合、残ウィンドウが１個なら新規エディタを起動して終了する
				if (pShareData->common.tabBar.bDispTabWnd &&
					!pShareData->common.tabBar.bDispTabWndMultiWin &&
					pShareData->common.tabBar.bTab_RetainEmptyWin
				) {
					// 自グループ内の残ウィンドウ数を調べる
					int nGroup = AppNodeManager::getInstance().GetEditNode(GetHwnd())->GetGroup();
					if (AppNodeGroupHandle(nGroup).GetEditorWindowsNum() == 1) {
						EditNode* pEditNode = AppNodeManager::getInstance().GetEditNode(GetHwnd());
						if (pEditNode) {
							pEditNode->bClosing = true;	// 自分はタブ表示してもらわなくていい
						}
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
		pfi = (EditInfo*)&pShareData->workBuffer.editInfo_MYWM_GETFILEINFO;

		// 編集ファイル情報を格納
		GetDocument().GetEditInfo(pfi);
		return 0L;
	case MYWM_CHANGESETTING:
		// 設定変更の通知
		switch ((e_PM_CHANGESETTING_SELECT)lParam) {
		case PM_CHANGESETTING_ALL:
			// 言語を選択する
			SelectLang::ChangeLang(GetDllShareData().common.window.szLanguageDll);
			ShareData::getInstance().RefreshString();

			// メインメニュー
			LayoutMainMenu();

			// 設定変更時、ツールバーを再作成するようにする（バーの内容変更も反映）
			toolbar.DestroyToolBar();
			LayoutToolBar();

			// 非アクティブなウィンドウのツールバーを更新する
			// アクティブなウィンドウはタイマにより更新されるが、それ以外のウィンドウは
			// タイマを停止させており設定変更すると全部有効となってしまうため、ここで
			// ツールバーを更新する
			if (!bIsActiveApp)
				toolbar.UpdateToolbar();

			// ファンクションキーを再作成する（バーの内容、位置、グループボタン数の変更も反映）
			funcKeyWnd.Close();
			LayoutFuncKey();

			// タブバーの表示／非表示切り替え
			LayoutTabBar();

			// ステータスバーの表示／非表示切り替え
			LayoutStatusBar();

			// 水平スクロールバーの表示／非表示切り替え
			{
				bool b1 = !pShareData->common.window.bScrollBarHorz;
				for (int i=0; i<GetAllViewCount(); ++i) {
					bool b2 = (GetView(i).hwndHScrollBar == NULL);
					if (b1 != b2) {		// 水平スクロールバーを使う
						GetView(i).DestroyScrollBar();
						GetView(i).CreateScrollBar();
					}
				}
			}

			LayoutMiniMap();

			// バー変更で画面が乱れないように
			EndLayoutBars();

			// アクセラレータテーブルを再作成する(Wine用)
			// ウィンドウ毎に作成したアクセラレータテーブルを破棄する(Wine用)
			DeleteAccelTbl();
			// ウィンドウ毎にアクセラレータテーブルを作成する(Wine用)
			CreateAccelTbl();
			
			if (pShareData->common.tabBar.bDispTabWnd) {
				// タブ表示のままグループ化する／しないが変更されていたらタブを更新する必要がある
				tabWnd.Refresh(false);
			}
			if (pShareData->common.tabBar.bDispTabWnd && !pShareData->common.tabBar.bDispTabWndMultiWin) {
				if (AppNodeManager::getInstance().GetEditNode(GetHwnd())->IsTopInGroup()) {
					if (!::IsWindowVisible(GetHwnd())) {
						// ::ShowWindow(GetHwnd(), SW_SHOWNA) だと非表示から表示に切り替わるときに Z-order がおかしくなることがあるので ::SetWindowPos を使う
						::SetWindowPos(GetHwnd(), NULL, 0, 0, 0, 0,
										SWP_SHOWWINDOW | SWP_NOACTIVATE
										| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);

						// このウィンドウの WS_EX_TOPMOST 状態を全ウィンドウに反映する
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

			GetDocument().autoSaveAgent.ReloadAutoSaveParam();
			GetDocument().OnChangeSetting();	// ビューに設定変更を反映させる
			GetDocument().docType.SetDocumentIcon();	// 文書アイコンの再設定

			break;
		case PM_CHANGESETTING_FONT:
			GetDocument().OnChangeSetting(true);	// フォントで文字幅が変わるので、レイアウト再構築
			break;
		case PM_CHANGESETTING_FONTSIZE:
			if ((wParam == -1 && GetLogfontCacheMode() == CharWidthCacheMode::Share)
				|| GetDocument().docType.GetDocumentType().GetIndex() == wParam
			) {
				GetDocument().OnChangeSetting( false );	// ビューに設定変更を反映させる(レイアウト情報の再作成しない)
			}
			break;
		case PM_CHANGESETTING_TYPE:
			typeNew = DocTypeManager().GetDocumentTypeOfPath(GetDocument().docFile.GetFilePath());
			if (GetDocument().docType.GetDocumentType().GetIndex() == wParam
				|| typeNew.GetIndex() == wParam
			) {
				GetDocument().OnChangeSetting();

				// アウトライン解析画面処理
				bool bAnalyzed = false;
				if (dlgFuncList.GetHwnd() && !bAnalyzed) {	// アウトラインを開いていれば再解析
					// SHOW_NORMAL: 解析方法が変化していれば再解析される。そうでなければ描画更新（変更されたカラーの適用）のみ。
					EFunctionCode nFuncCode = dlgFuncList.GetFuncCodeRedraw(dlgFuncList.nOutlineType);
					GetActiveView().GetCommander().HandleCommand(nFuncCode, true, (LPARAM)ShowDialogType::Normal, 0, 0, 0);
				}
				if (MyGetAncestor(::GetForegroundWindow(), GA_ROOTOWNER2) == GetHwnd())
					::SetFocus(GetActiveView().GetHwnd());	// フォーカスを戻す
			}
			break;
		case PM_CHANGESETTING_TYPE2:
			typeNew = DocTypeManager().GetDocumentTypeOfPath(GetDocument().docFile.GetFilePath());
			if (GetDocument().docType.GetDocumentType().GetIndex() == wParam
				|| typeNew.GetIndex() == wParam
			) {
				// indexのみ更新
				GetDocument().docType.SetDocumentTypeIdx();
				// タイプが変更になった場合は適用する
				if (GetDocument().docType.GetDocumentType().GetIndex() != wParam) {
					::SendMessage(hWnd, MYWM_CHANGESETTING, wParam, PM_CHANGESETTING_TYPE);
				}
			}
			break;
		case PM_PrintSetting:
			{
				if (pPrintPreview) {
					pPrintPreview->OnChangeSetting();
				}
			}
			break;
		default:
			break;
		}
		return 0L;
	case MYWM_SAVEEDITSTATE:
		{
			if (pPrintPreview) {
				// 一時的に設定を戻す
				SelectCharWidthCache(CharWidthFontMode::Edit, CharWidthCacheMode::Neutral);
			}
			// フォント変更前の座標の保存
			posSaveAry = SavePhysPosOfAllView();
			if (pPrintPreview) {
				// 設定を戻す
				SelectCharWidthCache(CharWidthFontMode::Print, CharWidthCacheMode::Local);
			}
		}
		return 0L; 
	case MYWM_SETACTIVEPANE:
		if ((int)wParam == -1) {
			if (lParam == 0) {
				nPane = splitterWnd.GetFirstPane();
			}else {
				nPane = splitterWnd.GetLastPane();
			}
			this->SetActivePane(nPane);
		}
		return 0L;
		
	case MYWM_SETCARETPOS:	// カーソル位置変更通知
		{
			//	LPARAMに新たな意味を追加
			//	bit 0 (MASK 1): (bit 1==0のとき) 0/選択クリア, 1/選択開始・変更
			//	bit 1 (MASK 2): 0: bit 0の設定に従う．1:現在の選択ロックs状態を継続
			//	既存の実装では どちらも0なので強制解除と解釈される．
			//	呼び出し時はe_PM_SETCARETPOS_SELECTSTATEの値を使うこと．
			bool bSelect = ((lParam & 1) != 0);
			if (lParam & 2) {
				// 現在の状態をKEEP
				bSelect = GetActiveView().GetSelectionInfo().bSelectingLock;
			}
			
			/*
			カーソル位置変換
			 物理位置(行頭からのバイト数、折り返し無し行位置)
			→
			 レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/
			Point* ppoCaret = &(pShareData->workBuffer.logicPoint);
			Point ptCaretPos = GetDocument().layoutMgr.LogicToLayout(*ppoCaret);
			// 改行の真ん中にカーソルが来ないように
			// Note. もとが改行単位の桁位置なのでレイアウト折り返しの桁位置を超えることはない。
			//       選択指定(bSelect==TRUE)の場合にはどうするのが妥当かよくわからないが、
			//       2007.08.22現在ではアウトライン解析ダイアログから桁位置0で呼び出される
			//       パターンしかないので実用上特に問題は無い。
			if (!bSelect) {
				const DocLine *pTmpDocLine = GetDocument().docLineMgr.GetLine(ppoCaret->y);
				if (pTmpDocLine) {
					if ((int)pTmpDocLine->GetLengthWithoutEOL() < ppoCaret->x) {
						ptCaretPos.x--;
					}
				}
			}
			//	選択範囲を考慮して移動
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
			pShareData->workBuffer.logicPoint = GetDocument().layoutMgr.LayoutToLogic(
				GetActiveView().GetCaret().GetCaretLayoutPos()
			);
		}
		return 0L;

	case MYWM_GETLINEDATA:	// 行(改行単位)データの要求
	{
		// 共有データ：自分Write→相手Read
		// return 0以上：行データあり。wParamオフセットを除いた行データ長。0はEOFかOffsetがちょうどバッファ長だった
		//       -1以下：エラー
		int	nLineNum = (int)wParam;
		int	nLineOffset = (int)lParam;
		if (nLineNum < 0 || (int)GetDocument().docLineMgr.GetLineCount() < nLineNum) {
			return -2; // 行番号不正。LineCount == nLineNum はEOF行として下で処理
		}
		size_t nLineLen = 0;
		const wchar_t* pLine = GetDocument().docLineMgr.GetLine(nLineNum)->GetDocLineStrWithEOL( &nLineLen );
		if (nLineOffset < 0 || (int)nLineLen < nLineOffset) {
			return -3; // オフセット位置不正
		}
		if (nLineNum == GetDocument().docLineMgr.GetLineCount()) {
			return 0; // EOF正常終了
		}
 		if (!pLine) {
			return -4; // 不明なエラー
		}
		if (nLineLen == nLineOffset) {
 			return 0;
 		}
		pLine = GetDocument().docLineMgr.GetLine(wParam)->GetDocLineStrWithEOL( &nLineLen );
		pLine += nLineOffset;
		nLineLen -= nLineOffset;
		size_t nEnd = t_min<size_t>(nLineLen, pShareData->workBuffer.GetWorkBufferCount<EDIT_CHAR>());
		auto_memcpy( pShareData->workBuffer.GetWorkBuffer<EDIT_CHAR>(), pLine, nEnd );
		return nLineLen;
	}

	case MYWM_ADDSTRINGLEN_W:
		{
			EDIT_CHAR* pWork = pShareData->workBuffer.GetWorkBuffer<EDIT_CHAR>();
			size_t addSize = t_min((size_t)wParam, pShareData->workBuffer.GetWorkBufferCount<EDIT_CHAR>());
			GetActiveView().GetCommander().HandleCommand(F_ADDTAIL_W, true, (LPARAM)pWork, (LPARAM)addSize, 0, 0);
			GetActiveView().GetCommander().HandleCommand(F_GOFILEEND, true, 0, 0, 0, 0);
		}
		return 0L;

	// タブウィンドウ
	case MYWM_TAB_WINDOW_NOTIFY:
		tabWnd.TabWindowNotify(wParam, lParam);
		{
			RECT rc;
			::GetClientRect(GetHwnd(), &rc);
			OnSize2(nWinSizeType, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ), false);
			GetActiveView().SetIMECompFormPos();
		}
		return 0L;

	// アウトライン
	case MYWM_OUTLINE_NOTIFY:
		dlgFuncList.OnOutlineNotify(wParam, lParam);
		return 0L;

	// バーの表示・非表示
	case MYWM_BAR_CHANGE_NOTIFY:
		if (GetHwnd() != (HWND)lParam) {
			switch ((BarChangeNotifyType)wParam) {
			case BarChangeNotifyType::Toolbar:
				LayoutToolBar();
				break;
			case BarChangeNotifyType::FuncKey:
				LayoutFuncKey();
				break;
			case BarChangeNotifyType::Tab:
				LayoutTabBar();
				if (pShareData->common.tabBar.bDispTabWnd
					&& !pShareData->common.tabBar.bDispTabWndMultiWin
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
				LayoutStatusBar();
				break;
			case BarChangeNotifyType::MiniMap:
				LayoutMiniMap();
				break;
			}
			EndLayoutBars();
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
		if (!statusBar.GetStatusHwnd()) {
			PrintMenubarMessage(NULL);
		}
		return 0;

	case WM_NCACTIVATE:
		// 編集ウィンドウ切替中（タブまとめ時）はタイトルバーのアクティブ／非アクティブ状態をできるだけ変更しないように（１）
		// 前面にいるのが編集ウィンドウならアクティブ状態を保持する
		if (pShareData->flags.bEditWndChanging && IsSakuraMainWindow(::GetForegroundWindow())) {
			wParam = TRUE;	// アクティブ
		}
		lRes = DefWindowProc(hwnd, uMsg, wParam, lParam);
		if (!statusBar.GetStatusHwnd()) {
			PrintMenubarMessage(NULL);
		}
		return lRes;

	case WM_SETTEXT:
		// 編集ウィンドウ切替中（タブまとめ時）はタイトルバーのアクティブ／非アクティブ状態をできるだけ変更しないように（２）
		// タイマーを使用してタイトルの変更を遅延する
		if (pShareData->flags.bEditWndChanging) {
			delete[] pszLastCaption;
			pszLastCaption = new TCHAR[::_tcslen((LPCTSTR)lParam) + 1];
			::_tcscpy(pszLastCaption, (LPCTSTR)lParam);	// 変更後のタイトルを記憶しておく
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
	int nRet = GetDocument().OnFileClose(bGrepNoConfirm);
	if (!nRet) {
		return nRet;
	}
	// パラメータでハンドルを貰う様にしたので検索を削除	2013/4/9 Uchi
	if (hWndActive) {
		// アクティブ化制御ウィンドウをアクティブ化する
		if (IsSakuraMainWindow(hWndActive)) {
			ActivateFrameWindow(hWndActive);	// エディタ
		}else {
			::SetForegroundWindow(hWndActive);	// タスクトレイ
		}
	}

	return nRet;
}


/*! WM_COMMAND処理 */
void EditWnd::OnCommand(WORD wNotifyCode, WORD wID , HWND hwndCtl)
{
	// 検索ボックスからの WM_COMMAND はすべてコンボボックス通知
	// ##### 検索ボックス処理はツールバー側の WindowProc に集約するほうがスマートかも
	if (toolbar.GetSearchHwnd() && hwndCtl == toolbar.GetSearchHwnd()) {
		switch (wNotifyCode) {
		case CBN_SETFOCUS:
			nCurrentFocus = F_SEARCH_BOX;
			break;
		case CBN_KILLFOCUS:
			nCurrentFocus = 0;
			// フォーカスがはずれたときに検索キーにしてしまう。
			// 検索キーワードを取得
			std::wstring	strText;
			if (toolbar.GetSearchKey(strText)) {	// キー文字列がある
				// 検索キーを登録
				if (strText.length() < _MAX_PATH) {
					SearchKeywordManager().AddToSearchKeys(strText.c_str());
				}
				GetActiveView().strCurSearchKey = strText;
				GetActiveView().bCurSearchUpdate = true;
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
		if (wID - IDM_SELWINDOW >= 0 && wID - IDM_SELWINDOW < (int)pShareData->nodes.nEditArrNum) {
			ActivateFrameWindow(pShareData->nodes.pEditArr[wID - IDM_SELWINDOW].GetHwnd());
		}
		// 最近使ったファイル
		else if (wID - IDM_SELMRU >= 0 && wID - IDM_SELMRU < 999) {
			// 指定ファイルが開かれているか調べる
			const MruFile mru;
			EditInfo checkEditInfo;
			mru.GetEditInfo(wID - IDM_SELMRU, &checkEditInfo);
			LoadInfo loadInfo(checkEditInfo.szPath, checkEditInfo.nCharCode, false);
			GetDocument().docFileOperation.FileLoad(&loadInfo);	//	Oct.  9, 2004 genta 共通関数化
		}
		// 最近使ったフォルダ
		else if (wID - IDM_SELOPENFOLDER >= 0 && wID - IDM_SELOPENFOLDER < 999) {
			// フォルダ取得
			const MruFolder mruFolder;
			LPCTSTR pszFolderPath = mruFolder.GetPath(wID - IDM_SELOPENFOLDER);

			// UNCであれば接続を試みる
			NetConnect(pszFolderPath);

			//「ファイルを開く」ダイアログ
			LoadInfo loadInfo(_T(""), CODE_AUTODETECT, false);
			DocFileOperation& docOp = GetDocument().docFileOperation;
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
			if (wID != F_SEARCH_BOX && nCurrentFocus == F_SEARCH_BOX) {
				::SetFocus(GetActiveView().GetHwnd());
			}

			// コマンドコードによる処理振り分け
			//	May 19, 2006 genta 上位ビットを渡す
			//	Jul. 7, 2007 genta 上位ビットを定数に
			GetDocument().HandleCommand((EFunctionCode)(wID | 0));
		}
		break;
	// アクセラレータからのメッセージ
	case 1:
		{
			// ビューにフォーカスを移動しておく
			if (wID != F_SEARCH_BOX && nCurrentFocus == F_SEARCH_BOX)
				::SetFocus(GetActiveView().GetHwnd());
			auto& csKeyBind = pShareData->common.keyBind;
			EFunctionCode nFuncCode = KeyBind::GetFuncCode(
				wID,
				csKeyBind.nKeyNameArrNum,
				csKeyBind.pKeyNameArr
			);
			GetDocument().HandleCommand((EFunctionCode)(nFuncCode | FA_FROMKEYBOARD));
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
		const CommonSetting_MainMenu*	pMenu = &pShareData->common.mainMenu;
		const MainMenu*	pMainMenu;
		int		nIdxStr;
		int		nIdxEnd;
		int		nLv;
		std::vector<HMENU>	hSubMenu;
		std::wstring tmpMenuName;
		const wchar_t* pMenuName;

		nIdxStr = pMenu->nMenuTopIdx[uPos];
		nIdxEnd = (uPos < MAX_MAINMENU_TOP) ? pMenu->nMenuTopIdx[uPos + 1] : -1;
		if (nIdxEnd < 0) {
			nIdxEnd = pMenu->nMainMenuNum;
		}

		// メニュー 初期化
		menuDrawer.ResetContents();
		numMenuItems = ::GetMenuItemCount(hMenu);
		for (int i=numMenuItems-1; i>=0; --i) {
			::DeleteMenu(hMenu, i, MF_BYPOSITION);
		}

		// メニュー作成
		hSubMenu.push_back(hMenu);
		nLv = 1;
		if (pMenu->mainMenuTbl[nIdxStr].type == MainMenuType::Special) {
			nLv = 0;
			--nIdxStr;
		}
		for (int i=nIdxStr+1; i<nIdxEnd; ++i) {
			pMainMenu = &pMenu->mainMenuTbl[i];
			if (pMainMenu->nLevel != nLv) {
				nLv = pMainMenu->nLevel;
				if (hSubMenu.size() < (size_t)nLv) {
					// 保護
					break;
				}
				hMenu = hSubMenu[nLv-1];
			}
			switch (pMainMenu->type) {
			case MainMenuType::Node:
				hMenuPopUp = ::CreatePopupMenu();
				if (pMainMenu->nFunc != 0 && pMainMenu->sName[0] == L'\0') {
					// ストリングテーブルから読み込み
					tmpMenuName = LSW(pMainMenu->nFunc);
					if (MAX_MAIN_MENU_NAME_LEN < tmpMenuName.length()) {
						tmpMenuName = tmpMenuName.substr(0, MAX_MAIN_MENU_NAME_LEN);
					}
					pMenuName = tmpMenuName.c_str();
				}else {
					pMenuName = pMainMenu->sName;
				}
				menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , 
					pMenuName, pMainMenu->sKey);
				if (hSubMenu.size() > (size_t)nLv) {
					hSubMenu[nLv] = hMenuPopUp;
				}else {
					hSubMenu.push_back(hMenuPopUp);
				}
				break;
			case MainMenuType::Leaf:
				InitMenu_Function(hMenu, pMainMenu->nFunc, pMainMenu->sName, pMainMenu->sKey);
				break;
			case MainMenuType::Separator:
				menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
				break;
			case MainMenuType::Special:
				bool	bInList;		// リストが1個以上ある
				bInList = InitMenu_Special(hMenu, pMainMenu->nFunc);
				// リストが無い場合の処理
				if (!bInList) {
					// 分割線に囲まれ、かつリストなし ならば 次の分割線をスキップ
					if ((i == nIdxStr + 1
						  || (pMenu->mainMenuTbl[i - 1].type == MainMenuType::Separator 
							&& pMenu->mainMenuTbl[i - 1].nLevel == pMainMenu->nLevel))
						&& i + 1 < nIdxEnd
						&& pMenu->mainMenuTbl[i + 1].type == MainMenuType::Separator 
						&& pMenu->mainMenuTbl[i + 1].nLevel == pMainMenu->nLevel) {
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

//@@@ 2002.01.14 YAZAKI 印刷PreviewをPrintPreviewに独立させたことによる変更
//	if (pPrintPreview)	return;	//	印刷Previewモードなら排除。（おそらく排除しなくてもいいと思うんだけど、念のため）

	// 機能が利用可能かどうか、チェック状態かどうかを一括チェック
	numMenuItems = ::GetMenuItemCount(hMenu);
	for (nPos=0; nPos<numMenuItems; ++nPos) {
		EFunctionCode	id = (EFunctionCode)::GetMenuItemID(hMenu, nPos);
		// 機能が利用可能か調べる
		//	Jan.  8, 2006 genta 機能が有効な場合には明示的に再設定しないようにする．
		if (!IsFuncEnable(GetDocument(), *pShareData, id)) {
			UINT fuFlags = MF_BYCOMMAND | MF_GRAYED;
			::EnableMenuItem(hMenu, id, fuFlags);
		}

		// 機能がチェック状態か調べる
		if (IsFuncChecked(GetDocument(), *pShareData, id)) {
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
		if (pShareData->common.customMenu.nCustMenuItemNumArr[j] > 0) {
			nFlag = MF_BYPOSITION | MF_STRING;
		}
		wchar_t buf[MAX_CUSTOM_MENU_NAME_LEN + 1];
		menuDrawer.MyAppendMenu(hMenu, nFlag,
			eFunc, GetDocument().funcLookup.Custmenu2Name(j, buf, _countof(buf)), pszKey);
	}
	// マクロ
	else if (eFunc >= F_USERMACRO_0 && eFunc < F_USERMACRO_0 + MAX_CUSTMACRO) {
		MacroRec *mp = &pShareData->common.macro.macroTable[eFunc - F_USERMACRO_0];
		if (mp->IsEnabled()) {
			psName = to_wchar(mp->szName[0] ? mp->szName : mp->szFile);
			menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING,
				eFunc, psName, pszKey);
		}else {
			psName = L"-- undefined macro --";
			menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED,
				eFunc, psName, pszKey);
		}
	}
	// プラグインコマンド
	else if (eFunc >= F_PLUGCOMMAND_FIRST && eFunc < F_PLUGCOMMAND_LAST) {
		wchar_t szLabel[256];
		if (0 < JackManager::getInstance().GetCommandName( eFunc, szLabel, _countof(szLabel) )) {
			menuDrawer.MyAppendMenu(
				hMenu, MF_BYPOSITION | MF_STRING,
				eFunc, szLabel, pszKey,
				TRUE, eFunc
			);
		}else {
			// not found
			psName = L"-- undefined plugin command --";
			menuDrawer.MyAppendMenu(
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
				!pShareData->flags.bRecordingKeyMacro);
			break;
		case F_SPLIT_V:	
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				splitterWnd.GetAllSplitRows() == 1);
			break;
		case F_SPLIT_H:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				splitterWnd.GetAllSplitCols() == 1);
			break;
		case F_SPLIT_VH:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				splitterWnd.GetAllSplitRows() == 1 || splitterWnd.GetAllSplitCols() == 1);
			break;
		case F_TAB_CLOSEOTHER:
			SetMenuFuncSel(hMenu, eFunc, pszKey, pShareData->common.tabBar.bDispTabWnd);
			break;
		case F_TOPMOST:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				((DWORD)::GetWindowLongPtr(GetHwnd(), GWL_EXSTYLE) & WS_EX_TOPMOST) == 0);
			break;
		case F_BIND_WINDOW:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				(!pShareData->common.tabBar.bDispTabWnd 
				|| pShareData->common.tabBar.bDispTabWndMultiWin));
			break;
		case F_SHOWTOOLBAR:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!pShareData->common.window.bMenuIcon | !toolbar.GetToolbarHwnd());
			break;
		case F_SHOWFUNCKEY:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!pShareData->common.window.bMenuIcon | !funcKeyWnd.GetHwnd());
			break;
		case F_SHOWTAB:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!pShareData->common.window.bMenuIcon | !tabWnd.GetHwnd());
			break;
		case F_SHOWSTATUSBAR:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!pShareData->common.window.bMenuIcon | !statusBar.GetStatusHwnd());
			break;
		case F_SHOWMINIMAP:
			SetMenuFuncSel( hMenu, eFunc, pszKey, 
				!pShareData->common.window.bMenuIcon | !GetMiniMap().GetHwnd() );
			break;
		case F_TOGGLE_KEY_SEARCH:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!pShareData->common.window.bMenuIcon | !IsFuncChecked(GetDocument(), *pShareData, F_TOGGLE_KEY_SEARCH));
			break;
		case F_WRAPWINDOWWIDTH:
			{
				int ketas;
				wchar_t*	pszLabel;
				EditView::TOGGLE_WRAP_ACTION mode = GetActiveView().GetWrapMode(&ketas);
				if (mode == EditView::TGWRAP_NONE) {
					menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_WRAPWINDOWWIDTH , L"", pszKey);
				}else {
					wchar_t szBuf[60];
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
								GetActiveView().GetTextArea().nViewColNum
							)
						);
					}else {
						auto_sprintf_s(
							szBuf,
							LSW(STR_WRAP_WIDTH_FIXED),	//L"折り返し桁数: %d 桁（指定）",
							GetDocument().docType.GetDocumentAttribute().nMaxLineKetas
						);
					}
					menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_WRAPWINDOWWIDTH , pszLabel, pszKey);
				}
			}
			break;
		default:
			menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, eFunc, 
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
			size_t nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true);
			WinListMenu(hMenu, pEditNodeArr, nRowNum, false);
			bInList = (nRowNum > 0);
			delete[] pEditNodeArr;
		}
		break;
	case F_FILE_USED_RECENTLY:		// 最近使ったファイル
		// MRUリストのファイルのリストをメニューにする
		{
			//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
			const MruFile mru;
			mru.CreateMenu(hMenu, menuDrawer);	//	ファイルメニュー
			bInList = (mru.MenuLength() > 0);
		}
		break;
	case F_FOLDER_USED_RECENTLY:	// 最近使ったフォルダ
		// 最近使ったフォルダのメニューを作成
		{
			//@@@ 2001.12.26 YAZAKI OPENFOLDERリストは、MruFolderにすべて依頼する
			const MruFolder mruFolder;
			mruFolder.CreateMenu(hMenu, menuDrawer);
			bInList = (mruFolder.MenuLength() > 0);
		}
		break;
	case F_CUSTMENU_LIST:			// カスタムメニューリスト
		wchar_t buf[MAX_CUSTOM_MENU_NAME_LEN + 1];
		//	右クリックメニュー
		if (pShareData->common.customMenu.nCustMenuItemNumArr[0] > 0) {
			 menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING,
				 F_MENU_RBUTTON, GetDocument().funcLookup.Custmenu2Name(0, buf, _countof(buf)), L"");
			bInList = true;
		}
		// カスタムメニュー
		for (int j=1; j<MAX_CUSTOM_MENU; ++j) {
			if (pShareData->common.customMenu.nCustMenuItemNumArr[j] > 0) {
				 menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING,
			 		F_CUSTMENU_BASE + j, GetDocument().funcLookup.Custmenu2Name(j, buf, _countof(buf)), L"" );
				bInList = true;
			}
		}
		break;
	case F_USERMACRO_LIST:			// 登録済みマクロリスト
		for (int j=0; j<MAX_CUSTMACRO; ++j) {
			MacroRec *mp = &pShareData->common.macro.macroTable[j];
			if (mp->IsEnabled()) {
				if (mp->szName[0]) {
					menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_USERMACRO_0 + j, mp->szName, _T(""));
				}else {
					menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_USERMACRO_0 + j, mp->szFile, _T(""));
				}
				bInList = true;
			}
		}
		break;
	case F_PLUGIN_LIST:				// プラグインコマンドリスト
		// プラグインコマンドを提供するプラグインを列挙する
		{
			auto& jackManager = JackManager::getInstance();
			const Plugin* prevPlugin = nullptr;
			HMENU hMenuPlugin = 0;

			Plug::Array plugs = jackManager.GetPlugs(PP_COMMAND);
			for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
				const Plugin* curPlugin = &(*it)->plugin;
				if (curPlugin != prevPlugin) {
					// プラグインが変わったらプラグインポップアップメニューを登録
					hMenuPlugin = ::CreatePopupMenu();
					menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPlugin, curPlugin->sName.c_str(), L"");
					prevPlugin = curPlugin;
				}

				// コマンドを登録
				menuDrawer.MyAppendMenu(hMenuPlugin, MF_BYPOSITION | MF_STRING,
					(*it)->GetFunctionCode(), to_tchar((*it)->sLabel.c_str()), _T(""),
					TRUE, (*it)->GetFunctionCode());
			}
			bInList = (prevPlugin != nullptr);
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
void EditWnd::SetMenuFuncSel(HMENU hMenu, EFunctionCode nFunc, const wchar_t* sKey, bool flag)
{
	const wchar_t* sName = L"";
	for (size_t i=0; i<_countof(gFuncMenuName); ++i) {
		if (gFuncMenuName[i].eFunc == nFunc) {
			sName = flag ? LSW(gFuncMenuName[i].nNameId[0]) : LSW(gFuncMenuName[i].nNameId[1]);
		}
	}
	assert(auto_strlen(sName));

	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, nFunc, sName, sKey);
}


STDMETHODIMP EditWnd::DragEnter(LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect)
{
	if (!pDataObject || !pdwEffect) {
		return E_INVALIDARG;
	}

	// 右ボタンファイルドロップの場合だけ処理する
	if (!((dwKeyState & MK_RBUTTON) && IsDataAvailable(pDataObject, CF_HDROP))) {
		*pdwEffect = DROPEFFECT_NONE;
		return E_INVALIDARG;
	}

	// 印刷Previewでは受け付けない
	if (pPrintPreview) {
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
	UINT cFiles = ::DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	// ファイルをドロップしたときは閉じて開く
	auto& csFile = pShareData->common.file;
	if (csFile.bDropFileAndClose) {
		cFiles = 1;
	}
	// 一度にドロップ可能なファイル数
	if (cFiles > csFile.nDropFileNumMax) {
		cFiles = csFile.nDropFileNumMax;
	}

	// アクティブにする
	ActivateFrameWindow(GetHwnd());

	for (size_t i=0; i<cFiles; ++i) {
		// ファイルパス取得、解決。
		TCHAR szFile[_MAX_PATH + 1];
		::DragQueryFile(hDrop, i, szFile, _countof(szFile));
		SakuraEnvironment::ResolvePath(szFile);

		// 指定ファイルが開かれているか調べる
		HWND hWndOwner;
		if (ShareData::getInstance().IsPathOpened(szFile, &hWndOwner)) {
			::SendMessage(hWndOwner, MYWM_GETFILEINFO, 0, 0);
			EditInfo* pfi = (EditInfo*)&pShareData->workBuffer.editInfo_MYWM_GETFILEINFO;
			// アクティブにする
			ActivateFrameWindow(hWndOwner);
			// MRUリストへの登録
			MruFile mru;
			mru.Add(pfi);
		}else {
			// 変更フラグがオフで、ファイルを読み込んでいない場合
			if (GetDocument().IsAcceptLoad()) {
				// ファイル読み込み
				LoadInfo loadInfo(szFile, CODE_AUTODETECT, false);
				GetDocument().docFileOperation.FileLoad(&loadInfo);
			}else {
				// ファイルをドロップしたときは閉じて開く
				if (csFile.bDropFileAndClose) {
					// ファイル読み込み
					LoadInfo loadInfo(szFile, CODE_AUTODETECT, false);
					GetDocument().docFileOperation.FileCloseOpen(loadInfo);
				}else {
					// 編集ウィンドウの上限チェック
					if (pShareData->nodes.nEditArrNum >= MAX_EDITWINDOWS) {	// 最大値修正
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

/*! WM_TIMER 処理 */
LRESULT EditWnd::OnTimer(WPARAM wParam, LPARAM lParam)
{
	// タイマー ID で処理を振り分ける
	switch (wParam) {
	case IDT_EDIT:
		OnEditTimer();
		break;
	case IDT_TOOLBAR:
		toolbar.OnToolbarTimer();
		break;
	case IDT_CAPTION:
		OnCaptionTimer();
		break;
	case IDT_SYSMENU:
		OnSysMenuTimer();
		break;
	case IDT_FIRST_IDLE:
		dlgFuncList.bEditWndReady = true;	// エディタ画面の準備完了
		AppNodeGroupHandle(0).PostMessageToAllEditors(MYWM_FIRST_IDLE, ::GetCurrentProcessId(), 0, NULL);	// プロセスの初回アイドリング通知
		::PostMessage(pShareData->handles.hwndTray, MYWM_FIRST_IDLE, (WPARAM)::GetCurrentProcessId(), (LPARAM)0);
		::KillTimer(hWnd, wParam);
		break;
	default:
		return 1L;
	}

	return 0L;
}


/*! キャプション更新用タイマーの処理 */
void EditWnd::OnCaptionTimer(void)
{
	// 編集画面の切替（タブまとめ時）が終わっていたらタイマーを終了してタイトルバーを更新する
	// まだ切替中ならタイマー継続
	if (!pShareData->flags.bEditWndChanging) {
		::KillTimer(GetHwnd(), IDT_CAPTION);
		::SetWindowText(GetHwnd(), pszLastCaption);
	}
}

/*! システムメニュー表示用タイマーの処理 */
void EditWnd::OnSysMenuTimer(void)
{
	::KillTimer(GetHwnd(), IDT_SYSMENU);

	if (iconClicked == IconClickStatus::Clicked) {
		ReleaseCapture();

		// システムメニュー表示
		RECT rec;
		GetWindowRect(GetHwnd(), &rec);
		POINT pt;
		pt.x = rec.left + GetSystemMetrics(SM_CXFRAME);
		pt.y = rec.top + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
		GetMonitorWorkRect(pt, &rec);
		::PostMessage(
			GetHwnd(),
			0x0313, // 右クリックでシステムメニューを表示する際に送信するモノらしい
			0,
			MAKELPARAM((pt.x > rec.left)? pt.x: rec.left, (pt.y < rec.bottom)? pt.y: rec.bottom)
		);
	}
	iconClicked = IconClickStatus::None;
}

// 印刷Previewモードのオン/オフ
void EditWnd::PrintPreviewModeONOFF(void)
{
	// Rebar があればそれをツールバー扱いする
	HWND hwndToolBar = toolbar.GetRebarHwnd() ? toolbar.GetRebarHwnd(): toolbar.GetToolbarHwnd();

	// 印刷Previewモードか
	if (pPrintPreview) {
		// 印刷Previewモードを解除します。
		delete pPrintPreview;	//	削除。
		pPrintPreview = nullptr;	//	nullptrか否かで、プリントPreviewモードか判断するため。

		// 通常モードに戻す
		::ShowWindow(this->splitterWnd.GetHwnd(), SW_SHOW);
		::ShowWindow(hwndToolBar, SW_SHOW);
		::ShowWindow(statusBar.GetStatusHwnd(), SW_SHOW);
		::ShowWindow(funcKeyWnd.GetHwnd(), SW_SHOW);
		::ShowWindow(tabWnd.GetHwnd(), SW_SHOW);
		::ShowWindow(dlgFuncList.GetHwnd(), SW_SHOW);

		// その他のモードレスダイアログも戻す
		::ShowWindow(dlgFind.GetHwnd(), SW_SHOW);
		::ShowWindow(dlgReplace.GetHwnd(), SW_SHOW);
		::ShowWindow(dlgGrep.GetHwnd(), SW_SHOW);

		::SetFocus(GetHwnd());

		LayoutMainMenu();

		::InvalidateRect(GetHwnd(), NULL, TRUE);
	}else {
		// 通常モードを隠す
		HMENU hMenu = ::GetMenu(GetHwnd());
		// Print Previewではメニューを削除
		::SetMenu(GetHwnd(), NULL);
		::DestroyMenu(hMenu);
		::DrawMenuBar(GetHwnd());

		::ShowWindow(this->splitterWnd.GetHwnd(), SW_HIDE);
		::ShowWindow(hwndToolBar, SW_HIDE);
		::ShowWindow(statusBar.GetStatusHwnd(), SW_HIDE);
		::ShowWindow(funcKeyWnd.GetHwnd(), SW_HIDE);
		::ShowWindow(tabWnd.GetHwnd(), SW_HIDE);
		::ShowWindow(dlgFuncList.GetHwnd(), SW_HIDE);

		// その他のモードレスダイアログも隠す
		::ShowWindow(dlgFind.GetHwnd(), SW_HIDE);
		::ShowWindow(dlgReplace.GetHwnd(), SW_HIDE);
		::ShowWindow(dlgGrep.GetHwnd(), SW_HIDE);

		pPrintPreview = new PrintPreview(*this);
		// 現在の印刷設定
		pPrintPreview->SetPrintSetting(
			&pShareData->printSettingArr[
				GetDocument().docType.GetDocumentAttribute().nCurrentPrintSetting]
		);

		// プリンタの情報を取得。

		// 現在のデフォルトプリンタの情報を取得
		BOOL bRes = pPrintPreview->GetDefaultPrinterInfo();
		if (!bRes) {
			TopInfoMessage(GetHwnd(), LS(STR_ERR_DLGEDITWND14));
			return;
		}

		// 印刷設定の反映
		pPrintPreview->OnChangePrintSetting();
		::InvalidateRect(GetHwnd(), NULL, TRUE);
		::UpdateWindow(GetHwnd() /* pPrintPreview->GetPrintPreviewBarHANDLE() */);

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

	int cx = LOWORD(lParam);
	int cy = HIWORD(lParam);
	auto& csWindow = pShareData->common.window;
	
	// ウィンドウサイズ継承
	if (wParam != SIZE_MINIMIZED) {						// 最小化は継承しない
		if (csWindow.eSaveWindowSize == WinSizeMode::Save) {		// ウィンドウサイズ継承をするか
			if (wParam == SIZE_MAXIMIZED) {					// 最大化はサイズを記録しない
				if (csWindow.nWinSizeType != (int)wParam) {
					csWindow.nWinSizeType = (int)wParam;
				}
			}else {
				// Aero Snapの縦方向最大化状態で終了して次回起動するときは元のサイズにする必要があるので、
				// GetWindowRect()ではなくGetWindowPlacement()で得たワークエリア座標をスクリーン座標に変換して記憶する
				WINDOWPLACEMENT wp;
				wp.length = sizeof(wp);
				::GetWindowPlacement(GetHwnd(), &wp);	// ワークエリア座標
				RECT rcWin = wp.rcNormalPosition;
				RECT rcWork, rcMon;
				GetMonitorWorkRect(GetHwnd(), &rcWork, &rcMon);
				::OffsetRect(&rcWin, rcWork.left - rcMon.left, rcWork.top - rcMon.top);	// スクリーン座標に変換
				// ウィンドウサイズに関するデータが変更されたか
				if (csWindow.nWinSizeType != (int)wParam ||
					csWindow.nWinSizeCX != rcWin.right - rcWin.left ||
					csWindow.nWinSizeCY != rcWin.bottom - rcWin.top
				) {
					csWindow.nWinSizeType = (int)wParam;
					csWindow.nWinSizeCX = rcWin.right - rcWin.left;
					csWindow.nWinSizeCY = rcWin.bottom - rcWin.top;
				}
			}
		}

		// 元に戻すときのサイズ種別を記憶
		EditNode* p = AppNodeManager::getInstance().GetEditNode(GetHwnd());
		if (p) {
			p->showCmdRestore = ::IsZoomed(p->GetHwnd())? SW_SHOWMAXIMIZED: SW_SHOWNORMAL;
		}
	}

	nWinSizeType = (int)wParam;	// サイズ変更のタイプ

	// Rebar があればそれをツールバー扱いする
	HWND hwndToolBar = toolbar.GetRebarHwnd() ? toolbar.GetRebarHwnd(): toolbar.GetToolbarHwnd();
	int nToolBarHeight = 0;
	if (hwndToolBar) {
		::SendMessage(hwndToolBar, WM_SIZE, wParam, lParam);
		::GetWindowRect(hwndToolBar, &rc);
		nToolBarHeight = rc.bottom - rc.top;
	}
	int nFuncKeyWndHeight = 0;
	if (funcKeyWnd.GetHwnd()) {
		::SendMessage(funcKeyWnd.GetHwnd(), WM_SIZE, wParam, lParam);
		::GetWindowRect(funcKeyWnd.GetHwnd(), &rc);
		nFuncKeyWndHeight = rc.bottom - rc.top;
	}
	bool bMiniMapSizeBox = true;
	if (wParam == SIZE_MAXIMIZED) {
		bMiniMapSizeBox = false;
	}
	int nStatusBarHeight = 0;
	if (statusBar.GetStatusHwnd()) {
		::SendMessage(statusBar.GetStatusHwnd(), WM_SIZE, wParam, lParam);
		::GetClientRect(statusBar.GetStatusHwnd(), &rc);
		int		nStArr[8];
		// ※pszLabel[3]: ステータスバー文字コード表示領域は大きめにとっておく
		const TCHAR*	pszLabel[7] = { _T(""), _T("99999 行 9999 列"), _T("CRLF"), _T("AAAAAAAAAAAA"), _T("Unicode BOM付"), _T("REC"), _T("上書") };	// Oct. 30, 2000 JEPRO 千万行も要らん	文字コード枠を広げる 2008/6/21	Uchi
		int		nStArrNum = 7;
		int		nAllWidth = rc.right - rc.left;
		int		nSbxWidth = ::GetSystemMetrics(SM_CXVSCROLL) + ::GetSystemMetrics(SM_CXEDGE); // サイズボックスの幅
		int		nBdrWidth = ::GetSystemMetrics(SM_CXSIZEFRAME) + ::GetSystemMetrics(SM_CXEDGE) * 2; // 境界の幅
		SIZE	sz;
		// 正確な幅を計算するために、表示フォントを取得してhdcに選択させる。
		HDC hdc = ::GetDC(statusBar.GetStatusHwnd());
		HFONT hFont = (HFONT)::SendMessage(statusBar.GetStatusHwnd(), WM_GETFONT, 0, 0);
		if (hFont) {
			hFont = (HFONT)::SelectObject(hdc, hFont);
		}
		nStArr[nStArrNum - 1] = nAllWidth;
		if (wParam != SIZE_MAXIMIZED) {
			nStArr[nStArrNum - 1] -= nSbxWidth;
		}
		for (int i=nStArrNum-1; i>0; --i) {
			::GetTextExtentPoint32(hdc, pszLabel[i], _tcslen(pszLabel[i]), &sz);
			nStArr[i - 1] = nStArr[i] - (sz.cx + nBdrWidth);
		}

		//	初期状態ではすべての部分が「枠あり」だが，メッセージエリアは枠を描画しないようにしている
		//	ため，初期化時の枠が変な風に残ってしまう．初期状態で枠を描画させなくするため，
		//	最初に「枠無し」状態を設定した後でバーの分割を行う．
		if (bUpdateStatus) {
			statusBar.SetStatusText(0, SBT_NOBORDERS, _T(""));
		}

		StatusBar_SetParts(statusBar.GetStatusHwnd(), nStArrNum, nStArr);
		if (hFont) {
			::SelectObject(hdc, hFont);
		}
		::ReleaseDC(statusBar.GetStatusHwnd(), hdc);

		::UpdateWindow(statusBar.GetStatusHwnd());	// 即時描画でちらつきを減らす
		::GetWindowRect(statusBar.GetStatusHwnd(), &rc);
		nStatusBarHeight = rc.bottom - rc.top;
		bMiniMapSizeBox = false;
	}
	RECT rcClient;
	::GetClientRect(GetHwnd(), &rcClient);

	// タブウィンドウ
	int nTabHeightBottom = 0;
	int nTabWndHeight = 0;
	if (tabWnd.GetHwnd()) {
		// タブ多段はSizeBox/ウィンドウ幅で高さが変わる可能性がある
		TabPosition tabPosition = pShareData->common.tabBar.eTabPosition;
		bool bHidden = false;
		if (tabPosition == TabPosition::Top) {
			// 上から下に移動するとゴミが表示されるので一度非表示にする
			if (tabWnd.eTabPosition != TabPosition::None && tabWnd.eTabPosition != TabPosition::Top) {
				bHidden = true;
				::ShowWindow( tabWnd.GetHwnd(), SW_HIDE );
			}
			tabWnd.SizeBox_ONOFF( false );
			::GetWindowRect( tabWnd.GetHwnd(), &rc );
			nTabWndHeight = rc.bottom - rc.top;
			if (csWindow.nFuncKeyWnd_Place == 0) {
				::MoveWindow(tabWnd.GetHwnd(), 0, nToolBarHeight + nFuncKeyWndHeight, cx, nTabWndHeight, TRUE);
			}else {
				::MoveWindow(tabWnd.GetHwnd(), 0, nToolBarHeight, cx, nTabWndHeight, TRUE);
			}
			tabWnd.OnSize();
			::GetWindowRect( tabWnd.GetHwnd(), &rc );
			if (nTabWndHeight != rc.bottom - rc.top) {
				nTabWndHeight = rc.bottom - rc.top;
				if (csWindow.nFuncKeyWnd_Place == 0) {
					::MoveWindow( tabWnd.GetHwnd(), 0, nToolBarHeight + nFuncKeyWndHeight, cx, nTabWndHeight, TRUE );
				}else {
					::MoveWindow( tabWnd.GetHwnd(), 0, nToolBarHeight, cx, nTabWndHeight, TRUE );
				}
			}
		}else if (tabPosition == TabPosition::Bottom) {
			// 上から下に移動するとゴミが表示されるので一度非表示にする
			if (tabWnd.eTabPosition != TabPosition::None && tabWnd.eTabPosition != TabPosition::Bottom) {
				bHidden = true;
				ShowWindow( tabWnd.GetHwnd(), SW_HIDE );
			}
			bool bSizeBox = true;
			if (statusBar.GetStatusHwnd()) {
				bSizeBox = false;
			}
			if (funcKeyWnd.GetHwnd()) {
				if (csWindow.nFuncKeyWnd_Place == 1 ){
					bSizeBox = false;
				}
			}
			if (wParam == SIZE_MAXIMIZED) {
				bSizeBox = false;
			}
			tabWnd.SizeBox_ONOFF( bSizeBox );
			::GetWindowRect( tabWnd.GetHwnd(), &rc );
			nTabWndHeight = rc.bottom - rc.top;
			::MoveWindow( tabWnd.GetHwnd(), 0,
				cy - nFuncKeyWndHeight - nStatusBarHeight - nTabWndHeight, cx, nTabWndHeight, TRUE );
			tabWnd.OnSize();
			::GetWindowRect( tabWnd.GetHwnd(), &rc );
			if (nTabWndHeight != rc.bottom - rc.top) {
				nTabWndHeight = rc.bottom - rc.top;
				::MoveWindow( tabWnd.GetHwnd(), 0,
					cy - nFuncKeyWndHeight - nStatusBarHeight - nTabWndHeight, cx, nTabWndHeight, TRUE );
			}
			nTabHeightBottom = rc.bottom - rc.top;
			nTabWndHeight = 0;
			bMiniMapSizeBox = false;
		}
		if (bHidden) {
			::ShowWindow( tabWnd.GetHwnd(), SW_SHOW );
		}
		tabWnd.eTabPosition = tabPosition;
	}

	if (funcKeyWnd.GetHwnd()) {
		if (csWindow.nFuncKeyWnd_Place == 0) {
			// ファンクションキー表示位置／0:上 1:下
			::MoveWindow(
				funcKeyWnd.GetHwnd(),
				0,
				nToolBarHeight,
				cx,
				nFuncKeyWndHeight, TRUE);
		}else if (csWindow.nFuncKeyWnd_Place == 1) {
			// ファンクションキー表示位置／0:上 1:下
			::MoveWindow(
				funcKeyWnd.GetHwnd(),
				0,
				cy - nFuncKeyWndHeight - nStatusBarHeight,
				cx,
				nFuncKeyWndHeight, TRUE
			);

			bool bSizeBox = true;
			if (statusBar.GetStatusHwnd()) {
				bSizeBox = false;
			}
			if (wParam == SIZE_MAXIMIZED) {
				bSizeBox = false;
			}
			funcKeyWnd.SizeBox_ONOFF(bSizeBox);
			bMiniMapSizeBox = false;
		}
		::UpdateWindow(funcKeyWnd.GetHwnd());	// 即時描画でちらつきを減らす
	}

	int nFuncListWidth = 0;
	int nFuncListHeight = 0;
	if (dlgFuncList.GetHwnd() && dlgFuncList.IsDocking()) {
		::SendMessage(dlgFuncList.GetHwnd(), WM_SIZE, wParam, lParam);
		::GetWindowRect(dlgFuncList.GetHwnd(), &rc);
		nFuncListWidth = rc.right - rc.left;
		nFuncListHeight = rc.bottom - rc.top;
	}

	DockSideType eDockSideFL = dlgFuncList.GetDockSide();
	int nTop = nToolBarHeight + nTabWndHeight;
	if (csWindow.nFuncKeyWnd_Place == 0) {
		nTop += nFuncKeyWndHeight;
	}
	int nHeight = cy - nToolBarHeight - nFuncKeyWndHeight - nTabWndHeight - nTabHeightBottom - nStatusBarHeight;
	if (dlgFuncList.GetHwnd() && dlgFuncList.IsDocking()) {
		::MoveWindow(
			dlgFuncList.GetHwnd(),
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
		nMiniMapWidth = GetDllShareData().common.window.nMiniMapWidth;
		::MoveWindow( pEditViewMiniMap->GetHwnd(), 
			(eDockSideFL == DockSideType::Right)? cx - nFuncListWidth - nMiniMapWidth: cx - nMiniMapWidth,
			(eDockSideFL == DockSideType::Top)? nTop + nFuncListHeight: nTop,
			nMiniMapWidth,
			(eDockSideFL == DockSideType::Top || eDockSideFL == DockSideType::Bottom)? nHeight - nFuncListHeight: nHeight,
			TRUE
		);
		GetMiniMap().SplitBoxOnOff(false, false, bMiniMapSizeBox);
	}

	::MoveWindow(
		splitterWnd.GetHwnd(),
		(eDockSideFL == DockSideType::Left)? nFuncListWidth: 0,
		(eDockSideFL == DockSideType::Top)? nTop + nFuncListHeight: nTop,
		((eDockSideFL == DockSideType::Left || eDockSideFL == DockSideType::Right)? cx - nFuncListWidth: cx) - nMiniMapWidth,
		(eDockSideFL == DockSideType::Top || eDockSideFL == DockSideType::Bottom)? nHeight - nFuncListHeight: nHeight,
		TRUE
	);

	// 印刷Previewモードか
	if (!pPrintPreview) {
		return 0L;
	}
	return pPrintPreview->OnSize(wParam, lParam);
}


// WM_PAINT 描画処理
LRESULT EditWnd::OnPaint(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
	// 印刷Previewモードか
	if (!pPrintPreview) {
		PAINTSTRUCT ps;
		::BeginPaint(hwnd, &ps);
		::EndPaint(hwnd, &ps);
		return 0L;
	}
	return pPrintPreview->OnPaint(hwnd, uMsg, wParam, lParam);
}

// 印刷Preview 垂直スクロールバーメッセージ処理 WM_VSCROLL
LRESULT EditWnd::OnVScroll(WPARAM wParam, LPARAM lParam)
{
	// 印刷Previewモードか
	if (!pPrintPreview) {
		return 0;
	}
	return pPrintPreview->OnVScroll(wParam, lParam);
}

// 印刷Preview 水平スクロールバーメッセージ処理
LRESULT EditWnd::OnHScroll(WPARAM wParam, LPARAM lParam)
{
	// 印刷Previewモードか
	if (!pPrintPreview) {
		return 0;
	}
	return pPrintPreview->OnHScroll(wParam, lParam);
}

LRESULT EditWnd::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	// キャプチャーして押されたら非クライアントでもこっちに来る
	if (iconClicked != IconClickStatus::None) {
		return 0;
	}
	ptDragPosOrg.x = LOWORD(lParam);	// horizontal position of cursor
	ptDragPosOrg.y = HIWORD(lParam);	// vertical position of cursor
	bDragMode = true;
	SetCapture(GetHwnd());

	return 0;
}

LRESULT EditWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (iconClicked != IconClickStatus::None) {
		if (iconClicked == IconClickStatus::Down) {
			iconClicked = IconClickStatus::Clicked;
			SetTimer(GetHwnd(), IDT_SYSMENU, GetDoubleClickTime(), NULL);
		}
		return 0;
	}

	bDragMode = false;
//	MYTRACE(_T("bDragMode = FALSE (OnLButtonUp)\n"));
	ReleaseCapture();
	::InvalidateRect(GetHwnd(), NULL, TRUE);
	return 0;
}


/*!	WM_MOUSEMOVE処理 */
LRESULT EditWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	if (iconClicked != IconClickStatus::None) {
		// 一回押された時だけ
		if (iconClicked == IconClickStatus::Down) {
			POINT pt;
			GetCursorPos(&pt); // スクリーン座標
			if (SendMessage(GetHwnd(), WM_NCHITTEST, 0, pt.x | (pt.y << 16)) != HTSYSMENU) {
				ReleaseCapture();
				iconClicked = IconClickStatus::None;

				if (GetDocument().docFile.GetFilePathClass().IsValidPath()) {
					NativeW memTitle;
					NativeW memDir;
					memTitle = to_wchar(GetDocument().docFile.GetFileName());
					memDir   = to_wchar(GetDocument().docFile.GetFilePathClass().GetDirPath().c_str());

					IDataObject *DataObject;
					IMalloc *Malloc;
					IShellFolder *Desktop, *Folder;
					LPITEMIDLIST PathID, ItemID;
					SHGetMalloc(&Malloc);
					SHGetDesktopFolder(&Desktop);
					DWORD Eaten, Attribs;
					if (SUCCEEDED(Desktop->ParseDisplayName(0, NULL, memDir.GetStringPtr(), &Eaten, &PathID, &Attribs))) {
						Desktop->BindToObject(PathID, NULL, IID_IShellFolder, (void**)&Folder);
						Malloc->Free(PathID);
						if (SUCCEEDED(Folder->ParseDisplayName(0, NULL, memTitle.GetStringPtr(), &Eaten, &ItemID, &Attribs))) {
							LPCITEMIDLIST List[1];
							List[0] = ItemID;
							Folder->GetUIObjectOf(0, 1, List, IID_IDataObject, NULL, (void**)&DataObject);
							Malloc->Free(ItemID);
#define DDASTEXT
#ifdef  DDASTEXT
							// テキストでも持たせる…便利
							{
								FORMATETC fmt;
								fmt.cfFormat = CF_UNICODETEXT;
								fmt.ptd      = NULL;
								fmt.dwAspect = DVASPECT_CONTENT;
								fmt.lindex   = -1;
								fmt.tymed    = TYMED_HGLOBAL;

								STGMEDIUM medium;
								const wchar_t* pFilePath = to_wchar(GetDocument().docFile.GetFilePath());
								size_t Len = wcslen(pFilePath);
								medium.tymed          = TYMED_HGLOBAL;
								medium.pUnkForRelease = NULL;
								medium.hGlobal        = GlobalAlloc(GMEM_MOVEABLE, (Len + 1) * sizeof(wchar_t));
								void* p = GlobalLock(medium.hGlobal);
								CopyMemory(p, pFilePath, (Len + 1) * sizeof(wchar_t));
								GlobalUnlock(medium.hGlobal);

								DataObject->SetData(&fmt, &medium, TRUE);
							}
#endif
							// 移動は禁止
							DWORD r;
							DropSource drop(TRUE);
							DoDragDrop(DataObject, &drop, DROPEFFECT_COPY | DROPEFFECT_LINK, &r);
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

//@@@ 2002.01.14 YAZAKI 印刷PreviewをPrintPreviewに独立させたことによる変更
	if (!pPrintPreview) {
		return 0;
	}else {
		return pPrintPreview->OnMouseMove(wParam, lParam);
	}
}


LRESULT EditWnd::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
	if (pPrintPreview) {
		return pPrintPreview->OnMouseWheel(wParam, lParam);
	}
	return Views_DispatchEvent(GetHwnd(), WM_MOUSEWHEEL, wParam, lParam);
}

/** マウスホイール処理 */
bool EditWnd::DoMouseWheel(WPARAM wParam, LPARAM lParam)
{
	// 印刷Previewモードか
	if (!pPrintPreview) {
		// タブ上ならウィンドウ切り替え
		if (pShareData->common.tabBar.bChgWndByWheel && tabWnd.hwndTab) {
			POINT pt;
			pt.x = (short)LOWORD(lParam);
			pt.y = (short)HIWORD(lParam);
			int nDelta = (short)HIWORD(wParam);
			HWND hwnd = ::WindowFromPoint(pt);
			if ((hwnd == tabWnd.hwndTab || hwnd == tabWnd.GetHwnd())) {
				// 現在開いている編集窓のリストを得る
				EditNode* pEditNodeArr;
				size_t nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true);
				if (nRowNum > 0) {
					// 自分のウィンドウを調べる
					int i, j;
					int nGroup = 0;
					for (i=0; i<nRowNum; ++i) {
						if (GetHwnd() == pEditNodeArr[i].GetHwnd()) {
							nGroup = pEditNodeArr[i].nGroup;
							break;
						}
					}
					if (i < nRowNum) {
						if (nDelta < 0) {
							// 次のウィンドウ
							for (j=i+1; j<nRowNum; ++j) {
								if (nGroup == pEditNodeArr[j].nGroup)
									break;
							}
							if (j >= nRowNum) {
								for (j=0; j<i; ++j) {
									if (nGroup == pEditNodeArr[j].nGroup)
										break;
								}
							}
						}else {
							// 前のウィンドウ
							for (j=i-1; j>=0; --j) {
								if (nGroup == pEditNodeArr[j].nGroup)
									break;
							}
							if (j < 0) {
								for (j=(int)nRowNum-1; j>i; --j) {
									if (nGroup == pEditNodeArr[j].nGroup)
										break;
								}
							}
						}

						// 次の（or 前の）ウィンドウをアクティブにする
						if (i != j) {
							ActivateFrameWindow(pEditNodeArr[j].GetHwnd());
						}
					}
					delete[] pEditNodeArr;
				}
				return true;	// 処理した
			}
		}
		return false;	// 処理しなかった
	}
	return false;	// 処理しなかった
}

/* 印刷ページ設定
	印刷Preview時にも、そうでないときでも呼ばれる可能性がある。
*/
bool EditWnd::OnPrintPageSetting(void)
{
	// 印刷設定（CANCEL押したときに破棄するための領域）

	auto& docType = GetDocument().docType;
	int nCurrentPrintSetting = docType.GetDocumentAttribute().nCurrentPrintSetting;
	int nLineNumberColumns;
	if (pPrintPreview) {
		nLineNumberColumns = GetActiveView().GetTextArea().DetectWidthOfLineNumberArea_calculate(pPrintPreview->pLayoutMgr_Print); // 印刷Preview時は文書の桁数
	}else {
		nLineNumberColumns = 3; // ファイルメニューからの設定時は最小値
	}

	DlgPrintSetting	dlgPrintSetting;
	bool bRes = dlgPrintSetting.DoModal(
		G_AppInstance(),
//@@@ 2002.01.14 YAZAKI 印刷PreviewをPrintPreviewに独立させたことによる変更
		GetHwnd(),
		&nCurrentPrintSetting, // 現在選択している印刷設定
		pShareData->printSettingArr, // 現在の設定はダイアログ側で保持する
		nLineNumberColumns // 行番号表示用に桁数を渡す
	) > 0;

	if (bRes) {
		bool bChangePrintSettingNo = false;
		// 現在選択されているページ設定の番号が変更されたか
		if (docType.GetDocumentAttribute().nCurrentPrintSetting != nCurrentPrintSetting) {
			// 変更フラグ(タイプ別設定)
			TypeConfig* type = new TypeConfig();
			DocTypeManager().GetTypeConfig(docType.GetDocumentType(), *type);
			type->nCurrentPrintSetting = nCurrentPrintSetting;
			DocTypeManager().SetTypeConfig(docType.GetDocumentType(), *type);
			delete type;
			docType.GetDocumentAttributeWrite().nCurrentPrintSetting = nCurrentPrintSetting; // 今の設定にも反映
			AppNodeGroupHandle(0).SendMessageToAllEditors(
				MYWM_CHANGESETTING,
				(WPARAM)docType.GetDocumentType().GetIndex(),
				(LPARAM)PM_CHANGESETTING_TYPE,
				EditWnd::getInstance().GetHwnd()
			);
			bChangePrintSettingNo = true;
		}

//@@@ 2002.01.14 YAZAKI 印刷PreviewをPrintPreviewに独立させたことによる変更
		//	印刷Preview時のみ。
		if (pPrintPreview) {
			// 現在の印刷設定
			// 2013.08.27 印刷設定番号が変更された時に対応できていなかった
			if (bChangePrintSettingNo) {
				pPrintPreview->SetPrintSetting(&pShareData->printSettingArr[docType.GetDocumentAttribute().nCurrentPrintSetting]);
			}

			// 印刷Preview スクロールバー初期化
			//pPrintPreview->InitPreviewScrollBar();

			// 印刷設定の反映
			// pPrintPreview->OnChangePrintSetting();

			//::InvalidateRect(GetHwnd(), NULL, TRUE);
		}
		AppNodeGroupHandle(0).SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0,
			(LPARAM)PM_PrintSetting,
			EditWnd::getInstance().GetHwnd()
		);
	}
//@@@ 2002.01.14 YAZAKI 印刷PreviewをPrintPreviewに独立させたことによる変更
	::UpdateWindow(GetHwnd() /* pPrintPreview->GetPrintPreviewBarHANDLE() */);
	return bRes;
}

///////////////////////////// by 鬼

LRESULT EditWnd::OnNcLButtonDown(WPARAM wp, LPARAM lp)
{
	LRESULT result;
	if (wp == HTSYSMENU) {
		SetCapture(GetHwnd());
		iconClicked = IconClickStatus::Down;
		result = 0;
	}else {
		result = DefWindowProc(GetHwnd(), WM_NCLBUTTONDOWN, wp, lp);
	}

	return result;
}

LRESULT EditWnd::OnNcLButtonUp(WPARAM wp, LPARAM lp)
{
	LRESULT result;
	if (iconClicked != IconClickStatus::None) {
		// 念のため
		ReleaseCapture();
		iconClicked = IconClickStatus::None;
		result = 0;
	}else if (wp == HTSYSMENU) {
		result = 0;
	}else {
		result = DefWindowProc(GetHwnd(), WM_NCLBUTTONUP, wp, lp);
	}

	return result;
}

LRESULT EditWnd::OnLButtonDblClk(WPARAM wp, LPARAM lp) // by 鬼(2)
{
	LRESULT result;
	if (iconClicked != IconClickStatus::None) {
		ReleaseCapture();
		iconClicked = IconClickStatus::DoubleClicked;

		SendMessage(GetHwnd(), WM_SYSCOMMAND, SC_CLOSE, 0);

		result = 0;
	}else {
		result = DefWindowProc(GetHwnd(), WM_LBUTTONDBLCLK, wp, lp);
	}

	return result;
}

// ドロップダウンメニュー(開く)
int	EditWnd::CreateFileDropDownMenu(HWND hwnd)
{
	// メニュー表示位置を決める
	// ※ TBN_DROPDOWN 時の NMTOOLBAR::iItem や NMTOOLBAR::rcButton にはドロップダウンメニュー(開く)ボタンが
	//    複数あるときはどれを押した時も１個目のボタン情報が入るようなのでマウス位置からボタン位置を求める
	POINT po;
	::GetCursorPos(&po);
	::ScreenToClient(hwnd, &po);
	int nIndex = Toolbar_Hittest(hwnd, &po);
	if (nIndex < 0) {
		return 0;
	}
	RECT rc;
	Toolbar_GetItemRect(hwnd, nIndex, &rc);
	po.x = rc.left;
	po.y = rc.bottom;
	::ClientToScreen(hwnd, &po);
	GetMonitorWorkRect(po, &rc);
	if (po.x < rc.left) {
		po.x = rc.left;
	}
	if (po.y < rc.top) {
		po.y = rc.top;
	}

	menuDrawer.ResetContents();

	// MRUリストのファイルのリストをメニューにする
	const MruFile mru;
	HMENU hMenu = mru.CreateMenu(menuDrawer);
	if (mru.MenuLength() > 0) {
		menuDrawer.MyAppendMenuSep(
			hMenu,
			MF_BYPOSITION | MF_SEPARATOR,
			0,
			NULL,
			false
		);
	}

	// 最近使ったフォルダのメニューを作成
	const MruFolder mruFolder;
	HMENU hMenuPopUp = mruFolder.CreateMenu(menuDrawer);
	if (mruFolder.MenuLength() > 0) {
		// アクティブ
		menuDrawer.MyAppendMenu(
			hMenu,
			MF_BYPOSITION | MF_STRING | MF_POPUP,
			(UINT_PTR)hMenuPopUp,
			LS(F_FOLDER_USED_RECENTLY),
			_T("")
		);
	}else {
		// 非アクティブ
		menuDrawer.MyAppendMenu(
			hMenu,
			MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED,
			(UINT_PTR)hMenuPopUp,
			LS(F_FOLDER_USED_RECENTLY),
			_T("")
		);
	}

	menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, false);

	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW, _T(""), _T("N"), false);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW_NEWWINDOW, _T(""), _T("medium"), false);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN, _T(""), _T("O"), false);

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
		GetHwnd(),
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
*/
void EditWnd::InitMenubarMessageFont(void)
{
	TEXTMETRIC	tm;
	HDC			hdc;
	HFONT		hFontOld;

	// LOGFONTの初期化
	LOGFONT lf = {0};
	lf.lfHeight			= DpiPointsToPixels(-9);	// 高DPI対応（ポイント数から算出）
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
	hFontCaretPosInfo = ::CreateFontIndirect(&lf);

	hdc = ::GetDC(::GetDesktopWindow());
	hFontOld = (HFONT)::SelectObject(hdc, hFontCaretPosInfo);
	::GetTextMetrics(hdc, &tm);
	nCaretPosInfoCharWidth = tm.tmAveCharWidth;
	nCaretPosInfoCharHeight = tm.tmHeight;
	::SelectObject(hdc, hFontOld);
	::ReleaseDC(::GetDesktopWindow(), hdc);
}

/*
	@brief メニューバーにメッセージを表示する
	
	事前にメニューバー表示用フォントが初期化されていなくてはならない．
	指定できる文字数は最大30バイト．それ以上の場合はうち切って表示する．
*/
void EditWnd::PrintMenubarMessage(const TCHAR* msg)
{
	if (!::GetMenu(GetHwnd()))
		return;

	POINT	po, poFrame;
	RECT	rc, rcFrame;
	HFONT	hFontOld;
	int		nStrLen;

	// msg == NULL のときは以前の pszMenubarMessage で再描画
	if (msg) {
		size_t len = _tcslen(msg);
		_tcsncpy(pszMenubarMessage, msg, MENUBAR_MESSAGE_MAX_LEN);
		if (len < MENUBAR_MESSAGE_MAX_LEN) {
			auto_memset(pszMenubarMessage + len, _T(' '), MENUBAR_MESSAGE_MAX_LEN - len);	//  null終端は不要
		}
	}

	HDC hdc = ::GetWindowDC(GetHwnd());
	poFrame.x = 0;
	poFrame.y = 0;
	::ClientToScreen(GetHwnd(), &poFrame);
	::GetWindowRect(GetHwnd(), &rcFrame);
	po.x = rcFrame.right - rcFrame.left;
	po.y = poFrame.y - rcFrame.top;
	hFontOld = (HFONT)::SelectObject(hdc, hFontCaretPosInfo);
	nStrLen = MENUBAR_MESSAGE_MAX_LEN;
	rc.left = po.x - nStrLen * nCaretPosInfoCharWidth - (::GetSystemMetrics(SM_CXSIZEFRAME) + 2);
	rc.right = rc.left + nStrLen * nCaretPosInfoCharWidth + 2;
	rc.top = po.y - nCaretPosInfoCharHeight - 2;
	rc.bottom = rc.top + nCaretPosInfoCharHeight;
	::SetTextColor(hdc, ::GetSysColor(COLOR_MENUTEXT));
	//	Sep. 6, 2003 genta Windows XP(Luna)の場合にはCOLOR_MENUBARを使わなくてはならない
	COLORREF bkColor =
		::GetSysColor(IsWinXP_or_later() ? COLOR_MENUBAR : COLOR_MENU);
	::SetBkColor(hdc, bkColor);
	/*
	int pnCaretPosInfoDx[64];	// 文字列描画用文字幅配列
	for (i=0; i<_countof(pnCaretPosInfoDx); ++i) {
		pnCaretPosInfoDx[i] = (nCaretPosInfoCharWidth);
	}
	*/
	::ExtTextOut(hdc, rc.left, rc.top, ETO_CLIPPED | ETO_OPAQUE,&rc,pszMenubarMessage,nStrLen,NULL/*pnCaretPosInfoDx*/);
	::SelectObject(hdc, hFontOld);
	::ReleaseDC(GetHwnd(), hdc);
}

/*!
	@brief メッセージの表示
	
	指定されたメッセージをステータスバーに表示する．
	ステータスバーが非表示の場合はメニューバーの右端に表示する．
	
	@param msg [in] 表示するメッセージ
*/
void EditWnd::SendStatusMessage(const TCHAR* msg)
{
	if (!statusBar.GetStatusHwnd()) {
		// メニューバーへ
		PrintMenubarMessage(msg);
	}else {
		// ステータスバーへ
		statusBar.SetStatusText(0, SBT_NOBORDERS, msg);
	}
}

/*! ファイル名変更通知 */
void EditWnd::ChangeFileNameNotify(const TCHAR* pszTabCaption, const TCHAR* _pszFilePath, bool bIsGrep)
{
	const TCHAR* pszFilePath = _pszFilePath;
	
	EditNode* p;
	int nIndex;
	
	if (!pszTabCaption) pszTabCaption = _T("");	// ガード
	if (!pszFilePath) pszFilePath = _FT("");
	
	RecentEditNode	recentEditNode;
	nIndex = recentEditNode.FindItemByHwnd(GetHwnd());
	if (nIndex != -1) {
		p = recentEditNode.GetItem(nIndex);
		if (p) {
			int	size = _countof(p->szTabCaption) - 1;
			_tcsncpy(p->szTabCaption, pszTabCaption, size);
			p->szTabCaption[size] = _T('\0');

			size = _countof2(p->szFilePath) - 1;
			_tcsncpy(p->szFilePath, pszFilePath, size);
			p->szFilePath[size] = _T('\0');

			p->bIsGrep = bIsGrep;
		}
	}
	recentEditNode.Terminate();

	// ファイル名変更通知をブロードキャストする。
	int nGroup = AppNodeManager::getInstance().GetEditNode(GetHwnd())->GetGroup();
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

	// タブまとめ時は WS_EX_TOPMOST 状態を全ウィンドウで同期する
	auto& csTabBar = pShareData->common.tabBar;
	if (pShareData->common.tabBar.bDispTabWnd && !pShareData->common.tabBar.bDispTabWndMultiWin) {
		hwndInsertAfter = GetHwnd();
		for (size_t i=0; i<pShareData->nodes.nEditArrNum; ++i) {
			HWND hwnd = pShareData->nodes.pEditArr[i].GetHwnd();
			if (hwnd != GetHwnd() && IsSakuraMainWindow(hwnd)) {
				if (!AppNodeManager::IsSameGroup(GetHwnd(), hwnd)) {
					continue;
				}
				::SetWindowPos(hwnd, hwndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				hwndInsertAfter = hwnd;
			}
		}
	}
}


// タイマーの更新を開始／停止する。
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
		if (pt.x < rc.left) {
			pt.x = rc.left;
		}
		pt.y = rc.top;
	}

	// ウィンドウ一覧メニューをポップアップ表示する
	if (tabWnd.GetHwnd()) {
		tabWnd.TabListMenu(pt);
	}else {
		menuDrawer.ResetContents();
		EditNode* pEditNodeArr;
		HMENU hMenu = ::CreatePopupMenu();
		size_t nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true);
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

/*! @brief 現在開いている編集窓のリストをメニューにする */
LRESULT EditWnd::WinListMenu(HMENU hMenu, EditNode* pEditNodeArr, size_t nRowNum, bool bFull)
{
	if (nRowNum > 0) {
		TCHAR szMenu[_MAX_PATH * 2 + 3];
		FileNameManager::getInstance().TransformFileName_MakeCache();

		NONCLIENTMETRICS met;
		met.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, met.cbSize, &met, 0);
		DCFont dcFont(met.lfMenuFont, GetHwnd());
		for (size_t i=0; i<nRowNum; ++i) {
			// トレイからエディタへの編集ファイル名要求通知
			::SendMessage(pEditNodeArr[i].GetHwnd(), MYWM_GETFILEINFO, 0, 0);
////	From Here Oct. 4, 2000 JEPRO commented out & modified	開いているファイル数がわかるように履歴とは違って1から数える
			const EditInfo*	pfi = (EditInfo*)&pShareData->workBuffer.editInfo_MYWM_GETFILEINFO;
			FileNameManager::getInstance().GetMenuFullLabel_WinList(szMenu, _countof(szMenu), pfi, pEditNodeArr[i].nId, i, dcFont.GetHDC());
			menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, IDM_SELWINDOW + pEditNodeArr[i].nIndex, szMenu, _T(""));
			if (GetHwnd() == pEditNodeArr[i].GetHwnd()) {
				::CheckMenuItem(hMenu, UINT(IDM_SELWINDOW + pEditNodeArr[i].nIndex), MF_BYCOMMAND | MF_CHECKED);
			}
		}
	}
	return 0L;
}

// ツールチップのテキストを取得
void EditWnd::GetTooltipText(TCHAR* wszBuf, size_t nBufCount, int nID) const
{
	// 機能文字列の取得 -> tmp -> wszBuf
	wchar_t tmp[256];
	size_t nLen;
	GetDocument().funcLookup.Funccode2Name(nID, tmp, _countof(tmp));
	nLen = _wcstotcs(wszBuf, tmp, nBufCount);

	// 機能に対応するキー名の取得(複数)
	auto& csKeyBind = pShareData->common.keyBind;
	NativeT** ppcAssignedKeyList;
	int nAssignedKeyNum = KeyBind::GetKeyStrList(
		G_AppInstance(),
		csKeyBind.nKeyNameArrNum,
		csKeyBind.pKeyNameArr,
		&ppcAssignedKeyList,
		nID
	);

	// wszBufへ結合
	if (0 < nAssignedKeyNum) {
		for (int j=0; j<nAssignedKeyNum; ++j) {
			const TCHAR* pszKey = ppcAssignedKeyList[j]->GetStringPtr();
			size_t nKeyLen = _tcslen(pszKey);
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


/*! タイマーの処理 */
void EditWnd::OnEditTimer(void)
{
	IncrementTimerCount(6);

	if (nTimerCount == 0 && !GetCapture()) { 
		// ファイルのタイムスタンプのチェック処理
		GetDocument().autoReloadAgent.CheckFileTimeStamp();

#if 0	// 書込禁止の監視を廃止（復活させるなら「更新の監視」付随ではなく別オプションにしてほしい）
		// ファイル書込可能のチェック処理
		if (GetDocument()->autoReloadAgent._ToDoChecking()) {
			bool bOld = GetDocument()->docLocker.IsDocWritable();
			GetDocument()->docLocker.CheckWritable(false);
			if (bOld != GetDocument()->docLocker.IsDocWritable()) {
				this->UpdateCaption();
			}
		}
#endif
	}

	GetDocument().autoSaveAgent.CheckAutoSave();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ビュー管理                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	EditViewの画面バッファを削除
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
	if (nEditViewMaxCount < nViewCount) {
		return false;
	}
	if (GetAllViewCount() < nViewCount) {
		for (int i=GetAllViewCount(); i<nViewCount; ++i) {
			assert(!pEditViewArr[i]);
			pEditViewArr[i] = new EditView(*this);
			pEditViewArr[i]->Create(splitterWnd.GetHwnd(), GetDocument(), i, FALSE, false);
		}
		nEditViewCount = nViewCount;

		std::vector<HWND> hWndArr;
		hWndArr.reserve(nViewCount + 1);
		for (int i=0; i<nViewCount; ++i) {
			hWndArr.push_back(GetView(i).GetHwnd());
		}
		hWndArr.push_back(NULL);

		splitterWnd.SetChildWndArr(&hWndArr[0]);
	}
	return true;
}

/*
	ビューの再初期化
*/
void EditWnd::InitAllViews()
{
	// 先頭へカーソルを移動
	for (int i=0; i<GetAllViewCount(); ++i) {
		// 移動履歴の消去
		auto& view = GetView(i);
		view.pHistory->Flush();

		// 現在の選択範囲を非選択状態に戻す
		view.GetSelectionInfo().DisableSelectArea(false);
		view.OnChangeSetting();
		view.GetCaret().MoveCursor(Point(0, 0), true);
		view.GetCaret().nCaretPosX_Prev = 0;
	}
	GetMiniMap().OnChangeSetting();
}


void EditWnd::Views_RedrawAll()
{
	// アクティブ以外を再描画してから…
	for (int v=0; v<GetAllViewCount(); ++v) {
		if (nActivePaneIndex != v) {
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
		if (nActivePaneIndex != v) {
			GetView(v).Redraw();
		}
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
	int nOldIndex = nActivePaneIndex;
	nActivePaneIndex = nIndex;
	pEditView = pEditViewArr[nActivePaneIndex];

	// フォーカスを移動する
	GetView(nOldIndex).GetCaret().underLine.CaretUnderLineOFF(true);	//	2002/05/11 YAZAKI
	if (::GetActiveWindow() == GetHwnd()
		&& ::GetFocus() != GetActiveView().GetHwnd()
	) {
		// ::SetFocus()でフォーカスを切り替える
		::SetFocus(GetActiveView().GetHwnd());
	}else {
		// 起動と同時にエディットボックスにフォーカスのあるダイアログを表示すると当該エディットボックスに
		// キャレットが表示されない問題(*1)を修正するのため、内部的な切り替えをするのはアクティブペインが
		// 切り替わるときだけにした。← EditView::OnKillFocus()は自スレッドのキャレットを破棄するので
		// (*1) -GREPDLGオプションによるGREPダイアログ表示や開ファイル後自動実行マクロでのInputBox表示
		if (nActivePaneIndex != nOldIndex) {
			// アクティブでないときに::SetFocus()するとアクティブになってしまう
			// （不可視なら可視になる）ので内部的に切り替えるだけにする
			GetView(nOldIndex).OnKillFocus();
			GetActiveView().OnSetFocus();
		}
	}

	GetActiveView().RedrawAll();	// フォーカス移動時の再描画

	splitterWnd.SetActivePane(nIndex);

	if (dlgFind.GetHwnd()) {		//「検索」ダイアログ
		// モードレス時：検索対象となるビューの変更
		dlgFind.ChangeView((LPARAM)&GetActiveView());
	}
	if (dlgReplace.GetHwnd()) {	//「置換」ダイアログ
		// モードレス時：検索対象となるビューの変更
		dlgReplace.ChangeView((LPARAM)&GetActiveView());
	}
	if (hokanMgr.GetHwnd()) {	//「入力補完」ダイアログ
		hokanMgr.Hide();
		// モードレス時：検索対象となるビューの変更
		hokanMgr.ChangeView((LPARAM)&GetActiveView());
	}
	if (dlgFuncList.GetHwnd()) {	//「アウトライン」ダイアログ	
		// モードレス時：現在位置表示の対象となるビューの変更
		dlgFuncList.ChangeView((LPARAM)&GetActiveView());
	}

	return;
}

/** すべてのペインの描画スイッチを設定する

	@param bDraw [in] 描画スイッチの設定値
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

	@param pViewExclude [in] Redrawから除外するビュー
*/
void EditWnd::RedrawAllViews(EditView* pViewExclude)
{
	for (int i=0; i<GetAllViewCount(); ++i) {
		EditView* pView = &GetView(i);
		if (pView == pViewExclude)
			continue;
		if (i == nActivePaneIndex) {
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
		auto& selInfo = GetView(i).GetSelectionInfo();
		if (selInfo.IsTextSelected()) {	// テキストが選択されているか
			// 現在の選択範囲を非選択状態に戻す
			selInfo.DisableSelectArea(true);
		}
	}
}


// すべてのペインで、行番号表示に必要な幅を再設定する（必要なら再描画する）
bool EditWnd::DetectWidthOfLineNumberAreaAllPane(bool bRedraw)
{
	if (GetAllViewCount() == 1) {
		return GetActiveView().GetTextArea().DetectWidthOfLineNumberArea(bRedraw);
	}
	// 以下2,4分割限定

	if (GetActiveView().GetTextArea().DetectWidthOfLineNumberArea(bRedraw)) {
		// ActivePaneで計算したら、再設定・再描画が必要と判明した
		if (splitterWnd.GetAllSplitCols() == 2) {
			GetView(nActivePaneIndex^1).GetTextArea().DetectWidthOfLineNumberArea(bRedraw);
		}else {
			// 表示されていないので再描画しない
			GetView(nActivePaneIndex^1).GetTextArea().DetectWidthOfLineNumberArea(false);
		}
		if (splitterWnd.GetAllSplitRows() == 2) {
			GetView(nActivePaneIndex^2).GetTextArea().DetectWidthOfLineNumberArea(bRedraw);
			if (splitterWnd.GetAllSplitCols() == 2) {
				GetView((nActivePaneIndex^1)^2).GetTextArea().DetectWidthOfLineNumberArea(bRedraw);
			}
		}else {
			GetView(nActivePaneIndex^2).GetTextArea().DetectWidthOfLineNumberArea(false);
			GetView((nActivePaneIndex^1)^2).GetTextArea().DetectWidthOfLineNumberArea(false);
		}
		return true;
	}
	return false;
}


/** 右端で折り返す
	@param nViewColNum	[in] 右端で折り返すペインの番号
	@retval 折り返しを変更したかどうか
*/
bool EditWnd::WrapWindowWidth(int nPane)
{
	// 右端で折り返す
	int nWidth = GetView(nPane).ViewColNumToWrapColNum(GetView(nPane).GetTextArea().nViewColNum);
	if (GetDocument().layoutMgr.GetMaxLineKetas() != nWidth) {
		ChangeLayoutParam(false, GetDocument().layoutMgr.GetTabSpace(), nWidth);
		ClearViewCaretPosInfo();
		return true;
	}
	return false;
}

/** 折り返し方法関連の更新
	@retval 画面更新したかどうか
*/
bool EditWnd::UpdateTextWrap(void)
{
	// この関数はコマンド実行ごとに処理の最終段階で利用する
	// （Undo登録＆全ビュー更新のタイミング）
	if (GetDocument().nTextWrapMethodCur == TextWrappingMethod::WindowWidth) {
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
	return false;	// 画面更新しなかった
}

/*!	レイアウトパラメータの変更

	具体的にはタブ幅と折り返し位置を変更する．
	現在のドキュメントのレイアウトのみを変更し，共通設定は変更しない．
*/
void EditWnd::ChangeLayoutParam(bool bShowProgress, size_t nTabSize, size_t nMaxLineKetas)
{
	HWND hwndProgress = NULL;
	if (bShowProgress && this) {
		hwndProgress = this->statusBar.GetProgressHwnd();
		// Status Barが表示されていないときはhwndProgressBar == NULL
	}

	if (hwndProgress) {
		::ShowWindow(hwndProgress, SW_SHOW);
	}

	// 座標の保存
	PointEx* posSave = SavePhysPosOfAllView();

	// レイアウトの更新
	GetDocument().layoutMgr.ChangeLayoutParam(nTabSize, nMaxLineKetas);
	ClearViewCaretPosInfo();

	// 座標の復元
	// レイアウト変更途中はカーソル移動の画面スクロールを見せない
	const bool bDrawSwitchOld = SetDrawSwitchOfAllViews(false);
	RestorePhysPosOfAllView(posSave);
	SetDrawSwitchOfAllViews(bDrawSwitchOld);

	for (int i=0; i<GetAllViewCount(); ++i) {
		if (GetView(i).GetHwnd()) {
			InvalidateRect(GetView(i).GetHwnd(), NULL, TRUE);
			GetView(i).AdjustScrollBars();
		}
	}
	if (GetMiniMap().GetHwnd()) {
		InvalidateRect(GetMiniMap().GetHwnd(), NULL, TRUE);
		GetMiniMap().AdjustScrollBars();
	}
	GetActiveView().GetCaret().ShowCaretPosInfo();

	if (hwndProgress) {
		::ShowWindow(hwndProgress, SW_HIDE);
	}
}


/*!
	レイアウトの変更に先立って，全てのViewの座標を物理座標に変換して保存する．

	@return データを保存した配列へのポインタ

	@note 取得した値はレイアウト変更後にEditWnd::RestorePhysPosOfAllViewへ渡す．
	渡し忘れるとメモリリークとなる．
*/
PointEx* EditWnd::SavePhysPosOfAllView()
{
	const int numOfViews = GetAllViewCount();
	const int numOfPositions = 6;
	
	PointEx* pptPosArray = new PointEx[numOfViews * numOfPositions];
	auto& layoutMgr = GetDocument().layoutMgr;
	for (int i=0; i<numOfViews; ++i) {
		auto& view = GetView(i);
		Point tmp = Point(0, view.pTextArea->GetViewTopLine());
		const Layout* layoutLine = layoutMgr.SearchLineByLayoutY(tmp.y);
		if (layoutLine) {
			int nLineCenter = layoutLine->GetLogicOffset() + (int)layoutLine->GetLengthWithoutEOL() / 2;
			pptPosArray[i * numOfPositions + 0].x = nLineCenter;
			pptPosArray[i * numOfPositions + 0].y = layoutLine->GetLogicLineNo();
		}else {
			pptPosArray[i * numOfPositions + 0].x = 0;
			pptPosArray[i * numOfPositions + 0].y = 0;
		}
		pptPosArray[i * numOfPositions + 0].ext = 0;
		auto& selInfo = view.GetSelectionInfo();
		if (selInfo.selectBgn.GetFrom().y >= 0) {
			pptPosArray[i * numOfPositions + 1] = layoutMgr.LayoutToLogicEx(
				selInfo.selectBgn.GetFrom()
			);
		}
		if (selInfo.selectBgn.GetTo().y >= 0) {
			pptPosArray[i * numOfPositions + 2] = layoutMgr.LayoutToLogicEx(
				selInfo.selectBgn.GetTo()
			);
		}
		if (selInfo.select.GetFrom().y >= 0) {
			pptPosArray[i * numOfPositions + 3] = layoutMgr.LayoutToLogicEx(
				selInfo.select.GetFrom()
			);
		}
		if (selInfo.select.GetTo().y >= 0) {
			pptPosArray[i * numOfPositions + 4] = layoutMgr.LayoutToLogicEx(
				selInfo.select.GetTo()
			);
		}
		pptPosArray[i * numOfPositions + 5] = layoutMgr.LayoutToLogicEx(
			view.GetCaret().GetCaretLayoutPos()
		);
	}
	return pptPosArray;
}


/*!	座標の復元

	EditWnd::SavePhysPosOfAllViewで保存したデータを元に座標値を再計算する．
*/
void EditWnd::RestorePhysPosOfAllView(PointEx* pptPosArray)
{
	const int numOfViews = GetAllViewCount();
	const int numOfPositions = 6;

	auto& layoutMgr = GetDocument().layoutMgr;
	for (int i=0; i<numOfViews; ++i) {
		Point tmp = layoutMgr.LogicToLayoutEx(
			pptPosArray[i * numOfPositions + 0]
		);
		auto& view = GetView(i);
		view.pTextArea->SetViewTopLine(tmp.y);
		auto& selInfo = view.GetSelectionInfo();
		if (selInfo.selectBgn.GetFrom().y >= 0) {
			selInfo.selectBgn.SetFrom(layoutMgr.LogicToLayoutEx(pptPosArray[i * numOfPositions + 1]));
		}
		if (selInfo.selectBgn.GetTo().y >= 0) {
			selInfo.selectBgn.SetTo(layoutMgr.LogicToLayoutEx(pptPosArray[i * numOfPositions + 2]));
		}
		if (selInfo.select.GetFrom().y >= 0) {
			selInfo.select.SetFrom(layoutMgr.LogicToLayoutEx(pptPosArray[i * numOfPositions + 3]));
		}
		if (selInfo.select.GetTo().y >= 0) {
			selInfo.select.SetTo(layoutMgr.LogicToLayoutEx(pptPosArray[i * numOfPositions + 4]));
		}
		Point ptPosXY = layoutMgr.LogicToLayoutEx(pptPosArray[i * numOfPositions + 5]);
		auto& caret = view.GetCaret();
		caret.MoveCursor(ptPosXY, false); // 2013.06.05 bScrollをtrue=>falase
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

		int nLeft = 0;
		auto& textArea = view.GetTextArea();
		if (textArea.nViewColNum < (int)view.GetRightEdgeForScrollBar()) {
			nLeft = view.GetRightEdgeForScrollBar() - textArea.nViewColNum;
		}
		if (nLeft < textArea.GetViewLeftCol()) {
			textArea.SetViewLeftCol(nLeft);
		}

		caret.ShowEditCaret();
	}
	GetActiveView().GetCaret().ShowCaretPosInfo();
	delete[] pptPosArray;
}

/*!
	@brief マウスの状態をクリアする（ホイールスクロール有無状態をクリア）

	@note ホイール操作によるページスクロール・横スクロール対応のために追加。
		  ページスクロール・横スクロールありフラグをOFFする。
*/
void EditWnd::ClearMouseState(void)
{
	SetPageScrollByWheel(FALSE);		// ホイール操作によるページスクロール有無
	SetHScrollByWheel(FALSE);			// ホイール操作による横スクロール有無
}

/*! ウィンドウ毎にアクセラレータテーブルを作成する(Wine用)
	@note Wineでは別プロセスで作成したアクセラレータテーブルを使用することができない。
	      IsWine()によりプロセス毎にアクセラレータテーブルが作成されるようになる
	      ため、ショートカットキーやカーソルキーが正常に処理されるようになる。
*/
void EditWnd::CreateAccelTbl(void)
{
	if (IsWine()) {
		auto& csKeyBind = pShareData->common.keyBind;
		hAccelWine = KeyBind::CreateAccerelator(
			csKeyBind.nKeyNameArrNum,
			csKeyBind.pKeyNameArr
		);

		if (!hAccelWine) {
			ErrorMessage(
				NULL,
				LS(STR_ERR_DLGEDITWND01)
			);
		}
	}

	hAccel = hAccelWine ? hAccelWine : pShareData->handles.hAccel;
}

/*! ウィンドウ毎に作成したアクセラレータテーブルを破棄する */
void EditWnd::DeleteAccelTbl(void)
{
	hAccel = NULL;

	if (hAccelWine) {
		::DestroyAcceleratorTable(hAccelWine);
		hAccelWine = NULL;
	}
}

// プラグインコマンドをエディタに登録する
void EditWnd::RegisterPluginCommand(int idCommand)
{
	Plug* plug = JackManager::getInstance().GetCommandById(idCommand);
	RegisterPluginCommand(plug);
}

// プラグインコマンドをエディタに登録する（一括）
void EditWnd::RegisterPluginCommand()
{
	const Plug::Array& plugs = JackManager::getInstance().GetPlugs(PP_COMMAND);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		RegisterPluginCommand(*it);
	}
}

// プラグインコマンドをエディタに登録する
void EditWnd::RegisterPluginCommand(Plug* plug)
{
	int iBitmap = MenuDrawer::TOOLBAR_ICON_PLUGCOMMAND_DEFAULT - 1;
	if (!plug->sIcon.empty()) {
		iBitmap = menuDrawer.pIcons->Add(to_tchar(plug->plugin.GetFilePath(to_tchar(plug->sIcon.c_str())).c_str()));
	}

	menuDrawer.AddToolButton(iBitmap, plug->GetFunctionCode());
}


const LOGFONT& EditWnd::GetLogfont(bool bTempSetting)
{
	if (bTempSetting && GetDocument().blfCurTemp) {
		return GetDocument().lfCur;
	}
	bool bUseTypeFont = GetDocument().docType.GetDocumentAttribute().bUseTypeFont;
	if (bUseTypeFont) {
		return GetDocument().docType.GetDocumentAttribute().lf;
	}
	return pShareData->common.view.lf;
}

int EditWnd::GetFontPointSize(bool bTempSetting)
{
	if (bTempSetting && GetDocument().blfCurTemp) {
		return GetDocument().nPointSizeCur;
	}
	bool bUseTypeFont = GetDocument().docType.GetDocumentAttribute().bUseTypeFont;
	if (bUseTypeFont) {
		return GetDocument().docType.GetDocumentAttribute().nPointSize;
	}
	return pShareData->common.view.nPointSize;
}


CharWidthCacheMode EditWnd::GetLogfontCacheMode()
{
	if (GetDocument().blfCurTemp) {
		return CharWidthCacheMode::Local;
	}
	bool bUseTypeFont = GetDocument().docType.GetDocumentAttribute().bUseTypeFont;
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
