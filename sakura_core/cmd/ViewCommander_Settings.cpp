/*!	@file
@brief ViewCommanderクラスのコマンド(設定系)関数群

	2012/12/15	ViewCommander.cpp,ViewCommander_New.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2003, MIK
	Copyright (C) 2005, genta, aroka
	Copyright (C) 2006, genta, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "typeprop/DlgTypeList.h"
#include "dlg/DlgFavorite.h"	// 履歴の管理	//@@@ 2003.04.08 MIK
#include "EditApp.h"
#include "util/shell.h"
#include "PropertyManager.h"
#include "util/window.h"


/*! ツールバーの表示/非表示

	@date 2006.12.19 ryoji 表示切替は EditWnd::LayoutToolBar(), EditWnd::EndLayoutBars() で行うように変更
*/
void ViewCommander::Command_SHOWTOOLBAR(void)
{
	auto& editWnd = GetEditWindow();	// Sep. 10, 2002 genta

	GetDllShareData().common.window.bDispToolBar = (editWnd.m_toolbar.GetToolbarHwnd() == NULL);	// ツールバー表示
	editWnd.LayoutToolBar();
	editWnd.EndLayoutBars();

	// 全ウィンドウに変更を通知する。
	AppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)BarChangeNotifyType::Toolbar,
		(LPARAM)editWnd.GetHwnd(),
		editWnd.GetHwnd()
	);
}


/*! ファンクションキーの表示/非表示

	@date 2006.12.19 ryoji 表示切替は EditWnd::LayoutFuncKey(), EditWnd::EndLayoutBars() で行うように変更
*/
void ViewCommander::Command_SHOWFUNCKEY(void)
{
	EditWnd& editWnd = GetEditWindow();	// Sep. 10, 2002 genta

	GetDllShareData().common.window.bDispFuncKeyWnd = !editWnd.m_funcKeyWnd.GetHwnd();	// ファンクションキー表示
	editWnd.LayoutFuncKey();
	editWnd.EndLayoutBars();

	// 全ウィンドウに変更を通知する。
	AppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)BarChangeNotifyType::FuncKey,
		(LPARAM)editWnd.GetHwnd(),
		editWnd.GetHwnd()
	);
}


/*! タブ(ウィンドウ)の表示/非表示

	@author MIK
	@date 2003.06.10 新規作成
	@date 2006.12.19 ryoji 表示切替は EditWnd::LayoutTabBar(), EditWnd::EndLayoutBars() で行うように変更
	@date 2007.06.20 ryoji グループIDリセット
 */
void ViewCommander::Command_SHOWTAB(void)
{
	auto& editWnd = GetEditWindow();	// Sep. 10, 2002 genta

	GetDllShareData().common.tabBar.bDispTabWnd = !editWnd.m_tabWnd.GetHwnd();	// タブバー表示
	editWnd.LayoutTabBar();
	editWnd.EndLayoutBars();

	// まとめるときは WS_EX_TOPMOST 状態を同期する	// 2007.05.18 ryoji
	if (GetDllShareData().common.tabBar.bDispTabWnd
		&& !GetDllShareData().common.tabBar.bDispTabWndMultiWin
	) {
		GetEditWindow().WindowTopMost(
			((DWORD)::GetWindowLongPtr(GetEditWindow().GetHwnd(), GWL_EXSTYLE) & WS_EX_TOPMOST)? 1: 2
		);
	}

	// 全ウィンドウに変更を通知する。
	AppNodeManager::getInstance().ResetGroupId();
	AppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)BarChangeNotifyType::Tab,
		(LPARAM)editWnd.GetHwnd(),
		editWnd.GetHwnd()
	);
}


/*! ステータスバーの表示/非表示

	@date 2006.12.19 ryoji 表示切替は EditWnd::LayoutStatusBar(), EditWnd::EndLayoutBars() で行うように変更
*/
void ViewCommander::Command_SHOWSTATUSBAR(void)
{
	auto& editWnd = GetEditWindow();	// Sep. 10, 2002 genta

	GetDllShareData().common.window.bDispStatusBar = !editWnd.m_statusBar.GetStatusHwnd();	// ステータスバー表示
	editWnd.LayoutStatusBar();
	editWnd.EndLayoutBars();

	// 全ウィンドウに変更を通知する。
	AppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)BarChangeNotifyType::StatusBar,
		(LPARAM)editWnd.GetHwnd(),
		editWnd.GetHwnd()
	);
}

/*! ミニマップの表示/非表示

	@date 2014.07.14 新規作成
*/
void ViewCommander::Command_SHOWMINIMAP(void)
{
	auto& editWnd = GetEditWindow();	//	Sep. 10, 2002 genta

	GetDllShareData().common.window.bDispMiniMap = (editWnd.GetMiniMap().GetHwnd() == NULL);
	editWnd.LayoutMiniMap();
	editWnd.EndLayoutBars();

	// 全ウィンドウに変更を通知する。
	AppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)BarChangeNotifyType::MiniMap,
		(LPARAM)editWnd.GetHwnd(),
		editWnd.GetHwnd()
	);
}


// タイプ別設定一覧
void ViewCommander::Command_TYPE_LIST(void)
{
	DlgTypeList dlgTypeList;
	DlgTypeList::Result result;
	result.documentType = GetDocument().m_docType.GetDocumentType();
	result.bTempChange = true;
	if (dlgTypeList.DoModal(G_AppInstance(), m_view.GetHwnd(), &result)) {
		// Nov. 29, 2000 genta
		// 一時的な設定適用機能を無理矢理追加
		if (result.bTempChange) {
			HandleCommand(F_CHANGETYPE, true, (LPARAM)result.documentType.GetIndex() + 1, 0, 0, 0);
		}else {
			// タイプ別設定
			EditApp::getInstance().OpenPropertySheetTypes(-1, result.documentType);
		}
	}
	return;
}


// タイプ別設定一時適用
void ViewCommander::Command_CHANGETYPE(int nTypePlusOne)
{
	TypeConfigNum type = TypeConfigNum(nTypePlusOne - 1);
	auto& doc = GetDocument();
	if (nTypePlusOne == 0) {
		type = doc.m_docType.GetDocumentType();
	}
	if (type.IsValidType() && type.GetIndex() < GetDllShareData().nTypesCount) {
		const TypeConfigMini* pConfig;
		DocTypeManager().GetTypeConfigMini(type, &pConfig);
		doc.m_docType.SetDocumentTypeIdx(pConfig->id, true);
		doc.m_docType.LockDocumentType();
		doc.OnChangeType();
	}
}


// タイプ別設定
void ViewCommander::Command_OPTION_TYPE(void)
{
	EditApp::getInstance().OpenPropertySheetTypes(-1, GetDocument().m_docType.GetDocumentType());
}


// 共通設定
void ViewCommander::Command_OPTION(void)
{
	// 設定プロパティシート テスト用
	EditApp::getInstance().OpenPropertySheet(-1);
}


// フォント設定
void ViewCommander::Command_FONT(void)
{
	HWND hwndFrame = GetMainWindow();

	// フォント設定ダイアログ
	auto& csView = GetDllShareData().common.view;
	LOGFONT lf = csView.lf;
	INT nPointSize;
#ifdef USE_UNFIXED_FONT
	bool bFixedFont = false;
#else
	bool bFixedFont = true;
#endif
	if (MySelectFont(&lf, &nPointSize, EditWnd::getInstance().m_splitterWnd.GetHwnd(), bFixedFont)) {
		csView.lf = lf;
		csView.nPointSize = nPointSize;

		if (csView.lf.lfPitchAndFamily & FIXED_PITCH) {
			csView.bFontIs_FixedPitch = true;	// 現在のフォントは固定幅フォントである
		}else {
			csView.bFontIs_FixedPitch = false;	// 現在のフォントは固定幅フォントでないアル
		}
		// 設定変更を反映させる
		// 全編集ウィンドウへメッセージをポストする
		AppNodeGroupHandle(0).PostMessageToAllEditors(
			MYWM_SAVEEDITSTATE,
			(WPARAM)0, (LPARAM)0, hwndFrame
		);
		AppNodeGroupHandle(0).PostMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0, (LPARAM)PM_CHANGESETTING_FONT, hwndFrame
		);

		// キャレットの表示
//		::HideCaret(GetHwnd());
//		::ShowCaret(GetHwnd());

//		// アクティブにする
//		// アクティブにする
//		ActivateFrameWindow(hwndFrame);
	}
	return;
}


/*! フォントサイズ設定
	@param fontSize フォントサイズ（1/10ポイント単位）
	@param shift フォントサイズを拡大or縮小するための変更量(fontSize=0のとき有効)

	@note TrueTypeのみサポート

	@date 2013.04.10 novice 新規作成
*/
void ViewCommander::Command_SETFONTSIZE(int fontSize, int shift, int mode)
{
	// The point sizes recommended by "The Windows Interface: An Application Design Guide", 1/10ポイント単位
	static const INT sizeTable[] = { 8*10, 9*10, 10*10, (INT)(10.5*10), 11*10, 12*10, 14*10, 16*10, 18*10, 20*10, 22*10, 24*10, 26*10, 28*10, 36*10, 48*10, 72*10 };
	auto& csView = GetDllShareData().common.view;
	const LOGFONT& lf = (mode == 0 ? csView.lf
		: GetEditWindow().GetLogfont(mode == 2));
	INT nPointSize;

	// TrueTypeのみ対応
	if (OUT_STROKE_PRECIS != lf.lfOutPrecision) {
		return;
	}

	if (!(0 <= mode && mode <= 2)) {
		return;
	}

	if (fontSize != 0) {
		// フォントサイズを直接選択する場合
		nPointSize = t_max(sizeTable[0], t_min(sizeTable[_countof(sizeTable) - 1], fontSize));
	}else if (shift != 0) {
		// 現在のフォントに対して、縮小or拡大したフォント選択する場合
		nPointSize = (mode == 0 ? csView.nPointSize
			: GetEditWindow().GetFontPointSize(mode == 2));

		// フォントの拡大or縮小するためのサイズ検索
		for (int i=0; i<_countof(sizeTable); ++i) {
			if (nPointSize <= sizeTable[i]) {
				int index = t_max(0, t_min((int)_countof(sizeTable) - 1, (int)(i + shift)));
				nPointSize = sizeTable[index];
				break;
			}
		}
	}else {
		// フォントサイズが変わらないので終了
		return;
	}
	// 新しいフォントサイズ設定
	int lfHeight = DpiPointsToPixels(-nPointSize, 10);
	int nTypeIndex = -1;
	auto& doc = GetDocument();
	if (mode == 0) {
		csView.lf.lfHeight = lfHeight;
		csView.nPointSize = nPointSize;
	}else if (mode == 1) {
		TypeConfigNum nDocType = doc.m_docType.GetDocumentType();
		auto type = std::make_unique<TypeConfig>();
		if (!DocTypeManager().GetTypeConfig(nDocType, *type)) {
			// 謎のエラー
			return;
		}
		type->bUseTypeFont = true; // タイプ別フォントを有効にする
		type->lf = lf;
		type->lf.lfHeight = lfHeight;
		type->nPointSize = nPointSize;
		DocTypeManager().SetTypeConfig(nDocType, *type);
		nTypeIndex = nDocType.GetIndex();
	}else if (mode == 2) {
		doc.m_blfCurTemp = true;
		doc.m_lfCur = lf;
		doc.m_lfCur.lfHeight = lfHeight;
		doc.m_nPointSizeCur = nPointSize;
		doc.m_nPointSizeOrg = GetEditWindow().GetFontPointSize(false);
	}

	HWND hwndFrame = GetMainWindow();

	// 設定変更を反映させる
	// 新たにタイプ別や一時設定が有効になってもフォント名は変わらないのでSIZEのみの変更通知をする
	if (mode == 0 || mode == 1) {
		// 全編集ウィンドウへメッセージをポストする
		AppNodeGroupHandle(0).PostMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)nTypeIndex,
			(LPARAM)PM_CHANGESETTING_FONTSIZE,
			hwndFrame
		);
	}else if (mode == 2) {
		// 自分だけ更新
		doc.OnChangeSetting(false);
	}
}


/*! 現在のウィンドウ幅で折り返し

	@date 2002.01.14 YAZAKI 現在のウィンドウ幅で折り返されているときは、最大値にするように
	@date 2002.04.08 YAZAKI ときどきウィンドウ幅で折り返されないことがあるバグ修正。
	@date 2005.08.14 genta ここでの設定は共通設定に反映しない．
	@date 2005.10.22 aroka 現在のウィンドウ幅→最大値→文書タイプの初期値 をトグルにする

	@note 変更する順序を変更したときはEditWnd::InitMenu()も変更すること
	@sa EditWnd::InitMenu()
*/
void ViewCommander::Command_WRAPWINDOWWIDTH(void)	// Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
{
	// Jan. 8, 2006 genta 判定処理をm_view.GetWrapMode()へ移動
	EditView::TOGGLE_WRAP_ACTION nWrapMode;
	LayoutInt newKetas;
	
	nWrapMode = m_view.GetWrapMode(&newKetas);
	auto& doc = GetDocument();
	doc.m_nTextWrapMethodCur = TextWrappingMethod::SettingWidth;
	doc.m_bTextWrapMethodCurTemp = (doc.m_nTextWrapMethodCur != m_view.m_pTypeData->nTextWrapMethod);
	if (nWrapMode == EditView::TGWRAP_NONE) {
		return;	// 折り返し桁は元のまま
	}

	GetEditWindow().ChangeLayoutParam(true, doc.m_layoutMgr.GetTabSpace(), newKetas);
	
	// Aug. 14, 2005 genta 共通設定へは反映させない
//	m_view.m_pTypeData->nMaxLineKetas = m_nViewColNum;

// 2013.12.30 左隅に移動しないように
//	m_view.GetTextArea().SetViewLeftCol(LayoutInt(0));		// 表示域の一番左の桁(0開始)

	// フォーカス移動時の再描画
	m_view.RedrawAll();
	return;
}


// from ViewCommander_New.cpp
/*!	履歴の管理(ダイアログ)
	@author	MIK
	@date	2003/04/07
*/
void ViewCommander::Command_Favorite(void)
{
	DlgFavorite	dlgFavorite;

	// ダイアログを表示する
	if (!dlgFavorite.DoModal(G_AppInstance(), m_view.GetHwnd(), (LPARAM)&GetDocument())) {
		return;
	}

	return;
}


/*!
	@brief テキストの折り返し方法を変更する
	
	@param[in] nWrapMethod 折り返し方法
		WRAP_NO_TEXT_WRAP  : 折り返さない
		WRAP_SETTING_WIDTH ; 指定桁で折り返す
		WRAP_WINDOW_WIDTH  ; 右端で折り返す
	
	@note ウィンドウが左右に分割されている場合、左側のウィンドウ幅を折り返し幅とする。
	
	@date 2008.05.31 nasukoji	新規作成
	@date 2009.08.28 nasukoji	テキストの最大幅を算出する
*/
void ViewCommander::Command_TEXTWRAPMETHOD(TextWrappingMethod nWrapMethod)
{
	auto& doc = GetDocument();

	// 現在の設定値と同じなら何もしない
	if (doc.m_nTextWrapMethodCur == nWrapMethod)
		return;

	int nWidth;

	switch (nWrapMethod) {
	case TextWrappingMethod::NoWrapping:		// 折り返さない
		nWidth = MAXLINEKETAS;	// アプリケーションの最大幅で折り返し
		break;

	case TextWrappingMethod::SettingWidth:	// 指定桁で折り返す
		nWidth = (Int)doc.m_docType.GetDocumentAttribute().nMaxLineKetas;
		break;

	case TextWrappingMethod::WindowWidth:		// 右端で折り返す
		// ウィンドウが左右に分割されている場合は左側のウィンドウ幅を使用する
		nWidth = (Int)m_view.ViewColNumToWrapColNum(GetEditWindow().GetView(0).GetTextArea().m_nViewColNum);
		break;

	default:
		return;	// 不正な値の時は何もしない
	}

	doc.m_nTextWrapMethodCur = nWrapMethod;	// 設定を記憶

	// 折り返し方法の一時設定適用／一時設定適用解除	// 2008.06.08 ryoji
	doc.m_bTextWrapMethodCurTemp = (doc.m_docType.GetDocumentAttribute().nTextWrapMethod != nWrapMethod);

	// 折り返し位置を変更
	GetEditWindow().ChangeLayoutParam(false, doc.m_layoutMgr.GetTabSpace(), (LayoutInt)nWidth);

	// 2009.08.28 nasukoji	「折り返さない」ならテキスト最大幅を算出、それ以外は変数をクリア
	if (doc.m_nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
		doc.m_layoutMgr.CalculateTextWidth();		// テキスト最大幅を算出する
		GetEditWindow().RedrawAllViews(nullptr);	// Scroll Barの更新が必要なので再表示を実行する
	}else {
		doc.m_layoutMgr.ClearLayoutLineWidth();		// 各行のレイアウト行長の記憶をクリアする
	}
}


/*!
	@brief 文字カウント方法を変更する
	
	@param[in] nMode 文字カウント方法
		SelectCountMode::Toggle : 文字カウント方法をトグル
		SelectCountMode::ByChar ; 文字数でカウント
		SelectCountMode::ByByte ; バイト数でカウント
*/
void ViewCommander::Command_SELECT_COUNT_MODE(int nMode)
{
	// 設定には保存せず、View毎に持つフラグを設定
	//BOOL* pbDispSelCountByByte = &GetDllShareData().common.statusBar.bDispSelCountByByte;
	auto& selectCountMode = GetEditWindow().m_nSelectCountMode;

	if (nMode == (int)SelectCountMode::Toggle) {
		// 文字数⇔バイト数トグル
		SelectCountMode nCurrentMode;
		if (selectCountMode == SelectCountMode::Toggle) {
			nCurrentMode = (GetDllShareData().common.statusBar.bDispSelCountByByte ?
								SelectCountMode::ByByte :
								SelectCountMode::ByChar);
		}else {
			nCurrentMode = selectCountMode;
		}
		selectCountMode = (nCurrentMode == SelectCountMode::ByByte ?
								SelectCountMode::ByChar :
								SelectCountMode::ByByte);
	}else if (nMode == (int)SelectCountMode::ByByte || nMode == (int)SelectCountMode::ByChar) {
		selectCountMode = (SelectCountMode)nMode;
	}
}


/*!	@brief 引用符の設定
	@date Jan. 29, 2005 genta 新規作成
*/
void ViewCommander::Command_SET_QUOTESTRING(const wchar_t* quotestr)
{
	if (!quotestr) {
		return;
	}

	auto& csFormat = GetDllShareData().common.format;
	wcsncpy(csFormat.szInyouKigou, quotestr, _countof(csFormat.szInyouKigou));
	csFormat.szInyouKigou[_countof(csFormat.szInyouKigou) - 1] = L'\0';
}

