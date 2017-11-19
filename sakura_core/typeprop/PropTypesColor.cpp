// タイプ別設定 - カラー

#include "StdAfx.h"
#include "PropTypes.h"
#include "env/ShareData.h"
#include "typeprop/ImpExpManager.h"
#include "DlgSameColor.h"
#include "DlgKeywordSelect.h"
#include "view/colors/EColorIndexType.h"
#include "uiparts/Graphics.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"
#include "prop/PropCommon.h"

using namespace std;

// カスタムカラー用の識別文字列
static const TCHAR* TSTR_PTRCUSTOMCOLORS = _T("ptrCustomColors");

WNDPROC	wpColorListProc;

static const DWORD p_helpids2[] = {	//11400
	IDC_LIST_COLORS,				HIDC_LIST_COLORS,				// 色指定
	IDC_CHECK_DISP,					HIDC_CHECK_DISP,				// 色分け表示
	IDC_CHECK_BOLD,					HIDC_CHECK_BOLD,				// 太字
	IDC_CHECK_UNDERLINE,			HIDC_CHECK_UNDERLINE,			// 下線
	IDC_BUTTON_TEXTCOLOR,			HIDC_BUTTON_TEXTCOLOR,			// 文字色
	IDC_BUTTON_BACKCOLOR,			HIDC_BUTTON_BACKCOLOR,			// 背景色
	IDC_BUTTON_SAMETEXTCOLOR,		HIDC_BUTTON_SAMETEXTCOLOR,		// 文字色統一
	IDC_BUTTON_SAMEBKCOLOR,			HIDC_BUTTON_SAMEBKCOLOR,		// 背景色統一
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT_COLOR,		// インポート
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT_COLOR,		// エクスポート
	IDC_COMBO_SET,					HIDC_COMBO_SET_COLOR,			// 強調キーワード１セット名
	IDC_BUTTON_KEYWORD_SELECT,		HIDC_BUTTON_KEYWORD_SELECT,		// 強調キーワード2～10
	IDC_EDIT_BLOCKCOMMENT_FROM,		HIDC_EDIT_BLOCKCOMMENT_FROM,	// ブロックコメント１開始
	IDC_EDIT_BLOCKCOMMENT_TO,		HIDC_EDIT_BLOCKCOMMENT_TO,		// ブロックコメント１終了
	IDC_EDIT_BLOCKCOMMENT_FROM2,	HIDC_EDIT_BLOCKCOMMENT_FROM2,	// ブロックコメント２開始
	IDC_EDIT_BLOCKCOMMENT_TO2,		HIDC_EDIT_BLOCKCOMMENT_TO2,		// ブロックコメント２終了
	IDC_EDIT_LINECOMMENT,			HIDC_EDIT_LINECOMMENT,			// 行コメント１
	IDC_EDIT_LINECOMMENT2,			HIDC_EDIT_LINECOMMENT2,			// 行コメント２
	IDC_EDIT_LINECOMMENT3,			HIDC_EDIT_LINECOMMENT3,			// 行コメント３
	IDC_EDIT_LINECOMMENTPOS,		HIDC_EDIT_LINECOMMENTPOS,		// 桁数１
	IDC_EDIT_LINECOMMENTPOS2,		HIDC_EDIT_LINECOMMENTPOS2,		// 桁数２
	IDC_EDIT_LINECOMMENTPOS3,		HIDC_EDIT_LINECOMMENTPOS3,		// 桁数３
	IDC_CHECK_LCPOS,				HIDC_CHECK_LCPOS,				// 桁指定１
	IDC_CHECK_LCPOS2,				HIDC_CHECK_LCPOS2,				// 桁指定２
	IDC_CHECK_LCPOS3,				HIDC_CHECK_LCPOS3,				// 桁指定３
	IDC_COMBO_STRINGLITERAL,		HIDC_COMBO_STRINGLITERAL,		// 文字列エスケープ
	IDC_CHECK_STRINGLINEONLY,		HIDC_CHECK_STRINGLINEONLY,		// 文字列は行内のみ
	IDC_CHECK_STRINGENDLINE,		HIDC_CHECK_STRINGENDLINE,		// 終了文字がない場合行末まで色分け
	IDC_EDIT_VERTLINE,				HIDC_EDIT_VERTLINE,				// 縦線の桁指定
//	IDC_STATIC,						-1,
	0, 0
};

TYPE_NAME_ID<StringLiteralType> StringLitteralArr[] = {
	{ StringLiteralType::CPP,    STR_STRINGESC_CPP },
	{ StringLiteralType::PLSQL,  STR_STRINGESC_PLSQL },
	{ StringLiteralType::HTML,   STR_STRINGESC_HTML },
	{ StringLiteralType::CSharp, STR_STRINGESC_CSHARP },
	{ StringLiteralType::Python, STR_STRINGESC_PYTHON },
};


// 行コメントに関する情報
struct {
	int nEditID;
	int nCheckBoxID;
	int nTextID;
} const cLineComment[COMMENT_DELIMITER_NUM] = {
	{ IDC_EDIT_LINECOMMENT	, IDC_CHECK_LCPOS , IDC_EDIT_LINECOMMENTPOS },
	{ IDC_EDIT_LINECOMMENT2	, IDC_CHECK_LCPOS2, IDC_EDIT_LINECOMMENTPOS2},
	{ IDC_EDIT_LINECOMMENT3	, IDC_CHECK_LCPOS3, IDC_EDIT_LINECOMMENTPOS3}
};

// 色の設定をインポート
bool PropTypesColor::Import(HWND hwndDlg)
{
	ColorInfo colorInfoArr[64];
	ImpExpColors cImpExpColors(colorInfoArr);

	// 色設定 I/O
	for (size_t i=0; i<types.nColorInfoArrNum; ++i) {
		colorInfoArr[i] = types.colorInfoArr[i];
		_tcscpy(colorInfoArr[i].szName, types.colorInfoArr[i].szName);
	}

	// インポート
	if (!cImpExpColors.ImportUI(hInstance, hwndDlg)) {
		// インポートをしていない
		return false;
	}

	// データのコピー
	types.nColorInfoArrNum = COLORIDX_LAST;
	for (size_t i=0; i<types.nColorInfoArrNum; ++i) {
		types.colorInfoArr[i] = colorInfoArr[i];
		_tcscpy(types.colorInfoArr[i].szName, colorInfoArr[i].szName);
	}
	// ダイアログデータの設定 color
	SetData(hwndDlg);

	return true;
}


// 色の設定をエクスポート
bool PropTypesColor::Export(HWND hwndDlg)
{
	ImpExpColors	cImpExpColors(types.colorInfoArr);

	// エクスポート
	return cImpExpColors.ExportUI(hInstance, hwndDlg);
}



LRESULT APIENTRY ColorList_SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int			xPos = 0;
	int			yPos;
	int			nIndex = -1;
	int			nItemNum;
	RECT		rcItem = {0, 0, 0, 0};
	int			i;
	POINT		poMouse;
	ColorInfo*	pColorInfo;

	switch (uMsg) {
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONUP:
		xPos = LOWORD(lParam);	// horizontal position of cursor
		yPos = HIWORD(lParam);	// vertical position of cursor

		poMouse.x = xPos;
		poMouse.y = yPos;
		nItemNum = List_GetCount(hwnd);
		for (i=0; i<nItemNum; ++i) {
			List_GetItemRect(hwnd, i, &rcItem);
			if (::PtInRect(&rcItem, poMouse)) {
//				MYTRACE(_T("hit at i == %d\n"), i);
//				MYTRACE(_T("\n"));
				nIndex = i;
				break;
			}
		}
		break;
	}
	switch (uMsg) {
	case WM_RBUTTONDOWN:

		if (nIndex == -1) {
			break;
		}
		if (18 <= xPos && xPos <= rcItem.right - 29) {
			List_SetCurSel(hwnd, nIndex);
			::SendMessage(::GetParent(hwnd), WM_COMMAND, MAKELONG(IDC_LIST_COLORS, LBN_SELCHANGE), (LPARAM)hwnd);
			pColorInfo = (ColorInfo*)List_GetItemData(hwnd, nIndex);
			// 下線
			if ((g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_NO_UNDERLINE) == 0) {
				pColorInfo->fontAttr.bUnderLine = !pColorInfo->fontAttr.bUnderLine; // toggle true/false
				::CheckDlgButtonBool(::GetParent(hwnd), IDC_CHECK_UNDERLINE, pColorInfo->fontAttr.bUnderLine);
				::InvalidateRect(hwnd, &rcItem, TRUE);
			}
		}
		break;

	case WM_LBUTTONDBLCLK:
		if (nIndex == -1) {
			break;
		}
		if (18 <= xPos && xPos <= rcItem.right - 29) {
			pColorInfo = (ColorInfo*)List_GetItemData(hwnd, nIndex);
			// 太字で表示
			if ((g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_NO_BOLD) == 0) {
				pColorInfo->fontAttr.bBoldFont = !pColorInfo->fontAttr.bBoldFont; // toggle true/false
				::CheckDlgButtonBool(::GetParent(hwnd), IDC_CHECK_BOLD, pColorInfo->fontAttr.bBoldFont);
				::InvalidateRect(hwnd, &rcItem, TRUE);
			}
		}
		break;
	case WM_LBUTTONUP:
		if (nIndex == -1) {
			break;
		}
		pColorInfo = (ColorInfo*)List_GetItemData(hwnd, nIndex);
		// 色分け/表示 する
		if (2 <= xPos && xPos <= 16
			&& ((g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_FORCE_DISP) == 0)
		) {
			if (pColorInfo->bDisp) {	// 色分け/表示する
				pColorInfo->bDisp = false;
			}else {
				pColorInfo->bDisp = true;
			}
			if (nIndex == COLORIDX_GYOU) {
				pColorInfo = (ColorInfo*)List_GetItemData(hwnd, nIndex);
			}

			::InvalidateRect(hwnd, &rcItem, TRUE);
		}else
		// 前景色見本 矩形
		if (rcItem.right - 27 <= xPos && xPos <= rcItem.right - 27 + 12
			&& ((g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_NO_TEXT) == 0)
		) {
			// 色選択ダイアログ
			// 2005.11.30 Moca カスタム色保持
			DWORD* pColors = (DWORD*)::GetProp(hwnd, _T("ptrCustomColors"));
			if (PropTypesColor::SelectColor(hwnd, &pColorInfo->colorAttr.cTEXT, pColors)) {
				::InvalidateRect(hwnd, &rcItem, TRUE);
				::InvalidateRect(::GetDlgItem(::GetParent(hwnd), IDC_BUTTON_TEXTCOLOR), NULL, TRUE);
			}
		}else
		// 前景色見本 矩形
		if (rcItem.right - 13 <= xPos && xPos <= rcItem.right - 13 + 12
			&& ((g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_NO_BACK) == 0)
		) {
			// 色選択ダイアログ
			DWORD* pColors = (DWORD*)::GetProp(hwnd, _T("ptrCustomColors"));
			if (PropTypesColor::SelectColor(hwnd, &pColorInfo->colorAttr.cBACK, pColors)) {
				::InvalidateRect(hwnd, &rcItem, TRUE);
				::InvalidateRect(::GetDlgItem(::GetParent(hwnd), IDC_BUTTON_BACKCOLOR), NULL, TRUE);
			}
		}
		break;
	case WM_DESTROY:
		if (::GetProp(hwnd, _T("ptrCustomColors"))) {
			::RemoveProp(hwnd, _T("ptrCustomColors"));
		}
		break;
	}
	return CallWindowProc(wpColorListProc, hwnd, uMsg, wParam, lParam);
}


// color メッセージ処理
INT_PTR PropTypesColor::DispatchEvent(
	HWND				hwndDlg,	// handle to dialog box
	UINT				uMsg,		// message
	WPARAM				wParam,		// first message parameter
	LPARAM				lParam 		// second message parameter
	)
{
	WORD				wNotifyCode;
	WORD				wID;
	HWND				hwndCtl;
	NMHDR*				pNMHDR;
	NM_UPDOWN*			pMNUD;
	int					idCtrl;
	int					nVal;
	int					nIndex;
	static HWND			hwndListColor;
	LPDRAWITEMSTRUCT	pDis;

	switch (uMsg) {
	case WM_INITDIALOG:
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		hwndListColor = ::GetDlgItem(hwndDlg, IDC_LIST_COLORS);

		// ダイアログデータの設定 color
		SetData(hwndDlg);

		// 色リストをフック
		wpColorListProc = (WNDPROC) ::SetWindowLongPtr(hwndListColor, GWLP_WNDPROC, (LONG_PTR)ColorList_SubclassProc);
		::SetProp(hwndListColor, _T("ptrCustomColors"), dwCustColors);
		
		return TRUE;

	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	// 通知コード
		wID			= LOWORD(wParam);	// 項目ID､ コントロールID､ またはアクセラレータID
		hwndCtl		= (HWND) lParam;	// コントロールのハンドル
		if (hwndListColor == hwndCtl) {
			switch (wNotifyCode) {
			case LBN_SELCHANGE:
				nIndex = List_GetCurSel(hwndListColor);
				nCurrentColorType = nIndex;		// 現在選択されている色タイプ

				{
					// 各種コントロールの有効／無効を切り替える
					unsigned int fAttribute = g_ColorAttributeArr[nIndex].fAttribute;
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_DISP),			((fAttribute & COLOR_ATTRIB_FORCE_DISP) == 0) ? TRUE: FALSE);
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_BOLD),			((fAttribute & COLOR_ATTRIB_NO_BOLD) == 0) ? TRUE: FALSE);
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_UNDERLINE),		((fAttribute & COLOR_ATTRIB_NO_UNDERLINE) == 0) ? TRUE: FALSE);
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_STATIC_MOZI),			((fAttribute & COLOR_ATTRIB_NO_TEXT) == 0) ? TRUE: FALSE);
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_TEXTCOLOR),		((fAttribute & COLOR_ATTRIB_NO_TEXT) == 0) ? TRUE: FALSE);
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_SAMETEXTCOLOR),	((fAttribute & COLOR_ATTRIB_NO_TEXT) == 0) ? TRUE: FALSE);
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_STATIC_HAIKEI),		((fAttribute & COLOR_ATTRIB_NO_BACK) == 0) ? TRUE: FALSE);
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_BACKCOLOR),		((fAttribute & COLOR_ATTRIB_NO_BACK) == 0) ? TRUE: FALSE);
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_SAMEBKCOLOR),	((fAttribute & COLOR_ATTRIB_NO_BACK) == 0) ? TRUE: FALSE);
				}

				// 色分け/表示 をする
				::CheckDlgButtonBool(hwndDlg, IDC_CHECK_DISP, types.colorInfoArr[nCurrentColorType].bDisp);
				// 太字で表示
				::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BOLD, types.colorInfoArr[nCurrentColorType].fontAttr.bBoldFont);
				// 下線を表示
				::CheckDlgButtonBool(hwndDlg, IDC_CHECK_UNDERLINE, types.colorInfoArr[nCurrentColorType].fontAttr.bUnderLine);

				::InvalidateRect(::GetDlgItem(hwndDlg, IDC_BUTTON_TEXTCOLOR), NULL, TRUE);
				::InvalidateRect(::GetDlgItem(hwndDlg, IDC_BUTTON_BACKCOLOR), NULL, TRUE);
				return TRUE;
			}
		}
		switch (wNotifyCode) {
		// ボタン／チェックボックスがクリックされた
		case BN_CLICKED:
			switch (wID) {
			case IDC_BUTTON_SAMETEXTCOLOR: // 文字色統一
				{
					// 文字色／背景色統一ダイアログを使う
					DlgSameColor dlgSameColor;
					COLORREF cr = types.colorInfoArr[nCurrentColorType].colorAttr.cTEXT;
					dlgSameColor.DoModal(::GetModuleHandle(NULL), hwndDlg, wID, &types, cr);
				}
				::InvalidateRect(hwndListColor, NULL, TRUE);
				return TRUE;

			case IDC_BUTTON_SAMEBKCOLOR:	// 背景色統一
				{
					// 文字色／背景色統一ダイアログを使う
					DlgSameColor dlgSameColor;
					COLORREF cr = types.colorInfoArr[nCurrentColorType].colorAttr.cBACK;
					dlgSameColor.DoModal(::GetModuleHandle(NULL), hwndDlg, wID, &types, cr);
				}
				::InvalidateRect(hwndListColor, NULL, TRUE);
				return TRUE;

			case IDC_BUTTON_TEXTCOLOR:	// テキスト色
				// 色選択ダイアログ
				if (SelectColor(hwndDlg, &types.colorInfoArr[nCurrentColorType].colorAttr.cTEXT, dwCustColors)) {
					::InvalidateRect(::GetDlgItem(hwndDlg, IDC_BUTTON_TEXTCOLOR), NULL, TRUE);
				}
				// 現在選択されている色タイプ
				List_SetCurSel(hwndListColor, nCurrentColorType);
				return TRUE;
			case IDC_BUTTON_BACKCOLOR:	// 背景色
				// 色選択ダイアログ
				if (SelectColor(hwndDlg, &types.colorInfoArr[nCurrentColorType].colorAttr.cBACK, dwCustColors)) {
					::InvalidateRect(::GetDlgItem(hwndDlg, IDC_BUTTON_BACKCOLOR), NULL, TRUE);
				}
				// 現在選択されている色タイプ
				List_SetCurSel(hwndListColor, nCurrentColorType);
				return TRUE;
			case IDC_CHECK_DISP:	// 色分け/表示 をする
				types.colorInfoArr[nCurrentColorType].bDisp = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_DISP);
				// 現在選択されている色タイプ
				List_SetCurSel(hwndListColor, nCurrentColorType);
				types.nRegexKeyMagicNumber = RegexKeyword::GetNewMagicNumber();
				return TRUE;
			case IDC_CHECK_BOLD:	// 太字か
				types.colorInfoArr[nCurrentColorType].fontAttr.bBoldFont = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_BOLD);
				// 現在選択されている色タイプ
				List_SetCurSel(hwndListColor, nCurrentColorType);
				return TRUE;
			case IDC_CHECK_UNDERLINE:	// 下線を表示
				types.colorInfoArr[nCurrentColorType].fontAttr.bUnderLine = ::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_UNDERLINE);
				// 現在選択されている色タイプ
				List_SetCurSel(hwndListColor, nCurrentColorType);
				return TRUE;

			case IDC_BUTTON_IMPORT:	// 色の設定をインポート
				Import(hwndDlg);
				types.nRegexKeyMagicNumber = RegexKeyword::GetNewMagicNumber();
				return TRUE;

			case IDC_BUTTON_EXPORT:	// 色の設定をエクスポート
				Export(hwndDlg);
				return TRUE;

			// 行コメント開始桁指定のON/OFF
			case IDC_CHECK_LCPOS:
			case IDC_CHECK_LCPOS2:
			case IDC_CHECK_LCPOS3:
				EnableTypesPropInput(hwndDlg);
				return TRUE;

			// 強調キーワードの選択
			case IDC_BUTTON_KEYWORD_SELECT:
				{
					DlgKeywordSelect dlgKeywordSelect;
					// 強調キーワード1を取得する。
					HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_SET);
					int nIdx = Combo_GetCurSel(hwndCombo);
					if (nIdx == CB_ERR || nIdx == 0) {
						nSet[0] = -1;
					}else {
						nSet[0] = nIdx - 1;
					}
					dlgKeywordSelect.DoModal(::GetModuleHandle(NULL), hwndDlg, nSet);
					RearrangeKeywordSet(hwndDlg);	// Jan. 23, 2005 genta キーワードセット再配置
					// 強調キーワード1を反映する。
					if (nSet[0] == -1) {
						Combo_SetCurSel(hwndCombo, 0);
					}else {
						Combo_SetCurSel(hwndCombo, nSet[0] + 1);
					}
				}
				break;
			// 強調キーワードの選択
			case IDC_BUTTON_EDITKEYWORD:
				{
					auto pPropKeyword = std::make_unique<PropKeyword>();
					PropCommon* pCommon = (PropCommon*)pPropKeyword.get();
					pCommon->hwndParent = ::GetParent(hwndDlg);
					pCommon->InitData();
					pCommon->nKeywordSet1 = nSet[0];
					INT_PTR res = ::DialogBoxParam(
						SelectLang::getLangRsrcInstance(),
						MAKEINTRESOURCE(IDD_PROP_KEYWORD),
						hwndDlg,
						PropKeyword::DlgProc_dialog,
						(LPARAM)pPropKeyword.get()
					);
					if (res == IDOK) {
						ShareDataLockCounter::WaitLock(pCommon->hwndParent);
						pCommon->ApplyData();
						SetData(hwndDlg);
						bChangeKeywordSet = true;
					}
					return TRUE;
				}
			case IDC_CHECK_STRINGLINEONLY:
				{
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_STRINGENDLINE),
						::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_STRINGLINEONLY));
					return TRUE;
				}
			}
			break;	// BN_CLICKED
		}
		break;	// WM_COMMAND
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch (idCtrl) {
		case IDC_SPIN_LCColNum:
			// 行コメント桁位置
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_LINECOMMENTPOS, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 1) {
				nVal = 1;
			}
			if (nVal > 1000) {
				nVal = 1000;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_LINECOMMENTPOS, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_LCColNum2:
			// 行コメント桁位置
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_LINECOMMENTPOS2, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 1) {
				nVal = 1;
			}
			if (nVal > 1000) {
				nVal = 1000;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_LINECOMMENTPOS2, nVal, FALSE);
			return TRUE;

		case IDC_SPIN_LCColNum3:
			// 行コメント桁位置
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_LINECOMMENTPOS3, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 1) {
				nVal = 1;
			}
			if (nVal > 1000) {
				nVal = 1000;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_LINECOMMENTPOS3, nVal, FALSE);
			return TRUE;
		default:
			switch (pNMHDR->code) {
			case PSN_HELP:
				OnHelp(hwndDlg, IDD_PROP_COLOR);
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE(_T("color PSN_KILLACTIVE\n"));
				// ダイアログデータの取得 color
				GetData(hwndDlg);
				return TRUE;
			case PSN_SETACTIVE:
				nPageNum = ID_PROPTYPE_PAGENUM_COLOR;
				return TRUE;
			}
			break;	// default
		}
		break;	// WM_NOTIFY
	case WM_DRAWITEM:
		idCtrl = (UINT) wParam;				// コントロールのID
		pDis = (LPDRAWITEMSTRUCT) lParam;	// 項目描画情報
		switch (idCtrl) {
		case IDC_BUTTON_TEXTCOLOR:	// テキスト色
			DrawColorButton(pDis, types.colorInfoArr[nCurrentColorType].colorAttr.cTEXT);
			return TRUE;
		case IDC_BUTTON_BACKCOLOR:	// 背景色
			DrawColorButton(pDis, types.colorInfoArr[nCurrentColorType].colorAttr.cBACK);
			return TRUE;
		case IDC_LIST_COLORS:		// 色種別リスト
			DrawColorListItem(pDis);
			return TRUE;
		}
		break;

	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids2);
		}
		return TRUE;
		// NOTREACHED
//		break;

	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids2);
		return TRUE;

	}
	return FALSE;
}


// ダイアログデータの設定 color
void PropTypesColor::SetData(HWND hwndDlg)
{

	HWND	hwndWork;
	int		nItem;

	nCurrentColorType = 0;	// 現在選択されている色タイプ

	// ユーザーがエディット コントロールに入力できるテキストの長さを制限する
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_LINECOMMENT)		, COMMENT_DELIMITER_BUFFERSIZE - 1);
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_LINECOMMENT2)		, COMMENT_DELIMITER_BUFFERSIZE - 1);
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_LINECOMMENT3)		, COMMENT_DELIMITER_BUFFERSIZE - 1);
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM)	, BLOCKCOMMENT_BUFFERSIZE - 1);
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO)	, BLOCKCOMMENT_BUFFERSIZE - 1);
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2), BLOCKCOMMENT_BUFFERSIZE - 1);
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2)	, BLOCKCOMMENT_BUFFERSIZE - 1);

	::DlgItem_SetText(hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM	, types.blockComments[0].getBlockCommentFrom());	// ブロックコメントデリミタ(From)
	::DlgItem_SetText(hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO		, types.blockComments[0].getBlockCommentTo());		// ブロックコメントデリミタ(To)
	::DlgItem_SetText(hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2	, types.blockComments[1].getBlockCommentFrom());	// ブロックコメントデリミタ2(From)
	::DlgItem_SetText(hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2	, types.blockComments[1].getBlockCommentTo());		// ブロックコメントデリミタ2(To)

	// 行コメントの開始桁位置設定
	for (int i=0; i<COMMENT_DELIMITER_NUM; ++i) {
		// テキスト
		::DlgItem_SetText(hwndDlg, cLineComment[i].nEditID, types.lineComment.getLineComment(i));	

		// 桁数チェックと、数値
		int nPos = types.lineComment.getLineCommentPos(i);
		if (nPos >= 0) {
			::CheckDlgButton(hwndDlg, cLineComment[i].nCheckBoxID, TRUE);
			::SetDlgItemInt(hwndDlg, cLineComment[i].nTextID, nPos + 1, FALSE);
		}else {
			::CheckDlgButton(hwndDlg, cLineComment[i].nCheckBoxID, FALSE);
			::SetDlgItemInt(hwndDlg, cLineComment[i].nTextID, (~nPos) + 1, FALSE);
		}
	}

	HWND	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_STRINGLITERAL);
	Combo_ResetContent(hwndCombo);
	int		nSelPos = 0;
	for (size_t i=0; i<_countof(StringLitteralArr); ++i) {
		Combo_InsertString(hwndCombo, i, LS(StringLitteralArr[i].nNameId));
		if (StringLitteralArr[i].nMethod == types.stringType) {		// テキストの折り返し方法
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_STRINGLINEONLY, types.bStringLineOnly);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_STRINGENDLINE, types.bStringEndLine);
	::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_STRINGENDLINE),
		::IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_STRINGLINEONLY));

	// セット名コンボボックスの値セット
	hwndWork = ::GetDlgItem(hwndDlg, IDC_COMBO_SET);
	Combo_ResetContent(hwndWork);  // コンボボックスを空にする
	// 一行目は空白
	Combo_AddString(hwndWork, L" ");
	//	Mar. 31, 2003 genta KeywordSetMgrをポインタに
	if (0 < pKeywordSetMgr->nKeywordSetNum) {
		for (size_t i=0; i<pKeywordSetMgr->nKeywordSetNum; ++i) {
			Combo_AddString(hwndWork, pKeywordSetMgr->GetTypeName(i));
		}
		if (types.nKeywordSetIdx[0] == -1) {
			// セット名コンボボックスのデフォルト選択
			Combo_SetCurSel(hwndWork, 0);
		}else {
			// セット名コンボボックスのデフォルト選択
			Combo_SetCurSel(hwndWork, types.nKeywordSetIdx[0] + 1);
		}
	}

	// 強調キーワード1～10の設定
	for (int i=0; i<MAX_KEYWORDSET_PER_TYPE; ++i) {
		nSet[i] = types.nKeywordSetIdx[i];
	}

	// 色をつける文字種類のリスト
	hwndWork = ::GetDlgItem(hwndDlg, IDC_LIST_COLORS);
	List_ResetContent(hwndWork);  // リストを空にする
	// 大きいフォント対応
	int nItemHeight = TextWidthCalc(hwndWork).GetTextHeight();
	List_SetItemHeight(hwndWork, 0, nItemHeight + 4);
	for (int i=0; i<COLORIDX_LAST; ++i) {
		GetDefaultColorInfoName(&types.colorInfoArr[i], i);
		nItem = ::List_AddString(hwndWork, types.colorInfoArr[i].szName);
		List_SetItemData(hwndWork, nItem, &types.colorInfoArr[i]);
	}
	// 現在選択されている色タイプ
	List_SetCurSel(hwndWork, nCurrentColorType);
	::SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_LIST_COLORS, LBN_SELCHANGE), (LPARAM)hwndWork);

	// 指定位置縦線の設定
	wchar_t szVertLine[MAX_VERTLINES * 15] = L"";
	int offset = 0;
	for (int i=0; i<MAX_VERTLINES && types.nVertLineIdx[i]!=0; ++i) {
		int nXCol = types.nVertLineIdx[i];
		int nXColEnd = nXCol;
		int nXColAdd = 1;
		if (nXCol < 0) {
			if (i < MAX_VERTLINES - 2) {
				nXCol = -nXCol;
				nXColEnd = types.nVertLineIdx[++i];
				nXColAdd = types.nVertLineIdx[++i];
				if (nXColEnd < nXCol || nXColAdd <= 0) {
					continue;
				}
				if (offset) {
					szVertLine[offset] = ',';
					szVertLine[offset + 1] = '\0';
					offset += 1;
				}
				offset += auto_sprintf(&szVertLine[offset], L"%d(%d,%d)", nXColAdd, nXCol, nXColEnd);
			}
		}else {
			if (offset) {
				szVertLine[offset] = ',';
				szVertLine[offset + 1] = '\0';
				offset += 1;
			}
			offset += auto_sprintf(&szVertLine[offset], L"%d", nXCol);
		}
	}
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_VERTLINE), MAX_VERTLINES * 15);
	::DlgItem_SetText(hwndDlg, IDC_EDIT_VERTLINE, szVertLine);
	return;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ダイアログデータの取得 color
int PropTypesColor::GetData(HWND hwndDlg)
{
	int		nIdx;
	HWND	hwndWork;

	// From Here May 12, 2001 genta
	// コメントの開始桁位置の取得
	// May 21, 2001 genta 桁位置を1から数えるように
	wchar_t buffer[COMMENT_DELIMITER_BUFFERSIZE];	// LineCommentを取得するためのバッファ
	int pos;
	bool en;
	BOOL bTranslated;

	for (int i=0; i<COMMENT_DELIMITER_NUM; ++i) {
		en = DlgButton_IsChecked(hwndDlg, cLineComment[i].nCheckBoxID);
		pos = ::GetDlgItemInt(hwndDlg, cLineComment[i].nTextID, &bTranslated, FALSE);
		if (!bTranslated) {
			en = false;
			pos = 0;
		}
		//	pos == 0のときは無効扱い
		if (pos == 0)	en = false;
		else			--pos;
		//	無効のときは1の補数で格納

		::DlgItem_GetText(hwndDlg, cLineComment[i].nEditID		, buffer	, COMMENT_DELIMITER_BUFFERSIZE);		// 行コメントデリミタ
		types.lineComment.CopyTo(i, buffer, en ? pos : ~pos);
	}

	wchar_t szFromBuffer[BLOCKCOMMENT_BUFFERSIZE];
	wchar_t szToBuffer[BLOCKCOMMENT_BUFFERSIZE];

	::DlgItem_GetText(hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM	, szFromBuffer	, BLOCKCOMMENT_BUFFERSIZE);		// ブロックコメントデリミタ(From)
	::DlgItem_GetText(hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO		, szToBuffer	, BLOCKCOMMENT_BUFFERSIZE);	// ブロックコメントデリミタ(To)
	types.blockComments[0].SetBlockCommentRule(szFromBuffer, szToBuffer);

	::DlgItem_GetText(hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2	, szFromBuffer	, BLOCKCOMMENT_BUFFERSIZE);	// ブロックコメントデリミタ(From)
	::DlgItem_GetText(hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2	, szToBuffer	, BLOCKCOMMENT_BUFFERSIZE);	// ブロックコメントデリミタ(To)
	types.blockComments[1].SetBlockCommentRule(szFromBuffer, szToBuffer);

	// 文字列区切り記号エスケープ方法
	int		nSelPos = Combo_GetCurSel(GetDlgItem(hwndDlg, IDC_COMBO_STRINGLITERAL));
	if (nSelPos >= 0) {
		types.stringType = StringLitteralArr[nSelPos].nMethod;
	}
	types.bStringLineOnly = IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_STRINGLINEONLY);
	types.bStringEndLine = IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_STRINGENDLINE);
	

	// セット名コンボボックスの値セット
	hwndWork = ::GetDlgItem(hwndDlg, IDC_COMBO_SET);
	nIdx = Combo_GetCurSel(hwndWork);
	if (nIdx == CB_ERR ||
		nIdx == 0
	) {
		types.nKeywordSetIdx[0] = -1;
	}else {
		types.nKeywordSetIdx[0] = nIdx - 1;
	}

	// 強調キーワード2～10の取得(1は別)
	for (nIdx=1; nIdx<MAX_KEYWORDSET_PER_TYPE; ++nIdx) {
		types.nKeywordSetIdx[nIdx] = nSet[nIdx];
	}

	// 指定位置縦線の設定
	wchar_t szVertLine[MAX_VERTLINES * 15];
	::DlgItem_GetText(hwndDlg, IDC_EDIT_VERTLINE, szVertLine, MAX_VERTLINES * 15);

	int offset = 0;
	int i = 0;
	while (i < MAX_VERTLINES) {
		int value = 0;
		for (; '0'<=szVertLine[offset] && szVertLine[offset]<='9'; ++offset) {
			value = szVertLine[offset] - '0' + value * 10;
		}
		if (value <= 0) {
			break;
		}
		if (szVertLine[offset] == '(') {
			++offset;
			int valueBegin = 0;
			int valueEnd = 0;
			for (; '0'<=szVertLine[offset] && szVertLine[offset]<='9'; ++offset) {
				valueBegin = szVertLine[offset] - '0' + valueBegin * 10;
			}
			if (valueBegin <= 0) {
				break;
			}
			if (szVertLine[offset] == ',') {
				++offset;
			}else if (szVertLine[offset] != ')') {
				break;
			}
			for (; '0'<=szVertLine[offset] && szVertLine[offset]<='9'; ++offset) {
				valueEnd = szVertLine[offset] - '0' + valueEnd * 10;
			}
			if (valueEnd <= 0) {
				valueEnd = MAXLINEKETAS;
			}
			if (szVertLine[offset] != ')') {
				break;
			}
			++offset;
			if (i + 2 < MAX_VERTLINES) {
				types.nVertLineIdx[i++] = -valueBegin;
				types.nVertLineIdx[i++] = valueEnd;
				types.nVertLineIdx[i++] = value;
			}else {
				break;
			}
		}else {
			types.nVertLineIdx[i++] = value;
		}
		if (szVertLine[offset] != ',') {
			break;
		}
		++offset;
	}
	if (i < MAX_VERTLINES) {
		types.nVertLineIdx[i] = 0;
	}
	return TRUE;
}


// 色ボタンの描画
void PropTypesColor::DrawColorButton(DRAWITEMSTRUCT* pDis, COLORREF color)
{
//	MYTRACE(_T("pDis->itemAction = "));

	COLORREF	cBtnHiLight		= (COLORREF)::GetSysColor(COLOR_3DHILIGHT);
	COLORREF	cBtnShadow		= (COLORREF)::GetSysColor(COLOR_3DSHADOW);
	COLORREF	cBtnDkShadow	= (COLORREF)::GetSysColor(COLOR_3DDKSHADOW);
	COLORREF	cBtnFace		= (COLORREF)::GetSysColor(COLOR_3DFACE);
	RECT		rc;
	RECT		rcFocus;

	// 描画対象
	Graphics gr(pDis->hDC);

	// ボタンの表面の色で塗りつぶす
	gr.SetBrushColor(cBtnFace);
	gr.FillMyRect(pDis->rcItem);

	// 枠の描画
	rc = pDis->rcItem;
	rc.top += 4;
	rc.left += 4;
	rc.right -= 4;
	rc.bottom -= 4;
	rcFocus = rc;
//	rc.right -= 11;

	if (pDis->itemState & ODS_SELECTED) {

		gr.SetPen(cBtnDkShadow);
		::MoveToEx(gr, 0, pDis->rcItem.bottom - 2, NULL);
		::LineTo(gr, 0, 0);
		::LineTo(gr, pDis->rcItem.right - 1, 0);

		gr.SetPen(cBtnShadow);
		::MoveToEx(gr, 1, pDis->rcItem.bottom - 3, NULL);
		::LineTo(gr, 1, 1);
		::LineTo(gr, pDis->rcItem.right - 2, 1);

		gr.SetPen(cBtnHiLight);
		::MoveToEx(gr, 0, pDis->rcItem.bottom - 1, NULL);
		::LineTo(gr, pDis->rcItem.right - 1, pDis->rcItem.bottom - 1);
		::LineTo(gr, pDis->rcItem.right - 1, -1);

		rc.top += 1;
		rc.left += 1;
		rc.right += 1;
		rc.bottom += 1;

		rcFocus.top += 1;
		rcFocus.left += 1;
		rcFocus.right += 1;
		rcFocus.bottom += 1;

	}else {
		gr.SetPen(cBtnHiLight);
		::MoveToEx(gr, 0, pDis->rcItem.bottom - 2, NULL);
		::LineTo(gr, 0, 0);
		::LineTo(gr, pDis->rcItem.right - 1, 0);

		gr.SetPen(cBtnShadow);
		::MoveToEx(gr, 1, pDis->rcItem.bottom - 2, NULL);
		::LineTo(gr, pDis->rcItem.right - 2, pDis->rcItem.bottom - 2);
		::LineTo(gr, pDis->rcItem.right - 2, 0);

		gr.SetPen(cBtnDkShadow);
		::MoveToEx(gr, 0, pDis->rcItem.bottom - 1, NULL);
		::LineTo(gr, pDis->rcItem.right - 1, pDis->rcItem.bottom - 1);
		::LineTo(gr, pDis->rcItem.right - 1, -1);
	}
	
	if ((pDis->itemState & ODS_DISABLED) == 0) {
		// 指定色で塗りつぶす
		gr.SetBrushColor(color);
		gr.SetPen(cBtnShadow);
		::RoundRect(gr, rc.left, rc.top, rc.right, rc.bottom , 5, 5);
	}

	// フォーカスの長方形
	if (pDis->itemState & ODS_FOCUS) {
		rcFocus.top -= 3;
		rcFocus.left -= 3;
		rcFocus.right += 2;
		rcFocus.bottom += 2;
		::DrawFocusRect(gr, &rcFocus);
	}
}


//	チェック状態に応じてダイアログボックス要素のEnable/Disableを
//	適切に設定する
void PropTypesColor::EnableTypesPropInput(HWND hwndDlg)
{
	//	行コメント開始桁位置入力ボックスのEnable/Disable設定
	//	1つ目
	if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_LCPOS)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_LINECOMMENTPOS), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_LCPOS), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_LCColNum), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_LINECOMMENTPOS), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_LCPOS), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_LCColNum), FALSE);
	}
	//	2つ目
	if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_LCPOS2)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_LINECOMMENTPOS2), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_LCPOS2), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_LCColNum2), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_LINECOMMENTPOS2), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_LCPOS2), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_LCColNum2), FALSE);
	}
	//	3つ目
	if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_LCPOS3)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_LINECOMMENTPOS3), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_LCPOS3), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_LCColNum3), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_LINECOMMENTPOS3), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_LCPOS3), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_LCColNum3), FALSE);
	}
}


/*!	@brief キーワードセットの再配列

	キーワードセットの色分けでは未指定のキーワードセット以降はチェックを省略する．
	そのためセットの途中に未指定のものがある場合はそれ以降を前に詰めることで
	指定された全てのキーワードセットが有効になるようにする．
	その際，色分けの設定も同時に移動する．

	nSet, types.colorInfoArr[]が変更される．

	@param hwndDlg [in] ダイアログボックスのウィンドウハンドル
*/
void PropTypesColor::RearrangeKeywordSet(HWND hwndDlg)
{
	for (int i=0; i<MAX_KEYWORDSET_PER_TYPE; ++i) {
		if (nSet[i] != -1) {
			continue;
		}

		// 未設定の場合
		int j;
		for (j=i; j<MAX_KEYWORDSET_PER_TYPE; ++j) {
			if (nSet[j] != -1) {
				// 後ろに設定済み項目があった場合
				nSet[i] = nSet[j];
				nSet[j] = -1;

				// 色設定を入れ替える
				// 構造体ごと入れ替えると名前が変わってしまうので注意
				ColorInfo &col1 = types.colorInfoArr[COLORIDX_KEYWORD1 + i];
				ColorInfo &col2   = types.colorInfoArr[COLORIDX_KEYWORD1 + j];

				std::swap(col1.bDisp, col2.bDisp);
				std::swap(col1.fontAttr, col2.fontAttr);
				std::swap(col1.colorAttr, col2.colorAttr);

				break;
			}
		}
		if (j == MAX_KEYWORDSET_PER_TYPE) {
			// 後ろには設定済み項目がなかった
			break;
		}
	}
	
	// リストボックス及び色設定ボタンを再描画
	::InvalidateRect(::GetDlgItem(hwndDlg, IDC_BUTTON_TEXTCOLOR), NULL, TRUE);
	::InvalidateRect(::GetDlgItem(hwndDlg, IDC_BUTTON_BACKCOLOR), NULL, TRUE);
	::InvalidateRect(::GetDlgItem(hwndDlg, IDC_LIST_COLORS), NULL, TRUE);
}


// 色種別リスト オーナー描画
void PropTypesColor::DrawColorListItem(DRAWITEMSTRUCT* pDis)
{
	ColorInfo*	pColorInfo;
//	RECT		rc0,rc1,rc2;
	RECT		rc1;
	COLORREF	cRim = (COLORREF)::GetSysColor(COLOR_3DSHADOW);

	if (!pDis || pDis->itemData == 0) {
		return;
	}

	// 描画対象
	Graphics gr(pDis->hDC);

//	rc0 = pDis->rcItem;
	rc1 = pDis->rcItem;
//	rc2 = pDis->rcItem;

	// アイテムデータの取得
	pColorInfo = (ColorInfo*)pDis->itemData;

	// アイテム矩形塗りつぶし
	gr.SetBrushColor(::GetSysColor(COLOR_WINDOW));
	gr.FillMyRect(pDis->rcItem);
	
	// アイテムが選択されている
	if (pDis->itemState & ODS_SELECTED) {
		gr.SetBrushColor(::GetSysColor(COLOR_HIGHLIGHT));
		gr.SetTextForeColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
	}else {
		gr.SetBrushColor(::GetSysColor(COLOR_WINDOW));
		gr.SetTextForeColor(::GetSysColor(COLOR_WINDOWTEXT));
	}

	rc1.left += (2 + 16);
	rc1.top += 2;
	rc1.right -= (2 + 27);
	rc1.bottom -= 2;
	// 選択ハイライト矩形
	gr.FillMyRect(rc1);
	// テキスト
	::SetBkMode(gr, TRANSPARENT);
	::TextOut(gr, rc1.left, rc1.top, pColorInfo->szName, (int)_tcslen(pColorInfo->szName));
	if (pColorInfo->fontAttr.bBoldFont) {	// 太字か
		::TextOut(gr, rc1.left + 1, rc1.top, pColorInfo->szName, (int)_tcslen(pColorInfo->szName));
	}
	if (pColorInfo->fontAttr.bUnderLine) {	// 下線か
		SIZE	sz;
		::GetTextExtentPoint32(gr, pColorInfo->szName, (int)_tcslen(pColorInfo->szName), &sz);
		::MoveToEx(gr, rc1.left,		rc1.bottom - 2, NULL);
		::LineTo(gr, rc1.left + sz.cx,	rc1.bottom - 2);
		::MoveToEx(gr, rc1.left,		rc1.bottom - 1, NULL);
		::LineTo(gr, rc1.left + sz.cx,	rc1.bottom - 1);
	}

	// アイテムにフォーカスがある
	if (pDis->itemState & ODS_FOCUS) {
		::DrawFocusRect(gr, &pDis->rcItem);
	}

	//「色分け/表示する」のチェック
	rc1 = pDis->rcItem;
	rc1.left += 2;
	rc1.top += 3;
	rc1.right = rc1.left + 12;
	rc1.bottom = rc1.top + 12;
	if (pColorInfo->bDisp) {	// 色分け/表示する
		// テキスト色を使う（「ハイコントラスト黒」のような設定でも見えるように）
		gr.SetPen(::GetSysColor(COLOR_WINDOWTEXT));

		::MoveToEx(gr,	rc1.left + 2, rc1.top + 6, NULL);
		::LineTo(gr,	rc1.left + 5, rc1.bottom - 3);
		::LineTo(gr,	rc1.right - 2, rc1.top + 4);
		rc1.top -= 1;
		rc1.bottom -= 1;
		::MoveToEx(gr,	rc1.left + 2, rc1.top + 6, NULL);
		::LineTo(gr,	rc1.left + 5, rc1.bottom - 3);
		::LineTo(gr,	rc1.right - 2, rc1.top + 4);
		rc1.top -= 1;
		rc1.bottom -= 1;
		::MoveToEx(gr,	rc1.left + 2, rc1.top + 6, NULL);
		::LineTo(gr,	rc1.left + 5, rc1.bottom - 3);
		::LineTo(gr,	rc1.right - 2, rc1.top + 4);
	}
//	return;


	if ((g_ColorAttributeArr[pColorInfo->nColorIdx].fAttribute & COLOR_ATTRIB_NO_BACK) == 0) {
		// 背景色 見本矩形
		rc1 = pDis->rcItem;
		rc1.left = rc1.right - 13;
		rc1.top += 2;
		rc1.right = rc1.left + 12;
		rc1.bottom -= 2;

		gr.SetBrushColor(pColorInfo->colorAttr.cBACK);
		gr.SetPen(cRim);
		::RoundRect(pDis->hDC, rc1.left, rc1.top, rc1.right, rc1.bottom , 3, 3);
	}

	if ((g_ColorAttributeArr[pColorInfo->nColorIdx].fAttribute & COLOR_ATTRIB_NO_TEXT) == 0) {
		// 前景色 見本矩形
		rc1 = pDis->rcItem;
		rc1.left = rc1.right - 27;
		rc1.top += 2;
		rc1.right = rc1.left + 12;
		rc1.bottom -= 2;
		gr.SetBrushColor(pColorInfo->colorAttr.cTEXT);
		gr.SetPen(cRim);
		::RoundRect(pDis->hDC, rc1.left, rc1.top, rc1.right, rc1.bottom , 3, 3);
	}
}


// 色選択ダイアログ
BOOL PropTypesColor::SelectColor(
	HWND hwndParent,
	COLORREF* pColor,
	DWORD* pCustColors
	)
{
	CHOOSECOLOR cc;
	cc.lStructSize = sizeof_raw(cc);
	cc.hwndOwner = hwndParent;
	cc.hInstance = NULL;
	cc.rgbResult = *pColor;
	cc.lpCustColors = pCustColors;
	cc.Flags = /*CC_PREVENTFULLOPEN |*/ CC_RGBINIT;
	cc.lCustData = 0;
	cc.lpfnHook = nullptr;
	cc.lpTemplateName = NULL;
	if (!::ChooseColor(&cc)) {
		return FALSE;
	}
	*pColor = cc.rgbResult;
	return TRUE;
}

