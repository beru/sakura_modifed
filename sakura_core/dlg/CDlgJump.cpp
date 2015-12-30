/*!	@file
	@brief 指定行へのジャンプダイアログボックス

	@author Norio Nakatani
	@date	1998/05/31 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, jepro, Stonee
	Copyright (C) 2002, aroka, MIK, YAZAKI
	Copyright (C) 2004, genta
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "dlg/CDlgJump.h"
#include "doc/CEditDoc.h"
#include "func/Funccode.h"		// Stonee, 2001/03/12
#include "outline/CFuncInfo.h"
#include "outline/CFuncInfoArr.h"// 2002/2/10 aroka ヘッダ整理
#include "util/shell.h"
#include "window/CEditWnd.h"
#include "sakura_rc.h"
#include "sakura.hh"

// ジャンプ CDlgJump.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12800
	IDC_BUTTON_JUMP,				HIDC_JUMP_BUTTON_JUMP,			// ジャンプ
	IDCANCEL,						HIDCANCEL_JUMP,					// キャンセル
	IDC_BUTTON_HELP,				HIDC_JUMP_BUTTON_HELP,			// ヘルプ
	IDC_CHECK_PLSQL,				HIDC_JUMP_CHECK_PLSQL,			// PL/SQL
	IDC_COMBO_PLSQLBLOCKS,			HIDC_JUMP_COMBO_PLSQLBLOCKS,	// 
	IDC_EDIT_LINENUM,				HIDC_JUMP_EDIT_LINENUM,			// 行番号
	IDC_EDIT_PLSQL_E1,				HIDC_JUMP_EDIT_PLSQL_E1,		// 
	IDC_RADIO_LINENUM_LAYOUT,		HIDC_JUMP_RADIO_LINENUM_LAYOUT,	// 折り返し単位
	IDC_RADIO_LINENUM_CRLF,			HIDC_JUMP_RADIO_LINENUM_CRLF,	// 改行単位
	IDC_SPIN_LINENUM,				HIDC_JUMP_EDIT_LINENUM,			// 12870,	//
	IDC_SPIN_PLSQL_E1,				HIDC_JUMP_EDIT_PLSQL_E1,		// 12871,	//
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgJump::CDlgJump()
{
	m_nLineNum = 0;			// 行番号
	m_bPLSQL = FALSE;		// PL/SQLソースの有効行か
	m_nPLSQL_E1 = 1;
	m_nPLSQL_E2 = 1;

	return;
}

// モーダルダイアログの表示
int CDlgJump::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam
)
{
	return CDialog::DoModal(hInstance, hwndParent, IDD_JUMP, lParam);
}


// From Here Oct. 6, 2000 JEPRO added 行番号入力ボックスにスピンコントロールを付けるため
// CDlgPrintSetting.cppのOnNotifyとOnSpin及びCpropComFile.cppのDispatchEvent_p2内のcase WM_NOTIFYを参考にした
BOOL CDlgJump::OnNotify(WPARAM wParam, LPARAM lParam)
{
	int nData;
	int idCtrl = (int)wParam;
	NM_UPDOWN* pMNUD  = (NM_UPDOWN*)lParam;
	// スピンコントロールの処理
	switch (idCtrl) {
	case IDC_SPIN_LINENUM:
	// ジャンプしたい行番号の指定
		nData = GetItemInt(IDC_EDIT_LINENUM, NULL, FALSE);
		if (pMNUD->iDelta < 0) {
			++nData;
		}else if (pMNUD->iDelta > 0) {
			--nData;
		}
		if (nData < 1) {
			nData = 1;
		}
		SetItemInt(IDC_EDIT_LINENUM, nData, FALSE);
		break;
	case IDC_SPIN_PLSQL_E1:
		nData = GetItemInt(IDC_EDIT_PLSQL_E1, NULL, FALSE);
		if (pMNUD->iDelta < 0) {
			++nData;
		}else if (pMNUD->iDelta > 0) {
			--nData;
		}
		if (nData < 1) {
			nData = 1;
		}
		SetItemInt(IDC_EDIT_PLSQL_E1, nData, FALSE);
		break;
	default:
		break;
	}
	return TRUE;
}
// To Here Oct. 6, 2000


BOOL CDlgJump::OnCbnSelChange(HWND hwndCtl, int wID)
{
	int	nIndex;
	int	nWorkLine;
	switch (wID) {
	case IDC_COMBO_PLSQLBLOCKS:
		nIndex = Combo_GetCurSel(GetItemHwnd(IDC_COMBO_PLSQLBLOCKS));
		nWorkLine = (int)Combo_GetItemData(GetItemHwnd(IDC_COMBO_PLSQLBLOCKS), nIndex);
		SetItemInt(IDC_EDIT_PLSQL_E1, nWorkLine, FALSE);
		return TRUE;
	}
	return FALSE;
}

BOOL CDlgJump::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_HELP:
		//「指定行へジャンプ」のヘルプ
		// Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_JUMP_DIALOG));	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
	case IDC_CHECK_PLSQL:		// PL/SQLソースの有効行か
		if (IsButtonChecked(IDC_CHECK_PLSQL)) {
			EnableItem(IDC_LABEL_PLSQL1, true);	// Sept. 12, 2000 JEPRO
			EnableItem(IDC_LABEL_PLSQL2, true);	// Sept. 12, 2000 JEPRO
			EnableItem(IDC_LABEL_PLSQL3, true);	// Sept. 12, 2000 JEPRO
			EnableItem(IDC_EDIT_PLSQL_E1, true);
			EnableItem(IDC_SPIN_PLSQL_E1, true);	// Oct. 6, 2000 JEPRO
			EnableItem(IDC_COMBO_PLSQLBLOCKS, true);
			m_pShareData->m_bLineNumIsCRLF_ForJump = true;
			EnableItem(IDC_RADIO_LINENUM_LAYOUT, false);
			EnableItem(IDC_RADIO_LINENUM_CRLF, false);
		}else {
			EnableItem(IDC_LABEL_PLSQL1, false);	// Sept. 12, 2000 JEPRO
			EnableItem(IDC_LABEL_PLSQL2, false);	// Sept. 12, 2000 JEPRO
			EnableItem(IDC_LABEL_PLSQL3, false);	// Sept. 12, 2000 JEPRO
			EnableItem(IDC_EDIT_PLSQL_E1, false);
			EnableItem(IDC_SPIN_PLSQL_E1, false);	// Oct. 6, 2000 JEPRO
			EnableItem(IDC_COMBO_PLSQLBLOCKS, false);
			EnableItem(IDC_RADIO_LINENUM_LAYOUT, true);
			EnableItem(IDC_RADIO_LINENUM_CRLF, true);
		}
		// 行番号の表示 false=折り返し単位／true=改行単位
		if (m_pShareData->m_bLineNumIsCRLF_ForJump) {
			CheckButton(IDC_RADIO_LINENUM_LAYOUT, false);
			CheckButton(IDC_RADIO_LINENUM_CRLF, true);
		}else {
			CheckButton(IDC_RADIO_LINENUM_LAYOUT, true);
			CheckButton(IDC_RADIO_LINENUM_CRLF, false);
		}
		return TRUE;
	case IDC_BUTTON_JUMP:			// 指定行へジャンプ	// Feb. 20, 2001 JEPRO ボタン名を[IDOK]→[IDC_BUTTON_JUMP]に変更
		// ダイアログデータの取得
		// From Here Feb. 20, 2001 JEPRO 次行をコメントアウト (CEditView_Command.cpp の Command_JUMP も関係しているので参照のこと)
//		::EndDialog(GetHwnd(), GetData());
//		次行から追加
		if (0 < GetData()) {
			CloseDialog(1);
		}else {
			OkMessage(GetHwnd(), LS(STR_DLGJUMP1));
		}
// To Here Feb. 20, 2001
		{	//@@@ 2002.2.2 YAZAKI 指定行へジャンプを、ダイアログを表示するコマンドと、実際にジャンプするコマンドに分離。
			CEditDoc* pCEditDoc = (CEditDoc*)m_lParam;
			pCEditDoc->m_pcEditWnd->GetActiveView().GetCommander().HandleCommand(F_JUMP, true, 0, 0, 0, 0);	// ジャンプコマンド発行
		}
		return TRUE;
	case IDCANCEL:
		::EndDialog(GetHwnd(), FALSE);
		return TRUE;
	}
	// 基底クラスメンバ
	return CDialog::OnBnClicked(wID);
}


// ダイアログデータの設定
void CDlgJump::SetData(void)
{
	CEditDoc* pCEditDoc = (CEditDoc*)m_lParam;
	CFuncInfoArr cFuncInfoArr;
	wchar_t szText[1024];
	int nIndexCurSel = 0;	// Sep. 11, 2004 genta 初期化

//	GetHwnd() = hwndDlg;
// From Here Oct. 7, 2000 JEPRO 前回入力した行番号を保持するように下行を変更
//	::DlgItem_SetText(GetHwnd(), IDC_EDIT_LINENUM, "");	// 行番号
	if (m_nLineNum == 0) {
		SetItemText(IDC_EDIT_LINENUM, _T(""));	// 行番号
	}else {
		SetItemInt(IDC_EDIT_LINENUM, m_nLineNum, FALSE);	// 前回の行番号
	}
// To Here Oct. 7, 2000
	SetItemInt(IDC_EDIT_PLSQL_E1, m_nPLSQL_E1, FALSE);
	// PL/SQL関数リスト作成
	HWND hwndCtrl = GetItemHwnd(IDC_COMBO_PLSQLBLOCKS);
	// タイプ別に設定されたアウトライン解析方法
	if (OUTLINE_PLSQL == pCEditDoc->m_cDocType.GetDocumentAttribute().m_eDefaultOutline) {
		pCEditDoc->m_cDocOutline.MakeFuncList_PLSQL(&cFuncInfoArr);
	}
	//$$ 条件により、レイアウト・ロジックの単位が混在するため、ミスの原因になりやすい
	int nWorkLine = -1;
	int nIndex = 0;
	int nPLSQLBlockNum = 0;
	for (int i=0; i<cFuncInfoArr.GetNum(); ++i) {
		CFuncInfo* pFI = cFuncInfoArr.GetAt(i);
		if (pFI->m_nInfo == 31 || pFI->m_nInfo == 41) {
		}
		if (pFI->m_nInfo == 31) {
			if (m_pShareData->m_bLineNumIsCRLF_ForJump) {	// 行番号の表示 false=折り返し単位／true=改行単位
				auto_sprintf(szText, LSW(STR_DLGJUMP_PSLQL),
					pFI->m_nFuncLineCRLF,
					pFI->m_cmemFuncName.GetStringPtr()
				);
			}else {
				auto_sprintf(szText, LSW(STR_DLGJUMP_PSLQL),
					pFI->m_nFuncLineLAYOUT,
					pFI->m_cmemFuncName.GetStringPtr()
				);
			}
			nIndex = Combo_AddString(hwndCtrl, szText);
			if (m_pShareData->m_bLineNumIsCRLF_ForJump) {	// 行番号の表示 false=折り返し単位／true=改行単位
				Combo_SetItemData(hwndCtrl, nIndex, (Int)pFI->m_nFuncLineCRLF);
			}else {
				Combo_SetItemData(hwndCtrl, nIndex, (Int)pFI->m_nFuncLineLAYOUT);
			}
			nPLSQLBlockNum++;
		}
		if (pFI->m_nInfo == 41) {
			if (m_pShareData->m_bLineNumIsCRLF_ForJump) {	// 行番号の表示 false=折り返し単位／true=改行単位
				auto_sprintf(szText, LSW(STR_DLGJUMP_PSLQL),
					pFI->m_nFuncLineCRLF,
					pFI->m_cmemFuncName.GetStringPtr()
				);
			}else {
				auto_sprintf(szText, LSW(STR_DLGJUMP_PSLQL),
					pFI->m_nFuncLineLAYOUT,
					pFI->m_cmemFuncName.GetStringPtr()
				);
			}
			nIndexCurSel = nIndex = Combo_AddString(hwndCtrl, szText);
			if (m_pShareData->m_bLineNumIsCRLF_ForJump) {	// 行番号の表示 false=折り返し単位／true=改行単位
				nWorkLine = (Int)pFI->m_nFuncLineCRLF;
				Combo_SetItemData(hwndCtrl, nIndex, (Int)pFI->m_nFuncLineCRLF);
			}else {
				nWorkLine = (Int)pFI->m_nFuncLineLAYOUT;
				Combo_SetItemData(hwndCtrl, nIndex, (Int)pFI->m_nFuncLineLAYOUT);
			}
			++nPLSQLBlockNum;
		}
	}
	Combo_SetCurSel(hwndCtrl, nIndexCurSel);

	// PL/SQLのパッケージ本体が検出された場合
	if (nWorkLine != -1) {
		m_nPLSQL_E1 = nWorkLine;
		SetItemInt(IDC_EDIT_PLSQL_E1, m_nPLSQL_E1, FALSE);
	}
	// PL/SQLのパッケージブロックが検出された場合
	if (0 < nPLSQLBlockNum) {
		m_bPLSQL = TRUE;
	}
	CheckButton(IDC_CHECK_PLSQL, m_bPLSQL);	// PL/SQLソースの有効行か
	if (IsButtonChecked(IDC_CHECK_PLSQL)) {
		EnableItem(IDC_LABEL_PLSQL1, true);	// Sept. 12, 2000 JEPRO
		EnableItem(IDC_LABEL_PLSQL2, true);	// Sept. 12, 2000 JEPRO
		EnableItem(IDC_LABEL_PLSQL3, true);	// Sept. 12, 2000 JEPRO
		EnableItem(IDC_EDIT_PLSQL_E1, true);
		EnableItem(IDC_SPIN_PLSQL_E1, true);	// Oct. 6, 2000 JEPRO
		EnableItem(IDC_COMBO_PLSQLBLOCKS, true);
		m_pShareData->m_bLineNumIsCRLF_ForJump = true;
		EnableItem(IDC_RADIO_LINENUM_LAYOUT, false);
		EnableItem(IDC_RADIO_LINENUM_CRLF, false);
	}else {
		EnableItem(IDC_LABEL_PLSQL1, false);	// Sept. 12, 2000 JEPRO
		EnableItem(IDC_LABEL_PLSQL2, false);	// Sept. 12, 2000 JEPRO
		EnableItem(IDC_LABEL_PLSQL3, false);	// Sept. 12, 2000 JEPRO
		EnableItem(IDC_EDIT_PLSQL_E1, false);
		EnableItem(IDC_SPIN_PLSQL_E1, false);	// Oct. 6, 2000 JEPRO
		EnableItem(IDC_COMBO_PLSQLBLOCKS, false);
		EnableItem(IDC_RADIO_LINENUM_LAYOUT, true);
		EnableItem(IDC_RADIO_LINENUM_CRLF, true);
	}
	// 行番号の表示 false=折り返し単位／true=改行単位
	if (m_pShareData->m_bLineNumIsCRLF_ForJump) {
		CheckButton(IDC_RADIO_LINENUM_LAYOUT, false);
		CheckButton(IDC_RADIO_LINENUM_CRLF, true);
	}else {
		CheckButton(IDC_RADIO_LINENUM_LAYOUT, true);
		CheckButton(IDC_RADIO_LINENUM_CRLF, false);
	}
	return;
}


// ダイアログデータの取得
// TRUE==正常   FALSE==入力エラー
int CDlgJump::GetData(void)
{
	BOOL pTranslated;

	// 行番号の表示 false=折り返し単位／true=改行単位
	m_pShareData->m_bLineNumIsCRLF_ForJump = !IsButtonChecked(IDC_RADIO_LINENUM_LAYOUT);

	// PL/SQLソースの有効行か
	m_bPLSQL = IsButtonChecked(IDC_CHECK_PLSQL);
	m_nPLSQL_E1 = GetItemInt(IDC_EDIT_PLSQL_E1, &pTranslated, FALSE);
	if (m_nPLSQL_E1 == 0 && !pTranslated) {
		m_nPLSQL_E1 = 1;
	}

//	m_nPLSQL_E2 = GetItemInt(IDC_EDIT_PLSQL_E2, &pTranslated, FALSE);
//	if (m_nPLSQL_E2 == 0 && !pTranslated) {
//		m_nPLSQL_E2 = 1;
//	}

	// 行番号
	m_nLineNum = GetItemInt(IDC_EDIT_LINENUM, &pTranslated, FALSE);
	if (m_nLineNum == 0 && !pTranslated) {
		return FALSE;
	}
	return TRUE;
}

//@@@ 2002.01.18 add start
LPVOID CDlgJump::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

