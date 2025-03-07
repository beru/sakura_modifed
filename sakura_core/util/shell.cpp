#include "StdAfx.h"
#include <HtmlHelp.h>
#include <ShlObj.h>
#include <ShellAPI.h>
#include <CdErr.h>
#include "util/shell.h"
#include "util/string_ex2.h"
#include "util/fileUtil.h"
#include "util/os.h"
#include "util/module.h"
#include "_os/OsVersionInfo.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "extmodule/HtmlHelp.h"

int CALLBACK MYBrowseCallbackProc(
	HWND hwnd,
	UINT uMsg,
	LPARAM lParam,
	LPARAM lpData
	)
{
	switch (uMsg) {
	case BFFM_INITIALIZED:
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)lpData);
		break;
	case BFFM_SELCHANGED:
		break;
	}
	return 0;

}


// フォルダ選択ダイアログ
BOOL SelectDir(
	HWND hWnd,
	const TCHAR* pszTitle,
	const TCHAR* pszInitFolder,
	TCHAR* strFolderName
	)
{
	TCHAR szInitFolder[MAX_PATH];
	_tcscpy_s(szInitFolder, pszInitFolder);
	// フォルダの最後が半角かつ'\\'の場合は、取り除く "c:\\"等のルートは取り除かない
	CutLastYenFromDirectoryPath(szInitFolder);

	// フォルダを開くとフックも含めて色々DLLが読み込まれるので移動
	CurrentDirectoryBackupPoint dirBack;
	ChangeCurrentDirectoryToExeDir();

	// SHBrowseForFolder()関数に渡す構造体
	BROWSEINFO bi;
	bi.hwndOwner = hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = strFolderName;
	bi.lpszTitle = pszTitle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE|BIF_NONEWFOLDERBUTTON/* | BIF_EDITBOX*//* | BIF_STATUSTEXT*/;
	bi.lpfn = MYBrowseCallbackProc;
	bi.lParam = (LPARAM)szInitFolder;
	bi.iImage = 0;
	// アイテムＩＤリストを返す
	// ITEMIDLISTはアイテムの一意を表す構造体
	LPITEMIDLIST pList = ::SHBrowseForFolder(&bi);
	if (pList) {
		// SHGetPathFromIDList()関数はアイテムＩＤリストの物理パスを探してくれる
		BOOL bRes = ::SHGetPathFromIDList(pList, strFolderName);
		// :SHBrowseForFolder()で取得したアイテムＩＤリストを削除
		::CoTaskMemFree(pList);
		if (bRes) {
			return TRUE;
		}else {
			return FALSE;
		}
	}
	return FALSE;
}


/*!	特殊フォルダのパスを取得する
	SHGetSpecialFolderPath API（shell32.dll version 4.71以上が必要）と同等の処理をする
*/
BOOL GetSpecialFolderPath(
	int nFolder,
	LPTSTR pszPath
	)
{
	LPMALLOC pMalloc;

	HRESULT hres = ::SHGetMalloc(&pMalloc);
	if (FAILED(hres))
		return FALSE;

	BOOL bRet = FALSE;
	LPITEMIDLIST pidl;
	hres = ::SHGetSpecialFolderLocation(NULL, nFolder, &pidl);
	if (SUCCEEDED(hres)) {
		bRet = ::SHGetPathFromIDList(pidl, pszPath);
		pMalloc->Free((void*)pidl);
	}

	pMalloc->Release();

	return bRet;
}


///////////////////////////////////////////////////////////////////////
// 独自拡張のプロパティシート関数群

static WNDPROC s_pOldPropSheetWndProc;	// プロパティシートの元のウィンドウプロシージャ

/*!	独自拡張プロパティシートのウィンドウプロシージャ */
static
LRESULT CALLBACK PropSheetWndProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	switch (uMsg) {
	case WM_SHOWWINDOW:
		// 追加ボタンの位置を調整する
		if (wParam) {
			RECT rcOk;
			RECT rcTab;
			POINT pt;

			HWND hwndBtn = ::GetDlgItem(hwnd, 0x02000);
			::GetWindowRect(::GetDlgItem(hwnd, IDOK), &rcOk);
			::GetWindowRect(PropSheet_GetTabControl(hwnd), &rcTab);
			pt.x = rcTab.left;
			pt.y = rcOk.top;
			::ScreenToClient(hwnd, &pt);
			::MoveWindow(hwndBtn, pt.x, pt.y, 140, rcOk.bottom - rcOk.top, FALSE);
		}
		break;

	case WM_COMMAND:
		// 追加ボタンが押された時はその処理を行う
		if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == 0x02000) {
			HWND hwndBtn = ::GetDlgItem(hwnd, 0x2000);
			RECT rc;
			POINT pt;

			// メニューを表示する
			::GetWindowRect(hwndBtn, &rc);
			pt.x = rc.left;
			pt.y = rc.bottom;
			GetMonitorWorkRect(pt, &rc);	// モニタのワークエリア

			HMENU hMenu = ::CreatePopupMenu();
			::InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, 100, LS(STR_SHELL_MENU_OPEN));
			::InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING, 101, LS(STR_SHELL_MENU_IMPEXP));

			int nId = ::TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
										(pt.x > rc.left)? pt.x: rc.left,
										(pt.y < rc.bottom)? pt.y: rc.bottom,
										0, hwnd, NULL);
			::DestroyMenu(hMenu);

			// 選択されたメニューの処理
			switch (nId) {
			case 100:	// 設定フォルダを開く
				TCHAR szPath[_MAX_PATH];
				GetInidir(szPath);

				// フォルダの ITEMIDLIST を取得して ShellExecuteEx() で開く
				// Note. MSDN の ShellExecute() の解説にある方法でフォルダを開こうとした場合、
				//       フォルダと同じ場所に <フォルダ名>.exe があるとうまく動かない。
				//       verbが"open"やNULLではexeのほうが実行され"explore"では失敗する
				//       （フォルダ名の末尾に'\\'を付加してもWindows 2000では付加しないのと同じ動作になってしまう）
				LPSHELLFOLDER pDesktopFolder;
				if (SUCCEEDED(::SHGetDesktopFolder(&pDesktopFolder))) {
					LPMALLOC pMalloc;
					if (SUCCEEDED(::SHGetMalloc(&pMalloc))) {
						LPITEMIDLIST pIDL;
						wchar_t pwszDisplayName[_MAX_PATH];
						_tcstowcs(pwszDisplayName, szPath, _countof(pwszDisplayName));
						if (SUCCEEDED(pDesktopFolder->ParseDisplayName(NULL, NULL, pwszDisplayName, NULL, &pIDL, NULL))) {
							SHELLEXECUTEINFO si = {0};
							si.cbSize   = sizeof(si);
							si.fMask    = SEE_MASK_IDLIST;
							si.lpVerb   = _T("open");
							si.lpIDList = pIDL;
							si.nShow    = SW_SHOWNORMAL;
							::ShellExecuteEx(&si);	// フォルダを開く
							pMalloc->Free((void*)pIDL);
						}
						pMalloc->Release();
					}
					pDesktopFolder->Release();
				}
				break;
			case 101:	// インポート／エクスポートの起点リセット（起点を設定フォルダにする）
				int nMsgResult = MYMESSAGEBOX(
					hwnd,
					MB_OKCANCEL | MB_ICONINFORMATION,
					GSTR_APPNAME,
					LS(STR_SHELL_IMPEXPDIR)
				);
				if (nMsgResult == IDOK) {
					DllSharedData *pShareData = &GetDllShareData();
					GetInidir(pShareData->history.szIMPORTFOLDER);
					AddLastChar(pShareData->history.szIMPORTFOLDER, _countof2(pShareData->history.szIMPORTFOLDER), _T('\\'));
				}
				break;
			}
		}
		break;

	case WM_DESTROY:
		::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)s_pOldPropSheetWndProc);
		break;
	}

	return ::CallWindowProc(s_pOldPropSheetWndProc, hwnd, uMsg, wParam, lParam);
}

/*!	独自拡張プロパティシートのコールバック関数 */
static
int CALLBACK PropSheetProc(
	HWND hwndDlg,
	UINT uMsg,
	LPARAM lParam
	)
{
	// プロパティシートの初期化時にボタン追加、プロパティシートのサブクラス化を行う
	if (uMsg == PSCB_INITIALIZED) {
		s_pOldPropSheetWndProc = (WNDPROC)::SetWindowLongPtr(hwndDlg, GWLP_WNDPROC, (LONG_PTR)PropSheetWndProc);
		HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(NULL);
		HWND hwndBtn = ::CreateWindowEx(0, _T("BUTTON"), LS(STR_SHELL_INIFOLDER), BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 140, 20, hwndDlg, (HMENU)0x02000, hInstance, NULL);
		::SendMessage(hwndBtn, WM_SETFONT, (WPARAM)::SendMessage(hwndDlg, WM_GETFONT, 0, 0), MAKELPARAM(FALSE, 0));
		::SetWindowPos(hwndBtn, ::GetDlgItem(hwndDlg, IDHELP), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	return 0;
}


/*!	独自拡張プロパティシート（共通設定／タイプ別設定画面用） */
INT_PTR MyPropertySheet(LPPROPSHEETHEADER lppsph)
{
	// 個人設定フォルダを使用するときは「設定フォルダ」ボタンを追加する
	if (ShareData::getInstance().IsPrivateSettings()) {
		lppsph->dwFlags |= PSH_USECALLBACK;
		lppsph->pfnCallback = PropSheetProc;
	}
	return ::PropertySheet(lppsph);
}

///////////////////////////////////////////////////////////////////////


/*	ヘルプの目次を表示
	目次タブを表示。問題があるバージョンでは、目次ページを表示。
*/
void ShowWinHelpContents(HWND hwnd)
{
	if (HasWinHelpContentsProblem()) {
		// 目次ページを表示する
		MyWinHelp(hwnd, HELP_CONTENTS , 0);
		return;
	}
	// 目次タブを表示する
	MyWinHelp(hwnd, HELP_COMMAND, (ULONG_PTR)"CONTENTS()");
	return;
}

// NetWork上のリソースに接続するためのダイアログを出現させる
// NO_ERROR:成功 ERROR_CANCELLED:キャンセル それ以外:失敗
// プロジェクトの設定でリンクモジュールにMpr.libを追加のこと
DWORD NetConnect (const TCHAR strNetWorkPass[])
{
	//char sPassWord[] = "\0";	//パスワード
	//char sUser[] = "\0";		//ユーザー名

	if (_tcslen(strNetWorkPass) < 3)	return ERROR_BAD_NET_NAME;  // UNCではない。
	if (strNetWorkPass[0] != _T('\\') && strNetWorkPass[1] != _T('\\'))	return ERROR_BAD_NET_NAME;  // UNCではない。

	TCHAR sTemp[256];
	// 3文字目から数えて最初の\の直前までを切り出す
	sTemp[0] = _T('\\');
	sTemp[1] = _T('\\');
    int i;
	for (i=2; strNetWorkPass[i]!=_T('\0'); ++i) {
		if (strNetWorkPass[i] == _T('\\')) {
			break;
		}
		sTemp[i] = strNetWorkPass[i];
	}
	sTemp[i] = _T('\0');	// 終端

	TCHAR sDrive[] = _T("");
	// NETRESOURCE作成
	NETRESOURCE nr = {0};
	nr.dwScope       = RESOURCE_GLOBALNET;
	nr.dwType        = RESOURCETYPE_DISK;
	nr.dwDisplayType = RESOURCEDISPLAYTYPE_SHARE;
	nr.dwUsage       = RESOURCEUSAGE_CONNECTABLE;
	nr.lpLocalName   = sDrive;
	nr.lpRemoteName  = sTemp;

	// ユーザー認証ダイアログを表示
	// 戻り値　エラーコードはWINERROR.Hを参照
	DWORD dwRet = WNetAddConnection3(0, &nr, NULL, NULL, CONNECT_UPDATE_PROFILE | CONNECT_INTERACTIVE);
	return dwRet;
}


/*!
	HTML Helpコンポーネントのアクセスを提供する。
	内部で保持すべきデータは特になく、至る所から使われるのでGlobal変数にするが、
	直接のアクセスはOpenHtmlHelp()関数のみから行う。
	他のファイルからはHtmlHelpクラスは隠されている。
*/
static HtmlHelpDll g_htmlHelp;

/*!
	HTML Helpを開く
	HTML Helpが利用可能であれば引数をそのまま渡し、そうでなければメッセージを表示する。

	@return 開いたヘルプウィンドウのウィンドウハンドル。開けなかったときはNULL。
*/

HWND OpenHtmlHelp(
	HWND		hWnd,	// [in] 呼び出し元ウィンドウのウィンドウハンドル
	LPCTSTR		szFile,	// [in] HTML Helpのファイル名。不等号に続けてウィンドウタイプ名を指定可能。
	UINT		uCmd,	// [in] HTML Help に渡すコマンド
	DWORD_PTR	data,	// [in] コマンドに応じたデータ
	bool		msgflag	// [in] エラーメッセージを表示するか。省略時はtrue。
	)
{
	if (g_htmlHelp.InitDll() == InitDllResultType::Success) {
		return g_htmlHelp.HtmlHelp(hWnd, szFile, uCmd, data);
	}
	if (msgflag) {
		::MessageBox(
			hWnd,
			LS(STR_SHELL_HHCTRL),
			LS(STR_SHELL_INFO),
			MB_OK | MB_ICONEXCLAMATION
		);
	}
	return NULL;
}

/*! ショートカット(.lnk)の解決 */
BOOL ResolveShortcutLink(
	HWND hwnd,
	LPCTSTR lpszLinkFile,
	LPTSTR lpszPath
	)
{
	// 初期化
	*lpszPath = 0; // assume failure

	// Get a pointer to the IShellLink interface.
//	hRes = 0;
	TCHAR szAbsLongPath[_MAX_PATH];
	if (!::GetLongFileName(lpszLinkFile, szAbsLongPath)) {
		return FALSE;
	}

	BOOL bRes = FALSE;
	CurrentDirectoryBackupPoint dirBack;
	ChangeCurrentDirectoryToExeDir();

	IShellLink* pIShellLink = nullptr;
	IPersistFile* pIPersistFile = nullptr;
	HRESULT hRes;
	if (SUCCEEDED(hRes = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&pIShellLink))) {
		// Get a pointer to the IPersistFile interface.
		if (SUCCEEDED(hRes = pIShellLink->QueryInterface(IID_IPersistFile, (void**)&pIPersistFile))) {
			// Ensure that the string is Unicode.
			wchar_t wsz[MAX_PATH];
			_tcstowcs(wsz, szAbsLongPath, _countof(wsz));
//			MultiByteToWideChar(CP_ACP, 0, lpszLinkFile, -1, wsz, MAX_PATH);
			// Load the shortcut.
			if (SUCCEEDED(hRes = pIPersistFile->Load(wsz, STGM_READ))) {
				// Resolve the link.
				if (SUCCEEDED(hRes = pIShellLink->Resolve(hwnd, SLR_ANY_MATCH))) {
					// Get the path to the link target.
					TCHAR szGotPath[MAX_PATH];
					szGotPath[0] = _T('\0');
					WIN32_FIND_DATA	wfd;
					if (SUCCEEDED(hRes = pIShellLink->GetPath(szGotPath, MAX_PATH, &wfd, SLGP_SHORTPATH))) {
						// Get the description of the target.
						TCHAR szDescription[MAX_PATH];
						if (SUCCEEDED(hRes = pIShellLink->GetDescription(szDescription, MAX_PATH))) {
							if (_T('\0') != szGotPath[0]) {
								// 正常終了
								_tcscpy_s(lpszPath, _MAX_PATH, szGotPath);
								bRes = TRUE;
							}
						}
					}
				}
			}
		}
	}
	// Release the pointer to the IPersistFile interface.
	if (pIPersistFile) {
		pIPersistFile->Release();
		pIPersistFile = nullptr;
	}
	// Release the pointer to the IShellLink interface.
	if (pIShellLink) {
		pIShellLink->Release();
		pIShellLink = nullptr;
	}
	return bRes;
}


/*! ヘルプファイルのフルパスを返す
 
    @return パスを格納したバッファのポインタ
 
    @note 実行ファイルと同じ位置の sakura.chm ファイルを返す。
        パスが UNC のときは _MAX_PATH に収まらない可能性がある。
*/
static
LPCTSTR GetHelpFilePath()
{
	static TCHAR szHelpFile[_MAX_PATH] = _T("");
	if (szHelpFile[0] == _T('\0')) {
		GetExedir(szHelpFile, _T("sakura.chm"));
	}
	return szHelpFile;
}

/*!	WinHelp のかわりに HtmlHelp を呼び出す */
BOOL MyWinHelp(
	HWND hwndCaller,
	UINT uCommand,
	DWORD_PTR dwData
	)
{
	UINT uCommandOrg = uCommand;	// WinHelp のコマンド
	bool bDesktop = false;	// デスクトップを親にしてヘルプ画面を出すかどうか

	// Note: HH_TP_HELP_CONTEXTMENU や HELP_WM_HELP ではヘルプコンパイル時の
	// トピックファイルを Cshelp.txt 以外にしている場合、
	// そのファイル名を .chm パス名に追加指定する必要がある。
	//     例）sakura.chm::/xxx.txt

	switch (uCommandOrg) {
	case HELP_COMMAND:	// [ヘルプ]-[目次]
	case HELP_CONTENTS:
		uCommand = HH_DISPLAY_TOC;
		hwndCaller = ::GetDesktopWindow();
		bDesktop = true;
		break;
	case HELP_KEY:	// [ヘルプ]-[キーワード検索]
		uCommand = HH_DISPLAY_INDEX;
		hwndCaller = ::GetDesktopWindow();
		bDesktop = true;
		break;
	case HELP_CONTEXT:	// メニュー上での[F1]キー／ダイアログの[ヘルプ]ボタン
		uCommand = HH_HELP_CONTEXT;
		hwndCaller = ::GetDesktopWindow();
		bDesktop = true;
		break;
	case HELP_CONTEXTMENU:	// コントロール上での右クリック
	case HELP_WM_HELP:		// [？]ボタンを押してコントロールをクリック／コントロールにフォーカスを置いて[F1]キー
		uCommand = HH_DISPLAY_TEXT_POPUP;
		{
			// ポップアップヘルプ用の構造体に値をセットする
			HWND hwndCtrl;	// ヘルプ表示対象のコントロール
			int nCtrlID;	// 対象コントロールの ID
			DWORD* pHelpIDs;	// コントロール ID とヘルプ ID の対応表へのポインタ

			HH_POPUP hp = {0};	// ポップアップヘルプ用の構造体
			hp.cbStruct = sizeof(hp);
			hp.pszFont = _T("ＭＳ Ｐゴシック, 9");
			hp.clrForeground = hp.clrBackground = -1;
			hp.rcMargins.left = hp.rcMargins.top = hp.rcMargins.right = hp.rcMargins.bottom = -1;
			if (uCommandOrg == HELP_CONTEXTMENU) {
				// マウスカーソル位置から対象コントロールと表示位置を求める
				if (!::GetCursorPos(&hp.pt))
					return FALSE;
				hwndCtrl = ::WindowFromPoint(hp.pt);
			}else {
				// 対象コントロールは hwndCaller
				// [F1]キーの場合もあるので対象コントロールの位置から表示位置を決める
				RECT rc;
				hwndCtrl = hwndCaller;
				if (!::GetWindowRect(hwndCtrl, &rc))
					return FALSE;
				hp.pt.x = (rc.left + rc.right) / 2;
				hp.pt.y = rc.top;
			}
			// 対象コントロールに対応するヘルプ ID を探す
			nCtrlID = ::GetDlgCtrlID(hwndCtrl);
			if (nCtrlID <= 0)
				return FALSE;
			pHelpIDs = (DWORD*)dwData;
			for (;;) {
				if (*pHelpIDs == 0)
					return FALSE;	// 見つからなかった
				if (*pHelpIDs == (DWORD)nCtrlID)
					break;			// 見つかった
				pHelpIDs += 2;
			}
			hp.idString = *(pHelpIDs + 1);	// 見つけたヘルプ ID を設定する
			dwData = (DWORD_PTR)&hp;	// 引数をポップアップヘルプ用の構造体に差し替える
		}
		break;
	default:
		return FALSE;
	}

	LPCTSTR lpszHelp = GetHelpFilePath();
	if (IsFileExists(lpszHelp, true)) {
		// HTML ヘルプを呼び出す
		HWND hWnd = OpenHtmlHelp(hwndCaller, lpszHelp, uCommand, dwData);
		if (bDesktop && hWnd) {
			::SetForegroundWindow(hWnd);	// ヘルプ画面を手前に出す
		}
	}else {
		if (uCommandOrg == HELP_CONTEXTMENU)
			return FALSE;	// 右クリックでは何もしないでおく

		// オンラインヘルプを呼び出す
		if (uCommandOrg != HELP_CONTEXT)
			dwData = 1;	// 目次ページ

		TCHAR buf[256];
		_stprintf(buf, _T("http://sakura-editor.sourceforge.net/cgi-bin/hid2.cgi?%d"), dwData);
		ShellExecute(::GetActiveWindow(), NULL, buf, NULL, NULL, SW_SHOWNORMAL);
	}

	return TRUE;
}

/*フォント選択ダイアログ
	@param plf [in/out]
	@param piPointSize [in/out] 1/10ポイント単位
*/
BOOL MySelectFont(
	LOGFONT* plf,
	INT* piPointSize,
	HWND hwndDlgOwner,
	bool FixedFontOnly
	)
{
	CHOOSEFONT cf = {0};
	// CHOOSEFONTの初期化
	cf.lStructSize = sizeof(cf);
	cf.hwndOwner = hwndDlgOwner;
	cf.hDC = NULL;
	cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
	if (FixedFontOnly) {
		// FIXEDフォント
		cf.Flags |= CF_FIXEDPITCHONLY;
	}
	cf.lpLogFont = plf;
	if (!ChooseFont(&cf)) {
#ifdef _DEBUG
		DWORD nErr;
		nErr = CommDlgExtendedError();
		switch (nErr) {
		case CDERR_FINDRESFAILURE:	MYTRACE(_T("CDERR_FINDRESFAILURE \n"));	break;
		case CDERR_INITIALIZATION:	MYTRACE(_T("CDERR_INITIALIZATION \n"));	break;
		case CDERR_LOCKRESFAILURE:	MYTRACE(_T("CDERR_LOCKRESFAILURE \n"));	break;
		case CDERR_LOADRESFAILURE:	MYTRACE(_T("CDERR_LOADRESFAILURE \n"));	break;
		case CDERR_LOADSTRFAILURE:	MYTRACE(_T("CDERR_LOADSTRFAILURE \n"));	break;
		case CDERR_MEMALLOCFAILURE:	MYTRACE(_T("CDERR_MEMALLOCFAILURE\n"));	break;
		case CDERR_MEMLOCKFAILURE:	MYTRACE(_T("CDERR_MEMLOCKFAILURE \n"));	break;
		case CDERR_NOHINSTANCE:		MYTRACE(_T("CDERR_NOHINSTANCE \n"));		break;
		case CDERR_NOHOOK:			MYTRACE(_T("CDERR_NOHOOK \n"));			break;
		case CDERR_NOTEMPLATE:		MYTRACE(_T("CDERR_NOTEMPLATE \n"));		break;
		case CDERR_STRUCTSIZE:		MYTRACE(_T("CDERR_STRUCTSIZE \n"));		break;
		case CFERR_MAXLESSTHANMIN:	MYTRACE(_T("CFERR_MAXLESSTHANMIN \n"));	break;
		case CFERR_NOFONTS:			MYTRACE(_T("CFERR_NOFONTS \n"));			break;
		}
#endif
		return FALSE;
	}
	*piPointSize = cf.iPointSize;

	return TRUE;
}

