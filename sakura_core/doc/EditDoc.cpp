/*!	@file
	@brief 文書関連情報の管理

	@author Norio Nakatani
	@date	1998/03/13 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, genta, YAZAKI, jepro, novice, asa-o, MIK,
	Copyright (C) 2002, YAZAKI, hor, genta, aroka, frozen, Moca, MIK
	Copyright (C) 2003, MIK, genta, ryoji, Moca, zenryaku, naoh, wmlhq
	Copyright (C) 2004, genta, novice, Moca, MIK, zenryaku
	Copyright (C) 2005, genta, naoh, FILE, Moca, ryoji, D.S.Koba, aroka
	Copyright (C) 2006, genta, ryoji, aroka
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, nasukoji
	Copyright (C) 2011, ryoji
	Copyright (C) 2013, Uchi

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
#include <stdlib.h>
#include <string.h>	// Apr. 03, 2003 genta
#include <OleCtl.h>
#include <memory>

#include "doc/EditDoc.h"
#include "doc/logic/DocLine.h" /// 2002/2/3 aroka
#include "doc/layout/Layout.h"	// 2007.08.22 ryoji 追加
#include "docplus/ModifyManager.h"
#include "_main/global.h"
#include "_main/AppMode.h"
#include "_main/ControlTray.h"
#include "_main/NormalProcess.h"
#include "window/EditWnd.h"
#include "_os/Clipboard.h"
#include "CodeChecker.h"
#include "EditApp.h"
#include "GrepAgent.h"
#include "print/PrintPreview.h"
#include "uiparts/VisualProgress.h"
#include "charset/CodeMediator.h"
#include "charset/charcode.h"
#include "debug/RunningTimer.h"
#include "env/SakuraEnvironment.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "func/Funccode.h"
#include "mem/MemoryIterator.h"	// 2007.08.22 ryoji 追加
#include "outline/FuncInfoArr.h" /// 2002/2/3 aroka
#include "macro/SMacroMgr.h"
#include "recent/MRUFolder.h"
#include "util/fileUtil.h"
#include "util/format.h"
#include "util/module.h"
#include "util/string_ex2.h"
#include "util/window.h"
#include "sakura_rc.h"

#define IDT_ROLLMOUSE	1

// 編集禁止コマンド
static const EFunctionCode EIsModificationForbidden[] = {
	F_WCHAR,
	F_IME_CHAR,
	F_UNDO,		// 2007.10.12 genta
	F_REDO,		// 2007.10.12 genta
	F_DELETE,
	F_DELETE_BACK,
	F_WordDeleteToStart,
	F_WordDeleteToEnd,
	F_WordCut,
	F_WordDelete,
	F_LineCutToStart,
	F_LineCutToEnd,
	F_LineDeleteToStart,
	F_LineDeleteToEnd,
	F_CUT_LINE,
	F_DELETE_LINE,
	F_DUPLICATELINE,
	F_INDENT_TAB,
	F_UNINDENT_TAB,
	F_INDENT_SPACE,
	F_UNINDENT_SPACE,
	F_LTRIM,		// 2001.12.03 hor
	F_RTRIM,		// 2001.12.03 hor
	F_SORT_ASC,	// 2001.12.11 hor
	F_SORT_DESC,	// 2001.12.11 hor
	F_MERGE,		// 2001.12.11 hor
	F_CUT,
	F_PASTE,
	F_PASTEBOX,
	F_INSTEXT_W,
	F_ADDTAIL_W,
	F_INS_DATE,
	F_INS_TIME,
	F_CTRL_CODE_DIALOG,	//@@@ 2002.06.02 MIK
	F_TOLOWER,
	F_TOUPPER,
	F_TOHANKAKU,
	F_TOZENKAKUKATA,
	F_TOZENKAKUHIRA,
	F_HANKATATOZENKATA,
	F_HANKATATOZENHIRA,
	F_TOZENEI,					// 2001/07/30 Misaka
	F_TOHANEI,
	F_TOHANKATA,				// 2002/08/29 ai
	F_TABTOSPACE,
	F_SPACETOTAB,  //---- Stonee, 2001/05/27
	F_CODECNV_AUTO2SJIS,
	F_CODECNV_EMAIL,
	F_CODECNV_EUC2SJIS,
	F_CODECNV_UNICODE2SJIS,
	F_CODECNV_UTF82SJIS,
	F_CODECNV_UTF72SJIS,
	F_CODECNV_UNICODEBE2SJIS,
	F_CODECNV_SJIS2JIS,
	F_CODECNV_SJIS2EUC,
	F_CODECNV_SJIS2UTF8,
	F_CODECNV_SJIS2UTF7,
	F_REPLACE_DIALOG,
	F_REPLACE,
	F_REPLACE_ALL,
	F_CHGMOD_INS,
	F_HOKAN,
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        生成と破棄                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*!
	@note
		pEditWnd はコンストラクタ内では使用しないこと．

	@date 2000.05.12 genta 初期化方法変更
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、Processにひとつあるのみ。
	@date 2002.01.14 YAZAKI 印刷PreviewをCPrintPreviewに独立させたことによる変更
	@date 2004.06.21 novice タグジャンプ機能追加
*/
EditDoc::EditDoc(EditApp& app)
	:
	docFile(*this),
	docFileOperation(*this),
	docEditor(*this),
	docType(*this),
	docOutline(*this),
	nCommandExecNum(0),				// コマンド実行回数
	hBackImg(NULL)
{
	MY_RUNNINGTIMER(runningTimer, "EditDoc::EditDoc");
	
	// レイアウト管理情報の初期化
	layoutMgr.Create(this, &docLineMgr);
	
	// レイアウト情報の変更
	// 2008.06.07 nasukoji	折り返し方法の追加に対応
	// 「指定桁で折り返す」以外の時は折り返し幅をMAXLINEKETASで初期化する
	// 「右端で折り返す」は、この後のOnSize()で再設定される
	const TypeConfig& ref = docType.GetDocumentAttribute();
	size_t nMaxLineKetas = ref.nMaxLineKetas;
	if (ref.nTextWrapMethod != TextWrappingMethod::SettingWidth) {
		nMaxLineKetas = MAXLINEKETAS;
	}
	layoutMgr.SetLayoutInfo(true, ref, ref.nTabSpace, nMaxLineKetas);

	// 自動保存の設定	// Aug, 21, 2000 genta
	autoSaveAgent.ReloadAutoSaveParam();

	//$$ ModifyManager インスタンスを生成
	ModifyManager::getInstance();

	//$$ CodeChecker インスタンスを生成
	CodeChecker::getInstance();

	// 2008.06.07 nasukoji	テキストの折り返し方法を初期化
	nTextWrapMethodCur = docType.GetDocumentAttribute().nTextWrapMethod;	// 折り返し方法
	bTextWrapMethodCurTemp = false;									// 一時設定適用中を解除
	blfCurTemp = false;
	nPointSizeCur = -1;
	nPointSizeOrg = -1;
	bTabSpaceCurTemp = false;

	// 文字コード種別を初期化
	docFile.SetCodeSet(ref.encoding.eDefaultCodetype, ref.encoding.bDefaultBom);
	docEditor.newLineCode = ref.encoding.eDefaultEoltype;

	// 排他制御オプションを初期化
	docFile.SetShareMode(GetDllShareData().common.file.nFileShareMode);

#ifdef _DEBUG
	{
		// 編集禁止コマンドの並びをチェック
		for (int i=0; i<_countof(EIsModificationForbidden)-1; ++i){
			assert( EIsModificationForbidden[i] <  EIsModificationForbidden[i+1] );
		}
	}
#endif
}


EditDoc::~EditDoc()
{
	if (hBackImg) {
		::DeleteObject(hBackImg);
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        生成と破棄                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void EditDoc::Clear()
{
	// ファイルの排他ロック解除
	docFileOperation.DoFileUnlock();

	// 書込み禁止のクリア
	docLocker.Clear();

	// Undo, Redoバッファのクリア
	docEditor.opeBuf.ClearAll();

	// テキストデータのクリア
	docLineMgr.DeleteAllLine();

	// ファイルパスとアイコンのクリア
	SetFilePathAndIcon(_T(""));

	// ファイルのタイムスタンプのクリア
	docFile.ClearFileTime();

	// 「基本」のタイプ別設定を適用
	docType.SetDocumentType(DocTypeManager().GetDocumentTypeOfPath(docFile.GetFilePath()), true);
	blfCurTemp = false;
	pEditWnd->pViewFontMiniMap->UpdateFont(&pEditWnd->GetLogfont());
	InitCharWidthCache( pEditWnd->pViewFontMiniMap->GetLogfont(), CharWidthFontMode::MiniMap );
	SelectCharWidthCache(CharWidthFontMode::Edit, pEditWnd->GetLogfontCacheMode());
	InitCharWidthCache(pEditWnd->GetLogfont());
	pEditWnd->pViewFont->UpdateFont(&pEditWnd->GetLogfont());

	// 2008.06.07 nasukoji	折り返し方法の追加に対応
	const TypeConfig& ref = docType.GetDocumentAttribute();
	size_t nMaxLineKetas = ref.nMaxLineKetas;
	if (ref.nTextWrapMethod != TextWrappingMethod::SettingWidth) {
		nMaxLineKetas = MAXLINEKETAS;
	}
	layoutMgr.SetLayoutInfo(true, ref, ref.nTabSpace, nMaxLineKetas);
	pEditWnd->ClearViewCaretPosInfo();
}

// 既存データのクリア
void EditDoc::InitDoc()
{
	AppMode::getInstance().SetViewMode(false);	// ビューモード $$ 今後OnClearDocを用意したい
	AppMode::getInstance().szGrepKey[0] = 0;	//$$

	EditApp::getInstance().pGrepAgent->bGrepMode = false;	// Grepモード	//$$同上
	autoReloadAgent.watchUpdateType = WatchUpdateType::Query; // Dec. 4, 2002 genta 更新監視方法 $$

	// 2005.06.24 Moca バグ修正
	// アウトプットウィンドウで「閉じて(無題)」を行ってもアウトプットウィンドウのまま
	if (AppMode::getInstance().IsDebugMode()) {
		AppMode::getInstance().SetDebugModeOFF();
	}

	// Sep. 10, 2002 genta
	// アイコン設定はファイル名設定と一体化のためここからは削除

	Clear();

	// 変更フラグ
	docEditor.SetModified(false, false);	// Jan. 22, 2002 genta

	// 文字コード種別
	const TypeConfig& ref = docType.GetDocumentAttribute();
	docFile.SetCodeSet(ref.encoding.eDefaultCodetype, ref.encoding.bDefaultBom);
	docEditor.newLineCode = ref.encoding.eDefaultEoltype;

	// Oct. 2, 2005 genta 挿入モード
	docEditor.SetInsMode(GetDllShareData().common.general.bIsINSMode);

	cookie.DeleteAll(L"document");
}

void EditDoc::SetBackgroundImage()
{
	FilePath path = docType.GetDocumentAttribute().szBackImgPath.c_str();
	if (hBackImg) {
		::DeleteObject(hBackImg);
		hBackImg = NULL;
	}
	if (path[0] == 0) {
		return;
	}
	if (_IS_REL_PATH(path.c_str())) {
		FilePath fullPath;
		GetInidirOrExedir(&fullPath[0], &path[0]);
		path = fullPath;
	}
	const TCHAR* ext = path.GetExt();
	if (auto_stricmp(ext, _T(".bmp")) != 0) {
		HANDLE hFile = ::CreateFile(path.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
		if (hFile == INVALID_HANDLE_VALUE) {
			return;
		}
		DWORD fileSize  = ::GetFileSize(hFile, NULL);
		HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, fileSize);
		if (!hGlobal) {
			::CloseHandle(hFile);
			return;
		}
		DWORD nRead;
		BOOL bRead = ::ReadFile(hFile, GlobalLock(hGlobal), fileSize, &nRead, NULL);
		::CloseHandle(hFile);
		hFile = NULL;
		if (!bRead) {
			::GlobalFree(hGlobal);
			return;
		}
		::GlobalUnlock(hGlobal);
		{
			IPicture* iPicture = nullptr;
			IStream* iStream = nullptr;
			// hGlobalの管理を移譲
			if (::CreateStreamOnHGlobal(hGlobal, TRUE, &iStream) != S_OK) {
				GlobalFree(hGlobal);
			}else {
				if (::OleLoadPicture(iStream, fileSize, FALSE, IID_IPicture, (void**)&iPicture) != S_OK) {
				}else {
					HBITMAP hBitmap = NULL;
					short imgType = PICTYPE_NONE;
					if (iPicture->get_Type(&imgType) == S_OK
						&& imgType == PICTYPE_BITMAP
						&& iPicture->get_Handle((OLE_HANDLE*)&hBitmap) == S_OK
					) {
						nBackImgWidth = nBackImgHeight = 1;
						hBackImg = (HBITMAP)::CopyImage(hBitmap, IMAGE_BITMAP, 0, 0, 0);
					}
				}
			}
			if (iStream)  iStream->Release();
			if (iPicture) iPicture->Release();
		}
	}else {
		hBackImg = (HBITMAP)::LoadImage(NULL, path.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	}
	if (hBackImg) {
		BITMAP bmp;
		GetObject(hBackImg, sizeof(BITMAP), &bmp);
		nBackImgWidth  = bmp.bmWidth;
		nBackImgHeight = bmp.bmHeight;
		if (nBackImgWidth == 0 || nBackImgHeight == 0) {
			::DeleteObject(hBackImg);
			hBackImg = NULL;
		}
	}
}

// 全ビューの初期化：ファイルオープン/クローズ時等に、ビューを初期化する
void EditDoc::InitAllView(void)
{
	nCommandExecNum = 0;	// コマンド実行回数
	
	// 2008.05.30 nasukoji	テキストの折り返し方法を初期化
	nTextWrapMethodCur = docType.GetDocumentAttribute().nTextWrapMethod;	// 折り返し方法
	bTextWrapMethodCurTemp = false;											// 一時設定適用中を解除
	blfCurTemp = false;
	bTabSpaceCurTemp = false;
	
	// 2009.08.28 nasukoji	「折り返さない」ならテキスト最大幅を算出、それ以外は変数をクリア
	if (nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
		layoutMgr.CalculateTextWidth();		// テキスト最大幅を算出する
	}else {
		layoutMgr.ClearLayoutLineWidth();	// 各行のレイアウト行長の記憶をクリアする
	}
	// EditWndに引越し
	pEditWnd->InitAllViews();
	
	return;
}

/*! ウィンドウの作成等

	@date 2001.09.29 genta マクロクラスを渡すように
	@date 2002.01.03 YAZAKI tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
*/
BOOL EditDoc::Create(EditWnd* pEditWnd)
{
	MY_RUNNINGTIMER(runningTimer, "EditDoc::Create");

	this->pEditWnd = pEditWnd;

	// Oct. 2, 2001 genta
	funcLookup.Init(GetDllShareData().common.macro.macroTable, &GetDllShareData().common);

	SetBackgroundImage();

	MY_TRACETIME(runningTimer, "End: PropSheet");

	return TRUE;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           設定                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	ファイル名の設定
	
	ファイル名を設定すると同時に，ウィンドウアイコンを適切に設定する．
	
	@param szFile [in] ファイルのパス名
	
	@author genta
	@date 2002.09.09
*/
void EditDoc::SetFilePathAndIcon(const TCHAR* szFile)
{
	TCHAR szWork[MAX_PATH];
	if (::GetLongFileName(szFile, szWork)) {
		szFile = szWork;
	}
	docFile.SetFilePath(szFile);
	docType.SetDocumentIcon();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           属性                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ドキュメントの文字コードを取得
EncodingType EditDoc::GetDocumentEncoding() const
{
	return docFile.GetCodeSet();
}

// ドキュメントのBOM付加を取得
bool EditDoc::GetDocumentBomExist() const
{
	return docFile.IsBomExist();
}

// ドキュメントの文字コードを設定
void EditDoc::SetDocumentEncoding(EncodingType eCharCode, bool bBom)
{
	if (!IsValidCodeType(eCharCode)) {
		return; // 無効な範囲を受け付けない
	}

	docFile.SetCodeSet(eCharCode, bBom);
}


void EditDoc::GetSaveInfo(SaveInfo* pSaveInfo) const
{
	pSaveInfo->filePath		= docFile.GetFilePath();
	pSaveInfo->eCharCode	= docFile.GetCodeSet();
	pSaveInfo->bBomExist	= docFile.IsBomExist();
	pSaveInfo->bChgCodeSet	= docFile.IsChgCodeSet();
	pSaveInfo->eol			= docEditor.newLineCode; // 編集時改行コードを保存時改行コードとして設定
}


// 編集ファイル情報を格納
void EditDoc::GetEditInfo(
	EditInfo* pfi	// [out]
	) const
{
	// ファイルパス
	_tcscpy(pfi->szPath, docFile.GetFilePath());

	// 表示域
	pfi->nViewTopLine = pEditWnd->GetActiveView().GetTextArea().GetViewTopLine();	// 表示域の一番上の行(0開始)
	pfi->nViewLeftCol = pEditWnd->GetActiveView().GetTextArea().GetViewLeftCol();	// 表示域の一番左の桁(0開始)

	// キャレット位置
	pfi->ptCursor.Set(pEditWnd->GetActiveView().GetCaret().GetCaretLogicPos());

	// 各種状態
	pfi->bIsModified = docEditor.IsModified();			// 変更フラグ
	pfi->nCharCode = docFile.GetCodeSet();				// 文字コード種別
	pfi->bBom = GetDocumentBomExist();
	pfi->nTypeId = docType.GetDocumentAttribute().id;

	// GREPモード
	pfi->bIsGrep = EditApp::getInstance().pGrepAgent->bGrepMode;
	wcscpy(pfi->szGrepKey, AppMode::getInstance().szGrepKey);

	// デバッグモニタ (アウトプットウィンドウ) モード
	pfi->bIsDebug = AppMode::getInstance().IsDebugMode();
}


/*! @brief 指定コマンドによる書き換えが禁止されているかどうか

	@retval true  禁止
	@retval false 許可

	@date 2000.08.14 genta 新規作成
	@date 2014.07.27 novice 編集禁止の場合の検索方法変更
*/
bool EditDoc::IsModificationForbidden(EFunctionCode nCommand) const
{
	// 編集可能の場合
	if (IsEditable()) {
		return false; // 常に書き換え許可
	}
	
	//	編集禁止の場合(バイナリサーチ)
	{
		int lbound = 0;
		int ubound = _countof(EIsModificationForbidden) - 1;

		while (lbound <= ubound) {
			int mid = (lbound + ubound) / 2;
			if (nCommand < EIsModificationForbidden[mid]) {
				ubound = mid - 1;
			}else if (nCommand > EIsModificationForbidden[mid]) {
				lbound = mid + 1;
			}else {
				return true;
			}
		}
	}

	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           状態                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! @brief このウィンドウで新しいファイルを開けるか

	新しいウィンドウを開かずに現在のウィンドウを再利用できるかどうかのテストを行う．
	変更済み，ファイルを開いている，Grepウィンドウ，アウトプットウィンドウの場合には
	再利用不可．

	@author Moca
	@date 2005.06.24 Moca
*/
bool EditDoc::IsAcceptLoad() const
{
	if (
		docEditor.IsModified()
		|| docFile.GetFilePathClass().IsValidPath()
		|| EditApp::getInstance().pGrepAgent->bGrepMode
		|| AppMode::getInstance().IsDebugMode()
	) {
		return false;
	}
	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         イベント                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! コマンドコードによる処理振り分け

	@param[in] nCommand MAKELONG(コマンドコード，送信元識別子)

	@date 2006.05.19 genta 上位16bitに送信元の識別子が入るように変更
	@date 2007.06.20 ryoji グループ内で巡回するように変更
*/
bool EditDoc::HandleCommand(EFunctionCode nCommand)
{
	// May. 19, 2006 genta 上位16bitに送信元の識別子が入るように変更したので
	// 下位16ビットのみを取り出す
	switch (LOWORD(nCommand)) {
	case F_PREVWINDOW:	// 前のウィンドウ
		{
			int nPane = pEditWnd->splitterWnd.GetPrevPane();
			if (nPane != -1) {
				pEditWnd->SetActivePane(nPane);
			}else {
				ControlTray::ActiveNextWindow(pEditWnd->GetHwnd());
			}
		}
		return TRUE;
	case F_NEXTWINDOW:	// 次のウィンドウ
		{
			int nPane = pEditWnd->splitterWnd.GetNextPane();
			if (nPane != -1) {
				pEditWnd->SetActivePane(nPane);
			}else {
				ControlTray::ActivePrevWindow(pEditWnd->GetHwnd());
			}
		}
		return TRUE;
	case F_CHG_CHARSET:
		return pEditWnd->GetActiveView().GetCommander().HandleCommand(nCommand, true, (LPARAM)CODE_NONE, 0, 0, 0);

	default:
		return pEditWnd->GetActiveView().GetCommander().HandleCommand(nCommand, true, 0, 0, 0, 0);
	}
}

/*!	タイプ別設定の適用を変更
	@date 2011.12.15 ViewCommander::Command_Type_Listから移動
*/
void EditDoc::OnChangeType()
{
	// 設定変更を反映させる
	bTextWrapMethodCurTemp = false;	// 折り返し方法の一時設定適用中を解除	// 2008.06.08 ryoji
	blfCurTemp = false;
	bTabSpaceCurTemp = false;
	OnChangeSetting();

	// 新規で無変更ならデフォルト文字コードを適用する	// 2011.01.24 ryoji
	if (!docFile.GetFilePathClass().IsValidPath()) {
		if (!docEditor.IsModified() && docLineMgr.GetLineCount() == 0) {
			const TypeConfig& types = docType.GetDocumentAttribute();
			docFile.SetCodeSet(types.encoding.eDefaultCodetype, types.encoding.bDefaultBom);
			docEditor.newLineCode = types.encoding.eDefaultEoltype;
			pEditWnd->GetActiveView().GetCaret().ShowCaretPosInfo();
		}
	}

	// 2006.09.01 ryoji タイプ変更後自動実行マクロを実行する
	RunAutoMacro(GetDllShareData().common.macro.nMacroOnTypeChanged);
}

/*! ビューに設定変更を反映させる
	@param [in] bDoLayout レイアウト情報の再作成

	@date 2004.06.09 Moca レイアウト再構築中にProgress Barを表示する．
	@date 2008.05.30 nasukoji	テキストの折り返し方法の変更処理を追加
	@date 2013.04.22 novice レイアウト情報の再作成を設定できるようにした
*/
void EditDoc::OnChangeSetting(
	bool bDoLayout
	)
{
	HWND hwndProgress = NULL;
	if (pEditWnd) {
		hwndProgress = pEditWnd->statusBar.GetProgressHwnd();
		// Status Barが表示されていないときはhwndProgressBar == NULL
	}

	if (hwndProgress) {
		::ShowWindow(hwndProgress, SW_SHOW);
	}

	// ファイルの排他モード変更
	if (docFile.GetShareMode() != GetDllShareData().common.file.nFileShareMode) {
		docFile.SetShareMode(GetDllShareData().common.file.nFileShareMode);

		// ファイルの排他ロック解除
		docFileOperation.DoFileUnlock();

		// ファイル書込可能のチェック処理
		bool bOld = docLocker.IsDocWritable();
		docLocker.CheckWritable(bOld && !AppMode::getInstance().IsViewMode());	// 書込可から不可に遷移したときだけメッセージを出す（出過ぎると鬱陶しいよね？）
		if (bOld != docLocker.IsDocWritable()) {
			pEditWnd->UpdateCaption();
		}

		// ファイルの排他ロック
		if (docLocker.IsDocWritable()) {
			docFileOperation.DoFileLock();
		}
	}

	// 共有データ構造体のアドレスを返す
	FileNameManager::getInstance().TransformFileName_MakeCache();

	PointEx* posSaveAry = nullptr;
	if (pEditWnd->posSaveAry) {
		if (bDoLayout) {
			posSaveAry = pEditWnd->posSaveAry;
			pEditWnd->posSaveAry = nullptr;
		}
	}else {
		if (pEditWnd->pPrintPreview) {
			// 一時的に設定を戻す
			SelectCharWidthCache(CharWidthFontMode::Edit, CharWidthCacheMode::Neutral);
		}
		if (bDoLayout) {
			posSaveAry = pEditWnd->SavePhysPosOfAllView();
		}
	}

	// 旧情報の保持
	const int nTypeId = docType.GetDocumentAttribute().id;
	const bool bFontTypeOld = docType.GetDocumentAttribute().bUseTypeFont;
	int nFontPointSizeOld = nPointSizeOrg;
	if (bFontTypeOld) {
		nFontPointSizeOld = docType.GetDocumentAttribute().nPointSize;
	}
	const auto nTabSpaceOld = docType.GetDocumentAttribute().nTabSpace;

	// 文書種別
	docType.SetDocumentType(DocTypeManager().GetDocumentTypeOfPath(docFile.GetFilePath()), false);
	const TypeConfig& ref = docType.GetDocumentAttribute();

	// タイプ別設定の種類が変更されたら、一時適用を元に戻す
	if (nTypeId != ref.id) {
		blfCurTemp = false;
		if (bDoLayout) {
			bTextWrapMethodCurTemp = false;
			bTabSpaceCurTemp = false;
		}
	}

	// フォントサイズの一時設定
	if (blfCurTemp) {
		if (bFontTypeOld != ref.bUseTypeFont) {
			blfCurTemp = false;
		}else if (nFontPointSizeOld != pEditWnd->GetFontPointSize(false)) {
			blfCurTemp = false; // フォント設定が変更された。元に戻す
		}else {
			// フォントの種類の変更に追随する
			int lfHeight = lfCur.lfHeight;
			lfCur = pEditWnd->GetLogfont(false);
			lfCur.lfHeight = lfHeight;
		}
	}

	// フォント更新
	pEditWnd->pViewFont->UpdateFont(&pEditWnd->GetLogfont());
	pEditWnd->pViewFontMiniMap->UpdateFont(&pEditWnd->GetLogfont());

	InitCharWidthCache( pEditWnd->pViewFontMiniMap->GetLogfont(), CharWidthFontMode::MiniMap );
	SelectCharWidthCache(CharWidthFontMode::Edit, pEditWnd->GetLogfontCacheMode());
	InitCharWidthCache(pEditWnd->GetLogfont());

	size_t nMaxLineKetas = ref.nMaxLineKetas;
	size_t nTabSpace = ref.nTabSpace;
	if (bDoLayout) {
		// 2008.06.07 nasukoji	折り返し方法の追加に対応
		// 折り返し方法の一時設定とタイプ別設定が一致したら一時設定適用中は解除
		if (nTextWrapMethodCur == ref.nTextWrapMethod) {
			if (nTextWrapMethodCur == TextWrappingMethod::SettingWidth
				&& layoutMgr.GetMaxLineKetas() != ref.nMaxLineKetas
			) {
				// 2013.05.29 折り返し幅が違うのでそのままにする
			}else if (bDoLayout) {
				bTextWrapMethodCurTemp = false;		// 一時設定適用中を解除
			}
		}
		// 一時設定適用中でなければ折り返し方法変更
		if (!bTextWrapMethodCurTemp) {
			nTextWrapMethodCur = ref.nTextWrapMethod;	// 折り返し方法
		}
		// 指定桁で折り返す：タイプ別設定を使用
		// 右端で折り返す：仮に現在の折り返し幅を使用
		// 上記以外：MAXLINEKETASを使用
		switch (nTextWrapMethodCur) {
		case TextWrappingMethod::NoWrapping:
			nMaxLineKetas = MAXLINEKETAS;
			break;
		case TextWrappingMethod::SettingWidth:
			if (bTextWrapMethodCurTemp) {
				// 2013.05.29 現在の一時適用の折り返し幅を使うように
				nMaxLineKetas = layoutMgr.GetMaxLineKetas();
			}
			break;
		case TextWrappingMethod::WindowWidth:
			nMaxLineKetas = layoutMgr.GetMaxLineKetas();	// 現在の折り返し幅
			break;
		}

		if (bTabSpaceCurTemp) {
			if (nTabSpaceOld != ref.nTabSpace) {
				// タイプ別設定が変更されたので一時適用解除
				bTabSpaceCurTemp = false;
			}else {
				// 一時適用継続
				nTabSpace = layoutMgr.GetTabSpace();
			}
		}
	}else {
		// レイアウトを再構築しないので元の設定を維持
		nMaxLineKetas = layoutMgr.GetMaxLineKetas();	// 現在の折り返し幅
		nTabSpace = layoutMgr.GetTabSpace();	// 現在のタブ幅
	}
	ProgressSubject* pOld = EditApp::getInstance().pVisualProgress->ProgressListener::Listen(&layoutMgr);
	layoutMgr.SetLayoutInfo(bDoLayout, ref, nTabSpace, nMaxLineKetas);
	EditApp::getInstance().pVisualProgress->ProgressListener::Listen(pOld);
	pEditWnd->ClearViewCaretPosInfo();
	
	// 2009.08.28 nasukoji	「折り返さない」ならテキスト最大幅を算出、それ以外は変数をクリア
	if (nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
		layoutMgr.CalculateTextWidth();		// テキスト最大幅を算出する
	}else {
		layoutMgr.ClearLayoutLineWidth();	// 各行のレイアウト行長の記憶をクリアする
	}
	// ビューに設定変更を反映させる
	int viewCount = pEditWnd->GetAllViewCount();
	for (int i=0; i<viewCount; ++i) {
		pEditWnd->GetView(i).OnChangeSetting();
	}
	if (posSaveAry) {
		pEditWnd->RestorePhysPosOfAllView(posSaveAry);
	}
	for (int i=0; i<viewCount; ++i) {
		pEditWnd->GetView(i).AdjustScrollBars();	// 2008.06.18 ryoji
	}
	if (hwndProgress) {
		::ShowWindow(hwndProgress, SW_HIDE);
	}
	if (pEditWnd->pPrintPreview) {
		// 設定を戻す
		SelectCharWidthCache(CharWidthFontMode::Print, CharWidthCacheMode::Local);
	}
}

/*! ファイルを閉じるときのMRU登録 & 保存確認 ＆ 保存実行

	@retval TRUE: 終了して良い / FALSE: 終了しない
*/
BOOL EditDoc::OnFileClose(bool bGrepNoConfirm)
{
	// クローズ事前処理
	CallbackResultType eBeforeCloseResult = NotifyBeforeClose();
	if (eBeforeCloseResult == CallbackResultType::Interrupt) {
		return FALSE;
	}
	
	// デバッグモニタモードのときは保存確認しない
	if (AppMode::getInstance().IsDebugMode()) {
		return TRUE;
	}

	// GREPモードで、かつ、「GREPモードで保存確認するか」がOFFだったら、保存確認しない
	// 2011.11.13 GrepモードでGrep直後は"未編集"状態になっているが保存確認が必要
	if (EditApp::getInstance().pGrepAgent->bGrepMode) {
		if (bGrepNoConfirm) { // Grepで保存確認しないモード
			return TRUE;
		}
		if (!GetDllShareData().common.search.bGrepExitConfirm) {
			return TRUE;
		}
	}else {
		// テキスト,文字コードセットが変更されていない場合は保存確認しない
		if (!docEditor.IsModified() && !docFile.IsChgCodeSet()) {
			return TRUE;
		}
	}

	// -- -- 保存確認 -- -- //
	TCHAR szGrepTitle[90];
	LPCTSTR pszTitle = docFile.GetFilePathClass().IsValidPath() ? docFile.GetFilePath() : NULL;
	if (EditApp::getInstance().pGrepAgent->bGrepMode) {
		LPCWSTR		pszGrepKey = AppMode::getInstance().szGrepKey;
		int			nLen = (int)wcslen(pszGrepKey);
		NativeW	memDes;
		LimitStringLengthW(pszGrepKey , nLen, 64, memDes);
		auto_sprintf(szGrepTitle, LS(STR_TITLE_GREP),
			memDes.GetStringPtr(),
			(nLen > memDes.GetStringLength()) ? _T("...") : _T("")
		);
		pszTitle = szGrepTitle;
	}
	if (!pszTitle) {
		const EditNode* node = AppNodeManager::getInstance().GetEditNode(EditWnd::getInstance().GetHwnd());
		auto_sprintf(szGrepTitle, _T("%s%d"), LS(STR_NO_TITLE1), node->nId);	// (無題)
		pszTitle = szGrepTitle;
	}
	// ウィンドウをアクティブにする
	HWND hwndMainFrame = EditWnd::getInstance().GetHwnd();
	ActivateFrameWindow(hwndMainFrame);
	int nBool;
	if (AppMode::getInstance().IsViewMode()) {	// ビューモード
		ConfirmBeep();
		int nRet = ::MYMESSAGEBOX(
			hwndMainFrame,
			MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
			GSTR_APPNAME,
			LS(STR_ERR_DLGEDITDOC30),
			pszTitle
		);
		switch (nRet) {
		case IDYES:
			nBool = docFileOperation.FileSaveAs();	// 2006.12.30 ryoji
			return nBool;
		case IDNO:
			return TRUE;
		case IDCANCEL:
		default:
			return FALSE;
		}
	}else {
		ConfirmBeep();
		int nRet;
		if (docFile.IsChgCodeSet()) {
			nRet = ::MYMESSAGEBOX(
				hwndMainFrame,
				MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
				GSTR_APPNAME,
				LS(STR_CHANGE_CHARSET),
				pszTitle);
		}else {
			nRet = ::MYMESSAGEBOX(
				hwndMainFrame,
				MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
				GSTR_APPNAME,
				LS(STR_ERR_DLGEDITDOC31),
				pszTitle
			);
		}
		switch (nRet) {
		case IDYES:
			if (docFile.GetFilePathClass().IsValidPath()) {
				nBool = docFileOperation.FileSave();	// 2006.12.30 ryoji
			}else {
				nBool = docFileOperation.FileSaveAs();	// 2006.12.30 ryoji
			}
			return nBool;
		case IDNO:
			return TRUE;
		case IDCANCEL:
		default:
			if (docFile.IsChgCodeSet()) {
				docFile.CancelChgCodeSet();	// 文字コードセットの変更をキャンセルする
				this->pEditWnd->GetActiveView().GetCaret().ShowCaretPosInfo();	// ステータス表示
			}
			return FALSE;
		}
	}
}

/*!	@brief マクロ自動実行

	@param type [in] 自動実行マクロ番号
	@return

	@author ryoji
	@date 2006.09.01 ryoji 作成
	@date 2007.07.20 genta HandleCommandに追加情報を渡す．
		自動実行マクロで発行したコマンドはキーマクロに保存しない
*/
void EditDoc::RunAutoMacro(int idx, LPCTSTR pszSaveFilePath)
{
	// 開ファイル／タイプ変更時はアウトラインを再解析する
	if (!pszSaveFilePath) {
		pEditWnd->dlgFuncList.Refresh();
	}

	static bool bRunning = false;
	if (bRunning) {
		return;	// 再入り実行はしない
	}
	bRunning = true;
	if (EditApp::getInstance().pSMacroMgr->IsEnabled(idx)) {
		if (!(::GetAsyncKeyState(VK_SHIFT) & 0x8000)) {	// Shift キーが押されていなければ実行
			if (pszSaveFilePath)
				docFile.SetSaveFilePath(pszSaveFilePath);
			// 2007.07.20 genta 自動実行マクロで発行したコマンドはキーマクロに保存しない
			HandleCommand((EFunctionCode)((F_USERMACRO_0 + idx) | FA_NONRECORD));
			docFile.SetSaveFilePath(_T(""));
		}
	}
	bRunning = false;
}

// (無題)の時のカレントディレクトリを設定する
void EditDoc::SetCurDirNotitle()
{
	if (docFile.GetFilePathClass().IsValidPath()) {
		return; // ファイルがあるときは何もしない
	}
	EOpenDialogDir eOpenDialogDir = GetDllShareData().common.edit.eOpenDialogDir;
	TCHAR szSelDir[_MAX_PATH];
	const TCHAR* pszDir = NULL;
	if (eOpenDialogDir == OPENDIALOGDIR_MRU) {
		const MruFolder mru;
		std::vector<LPCTSTR> vMRU = mru.GetPathList();
		int nCount = mru.Length();
		for (int i=0; i<nCount ; ++i) {
			DWORD attr = ::GetFileAttributes( vMRU[i] );
			if ((attr != -1) && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
				pszDir = vMRU[i];
				break;
			}
		}
	}else if (eOpenDialogDir == OPENDIALOGDIR_SEL) {
		FileNameManager::ExpandMetaToFolder( GetDllShareData().common.edit.openDialogSelDir, szSelDir, _countof(szSelDir) );
		pszDir = szSelDir;
	}
	if (pszDir) {
		::SetCurrentDirectory( pszDir );
	}
}
