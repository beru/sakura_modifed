/*!	@file
	@brief 共通設定ダイアログボックス、「全般」ページ

	@author Uchi
	@date 2010/5/9 CPropCommon.cより分離
*/
/*
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "CPropertyManager.h"
#include "recent/CMRUFile.h"
#include "recent/CMRUFolder.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

//@@@ 2001.02.04 Start by MIK: Popup Help
TYPE_NAME_ID<int> SpecialScrollModeArr[] = {
	{ 0,									STR_SCROLL_WITH_NO_KEY },		// _T("組み合わせなし") },
	{ (int)MouseFunctionType::CenterClick,		STR_SCROLL_WITH_MID_BTN },		// _T("マウス中ボタン") },
	{ (int)MouseFunctionType::LeftSideClick,	STR_SCROLL_WITH_SIDE_1_BTN },	// _T("マウスサイドボタン1") },
	{ (int)MouseFunctionType::RightSideClick,	STR_SCROLL_WITH_SIDE_2_BTN },	// _T("マウスサイドボタン2") },
	{ VK_CONTROL,							STR_SCROLL_WITH_CTRL_KEY },		// _T("CONTROLキー") },
	{ VK_SHIFT,								STR_SCROLL_WITH_SHIFT_KEY },	// _T("SHIFTキー") },
};

static const DWORD p_helpids[] = {	//10900
	IDC_BUTTON_CLEAR_MRU_FILE,		HIDC_BUTTON_CLEAR_MRU_FILE,			// 履歴をクリア（ファイル）
	IDC_BUTTON_CLEAR_MRU_FOLDER,	HIDC_BUTTON_CLEAR_MRU_FOLDER,		// 履歴をクリア（フォルダ）
	IDC_CHECK_FREECARET,			HIDC_CHECK_FREECARET,				// フリーカーソル
//DEL	IDC_CHECK_INDENT,				HIDC_CHECK_INDENT,				// 自動インデント ：タイプ別へ移動
//DEL	IDC_CHECK_INDENT_WSPACE,		HIDC_CHECK_INDENT_WSPACE,		// 全角空白もインデント ：タイプ別へ移動
	IDC_CHECK_USETRAYICON,			HIDC_CHECK_USETRAYICON,				// タスクトレイを使う
	IDC_CHECK_STAYTASKTRAY,			HIDC_CHECK_STAYTASKTRAY,			// タスクトレイに常駐
	IDC_CHECK_REPEATEDSCROLLSMOOTH,	HIDC_CHECK_REPEATEDSCROLLSMOOTH,	// 少し滑らかにする
	IDC_CHECK_CLOSEALLCONFIRM,		HIDC_CHECK_CLOSEALLCONFIRM,			// [すべて閉じる]で他に編集用のウィンドウがあれば確認する	// 2006.12.25 ryoji
	IDC_CHECK_EXITCONFIRM,			HIDC_CHECK_EXITCONFIRM,				// 終了の確認
	IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_WORD, HIDC_CHECK_STOPS_WORD,	// 単語単位で移動するときに単語の両端に止まる
	IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_PARAGRAPH, HIDC_CHECK_STOPS_PARAGRAPH,	// 段落単位で移動するときに段落の両端に止まる
	IDC_CHECK_NOMOVE_ACTIVATE_BY_MOUSE, HIDC_CHECK_NOMOVE_ACTIVATE_BY_MOUSE,		// マウスクリックでアクティブになったときはカーソルをクリック位置に移動しない 2007.10.08 genta
	IDC_HOTKEY_TRAYMENU,			HIDC_HOTKEY_TRAYMENU,				// 左クリックメニューのショートカットキー
	IDC_EDIT_REPEATEDSCROLLLINENUM,	HIDC_EDIT_REPEATEDSCROLLLINENUM,	// スクロール行数
	IDC_EDIT_MAX_MRU_FILE,			HIDC_EDIT_MAX_MRU_FILE,				// ファイル履歴の最大数
	IDC_EDIT_MAX_MRU_FOLDER,		HIDC_EDIT_MAX_MRU_FOLDER,			// フォルダ履歴の最大数
	IDC_RADIO_CARETTYPE0,			HIDC_RADIO_CARETTYPE0,				// カーソル形状（Windows風）
	IDC_RADIO_CARETTYPE1,			HIDC_RADIO_CARETTYPE1,				// カーソル形状（MS-DOS風）
	IDC_SPIN_REPEATEDSCROLLLINENUM,	HIDC_EDIT_REPEATEDSCROLLLINENUM,
	IDC_SPIN_MAX_MRU_FILE,			HIDC_EDIT_MAX_MRU_FILE,
	IDC_SPIN_MAX_MRU_FOLDER,		HIDC_EDIT_MAX_MRU_FOLDER,
	IDC_CHECK_MEMDC,				HIDC_CHECK_MEMDC,					// 画面キャッシュを使う
	IDC_COMBO_WHEEL_PAGESCROLL,		HIDC_COMBO_WHEEL_PAGESCROLL,		// 組み合わせてホイール操作した時ページスクロールする		// 2009.01.17 nasukoji
	IDC_COMBO_WHEEL_HSCROLL,		HIDC_COMBO_WHEEL_HSCROLL,			// 組み合わせてホイール操作した時横スクロールする			// 2009.01.17 nasukoji
//	IDC_STATIC,						-1,
	0, 0
};
//@@@ 2001.02.04 End

/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK PropGeneral::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropGeneral::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}


// General メッセージ処理
INT_PTR PropGeneral::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
	)
{
	WORD		wNotifyCode;
	WORD		wID;
//	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
	int			nVal;
//	LPDRAWITEMSTRUCT pDis;

	switch (uMsg) {
	case WM_INITDIALOG:
		// ダイアログデータの設定 General
		SetData(hwndDlg);
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
		// ユーザーがエディット コントロールに入力できるテキストの長さを制限する
		return TRUE;
		
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	// 通知コード
		wID			= LOWORD(wParam);	// 項目ID､ コントロールID､ またはアクセラレータID
//		hwndCtl		= (HWND) lParam;	// コントロールのハンドル
		switch (wNotifyCode) {
		// ボタン／チェックボックスがクリックされた
		case BN_CLICKED:
			switch (wID) {
			case IDC_CHECK_USETRAYICON:	// タスクトレイを使う
			// From Here 2001.12.03 hor
			//		操作しにくいって評判だったのでタスクトレイ関係のEnable制御をやめました
			//@@@ YAZAKI 2001.12.31 IDC_CHECKSTAYTASKTRAYのアクティブ、非アクティブのみ制御。
				if (DlgButton_IsChecked(hwndDlg, IDC_CHECK_USETRAYICON)) {
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_STAYTASKTRAY), TRUE);
				}else {
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_STAYTASKTRAY), FALSE);
				}
			// To Here 2001.12.03 hor
				return TRUE;

			case IDC_CHECK_STAYTASKTRAY:	// タスクトレイに常駐
				return TRUE;

			case IDC_BUTTON_CLEAR_MRU_FILE:
				// ファイルの履歴をクリア
				if (IDCANCEL == ::MYMESSAGEBOX(hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
					LS(STR_PROPCOMGEN_FILE1))
				) {
					return TRUE;
				}
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
//				m_pShareData->m_sHistory.m_nMRUArrNum = 0;
				{
					MRUFile cMRU;
					cMRU.ClearAll();
				}
				InfoMessage(hwndDlg, LS(STR_PROPCOMGEN_FILE2));
				return TRUE;
			case IDC_BUTTON_CLEAR_MRU_FOLDER:
				// フォルダの履歴をクリア
				if (IDCANCEL == ::MYMESSAGEBOX(hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
					LS(STR_PROPCOMGEN_DIR1))
				) {
					return TRUE;
				}
//@@@ 2001.12.26 YAZAKI OPENFOLDERリストは、MRUFolderにすべて依頼する
//				m_pShareData->m_sHistory.m_nOPENFOLDERArrNum = 0;
				{
					MRUFolder cMRUFolder;	//	MRUリストの初期化。ラベル内だと問題あり？
					cMRUFolder.ClearAll();
				}
				InfoMessage(hwndDlg, LS(STR_PROPCOMGEN_DIR2));
				return TRUE;
			}
			break;	// BN_CLICKED
		// 2009.01.12 nasukoji	コンボボックスのリストの項目が選択された
		case CBN_SELENDOK:
			HWND	hwndCombo;
			int		nSelPos;

			switch (wID) {
			// 組み合わせてホイール操作した時ページスクロールする
			case IDC_COMBO_WHEEL_PAGESCROLL:
				hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_WHEEL_PAGESCROLL);
				nSelPos = Combo_GetCurSel(hwndCombo);
				hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_WHEEL_HSCROLL);
				if (nSelPos && nSelPos == Combo_GetCurSel(hwndCombo)) {
					Combo_SetCurSel(hwndCombo, 0);
				}
				return TRUE;
			// 組み合わせてホイール操作した時横スクロールする
			case IDC_COMBO_WHEEL_HSCROLL:
				hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_WHEEL_HSCROLL);
				nSelPos = Combo_GetCurSel(hwndCombo);
				hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_WHEEL_PAGESCROLL);
				if (nSelPos && nSelPos == Combo_GetCurSel(hwndCombo)) {
					Combo_SetCurSel(hwndCombo, 0);
				}
				return TRUE;
			}
			break;	// CBN_SELENDOK
		}
		break;	// WM_COMMAND
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch (idCtrl) {
		case IDC_SPIN_REPEATEDSCROLLLINENUM:
			// キーリピート時のスクロール行数
//			MYTRACE(_T("IDC_SPIN_REPEATEDSCROLLLINENUM\n"));
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 1) {
				nVal = 1;
			}
			if (nVal > 10) {
				nVal = 10;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_MAX_MRU_FILE:
			// ファイルの履歴MAX
//			MYTRACE(_T("IDC_SPIN_MAX_MRU_FILE\n"));
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_MAX_MRU_FILE, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 0) {
				nVal = 0;
			}
			if (nVal > MAX_MRU) {
				nVal = MAX_MRU;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_MAX_MRU_FILE, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_MAX_MRU_FOLDER:
			// フォルダの履歴MAX
//			MYTRACE(_T("IDC_SPIN_MAX_MRU_FOLDER\n"));
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_MAX_MRU_FOLDER, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 0) {
				nVal = 0;
			}
			if (nVal > MAX_OPENFOLDER) {
				nVal = MAX_OPENFOLDER;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_MAX_MRU_FOLDER, nVal, FALSE);
			return TRUE;
		default:
			switch (pNMHDR->code) {
			case PSN_HELP:
				OnHelp(hwndDlg, IDD_PROP_GENERAL);
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE(_T("General PSN_KILLACTIVE\n"));
				// ダイアログデータの取得 General
				GetData(hwndDlg);
				return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			case PSN_SETACTIVE:
				m_nPageNum = ID_PROPCOM_PAGENUM_GENERAL;	// Oct. 25, 2000 JEPRO ZENPAN1→ZENPAN に変更(参照しているのはCPropCommon.cppのみの1箇所)
				return TRUE;
			}
			break;
		}

//		MYTRACE(_T("pNMHDR->hwndFrom=%xh\n"), pNMHDR->hwndFrom);
//		MYTRACE(_T("pNMHDR->idFrom  =%xh\n"), pNMHDR->idFrom);
//		MYTRACE(_T("pNMHDR->code    =%xh\n"), pNMHDR->code);
//		MYTRACE(_T("pMNUD->iPos    =%d\n"), pMNUD->iPos);
//		MYTRACE(_T("pMNUD->iDelta  =%d\n"), pMNUD->iDelta);
		break;

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids);	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		// NOTREACHED
//		break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids);	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}


// ダイアログデータの設定 General
void PropGeneral::SetData(HWND hwndDlg)
{
	auto& csGeneral = m_common.m_general;

	// カーソルのタイプ 0=win 1=dos 
	if (csGeneral.GetCaretType() == 0) {
		::CheckDlgButton(hwndDlg, IDC_RADIO_CARETTYPE0, TRUE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_CARETTYPE1, FALSE);
	}else {
		::CheckDlgButton(hwndDlg, IDC_RADIO_CARETTYPE0, FALSE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_CARETTYPE1, TRUE);
	}

	// フリーカーソルモード
	::CheckDlgButton(hwndDlg, IDC_CHECK_FREECARET, csGeneral.m_bIsFreeCursorMode ? 1 : 0);

	// 単語単位で移動するときに、単語の両端で止まるか
	::CheckDlgButton(hwndDlg, IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_WORD, csGeneral.m_bStopsBothEndsWhenSearchWord);

	// 段落単位で移動するときに、段落の両端で止まるか
	::CheckDlgButton(hwndDlg, IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_PARAGRAPH, csGeneral.m_bStopsBothEndsWhenSearchParagraph);

	//	2007.10.08 genta マウスクリックでアクティブになったときはカーソルをクリック位置に移動しない (2007.10.02 by nasukoji)
	::CheckDlgButton(hwndDlg, IDC_CHECK_NOMOVE_ACTIVATE_BY_MOUSE, csGeneral.m_bNoCaretMoveByActivation);

	// [すべて閉じる]で他に編集用のウィンドウがあれば確認する	// 2006.12.25 ryoji
	::CheckDlgButton(hwndDlg, IDC_CHECK_CLOSEALLCONFIRM, csGeneral.m_bCloseAllConfirm);

	// 終了時の確認をする
	::CheckDlgButton(hwndDlg, IDC_CHECK_EXITCONFIRM, csGeneral.m_bExitConfirm);

	// キーリピート時のスクロール行数
	::SetDlgItemInt(hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, (Int)csGeneral.m_nRepeatedScrollLineNum, FALSE);

	// キーリピート時のスクロールを滑らかにするか
	::CheckDlgButton(hwndDlg, IDC_CHECK_REPEATEDSCROLLSMOOTH, csGeneral.m_nRepeatedScroll_Smooth);

	// 2009.01.17 nasukoji	組み合わせてホイール操作した時ページスクロールする
	HWND	hwndCombo;
	int		nSelPos;

	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_WHEEL_PAGESCROLL);
	Combo_ResetContent(hwndCombo);
	nSelPos = 0;
	for (int i=0; i<_countof(SpecialScrollModeArr); ++i) {
		Combo_InsertString(hwndCombo, i, LS(SpecialScrollModeArr[i].nNameId));
		if (SpecialScrollModeArr[i].nMethod == csGeneral.m_nPageScrollByWheel) {	// ページスクロールとする組み合わせ操作
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	// 2009.01.12 nasukoji	組み合わせてホイール操作した時横スクロールする
	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_WHEEL_HSCROLL);
	Combo_ResetContent(hwndCombo);
	nSelPos = 0;
	for (int i=0; i<_countof(SpecialScrollModeArr); ++i) {
		Combo_InsertString(hwndCombo, i, LS(SpecialScrollModeArr[i].nNameId));
		if (SpecialScrollModeArr[i].nMethod == csGeneral.m_nHorizontalScrollByWheel) {	// 横スクロールとする組み合わせ操作
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	// 2007.09.09 Moca 画面キャッシュ設定追加
	// 画面キャッシュを使う
	::CheckDlgButton(hwndDlg, IDC_CHECK_MEMDC, m_common.m_window.m_bUseCompatibleBMP);

	// ファイルの履歴MAX
	::SetDlgItemInt(hwndDlg, IDC_EDIT_MAX_MRU_FILE, csGeneral.m_nMRUArrNum_MAX, FALSE);

	// フォルダの履歴MAX
	::SetDlgItemInt(hwndDlg, IDC_EDIT_MAX_MRU_FOLDER, csGeneral.m_nOPENFOLDERArrNum_MAX, FALSE);

	// タスクトレイを使う
	::CheckDlgButton(hwndDlg, IDC_CHECK_USETRAYICON, csGeneral.m_bUseTaskTray);
// From Here 2001.12.03 hor
//@@@ YAZAKI 2001.12.31 ここは制御する。
	if (csGeneral.m_bUseTaskTray) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_STAYTASKTRAY), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_STAYTASKTRAY), FALSE);
	}
// To Here 2001.12.03 hor
	// タスクトレイに常駐
	::CheckDlgButton(hwndDlg, IDC_CHECK_STAYTASKTRAY, csGeneral.m_bStayTaskTray);

	// タスクトレイ左クリックメニューのショートカット
	HotKey_SetHotKey(::GetDlgItem(hwndDlg, IDC_HOTKEY_TRAYMENU), csGeneral.m_wTrayMenuHotKeyCode, csGeneral.m_wTrayMenuHotKeyMods);

	return;
}


// ダイアログデータの取得 General
int PropGeneral::GetData(HWND hwndDlg)
{
	auto& csGeneral = m_common.m_general;
	
	// カーソルのタイプ 0=win 1=dos 
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_CARETTYPE0)) {
		csGeneral.SetCaretType(0);
	}
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_CARETTYPE1)) {
		csGeneral.SetCaretType(1);
	}

	// フリーカーソルモード
	csGeneral.m_bIsFreeCursorMode = DlgButton_IsChecked(hwndDlg, IDC_CHECK_FREECARET);

	// 単語単位で移動するときに、単語の両端で止まるか
	csGeneral.m_bStopsBothEndsWhenSearchWord = DlgButton_IsChecked(hwndDlg, IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_WORD);
	//	2007.10.08 genta マウスクリックでアクティブになったときはカーソルをクリック位置に移動しない (2007.10.02 by nasukoji)
	csGeneral.m_bNoCaretMoveByActivation = DlgButton_IsChecked(hwndDlg, IDC_CHECK_NOMOVE_ACTIVATE_BY_MOUSE);

	// 段落単位で移動するときに、段落の両端で止まるか
	csGeneral.m_bStopsBothEndsWhenSearchParagraph = DlgButton_IsChecked(hwndDlg, IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_PARAGRAPH);

	// [すべて閉じる]で他に編集用のウィンドウがあれば確認する	// 2006.12.25 ryoji
	csGeneral.m_bCloseAllConfirm = DlgButton_IsChecked(hwndDlg, IDC_CHECK_CLOSEALLCONFIRM);

	// 終了時の確認をする
	csGeneral.m_bExitConfirm = DlgButton_IsChecked(hwndDlg, IDC_CHECK_EXITCONFIRM);

	// キーリピート時のスクロール行数
	csGeneral.m_nRepeatedScrollLineNum = (LayoutInt)::GetDlgItemInt(hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, NULL, FALSE);
	if (csGeneral.m_nRepeatedScrollLineNum < LayoutInt(1)) {
		csGeneral.m_nRepeatedScrollLineNum = LayoutInt(1);
	}
	if (csGeneral.m_nRepeatedScrollLineNum > LayoutInt(10)) {
		csGeneral.m_nRepeatedScrollLineNum = LayoutInt(10);
	}

	// キーリピート時のスクロールを滑らかにするか
	csGeneral.m_nRepeatedScroll_Smooth = DlgButton_IsChecked(hwndDlg, IDC_CHECK_REPEATEDSCROLLSMOOTH);

	// 2009.01.17 nasukoji	組み合わせてホイール操作した時ページスクロールする
	HWND	hwndCombo;
	int		nSelPos;

	// 2007.09.09 Moca 画面キャッシュ設定追加
	// 画面キャッシュを使う
	m_common.m_window.m_bUseCompatibleBMP = DlgButton_IsChecked(hwndDlg, IDC_CHECK_MEMDC);

	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_WHEEL_PAGESCROLL);
	nSelPos = Combo_GetCurSel(hwndCombo);
	csGeneral.m_nPageScrollByWheel = SpecialScrollModeArr[nSelPos].nMethod;		// ページスクロールとする組み合わせ操作

	// 2009.01.17 nasukoji	組み合わせてホイール操作した時横スクロールする
	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_WHEEL_HSCROLL);
	nSelPos = Combo_GetCurSel(hwndCombo);
	csGeneral.m_nHorizontalScrollByWheel = SpecialScrollModeArr[nSelPos].nMethod;	// 横スクロールとする組み合わせ操作

	// ファイルの履歴MAX
	csGeneral.m_nMRUArrNum_MAX = ::GetDlgItemInt(hwndDlg, IDC_EDIT_MAX_MRU_FILE, NULL, FALSE);
	if (csGeneral.m_nMRUArrNum_MAX < 0) {
		csGeneral.m_nMRUArrNum_MAX = 0;
	}
	if (csGeneral.m_nMRUArrNum_MAX > MAX_MRU) {
		csGeneral.m_nMRUArrNum_MAX = MAX_MRU;
	}

	{	// 履歴の管理	//@@@ 2003.04.09 MIK
		RecentFile	cRecentFile;
		cRecentFile.UpdateView();
		cRecentFile.Terminate();
	}

	// フォルダの履歴MAX
	csGeneral.m_nOPENFOLDERArrNum_MAX = ::GetDlgItemInt(hwndDlg, IDC_EDIT_MAX_MRU_FOLDER, NULL, FALSE);
	if (csGeneral.m_nOPENFOLDERArrNum_MAX < 0) {
		csGeneral.m_nOPENFOLDERArrNum_MAX = 0;
	}
	if (csGeneral.m_nOPENFOLDERArrNum_MAX > MAX_OPENFOLDER) {
		csGeneral.m_nOPENFOLDERArrNum_MAX = MAX_OPENFOLDER;
	}

	{	// 履歴の管理	//@@@ 2003.04.09 MIK
		RecentFolder	cRecentFolder;
		cRecentFolder.UpdateView();
		cRecentFolder.Terminate();
	}

	// タスクトレイを使う
	csGeneral.m_bUseTaskTray = DlgButton_IsChecked(hwndDlg, IDC_CHECK_USETRAYICON);
//@@@ YAZAKI 2001.12.31 m_bUseTaskTrayに引きづられるように。
	if (csGeneral.m_bUseTaskTray) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_STAYTASKTRAY), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_STAYTASKTRAY), FALSE);
	}
	// タスクトレイに常駐
	csGeneral.m_bStayTaskTray = DlgButton_IsChecked(hwndDlg, IDC_CHECK_STAYTASKTRAY);

	// タスクトレイ左クリックメニューのショートカット
	LRESULT	lResult;
	lResult = HotKey_GetHotKey(::GetDlgItem(hwndDlg, IDC_HOTKEY_TRAYMENU));
	csGeneral.m_wTrayMenuHotKeyCode = LOBYTE(lResult);
	csGeneral.m_wTrayMenuHotKeyMods = HIBYTE(lResult);

	return TRUE;
}

