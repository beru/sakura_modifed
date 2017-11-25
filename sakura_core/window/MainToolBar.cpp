#include "StdAfx.h"
#include "window/MainToolBar.h"
#include "window/EditWnd.h"
#include "EditApp.h"
#include "util/os.h"
#include "util/tchar_receive.h"
#include "util/window.h"
#include "uiparts/ImageListMgr.h"

MainToolBar::MainToolBar(EditWnd& owner)
	:
	owner(owner),
	hwndToolBar(NULL),
	hwndReBar(NULL),
	hwndSearchBox(NULL),
	hFontSearchBox(NULL),
	pIcons(nullptr)
{
}

void MainToolBar::Create(ImageListMgr* pIcons)
{
	this->pIcons = pIcons;
}

// 検索ボックスでの処理
void MainToolBar::ProcSearchBox(MSG *msg)
{
	if (msg->message == WM_KEYDOWN /* && ::GetParent(msg->hwnd) == hwndSearchBox */) {
		if (msg->wParam == VK_RETURN) {  // リターンキー
			// 検索キーワードを取得
			std::wstring strText;
			if (0 < GetSearchKey(strText)) {	// キー文字列がある
				if (strText.size() < _MAX_PATH) {
					// 検索キーを登録
					SearchKeywordManager().AddToSearchKeys(strText.c_str());
				}
				owner.GetActiveView().strCurSearchKey = strText;
				owner.GetActiveView().bCurSearchUpdate = true;
				owner.GetActiveView().ChangeCurRegexp();

				// 検索ボックスを更新	// 2010/6/6 Uchi
				AcceptSharedSearchKey();

				//::SetFocus(hWnd);	//先にフォーカスを移動しておかないとキャレットが消える
				owner.GetActiveView().SetFocus();

				// 検索開始時のカーソル位置登録条件を変更
				owner.GetActiveView().ptSrchStartPos_PHY = owner.GetActiveView().GetCaret().GetCaretLogicPos();

				// 次を検索
				owner.OnCommand((WORD)0 /*メニュー*/, (WORD)F_SEARCH_NEXT, (HWND)0);
			}
		}else if (msg->wParam == VK_TAB) {	// タブキー
			// フォーカスを移動
			::SetFocus(owner.GetHwnd() );
		}
	}
}

/*! サブクラス化したツールバーのウィンドウプロシージャ */
static WNDPROC g_pOldToolBarWndProc;	// ツールバーの本来のウィンドウプロシージャ

static LRESULT CALLBACK ToolBarWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	// WinXP Visual Style のときにツールバー上でのマウス左右ボタン同時押しで無応答になる
	//（マウスをキャプチャーしたまま放さない） 問題を回避するために右ボタンを無視する
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		return 0L;				// 右ボタンの UP/DOWN は本来のウィンドウプロシージャに渡さない

	case WM_DESTROY:
		// サブクラス化解除
		::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)g_pOldToolBarWndProc);
		break;
	}
	return ::CallWindowProc(g_pOldToolBarWndProc, hWnd, msg, wParam, lParam);
}


/* ツールバー作成 */
void MainToolBar::CreateToolBar(void)
{
	if (hwndToolBar)
		return;
	
	REBARBANDINFO	rbBand;
	int				nFlag;
	TBBUTTON		tbb;
	int				i;
	int				nIdx;
	LONG_PTR		lToolType;
	nFlag = 0;

	auto& csToolBar = GetDllShareData().common.toolBar;
	// Rebar ウィンドウの作成
	if (IsVisualStyle()) {	// ビジュアルスタイル有効
		hwndReBar = ::CreateWindowEx(
			WS_EX_TOOLWINDOW,
			REBARCLASSNAME, // レバーコントロール
			NULL,
			WS_CHILD/* | WS_VISIBLE*/ | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
			RBS_BANDBORDERS | CCS_NODIVIDER,
			0, 0, 0, 0,
			owner.GetHwnd(),
			NULL,
			EditApp::getInstance().GetAppInstance(),
			NULL
		);

		if (!hwndReBar) {
			TopWarningMessage(owner.GetHwnd(), LS(STR_ERR_DLGEDITWND04));
			return;
		}

		if (csToolBar.bToolBarIsFlat) {	// フラットツールバーにする／しない
			PreventVisualStyle(hwndReBar);	// ビジュアルスタイル非適用のフラットな Rebar にする
		}

		REBARINFO rbi = {0};
		rbi.cbSize = sizeof(rbi);
		Rebar_SetbarInfo(hwndReBar, &rbi);

		nFlag = CCS_NORESIZE | CCS_NODIVIDER | CCS_NOPARENTALIGN | TBSTYLE_FLAT;	// ツールバーへの追加スタイル
	}

	// ツールバーウィンドウの作成
	hwndToolBar = ::CreateWindowEx(
		0,
		TOOLBARCLASSNAME,
		NULL,
		WS_CHILD/* | WS_VISIBLE*/ | WS_CLIPCHILDREN | /*WS_BORDER | */
/*		WS_EX_WINDOWEDGE| */
		TBSTYLE_TOOLTIPS |
//		TBSTYLE_WRAPABLE |
//		TBSTYLE_ALTDRAG |
//		CCS_ADJUSTABLE |
		nFlag,
		0, 0,
		0, 0,
		owner.GetHwnd(),
		(HMENU)ID_TOOLBAR,
		EditApp::getInstance().GetAppInstance(),
		NULL
	);
	if (!hwndToolBar) {
		if (csToolBar.bToolBarIsFlat) {	// フラットツールバーにする／しない
			csToolBar.bToolBarIsFlat = false;
		}
		TopWarningMessage(owner.GetHwnd(), LS(STR_ERR_DLGEDITWND05));
		DestroyToolBar();
	}else {
		// ツールバーをサブクラス化する
		g_pOldToolBarWndProc = (WNDPROC)::SetWindowLongPtr(
			hwndToolBar,
			GWLP_WNDPROC,
			(LONG_PTR)ToolBarWndProc
		);

		Toolbar_SetButtonSize(hwndToolBar, DpiScaleX(22), DpiScaleY(22));
		Toolbar_ButtonStructSize(hwndToolBar, sizeof(TBBUTTON));
		//	既に用意されているImage Listをアイコンとして登録
		pIcons->SetToolBarImages(hwndToolBar);
		// ツールバーにボタンを追加
		int count = 0;
		int nToolBarButtonNum = 0;
		// はじめにツールバー構造体の配列を作っておく
		std::vector<TBBUTTON> tbButtons(csToolBar.nToolBarButtonNum);
		TBBUTTON* pTbbArr = &tbButtons[0];
		for (i=0; i<csToolBar.nToolBarButtonNum; ++i) {
			nIdx = csToolBar.nToolBarButtonIdxArr[i];
			pTbbArr[nToolBarButtonNum] = owner.GetMenuDrawer().getButton(nIdx);
			// セパレータが続くときはひとつにまとめる
			// 折り返しボタンもTBSTYLE_SEP属性を持っているので
			// 折り返しの前のセパレータは全て削除される．
			if ((pTbbArr[nToolBarButtonNum].fsStyle & TBSTYLE_SEP) && (nToolBarButtonNum != 0)) {
				if ((pTbbArr[nToolBarButtonNum-1].fsStyle & TBSTYLE_SEP)) {
					pTbbArr[nToolBarButtonNum-1] = pTbbArr[nToolBarButtonNum];
					--nToolBarButtonNum;
				}
			}
			// 仮想折返しボタンがきたら直前のボタンに折返し属性を付ける
			if (pTbbArr[nToolBarButtonNum].fsState & TBSTATE_WRAP) {
				if (nToolBarButtonNum != 0) {
					pTbbArr[nToolBarButtonNum-1].fsState |= TBSTATE_WRAP;
				}
				continue;
			}
			++nToolBarButtonNum;
		}

		for (i=0; i<nToolBarButtonNum; ++i) {
			tbb = pTbbArr[i];

			switch (tbb.fsStyle) {
			case TBSTYLE_DROPDOWN:	// ドロップダウン
				// 拡張スタイルに設定
				Toolbar_SetExtendedStyle(hwndToolBar, TBSTYLE_EX_DRAWDDARROWS);
				Toolbar_AddButtons(hwndToolBar, 1, &tbb);
				++count;
				break;

			case TBSTYLE_COMBOBOX:	// コンボボックス
				{
					RECT			rc;
					TBBUTTONINFO	tbi;
					TBBUTTON		my_tbb;
					LOGFONT			lf;

					switch (tbb.idCommand) {
					case F_SEARCH_BOX:
						if (hwndSearchBox) {
							break;
						}
						
						// セパレータ作る
						memset_raw(&my_tbb, 0, sizeof(my_tbb));
						my_tbb.fsStyle   = TBSTYLE_BUTTON;  // ボタンにしないと描画が乱れる
						my_tbb.idCommand = tbb.idCommand;	// 同じIDにしておく
						if (tbb.fsState & TBSTATE_WRAP) {   // 折り返し
							my_tbb.fsState |=  TBSTATE_WRAP;
						}
						Toolbar_AddButtons(hwndToolBar, 1, &my_tbb);
						++count;

						// サイズを設定する
						tbi.cbSize = sizeof(tbi);
						tbi.dwMask = TBIF_SIZE;
						tbi.cx     = (WORD)DpiScaleX(160);	// ボックスの幅
						Toolbar_SetButtonInfo(hwndToolBar, tbb.idCommand, &tbi);

						// 位置とサイズを取得する
						rc.right = rc.left = rc.top = rc.bottom = 0;
						Toolbar_GetItemRect(hwndToolBar, count-1, &rc);

						// コンボボックスを作る
						// Mar. 8, 2003 genta 検索ボックスを1ドット下にずらした
						hwndSearchBox = CreateWindow(_T("COMBOBOX"), _T("Combo"),
								WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWN
								/*| CBS_SORT*/ | CBS_AUTOHSCROLL /*| CBS_DISABLENOSCROLL*/,
								rc.left, rc.top + 1, rc.right - rc.left, (rc.bottom - rc.top) * 10,
								hwndToolBar, (HMENU)(INT_PTR)tbb.idCommand, EditApp::getInstance().GetAppInstance(), NULL);
						if (hwndSearchBox) {
							owner.SetCurrentFocus(0);

							lf = owner.GetLogfont();
							//memset_raw(&lf, 0, sizeof(lf));
							lf.lfHeight			= DpiPointsToPixels(-9);
							lf.lfWidth			= 0;
							lf.lfEscapement		= 0;
							lf.lfOrientation	= 0;
							lf.lfWeight			= FW_NORMAL;
							lf.lfItalic			= FALSE;
							lf.lfUnderline		= FALSE;
							lf.lfStrikeOut		= FALSE;
							//lf.lfCharSet		= GetDllShareData().common.sView.lf.lfCharSet;
							lf.lfOutPrecision	= OUT_TT_ONLY_PRECIS;		// Raster Font を使わないように
							//lf.lfClipPrecision	= GetDllShareData().common.sView.lf.lfClipPrecision;
							//lf.lfQuality		= GetDllShareData().common.sView.lf.lfQuality;
							//lf.lfPitchAndFamily	= GetDllShareData().common.sView.lf.lfPitchAndFamily;
							//_tcsncpy(lf.lfFaceName, GetDllShareData().common.sView.lf.lfFaceName, _countof(lf.lfFaceName));	// 画面のフォントに設定	2012/11/27 Uchi
							hFontSearchBox = ::CreateFontIndirect(&lf);
							if (hFontSearchBox) {
								::SendMessage(hwndSearchBox, WM_SETFONT, (WPARAM)hFontSearchBox, MAKELONG (TRUE, 0));
							}

							// 入力長制限
							// Combo_LimitText(hwndSearchBox, (WPARAM)_MAX_PATH - 1);

							// 検索ボックスを更新
							AcceptSharedSearchKey();

							comboDel = ComboBoxItemDeleter(); // 再表示用の初期化
							comboDel.pRecent = &recentSearch;
							Dialog::SetComboBoxDeleter(hwndSearchBox, &comboDel);
						}
						break;

					default:
						break;
					}
				}
				break;

			case TBSTYLE_BUTTON:	// ボタン
			case TBSTYLE_SEP:		// セパレータ
			default:
				Toolbar_AddButtons(hwndToolBar, 1, &tbb);
				++count;
				break;
			}
		}
		if (csToolBar.bToolBarIsFlat) {	// フラットツールバーにする／しない
			lToolType = ::GetWindowLongPtr(hwndToolBar, GWL_STYLE);
			lToolType |= (TBSTYLE_FLAT);
			::SetWindowLongPtr(hwndToolBar, GWL_STYLE, lToolType);
			::InvalidateRect(hwndToolBar, NULL, TRUE);
		}
	}

	// ツールバーを Rebar に入れる
	if (hwndReBar && hwndToolBar) {
		// ツールバーの高さを取得する
		DWORD dwBtnSize = Toolbar_GetButtonSize(hwndToolBar);
		DWORD dwRows = Toolbar_GetRows(hwndToolBar);

		// バンド情報を設定する
		// 以前のプラットフォームに _WIN32_WINNT >= 0x0600 で定義される構造体のフルサイズを渡すと失敗する
		rbBand.cbSize = CCSIZEOF_STRUCT(REBARBANDINFO, wID);
		rbBand.fMask  = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE;
		rbBand.fStyle = RBBS_CHILDEDGE;
		rbBand.hwndChild  = hwndToolBar;	// ツールバー
		rbBand.cxMinChild = 0;
		rbBand.cyMinChild = HIWORD(dwBtnSize) * dwRows;
		rbBand.cx         = 250;

		// バンドを追加する
		Rebar_InsertBand(hwndReBar, -1, &rbBand);
		::ShowWindow(hwndToolBar, SW_SHOW);
	}

	return;
}

void MainToolBar::DestroyToolBar(void)
{
	if (hwndToolBar) {
		if (hwndSearchBox) {
			if (hFontSearchBox) {
				::DeleteObject(hFontSearchBox);
				hFontSearchBox = NULL;
			}

			::DestroyWindow(hwndSearchBox);
			hwndSearchBox = NULL;

			owner.SetCurrentFocus(0);
		}

		::DestroyWindow(hwndToolBar);
		hwndToolBar = NULL;

		//if (cTabWnd.owner->GetHwnd()) ::UpdateWindow(cTabWnd.owner->GetHwnd());
	}

	// Rebar を破棄する
	if (hwndReBar) {
		::DestroyWindow(hwndReBar);
		hwndReBar = NULL;
	}

	return;
}

// メッセージ処理。なんか処理したなら true を返す。
bool MainToolBar::EatMessage(MSG* msg)
{
	if (hwndSearchBox && ::IsDialogMessage(hwndSearchBox, msg)) {	// 検索コンボボックス
		ProcSearchBox(msg);
		return true;
	}
	return false;
}


/*!	@brief ToolBarのOwnerDraw

	@param pnmh [in] Owner Draw情報

	@note Common Control V4.71以降はNMTBCUSTOMDRAWを送ってくるが，
	Common Control V4.70はLPNMCUSTOMDRAWしか送ってこないので
	安全のため小さい方に合わせて処理を行う．
*/
LPARAM MainToolBar::ToolBarOwnerDraw(LPNMCUSTOMDRAW pnmh)
{
	switch (pnmh->dwDrawStage) {
	case CDDS_PREPAINT:
		// 描画開始前
		// アイテムを自前で描画する旨を通知する
		return CDRF_NOTIFYITEMDRAW;
	
	case CDDS_ITEMPREPAINT:
		// 面倒くさいので，枠はToolbarに描いてもらう
		// アイコンが登録されていないので中身は何も描かれない
		// 検索(ボックス)なら枠を描かない
		if (pnmh->dwItemSpec == F_SEARCH_BOX) {
			return CDRF_SKIPDEFAULT;
		}
		return CDRF_NOTIFYPOSTPAINT;
	
	case CDDS_ITEMPOSTPAINT:
		{
			// 描画
			// コマンド番号（pnmh->dwItemSpec）からアイコン番号を取得する
			int nIconId = Toolbar_GetBitmap(pnmh->hdr.hwndFrom, (WPARAM)pnmh->dwItemSpec);

			int offset = ((pnmh->rc.bottom - pnmh->rc.top) - pIcons->GetCy()) / 2;		// アイテム矩形からの画像のオフセット
			int shift = (pnmh->uItemState & (CDIS_SELECTED | CDIS_CHECKED)) ? 1 : 0;	//	Aug. 30, 2003 genta ボタンを押されたらちょっと画像をずらす

			//	Sep. 6, 2003 genta 押下時は右だけでなく下にもずらす
			pIcons->Draw(nIconId, pnmh->hdc, pnmh->rc.left + offset + shift, pnmh->rc.top + offset + shift,
				(pnmh->uItemState & CDIS_DISABLED) ? ILD_MASK : ILD_NORMAL
			);
		}
		break;
	default:
		break;
	}
	return CDRF_DODEFAULT;
}


/*! ツールバー更新用タイマーの処理 */
void MainToolBar::OnToolbarTimer(void)
{
}

/*!
	@brief ツールバーの表示を更新する
	
	@note 他から呼べるようにOnToolbarTimer()より切り出した
*/
void MainToolBar::UpdateToolbar(void)
{
	// 印刷Preview中なら、何もしない。
	if (owner.IsInPreviewMode())
		return;
	
	// ツールバーの状態更新
	if (hwndToolBar) {
		auto& csToolBar = GetDllShareData().common.toolBar;
		for (int i=0; i<csToolBar.nToolBarButtonNum; ++i) {
			TBBUTTON tbb = owner.GetMenuDrawer().getButton(
				csToolBar.nToolBarButtonIdxArr[i]
			);

			// 機能が利用可能か調べる
			Toolbar_EnableButton(
				hwndToolBar,
				tbb.idCommand,
				IsFuncEnable(owner.GetDocument(), GetDllShareData(), (EFunctionCode)tbb.idCommand)
			);

			// 機能がチェック状態か調べる
			Toolbar_CheckButton(
				hwndToolBar,
				tbb.idCommand,
				IsFuncChecked(owner.GetDocument(), GetDllShareData(), (EFunctionCode)tbb.idCommand)
			);
		}
	}
}

// 検索ボックスを更新
void MainToolBar::AcceptSharedSearchKey()
{
	if (hwndSearchBox) {
		// 2013.05.28 Combo_ResetContentだとちらつくのでDeleteStringでリストだけ削除
		while (Combo_GetCount(hwndSearchBox) > 0) {
			Combo_DeleteString(hwndSearchBox, 0);
		}
		size_t nSize = GetDllShareData().searchKeywords.searchKeys.size();
		for (size_t i=0; i<nSize; ++i) {
			Combo_AddString(hwndSearchBox, GetDllShareData().searchKeywords.searchKeys[i]);
		}
		const wchar_t* pszText;
		if (GetDllShareData().common.search.bInheritKeyOtherView
			&& owner.GetActiveView().nCurSearchKeySequence < GetDllShareData().common.search.nSearchKeySequence
			|| owner.GetActiveView().strCurSearchKey.size() == 0
		) {
			if (0 < nSize) {
				pszText = GetDllShareData().searchKeywords.searchKeys[0];
			}else {
				pszText = L"";
			}
		}else {
			pszText = owner.GetActiveView().strCurSearchKey.c_str();
		}
		std::wstring strText;
		GetSearchKey(strText);
		if (0 < nSize && wcscmp(strText.c_str(), pszText) != 0) {
			::SetWindowText(hwndSearchBox, to_tchar(pszText));
		}
	}
}

size_t MainToolBar::GetSearchKey(std::wstring& strText)
{
	if (hwndSearchBox) {
		int nBufferSize = ::GetWindowTextLength(hwndSearchBox) + 1;
		std::vector<TCHAR> vText(nBufferSize);

		::GetWindowText(hwndSearchBox, &vText[0], (int)vText.size());
		strText = to_wchar(&vText[0]);
	}else {
		strText = L"";
	}
	return strText.length();
}


/*!
ツールバーの検索ボックスにフォーカスを移動する.
*/
void MainToolBar::SetFocusSearchBox(void) const
{
	if (hwndSearchBox) {
		::SetFocus(hwndSearchBox);
	}
}

