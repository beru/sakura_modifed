#include "StdAfx.h"
#include "StdControl.h"
#include "util/tchar_receive.h"

namespace ApiWrap {

	LRESULT List_GetText(HWND hwndList, int nIndex, ACHAR* str)
	{
		LRESULT nCount = SendMessage(hwndList, LB_GETTEXTLEN, (WPARAM)nIndex, (LPARAM)0);
		if (nCount == LB_ERR)
			return LB_ERR;
		return SendMessage(hwndList, LB_GETTEXT, (WPARAM)nIndex, (LPARAM)(TCHAR*)TcharReceiver<ACHAR>(str, nCount + 1));	// +1: NULL •¶Žš•ª
	}

	LRESULT List_GetText(HWND hwndList, int nIndex, wchar_t* str)
	{
		LRESULT nCount = SendMessage(hwndList, LB_GETTEXTLEN, (WPARAM)nIndex, (LPARAM)0);
		if (nCount == LB_ERR)
			return LB_ERR;
		return SendMessage(hwndList, LB_GETTEXT, (WPARAM)nIndex, (LPARAM)(TCHAR*)TcharReceiver<wchar_t>(str, nCount + 1));	// +1: NULL •¶Žš•ª
	}

	UINT DlgItem_GetText(HWND hwndDlg, int nIDDlgItem, ACHAR* str, size_t nMaxCount)
	{
		ASSERT_GE(INT32_MAX, nMaxCount);
		return GetDlgItemText(hwndDlg, nIDDlgItem, TcharReceiver<ACHAR>(str, nMaxCount), (int)nMaxCount);
	}

	UINT DlgItem_GetText(HWND hwndDlg, int nIDDlgItem, wchar_t* str, size_t nMaxCount)
	{
		ASSERT_GE(INT32_MAX, nMaxCount);
		return GetDlgItemText(hwndDlg, nIDDlgItem, str, (int)nMaxCount);
	}

}

