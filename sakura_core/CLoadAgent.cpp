/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#include "StdAfx.h"
#include "CLoadAgent.h"
#include "CReadManager.h"
#include "_main/CAppMode.h"
#include "_main/CControlTray.h"
#include "CEditApp.h"
#include "env/CDocTypeManager.h"
#include "env/CShareData.h"
#include "doc/CEditDoc.h"
#include "view/CEditView.h"
#include "window/CEditWnd.h"
#include "uiparts/CVisualProgress.h"
#include "util/file.h"

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
		bool bDlgResult = pDoc->m_docFileOperation.OpenFileDialog(
			EditWnd::getInstance()->GetHwnd(),
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
					EditWnd::getInstance()->GetHwnd(),
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
	if (ShareData::getInstance()->ActiveAlreadyOpenedWindow(pLoadInfo->filePath, &hWndOwner, pLoadInfo->eCharCode)) {
		pLoadInfo->bOpened = true;
		return CallbackResultType::Interrupt;
	}

	// 現在のウィンドウに対してファイルを読み込めない場合は、新たなウィンドウを開き、そこにファイルを読み込ませる
	if (!pDoc->IsAcceptLoad()) {
		ControlTray::OpenNewEditor(
			G_AppInstance(),
			EditWnd::getInstance()->GetHwnd(),
			*pLoadInfo
		);
		return CallbackResultType::Interrupt;
	}

next:
	auto& csFile = GetDllShareData().m_common.m_file;
	// オプション：開こうとしたファイルが存在しないとき警告する
	if (csFile.GetAlertIfFileNotExist()) {
		if (!fexist(pLoadInfo->filePath)) {
			InfoBeep();
			//	Feb. 15, 2003 genta Popupウィンドウを表示しないように．
			//	ここでステータスメッセージを使っても画面に表示されない．
			TopInfoMessage(
				EditWnd::getInstance()->GetHwnd(),
				LS(STR_NOT_EXSIST_SAVE),	// Mar. 24, 2001 jepro 若干修正
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
		bool bLock = (pLoadInfo->IsSamePath(pDoc->m_docFile.GetFilePath()) && pDoc->m_docFile.IsFileLocking());
		if (bLock) {
			pDoc->m_docFileOperation.DoFileUnlock();
		}

		// チェック
		if (!file.IsFileReadable()) {
			if (bLock) {
				pDoc->m_docFileOperation.DoFileLock(false);
			}
			ErrorMessage(
				EditWnd::getInstance()->GetHwnd(),
				LS(STR_LOADAGENT_ERR_OPEN),
				pLoadInfo->filePath.c_str()
			);
			return CallbackResultType::Interrupt; // ファイルが存在しているのに読み取れない場合は中断
		}
		if (bLock) {
			pDoc->m_docFileOperation.DoFileLock(false);
		}
	}while (false);	//	1回しか通らない. breakでここまで飛ぶ

	// ファイルサイズチェック
	if (csFile.m_bAlertIfLargeFile) {
		WIN32_FIND_DATA wfd;
		HANDLE nFind = ::FindFirstFile( pLoadInfo->filePath.c_str(), &wfd );
		if (nFind != INVALID_HANDLE_VALUE) {
			::FindClose( nFind );
			LARGE_INTEGER nFileSize;
			nFileSize.HighPart = wfd.nFileSizeHigh;
			nFileSize.LowPart = wfd.nFileSizeLow;
			// csFile.m_nAlertFileSize はMB単位
			if ((nFileSize.QuadPart>>20) >= (csFile.m_nAlertFileSize)) {
				int nRet = MYMESSAGEBOX( EditWnd::getInstance()->GetHwnd(),
					MB_ICONQUESTION | MB_YESNO | MB_TOPMOST,
					GSTR_APPNAME,
					LS(STR_LOADAGENT_BIG_FILE),
					csFile.m_nAlertFileSize );
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
	pDoc->SetFilePathAndIcon( loadInfo.filePath );

	// 文書種別確定
	pDoc->m_docType.SetDocumentType( loadInfo.nType, true );
	pDoc->m_pEditWnd->m_pViewFontMiniMap->UpdateFont(&pDoc->m_pEditWnd->GetLogfont());
	InitCharWidthCache( pDoc->m_pEditWnd->m_pViewFontMiniMap->GetLogfont(), CharWidthFontMode::MiniMap );
	SelectCharWidthCache( CharWidthFontMode::Edit, pDoc->m_pEditWnd->GetLogfontCacheMode() );
	InitCharWidthCache( pDoc->m_pEditWnd->GetLogfont() );
	pDoc->m_pEditWnd->m_pViewFont->UpdateFont(&pDoc->m_pEditWnd->GetLogfont());

	// 起動と同時に読む場合は予めアウトライン解析画面を配置しておく
	// （ファイル読み込み開始とともにビューが表示されるので、あとで配置すると画面のちらつきが大きいの）
	if (!pDoc->m_pEditWnd->m_dlgFuncList.m_bEditWndReady) {
		pDoc->m_pEditWnd->m_dlgFuncList.Refresh();
		HWND hEditWnd = pDoc->m_pEditWnd->GetHwnd();
		if (!::IsIconic( hEditWnd ) && pDoc->m_pEditWnd->m_dlgFuncList.GetHwnd()) {
			RECT rc;
			::GetClientRect( hEditWnd, &rc );
			::SendMessage( hEditWnd, WM_SIZE, ::IsZoomed( hEditWnd )? SIZE_MAXIMIZED: SIZE_RESTORED, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ) );
		}
	}

	// ファイルが存在する場合はファイルを読む
	if (fexist(loadInfo.filePath)) {
		// CDocLineMgrの構成
		ReadManager reader;
		ProgressSubject* pOld = EditApp::getInstance()->m_pVisualProgress->ProgressListener::Listen(&reader);
		CodeConvertResult eReadResult = reader.ReadFile_To_CDocLineMgr(
			&pDoc->m_docLineMgr,
			loadInfo,
			&pDoc->m_docFile.m_fileInfo
		);
		if (eReadResult == CodeConvertResult::LoseSome) {
			eRet = LoadResultType::LoseSome;
		}
		EditApp::getInstance()->m_pVisualProgress->ProgressListener::Listen(pOld);
	}else {
		// 存在しないときもドキュメントに文字コードを反映する
		const TypeConfig& types = pDoc->m_docType.GetDocumentAttribute();
		pDoc->m_docFile.SetCodeSet( loadInfo.eCharCode, 
			( loadInfo.eCharCode == types.m_encoding.m_eDefaultCodetype ) ?
				types.m_encoding.m_bDefaultBom : CodeTypeName( loadInfo.eCharCode ).IsBomDefOn() );
	}

	// レイアウト情報の変更
	// 2008.06.07 nasukoji	折り返し方法の追加に対応
	// 「指定桁で折り返す」以外の時は折り返し幅をMAXLINEKETASで初期化する
	// 「右端で折り返す」は、この後のOnSize()で再設定される
	const TypeConfig& ref = pDoc->m_docType.GetDocumentAttribute();
	LayoutInt nMaxLineKetas = ref.m_nMaxLineKetas;
	if (ref.m_nTextWrapMethod != TextWrappingMethod::SettingWidth) {
		nMaxLineKetas = MAXLINEKETAS;
	}

	ProgressSubject* pOld = EditApp::getInstance()->m_pVisualProgress->ProgressListener::Listen(&pDoc->m_layoutMgr);
	pDoc->m_layoutMgr.SetLayoutInfo( true, ref, ref.m_nTabSpace, nMaxLineKetas );
	pDoc->m_pEditWnd->ClearViewCaretPosInfo();
	
	EditApp::getInstance()->m_pVisualProgress->ProgressListener::Listen(pOld);

	return eRet;
}


void LoadAgent::OnAfterLoad(const LoadInfo& loadInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	// 親ウィンドウのタイトルを更新
	pDoc->m_pEditWnd->UpdateCaption();

	// -- -- ※ InitAllViewでやってたこと -- -- //	// 2009.08.28 nasukoji	CEditView::OnAfterLoad()からここに移動
	pDoc->m_nCommandExecNum = 0;

	// テキストの折り返し方法を初期化
	pDoc->m_nTextWrapMethodCur = pDoc->m_docType.GetDocumentAttribute().m_nTextWrapMethod;	// 折り返し方法
	pDoc->m_bTextWrapMethodCurTemp = false;													// 一時設定適用中を解除
	pDoc->m_blfCurTemp = false;
	pDoc->m_bTabSpaceCurTemp = false;

	// 2009.08.28 nasukoji	「折り返さない」ならテキスト最大幅を算出、それ以外は変数をクリア
	if (pDoc->m_nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
		pDoc->m_layoutMgr.CalculateTextWidth();		// テキスト最大幅を算出する
	}else {
		pDoc->m_layoutMgr.ClearLayoutLineWidth();		// 各行のレイアウト行長の記憶をクリアする
	}
}


void LoadAgent::OnFinalLoad(LoadResultType eLoadResult)
{
	EditDoc* pDoc = GetListeningDoc();

	if (eLoadResult == LoadResultType::Failure) {
		pDoc->SetFilePathAndIcon(_T(""));
		pDoc->m_docFile.SetBomDefoult();
	}
	if (eLoadResult == LoadResultType::LoseSome) {
		AppMode::getInstance()->SetViewMode(true);
	}

	// 再描画 $$不足
	// EditWnd::getInstance()->GetActiveView().SetDrawSwitch(true);
	bool bDraw = EditWnd::getInstance()->GetActiveView().GetDrawSwitch();
	if (bDraw) {
		EditWnd::getInstance()->Views_RedrawAll(); // ビュー再描画
		InvalidateRect( EditWnd::getInstance()->GetHwnd(), NULL, TRUE );
	}
	Caret& caret = EditWnd::getInstance()->GetActiveView().GetCaret();
	caret.MoveCursor(caret.GetCaretLayoutPos(), true);
	EditWnd::getInstance()->GetActiveView().AdjustScrollBars();
}

