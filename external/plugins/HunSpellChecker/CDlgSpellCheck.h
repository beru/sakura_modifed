/*
	Copyright (C) 2013, Plugins developers

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

#ifndef _PLUGIN_CDLGSPELLCHECK_H__
#define _PLUGIN_CDLGSPELLCHECK_H__

#include "CPluginDialog.h"
#include <string>
#include <list>

#define SPELL_MAX_WORD_SIZE 256

///////////////////////////////////////////////////////////////////////////////
class CDlgSpellCheck : public CPluginDialog
{
public:
	CDlgSpellCheck();
	virtual ~CDlgSpellCheck();

	INT_PTR DoModal(HINSTANCE hInstance, HWND hwndParent, BOOL bOneShot);
	virtual BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnBnClicked(int wID);
	virtual BOOL OnLbnDblclk(int wID);
	virtual BOOL OnLbnSelChange(HWND hwndCtl, int wID);
	virtual BOOL OnCbnSelChange(HWND hWnd, int wID);
	virtual int GetData(void);
	virtual void SetData(void);

	enum {
		RUNMODE_FILETOP = 0,	//!< ファイル先頭から
		RUNMODE_CURRENT,		//!< カーソル位置から
		RUNMODE_ONESHOT			//!< カーソル位置の単語だけ
	};

public:
	BOOL						m_bOneShot;
	std::wstring				m_strWord;
	std::wstring				m_strReplace;
	std::list<std::wstring>		m_lstCorrection;

public:
	//Window
	BOOL Wnd_SetText(HWND hwnd, LPCWSTR str){
		return SetWindowTextW(hwnd, str);
	}

	//ComboBox
	int Combo_ResetContent(HWND hwndCtl){
		return (int)(DWORD)::SendMessage(hwndCtl, CB_RESETCONTENT, 0L, 0L);
	}
	int Combo_LimitText(HWND hwndCtl, int cchLimit){
		return (int)(DWORD)::SendMessage(hwndCtl, CB_LIMITTEXT, (WPARAM)cchLimit, 0L);
	}
	int Combo_GetCount(HWND hwndCtl){
		return (int)(DWORD)::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L);
	}
	int Combo_GetCurSel(HWND hwndCtl){
		return (int)(DWORD)::SendMessage(hwndCtl, CB_GETCURSEL, 0L, 0L);
	}
	int Combo_SetCurSel(HWND hwndCtl, int nIndex){
		return (int)(DWORD)::SendMessage(hwndCtl, CB_SETCURSEL, (WPARAM)nIndex, 0L);
	}
	LRESULT Combo_AddString(HWND hwndCombo, LPCWSTR str){
		return ::SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)str);
	}
	LRESULT Combo_GetText(HWND hwndCombo, LPWSTR str, int cchMax){
		return ::GetWindowText(hwndCombo, str, cchMax);
	}
	LRESULT Combo_GetLBText(HWND hwndCombo, int nIndex, WCHAR* str){
		return ::SendMessage(hwndCombo, CB_GETLBTEXT, nIndex, (LPARAM)str);
	}

	//ListBox
	LRESULT List_GetText(HWND hwndList, int nIndex, WCHAR* str, int cchMaxLength){
		LRESULT nCount = ::SendMessage(hwndList, LB_GETTEXTLEN, (WPARAM)nIndex, (LPARAM)0);
		if(nCount == LB_ERR) return LB_ERR;
		WCHAR* buffer = new WCHAR[nCount+1];
		memset(buffer, 0, (nCount+1) * sizeof(WCHAR));
		LRESULT nResult = ::SendMessage(hwndList, LB_GETTEXT, (WPARAM)nIndex, (LPARAM)buffer);
#ifdef __MINGW32__
		wcsncpy(str, buffer, cchMaxLength);
#else
		wcsncpy_s(str, cchMaxLength, buffer, _TRUNCATE);
#endif
		delete[] buffer;
		return nResult;
	}
	LRESULT List_AddString(HWND hwndList, LPCWSTR str){
		return ::SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)str);
	}
	int List_InsertString(HWND hwndCtl, int nIndex, LPCWSTR str){
		return (int)(DWORD)::SendMessage(hwndCtl, LB_INSERTSTRING, (WPARAM)nIndex, (LPARAM)str);
	}
	int List_GetCount(HWND hwndCtl){
		return (int)(DWORD)::SendMessage(hwndCtl, LB_GETCOUNT, 0L, 0L);
	}
	int List_SetCurSel(HWND hwndCtl, int nIndex){
		return (int)(DWORD)::SendMessage(hwndCtl, LB_SETCURSEL, (WPARAM)nIndex, 0L);
	}
	int List_GetCurSel(HWND hwndCtl){
		return (int)(DWORD)::SendMessage(hwndCtl, LB_GETCURSEL, 0L, 0L);
	}
	int List_SetTopIndex(HWND hwndCtl, int indexTop){
		return (int)(DWORD)::SendMessage(hwndCtl, LB_SETTOPINDEX, (WPARAM)indexTop, 0L);
	}
	int List_FindStringExact(HWND hwndCtl, int indexStart, LPCWSTR lpszFind){
		return (int)(DWORD)::SendMessage(hwndCtl, LB_FINDSTRINGEXACT, (WPARAM)indexStart, (LPARAM)lpszFind);
	}

	void EditCtl_LimitText(HWND hwndCtl, int cchLimit){
		::SendMessage(hwndCtl, EM_LIMITTEXT, (WPARAM)(cchLimit), 0L);
	}
};

#endif	// _PLUGIN_CDLGSPELLCHECK_H__
