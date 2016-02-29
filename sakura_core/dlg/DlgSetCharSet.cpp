/*!	@file
	@brief 文字コードセット設定ダイアログボックス

	@author Uchi
	@date 2010/6/14  新規作成
*/
/*
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "dlg/DlgSetCharSet.h"
#include "func/Funccode.h"
#include "util/shell.h"
#include "env/DllSharedData.h"
#include "charset/CodePage.h"
#include "sakura_rc.h"
#include "sakura.hh"

// 文字コードセット設定 DlgSetCharSet
const DWORD p_helpids[] = {
	IDOK,							HIDOK_GREP,							// 検索
	IDCANCEL,						HIDCANCEL_GREP,						// キャンセル
	IDC_BUTTON_HELP,				HIDC_GREP_BUTTON_HELP,				// ヘルプ
	IDC_COMBO_CHARSET,				HIDC_OPENDLG_COMBO_CODE,			// 文字コードセット
	IDC_CHECK_BOM,					HIDC_OPENDLG_CHECK_BOM,				// 条件
	IDC_CHECK_CP,					HIDC_OPENDLG_CHECK_CP,				// CP
	0, 0
};


DlgSetCharSet::DlgSetCharSet()
{
	m_pnCharSet = NULL;			// 文字コードセット
	m_pbBom = NULL;				// 文字コードセット
	m_bCP = false;
}


// モーダルダイアログの表示
int DlgSetCharSet::DoModal(
	HINSTANCE hInstance,
	HWND hwndParent,
	EncodingType* pnCharSet,
	bool* pbBom
	)
{
	m_pnCharSet = pnCharSet;	// 文字コードセット
	m_pbBom = pbBom;			// BOM

	return (int)Dialog::DoModal(hInstance, hwndParent, IDD_SETCHARSET, (LPARAM)NULL);
}


BOOL DlgSetCharSet::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	_SetHwnd(hwndDlg);
	
	m_hwndCharSet = GetItemHwnd(IDC_COMBO_CHARSET);	// 文字コードセットコンボボックス
	m_hwndCheckBOM = GetItemHwnd(IDC_CHECK_BOM);		// BOMチェックボックス

	// コンボボックスのユーザー インターフェイスを拡張インターフェースにする
	Combo_SetExtendedUI(m_hwndCharSet, TRUE);

	// 文字コードセット選択コンボボックス初期化
	CodeTypesForCombobox codeTypes;
	Combo_ResetContent(m_hwndCharSet);
	for (int i=1; i<codeTypes.GetCount(); ++i) {
		int idx = Combo_AddString(m_hwndCharSet, codeTypes.GetName(i));
		Combo_SetItemData(m_hwndCharSet, idx, codeTypes.GetCode(i));
	}

	// 基底クラスメンバ
	return Dialog::OnInitDialog(hwndDlg, wParam, lParam);
}


BOOL DlgSetCharSet::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_CHECK_CP:
		if (!m_bCP) {
			m_bCP = true;
			EnableItem(IDC_CHECK_CP, false);
			CodePage::AddComboCodePages( GetHwnd(), m_hwndCharSet, -1 );
		}
		return TRUE;
	case IDC_BUTTON_HELP:
		//「文字コードセット設定」のヘルプ
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_CHG_CHARSET));
		return TRUE;
	case IDOK:
		// ダイアログデータの取得
		if (GetData()) {
			CloseDialog(TRUE);
		}
		return TRUE;
	case IDCANCEL:
		CloseDialog(FALSE);
		return TRUE;
	}

	// 基底クラスメンバ
	return Dialog::OnBnClicked(wID);
}


// BOM の設定
void DlgSetCharSet::SetBOM(void)
{
	WPARAM fCheck;
	int nIdx = Combo_GetCurSel(m_hwndCharSet);
	LRESULT lRes = Combo_GetItemData(m_hwndCharSet, nIdx);
	CodeTypeName codeTypeName(lRes);
	if (codeTypeName.UseBom()) {
		::EnableWindow(m_hwndCheckBOM, TRUE);
		if (lRes == *m_pnCharSet) {
			fCheck = *m_pbBom ? BST_CHECKED : BST_UNCHECKED;
		}else {
			fCheck = codeTypeName.IsBomDefOn() ? BST_CHECKED : BST_UNCHECKED;
		}
	}else {
		::EnableWindow(m_hwndCheckBOM, FALSE);
		fCheck = BST_UNCHECKED;
	}
	BtnCtl_SetCheck(m_hwndCheckBOM, fCheck);
}


// 文字コード選択時の処理
BOOL DlgSetCharSet::OnCbnSelChange(HWND hwndCtl, int wID)
{
	int 		nIdx;
	LRESULT		lRes;
	WPARAM		fCheck;

	switch (wID) {
	// 文字コードの変更をBOMチェックボックスに反映
	case IDC_COMBO_CHARSET:
		SetBOM();
		nIdx = Combo_GetCurSel(hwndCtl);
		lRes = Combo_GetItemData(hwndCtl, nIdx);
		CodeTypeName	codeTypeName(lRes);
		if (codeTypeName.UseBom()) {
			::EnableWindow(m_hwndCheckBOM, TRUE);
			if (lRes == *m_pnCharSet) {
				fCheck = *m_pbBom ? BST_CHECKED : BST_UNCHECKED;
			}else {
				fCheck = codeTypeName.IsBomDefOn() ? BST_CHECKED : BST_UNCHECKED;
			}
		}else {
			::EnableWindow(m_hwndCheckBOM, FALSE);
			fCheck = BST_UNCHECKED;
		}
		BtnCtl_SetCheck(m_hwndCheckBOM, fCheck);
		break;
	}
	return TRUE;
}


LPVOID DlgSetCharSet::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}


// ダイアログデータの設定
void DlgSetCharSet::SetData(void)
{
	// 文字コードセット
	CodeTypesForCombobox codeTypes;

	int nIdxOld = Combo_GetCurSel(m_hwndCharSet);
	int nCurIdx = -1;
	for (int nIdx=0; nIdx<Combo_GetCount(m_hwndCharSet); ++nIdx) {
		EncodingType nCharSet = (EncodingType)Combo_GetItemData( m_hwndCharSet, nIdx );
		if (nCharSet == *m_pnCharSet) {
			nCurIdx = nIdx;
		}
	}
	if (nCurIdx == -1) {
		m_bCP = true;
		CheckButton(IDC_CHECK_CP, true);
		EnableItem(IDC_CHECK_CP, false);
		nCurIdx = CodePage::AddComboCodePages(GetHwnd(), m_hwndCharSet, *m_pnCharSet);
		if (nCurIdx == -1) {
			nCurIdx = nIdxOld;
		}
	}
	Combo_SetCurSel(m_hwndCharSet, nCurIdx);

	// BOMを設定
	SetBOM();
}


// ダイアログデータの取得
// TRUE==正常  FALSE==入力エラー
int DlgSetCharSet::GetData(void)
{
	// 文字コードセット
	int nIdx = Combo_GetCurSel(m_hwndCharSet);
	*m_pnCharSet = (EncodingType)Combo_GetItemData(m_hwndCharSet, nIdx);

	// BOM
	*m_pbBom = (BtnCtl_GetCheck(m_hwndCheckBOM) == BST_CHECKED);

	return TRUE;
}

