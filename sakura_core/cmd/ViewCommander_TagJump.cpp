#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "uiparts/WaitCursor.h"
#include "dlg/DlgCancel.h"
#include "dlg/DlgTagJumpList.h"
#include "dlg/DlgTagsMake.h"
#include "EditApp.h"
#include "_os/OsVersionInfo.h"
#include "util/window.h"
#include "util/module.h"
#include "util/string_ex2.h"
#include "env/SakuraEnvironment.h"
#include "GrepAgent.h"
#include "sakura_rc.h"

// ViewCommanderクラスのコマンド(タグジャンプ)関数群

// "までを切り取る
static
bool GetQuoteFilePath(const wchar_t* pLine, wchar_t* pFile, size_t size) {
	const wchar_t* pFileEnd = wcschr(pLine, L'\"');
	if (pFileEnd) {
		ptrdiff_t nFileLen = pFileEnd - pLine;
		if (0 < nFileLen && nFileLen < (int)size) {
			wmemcpy(pFile, pLine, nFileLen);
			pFile[nFileLen] = L'\0';
			return true;
		}
	}
	return false;
}

static
bool IsFileExists2(const wchar_t* pszFile)
{
	for (size_t i=0; pszFile[i]; ++i) {
		if (!WCODE::IsValidFilenameChar(pszFile, i)) {
			return false;
		}
	}
	if (_IS_REL_PATH(to_tchar(pszFile))) {
		return false;
	}
	return IsFileExists(to_tchar(pszFile), true);
}

/*! タグジャンプ

	@param bClose [in] true:元ウィンドウを閉じる
*/
bool ViewCommander::Command_TagJump(bool bClose)
{
	// 初期値を1ではなく元の位置を継承するように
	// 0以下は未指定扱い。(1開始)
	int			nJumpToLine;
	int			nJumpToColumn;
	nJumpToLine = 0;
	nJumpToColumn = 0;

	// ファイル名バッファ
	wchar_t		szJumpToFile[1024];
	wchar_t		szFile[_MAX_PATH] = {L'\0'};
	size_t		nBgn;
	int			nPathLen;
	wmemset(szJumpToFile, 0, _countof(szJumpToFile));

	/*
	  カーソル位置変換
	  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	  →
	  物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	Point ptXY = GetDocument().layoutMgr.LayoutToLogic(GetCaret().GetCaretLayoutPos());
	Point ptXYOrg = ptXY;
	// 現在行のデータを取得
	size_t nLineLen;
	const wchar_t* pLine = GetDocument().docLineMgr.GetLine(ptXY.y)->GetDocLineStrWithEOL(&nLineLen);
	if (!pLine) {
		goto can_not_tagjump;
	}

	// ノーマル
	// C:\RootFolder\SubFolders\FileName.ext(5395,11): str

	// ノーマル/ベースフォルダ/フォルダ毎
	// ◎"C:\RootFolder"
	// ■
	// ・FileName.ext(5395,11): str
	// ■"SubFolders"
	// ・FileName.ext(5395,11): str
	// ・FileName.ext(5396,11): str
	// ・FileName2.ext(123,12): str

	// ノーマル/ベースフォルダ
	// ■"C:\RootFolder"
	// ・FileName.ext(5395,11): str
	// ・SubFolders\FileName2.ext(5395,11): str
	// ・SubFolders\FileName2.ext(5396,11): str
	// ・SubFolders\FileName3.ext(123,11): str

	// ファイル毎(WZ風)
	// ■"C:\RootFolder\FileName.ext"
	// ・(5395,11 ): str
	// ■"C:\RootFolder\SubFolders\FileName2.ext"
	// ・(5395,11 ): str
	// ・(5396,11 ): str
	// ■"C:\RootFolder\SubFolders\FileName3.ext"
	// ・( 123,12 ): str

	// ファイル毎/ベースフォルダ
	// ◎"C:\RootFolder"
	// ■"FileName.ext"
	// ・(5395,11 ): str
	// ■"SubFolders\FileName2.ext"
	// ・(5395,11 ): str
	// ・(5396,11 ): str
	// ■"SubFolders\FileName3.ext"
	// ・( 123,12 ): str

	// ファイル毎/ベースフォルダ/フォルダ毎
	// ◎"C:\RootFolder"
	// ■
	// ◆"FileName.ext"
	// ・(5395,11 ): str
	// ■"SubFolders"
	// ◆"FileName2.ext"
	// ・(5395,11 ): str
	// ・(5396,11 ): str
	// ◆"FileName3.ext"
	// ・( 123,12 ): str

	// Grep結果のタグジャンプ検索
	// ・→◆→■→◎ の順に検索してパスを結合する
	do {
		enum TagListSeachMode {
			TAGLIST_FILEPATH,
			TAGLIST_SUBPATH,
			TAGLIST_ROOT,
		} searchMode = TAGLIST_FILEPATH;
		if (wmemcmp(pLine, L"■\"", 2) == 0) {
			// WZ風のタグリストか
			if (IsFilePath(&pLine[2], &nBgn, &nPathLen) && !_IS_REL_PATH(to_tchar(&pLine[2]))) {
				wmemcpy(szJumpToFile, &pLine[2 + nBgn], nPathLen);
				GetLineColumn(&pLine[2] + nPathLen, &nJumpToLine, &nJumpToColumn);
				break;
			}else if (!GetQuoteFilePath(&pLine[2], szFile, _countof(szFile))) {
				break;
			}
			searchMode = TAGLIST_ROOT;
		}else if (wmemcmp(pLine, L"◆\"", 2) == 0) {
			if (!GetQuoteFilePath(&pLine[2], szFile, _countof(szFile))) {
				break;
			}
			searchMode = TAGLIST_SUBPATH;
		}else if (wmemcmp(pLine, L"・", 1) == 0) {
			if (pLine[1] == L'"') {
				// ・"FileName.ext"
				if (!GetQuoteFilePath(&pLine[2], szFile, _countof(szFile))) {
					break;
				}
				searchMode = TAGLIST_SUBPATH;
			}else if (pLine[1] == L'(') {
				// ファイル毎(WZ風)
				GetLineColumn(&pLine[1], &nJumpToLine, &nJumpToColumn);
				searchMode = TAGLIST_FILEPATH;
			}else {
				// ノーマル/ファイル相対パス
				// ･FileName.ext(123,45): str
				// ･FileName.ext(123,45)  [SJIS]: str
				const wchar_t* pTagEnd = wcsstr(pLine, L"): ");
				if (!pTagEnd) {
					pTagEnd = wcsstr(pLine, L"]: ");
					if (pTagEnd) {
						ptrdiff_t fileEnd = pTagEnd - pLine - 1;
						for (; 1<fileEnd; --fileEnd) {
							if (L'[' == pLine[fileEnd]) {
								--fileEnd;
								break;
							}
						}
						for (; 1<fileEnd && L' '==pLine[fileEnd]; --fileEnd) {}
						if (')' == pLine[fileEnd]) {
							pTagEnd = &pLine[fileEnd];
						}else {
							pTagEnd = NULL;
						}
					}
				}
				if (pTagEnd) {
					ptrdiff_t fileEnd = pTagEnd - pLine - 1;
					for (; 1<fileEnd && (L'0' <= pLine[fileEnd] && pLine[fileEnd] <= L'9'); --fileEnd) {}
					if (1 < fileEnd && (L',' == pLine[fileEnd])) { --fileEnd; }
					for (; 1<fileEnd && (L'0' <= pLine[fileEnd] && pLine[fileEnd] <= L'9'); --fileEnd) {}
					if (1 < fileEnd && L'(' == pLine[fileEnd] && fileEnd - 1 < (int)_countof(szFile)) {
						wmemcpy(szFile, pLine + 1, fileEnd - 1);
						szFile[fileEnd - 1] = L'\0';
						GetLineColumn(&pLine[fileEnd + 1], &nJumpToLine, &nJumpToColumn);
						searchMode = TAGLIST_SUBPATH;
					}else {
						break;
					}
				}
			}
		}else {
			break;
		}
		ptXY.y--;

		for (; 0<=ptXY.y; --ptXY.y) {
			pLine = GetDocument().docLineMgr.GetLine(ptXY.y)->GetDocLineStrWithEOL(&nLineLen);
			if (!pLine) {
				break;
			}
			if (wmemcmp(pLine, L"・", 1) == 0) {
				continue;
			}else if (3 <= nLineLen && wmemcmp(pLine, L"◆\"", 2) == 0) {
				if (searchMode == TAGLIST_SUBPATH || searchMode == TAGLIST_ROOT) {
					continue;
				}
				// フォルダ毎：ファイル名
				if (GetQuoteFilePath(&pLine[2], szFile, _countof(szFile))) {
					searchMode = TAGLIST_SUBPATH;
					continue;
				}
				break;
			}else if (2 <= nLineLen && pLine[0] == L'■' && (pLine[1] == L'\r' || pLine[1] == L'\n')) {
				// ルートフォルダ
				if (searchMode == TAGLIST_ROOT) {
					continue;
				}
				searchMode = TAGLIST_ROOT;
			}else if (3 <= nLineLen && wmemcmp(pLine, L"■\"", 2) == 0) {
				if (searchMode == TAGLIST_ROOT) {
					continue;
				}
				// ファイル毎(WZ風)：フルパス
				if (IsFilePath(&pLine[2], &nBgn, &nPathLen) && !_IS_REL_PATH(to_tchar(&pLine[2]))) {
					wmemcpy(szJumpToFile, &pLine[2 + nBgn], nPathLen);
					break;
				}
				// 相対フォルダorファイル名
				wchar_t		szPath[_MAX_PATH];
				if (GetQuoteFilePath(&pLine[2], szPath, _countof(szPath))) {
					if (szFile[0]) {
						AddLastYenFromDirectoryPath(szPath);
					}
					auto_strcat(szPath, szFile);
					if (IsFileExists2(szPath)) {
						auto_strcpy(szJumpToFile, szPath);
						break;
					}
					// 相対パスだった→◎”を探す
					auto_strcpy(szFile, szPath);
					searchMode = TAGLIST_ROOT;
					continue;
				}
				break;
			}else if (3 <= nLineLen && wmemcmp(pLine, L"◎\"", 2) == 0) {
				if (GetQuoteFilePath(&pLine[2], szJumpToFile, _countof(szJumpToFile))) {
					AddLastYenFromDirectoryPath(szJumpToFile);
					auto_strcat(szJumpToFile, szFile);
					if (IsFileExists2(szJumpToFile)) {
						break;
					}
				}
				break;
			}else {
				break;
			}
		}
	}while (0);

	if (szJumpToFile[0] == L'\0') {
		pLine = GetDocument().docLineMgr.GetLine(ptXYOrg.y)->GetDocLineStrWithEOL(&nLineLen);
		if (!pLine) {
			goto can_not_tagjump;
		}
		const wchar_t* p = pLine;
		const wchar_t* p_end = p + nLineLen;

		// Borland 形式のメッセージからのTAG JUMP
		while (p < p_end) {
			// skip space
			for (; p < p_end && (*p == L' ' || *p == L'\t' || WCODE::IsLineDelimiter(*p, GetDllShareData().common.edit.bEnableExtEol)); ++p)
				;
			if (p >= p_end)
				break;
		
			// Check Path
			if (IsFilePath(p, &nBgn, &nPathLen)) {
				wmemcpy(szJumpToFile, &p[nBgn], nPathLen);
				GetLineColumn(&p[nBgn + nPathLen], &nJumpToLine, &nJumpToColumn);
				break;
			}
			// skip non-space
			for (; p < p_end && (*p != L' ' && *p != L'\t'); ++p)
				;
		}
	}
	
	// Grep形式で失敗した後もTagsを検索する
	if (szJumpToFile[0] == L'\0') {
		if (Command_TagJumpByTagsFile(bClose)) {
			return true;
		}
	}

	if (szJumpToFile[0]) {
		if (view.TagJumpSub(to_tchar(szJumpToFile), Point(nJumpToColumn, nJumpToLine), bClose)) {
			return true;
		}
	}

can_not_tagjump:;
	view.SendStatusMessage(LS(STR_ERR_TAGJMP1));
	return false;
}


// タグジャンプバック
void ViewCommander::Command_TagJumpBack(void)
{
	TagJump tagJump;

	// タグジャンプ情報の参照
	if (!TagJumpManager().PopTagJump(&tagJump) || !IsSakuraMainWindow(tagJump.hwndReferer)) {
		view.SendStatusMessage(LS(STR_ERR_TAGJMPBK1));
		return;
	}

	// アクティブにする
	ActivateFrameWindow(tagJump.hwndReferer);

	// カーソルを移動させる
	memcpy_raw(GetDllShareData().workBuffer.GetWorkBuffer<void>(), &(tagJump.point), sizeof(tagJump.point));
	::SendMessage(tagJump.hwndReferer, MYWM_SETCARETPOS, 0, 0);

	return;
}


/*
	タグファイルを作成する。
*/
bool ViewCommander::Command_TagsMake(void)
{
#define	CTAGS_COMMAND	_T("ctags.exe")

	TCHAR	szTargetPath[1024 /*_MAX_PATH+1*/];
	auto& docFile = GetDocument().docFile;
	if (docFile.GetFilePathClass().IsValidPath()) {
		_tcscpy_s(szTargetPath, docFile.GetFilePath());
		szTargetPath[_tcslen(szTargetPath) - _tcslen(docFile.GetFileName())] = _T('\0');
	}else {
		// サクラのフォルダからカレントディレクトリに変更
		::GetCurrentDirectory(_countof(szTargetPath), szTargetPath);
	}

	// ダイアログを表示する
	DlgTagsMake	dlgTagsMake;
	if (!dlgTagsMake.DoModal(G_AppInstance(), view.GetHwnd(), 0, szTargetPath)) {
		return false;
	}

	TCHAR	cmdline[1024];
	// exeのあるフォルダ
	TCHAR	szExeFolder[_MAX_PATH + 1];

	GetExedir(cmdline, CTAGS_COMMAND);
	SplitPath_FolderAndFile(cmdline, szExeFolder, NULL);

	// ctags.exeの存在チェック
	if (::GetFileAttributes(cmdline) == (DWORD)-1) {
		WarningMessage(view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD03));
		return false;
	}

	HANDLE	hStdOutWrite, hStdOutRead;
	DlgCancel	dlgCancel;
	WaitCursor	waitCursor(view.GetHwnd());

	PROCESS_INFORMATION	pi = {0};

	// 子プロセスの標準出力と接続するパイプを作成
	SECURITY_ATTRIBUTES	sa = {0};
	sa.nLength              = sizeof(sa);
	sa.bInheritHandle       = TRUE;
	sa.lpSecurityDescriptor = nullptr;
	hStdOutRead = hStdOutWrite = 0;
	if (CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 1000) == FALSE) {
		// エラー
		return false;
	}

	// 継承不能にする
	DuplicateHandle(GetCurrentProcess(), hStdOutRead,
				GetCurrentProcess(), NULL,
				0, FALSE, DUPLICATE_SAME_ACCESS);

	// CreateProcessに渡すSTARTUPINFOを作成
	STARTUPINFO	sui = {0};
	sui.cb          = sizeof(sui);
	sui.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	sui.wShowWindow = SW_HIDE;
	sui.hStdInput   = GetStdHandle(STD_INPUT_HANDLE);
	sui.hStdOutput  = hStdOutWrite;
	sui.hStdError   = hStdOutWrite;

	TCHAR	options[1024];
	_tcscpy(options, _T("--excmd=n"));	// デフォルトのオプション
	if (dlgTagsMake.nTagsOpt & 0x0001) _tcscat(options, _T(" -R"));	// サブフォルダも対象
	if (dlgTagsMake.szTagsCmdLine[0] != _T('\0')) {	// 個別指定のコマンドライン
		_tcscat(options, _T(" "));
		_tcscat(options, dlgTagsMake.szTagsCmdLine);
	}
	_tcscat(options, _T(" *"));	// 配下のすべてのファイル

	// コマンドライン文字列作成(MAX:1024)
	if (IsWin32NT()) {
		// システムディレクトリ付加
		TCHAR szCmdDir[_MAX_PATH];
		::GetSystemDirectory(szCmdDir, _countof(szCmdDir));
		// add /D to disable autorun
		auto_sprintf(
			cmdline,
			_T("\"%ts\\cmd.exe\" /D /C \"\"%ts\\%ts\" %ts\""),
			szCmdDir,
			szExeFolder,	// sakura.exeパス
			CTAGS_COMMAND,	// ctags.exe
			options			// ctagsオプション
		);
	}else {
		// 2010システムディレクトリ付加
		TCHAR szCmdDir[_MAX_PATH];
		::GetWindowsDirectory(szCmdDir, _countof(szCmdDir));
		auto_sprintf(
			cmdline,
			_T("\"%ts\\command.com\" /C \"%ts\\%ts\" %ts"),
			szCmdDir,
			szExeFolder,	//sakura.exeパス
			CTAGS_COMMAND,	//ctags.exe
			options			//ctagsオプション
		);
	}

	// コマンドライン実行
	BOOL bProcessResult = CreateProcess(
		NULL, cmdline, NULL, NULL, TRUE,
		CREATE_NEW_CONSOLE, NULL, dlgTagsMake.szPath, &sui, &pi
	);
	if (!bProcessResult) {
		WarningMessage(view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD04), cmdline);
		goto finish;
	}

	{
		DWORD	new_cnt;
		char	work[1024];
		bool	bLoopFlag = true;

		// 中断ダイアログ表示
		HWND hwndCancel = dlgCancel.DoModeless(G_AppInstance(), view.hwndParent, IDD_EXECRUNNING);
		HWND hwndMsg = ::GetDlgItem(hwndCancel, IDC_STATIC_CMD);
		SetWindowText(hwndMsg, LS(STR_ERR_CEDITVIEW_CMD05));

		// 実行結果の取り込み
		do {
			// CPU消費を減らすために200msec待つ
			switch (MsgWaitForMultipleObjects(1, &pi.hProcess, FALSE, 200, QS_ALLEVENTS)) {
			case WAIT_OBJECT_0:
				// 終了していればループフラグをfalseとする
				// ただしループの終了条件は プロセス終了 && パイプが空
				bLoopFlag = false;
				break;
			case WAIT_OBJECT_0 + 1:
				// 処理中のユーザー操作を可能にする
				if (!::BlockingHook(dlgCancel.GetHwnd())) {
					break;
				}
				break;
			default:
				break;
			}

			// 中断ボタン押下チェック
			if (dlgCancel.IsCanceled()) {
				// 指定されたプロセスと、そのプロセスが持つすべてのスレッドを終了させます。
				::TerminateProcess(pi.hProcess, 0);
				break;
			}

			new_cnt = 0;
			if (PeekNamedPipe(hStdOutRead, NULL, 0, NULL, &new_cnt, NULL)) {	// パイプの中の読み出し待機中の文字数を取得
				if (new_cnt > 0) {												// 待機中のものがある
					if (new_cnt >= _countof(work) - 2) {						// パイプから読み出す量を調整
						new_cnt = _countof(work) - 2;
					}
					DWORD read_cnt;
					::ReadFile(hStdOutRead, &work[0], new_cnt, &read_cnt, NULL);	// パイプから読み出し
					if (read_cnt == 0) {
						continue;
					}else {
						// 正常終了の時はメッセージが出力されないので
						// 何か出力されたらエラーメッセージと見なす．
					
						// 終了処理
						CloseHandle(hStdOutWrite);
						CloseHandle(hStdOutRead);
						if (pi.hProcess) CloseHandle(pi.hProcess);
						if (pi.hThread) CloseHandle(pi.hThread);

						dlgCancel.CloseDialog(TRUE);

						work[read_cnt] = L'\0';	// 表示用に0終端する
						WarningMessage(view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD06), work);

						return true;
					}
				}
			}
			Sleep(0);
		} while (bLoopFlag || new_cnt > 0);

	}


finish:
	// 終了処理
	CloseHandle(hStdOutWrite);
	CloseHandle(hStdOutRead);
	if (pi.hProcess) CloseHandle(pi.hProcess);
	if (pi.hThread) CloseHandle(pi.hThread);

	dlgCancel.CloseDialog(TRUE);

	InfoMessage(view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD07));

	return true;
}


/*
	ダイレクトタグジャンプ(メッセージ付)
*/
bool ViewCommander::Command_TagJumpByTagsFileMsg(bool bMsg)
{
	bool ret = Command_TagJumpByTagsFile(false);
	if (!ret && bMsg) {
		view.SendStatusMessage(LS(STR_ERR_TAGJMP1));
	}
	return ret;
}


/*
	ダイレクトタグジャンプ
*/
bool ViewCommander::Command_TagJumpByTagsFile(bool bClose)
{
	NativeW memKeyW;
	view.GetCurrentTextForSearch(memKeyW, true, true);
	if (memKeyW.GetStringLength() == 0) {
		return false;
	}
	
	TCHAR szDirFile[1024];
	if (!Sub_PreProcTagJumpByTagsFile(szDirFile, _countof(szDirFile))) {
		return false;
	}
	DlgTagJumpList	dlgTagJumpList(true);	// タグジャンプリスト
	
	dlgTagJumpList.SetFileName(szDirFile);
	dlgTagJumpList.SetKeyword(memKeyW.GetStringPtr());

	int nMatchAll = dlgTagJumpList.FindDirectTagJump();

	// 複数あれば選択してもらう。
	if (1 < nMatchAll) {
		if (! dlgTagJumpList.DoModal(G_AppInstance(), view.GetHwnd(), (LPARAM)0)) {
			nMatchAll = 0;
			return true;	// キャンセル
		}
	}

	// タグジャンプする。
	if (0 < nMatchAll) {
		TCHAR fileName[1024];
		int   fileLine;

		if (!dlgTagJumpList.GetSelectedFullPathAndLine(fileName, _countof(fileName), &fileLine , NULL)) {
			return false;
		}
		return view.TagJumpSub(fileName, Point(0, fileLine), bClose);
	}

	return false;
}


/*!
	キーワードを指定してタグジャンプ(ダイアログ)
	@param keyword NULL許容
*/
bool ViewCommander::Command_TagJumpByTagsFileKeyword(const wchar_t* keyword)
{
	TCHAR szCurrentPath[1024];

	if (!Sub_PreProcTagJumpByTagsFile(szCurrentPath, _countof(szCurrentPath))) {
		return false;
	}

	DlgTagJumpList dlgTagJumpList(false);
	dlgTagJumpList.SetFileName(szCurrentPath);
	dlgTagJumpList.SetKeyword(keyword);

	if (!dlgTagJumpList.DoModal(G_AppInstance(), view.GetHwnd(), 0)) {
		return true;	// キャンセル
	}

	// タグジャンプする。
	TCHAR fileName[1024];
	int	 fileLine;	// 行番号
	if (!dlgTagJumpList.GetSelectedFullPathAndLine(fileName, _countof(fileName), &fileLine, NULL)) {
		return false;
	}

	return view.TagJumpSub(fileName, Point(0, fileLine));
}


/*!
	タグジャンプの前処理
	実行可能確認と、基準ファイル名の設定
*/
bool ViewCommander::Sub_PreProcTagJumpByTagsFile(TCHAR* szCurrentPath, size_t count)
{
	if (count) {
		szCurrentPath[0] = _T('\0');
	}

	// 実行可能確認
	auto& docFile = GetDocument().docFile;
	if (! docFile.GetFilePathClass().IsValidPath()) {
		// Grep、アウトプットは行番号タグジャンプがあるので無効にする(要検討)
		if (
			EditApp::getInstance().pGrepAgent->bGrepMode
		    || AppMode::getInstance().IsDebugMode()
		) {
		    return false;
		}
	}
	
	// 基準ファイル名の設定
	if (docFile.GetFilePathClass().IsValidPath()) {
		auto_strcpy(szCurrentPath, docFile.GetFilePath());
	}else {
		if (::GetCurrentDirectory(count - _countof(_T("\\dmy")) - MAX_TYPES_EXTS, szCurrentPath) == 0) {
			return false;
		}
		// (無題)でもファイル名を要求してくるのでダミーをつける
		// 現在のタイプ別の1番目の拡張子を拝借
		TCHAR szExts[MAX_TYPES_EXTS];
		DocTypeManager::GetFirstExt(view.pTypeData->szTypeExts, szExts, _countof(szExts));
		size_t nExtLen = auto_strlen( szExts );
		_tcscat(szCurrentPath, _T("\\dmy"));
		if (nExtLen) {
			_tcscat(szCurrentPath, _T("."));
			_tcscat( szCurrentPath, szExts );
		}
	}
	return true;
}
