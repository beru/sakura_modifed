/*!	@file
	@brief 指定行へのジャンプダイアログボックス
*/
#include "StdAfx.h"
#include "dlg/DlgJump.h"
#include "doc/EditDoc.h"
#include "func/Funccode.h"
#include "outline/FuncInfo.h"
#include "outline/FuncInfoArr.h"
#include "util/shell.h"
#include "window/EditWnd.h"
#include "sakura_rc.h"
#include "sakura.hh"

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
};

DlgJump::DlgJump()
{
	nLineNum = 0;			// 行番号
	bPLSQL = FALSE;		// PL/SQLソースの有効行か
	nPLSQL_E1 = 1;
	nPLSQL_E2 = 1;

	return;
}

// モーダルダイアログの表示
INT_PTR DlgJump::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam
	)
{
	return Dialog::DoModal(hInstance, hwndParent, IDD_JUMP, lParam);
}


// 行番号入力ボックスにスピンコントロールを付けるため
// CDlgPrintSetting.cppのOnNotifyとOnSpin及びCpropComFile.cppのDispatchEvent_p2内のcase WM_NOTIFYを参考にした
BOOL DlgJump::OnNotify(WPARAM wParam, LPARAM lParam)
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


BOOL DlgJump::OnCbnSelChange(HWND hwndCtl, int wID)
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

BOOL DlgJump::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_HELP:
		//「指定行へジャンプ」のヘルプ
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_JUMP_DIALOG));
		return TRUE;
	case IDC_CHECK_PLSQL:		// PL/SQLソースの有効行か
		if (IsButtonChecked(IDC_CHECK_PLSQL)) {
			EnableItem(IDC_LABEL_PLSQL1, true);
			EnableItem(IDC_LABEL_PLSQL2, true);
			EnableItem(IDC_LABEL_PLSQL3, true);
			EnableItem(IDC_EDIT_PLSQL_E1, true);
			EnableItem(IDC_SPIN_PLSQL_E1, true);
			EnableItem(IDC_COMBO_PLSQLBLOCKS, true);
			pShareData->bLineNumIsCRLF_ForJump = true;
			EnableItem(IDC_RADIO_LINENUM_LAYOUT, false);
			EnableItem(IDC_RADIO_LINENUM_CRLF, false);
		}else {
			EnableItem(IDC_LABEL_PLSQL1, false);
			EnableItem(IDC_LABEL_PLSQL2, false);
			EnableItem(IDC_LABEL_PLSQL3, false);
			EnableItem(IDC_EDIT_PLSQL_E1, false);
			EnableItem(IDC_SPIN_PLSQL_E1, false);
			EnableItem(IDC_COMBO_PLSQLBLOCKS, false);
			EnableItem(IDC_RADIO_LINENUM_LAYOUT, true);
			EnableItem(IDC_RADIO_LINENUM_CRLF, true);
		}
		// 行番号の表示 false=折り返し単位／true=改行単位
		if (pShareData->bLineNumIsCRLF_ForJump) {
			CheckButton(IDC_RADIO_LINENUM_LAYOUT, false);
			CheckButton(IDC_RADIO_LINENUM_CRLF, true);
		}else {
			CheckButton(IDC_RADIO_LINENUM_LAYOUT, true);
			CheckButton(IDC_RADIO_LINENUM_CRLF, false);
		}
		return TRUE;
	case IDC_BUTTON_JUMP:			// 指定行へジャンプ
		// ダイアログデータの取得
//		次行から追加
		if (0 < GetData()) {
			CloseDialog(1);
		}else {
			OkMessage(GetHwnd(), LS(STR_DLGJUMP1));
		}
		{
			EditDoc* pEditDoc = (EditDoc*)lParam;
			pEditDoc->pEditWnd->GetActiveView().GetCommander().HandleCommand(F_JUMP, true, 0, 0, 0, 0);	// ジャンプコマンド発行
		}
		return TRUE;
	case IDCANCEL:
		::EndDialog(GetHwnd(), FALSE);
		return TRUE;
	}
	// 基底クラスメンバ
	return Dialog::OnBnClicked(wID);
}


// ダイアログデータの設定
void DlgJump::SetData(void)
{
	EditDoc* pEditDoc = (EditDoc*)lParam;
	FuncInfoArr funcInfoArr;
	wchar_t szText[1024];
	LONG_PTR nIndexCurSel = 0;

	if (nLineNum == 0) {
		SetItemText(IDC_EDIT_LINENUM, _T(""));	// 行番号
	}else {
		SetItemInt(IDC_EDIT_LINENUM, nLineNum, FALSE);	// 前回の行番号
	}
	SetItemInt(IDC_EDIT_PLSQL_E1, nPLSQL_E1, FALSE);
	// PL/SQL関数リスト作成
	HWND hwndCtrl = GetItemHwnd(IDC_COMBO_PLSQLBLOCKS);
	// タイプ別に設定されたアウトライン解析方法
	if (pEditDoc->docType.GetDocumentAttribute().eDefaultOutline == OutlineType::PLSQL) {
		pEditDoc->docOutline.MakeFuncList_PLSQL(&funcInfoArr);
	}
	//$$ 条件により、レイアウト・ロジックの単位が混在するため、ミスの原因になりやすい
	int nWorkLine = -1;
	LONG_PTR nIndex = 0;
	int nPLSQLBlockNum = 0;
	for (size_t i=0; i<funcInfoArr.GetNum(); ++i) {
		FuncInfo* pFI = funcInfoArr.GetAt(i);
		if (pFI->nInfo == 31 || pFI->nInfo == 41) {
		}
		if (pFI->nInfo == 31) {
			if (pShareData->bLineNumIsCRLF_ForJump) {	// 行番号の表示 false=折り返し単位／true=改行単位
				auto_sprintf(szText, LSW(STR_DLGJUMP_PSLQL),
					pFI->nFuncLineCRLF,
					pFI->memFuncName.GetStringPtr()
				);
			}else {
				auto_sprintf(szText, LSW(STR_DLGJUMP_PSLQL),
					pFI->nFuncLineLAYOUT,
					pFI->memFuncName.GetStringPtr()
				);
			}
			nIndex = Combo_AddString(hwndCtrl, szText);
			if (pShareData->bLineNumIsCRLF_ForJump) {	// 行番号の表示 false=折り返し単位／true=改行単位
				Combo_SetItemData(hwndCtrl, nIndex, pFI->nFuncLineCRLF);
			}else {
				Combo_SetItemData(hwndCtrl, nIndex, pFI->nFuncLineLAYOUT);
			}
			nPLSQLBlockNum++;
		}
		if (pFI->nInfo == 41) {
			if (pShareData->bLineNumIsCRLF_ForJump) {	// 行番号の表示 false=折り返し単位／true=改行単位
				auto_sprintf(szText, LSW(STR_DLGJUMP_PSLQL),
					pFI->nFuncLineCRLF,
					pFI->memFuncName.GetStringPtr()
				);
			}else {
				auto_sprintf(szText, LSW(STR_DLGJUMP_PSLQL),
					pFI->nFuncLineLAYOUT,
					pFI->memFuncName.GetStringPtr()
				);
			}
			nIndexCurSel = nIndex = Combo_AddString(hwndCtrl, szText);
			if (pShareData->bLineNumIsCRLF_ForJump) {	// 行番号の表示 false=折り返し単位／true=改行単位
				nWorkLine = (int)pFI->nFuncLineCRLF;
				Combo_SetItemData(hwndCtrl, nIndex, pFI->nFuncLineCRLF);
			}else {
				nWorkLine = (int)pFI->nFuncLineLAYOUT;
				Combo_SetItemData(hwndCtrl, nIndex, pFI->nFuncLineLAYOUT);
			}
			++nPLSQLBlockNum;
		}
	}
	Combo_SetCurSel(hwndCtrl, nIndexCurSel);

	// PL/SQLのパッケージ本体が検出された場合
	if (nWorkLine != -1) {
		nPLSQL_E1 = nWorkLine;
		SetItemInt(IDC_EDIT_PLSQL_E1, nPLSQL_E1, FALSE);
	}
	// PL/SQLのパッケージブロックが検出された場合
	if (0 < nPLSQLBlockNum) {
		bPLSQL = TRUE;
	}
	CheckButton(IDC_CHECK_PLSQL, bPLSQL);	// PL/SQLソースの有効行か
	if (IsButtonChecked(IDC_CHECK_PLSQL)) {
		EnableItem(IDC_LABEL_PLSQL1, true);
		EnableItem(IDC_LABEL_PLSQL2, true);
		EnableItem(IDC_LABEL_PLSQL3, true);
		EnableItem(IDC_EDIT_PLSQL_E1, true);
		EnableItem(IDC_SPIN_PLSQL_E1, true);
		EnableItem(IDC_COMBO_PLSQLBLOCKS, true);
		pShareData->bLineNumIsCRLF_ForJump = true;
		EnableItem(IDC_RADIO_LINENUM_LAYOUT, false);
		EnableItem(IDC_RADIO_LINENUM_CRLF, false);
	}else {
		EnableItem(IDC_LABEL_PLSQL1, false);
		EnableItem(IDC_LABEL_PLSQL2, false);
		EnableItem(IDC_LABEL_PLSQL3, false);
		EnableItem(IDC_EDIT_PLSQL_E1, false);
		EnableItem(IDC_SPIN_PLSQL_E1, false);
		EnableItem(IDC_COMBO_PLSQLBLOCKS, false);
		EnableItem(IDC_RADIO_LINENUM_LAYOUT, true);
		EnableItem(IDC_RADIO_LINENUM_CRLF, true);
	}
	// 行番号の表示 false=折り返し単位／true=改行単位
	if (pShareData->bLineNumIsCRLF_ForJump) {
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
int DlgJump::GetData(void)
{
	BOOL pTranslated;

	// 行番号の表示 false=折り返し単位／true=改行単位
	pShareData->bLineNumIsCRLF_ForJump = !IsButtonChecked(IDC_RADIO_LINENUM_LAYOUT);

	// PL/SQLソースの有効行か
	bPLSQL = IsButtonChecked(IDC_CHECK_PLSQL);
	nPLSQL_E1 = GetItemInt(IDC_EDIT_PLSQL_E1, &pTranslated, FALSE);
	if (nPLSQL_E1 == 0 && !pTranslated) {
		nPLSQL_E1 = 1;
	}

	// 行番号
	nLineNum = GetItemInt(IDC_EDIT_LINENUM, &pTranslated, FALSE);
	if (nLineNum == 0 && !pTranslated) {
		return FALSE;
	}
	return TRUE;
}

LPVOID DlgJump::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

