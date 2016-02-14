/*! @file
	@brief タイプ別設定 - スクリーン

	@date 2008.04.12 kobake CPropTypes.cppから分離
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, jepro, genta, MIK, hor, Stonee, asa-o
	Copyright (C) 2002, YAZAKI, aroka, MIK, genta, こおり, Moca
	Copyright (C) 2003, MIK, zenryaku, Moca, naoh, KEITA, genta
	Copyright (C) 2005, MIK, genta, Moca, ryoji
	Copyright (C) 2006, ryoji, fon, novice
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2008, nasukoji
	Copyright (C) 2009, ryoji, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CPropTypes.h"
#include "dlg/CDlgOpenFile.h"
#include "util/module.h"
#include "util/shell.h"
#include "util/window.h"
#include "util/file.h" // _IS_REL_PATH
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids1[] = {	//11300
	IDC_EDIT_TYPENAME,				HIDC_EDIT_TYPENAME,			// 設定の名前
	IDC_EDIT_TYPEEXTS,				HIDC_EDIT_TYPEEXTS,			// ファイル拡張子

	IDC_COMBO_WRAPMETHOD,			HIDC_COMBO_WRAPMETHOD,		// テキストの折り返し方法		// 2008.05.30 nasukoji
	IDC_EDIT_MAXLINELEN,			HIDC_EDIT_MAXLINELEN,		// 折り返し桁数
	IDC_SPIN_MAXLINELEN,			HIDC_EDIT_MAXLINELEN,
	IDC_EDIT_CHARSPACE,				HIDC_EDIT_CHARSPACE,		// 文字の間隔
	IDC_SPIN_CHARSPACE,				HIDC_EDIT_CHARSPACE,
	IDC_EDIT_LINESPACE,				HIDC_EDIT_LINESPACE,		// 行の間隔
	IDC_SPIN_LINESPACE,				HIDC_EDIT_LINESPACE,
	IDC_EDIT_TABSPACE,				HIDC_EDIT_TABSPACE,			// TAB幅 // Sep. 19, 2002 genta
	IDC_SPIN_TABSPACE,				HIDC_EDIT_TABSPACE,
	IDC_EDIT_TABVIEWSTRING,			HIDC_EDIT_TABVIEWSTRING,	// TAB表示文字列
	IDC_CHECK_TAB_ARROW,			HIDC_CHECK_TAB_ARROW,		// 矢印表示	// 2006.08.06 ryoji
	IDC_CHECK_INS_SPACE,			HIDC_CHECK_INS_SPACE,		// スペースの挿入

	IDC_CHECK_INDENT,				HIDC_CHECK_INDENT,			// 自動インデント	// 2006.08.19 ryoji
	IDC_CHECK_INDENT_WSPACE,		HIDC_CHECK_INDENT_WSPACE,	// 全角空白もインデント	// 2006.08.19 ryoji
	IDC_COMBO_SMARTINDENT,			HIDC_COMBO_SMARTINDENT,		// スマートインデント
	IDC_EDIT_INDENTCHARS,			HIDC_EDIT_INDENTCHARS,		// その他のインデント対象文字
	IDC_COMBO_INDENTLAYOUT,			HIDC_COMBO_INDENTLAYOUT,	// 折り返し行インデント	// 2006.08.06 ryoji
	IDC_CHECK_RTRIM_PREVLINE,		HIDC_CHECK_RTRIM_PREVLINE,	// 改行時に末尾の空白を削除	// 2006.08.06 ryoji

	IDC_RADIO_OUTLINEDEFAULT,		HIDC_RADIO_OUTLINEDEFAULT,	// 標準ルール	// 2006.08.06 ryoji
	IDC_COMBO_OUTLINES,				HIDC_COMBO_OUTLINES,		// アウトライン解析方法
	IDC_RADIO_OUTLINERULEFILE,		HIDC_RADIO_OUTLINERULEFILE,	// ルールファイル	// 2006.08.06 ryoji
	IDC_EDIT_OUTLINERULEFILE,		HIDC_EDIT_OUTLINERULEFILE,	// ルールファイル名	// 2006.08.06 ryoji
	IDC_BUTTON_RULEFILE_REF,		HIDC_BUTTON_RULEFILE_REF,	// ルールファイル参照	// 2006/09/09 novice

	IDC_CHECK_USETYPEFONT,			HIDC_CHECK_USETYPEFONT,		// タイプ別フォント使用する
	IDC_BUTTON_TYPEFONT,			HIDC_BUTTON_TYPEFONT,		// タイプ別フォント

	IDC_CHECK_WORDWRAP,				HIDC_CHECK_WORDWRAP,		// 英文ワードラップ
	IDC_CHECK_KINSOKURET,			HIDC_CHECK_KINSOKURET,		// 改行文字をぶら下げる	//@@@ 2002.04.14 MIK
	IDC_CHECK_KINSOKUHIDE,			HIDC_CHECK_KINSOKUHIDE,		// ぶら下げを隠す		// 2012.11.30 Uchi
	IDC_CHECK_KINSOKUKUTO,			HIDC_CHECK_KINSOKUKUTO,		// 句読点をぶら下げる	//@@@ 2002.04.17 MIK
	IDC_EDIT_KINSOKUKUTO,			HIDC_EDIT_KINSOKUKUTO,		// 句読点ぶら下げ文字	// 2009.08.07 ryoji
	IDC_CHECK_KINSOKUHEAD,			HIDC_CHECK_KINSOKUHEAD,		// 行頭禁則	//@@@ 2002.04.08 MIK
	IDC_EDIT_KINSOKUHEAD,			HIDC_EDIT_KINSOKUHEAD,		// 行頭禁則	//@@@ 2002.04.08 MIK
	IDC_CHECK_KINSOKUTAIL,			HIDC_CHECK_KINSOKUTAIL,		// 行末禁則	//@@@ 2002.04.08 MIK
	IDC_EDIT_KINSOKUTAIL,			HIDC_EDIT_KINSOKUTAIL,		// 行末禁則	//@@@ 2002.04.08 MIK
//	IDC_STATIC,						-1,
	0, 0
};


// アウトライン解析方法・標準ルール
TYPE_NAME_ID<EOutlineType> OlmArr[] = {
//	{ OUTLINE_C,		_T("C") },
	{ OUTLINE_CPP,		STR_OUTLINE_CPP },
	{ OUTLINE_PLSQL,	STR_OUTLINE_PLSQL },
	{ OUTLINE_JAVA,		STR_OUTLINE_JAVA },
	{ OUTLINE_COBOL,	STR_OUTLINE_COBOL },
	{ OUTLINE_PERL,		STR_OUTLINE_PERL },			// Sep. 8, 2000 genta
	{ OUTLINE_ASM,		STR_OUTLINE_ASM },
	{ OUTLINE_VB,		STR_OUTLINE_VB },			// 2001/06/23 N.Nakatani
	{ OUTLINE_PYTHON,	STR_OUTLINE_PYTHON },		// 2007.02.08 genta
	{ OUTLINE_ERLANG,	STR_OUTLINE_ERLANG },		// 2009.08.10 genta
	{ OUTLINE_WZTXT,	STR_OUTLINE_WZ },			// 2003.05.20 zenryaku, 2003.06.23 Moca 名称変更
	{ OUTLINE_HTML,		STR_OUTLINE_HTML },			// 2003.05.20 zenryaku
	{ OUTLINE_TEX,		STR_OUTLINE_TEX },			// 2003.07.20 naoh
	{ OUTLINE_TEXT,		STR_OUTLINE_TEXT }			// Jul. 08, 2001 JEPRO 常に最後尾におく
};

TYPE_NAME_ID<TabArrowType> TabArrowArr[] = {
	{ TabArrowType::String,	STR_TAB_SYMBOL_CHARA },			// _T("文字指定")
	{ TabArrowType::Short,	STR_TAB_SYMBOL_SHORT_ARROW },	// _T("短い矢印")
	{ TabArrowType::Long,	STR_TAB_SYMBOL_LONG_ARROW },	// _T("長い矢印")
};

TYPE_NAME_ID<SmartIndentType> SmartIndentArr[] = {
	{ SmartIndentType::None,	STR_SMART_INDENT_NONE },
	{ SmartIndentType::Cpp,	STR_SMART_INDENT_C_CPP },
};

/*!	2行目以降のインデント方法

	@sa CLayoutMgr::SetLayoutInfo()
	@date Oct. 1, 2002 genta 
*/
TYPE_NAME_ID<int> IndentTypeArr[] = {
	{ 0, STR_WRAP_INDENT_NONE },	// _T("なし")
	{ 1, STR_WRAP_INDENT_TX2X },	// _T("tx2x")
	{ 2, STR_WRAP_INDENT_BOL },		// _T("論理行先頭")
};

// 2008.05.30 nasukoji	テキストの折り返し方法
TYPE_NAME_ID<TextWrappingMethod> WrapMethodArr[] = {
	{ TextWrappingMethod::NoWrapping,	STR_WRAP_METHOD_NO_WRAP },		// _T("折り返さない")
	{ TextWrappingMethod::SettingWidth,	STR_WRAP_METHOD_SPEC_WIDTH },	// _T("指定桁で折り返す")
	{ TextWrappingMethod::WindowWidth,	STR_WRAP_METHOD_WIN_WIDTH },	// _T("右端で折り返す")
};

// 静的メンバ
std::vector<TYPE_NAME_ID2<EOutlineType>> PropTypes::m_OlmArr;	// アウトライン解析ルール配列
std::vector<TYPE_NAME_ID2<SmartIndentType>> PropTypes::m_SIndentArr;	// スマートインデントルール配列

// スクリーンタブの初期化
void PropTypesScreen::CPropTypes_Screen()
{
	// プラグイン無効の場合、ここで静的メンバを初期化する。プラグイン有効の場合はAddXXXMethod内で初期化する。
	if (m_OlmArr.empty()) {
		InitTypeNameId2(m_OlmArr, OlmArr, _countof(OlmArr));	// アウトライン解析ルール
	}
	if (m_SIndentArr.empty()) {
		InitTypeNameId2(m_SIndentArr, SmartIndentArr, _countof(SmartIndentArr));	// スマートインデントルール
	}
}

// Screen メッセージ処理
INT_PTR PropTypesScreen::DispatchEvent(
	HWND		hwndDlg,	// handle to dialog box
	UINT		uMsg,		// message
	WPARAM		wParam,		// first message parameter
	LPARAM		lParam 		// second message parameter
	)
{
	WORD		wNotifyCode;
	WORD		wID;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
	int			nVal;

	switch (uMsg) {
	case WM_INITDIALOG:
		m_hwndThis = hwndDlg;
		// ダイアログデータの設定 Screen
		SetData(hwndDlg);
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// エディットコントロールの入力文字数制限
		EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_TYPENAME       ), _countof(m_types.m_szTypeName     ) - 1);
		EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_TYPEEXTS       ), _countof(m_types.m_szTypeExts     ) - 1);
		EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_INDENTCHARS    ), _countof(m_types.m_szIndentChars  ) - 1);
		EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_TABVIEWSTRING  ), _countof(m_types.m_szTabViewString) - 1);
		EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_OUTLINERULEFILE), _countof2(m_types.m_szOutlineRuleFilename) - 1);	//	Oct. 5, 2002 genta 画面上でも入力制限

		if (m_types.m_nIdx == 0) {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_TYPENAME), FALSE);	// 設定の名前
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_TYPEEXTS), FALSE);	// ファイル拡張子
		}
		return TRUE;
		
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	// 通知コード
		wID			= LOWORD(wParam);	// 項目ID､ コントロールID､ またはアクセラレータID
//		hwndCtl		= (HWND) lParam;	// コントロールのハンドル
		switch (wNotifyCode) {
		case CBN_SELCHANGE:
			switch (wID) {
			case IDC_CHECK_TAB_ARROW:
				{
					// Mar. 31, 2003 genta 矢印表示のON/OFFをTAB文字列設定に連動させる
					HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_CHECK_TAB_ARROW);
					int nSelPos = Combo_GetCurSel(hwndCombo);
					if (TabArrowType::String == TabArrowArr[nSelPos].nMethod) {
						::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_TABVIEWSTRING), TRUE);
					}else {
						::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_TABVIEWSTRING), FALSE);
					}
				}
				break;
			}
			break;

		// ボタン／チェックボックスがクリックされた
		case BN_CLICKED:
			switch (wID) {
			/*	2002.04.01 YAZAKI オートインデントを削除（もともと不要）
				アウトライン解析にルールファイル関連を追加
			*/
			case IDC_RADIO_OUTLINEDEFAULT:	// アウトライン解析→標準ルール
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_COMBO_OUTLINES), TRUE);
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_OUTLINERULEFILE), FALSE);
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_RULEFILE_REF), FALSE);

				Combo_SetCurSel(::GetDlgItem(hwndDlg, IDC_COMBO_OUTLINES), 0);

				return TRUE;
			case IDC_RADIO_OUTLINERULEFILE:	// アウトライン解析→ルールファイル
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_COMBO_OUTLINES), FALSE);
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_OUTLINERULEFILE), TRUE);
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_RULEFILE_REF), TRUE);
				return TRUE;

			case IDC_BUTTON_RULEFILE_REF:	// アウトライン解析→ルールファイルの「参照...」ボタン
				{
					DlgOpenFile	dlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
					// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
					if (_IS_REL_PATH(m_types.m_szOutlineRuleFilename)) {
						GetInidirOrExedir(szPath, m_types.m_szOutlineRuleFilename);
					}else {
						_tcscpy(szPath, m_types.m_szOutlineRuleFilename);
					}
					// ファイルオープンダイアログの初期化
					dlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						_T("*.*"),
						szPath
					);
					if (dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
						_tcscpy(m_types.m_szOutlineRuleFilename, szPath);
						::DlgItem_SetText(hwndDlg, IDC_EDIT_OUTLINERULEFILE, m_types.m_szOutlineRuleFilename);
					}
				}
				return TRUE;

			case IDC_BUTTON_TYPEFONT:
				{
					LOGFONT lf;
					INT nPointSize;

					if (m_types.m_bUseTypeFont) {
						lf = m_types.m_lf;
					}else {
						lf = m_pShareData->m_common.m_view.m_lf;
					}

					bool bFixedFont = true;
					if (m_pShareData->m_common.m_view.m_lf.lfPitchAndFamily & FIXED_PITCH) {
					}else {
						bool bFixedFont = false;
					}

					if (MySelectFont(&lf, &nPointSize, hwndDlg, bFixedFont)) {
						m_types.m_lf = lf;
						m_types.m_nPointSize = nPointSize;
						m_types.m_bUseTypeFont = true;		// タイプ別フォントの使用
						::CheckDlgButton(hwndDlg, IDC_CHECK_USETYPEFONT, m_types.m_bUseTypeFont);
						::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_USETYPEFONT), m_types.m_bUseTypeFont);
						// フォント表示	// 2013/6/23 Uchi
						HFONT hFont = SetFontLabel(hwndDlg, IDC_STATIC_TYPEFONT, m_types.m_lf, m_types.m_nPointSize, m_types.m_bUseTypeFont);
						if (m_hTypeFont) {
							::DeleteObject(m_hTypeFont);
						}
						m_hTypeFont = hFont;
					}
				}
				return TRUE;
			case IDC_CHECK_USETYPEFONT:	// 2013/6/24 Uchi
				if (!IsDlgButtonChecked(hwndDlg, IDC_CHECK_USETYPEFONT)) {
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_USETYPEFONT), FALSE);
					// フォント表示
					HFONT hFont = SetFontLabel(hwndDlg, IDC_STATIC_TYPEFONT, m_types.m_lf, m_types.m_nPointSize, FALSE);
					if (m_hTypeFont) {
						::DeleteObject(m_hTypeFont);
					}
					m_hTypeFont = hFont;
				}
				return TRUE;
			case IDC_CHECK_KINSOKURET:		// 改行文字をぶら下げる
			case IDC_CHECK_KINSOKUKUTO:		// 句読点をぶら下げる
				// ぶら下げを隠すの有効化	2012/11/30 Uchi
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_KINSOKUHIDE), 
					IsDlgButtonChecked(hwndDlg, IDC_CHECK_KINSOKURET) 
				 || IsDlgButtonChecked(hwndDlg, IDC_CHECK_KINSOKUKUTO));
			}
			break;	// BN_CLICKED
		}
		break;	// WM_COMMAND
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch (idCtrl) {
		case IDC_SPIN_MAXLINELEN:
			// 折り返し桁数
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_MAXLINELEN, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < MINLINEKETAS) {
				nVal = MINLINEKETAS;
			}
			if (nVal > MAXLINEKETAS) {
				nVal = MAXLINEKETAS;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_MAXLINELEN, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_CHARSPACE:
			// 文字の隙間
//			MYTRACE(_T("IDC_SPIN_CHARSPACE\n"));
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_CHARSPACE, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 0) {
				nVal = 0;
			}
			if (nVal > COLUMNSPACE_MAX) { // Feb. 18, 2003 genta 最大値の定数化
				nVal = COLUMNSPACE_MAX;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_CHARSPACE, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_LINESPACE:
			// 行の隙間
//			MYTRACE(_T("IDC_SPIN_LINESPACE\n"));
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_LINESPACE, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
//	From Here Oct. 8, 2000 JEPRO 行間も最小0まで設定できるように変更(昔に戻っただけ?)
//			if (nVal < 1) {
//				nVal = 1;
//			}
			if (nVal < 0) {
				nVal = 0;
			}
//	To Here  Oct. 8, 2000
			if (nVal > LINESPACE_MAX) { // Feb. 18, 2003 genta 最大値の定数化
				nVal = LINESPACE_MAX;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_LINESPACE, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_TABSPACE:
			//	Sep. 22, 2002 genta
			// TAB幅
//			MYTRACE(_T("IDC_SPIN_CHARSPACE\n"));
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_TABSPACE, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 1) {
				nVal = 1;
			}
			if (nVal > 64) {
				nVal = 64;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_TABSPACE, nVal, FALSE);
			return TRUE;

		default:
			switch (pNMHDR->code) {
			case PSN_HELP:
				OnHelp(hwndDlg, IDD_PROP_SCREEN);
				return TRUE;
			case PSN_KILLACTIVE:
				// ダイアログデータの取得 Screen
				GetData(hwndDlg);

				return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			case PSN_SETACTIVE:
				m_nPageNum = ID_PROPTYPE_PAGENUM_SCREEN;
				return TRUE;
			}
			break;
		}

//		MYTRACE(_T("pNMHDR->hwndFrom	=%xh\n"),	pNMHDR->hwndFrom);
//		MYTRACE(_T("pNMHDR->idFrom	=%xh\n"),	pNMHDR->idFrom);
//		MYTRACE(_T("pNMHDR->code		=%xh\n"),	pNMHDR->code);
//		MYTRACE(_T("pMNUD->iPos		=%d\n"),		pMNUD->iPos);
//		MYTRACE(_T("pMNUD->iDelta		=%d\n"),		pMNUD->iDelta);
		break;

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids1);	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		// NOTREACHED
//		break;
//@@@ 2001.02.04 End

//@@@ 2001.11.17 add start MIK
	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids1);	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//@@@ 2001.11.17 add end MIK

	case WM_DESTROY:
		// タイプフォント破棄	// 2013/6/23 Uchi
		if (m_hTypeFont) {
			::DeleteObject(m_hTypeFont);
			m_hTypeFont = NULL;
		}
		return TRUE;
	}
	return FALSE;
}


// ダイアログデータの設定 Screen
void PropTypesScreen::SetData(HWND hwndDlg)
{
	::DlgItem_SetText(hwndDlg, IDC_EDIT_TYPENAME, m_types.m_szTypeName);	// 設定の名前
	::DlgItem_SetText(hwndDlg, IDC_EDIT_TYPEEXTS, m_types.m_szTypeExts);	// ファイル拡張子

	// レイアウト
	{
		// 2008.05.30 nasukoji	テキストの折り返し方法
		HWND	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_WRAPMETHOD);
		Combo_ResetContent(hwndCombo);
		int		nSelPos = 0;
		for (int i=0; i<_countof(WrapMethodArr); ++i) {
			Combo_InsertString(hwndCombo, i, LS(WrapMethodArr[i].nNameId));
			if (WrapMethodArr[i].nMethod == m_types.m_nTextWrapMethod) {		// テキストの折り返し方法
				nSelPos = i;
			}
		}
		Combo_SetCurSel(hwndCombo, nSelPos);

		::SetDlgItemInt(hwndDlg, IDC_EDIT_MAXLINELEN, (Int)m_types.m_nMaxLineKetas, FALSE);	// 折り返し文字数
		::SetDlgItemInt(hwndDlg, IDC_EDIT_CHARSPACE, m_types.m_nColumnSpace, FALSE);			// 文字の間隔
		::SetDlgItemInt(hwndDlg, IDC_EDIT_LINESPACE, m_types.m_nLineSpace, FALSE);			// 行の間隔
		::SetDlgItemInt(hwndDlg, IDC_EDIT_TABSPACE, (Int)m_types.m_nTabSpace, FALSE);			// TAB幅	//	Sep. 22, 2002 genta
		::DlgItem_SetText(hwndDlg, IDC_EDIT_TABVIEWSTRING, m_types.m_szTabViewString);		// TAB表示(8文字)
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_TABVIEWSTRING), m_types.m_bTabArrow == TabArrowType::String);	// Mar. 31, 2003 genta 矢印表示のON/OFFをTAB文字列設定に連動させる

		// 矢印表示	//@@@ 2003.03.26 MIK
		hwndCombo = ::GetDlgItem(hwndDlg, IDC_CHECK_TAB_ARROW);
		Combo_ResetContent(hwndCombo);
		nSelPos = 0;
		for (int i=0; i<_countof(TabArrowArr); ++i) {
			Combo_InsertString(hwndCombo, i, LS(TabArrowArr[i].nNameId));
			if (TabArrowArr[i].nMethod == m_types.m_bTabArrow) {
				nSelPos = i;
			}
		}
		Combo_SetCurSel(hwndCombo, nSelPos);

		::CheckDlgButtonBool(hwndDlg, IDC_CHECK_INS_SPACE, m_types.m_bInsSpace);				// SPACEの挿入 [チェックボックス]	// From Here 2001.12.03 hor
	}

	// インデント
	{
		// 自動インデント
		::CheckDlgButtonBool(hwndDlg, IDC_CHECK_INDENT, m_types.m_bAutoIndent);

		// 日本語空白もインデント
		::CheckDlgButtonBool(hwndDlg, IDC_CHECK_INDENT_WSPACE, m_types.m_bAutoIndent_ZENSPACE);

		// スマートインデント種別
		HWND	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_SMARTINDENT);
		Combo_ResetContent(hwndCombo);
		int		nSelPos = 0;
		int nSize = (int)m_SIndentArr.size();
		for (int i=0; i<nSize; ++i) {
			if (!m_SIndentArr[i].pszName) {
				Combo_InsertString(hwndCombo, i, LS(m_SIndentArr[i].nNameId));
			}else {
				Combo_InsertString(hwndCombo, i, m_SIndentArr[i].pszName);
			}
			if (m_SIndentArr[i].nMethod == m_types.m_eSmartIndent) {	// スマートインデント種別
				nSelPos = i;
			}
		}
		Combo_SetCurSel(hwndCombo, nSelPos);

		// その他のインデント対象文字
		::DlgItem_SetText(hwndDlg, IDC_EDIT_INDENTCHARS, m_types.m_szIndentChars);

		// 折り返し行インデント	//	Oct. 1, 2002 genta コンボボックスに変更
		hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_INDENTLAYOUT);
		Combo_ResetContent(hwndCombo);
		nSelPos = 0;
		for (int i=0; i<_countof(IndentTypeArr); ++i) {
			Combo_InsertString(hwndCombo, i, LS(IndentTypeArr[i].nNameId));
			if (IndentTypeArr[i].nMethod == m_types.m_nIndentLayout) {	// 折り返しインデント種別
				nSelPos = i;
			}
		}
		Combo_SetCurSel(hwndCombo, nSelPos);

		// 改行時に末尾の空白を削除	// 2005.10.11 ryoji
		::CheckDlgButton(hwndDlg, IDC_CHECK_RTRIM_PREVLINE, m_types.m_bRTrimPrevLine);
	}

	// アウトライン解析方法
	// 2002.04.01 YAZAKI ルールファイル関連追加
	{
		// 標準ルールのコンボボックス初期化
		HWND	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_OUTLINES);
		Combo_ResetContent(hwndCombo);
		int		nSelPos = 0;
		int nSize = (int)m_OlmArr.size();
		for (int i=0; i<nSize; ++i) {
			if (!m_OlmArr[i].pszName) {
				Combo_InsertString(hwndCombo, i, LS(m_OlmArr[i].nNameId));
			}else {
				Combo_InsertString(hwndCombo, i, m_OlmArr[i].pszName);
			}
			if (m_OlmArr[i].nMethod == m_types.m_eDefaultOutline) {	// アウトライン解析方法
				nSelPos = i;
			}
		}

		// ルールファイル	// 2003.06.23 Moca ルールファイル名は使わなくてもセットしておく
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_OUTLINERULEFILE), TRUE);
		::DlgItem_SetText(hwndDlg, IDC_EDIT_OUTLINERULEFILE, m_types.m_szOutlineRuleFilename);

		// 標準ルール
		if (m_types.m_eDefaultOutline != OUTLINE_FILE) {
			::CheckDlgButton(hwndDlg, IDC_RADIO_OUTLINEDEFAULT, TRUE);
			::CheckDlgButton(hwndDlg, IDC_RADIO_OUTLINERULEFILE, FALSE);

			::EnableWindow(::GetDlgItem(hwndDlg, IDC_COMBO_OUTLINES), TRUE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_OUTLINERULEFILE), FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_RULEFILE_REF), FALSE);

			Combo_SetCurSel(hwndCombo, nSelPos);
		// ルールファイル
		}else {
			::CheckDlgButton(hwndDlg, IDC_RADIO_OUTLINEDEFAULT, FALSE);
			::CheckDlgButton(hwndDlg, IDC_RADIO_OUTLINERULEFILE, TRUE);

			::EnableWindow(::GetDlgItem(hwndDlg, IDC_COMBO_OUTLINES), FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_RULEFILE_REF), TRUE);
		}
	}

	// フォント
	{
		::CheckDlgButton(hwndDlg, IDC_CHECK_USETYPEFONT, m_types.m_bUseTypeFont);			// タイプ別フォントの使用
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_USETYPEFONT), m_types.m_bUseTypeFont);
		m_hTypeFont = SetFontLabel(hwndDlg, IDC_STATIC_TYPEFONT, m_types.m_lf, m_types.m_nPointSize, m_types.m_bUseTypeFont);
	}

	// その他
	{
		// 英文ワードラップをする
		::CheckDlgButtonBool(hwndDlg, IDC_CHECK_WORDWRAP, m_types.m_bWordWrap);

		// 禁則処理
		{	//@@@ 2002.04.08 MIK start
			::CheckDlgButtonBool(hwndDlg, IDC_CHECK_KINSOKUHEAD, m_types.m_bKinsokuHead);
			::CheckDlgButtonBool(hwndDlg, IDC_CHECK_KINSOKUTAIL, m_types.m_bKinsokuTail);
			::CheckDlgButtonBool(hwndDlg, IDC_CHECK_KINSOKURET,  m_types.m_bKinsokuRet );	// 改行文字をぶら下げる	//@@@ 2002.04.13 MIK
			::CheckDlgButtonBool(hwndDlg, IDC_CHECK_KINSOKUKUTO, m_types.m_bKinsokuKuto);	// 句読点をぶら下げる	//@@@ 2002.04.17 MIK
			::CheckDlgButtonBool(hwndDlg, IDC_CHECK_KINSOKUHIDE, m_types.m_bKinsokuHide);	// ぶら下げを隠す			// 2011/11/30 Uchi
			EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_KINSOKUHEAD), _countof(m_types.m_szKinsokuHead) - 1);
			EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_KINSOKUTAIL), _countof(m_types.m_szKinsokuTail) - 1);
			EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_KINSOKUKUTO), _countof(m_types.m_szKinsokuKuto) - 1);	// 2009.08.07 ryoji
			::DlgItem_SetText(hwndDlg, IDC_EDIT_KINSOKUHEAD, m_types.m_szKinsokuHead);
			::DlgItem_SetText(hwndDlg, IDC_EDIT_KINSOKUTAIL, m_types.m_szKinsokuTail);
			::DlgItem_SetText(hwndDlg, IDC_EDIT_KINSOKUKUTO, m_types.m_szKinsokuKuto);	// 2009.08.07 ryoji
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_KINSOKUHIDE), (m_types.m_bKinsokuRet || m_types.m_bKinsokuKuto) ? TRUE : FALSE);	// ぶら下げを隠すの有効化	2012/11/30 Uchi
		}	//@@@ 2002.04.08 MIK end
	}
}


// ダイアログデータの取得 Screen
int PropTypesScreen::GetData(HWND hwndDlg)
{
	::DlgItem_GetText(hwndDlg, IDC_EDIT_TYPENAME, m_types.m_szTypeName, _countof(m_types.m_szTypeName));	// 設定の名前
	::DlgItem_GetText(hwndDlg, IDC_EDIT_TYPEEXTS, m_types.m_szTypeExts, _countof(m_types.m_szTypeExts));	// ファイル拡張子

	// レイアウト
	{
		// 2008.05.30 nasukoji	テキストの折り返し方法
		HWND	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_WRAPMETHOD);
		int		nSelPos = Combo_GetCurSel(hwndCombo);
		m_types.m_nTextWrapMethod = WrapMethodArr[nSelPos].nMethod;		// テキストの折り返し方法

		// 折り返し桁数
		m_types.m_nMaxLineKetas = LayoutInt(::GetDlgItemInt(hwndDlg, IDC_EDIT_MAXLINELEN, NULL, FALSE));
		if (m_types.m_nMaxLineKetas < LayoutInt(MINLINEKETAS)) {
			m_types.m_nMaxLineKetas = LayoutInt(MINLINEKETAS);
		}
		if (m_types.m_nMaxLineKetas > LayoutInt(MAXLINEKETAS)) {
			m_types.m_nMaxLineKetas = LayoutInt(MAXLINEKETAS);
		}

		// 文字の間隔
		m_types.m_nColumnSpace = ::GetDlgItemInt(hwndDlg, IDC_EDIT_CHARSPACE, NULL, FALSE);
		if (m_types.m_nColumnSpace < 0) {
			m_types.m_nColumnSpace = 0;
		}
		if (m_types.m_nColumnSpace > COLUMNSPACE_MAX) { // Feb. 18, 2003 genta 最大値の定数化
			m_types.m_nColumnSpace = COLUMNSPACE_MAX;
		}

		// 行の間隔
		m_types.m_nLineSpace = ::GetDlgItemInt(hwndDlg, IDC_EDIT_LINESPACE, NULL, FALSE);
		if (m_types.m_nLineSpace < 0) {
			m_types.m_nLineSpace = 0;
		}
		if (m_types.m_nLineSpace > LINESPACE_MAX) {	// Feb. 18, 2003 genta 最大値の定数化
			m_types.m_nLineSpace = LINESPACE_MAX;
		}

		// TAB幅
		m_types.m_nTabSpace = LayoutInt(::GetDlgItemInt(hwndDlg, IDC_EDIT_TABSPACE, NULL, FALSE));
		if (m_types.m_nTabSpace < LayoutInt(1)) {
			m_types.m_nTabSpace = LayoutInt(1);
		}
		if (m_types.m_nTabSpace > LayoutInt(64)) {
			m_types.m_nTabSpace = LayoutInt(64);
		}

		// TAB表示文字列
		WIN_CHAR szTab[8 + 1]; // +1. happy
		::DlgItem_GetText(hwndDlg, IDC_EDIT_TABVIEWSTRING, szTab, _countof(szTab));
		wcscpy_s(m_types.m_szTabViewString, L"^       ");
		for (int i=0; i<8; ++i) {
			if (!TCODE::IsTabAvailableCode(szTab[i])) {
				break;
			}
			m_types.m_szTabViewString[i] = szTab[i];
		}

		// タブ矢印表示	//@@@ 2003.03.26 MIK
		hwndCombo = ::GetDlgItem(hwndDlg, IDC_CHECK_TAB_ARROW);
		nSelPos = Combo_GetCurSel(hwndCombo);
		m_types.m_bTabArrow = TabArrowArr[nSelPos].nMethod;		// テキストの折り返し方法

		// SPACEの挿入
		m_types.m_bInsSpace = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_INS_SPACE);
	}

	// インデント
	{
		// 自動インデント
		m_types.m_bAutoIndent = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_INDENT);

		// 日本語空白もインデント
		m_types.m_bAutoIndent_ZENSPACE = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_INDENT_WSPACE);

		// スマートインデント種別
		HWND	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_SMARTINDENT);
		int		nSelPos = Combo_GetCurSel(hwndCombo);
		if (nSelPos >= 0) {
			m_types.m_eSmartIndent = m_SIndentArr[nSelPos].nMethod;	// スマートインデント種別
		}

		// その他のインデント対象文字
		::DlgItem_GetText(hwndDlg, IDC_EDIT_INDENTCHARS, m_types.m_szIndentChars, _countof(m_types.m_szIndentChars));

		// 折り返し行インデント	//	Oct. 1, 2002 genta コンボボックスに変更
		hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_INDENTLAYOUT);
		nSelPos = Combo_GetCurSel(hwndCombo);
		m_types.m_nIndentLayout = IndentTypeArr[nSelPos].nMethod;	// 折り返し部インデント種別

		// 改行時に末尾の空白を削除	// 2005.10.11 ryoji
		m_types.m_bRTrimPrevLine = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_RTRIM_PREVLINE);
	}

	// アウトライン解析方法
	// 2002.04.01 YAZAKI ルールファイル関連追加
	{
		// 標準ルール
		if (!IsDlgButtonChecked(hwndDlg, IDC_RADIO_OUTLINERULEFILE)) {
			HWND	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_OUTLINES);
			int		nSelPos = Combo_GetCurSel(hwndCombo);
			if (nSelPos >= 0) {
				m_types.m_eDefaultOutline = m_OlmArr[nSelPos].nMethod;	// アウトライン解析方法
			}
		// ルールファイル
		}else {
			m_types.m_eDefaultOutline = OUTLINE_FILE;
		}

		// ルールファイル	// 2003.06.23 Moca ルールを使っていなくてもファイル名を保持
		::DlgItem_GetText(hwndDlg, IDC_EDIT_OUTLINERULEFILE, m_types.m_szOutlineRuleFilename, _countof2(m_types.m_szOutlineRuleFilename));
	}

	// フォント
	{
		LOGFONT lf;
		m_types.m_bUseTypeFont = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_USETYPEFONT);		// タイプ別フォントの使用
		if (m_types.m_bUseTypeFont) {
			lf = m_types.m_lf;
		}else {
			lf = m_pShareData->m_common.m_view.m_lf;
		}
	}

	// その他
	{
		// 英文ワードラップをする
		m_types.m_bWordWrap = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_WORDWRAP);

		// 禁則処理
		{	//@@@ 2002.04.08 MIK start
			m_types.m_bKinsokuHead = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_KINSOKUHEAD);
			m_types.m_bKinsokuTail = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_KINSOKUTAIL);
			m_types.m_bKinsokuRet  = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_KINSOKURET );	// 改行文字をぶら下げる	//@@@ 2002.04.13 MIK
			m_types.m_bKinsokuKuto = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_KINSOKUKUTO);	// 句読点をぶら下げる	//@@@ 2002.04.17 MIK
			m_types.m_bKinsokuHide = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_KINSOKUHIDE);	// ぶら下げを隠す		// 2011/11/30 Uchi
			::DlgItem_GetText(hwndDlg, IDC_EDIT_KINSOKUHEAD, m_types.m_szKinsokuHead, _countof(m_types.m_szKinsokuHead));
			::DlgItem_GetText(hwndDlg, IDC_EDIT_KINSOKUTAIL, m_types.m_szKinsokuTail, _countof(m_types.m_szKinsokuTail));
			::DlgItem_GetText(hwndDlg, IDC_EDIT_KINSOKUKUTO, m_types.m_szKinsokuKuto, _countof(m_types.m_szKinsokuKuto));	// 2009.08.07 ryoji
		}	//@@@ 2002.04.08 MIK end

	}

	return TRUE;
}

// アウトライン解析ルールの追加
void PropTypesScreen::AddOutlineMethod(int nMethod, const WCHAR* szName)
{
	if (m_OlmArr.empty()) {
		InitTypeNameId2(m_OlmArr, OlmArr, _countof(OlmArr));	// アウトライン解析ルール
	}
	TYPE_NAME_ID2<EOutlineType> method;
	method.nMethod = (EOutlineType)nMethod;
	method.nNameId = 0;
	const TCHAR* tszName = to_tchar(szName);
	TCHAR* pszName = new TCHAR[_tcslen(tszName) + 1];
	_tcscpy(pszName, tszName);
	method.pszName = pszName;
	m_OlmArr.push_back(method);
}

void PropTypesScreen::RemoveOutlineMethod(int nMethod, const WCHAR* szName)
{
	int nSize = (int)m_OlmArr.size();
	for (int i=0; i<nSize; ++i) {
		if (m_OlmArr[i].nMethod == (EOutlineType)nMethod) {
			delete [] m_OlmArr[i].pszName;
			m_OlmArr.erase(m_OlmArr.begin() + i);
			break;
		}
	}
}

// スマートインデントルールの追加
void PropTypesScreen::AddSIndentMethod(SmartIndentType nMethod, const WCHAR* szName)
{
	if (m_SIndentArr.empty()) {
		InitTypeNameId2(m_SIndentArr, SmartIndentArr, _countof(SmartIndentArr));	// スマートインデントルール
	}
	TYPE_NAME_ID2<SmartIndentType> method;
	method.nMethod = (SmartIndentType)nMethod;
	method.nNameId = 0;
	const TCHAR* tszName = to_tchar(szName);
	TCHAR* pszName = new TCHAR[_tcslen(tszName) + 1];
	_tcscpy(pszName, tszName);
	method.pszName = pszName;
	m_SIndentArr.push_back(method);
}

void PropTypesScreen::RemoveSIndentMethod(SmartIndentType nMethod, const WCHAR* szName)
{
	int nSize = (int)m_SIndentArr.size();
	for (int i=0; i<nSize; ++i) {
		if (m_SIndentArr[i].nMethod == (SmartIndentType)nMethod) {
			delete [] m_SIndentArr[i].pszName;
			m_SIndentArr.erase(m_SIndentArr.begin() + i);
			break;
		}
	}
}

