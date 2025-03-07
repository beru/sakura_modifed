#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "dlg/DlgCompare.h"
#include "dlg/DlgDiff.h"
#include "charset/CodeMediator.h"
#include "charset/CodePage.h"
#include "env/ShareData.h"
#include "util/window.h"
#include "util/os.h"
#include "_main/Mutex.h"

// ViewCommanderクラスのコマンド(Diff)関数群

/*!
	@return true:正常終了 / false:エラー終了
*/
static bool Commander_COMPARE_core(
	ViewCommander& commander,
	bool& bDifferent,
	HWND hwnd,
	Point& poSrc,
	Point& poDes
	)
{
	const wchar_t*	pLineSrc;
	const wchar_t*	pLineDes;
	int nLineLenDes;
	int max_size = (int)GetDllShareData().workBuffer.GetWorkBufferCount<EDIT_CHAR>();
	const DocLineMgr& docMgr = commander.GetDocument().docLineMgr;

	bDifferent = true;
	{
		pLineDes = GetDllShareData().workBuffer.GetWorkBuffer<const EDIT_CHAR>();
		int nLineOffset = 0;
		for (;;) {
			size_t nLineLenSrc;
			pLineSrc = docMgr.GetLine(poSrc.y)->GetDocLineStrWithEOL(&nLineLenSrc);
			do {
				// workBuffer#Workの排他制御。外部コマンド出力/TraceOut/Diffが対象
				LockGuard<Mutex> guard( ShareData::GetMutexShareWork() );
				// 行(改行単位)データの要求
				nLineLenDes = ::SendMessage( hwnd, MYWM_GETLINEDATA, poDes.y, nLineOffset );
				if (nLineLenDes < 0) {
					return false;
				}
				// どっちも最終行(EOF)に到達。同一と判定
				if (!pLineSrc && nLineLenDes == 0) {
					bDifferent = false;
					return true;
				}
				// どちらかだけが、最終行に到達
				if (!pLineSrc || nLineLenDes == 0) {
					return true;
				}
				int nDstEndPos = std::min( nLineLenDes, max_size ) + nLineOffset;
				if (poDes.x < nLineOffset) {
					// 1行目行頭データ読み飛ばし
					if (nLineLenDes < poDes.x) {
						poDes.x = nLineLenDes - 1;
						return true;
					}
					nLineOffset = poDes.x;
				}else {
					// Note: サロゲート/改行の途中にカーソルがくることがある
					while (poDes.x < nDstEndPos) {
						if ((int)nLineLenSrc <= poSrc.x) {
							return true;
						}
						if (pLineSrc[poSrc.x] != pLineDes[poDes.x - nLineOffset]) {
							return true;
						}
						poSrc.x++;
						poDes.x++;
					}
				}
				nLineOffset += max_size;
			}while (max_size < nLineLenDes);

			if (poSrc.x < (int)nLineLenSrc) {
				return true;
			}
			poSrc.x = 0;
			poSrc.y++;
			poDes.x = 0;
			poDes.y++;
			nLineOffset = 0;
		}
	}
	assert_warning(0);
	return false;
}

// ファイル内容比較
void ViewCommander::Command_Compare(void)
{
	HWND		hwndCompareWnd = NULL;
	TCHAR		szPath[_MAX_PATH + 1];
	DlgCompare	dlgCompare;
	HWND		hwndMsgBox;
	auto& commonSetting = GetDllShareData().common;
	auto& csCompare = commonSetting.compare;
	// 比較後、左右に並べて表示
	dlgCompare.bCompareAndTileHorz = csCompare.bCompareAndTileHorz;
	INT_PTR bDlgCompareResult = dlgCompare.DoModal(
		G_AppInstance(),
		view.GetHwnd(),
		(LPARAM)&GetDocument(),
		GetDocument().docFile.GetFilePath(),
		szPath,
		&hwndCompareWnd
	);
	if (!bDlgCompareResult) {
		return;
	}
	// 比較後、左右に並べて表示
	csCompare.bCompareAndTileHorz = dlgCompare.bCompareAndTileHorz;

	// タブウィンドウ時は禁止
	if (commonSetting.tabBar.bDispTabWnd
		&& !commonSetting.tabBar.bDispTabWndMultiWin
	) {
		hwndMsgBox = view.GetHwnd();
		csCompare.bCompareAndTileHorz = false;
	}else {
		hwndMsgBox = hwndCompareWnd;
	}

	/*
	  カーソル位置変換
	  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	  →
	  物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	Point poSrc = GetDocument().layoutMgr.LayoutToLogic(GetCaret().GetCaretLayoutPos());
	// カーソル位置取得 -> poDes
	Point poDes;
	{
		::SendMessage(hwndCompareWnd, MYWM_GETCARETPOS, 0, 0);
		Point* ppoCaretDes = &(GetDllShareData().workBuffer.logicPoint);
		poDes.x = ppoCaretDes->x;
		poDes.y = ppoCaretDes->y;
	}
	bool bDifferent = false;
	// 本処理
	Commander_COMPARE_core(*this, bDifferent, hwndCompareWnd, poSrc, poDes);

	// 比較後、左右に並べて表示
// チェックボックスをボタン化すれば以下の行(To Here まで)は不要のはずだが
// うまくいかなかったので元に戻してある…
	if (GetDllShareData().common.compare.bCompareAndTileHorz) {
		HWND hWndArr[2];
		hWndArr[0] = GetMainWindow();
		hWndArr[1] = hwndCompareWnd;
		for (size_t i=0; i<2; ++i) {
			if (::IsZoomed( hWndArr[i] )) {
				::ShowWindow( hWndArr[i], SW_RESTORE );
			}
		}
		// デスクトップサイズを得る
		RECT rcDesktop;
		// マルチモニタ対応
		::GetMonitorWorkRect( hWndArr[0], &rcDesktop );
		int width = (rcDesktop.right - rcDesktop.left) / 2;
		for (int i=1; i>=0; --i) {
			::SetWindowPos(
				hWndArr[i], 0,
				width * i + rcDesktop.left, rcDesktop.top, // タスクバーが左にある場合を考慮
				width, rcDesktop.bottom - rcDesktop.top,
				SWP_NOOWNERZORDER | SWP_NOZORDER
			);
		}
//		::TileWindows( NULL, MDITILE_VERTICAL, NULL, 2, phwndArr );
	}

	if (!bDifferent) {
		TopInfoMessage(hwndMsgBox, LS(STR_ERR_CEDITVIEW_CMD22));
	}else {
//		TopInfoMessage(hwndMsgBox, LS(STR_ERR_CEDITVIEW_CMD23));
		/* カーソルを移動させる
			比較相手は、別プロセスなのでメッセージを飛ばす。
		*/
		GetDllShareData().workBuffer.logicPoint = poDes;
		::SendMessage(hwndCompareWnd, MYWM_SETCARETPOS, 0, 0);

		// カーソルを移動させる
		GetDllShareData().workBuffer.logicPoint = poSrc;
		::PostMessage(GetMainWindow(), MYWM_SETCARETPOS, 0, 0);
		TopWarningMessage(hwndMsgBox, LS(STR_ERR_CEDITVIEW_CMD23));	// 位置を変更してからメッセージ
	}

	// 開いているウィンドウをアクティブにする
	// アクティブにする
	ActivateFrameWindow(GetMainWindow());
	return;
}


static
EncodingType GetFileCharCode( LPCTSTR pszFile )
{
	const TypeConfigMini* typeMini;
	DocTypeManager().GetTypeConfigMini( DocTypeManager().GetDocumentTypeOfPath( pszFile ), &typeMini );
	return CodeMediator(typeMini->encoding).CheckKanjiCodeOfFile( pszFile );
}


static
EncodingType GetDiffCreateTempFileCode(EncodingType code)
{
	EEncodingTrait e = CodePage::GetEncodingTrait(code);
	if (e != ENCODING_TRAIT_ASCII) {
		return CODE_UTF8;
	}
	return code;
}


/*!	差分表示
	@note	HandleCommandからの呼び出し対応(ダイアログなし版)
*/
void ViewCommander::Command_Diff(const wchar_t* _szDiffFile2, int nFlgOpt)
{
	const std::tstring strDiffFile2 = to_tchar(_szDiffFile2);
	const TCHAR* szDiffFile2 = strDiffFile2.c_str();

	bool bTmpFile1 = false;
	TCHAR szTmpFile1[_MAX_PATH * 2];

	if (!IsFileExists( szDiffFile2, true )) {
		WarningMessage(view.GetHwnd(), LS(STR_ERR_DLGEDITVWDIFF1));
		return;
	}

	// 自ファイル
	// Unicodeのときは、いつもファイル出力
	EncodingType code = GetDocument().GetDocumentEncoding();
	EncodingType saveCode = GetDiffCreateTempFileCode(code);
	EncodingType code2 = GetFileCharCode(szDiffFile2);
	EncodingType saveCode2 = GetDiffCreateTempFileCode(code2);
	// コードが違うときは必ずUTF-8ファイル出力
	if (saveCode != saveCode2) {
		saveCode = CODE_UTF8;
		saveCode2 = CODE_UTF8;
	}

	if (GetDocument().docEditor.IsModified()
		|| saveCode != code
		|| !GetDocument().docFile.GetFilePathClass().IsValidPath() // Grep/アウトプットも対象にする
	) {
		if (!view.MakeDiffTmpFile(szTmpFile1, NULL, saveCode, GetDocument().GetDocumentBomExist())) {
			return;
		}
		bTmpFile1 = true;
	}else {
		_tcscpy( szTmpFile1, GetDocument().docFile.GetFilePath() );
	}

	bool bTmpFile2 = false;
	TCHAR	szTmpFile2[_MAX_PATH * 2];
	bool bTmpFileMode = code2 != saveCode2;
	if (!bTmpFileMode) {
		_tcscpy(szTmpFile2, szDiffFile2);
	}else if (view.MakeDiffTmpFile2( szTmpFile2, szDiffFile2, code2, saveCode2 )) {
		bTmpFile2 = true;
	}else {
		if (bTmpFile1) _tunlink( szTmpFile1 );
		return;
	}

	bool bUTF8io = true;
	if (saveCode == CODE_SJIS) {
		bUTF8io = false;
	}

	// 差分表示
	view.ViewDiffInfo(szTmpFile1, szTmpFile2, nFlgOpt, bUTF8io);

	// 一時ファイルを削除する
	if (bTmpFile1) _tunlink( szTmpFile1 );
	if (bTmpFile2) _tunlink( szTmpFile2 );

	return;
}


/*!	差分表示
	@note	HandleCommandからの呼び出し対応(ダイアログあり版)
*/
void ViewCommander::Command_Diff_Dialog(void)
{
	DlgDiff dlgDiff;
	bool bTmpFile1 = false, bTmpFile2 = false;

	auto& docFile = GetDocument().docFile;
	auto& docEditor = GetDocument().docEditor;
	// DIFF差分表示ダイアログを表示する
	INT_PTR nDiffDlgResult = dlgDiff.DoModal(
		G_AppInstance(),
		view.GetHwnd(),
		(LPARAM)&GetDocument(),
		docFile.GetFilePath()
	);
	if (!nDiffDlgResult) {
		return;
	}
	
	// 自ファイル
	TCHAR szTmpFile1[_MAX_PATH * 2];
	EncodingType code = GetDocument().GetDocumentEncoding();
	EncodingType saveCode = GetDiffCreateTempFileCode(code);
	EncodingType code2 = dlgDiff.nCodeTypeDst;
	if (code2 == CODE_ERROR) {
		if (dlgDiff.szFile2[0] != _T('\0')) {
			// ファイル名指定
			code2 = GetFileCharCode(dlgDiff.szFile2);
		}
	}
	EncodingType saveCode2 = GetDiffCreateTempFileCode(code2);
	// コードが違うときは必ずUTF-8ファイル出力
	if (saveCode != saveCode2) {
		saveCode = CODE_UTF8;
		saveCode2 = CODE_UTF8;
	}
	if (GetDocument().docEditor.IsModified()
		|| code != saveCode
		|| !GetDocument().docFile.GetFilePathClass().IsValidPath() // Grep/アウトプットも対象にする
	) {
		if (!view.MakeDiffTmpFile( szTmpFile1, NULL, saveCode, GetDocument().GetDocumentBomExist() )) { return; }
		bTmpFile1 = true;
	}else {
		_tcscpy( szTmpFile1, GetDocument().docFile.GetFilePath() );
	}
		
	// 相手ファイル
	// UNICODE,UNICODEBEの場合は常に一時ファイルでUTF-8にする
	TCHAR szTmpFile2[_MAX_PATH * 2];
	// ファイル名がない(=無題,Grep,アウトプット)もTmpFileModeにする
	bool bTmpFileMode = dlgDiff.bIsModifiedDst || code2 != saveCode2 || dlgDiff.szFile2[0] == _T('\0');
	if (!bTmpFileMode) {
		// 未変更でファイルありでASCII系コードの場合のみ,そのままファイルを利用する
		_tcscpy( szTmpFile2, dlgDiff.szFile2 );
	}else if (dlgDiff.hWnd_Dst) {
		// ファイル一覧から選択
		if (view.MakeDiffTmpFile( szTmpFile2, dlgDiff.hWnd_Dst, saveCode2, dlgDiff.bBomDst )) {
			bTmpFile2 = true;
		}else {
			if (bTmpFile1) _tunlink( szTmpFile1 );
			return;
		}
	}else {
		// ファイル名指定で非ASCII系だった場合
		if (view.MakeDiffTmpFile2( szTmpFile2, dlgDiff.szFile2, code2, saveCode2 )) {
			bTmpFile2 = true;
		}else {
			// Error
			if (bTmpFile1) _tunlink( szTmpFile1 );
			return;
		}
	}
	
	bool bUTF8io = true;
	if (saveCode == CODE_SJIS) {
		bUTF8io = false;
	}

	// 差分表示
	view.ViewDiffInfo(szTmpFile1, szTmpFile2, dlgDiff.nDiffFlgOpt, bUTF8io);
	
	// 一時ファイルを削除する
	if (bTmpFile1) {
		_tunlink(szTmpFile1);
	}
	if (bTmpFile2) {
		_tunlink(szTmpFile2);
	}

	return;
}


//	次の差分を探し，見つかったら移動する
void ViewCommander::Command_Diff_Next(void)
{
	bool bFound = false;
	bool bRedo = true;

	Point ptXY(0, GetCaret().GetCaretLogicPos().y);
	int nYOld_Logic = ptXY.y;
	size_t tmp_y;
	auto& selInfo = view.GetSelectionInfo();

re_do:;	
	if (DiffLineMgr(GetDocument().docLineMgr).SearchDiffMark(ptXY.y, SearchDirection::Forward, &tmp_y)) {
		ptXY.y = (int)tmp_y;
		bFound = true;
		Point ptXY_Layout = GetDocument().layoutMgr.LogicToLayout(ptXY);
		if (selInfo.bSelectingLock) {
			if (!selInfo.IsTextSelected()) {
				selInfo.BeginSelectArea();
			}
		}else {
			if (selInfo.IsTextSelected()) {
				selInfo.DisableSelectArea(true);
			}
		}

		if (selInfo.bSelectingLock) {
			selInfo.ChangeSelectAreaByCurrentCursor(ptXY_Layout);
		}
		GetCaret().MoveCursor(ptXY_Layout, true);
	}

	if (GetDllShareData().common.search.bSearchAll) {
		// 見つからなかった。かつ、最初の検索
		if (!bFound	&& bRedo) {
			ptXY.y = 0 - 1;	// 1個手前を指定
			bRedo = false;
			goto re_do;		// 先頭から再検索
		}
	}

	if (bFound) {
		if (nYOld_Logic >= ptXY.y) {
			view.SendStatusMessage(LS(STR_ERR_SRNEXT1));
		}
	}else {
		view.SendStatusMessage(LS(STR_ERR_SRNEXT2));
		AlertNotFound(view.GetHwnd(), false, LS(STR_DIFF_NEXT_NOT_FOUND));
	}

	return;
}


// 前の差分を探し，見つかったら移動する
void ViewCommander::Command_Diff_Prev(void)
{
	bool bFound = false;
	bool bRedo = true;

	Point ptXY(0, GetCaret().GetCaretLogicPos().y);
	int			nYOld_Logic = ptXY.y;
	size_t		tmp_y;
	auto& selInfo = view.GetSelectionInfo();

re_do:;
	if (DiffLineMgr(GetDocument().docLineMgr).SearchDiffMark(ptXY.y, SearchDirection::Backward, &tmp_y)) {
		ptXY.y = (int)tmp_y;
		bFound = true;
		Point ptXY_Layout = GetDocument().layoutMgr.LogicToLayout(ptXY);
		if (selInfo.bSelectingLock) {
			if (!selInfo.IsTextSelected()) {
				selInfo.BeginSelectArea();
			}
		}else {
			if (selInfo.IsTextSelected()) {
				selInfo.DisableSelectArea(true);
			}
		}

		if (selInfo.bSelectingLock) {
			selInfo.ChangeSelectAreaByCurrentCursor(ptXY_Layout);
		}
		GetCaret().MoveCursor(ptXY_Layout, true);
	}

	if (GetDllShareData().common.search.bSearchAll) {
		// 見つからなかった、かつ、最初の検索
		if (!bFound	&& bRedo) {
			ptXY.y = (int)GetDocument().docLineMgr.GetLineCount();	// 1個手前を指定
			bRedo = false;
			goto re_do;	// 末尾から再検索
		}
	}

	if (bFound) {
		if (nYOld_Logic <= ptXY.y) view.SendStatusMessage(LS(STR_ERR_SRPREV1));
	}else {
		view.SendStatusMessage(LS(STR_ERR_SRPREV2));
		AlertNotFound(view.GetHwnd(), false, LS(STR_DIFF_PREV_NOT_FOUND));
	}

	return;
}


/*!	差分表示の全解除 */
void ViewCommander::Command_Diff_Reset(void)
{
	DiffLineMgr(GetDocument().docLineMgr).ResetAllDiffMark();

	// 分割したビューも更新
	GetEditWindow().Views_Redraw();
	return;
}

