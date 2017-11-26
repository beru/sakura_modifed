// DIFF差分表示
#include "StdAfx.h"
#include <stdio.h>
#include <stdlib.h>
#include "view/EditView.h"
#include "_main/global.h"
#include "_main/Mutex.h"
#include "dlg/DlgDiff.h"
#include "doc/EditDoc.h"
#include "doc/logic/DocLine.h"
#include "doc/logic/DocLineMgr.h"
#include "uiparts/WaitCursor.h"
#include "_os/OsVersionInfo.h"
#include "env/ShareData.h"
#include "env/SakuraEnvironment.h"
#include "util/module.h"
#include "util/fileUtil.h"
#include "window/EditWnd.h"
#include "io/TextStream.h"
#include "io/FileLoad.h"
#include "WriteManager.h"
#include "sakura_rc.h"

#define	SAKURA_DIFF_TEMP_PREFIX	_T("sakura_diff_")

class OutputAdapterDiff: public OutputAdapter
{
public:
	OutputAdapterDiff(EditView* view, int nFlgFile12_){
		this->view = view;
		bLineHead = true;
		bDiffInfo = false;
		nDiffLen = 0;
		bFirst = true;
		nFlgFile12 = nFlgFile12_;
		szDiffData[0] = 0;
	}
	~OutputAdapterDiff(){};

	bool OutputW(const wchar_t* pBuf, int size = -1){ return true; };
	bool OutputA(const char* pBuf, int size = -1);
	bool IsEnableRunningDlg(){ return false; }
	bool IsActiveDebugWindow(){ return false; }

public:
	bool	bDiffInfo;			// DIFF情報か
	int		nDiffLen;			// DIFF情報長
	char	szDiffData[100];	// DIFF情報
protected:
	EditView* view;
	bool	bLineHead;			// 行頭か
	bool	bFirst;				// 先頭か？
	int		nFlgFile12;
};


/*!	差分表示
	@param	pszFile1	[in]	自ファイル名
	@param	pszFile2	[in]	相手ファイル名
    @param  nFlgOpt     [in]    0b000000000
                                    ||||||+--- -i ignore-case         大文字小文字同一視
                                    |||||+---- -w ignore-all-space    空白無視
                                    ||||+----- -b ignore-space-change 空白変更無視
                                    |||+------ -B ignore-blank-lines  空行無視
                                    ||+------- -t expand-tabs         TAB-SPACE変換
                                    |+--------    (編集中のファイルが旧ファイル)
                                    +---------    (DIFF差分がないときにメッセージ表示)
	@note	HandleCommandからの呼び出し対応(ダイアログなし版)
*/
void EditView::ViewDiffInfo(
	const TCHAR*	pszFile1,
	const TCHAR*	pszFile2,
	int				nFlgOpt,
	bool 			bUTF8
	)
{
	WaitCursor	waitCursor(this->GetHwnd());
	int		nFlgFile12 = 1;

	// exeのあるフォルダ
	TCHAR	szExeFolder[_MAX_PATH + 1];

	TCHAR	cmdline[1024];
	GetExedir(cmdline, _T("diff.exe"));
	SplitPath_FolderAndFile(cmdline, szExeFolder, NULL);

	//	diff.exeの存在チェック
	if (!IsFileExists(cmdline, true)) {
		WarningMessage(GetHwnd(), LS(STR_ERR_DLGEDITVWDIFF2));
		return;
	}
	cmdline[0] = _T('\0');

	// 今あるDIFF差分を消去する。
	if (DiffManager::getInstance().IsDiffUse())
		GetCommander().Command_Diff_Reset();

	// オプションを作成する
	TCHAR	szOption[16];	// "-cwbBt"
	_tcscpy(szOption, _T("-"));
	if (nFlgOpt & 0x0001) _tcscat(szOption, _T("i"));	// -i ignore-case         大文字小文字同一視
	if (nFlgOpt & 0x0002) _tcscat(szOption, _T("w"));	// -w ignore-all-space    空白無視
	if (nFlgOpt & 0x0004) _tcscat(szOption, _T("b"));	// -b ignore-space-change 空白変更無視
	if (nFlgOpt & 0x0008) _tcscat(szOption, _T("B"));	// -B ignore-blank-lines  空行無視
	if (nFlgOpt & 0x0010) _tcscat(szOption, _T("t"));	// -t expand-tabs         TAB-SPACE変換
	if (_tcscmp(szOption, _T("-")) == 0) _tcscpy(szOption, _T(""));	// オプションなし
	if (nFlgOpt & 0x0020) nFlgFile12 = 0;
	else                  nFlgFile12 = 1;

	{
		// コマンドライン文字列作成(MAX:1024)
		auto_sprintf(
			cmdline,
			_T("\"%ts\\%ts\" %ts \"%ts\" \"%ts\""),
			szExeFolder,		// sakura.exeパス
			_T("diff.exe"),		// diff.exe
			szOption,			// diffオプション
			(nFlgFile12 ? pszFile2 : pszFile1),
			(nFlgFile12 ? pszFile1 : pszFile2)
		);
	}

	{
		int nFlgOpt = 0;
		nFlgOpt |= 0x01;  // GetStdOut
		if (bUTF8) {
			nFlgOpt |= 0x80;  // UTF-8 out (SJISと違ってASCIIセーフなので)
			nFlgOpt |= 0x100; // UTF-8 in
		}
		nFlgOpt |= 0x40;  // 拡張情報出力無効
		OutputAdapterDiff oa(this, nFlgFile12);
		bool ret = ExecCmd( cmdline, nFlgOpt, NULL, &oa );

		if (ret) {
			if (oa.bDiffInfo && oa.nDiffLen > 0) {
				oa.szDiffData[oa.nDiffLen] = '\0';
				AnalyzeDiffInfo( oa.szDiffData, nFlgFile12 );
			}
		}
	}

	// DIFF差分が見つからなかったときにメッセージ表示
	if (nFlgOpt & 0x0040) {
		if (!DiffManager::getInstance().IsDiffUse()) {
			InfoMessage( this->GetHwnd(), LS(STR_ERR_DLGEDITVWDIFF5) );
		}
	}


	// 分割したビューも更新
	editWnd.Views_Redraw();

	return;
}

bool OutputAdapterDiff::OutputA(const char* pBuf, int size)
{
	if (size == -1) {
		size = auto_strlen(pBuf);
	}
	//	先頭がBinary filesならバイナリファイルのため意味のある差分が取られなかった
	if (bFirst) {
		bFirst = false;
		if (strncmp( pBuf, "Binary files ", strlen( "Binary files " ) ) == 0) {
			WarningMessage(NULL, LS(STR_ERR_DLGEDITVWDIFF4));
			return false;
		}
	}

	// 読み出した文字列をチェックする
	int j;
	for (j=0; j<(int)size/*-1*/; ++j) {
		if (bLineHead) {
			if (pBuf[j] != '\n' && pBuf[j] != '\r') {
				bLineHead = false;
			
				// DIFF情報の始まりか？
				if (pBuf[j] >= '0' && pBuf[j] <= '9') {
					bDiffInfo = true;
					nDiffLen = 0;
					szDiffData[nDiffLen++] = pBuf[j];
				}
			}
		}else {
			// 行末に達したか？
			if (pBuf[j] == '\n' || pBuf[j] == '\r') {
				// DIFF情報があれば解析する
				if (bDiffInfo && nDiffLen > 0) {
					szDiffData[nDiffLen] = '\0';
					view->AnalyzeDiffInfo(szDiffData, nFlgFile12);
					nDiffLen = 0;
				}
				
				bDiffInfo = false;
				bLineHead = true;
			}else if (bDiffInfo) {
				// DIFF情報に追加する
				szDiffData[nDiffLen++] = pBuf[j];
				if (nDiffLen >= 99) {
					nDiffLen = 0;
					bDiffInfo = false;
				}
			}
		}
	}
	return true;
}

/*!	DIFF差分情報を解析しマーク登録
	@param	pszDiffInfo	[in]	新ファイル名
	@param	nFlgFile12	[in]	編集中ファイルは...
									0	ファイル1(旧ファイル)
									1	ファイル2(新ファイル)
*/
void EditView::AnalyzeDiffInfo(
	const char*	pszDiffInfo,
	int			nFlgFile12
	)
{
	/*
	 * 99a99		旧ファイル99行の次行に新ファイル99行が追加された。
	 * 99a99,99		旧ファイル99行の次行に新ファイル99～99行が追加された。
	 * 99c99		旧ファイル99行が新ファイル99行に変更された。
	 * 99,99c99,99	旧ファイル99～99行が新ファイル99～99行に変更された。
	 * 99d99		旧ファイル99行が新ファイル99行の次行から削除された。
	 * 99,99d99		旧ファイル99～99行が新ファイル99行の次行から削除された。
	 * s1,e1 mode s2,e2
	 * 先頭の場合0の次行となることもある
	 */
	const char* q;

	// 前半ファイルの開始行
	int s1 = 0;
	int e1;
	for (q=pszDiffInfo; *q; ++q) {
		if (*q == ',') break;
		if (*q == 'a' || *q == 'c' || *q == 'd') break;
		// 行番号を抽出
		if (*q >= '0' && *q <= '9') s1 = s1 * 10 + (*q - '0');
		else return;
	}
	if (!*q) return;

	// 前半ファイルの終了行
	if (*q != ',') {
		// 開始・終了行番号は同じ
		e1 = s1;
	}else {
		e1 = 0;
		for (++q; *q; ++q) {
			if (*q == 'a' || *q == 'c' || *q == 'd') break;
			// 行番号を抽出
			if (*q >= '0' && *q <= '9') e1 = e1 * 10 + (*q - '0');
			else return;
		}
	}
	if (!*q) return;

	// DIFFモードを取得
	char mode = *q;

	// 後半ファイルの開始行
	int s2 = 0;
	int e2;
	for (++q; *q; ++q) {
		if (*q == ',') break;
		// 行番号を抽出
		if (*q >= '0' && *q <= '9') s2 = s2 * 10 + (*q - '0');
		else return;
	}

	// 後半ファイルの終了行
	if (*q != ',') {
		// 開始・終了行番号は同じ
		e2 = s2;
	}else {
		e2 = 0;
		for (++q; *q; ++q) {
			// 行番号を抽出
			if (*q >= '0' && *q <= '9') e2 = e2 * 10 + (*q - '0');
			else return;
		}
	}

	// 行末に達してなければエラー
	if (*q) {
		return;
	}

	// 抽出したDIFF情報から行番号に差分マークを付ける
	if (nFlgFile12 == 0) {	// 編集中ファイルは旧ファイル
		if      (mode == 'a') DiffLineMgr(pEditDoc->docLineMgr).SetDiffMarkRange(DiffMark::Delete, s1, e1);
		else if (mode == 'c') DiffLineMgr(pEditDoc->docLineMgr).SetDiffMarkRange(DiffMark::Change, s1 - 1, e1 - 1);
		else if (mode == 'd') DiffLineMgr(pEditDoc->docLineMgr).SetDiffMarkRange(DiffMark::Append, s1 - 1, e1 - 1);
	}else {	// 編集中ファイルは新ファイル
		if      (mode == 'a') DiffLineMgr(pEditDoc->docLineMgr).SetDiffMarkRange(DiffMark::Append, s2 - 1, e2 - 1);
		else if (mode == 'c') DiffLineMgr(pEditDoc->docLineMgr).SetDiffMarkRange(DiffMark::Change, s2 - 1, e2 - 1);
		else if (mode == 'd') DiffLineMgr(pEditDoc->docLineMgr).SetDiffMarkRange(DiffMark::Delete, s2, e2);
	}
	
	return;
}

static
bool MakeDiffTmpFile_core(
	TextOutputStream& out,
	HWND hwnd,
	EditView& view,
	bool bBom
	)
{
	int y = 0;
	const wchar_t*	pLineData;
	if (!hwnd) {
		const DocLineMgr& docMgr = view.pEditDoc->docLineMgr;
		for (;;){
			size_t nLineLen;
			pLineData = docMgr.GetLine(y)->GetDocLineStrWithEOL(&nLineLen);
			// 正常終了
			if (nLineLen == 0 || !pLineData) {
				break;
			}
			if (bBom) {
				NativeW line2(L"\ufeff");
				line2.AppendString(pLineData, nLineLen);
				out.WriteString(line2.GetStringPtr(), line2.GetStringLength());
				bBom = false;
			}else {
				out.WriteString(pLineData,nLineLen);
			}
			++y;
		}
	}else if (IsSakuraMainWindow(hwnd)) {
		const int max_size = (int)GetDllShareData().workBuffer.GetWorkBufferCount<const EDIT_CHAR>();
		pLineData = GetDllShareData().workBuffer.GetWorkBuffer<const EDIT_CHAR>();
		for (;;) {
			int nLineOffset = 0;
			int nLineLen = 0; //初回用仮値
			do {
				// workBuffer#m_Workの排他制御。外部コマンド出力/TraceOut/Diffが対象
				LockGuard<Mutex> guard(ShareData::GetMutexShareWork());
				{
					nLineLen = ::SendMessage(hwnd, MYWM_GETLINEDATA, y, nLineOffset);
					if (nLineLen == 0) { return true; } // EOF => 正常終了
					if (nLineLen < 0) { return false; } // 何かエラー
					if (bBom) {
						NativeW cLine2(L"\ufeff");
						cLine2.AppendString(pLineData, t_min(nLineLen, max_size));
						out.WriteString(cLine2.GetStringPtr(), cLine2.GetStringLength());
						bBom = false;
					}else {
						out.WriteString(pLineData, t_min(nLineLen, max_size));
					}
				}
				nLineOffset += max_size;
			}while (max_size < nLineLen);
			++y;
		}
	}else {
		return false;
	}
	if (bBom) {
		out.WriteString(L"\ufeff", 1);
	}
	return true;
}

/*!	一時ファイルを作成する */
bool EditView::MakeDiffTmpFile(
	TCHAR* filename,
	HWND hWnd,
	EncodingType code,
	bool bBom
	)
{
	// 一時
	TCHAR* pszTmpName = _ttempnam(NULL, SAKURA_DIFF_TEMP_PREFIX);
	if (!pszTmpName) {
		WarningMessage(NULL, LS(STR_DIFF_FAILED));
		return false;
	}

	_tcscpy(filename, pszTmpName);
	free(pszTmpName);

	// 自分か？
	if (!hWnd) {
		CodeConvertResult eWriteResult = WriteManager().WriteFile_From_CDocLineMgr(
			pEditDoc->docLineMgr,
			SaveInfo(
				filename,
				code,
				EolType::None,
				bBom
			)
		);
		return CodeConvertResult::Failure != eWriteResult;
	}

	TextOutputStream out(filename, code, true, false);
	if (!out) {
		WarningMessage(NULL, LS(STR_DIFF_FAILED_TEMP));
		return false;
	}

	bool bError = false;
	try {
		if (!MakeDiffTmpFile_core(out, hWnd, *this, bBom)) {
			bError = true;
		}
	}
	catch (...) {
		bError = true;
	}
	if (bError) {
		out.Close();
		_tunlink(filename);	// 関数の実行に失敗したとき、一時ファイルの削除は関数内で行う。
		WarningMessage( NULL, LS(STR_DIFF_FAILED_TEMP) );
	}

	return true;
}



/*!	外部ファイルを指定でのファイルを表示
*/
bool EditView::MakeDiffTmpFile2(
	TCHAR* tmpName,
	const TCHAR* orgName,
	EncodingType code,
	EncodingType saveCode
	)
{
	//一時
	TCHAR* pszTmpName = _ttempnam(NULL, SAKURA_DIFF_TEMP_PREFIX);
	if (!pszTmpName) {
		WarningMessage( NULL, LS(STR_DIFF_FAILED) );
		return false;
	}

	_tcscpy(tmpName, pszTmpName);
	free( pszTmpName );

	bool bBom = false;
	const TypeConfigMini* typeMini;
	DocTypeManager().GetTypeConfigMini(DocTypeManager().GetDocumentTypeOfPath( orgName ), &typeMini);
	FileLoad fl;
	TextOutputStream out(tmpName, saveCode, true, false);
	if (!out) {
		WarningMessage(NULL, LS(STR_DIFF_FAILED_TEMP));
		return false;
	}
	try {
		bool bBigFile;
#ifdef _WIN64
		bBigFile = true;
#else
		bBigFile = false;
#endif
		fl.FileOpen(
			typeMini->encoding,
			orgName,
			bBigFile,
			code,
			GetDllShareData().common.file.GetAutoMIMEdecode(),
			&bBom
		);
		NativeW line;
		Eol eol;
		while (fl.ReadLine(&line, &eol) != CodeConvertResult::Failure) {
			const wchar_t*	pLineData;
			size_t			nLineLen;
			pLineData= line.GetStringPtr(&nLineLen);
			if (nLineLen == 0 || !pLineData) {
				break;
			}
			if (bBom) {
				NativeW line2(L"\ufeff");
				line2.AppendString(pLineData, nLineLen);
				out.WriteString(line2.GetStringPtr(), line2.GetStringLength());
				bBom = false;
			}else {
				out.WriteString(pLineData,nLineLen);
			}
		}
		if (bBom) {
			out.WriteString(L"\ufeff", 1);
		}
	}
	catch (...) {
		out.Close();
		_tunlink( tmpName );	// 関数の実行に失敗したとき、一時ファイルの削除は関数内で行う。
		WarningMessage(NULL, LS(STR_DIFF_FAILED_TEMP));
		return false;
	}

	return true;
}


