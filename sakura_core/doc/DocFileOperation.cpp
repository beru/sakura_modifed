#include "StdAfx.h"
#include "DocFileOperation.h"
#include "DocVisitor.h"
#include "EditDoc.h"

#include "recent/MRUFile.h"
#include "recent/MRUFolder.h"
#include "dlg/DlgOpenFile.h"
#include "_main/AppMode.h" 
#include "_main/ControlTray.h"
#include "EditApp.h"
#include "window/EditWnd.h"
#include "uiparts/WaitCursor.h"
#include "util/window.h"
#include "env/DllSharedData.h"
#include "env/SakuraEnvironment.h"
#include "plugin/Plugin.h"
#include "plugin/JackManager.h"
#include "timer.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ロック                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool DocFileOperation::_ToDoLock() const
{
	// ファイルを開いていない
	if (!doc.docFile.GetFilePathClass().IsValidPath()) {
		return false;
	}

	// ビューモード
	if (AppMode::getInstance().IsViewMode()) {
		return false;
	}

	// 排他設定
	if (GetDllShareData().common.file.nFileShareMode == FileShareMode::NonExclusive) {
		return false;
	}
	return true;
}

void DocFileOperation::DoFileLock(bool bMsg)
{
	if (this->_ToDoLock()) {
		doc.docFile.FileLock(GetDllShareData().common.file.nFileShareMode, bMsg);
	}
}

void DocFileOperation::DoFileUnlock()
{
	doc.docFile.FileUnlock();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ロードUI                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//「ファイルを開く」ダイアログ
bool DocFileOperation::OpenFileDialog(
	HWND			hwndParent,		// [in]
	const TCHAR*	pszOpenFolder,	// [in]     NULL以外を指定すると初期フォルダを指定できる
	LoadInfo*		pLoadInfo,		// [in/out] ロード情報
	std::vector<std::tstring>&	files
	)
{
	// アクティブにする
	ActivateFrameWindow(hwndParent);

	// ファイルオープンダイアログを表示
	DlgOpenFile dlgOpenFile;
	dlgOpenFile.Create(
		G_AppInstance(),
		hwndParent,
		_T("*.*"),
		pszOpenFolder ? pszOpenFolder : SakuraEnvironment::GetDlgInitialDir().c_str(),	// 初期フォルダ
		MruFile().GetPathList(),														// MRUリストのファイルのリスト
		MruFolder().GetPathList()														// OPENFOLDERリストのファイルのリスト
	);
	return dlgOpenFile.DoModalOpenDlg( pLoadInfo, &files );
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       ロードフロー                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool DocFileOperation::DoLoadFlow(LoadInfo* pLoadInfo)
{
	LoadResultType eLoadResult = LoadResultType::Failure;

	try {
		// ロード前チェック
		if (doc.NotifyCheckLoad(pLoadInfo) == CallbackResultType::Interrupt) {
			throw FlowInterruption();
		}

		// ロード処理
		doc.NotifyBeforeLoad(pLoadInfo);				// 前処理
		eLoadResult = doc.NotifyLoad(*pLoadInfo);	// 本処理
		doc.NotifyAfterLoad(*pLoadInfo);				// 後処理
	}catch (FlowInterruption) {
		eLoadResult = LoadResultType::Interrupt;
	}catch (...) {
		// 予期せぬ例外が発生した場合も NotifyFinalLoad は必ず呼ぶ！
		doc.NotifyFinalLoad(LoadResultType::Failure);
		throw;
	}
	
	// 最終処理
	doc.NotifyFinalLoad(eLoadResult);

	return eLoadResult == LoadResultType::OK;
}

// ファイルを開く
bool DocFileOperation::FileLoad(
	LoadInfo* pLoadInfo		// [in/out]
	)
{
	Timer t;

	bool bRet = DoLoadFlow(pLoadInfo);
	// オープン後自動実行マクロを実行する
	if (bRet) {
		doc.RunAutoMacro(GetDllShareData().common.macro.nMacroOnOpened);

		// プラグイン：DocumentOpenイベント実行
		Plug::Array plugs;
		WSHIfObj::List params;
		JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(doc.pEditWnd->GetActiveView(), params);
		}
	}

	TRACE(L"FileLoad time %f\n", t.ElapsedSecond());

	return bRet;
}

// ファイルを開く（自動実行マクロを実行しない）
bool DocFileOperation::FileLoadWithoutAutoMacro(
	LoadInfo* pLoadInfo		// [in/out]
	)
{
	return DoLoadFlow(pLoadInfo);
}

// 同一ファイルの再オープン
void DocFileOperation::ReloadCurrentFile(
	EncodingType nCharCode		// [in] 文字コード種別
	)
{
	auto& activeView = doc.pEditWnd->GetActiveView();

	// プラグイン：DocumentCloseイベント実行
	Plug::Array plugs;
	WSHIfObj::List params;
	JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_CLOSE, 0, &plugs);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		(*it)->Invoke(activeView, params);
	}

	auto& caret = activeView.GetCaret();
	if (!fexist(doc.docFile.GetFilePath())) {
		// ファイルが存在しない
		doc.docFile.SetCodeSet(nCharCode,  CodeTypeName(nCharCode).IsBomDefOn());
		// カーソル位置表示を更新する
		caret.ShowCaretPosInfo();
		return;
	}

	auto& textArea = activeView.GetTextArea();
	// カーソル位置保存
	size_t nViewTopLine = textArea.GetViewTopLine();	// 表示域の一番上の行(0開始)
	size_t nViewLeftCol = textArea.GetViewLeftCol();	// 表示域の一番左の桁(0開始)
	Point	ptCaretPosXY = caret.GetCaretLayoutPos();

	// ロード
	LoadInfo loadInfo;
	loadInfo.filePath = doc.docFile.GetFilePath();
	loadInfo.eCharCode = nCharCode;
	loadInfo.bViewMode = AppMode::getInstance().IsViewMode();
	loadInfo.bWritableNoMsg = !doc.IsEditable(); // すでに編集できない状態ならファイルロックのメッセージを表示しない
	loadInfo.bRequestReload = true;
	bool bRet = this->DoLoadFlow(&loadInfo);

	// カーソル位置復元 (※ここではオプションのカーソル位置復元（＝改行単位）が指定されていない場合でも復元する)
	if (ptCaretPosXY.y < (int)doc.layoutMgr.GetLineCount()) {
		textArea.SetViewTopLine(nViewTopLine);
		textArea.SetViewLeftCol(nViewLeftCol);
	}
	caret.MoveCursorProperly(ptCaretPosXY, true);
	caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

	// オープン後自動実行マクロを実行する
	if (bRet) {
		doc.RunAutoMacro(GetDllShareData().common.macro.nMacroOnOpened);
		// プラグイン：DocumentOpenイベント実行
		Plug::Array plugs;
		WSHIfObj::List params;
		JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(doc.pEditWnd->GetActiveView(), params);
		}
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         セーブUI                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! 「ファイル名を付けて保存」ダイアログ */
bool DocFileOperation::SaveFileDialog(
	SaveInfo*	pSaveInfo	// [out]
	)
{
	// 拡張子指定
	// 一時適用や拡張子なしの場合の拡張子をタイプ別設定から持ってくる
	TCHAR szDefaultWildCard[_MAX_PATH + 10];	// ユーザー指定拡張子
	{
		LPCTSTR	szExt;
		const TypeConfig& type = doc.docType.GetDocumentAttribute();
		// ファイルパスが無い場合は *.txt とする
		if (!this->doc.docFile.GetFilePathClass().IsValidPath()) {
			szExt = _T("");
		}else {
			szExt = this->doc.docFile.GetFilePathClass().GetExt();
		}
		if (type.nIdx == 0) {
			// 基本
			if (szExt[0] == _T('\0')) { 
				// ファイルパスが無いまたは拡張子なし
				_tcscpy(szDefaultWildCard, _T("*.txt"));
			}else {
				// 拡張子あり
				_tcscpy(szDefaultWildCard, _T("*"));
				_tcscat(szDefaultWildCard, szExt);
			}
		}else {
			szDefaultWildCard[0] = _T('\0'); 
			DocTypeManager::ConvertTypesExtToDlgExt(type.szTypeExts, szExt, szDefaultWildCard);
		}

		if (!this->doc.docFile.GetFilePathClass().IsValidPath()) {
			//「新規から保存時は全ファイル表示」オプション
			if (GetDllShareData().common.file.bNoFilterSaveNew)
				_tcscat(szDefaultWildCard, _T(";*.*"));	// 全ファイル表示
		}else {
			//「新規以外から保存時は全ファイル表示」オプション
			if (GetDllShareData().common.file.bNoFilterSaveFile)
				_tcscat(szDefaultWildCard, _T(";*.*"));	// 全ファイル表示
		}
	}
	// 無題に、無題番号を付ける
	if (pSaveInfo->filePath[0] == _T('\0')) {
		const EditNode* node = AppNodeManager::getInstance().GetEditNode(doc.pEditWnd->GetHwnd());
		if (0 < node->nId) {
			TCHAR szText[16];
			auto_sprintf(szText, _T("%d"), node->nId);
			auto_strcpy(pSaveInfo->filePath, LS(STR_NO_TITLE2));	// 無題
			auto_strcat(pSaveInfo->filePath, szText);
		}
	}

	// ダイアログを表示
	DlgOpenFile dlgOpenFile;
	dlgOpenFile.Create(
		G_AppInstance(),
		EditWnd::getInstance().GetHwnd(),
		szDefaultWildCard,
		SakuraEnvironment::GetDlgInitialDir().c_str(),	// 初期フォルダ
		MruFile().GetPathList(),	// 最近のファイル
		MruFolder().GetPathList()	// 最近のフォルダ
	);
	return dlgOpenFile.DoModalSaveDlg(pSaveInfo, pSaveInfo->eCharCode == CODE_CODEMAX);
}

//「ファイル名を付けて保存」ダイアログ
bool DocFileOperation::SaveFileDialog(LPTSTR szPath)
{
	SaveInfo saveInfo;
	saveInfo.filePath = szPath;
	saveInfo.eCharCode = CODE_CODEMAX; //###トリッキー
	bool bRet = SaveFileDialog(&saveInfo);
	_tcscpy_s(szPath, _MAX_PATH, saveInfo.filePath);
	return bRet;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       セーブフロー                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool DocFileOperation::DoSaveFlow(SaveInfo* pSaveInfo)
{
	SaveResultType eSaveResult = SaveResultType::Failure;
	try {
		// オプション：無変更でも上書きするか
		// ### 無変更なら上書きしないで抜ける処理はどの CDocListener の OnCheckSave() よりも前に
		// ### （保存するかどうか問い合わせたりするよりも前に）やるぺきことなので、
		// ### スマートじゃない？かもしれないけど、とりあえずここに配置しておく
		if (!GetDllShareData().common.file.bEnableUnmodifiedOverwrite) {
			// 上書きの場合
			if (pSaveInfo->bOverwriteMode) {
				// 無変更の場合は警告音を出し、終了
				if (!doc.docEditor.IsModified() &&
					pSaveInfo->eol == EolType::None &&	// ※改行コード指定保存がリクエストされた場合は、「変更があったもの」とみなす
					!pSaveInfo->bChgCodeSet
				) {		// 文字コードセットの変更が有った場合は、「変更があったもの」とみなす
					EditApp::getInstance().soundSet.NeedlessToSaveBeep();
					throw FlowInterruption();
				}
			}
		}

		// セーブ前チェック
		if (doc.NotifyCheckSave(pSaveInfo) == CallbackResultType::Interrupt) {
			throw FlowInterruption();
		}

		// セーブ前おまけ処理
		if (doc.NotifyPreBeforeSave(pSaveInfo) == CallbackResultType::Interrupt) {
			throw FlowInterruption();
		}

		// 保存前自動実行マクロを実行する
		doc.RunAutoMacro(GetDllShareData().common.macro.nMacroOnSave, pSaveInfo->filePath);

		// プラグイン：DocumentBeforeSaveイベント実行
		Plug::Array plugs;
		WSHIfObj::List params;
		JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_BEFORE_SAVE, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(doc.pEditWnd->GetActiveView(), params);
		}

		if (!pSaveInfo->bOverwriteMode) {	// 上書きでなければ前文書のクローズイベントを呼ぶ
			// プラグイン：DocumentCloseイベント実行
			plugs.clear();
			JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_CLOSE, 0, &plugs);
			for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
				(*it)->Invoke(doc.pEditWnd->GetActiveView(), params);
			}
		}

		// セーブ処理
		doc.NotifyBeforeSave(*pSaveInfo);	// 前処理
		doc.NotifySave(*pSaveInfo);			// 本処理
		doc.NotifyAfterSave(*pSaveInfo);	// 後処理

		// プラグイン：DocumentAfterSaveイベント実行
		plugs.clear();
		JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_AFTER_SAVE, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(doc.pEditWnd->GetActiveView(), params);
		}

		// 結果
		eSaveResult = SaveResultType::OK; //###仮
	}catch (FlowInterruption) {
		eSaveResult = SaveResultType::Interrupt;
	}catch (...) {
		// 予期せぬ例外が発生した場合も NotifyFinalSave は必ず呼ぶ！
		doc.NotifyFinalSave(SaveResultType::Failure);
		throw;
	}

	// 最終処理
	doc.NotifyFinalSave(eSaveResult);

	return eSaveResult == SaveResultType::OK;
}


/*! 上書き保存

	@return 保存が行われたor保存不要のため何も行わなかったときにtrueを返す
*/
bool DocFileOperation::FileSave()
{
	// ファイル名が指定されていない場合は「名前を付けて保存」のフローへ遷移
	if (!doc.docFile.GetFilePathClass().IsValidPath()) {
		return FileSaveAs();
	}

	// セーブ情報
	SaveInfo saveInfo;
	doc.GetSaveInfo(&saveInfo);
	saveInfo.eol = EolType::None;			// 改行コード無変換
	saveInfo.bOverwriteMode = true;	// 上書き要求

	// 上書き処理
	return doc.docFileOperation.DoSaveFlow(&saveInfo);
}


/*! 名前を付けて保存フロー */
bool DocFileOperation::FileSaveAs(
	const wchar_t* filename,
	EncodingType eCodeType,
	EolType eEolType,
	bool bDialog
	)
{
	// セーブ情報
	SaveInfo saveInfo;
	doc.GetSaveInfo(&saveInfo);
	saveInfo.eol = EolType::None; // 初期値は変換しない
	if (filename) {
		// ダイアログなし保存、またはマクロの引数あり
		saveInfo.filePath = to_tchar(filename);
		if (EolType::None <= eEolType && eEolType < EolType::CodeMax) {
			saveInfo.eol = eEolType;
		}
		if (IsValidCodeType(eCodeType) && eCodeType != saveInfo.eCharCode) {
			saveInfo.eCharCode = eCodeType;
			saveInfo.bBomExist = CodeTypeName(eCodeType).IsBomDefOn();
		}
	}
	if (bDialog) {
		if (!filename && AppMode::getInstance().IsViewMode()) {
			saveInfo.filePath = _T(""); //※読み込み専用モードのときはファイル名を指定しない
		}

		// ダイアログ表示
		if (!SaveFileDialog(&saveInfo)) {
			return false;
		}
	}

	// セーブ処理
	if (DoSaveFlow(&saveInfo)) {
		// オープン後自動実行マクロを実行する（ANSI版ではここで再ロード実行→自動実行マクロが実行される）
		// 提案時の Patches#1550557 に、「名前を付けて保存」でオープン後自動実行マクロが実行されることの是非について議論の経緯あり
		//   →”ファイル名に応じて表示を変化させるマクロとかを想定すると、これはこれでいいように思います。”
		doc.RunAutoMacro(GetDllShareData().common.macro.nMacroOnOpened);

		// プラグイン：DocumentOpenイベント実行
		Plug::Array plugs;
		WSHIfObj::List params;
		JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(doc.pEditWnd->GetActiveView(), params);
		}

		return true;
	}

	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         クローズ                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	閉じて(無題)。
	ユーザキャンセル操作等によりクローズされなかった場合は false を返す。
*/
bool DocFileOperation::FileClose()
{
	// ファイルを閉じるときのMRU登録 & 保存確認 & 保存実行
	if (!doc.OnFileClose(false)) {
		return false;
	}

	// プラグイン：DocumentCloseイベント実行
	Plug::Array plugs;
	WSHIfObj::List params;
	JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_CLOSE, 0, &plugs);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		(*it)->Invoke(doc.pEditWnd->GetActiveView(), params);
	}

	// 既存データのクリア
	doc.InitDoc();

	// 全ビューの初期化
	doc.InitAllView();

	doc.SetCurDirNotitle();

	// 無題番号取得
	AppNodeManager::getInstance().GetNoNameNumber(doc.pEditWnd->GetHwnd());

	// 親ウィンドウのタイトルを更新
	doc.pEditWnd->UpdateCaption();

	// オープン後自動実行マクロを実行する
	doc.RunAutoMacro(GetDllShareData().common.macro.nMacroOnOpened);

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          その他                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* 閉じて開く */
void DocFileOperation::FileCloseOpen(const LoadInfo& argLoadInfo)
{
	// ファイルを閉じるときのMRU登録 & 保存確認 & 保存実行
	if (!doc.OnFileClose(false)) {
		return;
	}

	// プラグイン：DocumentCloseイベント実行
	Plug::Array plugs;
	WSHIfObj::List params;
	JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_CLOSE, 0, &plugs);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		(*it)->Invoke(doc.pEditWnd->GetActiveView(), params);
	}

	// ファイル名指定が無い場合はダイアログで入力させる
	LoadInfo loadInfo = argLoadInfo;
	if (loadInfo.filePath.Length() == 0) {
		std::vector<std::tstring> files;
		if (!OpenFileDialog(EditWnd::getInstance().GetHwnd(), NULL, &loadInfo, files)) {
			return;
		}
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

	// 既存データのクリア
	doc.InitDoc();

	// 全ビューの初期化
	doc.InitAllView();

	// 開く
	FileLoadWithoutAutoMacro(&loadInfo);

	if (!doc.docFile.GetFilePathClass().IsValidPath()) {
		doc.SetCurDirNotitle();
		AppNodeManager::getInstance().GetNoNameNumber(doc.pEditWnd->GetHwnd());
	}

	// 親ウィンドウのタイトルを更新
	doc.pEditWnd->UpdateCaption();

	// オープン後自動実行マクロを実行する
	// ※ロードしてなくても(無題)には変更済み
	doc.RunAutoMacro(GetDllShareData().common.macro.nMacroOnOpened);

	// プラグイン：DocumentOpenイベント実行
	plugs.clear();
	JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		(*it)->Invoke(doc.pEditWnd->GetActiveView(), params);
	}
}


