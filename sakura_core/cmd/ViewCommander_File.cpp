#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "_main/ControlTray.h"
#include "uiparts/WaitCursor.h"
#include "dlg/DlgProperty.h"
#include "dlg/DlgCancel.h"
#include "dlg/DlgProfileMgr.h"
#include "doc/DocReader.h"	//  Command_Property_File for _DEBUG
#include "print/PrintPreview.h"
#include "io/BinaryStream.h"
#include "io/FileLoad.h"
#include "WriteManager.h"
#include "EditApp.h"
#include "recent/MRUFile.h"
#include "util/window.h"
#include "charset/CodeFactory.h"
#include "plugin/Plugin.h"
#include "plugin/JackManager.h"
#include "env/SakuraEnvironment.h"
#include "debug/RunningTimer.h"
#include "sakura_rc.h"

// ViewCommanderクラスのコマンド(ファイル操作系)関数群

// 新規作成
void ViewCommander::Command_FileNew(void)
{
	// 新たな編集ウィンドウを起動
	LoadInfo loadInfo;
	loadInfo.filePath = _T("");
	loadInfo.eCharCode = CODE_NONE;
	loadInfo.bViewMode = false;
	std::tstring curDir = SakuraEnvironment::GetDlgInitialDir();
	ControlTray::OpenNewEditor(
		G_AppInstance(),
		view.GetHwnd(),
		loadInfo,
		NULL,
		false,
		curDir.c_str(),
		false
	);
	return;
}


// 新規作成（新しいウィンドウで開く）
void ViewCommander::Command_FileNew_NewWindow(void)
{
	// 新たな編集ウィンドウを起動
	LoadInfo loadInfo;
	loadInfo.filePath = _T("");
	loadInfo.eCharCode = CODE_DEFAULT;
	loadInfo.bViewMode = false;
	std::tstring curDir = SakuraEnvironment::GetDlgInitialDir();
	ControlTray::OpenNewEditor(
		G_AppInstance(),
		view.GetHwnd(),
		loadInfo,
		NULL,
		false,
		curDir.c_str(),
		true
	);
	return;
}


/*! @brief ファイルを開く */
void ViewCommander::Command_FileOpen(
	const wchar_t* filename,
	EncodingType nCharCode,
	bool bViewMode,
	const wchar_t* defaultName
	)
{
	if (!IsValidCodeType(nCharCode) && nCharCode != CODE_AUTODETECT) {
		nCharCode = CODE_AUTODETECT;
	}
	// ロード情報
	LoadInfo loadInfo(filename ? to_tchar(filename) : _T(""), nCharCode, bViewMode);
	std::vector<std::tstring> files;
	std::tstring defName = (defaultName ? to_tchar(defaultName) : _T(""));

	// 必要であれば「ファイルを開く」ダイアログ
	if (!loadInfo.filePath.IsValidPath()) {
		if (!defName.empty()) {
			TCHAR szPath[_MAX_PATH];
			TCHAR szDir[_MAX_DIR];
			TCHAR szName[_MAX_FNAME];
			TCHAR szExt  [_MAX_EXT];
			my_splitpath_t(defName.c_str(), szPath, szDir, szName, szExt);
			auto_strcat(szPath, szDir);
			if (auto_stricmp(defName.c_str(), szPath) == 0) {
				// defNameはフォルダ名だった
			}else {
				FilePath path = defName.c_str();
				if (auto_stricmp(path.GetDirPath().c_str(), szPath) == 0) {
					// フォルダ名までは実在している
					loadInfo.filePath = defName.c_str();
				}
			}
		}
		bool bDlgResult = GetDocument().docFileOperation.OpenFileDialog(
			EditWnd::getInstance().GetHwnd(),	// [in]  オーナーウィンドウ
			defName.length() == 0 ? NULL : defName.c_str(),	// [in]  フォルダ
			&loadInfo,							// [out] ロード情報受け取り
			files								// [out] ファイル名
		);
		if (!bDlgResult) return;

		loadInfo.filePath = files[0].c_str();
		// 他のファイルは新規ウィンドウ
		size_t nSize = files.size();
		for (size_t i=1; i<nSize; ++i) {
			LoadInfo filesLoadInfo = loadInfo;
			filesLoadInfo.filePath = files[i].c_str();
			ControlTray::OpenNewEditor(
				G_AppInstance(),
				EditWnd::getInstance().GetHwnd(),
				filesLoadInfo,
				NULL,
				true
			);
		}
	}

	// 開く
	GetDocument().docFileOperation.FileLoad(&loadInfo);
}


/*! 上書き保存

	F_FILESAVEALLとの組み合わせのみで使われるコマンド．
	@param warnbeep [in] true: 保存不要 or 保存禁止のときに警告を出す
	@param askname	[in] true: ファイル名未設定の時に入力を促す
*/
bool ViewCommander::Command_FileSave(bool warnbeep, bool askname)
{
	auto& doc = GetDocument();

	// ファイル名が指定されていない場合は「名前を付けて保存」のフローへ遷移
	if (!GetDocument().docFile.GetFilePathClass().IsValidPath()) {
		if (!askname) {
			return false;	// 保存しない
		}
		return doc.docFileOperation.FileSaveAs();
	}

	// セーブ情報
	SaveInfo saveInfo;
	doc.GetSaveInfo(&saveInfo);
	saveInfo.eol = EolType::None; // 改行コード無変換
	saveInfo.bOverwriteMode = true; // 上書き要求

	// 上書き処理
	auto& soundSet = EditApp::getInstance().soundSet;
	if (!warnbeep) soundSet.MuteOn();
	bool bRet = doc.docFileOperation.DoSaveFlow(&saveInfo);
	if (!warnbeep) soundSet.MuteOff();

	return bRet;
}


// 名前を付けて保存ダイアログ
bool ViewCommander::Command_FileSaveAs_Dialog(
	const wchar_t* fileNameDef,
	EncodingType eCodeType,
	EolType eEolType
	)
{
	return GetDocument().docFileOperation.FileSaveAs(fileNameDef, eCodeType, eEolType, true);
}


/* 名前を付けて保存
	filenameで保存。NULLは厳禁。
*/
bool ViewCommander::Command_FileSaveAs(
	const wchar_t* filename,
	EolType eEolType
	)
{
	return GetDocument().docFileOperation.FileSaveAs(filename, CODE_NONE, eEolType, false);
}


/*!	全て上書き保存

	編集中の全てのウィンドウで上書き保存を行う．
	ただし，上書き保存の指示を出すのみで実行結果の確認は行わない．

	上書き禁止及びファイル名未設定のウィンドウでは何も行わない．
*/
bool ViewCommander::Command_FileSaveAll(void)
{
	AppNodeGroupHandle(0).SendMessageToAllEditors(
		WM_COMMAND,
		MAKELONG(F_FILESAVE_QUIET, 0),
		0,
		NULL
	);
	return true;
}


// 閉じて(無題)
void ViewCommander::Command_FileClose(void)
{
	GetDocument().docFileOperation.FileClose();
}


/*! @brief 閉じて開く */
void ViewCommander::Command_FileClose_Open(
	LPCWSTR filename,
	EncodingType nCharCode,
	bool bViewMode
	)
{
	GetDocument().docFileOperation.FileCloseOpen(LoadInfo(to_tchar(filename), nCharCode, bViewMode));

	// プラグイン：DocumentOpenイベント実行
	Plug::Array plugs;
	WSHIfObj::List params;
	JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		(*it)->Invoke(GetEditWindow().GetActiveView(), params);
	}
}


// ファイルの再オープン
void ViewCommander::Command_File_Reopen(
	EncodingType	nCharCode,	// [in] 開き直す際の文字コード
	bool			bNoConfirm	// [in] ファイルが更新された場合に確認を行わ「ない」かどうか。true:確認しない false:確認する
	)
{
	auto& doc = GetDocument();
	if (!bNoConfirm && fexist(doc.docFile.GetFilePath()) && doc.docEditor.IsModified()) {
		int nDlgResult = MYMESSAGEBOX(
			view.GetHwnd(),
			MB_OKCANCEL | MB_ICONQUESTION | MB_TOPMOST,
			GSTR_APPNAME,
			LS(STR_ERR_CEDITVIEW_CMD29),
			doc.docFile.GetFilePath()
		);
		if (nDlgResult == IDOK) {
			// 継続。下へ進む
		}else {
			return; // 中断
		}
	}

	// 同一ファイルの再オープン
	doc.docFileOperation.ReloadCurrentFile(nCharCode);
}


// 印刷
void ViewCommander::Command_Print(void)
{
	Command_Print_Preview();

	// 印刷実行
	GetEditWindow().pPrintPreview->OnPrint();
}


// 印刷Preview
void ViewCommander::Command_Print_Preview(void)
{
	// 印刷Previewモードのオン/オフ
	GetEditWindow().PrintPreviewModeONOFF();
	return;
}


// 印刷のページレイアウトの設定
void ViewCommander::Command_Print_PageSetUp(void)
{
	// 印刷ページ設定
	GetEditWindow().OnPrintPageSetting();
	return;
}


// C/C++ヘッダファイルまたはソースファイル オープン機能
bool ViewCommander::Command_Open_HfromtoC(bool bCheckOnly)
{
	if (Command_Open_HHPP(bCheckOnly, false))	return true;
	if (Command_Open_CCPP(bCheckOnly, false))	return true;
	ErrorBeep();
	return false;
}


// C/C++ヘッダファイル オープン機能
//BOOL ViewCommander::Command_OPENINCLUDEFILE(bool bCheckOnly)
bool ViewCommander::Command_Open_HHPP(bool bCheckOnly, bool bBeepWhenMiss)
{
	static const TCHAR* source_ext[] = { _T("c"), _T("cpp"), _T("cxx"), _T("cc"), _T("cp"), _T("c++") };
	static const TCHAR* header_ext[] = { _T("h"), _T("hpp"), _T("hxx"), _T("hh"), _T("hp"), _T("h++") };
	return view.OPEN_ExtFromtoExt(
		bCheckOnly, bBeepWhenMiss, source_ext, header_ext,
		_countof(source_ext), _countof(header_ext),
		LS(STR_ERR_CEDITVIEW_CMD08));
}


// C/C++ソースファイル オープン機能
bool ViewCommander::Command_Open_CCPP(bool bCheckOnly, bool bBeepWhenMiss)
{
	static const TCHAR* source_ext[] = { _T("c"), _T("cpp"), _T("cxx"), _T("cc"), _T("cp"), _T("c++") };
	static const TCHAR* header_ext[] = { _T("h"), _T("hpp"), _T("hxx"), _T("hh"), _T("hp"), _T("h++") };
	return view.OPEN_ExtFromtoExt(
		bCheckOnly, bBeepWhenMiss, header_ext, source_ext,
		_countof(header_ext), _countof(source_ext),
		LS(STR_ERR_CEDITVIEW_CMD09));
}


// Oracle SQL*Plusをアクティブ表示
void ViewCommander::Command_Activate_SQLPlus(void)
{
	HWND hwndSQLPLUS = ::FindWindow(_T("SqlplusWClass"), _T("Oracle SQL*Plus"));
	if (!hwndSQLPLUS) {
		ErrorMessage(view.GetHwnd(), LS(STR_SQLERR_ACTV_BUT_NOT_RUN));	// "Oracle SQL*Plusをアクティブ表示します。\n\n\nOracle SQL*Plusが起動されていません。\n"
		return;
	}
	// Oracle SQL*Plusをアクティブにする
	// アクティブにする
	ActivateFrameWindow(hwndSQLPLUS);
	return;
}


// Oracle SQL*Plusで実行
void ViewCommander::Command_PLSQL_Compile_On_SQLPlus(void)
{
	int			nRet;
	BOOL		nBool;
	TCHAR		szPath[MAX_PATH + 2];

	HWND hwndSQLPLUS = ::FindWindow(_T("SqlplusWClass"), _T("Oracle SQL*Plus"));
	if (!hwndSQLPLUS) {
		ErrorMessage(view.GetHwnd(), LS(STR_SQLERR_EXEC_BUT_NOT_RUN));	// "Oracle SQL*Plusで実行します。\n\n\nOracle SQL*Plusが起動されていません。\n"
		return;
	}
	// テキストが変更されている場合
	if (GetDocument().docEditor.IsModified()) {
		nRet = ::MYMESSAGEBOX(
			view.GetHwnd(),
			MB_YESNOCANCEL | MB_ICONEXCLAMATION,
			GSTR_APPNAME,
			LS(STR_ERR_CEDITVIEW_CMD18),
			GetDocument().docFile.GetFilePathClass().IsValidPath() ? GetDocument().docFile.GetFilePath() : LS(STR_NO_TITLE1)
		);
		switch (nRet) {
		case IDYES:
			if (GetDocument().docFile.GetFilePathClass().IsValidPath()) {
				nBool = Command_FileSave();
			}else {
				nBool = Command_FileSaveAs_Dialog(NULL, CODE_NONE, EolType::None);
			}
			if (!nBool) {
				return;
			}
			break;
		case IDNO:
			return;
		case IDCANCEL:
		default:
			return;
		}
	}
	if (GetDocument().docFile.GetFilePathClass().IsValidPath()) {
		// ファイルパスに空白が含まれている場合はダブルクォーテーションで囲む
		if (_tcschr(GetDocument().docFile.GetFilePath(), TCODE::SPACE) != NULL) {
			auto_sprintf( szPath, _T("@\"%ts\"\r\n"), GetDocument().docFile.GetFilePath() );
		}else {
			auto_sprintf( szPath, _T("@%ts\r\n"), GetDocument().docFile.GetFilePath() );
		}
		// クリップボードにデータを設定
		view.MySetClipboardData(szPath, _tcslen(szPath), false);

		// Oracle SQL*Plusをアクティブにする
		// アクティブにする
		ActivateFrameWindow(hwndSQLPLUS);

		// Oracle SQL*Plusにペーストのコマンドを送る
		DWORD_PTR dwResult;
		LRESULT bResult = ::SendMessageTimeout(
			hwndSQLPLUS,
			WM_COMMAND,
			MAKELONG(201, 0),
			0,
			SMTO_ABORTIFHUNG | SMTO_NORMAL,
			3000,
			&dwResult
		);
		if (!bResult) {
			TopErrorMessage(view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD20));
		}
	}else {
		ErrorBeep();
		ErrorMessage(view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD21));
		return;
	}
	return;
}


// ブラウズ
void ViewCommander::Command_Browse(void)
{
	if (!GetDocument().docFile.GetFilePathClass().IsValidPath()) {
		ErrorBeep();
		return;
	}
	SHELLEXECUTEINFO info; 
	info.cbSize = sizeof(info);
	info.fMask = 0;
	info.hwnd = NULL;
	info.lpVerb = NULL;
	info.lpFile = GetDocument().docFile.GetFilePath();
	info.lpParameters = NULL;
	info.lpDirectory = NULL;
	info.nShow = SW_SHOWNORMAL;
	info.hInstApp = 0;
	info.lpIDList = NULL;
	info.lpClass = NULL;
	info.hkeyClass = 0; 
	info.dwHotKey = 0;
	info.hIcon =0;

	::ShellExecuteEx(&info);

	return;
}


// ビューモード
void ViewCommander::Command_ViewMode(void)
{
	// ビューモードを反転
	AppMode::getInstance().SetViewMode(!AppMode::getInstance().IsViewMode());

	// 排他制御の切り替え
	// ※ビューモード ON 時は排他制御 OFF、ビューモード OFF 時は排他制御 ON の仕様（>>data:5262）を即時反映する
	auto& doc = GetDocument();
	doc.docFileOperation.DoFileUnlock();	// ファイルの排他ロック解除
	doc.docLocker.CheckWritable(!AppMode::getInstance().IsViewMode());	// ファイル書込可能のチェック
	if (doc.docLocker.IsDocWritable()) {
		doc.docFileOperation.DoFileLock();	// ファイルの排他ロック
	}

	// 親ウィンドウのタイトルを更新
	GetEditWindow().UpdateCaption();
}


// ファイルのプロパティ
void ViewCommander::Command_Property_File(void)
{
#ifdef _DEBUG
	{
		// 全行データを返すテスト
		size_t nDataAllLen;
		RunningTimer runningTimer("ViewCommander::Command_Property_File 全行データを返すテスト");
		runningTimer.Reset();
		wchar_t* pDataAll = DocReader(GetDocument().docLineMgr).GetAllData(&nDataAllLen);
//		MYTRACE(_T("全データ取得             (%dバイト) 所要時間(ミリ秒) = %d\n"), nDataAllLen, runningTimer.Read());
		free(pDataAll);
		pDataAll = NULL;
//		MYTRACE(_T("全データ取得のメモリ開放 (%dバイト) 所要時間(ミリ秒) = %d\n"), nDataAllLen, runningTimer.Read());
	}
#endif


	DlgProperty	dlgProperty;
//	cDlgProperty.Create(G_AppInstance(), view.GetHwnd(), GetDocument());
	dlgProperty.DoModal(G_AppInstance(), view.GetHwnd(), (LPARAM)&GetDocument());
	return;
}


void ViewCommander::Command_ProfileMgr( void )
{
	DlgProfileMgr profMgr;
	if (profMgr.DoModal( G_AppInstance(), view.GetHwnd(), 0 )) {
		TCHAR szOpt[MAX_PATH+10];
		auto_sprintf( szOpt, _T("-PROF=\"%ts\""), profMgr.strProfileName.c_str() );
		LoadInfo loadInfo;
		loadInfo.filePath = _T("");
		loadInfo.eCharCode = CODE_DEFAULT;
		loadInfo.bViewMode = false;
		ControlTray::OpenNewEditor(
			G_AppInstance(),
			view.GetHwnd(),
			loadInfo,
			szOpt,
			false,
			NULL,
			false
		);
	}
}

// 編集の全終了
void ViewCommander::Command_ExitAllEditors(void)
{
	ControlTray::CloseAllEditor(true, GetMainWindow(), true, 0);
	return;
}


// サクラエディタの全終了
void ViewCommander::Command_ExitAll(void)
{
	ControlTray::TerminateApplication(GetMainWindow());
	return;
}


/*!	@brief 編集中の内容を別名保存

	主に編集中の一時ファイル出力などの目的に使用する．
	現在開いているファイル(szFilePath)には影響しない．

	@retval	true 正常終了
	@retval	false ファイル作成に失敗
*/
bool ViewCommander::Command_PutFile(
	LPCWSTR			filename,	// [in] filename 出力ファイル名
	EncodingType	nCharCode,	// [in] nCharCode 文字コード指定
								//  @li CODE_xxxxxxxxxx:各種文字コード
								//  @li CODE_AUTODETECT:現在の文字コードを維持
	int				nFlgOpt		// [in] nFlgOpt 動作オプション
								//  @li 0x01:選択範囲を出力 (非選択状態でも空ファイルを出力する)
)
{
	bool bResult = true;
	EncodingType nSaveCharCode = nCharCode;
	if (filename[0] == L'\0') {
		return false;
	}

	if (nSaveCharCode == CODE_AUTODETECT) {
		nSaveCharCode = GetDocument().GetDocumentEncoding();
	}

	// EditDoc::FileWrite()にならって砂時計カーソル
	WaitCursor waitCursor(view.GetHwnd());

	std::unique_ptr<CodeBase> pcSaveCode(CodeFactory::CreateCodeBase(nSaveCharCode, 0));

	bool bBom = false;
	if (CodeTypeName(nSaveCharCode).UseBom()) {
		bBom = GetDocument().GetDocumentBomExist();
	}

	if (nFlgOpt & 0x01) {	// 選択範囲を出力
		try {
			BinaryOutputStream out(to_tchar(filename), true);

			// 選択範囲の取得 -> mem
			NativeW mem;
			view.GetSelectedDataSimple(mem);

			// BOM追加
			NativeW mem2;
			const NativeW* pConvBuffer;
			if (bBom) {
				NativeW memBom;
				std::unique_ptr<CodeBase> pcUtf16(CodeFactory::CreateCodeBase(CODE_UNICODE, 0));
				pcUtf16->GetBom(memBom._GetMemory());
				mem2.AppendNativeData(memBom);
				mem2.AppendNativeData(mem);
				mem.Clear();
				pConvBuffer = &mem2;
			}else {
				pConvBuffer = &mem;
			}

			// 書き込み時のコード変換 -> dst
			Memory dst;
			pcSaveCode->UnicodeToCode(*pConvBuffer, &dst);

			// 書込
			if (0 < dst.GetRawLength())
				out.Write(dst.GetRawPtr(), dst.GetRawLength());
		}catch (Error_FileOpen) {
			WarningMessage(
				NULL,
				LS(STR_SAVEAGENT_OTHER_APP),
				to_tchar(filename)
			);
			bResult = false;
		}catch (Error_FileWrite) {
			WarningMessage(
				NULL,
				LS(STR_ERR_DLGEDITVWCMDNW11)
			);
			bResult = false;
		}
	}else {	// ファイル全体を出力
		HWND	hwndProgress;
		auto&	editWnd = GetEditWindow();
		hwndProgress = editWnd.statusBar.GetProgressHwnd();
		if (hwndProgress) {
			::ShowWindow(hwndProgress, SW_SHOW);
		}

		// 一時ファイル出力
		CodeConvertResult eRet = WriteManager().WriteFile_From_CDocLineMgr(
			GetDocument().docLineMgr,
			SaveInfo(
				to_tchar(filename),
				nSaveCharCode,
				EolType::None,
				bBom
			)
		);
		bResult = (eRet != CodeConvertResult::Failure);
		if (hwndProgress) {
			::ShowWindow(hwndProgress, SW_HIDE);
		}
	}
	return bResult;
}


/*!	@brief カーソル位置にファイルを挿入

	現在のカーソル位置に指定のファイルを読み込む．

	@param[in] filename 入力ファイル名
	@param[in] nCharCode 文字コード指定
		@li	CODE_xxxxxxxxxx:各種文字コード
		@li	CODE_AUTODETECT:前回文字コードもしくは自動判別の結果による
	@param[in] nFlgOpt 動作オプション（現在は未定義．0を指定のこと）

	@retval	true 正常終了
	@retval	false ファイルオープンに失敗
*/
bool ViewCommander::Command_InsFile(
	LPCWSTR filename,
	EncodingType nCharCode,
	int nFlgOpt
	)
{
	FileLoad	fl;
	Eol			eol;
	int			nLineNum = 0;

	DlgCancel*	pDlgCancel = nullptr;
	HWND		hwndCancel = NULL;
	HWND		hwndProgress = NULL;
	int			nOldPercent = -1;
	bool		bResult = true;

	if (filename[0] == L'\0') {
		return false;
	}

	// EditDoc::FileLoad()にならって砂時計カーソル
	WaitCursor waitCursor(view.GetHwnd());

	// 範囲選択中なら挿入後も選択状態にするため
	bool bBeforeTextSelected = view.GetSelectionInfo().IsTextSelected();
	Point ptFrom;
	if (bBeforeTextSelected) {
		ptFrom = view.GetSelectionInfo().select.GetFrom();
	}


	EncodingType	nSaveCharCode = nCharCode;
	if (nSaveCharCode == CODE_AUTODETECT) {
		EditInfo    fi;
		const MruFile  mru;
		if (mru.GetEditInfo(to_tchar(filename), &fi)) {
				nSaveCharCode = fi.nCharCode;
		}else {
			nSaveCharCode = GetDocument().GetDocumentEncoding();
		}
	}

	// ここまできて文字コードが決定しないならどこかおかしい
	if (!IsValidCodeType(nSaveCharCode)) nSaveCharCode = CODE_SJIS;

	try {
		bool bBigFile;
#ifdef _WIN64
		bBigFile = true;
#else
		bBigFile = false;
#endif
		// ファイルを開く
		fl.FileOpen(view.pTypeData->encoding, to_tchar(filename), bBigFile, nSaveCharCode, 0 );

		// ファイルサイズが65KBを越えたら進捗ダイアログ表示
		if (0x10000 < fl.GetFileSize()) {
			pDlgCancel = new DlgCancel;
			if ((hwndCancel = pDlgCancel->DoModeless(::GetModuleHandle(NULL), NULL, IDD_OPERATIONRUNNING))) {
				hwndProgress = ::GetDlgItem(hwndCancel, IDC_PROGRESS);
				Progress_SetRange(hwndProgress, 0, 101);
				Progress_SetPos(hwndProgress, 0);
			}
		}

		// ReadLineはファイルから 文字コード変換された1行を読み出します
		// エラー時はthrow Error_FileRead を投げます
		NativeW buf;
		while (CodeConvertResult::Failure != fl.ReadLine(&buf, &eol)) {

			const wchar_t* pLine = buf.GetStringPtr();
			size_t nLineLen = buf.GetStringLength();

			++nLineNum;
			Command_InsText(false, pLine, nLineLen, true);

			// 進捗ダイアログ有無
			if (!pDlgCancel) {
				continue;
			}
			// 処理中のユーザー操作を可能にする
			if (!::BlockingHook(pDlgCancel->GetHwnd())) {
				break;
			}
			// 中断ボタン押下チェック
			if (pDlgCancel->IsCanceled()) {
				break;
			}
			if ((nLineNum & 0xFF) == 0) {
				if (nOldPercent != fl.GetPercent()) {
					Progress_SetPos(hwndProgress, fl.GetPercent() + 1);
					Progress_SetPos(hwndProgress, fl.GetPercent());
					nOldPercent = fl.GetPercent();
				}
				view.Redraw();
			}
		}
		// ファイルを明示的に閉じるが、ここで閉じないときはデストラクタで閉じている
		fl.FileClose();
	}catch (Error_FileOpen) {
		WarningMessage(NULL, LS(STR_GREP_ERR_FILEOPEN), to_tchar(filename));
		bResult = false;
	}catch (Error_FileRead) {
		WarningMessage(NULL, LS(STR_ERR_DLGEDITVWCMDNW12));
		bResult = false;
	} // 例外処理終わり

	delete pDlgCancel;

	if (bBeforeTextSelected) {	// 挿入された部分を選択状態に
		view.GetSelectionInfo().SetSelectArea(
			Range(
				ptFrom,
				GetCaret().GetCaretLayoutPos()
				/*
				nCaretPosY, nCaretPosX
				*/
			)
		);
		view.GetSelectionInfo().DrawSelectArea();
	}
	view.Redraw();
	return bResult;
}

