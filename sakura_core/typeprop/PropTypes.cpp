/*!	@file
	@brief タイプ別設定ダイアログボックス

	@author Norio Nakatani
	@date 1998/12/24  新規作成
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
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "PropTypes.h"
#include "EditApp.h"
#include "view/colors/EColorIndexType.h"
#include "util/shell.h"
#include "sakura_rc.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      メッセージ処理                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

typedef INT_PTR (PropTypes::*DISPATCH_EVENT_TYPE)(HWND, UINT, WPARAM, LPARAM);

// 共通ダイアログプロシージャ
INT_PTR CALLBACK PropTypesCommonProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam, DISPATCH_EVENT_TYPE pDispatch)
{
	PROPSHEETPAGE* pPsp;
	PropTypes* pPropTypes;
	switch (uMsg) {
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pPropTypes = reinterpret_cast<PropTypes*>(pPsp->lParam);
		if (pPropTypes) {
			return (pPropTypes->*pDispatch)(hwndDlg, uMsg, wParam, pPsp->lParam);
		}else {
			return FALSE;
		}
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pPropTypes = (PropTypes*)::GetWindowLongPtr(hwndDlg, DWLP_USER);
		if (pPropTypes) {
			return (pPropTypes->*pDispatch)(hwndDlg, uMsg, wParam, lParam);
		}else {
			return FALSE;
		}
	}
}

// 各種ダイアログプロシージャ
typedef	INT_PTR (PropTypes::*pDispatchPage)(HWND, UINT, WPARAM, LPARAM);
#define GEN_PROPTYPES_CALLBACK(FUNC, CLASS) \
INT_PTR CALLBACK FUNC(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) \
{ \
	return PropTypesCommonProc(hwndDlg, uMsg, wParam, lParam, reinterpret_cast<pDispatchPage>(&CLASS::DispatchEvent)); \
}
GEN_PROPTYPES_CALLBACK(PropTypesScreenDlgProc,		PropTypesScreen)
GEN_PROPTYPES_CALLBACK(PropTypesWindowDlgProc,		PropTypesWindow)
GEN_PROPTYPES_CALLBACK(PropTypesColorDlgProc,		PropTypesColor)
GEN_PROPTYPES_CALLBACK(PropTypesSupportDlgProc,		PropTypesSupport)
GEN_PROPTYPES_CALLBACK(PropTypesRegexDlgProc,		PropTypesRegex)
GEN_PROPTYPES_CALLBACK(PropTypesKeyHelpDlgProc,		PropTypesKeyHelp)



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        生成と破棄                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

PropTypes::PropTypes()
{
	{
		assert(sizeof(PropTypesScreen)  - sizeof(PropTypes) == 0);
		assert(sizeof(PropTypesWindow)  - sizeof(PropTypes) == 0);
		assert(sizeof(PropTypesColor)   - sizeof(PropTypes) == 0);
		assert(sizeof(PropTypesSupport) - sizeof(PropTypes) == 0);
		assert(sizeof(PropTypesRegex)   - sizeof(PropTypes) == 0);
		assert(sizeof(PropTypesKeyHelp) - sizeof(PropTypes) == 0);
	}

	// 共有データ構造体のアドレスを返す
	pShareData = &GetDllShareData();

	// Mar. 31, 2003 genta メモリ削減のためポインタに変更
	pKeywordSetMgr = &pShareData->common.specialKeyword.keywordSetMgr;

	hInstance = NULL;		// アプリケーションインスタンスのハンドル
	hwndParent = NULL;	// オーナーウィンドウのハンドル
	hwndThis  = NULL;		// このダイアログのハンドル
	nPageNum = ID_PROPTYPE_PAGENUM_SCREEN;

	(static_cast<PropTypesScreen*>(this))->CPropTypes_Screen();
}

PropTypes::~PropTypes()
{
}

// 初期化
void PropTypes::Create(HINSTANCE hInstApp, HWND hwndParent)
{
	this->hInstance = hInstApp;		// アプリケーションインスタンスのハンドル
	this->hwndParent = hwndParent;	// オーナーウィンドウのハンドル
}

struct TypePropSheetInfo {
	int nTabNameId;											// TABの表示名
	unsigned int resId;										// Property sheetに対応するDialog resource
	INT_PTR (CALLBACK *DProc)(HWND, UINT, WPARAM, LPARAM);	// Dialog Procedure
};

// キーワード：タイプ別設定タブ順序(プロパティシート)
// プロパティシートの作成
INT_PTR PropTypes::DoPropertySheet(int nPageNum)
{
	INT_PTR	nRet;
	int		nIdx;

	// 2001/06/14 Start by asa-o: タイプ別設定に支援タブ追加
	// 2001.11.17 add start MIK タイプ別設定に正規表現キーワードタブ追加
	// 2006.04.10 fon ADD-start タイプ別設定に「キーワードヘルプ」タブを追加
	// 2013.03.10 aroka ADD-start タイプ別設定に「ウィンドウ」タブを追加
	static const TypePropSheetInfo TypePropSheetInfoList[] = {
		{ STR_PROPTYPE_SCREEN,			IDD_PROP_SCREEN,	PropTypesScreenDlgProc },
		{ STR_PROPTYPE_COLOR,			IDD_PROP_COLOR,		PropTypesColorDlgProc },
		{ STR_PROPTYPE_WINDOW,			IDD_PROP_WINDOW,	PropTypesWindowDlgProc },
		{ STR_PROPTYPE_SUPPORT,			IDD_PROP_SUPPORT,	PropTypesSupportDlgProc },
		{ STR_PROPTYPE_REGEX_KEYWORD,	IDD_PROP_REGEX,		PropTypesRegexDlgProc },
		{ STR_PROPTYPE_KEYWORD_HELP,	IDD_PROP_KEYHELP,	PropTypesKeyHelpDlgProc }
	};

	// カスタム色を共有メモリから取得
	memcpy_raw( dwCustColors, pShareData->dwCustColors, sizeof(dwCustColors) );
	// 2005.11.30 Moca カスタム色の先頭にテキスト色を設定しておく
	dwCustColors[0] = types.colorInfoArr[COLORIDX_TEXT].colorAttr.cTEXT;
	dwCustColors[1] = types.colorInfoArr[COLORIDX_TEXT].colorAttr.cBACK;

	std::tstring sTabname[_countof(TypePropSheetInfoList)];
	bChangeKeywordSet = false;
	PROPSHEETPAGE psp[_countof(TypePropSheetInfoList)];
	for (nIdx=0; nIdx<_countof(TypePropSheetInfoList); ++nIdx) {
		sTabname[nIdx] = LS(TypePropSheetInfoList[nIdx].nTabNameId);

		PROPSHEETPAGE* p = &psp[nIdx];
		memset_raw(p, 0, sizeof_raw(*p));
		p->dwSize      = sizeof_raw(*p);
		p->dwFlags     = PSP_USETITLE | PSP_HASHELP;
		p->hInstance   = SelectLang::getLangRsrcInstance();
		p->pszTemplate = MAKEINTRESOURCE(TypePropSheetInfoList[nIdx].resId);
		p->pszIcon     = NULL;
		p->pfnDlgProc  = TypePropSheetInfoList[nIdx].DProc;
		p->pszTitle    = sTabname[nIdx].c_str();
		p->lParam      = (LPARAM)this;
		p->pfnCallback = nullptr;
	}

	PROPSHEETHEADER	psh = {0};

	//	Jun. 29, 2002 こおり
	//	Windows 95対策．Property SheetのサイズをWindows95が認識できる物に固定する．
	psh.dwSize = sizeof_old_PROPSHEETHEADER;

	// JEPROtest Sept. 30, 2000 タイプ別設定の隠れ[適用]ボタンの正体はここ。行頭のコメントアウトを入れ替えてみればわかる
	psh.dwFlags    = /*PSH_USEICONID |*/ PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE/* | PSH_HASHELP*/ | PSH_USEPAGELANG;
	psh.hwndParent = hwndParent;
	psh.hInstance  = SelectLang::getLangRsrcInstance();
	psh.pszIcon    = NULL;
	psh.pszCaption = LS(STR_PROPTYPE);	// _T("タイプ別設定");	// Sept. 8, 2000 jepro 単なる「設定」から変更
	psh.nPages     = nIdx;

	//- 20020106 aroka # psh.nStartPage は unsigned なので負にならない
	if (nPageNum == -1) {
		psh.nStartPage = nPageNum;
	}else if (0 > nPageNum) {			//- 20020106 aroka
		psh.nStartPage = 0;
	}else {
		psh.nStartPage = nPageNum;
	}
	
	if (psh.nPages - 1 < psh.nStartPage) {
		psh.nStartPage = psh.nPages - 1;
	}
	psh.ppsp = psp;
	psh.pfnCallback = nullptr;

	nRet = MyPropertySheet(&psh);	// 2007.05.24 ryoji 独自拡張プロパティシート

	if (nRet == -1) {
		TCHAR* pszMsgBuf;
		::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			::GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // デフォルト言語
			(LPTSTR)&pszMsgBuf,
			0,
			NULL
		);
		PleaseReportToAuthor(
			NULL,
			LS(STR_PROPTYPE_ERR),
			psh.nStartPage,
			pszMsgBuf
		);
		::LocalFree(pszMsgBuf);
	}

	// カスタム色を共有メモリに設定
	memcpy_raw( pShareData->dwCustColors, dwCustColors, sizeof(dwCustColors) );

	return nRet;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         イベント                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ヘルプ
// 2001.05.18 Stonee 機能番号からヘルプトピック番号を調べるようにした
// 2001.07.03 JEPRO  支援タブのヘルプを有効化
// 2001.11.17 MIK    IDD_PROP_REGEX
void PropTypes::OnHelp(HWND hwndParent, int nPageID)
{
	int nContextID;
	switch (nPageID) {
	case IDD_PROP_SCREEN:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_SCREEN);			break;
	case IDD_PROP_COLOR:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_COLOR);			break;
	case IDD_PROP_WINDOW:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_WINDOW);			break;
	case IDD_PROP_SUPPORT:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_HELPER);			break;
	case IDD_PROP_REGEX:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_REGEX_KEYWORD);	break;
	case IDD_PROP_KEYHELP:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_KEYHELP);			break;
	default:				nContextID = -1;												break;
	}
	if (nContextID != -1) {
		MyWinHelp(hwndParent, HELP_CONTEXT, nContextID);	// 2006.10.10 ryoji MyWinHelpに変更に変更
	}
}


/*!	コントロールにフォント設定する
	@date 2013.04.24 Uchi
*/
HFONT PropTypes::SetCtrlFont(HWND hwndDlg, int idc_ctrl, const LOGFONT& lf)
{

	// 論理フォントを作成
	HWND hCtrl = ::GetDlgItem(hwndDlg, idc_ctrl);
	HFONT hFont = ::CreateFontIndirect(&lf);
	if (hFont) {
		// フォントの設定
		::SendMessage(hCtrl, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
	}

	return hFont;
}


/*!	フォントラベルにフォントとフォント名設定する
	@date 2013.04.24 Uchi
*/
HFONT PropTypes::SetFontLabel(HWND hwndDlg, int idc_static, const LOGFONT& lf, int nps, bool bUse)
{
	HFONT	hFont;

	if (bUse) {
		LOGFONT lfTemp = lf;
		// 大きすぎるフォントは小さく表示
		if (lfTemp.lfHeight < -16) {
			lfTemp.lfHeight = -16;
		}
		hFont = SetCtrlFont(hwndDlg, idc_static, lfTemp);
		// フォント名の設定
		TCHAR szFontName[80];
		auto_sprintf_s(szFontName, nps % 10 ? _T("%s(%.1fpt)") : _T("%s(%.0fpt)"),
			lf.lfFaceName, double(nps)/10);
		::DlgItem_SetText(hwndDlg, idc_static, szFontName);
	}else {
		hFont = NULL;
		::DlgItem_SetText(hwndDlg, idc_static, _T(""));
	}

	return hFont;
}

