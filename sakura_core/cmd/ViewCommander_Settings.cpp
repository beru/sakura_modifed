#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "typeprop/DlgTypeList.h"
#include "dlg/DlgFavorite.h"	// 履歴の管理
#include "EditApp.h"
#include "util/shell.h"
#include "PropertyManager.h"
#include "util/window.h"

// ViewCommanderクラスのコマンド(設定系)関数群

/*! ツールバーの表示/非表示 */
void ViewCommander::Command_ShowToolBar(void)
{
	auto& editWnd = GetEditWindow();

	GetDllShareData().common.window.bDispToolBar = (editWnd.toolbar.GetToolbarHwnd() == NULL);	// ツールバー表示
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


/*! ファンクションキーの表示/非表示 */
void ViewCommander::Command_ShowFuncKey(void)
{
	EditWnd& editWnd = GetEditWindow();

	GetDllShareData().common.window.bDispFuncKeyWnd = !editWnd.funcKeyWnd.GetHwnd();	// ファンクションキー表示
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


/*! タブ(ウィンドウ)の表示/非表示 */
void ViewCommander::Command_ShowTab(void)
{
	auto& editWnd = GetEditWindow();

	GetDllShareData().common.tabBar.bDispTabWnd = !editWnd.tabWnd.GetHwnd();	// タブバー表示
	editWnd.LayoutTabBar();
	editWnd.EndLayoutBars();

	// まとめるときは WS_EX_TOPMOST 状態を同期する
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


/*! ステータスバーの表示/非表示 */
void ViewCommander::Command_ShowStatusBar(void)
{
	auto& editWnd = GetEditWindow();

	GetDllShareData().common.window.bDispStatusBar = !editWnd.statusBar.GetStatusHwnd();	// ステータスバー表示
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

/*! ミニマップの表示/非表示 */
void ViewCommander::Command_ShowMiniMap(void)
{
	auto& editWnd = GetEditWindow();

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
void ViewCommander::Command_Type_List(void)
{
	DlgTypeList dlgTypeList;
	DlgTypeList::Result result;
	result.documentType = GetDocument().docType.GetDocumentType();
	result.bTempChange = true;
	if (dlgTypeList.DoModal(G_AppInstance(), view.GetHwnd(), &result)) {
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
void ViewCommander::Command_ChangeType(int nTypePlusOne)
{
	TypeConfigNum type = TypeConfigNum(nTypePlusOne - 1);
	auto& doc = GetDocument();
	if (nTypePlusOne == 0) {
		type = doc.docType.GetDocumentType();
	}
	if (type.IsValidType() && type.GetIndex() < GetDllShareData().nTypesCount) {
		const TypeConfigMini* pConfig;
		DocTypeManager().GetTypeConfigMini(type, &pConfig);
		doc.docType.SetDocumentTypeIdx(pConfig->id, true);
		doc.docType.LockDocumentType();
		doc.OnChangeType();
	}
}


// タイプ別設定
void ViewCommander::Command_Option_Type(void)
{
	EditApp::getInstance().OpenPropertySheetTypes(-1, GetDocument().docType.GetDocumentType());
}


// 共通設定
void ViewCommander::Command_Option(void)
{
	// 設定プロパティシート テスト用
	EditApp::getInstance().OpenPropertySheet(-1);
}


// フォント設定
void ViewCommander::Command_Font(void)
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
	if (MySelectFont(&lf, &nPointSize, EditWnd::getInstance().splitterWnd.GetHwnd(), bFixedFont)) {
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
*/
void ViewCommander::Command_SetFontSize(int fontSize, int shift, int mode)
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
		for (size_t i=0; i<_countof(sizeTable); ++i) {
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
		TypeConfigNum nDocType = doc.docType.GetDocumentType();
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
		doc.blfCurTemp = true;
		doc.lfCur = lf;
		doc.lfCur.lfHeight = lfHeight;
		doc.nPointSizeCur = nPointSize;
		doc.nPointSizeOrg = GetEditWindow().GetFontPointSize(false);
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

	@note 変更する順序を変更したときはEditWnd::InitMenu()も変更すること
	@sa EditWnd::InitMenu()
*/
void ViewCommander::Command_WrapWindowWidth(void)
{
	EditView::TOGGLE_WRAP_ACTION nWrapMode;
	int newKetas;
	
	nWrapMode = view.GetWrapMode(&newKetas);
	auto& doc = GetDocument();
	doc.nTextWrapMethodCur = TextWrappingMethod::SettingWidth;
	doc.bTextWrapMethodCurTemp = (doc.nTextWrapMethodCur != view.pTypeData->nTextWrapMethod);
	if (nWrapMode == EditView::TGWRAP_NONE) {
		return;	// 折り返し桁は元のまま
	}

	GetEditWindow().ChangeLayoutParam(true, doc.layoutMgr.GetTabSpace(), newKetas);
	
	// フォーカス移動時の再描画
	view.RedrawAll();
	return;
}


/*!	履歴の管理(ダイアログ) */
void ViewCommander::Command_Favorite(void)
{
	DlgFavorite	dlgFavorite;

	// ダイアログを表示する
	if (!dlgFavorite.DoModal(G_AppInstance(), view.GetHwnd(), (LPARAM)&GetDocument())) {
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
*/
void ViewCommander::Command_TextWrapMethod(TextWrappingMethod nWrapMethod)
{
	auto& doc = GetDocument();

	// 現在の設定値と同じなら何もしない
	if (doc.nTextWrapMethodCur == nWrapMethod)
		return;

	size_t nWidth;

	switch (nWrapMethod) {
	case TextWrappingMethod::NoWrapping:		// 折り返さない
		nWidth = MAXLINEKETAS;	// アプリケーションの最大幅で折り返し
		break;

	case TextWrappingMethod::SettingWidth:	// 指定桁で折り返す
		nWidth = doc.docType.GetDocumentAttribute().nMaxLineKetas;
		break;

	case TextWrappingMethod::WindowWidth:		// 右端で折り返す
		// ウィンドウが左右に分割されている場合は左側のウィンドウ幅を使用する
		nWidth = view.ViewColNumToWrapColNum(GetEditWindow().GetView(0).GetTextArea().nViewColNum);
		break;

	default:
		return;	// 不正な値の時は何もしない
	}

	doc.nTextWrapMethodCur = nWrapMethod;	// 設定を記憶

	// 折り返し方法の一時設定適用／一時設定適用解除
	doc.bTextWrapMethodCurTemp = (doc.docType.GetDocumentAttribute().nTextWrapMethod != nWrapMethod);

	// 折り返し位置を変更
	GetEditWindow().ChangeLayoutParam(false, doc.layoutMgr.GetTabSpace(), nWidth);

	// 「折り返さない」ならテキスト最大幅を算出、それ以外は変数をクリア
	if (doc.nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
		doc.layoutMgr.CalculateTextWidth();		// テキスト最大幅を算出する
		GetEditWindow().RedrawAllViews(nullptr);	// Scroll Barの更新が必要なので再表示を実行する
	}else {
		doc.layoutMgr.ClearLayoutLineWidth();		// 各行のレイアウト行長の記憶をクリアする
	}
}


/*!
	@brief 文字カウント方法を変更する
	
	@param[in] nMode 文字カウント方法
		SelectCountMode::Toggle : 文字カウント方法をトグル
		SelectCountMode::ByChar ; 文字数でカウント
		SelectCountMode::ByByte ; バイト数でカウント
*/
void ViewCommander::Command_Select_Count_Mode(int nMode)
{
	// 設定には保存せず、View毎に持つフラグを設定
	//BOOL* pbDispSelCountByByte = &GetDllShareData().common.statusBar.bDispSelCountByByte;
	auto& selectCountMode = GetEditWindow().nSelectCountMode;

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


/*!	@brief 引用符の設定 */
void ViewCommander::Command_Set_QuoteString(const wchar_t* quotestr)
{
	if (!quotestr) {
		return;
	}

	auto& csFormat = GetDllShareData().common.format;
	wcsncpy(csFormat.szInyouKigou, quotestr, _countof(csFormat.szInyouKigou));
	csFormat.szInyouKigou[_countof(csFormat.szInyouKigou) - 1] = L'\0';
}

