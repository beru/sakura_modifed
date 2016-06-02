/*!	@file
	@brief プラグイン設定ダイアログボックス

	@author Uchi
	@date 2010/3/22
*/
/*
	Copyright (C) 2010, Uchi

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
#include <limits.h>
#include "dlg/DlgPluginOption.h"
#include "prop/PropCommon.h"
#include "util/shell.h"
#include "util/window.h"
#include "util/string_ex2.h"
#include "util/module.h"
#include "sakura_rc.h"
#include "sakura.hh"

// BOOL変数の表示
#define	BOOL_DISP_TRUE	_T("\u2611")
#define	BOOL_DISP_FALSE	_T("\u2610")

// 編集領域を表示、非表示にする
static inline void CtrlShow(
	HWND hwndDlg,
	int id,
	BOOL bShow
	)
{
	HWND hWnd = ::GetDlgItem(hwndDlg, id);
	::ShowWindow(hWnd, bShow ? SW_SHOW: SW_HIDE);
	::EnableWindow(hWnd, bShow);
}

const DWORD p_helpids[] = {
	IDC_LIST_PLUGIN_OPTIONS,		HIDC_LIST_PLUGIN_OPTIONS,		// オプションリスト
	IDC_EDIT_PLUGIN_OPTION,			HIDC_EDIT_PLUGIN_OPTION,		// オプション編集
	IDC_EDIT_PLUGIN_OPTION_DIR,		HIDC_EDIT_PLUGIN_OPTION,		// オプション編集
	IDC_BUTTON_PLUGIN_OPTION_DIR,	HIDC_EDIT_PLUGIN_OPTION,		// オプション編集
	IDC_EDIT_PLUGIN_OPTION_NUM,		HIDC_EDIT_PLUGIN_OPTION,		// オプション編集
	IDC_SPIN_PLUGIN_OPTION,			HIDC_EDIT_PLUGIN_OPTION,		// オプション編集
	IDC_CHECK_PLUGIN_OPTION,		HIDC_EDIT_PLUGIN_OPTION,		// オプション編集
	IDC_COMBO_PLUGIN_OPTION,		HIDC_EDIT_PLUGIN_OPTION,		// オプション編集
	IDOK,							HIDC_FAVORITE_IDOK,				// OK
	IDCANCEL,						HIDC_FAVORITE_IDCANCEL,			// キャンセル
	IDC_PLUGIN_README,				HIDC_PLUGIN_README,				// ReadMe
	IDC_BUTTON_HELP,				HIDC_BUTTON_FAVORITE_HELP,		// ヘルプ
//	IDC_STATIC,						-1,
	0, 0
};

DlgPluginOption::DlgPluginOption(PropPlugin& propPlugin)
	:
	propPlugin(propPlugin)
{

}

DlgPluginOption::~DlgPluginOption()
{

}

// モーダルダイアログの表示
INT_PTR DlgPluginOption::DoModal(
	HINSTANCE			hInstance,
	HWND				hwndParent,
	int 				ID
	)
{
	// プラグイン番号（エディタがふる番号）
	id = ID;
	plugin = PluginManager::getInstance().GetPlugin(id);
	if (!plugin) {
		::ErrorMessage(hwndParent, LS(STR_DLGPLUGINOPT_LOAD));
		return 0;
	}

	return Dialog::DoModal(hInstance, hwndParent, IDD_PLUGIN_OPTION, (LPARAM)NULL);
}

// ダイアログデータの設定
void DlgPluginOption::SetData(void)
{
	HWND	hwndList;
	int		i;
	LV_ITEM	lvi;
	TCHAR	buf[MAX_LENGTH_VALUE + 1];
	bool bLoadDefault = false;

	// タイトル
	auto_sprintf(buf, LS(STR_DLGPLUGINOPT_TITLE), plugin->sName.c_str());
	::SetWindowText(GetHwnd(), buf);

	// リスト
	hwndList = GetItemHwnd(IDC_LIST_PLUGIN_OPTIONS);
	ListView_DeleteAllItems(hwndList);	// リストを空にする
	line = -1;							// 行非選択

	auto profile = std::make_unique<DataProfile>();
	profile->SetReadingMode();
	profile->ReadProfile(plugin->GetOptionPath().c_str());

	PluginOption* pOpt;
	PluginOption::ArrayIter it;
	for (i=0, it=plugin->options.begin(); it!=plugin->options.end(); ++i, ++it) {
		pOpt = *it;

		auto_snprintf_s(buf, _countof(buf), _T("%ls"), pOpt->GetLabel().c_str());
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = buf;
		lvi.iItem    = i;
		lvi.iSubItem = 0;
		lvi.lParam   = pOpt->GetIndex();
		ListView_InsertItem(hwndList, &lvi);

		wstring sSection;
		wstring sKey;
		wstring sValue;
		wstring sType;

		pOpt->GetKey(&sSection, &sKey);
		if (sSection.empty() || sKey.empty()) {
			sValue = L"";
		}else {
			if (!profile->IOProfileData(sSection.c_str(), sKey.c_str(), sValue)) {
				// Optionが見つからなかったらDefault値を設定
				sValue = pOpt->GetDefaultVal();
				if (sValue != wstring(L"")) {
					bLoadDefault = true;
					profile->SetWritingMode();
					profile->IOProfileData(sSection.c_str(), sKey.c_str(), sValue);
					profile->SetReadingMode();
				}
			}
		}

		if (pOpt->GetType() == OPTION_TYPE_BOOL) {
			_tcscpy(buf, sValue == wstring(L"0") || sValue == wstring(L"") ? BOOL_DISP_FALSE : BOOL_DISP_TRUE);
		}else if (pOpt->GetType() == OPTION_TYPE_INT) {
			// 数値へ正規化
			auto_sprintf( buf, _T("%d"), _wtoi(sValue.c_str()));
		}else if (pOpt->GetType() == OPTION_TYPE_SEL) {
			// 値から表示へ
			wstring	sView;
			wstring	sTrg;
			std::vector<wstring> selects = pOpt->GetSelects();
			buf[0] = 0;
			for (auto it=selects.begin(); it!=selects.end(); ++it) {
				SepSelect(*it, &sView, &sTrg);
				if (sValue == sTrg) {
					auto_snprintf_s(buf, _countof(buf), _T("%ls"), sView.c_str());
					break;
				}
			}
		}else {
			auto_snprintf_s(buf, _countof(buf), _T("%ls"), sValue.c_str());
		}
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = i;
		lvi.iSubItem = 1;
		lvi.pszText  = buf;
		ListView_SetItem(hwndList, &lvi);
		ListView_SetItemState(hwndList, i, 0, LVIS_SELECTED | LVIS_FOCUSED);
	}

	if (bLoadDefault) {
		profile->SetWritingMode();
		profile->WriteProfile(plugin->GetOptionPath().c_str(), (plugin->sName + LSW(STR_DLGPLUGINOPT_INIHEAD)).c_str());
	}

	if (i == 0) {
		// オプションが無い
		EnableItem(IDC_LIST_PLUGIN_OPTIONS, false);
		EnableItem(IDOK, false);
	
		SetItemText(IDC_STATIC_MSG, LS(STR_DLGPLUGINOPT_OPTION));
	}

	// ReadMe Button
	sReadMeName = propPlugin.GetReadMeFile(to_tchar(pShareData->common.plugin.pluginTable[id].szName));
	EnableItem(IDC_PLUGIN_README, !sReadMeName.empty());
	return;
}

// ダイアログデータの取得
// TRUE==正常  FALSE==入力エラー
int DlgPluginOption::GetData(void)
{
	// .ini ファイルへの書き込み
	LV_ITEM	lvi;

	// リスト
	HWND hwndList = GetItemHwnd(IDC_LIST_PLUGIN_OPTIONS);

	auto profile = std::make_unique<DataProfile>();
	profile->SetReadingMode();
	profile->ReadProfile(plugin->GetOptionPath().c_str());
	profile->SetWritingMode();

	PluginOption* pOpt;
	TCHAR	buf[MAX_LENGTH_VALUE + 1];
	PluginOption::ArrayIter it;
	int i;
	for (i=0, it=plugin->options.begin(); it!=plugin->options.end(); ++i, ++it) {
		pOpt = *it;

		memset_raw(&lvi, 0, sizeof(lvi));
		lvi.mask       = LVIF_TEXT;
		lvi.iItem      = i;
		lvi.iSubItem   = 1;
		lvi.pszText    = buf;
		lvi.cchTextMax = MAX_LENGTH_VALUE + 1;
		ListView_GetItem(hwndList, &lvi);

		if (pOpt->GetType() == OPTION_TYPE_BOOL) {
			if (_tcscmp(buf,  BOOL_DISP_FALSE) == 0) {
				_tcscpy (buf, _T("0"));
			}else {
				_tcscpy (buf, _T("1"));
			}
		}else if (pOpt->GetType() == OPTION_TYPE_SEL) {
			// 表示から値へ
			wstring	sView;
			wstring	sTrg;
			std::vector<wstring>	selects;
			selects = pOpt->GetSelects();
			wstring sWbuf = to_wchar(buf);

			for (auto it=selects.begin(); it!=selects.end(); ++it) {
				SepSelect(*it, &sView, &sTrg);
				if (sView == sWbuf) {
					auto_sprintf( buf, _T("%ls"), sTrg.c_str());
					break;
				}
			}
		}

		wstring sSection;
		wstring sKey;
		wstring sValue;

		pOpt->GetKey(&sSection, &sKey);
		if (sSection.empty() || sKey.empty()) {
			continue;
		}

		sValue = to_wchar(buf);

		profile->IOProfileData(sSection.c_str(), sKey.c_str(), sValue);
	}

	profile->WriteProfile(
		plugin->GetOptionPath().c_str(),
		(plugin->sName + LSW(STR_DLGPLUGINOPT_INIHEAD)).c_str()
		);

	return TRUE;
}

BOOL DlgPluginOption::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	LV_COLUMN	col;
	RECT		rc;
	long		lngStyle;

	_SetHwnd(hwndDlg);

	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_PLUGIN_OPTIONS);
	::GetWindowRect(hwndList, &rc);

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 40 / 100;
	col.pszText  = const_cast<TCHAR*>(LS(STR_DLGPLUGINOPT_LIST1));
	col.iSubItem = 0;
	ListView_InsertColumn(hwndList, 0, &col);

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 55 / 100;
	col.pszText  = const_cast<TCHAR*>(LS(STR_DLGPLUGINOPT_LIST2));
	col.iSubItem = 1;
	ListView_InsertColumn(hwndList, 1, &col);

	// 行選択
	lngStyle = ListView_GetExtendedListViewStyle(hwndList);
	lngStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle(hwndList, lngStyle);

	// 編集領域の非アクティブ化
	::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_PLUGIN_OPTION), FALSE);
	CtrlShow(hwndDlg, IDC_EDIT_PLUGIN_OPTION_DIR,  FALSE);
	CtrlShow(hwndDlg, IDC_BUTTON_PLUGIN_OPTION_DIR,FALSE);
	CtrlShow(hwndDlg, IDC_EDIT_PLUGIN_OPTION_NUM,  FALSE);
	CtrlShow(hwndDlg, IDC_SPIN_PLUGIN_OPTION,      FALSE);
	CtrlShow(hwndDlg, IDC_CHECK_PLUGIN_OPTION,     FALSE);
	CtrlShow(hwndDlg, IDC_COMBO_PLUGIN_OPTION,     FALSE);

	// 桁数制限
	EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_PLUGIN_OPTION   ), MAX_LENGTH_VALUE);
	EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_PLUGIN_OPTION_DIR), _MAX_PATH);
	EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_PLUGIN_OPTION_NUM), 11);

	// 基底クラスメンバ
	return Dialog::OnInitDialog(GetHwnd(), wParam, lParam);
}

BOOL DlgPluginOption::OnNotify(WPARAM wParam, LPARAM lParam)
{
	NMHDR* pNMHDR;
	int idCtrl = (int)wParam;
	switch (idCtrl) {
	case IDC_LIST_PLUGIN_OPTIONS:
		pNMHDR = (NMHDR*)lParam;
		switch (pNMHDR->code) {
		case LVN_ITEMCHANGED:
			ChangeListPosition();
			break;
		case NM_DBLCLK:
			// リストビューへのダブルクリックで編集領域へ移動	2013/5/23 Uchi
			MoveFocusToEdit();
			break;
		}
		return TRUE;

	case IDC_SPIN_PLUGIN_OPTION:
		int			nVal;
		NM_UPDOWN*	pMNUD;
		
		pMNUD  = (NM_UPDOWN*)lParam;

		nVal = GetItemInt(IDC_EDIT_PLUGIN_OPTION_NUM, NULL, TRUE);
		if (pMNUD->iDelta < 0) {
			if (nVal < INT_MAX) ++nVal;
		}else if (pMNUD->iDelta > 0) {
			// INT_MINは SetDlgItemInt で扱えない
			if (nVal > -INT_MAX) --nVal;
		}
		SetItemInt(IDC_EDIT_PLUGIN_OPTION_NUM, nVal, TRUE);

		// 編集中のデータの戻し
		SetFromEdit(line);
		return TRUE;
	}

	// 基底クラスメンバ
	return Dialog::OnNotify(wParam, lParam);
}


BOOL DlgPluginOption::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_CHECK_PLUGIN_OPTION:
		// 編集中のデータの戻し
		SetFromEdit(line);
		return TRUE;

	case IDC_BUTTON_PLUGIN_OPTION_DIR:	// 2013/05/22 Uchi
		// ディレクトリ選択
		SelectDirectory(line);
		return TRUE;

	case IDC_PLUGIN_README:		// 2012/12/22 Uchi
		// ReadMe
		{
			if (!sReadMeName.empty()) {
				if (!propPlugin.BrowseReadMe(sReadMeName)) {
					WarningMessage(GetHwnd(), LS(STR_PROPCOMPLG_ERR2));
				}
			}else {
				WarningMessage(GetHwnd(), LS(STR_PROPCOMPLG_ERR3));
			}
		}
		return TRUE;

	case IDC_BUTTON_HELP:
		// ヘルプ
		MyWinHelp(GetHwnd(), HELP_CONTEXT, HLP000153);	// 『プラグイン設定』Helpの指定 	2011/11/26 Uchi
		return TRUE;

	case IDOK:
		// 編集中のデータの戻し
		SetFromEdit(line);
		// ダイアログデータの取得
		::EndDialog(GetHwnd(), (BOOL)GetData());
		return TRUE;

	case IDCANCEL:
		::EndDialog(GetHwnd(), FALSE);
		return TRUE;
	}

	// 基底クラスメンバ
	return Dialog::OnBnClicked(wID);
}

BOOL DlgPluginOption::OnCbnSelChange(HWND hwndCtl, int wID)
{
	switch (wID) {
	case IDC_COMBO_PLUGIN_OPTION:
		// 編集中のデータの戻し
		SetFromEdit(line);
		return TRUE;
	}

	// 基底クラスメンバ
	return Dialog::OnCbnSelChange(hwndCtl, wID);
}


BOOL DlgPluginOption::OnEnChange(HWND hwndCtl, int wID)
{
	switch (wID) {
	case IDC_EDIT_PLUGIN_OPTION:
	case IDC_EDIT_PLUGIN_OPTION_DIR:
	case IDC_EDIT_PLUGIN_OPTION_NUM:
		// 編集中のデータの戻し
		SetFromEdit(line);
		return TRUE;
	}

	// 基底クラスメンバ
	return Dialog::OnEnChange(hwndCtl, wID);
}


BOOL DlgPluginOption::OnActivate(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam)) {
	case WA_INACTIVE:
		SetFromEdit(line);
		break;
	case WA_ACTIVE:
	case WA_CLICKACTIVE:
	default:
		break;
	}

	// 基底クラスメンバ
	return Dialog::OnActivate(wParam, lParam);
}


LPVOID DlgPluginOption::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}


void DlgPluginOption::ChangeListPosition(void)
{
	HWND hwndList = GetItemHwnd(IDC_LIST_PLUGIN_OPTIONS);

	// 現在のFocus取得
	int current = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED);
	if (current == -1 || current == line) {
		return;
	}

	TCHAR	buf[MAX_LENGTH_VALUE + 1];
	LVITEM	lvi;

	// 戻し
	if (line >= 0) {
		SetFromEdit(line);
	}

	line = current;

	// 編集領域に書き込み
	SetToEdit(current);

	memset_raw(&lvi, 0, sizeof(lvi));
	lvi.mask       = LVIF_TEXT;
	lvi.iItem      = current;
	lvi.iSubItem   = 1;
	lvi.pszText    = buf;
	lvi.cchTextMax = MAX_LENGTH_VALUE + 1;

	ListView_GetItem(hwndList, &lvi);
	SetItemText(IDC_EDIT_PLUGIN_OPTION, buf);
}

void DlgPluginOption::MoveFocusToEdit(void)
{
	// 現在のFocus取得
	int iLine = ListView_GetNextItem(GetItemHwnd(IDC_LIST_PLUGIN_OPTIONS), -1, LVNI_SELECTED);
	wstring	sType;
	HWND hwndCtrl;

	if (iLine >= 0) {
		// Focusの切り替え
		sType = plugin->options[iLine]->GetType();
		transform(sType.begin(), sType.end(), sType.begin(), my_towlower2);
		if (sType == OPTION_TYPE_BOOL) {
			hwndCtrl = GetItemHwnd(IDC_CHECK_PLUGIN_OPTION);
		}else if (sType == OPTION_TYPE_INT) {
			hwndCtrl = GetItemHwnd(IDC_EDIT_PLUGIN_OPTION_NUM);
		}else if (sType == OPTION_TYPE_SEL) {
			hwndCtrl = GetItemHwnd(IDC_COMBO_PLUGIN_OPTION);
		}else if (sType == OPTION_TYPE_DIR) {
			hwndCtrl = GetItemHwnd(IDC_EDIT_PLUGIN_OPTION_DIR);
		}else {
			hwndCtrl = GetItemHwnd(IDC_EDIT_PLUGIN_OPTION);
		}
		::SetFocus(hwndCtrl);
	}
}


// 編集領域に書き込み
void DlgPluginOption::SetToEdit(int iLine)
{
	HWND hwndList = GetItemHwnd(IDC_LIST_PLUGIN_OPTIONS);

	TCHAR buf[MAX_LENGTH_VALUE + 1];
	LVITEM lvi;
	wstring	sType;

	if (iLine >= 0) {
		GetItemText(IDC_EDIT_PLUGIN_OPTION, buf, MAX_LENGTH_VALUE + 1);
		memset_raw(&lvi, 0, sizeof(lvi));
		lvi.mask       = LVIF_TEXT;
		lvi.iItem      = iLine;
		lvi.iSubItem   = 1;
		lvi.pszText    = buf;
		lvi.cchTextMax = MAX_LENGTH_VALUE + 1;
		ListView_GetItem(hwndList, &lvi);

		sType = plugin->options[iLine]->GetType();
		transform(sType.begin(), sType.end(), sType.begin(), my_towlower2);
		if (sType == OPTION_TYPE_BOOL) {
			CheckButton(IDC_CHECK_PLUGIN_OPTION, _tcscmp(buf,  BOOL_DISP_FALSE) != 0);
			SetItemText(IDC_CHECK_PLUGIN_OPTION, plugin->options[iLine]->GetLabel().c_str());

			// 編集領域の切り替え
			SelectEdit(IDC_CHECK_PLUGIN_OPTION);
		}else if (sType == OPTION_TYPE_INT) {
			SetItemText(IDC_EDIT_PLUGIN_OPTION_NUM, buf);

			// 編集領域の切り替え
			SelectEdit(IDC_EDIT_PLUGIN_OPTION_NUM);
		}else if (sType == OPTION_TYPE_SEL) {
			// CONBO 設定
			std::vector<wstring> selects;
			selects = plugin->options[iLine]->GetSelects();

			HWND hwndCombo = GetItemHwnd(IDC_COMBO_PLUGIN_OPTION);
			Combo_ResetContent(hwndCombo);

			wstring	sView;
			wstring	sValue;
			wstring	sWbuf = to_wchar(buf);
			int nSelIdx = -1;		// 選択
			int i = 0;
			for (auto it=selects.begin(); it!=selects.end(); ++it) {
				SepSelect(*it, &sView, &sValue);
				LONG_PTR nItemIdx = Combo_AddString(hwndCombo, sView.c_str());
				if (sView == sWbuf) {
					nSelIdx = i;
				}
				Combo_SetItemData(hwndCombo, nItemIdx, i++);
			}
			Combo_SetCurSel(hwndCombo, nSelIdx);

			// 編集領域の切り替え
			SelectEdit(IDC_COMBO_PLUGIN_OPTION);
		}else if (sType == OPTION_TYPE_DIR) {
			SetItemText(IDC_EDIT_PLUGIN_OPTION_DIR, buf);

			// 編集領域の切り替え
			SelectEdit(IDC_EDIT_PLUGIN_OPTION_DIR);
		}else {
			SetItemText(IDC_EDIT_PLUGIN_OPTION, buf);

			// 編集領域の切り替え
			SelectEdit(IDC_EDIT_PLUGIN_OPTION);
		}
	}
}

// 編集領域の切り替え
void DlgPluginOption::SelectEdit(int IDCenable)
{
	CtrlShow(GetHwnd(), IDC_EDIT_PLUGIN_OPTION,        (IDCenable == IDC_EDIT_PLUGIN_OPTION));
	CtrlShow(GetHwnd(), IDC_EDIT_PLUGIN_OPTION_DIR,    (IDCenable == IDC_EDIT_PLUGIN_OPTION_DIR));
	CtrlShow(GetHwnd(), IDC_BUTTON_PLUGIN_OPTION_DIR,  (IDCenable == IDC_EDIT_PLUGIN_OPTION_DIR));
	CtrlShow(GetHwnd(), IDC_EDIT_PLUGIN_OPTION_NUM,    (IDCenable == IDC_EDIT_PLUGIN_OPTION_NUM));
	CtrlShow(GetHwnd(), IDC_SPIN_PLUGIN_OPTION,        (IDCenable == IDC_EDIT_PLUGIN_OPTION_NUM));
	CtrlShow(GetHwnd(), IDC_CHECK_PLUGIN_OPTION,       (IDCenable == IDC_CHECK_PLUGIN_OPTION));
	CtrlShow(GetHwnd(), IDC_COMBO_PLUGIN_OPTION,       (IDCenable == IDC_COMBO_PLUGIN_OPTION));
}

// 編集領域から戻し
void DlgPluginOption::SetFromEdit(int iLine)
{
	HWND hwndList = GetItemHwnd(IDC_LIST_PLUGIN_OPTIONS);

	TCHAR	buf[MAX_LENGTH_VALUE + 1];
	int		nVal;
	LVITEM	lvi;
	wstring	sType;

	if (iLine >= 0) {
		sType = plugin->options[iLine]->GetType();
		transform(sType.begin (), sType.end (), sType.begin (), my_towlower2);
		if (sType == OPTION_TYPE_BOOL) {
			if (IsButtonChecked(IDC_CHECK_PLUGIN_OPTION)) {
				_tcscpy(buf, BOOL_DISP_TRUE);
			}else {
				_tcscpy(buf, BOOL_DISP_FALSE);
			}
			lvi.mask     = LVIF_TEXT;
			lvi.iItem    = iLine;
			lvi.iSubItem = 1;
			lvi.pszText  = buf;
			ListView_SetItem(hwndList, &lvi);
		}else if (sType == OPTION_TYPE_INT) {
			nVal = GetItemInt(IDC_EDIT_PLUGIN_OPTION_NUM, NULL, TRUE);
			auto_sprintf( buf, _T("%d"), nVal);
		}else if (sType == OPTION_TYPE_SEL) {
			GetItemText(IDC_COMBO_PLUGIN_OPTION, buf, MAX_LENGTH_VALUE + 1);
		}else if (sType == OPTION_TYPE_DIR) {
			GetItemText(IDC_EDIT_PLUGIN_OPTION_DIR, buf, MAX_LENGTH_VALUE + 1);
		}else {
			GetItemText(IDC_EDIT_PLUGIN_OPTION, buf, MAX_LENGTH_VALUE + 1);
		}
		memset_raw(&lvi, 0, sizeof(lvi));
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = iLine;
		lvi.iSubItem = 1;
		lvi.pszText  = buf;
		ListView_SetItem(hwndList, &lvi);
	}
}

// 選択用文字列分解
void DlgPluginOption::SepSelect(
	wstring sTrg,
	wstring* spView,
	wstring* spValue
	)
{
	auto ix = sTrg.find(L':');
	if (ix == std::wstring::npos) {
		*spView = *spValue = sTrg;
	}else {
#ifdef _DEBUG
		*spView  = sTrg;
#else
		*spView  = sTrg.substr(0, ix);
#endif
		*spValue = sTrg.substr(ix + 1);
	}
}

// ディレクトリを選択する
void DlgPluginOption::SelectDirectory(int iLine)
{
	TCHAR szDir[_MAX_PATH + 1];

	// 検索フォルダ
	GetItemText(IDC_EDIT_PLUGIN_OPTION_DIR, szDir, _countof(szDir));
	if (_IS_REL_PATH(szDir)) {
		TCHAR	folder[_MAX_PATH];
		_tcscpy(folder, szDir);
		GetInidirOrExedir(szDir, folder);
	}

	// 項目名の取得
	HWND hwndList = GetItemHwnd(IDC_LIST_PLUGIN_OPTIONS);
	LVITEM	lvi;
	TCHAR buf[MAX_LENGTH_VALUE + 1];
	memset_raw( &lvi, 0, sizeof( lvi ));
	lvi.mask       = LVIF_TEXT;
	lvi.iItem      = iLine;
	lvi.iSubItem   = 0;
	lvi.pszText    = buf;
	lvi.cchTextMax = MAX_LENGTH_VALUE + 1;
	ListView_GetItem(hwndList, &lvi);

	TCHAR sTitle[MAX_LENGTH_VALUE + 10];
	auto_sprintf(sTitle, LS(STR_DLGPLUGINOPT_SELECT), buf);
	if (SelectDir(GetHwnd(), (const TCHAR*)sTitle /*_T("ディレクトリの選択")*/, szDir, szDir)) {
		// 末尾に\マークを追加する．
		AddLastChar(szDir, _countof(szDir), _T('\\'));
		SetItemText(IDC_EDIT_PLUGIN_OPTION_DIR, szDir);
	}
}

