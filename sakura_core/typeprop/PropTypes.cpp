// タイプ別設定ダイアログボックス

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
	// カスタム色の先頭にテキスト色を設定しておく
	dwCustColors[0] = types.colorInfoArr[COLORIDX_TEXT].colorAttr.cTEXT;
	dwCustColors[1] = types.colorInfoArr[COLORIDX_TEXT].colorAttr.cBACK;

	std::tstring sTabname[_countof(TypePropSheetInfoList)];
	bChangeKeywordSet = false;
	PROPSHEETPAGE psp[_countof(TypePropSheetInfoList)];
	size_t nIdx;
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

	//	Windows 95対策．Property SheetのサイズをWindows95が認識できる物に固定する．
	psh.dwSize = sizeof_old_PROPSHEETHEADER;

	// タイプ別設定の隠れ[適用]ボタンの正体はここ。行頭のコメントアウトを入れ替えてみればわかる
	psh.dwFlags    = /*PSH_USEICONID |*/ PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE/* | PSH_HASHELP*/ | PSH_USEPAGELANG;
	psh.hwndParent = hwndParent;
	psh.hInstance  = SelectLang::getLangRsrcInstance();
	psh.pszIcon    = NULL;
	psh.pszCaption = LS(STR_PROPTYPE);
	psh.nPages     = nIdx;

	// psh.nStartPage は unsigned なので負にならない
	if (nPageNum == -1) {
		psh.nStartPage = this->nPageNum;
	}else if (0 > nPageNum) {
		psh.nStartPage = 0;
	}else {
		psh.nStartPage = nPageNum;
	}
	
	if (psh.nPages - 1 < psh.nStartPage) {
		psh.nStartPage = psh.nPages - 1;
	}
	psh.ppsp = psp;
	psh.pfnCallback = nullptr;

	nRet = MyPropertySheet(&psh);	// 独自拡張プロパティシート

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
		MyWinHelp(hwndParent, HELP_CONTEXT, nContextID);
	}
}


/*!	コントロールにフォント設定する */
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


/*!	フォントラベルにフォントとフォント名設定する */
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
		auto_sprintf_s(szFontName, (nps % 10) ? _T("%s(%.1fpt)") : _T("%s(%.0fpt)"),
			lf.lfFaceName, double(nps)/10);
		::DlgItem_SetText(hwndDlg, idc_static, szFontName);
	}else {
		hFont = NULL;
		::DlgItem_SetText(hwndDlg, idc_static, _T(""));
	}

	return hFont;
}

