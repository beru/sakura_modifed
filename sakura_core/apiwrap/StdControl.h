#pragma once

/*
内部コードがWCHARなので、検索キーワードなどもWCHARで保持する。
そのため、検索ダイアログのコンボボックスなどに、WCHARを設定する場面が出てくる。
UNICODE版では問題無いが、ANSI版では設定の前にコード変換する必要がある。
呼び出し側で変換しても良いが、頻度が多いので、WCHARを直接受け取るAPIラップ関数を提供する。

また、SendMessageの直接呼び出しは、どうしてもWPARAM,LPARAMへの強制キャストが生じるため、
コンパイラの型チェックが働かず、wchar_t, charの混在するソースコードの中ではバグの温床になりやすい。
そういった意味でも、このファイル内のラップ関数を使うことを推奨する。
*/

#include "../util/tchar_convert.h"

namespace ApiWrap {

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      ウィンドウ共通                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline BOOL Wnd_SetText(HWND hwnd, const char* str) {
		return SetWindowTextA(hwnd, str);
	}
	inline BOOL Wnd_SetText(HWND hwnd, const wchar_t* str) {
		return SetWindowTextW(hwnd, str);
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      コンボボックス                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline LRESULT Combo_AddString(HWND hwndCombo, const char* str) {
		return ::SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)to_tchar(str));
	}

	inline LONG_PTR Combo_AddString(HWND hwndCombo, const wchar_t* str) {
		return ::SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)to_tchar(str));
	}

	inline LRESULT Combo_GetLBText(HWND hwndCombo, int nIndex, TCHAR* str) {
		return ::SendMessage(hwndCombo, CB_GETLBTEXT, nIndex, (LPARAM)str);
	}

	inline LRESULT Combo_GetText(HWND hwndCombo, TCHAR* str, int cchMax) {
		return ::GetWindowText(hwndCombo, str, cchMax);
	}

	inline int Combo_DeleteString(HWND hwndCtl, int index)				{ return (int)(DWORD)::SendMessage(hwndCtl, CB_DELETESTRING, (WPARAM)index, 0L); }
	inline int Combo_FindStringExact(HWND hwndCtl, int indexStart, const char* lpszFind)	{ return (int)(DWORD)::SendMessage(hwndCtl, CB_FINDSTRINGEXACT, (WPARAM)indexStart, (LPARAM)to_tchar(lpszFind)); }
	inline int Combo_FindStringExact(HWND hwndCtl, int indexStart, const wchar_t* lpszFind)	{ return (int)(DWORD)::SendMessage(hwndCtl, CB_FINDSTRINGEXACT, (WPARAM)indexStart, (LPARAM)to_tchar(lpszFind)); }
	
	inline int Combo_GetCount(HWND hwndCtl)								{ return (int)(DWORD)::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L); }
	inline int Combo_GetCurSel(HWND hwndCtl)							{ return (int)(DWORD)::SendMessage(hwndCtl, CB_GETCURSEL, 0L, 0L); }
	inline int Combo_SetCurSel(HWND hwndCtl, LONG_PTR index)				{ return (int)(DWORD)::SendMessage(hwndCtl, CB_SETCURSEL, (WPARAM)index, 0L); }
	inline LRESULT Combo_GetItemData(HWND hwndCtl, size_t index)			{ return ((LRESULT)(ULONG_PTR)::SendMessage(hwndCtl, CB_GETITEMDATA, (WPARAM)index, 0L)); }
	inline bool Combo_SetItemData(HWND hwndCtl, size_t index, LPARAM data)	{ return ::SendMessage(hwndCtl, CB_SETITEMDATA, (WPARAM)index, data) != CB_ERR; }
	inline int Combo_GetLBTextLen(HWND hwndCtl, int index)				{ return (int)(DWORD)::SendMessage(hwndCtl, CB_GETLBTEXTLEN, (WPARAM)index, 0L); }
	inline int Combo_InsertString(HWND hwndCtl, int index, const char* lpsz)	{ return (int)(DWORD)::SendMessage(hwndCtl, CB_INSERTSTRING, (WPARAM)index, (LPARAM)to_tchar(lpsz)); }
	inline int Combo_InsertString(HWND hwndCtl, int index, const wchar_t* lpsz)	{ return (int)(DWORD)::SendMessage(hwndCtl, CB_INSERTSTRING, (WPARAM)index, (LPARAM)to_tchar(lpsz)); }
	inline int Combo_LimitText(HWND hwndCtl, int cchLimit)				{ return (int)(DWORD)::SendMessage(hwndCtl, CB_LIMITTEXT, (WPARAM)cchLimit, 0L); }
	inline int Combo_ResetContent(HWND hwndCtl)							{ return (int)(DWORD)::SendMessage(hwndCtl, CB_RESETCONTENT, 0L, 0L); }
	inline int Combo_SetEditSel(HWND hwndCtl, int ichStart, int ichEnd)	{ return (int)(DWORD)::SendMessage(hwndCtl, CB_SETEDITSEL, 0L, MAKELPARAM(ichStart, ichEnd)); }
	inline int Combo_SetExtendedUI(HWND hwndCtl, UINT flags)			{ return (int)(DWORD)::SendMessage(hwndCtl, CB_SETEXTENDEDUI, (WPARAM)flags, 0L); }
	inline BOOL Combo_ShowDropdown(HWND hwndCtl, BOOL fShow)			{ return (BOOL)(DWORD)::SendMessage(hwndCtl, CB_SHOWDROPDOWN, (WPARAM)fShow, 0L); }
	inline int Combo_SetDroppedWidth(HWND hwndCtl, int width)			{ return (int)(DWORD)::SendMessage(hwndCtl, CB_SETDROPPEDWIDTH, (WPARAM)width, 0L); }
	inline BOOL Combo_GetDroppedState(HWND hwndCtl)						{ return (BOOL)(DWORD)::SendMessage(hwndCtl, CB_GETDROPPEDSTATE, 0L, 0L ); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      リストボックス                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	LRESULT List_GetText(HWND hwndList, int nIndex, char* str);
	LRESULT List_GetText(HWND hwndList, int nIndex, wchar_t* str);

	inline LRESULT List_AddString(HWND hwndList, const char* str) {
		return ::SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)to_tchar(str));
	}
	inline LRESULT List_AddString(HWND hwndList, const wchar_t* str) {
		return ::SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)to_tchar(str));
	}
	inline int List_AddItemData(HWND hwndCtl, int data)					{ return (int)(DWORD)::SendMessage(hwndCtl, LB_ADDSTRING, 0L, (LPARAM)data); }
	inline int List_AddItemData(HWND hwndCtl, void* data)				{ return (int)(DWORD)::SendMessage(hwndCtl, LB_ADDSTRING, 0L, (LPARAM)data); }
	inline int List_DeleteString(HWND hwndCtl, int index)				{ return (int)(DWORD)::SendMessage(hwndCtl, LB_DELETESTRING, (WPARAM)index, 0L); }
	inline int List_FindStringExact(HWND hwndCtl, int indexStart, char* lpszFind)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_FINDSTRINGEXACT, (WPARAM)indexStart, (LPARAM)to_tchar(lpszFind)); }
	inline int List_FindStringExact(HWND hwndCtl, int indexStart, wchar_t* lpszFind)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_FINDSTRINGEXACT, (WPARAM)indexStart, (LPARAM)to_tchar(lpszFind)); }
	inline int List_GetCaretIndex(HWND hwndCtl)							{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETCARETINDEX, 0L, 0L); }
	inline int List_GetCount(HWND hwndCtl)								{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETCOUNT, 0L, 0L); }
	inline int List_GetCurSel(HWND hwndCtl)								{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETCURSEL, 0L, 0L); }
	inline int List_GetTextLen(HWND hwndCtl, int nIndex)				{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETTEXTLEN, nIndex, 0L); }
	inline int List_SetCurSel(HWND hwndCtl, int index)					{ return (int)(DWORD)::SendMessage(hwndCtl, LB_SETCURSEL, (WPARAM)index, 0L); }
	inline LRESULT List_GetItemData(HWND hwndCtl, int index)			{ return (LRESULT)(ULONG_PTR)::SendMessage(hwndCtl, LB_GETITEMDATA, (WPARAM)index, 0L); }
	inline int List_SetItemData(HWND hwndCtl, int index, int data)		{ return (int)(DWORD)::SendMessage(hwndCtl, LB_SETITEMDATA, (WPARAM)index, (LPARAM)data); }
	inline int List_SetItemData(HWND hwndCtl, int index, void* data)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_SETITEMDATA, (WPARAM)index, (LPARAM)data); }
	inline int List_GetItemRect(HWND hwndCtl, int index, RECT* lprc)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETITEMRECT, (WPARAM)index, (LPARAM)lprc); }
	inline int List_GetTopIndex(HWND hwndCtl)							{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETTOPINDEX, 0L, 0L); }
	inline int List_InsertItemData(HWND hwndCtl, int index, int data)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_INSERTSTRING, (WPARAM)index, (LPARAM)data); }
	inline int List_InsertItemData(HWND hwndCtl, int index, void* data)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_INSERTSTRING, (WPARAM)index, (LPARAM)data); }
	inline int List_InsertString(HWND hwndCtl, int index, const char* lpsz)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_INSERTSTRING, (WPARAM)index, (LPARAM)to_tchar(lpsz)); }
	inline int List_InsertString(HWND hwndCtl, int index, const wchar_t* lpsz)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_INSERTSTRING, (WPARAM)index, (LPARAM)to_tchar(lpsz)); }
	inline BOOL List_ResetContent(HWND hwndCtl)							{ return (BOOL)(DWORD)::SendMessage(hwndCtl, LB_RESETCONTENT, 0L, 0L); }
	inline void List_SetHorizontalExtent(HWND hwndCtl, int cxExtent)	{ ::SendMessage(hwndCtl, LB_SETHORIZONTALEXTENT, (WPARAM)cxExtent, 0L); }
	inline int List_GetItemHeight(HWND hwndCtl, int index)				{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETITEMHEIGHT, (WPARAM)index, 0L); }
	inline int List_SetItemHeight(HWND hwndCtl, int index, int cy)		{ return (int)(DWORD)::SendMessage(hwndCtl, LB_SETITEMHEIGHT, (WPARAM)index, MAKELPARAM(cy, 0)); }
	inline int List_SetTopIndex(HWND hwndCtl, int indexTop)				{ return (int)(DWORD)::SendMessage(hwndCtl, LB_SETTOPINDEX, (WPARAM)indexTop, 0L); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      エディット コントロール                //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline void EditCtl_LimitText(HWND hwndCtl, size_t cchLimit)			{ ::SendMessage(hwndCtl, EM_LIMITTEXT, (WPARAM)cchLimit, 0L); }
	inline void EditCtl_SetSel(HWND hwndCtl, int ichStart, int ichEnd)	{ ::SendMessage(hwndCtl, EM_SETSEL, ichStart, ichEnd); }

	inline void EditCtl_ReplaceSel(HWND hwndCtl, const TCHAR* lpsz)		{ ::SendMessage(hwndCtl, EM_REPLACESEL, 0, (LPARAM)lpsz); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      ボタン コントロール                    //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline int BtnCtl_GetCheck(HWND hwndCtl)							{ return (int)(DWORD)::SendMessage(hwndCtl, BM_GETCHECK, 0L, 0L); }
	inline void BtnCtl_SetCheck(HWND hwndCtl, WPARAM check)				{ ::SendMessage(hwndCtl, BM_SETCHECK, check, 0L); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      スタティック コントロール              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline HICON StCtl_SetIcon(HWND hwndCtl, HICON hIcon)				{ return (HICON)(UINT_PTR)::SendMessage(hwndCtl, STM_SETICON, (WPARAM)hIcon, 0L); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       ダイアログ内                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline BOOL DlgItem_SetText(HWND hwndDlg, int nIDDlgItem, const char* str) {
		return SetDlgItemText(hwndDlg, nIDDlgItem, to_tchar(str));
	}
	inline BOOL DlgItem_SetText(HWND hwndDlg, int nIDDlgItem, const wchar_t* str) {
		return SetDlgItemText(hwndDlg, nIDDlgItem, to_tchar(str));
	}

	UINT DlgItem_GetText(HWND hwndDlg, int nIDDlgItem, char* str, size_t nMaxCount);
	UINT DlgItem_GetText(HWND hwndDlg, int nIDDlgItem, wchar_t* str, size_t nMaxCount);
	// GetDlgItemText

	inline bool DlgButton_IsChecked(HWND hDlg, int nIDButton)	{ return ::IsDlgButtonChecked(hDlg, nIDButton) == BST_CHECKED; }
}
using namespace ApiWrap;

