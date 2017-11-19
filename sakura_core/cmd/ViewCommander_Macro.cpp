#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "macro/SMacroMgr.h"
#include "dlg/DlgExec.h"
#include "dlg/DlgOpenFile.h"
#include "EditApp.h"
#include "recent/RecentCurDir.h"
#include "util/module.h"
#include "env/ShareData.h"
#include "env/SakuraEnvironment.h"

// ViewCommanderクラスのコマンド(マクロ系)関数群

// キーマクロの記録開始／終了
void ViewCommander::Command_RecKeyMacro(void)
{
	auto& flags = GetDllShareData().flags;
	if (flags.bRecordingKeyMacro) {									// キーボードマクロの記録中
		flags.bRecordingKeyMacro = false;
		flags.hwndRecordingKeyMacro = NULL;							// キーボードマクロを記録中のウィンドウ
		// キーマクロをマクロ用フォルダに「RecKey.mac」という名で保存
		TCHAR szInitDir[MAX_PATH];
		int nRet;
		// 記録用キーマクロのフルパスをShareData経由で取得
		nRet = ShareData::getInstance().GetMacroFilename(-1, szInitDir, MAX_PATH);
		auto& csMacro = GetDllShareData().common.macro;
		if (nRet <= 0) {
			ErrorMessage(view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD24), nRet);
			return;
		}else {
			_tcscpy(csMacro.szKeyMacroFileName, szInitDir);
		}
		int nSaveResult = pSMacroMgr->Save(
			STAND_KEYMACRO,
			G_AppInstance(),
			csMacro.szKeyMacroFileName
		);
		if (!nSaveResult) {
			ErrorMessage(	view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD25), csMacro.szKeyMacroFileName);
		}
	}else {
		flags.bRecordingKeyMacro = true;
		flags.hwndRecordingKeyMacro = GetMainWindow();	// キーボードマクロを記録中のウィンドウ
		// キーマクロのバッファをクリアする
		pSMacroMgr->Clear(STAND_KEYMACRO);
	}
	// 親ウィンドウのタイトルを更新
	GetEditWindow().UpdateCaption();

	// キャレットの行桁位置を表示する
	GetCaret().ShowCaretPosInfo();
}


// キーマクロの保存
void ViewCommander::Command_SaveKeyMacro(void)
{
	auto& flags = GetDllShareData().flags;
	flags.bRecordingKeyMacro = false;
	flags.hwndRecordingKeyMacro = NULL;	// キーボードマクロを記録中のウィンドウ

	if (!pSMacroMgr->IsSaveOk()) {
		// 保存不可
		ErrorMessage(view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD26));
	}

	TCHAR szPath[_MAX_PATH + 1];
	TCHAR szInitDir[_MAX_PATH + 1];
	szPath[0] = 0;
	auto& macroFolder = GetDllShareData().common.macro.szMACROFOLDER;
	if (_IS_REL_PATH(macroFolder)) {
		GetInidirOrExedir(szInitDir, macroFolder);
	}else {
		_tcscpy(szInitDir, macroFolder);	// マクロ用フォルダ
	}
	// ファイルオープンダイアログの初期化
	DlgOpenFile	dlgOpenFile;
	dlgOpenFile.Create(
		G_AppInstance(),
		view.GetHwnd(),
		_T("*.mac"),
		szInitDir
	);
	if (!dlgOpenFile.DoModal_GetSaveFileName(szPath)) {
		return;
	}
	// ファイルのフルパスを、フォルダとファイル名に分割
	// [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt]
//	::SplitPath_FolderAndFile(szPath, macroFolder, NULL);
//	wcscat(macroFolder, L"\\");

	// キーボードマクロの保存
	if (!pSMacroMgr->Save(STAND_KEYMACRO, G_AppInstance(), szPath)) {
		ErrorMessage(view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD27), szPath);
	}
}


/*! キーマクロの読み込み */
void ViewCommander::Command_LoadKeyMacro(void)
{
	auto& flags = GetDllShareData().flags;
	flags.bRecordingKeyMacro = false;
	flags.hwndRecordingKeyMacro = NULL;	// キーボードマクロを記録中のウィンドウ

	TCHAR szPath[_MAX_PATH + 1];
	TCHAR szInitDir[_MAX_PATH + 1];
	const TCHAR* pszFolder = GetDllShareData().common.macro.szMACROFOLDER;
	szPath[0] = 0;
	
	if (_IS_REL_PATH(pszFolder)) {
		GetInidirOrExedir(szInitDir, pszFolder);
	}else {
		_tcscpy_s(szInitDir, pszFolder);	// マクロ用フォルダ
	}
	// ファイルオープンダイアログの初期化
	DlgOpenFile dlgOpenFile;
	dlgOpenFile.Create(
		G_AppInstance(),
		view.GetHwnd(),
		_T("*.*"),
		szInitDir
	);
	if (!dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
		return;
	}

	// キーボードマクロの読み込み
	// 読み込みといいつつも、ファイル名をコピーするだけ。実行直前に読み込む
	_tcscpy(GetDllShareData().common.macro.szKeyMacroFileName, szPath);
//	GetDllShareData().CKeyMacroMgr.LoadKeyMacro(G_AppInstance(), view.GetHwnd(), szPath);
}

// キーマクロの実行
void ViewCommander::Command_ExecKeyMacro(void)
{
	auto& flags = GetDllShareData().flags;
	// 記録中は終了してから実行
	if (flags.bRecordingKeyMacro) {
		Command_RecKeyMacro();
	}
	flags.bRecordingKeyMacro = false;
	flags.hwndRecordingKeyMacro = NULL;	// キーボードマクロを記録中のウィンドウ

	// キーボードマクロの実行
	auto& csMacro = GetDllShareData().common.macro;
	if (csMacro.szKeyMacroFileName[0]) {
		// ファイルが保存されていたら
		bool bLoadResult = pSMacroMgr->Load(
			view,
			STAND_KEYMACRO,
			G_AppInstance(),
			csMacro.szKeyMacroFileName,
			NULL
		);
		if (!bLoadResult) {
			ErrorMessage(view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD28), csMacro.szKeyMacroFileName);
		}else {
			pSMacroMgr->Exec(STAND_KEYMACRO, G_AppInstance(), view, 0);
		}
	}
}


/*! 名前を指定してマクロ実行
	@param pszPath	マクロのファイルパス、またはマクロのコード。
	@param pszType	種別。NULLの場合ファイル指定、それ以外の場合は言語の拡張子を指定
 */
void ViewCommander::Command_ExecExtMacro(const wchar_t* pszPathW, const wchar_t* pszTypeW)
{
	TCHAR			szPath[_MAX_PATH + 1];
	const TCHAR*	pszPath = NULL;				// 第1引数をTCHAR*に変換した文字列
	const TCHAR*	pszType = NULL;				// 第2引数をTCHAR*に変換した文字列
	HWND			hwndRecordingKeyMacro = NULL;

	if (pszPathW) {
		// to_tchar()で取得した文字列はdeleteしないこと。
		pszPath = to_tchar(pszPathW);
		pszType = to_tchar(pszTypeW);

	}else {
		// ファイルが指定されていない場合、ダイアログを表示する
		szPath[0] = 0;
		// マクロフォルダ
		const TCHAR* pszFolder = GetDllShareData().common.macro.szMACROFOLDER;
		// ファイル選択ダイアログの初期フォルダ
		TCHAR szInitDir[_MAX_PATH + 1];
		if (_IS_REL_PATH(pszFolder)) {
			GetInidirOrExedir(szInitDir, pszFolder);
		}else {
			_tcscpy_s(szInitDir, pszFolder);	// マクロ用フォルダ
		}
		// ファイルオープンダイアログの初期化
		DlgOpenFile dlgOpenFile;
		dlgOpenFile.Create(
			G_AppInstance(),
			view.GetHwnd(),
			_T("*.*"),
			szInitDir
		);
		if (!dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
			return;
		}
		pszPath = szPath;
		pszType = NULL;
	}

	auto& flags = GetDllShareData().flags;
	// キーマクロ記録中の場合、追加する
	if (flags.bRecordingKeyMacro &&						// キーボードマクロの記録中
		flags.hwndRecordingKeyMacro == GetMainWindow()	// キーボードマクロを記録中のウィンドウ
	) {
		LPARAM lparams[] = {(LPARAM)pszPath, 0, 0, 0};
		pSMacroMgr->Append(STAND_KEYMACRO, F_EXECEXTMACRO, lparams, view);
		// キーマクロの記録を一時停止する
		flags.bRecordingKeyMacro = false;
		hwndRecordingKeyMacro = flags.hwndRecordingKeyMacro;
		flags.hwndRecordingKeyMacro = NULL;	// キーボードマクロを記録中のウィンドウ
	}

	// 古い一時マクロの退避
	MacroManagerBase* oldMacro = pSMacroMgr->SetTempMacro(nullptr);
	bool bLoadResult = pSMacroMgr->Load(
		view,
		TEMP_KEYMACRO,
		G_AppInstance(),
		pszPath,
		pszType
	);
	if (!bLoadResult) {
		ErrorMessage(view.GetHwnd(), LS(STR_ERR_MACROERR1), pszPath);
	}else {
		pSMacroMgr->Exec(TEMP_KEYMACRO, G_AppInstance(), view, FA_NONRECORD | FA_FROMMACRO);
	}

	// 終わったら開放
	pSMacroMgr->Clear(TEMP_KEYMACRO);
	if (oldMacro) {
		pSMacroMgr->SetTempMacro(oldMacro);
	}

	// キーマクロ記録中だった場合は再開する
	if (hwndRecordingKeyMacro) {
		flags.bRecordingKeyMacro = true;
		flags.hwndRecordingKeyMacro = hwndRecordingKeyMacro;	// キーボードマクロを記録中のウィンドウ
	}
}


/*! 外部コマンド実行ダイアログ表示 */
void ViewCommander::Command_ExecCommand_Dialog(void)
{
	DlgExec dlgExec;

	// モードレスダイアログの表示
	if (!dlgExec.DoModal(G_AppInstance(), view.GetHwnd(), 0)) {
		return;
	}

	view.AddToCmdArr(dlgExec.szCommand);
	const wchar_t* cmd_string = to_wchar(dlgExec.szCommand);
	const wchar_t* curDir = to_wchar(dlgExec.szCurDir);
	const wchar_t* pszDir = curDir;
	if (curDir[0] == L'\0') {
		pszDir = NULL;
	}else {
		RecentCurDir cRecentCurDir;
		cRecentCurDir.AppendItem(dlgExec.szCurDir);
		cRecentCurDir.Terminate();
	}

	//HandleCommand(F_EXECMD, true, (LPARAM)cmd_string, 0, 0, 0);	// 外部コマンド実行コマンドの発行
	HandleCommand(F_EXECMD, true, (LPARAM)cmd_string, (LPARAM)(GetDllShareData().nExecFlgOpt), (LPARAM)pszDir, 0);	// 外部コマンド実行コマンドの発行
}


// 外部コマンド実行
void ViewCommander::Command_ExecCommand(
	LPCWSTR cmd_string,
	const int nFlgOpt,
	LPCWSTR pszCurDir
	)
{
	// パラメータ置換 (超暫定)
	const int bufmax = 1024;
	wchar_t buf[bufmax + 1];
	SakuraEnvironment::ExpandParameter(cmd_string, buf, bufmax);

	// 子プロセスの標準出力をリダイレクトする
	std::tstring buf2 = to_tchar(buf);
	std::tstring buf3;
	if (pszCurDir) {
		buf3 = to_tchar(pszCurDir);
	}
	view.ExecCmd(buf2.c_str(), nFlgOpt, (pszCurDir ? buf3.c_str() : nullptr));
}

