// 外部コマンドの実行

#include "StdAfx.h"
#include "EditView.h"
#include "_main/AppMode.h"
#include "_os/OsVersionInfo.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "env/FormatManager.h"
#include "dlg/DlgCancel.h"
#include "charset/CodeFactory.h"
#include "charset/Utf8.h"
#include "util/window.h"
#include "util/tchar_template.h"
#include "sakura_rc.h" // IDD_EXECRUNNING

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       外部コマンド                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class OutputAdapterDefault: public OutputAdapter
{
public:
	OutputAdapterDefault(EditView* view, BOOL bToEditWindow)
		:
		bWindow(bToEditWindow),
		view(view),
		commander(view->GetCommander())
	{
		pCShareData = &ShareData::getInstance();
		
	}
	~OutputAdapterDefault(){};

	bool OutputW(const wchar_t* pBuf, int size = -1);
	bool OutputA(const char* pBuf, int size = -1);
	bool IsActiveDebugWindow(){ return !bWindow; }

protected:
	void OutputBuf(const wchar_t* pBuf, int size);

	BOOL			bWindow;
	EditView*		view;
	ShareData*		pCShareData;
	ViewCommander&	commander;
};

class OutputAdapterUTF8: public OutputAdapterDefault
{
public:
	OutputAdapterUTF8(EditView* view, BOOL bToEditWindow) : OutputAdapterDefault(view, bToEditWindow)
		,pCodeBase(CodeFactory::CreateCodeBase(CODE_UTF8,0))
	{}
	~OutputAdapterUTF8(){};

	bool OutputA(const char* pBuf, int size = -1);

protected:
	std::unique_ptr<CodeBase> pCodeBase;
};

/*!	@brief	外部コマンドの実行

	@param[in] pszCmd コマンドライン
	@param[in] nFlgOpt オプション
		@li	0x01	標準出力を得る
		@li	0x02	標準出力のリダイレクト先（無効=アウトプットウィンドウ / 有効=編集中のウィンドウ）
		@li	0x04	編集中ファイルを標準入力へ
		@li	0x08	標準出力をUnicodeで行う
		@li	0x10	標準入力をUnicodeで行う
		@li	0x20	情報出力する
		@li	0x40	情報出力しない
		@li	0x80	標準出力をUTF-8で行う
		@li	0x100	標準入力をUTF-8で行う
		@li	0x200	カレントディレクトリを指定

	@note	子プロセスの標準出力取得はパイプを使用する
	@note	子プロセスの標準入力への送信は一時ファイルを使用

	TODO:	標準入力・標準エラーの取込選択。カレントディレクトリ。UTF-8等への対応
*/
bool EditView::ExecCmd(
	const TCHAR* pszCmd,
	int nFlgOpt,
	const TCHAR* pszCurDir,
	OutputAdapter* customOa
	)
{
	HANDLE hStdOutWrite, hStdOutRead;
	PROCESS_INFORMATION	pi = {0};
	DlgCancel dlgCancel;
	OutputAdapter* oaInst = nullptr;

	bool bEditable = pEditDoc->IsEditable();

	bool	bGetStdout		= (nFlgOpt & 0x01) != 0;	//	子プロセスの標準出力を得る
	BOOL	bToEditWindow	= ((nFlgOpt & 0x02) && bEditable) ? TRUE : FALSE;	//	TRUE=編集中のウィンドウ / FALSAE=アウトプットウィンドウ
	BOOL	bSendStdin		= (nFlgOpt & 0x04) ? TRUE : FALSE;	//	編集中ファイルを子プロセスSTDINに渡す
	EncodingType outputEncoding;
	if (nFlgOpt & 0x08) {
		outputEncoding = CODE_UNICODE;
	}else if (nFlgOpt & 0x80) {
		outputEncoding = CODE_UTF8;
	}else {
		outputEncoding = CODE_SJIS;
	}
	EncodingType sendEncoding;
	if (nFlgOpt & 0x10) {
		sendEncoding = CODE_UNICODE;
	}else if (nFlgOpt & 0x100) {
		sendEncoding = CODE_UTF8;
	}else {
		sendEncoding = CODE_SJIS;
	}
	BOOL bOutputExtInfo	= !bToEditWindow;
	if (nFlgOpt & 0x20) bOutputExtInfo = TRUE;
	if (nFlgOpt & 0x40) bOutputExtInfo = FALSE;
	bool bCurDir = (nFlgOpt & 0x200) == 0x200;

	// 編集中のウィンドウに出力する場合の選択範囲処理用
	Point ptFrom(0, 0);
	bool bBeforeTextSelected = GetSelectionInfo().IsTextSelected();
	if (bBeforeTextSelected) {
		ptFrom = this->GetSelectionInfo().select.GetFrom();
	}

	// 子プロセスの標準出力と接続するパイプを作成
	SECURITY_ATTRIBUTES	sa;
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = nullptr;
	if (CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 1000) == FALSE) {
		// エラー。対策無し
		return false;
	}
	// hStdOutReadのほうは子プロセスでは使用されないので継承不能にする（子プロセスのリソースを無駄に増やさない）
	DuplicateHandle(GetCurrentProcess(), hStdOutRead,
				GetCurrentProcess(), &hStdOutRead,					// 新しい継承不能ハンドルを受け取る
				0, FALSE,
				DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS);	// 元の継承可能ハンドルは DUPLICATE_CLOSE_SOURCE で閉じる


	// 子プロセスの標準入力ハンドル
	// CDocLineMgr::WriteFileなど既存のファイル出力系の関数のなかには
	// ファイルハンドルを返すタイプのものがないので、一旦書き出してから
	// 一時ファイル属性でオープンすることに。
	HANDLE hStdIn = NULL;
	if (bSendStdin) {	// 現在編集中のファイルを子プロセスの標準入力へ
		TCHAR szPathName[MAX_PATH];
		TCHAR szTempFileName[MAX_PATH];

		GetTempPath(MAX_PATH, szPathName);
		GetTempFileName(szPathName, TEXT("skr_"), 0, szTempFileName);
		DEBUG_TRACE(_T("EditView::ExecCmd() TempFilename=[%ts]\n"), szTempFileName);

		int nFlgOpt = bBeforeTextSelected ? 0x01 : 0x00;		// 選択範囲を出力
		if (!GetCommander().Command_PutFile(to_wchar(szTempFileName), sendEncoding, nFlgOpt)) {	// 一時ファイル出力
			hStdIn = NULL;
		}else {
			// 子プロセスへの継承用にファイルを開く
			hStdIn = CreateFile(
				szTempFileName,
				GENERIC_READ,
				0,
				&sa,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
				NULL
			);
			if (hStdIn == INVALID_HANDLE_VALUE) hStdIn = NULL;
		}
	}
	
	if (!hStdIn) {	// 標準入力を制御しない場合、または一時ファイルの生成に失敗した場合
		bSendStdin = FALSE;
		hStdIn = GetStdHandle(STD_INPUT_HANDLE);
		if (!hStdIn) {
			HANDLE hStdInWrite = NULL;
			if (CreatePipe(&hStdIn, &hStdInWrite, &sa, 1000) == FALSE) {
				// エラー
				hStdIn = hStdInWrite = NULL;
			}
			if (hStdInWrite) {
				::CloseHandle(hStdInWrite);
			}
		}
	}
	
	// CreateProcessに渡すSTARTUPINFOを作成
	STARTUPINFO	sui = {0};
	sui.cb = sizeof(sui);
	if (bGetStdout || bSendStdin) {
		sui.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		sui.wShowWindow = bGetStdout ? SW_HIDE : SW_SHOW;
		sui.hStdInput = hStdIn;
		sui.hStdOutput = bGetStdout ? hStdOutWrite : GetStdHandle(STD_OUTPUT_HANDLE);
		sui.hStdError = bGetStdout ? hStdOutWrite : GetStdHandle(STD_ERROR_HANDLE);
	}
	bool bRet = false;

	// コマンドライン実行
	TCHAR cmdline[1024];
	_tcscpy_s(cmdline, pszCmd);
	if (CreateProcess(NULL, cmdline, NULL, NULL, TRUE,
				CREATE_NEW_CONSOLE, NULL, bCurDir ? pszCurDir : NULL, &sui, &pi) == FALSE
	) {
		// 実行に失敗した場合、コマンドラインベースのアプリケーションと判断して
		// command(9x) か cmd(NT) を呼び出す

		TCHAR szCmdDir[_MAX_PATH];
		::GetSystemDirectory(szCmdDir, _countof(szCmdDir));

		// コマンドライン文字列作成
		auto_sprintf_s(
			cmdline,
			_T("\"%ts\\%ts\" %ts%ts%ts"),
			szCmdDir,
			_T("cmd.exe"),
			(outputEncoding == CODE_UNICODE ? _T("/U") : _T("")),		// Unicdeモードでコマンド実行
			(bGetStdout ? _T("/C ") : _T("/K ")),
			pszCmd
		);
		if (CreateProcess(NULL, cmdline, NULL, NULL, TRUE,
					CREATE_NEW_CONSOLE, NULL, bCurDir ? pszCurDir : NULL, &sui, &pi) == FALSE
		) {
			MessageBox(NULL, cmdline, LS(STR_EDITVIEW_EXECCMD_ERR), MB_OK | MB_ICONEXCLAMATION);
			goto finish;
		}
	}

	// ファイル全体に対するフィルタ動作
	//	現在編集中のファイルからのデータ書きだしおよびデータ取り込みが
	//	指定されていて，かつ範囲選択が行われていない場合は
	//	「すべて選択」されているものとして，編集データ全体を
	//	コマンドの出力結果と置き換える．
	if (1
		&& !bBeforeTextSelected
		&& bSendStdin
		&& bGetStdout
		&& bToEditWindow
	) {
		GetSelectionInfo().SetSelectArea(
			Range(
				Point(0, 0),
				Point(0, (int)pEditDoc->layoutMgr.GetLineCount())
			)
		);
		DeleteData(true);
	}

	// hStdOutWrite は CreateProcess() で継承したので親プロセスでは用済み
	// hStdInも親プロセスでは使用しないが、Win9x系では子プロセスが終了してから
	// クローズするようにしないと一時ファイルが自動削除されない
	CloseHandle(hStdOutWrite);
	hStdOutWrite = NULL;

	if (bGetStdout) {
		DWORD	new_cnt;
		int		bufidx = 0;
		bool	bLoopFlag = true;
		bool	bCancelEnd = false; // キャンセルでプロセス停止
		oaInst =  (customOa
					? NULL
					: (outputEncoding == CODE_UTF8
						? new OutputAdapterUTF8(this, bToEditWindow)
						: new OutputAdapterDefault(this, bToEditWindow)) );
		OutputAdapter& oa = customOa ? *customOa: *oaInst;

		// 中断ダイアログ表示
		if (oa.IsEnableRunningDlg()) {
			dlgCancel.DoModeless(G_AppInstance(), hwndParent, IDD_EXECRUNNING);
			// ダイアログにコマンドを表示
			::DlgItem_SetText(dlgCancel.GetHwnd(), IDC_STATIC_CMD, pszCmd);
		}
		// 実行したコマンドラインを表示
		if (bOutputExtInfo) {
			TCHAR szTextDate[1024], szTextTime[1024];
			SYSTEMTIME systime;
			::GetLocalTime(&systime);
			FormatManager().MyGetDateFormat(systime, szTextDate, _countof(szTextDate) - 1);
			FormatManager().MyGetTimeFormat(systime, szTextTime, _countof(szTextTime) - 1);
			wchar_t szOutTemp[1024 * 2 + 100];
			oa.OutputW(L"\r\n");
			oa.OutputW(L"#============================================================\r\n");
			int len = auto_snprintf_s(szOutTemp, _countof(szOutTemp),
				L"#DateTime : %ts %ts\r\n", szTextDate, szTextTime);
			oa.OutputW(szOutTemp, len);
			len = auto_snprintf_s(szOutTemp, _countof(szOutTemp),
				L"#CmdLine  : %ts\r\n", pszCmd);
			oa.OutputW(szOutTemp, len);
			oa.OutputW(L"#============================================================\r\n");
		}
		
		// charで読む
		typedef char PIPE_CHAR;
		const int WORK_NULL_TERMS = sizeof(wchar_t); // 出力用\0の分
		const int MAX_BUFIDX = 10; // bufidxの分
		const DWORD MAX_WORK_READ = 1024*5; // 5KiB ReadFileで読み込む限界値
		// ボトルネックはアウトプットウィンドウへの転送
		// 相手のプログラムがVC9のstdoutではデフォルトで4096。VC6,VC8やWinXPのtypeコマンドでは1024
		// テキストモードだと new_cntが改行に\rがつく分だけ向こう側の設定値より多く一度に送られてくる
		// 4KBだと 4096 -> 100 -> 4096 -> 100 のように読み取ることになるので5KBにした
		PIPE_CHAR work[MAX_WORK_READ + MAX_BUFIDX + WORK_NULL_TERMS];
		// 実行結果の取り込み
		do {
			// プロセスが終了していないか確認
			switch (MsgWaitForMultipleObjects(1, &pi.hProcess, FALSE, 20, QS_ALLEVENTS)) {
			case WAIT_OBJECT_0:
				// 終了していればループフラグをfalseとする
				// ただしループの終了条件は プロセス終了 && パイプが空
				bLoopFlag = false;
				break;
			case WAIT_OBJECT_0 + 1:
				// 処理中のユーザー操作を可能にする
				if (!::BlockingHook(dlgCancel.GetHwnd())) {
					// WM_QUIT受信。ただちに終了処理
					::TerminateProcess(pi.hProcess, 0);
					goto finish;
				}
				break;
			default:
				break;
			}
			// 中断ボタン押下チェック
			if (dlgCancel.IsCanceled()) {
				// 指定されたプロセスと、そのプロセスが持つすべてのスレッドを終了させます。
				::TerminateProcess(pi.hProcess, 0);
				bCancelEnd  = true;
				break;
			}
			new_cnt = 0;

			if (PeekNamedPipe(hStdOutRead, NULL, 0, NULL, &new_cnt, NULL)) {	// パイプの中の読み出し待機中の文字数を取得
				while (new_cnt > 0) {												// 待機中のものがある

					if (new_cnt > MAX_WORK_READ) {							// パイプから読み出す量を調整
						new_cnt = MAX_WORK_READ;
					}
					DWORD	read_cnt = 0;
					::ReadFile(hStdOutRead, &work[bufidx], new_cnt, &read_cnt, NULL);	// パイプから読み出し
					read_cnt += bufidx;													// work内の実際のサイズにする

					if (read_cnt == 0) {
						break;
					}
					// Unicode で データを受け取る
					if (outputEncoding == CODE_UNICODE) {
						char byteCarry = 0;
						wchar_t* workw = (wchar_t*)work;
						int read_cntw = (int)read_cnt/sizeof(wchar_t);
						if (read_cnt % (int)sizeof(wchar_t)) {
							byteCarry = work[read_cnt-1];
						}
						if (read_cntw) {
							workw[read_cntw] = L'\0';
							bool bCarry = false;
							// 読み出した文字列をチェックする
							if (workw[read_cntw-1] == L'\r') {
								bCarry = true;
								read_cntw -= 1;
								workw[read_cntw] = L'\0';
							}
							if (!oa.OutputW(workw, read_cntw)) {
								goto finish;
							}
							bufidx = 0;
							if (bCarry) {
								workw[0] = L'\r';
								bufidx = sizeof(wchar_t);
								DEBUG_TRACE(_T("ExecCmd: Carry last character [CR]\n"));
							}
						}
						if (read_cnt % (int)sizeof(wchar_t)) {
							// 高確率で0だと思うが1だと困る
							DEBUG_TRACE(_T("ExecCmd: Carry Unicode 1byte [%x]\n"), byteCarry);
							work[bufidx] = byteCarry;
							bufidx += 1;
						}
					}else if (outputEncoding == CODE_SJIS) {
						// 読み出した文字列をチェックする
						// \r\n を \r だけとか漢字の第一バイトだけを出力するのを防ぐ必要がある
						int j;
						for (j=0; j<(int)read_cnt-1; ++j) {
							if (NativeA::GetSizeOfChar(work, read_cnt, j) == 2) {
								++j;
							}else {
								if (work[j] == _T2(PIPE_CHAR, '\r') && work[j + 1] == _T2(PIPE_CHAR,'\n')) {
									++j;
								}
							}
						}
						//	改行コードが分割されるのを防ぐ
						if ((DWORD)j == read_cnt - 1) {
							if (_IS_SJIS_1(work[j])) {
								j = read_cnt + 1; // ぴったり出力できないことを主張
							}else if (work[j] == _T2(PIPE_CHAR, '\r')) {
								// CRLFの一部ではない改行が末尾にある
								// 次の読み込みで、CRLFの一部になる可能性がある
								j = read_cnt + 1;
							}else {
								j = read_cnt;
							}
						}
						if (j == (int)read_cnt) {	// ぴったり出力できる場合
							work[read_cnt] = '\0';
							if (!oa.OutputA(work, read_cnt)) {
								goto finish;
							}
							bufidx = 0;
						}else {
							char tmp = work[read_cnt-1];
							work[read_cnt-1] = '\0';
							if (!oa.OutputA(work, read_cnt-1)) {
								goto finish;
							}
							work[0] = tmp;
							bufidx = 1;
							DEBUG_TRACE(_T("ExecCmd: Carry last character [%x]\n"), tmp);
						}
					}else if (outputEncoding == CODE_UTF8) {
						int j;
						int checklen = 0;
						for (j=0; j<(int)read_cnt;) {
							ECharSet echarset;
							checklen = CheckUtf8Char2(work + j , read_cnt - j, &echarset, true, 0);
							if (echarset == CHARSET_BINARY2) {
								break;
							}else if (read_cnt - 1 == j && work[j] == _T2(PIPE_CHAR, '\r')) {
								// CRLFの一部ではない改行が末尾にある
								// 次の読み込みで、CRLFの一部になる可能性がある
								break;
							}else {
								j += checklen;
							}
						}
						if (j == (int)read_cnt) {	// ぴったり出力できる場合
							work[read_cnt] = '\0';
							if (!oa.OutputA(work, read_cnt)) {
								goto finish;
							}
							bufidx = 0;
						}else {
							DEBUG_TRACE(_T("read_cnt %d j %d\n"), read_cnt, j);
							char tmp[5];
							int len = read_cnt - j;
							memcpy(tmp, &work[j], len);
							work[j] = '\0';
							if (!oa.OutputA(work, j)) {
								goto finish;
							}
							memcpy(work, tmp, len);
							bufidx = len;
							DEBUG_TRACE(_T("ExecCmd: Carry last character [%x]\n"), tmp[0]);
						}
					}
					// 子プロセスの出力をどんどん受け取らないと子プロセスが
					// 停止してしまうため，バッファが空になるまでどんどん読み出す．
					new_cnt = 0;
					if (!PeekNamedPipe(hStdOutRead, NULL, 0, NULL, &new_cnt, NULL)) {
						break;
					}
					Sleep(0);
					
					// 相手が出力しつづけていると止められないから
					// BlockingHookとキャンセル確認を読取ループ中でも行う
					// bLoopFlag が立っていないときは、すでにプロセスは終了しているからTerminateしない
					if (!::BlockingHook(dlgCancel.GetHwnd())) {
						if (bLoopFlag) {
							::TerminateProcess(pi.hProcess, 0);
						}
						goto finish;
					}
					if (dlgCancel.IsCanceled()) {
						// 指定されたプロセスと、そのプロセスが持つすべてのスレッドを終了させます。
						if (bLoopFlag) {
							::TerminateProcess(pi.hProcess, 0);
						}
						bCancelEnd = true;
						goto user_cancel;
					}
				}
			}
		} while (bLoopFlag || new_cnt > 0);

user_cancel:

		// 最後の文字の出力(たいていCR)
		if (0 < bufidx) {
			if (outputEncoding == CODE_UNICODE) {
				if (bufidx % (int)sizeof(wchar_t)) {
					DEBUG_TRACE(_T("ExecCmd: Carry last Unicode byte [%x]\n"), work[bufidx-1]);
					// UTF-16なのに奇数バイトだった
					work[bufidx] = 0x00; // 上位バイトを0にしてごまかす
					bufidx += 1;
				}
				wchar_t* workw = (wchar_t*)work;
				int bufidxw = bufidx / (int)sizeof(wchar_t);
				workw[bufidxw] = L'\0';
				oa.OutputW(workw, bufidxw);
			}else if (outputEncoding == CODE_SJIS) {
				work[bufidx] = '\0';
				oa.OutputA(work, bufidx);
			}else if (outputEncoding == CODE_UTF8) {
				work[bufidx] = '\0';
				oa.OutputA(work, bufidx);
			}
		}

		if (bCancelEnd && bOutputExtInfo) {
			// 最後にテキストを追加
			oa.OutputW(LSW(STR_EDITVIEW_EXECCMD_STOP));
		}
		
		{
			DWORD result;
			::GetExitCodeProcess(pi.hProcess, &result);
			if (bOutputExtInfo) {
				wchar_t endCode[128];
				auto_sprintf_s(endCode, LSW(STR_EDITVIEW_EXECCMD_RET), result);
				oa.OutputW(endCode);
			}
			// 終了コードが1以上の時はアウトプットをアクティブにする
			if (!bToEditWindow && result > 0 && oa.IsActiveDebugWindow()) {
				ActivateFrameWindow( GetDllShareData().handles.hwndDebug );
			}
		}
		if (bToEditWindow) {
			if (bBeforeTextSelected) {	// 挿入された部分を選択状態に
				GetSelectionInfo().SetSelectArea(
					Range(
						ptFrom,
						GetCaret().GetCaretLayoutPos()// Point(nCaretPosY, nCaretPosX)
					)
				);
				GetSelectionInfo().DrawSelectArea();
			}
			// 編集中のウィンドウに出力時は最後に再描画
			RedrawAll();
		}
		if (!bCancelEnd) {
			bRet = true;
		}
	}else {
		bRet = true;
	}


finish:
	// 終了処理
	if (hStdIn) CloseHandle(hStdIn);	// 2007標準入力の制御のため
	if (hStdOutWrite) CloseHandle(hStdOutWrite);
	CloseHandle(hStdOutRead);
	if (pi.hProcess) CloseHandle(pi.hProcess);
	if (pi.hThread) CloseHandle(pi.hThread);
	delete oaInst;
	return bRet;
}

/*!
	@param pBuf size未指定なら要NUL終端
	@param size wchar_t単位 
*/
void OutputAdapterDefault::OutputBuf(const wchar_t* pBuf, int size)
{
	if (bWindow) {
		commander.Command_InsText(false, pBuf, size, true);
	}else {
		pCShareData->TraceOutString(pBuf , size);
	}
}

bool OutputAdapterDefault::OutputW(const wchar_t* pBuf, int size)
{
	OutputBuf(pBuf, size);
	return true;
}

/*
	@param pBuf size未指定なら要NUL終端
	@param size char単位 
*/
bool OutputAdapterDefault::OutputA(const char* pBuf, int size)
{
	NativeW buf;
	if (size == -1) {
		buf.SetStringOld(pBuf);
	}else {
		buf.SetStringOld(pBuf, size);
	}
	OutputBuf( buf.GetStringPtr(), (int)buf.GetStringLength() );
	return true;
}

/*
	@param pBuf size未指定なら要NUL終端
	@param size char単位 
*/
bool OutputAdapterUTF8::OutputA(const char* pBuf, int size)
{
	Memory input;
	NativeW buf;
	if (size == -1) {
		input.SetRawData(pBuf, strlen(pBuf));
	}else {
		input.SetRawData(pBuf, size);
	}
	pCodeBase->CodeToUnicode(input, &buf);
	OutputBuf(buf.GetStringPtr(), (int)buf.GetStringLength());
	return true;
}

