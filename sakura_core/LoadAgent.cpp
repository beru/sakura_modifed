#include "StdAfx.h"
#include "LoadAgent.h"
#include "ReadManager.h"
#include "_main/AppMode.h"
#include "_main/ControlTray.h"
#include "EditApp.h"
#include "env/DocTypeManager.h"
#include "env/ShareData.h"
#include "doc/EditDoc.h"
#include "view/EditView.h"
#include "window/EditWnd.h"
#include "uiparts/VisualProgress.h"
#include "util/fileUtil.h"

CallbackResultType LoadAgent::OnCheckLoad(LoadInfo* pLoadInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	// リロード要求の場合は、継続。
	if (pLoadInfo->bRequestReload) {
		goto next;
	}

	// フォルダが指定された場合は「ファイルを開く」ダイアログを表示し、実際のファイル入力を促す
	if (IsDirectory(pLoadInfo->filePath)) {
		std::vector<std::tstring> files;
		LoadInfo loadInfo(_T(""), CODE_AUTODETECT, false);
		bool bDlgResult = pDoc->docFileOperation.OpenFileDialog(
			EditWnd::getInstance().GetHwnd(),
			pLoadInfo->filePath,	// 指定されたフォルダ
			&loadInfo,
			files
		);
		if (!bDlgResult) {
			return CallbackResultType::Interrupt; // キャンセルされた場合は中断
		}
		size_t nSize = files.size();
		if (0 < nSize) {
			loadInfo.filePath = files[0].c_str();
			// 他のファイルは新規ウィンドウ
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
		*pLoadInfo = loadInfo;
	}

	// 他のウィンドウで既に開かれている場合は、それをアクティブにする
	HWND hWndOwner;
	if (ShareData::getInstance().ActiveAlreadyOpenedWindow(pLoadInfo->filePath, &hWndOwner, pLoadInfo->eCharCode)) {
		pLoadInfo->bOpened = true;
		return CallbackResultType::Interrupt;
	}

	// 現在のウィンドウに対してファイルを読み込めない場合は、新たなウィンドウを開き、そこにファイルを読み込ませる
	if (!pDoc->IsAcceptLoad()) {
		ControlTray::OpenNewEditor(
			G_AppInstance(),
			EditWnd::getInstance().GetHwnd(),
			*pLoadInfo
		);
		return CallbackResultType::Interrupt;
	}

next:
	auto& csFile = GetDllShareData().common.file;
	// オプション：開こうとしたファイルが存在しないとき警告する
	if (csFile.GetAlertIfFileNotExist()) {
		if (!fexist(pLoadInfo->filePath)) {
			InfoBeep();
			//	Popupウィンドウを表示しないように．
			//	ここでステータスメッセージを使っても画面に表示されない．
			TopInfoMessage(
				EditWnd::getInstance().GetHwnd(),
				LS(STR_NOT_EXSIST_SAVE),
				pLoadInfo->filePath.GetBufferPointer()
			);
		}
	}

	// 読み取り可能チェック
	do {
		File file(pLoadInfo->filePath.c_str());

		// ファイルが存在しない場合はチェック省略
		if (!file.IsFileExist()) {
			break;
		}

		// ロックは一時的に解除してチェックする（チェックせずに後戻りできないところまで進めるより安全）
		// ※ ロックしていてもアクセス許可の変更によって読み取れなくなっていることもある
		bool bLock = (pLoadInfo->IsSamePath(pDoc->docFile.GetFilePath()) && pDoc->docFile.IsFileLocking());
		if (bLock) {
			pDoc->docFileOperation.DoFileUnlock();
		}

		// チェック
		if (!file.IsFileReadable()) {
			if (bLock) {
				pDoc->docFileOperation.DoFileLock(false);
			}
			ErrorMessage(
				EditWnd::getInstance().GetHwnd(),
				LS(STR_LOADAGENT_ERR_OPEN),
				pLoadInfo->filePath.c_str()
			);
			return CallbackResultType::Interrupt; // ファイルが存在しているのに読み取れない場合は中断
		}
		if (bLock) {
			pDoc->docFileOperation.DoFileLock(false);
		}
	}while (false);	//	1回しか通らない. breakでここまで飛ぶ

	// ファイルサイズチェック
	if (csFile.bAlertIfLargeFile) {
		WIN32_FIND_DATA wfd;
		HANDLE nFind = ::FindFirstFile( pLoadInfo->filePath.c_str(), &wfd );
		if (nFind != INVALID_HANDLE_VALUE) {
			::FindClose( nFind );
			LARGE_INTEGER nFileSize;
			nFileSize.HighPart = wfd.nFileSizeHigh;
			nFileSize.LowPart = wfd.nFileSizeLow;
			// csFile.nAlertFileSize はMB単位
			if ((nFileSize.QuadPart>>20) >= (csFile.nAlertFileSize)) {
				int nRet = MYMESSAGEBOX( EditWnd::getInstance().GetHwnd(),
					MB_ICONQUESTION | MB_YESNO | MB_TOPMOST,
					GSTR_APPNAME,
					LS(STR_LOADAGENT_BIG_FILE),
					csFile.nAlertFileSize );
				if (nRet != IDYES) {
					return CallbackResultType::Interrupt;
				}
			}
		}
	}

	return CallbackResultType::Continue;
}

void LoadAgent::OnBeforeLoad(LoadInfo* pLoadInfo)
{
}

LoadResultType LoadAgent::OnLoad(const LoadInfo& loadInfo)
{
	LoadResultType eRet = LoadResultType::OK;
	EditDoc* pDoc = GetListeningDoc();

	// 既存データのクリア
	pDoc->InitDoc(); //$$

	// パスを確定
	pDoc->SetFilePathAndIcon(loadInfo.filePath);

	// 文書種別確定
	pDoc->docType.SetDocumentType(loadInfo.nType, true);
	pDoc->pEditWnd->pViewFontMiniMap->UpdateFont(&pDoc->pEditWnd->GetLogfont());
	InitCharWidthCache(pDoc->pEditWnd->pViewFontMiniMap->GetLogfont(), CharWidthFontMode::MiniMap);
	SelectCharWidthCache(CharWidthFontMode::Edit, pDoc->pEditWnd->GetLogfontCacheMode());
	InitCharWidthCache(pDoc->pEditWnd->GetLogfont());
	pDoc->pEditWnd->pViewFont->UpdateFont(&pDoc->pEditWnd->GetLogfont());

	// 起動と同時に読む場合は予めアウトライン解析画面を配置しておく
	// （ファイル読み込み開始とともにビューが表示されるので、あとで配置すると画面のちらつきが大きいの）
	if (!pDoc->pEditWnd->dlgFuncList.bEditWndReady) {
		pDoc->pEditWnd->dlgFuncList.Refresh();
		HWND hEditWnd = pDoc->pEditWnd->GetHwnd();
		if (!::IsIconic( hEditWnd ) && pDoc->pEditWnd->dlgFuncList.GetHwnd()) {
			RECT rc;
			::GetClientRect( hEditWnd, &rc );
			::SendMessage( hEditWnd, WM_SIZE, ::IsZoomed( hEditWnd )? SIZE_MAXIMIZED: SIZE_RESTORED, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ) );
		}
	}

	// ファイルが存在する場合はファイルを読む
	if (fexist(loadInfo.filePath)) {
		// CDocLineMgrの構成
		ReadManager reader;
		ProgressSubject* pOld = EditApp::getInstance().pVisualProgress->ProgressListener::Listen(&reader);
		CodeConvertResult eReadResult = reader.ReadFile_To_CDocLineMgr(
			pDoc->docLineMgr,
			loadInfo,
			&pDoc->docFile.fileInfo
		);
		if (eReadResult == CodeConvertResult::LoseSome) {
			eRet = LoadResultType::LoseSome;
		}
		EditApp::getInstance().pVisualProgress->ProgressListener::Listen(pOld);
	}else {
		// 存在しないときもドキュメントに文字コードを反映する
		const TypeConfig& types = pDoc->docType.GetDocumentAttribute();
		pDoc->docFile.SetCodeSet( loadInfo.eCharCode, 
			( loadInfo.eCharCode == types.encoding.eDefaultCodetype ) ?
				types.encoding.bDefaultBom : CodeTypeName( loadInfo.eCharCode ).IsBomDefOn() );
	}

	// レイアウト情報の変更
	// 「指定桁で折り返す」以外の時は折り返し幅をMAXLINEKETASで初期化する
	// 「右端で折り返す」は、この後のOnSize()で再設定される
	const TypeConfig& ref = pDoc->docType.GetDocumentAttribute();
	size_t nMaxLineKetas = ref.nMaxLineKetas;
	if (ref.nTextWrapMethod != TextWrappingMethod::SettingWidth) {
		nMaxLineKetas = MAXLINEKETAS;
	}

	ProgressSubject* pOld = EditApp::getInstance().pVisualProgress->ProgressListener::Listen(&pDoc->layoutMgr);
	pDoc->layoutMgr.SetLayoutInfo(true, ref, ref.nTabSpace, nMaxLineKetas);
	pDoc->pEditWnd->ClearViewCaretPosInfo();
	
	EditApp::getInstance().pVisualProgress->ProgressListener::Listen(pOld);

	return eRet;
}


void LoadAgent::OnAfterLoad(const LoadInfo& loadInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	// 親ウィンドウのタイトルを更新
	pDoc->pEditWnd->UpdateCaption();

	// -- -- ※ InitAllViewでやってたこと -- --
	pDoc->nCommandExecNum = 0;

	// テキストの折り返し方法を初期化
	pDoc->nTextWrapMethodCur = pDoc->docType.GetDocumentAttribute().nTextWrapMethod;	// 折り返し方法
	pDoc->bTextWrapMethodCurTemp = false;													// 一時設定適用中を解除
	pDoc->blfCurTemp = false;
	pDoc->bTabSpaceCurTemp = false;

	// 「折り返さない」ならテキスト最大幅を算出、それ以外は変数をクリア
	if (pDoc->nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
		pDoc->layoutMgr.CalculateTextWidth();		// テキスト最大幅を算出する
	}else {
		pDoc->layoutMgr.ClearLayoutLineWidth();		// 各行のレイアウト行長の記憶をクリアする
	}
}


void LoadAgent::OnFinalLoad(LoadResultType eLoadResult)
{
	EditDoc* pDoc = GetListeningDoc();

	if (eLoadResult == LoadResultType::Failure) {
		pDoc->SetFilePathAndIcon(_T(""));
		pDoc->docFile.SetBomDefoult();
	}
	if (eLoadResult == LoadResultType::LoseSome) {
		AppMode::getInstance().SetViewMode(true);
	}

	// 再描画 $$不足
	auto& editWnd = EditWnd::getInstance();
	// editWnd.GetActiveView().SetDrawSwitch(true);
	bool bDraw = editWnd.GetActiveView().GetDrawSwitch();
	if (bDraw) {
		editWnd.Views_RedrawAll(); // ビュー再描画
		InvalidateRect(editWnd.GetHwnd(), nullptr, TRUE);
	}
	Caret& caret = editWnd.GetActiveView().GetCaret();
	caret.MoveCursor(caret.GetCaretLayoutPos(), true);
	editWnd.GetActiveView().AdjustScrollBars();
}

