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
	pnCharSet = NULL;			// 文字コードセット
	pbBom = NULL;				// 文字コードセット
	bCP = false;
}


// モーダルダイアログの表示
INT_PTR DlgSetCharSet::DoModal(
	HINSTANCE hInstance,
	HWND hwndParent,
	EncodingType* pnCharSet,
	bool* pbBom
	)
{
	pnCharSet = pnCharSet;	// 文字コードセット
	pbBom = pbBom;			// BOM

	return Dialog::DoModal(hInstance, hwndParent, IDD_SETCHARSET, (LPARAM)NULL);
}


BOOL DlgSetCharSet::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	_SetHwnd(hwndDlg);
	
	hwndCharSet = GetItemHwnd(IDC_COMBO_CHARSET);	// 文字コードセットコンボボックス
	hwndCheckBOM = GetItemHwnd(IDC_CHECK_BOM);		// BOMチェックボックス

	// コンボボックスのユーザー インターフェイスを拡張インターフェースにする
	Combo_SetExtendedUI(hwndCharSet, TRUE);

	// 文字コードセット選択コンボボックス初期化
	CodeTypesForCombobox codeTypes;
	Combo_ResetContent(hwndCharSet);
	for (size_t i=1; i<codeTypes.GetCount(); ++i) {
		int idx = Combo_AddString(hwndCharSet, codeTypes.GetName(i));
		Combo_SetItemData(hwndCharSet, idx, codeTypes.GetCode(i));
	}

	// 基底クラスメンバ
	return Dialog::OnInitDialog(hwndDlg, wParam, lParam);
}


BOOL DlgSetCharSet::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_CHECK_CP:
		if (!bCP) {
			bCP = true;
			EnableItem(IDC_CHECK_CP, false);
			CodePage::AddComboCodePages( GetHwnd(), hwndCharSet, -1 );
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
	int nIdx = Combo_GetCurSel(hwndCharSet);
	LRESULT lRes = Combo_GetItemData(hwndCharSet, nIdx);
	CodeTypeName codeTypeName(lRes);
	if (codeTypeName.UseBom()) {
		::EnableWindow(hwndCheckBOM, TRUE);
		if (lRes == *pnCharSet) {
			fCheck = *pbBom ? BST_CHECKED : BST_UNCHECKED;
		}else {
			fCheck = codeTypeName.IsBomDefOn() ? BST_CHECKED : BST_UNCHECKED;
		}
	}else {
		::EnableWindow(hwndCheckBOM, FALSE);
		fCheck = BST_UNCHECKED;
	}
	BtnCtl_SetCheck(hwndCheckBOM, fCheck);
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
			::EnableWindow(hwndCheckBOM, TRUE);
			if (lRes == *pnCharSet) {
				fCheck = *pbBom ? BST_CHECKED : BST_UNCHECKED;
			}else {
				fCheck = codeTypeName.IsBomDefOn() ? BST_CHECKED : BST_UNCHECKED;
			}
		}else {
			::EnableWindow(hwndCheckBOM, FALSE);
			fCheck = BST_UNCHECKED;
		}
		BtnCtl_SetCheck(hwndCheckBOM, fCheck);
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

	int nIdxOld = Combo_GetCurSel(hwndCharSet);
	int nCurIdx = -1;
	for (int nIdx=0; nIdx<Combo_GetCount(hwndCharSet); ++nIdx) {
		EncodingType nCharSet = (EncodingType)Combo_GetItemData( hwndCharSet, nIdx );
		if (nCharSet == *pnCharSet) {
			nCurIdx = nIdx;
		}
	}
	if (nCurIdx == -1) {
		bCP = true;
		CheckButton(IDC_CHECK_CP, true);
		EnableItem(IDC_CHECK_CP, false);
		nCurIdx = CodePage::AddComboCodePages(GetHwnd(), hwndCharSet, *pnCharSet);
		if (nCurIdx == -1) {
			nCurIdx = nIdxOld;
		}
	}
	Combo_SetCurSel(hwndCharSet, nCurIdx);

	// BOMを設定
	SetBOM();
}


// ダイアログデータの取得
// TRUE==正常  FALSE==入力エラー
int DlgSetCharSet::GetData(void)
{
	// 文字コードセット
	int nIdx = Combo_GetCurSel(hwndCharSet);
	*pnCharSet = (EncodingType)Combo_GetItemData(hwndCharSet, nIdx);

	// BOM
	*pbBom = (BtnCtl_GetCheck(hwndCheckBOM) == BST_CHECKED);

	return TRUE;
}

