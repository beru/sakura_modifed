/*!	@file
	@brief 共通設定ダイアログボックス、「全般」ページ

	@author Norio Nakatani
	@date 1998/12/24 新規作成
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2001, genta, MIK, hor, Stonee, YAZAKI
	Copyright (C) 2002, YAZAKI, aroka, MIK, Moca, こおり
	Copyright (C) 2003, MIK, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, genta, ryoji
	Copyright (C) 2009, nasukoji
	Copyright (C) 2010, Uchi
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
#include "prop/PropCommon.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "env/DocTypeManager.h"
#include "EditApp.h"
#include "util/shell.h"
#include "sakura_rc.h"

int	PropCommon::SearchIntArr(int nKey, int* pnArr, int nArrNum)
{
	for (int i=0; i<nArrNum; ++i) {
		if (nKey == pnArr[i]) {
			return i;
		}
	}
	return -1;
}


/*!
	プロパティページごとのWindow Procedureを引数に取ることで
	処理の共通化を狙った．

	@param DispatchPage 真のWindow Procedureのメンバ関数ポインタ
	@param hwndDlg ダイアログボックスのWindow Handlw
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR PropCommon::DlgProc(
	INT_PTR (PropCommon::*DispatchPage)(HWND, UINT, WPARAM, LPARAM),
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam
)
{
	PROPSHEETPAGE* pPsp;
	PropCommon*	pPropCommon;
	switch (uMsg) {
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pPropCommon = (PropCommon*)(pPsp->lParam);
		if (pPropCommon) {
			return (pPropCommon->*DispatchPage)(hwndDlg, uMsg, wParam, pPsp->lParam);
		}else {
			return FALSE;
		}
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pPropCommon = (PropCommon*)::GetWindowLongPtr(hwndDlg, DWLP_USER);
		if (pPropCommon) {
			return (pPropCommon->*DispatchPage)(hwndDlg, uMsg, wParam, lParam);
		}else {
			return FALSE;
		}
	}
}
//	To Here Jun. 2, 2001 genta

// 独立ウィンドウ用 2013.3.14 aroka
INT_PTR PropCommon::DlgProc2(
	INT_PTR (PropCommon::*DispatchPage)(HWND, UINT, WPARAM, LPARAM),
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam
)
{
	PropCommon*	pPropCommon;
	switch (uMsg) {
	case WM_INITDIALOG:
		pPropCommon = (PropCommon*)(lParam);
		if (pPropCommon) {
			return (pPropCommon->*DispatchPage)(hwndDlg, uMsg, IDOK, lParam);
		}else {
			return FALSE;
		}
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pPropCommon = (PropCommon*)::GetWindowLongPtr(hwndDlg, DWLP_USER);
		if (pPropCommon) {
			return (pPropCommon->*DispatchPage)(hwndDlg, uMsg, wParam, lParam);
		}else {
			return FALSE;
		}
	}
}

//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、Processにひとつあるのみ。
PropCommon::PropCommon()
{
	{
		assert(sizeof(PropGeneral)   - sizeof(PropCommon) == 0);
		assert(sizeof(PropWin)       - sizeof(PropCommon) == 0);
		assert(sizeof(PropMainMenu)  - sizeof(PropCommon) == 0);
		assert(sizeof(PropToolbar)   - sizeof(PropCommon) == 0);
		assert(sizeof(PropTab)       - sizeof(PropCommon) == 0);
		assert(sizeof(PropStatusbar) - sizeof(PropCommon) == 0);
		assert(sizeof(PropEdit)      - sizeof(PropCommon) == 0);
		assert(sizeof(PropFile)      - sizeof(PropCommon) == 0);
		assert(sizeof(PropFileName)  - sizeof(PropCommon) == 0);
		assert(sizeof(PropBackup)    - sizeof(PropCommon) == 0);
		assert(sizeof(PropFormat)    - sizeof(PropCommon) == 0);
		assert(sizeof(PropGrep)      - sizeof(PropCommon) == 0);
		assert(sizeof(PropKeybind)   - sizeof(PropCommon) == 0);
		assert(sizeof(PropCustmenu)  - sizeof(PropCommon) == 0);
		assert(sizeof(PropKeyword)   - sizeof(PropCommon) == 0);
		assert(sizeof(PropHelper)    - sizeof(PropCommon) == 0);
		assert(sizeof(PropMacro)     - sizeof(PropCommon) == 0);
		assert(sizeof(PropPlugin)    - sizeof(PropCommon) == 0);
	}

	// 共有データ構造体のアドレスを返す
	pShareData = &GetDllShareData();

	hwndParent = NULL;	// オーナーウィンドウのハンドル
	hwndThis  = NULL;		// このダイアログのハンドル
	nPageNum = ID_PROPCOM_PAGENUM_GENERAL;
	nKeywordSet1 = -1;

	return;
}


PropCommon::~PropCommon()
{
}


// 初期化
//@@@ 2002.01.03 YAZAKI tbMyButtonなどをCShareDataからMenuDrawerへ移動したことによる修正。
void PropCommon::Create(HWND hwndParent, ImageListMgr* pIcons, MenuDrawer* pMenuDrawer)
{
	this->hwndParent = hwndParent;	// オーナーウィンドウのハンドル
	this->pIcons = pIcons;

	// 2007.11.02 ryoji マクロ設定を変更したあと、画面を閉じないでカスタムメニュー、ツールバー、
	//                  キー割り当ての画面に切り替えた時に各画面でマクロ設定の変更が反映されるよう、
	//                  common.macro.macroTable（ローカルメンバ）でlookupを初期化する
	lookup.Init(common.macro.macroTable, &common);	//	機能名・番号resolveクラス．

//@@@ 2002.01.03 YAZAKI tbMyButtonなどをCShareDataからMenuDrawerへ移動したことによる修正。
	this->pMenuDrawer = pMenuDrawer;

	return;
}


//	From Here Jun. 2, 2001 genta
/*!
	「共通設定」プロパティシートの作成時に必要な情報を
	保持する構造体
*/
struct ComPropSheetInfo {
	int nTabNameId;											// TABの表示名
	unsigned int resId;										// Property sheetに対応するDialog resource
	INT_PTR (CALLBACK *DProc)(HWND, UINT, WPARAM, LPARAM);	// Dialog Procedure
};
//	To Here Jun. 2, 2001 genta

//	キーワード：共通設定タブ順序(プロパティシート)
/*! プロパティシートの作成
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、Processにひとつあるのみ。
*/
INT_PTR PropCommon::DoPropertySheet(int nPageNum, bool bTrayProc)
{
	INT_PTR	nRet;
	size_t nIdx;

	this->bTrayProc = bTrayProc;

	// From Here Jun. 2, 2001 genta
	// Feb. 11, 2007 genta URLをTABと入れ換え	// 2007.02.13 順序変更（TABをWINの次に）
	// 「共通設定」プロパティシートの作成時に必要な情報の配列．
	// 順序変更 Win,Toolbar,Tab,Statusbarの順に、File,FileName 順に	2008/6/22 Uchi 
	// DProcの変更	2010/5/9 Uchi
	static const ComPropSheetInfo ComPropSheetInfoList[] = {
		{ STR_PROPCOMMON_GENERAL,	IDD_PROP_GENERAL,	PropGeneral::DlgProc_page },
		{ STR_PROPCOMMON_WINDOW,	IDD_PROP_WIN,		PropWin::DlgProc_page },
		{ STR_PROPCOMMON_MAINMENU,	IDD_PROP_MAINMENU,	PropMainMenu::DlgProc_page },	// 2010/5/8 Uchi
		{ STR_PROPCOMMON_TOOLBAR,	IDD_PROP_TOOLBAR,	PropToolbar::DlgProc_page },
		{ STR_PROPCOMMON_TABS,		IDD_PROP_TAB,		PropTab::DlgProc_page },
		{ STR_PROPCOMMON_STATBAR,	IDD_PROP_STATUSBAR,	PropStatusbar::DlgProc_page },	// 文字コード表示指定	2008/6/21	Uchi
		{ STR_PROPCOMMON_EDITING,	IDD_PROP_EDIT,		PropEdit::DlgProc_page },
		{ STR_PROPCOMMON_FILE,		IDD_PROP_FILE,		PropFile::DlgProc_page },
		{ STR_PROPCOMMON_FILENAME,	IDD_PROP_FNAME,		PropFileName::DlgProc_page },
		{ STR_PROPCOMMON_BACKUP,	IDD_PROP_BACKUP,	PropBackup::DlgProc_page },
		{ STR_PROPCOMMON_FORMAT,	IDD_PROP_FORMAT,	PropFormat::DlgProc_page },
		{ STR_PROPCOMMON_SEARCH,	IDD_PROP_GREP,		PropGrep::DlgProc_page },	// 2006.08.23 ryoji タイトル変更（Grep -> 検索）
		{ STR_PROPCOMMON_KEYS,		IDD_PROP_KEYBIND,	PropKeybind::DlgProc_page },
		{ STR_PROPCOMMON_CUSTMENU,	IDD_PROP_CUSTMENU,	PropCustmenu::DlgProc_page },
		{ STR_PROPCOMMON_KEYWORD,	IDD_PROP_KEYWORD,	PropKeyword::DlgProc_page },
		{ STR_PROPCOMMON_SUPPORT,	IDD_PROP_HELPER,	PropHelper::DlgProc_page },
		{ STR_PROPCOMMON_MACRO,		IDD_PROP_MACRO,		PropMacro::DlgProc_page },
		{ STR_PROPCOMMON_PLUGIN,	IDD_PROP_PLUGIN,	PropPlugin::DlgProc_page },
	};

	std::tstring		sTabname[_countof(ComPropSheetInfoList)];
	PROPSHEETPAGE		psp[_countof(ComPropSheetInfoList)];
	for (nIdx=0; nIdx<_countof(ComPropSheetInfoList); ++nIdx) {
		sTabname[nIdx] = LS(ComPropSheetInfoList[nIdx].nTabNameId);

		PROPSHEETPAGE* p = &psp[nIdx];
		memset_raw(p, 0, sizeof_raw(*p));
		p->dwSize      = sizeof_raw(*p);
		p->dwFlags     = PSP_USETITLE | PSP_HASHELP;
		p->hInstance   = SelectLang::getLangRsrcInstance();
		p->pszTemplate = MAKEINTRESOURCE(ComPropSheetInfoList[nIdx].resId);
		p->pszIcon     = NULL;
		p->pfnDlgProc  = ComPropSheetInfoList[nIdx].DProc;
		p->pszTitle    = sTabname[nIdx].c_str();
		p->lParam      = (LPARAM)this;
		p->pfnCallback = nullptr;
	}
	//	To Here Jun. 2, 2001 genta

	PROPSHEETHEADER psh;
	memset_raw(&psh, 0, sizeof_raw(psh));
	
	//	Jun. 29, 2002 こおり
	//	Windows 95対策．Property SheetのサイズをWindows95が認識できる物に固定する．
	psh.dwSize = sizeof_old_PROPSHEETHEADER;

	//	JEPROtest Sept. 30, 2000 共通設定の隠れ[適用]ボタンの正体はここ。行頭のコメントアウトを入れ替えてみればわかる
	psh.dwFlags    = PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE | PSH_USEPAGELANG;
	psh.hwndParent = hwndParent;
	psh.hInstance  = SelectLang::getLangRsrcInstance();
	psh.pszIcon    = NULL;
	psh.pszCaption = LS(STR_PROPCOMMON);	//_T("共通設定");
	psh.nPages     = nIdx;

	//- 20020106 aroka # psh.nStartPage は unsigned なので負にならない
	if (nPageNum == -1) {
		psh.nStartPage = nPageNum;
	}else
	if (0 > nPageNum) {			//- 20020106 aroka
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
		TCHAR*	pszMsgBuf;
		::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			::GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	// デフォルト言語
			(LPTSTR)&pszMsgBuf,
			0,
			NULL
		);
		PleaseReportToAuthor(
			NULL,
			LS(STR_ERR_DLGPROPCOMMON24),
			psh.nStartPage,
			pszMsgBuf
		);
		::LocalFree(pszMsgBuf);
	}

	return nRet;
}

/*!	ShareDataから一時領域へ設定をコピーする
	@date 2002.12.11 Moca CEditDoc::OpenPropertySheetから移動
*/
void PropCommon::InitData(void)
{
	common = pShareData->common;

	// 2002/04/25 YAZAKI TypeConfig全体を保持する必要はない。	;
	for (int i=0; i<GetDllShareData().nTypesCount; ++i) {
		KeywordSetIndex indexs;
		TypeConfig type;
		DocTypeManager().GetTypeConfig(TypeConfigNum(i), type);
		indexs.typeId = type.id;
		for (int j=0; j<MAX_KEYWORDSET_PER_TYPE; ++j) {
			indexs.index[j] = type.nKeywordSetIdx[j];
		}
		types_nKeywordSetIdx.push_back(indexs);
	}
}

/*!	ShareData に 設定を適用・コピーする
	@note ShareDataにコピーするだけなので，更新要求などは，利用する側で処理してもらう
	@date 2002.12.11 Moca CEditDoc::OpenPropertySheetから移動
*/
void PropCommon::ApplyData(void)
{
	pShareData->common = common;

	const int nSize = (int)types_nKeywordSetIdx.size();
	for (int i=0; i<nSize; ++i) {
		TypeConfigNum configIdx = DocTypeManager().GetDocumentTypeOfId(types_nKeywordSetIdx[i].typeId);
		if (configIdx.IsValidType()) {
			TypeConfig type;
			DocTypeManager().GetTypeConfig(configIdx, type);
			// 2002/04/25 YAZAKI TypeConfig全体を保持する必要はない。
			// 変更された設定値のコピー
			for (int j = 0; j < MAX_KEYWORDSET_PER_TYPE; ++j) {
				type.nKeywordSetIdx[j] = types_nKeywordSetIdx[i].index[j];
			}
			DocTypeManager().SetTypeConfig(configIdx, type);
		}
	}
}


// ヘルプ
// Stonee, 2001/05/18 機能番号からヘルプトピック番号を調べるようにした
void PropCommon::OnHelp(HWND hwndParent, int nPageID)
{
	int		nContextID;
	switch (nPageID) {
	case IDD_PROP_GENERAL:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_GENERAL);
		break;
	case IDD_PROP_FORMAT:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_FORMAT);
		break;
	case IDD_PROP_FILE:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_FILE);
		break;
//	Sept. 10, 2000 JEPRO ID名を実際の名前に変更するため以下の行はコメントアウト
//	変更は少し後の行(Sept. 9, 2000)で行っている
//	case IDD_PROP1P5:
//		nContextID = 84;
//		break;
	case IDD_PROP_TOOLBAR:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_TOOLBAR);
		break;
	case IDD_PROP_KEYWORD:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_KEYWORD);
		break;
	case IDD_PROP_CUSTMENU:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_CUSTMENU);
		break;
	case IDD_PROP_HELPER:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_HELPER);
		break;

	// From Here Sept. 9, 2000 JEPRO 共通設定のヘルプボタンが効かなくなっていた部分を以下の追加によって修正
	case IDD_PROP_EDIT:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_EDIT);
		break;
	case IDD_PROP_BACKUP:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_BACKUP);
		break;
	case IDD_PROP_WIN:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_WINDOW);
		break;
	case IDD_PROP_TAB:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_TAB);
		break;
	case IDD_PROP_STATUSBAR:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_STATUSBAR);
		break;
	case IDD_PROP_GREP:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_GREP);
		break;
	case IDD_PROP_KEYBIND:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_KEYBIND);
		break;
	// To Here Sept. 9, 2000
	case IDD_PROP_MACRO:	//@@@ 2002.01.02
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_MACRO);
		break;
	case IDD_PROP_FNAME:	// 2002.12.09 Moca FNAME追加
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_FNAME);
		break;
	case IDD_PROP_PLUGIN:	//@@@ 2002.01.02
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_PLUGIN);
		break;
	case IDD_PROP_MAINMENU:	//@@@ 2010/6/2 Uchi
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_MAINMENU);
		break;

	default:
		nContextID = -1;
		break;
	}
	if (nContextID != -1) {
		MyWinHelp(hwndParent, HELP_CONTEXT, nContextID);	// 2006.10.10 ryoji MyWinHelpに変更に変更
	}
	return;
}


/*!	コントロールにフォント設定する
	@date 2013.04.24 Uchi
*/
HFONT PropCommon::SetCtrlFont(HWND hwndDlg, int idc_ctrl, const LOGFONT& lf)
{
	HFONT	hFont;
	HWND	hCtrl;

	// 論理フォントを作成
	hCtrl = ::GetDlgItem(hwndDlg, idc_ctrl);
	hFont = ::CreateFontIndirect(&lf);
	if (hFont) {
		// フォントの設定
		::SendMessage(hCtrl, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
	}
	return hFont;
}


/*!	フォントラベルにフォントとフォント名設定する
	@date 2013.04.24 Uchi
*/
HFONT PropCommon::SetFontLabel(HWND hwndDlg, int idc_static, const LOGFONT& lf, int nps)
{
	HFONT	hFont;
	TCHAR	szFontName[80];
	LOGFONT lfTemp;
	lfTemp = lf;
	// 大きすぎるフォントは小さく表示
	if (lfTemp.lfHeight < -16) {
		lfTemp.lfHeight = -16;
	}

	hFont = SetCtrlFont(hwndDlg, idc_static, lfTemp);

	// フォント名の設定
	auto_sprintf_s(szFontName, (nps % 10) ? _T("%s(%.1fpt)") : _T("%s(%.0fpt)"),
		lf.lfFaceName, double(nps)/10);
	::DlgItem_SetText(hwndDlg, idc_static, szFontName);

	return hFont;
}

