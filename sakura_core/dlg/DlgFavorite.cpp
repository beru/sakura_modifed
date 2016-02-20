/*!	@file
	@brief 履歴の管理ダイアログボックス

	@author MIK
	@date 2003.4.8
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2006, ryoji
	Copyright (C) 2010, Moca

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
#include <algorithm>
#include "DlgFavorite.h"
#include "dlg/DlgInput1.h"
#include "env/DllSharedData.h"
#include "func/Funccode.h"
#include "util/shell.h"
#include "util/window.h"
#include "util/fileUtil.h"
#include "util/os.h"
#include "util/input.h"
#include "sakura_rc.h"
#include "sakura.hh"

const DWORD p_helpids[] = {
	IDC_TAB_FAVORITE,				HIDC_TAB_FAVORITE,				// タブ
	IDC_LIST_FAVORITE_FILE,			HIDC_LIST_FAVORITE_FILE,		// ファイル
	IDC_LIST_FAVORITE_FOLDER,		HIDC_LIST_FAVORITE_FOLDER,		// フォルダ
	IDC_LIST_FAVORITE_EXCEPTMRU,	HIDC_LIST_FAVORITE_EXCEPTMRU,	// MRU除外
	IDC_LIST_FAVORITE_SEARCH,		HIDC_LIST_FAVORITE_SEARCH,		// 検索
	IDC_LIST_FAVORITE_REPLACE,		HIDC_LIST_FAVORITE_REPLACE,		// 置換
	IDC_LIST_FAVORITE_GREP_FILE,	HIDC_LIST_FAVORITE_GREPFILE,	// GREPファイル
	IDC_LIST_FAVORITE_GREP_FOLDER,	HIDC_LIST_FAVORITE_GREPFOLDER,	// GREPフォルダ
	IDC_LIST_FAVORITE_CMD,			HIDC_LIST_FAVORITE_CMD,			// コマンド
	IDC_LIST_FAVORITE_CUR_DIR,		HIDC_LIST_FAVORITE_CUR_DIR,		// カレントディレクトリ
//	IDC_STATIC_BUTTONS,				-1,
	IDC_BUTTON_CLEAR,				HIDC_BUTTON_FAVORITE_CLEAR,		// すべて	// 2006.10.10 ryoji
	IDC_BUTTON_DELETE_NOFAVORATE,   HIDC_BUTTON_FAVORITE_DELETE_NOFAVORATE,	// お気に入り以外
	IDC_BUTTON_DELETE_NOTFOUND,		HIDC_BUTTON_FAVORITE_DELETE_NOTFOUND,	// 存在しない項目
	IDC_BUTTON_DELETE_SELECTED,     HIDC_BUTTON_FAVORITE_DELETE_SELECTED,	// 選択項目
	IDC_BUTTON_ADD_FAVORITE,        HIDC_BUTTON_ADD_FAVORITE,		// 追加
	IDOK,							HIDC_FAVORITE_IDOK,				// 閉じる
	IDC_BUTTON_HELP,				HIDC_BUTTON_FAVORITE_HELP,		// ヘルプ
//	IDC_STATIC_FAVORITE_MSG,		-1,
	0, 0
};

static const AnchorListItem anchorList[] = {
	{IDC_TAB_FAVORITE,              AnchorStyle::LeftRight},
	{IDC_STATIC_BUTTONS,			AnchorStyle::Bottom},
	{IDC_BUTTON_CLEAR, 				AnchorStyle::Bottom},
	{IDC_BUTTON_DELETE_NOFAVORATE,	AnchorStyle::Bottom},
	{IDC_BUTTON_DELETE_NOTFOUND,	AnchorStyle::Bottom},
	{IDC_BUTTON_DELETE_SELECTED,	AnchorStyle::Bottom},
	{IDC_BUTTON_ADD_FAVORITE, 		AnchorStyle::Bottom},
	{IDOK, 							AnchorStyle::Bottom},
	{IDC_BUTTON_HELP, 				AnchorStyle::Bottom},
	{IDC_STATIC_FAVORITE_MSG, 		AnchorStyle::Bottom},
};

// SDKにしか定義されていない。
#ifndef	ListView_SetCheckState
//#if (_WIN32_IE >= 0x0300)
#define ListView_SetCheckState(hwndLV, i, fCheck) \
  ListView_SetItemState(hwndLV, i, INDEXTOSTATEIMAGEMASK((fCheck) ? 2 : 1), LVIS_STATEIMAGEMASK)
//#endif
#endif

static int FormatFavoriteColumn(TCHAR*, int, int , bool);
static int ListView_GetLParamInt(HWND, int);
static int CALLBACK CompareListViewFunc(LPARAM, LPARAM, LPARAM);

struct CompareListViewLParam {
	int         nSortColumn;
	bool        bAbsOrder;
	HWND        hwndListView;
	const Recent* pRecent;
};

/*
	Recentの各実装クラスは DllSharedData へ直接アクセスしている。
	履歴はほかのウィンドウが書き換える可能性があるため、
	ダイアログがアクティブになった際に変更を確認し再取得するようになっている。
	編集中は変更を確認していないので、裏でDllSharedDataを変更されるとListViewと
	DllSharedDataが一致しない可能性もある。
*/


DlgFavorite::DlgFavorite()
	 :
	 Dialog(true)
{
	m_nCurrentTab = 0;
	m_szMsg[0] = 0;

	// サイズ変更時に位置を制御するコントロール数
	assert(_countof(anchorList) == _countof(m_rcItems));

	{
		FavoriteInfo* pFavInfo = &m_aFavoriteInfo[0];
		pFavInfo->m_pRecent    = &m_recentFile;
		pFavInfo->m_strCaption = LS(STR_DLGFAV_FILE);
		pFavInfo->m_pszCaption = const_cast<TCHAR*>(pFavInfo->m_strCaption.c_str());
		pFavInfo->m_nId        = IDC_LIST_FAVORITE_FILE;
		pFavInfo->m_bHaveFavorite = true;
		pFavInfo->m_bFilePath  = true;
		pFavInfo->m_bHaveView  = true;
		pFavInfo->m_bEditable  = false;
		pFavInfo->m_bAddExcept = true;

		++pFavInfo;
		pFavInfo->m_pRecent    = &m_recentFolder;
		pFavInfo->m_strCaption = LS(STR_DLGFAV_FOLDER);
		pFavInfo->m_pszCaption = const_cast<TCHAR*>(pFavInfo->m_strCaption.c_str());
		pFavInfo->m_nId        = IDC_LIST_FAVORITE_FOLDER;
		pFavInfo->m_bHaveFavorite = true;
		pFavInfo->m_bFilePath  = true;
		pFavInfo->m_bHaveView  = true;
		pFavInfo->m_bEditable  = false;
		pFavInfo->m_bAddExcept = true;

		++pFavInfo;
		pFavInfo->m_pRecent    = &m_recentExceptMRU;
		pFavInfo->m_strCaption = LS(STR_DLGFAV_FF_EXCLUDE);
		pFavInfo->m_pszCaption = const_cast<TCHAR*>(pFavInfo->m_strCaption.c_str());
		pFavInfo->m_nId        = IDC_LIST_FAVORITE_EXCEPTMRU;
		pFavInfo->m_bHaveFavorite = false;
		pFavInfo->m_bFilePath  = false;
		pFavInfo->m_bHaveView  = false;
		pFavInfo->m_bEditable  = true;
		pFavInfo->m_bAddExcept = false;
		m_nExceptTab = (pFavInfo - m_aFavoriteInfo);

		++pFavInfo;
		pFavInfo->m_pRecent    = &m_recentSearch;
		pFavInfo->m_strCaption = LS(STR_DLGFAV_SEARCH);
		pFavInfo->m_pszCaption = const_cast<TCHAR*>(pFavInfo->m_strCaption.c_str());
		pFavInfo->m_nId        = IDC_LIST_FAVORITE_SEARCH;
		pFavInfo->m_bHaveFavorite = false;
		pFavInfo->m_bFilePath  = false;
		pFavInfo->m_bHaveView  = false;
		pFavInfo->m_bEditable  = true;
		pFavInfo->m_bAddExcept = false;

		++pFavInfo;
		pFavInfo->m_pRecent    = &m_recentReplace;
		pFavInfo->m_strCaption = LS(STR_DLGFAV_REPLACE);
		pFavInfo->m_pszCaption = const_cast<TCHAR*>(pFavInfo->m_strCaption.c_str());
		pFavInfo->m_nId        = IDC_LIST_FAVORITE_REPLACE;
		pFavInfo->m_bHaveFavorite = false;
		pFavInfo->m_bFilePath  = false;
		pFavInfo->m_bHaveView  = false;
		pFavInfo->m_bEditable  = true;
		pFavInfo->m_bAddExcept = false;

		++pFavInfo;
		pFavInfo->m_pRecent    = &m_recentGrepFile;
		pFavInfo->m_strCaption = LS(STR_DLGFAV_GREP_FILE);
		pFavInfo->m_pszCaption = const_cast<TCHAR*>(pFavInfo->m_strCaption.c_str());
		pFavInfo->m_nId        = IDC_LIST_FAVORITE_GREP_FILE;
		pFavInfo->m_bHaveFavorite = false;
		pFavInfo->m_bFilePath  = false;
		pFavInfo->m_bHaveView  = false;
		pFavInfo->m_bEditable  = true;
		pFavInfo->m_bAddExcept = false;

		++pFavInfo;
		pFavInfo->m_pRecent    = &m_recentGrepFolder;
		pFavInfo->m_strCaption = LS(STR_DLGFAV_GREP_FOLDER);
		pFavInfo->m_pszCaption = const_cast<TCHAR*>(pFavInfo->m_strCaption.c_str());
		pFavInfo->m_nId        = IDC_LIST_FAVORITE_GREP_FOLDER;
		pFavInfo->m_bHaveFavorite = false;
		pFavInfo->m_bFilePath  = true;
		pFavInfo->m_bHaveView  = false;
		pFavInfo->m_bEditable  = false;
		pFavInfo->m_bAddExcept = false;

		++pFavInfo;
		pFavInfo->m_pRecent    = &m_recentCmd;
		pFavInfo->m_strCaption = LS(STR_DLGFAV_EXT_COMMAND);
		pFavInfo->m_pszCaption = const_cast<TCHAR*>(pFavInfo->m_strCaption.c_str());
		pFavInfo->m_nId        = IDC_LIST_FAVORITE_CMD;
		pFavInfo->m_bHaveFavorite = false;
		pFavInfo->m_bFilePath  = false;
		pFavInfo->m_bHaveView  = false;
		pFavInfo->m_bEditable  = true;
		pFavInfo->m_bAddExcept = false;

		++pFavInfo;
		pFavInfo->m_pRecent    = &m_recentCurDir;
		pFavInfo->m_strCaption = LS(STR_DLGFAV_CURRENT_DIR);
		pFavInfo->m_pszCaption = const_cast<TCHAR*>(pFavInfo->m_strCaption.c_str());
		pFavInfo->m_nId        = IDC_LIST_FAVORITE_CUR_DIR;
		pFavInfo->m_bHaveFavorite = false;
		pFavInfo->m_bFilePath  = true;
		pFavInfo->m_bHaveView  = false;
		pFavInfo->m_bEditable  = false;
		pFavInfo->m_bAddExcept = false;

		++pFavInfo;
		pFavInfo->m_pRecent    = NULL;
		pFavInfo->m_pszCaption = NULL;
		pFavInfo->m_nId        = -1;
		pFavInfo->m_bHaveFavorite = false;
		pFavInfo->m_bFilePath  = false;
		pFavInfo->m_bHaveView  = false;
		pFavInfo->m_bEditable  = false;
		pFavInfo->m_bAddExcept = false;

		// これ以上増やすときはテーブルサイズも書き換えてね
		assert((pFavInfo - m_aFavoriteInfo) < _countof(m_aFavoriteInfo));
	}
	for (int i=0; i<FAVORITE_INFO_MAX; ++i) {
		auto& info = m_aListViewInfo[i];
		info.hListView   = 0;
		info.nSortColumn = -1;
		info.bSortAscending = false;
	}
	m_ptDefaultSize.x = -1;
	m_ptDefaultSize.y = -1;
}

DlgFavorite::~DlgFavorite()
{
	for (int nTab=0; m_aFavoriteInfo[nTab].m_pRecent; ++nTab) {
		m_aFavoriteInfo[nTab].m_pRecent->Terminate();
	}
}

// モーダルダイアログの表示
int DlgFavorite::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam
)
{
	return (int)Dialog::DoModal(hInstance, hwndParent, IDD_FAVORITE, lParam);
}

// ダイアログデータの設定
void DlgFavorite::SetData(void)
{
	for (int nTab=0; m_aFavoriteInfo[nTab].m_pRecent; ++nTab) {
		SetDataOne(nTab, 0);
	}

	SetItemText(IDC_STATIC_FAVORITE_MSG, _T(""));

	UpdateUIState();

	return;
}

/* ダイアログデータの1つのタブの設定・更新
	@param nIndex       タブのIndex
	@param nLvItemIndex 選択・表示したいListViewのIndex。-1で選択しない
*/
void DlgFavorite::SetDataOne(int nIndex, int nLvItemIndex)
{
	LV_ITEM	lvi;
	int nNewFocus = -1;

	const Recent*  pRecent = m_aFavoriteInfo[nIndex].m_pRecent;

	// リスト
	HWND hwndList = GetItemHwnd(m_aFavoriteInfo[nIndex].m_nId);
	ListView_DeleteAllItems(hwndList);  // リストを空にする

	const int nViewCount = pRecent->GetViewCount();
	const int nItemCount = pRecent->GetItemCount();
	m_aFavoriteInfo[nIndex].m_nViewCount = nViewCount;

	TCHAR tmp[1024];
	for (int i=0; i<nItemCount; ++i) {
		FormatFavoriteColumn(tmp, _countof(tmp), i, i < nViewCount);
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = tmp;
		lvi.iItem    = i;
		lvi.iSubItem = 0;
		lvi.lParam   = i;
		ListView_InsertItem(hwndList, &lvi);

		const TCHAR* p;
		p = pRecent->GetItemText(i);
		auto_snprintf_s(tmp, _countof(tmp), _T("%ts"), p ? p : _T(""));
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = i;
		lvi.iSubItem = 1;
		lvi.pszText  = tmp;
		ListView_SetItem(hwndList, &lvi);

		if (m_aFavoriteInfo[nIndex].m_bHaveFavorite) {
			ListView_SetCheckState(hwndList, i, (BOOL)pRecent->IsFavorite(i));
		}
	}

	if (m_aListViewInfo[nIndex].nSortColumn != -1) {
		// ソートを維持
		ListViewSort(m_aListViewInfo[nIndex], pRecent, m_aListViewInfo[nIndex].nSortColumn, false);
	}

	if (nLvItemIndex != -1 && nLvItemIndex < nItemCount) {
		nNewFocus = nLvItemIndex;
	}

	// アイテムがあってどれも非選択なら、要求に近いアイテム(先頭か末尾)を選択
	if (nItemCount > 0 && nLvItemIndex != -1 && nNewFocus == -1) {
		nNewFocus = (0 < nLvItemIndex ? nItemCount - 1: 0);
	}

	if (nNewFocus != -1) {
		ListView_SetItemState(hwndList, nNewFocus, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		ListView_EnsureVisible(hwndList, nNewFocus, FALSE);
	}

	return;
}

/*! ダイアログデータを取得し、共有データのお気に入りを更新
	
	@retval TRUE 正常(今のところFALSEは返さない)
*/
int DlgFavorite::GetData(void)
{
	for (int nTab=0; m_aFavoriteInfo[nTab].m_pRecent; ++nTab) {
		if (m_aFavoriteInfo[nTab].m_bHaveFavorite) {
			GetFavorite(nTab);
			// リストを更新する。
			Recent* pRecent = m_aFavoriteInfo[nTab].m_pRecent;
			pRecent->UpdateView();
		}
	}

	return TRUE;
}

BOOL DlgFavorite::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	HWND		hwndList;
	TCITEM		tcitem;
	LV_COLUMN	col;

	_SetHwnd(hwndDlg);
	::SetWindowLongPtr(GetHwnd(), DWLP_USER, lParam);

	RECT rc;
	::GetWindowRect(hwndDlg, &rc);
	m_ptDefaultSize.x = rc.right - rc.left;
	m_ptDefaultSize.y = rc.bottom - rc.top;

	for (int i=0; i<_countof(anchorList); ++i) {
		GetItemClientRect(anchorList[i].id, m_rcItems[i]);
	}

	CreateSizeBox();
	Dialog::OnSize();

	RECT rcDialog = GetDllShareData().m_common.others.rcFavoriteDialog;
	if (0
		|| rcDialog.left != 0
		|| rcDialog.bottom != 0
	) {
		m_xPos = rcDialog.left;
		m_yPos = rcDialog.top;
		m_nWidth = rcDialog.right - rcDialog.left;
		m_nHeight = rcDialog.bottom - rcDialog.top;
	}

	// リストビューの表示位置を取得する。
	m_nCurrentTab = 0;
	HWND hwndBaseList = ::GetDlgItem(hwndDlg, m_aFavoriteInfo[0].m_nId);
	{
		rc.left = rc.top = rc.right = rc.bottom = 0;
		GetItemClientRect(m_aFavoriteInfo[0].m_nId, rc);
		m_rcListDefault = rc;
	}

	// ウィンドウのリサイズ
	SetDialogPosSize();

	HWND hwndTab = ::GetDlgItem(hwndDlg, IDC_TAB_FAVORITE);
	TabCtrl_DeleteAllItems(hwndTab);

	GetItemClientRect(m_aFavoriteInfo[0].m_nId, rc);

	// リストビューのItem/SubItem幅を計算
	std::tstring pszFavTest = LS(STR_DLGFAV_FAVORITE);
	TCHAR* pszFAVORITE_TEXT = const_cast<TCHAR*>(pszFavTest.c_str());
	const int nListViewWidthClient = rc.right - rc.left
		 - TextWidthCalc::WIDTH_MARGIN_SCROLLBER - ::GetSystemMetrics(SM_CXVSCROLL);
	// 初期値は従来方式の%指定
	int nItemCx = nListViewWidthClient * 16 / 100;
	int nSubItem1Cx = nListViewWidthClient * 79 / 100;
	
	{
		// 適用されているフォントから算出
		TextWidthCalc calc(hwndBaseList);
		calc.SetTextWidthIfMax(pszFAVORITE_TEXT, TextWidthCalc::WIDTH_LV_HEADER);
		TCHAR szBuf[200];
		for (int i=0; i<40; ++i) {
			//「M (非表示)」等の幅を求める
			FormatFavoriteColumn(szBuf, _countof(szBuf), i, false);
			calc.SetTextWidthIfMax(szBuf, TextWidthCalc::WIDTH_LV_ITEM_CHECKBOX);
		}
		
		if (0 < calc.GetCx()) {
			nItemCx = calc.GetCx();
			nSubItem1Cx = nListViewWidthClient - nItemCx;
		}
	}

	for (int nTab=0; m_aFavoriteInfo[nTab].m_pRecent; ++nTab) {
		hwndList = GetDlgItem(hwndDlg, m_aFavoriteInfo[nTab].m_nId);
		m_aListViewInfo[nTab].hListView = hwndList;
		
		::MoveWindow(hwndList, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, FALSE);
		::ShowWindow(hwndList, SW_HIDE);

		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = nItemCx;
		col.pszText  = pszFAVORITE_TEXT;
		col.iSubItem = 0;
		ListView_InsertColumn(hwndList, 0, &col);

		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = nSubItem1Cx;
		col.pszText  = const_cast<TCHAR*>(m_aFavoriteInfo[nTab].m_pszCaption);
		col.iSubItem = 1;
		ListView_InsertColumn(hwndList, 1, &col);

		// 行選択
		long lngStyle = ListView_GetExtendedListViewStyle(hwndList);
		lngStyle |= LVS_EX_FULLROWSELECT;
		if (m_aFavoriteInfo[nTab].m_bHaveFavorite) lngStyle |= LVS_EX_CHECKBOXES;
		ListView_SetExtendedListViewStyle(hwndList, lngStyle);

		// タブ項目追加
		tcitem.mask = TCIF_TEXT;
		tcitem.pszText = const_cast<TCHAR*>(m_aFavoriteInfo[nTab].m_pszCaption);
		TabCtrl_InsertItem(hwndTab, nTab, &tcitem);
	}

	hwndList = ::GetDlgItem(hwndDlg, m_aFavoriteInfo[m_nCurrentTab].m_nId);
	::ShowWindow(hwndList, SW_SHOW);
	TabCtrl_SetCurSel(hwndTab, m_nCurrentTab);
	//ChangeSlider(m_nCurrentTab);

	return Dialog::OnInitDialog(GetHwnd(), wParam, lParam);
}

BOOL DlgFavorite::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_HELP:
		// ヘルプ
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_FAVORITE));	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;

	case IDOK:
		// ダイアログデータの取得
		::EndDialog(GetHwnd(), (BOOL)GetData());
		return TRUE;

	case IDCANCEL:
		// 2010.03.20 キャンセルを廃止。OKと同じにする。
		// [X]ボタンを押すと通過する
		::EndDialog(GetHwnd(), (BOOL)GetData());
		return TRUE;

	// 2010.03.20 Moca 「お気に入り以外かすべて削除」選択メッセージボックスを廃止し
	//     それぞれのボタンに変更
	// すべて削除
	case IDC_BUTTON_CLEAR:
		{
			SetItemText(IDC_STATIC_FAVORITE_MSG, _T(""));
			Recent* pRecent = m_aFavoriteInfo[m_nCurrentTab].m_pRecent;
			if (pRecent) {
				const int nRet = ConfirmMessage(GetHwnd(), 
					LS(STR_DLGFAV_CONF_DEL_FAV),	// "最近使った%tsの履歴を削除します。\nよろしいですか？\n"
					m_aFavoriteInfo[m_nCurrentTab].m_pszCaption);
				if (nRet == IDYES) {
					pRecent->DeleteAllItem();
					RefreshListOne(m_nCurrentTab);
					UpdateUIState();
				}
			}
		}
		return TRUE;
	// お気に入り以外削除
	case IDC_BUTTON_DELETE_NOFAVORATE:
		{
			SetItemText(IDC_STATIC_FAVORITE_MSG, _T(""));
			if (m_aFavoriteInfo[m_nCurrentTab].m_bHaveFavorite) {
				int const nRet = ConfirmMessage(GetHwnd(), 
					LS(STR_DLGFAV_CONF_DEL_NOTFAV),	// "最近使った%tsの履歴のお気に入り以外を削除します。\nよろしいですか？"
					m_aFavoriteInfo[m_nCurrentTab].m_pszCaption);
				Recent* const pRecent = m_aFavoriteInfo[m_nCurrentTab].m_pRecent;
				if (nRet == IDYES && pRecent) {
					GetFavorite(m_nCurrentTab);
					pRecent->DeleteItemsNoFavorite();
					pRecent->UpdateView();
					RefreshListOne(m_nCurrentTab);
					UpdateUIState();
				}
			}
		}
		return TRUE;
	// 存在しない項目 を削除
	case IDC_BUTTON_DELETE_NOTFOUND:
		{
			SetItemText(IDC_STATIC_FAVORITE_MSG, _T(""));
			if (m_aFavoriteInfo[m_nCurrentTab].m_bFilePath) {
				const int nRet = ConfirmMessage(GetHwnd(), 
					LS(STR_DLGFAV_CONF_DEL_PATH),	// "最近使った%tsの存在しないパスを削除します。\nよろしいですか？"
					m_aFavoriteInfo[m_nCurrentTab].m_pszCaption);
				Recent* const pRecent = m_aFavoriteInfo[m_nCurrentTab].m_pRecent;
				if (nRet == IDYES && pRecent) {
					GetFavorite(m_nCurrentTab);

					// 存在しないパスの削除
					for (int i=pRecent->GetItemCount()-1; i>=0; --i) {
						TCHAR szPath[_MAX_PATH];
						auto_strcpy(szPath, pRecent->GetItemText(i));
						CutLastYenFromDirectoryPath(szPath);
						if (!IsFileExists(szPath, false)) {
							pRecent->DeleteItem(i);
						}
					}
					pRecent->UpdateView();
					RefreshListOne(m_nCurrentTab);
					UpdateUIState();
				}
			}
		}
		return TRUE;
	// 選択項目の削除
	case IDC_BUTTON_DELETE_SELECTED:
		{
			DeleteSelected();
		}
		return TRUE;
	case IDC_BUTTON_ADD_FAVORITE:
		{
			AddItem();
		}
		return TRUE;
	}

	// 基底クラスメンバ
	return Dialog::OnBnClicked(wID);
}

BOOL DlgFavorite::OnNotify(WPARAM wParam, LPARAM lParam)
{
	LPNMHDR	lpnmhdr;
	HWND	hwndTab;
	HWND	hwndList;

	hwndTab = GetItemHwnd(IDC_TAB_FAVORITE);
	lpnmhdr = (LPNMHDR) lParam;
	if (lpnmhdr->hwndFrom == hwndTab) {
		switch (lpnmhdr->code) {
		case TCN_SELCHANGE:
			TabSelectChange(false);
			return TRUE;
			//break;
		}
	}else {
		hwndList = m_aListViewInfo[m_nCurrentTab].hListView;
		if (hwndList == lpnmhdr->hwndFrom) {
			NM_LISTVIEW* pnlv = (NM_LISTVIEW*)lParam;
			switch (lpnmhdr->code) {
			case NM_DBLCLK:
				EditItem();
				return TRUE;
			case NM_RCLICK:
				{
					POINT po;
					if (GetCursorPos(&po) != 0) {
						RightMenu(po);
					}
				}
				return TRUE;

			// ListViewヘッダクリック:ソートする
			case LVN_COLUMNCLICK:
				ListViewSort(
					m_aListViewInfo[m_nCurrentTab],
					m_aFavoriteInfo[m_nCurrentTab].m_pRecent,
					pnlv->iSubItem, true);
				return TRUE;
			
			// ListViewでDeleteキーが押された:削除
			case LVN_KEYDOWN:
				switch (((NMLVKEYDOWN*)lParam)->wVKey) {
				case VK_DELETE:
					DeleteSelected();
					return TRUE;
				case VK_APPS:
					{
						POINT po;
						RECT rc;
						hwndList = GetItemHwnd(m_aFavoriteInfo[m_nCurrentTab].m_nId);
						::GetWindowRect(hwndList, &rc);
						po.x = rc.left;
						po.y = rc.top;
						RightMenu(po);
					}
					return TRUE;
				}
				int nIdx = GetCtrlKeyState();
				WORD wKey = ((NMLVKEYDOWN*)lParam)->wVKey;
				if ((wKey == VK_NEXT && nIdx == _CTRL)) {
					int next = m_nCurrentTab + 1;
					if (_countof(m_aFavoriteInfo) - 1 <= next) {
						next = 0;
					}
					TabCtrl_SetCurSel(GetItemHwnd(IDC_TAB_FAVORITE), next);
					TabSelectChange(true);
					return FALSE;
				}else if ((wKey == VK_PRIOR && nIdx == _CTRL)) {
					int prev = m_nCurrentTab - 1;
					if (prev < 0) {
						prev = _countof(m_aFavoriteInfo) - 2;
					}
					TabCtrl_SetCurSel(GetItemHwnd(IDC_TAB_FAVORITE), prev);
					TabSelectChange(true);
					return FALSE;
				}
			}
		}
	}

	// 基底クラスメンバ
	return Dialog::OnNotify(wParam, lParam);
}

void DlgFavorite::TabSelectChange(bool bSetFocus)
{
	SetItemText(IDC_STATIC_FAVORITE_MSG, _T(""));
	HWND hwndTab = GetItemHwnd(IDC_TAB_FAVORITE);
	int nIndex = TabCtrl_GetCurSel(hwndTab);
	if (nIndex != -1) {
		// 新しく表示する。
		HWND hwndList = GetItemHwnd(m_aFavoriteInfo[nIndex].m_nId);
		::ShowWindow(hwndList, SW_SHOW);

		// 現在表示中のリストを隠す。
		HWND hwndList2 = GetItemHwnd(m_aFavoriteInfo[m_nCurrentTab].m_nId);
		::ShowWindow(hwndList2, SW_HIDE);

		if (bSetFocus) {
			::SetFocus(hwndList);
		}

		m_nCurrentTab = nIndex;

		UpdateUIState();

		//ChangeSlider(nIndex);
	}
}

BOOL DlgFavorite::OnActivate(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam)) {
	case WA_ACTIVE:
	case WA_CLICKACTIVE:
		RefreshList();
		SetItemText(IDC_STATIC_FAVORITE_MSG, m_szMsg);
		return TRUE;
		//break;

	case WA_INACTIVE:
	default:
		break;
	}

	// 基底クラスメンバ
	return Dialog::OnActivate(wParam, lParam);
}

LPVOID DlgFavorite::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

/*
	リストを更新する。
*/
bool DlgFavorite::RefreshList(void)
{
	int		nTab;
	bool	bret;
	bool	ret_val = false;
	TCHAR	msg[1024];

	msg[0] = 0;
	m_szMsg[0] = 0;

	// 全リストの現在選択中のアイテムを取得する。
	for (nTab=0; m_aFavoriteInfo[nTab].m_pRecent; ++nTab) {
		bret = RefreshListOne(nTab);
		if (bret) {
			ret_val = true;
		
			if (msg[0] != _T('\0')) _tcscat(msg, LS(STR_DLGFAV_DELIMITER));
			_tcscat(msg, m_aFavoriteInfo[nTab].m_pszCaption);
		}
	}

	if (ret_val) {
		auto_snprintf_s(m_szMsg, _countof(m_szMsg),
			LS(STR_DLGFAV_FAV_REFRESH),	// "履歴(%ts)が更新されたため編集中情報を破棄し再表示しました。"
			msg);
	}

	return ret_val;
}

/*
	履歴種別リストのうち1個のリストビューを更新する。
*/
bool DlgFavorite::RefreshListOne(int nIndex)
{
	HWND	hwndList;
	int		nCount;
	int		nCurrentIndex;
	int		nItemCount;
	int		i;
	BOOL	bret;
	LVITEM	lvitem;

	Recent*	pRecent = m_aFavoriteInfo[nIndex].m_pRecent;
	nItemCount    = pRecent->GetItemCount();
	hwndList      = GetItemHwnd(m_aFavoriteInfo[nIndex].m_nId);
	nCount        = ListView_GetItemCount(hwndList);
	nCurrentIndex = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);
	if (nCurrentIndex == -1) nCurrentIndex = ListView_GetNextItem(hwndList, -1, LVNI_FOCUSED);

	if (nItemCount != nCount) goto changed;	// 個数が変わったので再構築

	// お気に入り数が変わったので再構築
	if (m_aFavoriteInfo[nIndex].m_nViewCount != pRecent->GetViewCount()) goto changed;

	for (i=0; i<nCount; ++i) {
		TCHAR	szText[1024];
		auto_memset(szText, 0, _countof(szText));
		memset_raw(&lvitem, 0, sizeof(lvitem));
		lvitem.mask       = LVIF_TEXT | LVIF_PARAM;
		lvitem.pszText    = szText;
		lvitem.cchTextMax = _countof(szText);
		lvitem.iItem      = i;
		lvitem.iSubItem   = 1;
		bret = ListView_GetItem(hwndList, &lvitem);
		if (!bret) goto changed;	// エラーなので再構築

		// アイテム内容が変わったので再構築
		if (lvitem.lParam != pRecent->FindItemByText(szText)) goto changed;
	}

	return false;

changed:
	SetDataOne(nIndex, nCurrentIndex);
	
	return true;
}

// お気に入りのフラグだけ適用
void DlgFavorite::GetFavorite(int nIndex)
{
	Recent* const pRecent = m_aFavoriteInfo[nIndex].m_pRecent;
	const HWND hwndList = m_aListViewInfo[nIndex].hListView;
	if (m_aFavoriteInfo[nIndex].m_bHaveFavorite) {
		const int nCount = ListView_GetItemCount(hwndList);
		for (int i=0; i<nCount; ++i) {
			const int  recIndex = ListView_GetLParamInt(hwndList, i);
			const BOOL bret = ListView_GetCheckState(hwndList, i);
			pRecent->SetFavorite(recIndex, bret ? true : false);
		}
	}
}


/*
	選択中の項目を削除
	リストの更新もする
*/
int DlgFavorite::DeleteSelected()
{
	SetItemText(IDC_STATIC_FAVORITE_MSG, _T(""));
	int     nDelItemCount = 0;
	Recent* pRecent = m_aFavoriteInfo[m_nCurrentTab].m_pRecent;
	if (pRecent) {
		HWND hwndList = m_aListViewInfo[m_nCurrentTab].hListView;
		int nSelectedCount = ListView_GetSelectedCount(hwndList);
		if (0 < nSelectedCount) {
			GetFavorite(m_nCurrentTab);

			int nLastSelectedItem = -1;
			std::vector<int> selRecIndexs;
			{
				int nLvItem = -1;
				while ((nLvItem = ListView_GetNextItem(hwndList, nLvItem, LVNI_SELECTED)) != -1) {
					int nRecIndex = ListView_GetLParamInt(hwndList, nLvItem);
					if (0 <= nRecIndex) {
						selRecIndexs.push_back(nRecIndex);
						nLastSelectedItem = nLvItem;
					}
				}
			}
			std::sort(selRecIndexs.rbegin(), selRecIndexs.rend());
			// 大きいほうから削除しないと、Recent側のindexがずれる
			size_t nSize = selRecIndexs.size();
			for (size_t n=0; n<nSize; ++n) {
				pRecent->DeleteItem(selRecIndexs[n]);
				++nDelItemCount;
			}
			pRecent->UpdateView();
			if (0 < nDelItemCount) {
				int nItem = nLastSelectedItem;
				if (nItem != -1) {
					nItem += 1; // 削除したアイテムの次のアイテム
					nItem -= nDelItemCount; // 新しい位置は、削除した分だけずれる
					if (pRecent->GetItemCount() <= nItem) {
						// 旧データの最後の要素が削除されているときは、
						// 新データの最後を選択
						nItem = pRecent->GetItemCount() -1;
					}
				}
				int nLvTopIndex = ListView_GetTopIndex(hwndList);
				SetDataOne(m_nCurrentTab, nItem);
				if (nDelItemCount == 1) {
					// 1つ削除のときは、Yスクロール位置を保持
					// 2つ以上は複雑なのでSetDataOneにおまかせする
					nLvTopIndex = t_max(0, t_min(pRecent->GetItemCount() - 1, nLvTopIndex));
					int nNowLvTopIndex = ListView_GetTopIndex(hwndList);
					if (nNowLvTopIndex != nLvTopIndex) {
						Rect rect;
						if (ListView_GetItemRect(hwndList, nNowLvTopIndex, &rect, LVIR_BOUNDS)) {
							// ListView_ScrollのY座標はpixel単位でスクロール変化分を指定
							ListView_Scroll(hwndList, 0,
								(nLvTopIndex - nNowLvTopIndex) * (rect.bottom - rect.top));
						}
					}
				}
				UpdateUIState();
			}
		}
	}
	return nDelItemCount;
}

void DlgFavorite::UpdateUIState()
{
	Recent& recent = *(m_aFavoriteInfo[m_nCurrentTab].m_pRecent);

	EnableItem(IDC_BUTTON_ADD_FAVORITE,
		m_aFavoriteInfo[m_nCurrentTab].m_bEditable && recent.GetItemCount() <= recent.GetArrayCount());

	// 削除の有効・無効化
	EnableItem(IDC_BUTTON_CLEAR,
		0 < recent.GetItemCount());

	EnableItem(IDC_BUTTON_DELETE_NOFAVORATE,
		m_aFavoriteInfo[m_nCurrentTab].m_bHaveFavorite && 0 < recent.GetItemCount());

	EnableItem(IDC_BUTTON_DELETE_NOTFOUND,
		m_aFavoriteInfo[m_nCurrentTab].m_bFilePath && 0 < recent.GetItemCount());

	EnableItem(IDC_BUTTON_DELETE_SELECTED,
		0 < recent.GetItemCount());
}

void DlgFavorite::AddItem()
{
	if (!m_aFavoriteInfo[m_nCurrentTab].m_bEditable) {
		return;
	}
	TCHAR szAddText[_MAX_PATH];
	int max_size = _MAX_PATH;
	szAddText[0] = 0;

	DlgInput1	dlgInput1;
	std::tstring strTitle = LS(STR_DLGFAV_ADD);
	std::tstring strMessage = LS(STR_DLGFAV_ADD_PROMPT);
	if (
		!dlgInput1.DoModal(
			G_AppInstance(),
			GetHwnd(),
			strTitle.c_str(),
			strMessage.c_str(),
			max_size,
			szAddText
		)
	) {
		return;
	}

	Recent& recent = *(m_aFavoriteInfo[m_nCurrentTab].m_pRecent);
	GetFavorite(m_nCurrentTab);
	if (recent.AppendItemText(szAddText)) {
		SetDataOne(m_nCurrentTab, -1);
		UpdateUIState();
	}
}

void DlgFavorite::EditItem()
{
	if (!m_aFavoriteInfo[m_nCurrentTab].m_bEditable) {
		return;
	}
	HWND hwndList = m_aListViewInfo[m_nCurrentTab].hListView;
	int nSelectedCount = ListView_GetSelectedCount(hwndList);
	if (0 < nSelectedCount) {
		int nLvItem = -1;
		nLvItem = ListView_GetNextItem(hwndList, nLvItem, LVNI_SELECTED);
		if (nLvItem != -1) {
			int nRecIndex = ListView_GetLParamInt(hwndList, nLvItem);
			Recent& recent = *(m_aFavoriteInfo[m_nCurrentTab].m_pRecent);
			TCHAR szText[_MAX_PATH];
			int max_size = _MAX_PATH;
			_tcsncpy_s(szText, max_size, recent.GetItemText(nRecIndex), _TRUNCATE);
			DlgInput1	dlgInput1;
			std::tstring strTitle = LS(STR_DLGFAV_EDIT);
			std::tstring strMessage = LS(STR_DLGFAV_EDIT_PROMPT);
			if (
				!dlgInput1.DoModal(
					G_AppInstance(),
					GetHwnd(),
					strTitle.c_str(),
					strMessage.c_str(),
					max_size,
					szText
				)
			) {
				return;
			}
			GetFavorite(m_nCurrentTab);
			if (recent.EditItemText(nRecIndex, szText)) {
				SetDataOne(m_nCurrentTab, nRecIndex);
				UpdateUIState();
			}
		}
	}
}

void DlgFavorite::RightMenu(POINT& menuPos)
{
	HMENU hMenu = ::CreatePopupMenu();
	const int MENU_EDIT = 100;
	const int MENU_ADD_EXCEPT = 101;
	const int MENU_ADD_NEW = 102;
	const int MENU_DELETE_ALL = 200;
	const int MENU_DELETE_NOFAVORATE = 201;
	const int MENU_DELETE_NOTFOUND = 202;
	const int MENU_DELETE_SELECTED = 203;
	Recent& recent = *m_aFavoriteInfo[m_nCurrentTab].m_pRecent;
	Recent& exceptMRU = *m_aFavoriteInfo[m_nExceptTab].m_pRecent;
	
	int iPos = 0;
	int nEnable;
	nEnable = (m_aFavoriteInfo[m_nCurrentTab].m_bEditable && 0 < recent.GetItemCount() ? 0 : MF_GRAYED);
	::InsertMenu(hMenu, iPos++, MF_BYPOSITION | MF_STRING | nEnable, MENU_EDIT, LS(STR_DLGFAV_MENU_EDIT));
	::InsertMenu(hMenu, iPos++, MF_BYPOSITION | MF_SEPARATOR, 0,	NULL);
	nEnable = (m_aFavoriteInfo[m_nCurrentTab].m_bEditable ? 0 : MF_GRAYED);
	::InsertMenu(hMenu, iPos++, MF_BYPOSITION | MF_STRING | nEnable, MENU_ADD_NEW, LS(STR_DLGFAV_MENU_ADD));
	if (m_aFavoriteInfo[m_nCurrentTab].m_bAddExcept) {
		nEnable = (exceptMRU.GetItemCount() <= exceptMRU.GetArrayCount() ? 0 : MF_GRAYED);
		::InsertMenu(hMenu, iPos++, MF_BYPOSITION | MF_STRING | nEnable, MENU_ADD_EXCEPT, LS(STR_DLGFAV_MENU_EXCLUDE));
	}
	::InsertMenu(hMenu, iPos++, MF_BYPOSITION | MF_SEPARATOR, 0,	NULL);
	nEnable = (0 < recent.GetItemCount() ? 0 : MF_GRAYED);
	::InsertMenu(hMenu, iPos++, MF_BYPOSITION | MF_STRING | nEnable, MENU_DELETE_ALL, LS(STR_DLGFAV_MENU_DEL_ALL));
	nEnable = (m_aFavoriteInfo[m_nCurrentTab].m_bHaveFavorite && 0 < recent.GetItemCount() ? 0 : MF_GRAYED);
	::InsertMenu(hMenu, iPos++, MF_BYPOSITION | MF_STRING | nEnable, MENU_DELETE_NOFAVORATE, LS(STR_DLGFAV_MENU_DEL_NOTFAV));
	nEnable = (m_aFavoriteInfo[m_nCurrentTab].m_bFilePath && 0 < recent.GetItemCount() ? 0 : MF_GRAYED);
	::InsertMenu(hMenu, iPos++, MF_BYPOSITION | MF_STRING | nEnable, MENU_DELETE_NOTFOUND, LS(STR_DLGFAV_MENU_DEL_INVALID));
	nEnable = (0 < recent.GetItemCount() ? 0 : MF_GRAYED);
	::InsertMenu(hMenu, iPos++, MF_BYPOSITION | MF_STRING | nEnable, MENU_DELETE_SELECTED, LS(STR_DLGFAV_MENU_DEL_SEL));

	// メニューを表示する
	POINT pt = menuPos;
	RECT rcWork;
	GetMonitorWorkRect(pt, &rcWork);	// モニタのワークエリア
	int nId = ::TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
								(pt.x > rcWork.left)? pt.x: rcWork.left,
								(pt.y < rcWork.bottom)? pt.y: rcWork.bottom,
								0, GetHwnd(), NULL);
	::DestroyMenu(hMenu);	// サブメニューは再帰的に破棄される

	switch (nId) {
	case MENU_EDIT:
		EditItem();
		break;
	case MENU_ADD_EXCEPT:
		{
			SetItemText(IDC_STATIC_FAVORITE_MSG, _T(""));
			Recent *pRecent = m_aFavoriteInfo[m_nCurrentTab].m_pRecent;
			if (pRecent) {
				HWND hwndList = m_aListViewInfo[m_nCurrentTab].hListView;
				int nSelectedCount = ListView_GetSelectedCount(hwndList);
				if (0 < nSelectedCount) {
					int nLvItem = -1;
					bool bAddFalse = false;
					Recent& exceptMRU = *m_aFavoriteInfo[m_nExceptTab].m_pRecent;
					while ((nLvItem = ListView_GetNextItem(hwndList, nLvItem, LVNI_SELECTED)) != -1) {
						int nRecIndex = ListView_GetLParamInt(hwndList, nLvItem);
						if (exceptMRU.GetArrayCount() <= exceptMRU.GetItemCount()) {
							bAddFalse = true;
						}else {
							exceptMRU.AppendItemText(pRecent->GetItemText(nRecIndex));
						}
					}
					if (bAddFalse) {
						WarningMessage(GetHwnd(), LS(STR_DLGFAV_LIST_LIMIT_OVER));	// "除外リストがいっぱいで追加できませんでした。"
					}
					SetDataOne(m_nExceptTab, -1);
					UpdateUIState();
				}
			}
		}
		break;
	case MENU_ADD_NEW:
		AddItem();
		break;
	case MENU_DELETE_ALL:
		OnBnClicked(IDC_BUTTON_CLEAR);
		break;
	case MENU_DELETE_NOFAVORATE:
		OnBnClicked(IDC_BUTTON_DELETE_NOFAVORATE);
		break;
	case MENU_DELETE_NOTFOUND:
		OnBnClicked(IDC_BUTTON_DELETE_NOTFOUND);
		break;
	case MENU_DELETE_SELECTED:
		OnBnClicked(IDC_BUTTON_DELETE_SELECTED);
		break;
	}
}

int FormatFavoriteColumn(
	TCHAR* buf,
	int size,
	int index,
	bool view
	)
{
	// 2010.03.21 Moca Textに連番を設定することによってアクセスキーにする
	// 0 - 9 A - Z
	const int mod = index % 36;
	const TCHAR c = (TCHAR)(((mod) <= 9) ? (_T('0') + mod) : (_T('A') + mod - 10));
	return auto_snprintf_s(buf, size, _T("%tc %ts"), c, (view ? _T("  ") : LS(STR_DLGFAV_HIDDEN)));
}


/*!
	ListViewのItem(index)からLParamをint型として取得
*/
static int ListView_GetLParamInt(HWND hwndList, int lvIndex)
{
	LV_ITEM	lvitem;
	memset_raw( &lvitem, 0, sizeof(lvitem) );
	lvitem.mask = LVIF_PARAM;
	lvitem.iItem = lvIndex;
	lvitem.iSubItem = 0;
	if (ListView_GetItem(hwndList, &lvitem)) {
		return (int)lvitem.lParam;
	}
	return -1;
}

/*!
	
	@param info [in,out] リストビューのソート状態情報
	@param pRecent       ソートアイテム
	@param column        ソートしたい列番号
	@param bReverse      ソート済みの場合に降順に切り替える
*/
// static
void DlgFavorite::ListViewSort(
	ListViewSortInfo& info,
	const Recent* pRecent,
	int column,
	bool bReverse
	)
{
	CompareListViewLParam lparamInfo;
	// ソート順の決定
	if (info.nSortColumn != column) {
		info.bSortAscending = true;
	}else {
		// ソート逆順(降順)
		info.bSortAscending = (bReverse ? (!info.bSortAscending): true);
	}
	
	// ヘッダ書き換え
	TCHAR szHeader[200];
	LV_COLUMN	col;
	if (info.nSortColumn != -1) {
		// 元のソートの「 ▼」を取り除く
		col.mask = LVCF_TEXT;
		col.pszText = szHeader;
		col.cchTextMax = _countof(szHeader);
		col.iSubItem = 0;
		ListView_GetColumn(info.hListView, info.nSortColumn, &col);
		int nLen = (int)_tcslen(szHeader) - _tcslen(_T("▼"));
		if (0 <= nLen) {
			szHeader[nLen] = _T('\0');
		}
		col.mask = LVCF_TEXT;
		col.pszText = szHeader;
		col.iSubItem = 0;
		ListView_SetColumn(info.hListView, info.nSortColumn, &col);
	}
	// 「▼」を付加
	col.mask = LVCF_TEXT;
	col.pszText = szHeader;
	col.cchTextMax = _countof(szHeader) - 4;
	col.iSubItem = 0;
	ListView_GetColumn(info.hListView, column, &col);
	_tcscat(szHeader, info.bSortAscending ? _T("▼") : _T("▲"));
	col.mask = LVCF_TEXT;
	col.pszText = szHeader;
	col.iSubItem = 0;
	ListView_SetColumn(info.hListView, column, &col);

	info.nSortColumn = column;

	lparamInfo.nSortColumn = column;
	lparamInfo.hwndListView = info.hListView;
	lparamInfo.pRecent = pRecent;
	lparamInfo.bAbsOrder = info.bSortAscending;

	ListView_SortItems(info.hListView, CompareListViewFunc, (LPARAM)&lparamInfo);
}


static int CALLBACK CompareListViewFunc(
	LPARAM lParamItem1,
	LPARAM lParamItem2,
	LPARAM lParamSort
	)
{
	CompareListViewLParam* pCompInfo = reinterpret_cast<CompareListViewLParam*>(lParamSort);
	int nRet = 0;
	if (pCompInfo->nSortColumn == 0) {
		nRet = lParamItem1 - lParamItem2;
	}else {
		const Recent* p = pCompInfo->pRecent;
		nRet = auto_stricmp(p->GetItemText((int)lParamItem1), p->GetItemText((int)lParamItem2));
	}
	return pCompInfo->bAbsOrder ? nRet : -nRet;
}

INT_PTR DlgFavorite::DispatchEvent(
	HWND hWnd,
	UINT wMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	INT_PTR result;
	result = Dialog::DispatchEvent(hWnd, wMsg, wParam, lParam);

	if (wMsg == WM_GETMINMAXINFO) {
		return OnMinMaxInfo(lParam);
	}
	return result;
}

BOOL DlgFavorite::OnSize(WPARAM wParam, LPARAM lParam)
{
	// 基底クラスメンバ
	Dialog::OnSize(wParam, lParam);

	GetWindowRect(&GetDllShareData().m_common.others.rcFavoriteDialog);

	RECT rc;
	POINT ptNew;
	GetWindowRect(&rc);
	ptNew.x = rc.right - rc.left;
	ptNew.y = rc.bottom - rc.top;

	for (int i=0 ; i<_countof(anchorList); ++i) {
		ResizeItem(GetItemHwnd(anchorList[i].id), m_ptDefaultSize, ptNew, m_rcItems[i], anchorList[i].anchor);
	}

	for (int i=0; i<FAVORITE_INFO_MAX; ++i) {
		HWND hwndList = GetItemHwnd(m_aFavoriteInfo[i].m_nId);
		ResizeItem(hwndList, m_ptDefaultSize, ptNew, m_rcListDefault, AnchorStyle::All, (i == m_nCurrentTab));
	}
	::InvalidateRect(GetHwnd(), NULL, TRUE);
	return TRUE;
}

BOOL DlgFavorite::OnMove(WPARAM wParam, LPARAM lParam)
{
	GetWindowRect(&GetDllShareData().m_common.others.rcFavoriteDialog);
	
	return Dialog::OnMove(wParam, lParam);
}

BOOL DlgFavorite::OnMinMaxInfo(LPARAM lParam)
{
	LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
	if (m_ptDefaultSize.x < 0) {
		return 0;
	}
	lpmmi->ptMinTrackSize.x = m_ptDefaultSize.x;
	lpmmi->ptMinTrackSize.y = m_ptDefaultSize.y;
	lpmmi->ptMaxTrackSize.x = m_ptDefaultSize.x * 2;
	lpmmi->ptMaxTrackSize.y = m_ptDefaultSize.y * 2;
	return 0;
}

