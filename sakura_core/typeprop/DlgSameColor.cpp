// 文字色／背景色統一ダイアログ

#include "StdAfx.h"
#include "DlgSameColor.h"
#include "types/Type.h"
#include "view/colors/EColorIndexType.h"
#include "uiparts/Graphics.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids[] = {
	IDOK,						HIDOK_SAMECOLOR,						// OK
	IDCANCEL,					HIDCANCEL_SAMECOLOR,					// キャンセル
	IDC_BUTTON_HELP,			HIDC_BUTTON_SAMECOLOR_HELP,				// ヘルプ
	IDC_LIST_COLORS,			HIDC_LIST_SAMECOLOR_COLORS,				// 変更対象の色
	IDC_BUTTON_SELALL,			HIDC_BUTTON_SAMECOLOR_SELALL,			// 全チェック
	IDC_BUTTON_SELNOTING,		HIDC_BUTTON_SAMECOLOR_SELNOTING,		// 全解除
	IDC_LIST_ITEMINFO,			HIDC_LIST_SAMECOLOR_ITEMINFO,			// 選択中の色に対応する項目のリスト
	IDC_STATIC_COLOR,			HIDC_STATIC_COLOR,						// 統一色
	0, 0
};

LPVOID DlgSameColor::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

DlgSameColor::DlgSameColor() :
	wpColorStaticProc(nullptr),
	wpColorListProc(nullptr),
	wID(0),
	pTypes(nullptr),
	cr(0)
{
	return;
}

DlgSameColor::~DlgSameColor()
{
	return;
}

/*!
	標準以外のメッセージを捕捉する
*/
INT_PTR DlgSameColor::DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR result;
	result = Dialog::DispatchEvent(hWnd, wMsg, wParam, lParam);
	switch (wMsg) {
	case WM_COMMAND:
		// 色選択リストボックスの選択が変更された場合の処理
		if (LOWORD(wParam) == IDC_LIST_COLORS && HIWORD(wParam) == LBN_SELCHANGE) {
			OnSelChangeListColors((HWND)lParam);
		}
		break;

	case WM_CTLCOLORLISTBOX:
		{
			// 項目リストの背景色を設定する処理
			HWND hwndLB = (HWND)lParam;
			if (::GetDlgCtrlID(hwndLB) == IDC_LIST_ITEMINFO) {
				HDC hdcLB = (HDC)wParam;
				::SetTextColor(hdcLB, ::GetSysColor(COLOR_WINDOWTEXT));
				::SetBkMode(hdcLB, TRANSPARENT);
				return (INT_PTR)::GetSysColorBrush(COLOR_BTNFACE);
			}
		}
		break;

	default:
		break;
	}
	return result;
}

/*! モーダルダイアログの表示
	@param wID [in] タイプ別設定ダイアログで押されたボタンID
	@param pTypes  [in/out] タイプ別設定データ
	@param cr [in] 指定色
*/
INT_PTR DlgSameColor::DoModal(HINSTANCE hInstance, HWND hwndParent, WORD wID, TypeConfig* pTypes, COLORREF cr)
{
	wID = wID;
	this->pTypes = pTypes;
	cr = cr;

	(void)Dialog::DoModal(hInstance, hwndParent, IDD_SAMECOLOR, (LPARAM)NULL);

	return TRUE;
}

/*! WM_INITDIALOG 処理 */
BOOL DlgSameColor::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	BOOL bRet = Dialog::OnInitDialog(hwndDlg, wParam, lParam);

	HWND hwndStatic = GetItemHwnd(IDC_STATIC_COLOR);
	HWND hwndList = GetItemHwnd(IDC_LIST_COLORS);

	// 指定色スタティック、色選択リストをサブクラス化
	::SetWindowLongPtr(hwndStatic, GWLP_USERDATA, (LONG_PTR)this);
	wpColorStaticProc = (WNDPROC)::SetWindowLongPtr(hwndStatic, GWLP_WNDPROC, (LONG_PTR)ColorStatic_SubclassProc);
	::SetWindowLongPtr(hwndList, GWLP_USERDATA, (LONG_PTR)this);
	wpColorListProc = (WNDPROC)::SetWindowLongPtr(hwndList, GWLP_WNDPROC, (LONG_PTR)ColorList_SubclassProc);


	wchar_t szText[30];
	int nItem;

	switch (wID) {	// タイプ別設定ダイアログで押されたボタンID
	case IDC_BUTTON_SAMETEXTCOLOR:
		// タイプ別設定から文字色を重複しないように取り出す
		::SetWindowText(GetHwnd(), LS(STR_DLGSMCLR_BTN1));
		for (int i=0; i<COLORIDX_LAST; ++i) {
			if ((g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_TEXT) != 0) {
				continue;
			}
			if (cr != pTypes->colorInfoArr[i].colorAttr.cTEXT) {
				_ultow(pTypes->colorInfoArr[i].colorAttr.cTEXT, szText, 10);
				if (List_FindStringExact(hwndList, -1, szText) == LB_ERR) {
					nItem = ::List_AddString(hwndList, szText);
					List_SetItemData(hwndList, nItem, FALSE); 
				}
			}
		}
		break;

	case IDC_BUTTON_SAMEBKCOLOR:
		// タイプ別設定から背景色を重複しないように取り出す
		::SetWindowText(GetHwnd(), LS(STR_DLGSMCLR_BTN2));
		for (int i=0; i<COLORIDX_LAST; ++i) {
			if ((g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_BACK) != 0) {
				continue;
			}
			if (cr != pTypes->colorInfoArr[i].colorAttr.cBACK) {
				_ultow(pTypes->colorInfoArr[i].colorAttr.cBACK, szText, 10);
				if (List_FindStringExact(hwndList, -1, szText) == LB_ERR) {
					nItem = ::List_AddString(hwndList, szText);
					List_SetItemData(hwndList, nItem, FALSE); 
				}
			}
		}
		break;

	default:
		CloseDialog(IDCANCEL);
		break;
	}

	if (0 < List_GetCount(hwndList)) {
		List_SetCurSel(hwndList, 0);
		OnSelChangeListColors(hwndList);
	}

	return bRet;
}

/*! BN_CLICKED 処理 */
BOOL DlgSameColor::OnBnClicked(int wID)
{
	HWND hwndList = GetItemHwnd(IDC_LIST_COLORS);
	int nItemNum = List_GetCount(hwndList);
	BOOL bCheck;

	switch (wID) {
	case IDC_BUTTON_HELP:
		// ヘルプ
		MyWinHelp(GetHwnd(), HELP_CONTEXT, HLP000316);
		return TRUE;

	case IDC_BUTTON_SELALL:
	case IDC_BUTTON_SELNOTING:
		// 全選択／全解除の処理
		bCheck = (wID == IDC_BUTTON_SELALL);
		for (int i=0; i<nItemNum; ++i) {
			List_SetItemData(hwndList, i, bCheck);
		}
		::InvalidateRect(hwndList, NULL, TRUE);
		break;

	case IDOK:
		// タイプ別設定から選択色と同色のものを取り出して指定色に一括変更する
		wchar_t szText[30];
		LPWSTR pszStop;
		COLORREF cr;

		for (int i=0; i<nItemNum; ++i) {
			bCheck = (BOOL)List_GetItemData(hwndList, i);
			if (bCheck) {
				List_GetText(hwndList, i, szText);
				cr = wcstoul(szText, &pszStop, 10);

				switch (wID) {
				case IDC_BUTTON_SAMETEXTCOLOR:
					for (int j=0; j<COLORIDX_LAST; ++j) {
						auto& colorAttr = pTypes->colorInfoArr[j].colorAttr.cTEXT;
						if (cr == colorAttr) {
							colorAttr = cr;
						}
					}
					break;

				case IDC_BUTTON_SAMEBKCOLOR:
					for (int j=0; j<COLORIDX_LAST; ++j) {
						auto& colorAttr = pTypes->colorInfoArr[j].colorAttr.cBACK;
						if (cr == colorAttr) {
							colorAttr = cr;
						}
					}
					break;

				default:
					break;
				}
			}
		}
		break;

	case IDCANCEL:
		break;
	}
	return Dialog::OnBnClicked(wID);
}

/*! WM_DRAWITEM 処理 */
BOOL DlgSameColor::OnDrawItem(WPARAM wParam, LPARAM lParam)
{
	LPDRAWITEMSTRUCT pDis = (LPDRAWITEMSTRUCT)lParam;	// 項目描画情報
	if (IDC_LIST_COLORS != pDis->CtlID) {	// オーナー描画にしているのは色選択リストだけ
		return TRUE;
	}

	// 描画対象
	Graphics gr(pDis->hDC);

	//
	// 色選択リストの描画処理
	//
	RECT		rc;
	wchar_t		szText[30];
	LPWSTR		pszStop;
	COLORREF	cr;

	List_GetText(pDis->hwndItem, pDis->itemID, szText);
	cr = wcstoul(szText, &pszStop, 10);

	rc = pDis->rcItem;

	// アイテム矩形塗りつぶし
	::FillRect(gr, &pDis->rcItem, ::GetSysColorBrush(COLOR_WINDOW));

	// アイテムが選択状態
	if (pDis->itemState & ODS_SELECTED) {
		rc = pDis->rcItem;
		rc.left += (rc.bottom - rc.top);
		::FillRect(gr, &rc, ::GetSysColorBrush(COLOR_HIGHLIGHT));
	}

	// アイテムにフォーカスがある
	if (pDis->itemState & ODS_FOCUS) {
		::DrawFocusRect(gr, &pDis->rcItem);
	}

	// チェックボックス表示
	rc = pDis->rcItem;
	rc.top += 2;
	rc.bottom -= 2;
	rc.left += 2;
	rc.right = rc.left + (rc.bottom - rc.top);
	UINT uState =  DFCS_BUTTONCHECK | DFCS_FLAT;
	if ((BOOL)pDis->itemData) {
		uState |= DFCS_CHECKED;		// チェック状態
	}
	::DrawFrameControl(gr, &rc, DFC_BUTTON, uState);

	// 色見本矩形
	rc = pDis->rcItem;
	rc.left += rc.bottom - rc.top + 2;
	rc.top += 2;
	rc.bottom -= 2;
	rc.right -= 2;
	gr.SetBrushColor(cr);
	gr.SetPen(::GetSysColor(COLOR_3DSHADOW));
	::RoundRect(gr, rc.left, rc.top, rc.right, rc.bottom , 5, 5);

	return TRUE;
}

/*! 色選択リストの LBN_SELCHANGE 処理 */
BOOL DlgSameColor::OnSelChangeListColors(HWND hwndCtl)
{
	// 色選択リストで現在フォーカスのある色について
	// タイプ別設定から同色の項目を取り出して項目リストに表示する
	HWND hwndListInfo = GetItemHwnd(IDC_LIST_ITEMINFO);
	List_ResetContent(hwndListInfo);

	int i = List_GetCaretIndex(hwndCtl);
	if (i != LB_ERR) {
		wchar_t szText[30];
		List_GetText(hwndCtl, i, szText);
		LPWSTR pszStop;
		COLORREF cr = wcstoul(szText, &pszStop, 10);

		switch (wID) {
		case IDC_BUTTON_SAMETEXTCOLOR:
			for (int j=0; j<COLORIDX_LAST; ++j) {
				if ((g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_TEXT) != 0) {
					continue;
				}
				if (cr == pTypes->colorInfoArr[j].colorAttr.cTEXT) {
					::List_AddString(hwndListInfo, pTypes->colorInfoArr[j].szName);
				}
			}
			break;

		case IDC_BUTTON_SAMEBKCOLOR:
			for (int j=0; j<COLORIDX_LAST; ++j) {
				if ((g_ColorAttributeArr[j].fAttribute & COLOR_ATTRIB_NO_BACK) != 0) {	// 2006.12.18 ryoji フラグ利用で簡素化
					continue;
				}
				if (cr == pTypes->colorInfoArr[j].colorAttr.cBACK) {
					::List_AddString(hwndListInfo, pTypes->colorInfoArr[j].szName);
				}
			}
			break;

		default:
			break;
		}
	}

	return TRUE;
}

/*! サブクラス化された指定色スタティックのウィンドウプロシージャ */
LRESULT CALLBACK DlgSameColor::ColorStatic_SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC		hDC;
	RECT	rc;

	DlgSameColor* pDlgSameColor = (DlgSameColor*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (uMsg) {
	case WM_PAINT:
		// ウィンドウ描画
		PAINTSTRUCT ps;

		hDC = ::BeginPaint(hwnd, &ps);

		// 色見本矩形
		::GetClientRect(hwnd, &rc);
		rc.left += 2;
		rc.top += 2;
		rc.right -=2;
		rc.bottom -= 2;
		{
			Graphics gr(hDC);
			gr.SetBrushColor(pDlgSameColor->cr);
			gr.SetPen(::GetSysColor(COLOR_3DSHADOW));
			::RoundRect(gr, rc.left, rc.top, rc.right, rc.bottom, 5, 5);
		}
		::EndPaint(hwnd, &ps);
		return (LRESULT)0;

	case WM_ERASEBKGND:
		// 背景描画
		hDC = (HDC)wParam;
		::GetClientRect(hwnd, &rc);

		// 親にWM_CTLCOLORSTATICを送って背景ブラシを取得し、背景描画する
		{
			HBRUSH	hBrush = (HBRUSH)::SendMessage(GetParent(hwnd), WM_CTLCOLORSTATIC, wParam, (LPARAM)hwnd);
			HBRUSH	hBrushOld = (HBRUSH)::SelectObject(hDC, hBrush);
			::FillRect(hDC, &rc, hBrush);
			::SelectObject(hDC, hBrushOld);
		}
		return (LRESULT)1;

	case WM_DESTROY:
		// サブクラス化解除
		::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)pDlgSameColor->wpColorStaticProc);
		pDlgSameColor->wpColorStaticProc = nullptr;
		return (LRESULT)0;

	default:
		break;
	}

	return CallWindowProc(pDlgSameColor->wpColorStaticProc, hwnd, uMsg, wParam, lParam);
}

/*! サブクラス化された色選択リストのウィンドウプロシージャ */
LRESULT CALLBACK DlgSameColor::ColorList_SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT po;
	RECT rcItem;
	RECT rc;
	int nItemNum;

	DlgSameColor* pDlgSameColor = (DlgSameColor*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (uMsg) {
	case WM_LBUTTONUP:
		// マウスボタン下にある項目の選択／選択解除をトグルする
		po.x = LOWORD(lParam);	// horizontal position of cursor
		po.y = HIWORD(lParam);	// vertical position of cursor
		nItemNum = List_GetCount(hwnd);
		for (int i=0; i<nItemNum; ++i) {
			List_GetItemRect(hwnd, i, &rcItem);
			rc = rcItem;
			rc.top += 2;
			rc.bottom -= 2;
			rc.left += 2;
			rc.right = rc.left + (rc.bottom - rc.top);
			if (::PtInRect(&rc, po)) {
				BOOL bCheck = !(BOOL)List_GetItemData(hwnd, i);
				List_SetItemData(hwnd, i, bCheck);
				::InvalidateRect(hwnd, &rcItem, TRUE);
				break;
			}
		}
		break;

	case WM_KEYUP:
		// フォーカス項目の選択／選択解除をトグルする
		if (wParam == VK_SPACE) {
			int i = List_GetCaretIndex(hwnd);
			if (i != LB_ERR) {
				BOOL bCheck = !(BOOL)List_GetItemData(hwnd, i);
				List_SetItemData(hwnd, i, bCheck);
				::InvalidateRect(hwnd, NULL, TRUE);
			}
		}
		break;

	case WM_DESTROY:
		// サブクラス化解除
		::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)pDlgSameColor->wpColorListProc);
		pDlgSameColor->wpColorListProc = NULL;
		return (LRESULT)0;

	default:
		break;
	}

	return ::CallWindowProc(pDlgSameColor->wpColorListProc, hwnd, uMsg, wParam, lParam);
}

