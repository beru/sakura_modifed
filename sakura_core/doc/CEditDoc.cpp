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
#include "doc/CEditDoc.h"
#include "doc/logic/CDocLine.h" /// 2002/2/3 aroka
#include "doc/layout/CLayout.h"	// 2007.08.22 ryoji 追加
#include "docplus/CModifyManager.h"
#include "_main/global.h"
#include "_main/CAppMode.h"
#include "_main/CControlTray.h"
#include "_main/CNormalProcess.h"
#include "window/CEditWnd.h"
#include "_os/CClipboard.h"
#include "CCodeChecker.h"
#include "CEditApp.h"
#include "CGrepAgent.h"
#include "print/CPrintPreview.h"
#include "uiparts/CVisualProgress.h"
#include "charset/CCodeMediator.h"
#include "charset/charcode.h"
#include "debug/CRunningTimer.h"
#include "env/CSakuraEnvironment.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "func/Funccode.h"
#include "mem/CMemoryIterator.h"	// 2007.08.22 ryoji 追加
#include "outline/CFuncInfoArr.h" /// 2002/2/3 aroka
#include "macro/CSMacroMgr.h"
#include "recent/CMRUFolder.h"
#include "util/file.h"
#include "util/format.h"
#include "util/module.h"
#include "util/other_util.h"
#include "util/string_ex2.h"
#include "util/window.h"
#include "sakura_rc.h"

#define IDT_ROLLMOUSE	1

//! 編集禁止コマンド
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
		m_pEditWnd はコンストラクタ内では使用しないこと．

	@date 2000.05.12 genta 初期化方法変更
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、Processにひとつあるのみ。
	@date 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	@date 2004.06.21 novice タグジャンプ機能追加
*/
EditDoc::EditDoc(EditApp* pApp)
	:
	m_docFile(this),					// warning C4355: 'this' : ベース メンバー初期化子リストで使用されました。
	m_docFileOperation(this),			// warning C4355: 'this' : ベース メンバー初期化子リストで使用されました。
	m_docEditor(this),					// warning C4355: 'this' : ベース メンバー初期化子リストで使用されました。
	m_docType(this),					// warning C4355: 'this' : ベース メンバー初期化子リストで使用されました。
	m_docOutline(this),				// warning C4355: 'this' : ベース メンバー初期化子リストで使用されました。
	m_nCommandExecNum(0),				// コマンド実行回数
	m_hBackImg(NULL)
{
	MY_RUNNINGTIMER(cRunningTimer, "EditDoc::EditDoc");
	
	// レイアウト管理情報の初期化
	m_layoutMgr.Create(this, &m_docLineMgr);
	
	// レイアウト情報の変更
	// 2008.06.07 nasukoji	折り返し方法の追加に対応
	// 「指定桁で折り返す」以外の時は折り返し幅をMAXLINEKETASで初期化する
	// 「右端で折り返す」は、この後のOnSize()で再設定される
	const TypeConfig& ref = m_docType.GetDocumentAttribute();
	LayoutInt nMaxLineKetas = ref.m_nMaxLineKetas;
	if (ref.m_nTextWrapMethod != TextWrappingMethod::SettingWidth) {
		nMaxLineKetas = MAXLINEKETAS;
	}
	m_layoutMgr.SetLayoutInfo(true, ref, ref.m_nTabSpace, nMaxLineKetas);

	// 自動保存の設定	// Aug, 21, 2000 genta
	m_autoSaveAgent.ReloadAutoSaveParam();

	//$$ ModifyManager インスタンスを生成
	ModifyManager::getInstance();

	//$$ CodeChecker インスタンスを生成
	CodeChecker::getInstance();

	// 2008.06.07 nasukoji	テキストの折り返し方法を初期化
	m_nTextWrapMethodCur = m_docType.GetDocumentAttribute().m_nTextWrapMethod;	// 折り返し方法
	m_bTextWrapMethodCurTemp = false;									// 一時設定適用中を解除
	m_blfCurTemp = false;
	m_nPointSizeCur = -1;
	m_nPointSizeOrg = -1;
	m_bTabSpaceCurTemp = false;

	// 文字コード種別を初期化
	m_docFile.SetCodeSet(ref.m_encoding.m_eDefaultCodetype, ref.m_encoding.m_bDefaultBom);
	m_docEditor.m_newLineCode = ref.m_encoding.m_eDefaultEoltype;

	// 排他制御オプションを初期化
	m_docFile.SetShareMode(GetDllShareData().m_common.m_file.m_nFileShareMode);

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
	if (m_hBackImg) {
		::DeleteObject(m_hBackImg);
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        生成と破棄                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void EditDoc::Clear()
{
	// ファイルの排他ロック解除
	m_docFileOperation.DoFileUnlock();

	// 書込み禁止のクリア
	m_docLocker.Clear();

	// アンドゥ・リドゥバッファのクリア
	m_docEditor.m_opeBuf.ClearAll();

	// テキストデータのクリア
	m_docLineMgr.DeleteAllLine();

	// ファイルパスとアイコンのクリア
	SetFilePathAndIcon(_T(""));

	// ファイルのタイムスタンプのクリア
	m_docFile.ClearFileTime();

	// 「基本」のタイプ別設定を適用
	m_docType.SetDocumentType(DocTypeManager().GetDocumentTypeOfPath(m_docFile.GetFilePath()), true);
	m_blfCurTemp = false;
	m_pEditWnd->m_pViewFontMiniMap->UpdateFont(&m_pEditWnd->GetLogfont());
	InitCharWidthCache( m_pEditWnd->m_pViewFontMiniMap->GetLogfont(), CharWidthFontMode::MiniMap );
	SelectCharWidthCache(CharWidthFontMode::Edit, m_pEditWnd->GetLogfontCacheMode());
	InitCharWidthCache(m_pEditWnd->GetLogfont());
	m_pEditWnd->m_pViewFont->UpdateFont(&m_pEditWnd->GetLogfont());

	// 2008.06.07 nasukoji	折り返し方法の追加に対応
	const TypeConfig& ref = m_docType.GetDocumentAttribute();
	LayoutInt nMaxLineKetas = ref.m_nMaxLineKetas;
	if (ref.m_nTextWrapMethod != TextWrappingMethod::SettingWidth) {
		nMaxLineKetas = MAXLINEKETAS;
	}
	m_layoutMgr.SetLayoutInfo(true, ref, ref.m_nTabSpace, nMaxLineKetas);
	m_pEditWnd->ClearViewCaretPosInfo();
}

// 既存データのクリア
void EditDoc::InitDoc()
{
	AppMode::getInstance()->SetViewMode(false);	// ビューモード $$ 今後OnClearDocを用意したい
	AppMode::getInstance()->m_szGrepKey[0] = 0;	//$$

	EditApp::getInstance()->m_pGrepAgent->m_bGrepMode = false;	// Grepモード	//$$同上
	m_autoReloadAgent.m_eWatchUpdate = WU_QUERY; // Dec. 4, 2002 genta 更新監視方法 $$

	// 2005.06.24 Moca バグ修正
	// アウトプットウィンドウで「閉じて(無題)」を行ってもアウトプットウィンドウのまま
	if (AppMode::getInstance()->IsDebugMode()) {
		AppMode::getInstance()->SetDebugModeOFF();
	}

	// Sep. 10, 2002 genta
	// アイコン設定はファイル名設定と一体化のためここからは削除

	Clear();

	// 変更フラグ
	m_docEditor.SetModified(false, false);	// Jan. 22, 2002 genta

	// 文字コード種別
	const TypeConfig& ref = m_docType.GetDocumentAttribute();
	m_docFile.SetCodeSet(ref.m_encoding.m_eDefaultCodetype, ref.m_encoding.m_bDefaultBom);
	m_docEditor.m_newLineCode = ref.m_encoding.m_eDefaultEoltype;

	// Oct. 2, 2005 genta 挿入モード
	m_docEditor.SetInsMode(GetDllShareData().m_common.m_general.m_bIsINSMode);

	m_cookie.DeleteAll(L"document");
}

void EditDoc::SetBackgroundImage()
{
	FilePath path = m_docType.GetDocumentAttribute().m_szBackImgPath.c_str();
	if (m_hBackImg) {
		::DeleteObject(m_hBackImg);
		m_hBackImg = NULL;
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
			IPicture* iPicture = NULL;
			IStream* iStream = NULL;
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
						m_nBackImgWidth = m_nBackImgHeight = 1;
						m_hBackImg = (HBITMAP)::CopyImage(hBitmap, IMAGE_BITMAP, 0, 0, 0);
					}
				}
			}
			if (iStream)  iStream->Release();
			if (iPicture) iPicture->Release();
		}
	}else {
		m_hBackImg = (HBITMAP)::LoadImage(NULL, path.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	}
	if (m_hBackImg) {
		BITMAP bmp;
		GetObject(m_hBackImg, sizeof(BITMAP), &bmp);
		m_nBackImgWidth  = bmp.bmWidth;
		m_nBackImgHeight = bmp.bmHeight;
		if (m_nBackImgWidth == 0 || m_nBackImgHeight == 0) {
			::DeleteObject(m_hBackImg);
			m_hBackImg = NULL;
		}
	}
}

// 全ビューの初期化：ファイルオープン/クローズ時等に、ビューを初期化する
void EditDoc::InitAllView(void)
{
	m_nCommandExecNum = 0;	// コマンド実行回数
	
	// 2008.05.30 nasukoji	テキストの折り返し方法を初期化
	m_nTextWrapMethodCur = m_docType.GetDocumentAttribute().m_nTextWrapMethod;	// 折り返し方法
	m_bTextWrapMethodCurTemp = false;											// 一時設定適用中を解除
	m_blfCurTemp = false;
	m_bTabSpaceCurTemp = false;
	
	// 2009.08.28 nasukoji	「折り返さない」ならテキスト最大幅を算出、それ以外は変数をクリア
	if (m_nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
		m_layoutMgr.CalculateTextWidth();		// テキスト最大幅を算出する
	}else {
		m_layoutMgr.ClearLayoutLineWidth();	// 各行のレイアウト行長の記憶をクリアする
	}
	// EditWndに引越し
	m_pEditWnd->InitAllViews();
	
	return;
}

/*! ウィンドウの作成等

	@date 2001.09.29 genta マクロクラスを渡すように
	@date 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
*/
BOOL EditDoc::Create(EditWnd* pEditWnd)
{
	MY_RUNNINGTIMER(cRunningTimer, "EditDoc::Create");

	m_pEditWnd = pEditWnd;

	// Oct. 2, 2001 genta
	m_funcLookup.Init(GetDllShareData().m_common.m_macro.m_macroTable, &GetDllShareData().m_common);

	SetBackgroundImage();

	MY_TRACETIME(cRunningTimer, "End: PropSheet");

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
	m_docFile.SetFilePath(szFile);
	m_docType.SetDocumentIcon();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           属性                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ドキュメントの文字コードを取得
ECodeType EditDoc::GetDocumentEncoding() const
{
	return m_docFile.GetCodeSet();
}

// ドキュメントのBOM付加を取得
bool EditDoc::GetDocumentBomExist() const
{
	return m_docFile.IsBomExist();
}

// ドキュメントの文字コードを設定
void EditDoc::SetDocumentEncoding(ECodeType eCharCode, bool bBom)
{
	if (!IsValidCodeType(eCharCode)) {
		return; // 無効な範囲を受け付けない
	}

	m_docFile.SetCodeSet(eCharCode, bBom);
}


void EditDoc::GetSaveInfo(SaveInfo* pSaveInfo) const
{
	pSaveInfo->filePath   = m_docFile.GetFilePath();
	pSaveInfo->eCharCode   = m_docFile.GetCodeSet();
	pSaveInfo->bBomExist   = m_docFile.IsBomExist();
	pSaveInfo->bChgCodeSet = m_docFile.IsChgCodeSet();
	pSaveInfo->eol        = m_docEditor.m_newLineCode; // 編集時改行コードを保存時改行コードとして設定
}


// 編集ファイル情報を格納
void EditDoc::GetEditInfo(
	EditInfo* pfi	// [out]
) const
{
	// ファイルパス
	_tcscpy(pfi->m_szPath, m_docFile.GetFilePath());

	// 表示域
	pfi->m_nViewTopLine = m_pEditWnd->GetActiveView().GetTextArea().GetViewTopLine();	// 表示域の一番上の行(0開始)
	pfi->m_nViewLeftCol = m_pEditWnd->GetActiveView().GetTextArea().GetViewLeftCol();	// 表示域の一番左の桁(0開始)

	// キャレット位置
	pfi->m_ptCursor.Set(m_pEditWnd->GetActiveView().GetCaret().GetCaretLogicPos());

	// 各種状態
	pfi->m_bIsModified = m_docEditor.IsModified();			// 変更フラグ
	pfi->m_nCharCode = m_docFile.GetCodeSet();				// 文字コード種別
	pfi->m_bBom = GetDocumentBomExist();
	pfi->m_nTypeId = m_docType.GetDocumentAttribute().m_id;

	// GREPモード
	pfi->m_bIsGrep = EditApp::getInstance()->m_pGrepAgent->m_bGrepMode;
	wcscpy(pfi->m_szGrepKey, AppMode::getInstance()->m_szGrepKey);

	// デバッグモニタ (アウトプットウィンドウ) モード
	pfi->m_bIsDebug = AppMode::getInstance()->IsDebugMode();
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
		m_docEditor.IsModified()
		|| m_docFile.GetFilePathClass().IsValidPath()
		|| EditApp::getInstance()->m_pGrepAgent->m_bGrepMode
		|| AppMode::getInstance()->IsDebugMode()
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
			int nPane = m_pEditWnd->m_splitterWnd.GetPrevPane();
			if (nPane != -1) {
				m_pEditWnd->SetActivePane(nPane);
			}else {
				ControlTray::ActiveNextWindow(m_pEditWnd->GetHwnd());
			}
		}
		return TRUE;
	case F_NEXTWINDOW:	// 次のウィンドウ
		{
			int nPane = m_pEditWnd->m_splitterWnd.GetNextPane();
			if (nPane != -1) {
				m_pEditWnd->SetActivePane(nPane);
			}else {
				ControlTray::ActivePrevWindow(m_pEditWnd->GetHwnd());
			}
		}
		return TRUE;
	case F_CHG_CHARSET:
		return m_pEditWnd->GetActiveView().GetCommander().HandleCommand(nCommand, true, (LPARAM)CODE_NONE, 0, 0, 0);

	default:
		return m_pEditWnd->GetActiveView().GetCommander().HandleCommand(nCommand, true, 0, 0, 0, 0);
	}
}

/*!	タイプ別設定の適用を変更
	@date 2011.12.15 CViewCommander::Command_TYPE_LISTから移動
*/
void EditDoc::OnChangeType()
{
	// 設定変更を反映させる
	m_bTextWrapMethodCurTemp = false;	// 折り返し方法の一時設定適用中を解除	// 2008.06.08 ryoji
	m_blfCurTemp = false;
	m_bTabSpaceCurTemp = false;
	OnChangeSetting();

	// 新規で無変更ならデフォルト文字コードを適用する	// 2011.01.24 ryoji
	if (!m_docFile.GetFilePathClass().IsValidPath()) {
		if (!m_docEditor.IsModified() && m_docLineMgr.GetLineCount() == 0) {
			const TypeConfig& types = m_docType.GetDocumentAttribute();
			m_docFile.SetCodeSet(types.m_encoding.m_eDefaultCodetype, types.m_encoding.m_bDefaultBom);
			m_docEditor.m_newLineCode = types.m_encoding.m_eDefaultEoltype;
			m_pEditWnd->GetActiveView().GetCaret().ShowCaretPosInfo();
		}
	}

	// 2006.09.01 ryoji タイプ変更後自動実行マクロを実行する
	RunAutoMacro(GetDllShareData().m_common.m_macro.m_nMacroOnTypeChanged);
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
	EditWnd* pEditWnd = m_pEditWnd;	// Sep. 10, 2002 genta

	if (pEditWnd) {
		hwndProgress = pEditWnd->m_statusBar.GetProgressHwnd();
		// Status Barが表示されていないときはm_hwndProgressBar == NULL
	}

	if (hwndProgress) {
		::ShowWindow(hwndProgress, SW_SHOW);
	}

	// ファイルの排他モード変更
	if (m_docFile.GetShareMode() != GetDllShareData().m_common.m_file.m_nFileShareMode) {
		m_docFile.SetShareMode(GetDllShareData().m_common.m_file.m_nFileShareMode);

		// ファイルの排他ロック解除
		m_docFileOperation.DoFileUnlock();

		// ファイル書込可能のチェック処理
		bool bOld = m_docLocker.IsDocWritable();
		m_docLocker.CheckWritable(bOld && !AppMode::getInstance()->IsViewMode());	// 書込可から不可に遷移したときだけメッセージを出す（出過ぎると鬱陶しいよね？）
		if (bOld != m_docLocker.IsDocWritable()) {
			pEditWnd->UpdateCaption();
		}

		// ファイルの排他ロック
		if (m_docLocker.IsDocWritable()) {
			m_docFileOperation.DoFileLock();
		}
	}

	// 共有データ構造体のアドレスを返す
	FileNameManager::getInstance()->TransformFileName_MakeCache();

	LogicPointEx* posSaveAry = NULL;
	if (m_pEditWnd->m_posSaveAry) {
		if (bDoLayout) {
			posSaveAry = m_pEditWnd->m_posSaveAry;
			m_pEditWnd->m_posSaveAry = NULL;
		}
	}else {
		if (m_pEditWnd->m_pPrintPreview) {
			// 一時的に設定を戻す
			SelectCharWidthCache(CharWidthFontMode::Edit, CharWidthCacheMode::Neutral);
		}
		if (bDoLayout) {
			posSaveAry = m_pEditWnd->SavePhysPosOfAllView();
		}
	}

	// 旧情報の保持
	const int nTypeId = m_docType.GetDocumentAttribute().m_id;
	const bool bFontTypeOld = m_docType.GetDocumentAttribute().m_bUseTypeFont;
	int nFontPointSizeOld = m_nPointSizeOrg;
	if (bFontTypeOld) {
		nFontPointSizeOld = m_docType.GetDocumentAttribute().m_nPointSize;
	}
	const KetaXInt nTabSpaceOld = m_docType.GetDocumentAttribute().m_nTabSpace;

	// 文書種別
	m_docType.SetDocumentType(DocTypeManager().GetDocumentTypeOfPath(m_docFile.GetFilePath()), false);
	const TypeConfig& ref = m_docType.GetDocumentAttribute();

	// タイプ別設定の種類が変更されたら、一時適用を元に戻す
	if (nTypeId != ref.m_id) {
		m_blfCurTemp = false;
		if (bDoLayout) {
			m_bTextWrapMethodCurTemp = false;
			m_bTabSpaceCurTemp = false;
		}
	}

	// フォントサイズの一時設定
	if (m_blfCurTemp) {
		if (bFontTypeOld != ref.m_bUseTypeFont) {
			m_blfCurTemp = false;
		}else if (nFontPointSizeOld != pEditWnd->GetFontPointSize(false)) {
			m_blfCurTemp = false; // フォント設定が変更された。元に戻す
		}else {
			// フォントの種類の変更に追随する
			int lfHeight = m_lfCur.lfHeight;
			m_lfCur = pEditWnd->GetLogfont(false);
			m_lfCur.lfHeight = lfHeight;
		}
	}

	// フォント更新
	m_pEditWnd->m_pViewFont->UpdateFont(&m_pEditWnd->GetLogfont());
	m_pEditWnd->m_pViewFontMiniMap->UpdateFont(&m_pEditWnd->GetLogfont());

	InitCharWidthCache( m_pEditWnd->m_pViewFontMiniMap->GetLogfont(), CharWidthFontMode::MiniMap );
	SelectCharWidthCache(CharWidthFontMode::Edit, m_pEditWnd->GetLogfontCacheMode());
	InitCharWidthCache(m_pEditWnd->GetLogfont());

	LayoutInt nMaxLineKetas = ref.m_nMaxLineKetas;
	LayoutInt nTabSpace = ref.m_nTabSpace;
	if (bDoLayout) {
		// 2008.06.07 nasukoji	折り返し方法の追加に対応
		// 折り返し方法の一時設定とタイプ別設定が一致したら一時設定適用中は解除
		if (m_nTextWrapMethodCur == ref.m_nTextWrapMethod) {
			if (m_nTextWrapMethodCur == TextWrappingMethod::SettingWidth
				&& m_layoutMgr.GetMaxLineKetas() != ref.m_nMaxLineKetas
			) {
				// 2013.05.29 折り返し幅が違うのでそのままにする
			}else if (bDoLayout) {
				m_bTextWrapMethodCurTemp = false;		// 一時設定適用中を解除
			}
		}
		// 一時設定適用中でなければ折り返し方法変更
		if (!m_bTextWrapMethodCurTemp) {
			m_nTextWrapMethodCur = ref.m_nTextWrapMethod;	// 折り返し方法
		}
		// 指定桁で折り返す：タイプ別設定を使用
		// 右端で折り返す：仮に現在の折り返し幅を使用
		// 上記以外：MAXLINEKETASを使用
		switch (m_nTextWrapMethodCur) {
		case TextWrappingMethod::NoWrapping:
			nMaxLineKetas = MAXLINEKETAS;
			break;
		case TextWrappingMethod::SettingWidth:
			if (m_bTextWrapMethodCurTemp) {
				// 2013.05.29 現在の一時適用の折り返し幅を使うように
				nMaxLineKetas = m_layoutMgr.GetMaxLineKetas();
			}
			break;
		case TextWrappingMethod::WindowWidth:
			nMaxLineKetas = m_layoutMgr.GetMaxLineKetas();	// 現在の折り返し幅
			break;
		}

		if (m_bTabSpaceCurTemp) {
			if (nTabSpaceOld != ref.m_nTabSpace) {
				// タイプ別設定が変更されたので一時適用解除
				m_bTabSpaceCurTemp = false;
			}else {
				// 一時適用継続
				nTabSpace = m_layoutMgr.GetTabSpace();
			}
		}
	}else {
		// レイアウトを再構築しないので元の設定を維持
		nMaxLineKetas = m_layoutMgr.GetMaxLineKetas();	// 現在の折り返し幅
		nTabSpace = m_layoutMgr.GetTabSpace();	// 現在のタブ幅
	}
	ProgressSubject* pOld = EditApp::getInstance()->m_pVisualProgress->ProgressListener::Listen(&m_layoutMgr);
	m_layoutMgr.SetLayoutInfo(bDoLayout, ref, nTabSpace, nMaxLineKetas);
	EditApp::getInstance()->m_pVisualProgress->ProgressListener::Listen(pOld);
	m_pEditWnd->ClearViewCaretPosInfo();
	
	// 2009.08.28 nasukoji	「折り返さない」ならテキスト最大幅を算出、それ以外は変数をクリア
	if (m_nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
		m_layoutMgr.CalculateTextWidth();		// テキスト最大幅を算出する
	}else {
		m_layoutMgr.ClearLayoutLineWidth();	// 各行のレイアウト行長の記憶をクリアする
	}
	// ビューに設定変更を反映させる
	int viewCount = m_pEditWnd->GetAllViewCount();
	for (int i=0; i<viewCount; ++i) {
		m_pEditWnd->GetView(i).OnChangeSetting();
	}
	if (posSaveAry) {
		m_pEditWnd->RestorePhysPosOfAllView(posSaveAry);
	}
	for (int i=0; i<viewCount; ++i) {
		m_pEditWnd->GetView(i).AdjustScrollBars();	// 2008.06.18 ryoji
	}
	if (hwndProgress) {
		::ShowWindow(hwndProgress, SW_HIDE);
	}
	if (m_pEditWnd->m_pPrintPreview) {
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
	if (AppMode::getInstance()->IsDebugMode()) {
		return TRUE;
	}

	// GREPモードで、かつ、「GREPモードで保存確認するか」がOFFだったら、保存確認しない
	// 2011.11.13 GrepモードでGrep直後は"未編集"状態になっているが保存確認が必要
	if (EditApp::getInstance()->m_pGrepAgent->m_bGrepMode) {
		if (bGrepNoConfirm) { // Grepで保存確認しないモード
			return TRUE;
		}
		if (!GetDllShareData().m_common.m_search.m_bGrepExitConfirm) {
			return TRUE;
		}
	}else {
		// テキスト,文字コードセットが変更されていない場合は保存確認しない
		if (!m_docEditor.IsModified() && !m_docFile.IsChgCodeSet()) {
			return TRUE;
		}
	}

	// -- -- 保存確認 -- -- //
	TCHAR szGrepTitle[90];
	LPCTSTR pszTitle = m_docFile.GetFilePathClass().IsValidPath() ? m_docFile.GetFilePath() : NULL;
	if (EditApp::getInstance()->m_pGrepAgent->m_bGrepMode) {
		LPCWSTR		pszGrepKey = AppMode::getInstance()->m_szGrepKey;
		int			nLen = (int)wcslen(pszGrepKey);
		NativeW	cmemDes;
		LimitStringLengthW(pszGrepKey , nLen, 64, cmemDes);
		auto_sprintf(szGrepTitle, LS(STR_TITLE_GREP),
			cmemDes.GetStringPtr(),
			(nLen > cmemDes.GetStringLength()) ? _T("...") : _T("")
		);
		pszTitle = szGrepTitle;
	}
	if (!pszTitle) {
		const EditNode* node = AppNodeManager::getInstance()->GetEditNode(EditWnd::getInstance()->GetHwnd());
		auto_sprintf(szGrepTitle, _T("%s%d"), LS(STR_NO_TITLE1), node->m_nId);	// (無題)
		pszTitle = szGrepTitle;
	}
	// ウィンドウをアクティブにする
	HWND hwndMainFrame = EditWnd::getInstance()->GetHwnd();
	ActivateFrameWindow(hwndMainFrame);
	int nBool;
	if (AppMode::getInstance()->IsViewMode()) {	// ビューモード
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
			nBool = m_docFileOperation.FileSaveAs();	// 2006.12.30 ryoji
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
		if (m_docFile.IsChgCodeSet()) {
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
			if (m_docFile.GetFilePathClass().IsValidPath()) {
				nBool = m_docFileOperation.FileSave();	// 2006.12.30 ryoji
			}else {
				nBool = m_docFileOperation.FileSaveAs();	// 2006.12.30 ryoji
			}
			return nBool;
		case IDNO:
			return TRUE;
		case IDCANCEL:
		default:
			if (m_docFile.IsChgCodeSet()) {
				m_docFile.CancelChgCodeSet();	// 文字コードセットの変更をキャンセルする
				this->m_pEditWnd->GetActiveView().GetCaret().ShowCaretPosInfo();	// ステータス表示
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
		m_pEditWnd->m_dlgFuncList.Refresh();
	}

	static bool bRunning = false;
	if (bRunning) {
		return;	// 再入り実行はしない
	}
	bRunning = true;
	if (EditApp::getInstance()->m_pSMacroMgr->IsEnabled(idx)) {
		if (!(::GetAsyncKeyState(VK_SHIFT) & 0x8000)) {	// Shift キーが押されていなければ実行
			if (pszSaveFilePath)
				m_docFile.SetSaveFilePath(pszSaveFilePath);
			// 2007.07.20 genta 自動実行マクロで発行したコマンドはキーマクロに保存しない
			HandleCommand((EFunctionCode)((F_USERMACRO_0 + idx) | FA_NONRECORD));
			m_docFile.SetSaveFilePath(_T(""));
		}
	}
	bRunning = false;
}

// (無題)の時のカレントディレクトリを設定する
void EditDoc::SetCurDirNotitle()
{
	if (m_docFile.GetFilePathClass().IsValidPath()) {
		return; // ファイルがあるときは何もしない
	}
	EOpenDialogDir eOpenDialogDir = GetDllShareData().m_common.m_edit.m_eOpenDialogDir;
	TCHAR szSelDir[_MAX_PATH];
	const TCHAR* pszDir = NULL;
	if (eOpenDialogDir == OPENDIALOGDIR_MRU) {
		const MRUFolder mru;
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
		FileNameManager::ExpandMetaToFolder( GetDllShareData().m_common.m_edit.m_OpenDialogSelDir, szSelDir, _countof(szSelDir) );
		pszDir = szSelDir;
	}
	if (pszDir) {
		::SetCurrentDirectory( pszDir );
	}
}
