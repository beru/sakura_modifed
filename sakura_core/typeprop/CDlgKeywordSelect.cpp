/*! @file
	@brief 強調キーワード選択ダイアログ

	@author MIK
	@date 2005/01/13 作成
*/
/*
	Copyright (C) 2005, MIK
	Copyright (C) 2006, ryoji

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
#include "CDlgKeywordSelect.h"
#include "env/DLLSHAREDATA.h"
#include "CKeyWordSetMgr.h"
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids[] = {	// 2006.10.10 ryoji
	IDOK,				HIDOK_KEYWORD_SELECT,
	IDCANCEL,			HIDCANCEL_KEYWORD_SELECT,
	IDC_COMBO1,			HIDC_COMBO_KEYWORD_SELECT,
	IDC_COMBO2,			HIDC_COMBO_KEYWORD_SELECT,
	IDC_COMBO3,			HIDC_COMBO_KEYWORD_SELECT,
	IDC_COMBO4,			HIDC_COMBO_KEYWORD_SELECT,
	IDC_COMBO5,			HIDC_COMBO_KEYWORD_SELECT,
	IDC_COMBO6,			HIDC_COMBO_KEYWORD_SELECT,
	IDC_COMBO7,			HIDC_COMBO_KEYWORD_SELECT,
	IDC_COMBO8,			HIDC_COMBO_KEYWORD_SELECT,
	IDC_COMBO9,			HIDC_COMBO_KEYWORD_SELECT,
	IDC_COMBO10,		HIDC_COMBO_KEYWORD_SELECT,
	0, 0
};

static const int keyword_select_target_combo[KEYWORD_SELECT_NUM] = {
	IDC_COMBO1,
	IDC_COMBO2,
	IDC_COMBO3,
	IDC_COMBO4,
	IDC_COMBO5,
	IDC_COMBO6,
	IDC_COMBO7,
	IDC_COMBO8,
	IDC_COMBO9,
	IDC_COMBO10
};


DlgKeywordSelect::DlgKeywordSelect()
{
	m_pKeyWordSetMgr = &(m_pShareData->m_common.m_specialKeyword.m_keyWordSetMgr);

	return;
}

DlgKeywordSelect::~DlgKeywordSelect()
{
	return;
}


// モーダルダイアログの表示
int DlgKeywordSelect::DoModal(HINSTANCE hInstance, HWND hwndParent, int* pnSet)
{
	for (int i=0; i<KEYWORD_SELECT_NUM; ++i) {
		m_nSet[i] = pnSet[i];
	}

	(void)Dialog::DoModal(hInstance, hwndParent, IDD_DIALOG_KEYWORD_SELECT, (LPARAM)NULL);

	for (int i=0; i<KEYWORD_SELECT_NUM; ++i) {
		pnSet[i] = m_nSet[i];
	}

	return TRUE;
}

// 初期化処理
BOOL DlgKeywordSelect::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	_SetHwnd(hwndDlg);

	return Dialog::OnInitDialog(hwndDlg, wParam, lParam);
}


BOOL DlgKeywordSelect::OnBnClicked(int wID)
{
	switch (wID) {
	case IDOK:
		GetData();
		break;
	case IDCANCEL:
		break;
	}
	return Dialog::OnBnClicked(wID);
}

// ダイアログデータの設定
void DlgKeywordSelect::SetData(void)
{
	for (int index=0; index<KEYWORD_SELECT_NUM; ++index) {
		HWND hwndCombo = GetItemHwnd(keyword_select_target_combo[index]);

		// コンボボックスを空にする
		Combo_ResetContent(hwndCombo);
		
		// 一行目は空白
		Combo_AddString(hwndCombo, L" ");

		if (m_pKeyWordSetMgr->m_nKeyWordSetNum > 0) {
			for (int i=0; i<m_pKeyWordSetMgr->m_nKeyWordSetNum; ++i) {
				Combo_AddString(hwndCombo, m_pKeyWordSetMgr->GetTypeName(i));
			}

			if (m_nSet[index] == -1) {
				// セット名コンボボックスのデフォルト選択
				Combo_SetCurSel(hwndCombo, 0);
			}else {
				// セット名コンボボックスのデフォルト選択
				Combo_SetCurSel(hwndCombo, m_nSet[index] + 1);
			}
		}
	}
}


// ダイアログデータの設定
int DlgKeywordSelect::GetData(void)
{
	for (int index=0; index<KEYWORD_SELECT_NUM; ++index) {
		HWND hwndCombo = GetItemHwnd(keyword_select_target_combo[index]);

		int n = Combo_GetCurSel(hwndCombo);
		if (n == CB_ERR || n == 0) {
			m_nSet[index] = -1;
		}else {
			m_nSet[index] = n - 1;
		}
	}

	return TRUE;
}

LPVOID DlgKeywordSelect::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

