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

#include "StdAfx.h"
#include <windows.h>
#include "resource.h"
#include "CDlgSpellCheck.h"
#include "CPluginService.h"

///////////////////////////////////////////////////////////////////////////////
CDlgSpellCheck::CDlgSpellCheck()
{
	m_nRetCode   = IDCANCEL;
	m_bOneShot   = FALSE;
}

///////////////////////////////////////////////////////////////////////////////
CDlgSpellCheck::~CDlgSpellCheck()
{
}


///////////////////////////////////////////////////////////////////////////////
/*! モーダルダイアログの表示
*/
INT_PTR CDlgSpellCheck::DoModal(HINSTANCE hInstance, HWND hwndParent, BOOL bOneShot)
{
	//とりあえずキャンセル状態にしておく。
	m_nRetCode = IDCANCEL;
	m_strReplace = L"";
	m_bOneShot = bOneShot;
	return CPluginDialog::DoModal(hInstance, hwndParent, IDD_HUNSPELLCHECKER, (LPARAM)NULL);
}

///////////////////////////////////////////////////////////////////////////////
/*! 初期化処理
*/
BOOL CDlgSpellCheck::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	BOOL bResult = CPluginDialog::OnInitDialog(hwndDlg, wParam, lParam);
	HWND hButtonIgnoreAll         = ::GetDlgItem(GetHwnd(), IDC_BUTTON_IGNORE_ALL);
	HWND hButtonReplaceAll        = ::GetDlgItem(GetHwnd(), IDC_BUTTON_REPLACE_ALL);
	HWND hButtonAddDictReplace    = ::GetDlgItem(GetHwnd(), IDC_BUTTON_ADD_DICT_REPLACE);
	HWND hButtonAddDictReplaceAll = ::GetDlgItem(GetHwnd(), IDC_BUTTON_ADD_DICT_REPLACE_ALL);
	::EnableWindow(hButtonIgnoreAll,        !m_bOneShot);
	::EnableWindow(hButtonReplaceAll,       !m_bOneShot);
	::EnableWindow(hButtonAddDictReplace,    FALSE);
	::EnableWindow(hButtonAddDictReplaceAll, FALSE);
	::ShowWindow(hButtonAddDictReplace,      SW_HIDE);
	::ShowWindow(hButtonAddDictReplaceAll,   SW_HIDE);
	SetData();
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CDlgSpellCheck::OnBnClicked(int wID)
{
	switch(wID){
	case IDC_BUTTON_IGNORE:
	case IDC_BUTTON_IGNORE_ALL:
	case IDC_BUTTON_REPLACE:
	case IDC_BUTTON_REPLACE_ALL:
		m_nRetCode = wID;
		if(GetData()){
			CloseDialog(IDOK);
		}
		break;

	case IDCANCEL:
		m_nRetCode = wID;
		CloseDialog(IDCANCEL);
		break;

	case IDC_LIST_NEW_WORD:
		{
			OnLbnDblclk(wID);
			HWND hwndCombo = ::GetDlgItem(GetHwnd(), IDC_COMBO_NEW_WORD);
			Wnd_SetText(hwndCombo, m_strWord.c_str());
		}
		break;

	case IDC_EDIT_UNKNOWN_WORD:
		{
			HWND hwndCombo = ::GetDlgItem(GetHwnd(), IDC_COMBO_NEW_WORD);
			Wnd_SetText(hwndCombo, m_strWord.c_str());
		}
		break;

	case IDC_BUTTON_ADD_DICT_REPLACE:
	case IDC_BUTTON_ADD_DICT_REPLACE_ALL:
	default:
		break;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CDlgSpellCheck::OnLbnDblclk(int wID)
{
	switch(wID){
	case IDC_LIST_NEW_WORD:
		{
			HWND hwndList = ::GetDlgItem(GetHwnd(), IDC_LIST_NEW_WORD);
			HWND hwndCombo = ::GetDlgItem(GetHwnd(), IDC_COMBO_NEW_WORD);
			int nIndex = List_GetCurSel(hwndList);
			if(LB_ERR != nIndex){
				WCHAR word[SPELL_MAX_WORD_SIZE];
				List_GetText(hwndList, nIndex, word, SPELL_MAX_WORD_SIZE);
				Wnd_SetText(hwndCombo, word);
			}
			return TRUE;
		}
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CDlgSpellCheck::OnLbnSelChange(HWND hwndCtl, int wID)
{
	switch(wID){
	case IDC_LIST_NEW_WORD:
		return OnLbnDblclk(wID);
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CDlgSpellCheck::OnCbnSelChange(HWND hWnd, int wID)
{
	switch(wID){
	case IDC_COMBO_NEW_WORD:
		{
			HWND hwndList = ::GetDlgItem(GetHwnd(), IDC_LIST_NEW_WORD);
			HWND hwndCombo = ::GetDlgItem(GetHwnd(), IDC_COMBO_NEW_WORD);
			int nIndex = Combo_GetCurSel(hwndCombo);
			if(CB_ERR != nIndex){
				WCHAR word[SPELL_MAX_WORD_SIZE];
				Combo_GetLBText(hwndCombo, nIndex, word);
				nIndex = List_FindStringExact(hwndList, -1, word); 
				List_SetCurSel(hwndList, nIndex);
			}
			return TRUE;
		}
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
void CDlgSpellCheck::SetData(void)
{
	HWND hwndList = ::GetDlgItem(GetHwnd(), IDC_LIST_NEW_WORD);
	HWND hwndCombo = ::GetDlgItem(GetHwnd(), IDC_COMBO_NEW_WORD);
	HWND hwndUnknown = ::GetDlgItem(GetHwnd(), IDC_EDIT_UNKNOWN_WORD);

	EditCtl_LimitText(hwndUnknown, SPELL_MAX_WORD_SIZE - 1);
	Combo_LimitText(hwndCombo, SPELL_MAX_WORD_SIZE - 1);
	Combo_ResetContent(hwndCombo);

	Wnd_SetText(hwndUnknown, m_strWord.c_str());

	//代替案リストを表示する。
	int i = 0;
	int pos = 0;
	for(std::list<std::wstring>::iterator it = m_lstCorrection.begin(); it != m_lstCorrection.end(); ++it){

		std::wstring word = *it;
		int length = word.length();
		if(length >= SPELL_MAX_WORD_SIZE){
			word = word.substr(0, SPELL_MAX_WORD_SIZE - 1);
		}

		List_AddString(hwndList, word.c_str());
		Combo_AddString(hwndCombo, word.c_str());

		if(i == 0){
			Wnd_SetText(hwndCombo, word.c_str());
			List_SetCurSel(hwndList, 0);
		}
		if(pos == 0){
			if(wcsicmp(word.c_str(), m_strWord.c_str()) == 0){
				Wnd_SetText(hwndCombo, word.c_str());
				pos = i;
				List_SetCurSel(hwndList, pos);
			}
		}

		i++;
	}

	pos = List_GetCurSel(hwndList);
	if(pos >= 0){
		List_SetTopIndex(hwndList, pos);
	}
}

///////////////////////////////////////////////////////////////////////////////
int CDlgSpellCheck::GetData( void )
{
	switch(m_nRetCode){
	case IDC_BUTTON_REPLACE:
	case IDC_BUTTON_REPLACE_ALL:
		{
			WCHAR word[SPELL_MAX_WORD_SIZE];
			memset(word, 0, sizeof(word));
			HWND hwndCombo = ::GetDlgItem(GetHwnd(), IDC_COMBO_NEW_WORD);
			Combo_GetText(hwndCombo, word, SPELL_MAX_WORD_SIZE);
			m_strReplace = word;
		}
		break;

	case IDC_BUTTON_ADD_DICT_REPLACE:
	case IDC_BUTTON_ADD_DICT_REPLACE_ALL:
		break;
	}

	return TRUE;
}
